/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: HierarchicalZ.cpp,v $
 * $Revision: 1.23 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:52 $
 *
 * Hierarchical Z implementation file.
 *
 */


/**
 *
 *  @file HierarchicalZ.cpp
 *
 *  This file implements the Hierarchical Z class.
 *
 *  The Hierarchical Z class implements the Hierarchical Z
 *  simulation box.
 *
 */

#include "HierarchicalZ.h"
#include "RasterizerCommand.h"
#include "FFIFOStateInfo.h"
#include "RasterizerStateInfo.h"
#include "HZStateInfo.h"
#include "HZAccess.h"
#include "HZUpdate.h"
#include "GPUMath.h"

#include <fstream>
#include <sstream>

using namespace gpu3d;

/*  HierarchicalZ constructor.  */
HierarchicalZ::HierarchicalZ(u32bit stampCycle, u32bit overWidth, u32bit overHeight,
    u32bit scanWidth, u32bit scanHeight, u32bit genWidth, u32bit genHeight, bool disabHZ, u32bit stampBlock,
    u32bit hzSize, u32bit cacheLines, u32bit cacheLineSize, u32bit queueSize, u32bit hzBufferLat,
    u32bit hzUpdateLat, u32bit clearBlocks, u32bit nStampUnits, char **suPrefixes, bool microTrisAsFrag,
    u32bit microTriSzLimit, char *name, Box *parent) :

    stampsCycle(stampCycle), overH(overHeight), overW(overWidth),
    scanH((u32bit) ceil(scanHeight / (f32bit) genHeight)),
    scanW((u32bit) ceil(scanWidth / (f32bit) genWidth)), genH(genHeight / STAMP_HEIGHT), genW(genWidth / STAMP_WIDTH),
    disableHZ(disabHZ), blockStamps(stampBlock), hzBufferSize(hzSize),
    hzCacheLines(cacheLines), hzCacheLineSize(cacheLineSize), hzQueueSize(queueSize), hzBufferLatency(hzBufferLat),
    hzUpdateLatency(hzUpdateLat), clearBlocksCycle(clearBlocks), numStampUnits(nStampUnits), 
    Box(name, parent)

{
    DynamicObject *defaultState[1];
    u32bit i;
    string queueName;

    /*  Check parameters.  */
    GPU_ASSERT(
        if (stampsCycle >= (2 * hzQueueSize))
            panic("HierarchicalZ", "HierarchicalZ", "HZ fragment queue must at least store two cycles of max input fragments.");
    )

    /*  Create statistics.  */
    inputs = &getSM().getNumericStatistic("InputFragments", u32bit(0), "HierarchicalZ", "HZ");
    outputs = &getSM().getNumericStatistic("OutputFragments", u32bit(0), "HierarchicalZ", "HZ");
    outTriangle = &getSM().getNumericStatistic("OutsideTriangleFragments", u32bit(0), "HierarchicalZ", "HZ");
    outViewport = &getSM().getNumericStatistic("OutsideViewportFragments", u32bit(0), "HierarchicalZ", "HZ");
    culled = &getSM().getNumericStatistic("CulledFragments", u32bit(0), "HierarchicalZ", "HZ");
    cullOutside = &getSM().getNumericStatistic("CulledOutsideFragments", u32bit(0), "HierarchicalZ", "HZ");
    cullHZ = &getSM().getNumericStatistic("CulledHZFragments", u32bit(0), "HierarchicalZ", "HZ");
    misses = &getSM().getNumericStatistic("MissesHZCache", u32bit(0), "HierarchicalZ", "HZ");
    hits = &getSM().getNumericStatistic("HitsHZCache", u32bit(0), "HierarchicalZ", "HZ");
    reads = &getSM().getNumericStatistic("ReadsHZBuffer", u32bit(0), "HierarchicalZ", "HZ");
    writes = &getSM().getNumericStatistic("WritesHZBuffer", u32bit(0), "HierarchicalZ", "HZ");


    /*  Create Hierarchical Z signals.  */

    /*  Create signals with Fragment Generation.  */

    /*  Fragment input signal from Fragment Generation box.  */
    inputStamps = newInputSignal("HZInputFragment", stampsCycle * STAMP_FRAGMENTS, 1, NULL);

    /*  Create state signal to the Fragment Generation box.  */
    hzTestState = newOutputSignal("HZTestState", 1, 1, NULL);

    /*  Create default state signal value.  */
    defaultState[0] = new HZStateInfo(HZST_READY);

    /*  Set default signal value.  */
    hzTestState->setData(defaultState);

    /*  Create signals with Fragment FIFO box.  */

    /*  Create output stamp signal to Interpolator box.  */
    outputStamps = newOutputSignal("HZOutput", stampsCycle * STAMP_FRAGMENTS, 1, NULL);

    /*  Create state signal from the Fragment FIFO.  */
    fFIFOState = newInputSignal("FFIFOStateHZ", 1, 1, NULL);


    /*  Create signals with the main rasterizer box.  */

    /*  Create command signal from the main rasterizer box.  */
    hzCommand = newInputSignal("HZCommand", 1, 1, NULL);

    /*  Create state signal to the main rasterizer box.  */
    hzState = newOutputSignal("HZState", 1, 1, NULL);

    /*  Create default signal value.  */
    defaultState[0] = new RasterizerStateInfo(RAST_RESET);

    /*  Set default signal value.  */
    hzState->setData(defaultState);


    /*  Create signals with the Z Test box.  */

    /*  Check number of attached Z units.  */
    GPU_ASSERT(
        if (numStampUnits < 1)
            panic("HierarchicalZ", "HierarchicalZ", "At least one Z Stencil unit must be attached.");
        if (suPrefixes == NULL)
            panic("HierarchicalZ", "HierarchicalZ", "Z Stencil prefix array required.");
        for(i = 0; i < numStampUnits; i++)
            if (suPrefixes[i] == NULL)
                panic("HierarchicalZ", "HierarchicalZ", "Z Stencil prefix missing.");
    )

    /*  Allocate the array of update signals from the Z Stencil units.  */
    hzUpdate = new Signal*[numStampUnits];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (hzUpdate == NULL)
            panic("HierarchicalZ", "HierarchicalZ", "Error allocating array of update signals from Z Stencil.");
    )

    /*  Create update signal with all the attached Z Stencil Test boxes.  */
    for(i = 0; i < numStampUnits; i++)
    {
        /*  Create update signal from Z Test box.  */
        hzUpdate[i] = newInputSignal("HZUpdate", 1, hzUpdateLatency, suPrefixes[i]);
    }


    /*  Create internal HZ buffer access signals.  */

    /*  NOTE:  Bandwidth of HZ Buffer write signal is one per Z Stencil unit attached
        (update signal) and one for reads for the Hierarchical Z test.  */

    /*  Create write signal.  */
    hzBufferWrite = newInputSignal("HZBuffer", numStampUnits + 1, hzBufferLatency, NULL);

    /*  Create read signal.  */
    hzBufferRead = newOutputSignal("HZBuffer", numStampUnits + 1, hzBufferLatency, NULL);


    /*  Allocate the first level hierarchical Z buffer.  */
    hzLevel0 = new u32bit[hzBufferSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (hzLevel0 == NULL)
            panic("HierarchicalZ", "HierarchicalZ", "Error allocating Hierarchical Z Buffer Level 0.");
    )

    /*  Allocate the hz cache.  */
    hzCache = new HZCacheLine[hzCacheLines];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (hzCache == NULL)
            panic("HierarchicalZ", "HierarchicalZ", "Error allocation Hierarchical Z Cache.");
    )

    /*  Create the HZ Cache lines.  */
    for(i = 0; i <  hzCacheLines; i++)
    {
        /*  Allocate HZ Cache line z values.  */
        hzCache[i].z = new u32bit[hzCacheLineSize];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (hzCache[i].z == NULL)
                panic("HierarchicalZ", "HierarchicalZ", "Error allocation Hierarchical Z Cache line.");
        )
    }

    /*  Allocate the stamp queue for the early z test.  */
    hzQueue = new HZQueue[hzQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (hzQueue == NULL)
            panic("HierarchicalZ", "HierarchicalZ", "Error allocating early test stamp queue.");
    )

    //  Rename MicroStamp Queue.
    queueName.clear();
    queueName.append(name);
    queueName.append("::MicroStampQueue");

    /*  Calculate block address shift.  */
    blockShift = GPUMath::calculateShift(blockStamps * STAMP_FRAGMENTS);

    //  Precalculate the size in depth elements (32 bits) of a HZ block.
    blockSize = blockStamps * STAMP_FRAGMENTS;

    //  Precalculate block size mask.
    blockSizeMask = GPUMath::buildMask(blockSize);
        
    /*  Precalculate block inside HZ cache mask.  */
    hzBlockMask = GPUMath::buildMask(hzCacheLineSize);

    /*  Precalculate HZ cache line mask.  */
    hzLineMask = ~hzBlockMask;

    /*  Create dummy last rasterizer command.  */
    lastRSCommand = new RasterizerCommand(RSCOM_RESET);

    /*  Set initial state to reset.  */
    state = RAST_RESET;
}


/*  Simulation function.  */
void HierarchicalZ::clock(u64bit cycle)
{
    RasterizerCommand *rastCommand;
    FFIFOStateInfo *ffStateInfo;
    FFIFOState ffState;
    HZUpdate *blockUpdate;
    HZAccess *hzOperation;
    u32bit cacheEntry;
    u32bit i;
    u32bit j;

    GPU_DEBUG_BOX(
        printf("HierarchicalZ => Clock %lld.\n", cycle);
    )

    /*  Receive state from the Interpolator box.  */
    if (fFIFOState->read(cycle, (DynamicObject *&) ffStateInfo))
    {
        /*  Get Interpolator state.  */
        ffState = ffStateInfo->getState();

        /*  Delete Interpolator state info object.  */
        delete ffStateInfo;
    }
    else
    {
        panic("HierarchicalZ", "clock", "Missing state signal from the Interpolator box.");
    }


    /*  Reset HZ Level 0 memory data bus.  */
    dataBus = TRUE;


    /*  Update Hierarchical Z level 0.  */
    for(i = 0; (i < numStampUnits) && (!disableHZ); i++)
    {
        if (hzUpdate[i]->read(cycle, (DynamicObject *&) blockUpdate))
        {
            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Received update request for block %x from Z Stencil Test unit %d with value %08x.\n",
                    blockUpdate->getBlockAddress(), i, blockUpdate->getBlockZ());
            )

            /*  Send write to the HZ Level 0 buffer.  */
            hzBufferWrite->write(cycle, new HZAccess(HZ_WRITE, blockUpdate->getBlockAddress(), blockUpdate->getBlockZ()));

            /*  Block data bus.  */
            dataBus = FALSE;

            /*  Delete update object.  */
            delete blockUpdate;
        }
    }

    /*  Process HZ Buffer Level 0 accesses.  */
    while ((!disableHZ) && hzBufferRead->read(cycle, (DynamicObject *&) hzOperation))
    {
        /*  Select access type.  */
        switch(hzOperation->getOperation())
        {
            case HZ_READ:

                GPU_DEBUG_BOX(
                    printf("HierarchicalZ => Received HZ Buffer read request for block %x.\n",
                        hzOperation->getAddress());
                )

                /*  Read the HZ buffer and fill the line in the HZ Cache.  */
                for(i = 0; i < hzCacheLineSize; i++)
                    hzCache[hzOperation->getCacheEntry()].z[i] = hzLevel0[hzOperation->getAddress() + i];

                /*  Set hz queue entry read flag.  */
                hzCache[hzOperation->getCacheEntry()].read = TRUE;

                /*  Update statistics.  */
                reads->inc();

                break;

            case HZ_WRITE:

                GPU_DEBUG_BOX(
                    printf("HierarchicalZ => Received HZ Buffer write request for block %x <- %x.\n",
                        hzOperation->getAddress(), hzOperation->getData());
                )

                /*  Update the HZ buffer.  */
                hzLevel0[hzOperation->getAddress()] = hzOperation->getData();

                /*  Update statistics.  */
                writes->inc();

                break;
        }

        /*  Delete HZ Access object.  */
        delete hzOperation;
    }

    /*  Simulate current cycle.  */
    switch(state)
    {
        case RAST_RESET:

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => RESET state.\n");
            )

            /*  Reset state.  */

            /*  Reset the box state.  */
            hRes = 400;
            vRes = 400;
            startX = 0;
            startY = 0;
            width = 400;
            height = 400;
            scissorTest = false;
            scissorIniX = 0;
            scissorIniY = 0;
            scissorWidth = 400;
            scissorHeight = 400;
            //hzEnabled = TRUE;
            hzEnabled = FALSE;
            modifyDepth = FALSE;
            clearDepth = 0x00ffffff;
            zBufferPrecission = 24;
            depthFunction = GPU_LESS;
            msaaSamples = 2;
            multisampling = false;
            

            /*  Reset the HZ cache.  */
            for(i = 0; i < hzCacheLines; i++)
            {
                hzCache[i].block = 0;
                hzCache[i].read = false;
                hzCache[i].valid = false;
            }

            /*  Initialize the early z test queue counters.  */
            nextFree = 0;
            nextRead = 0;
            nextTest = 0;
            nextSend = 0;
            freeHZQE = hzQueueSize;
            readHZQE = 0;
            testHZQE = 0;
            sendHZQE = 0;

            /*  Change to ready state.  */
            state = RAST_READY;

            break;

        case RAST_READY:

            /*  Ready state.  */

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => READY state.\n");
            )

            /*  Process commands from the Rasterizer.  */
            if (hzCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand);

            break;

        case RAST_DRAWING:

            /*  Draw state.  */

            /*  Check end of the batch.  */
            if (lastFragment && (freeHZQE == hzQueueSize) && (ffState == FFIFO_READY))
            {
                GPU_DEBUG_BOX(
                    printf("HierarchicalZ => Sending LAST FRAGMENT to Fragment FIFO.\n");
                )

                /*  Send last stamp/fragment to Interpolator.  */
                for(j = 0; j < STAMP_FRAGMENTS; j++)
                {
                    /*  Send fragment to the Interpolator.  */
                    outputStamps->write(cycle, hzQueue[nextSend].stamp[j]);

                    /*  Update statistics.  */
                    outputs->inc();
                }

                /*  Delete last stamp.  */
                delete[] hzQueue[nextSend].stamp;

                /*  Change to END state.  */
                state = RAST_END;
            }

            //  Check if there are free queue entries.
            if (freeHZQE >= stampsCycle)
            {
                //  Receive stamps from Fragment Generation.
                receiveStamps(cycle);
            }

            //  Read block Z value for stamps.
            for(i = 0; (i < stampsCycle) && (readHZQE > 0); i++)
            {
                //  Search the stamp block in the HZ cache.  */
                if (searchHZCache(hzQueue[nextRead].block[hzQueue[nextRead].currentBlock], cacheEntry))
                {
                    GPU_DEBUG_BOX(
                        printf("HierarchicalZ => Block %x for stamp at %d found at %d.\n",
                            hzQueue[nextRead].block[hzQueue[nextRead].currentBlock], nextRead, cacheEntry);
                    )

                    //  Set stamp HZ cache entry.  */
                    hzQueue[nextRead].cache[hzQueue[nextRead].currentBlock] = cacheEntry;

                    //  Update the current block being requested to the HZ buffer.
                    hzQueue[nextRead].currentBlock++;
                    
                    //  Check if all the blocks for the stamp where requested.
                    if (hzQueue[nextRead].currentBlock == hzQueue[nextRead].numBlocks)
                    {                  
                        //  Update number of read queue entries.
                        readHZQE--;

                        //  Update number of test queue entries.
                        testHZQE++;

                        //  Reset pointer to stamp block being processed.
                        hzQueue[nextRead].currentBlock = 0;

                        //  Update pointer to the next read queue entry.
                        nextRead = GPU_MOD(nextRead + 1, hzQueueSize);
                    }
                    
                    //  Update statistics.
                    hits->inc();
                }
                else
                {
                    //  Check if the HZ Level 0 buffer can be accessed in the current cycle.
                    if (dataBus)
                    {
                        //  Add a request to the HZ cache.
                        if (insertHZCache(hzQueue[nextRead].block[hzQueue[nextRead].currentBlock], cacheEntry))
                        {
                            GPU_DEBUG_BOX(
                                printf("HierarchicalZ => Read block %x into entry %d for stamp at %d.\n",
                                    hzQueue[nextRead].block[hzQueue[nextRead].currentBlock], cacheEntry, nextRead);
                            )

                            //  Send the HZ buffer read request through the read signal.
                            hzBufferWrite->write(cycle, new HZAccess(HZ_READ,
                                        hzQueue[nextRead].block[hzQueue[nextRead].currentBlock] & hzLineMask, cacheEntry));

                            //  Set stamp HZ cache entry.
                            hzQueue[nextRead].cache[hzQueue[nextRead].currentBlock] = cacheEntry;


                            //  Update the current block being requested to the HZ buffer.
                            hzQueue[nextRead].currentBlock++;
                            
                            //  Check if all the blocks for the stamp where requested.
                            if (hzQueue[nextRead].currentBlock == hzQueue[nextRead].numBlocks)
                            {                  
                                //  Update number of read queue entries.
                                readHZQE--;

                                //  Update number of test queue entries.
                                testHZQE++;

                                //  Reset point to the next stamp block to be processed.
                                hzQueue[nextRead].currentBlock = 0;
                                
                                //  Update pointer to the next read queue entry.
                                nextRead = GPU_MOD(nextRead + 1, hzQueueSize);
                            }
                            
                            //  Only one access per cycle supported.
                            dataBus = FALSE;
                        }
                    }

                    //  Update statistics.
                    misses->inc();
                }
            }


            /*  Send stamps to the Fragment FIFO.  */
            /*  Check if the Fragment FIFO can receive stamps.  */
            if (ffState == FFIFO_READY)
            {
                for(i = 0; (i < stampsCycle) && (sendHZQE > 0);)
                {
                    /*  Check if the stamp has been culled.  */
                    if (!hzQueue[nextSend].culled)
                    {
                        GPU_DEBUG_BOX(
                            printf("HierarchicalZ => Sending stamp at %d to Fragment FIFO.\n", nextSend);
                        )

                        /*  Send stamp to the Fragment FIFO.  */
                        for(j = 0; j < STAMP_FRAGMENTS; j++)
                        {
                            /*  Send fragment to the Fragment FIFO.  */
                            outputStamps->write(cycle, hzQueue[nextSend].stamp[j]);

                            /*  Update statistics.  */
                            outputs->inc();
                        }

                        /*  Update number of stamps issued in current cycle.  */
                        i++;
                    }
                    else
                    {
                        GPU_DEBUG_BOX(
                            printf("HierarchicalZ => Culling stamp at %d.\n", nextSend);
                        )

                        /*  Delete stamp fragments.  */
                        for(j = 0; j < STAMP_FRAGMENTS; j++)
                        {
                            /*  Update statistics.  */
                            culled->inc();

                            /*  First delete the Fragment object.  */
                            delete hzQueue[nextSend].stamp[j]->getFragment();

                            /*  Delete the FragmentInput object.  */
                            delete hzQueue[nextSend].stamp[j];

                        }
                    }

                    /*  Update number of stamps to send to Fragment FIFO.  */
                    sendHZQE--;

                    /*  Update number of free queue entries.  */
                    freeHZQE++;

                    /*  Delete the stamp.  */
                    delete[] hzQueue[nextSend].stamp;

                    /*  Update pointer to the next stamp to send in the queue.  */
                    nextSend = GPU_MOD(nextSend + 1, hzQueueSize);
                }
            }

            //  Early Z stamp test.
            for(i = 0; (i < stampsCycle) && (testHZQE > 0); i++)
            {
                //  Check if the stamp block Z has been read.
                if (hzCache[hzQueue[nextTest].cache[hzQueue[nextTest].currentBlock]].read)
                {                              
                    u32bit blockZ;

                    //  Read the block depth value from the cache.                                        
                    blockZ = hzCache[hzQueue[nextTest].cache[hzQueue[nextTest].currentBlock]].z[hzQueue[nextTest].block[hzQueue[nextTest].currentBlock] & hzBlockMask];
                    
                    //  Update block depth value for the stamp.
                    hzQueue[nextTest].blockZ = GPU_MAX(hzQueue[nextTest].blockZ, blockZ);
                    
                    //  Update cache reserve counter.
                    hzCache[hzQueue[nextTest].cache[hzQueue[nextTest].currentBlock]].reserves--;

                    //  Update the number of stamp block values read.
                    hzQueue[nextTest].currentBlock++;
                    
                    //  Check if all the blocks required for the stamp where read.
                    if (hzQueue[nextTest].currentBlock == hzQueue[nextTest].numBlocks)
                    {
                        GPU_DEBUG_BOX(
                            printf("HierarchicalZ => Testing stamp at %d.  Block Z (%x) = %x | Stamp Z = %x.\n",
                                nextTest, hzQueue[nextTest].block[0], hzQueue[nextTest].blockZ, hzQueue[nextTest].stampZ);
                            printf("HZ %lld => hzBlockMask %d block %d cache line offset %d line %d\n", cycle,
                                hzBlockMask, hzQueue[nextTest].block[0], hzQueue[nextTest].block[0] & hzBlockMask,
                                hzQueue[nextTest].cache[0]);
                        )

                        //  Test and cull the stamp.
                        if (!hzCompare(hzQueue[nextTest].stampZ, hzQueue[nextTest].blockZ))
                        {
                            GPU_DEBUG_BOX(
                                printf("HierarchicalZ => Stamp culled.\n");
                            )

                            //  Cull the stamp.
                            hzQueue[nextTest].culled = TRUE;

                            //  Update statistics.
                            cullHZ->inc();
                        }

                        //  Update number of stamps to test.
                        testHZQE--;

                        //  Update number of stamps to send.
                        sendHZQE++;

                        //  Update pointer to the next test stamp.
                        nextTest = GPU_MOD(nextTest + 1, hzQueueSize);
                    }
                }
            }

            break;

        case RAST_END:

            /*  End state.  */
            GPU_DEBUG_BOX(
                printf("HierarchicalZ => END state.\n");
            )

            /*  Wait for END command from the Rasterizer.  */
            if (hzCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand);


            break;

        case RAST_CLEAR:

            /*  Clear HZ buffer state.  */

            /*  Check if clear has finished.  */
            if (clearCycles > 0)
            {
                /*  Update remaining clear cycles.  */
                clearCycles--;
            }
            else
            {
                /*  Clear the HZ buffer.  */
                for(i = 0; i < hzBufferSize; i++)
                    hzLevel0[i] = clearDepth & 0x00ffffff;

                /*  Reset the HZ cache.  */
                for(i = 0; i < hzCacheLines; i++)
                {
                    hzCache[i].block = 0;
                    hzCache[i].valid = FALSE;
                }

                /*  Change to end state.  */
                state = RAST_CLEAR_END;
            }

            break;

        case RAST_CLEAR_END:

            /*  End state.  */
            GPU_DEBUG_BOX(
                printf("HierarchicalZ => CLEAR_END state.\n");
            )

            /*  Wait for END command from the Rasterizer.  */
            if (hzCommand->read(cycle, (DynamicObject *&) rastCommand))
                processCommand(rastCommand);


            break;

        default:
            panic("HierarchicalZ", "clock", "Unsupported HierarchicalZ state.");
            break;
    }

    /*  Send stamp queues state to the Triangle Traversal box.  */
    if (freeHZQE > (2 * stampsCycle) )
    {
        GPU_DEBUG_BOX(
            printf("HierarchicalZ => Sending READY.\n");
        )

        hzTestState->write(cycle, new HZStateInfo(HZST_READY));
    }
    else
    {
        GPU_DEBUG_BOX(
            printf("HierarchicalZ => Sending BUSY.\n");
        )

        hzTestState->write(cycle, new HZStateInfo(HZST_BUSY));
    }

    /*  Send state to the main rasterizer box.  */
    hzState->write(cycle, new RasterizerStateInfo(state));
}


/*  Processes a rasterizer command.  */
void HierarchicalZ::processCommand(RasterizerCommand *command)
{
    u32bit samples;
    
    /*  Delete last command.  */
    delete lastRSCommand;

    /*  Store current command as last received command.  */
    lastRSCommand = command;

    /*  Process command.  */
    switch(command->getCommand())
    {
        case RSCOM_RESET:
            /*  Reset command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => RESET command received.\n");
            )

            /*  Change to reset state.  */
            state = RAST_RESET;

            break;

        case RSCOM_DRAW:
            /*  Draw command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => DRAW command received.\n");
            )


            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("HierarchicalZ", "processCommand", "DRAW command can only be received in READY state.");
            )

            /*  Initialize the early z test queue counters.  */
            nextFree = 0;
            nextRead = 0;
            nextTest = 0;
            nextSend = 0;
            freeHZQE = hzQueueSize;
            readHZQE = 0;
            testHZQE = 0;
            sendHZQE = 0;

            /*  Last fragment not received.  */
            lastFragment = FALSE;

            /*  Reset fragment counter.  */
            fragmentCounter = 0;

            /*  Calculate the scissor bounding box.  */
            if (scissorTest)
            {
                /*  Bound the scissor box to the render window.  */
                scissorMinX = GPU_MAX(scissorIniX, 0);
                scissorMaxX = GPU_MIN(scissorIniX + s32bit(scissorWidth), s32bit(hRes));
                scissorMinY = GPU_MAX(scissorIniY, 0);
                scissorMaxY = GPU_MIN(scissorIniY + s32bit(scissorHeight), s32bit(vRes));
            }
            else
            {
                /*  Use the whole render window as the scissor box.  */
                //scissorMinX = 0;
                //scissorMaxX = hRes;
                //scissorMinY = 0;
                //scissorMaxY = vRes;
                
                //  NOTE: Change to match emulator.  Cull to the viewport.
                scissorMinX = GPU_MAX(startX, 0);
                scissorMaxX = GPU_MIN(startX + s32bit(width), s32bit(hRes));
                scissorMinY = GPU_MAX(startY, 0);
                scissorMaxY = GPU_MIN(startY + s32bit(height), s32bit(vRes));
            }

            //  Set display in the pixel mapper.
            samples = multisampling ? msaaSamples : 1;
            pixelMapper.setupDisplay(hRes, vRes, STAMP_WIDTH, STAMP_WIDTH, genW, genH, scanW, scanH, overW, overH, samples, 1);
        
//printf("HZ => Scissor box %d %d x %d %d\n", scissorMinX, scissorMinY, scissorMaxX, scissorMaxY);

            /*  Change to drawing state.  */
            state = RAST_DRAWING;

            break;

        case RSCOM_END:
            /*  End command received from Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => END command received.\n");
            )


            /*  Check current state.  */
            GPU_ASSERT(
                if ((state != RAST_END) && (state != RAST_CLEAR_END))
                    panic("HierarchicalZ", "processCommand", "END command can only be received in END state.");
            )

            /*  Change to ready state.  */
            state = RAST_READY;

            break;

        case RSCOM_CLEAR_ZSTENCIL_BUFFER:
            /*  Clear HZ buffer command.  */

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => CLEAR_ZSTENCIL_BUFFER command received.\n");
            )

            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("HierarchicalZ", "processCommand", "CLEAR_ZSTENCIL_BUFFER command can only be received in READY state.");
            )

            /*  Calculate clear cycles.  */
            clearCycles = (u32bit) ceil((((f32bit) (hRes * vRes)) / ((f32bit) (STAMP_FRAGMENTS * blockStamps))) / clearBlocksCycle);

            /*  Change to clear state.  */
            state = RAST_CLEAR;

            break;

        case RSCOM_REG_WRITE:
            /*  Write register command from the Rasterizer main box.  */

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => REG_WRITE command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("HierarchicalZ", "processCommand", "REGISTER WRITE command can only be received in READY state.");
            )

            /*  Process register write.  */
            processRegisterWrite(command->getRegister(),
                command->getSubRegister(), command->getRegisterData());

            break;

        default:
            panic("HierarchicalZ", "proccessCommand", "Unsupported command.");
            break;
    }
}

void HierarchicalZ::processRegisterWrite(GPURegister reg, u32bit subreg,
    GPURegData data)
{
    /*  Process register write.  */
    switch(reg)
    {
        case GPU_DISPLAY_X_RES:
            /*  Write display horizontal resolution register.  */
            hRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_DISPLAY_X_RES = %d.\n", hRes);
            )

            break;

        case GPU_DISPLAY_Y_RES:
            /*  Write display vertical resolution register.  */
            vRes = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_DISPLAY_Y_RES = %d.\n", vRes);
            )

            break;

        case GPU_VIEWPORT_INI_X:
            /*  Write viewport initial x coordinate register.  */
            startX = data.intVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_VIEWPORT_INI_X = %d.\n", startX);
            )

            break;

        case GPU_VIEWPORT_INI_Y:
            /*  Write viewport initial y coordinate register.  */
            startY = data.intVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_VIEWPORT_INI_Y = %d.\n", startY);
            )

            break;

        case GPU_VIEWPORT_WIDTH:
            /*  Write viewport width register.  */
            width = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_VIEWPORT_WIDTH = %d.\n", width);
            )

            break;

        case GPU_VIEWPORT_HEIGHT:
            /*  Write viewport height register.  */
            height = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_VIEWPORT_HEIGHT = %d.\n", height);
            )

            break;

        case GPU_SCISSOR_TEST:
            /*  Write scissor test enable flag.  */
            scissorTest = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_SCISSOR_TEST = %s.\n", scissorTest?"TRUE":"FALSE");
            )

            break;

        case GPU_SCISSOR_INI_X:
            /*  Write scissor left most X coordinate.  */
            scissorIniX = data.intVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_SCISSOR_INI_X = %d.\n", scissorIniX);
            )

            break;

        case GPU_SCISSOR_INI_Y:
            /*  Write scissor bottom most Y coordinate.  */
            scissorIniY = data.intVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_SCISSOR_INI_Y = %d.\n", scissorIniY);
            )

            break;

        case GPU_SCISSOR_WIDTH:
            /*  Write scissor width regsiter.  */
            scissorWidth = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_SCISSOR_WIDTH = %d.\n", scissorWidth);
            )

            break;

        case GPU_SCISSOR_HEIGHT:
            /*  Write scissor height register.  */
            scissorHeight = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_SCISSOR_HEIGHT = %d.\n", scissorHeight);
            )

            break;

        case GPU_Z_BUFFER_CLEAR:
            /*  Write depth clear value.  */
            clearDepth = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_Z_BUFFER_CLEAR = %x.\n", clearDepth);
            )

            break;

        case GPU_Z_BUFFER_BIT_PRECISSION:

            /*  Write z buffer bit precission.  */

            zBufferPrecission = data.uintVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Writing register GPU_Z_BUFFER_BIT_PRECISSION = %d.\n",
                    zBufferPrecission);
            )

            break;

        case GPU_HIERARCHICALZ:

            /*  Write early HZ test enable flag.  */

            hzEnabled = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Writing register GPU_HIERARCHICALZ = %s.\n",
                    hzEnabled?"TRUE":"FALSE");
            )

            break;

        case GPU_MODIFY_FRAGMENT_DEPTH:

            /*  Write fragment shader modifies depth flag.  */
            modifyDepth = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Writing register GPU_MODIFY_FRAGMENT_DEPTH = %s.\n",
                    modifyDepth?"TRUE":"FALSE");
            )

            break;

        case GPU_DEPTH_FUNCTION:

            /*  Write depth compare function register.  */
            depthFunction = data.compare;

            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_DEPTH_FUNCTION = ");

                switch(depthFunction)
                {
                    case GPU_NEVER:
                        printf("NEVER.\n");
                        break;

                    case GPU_ALWAYS:
                        printf("ALWAYS.\n");
                        break;

                    case GPU_LESS:
                        printf("LESS.\n");
                        break;

                    case GPU_LEQUAL:
                        printf("LEQUAL.\n");
                        break;

                    case GPU_EQUAL:
                        printf("EQUAL.\n");
                        break;

                    case GPU_GEQUAL:
                        printf("GEQUAL.\n");
                        break;

                    case GPU_GREATER:
                        printf("GREATER.\n");
                        break;

                    case GPU_NOTEQUAL:
                        printf("NOTEQUAL.\n");

                    default:
                        panic("HierarchicalZ", "processRegisterWrite", "Undefined compare function mode");
                        break;
                }
            )

            break;

        case GPU_MULTISAMPLING:
        
            //  Write Multisampling enable flag.
            multisampling = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_MULTISAMPLING = %s\n", multisampling?"TRUE":"FALSE");
            )
                       
            break;
            
        case GPU_MSAA_SAMPLES:
        
            //  Write MSAA z samples per fragment register.
            msaaSamples = data.uintVal;
            
            GPU_DEBUG_BOX(
                printf("HierarchicalZ => Write GPU_MSAA_SAMPLES = %d\n", msaaSamples);
            )

            break;            

        default:
            panic("HierarchicalZ", "processRegisterWrite", "Unsupported register.");
            break;
    }

}

void HierarchicalZ::receiveStamps(u64bit cycle)
{
    FragmentInput **stamp;
    bool receivedFragment;
    FragmentInput *frInput;
    Fragment *fr;
    u32bit minZ;
    u32bit i;
    u32bit j;
    s32bit x;
    s32bit y;
    bool cullStamp;
    StampInput stampInput;

    /*  Receive stamps from Fragment Generation.  */
    for(i = 0; i < stampsCycle; i++)
    {
        /*  Allocate the current stamp.  */
        stamp = new FragmentInput*[STAMP_FRAGMENTS];

        /*  Reset received fragment flag.  */
        receivedFragment = TRUE;

        /*  Receive fragments in a stamp.  */
        for(j = 0; (j < STAMP_FRAGMENTS) && receivedFragment; j++)
        {
            /*  Get fragment from Fragment Generation.  */
            receivedFragment = inputStamps->read(cycle, (DynamicObject *&) frInput);

            /*  Check if a fragment has been received.  */
            if (receivedFragment)
            {
                /*  Store fragment in the current stamp.  */
                stamp[j] = frInput;

                /*  Update fragment counter.  */
                fragmentCounter++;

                /*  Update statistics.  */
                inputs->inc();
            }
        }

        /*  Check if a whole stamp has been received.  */
        GPU_ASSERT(
            if (!receivedFragment && (j > 1))
                panic("HierarchicalZ", "receiveStamps", "Missing fragments in a stamp.");
        )

        /*  Check if a stamp has been received.  */
        if (receivedFragment)
        {
            //  Check if it is the last fragment stamp.
            if (stamp[0]->getFragment() != NULL)
            {
                /*  Set stamp cull bits.  */
                for(j = 0, cullStamp = TRUE; j < STAMP_FRAGMENTS; j++)
                {
                    /*  Get stamp fragment.  */
                    fr = stamp[j]->getFragment();

                    /*  Check if fragment is inside the triangle.  */
                    if (fr->isInsideTriangle())
                    {
                        /*  Get the fragment coordinates.  */
                        x = fr->getX();
                        y = fr->getY();

                        /*  Determine if the fragment is inside the current viewport.  */
                        if ((x >= scissorMinX) && (x < scissorMaxX) && (y >= scissorMinY) && (y < scissorMaxY))
                        {
                            GPU_DEBUG_BOX(
                                printf("HierarchicalZ => Fragment %d in received stamp at (%d, %d) inside scissor box.\n",
                                    j, x, y);
                            )

                            /*  Set fragment as not culled.  */
                            stamp[j]->setCull(FALSE);

                            /*  Update cull stamp flag.  */
                            cullStamp = FALSE;
                        }
                        else
                        {
                            GPU_DEBUG_BOX(
                                printf("HierarchicalZ => Fragment %d in received stamp at (%d, %d) outside scissor box.  Set fragment as culled.\n",
                                    j, x, y);
                            )

                            /*  Set fragment as culled.  */
                            stamp[j]->setCull(TRUE);

                            /*  Update cull stamp flag.  */
                            cullStamp = cullStamp && TRUE;

                            /*  Update statistics.  */
                            outViewport->inc();
                        }
                    }
                    else
                    {
                        GPU_DEBUG_BOX(
                            printf("HierarchicalZ => Fragment %d in received stamp outside triangle.  Set fragment as culled.\n",
                                j, x, y);
                        )

                        /*  Set fragment as culled.  */
                        stamp[j]->setCull(TRUE);

                        /*  Update cull stamp flag.  */
                        cullStamp = cullStamp && TRUE;

                        /*  Update statistics.  */
                        outTriangle->inc();
                    }
                }

                /*  Check if the whole stamp can be culled.  */
                if (!cullStamp)
                {
                    GPU_DEBUG_BOX(
                        printf("HierarchicalZ => Adding stamp at %d\n", nextFree);
                    )

                    /*  Add the stamp to the early Z test queue.  */
                    hzQueue[nextFree].stamp = stamp;

                    //  Calculate the blocks required to evaluate the stamp.
                    hzQueue[nextFree].numBlocks = stampBlocks(
                        stamp[0]->getFragment()->getX(),
                        stamp[0]->getFragment()->getY(),
                        hzQueue[nextFree].block);

                    //  Check if multisampling is enabled.
                    if (!multisampling)
                    {
                        /*  Calculate the stamp minimum Z.  */
                        for(j = 0, minZ = 0x00ffffff; j < STAMP_FRAGMENTS; j++)
                        {
                            if (!stamp[j]->isCulled())
                                minZ = minimumDepth(minZ, stamp[j]->getFragment()->getZ());
                        }
                    }
                    else
                    {
                        /*  Calculate the stamp minimum Z.  */
                        for(j = 0, minZ = 0x00ffffff; j < STAMP_FRAGMENTS; j++)
                        {
                            //  Get pointer to the fragment Z samples.
                            u32bit *fragmentZSamples = stamp[j]->getFragment()->getMSAASamples();

                            bool *sampleCoverage = stamp[j]->getFragment()->getMSAACoverage();
                                                        
                            //  Get minimum z from fragment Z samples.
                            for(u32bit k = 0; k < msaaSamples; k++)
                            {
                                if (sampleCoverage[k])
                                {
                                    //  Compare against the current minimum z for the whole stamp.
                                    minZ = minimumDepth(minZ, fragmentZSamples[k]);
                                }
                            }
                        }
                    }
                    
                    /*  Store the stamp minimum Z.  */
                    hzQueue[nextFree].stampZ = minZ;

                    /*  Set stamp as not early culled.  */
                    hzQueue[nextFree].culled = FALSE;

                    //  Reset pointer to the next block to be processed.
                    hzQueue[nextFree].currentBlock = 0;
                    
                    //  Reset block depth value for the stamp.
                    hzQueue[nextFree].blockZ = 0;
                    
                    /*  Update pointer to the next free queue entry.  */
                    nextFree = GPU_MOD(nextFree + 1, hzQueueSize);

                    /*  Update number of free queue entries.  */
                    freeHZQE--;

                    /*  Check if Early Z test is enabled.  */
                    if ((!disableHZ) && hzEnabled && !modifyDepth)
                    {
                        /*  Update number of queue entries waiting for read.  */
                        readHZQE++;
                    }
                    else
                    {
                        /*  Update number of stamps to send to Fragment FIFO.  */
                        sendHZQE++;
                    }
                }
                else
                {
                    GPU_DEBUG_BOX(
                        printf("HierarchicalZ => Culling whole stamp.\n");
                    )

                    /*  Delete stamp.  */
                    for(j = 0; j < STAMP_FRAGMENTS; j++)
                    {
                        /*  Get fragment.  */
                        fr = stamp[j]->getFragment();

                        /*  Delete fragment.  */
                        delete fr;

                        /*  Delete fragment input.  */
                        delete stamp[j];

                        /*  Update statistics.  */
                        cullOutside->inc();
                        culled->inc();
                    }

                    /*  Delete stamp.  */
                    delete[] stamp;
                }
            }
            else
            {
                /*  Add the stamp to the early Z test queue.  */
                hzQueue[nextFree].stamp = stamp;

                /*  Set last fragment as received.  */
                lastFragment = TRUE;
            }
        }
        else
        {
            /*  Delete the stamp.  */
            delete[] stamp;
        }
    }
}

/*  Returns the minimum of two depth values.  */
u32bit HierarchicalZ::minimumDepth(u32bit a, u32bit b)
{
    /*  Select depth precission mode.  */
    switch(zBufferPrecission)
    {
        case 24:

            /*  Calculate the minimum for a 24-bit depth value.  */
            return ((a & 0x00ffffff) < (b & 0x00ffffff))?a:b;

        default:

            panic("HierarchicalZ", "minimumDepth", "Unsupported depth buffer format.");
            break;
    }
    return 0; // avoids "return missing" warning

}

//  Calculates the stamp block address inside the HZ buffer.
u32bit HierarchicalZ::stampBlocks(s32bit x, s32bit y, u32bit *blocks)
{
    u32bit address;
    u32bit block;
    u32bit requestSize;
    
    //  Check for multisampling mode.
    if (!multisampling)
    {
        //  Calculate stamp address.  Use pixel resolution.
        //address = GPUMath::pixel2Memory(x, y, startX, startY, width, height,
        //    hRes, vRes, STAMP_WIDTH, STAMP_WIDTH, genW, genH, scanW, scanH, overW, overH, 1, 1);
        address = pixelMapper.computeAddress(x, y);

        //  Calculate the number of depth elements covered by the stamp.
        requestSize = STAMP_FRAGMENTS;
    }
    else
    {
        //  Calculate stamp address.  Use sample resolution.
        //address = GPUMath::pixel2Memory(x, y, startX, startY, width, height,
        //    hRes, vRes, STAMP_WIDTH, STAMP_WIDTH, genW, genH, scanW, scanH, overW, overH, msaaSamples, 1);
        address = pixelMapper.computeAddress(x, y);

        //  Calculate the number of depth elements covered by the stamp.
        requestSize = STAMP_FRAGMENTS * msaaSamples;
    }

    //  Check if the stamp has to access more than one HZ block to be evaluated
    if ((requestSize > blockSize) || (((address & blockSizeMask) + requestSize) > blockSize))
    {
        u32bit numBlocks = 0;

        //  Split the request in the number of blocks required to evaluate the whole stamp.
        while(requestSize > 0)
        {
            GPU_ASSERT(
                if (numBlocks == MAX_STAMP_BLOCKS)
                    panic("HierarchicalZ", "stampBlocks", "Reached maximum number of blocks per stamp.");
            )
    
            //  Compute the block address for the current block.        
            blocks[numBlocks] = address >> blockShift;

            //  Update the number depth elements that are still to be requested.  
            requestSize = requestSize - GPU_MIN(requestSize, (blockSize - (address & blockSizeMask)));

            //  Update the address to point to the next block address.
            address = address - (address & blockSizeMask) + blockSize;
            
            //  Update the number of blocks that has been processed.
            numBlocks++;
        }

        return numBlocks;        
    }
    else
    {
        //  Calculate stamp block address.
        blocks[0] = address >> blockShift;

        return 1;
    }
}

/*  Search inside the Hierarchical Z cache for a HZ block.  */
bool HierarchicalZ::searchHZCache(u32bit block, u32bit &cacheEntry)
{
    u32bit i;

    /*  Search in the cache entry for the block.  */
    for(i = 0; (i < hzCacheLines) && (hzCache[i].block != (block & hzLineMask)); i++);

    /*  Check if block was found.  */
    if ((i < hzCacheLines) && hzCache[i].valid)
    {
        /*  Update cache reserve counter.  */
        hzCache[i].reserves++;

        /*  Return the cache entry.  */
        cacheEntry = i;

        /*  Return valid block found.  */
        return TRUE;
    }

    /*  Block not found in the cache.  */
    return FALSE;
}

/*  Inserts a block inside the HZ cache.  */
bool HierarchicalZ::insertHZCache(u32bit block, u32bit &cacheEntry)
{
    u32bit i;

    /*  Search for an invalid cache entry.  */
    for(i = 0; (i < hzCacheLines) && hzCache[i].valid; i++);

    /*  Check if invalid cache entry found.  */
    if (i < hzCacheLines)
    {
        /*  Set cache entry new block.  */
        hzCache[i].block = block & hzLineMask;

        /*  Set valid bit.  */
        hzCache[i].valid = true;

        /*  Reset cache entry read block.  */
        hzCache[i].read = false;

        /*  Set cache entry reserve counter.  */
        hzCache[i].reserves = 1;

        /*  Return the selected cache entry.  */
        cacheEntry = i;

        /*  Block request insert in the HZ cache.  */
        return TRUE;
    }
    else
    {
        /*  Search for an unreserved cache entry.  */
        for(i = 0; (i < hzCacheLines) && (hzCache[i].reserves > 0); i++);

        /*  Check if unreserved cache entry was found.  */
        if (i < hzCacheLines)
        {
            /*  Set cache entry new block.  */
            hzCache[i].block = block & hzLineMask;

            /*  Reset cache entry read block.  */
            hzCache[i].read = false;

            /*  Set cache entry reserve counter.  */
            hzCache[i].reserves = 1;

            /*  Return the selected cache entry.  */
            cacheEntry = i;

            /*  Block request insert in the HZ cache.  */
            return TRUE;
        }
    }

    /*  No available cache entry.  */
    return FALSE;
}

/*  Performs the comparation between the fragment and hierarchical z values.  */
bool HierarchicalZ::hzCompare(u32bit fragZ, u32bit hzZ)
{
    /*  Check compare function.  */
    switch(depthFunction)
    {
        case GPU_NEVER:
            panic("HierarchicalZ", "hzCompare", "Unsupported hierarchical Z compare mode.");
            break;

        case GPU_ALWAYS:
            panic("HierarchicalZ", "hzCompare", "Unsupported hierarchical Z compare mode.");
            break;

        case GPU_LESS:

            return fragZ < hzZ;

            break;

        case GPU_LEQUAL:

//printf("HZ -> compare LEQUAL fragZ %x hzZ %x pass? %s\n", fragZ, hzZ, (fragZ <= hzZ)?"T":"F");
            return fragZ <= hzZ;

            break;

        case GPU_EQUAL:

            /*  If Hierarchical stores the maximum depth for the block any fragment with
                depth greater than the block HZ depth can't pass the depth equal test.  */
            return fragZ <= hzZ;

            //panic("HierarchicalZ", "hzCompare", "Unsupported hierarchical Z compare mode.");
            break;

        case GPU_GEQUAL:
            panic("HierarchicalZ", "hzCompare", "Unsupported hierarchical Z compare mode.");
            break;

        case GPU_GREATER:
            panic("HierarchicalZ", "hzCompare", "Unsupported hierarchical Z compare mode.");
            break;

        case GPU_NOTEQUAL:
            panic("HierarchicalZ", "hzCompare", "Unsupported hierarchical Z compare mode.");

        default:
            panic("HierarchicalZ", "hzCompare", "Undefined compare function mode");
            break;
    }

    //  Remove VS2005 warning.
    return false;
}

void HierarchicalZ::getState(string &stateString)
{
    stringstream stateStream;

    stateStream.clear();

    stateStream << " state = ";

    switch(state)
    {
        case RAST_RESET:
            stateStream << "RAST_RESET";
            break;
        case RAST_READY:
            stateStream << "RAST_READY";
            break;
        case RAST_DRAWING:
            stateStream << "RAST_DRAW";
            break;
        case RAST_END:
            stateStream << "RAST_END";
            break;
        case RAST_CLEAR:
            stateStream << "RAST_CLEAR";
            break;
        case RAST_CLEAR_END:
            stateStream << "RAST_CLEAR_END";
            break;
        default:
            stateStream << "undefined";
            break;
    }

    stateStream << " | Fragment Counter = " << fragmentCounter;
    stateStream << " | Free = " << freeHZQE;
    stateStream << " | Read = " << readHZQE;
    stateStream << " | Test = " << testHZQE;
    stateStream << " | Send = " << sendHZQE;
    stateStream << " | Last Fragment = " << lastFragment;

    stateString.assign(stateStream.str());
}

//  Saves the HZ buffer content into a file.
void HierarchicalZ::saveHZBuffer()
{
    ofstream out;

    //  Open/create snapshot file for the HZ buffer content.    
    out.open("hzbuffer.snapshot", ios::binary);
    
    //  Check if file was open/created correctly.
    if (!out.is_open())
    {
        panic("HierarchicalZ", "saveHZBuffer", "Error creating Hierarchical Z Buffer snapshot file.");
    }
    
    //  Dump the HZ buffer content into the file.
    out.write((char *) hzLevel0, hzBufferSize * 4);

    //  Close the file.
    out.close();
}

//  Loads the HZ buffer content from a file.
void HierarchicalZ::loadHZBuffer()
{
    ifstream in;

    //  Open snapshot file for the HZ buffer content.    
    in.open("hzbuffer.snapshot", ios::binary);
    
    //  Check if file was open correctly.
    if (!in.is_open())
    {
        panic("HierarchicalZ", "loadHZBuffer", "Error opening Hierarchical Z Buffer snapshot file.");
    }
    
    //  Load the HZ buffer content from the file.
    in.read((char *) hzLevel0, hzBufferSize * 4);

    //  Close the file.
    in.close();
}



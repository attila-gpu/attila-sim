/**************************************************************************
 *
 * Copyright (c) 2002 - 2004 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: ColorWrite.cpp,v $
 * $Revision: 1.27 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:45 $
 *
 * Color Write box implementation file.
 *
 */

/**
 *
 *  @file ColorWrite.cpp
 *
 *  This file implements the Color Write box.
 *
 *  This class implements the final color blend and color write
 *  stages of the GPU pipeline.
 *
 */

#include "ColorWrite.h"
#include "MemoryTransaction.h"
#include "BlendOperation.h"
#include "RasterizerStateInfo.h"
#include "ColorWriteStatusInfo.h"
#include "ColorBlockStateInfo.h"
#include "GPUMath.h"
#include <cstdio>
#include "GPU.h" // STAMP_FRAGMENTS
#include <sstream>

using namespace gpu3d;

/*  Color Write box constructor.  */
ColorWrite::ColorWrite(u32bit numStamps, u32bit overWidth, u32bit overHeight,
    u32bit scanWidth, u32bit scanHeight, u32bit genWidth, u32bit genHeight, u32bit pixelBytes,
    bool colorComprDisabled, u32bit cacheWays, u32bit cacheLines, u32bit stampsPerLine,
    u32bit portWidth, bool extraReadP, bool extraWriteP, u32bit cacheReqQSz, u32bit inputReqQSz, u32bit outputReqQSz,
    u32bit colorBlocks, u32bit blocksPerCycle, u32bit comprCycles,
    u32bit decomprCycles, u32bit blendQSize, u32bit blendStartLat, u32bit blendLat,
    u32bit blockLatency, u32bit fragMapMode, FragmentOpEmulator &fragEmu,
    char *name, char *prefix, Box *parent) :

    stampsCycle(numStamps), overH(overHeight), overW(overWidth),
    scanH((u32bit) ceil(scanHeight / (f32bit) genHeight)), scanW((u32bit) ceil(scanWidth / (f32bit) genWidth)),
    genH(genHeight / STAMP_HEIGHT), genW(genWidth / STAMP_WIDTH), bytesPixel(pixelBytes),
    disableColorCompr(colorComprDisabled), colorCacheWays(cacheWays), colorCacheLines(cacheLines),
    stampsLine(stampsPerLine), cachePortWidth(portWidth), extraReadPort(extraReadP), extraWritePort(extraWriteP),
    blocksCycle(blocksPerCycle), blendQueueSize(blendQSize), blendRate(blendStartLat), blendLatency(blendLat),
    blockStateLatency(blockLatency), fragmentMapMode(fragMapMode), frEmu(fragEmu), Box(name, parent)

{
    DynamicObject *defaultState[1];
    u32bit i;
    char fullName[64];
    char postfix[32];

    /*  Check parameters.  */
    GPU_ASSERT(
        if (stampsCycle == 0)
            panic("ColorWrite", "ColorWrite", "At least a stamp must be received per cycle.");
        if (overW == 0)
            panic("ColorWrite", "ColorWrite", "Over scan tile must be at least 1 scan tile wide.");
        if (overH == 0)
            panic("ColorWrite", "ColorWrite", "Over scan tile must be at least 1 scan tile high.");
        if (scanW == 0)
            panic("ColorWrite", "ColorWrite", "Scan tile must be at least 1 generation tile wide.");
        if (scanH == 0)
            panic("ColorWrite", "ColorWrite", "Scan tile must be at least 1 generation tile high.");
        if (genW == 0)
            panic("ColorWrite", "ColorWrite", "Scan tile must be at least 1 stamp wide.");
        if (genH == 0)
            panic("ColorWrite", "ColorWrite", "Scan tile must be at least 1 stamp high.");
        if (bytesPixel != 4)
            panic("ColorWrite", "ColorWrite", "Only supported 32bit color values.");
        if (colorCacheWays < 1)
            panic("ColorWrite", "ColorWrite", "Color Cache requires at least 1 way.");
        if (colorCacheLines < 1)
            panic("ColorWrite", "ColorWrite", "Color Cache requires at least 1 line.");
        if (stampsLine < 1)
            panic("ColorWrite", "ColorWrite", "Color Cache requires at least one stamp per line.");
        if (stampsLine == 0)
            panic("ColorWrite", "ColorWrite", "Color cache ports must be at least 1 byte wide.");
        if (blocksCycle == 0)
            panic("ColorWrite", "ColorWrite", "At least a color block status register must be cleared per cycle");
        if ((!disableColorCompr) && (stampsLine < 16))
            panic("ColorWrite", "ColorWrite", "Color compression only supported for 8x8 fragment tiles and 256 byte cache lines.");
        if (blendQueueSize < 2)
            panic("ColorWrite", "ColorWrite", "Blend queue requires at least two entries.");
        if (blendRate == 0)
            panic("ColorWrite", "ColorWrite", "At least one cycle required between stamps being blended.");
        if (blendLatency < 1)
            panic("ColorWrite", "ColorWrite", "Blend ALU latency must be 1 cycle or greater.");
        if (blockStateLatency < 1)
            panic("ColorWrite", "ColorWrite", "Color block state update signal latency must be 1 cycle or greater.");
    )

    /*  Create the full name and postfix for the statistics.  */
    sprintf(fullName, "%s::%s", prefix, name);
    sprintf(postfix, "CW-%s", prefix);


    /*  Create statistics.  */
    inputs = &getSM().getNumericStatistic("InputFragments", u32bit(0), fullName, postfix);
    blended = &getSM().getNumericStatistic("BlendedFragments", u32bit(0), fullName, postfix);
    logoped = &getSM().getNumericStatistic("LogicOpFragments", u32bit(0), fullName, postfix);
    outside = &getSM().getNumericStatistic("OutsideFragments", u32bit(0), fullName, postfix);
    culled = &getSM().getNumericStatistic("CulledFragments", u32bit(0), fullName, postfix);
    readTrans = &getSM().getNumericStatistic("ReadTransactions", u32bit(0), fullName, postfix);
    writeTrans = &getSM().getNumericStatistic("WriteTransactions", u32bit(0), fullName, postfix);
    readBytes = &getSM().getNumericStatistic("ReadBytes", u32bit(0), fullName, postfix);
    writeBytes = &getSM().getNumericStatistic("WriteBytes", u32bit(0), fullName, postfix);
    fetchOK = &getSM().getNumericStatistic("FetchOK", u32bit(0), fullName, postfix);
    fetchFail = &getSM().getNumericStatistic("FetchFailed", u32bit(0), fullName, postfix);
    allocateOK = &getSM().getNumericStatistic("AllocateOK", u32bit(0), fullName, postfix);
    allocateFail = &getSM().getNumericStatistic("AllocateFailed", u32bit(0), fullName, postfix);
    readOK = &getSM().getNumericStatistic("ReadOK", u32bit(0), fullName, postfix);
    readFail = &getSM().getNumericStatistic("ReadFailed", u32bit(0), fullName, postfix);
    writeOK = &getSM().getNumericStatistic("WriteOK", u32bit(0), fullName, postfix);
    writeFail = &getSM().getNumericStatistic("WriteFailed", u32bit(0), fullName, postfix);
    rawDep = &getSM().getNumericStatistic("RAWDependence", u32bit(0), fullName, postfix);

    /*  Create signals.  */


    /*  Create signals with the main Rasterizer box.  */

    /*  Create command signal from the main Rasterizer box.  */
    rastCommand = newInputSignal("ColorWriteCommand", 1, 1, prefix);

    /*  Create state signal to the main Rasterizer box.  */
    rastState = newOutputSignal("ColorWriteRasterizerState", 1, 1, prefix);

    /*  Create default signal value.  */
    defaultState[0] = new RasterizerStateInfo(RAST_RESET);

    /*  Set default signal value.  */
    rastState->setData(defaultState);


    /*  Create signals with Z Test box.  */

    /*  Create input fragments signal from Z Test.   */
    //fragments[Z_STENCIL_TEST] = newInputSignal("ColorWrite", stampsCycle * STAMP_FRAGMENTS, 1, prefix);

    /*  Create state signal to Z Test.  */
    //colorWriteState[Z_STENCIL_TEST] = newOutputSignal("ColorWriteState", 1, 1, prefix);

    /*  Create default signal value.  */
    //defaultState[0] = new ColorWriteStatusInfo(CWS_READY);

    /*  Set default signal value.  */
    //colorWriteState[Z_STENCIL_TEST]->setData(defaultState);


    /*  Create signals with Fragment FIFO box.  */

    /*  Create input fragments signal from Z Test.   */
    fragments[FRAGMENT_FIFO] = newInputSignal("ColorWriteInput", stampsCycle * STAMP_FRAGMENTS, 1, prefix);

    /*  Create state signal to Z Test.  */
    colorWriteState[FRAGMENT_FIFO] = newOutputSignal("ColorWriteFFIFOState", 1, 1, prefix);

    /*  Create default signal value.  */
    defaultState[0] = new ColorWriteStatusInfo(CWS_READY);

    /*  Set default signal value.  */
    colorWriteState[FRAGMENT_FIFO]->setData(defaultState);


    /*  Create signals with Memory Controller box.  */

    /*  Create request signal to the Memory Controller.  */
    memRequest = newOutputSignal("ColorWriteMemoryRequest", 1, 1, prefix);

    /*  Create data signal from the Memory Controller.  */
    memData = newInputSignal("ColorWriteMemoryData", 2, 1, prefix);


    /*  Create signals that simulate the blend latency.  */

    /*  Create start signal for blend operation.  */
    blendStart = newOutputSignal("ColorWriteBlend", 1, blendLatency, prefix);

    /*  Create end signal for blend operation.  */
    blendEnd = newInputSignal("ColorWriteBlend", 1, blendLatency, prefix);


    /*  Create signal with the DAC.  */

    /*  Create block state info signal to the DAC.  */
    blockStateDAC = newOutputSignal("ColorBlockState", 1, blockStateLatency, prefix);


    /*
        !!!NOTE: A cache line contains a number of stamps from
        the same tile.  The stamps in a line must be in sequential
        lines.  !!!

     */
    bytesLine = stampsLine * STAMP_FRAGMENTS * bytesPixel;

    /*  Calculate the line shift and mask.  */
    lineShift = GPUMath::calculateShift(bytesLine);
    lineMask = GPUMath::buildMask(colorCacheLines);

    /*  Calculate stamp mask.  */
    stampMask = GPUMath::buildMask(STAMP_FRAGMENTS * bytesPixel);

    /*  Initialize color cache/write buffer.  */
    colorCache = new ColorCache(
        colorCacheWays,                     /*  Number of ways in the Color Cache.  */
        colorCacheLines,                    /*  Number of lines per way.  */
        stampsLine,                         /*  Stamps per line.  */
        STAMP_FRAGMENTS * bytesPixel,       /*  Bytes per stamp.  */
        1 + (extraReadPort?1:0),            /*  Read ports.  */
        1 + (extraWritePort?1:0),           /*  Write ports.  */
        cachePortWidth,                     /*  Width of the color cache ports (bytes).  */
        cacheReqQSz,                        /*  Size of the memory request queue.  */
        inputReqQSz,                        /*  Size of the input request queue.  */
        outputReqQSz,                       /*  Size of the output request queue.  */
        disableColorCompr,                  /*  Disables color compression.  */
        colorBlocks,                        /*  Number of color blocks in the state memory.  */
        blocksCycle,                        /*  Number of color blocks cleared per cycle.  */
        comprCycles,                        /*  Number of cycles to compress a color block.  */
        decomprCycles,                      /*  Number of cycles to decompress a color block.  */
        postfix                             /*  Postfix for the cache statistics.  */
        );

    /*  Allocate the blend queue.  */
    blendQueue = new BlendQueue[blendQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (blendQueue == NULL)
            panic("ColorWrite", "ColorWrite", "Error allocating the blend queue.");
    )

    /*  Allocate the buffers for the stamp fragment color.  */
    for(i = 0; i < blendQueueSize; i++)
    {
        /*  Allocate stamp input colors (float).  */
        blendQueue[i].inColor = new QuadFloat[STAMP_FRAGMENTS];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (blendQueue[i].inColor == NULL)
                panic("ColorWrite", "ColorWrite", "Error allocating stamp input color buffer in the blend queue.");
        )

        /*  Allocate stamp output colors (integer).  */
        blendQueue[i].outColor = new u8bit[STAMP_FRAGMENTS * bytesPixel];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (blendQueue[i].outColor == NULL)
                panic("ColorWrite", "ColorWrite", "Error allocating stamp output color buffer.");
        )

        /*  Allocate destination color buffer (integer).  */
        blendQueue[i].destColor = new u8bit[STAMP_FRAGMENTS * bytesPixel];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (blendQueue[i].destColor == NULL)
                panic("ColorWrite", "ColorWrite", "Error allocating stamp destination color buffer (integer).");
        )

        /*  Allocate destination color buffer (float).  */
        blendQueue[i].destColorF = new QuadFloat[STAMP_FRAGMENTS];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (blendQueue[i].destColorF == NULL)
                panic("ColorWrite", "ColorWrite", "Error allocating stamp destination color buffer (float).");
        )

        /*  Allocate stamp write mask.  */
        blendQueue[i].mask = new bool[STAMP_FRAGMENTS * bytesPixel];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (blendQueue[i].mask == NULL)
                panic("ColorWrite", "ColorWrite", "Error allocating stamp write mask.");
        )

        /*  Allocate container for the stamp cookies.  */
        blendQueue[i].stampCookies = new DynamicObject();
    }

    /*  Allocate the current stamp.  */
    stamp = new FragmentInput*[STAMP_FRAGMENTS];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (stamp == NULL)
            panic("ColorWrite", "ColorWrite", "Error allocating the current stamp.");
    )

    /*  Set memory ticket for clear.  */
    //ticket = 0;

    /*  Create a dummy initial rasterizer command.  */
    lastRSCommand = new RasterizerCommand(RSCOM_RESET);

    /*  Set initial state to reset.  */
    state = RAST_RESET;
}

/*  Color write simulation function.  */
void ColorWrite::clock(u64bit cycle)
{
    MemoryTransaction *memTrans;
    RasterizerCommand *rastComm;
    FragmentInput *frInput;
    BlendOperation *blendOp;
    u32bit opBlend;
    u32bit colorBufferSize;
    u32bit colorBufferBlocks;
    ColorBlockState *colorBlockState;
    ColorBlockStateInfo *colorBlockStateInfo;
    bool waitWrite;
    u32bit i, j;

    GPU_DEBUG(
        printf("ColorWrite => Clock %lld\n", cycle);
    )

    /*  Receive state from Memory Controller.  */
    while(memData->read(cycle, (DynamicObject *&) memTrans))
        processMemoryTransaction(cycle, memTrans);

    /*  Update color cache structures.  */
    memTrans = colorCache->update(cycle, memoryState);

    /*  Check if a memory transaction was generated.  */
    if (memTrans != NULL)
    {
        /*  Send memory transaction to the Memory Controller.  */
        memRequest->write(cycle, memTrans);

        /*  Update statistics.  */
        switch(memTrans->getCommand())
        {
            case MT_READ_REQ:

                readTrans->inc();
                break;

            case MT_WRITE_DATA:

                writeTrans->inc();
                writeBytes->inc(memTrans->getSize());
                break;

            default:
                panic("ColorWrite", "clock", "Unknown memory transaction.");
        }
    }

/*if ((cycle > 5024880) && (GPU_MOD(cycle, 1000) == 0))
{
printf("CW %lld => state ", cycle);
switch(state)
{
    case RAST_RESET:    printf("RESET\n"); break;
    case RAST_READY:    printf("READY\n"); break;
    case RAST_DRAWING:  printf("DRAWING\n"); break;
    case RAST_END:      printf("END\n"); break;
}
printf("CW => fetchStamps %d readStamps %d writeStamps %d freeStamps %d\n",
fetchStamps, readStamps, writeStamps, freeStamps);
}*/


    /*  Simulate current cycle.  */
    switch(state)
    {
        case RAST_RESET:

            /*  Reset state.  */

            GPU_DEBUG(
                printf("ColorWrite => Reset state.\n");
            )

            /*  Reset the color cache.  */
            colorCache->reset();
            colorCache->update(cycle, memoryState);

            /*  Reset Color Write registers to default values.  */
            hRes = 400;
            vRes = 400;
            startX = 0;
            startY = 0;
            width = 400;
            height = 400;
            clearColor = 0x00000000;
            earlyZ = TRUE;
            //earlyZ = FALSE;
            blend = FALSE;
            //blend = TRUE;
            equation = BLEND_FUNC_ADD;
            srcRGB = BLEND_ONE;
            srcAlpha = BLEND_ONE;
            dstRGB = BLEND_ZERO;
            //dstRGB = BLEND_ONE;
            dstAlpha = BLEND_ZERO;
            constantColor[0] = 0.0;
            constantColor[1] = 0.0;
            constantColor[2] = 0.0;
            constantColor[3] = 0.0;
            logicOperation = FALSE;
            logicOpMode = LOGICOP_COPY;
            writeR = writeG = writeB = writeA = TRUE;

            /*  By default set front buffer at 1 MB and back buffer at 2 MB.  */
            frontBuffer = 0x200000;
            backBuffer = 0x400000;

            /*  Set color buffer address in the color cache.  */
            colorCache->swap(backBuffer);

            /*  Reset free stamps counter.  */
            freeStamps = blendQueueSize;

            /*  Allocate space for the latency map.  */
            latencyMap = new u32bit[((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01))];

            /*  Clear the latency map before the next batch.  */
            clearLatencyMap = true;
            //memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);

            /*  Reset blend start cycle counter.  */
            blendCycles = 0;

            /*  Change state to ready.  */
            state = RAST_READY;

            break;

        case RAST_READY:

            /*  Ready state.  */

            GPU_DEBUG(
                printf("ColorWrite => Ready state.\n");
            )

            /*  Receive and process a rasterizer command.  */
            if(rastCommand->read(cycle, (DynamicObject *&) rastComm))
                processCommand(rastComm);

            break;

        case RAST_DRAWING:

            /*  Draw state.  */

            GPU_DEBUG(
                printf("ColorWrite => Drawing state.\n");
            )

            /*  Check if there are free blend queue entries.  */
            if (freeStamps >= stampsCycle)
            {
                /*  Receive stamps from Z Test box.  */
                for(i = 0; i < stampsCycle; i++)
                {
                    /*  Reset received fragment flag.  */
                    receivedFragment = TRUE;

                    /*  Receive fragments in a stamp.  */
                    for(j = 0; (j < STAMP_FRAGMENTS) && receivedFragment; j++)
                    {
                        /*  Get fragment from Z Test.  */
                        receivedFragment = fragments[fragmentSource]->read(cycle, (DynamicObject *&) frInput);

                        /*  Check if a fragment has been received.  */
                        if (receivedFragment)
                        {
                            /*  Store fragment in the current stamp.  */
                            stamp[j] = frInput;

                            /*  Count triangles.  */
                            if (frInput->getTriangleID() != currentTriangle)
                            {
                                /*  Change current triangle.  */
                                currentTriangle = frInput->getTriangleID();

                                /*  Update triangle counter.  */
                                triangleCounter++;
                            }

                            /*  Update fragment counter.  */
                            fragmentCounter++;

                            /*  Update statistics.  */
                            inputs->inc();
                        }
                    }

                    /*  Check if a whole stamp has been received.  */
                    GPU_ASSERT(
                        if (!receivedFragment && (j > 1))
                            panic("ColorWrite", "clock", "Missing fragments in a stamp.");
                    )

                    /*  Check if a stamp has been received.  */
                    if (receivedFragment)
                    {
                        /*  Process the received stamp.  */
                        processStamp();
                    }
                }
            }

            /*  Check if there are stamps waiting to be fetched.  */
            if (fetchStamps > 0)
            {
                /*  Check if blend mode is enabled.  */
                if (blend || logicOperation)
                {

                    /*  Try to fetch the next stamp in the queue.  */
                    if (colorCache->fetch(blendQueue[fetchBlend].address,
                        blendQueue[fetchBlend].way, blendQueue[fetchBlend].line,
                        blendQueue[fetchBlend].stampCookies))
                    {
                        GPU_DEBUG(
                            printf("ColorWrite => Fetched stamp at %x to cache line (%d, %d)\n",
                                blendQueue[fetchBlend].address, blendQueue[fetchBlend].way,
                                blendQueue[fetchBlend].line);
                        )

                        /*  Update fetch stamp counter.  */
                        fetchStamps--;

                        /*  Update read stamps counter.  */
                        readStamps++;

                        /*  Move the fetch pointer.  */
                        fetchBlend = GPU_MOD(fetchBlend + 1, blendQueueSize);

                        /*  Update statistics.  */
                        fetchOK->inc();
                    }
                    else
                    {
                        /*  Update statistics.  */
                        fetchFail->inc();
                    }
                }
                else
                {
                    /*  Try to allocate a cache (buffer mode) line.  */
                    if(colorCache->allocate(blendQueue[fetchBlend].address,
                        blendQueue[fetchBlend].way, blendQueue[fetchBlend].line,
                        blendQueue[fetchBlend].stampCookies))
                    {
                        GPU_DEBUG(
                            printf("ColorWrite => Allocating cache line (%d, %d) for stamp at %x.\n",
                                blendQueue[fetchBlend].way, blendQueue[fetchBlend].line,
                                blendQueue[fetchBlend].address);
                        )

                        /*  Convert float stamp color to integer color format.  */
                        colorFloat2Int(blendQueue[fetchBlend].inColor,
                            blendQueue[fetchBlend].outColor);

                        /*  Update fetch stamps counter.  */
                        fetchStamps--;

                        /*  Update fetch pointer.  */
                        fetchBlend = GPU_MOD(fetchBlend + 1, blendQueueSize);

                        /*  Update write stamps counter.  */
                        writeStamps++;

                        /*  Update statistics.  */
                        allocateOK->inc();
                    }
                    else
                    {
                        /*  Update statistics.  */
                        allocateFail->inc();
                    }
                }
            }

            /*  Update number of cycles remaining until another stamp can be blended.  */
            if (blendCycles > 0)
            {
                blendCycles--;
            }

            /*  Check if there are stamps waiting to be read.  */
            if ((blend || logicOperation) && (readStamps > 0) && (blendCycles == 0))
            {
                /*  Reset wait flag.  */
                waitWrite = FALSE;

                /*  Check if the same stamp is blended ahead in the
                    queue.  */
                for(i = writeBlend; (i != readBlend);
                    i = GPU_MOD(i + 1, blendQueueSize))
                {
                    /*  Check stamp address.  */
                    if (blendQueue[i].address == blendQueue[readBlend].address)
                    {
                        GPU_DEBUG(
                            printf("ColorWrite => Reading a stamp at %x before writing it.\n",
                            blendQueue[readBlend].address);
                        )

                        /*  Wait for the stamp to be written.  */
                        waitWrite = TRUE;
                    }
                }

                /*  Check if it must wait to a write.  */
                if (!waitWrite)
                {
                    /*  Try to read the next stamp in the queue.  */
                    if (colorCache->read(blendQueue[readBlend].address,
                        blendQueue[readBlend].way, blendQueue[readBlend].line,
                        blendQueue[readBlend].destColor))
                    {
                        GPU_DEBUG(
                            printf("ColorWrite => Reading stamp at %x\n",
                                blendQueue[readBlend].address);
                        )

                        /*  Convert fixed point color data to float point data.  */
                        colorInt2Float(blendQueue[readBlend].destColor,
                            blendQueue[readBlend].destColorF);

                        /*  Create blend operation object.  */
                        blendOp = new BlendOperation(readBlend);

                        /*  Copy parent command cookies.  */
                        blendOp->copyParentCookies(*blendQueue[readBlend].stampCookies);

                        /*  Start blend operation.  */
                        blendStart->write(cycle, blendOp);

                        /*  Set blend start cycle counter.  */
                        blendCycles = blendRate;

                        /*  Update read stamps counter.  */
                        readStamps--;

                        /*  Move the read pointer.  */
                        readBlend = GPU_MOD(readBlend + 1, blendQueueSize);

                        /*  Update statistics.  */
                        readOK->inc();
                    }
                    else
                    {
                        /*  Update statistics.  */
                        readFail->inc();
                    }
                }
                else
                {
                    /*  Update statistics.  */
                    rawDep->inc();
                }
            }

            /*  Check end of blend operation.  */
            if (blendEnd->read(cycle, (DynamicObject *&) blendOp))
            {
                /*  Get operation entry.  */
                opBlend = blendOp->getID();

                /*  Determine if blend mode is active.  */
                if (blend)
                {
                    GPU_DEBUG(
                        printf("ColorWrite => Blending stamp.\n");
                    )

                    /*  Perform blend operation.  */
                    frEmu.blend(blendQueue[opBlend].inColor,
                        blendQueue[opBlend].inColor,
                        blendQueue[opBlend].destColorF);

                    /*  Update statistics.  */
                    blended->inc(STAMP_FRAGMENTS);
                }

                /*  Convert the stamp color data to integer format.  */
                colorFloat2Int(blendQueue[opBlend].inColor,
                    blendQueue[opBlend].outColor);

                /*  Determine if logical operation is active.  */
                if (logicOperation)
                {
                    GPU_DEBUG(
                        printf("ColorWrite => LogicOp stamp.\n");
                    )

                    /*  Perform logical operation.  */
                    frEmu.logicOp(blendQueue[opBlend].outColor,
                        blendQueue[opBlend].outColor,
                        blendQueue[opBlend].destColor);

                    /*  Update statistics.  */
                    logoped->inc(STAMP_FRAGMENTS);
                }

                /*  Delete blend operation object.  */
                delete blendOp;

                /*  Update write stamps counter.  */
                writeStamps++;
            }

            /*  Check if there are stamps waiting to be written.  */
            if (writeStamps > 0)
            {
                /*  Try to write stamp.  */
                if (colorCache->write(
                    blendQueue[writeBlend].address,
                    blendQueue[writeBlend].way,
                    blendQueue[writeBlend].line,
                    blendQueue[writeBlend].outColor,
                    blendQueue[writeBlend].mask))
                {
                    GPU_DEBUG(
                        printf("ColorWrite => Writing stamp at %x\n",
                           blendQueue[writeBlend].address);
                    )

                    /*  Update write stamps counter.  */
                    writeStamps--;

                    /*  Update free stamps counter.  */
                    freeStamps++;

                    /*  Update latency map.  */
                    switch(fragmentMapMode)
                    {
                        case COLOR_MAP:

                            latencyMap[(blendQueue[writeBlend].y >> 1) * ((hRes >> 1) + (hRes & 0x01)) + (blendQueue[writeBlend].x >> 1)] =
                                u32bit(blendQueue[writeBlend].outColor[0] + (blendQueue[writeBlend].outColor[1] << 8) + (blendQueue[writeBlend].outColor[2] << 16));

                            break;

                        case OVERDRAW_MAP:

                            latencyMap[(blendQueue[writeBlend].y >> 1) * ((hRes >> 1) + (hRes & 0x01)) + (blendQueue[writeBlend].x >> 1)]++                                ;

                            break;

                        case FRAGMENT_LATENCY_MAP:

                            latencyMap[(blendQueue[writeBlend].y >> 1) * ((hRes >> 1) + (hRes & 0x01)) + (blendQueue[writeBlend].x >> 1)] =
                                u32bit(cycle - blendQueue[writeBlend].startCycle);

                            break;

                        case SHADER_LATENCY_MAP:

                            latencyMap[(blendQueue[writeBlend].y >> 1) * ((hRes >> 1) + (hRes & 0x01)) + (blendQueue[writeBlend].x >> 1)] =
                                blendQueue[writeBlend].shaderLatency;

                            break;

                        default:
                            panic("ColorWrite", "clock", "Unsuported fragment map mode.");
                            break;
                    }

                    /*  Move write pointer.  */
                    writeBlend = GPU_MOD(writeBlend + 1, blendQueueSize);

                    /*  Update statistics.  */
                    writeOK->inc();
                }
                else
                {
                    /*  Update statistics.  */
                    writeFail->inc();
                }
            }

            /*  Check end of the batch.  */
            if (lastFragment && (freeStamps == blendQueueSize) && (memTrans == NULL))
            {
                /*  Change state to end.  */
                state = RAST_END;
            }

            break;


        case RAST_END:

            /*  End of batch state.  */

            GPU_DEBUG(
                printf("ColorWrite => End state.\n");
            )

            /*  Wait for end command.  */
            if(rastCommand->read(cycle, (DynamicObject *&) rastComm))
                processCommand(rastComm);

            break;

        case RAST_SWAP:

            /*  Swap back and front buffer state.  */

            GPU_DEBUG(
                printf("ColorWrite => Swap state.\n");
            )

            /*  Check if the flush of the color cache has finished.  */
            if (!endFlush)
            {
                /*  Continue flushing the cache.  */
                if(!colorCache->flush())
                {
                    /*  Set new color buffer address in the color cache.  */
                    colorCache->swap(backBuffer);

                    /*  Calculate the size of the color buffer.  */
                    colorBufferSize = ((u32bit) ceil(vRes / (f32bit) (overH * scanH * genH * STAMP_HEIGHT))) *
                        ((u32bit) ceil(hRes / (f32bit) (overW * scanW * genW * STAMP_WIDTH))) *
                        overW * overH * scanW * scanH * genW * genH * STAMP_WIDTH * STAMP_HEIGHT * bytesPixel;

                    /*  Calculate the number of blocks to copy.  */
                    colorBufferBlocks = colorBufferSize / (stampsLine * STAMP_FRAGMENTS * bytesPixel);

                    /*  Allocate the color block states.  */
                    colorBlockState = new ColorBlockState[colorBufferBlocks];

                    /*  Check allocation.  */
                    GPU_ASSERT(
                        if (colorBlockState == NULL)
                            panic("ColorCache", "copyBlockStateMemory", "Error allocating color block state memory.");
                    )

                    /*  Get the color block states.  */
                    colorCache->copyBlockStateMemory(colorBlockState, colorBufferBlocks);

                    /*  Create object for the DAC.  */
                    colorBlockStateInfo = new ColorBlockStateInfo(colorBlockState, colorBufferBlocks);

                    /*  Copy cookies from the last command from the
                        Command Processor.  */
                    colorBlockStateInfo->copyParentCookies(*lastRSCommand);

                    /*  Add a cookie level.  */
                    colorBlockStateInfo->addCookie();

                    /*  Send color buffer block state to the DAC.  */
                    blockStateDAC->write(cycle, colorBlockStateInfo);

                    /*  Set cycles for copying the color buffer block state to the DAC.  */
                    copyStateCycles = (u32bit) ceil((f32bit) colorBufferBlocks / (f32bit) blocksCycle);

                    /*  Set flush end.  */
                    endFlush = TRUE;
                };
            }
            else
            {
                /*  Check if copying color block state memory to the DAC.  */
                if (copyStateCycles > 0)
                {
                    /*  Update cycle counter.  */
                    copyStateCycles--;

                    /*  Check if end of the copy.  */
                    if (copyStateCycles == 0)
                    {
printf("ColorWrite => End of swap.  Cycle %lld\n", cycle);

                        /*  Clear the latency map before the next batch.  */
                        clearLatencyMap = true;

                        //for(j = 0; j < ((vRes >> 1) + (vRes & 0x01)); j++)
                        //    for(i = 0; i < ((hRes >> 1) + (hRes & 0x01)); i++)
                        //        latencyMap[j * ((hRes >> 1) + (hRes & 0x01)) + i] = 0;

                        /*  End of swap.  */
                        state = RAST_END;
                    }
                }
            }

            break;

        case RAST_CLEAR:

            /*  Clear color buffer state.  */

            GPU_DEBUG(
                printf("ColorWrite => Clear state.\n");
            )

            /*  Check if the color cache has finished the clear.  */
            if (!colorCache->clear(clearColor))
            {
                /*  Change to end state.  */
                state = RAST_END;
            }

            break;

        default:

            panic("ColorWrite", "ColorWrite", "Unsupported state.");
            break;

    }

    /*  NOTE: RESERVE BUFFER FOR ON THE FLY FRAGMENTS IN THE
        INTERPOLATOR.  ONLY FOR CURRENT TEST IMPLEMENTATION.  */

    /*  Send current state to Z Test box.  */
    if (freeStamps >= ((2 + 8) * stampsCycle))
    {
        GPU_DEBUG(
            printf("ColorWrite => Sending READY.\n");
        )

        /*  Send a ready signal.  */
        //colorWriteState[Z_STENCIL_TEST]->write(cycle, new ColorWriteStatusInfo(CWS_READY));
        colorWriteState[FRAGMENT_FIFO]->write(cycle, new ColorWriteStatusInfo(CWS_READY));
    }
    else
    {
        GPU_DEBUG(
            printf("ColorWrite => Sending BUSY.\n");
        )

        /*  Send a busy signal.  */
        //colorWriteState[Z_STENCIL_TEST]->write(cycle, new ColorWriteStatusInfo(CWS_BUSY));
        colorWriteState[FRAGMENT_FIFO]->write(cycle, new ColorWriteStatusInfo(CWS_BUSY));
    }

    /*  Send current rasterizer state.  */
    rastState->write(cycle, new RasterizerStateInfo(state));

}


/*  Processes a rasterizer command.  */
void ColorWrite::processCommand(RasterizerCommand *command)
{
    u32bit colorBuffer;
    u32bit i;

    /*  Delete last command.  */
    delete lastRSCommand;

    /*  Store current command as last received command.  */
    lastRSCommand = command;

    /*  Process command.  */
    switch(command->getCommand())
    {
        case RSCOM_RESET:
            /*  Reset command from the Rasterizer main box.  */

            GPU_DEBUG(
                printf("ColorWrite => RESET command received.\n");
            )

            /*  Change to reset state.  */
            state = RAST_RESET;

            break;

        case RSCOM_DRAW:
            /*  Draw command from the Rasterizer main box.  */

            GPU_DEBUG(
                printf("ColorWrite => DRAW command received.\n");
            )

             /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processRegisterWrite", "DRAW command can only be received in READY state.");
            )

            /*  Configure the blend emulation.  */
            frEmu.setBlending(equation, srcRGB, srcAlpha, dstRGB, dstAlpha,
                constantColor);

            /*  Configure logical operation emulation.  */
            frEmu.setLogicOpMode(logicOpMode);

            /*  Set the pixel color format.  */
            frEmu.setFragmentBytes(bytesPixel);

            /*  Reset next free entry in the blend queue entry.  */
            freeBlend = 0;

            /*  Reset next stamp to fetch pointer.  */
            fetchBlend = 0;

            /*  Reset next stamp to read pointer.  */
            readBlend = 0;

            /*  Reset next stamp to write pointer.  */
            writeBlend = 0;

            /*  Reset free blend queue entries counter.  */
            freeStamps = blendQueueSize;

            /*  Reset fetch stamps counter.  */
            fetchStamps = 0;

            /*  Reset read stamps counter.  */
            readStamps = 0;

            /*  Reset write stamps counter.  */
            writeStamps = 0;

            /*  Reset total number of stamps in the blend queue counter.  */
            blendStamps = 0;

            /*  Reset triangle counter.  */
            triangleCounter = 0;

            /*  Reset batch fragment counter.  */
            fragmentCounter = 0;

            /*  Reset previous processed triangle.  */
            currentTriangle = (u32bit) -1;

            /*  Last batch fragment no received yet.  */
            lastFragment = FALSE;

            /*  Set source of fragments.  */
            fragmentSource = earlyZ?FRAGMENT_FIFO:Z_STENCIL_TEST;

            /*  Check if the latency map must be cleared.  */
            if (clearLatencyMap)
            {
                /*  Clear latency map.  */
                memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);

                /*  Unset clear flag.  */
                clearLatencyMap = false;
            }

            /*  Change to drawing state.  */
            state = RAST_DRAWING;

            break;

        case RSCOM_END:
            /*  End command received from Rasterizer main box.  */

            GPU_DEBUG(
                printf("ColorWrite => END command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_END)
                    panic("ColorWrite", "processCommand", "END command can only be received in END state.");
            )

            /*  Change to ready state.  */
            state = RAST_READY;

            break;

        case RSCOM_REG_WRITE:
            /*  Write register command from the Rasterizer main box.  */

            GPU_DEBUG(
                printf("ColorWrite => REG_WRITE command received.\n");
            )

             /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "REGISTER WRITE command can only be received in READY state.");
           )

            /*  Process register write.  */
            processRegisterWrite(command->getRegister(),
                command->getSubRegister(), command->getRegisterData());

            break;

        case RSCOM_SWAP:
            /*  Swap front and back buffer command.  */

            GPU_DEBUG(
                printf("ColorWrite => SWAP command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "SWAP command can only be received in READY state.");
            )

            /*  Update frame counter.  */
            frameCounter++;

            /*  Swap primary and secondary color buffers.  */
            colorBuffer = frontBuffer;
            frontBuffer = backBuffer;
            backBuffer = colorBuffer;

            /*  Reset color cache flush flag.  */
            endFlush = FALSE;

            /*  Change state to swap state.  */
            state = RAST_SWAP;

            break;

        case RSCOM_CLEAR_COLOR_BUFFER:
            /*  Clear color buffer command.  */

            GPU_DEBUG(
                printf("ColorWrite => CLEAR_COLOR_BUFFER command received.\n");
            )

            /*  Check current state.  */
            GPU_ASSERT(
                if (state != RAST_READY)
                    panic("ColorWrite", "processCommand", "CLEAR command can only be received in READY state.");
            )

            /*  Clear the color cache as any info there will be invalid now.  */
            colorCache->clear(clearColor);

            /*  Change to clear state.  */
            state = RAST_CLEAR;

            break;

        default:
            panic("ColorWrite", "proccessCommand", "Unsupported command.");
            break;
    }
}

void ColorWrite::processRegisterWrite(GPURegister reg, u32bit subreg,
    GPURegData data)
{
    /*  Process register write.  */
    switch(reg)
    {
        case GPU_DISPLAY_X_RES:
            /*  Write display horizontal resolution register.  */
            hRes = data.uintVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_DISPLAY_X_RES = %d.\n", hRes);
            )

            /*  Delete old latency map.  */
            delete[] latencyMap;

            /*  Reallocate space for the latency map.  */
            latencyMap = new u32bit[((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01))];

            /*  Clear latency map before the next batch.  */
            clearLatencyMap = true;
            //memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);

            break;

        case GPU_DISPLAY_Y_RES:
            /*  Write display vertical resolution register.  */
            vRes = data.uintVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_DISPLAY_Y_RES = %d.\n", vRes);
            )

            /*  Delete old latency map.  */
            delete[] latencyMap;

            /*  Reallocate space for the latency map.  */
            latencyMap = new u32bit[((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01))];

            /*  Clear latency map before the next batch.  */
            clearLatencyMap = true;
            //memset(latencyMap, 0, ((hRes >> 1) + (hRes & 0x01)) * ((vRes >> 1) + (vRes & 0x01)) * 4);

            break;

        case GPU_VIEWPORT_INI_X:
            /*  Write viewport initial x coordinate register.  */
            startX = data.intVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_VIEWPORT_INI_X = %d.\n", startX);
            )

            break;

        case GPU_VIEWPORT_INI_Y:
            /*  Write viewport initial y coordinate register.  */
            startY = data.intVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_VIEWPORT_INI_Y = %d.\n", startY);
            )

            break;

        case GPU_VIEWPORT_WIDTH:
            /*  Write viewport width register.  */
            width = data.uintVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_VIEWPORT_WIDTH = %d.\n", width);
            )

            break;

        case GPU_VIEWPORT_HEIGHT:
            /*  Write viewport height register.  */
            height = data.uintVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_VIEWPORT_HEIGHT = %d.\n", height);
            )

            break;

        case GPU_COLOR_BUFFER_CLEAR:
            /*  Write color buffer clear color value.  */
            clearColor = data.uintVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_COLOR_BUFFER_CLEAR = %x.\n", clearColor);
            )

            break;

        case GPU_FRONTBUFFER_ADDR:
            /*  Write front color buffer address register.  */
            frontBuffer = data.uintVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_FRONTBUFFER_ADDR = %x.\n",
                    frontBuffer);
            )

            break;

        case GPU_BACKBUFFER_ADDR:
            /*  Write back color buffer address register.  */
            backBuffer = data.uintVal;

            /*  Set color buffer address in the color cache.  */
            colorCache->swap(backBuffer);

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_BACKBUFFER_ADDR = %x.\n",
                    backBuffer);
            )

            break;

        case GPU_EARLYZ:

            /*  Set early Z test flag.  */
            //earlyZ = data.booleanVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_EARLY_Z = %s.\n",
                    data.booleanVal?"ENABLED":"DISABLED");
            )

            break;

        case GPU_COLOR_BLEND:
            /*  Write blend enabled flag register.  */
            blend = data.booleanVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_COLOR_BLEND = %s.\n", blend?"ENABLED":"DISABLED");
            )

            break;

        case GPU_BLEND_EQUATION:
            /*  Write blend equation register.  */
            equation = data.blendEquation;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_BLEND_EQUATION = ");

                switch(equation)
                {
                    case BLEND_FUNC_ADD:
                        printf("FUNC_ADD.\n");
                        break;

                    case BLEND_FUNC_SUBTRACT:
                        printf("FUNC_SUBTRACT.\n");
                        break;

                    case BLEND_FUNC_REVERSE_SUBTRACT:
                        printf("FUNC_REVERSE_SUBTRACT.\n");
                        break;

                    case BLEND_MIN:
                        printf("MIN.\n");
                        break;

                    case BLEND_MAX:
                        printf("MAX.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend equation mode.");
                        break;
                }
            )

            break;

        case GPU_BLEND_SRC_RGB:
            /*  Write source RGB weight function.  */
            srcRGB = data.blendFunction;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_BLEND_SRC_RGB = ");

                switch(srcRGB)
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;
                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }

            )

            break;

        case GPU_BLEND_DST_RGB:
            /*  Write destination RGB weight function.  */
            dstRGB = data.blendFunction;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_BLEND_DST_RGB = ");

                switch(srcRGB)
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;
                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }

            )
            break;

        case GPU_BLEND_SRC_ALPHA:
            /*  Write source alpha weight function.  */
            srcAlpha = data.blendFunction;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_BLEND_SRC_ALPHA = ");

                switch(srcRGB)
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;
                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }

            )
            break;

        case GPU_BLEND_DST_ALPHA:
            /*  Write destination ALPHA weight function.  */
            dstAlpha = data.blendFunction;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_BLEND_DST_ALPHA = ");

                switch(srcRGB)
                {
                    case BLEND_ZERO:
                        printf("ZERO.\n");
                        break;
                    case BLEND_ONE:
                        printf("ONE.\n");
                        break;

                    case BLEND_SRC_COLOR:
                        printf("SRC_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_COLOR:
                        printf("ONE_MINUS_SRC_COLOR.\n");
                        break;

                    case BLEND_DST_COLOR:
                        printf("DST_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_COLOR:
                        printf("ONE_MINUS_DST_COLOR.\n");
                        break;

                    case BLEND_SRC_ALPHA:
                        printf("SRC_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_SRC_ALPHA:
                        printf("ONE_MINUS_SRC_ALPHA.\n");
                        break;

                    case BLEND_DST_ALPHA:
                        printf("DST_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_DST_ALPHA:
                        printf("ONE_MINUS_DST_ALPHA.\n");
                        break;

                    case BLEND_CONSTANT_COLOR:
                        printf("CONSTANT_COLOR.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_COLOR:
                        printf("ONE_MINUS_CONSTANT_COLOR.\n");
                        break;

                    case BLEND_CONSTANT_ALPHA:
                        printf("CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_ONE_MINUS_CONSTANT_ALPHA:
                        printf("ONE_MINUS_CONSTANT_ALPHA.\n");
                        break;

                    case BLEND_SRC_ALPHA_SATURATE:
                        printf("ALPHA_SATURATE.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported blend weight function.");
                        break;
                }

            )
            break;

        case GPU_BLEND_COLOR:
            /*  Write blend constant color register.  */
            constantColor[0] = data.qfVal[0];
            constantColor[1] = data.qfVal[1];
            constantColor[2] = data.qfVal[2];
            constantColor[3] = data.qfVal[3];

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_BLEND_COLOR = {%f, %f, %f, %f}\n",
                    constantColor[0], constantColor[1], constantColor[2], constantColor[3]);
            )

            break;

        case GPU_COLOR_MASK_R:
            /*  Write red component write mask register.  */
            writeR = data.booleanVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_COLOR_MASK_R = %s.\n", writeR?"ENABLED":"DISABLED");
            )

            break;

        case GPU_COLOR_MASK_G:
            /*  Write green component write mask register.  */
            writeG = data.booleanVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_COLOR_MASK_G = %s.\n", writeG?"ENABLED":"DISABLED");
            )

            break;

        case GPU_COLOR_MASK_B:
            /*  Write blue component write mask register.  */
            writeB = data.booleanVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_COLOR_MASK_B = %s.\n", writeB?"ENABLED":"DISABLED");
            )

            break;

        case GPU_COLOR_MASK_A:
            /*  Write alpha component write mask register.  */
            writeA = data.booleanVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_COLOR_MASK_A = %s.\n", writeA?"ENABLED":"DISABLED");
            )

            break;

        case GPU_LOGICAL_OPERATION:
            /*  Write logical operation enable flag register.  */
            logicOperation = data.booleanVal;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_LOGICAL_OPERATION = %s.\n", writeR?"ENABLED":"DISABLED");
            )

            break;

        case GPU_LOGICOP_FUNCTION:
            /*  Write logical operation function register.  */
            logicOpMode = data.logicOp;

            GPU_DEBUG(
                printf("ColorWrite => Write GPU_LOGICOP_FUNCTION = ");

                switch(logicOpMode)
                {
                    case LOGICOP_CLEAR:
                        printf("CLEAR.\n");
                        break;

                    case LOGICOP_AND:
                        printf("AND.\n");
                        break;

                    case LOGICOP_AND_REVERSE:
                        printf("AND_REVERSE.\n");
                        break;

                    case LOGICOP_COPY:
                        printf("COPY.\n");
                        break;

                    case LOGICOP_AND_INVERTED:
                        printf("AND_INVERTED.\n");
                        break;

                    case LOGICOP_NOOP:
                        printf("NOOP.\n");
                        break;

                    case LOGICOP_XOR:
                        printf("XOR.\n");
                        break;

                    case LOGICOP_OR:
                        printf("OR.\n");
                        break;

                    case LOGICOP_NOR:
                        printf("NOR.\n");
                        break;

                    case LOGICOP_EQUIV:
                        printf("EQUIV.\n");
                        break;

                    case LOGICOP_INVERT:
                        printf("INVERT.\n");
                        break;

                    case LOGICOP_OR_REVERSE:
                        printf("OR_REVERSE.\n");
                        break;

                    case LOGICOP_COPY_INVERTED:
                        printf("COPY_INVERTED.\n");
                        break;

                    case LOGICOP_OR_INVERTED:
                        printf("OR_INVERTED.\n");
                        break;

                    case LOGICOP_NAND:
                        printf("NAND.\n");
                        break;

                    case LOGICOP_SET:
                        printf("SET.\n");
                        break;

                    default:
                        panic("ColorWrite", "processRegisterWrite", "Unsupported logical operation function.");
                        break;
                }

            )

            break;

        default:
            panic("ColorWrite", "processRegisterWrite", "Unsupported register.");
            break;
    }

}

/*  Processes a fragment.  */
void ColorWrite::processStamp()
{
    Fragment *fr;
    QuadFloat *attrib;
    u32bit address;
    s32bit x, y;
    u32bit i;
    u32bit j;
    bool cullStamp;

    /*  Get first stamp fragment.  */
    fr = stamp[0]->getFragment();

    /*  Copy the stamp cookies.  */
    blendQueue[freeBlend].stampCookies->copyParentCookies(*stamp[0]);
    blendQueue[freeBlend].stampCookies->removeCookie();

    /*  Check if first fragment exists (not last fragment).  */
    if (fr != NULL)
    {
        /*  Get first stamp fragment attributes.  */
        attrib = stamp[0]->getAttributes();

        /*  Get fragment coordinates.  */
        //x = (s32bit) attrib[POSITION_ATTRIBUTE][0];
        //y = (s32bit) attrib[POSITION_ATTRIBUTE][1];
        x = fr->getX();
        y = fr->getY();

        /*  Calculate address of the first stamp fragment pixel in the color
            buffer.  */
        address = GPUMath::pixel2Memory(x, y, startX, startY, width, height,
            hRes, vRes, STAMP_WIDTH, STAMP_WIDTH, genW, genH, scanW, scanH, overW, overH, bytesPixel);

        /*  Add the current back (secondary) color buffer base address.  */
        address += backBuffer;

        /*  Set color buffer address for the stamp.  */
        blendQueue[freeBlend].address = address;

        /*  Calculate offset inside a line cache.  */
        blendQueue[freeBlend].offset = address & stampMask;

        /*  Set stamp x and y coordinates.  */
        blendQueue[freeBlend].x = x;
        blendQueue[freeBlend].y = y;

        /*  Set stamp fragment cycle.  */
        blendQueue[freeBlend].startCycle = stamp[0]->getStartCycle();

        /*  Set stamp latency in the shader units.  */
        blendQueue[freeBlend].shaderLatency = stamp[0]->getShaderLatency();
    }
    else
    {
        /*  Last fragment/stamp received.  */
        lastFragment = TRUE;
    }

    /*  Store the stamp fragment data in the blend queue entry.  */
    for(i = 0; (i < STAMP_FRAGMENTS) && (!lastFragment); i++)
    {
        /*  Get stamp fragment.  */
        fr = stamp[i]->getFragment();

        /*  Check if the fragment is inside the triangle.
            Cull the fragment. */
        if ((fr != NULL) && (!stamp[i]->isCulled()))
        {
            /*  Not culled fragment and fragment inside the triangle.  */

            /*  Get fragment attributes.  */
            attrib = stamp[i]->getAttributes();

            GPU_DEBUG(
                /*  Get fragment position.  */
                //x = (s32bit) attrib[POSITION_ATTRIBUTE][0];
                //y = (s32bit) attrib[POSITION_ATTRIBUTE][1];
                x = fr->getX();
                y = fr->getY();
                printf("ColorWrite => Received Fragment Not Culled (%f, %f) = {%f, %f, %f, %f} %s.\n",
                    (f32bit) x, (f32bit) y,
                    attrib[COLOR_ATTRIBUTE][0], attrib[COLOR_ATTRIBUTE][1], attrib[COLOR_ATTRIBUTE][2],
                    attrib[COLOR_ATTRIBUTE][3], fr->isInsideTriangle()?"IN":"OUT");
            )

            /*  Queue the fragment in the blend queue.  */
            blendQueue[freeBlend].inColor[i][0] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE][0], 0.0f, 1.0f);
            blendQueue[freeBlend].inColor[i][1] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE][1], 0.0f, 1.0f);
            blendQueue[freeBlend].inColor[i][2] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE][2], 0.0f, 1.0f);
            blendQueue[freeBlend].inColor[i][3] = GPU_CLAMP(attrib[COLOR_ATTRIBUTE][3], 0.0f, 1.0f);

            /*  Set write mask for the fragment.  */
            blendQueue[freeBlend].mask[i * bytesPixel] = writeR;
            blendQueue[freeBlend].mask[i * bytesPixel + 1] = writeG;
            blendQueue[freeBlend].mask[i * bytesPixel + 2] = writeB;
            blendQueue[freeBlend].mask[i * bytesPixel + 3] = writeA;

            /*  Delete fragment attributes.  */
            delete[] attrib;

            /*  Delete the fragment in the emulator.  */
            delete fr;

            /*  Delete fragment input.  */
            delete stamp[i];
        }
        else
        {
            /*  Fragment culled or outside the triangle.  */

            /*  Set fragment as not to be written.  */
            for(j = 0; j < bytesPixel; j++)
                blendQueue[freeBlend].mask[i * bytesPixel + j] = FALSE;

            /*  Get fragment attributes.  */
            attrib = stamp[i]->getAttributes();

            GPU_DEBUG(
                if (fr == NULL)
                {
                    printf("ColorWrite => Received Last Fragment (NULL).\n");
                }
                else
                {
                    printf("ColorWrite => Received Fragment Culled (%f, %f) = {%f, %f, %f} %s.\n",
                        (f32bit) x, (f32bit) y,
                        attrib[COLOR_ATTRIBUTE][0], attrib[COLOR_ATTRIBUTE][1], attrib[COLOR_ATTRIBUTE][2],
                        fr->isInsideTriangle()?"IN":"OUT");
                }
            )


            /*  Update statistics.  */
            outside->inc();

            /*  Check if there is a fragment before deleting it.  */
            if (fr != NULL)
            {
                /*  Delete fragment attributes.  */
                delete[] attrib;

                /*  Delete the fragment in the emulator.  */
                delete fr;
            }

            /*  Delete fragment input.  */
            delete stamp[i];
        }
    }

    if (!lastFragment)
    {
        /*  Calculate if the stamp should be culled (all fragments masked).  */
        for(i = 0, cullStamp = TRUE; i < STAMP_FRAGMENTS * bytesPixel; i++)
            cullStamp &= !blendQueue[freeBlend].mask[i];

        /*  Check if the whole stamp can be culled.  */
        if (!cullStamp)
        {
            /*  Update fetch stamp counter.  */
            fetchStamps++;

            /*  Update free blend queue entries counter.  */
            freeStamps--;

            /*  Move pointer to the next blend queue free entry.  */
            freeBlend = GPU_MOD(freeBlend + 1, blendQueueSize);
        }
        else
        {
            GPU_DEBUG(
                printf("ColorWrite => Culling stamp.\n");
            )

            /*  Update statistics.  */
            culled->inc(STAMP_FRAGMENTS);
        }
    }
    else
    {
        /*  Delete all fragment input from the last stamp.  */
        for(i = 0; i < STAMP_FRAGMENTS; i++)
            delete stamp[i];
    }

}


/*  Processes a memory transaction.  */
void ColorWrite::processMemoryTransaction(u64bit cycle,
    MemoryTransaction *memTrans)
{
    /*  Get transaction type.  */
    switch(memTrans->getCommand())
    {
        case MT_STATE:

            /*  Received state of the Memory controller.  */
            memoryState = memTrans->getState();

            GPU_DEBUG(
                printf("ColorWrite => Memory Controller State = %d\n",
                    memoryState);
            )

            break;

        default:

            GPU_DEBUG(
                printf("ColorWrite => Memory Transaction to the Color Cache.\n");
            )

            /*  Let the color cache process any other transaction.  */
            colorCache->processMemoryTransaction(memTrans);

            /*  Update statistics.  */
            readBytes->inc(memTrans->getSize());

            break;
    }

    /*  Delete processed memory transaction.  */
    delete memTrans;
}


/*  Converts color data from integer format to float point format.  */
void ColorWrite::colorInt2Float(u8bit *in, QuadFloat *out)
{
    u32bit i;

    /*  Check number of bytes per pixel.  */
    GPU_ASSERT(
        if (bytesPixel != 4)
            panic("ColorWrite", "colorInt2Float", "Unsupported pixel format.");
    )

    /*  Convert all pixels in the stamp.  */
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        /*  Convert 8bit integer color components to 32 bit float point
            color components.  */
        out[i][0] = f32bit(in[i * 4]) * (1.0f / 255.0f);
        out[i][1] = f32bit(in[i * 4 + 1]) * (1.0f / 255.0f);
        out[i][2] = f32bit(in[i * 4 + 2]) * (1.0f / 255.0f);
        out[i][3] = f32bit(in[i * 4 + 3]) * (1.0f / 255.0f);
    }

}

/*  Converts color data from float poing format to integer format.  */
void ColorWrite::colorFloat2Int(QuadFloat *in, u8bit *out)
{
    u32bit i;

    /*  Check number of bytes per pixel.  */
    GPU_ASSERT(
        if (bytesPixel != 4)
            panic("ColorWrite", "colorInt2Float", "Unsupported pixel format.");
    )

    /*  Convert all pixels in the stamp.  */
    for(i = 0; i < STAMP_FRAGMENTS; i++)
    {
        /*  Convert 32bit float point color components to 8 bit integer
            color components.  */
        out[i * 4] = u32bit(255.0f * GPU_CLAMP(in[i][0], 0.0f, 1.0f));
        out[i * 4 + 1] = u32bit(255.0f * GPU_CLAMP(in[i][1], 0.0f, 1.0f));
        out[i * 4 + 2] = u32bit(255.0f * GPU_CLAMP(in[i][2], 0.0f, 1.0f));
        out[i * 4 + 3] = u32bit(255.0f * GPU_CLAMP(in[i][3], 0.0f, 1.0f));
    }

}



/*  UNUSED PIECES OF CODE  */

/*  OLD VERSION OF THE CLEAR STATE.  CLEARS MEMORY RATHER THAN THE
    BLOCK STATE TABLE IN CURRENT IMPLEMENTATION.  */

void oldClear()
{
    MemoryTransaction *memTrans;
    MemState memoryState;
    u32bit busCycles = 0;
    u32bit freeTickets = 0;
    u32bit clearAddress;
    u32bit endClearAddress;
    u32bit ticket;
    u32bit cycle;
    Signal *memRequest;
    u8bit *clearBuffer;
    RasterizerCommand *lastRSCommand;
    RasterizerState state;

    /*  Clear color buffer state.  */

    GPU_DEBUG(
        printf("ColorWrite => Clear state.\n");
    )

    /*  Check if memory bus is busy.  */
    if (busCycles > 0)
    {
        /*  Update bus use counter.  */
        busCycles--;

        GPU_DEBUG(
            printf("ColorWrite => Waiting %d cycles for end of transmission.\n",
                busCycles);
        )

        /*  Check end of transmission.  */
        if (busCycles == 0)
        {
            GPU_DEBUG(
                printf("ColorWrite => End of transmission\n");
            )

            /*  Update free memory tickets counter.  */
            freeTickets++;
        }
    }

    /**  NOTE:  THE COLOR BUFFER SHOULD BE A MULTIPLE OF
        MAX_TRANSACTION_SIZE (128?) OR HAVE ENOUGH PADDING
        SO THE LAST TRANSACTION DOES NOT OVERWRITES ANYTHING.  **/

    /*  Check if memory controller and the bus are ready and
        there is more data to send.  */
    if ((busCycles == 0) && ((memoryState & MS_WRITE_ACCEPT) != 0) &&
        (clearAddress < endClearAddress))
    {

        /*  Check there are enough memory tickets.  */
        GPU_ASSERT(
            if (freeTickets == 0)
                panic("ColorWrite", "clock", "No free memory tickets.");
        )

        GPU_DEBUG(
            printf("ColorWrite => Writing to memory at %x %d bytes.\n",
                clearAddress, MAX_TRANSACTION_SIZE);
        )

                /*  Create write transaction.  */
        memTrans = new MemoryTransaction(
            MT_WRITE_DATA,
            clearAddress,
            MAX_TRANSACTION_SIZE,
            clearBuffer,
            COLORWRITE,
            ticket++
        );

        /*  Copy cookies from the last DRAW command from the
            Command Processor.  */
        memTrans->copyParentCookies(*lastRSCommand);

        /*  Add a cookie level.  */
        memTrans->addCookie();

        /*  Send transaction to memory controller.  */
        memRequest->write(cycle, memTrans);

        /*  Set bus counter.  */
        busCycles = memTrans->getBusCycles();

        /*  Update clear address.  */
        clearAddress += MAX_TRANSACTION_SIZE;

        /*  Update free ticket counter.  */
        freeTickets--;
    }

    /*  Check if the full color buffer has been cleared.  */
    if ((busCycles == 0) && (clearAddress >= endClearAddress))
    {
        /*  Change to end state.  */
        state = RAST_END;
    }

}

/*  Returns the unit fragment latency map.  */
u32bit *ColorWrite::getLatencyMap(u32bit &mapWidth, u32bit &mapHeight)
{
    mapWidth = (hRes >> 1) + (hRes & 0x01);
    mapHeight = (vRes >> 1) + (vRes & 0x01);

    return latencyMap;
}


void ColorWrite::getState(string &stateString)
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
        case RAST_SWAP:
            stateStream << "RAST_SWAP";
            break;
        default:
            stateStream << "undefined";
            break;
    }

    stateStream << " | Fragment Counter = " << fragmentCounter;
    stateStream << " | Triangle Counter = " << triangleCounter;
    stateStream << " | Last Fragment = " << lastFragment;
    stateStream << " | Fetch Stamps = " << fetchStamps;
    stateStream << " | Write Stamps = " << writeStamps;

    stateString.assign(stateStream.str());
}


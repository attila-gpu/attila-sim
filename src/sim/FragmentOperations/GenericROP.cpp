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
 * $RCSfile: GenericROP.cpp,v $
 * $Revision: 1.10 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:47 $
 *
 * Generic ROP box implementation file.
 *
 */

/**
 *
 *  @file GenericROP.cpp
 *
 *  This file implements the Generic ROP box.
 *
 *  This class implements a generic ROP pipeline that will be used
 *  to implement the Z Stencil and Color Write stages of the GPU pipeline.
 *
 */

#include "GenericROP.h"
#include "MemoryTransaction.h"
#include "RasterizerStateInfo.h"
#include "ROPOperation.h"
#include "ROPStatusInfo.h"
#include "GPUMath.h"
#include "GPU.h" // Access to STAMP_FRAGMENTS

#include <sstream>

using namespace gpu3d;

using gpu3d::tools::Queue;

/*  Generic ROP box constructor.  */
GenericROP::GenericROP(

    //  Rate and latency parameters
    u32bit numStamps, u32bit ropOpRate, u32bit ropLat,

    //  Buffer parameters
    u32bit overWidth, u32bit overHeight, u32bit scanWidth, u32bit scanHeight, u32bit genWidth,
    u32bit genHeight,

    //  Cache parameters
    //u32bit cacheWays, u32bit cacheLines, u32bit stampsPerLine, u32bit portWidth, bool extraReadP,
    //bool extraWriteP, u32bit cacheReqQSz, u32bit inputReqQSz, u32bit outputReqQSz, u32bit blocks,
    //u32bit blocksCycle, u32bit comprCycles, u32bit decomprCycles, bool comprDisabled,

    //  Queue parameters
    u32bit inQSz, u32bit fetchQSz, u32bit readQSz, u32bit opQSz, u32bit writeQSz,

    //  Emulator classes
    FragmentOpEmulator &fragEmu,

    //  Other parameters
    char *stageName, char *stageShortName, char *name, char *prefix, Box *parent) :

    stampsCycle(numStamps), ropRate(ropOpRate), ropLatency(ropLat),

    overH(overHeight), overW(overWidth), scanH((u32bit) ceil(scanHeight / (f32bit) genHeight)),
    scanW((u32bit) ceil(scanWidth / (f32bit) genWidth)), genH(genHeight / STAMP_HEIGHT),
    genW(genWidth / STAMP_WIDTH),

    inQueueSize(inQSz), fetchQueueSize(fetchQSz), readQueueSize(readQSz), opQueueSize(opQSz),
    writeQueueSize(writeQSz),

    frEmu(fragEmu),

    ropName(stageName), ropShortName(stageShortName), Box(name, parent)

{
    char fullName[64];
    char postfix[32];

    /*  Check parameters.  */
    GPU_ASSERT(
        if (stampsCycle == 0)
            panic(ropName, ropName, "At least a stamp must be received per cycle.");
        if (overW == 0)
            panic(ropName, ropName, "Over scan tile must be at least 1 scan tile wide.");
        if (overH == 0)
            panic(ropName, ropName, "Over scan tile must be at least 1 scan tile high.");
        if (scanW == 0)
            panic(ropName, ropName, "Scan tile must be at least 1 generation tile wide.");
        if (scanH == 0)
            panic(ropName, ropName, "Scan tile must be at least 1 generation tile high.");
        if (genW == 0)
            panic(ropName, ropName, "Scan tile must be at least 1 stamp wide.");
        if (genH == 0)
            panic(ropName, ropName, "Scan tile must be at least 1 stamp high.");

        if (inQueueSize < 2)
            panic(ropName, ropName, "ROP input stamp queue requires at least two entries.");
        if (fetchQueueSize < 1)
            panic(ropName, ropName, "ROP fetch stamp queue requires at least one entry.");
        if (readQueueSize < 1)
            panic(ropName, ropName, "ROP read stamp queue requires at least one entry.");
        if (opQueueSize < ropLatency)
            panic(ropName, ropName, "ROP operation stamp queue requires at least as many entries as the ROP operation latency.");
        if (writeQueueSize < 1)
            panic(ropName, ropName, "ROP written stamp queue requires at least one entry.");

        if (ropRate == 0)
            panic(ropName, ropName, "ROP operation rate must be at least 1.");
        if (ropLatency < 1)
            panic(ropName, ropName, "ROP operation latency must be 1 or greater.");
    )

    /*  Create the full name and postfix for the statistics.  */
    sprintf(fullName, "%s::%s", prefix, name);
    sprintf(postfix, "%s-%s", ropShortName, prefix);

    /*  Create statistics.  */
    inputs = &getSM().getNumericStatistic("InputFragments", u32bit(0), fullName, postfix);
    operated = &getSM().getNumericStatistic("OperatedFragments", u32bit(0), fullName, postfix);
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
    ropOpBusy = &getSM().getNumericStatistic("ROPOperatorBusy", u32bit(0), fullName, postfix);
    consumerBusy = &getSM().getNumericStatistic("ConsumerBusy", u32bit(0), fullName, postfix);
    inputEmpty = &getSM().getNumericStatistic("InputQEmpty", u32bit(0), fullName, postfix);
    fetchEmpty = &getSM().getNumericStatistic("FetchQEmpty", u32bit(0), fullName, postfix);
    readEmpty = &getSM().getNumericStatistic("ReadQEmpty", u32bit(0), fullName, postfix);
    opEmpty = &getSM().getNumericStatistic("OperationQEmpty", u32bit(0), fullName, postfix);
    writeEmpty = &getSM().getNumericStatistic("WriteQEmpty", u32bit(0), fullName, postfix);
    fetchFull = &getSM().getNumericStatistic("FetchQFullStall", u32bit(0), fullName, postfix);
    readFull = &getSM().getNumericStatistic("ReadQFullStall", u32bit(0), fullName, postfix);
    opFull = &getSM().getNumericStatistic("OperationQFullStall", u32bit(0), fullName, postfix);
    writeFull = &getSM().getNumericStatistic("WriteQFullStall", u32bit(0), fullName, postfix);

    //
    //  WARNING
    //
    //  Signal creation and related initializations must be performed in the
    //  derived class constructor
    //


    //
    //  WARNING.
    //
    //  The ROP Cache should be created and initialized in the derived class
    //  constructor from a Cache class derived from the generic ROP cache class.
    //


    string queueName;

    //  Set queue names.
    queueName.clear();
    queueName.append("InputStampQueue");
    queueName.append(postfix);
    inQueue.setName(queueName);
    queueName.clear();
    queueName.append("FetchStampQueue");
    queueName.append(postfix);
    fetchQueue.setName(queueName);
    queueName.clear();
    queueName.append("ReadStampQueue");
    queueName.append(postfix);
    readQueue.setName(queueName);
    queueName.clear();
    queueName.append("OpStampQueue");
    queueName.append(postfix);
    opQueue.setName(queueName);
    queueName.clear();
    queueName.append("WriteStampQueue");
    queueName.append(postfix);
    writeQueue.setName(queueName);
    queueName.append("FreeStampQueue");
    queueName.append(postfix);
    freeQueue.setName(queueName);


    //  Create and initialize the ROP stage queues.
    inQueue.resize(inQueueSize);
    fetchQueue.resize(fetchQueueSize);
    readQueue.resize(readQueueSize);
    opQueue.resize(opQueueSize);
    writeQueue.resize(writeQueueSize);

    //
    //  WARNING:
    //
    //  The free stamp queue must be initialized in the derived class
    //  constructor.
    //
    //

    //  Calculate the total number of stamps in the free stamp queue.
    u32bit freeQueueSize = inQueueSize + fetchQueueSize + readQueueSize +
        ropLatency + opQueueSize + writeQueueSize;


    //  Calculate the size of the CAM used to detect RAW dependences in the
    //  ROP pipeline.
    sizeCAM = readQueueSize + ropLatency + opQueueSize;

    //  Initialize the read non-written CAM for RAW dependence detection.
    rawCAM.clear();
    rawCAM.resize(sizeCAM);

    //  Initialize the read non-written CAM.
    for(int i = 0; i < rawCAM.size(); i++)
        rawCAM[i] = NULL;

    //  Initialize RAW CAM counters and pointers.
    freeCAM = firstCAM = 0;
    stampsCAM = 0;

    //
    //  WARNING:
    //
    //  Who can initialize the free queue?  Only the derived classes know the specific ROPQueue
    //  element size.

    //  Initialize the ROP Queue objects in the free stamp queue.
    //for(i = 0; i < freeQueueSize; i++)
    //    freeQueue.add(new ROPQueue);

    //  Set initial state to reset.
    state = RAST_RESET;
}

/*  Generic ROP simulation function.  */
void GenericROP::clock(u64bit cycle)
{
    MemoryTransaction *memTrans;
    RasterizerCommand *rastComm;
    ROPStatusInfo *consumerStateInfo;

    GPU_DEBUG_BOX(
        printf("%s => Clock %lld\n", getName(), cycle);
    )

    //  Receive state from Memory Controller.
    while(memData->read(cycle, (DynamicObject *&) memTrans))
        processMemoryTransaction(cycle, memTrans);

    //  Update ROP cache structures.
    memTrans = ropCache->update(cycle, memoryState);

    //  Call the specific post cache update/clock function.
    postCacheUpdate(cycle);

    //  Check if a memory transaction was generated.
    if (memTrans != NULL)
    {
        //  Send memory transaction to the Memory Controller.
        memRequest->write(cycle, memTrans);

        //  Update statistics.
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
                panic(getName(), "clock", "Unknown memory transaction.");
        }
    }

    //  Check if there is a consumer stage attached to the Generic ROP pipeline
    //  that receives processed fragments from the Generic ROP.
    if (consumerStateSignal != NULL)
    {
        //  Receive state from the consumer stage.
        if (consumerStateSignal->read(cycle, (DynamicObject *&) consumerStateInfo))
        {
            //  Get consumer state.
            consumerState = consumerStateInfo->getState();

            //  Delete state container object.
            delete consumerStateInfo;
        }
        else
        {
            panic(getName(), "clock", "Missing state signal from consumer stage.");
        }
    }
    else
    {
        //  No consumer so always ready to process written stamps.
        consumerState = ROP_READY;
    }

    //  Simulate current cycle.
    switch(state)
    {
        case RAST_RESET:

            //  Reset state.

            GPU_DEBUG_BOX(
                printf("%s => Reset state.\n", getName());
            )

            //  Call the specific reset function.
            reset();

            //  Reset the ROP cache.
            ropCache->reset();
            ropCache->setBytesPerPixel(bytesPixel[0]);
            ropCache->setCompression(compression);
            ropCache->update(cycle, memoryState);

            //  Set ROP buffer address in the Z cache.
            ropCache->swap(bufferAddress[0]);

            //  Reset rop operation start cycle counter.
            ropCycles = 0;

            //  Change state to ready.
            state = RAST_READY;

            break;

        case RAST_READY:

            //  Ready state.

            GPU_DEBUG_BOX(
                printf("%s => Ready state.\n", getName());
            )

            //  Receive and process a rasterizer command.
            if(rastCommand->read(cycle, (DynamicObject *&) rastComm))
            {
                //  Call the specific process command function
                processCommand(rastComm, cycle);
            }

            break;

        case RAST_DRAWING:

            //  Draw state.

            GPU_DEBUG_BOX(
                printf("%s => Drawing state.\n", getName());
            )

            //  Check end of the batch.
            if (lastFragment && freeQueue.full() && (memTrans == NULL))
            {
                GPU_DEBUG_BOX(
                    printf("%s (%p) => Sending last stamp\n", getName(), this);
                )

                //  Call the specific end batch function of the derived class.
                endBatch(cycle);

                //  Change state to end.
                state = RAST_END;
            }

            //  Terminate the processing of the stamp.
            terminateStamp(cycle);

            //  Write stamp
            writeStamp();

            //  End ROP operation
            endOperation(cycle);

            //  Start ROP operation
            startOperation(cycle);

            //  Read stamp data
            readStamp();

            //  Fetch stamp data
            fetchStamp();

            //  Receive stamps.
            receiveStamps(cycle);

            break;

        case RAST_END:

            /*  End of batch state.  */

            GPU_DEBUG_BOX(
                printf("ZStencilTest => End state.\n");
            )

            /*  Wait for end command.  */
            if (rastCommand->read(cycle, (DynamicObject *&) rastComm))
                processCommand(rastComm, cycle);

            break;

        case RAST_SWAP:

            /*  Swap buffer state.  */

            GPU_DEBUG_BOX(
                printf("%s => Swap state.\n", getName());
            )

            swap(cycle);

            break;

        case RAST_FLUSH:

            //  ROP Flush state.

            GPU_DEBUG_BOX(
                printf("%s => Flush state.\n", getName());
            )

            flush(cycle);

            break;

        case RAST_SAVE_STATE:

            //  ROP Save State state.

            GPU_DEBUG_BOX(
                printf("%s => Save State state.\n", getName());
            )

            //  Check if the save state request has finished.
            if (!endFlush)
            {
                //  Continue saving the block state info into memory.
                if(!ropCache->saveState())
                {
                    //  Set flush end.
                    endFlush = true;
                }
            }
            else
            {
                //  End of save state.
                state = RAST_END;
            }

            break;

        case RAST_RESTORE_STATE:

            //  ROP Restore State state.

            GPU_DEBUG_BOX(
                printf("%s => Restore State state.\n", getName());
            )

            //  Check if the restore state request has finished.
            if (!endFlush)
            {
                //  Continue saving the block state info into memory.
                if(!ropCache->restoreState())
                {
                    //  Set flush end.
                    endFlush = true;
                }
            }
            else
            {
//printf("%s (%lld) => End of RAST_RESTORE_STATE.\n", getName(), cycle);
                //  End of swap.
                state = RAST_END;
            }

            break;

        case RAST_RESET_STATE:
                   
            //  ROP Reset Block State state.

            GPU_DEBUG_BOX(
                printf("%s => Reset Block State state.\n", getName());
            )

            //  Check if the reset block state request has finished.
            if (!endFlush)
            {
                //  Continue reset of the block state until it has finished..
                if(!ropCache->resetState())
                {
                    //  Set flush end.
                    endFlush = true;
                }
            }
            else
            {
                //  End of swap.
                state = RAST_END;
            }

            break;
        
        case RAST_CLEAR:

            //  Clear Z and stencil buffer state.

            GPU_DEBUG_BOX(
                printf("%s => Clear state.\n", getName());
            )

            //  Call the specific clear function
            clear();

            break;

        default:

            panic(getName(), "clock", "Unsupported state.");
            break;

    }

    //  Send current state to stage that produces the fragments to be processed
    //    by the ROP stage.
    if (inQueue.items() < (inQueueSize - (2 * stampsCycle)))
    {
        GPU_DEBUG_BOX(
            printf("%s => Sending READY.\n", getName());
        )

        //  Send a ready signal.
        ropStateSignal->write(cycle, new ROPStatusInfo(ROP_READY));
    }
    else
    {
        GPU_DEBUG_BOX(
            printf("%s => Sending BUSY.\n", getName());
        )

        //  Send a busy signal.
        ropStateSignal->write(cycle, new ROPStatusInfo(ROP_BUSY));
    }

    //  Send current rasterizer state.
    rastState->write(cycle, new RasterizerStateInfo(state));
}


//  Processes a stamp.
void GenericROP::processStamp()
{
    Fragment *fr;
    QuadFloat *attrib;
    u32bit address;
    s32bit x, y;
    u32bit i;
    bool cullStamp;
    ROPQueue *currentStamp;

    //  Get a free queue object from the free queue
    if (!freeQueue.empty())
        currentStamp = freeQueue.pop();
    else
        panic(getName(), "processStamp", "No free queue objects.");

    //  Keep the stamp in the queue.
    currentStamp->stamp = stamp;

    //  Get first stamp fragment.
    fr = stamp[0]->getFragment();

    //  NOTE: ALL THE FRAGMENTS IN THE LAST STAMP MUST BE NULL.

    //  Check if first fragment exists (not last fragment).
    if (fr != NULL)
    {
        //
        //  WARNNING
        //  Z BUFFER AND COLOR BUFFER COULD USE DIFFERENT MEMORY LAYOUTS, SO THEY MAY HAVE
        //  TO USE DIFFERENT FUNCTIONS TO CALCULATE THE ADDRESS.  FOR NOW WE ARE USING
        //  THE SAME FUNCTION.
        //
        //  IF IT IS ALWAYS THE SAME ADDRESS IT WOULD BE BETTER TO CALCULATE THE ADDRESS
        //  AT RASTERIZATION AND STORE IT AS A FRAGMENT PARAMETER.
        //

        //  Get fragment attributes.
        attrib = stamp[0]->getAttributes();

        //  Get fragment position.
        //x = (s32bit) attrib[POSITION_ATTRIBUTE][0];
        //y = (s32bit) attrib[POSITION_ATTRIBUTE][1];
        x = fr->getX();
        y = fr->getY();

        //  Compute the addresses for all the active output buffers.
        for(u32bit b = 0, current = 0; b < numActiveBuffers; b++, current++)
        {
            //  Search for the next active buffer.
            for(;(!activeBuffer[current]) && (current < MAX_RENDER_TARGETS); current++);
            
            GPU_ASSERT(
                if (current == MAX_RENDER_TARGETS)
                    panic("GenericROP", "processStamp", "Expected an active output buffer.");
            )
            
            address = pixelMapper[current].computeAddress(x, y);
            
            //  Add the current ROP buffer base address.
            address += bufferAddress[current];

            //  Set ROP buffer address for the stamp.
            currentStamp->address[current] = address;
        }
        
        //  Reset output buffer counter.
        currentStamp->nextBuffer = 0;
        
        //  Reset multisampling counter.
        currentStamp->nextSample = 0;
        
        //  Store the stamp fragment data in the ROP queue entry.
        for(i = 0; (i < STAMP_FRAGMENTS) && (!lastFragment); i++)
        {
            //  Get stamp fragment.
            fr = stamp[i]->getFragment();

            //  Check if fragment is NULL.
            GPU_ASSERT(
                if (fr == NULL)
                    panic(getName(), "processStamp", "NULL fragment not in last stamp.");
            )

//if ((fr->getX() == 82) && (fr->getY() == 890))
//{
//printf("%s => Fragment (%d, %d) triangleID %d passed through here, culled %s\n", getName(), 82, 890,
//stamp[i]->getTriangleID(), stamp[i]->isCulled() ? "T" : "F");
//}

            //  Set fragment cull flag in the stamp.
            currentStamp->culled[i] = stamp[i]->isCulled();

            //  Check if the fragment is culled.
            if (!stamp[i]->isCulled())
            {
                //  Not last fragment and fragment inside the triangle.

                //  Get fragment attributes.
                attrib = stamp[i]->getAttributes();

                GPU_DEBUG_BOX(
                    //  Get fragment position.
                    //x = (s32bit) attrib[POSITION_ATTRIBUTE][0];
                    //y = (s32bit) attrib[POSITION_ATTRIBUTE][1];
                    x = fr->getX();
                    y = fr->getY();
                    printf("%s => Received Fragment Not Culled (%d, %d) Inside? %s.\n",
                        getName(), x, y, fr->isInsideTriangle()?"IN":"OUT");
                )
            }
            else
            {
                //  Fragment culled.
                GPU_DEBUG_BOX(
                    printf("%s => Received Fragment Culled (%d, %d) Inside? %s.\n",
                        getName(),
                        fr->getX(), fr->getY(),
                        fr->isInsideTriangle()?"IN":"OUT");
                )

                //  Update statistics.
                outside->inc();
            }
        }

        //  Calculate if the stamp should be culled (all fragments masked).
        for(i = 0, cullStamp = TRUE; i < STAMP_FRAGMENTS; i++)
            cullStamp = cullStamp && currentStamp->culled[i];

        //  Check if the whole stamp can be culled.
        if (!cullStamp)
        {
            //  The stamp can not be culled

            //  Add the stamp to the input queue.
            inQueue.add(currentStamp);
        }
        else
        {

            //  Stamp can be culled.
            GPU_DEBUG_BOX(
                printf("%s => Culling whole stamp.\n", getName());
            )

            //  Delete stamp.
            for(i = 0; i < STAMP_FRAGMENTS; i++)
            {
                //  Get fragment.
                fr = stamp[i]->getFragment();

                //  Get fragment attribute.
                attrib = stamp[i]->getAttributes();

                //  Delete fragment.
                delete fr;

                //  Delete attributes.
                delete[] attrib;

                //  Delete fragment input.
                delete stamp[i];

                //  Update statistics.
                culled->inc();
            }

            //  Delete stamp.
            delete[] stamp;

            //  Return stamp queue object to the free queue.
            freeQueue.add(currentStamp);
        }
    }
    else
    {
        //  WARNING
        //  THE LAST STAMP MUST ALWAYS BE THE LAST STAMP IN A BATCH.
        //  IT IS STORED IN THE LAST QUEUE ENTRY.
        //

        //  Last fragment/stamp received.
        lastFragment = TRUE;

        //  Set last batch stamp data.
        lastBatchStamp.lastStamp = true;
        lastBatchStamp.stamp = stamp;

        //  Return the stamp queue object to the free queue.
        freeQueue.add(currentStamp);

        GPU_DEBUG_BOX(
            printf("%s (%p) => Received last stamp\n", getName(), this);
        )
    }
}

//  Receive stamps from the producer stage.
void GenericROP::receiveStamps(u64bit cycle)
{
    FragmentInput *frInput;
    bool receivedFragment;
    int i, j;

    GPU_DEBUG_BOX(
        cout << getName() << " => Waiting for " << stampsCycle << " stamps" << endl;
    )

    //  Receive N stamps from the producer stage
    for(i = 0; i < stampsCycle; i++)
    {
        //  Allocate the current stamp.
        stamp = new FragmentInput*[STAMP_FRAGMENTS];

        //  Reset received fragment flag
        receivedFragment = true;

        //  Receive all the fragments from a stamp.
        for(j = 0; (j < STAMP_FRAGMENTS) && receivedFragment; j++)
        {
            //*  Get fragment from producer stage input signal.
            receivedFragment = inFragmentSignal->read(cycle, (DynamicObject *&) frInput);

            //  Check if a fragment has been received.
            if (receivedFragment)
            {
                GPU_DEBUG_BOX(
                    cout << getName() << " => Received fragment " << j << " for stamp " << i << endl;
                )

                // Store fragment in the current stamp.
                stamp[j] = frInput;

                //  Count triangles.
                if (frInput->getTriangleID() != currentTriangle)
                {
                    //  Change current triangle.
                    currentTriangle = frInput->getTriangleID();

                    //  Update triangle counter.
                    triangleCounter++;
                }

                //  Update fragment counter.
                fragmentCounter++;

                //  Update statistics.
                inputs->inc();
            }
        }

        //  Check if a whole stamp has been received.
        GPU_ASSERT(
            if (!receivedFragment && (j > 1))
                panic(getName(), "clock", "Missing fragments in a stamp.");
        )

        //  Check if a stamp has been received.
        if (receivedFragment)
        {
            //  Process the received stamp.
            processStamp();
        }
        else
        {
            delete[] stamp;
        }
    }
}

//  Fetch data for the current input stamp.
void GenericROP::fetchStamp()
{
    ROPQueue *currentStamp;

    //  Check if there are stamps in the input stamp queue waiting to be fetched.
    if (!inQueue.empty())
    {
        //  Get the current stamp from the head of the input stamp queue.
        currentStamp = inQueue.head();

        //  Check if bypass flag for the ROP stage is enabled or disabled.
        if (!bypassROP[currentStamp->nextBuffer])
        {
            //  Check if read data flag is enabled.
            if (readDataROP[currentStamp->nextBuffer])
            {
                //  The stamp requires to read data from the ROP buffer before processing.

                //  Check if the fetch stamp queue has free entries.
                if (!fetchQueue.full())
                {
                    bool fetchResult;
                    
                    //  Check if multisampling is enabled.
                    if (!multisampling)
                    {
                        //  Try to fetch data for the next stamp in the queue.
                        fetchResult = ropCache->fetch(currentStamp->address[currentStamp->nextBuffer],
                                                      currentStamp->way[currentStamp->nextBuffer],
                                                      currentStamp->line[currentStamp->nextBuffer],
                                                      currentStamp->stamp[0]);
                    }
                    else
                    {
                        //  Try to fetch data for the next stamp in the queue.
                        fetchResult = ropCache->fetch(currentStamp->address[currentStamp->nextBuffer],
                                                      currentStamp->way[currentStamp->nextBuffer],
                                                      currentStamp->line[currentStamp->nextBuffer],
                                                      currentStamp->stamp[0], msaaSamples);
                    }
                    
                    //  Check result of the fetch operation.
                    if (fetchResult)
                    {
                        GPU_DEBUG_BOX(
                            printf("%s => Fetched stamp at %x to cache line (%d, %d) for buffer %d\n", getName(),
                                currentStamp->address[currentStamp->nextBuffer],
                                currentStamp->way[currentStamp->nextBuffer],
                                currentStamp->line[currentStamp->nextBuffer],
                                currentStamp->nextBuffer);
                        )
                        
                        //  Update the pointer to the next output buffer.
                        for(currentStamp->nextBuffer++;
                            (!activeBuffer[currentStamp->nextBuffer]) && (currentStamp->nextBuffer < MAX_RENDER_TARGETS);
                            currentStamp->nextBuffer++);
                        
                        //  Check if data fetched for all the active buffers.
                        if (currentStamp->nextBuffer == MAX_RENDER_TARGETS)
                        {
                            //  Fetch was succesful.  Add stamp to the fetched stamp queue.
                            fetchQueue.add(currentStamp);

                            //  Remove stamp from the input stamp queue.
                            inQueue.remove();

                            //  Reset the pointer to the next output buffer.
                            currentStamp->nextBuffer = 0;
                        }
                                               
                        //  Update statistics.  */
                        fetchOK->inc();
                    }
                    else
                    {
                        //  Update statistics.
                        fetchFail->inc();
                    }
                }
                else
                {
                    //  Update statistics.
                    fetchFull->inc();
                }
            }
            else
            {
                //  Read data flag is disabled so we have to allocate a line in the write buffer

                //  Check if the read stamp queue is full.
                if (!readQueue.full())
                {
                    bool allocateResult;
                    
                    //  Check if multisampling is enabled.
                    if (!multisampling)
                    {
                        //  Try to allocate a cache (buffer mode) line.
                        allocateResult = ropCache->allocate(currentStamp->address[currentStamp->nextBuffer],
                                                            currentStamp->way[currentStamp->nextBuffer],
                                                            currentStamp->line[currentStamp->nextBuffer],
                                                            currentStamp->stamp[0]);
                    }                                                            
                    else
                    {
                        //  Try to allocate a cache (buffer mode) line.
                        allocateResult = ropCache->allocate(currentStamp->address[currentStamp->nextBuffer],
                                                            currentStamp->way[currentStamp->nextBuffer],
                                                            currentStamp->line[currentStamp->nextBuffer],
                                                            currentStamp->stamp[0], msaaSamples);
                    }

                    //  Check result of the allocate operation.
                    if (allocateResult)
                    {
                        GPU_DEBUG_BOX(
                            printf("%s => Allocating cache line (%d, %d) for stamp at %x buffer %d.\n",
                                getName(), currentStamp->way[currentStamp->nextBuffer], currentStamp->line[currentStamp->nextBuffer],
                                currentStamp->address[currentStamp->nextBuffer], currentStamp->nextBuffer);
                        )

                        //  Update the pointer to the next output buffer.
                        for(currentStamp->nextBuffer++;
                            (!activeBuffer[currentStamp->nextBuffer]) && (currentStamp->nextBuffer < MAX_RENDER_TARGETS);
                            currentStamp->nextBuffer++);

                        //  Check if data fetched for all the active buffers.
                        if (currentStamp->nextBuffer == MAX_RENDER_TARGETS)
                        {
                            // Add current stamp to the read stamp queue.
                            readQueue.add(currentStamp);

                            //  Remove current stamp from the input stamp queue.
                            inQueue.remove();
                            
                            //  Reset the pointer to the next output buffer.
                            currentStamp->nextBuffer = 0;
                        }

                        //  Update statistics.
                        allocateOK->inc();
                    }
                    else
                    {
                        //  Update statistics.
                        allocateFail->inc();
                    }
                }
                else
                {
                    //  Update statistics.
                    readFull->inc();
                }
            }
        }
        else
        {
            //  Bypass flag is enabled so the stamp is bypassed to the final stage
            //  of the pipeline.

            //  Check if the written stamp queue is full.
            if (!writeQueue.full())
            {
                GPU_DEBUG_BOX(
                    printf("%s => Bypassing stamp to the next stage.\n", getName());
                )

                //  Add stamp to the written stamp queue.
                writeQueue.add(currentStamp);

                //  Remove stamp from the input stamp queue.
                inQueue.remove();
            }
            else
            {
                //  Update statistics.
                writeFull->inc();
            }
        }
    }
    else
    {
        //  Update statistics.
        inputEmpty->inc();
    }
}

//  Try to read data for the stamp in the head of the fetched stamp queue.
void GenericROP::readStamp()
{
    bool waitWrite;
    ROPQueue *currentStamp;

    //  Check if there are stamps waiting to be read.
    if (!fetchQueue.empty())
    {
        //  Get current fetched stamp.
        currentStamp = fetchQueue.head();

        // Reset wait flag.
        waitWrite = false;

        //  Check if a stamp to the same address is present in the ROP pipeline ahead of the
        //  the current stamp.

        //  Search the RAW dependece CAM for the current stamp address.
        for(int i = firstCAM, n = stampsCAM; n > 0; n--, i = GPU_MOD(i + 1, sizeCAM))
        {
            //  Check stamp address against CAM stamp address
            if ((currentStamp->address[currentStamp->nextBuffer] == rawCAM[i]->address[currentStamp->nextBuffer]) &&
                 (currentStamp != rawCAM[i]))
            {
                GPU_DEBUG_BOX(
                    printf("%s => Reading a stamp at %x before writing it.\n", getName(),
                        currentStamp->address[currentStamp->nextBuffer]);
                )

                //  Stall the pipeline until the stamp in the CAM writes the data to.
                waitWrite = TRUE;
            }
        }

        //  Check if the pipeline must stall because a RAW dependence.
        if (!waitWrite)
        {
            //  Check if there are free entries in the read stamp queue.
            if (!readQueue.full())
            {
                //  Check if multisampling is enabled.
                if (!multisampling)
                {
                    //  Try to read data for the current fetched stamp.
                    if (ropCache->read(currentStamp->address[currentStamp->nextBuffer],
                                       currentStamp->way[currentStamp->nextBuffer],
                                       currentStamp->line[currentStamp->nextBuffer],
                                       STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer],
                                       &currentStamp->data[STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer] * currentStamp->nextBuffer]))
                    {
                        GPU_DEBUG_BOX(
                            printf("%s => Reading stamp at %x for buffer %d\n", getName(),
                                currentStamp->address[currentStamp->nextBuffer], currentStamp->nextBuffer);
                        )


                        //  Check if the stamp must be added to the RAW CAM.
                        if (currentStamp->nextBuffer == 0)
                        {
                            //  Check that the RAW CAM vector is not full.
                            GPU_ASSERT(
                                if (stampsCAM == sizeCAM)
                                    panic(getName(), "readStamp", "RAW dependence CAM is full.");
                            )

                            //  Add stamp to the RAW CAM.
                            rawCAM[freeCAM] = currentStamp;
                            stampsCAM++;
                            freeCAM = GPU_MOD(freeCAM + 1, sizeCAM);
                        }

                        //  Update the pointer to the next output buffer.
                        for(currentStamp->nextBuffer++;
                            (!activeBuffer[currentStamp->nextBuffer]) && (currentStamp->nextBuffer < MAX_RENDER_TARGETS);
                            currentStamp->nextBuffer++);

                        //  Check if data fetched for all the active buffers.
                        if (currentStamp->nextBuffer == MAX_RENDER_TARGETS)
                        {
                            //  Add stamp to the read stamp queue.
                            readQueue.add(currentStamp);

                            //  Remove stamp from the fetch queue.
                            fetchQueue.remove();
                            //  Reset the pointer to the next output buffer.
                            currentStamp->nextBuffer = 0;
                        }
                        
                        //  Update statistics.
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
                    //  Compute the data offset for the current read operation.
                    u32bit dataOffset = currentStamp->nextSample * bytesPixel[currentStamp->nextBuffer] * STAMP_FRAGMENTS +
                                        currentStamp->nextBuffer * msaaSamples * bytesPixel[currentStamp->nextBuffer] * STAMP_FRAGMENTS;
                    
                    //  Compute the address for the current read operation.
                    u32bit readAddress = currentStamp->address[currentStamp->nextBuffer] +
                                         currentStamp->nextSample * bytesPixel[currentStamp->nextBuffer] * STAMP_FRAGMENTS;
                    
                    //  Try to read data for the current fetched stamp.
                    if (ropCache->read(readAddress,
                                       currentStamp->way[currentStamp->nextBuffer],
                                       currentStamp->line[currentStamp->nextBuffer],
                                       STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer],
                                       &currentStamp->data[dataOffset]))
                    {
                        GPU_DEBUG_BOX(
                            printf("%s => Reading sample %d for stamp at %x for buffer %d\n", getName(), currentStamp->nextSample,
                                currentStamp->address[currentStamp->nextBuffer], currentStamp->nextBuffer);
                        )

                        //  Check if the stamp must be added to the RAW CAM.
                        if (currentStamp->nextSample == 0)
                        {
                            //  Check that the RAW CAM vector is not full.
                            GPU_ASSERT(
                                if (stampsCAM == sizeCAM)
                                    panic(getName(), "readStamp", "RAW dependence CAM is full.");
                            )

                            //  Add stamp to the RAW CAM.
                            rawCAM[freeCAM] = currentStamp;
                            stampsCAM++;
                            freeCAM = GPU_MOD(freeCAM + 1, sizeCAM);
                        }

                        //  Update samples read for the stamp.
                        currentStamp->nextSample++;
                        
                        //  Check if all the sample data was read.
                        if (currentStamp->nextSample == msaaSamples)
                        {
                            //  Reset multisampling counter.
                            currentStamp->nextSample = 0;
                            
                            //  Update the pointer to the next output buffer.
                            for(currentStamp->nextBuffer++;
                                (!activeBuffer[currentStamp->nextBuffer]) && (currentStamp->nextBuffer < MAX_RENDER_TARGETS);
                                currentStamp->nextBuffer++);

                            //  Check if data fetched for all the active buffers.
                            if (currentStamp->nextBuffer == MAX_RENDER_TARGETS)
                            {
                                //  Add stamp to the read stamp queue.
                                readQueue.add(currentStamp);

                                //  Remove stamp from the fetch queue.
                                fetchQueue.remove();
                                
                                //  Reset the pointer to the next output buffer.
                                currentStamp->nextBuffer = 0;
                            }
                        }
                        
                        
                        /*  Update statistics.  */
                        readOK->inc();
                    }
                    else
                    {
                        /*  Update statistics.  */
                        readFail->inc();
                    }
                }
            }
            else
            {
                //  Update statistics.
                readFull->inc();
            }
        }
        else
        {
            //  Update statistics.
            rawDep->inc();
        }
    }
    else
    {
        //  Update statistics.
        fetchEmpty->inc();
    }
}

//  Start ROP operation for the stamp in the head of the read queue.
void GenericROP::startOperation(u64bit cycle)
{
    ROPQueue *currentStamp;
    ROPOperation *ropOperation;

    //  Update cycles until the next stamp can be operated
    if (ropCycles > 0)
    {
        ropCycles--;
    }

    //  Check if a new stamp can start to operate
    if (ropCycles == 0)
    {
        //  Check if there stamps in the read stamp queue.
        if (!readQueue.empty())
        {
            //  Check if the operation stamp queue has free entries.
            if (opQueue.items() <= (opQueueSize - ropLatency))
            {
                //  Get next stamp from the read stamp queue.
                currentStamp = readQueue.head();

                //  Send stamp through the ROP operation latency signal.

                //  Create ROP operation object.
                ropOperation = new ROPOperation(currentStamp);

                //  Copy parent command cookies.
                ropOperation->copyParentCookies(*currentStamp->stamp[0]);

                //  Remove the fragment level cookie.
                ropOperation->removeCookie();

                //  Start test operation.  */
                operationStart->write(cycle, ropOperation);

                //  Check if multisampling is enabled.
                if (!multisampling)
                {
                    //  Set start test cycle counter.
                    ropCycles = ropRate * numActiveBuffers;
                }
                else
                {
                    //  Set start test cycle counter.
                    ropCycles = ropRate * msaaSamples * numActiveBuffers;
                }
                
                //  Remove current stamp from the head of the read stamp queue.
                readQueue.remove();
            }
            else
            {
                //  Update statistics.
                opFull->inc();
            }

        }
        else
        {
            //  Update statistics.
            readEmpty->inc();
        }
    }
    else
    {
        //  Update statistics.
        ropOpBusy->inc();
    }
}

//  End ROP operation for a stamp
void GenericROP::endOperation(u64bit cycle)
{
    ROPOperation *ropOperation;
    ROPQueue *currentStamp;

    //  Check end of test operation.
    if (operationEnd->read(cycle, (DynamicObject *&) ropOperation))
    {
        //  Get processed stamp.
        currentStamp = ropOperation->getROPStamp();

        //  Delete ROP operation object.
        delete ropOperation;

        GPU_DEBUG_BOX(
            printf("%s => Finished ROP operation on stamp.\n", getName());
        )

        //  Call the operate stamp function for the derived classes.
        operateStamp(cycle, currentStamp);

        GPU_ASSERT(
            if (opQueue.full())
                panic(getName(), "endOperation", "Operated stamp queue full.");
        )

        //  Add stamp to the operated stamp queue.
        opQueue.add(currentStamp);

        //  Update statistics.
        operated->inc();
    }
}

//  Write stamp data into the buffer.
void GenericROP::writeStamp()
{
    ROPQueue *currentStamp;

    //  Check if the operated stamp queue is empty.
    if (!opQueue.empty())
    {
        //  Check if the written stamp queue has free entries.
        if (!writeQueue.full())
        {
            //  Get current stamp from the head of the operated stamp queue.
            currentStamp = opQueue.head();

            //  Check if multisampling is enabled.
            if (!multisampling)
            {
                //  Try to write the stamp data.
                if (ropCache->write(currentStamp->address[currentStamp->nextBuffer],
                                    currentStamp->way[currentStamp->nextBuffer],
                                    currentStamp->line[currentStamp->nextBuffer],
                                    STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer],
                                    &currentStamp->data[currentStamp->nextBuffer * STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer]],
                                    &currentStamp->mask[currentStamp->nextBuffer * STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer]]))
                {
                    GPU_DEBUG_BOX(
                        printf("%s => Writing stamp at %x for buffer %d\n", getName(),
                            currentStamp->address[currentStamp->nextBuffer], currentStamp->nextBuffer);
                        printf("%s => Written data :", getName());
                        for(u32bit b = 0; b < STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer]; b++)
                            printf(" %02x", currentStamp->data[currentStamp->nextBuffer * STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer] + b]);
                        printf("\n");
                        printf("%s => Write mask :", getName());
                        for(u32bit b = 0; b < STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer]; b++)
                            printf(" %d", currentStamp->mask[currentStamp->nextBuffer * STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer] + b]);
                        printf("\n");                            
                    )

                    //  Update the pointer to the next output buffer.
                    for(currentStamp->nextBuffer++;
                        (!activeBuffer[currentStamp->nextBuffer]) && (currentStamp->nextBuffer < MAX_RENDER_TARGETS);
                        currentStamp->nextBuffer++);

                    //  Check if data fetched for all the active buffers.
                    if (currentStamp->nextBuffer == MAX_RENDER_TARGETS)
                    {
                        //  Add stamp to the written stamp queue.
                        writeQueue.add(currentStamp);

                        //  Remove stamp from the operated stamp queue.
                        opQueue.remove();

                        //  Check if ROP data read is enabled and RAW CAM is in use.
                        if (readDataROP[0])
                        {
                            GPU_ASSERT(
                                if (stampsCAM == 0)
                                    panic(getName(), "writeStamp", "RAW CAM vector empty.");
                                if (rawCAM[firstCAM] == NULL)
                                    panic(getName(), "writeStamp", "Unitialized RAW CAM entry accessed.");
                            )

                            //  Remove stamp from the RAW CAM vector.
                            rawCAM[firstCAM] = NULL;
                            firstCAM = GPU_MOD(firstCAM + 1, sizeCAM);
                            stampsCAM--;
                        }
                        
                        //  Reset the pointer to the next output buffer.
                        currentStamp->nextBuffer = 0;
                    }
                    
                    //  Update statistics.
                    writeOK->inc();
                }
                else
                {
                    //  Update statistics.
                    writeFail->inc();
                }
            }
            else
            {
                //  Compute the address for the write operation.
                u32bit writeAddress = currentStamp->address[currentStamp->nextBuffer] +
                                      currentStamp->nextSample * STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer];
                
                //  Compute the data offset for the write operation;
                u32bit dataOffset = currentStamp->nextSample * STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer] +
                                    msaaSamples * STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer] * currentStamp->nextBuffer;
                
                //  Try to write the stamp data.
                if (ropCache->write(writeAddress,
                                    currentStamp->way[currentStamp->nextBuffer],
                                    currentStamp->line[currentStamp->nextBuffer],
                                    STAMP_FRAGMENTS * bytesPixel[currentStamp->nextBuffer],
                                    &currentStamp->data[dataOffset],
                                    &currentStamp->mask[dataOffset]))
                {
                    GPU_DEBUG_BOX(
                        printf("%s => Writing sample %d for stamp at %x for buffer %d\n", getName(),
                            currentStamp->nextSample, currentStamp->address[currentStamp->nextBuffer], currentStamp->nextBuffer);
                    )

                    //  Update samples written for the stamp.
                    currentStamp->nextSample++;
                    
                    //  Check if all the sample data for the stamp was written.
                    if (currentStamp->nextSample == msaaSamples)
                    {
                        //  Reset multisampling counter.
                        currentStamp->nextSample = 0;
                    
                        //  Update the pointer to the next output buffer.
                        for(currentStamp->nextBuffer++;
                            (!activeBuffer[currentStamp->nextBuffer]) && (currentStamp->nextBuffer < MAX_RENDER_TARGETS);
                            currentStamp->nextBuffer++);

                        //  Check if data fetched for all the active buffers.
                        if (currentStamp->nextBuffer == MAX_RENDER_TARGETS)
                        {
                            //  Add stamp to the written stamp queue.
                            writeQueue.add(currentStamp);

                            //  Remove stamp from the operated stamp queue.
                            opQueue.remove();

                            //  Check if ROP data read is enabled and RAW CAM is in use.
                            if (readDataROP[0])
                            {
                                GPU_ASSERT(
                                    if (stampsCAM == 0)
                                        panic(getName(), "writeStamp", "RAW CAM vector empty.");
                                    if (rawCAM[firstCAM] == NULL)
                                        panic(getName(), "writeStamp", "Unitialized RAW CAM entry accessed.");
                                )

                                //  Remove stamp from the RAW CAM vector.
                                rawCAM[firstCAM] = NULL;
                                firstCAM = GPU_MOD(firstCAM + 1, sizeCAM);
                                stampsCAM--;
                            }
                            
                            //  Update pointer to the next output buffer.
                            currentStamp->nextBuffer = 0;
                        }
                    }
                    
                    /*  Update statistics.  */
                    writeOK->inc();
                }
                else
                {
                    //  Update statistics.
                    writeFail->inc();
                }
            }
        }
        else
        {
            //  Update statistics.
            writeFull->inc();
        }
    }
    else
    {
        //  Update statistics.
        opEmpty->inc();
    }
}

//  Terminate the processing the stamp and remove it from the pipeline.
void GenericROP::terminateStamp(u64bit cycle)
{
    ROPQueue *currentStamp;
    bool processed;

    //  Check if the written stamp queue has stamps to process.
    if (!writeQueue.empty())
    {
        //  Check if the consumer stage is ready.
        if (consumerState == ROP_READY)
        {
            //  Get current stamp from the head of the written stamp queue.
            currentStamp = writeQueue.head();

            //  Call the specific function for post write processing from the derived class.
            postWriteProcess(cycle, currentStamp);

            //  Remove the stamp from the written stamp queue.
            writeQueue.remove();

            //  Add stamp to the free queue.
            freeQueue.add(currentStamp);
        }
        else
        {
            //  Update statistics.
            consumerBusy->inc();
        }
    }
    else
    {
        //  Update statistics.
        writeEmpty->inc();
    }
}

//  Processes a memory transaction.
void GenericROP::processMemoryTransaction(u64bit cycle,
    MemoryTransaction *memTrans)
{
    //  Get transaction type.
    switch(memTrans->getCommand())
    {
        case MT_STATE:

            //  Received state of the Memory controller.
            memoryState = memTrans->getState();

            GPU_DEBUG_BOX(
                printf("%s => Memory Controller State = %d\n", getName(), memoryState);
            )

            break;

        default:

            GPU_DEBUG_BOX(
                printf("%s => Memory Transaction to the Cache.\n", getName());
            )

            //  Update statistics.
            readBytes->inc(memTrans->getSize());

            //  Let the rop cache process any other transaction.
            ropCache->processMemoryTransaction(memTrans);

            break;
    }

    //  Delete processed memory transaction.
    delete memTrans;
}

void GenericROP::getState(string &stateString)
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

    /*stateStream << " | Fragment Counter = " << fragmentCounter;
    stateStream << " | Triangle Counter = " << triangleCounter;
    stateStream << " | Last Fragment = " << lastFragment;
    stateStream << " | Fetch Stamps = " << fetchStamps;
    stateStream << " | Read Stamps = " << readStamps;
    stateStream << " | Test Stamps = " << testStamps;
    stateStream << " | Write Stamps = " << writeStamps;
    stateStream << " | Color Stamps = " << colorStamps;*/

    stateString.assign(stateStream.str());
}


void GenericROP::stallReport(u64bit cycle, string &stallReport)
{
    stringstream reportStream;
    
    reportStream << " " << getName() << " stall report for cycle " << cycle << endl;
    reportStream << "------------------------------------------------------------" << endl;


    reportStream << "InputQueue = " << inQueue.items() << " | FetchQueue = " << fetchQueue.items();
    reportStream << " | ReadQueue = " << readQueue.items() << " | OpQueue = " << opQueue.items() << " | writeQueue = " << writeQueue.items();
    reportStream << endl << endl;
    
    string report;
    
    ropCache->stallReport(cycle, report);
    
    reportStream << report;
    
    stallReport.assign(reportStream.str());
}


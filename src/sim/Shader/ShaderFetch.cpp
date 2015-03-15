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
 * $RCSfile: ShaderFetch.cpp,v $
 * $Revision: 1.32 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:53 $
 *
 * Shader Unit Fetch Box (unified shader version).
 *
 */

/**
 *
 *  @file ShaderFetch.cpp
 *
 *  This file implements the ShaderFetch Box Class (unified shader version).
 *  This box implements the simulation of the Fetch stage
 *  of a GPU shader unit.
 *
 */

#include "ShaderFetch.h"
#include "ShaderStateInfo.h"
#include "ConsumerStateInfo.h"
#include "ShaderDecodeStateInfo.h"
#include "Streamer.h"
#include "ShaderDecodeExecute.h"
#include "ShaderInstruction.h"
#include <cmath>
#include <cstdio>
#include <sstream>

using namespace gpu3d;


/*  Constructor for the ShaderFetch BOX class.  */

ShaderFetch::ShaderFetch(ShaderEmulator &shEmu, bool unified, u32bit nThreads,
    u32bit nInputBuffers, u32bit resources, u32bit threadsPerCycle, u32bit instrPerCycle, u32bit threadsPerGroup,
    bool lockStepMode, bool scalar, bool thWindow, u32bit fDelay, bool swpOnBlck, u32bit texUnits, u32bit inCycle,
    u32bit outCycle, u32bit outputLatency, bool microTrisAsFrag, char *name, char *shPrefix, char *sh2Prefix, 
    Box* parent):

    /*  Pre-initializations.  */
    Box(name, parent), shEmul(shEmu), unifiedModel(unified), numBuffers(nThreads + nInputBuffers),
    numThreads(nThreads), numInputBuffers(nInputBuffers), numResources(resources),
    threadsCycle(threadsPerCycle), threadGroup(threadsPerGroup), instrCycle(instrPerCycle),
    lockStep(lockStepMode), scalarALU(scalar), threadWindow(thWindow), fetchDelay(fDelay), swapOnBlock(swpOnBlck),
    textureUnits(texUnits), inputCycle(inCycle), outputCycle(outCycle), maxOutLatency(outputLatency),
    currentThread(0), nextThread(0), nextThreadInGroup(0), readyThreadGroup(true), numFreeThreads(nThreads + nInputBuffers),
    transInProgress(FALSE), numFinishedThreads(0), firstFilledBuffer(0), firstEmptyBuffer(0),
    lastEmptyBuffer(nThreads + nInputBuffers -1), firstFinThread(0), lastFinThread(0),
    freeResources(resources), numReadyThreads(0)

{
    u32bit i;
    u32bit j;
    DynamicObject *defValue[1];
    char fullName[64];
    char postfix[32];

    /*  Check parameters.  */
    GPU_ASSERT(
        if (threadGroup == 0)
            panic("ShaderFetch", "ShaderFetch", "At least a shader thread required per thread group.");
        if (numThreads < threadGroup)
            panic("ShaderFetch", "ShaderFetch", "At least a group of shader threads required.");
        if (numInputBuffers < threadGroup)
            panic("ShaderFetch", "ShaderFetch", "At least a group of threads/input buffers required.");
        if (numResources < numThreads)
            panic("ShaderFetch", "ShaderFetch", "At least one resource per shader thread required.");
        if (GPU_MOD(numBuffers, threadGroup) != 0)
            panic("ShaderFetch", "ShaderFetch", "The number of thread/buffers must be a multiple of the thread group.");
        if (threadsCycle == 0)
            panic("ShaderFetch", "ShaderFetch", "At least one shader thread must be fetched per cycle.");
        if (instrCycle == 0)
            panic("ShaderFetch", "ShaderFetch", "At least one instruction from a shader thread must be fetched per cycle.");
        if (inputCycle == 0)
            panic("ShaderFetch", "ShaderFetch", "At least one shader input must be received per cycle.");
        if (outputCycle == 0)
            panic("ShaderFetch", "ShaderFetch", "At least one shader output must be sent per cycle.");
        if (maxOutLatency == 0)
            panic("ShaderFetch", "ShaderFetch", "Output signal latency must be at least 1.");
        if (shPrefix == NULL)
            panic("ShaderFetch", "ShaderFetch", "Shader prefix string required.");
        if (unifiedModel && (shPrefix == NULL))
            panic("ShaderFetch", "ShaderFetch", "Secondary shader prefix string required for unified shader model.");
        //if (lockStep && ((threadGroup > outputCycle) && (GPU_MOD(threadGroup, outputCycle) != 0)))
        //    panic("ShaderFetch", "ShaderFetch", "Outputs per cycle must be a divisor of the thread groups in lock step mode when the outputs per cycle is smaller than the thread group.");
        //if (lockStep && (threadGroup < outputCycle))
        //    panic("ShaderFetch", "ShaderFetch", "Outputs per cycle must be as large or smaller than a thread group.");
        //if (lockStep && ((threadGroup > threadsCycle) && (GPU_MOD(threadGroup, threadsCycle) != 0)))
        //    panic("ShaderFetch", "ShaderFetch", "Thread rate must be a divisor of thread groups in lock step mode when the thread rate is smaller than the thread group.");
        //if (lockStep && ((threadGroup < threadsCycle) && (GPU_MOD(threadsCycle, threadGroup) != 0)))
        //    panic("ShaderFetch", "ShaderFetch", "Thread rate must be multiple of thread groups in lock step mode when the thread rate is greater than the thread group.");
        if (scalarALU && (instrCycle != 2))
            panic("ShaderFetch", "ShaderFetch", "SIMD + scalar fetch mode requires a fetch rate of two instructions per cycle.");
        if (!threadWindow && (fetchDelay == 0))
            printf("ShaderFetch => WARNING!! to avoid desynchronization in small batches due to dependences you should set a large enough delay between fetch iterations.\n");
    )

    /*  Create the full name and postfix for the statistics.  */
    sprintf(fullName, "%s::%s", shPrefix, name);
    sprintf(postfix, "ShF-%s", shPrefix);

//printf("ShFetch %s -> %p \n", fullName, this);

    /*  Create statistics.  */
    fetched = &getSM().getNumericStatistic("FetchedInstr", u32bit(0), fullName, postfix);
    reFetched = &getSM().getNumericStatistic("ReFetchedInstr", u32bit(0), fullName, postfix);
    inputs = &getSM().getNumericStatistic("Inputs", u32bit(0), fullName, postfix);
    outputs = &getSM().getNumericStatistic("Outputs", u32bit(0), fullName, postfix);
    inputInAttribs = &getSM().getNumericStatistic("InputActiveInputAttributes", u32bit(0), fullName, postfix);
    inputOutAttribs = &getSM().getNumericStatistic("InputActiveOutputAttributes", u32bit(0), fullName, postfix);
    inputRegisters = &getSM().getNumericStatistic("InputRegisters", u32bit(0), fullName, postfix);
    blocks = &getSM().getNumericStatistic("Blocks", u32bit(0), fullName, postfix);
    unblocks = &getSM().getNumericStatistic("Unblocks", u32bit(0), fullName, postfix);
    nThReady = &getSM().getNumericStatistic("ReadyThreads", u32bit(0), fullName, postfix);
    nThReadyAvg = &getSM().getNumericStatistic("ReadyThreadsAvg", u32bit(0), fullName, postfix);
    nThReadyMax = &getSM().getNumericStatistic("ReadyThreadsMax", u32bit(0), fullName, postfix);
    nThReadyMin = &getSM().getNumericStatistic("ReadyThreadsMin", u32bit(0), fullName, postfix);
    nThBlocked = &getSM().getNumericStatistic("BlockedThreads", u32bit(0), fullName, postfix);
    nThFinished = &getSM().getNumericStatistic("FinishedThreads", u32bit(0), fullName, postfix);
    nThFree = &getSM().getNumericStatistic("FreeThreads", u32bit(0), fullName, postfix);
    nResources = &getSM().getNumericStatistic("UsedResources", u32bit(0), fullName, postfix);
    emptyCycles = &getSM().getNumericStatistic("EmptyCycles", u32bit(0), fullName, postfix);
    fetchCycles = &getSM().getNumericStatistic("FetchCycles", u32bit(0), fullName, postfix);
    noReadyCycles = &getSM().getNumericStatistic("NoReadyCycles", u32bit(0), fullName, postfix);

    /*  Create Box Signals.  */

    /*  Check if unified model is enabled.  */
    if (unifiedModel)
    {
        /*  Signal from the Command Processor.  Commands to the Shader.  */
        commProcSignal[FRAGMENT_PARTITION] = newInputSignal("CommShaderCommand", 1, 1, shPrefix);
        commProcSignal[VERTEX_PARTITION] = newInputSignal("CommShaderCommand", 1, 1, sh2Prefix);
    }
    else
    {
        /*  Signal from the Command Processor.  Commands to the Shader.  */
        commProcSignal[PRIMARY_PARTITION] = newInputSignal("CommShaderCommand", 1, 1, shPrefix);
    }

    /*  Signal from the Consumer unit.  Inputs and commands to the Shader.  */
    commandSignal = newInputSignal("ShaderCommand", inputCycle, 1, shPrefix);

    /*  Signal from the Decoder/Execute Box.  New PC and END signal.  */
    //newPCSignal = newInputSignal("ShaderControl", (2 + textureUnits) * GPU_MAX(threadGroup, threadsCycle) * instrCycle, 1, shPrefix);
    newPCSignal = newInputSignal("ShaderControl",  textureUnits * STAMP_FRAGMENTS + ((MAX_EXEC_BW + 1) * threadsCycle * instrCycle), 1, shPrefix);

    /*  Signal from the Decoder/Execute Box.  Decoder state.  */
    decodeStateSignal = newInputSignal("ShaderDecodeState", 1, 1, shPrefix);

    /*  Signal to the Streamer.  Shader state.  */
    readySignal = newOutputSignal("ShaderState", 1, 1, shPrefix);

    /*  Build initial signal state.  */
    defValue[0] = new ShaderStateInfo(SH_READY);

    /*  Set default state of ShaderReady signal.  */
    readySignal->setData(defValue);

    /*  Signal to the Decoder/Execute Box.  */
    instructionSignal = newOutputSignal("ShaderInstruction", threadsCycle * instrCycle, 1, shPrefix);

    /*  Signal from the Shader to a consumer.  Shader output.  */
    outputSignal = newOutputSignal("ShaderOutput", outputCycle, maxOutLatency, shPrefix);

    /*  Signal from the consumer to the Shader.  State of the consumer.  */
    consumerSignal = newInputSignal("ConsumerState", 1, 1, shPrefix);

    /*  Create and initialize the thread table.  */
    threadTable = new ThreadTable[numBuffers];

    /*  Active Threads are ready and free at initialization.
        Non Active Threads (buffers) are never ready and are set
        free at initialization. */

    /*for (i = 0; i < numThreads; i++)
    {
        threadTable[i].ready = TRUE;
        threadTable[i].free = TRUE;
        threadTable[i].end = FALSE;
        threadTable[i].PC = 0;
        threadTable[i].instructionCount = 0;
    }*/

    for(i = 0; i < numBuffers; i++)
    {
        threadTable[i].ready = false;
        threadTable[i].blocked = false;
        threadTable[i].free = true;
        threadTable[i].end = false;
        threadTable[i].repeat = false;
        threadTable[i].zexported = false;
        threadTable[i].PC = 0;
        threadTable[i].instructionCount = 0;
        threadTable[i].nextFetchCycle = 0;
    }

    /*  Allocate free thread/buffer list.  */
    freeBufferList = new u32bit[numBuffers];

    /*  Initializate free buffer list.  */
    for(i = 0; i < numBuffers; i++)
        freeBufferList[i] = i;

    /*  Allocate finished thread table.  */
    finThreadList = new u32bit[numThreads];

    /*  Set initial shader state.  */
    shState = SH_EMPTY;

    /*  Reset shader input register tables for all the targets.  */
    for(i = 0; i < 3; i++)
    {
        /*  Reset the inputs as not active for the shader target.  */
        for(j = 0; j < MAX_VERTEX_ATTRIBUTES; j++)
            input[i][j] = FALSE;

        /*  Reset the number of active inputs for the shader target.  */
        activeInputs[i] = 0;
    }

    //  Reset the number of shader outputs for each shader target.
    for(u32bit target = 0; target < 3; target++)
    {
        //  Reset shader output register table.
        for(u32bit o = 0; o < MAX_VERTEX_ATTRIBUTES; o++)        
            output[target][o] = false;

        //  Reset active shader output register counter.
        activeOutputs[target] = 0;
    }
    
    //  Fragment target and triangle target have outputs enabled by default.
    output[FRAGMENT_PARTITION][0] = true;
    activeOutputs[FRAGMENT_PARTITION] = 1;
    output[TRIANGLE_PARTITION][0] =
    output[TRIANGLE_PARTITION][1] =
    output[TRIANGLE_PARTITION][2] =
    output[TRIANGLE_PARTITION][3] = true;
    activeOutputs[TRIANGLE_PARTITION] = 4;

    /*  Check model.  */
    if (unifiedModel)
    {
        /*  Reset default start PCs.  */
        initPC[VERTEX_TARGET] = 0;
        initPC[FRAGMENT_TARGET] = 0;
        initPC[TRIANGLE_TARGET] = TRIANGLE_SETUP_PROGRAM_PC;

        /*  Reset default per thread resource usage.  */
        threadResources[VERTEX_TARGET] = 1;
        threadResources[FRAGMENT_TARGET] = 1;
        threadResources[TRIANGLE_TARGET] = 2;

        /*  Set fixed input attribute number for triangle shader target.  */
        activeInputs[TRIANGLE_PARTITION] = 3;
    }
    else
    {
        /*  Reset default start PC.  */
        initPC[PRIMARY_PARTITION] = 0;

        /*  Reset default per thread resource usage.  */
        threadResources[PRIMARY_PARTITION] = 1;
    }

    /*  Reset default maximum resource usage.  */
    maxThreadResources = 3;

    /*  Allocate buffer for the instruction fetched for a thread group.  */
    fetchGroup = new ShaderExecInstruction**[threadGroup];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (fetchGroup == NULL)
            panic("ShaderFetch", "ShaderFetch", "Error allocating thread group fetch buffer.");
    )

    for(i = 0; i < threadGroup; i++)
    {
        /*  Allocate buffer for each of the threads in the group.  */
        fetchGroup[i] = new ShaderExecInstruction*[instrCycle];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (fetchGroup[i] == NULL)
                panic("ShaderFetch", "ShaderFetch", "Error allocating thread fetch buffer.");
        )
    }

    /*  Calculate the number of thread groups in the shader.  */
    numGroups = numBuffers / threadGroup;

    /*  Allocate ready threads per group counter list.  */
    readyThreads = new u32bit[numGroups];

    //  Allocate finished threads per group counter list.
    finThreads = new u32bit[numGroups];
    
    /*  Allocate queue for the ready thread groups.  */
    readyGroups = new u32bit[2 * numGroups];

    /*  Allocate flag array storing if a group is blocked outside the group queues.  */
    groupBlocked = new bool[numGroups];

    /*  Allocate queue for the fetched thread groups.  */
    fetchedGroups = new u32bit[numGroups];

    /*  Allocate the shader input batch queues.  */
    batchGroupQueue[VERTEX_PARTITION][0] = new u32bit[numGroups];
    batchGroupQueue[VERTEX_PARTITION][1] = new u32bit[numGroups];
    batchGroupQueue[VERTEX_PARTITION][2] = new u32bit[numGroups];
    batchGroupQueue[VERTEX_PARTITION][3] = new u32bit[numGroups];
    batchGroupQueue[FRAGMENT_PARTITION][0] = new u32bit[numGroups];
    batchGroupQueue[FRAGMENT_PARTITION][1] = new u32bit[numGroups];
    batchGroupQueue[FRAGMENT_PARTITION][2] = new u32bit[numGroups];
    batchGroupQueue[FRAGMENT_PARTITION][3] = new u32bit[numGroups];
    batchGroupQueue[TRIANGLE_PARTITION][0] = new u32bit[numGroups];
    batchGroupQueue[TRIANGLE_PARTITION][1] = new u32bit[numGroups];
    batchGroupQueue[TRIANGLE_PARTITION][2] = new u32bit[numGroups];
    batchGroupQueue[TRIANGLE_PARTITION][3] = new u32bit[numGroups];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (readyThreads == NULL)
            panic("ShaderFetch", "ShaderFetch", "Error allocating the per group ready thread counter list.");
        if (finThreads == NULL)
            panic("ShaderFetch", "ShaderFetch", "Error allocating the per group finished thread counter list.");
        if (readyGroups == NULL)
            panic("ShaderFetch", "ShaderFetch", "Error allocating the ready thread group queue.");
        if (groupBlocked == NULL)
            panic("ShaderFetch", "ShaderFetch", "Error allocating blocked group flag array.");
        if (fetchedGroups == NULL)
            panic("ShaderFetch", "ShaderFetch", "Error allocating the fetched thread group queue.");
        if ((batchGroupQueue[VERTEX_PARTITION][0] == NULL) || (batchGroupQueue[VERTEX_PARTITION][1] == NULL)
            || (batchGroupQueue[VERTEX_PARTITION][2] == NULL) || (batchGroupQueue[VERTEX_PARTITION][3] == NULL)
            || (batchGroupQueue[TRIANGLE_PARTITION][0] == NULL) || (batchGroupQueue[TRIANGLE_PARTITION][1] == NULL)
            || (batchGroupQueue[TRIANGLE_PARTITION][2] == NULL) || (batchGroupQueue[TRIANGLE_PARTITION][3] == NULL)
            || (batchGroupQueue[FRAGMENT_PARTITION][0] == NULL) || (batchGroupQueue[FRAGMENT_PARTITION][1] == NULL)
            || (batchGroupQueue[FRAGMENT_PARTITION][2] == NULL) || (batchGroupQueue[FRAGMENT_PARTITION][3] == NULL))
            panic("ShaderFetch", "ShaderFetch", "Error allocating the batch group queues.");
    )

    /*  Reset ready group queue counters and pointers.  */
    nextReadyGroup = nextFreeReady = 0;
    if (threadWindow)
        numReadyGroups = 0;
    else
        numReadyGroups = numGroups;

    readyThreadGroup = false;

    /*  Reset fetched group queue counters and pointers.  */
    numFetchedGroups = nextFetchedGroup = nextFreeFetched = 0;

    /*  Reset ready threads counters.  */
    for(i = 0; i < numGroups; i++)
    {
        readyThreads[i] = 0;

        finThreads[i] = 0;
        
        /*  Threads start blocked outside the ready group queue when the thread window is enabled.
            If the thread window is disabled they must not be added to the ready group queue (not
            used) and therefore start as 'inside'.  */
        groupBlocked[i] = threadWindow;

        /*  Reset fetched groups with a non valid thread ID (required for swapOnBlock mode).  */
        fetchedGroups[i] = numGroups;
    }

    /*  Reset pointers to the batch shader input queue.  */
    fetchBatch[VERTEX_PARTITION] = loadBatch[VERTEX_PARTITION] =
    nextFetchInBatch [VERTEX_PARTITION] = lastInBatch[VERTEX_PARTITION][0] =
    lastInBatch[VERTEX_PARTITION][1] = 0;
    closedBatch[VERTEX_PARTITION][0] = closedBatch[VERTEX_PARTITION][1] = false;
    batchEnding[VERTEX_PARTITION] = false;

    fetchBatch[FRAGMENT_PARTITION] = loadBatch[FRAGMENT_PARTITION] =
    nextFetchInBatch [FRAGMENT_PARTITION] = lastInBatch[FRAGMENT_PARTITION][0] =
    lastInBatch[FRAGMENT_PARTITION][1] = 0;
    closedBatch[FRAGMENT_PARTITION][0] = closedBatch[FRAGMENT_PARTITION][1] = false;
    batchEnding[FRAGMENT_PARTITION] = false;

    fetchBatch[TRIANGLE_PARTITION] = loadBatch[TRIANGLE_PARTITION] =
    nextFetchInBatch [TRIANGLE_PARTITION] = lastInBatch[TRIANGLE_PARTITION][0] =
    lastInBatch[TRIANGLE_PARTITION][1] = 0;
    closedBatch[TRIANGLE_PARTITION][0] = closedBatch[TRIANGLE_PARTITION][1] = false;
    batchEnding[TRIANGLE_PARTITION] = false;

    currentBatchPartition = VERTEX_PARTITION;

    /*  Reset number of pending z exports to sent of active threads.  */
    pendingZExports = 0;

    /*  Reset current z export thread.  */
    currentZExportThread = 0;

    /*  Reset MSAA registers.  */
    multisampling = false;
    msaaSamples = 4;
}

/*  Clock instruction for ShaderFetch Box.  Drives the time
 *  of the simulation.
 *
 */

void ShaderFetch::clock(u64bit cycle)
{
    ShaderCommand *command;
    ShaderDecodeCommand *decodeCommand;
    ShaderDecodeState decoderState;
    ConsumerState consumerState;
    ShaderInput *shInput;
    ShaderDecodeStateInfo *shDecStateInfo;
    ConsumerStateInfo *consumerStateInfo;
    u32bit visited;
    u32bit i;
    u32bit j;
    u32bit k;
    u32bit pc;
    u32bit outputThread;
    ShaderExecInstruction *shExecInstr;
    //u32bit nextGroup;
    bool searchReadyGroup;
    bool anyBlocked;
    u32bit fetchedThreads;
    bool lastVisitedGroupReady;

    GPU_DEBUG_BOX( printf("ShaderFetch >> Clock %lld.\n", cycle); )

/*if ((cycle > 5024880) && (GPU_MOD(cycle, 1000) == 0))
{
printf("ShF (%p) %lld => ready %d free %d fin %d blocked %d res %d | in %d out %d group %d rate %d way %d\n",
this, cycle, numReadyThreads, numFreeThreads, numFinishedThreads,
numBuffers - numReadyThreads - numFreeThreads - numFinishedThreads, freeResources,
inputCycle, outputCycle, threadGroup, threadsCycle, instrCycle);
printf("ShF => numReadyGroups %d numFetchedGroups %d numGroups %d\n", numReadyGroups,
numFetchedGroups, numGroups);
}*/

    /*  Receive inputs and commands from the consumer unit attached to the shader.  */
    while (commandSignal->read(cycle, (DynamicObject *&) shInput))
    {
        processShaderInput(cycle, shInput);
    }

    /*  Check shader model.  */
    if (unifiedModel)
    {
        /*  Receive commands from the Command Processor.  */
        if (commProcSignal[VERTEX_PARTITION]->read(cycle, (DynamicObject *&) command))
            processShaderCommand(command, VERTEX_PARTITION);

        /*  Receive commands from the Command Processor.  */
        if (commProcSignal[FRAGMENT_PARTITION]->read(cycle, (DynamicObject *&) command))
            processShaderCommand(command, FRAGMENT_PARTITION);
    }
    else
    {
        /*  Receive commands from the Command Processor.  */
        if (commProcSignal[PRIMARY_PARTITION]->read(cycle, (DynamicObject *&) command))
            processShaderCommand(command, PRIMARY_PARTITION);
    }

    /*  Receive feedback from decode stage.  */
    while (newPCSignal->read(cycle, (DynamicObject *&) decodeCommand))
        processDecodeCommand(decodeCommand);

    /*  Receive state from the decode stage.  */
    if (decodeStateSignal->read(cycle, (DynamicObject *&) shDecStateInfo))
    {
        /*  Store shader decode state.  */
        decoderState = shDecStateInfo->getState();

        /*  Delete received decoder state.  */
        delete shDecStateInfo;
    }
    else
    {
        /*  No decoder state?  Where did those electrons went?  */
        panic("ShaderFetch", "clock", "No decoder state signal.");
    }

    /*  Process fetched thread groups.  */
    while ((numFetchedGroups > 0) && (threadTable[fetchedGroups[nextFetchedGroup] * threadGroup].nextFetchCycle <= cycle))
    {
        /*  Check if the thread group is ready.  */
        if (readyThreads[fetchedGroups[nextFetchedGroup]] == threadGroup)
        {
//printf("ShF %lld => adding ready group %d at %d\n", cycle, fetchedGroups[nextFetchedGroup], nextFreeReady);

            /*  Check if ready group queue is full.  */
            GPU_ASSERT(
                if (numReadyGroups == numGroups)
                    panic("ShaderFetch", "clock", "Ready group queue full.");
            )

            /*  Add group to the ready group queue.  */
            readyGroups[nextFreeReady] = fetchedGroups[nextFetchedGroup];

            /*  Update pointer to the next free ready group entry.  */
            nextFreeReady = GPU_MOD(nextFreeReady + 1, 2 * numGroups);

            /*  Update number of ready groups.  */
            numReadyGroups++;
        }
        else
        {
            /*  Mark group as blocked and not stored in any queue.  */
            groupBlocked[fetchedGroups[nextFetchedGroup]] = true;
        }

        /*  Skip to the next fetched group.  */
        nextFetchedGroup = GPU_MOD(nextFetchedGroup + 1, numGroups);
        numFetchedGroups--;
    }

    /*  Is decoder stage ready?  */
    if (decoderState == SHDEC_READY)
    {
        /*  Determine the execution mode active.  */
        if (lockStep)
        {
            /*  Search for a ready group.  */
            searchReadyGroup = (numReadyThreads > 0);

            /*  Fetch N instructions from groups of M threads.  */
            for(fetchedThreads = 0; (fetchedThreads < threadsCycle) && ((nextThreadInGroup != 0) || ((numReadyGroups > 0) && searchReadyGroup));)
            {
                /*  Check if a whole group of threads was fetched.  */
                if (nextThreadInGroup == 0)
                {
                    /*  Check if thread window is enabled.  */
                    if (threadWindow)
                    {
                        /*  Keep if the last visited group was ready.  */
                        lastVisitedGroupReady = readyThreadGroup;

                        /*  No ready thread group found yet.  */
                        readyThreadGroup = false;

                        /*  Search for the next ready thread group.  */
                        while ((!readyThreadGroup) && (numReadyGroups > 0))
                        {
                            /*  Get the next ready group in the queue.  */
                            nextGroup = readyGroups[nextReadyGroup];
                            nextThread = nextGroup * threadGroup;

                            /*  Sets if the current thread group is ready.  */
                            readyThreadGroup = threadTable[nextThread].ready && !threadTable[nextThread].free;

                            /*  Check that all the threads in the group are ready.  */
                            for(j = 1; (j < threadGroup) && readyThreadGroup; j++)
                            {
                                /*  Determine if the thread in the group is ready.  */
                                readyThreadGroup = readyThreadGroup && (threadTable[GPU_MOD(nextThread + j, numBuffers)].ready &&
                                    !threadTable[GPU_MOD(nextThread + j, numBuffers)].free);
                            }

                            GPU_ASSERT(
                                if (threadTable[nextThread].nextFetchCycle > cycle)
                                    panic("ShaderFetch", "clock", "Fetching from a thread with active fetch delay.");
                            )

                            /*  Check if the swap on block is enabled or disabled.  If
                                disabled swap to the next ready thread always (round robin),
                                if not swap only when the thread/group becomes blocked and insert
                                into the fetched group queue.   */
                            if (!swapOnBlock)
                            {
                                /*  Update pointer to the next ready group.  */
                                nextReadyGroup = GPU_MOD(nextReadyGroup + 1, 2 * numGroups);

                                /*  Update number of ready thread groups.  */
                                numReadyGroups--;
                            }
                            else if (swapOnBlock && !readyThreadGroup)
                            {
                                /*  Update pointer to the next ready group.  */
                                nextReadyGroup = GPU_MOD(nextReadyGroup + 1, 2 * numGroups);

                                /*  Update number of ready thread groups.  */
                                numReadyGroups--;

                                /*  Check if in the last iteration the group was ready.  If so it means that
                                    instructions were fetched from the group and must be added to the fetched group
                                    queue.  */
                                if (lastVisitedGroupReady)
                                {
                                    /*  Add thread group to the fetched groups queue.  */
                                    fetchedGroups[nextFreeFetched] = nextGroup;

                                    /*  Update pointer to the next free entry in the fetched group queue.  */
                                    nextFreeFetched = GPU_MOD(nextFreeFetched + 1, numGroups);

                                    /*  Update number of fetched groups.  */
                                    numFetchedGroups++;
                                }
                            }

                            /*  Check if the thread group was ready.  */
                            if (readyThreadGroup)
                            {
                                /*  Check that all the threads in the group are ready.  */
                                GPU_ASSERT(
                                    if (readyThreads[nextGroup] < threadGroup)
                                        panic("ShaderFetch", "clock", "Non ready threads in the current thread group.");
                                )

                                /*  Get PC for the current thread group.  */
                                groupPC = threadTable[nextThread].PC;

                                /*  Fetch all the instructions for the group.  */
                                fetchInstructionGroup(cycle, nextThread);

                            }
                            else
                            {
                                /*  Mark thread in the group as not stored in any queue.  */
                                groupBlocked[nextGroup] = true;
                            }
                        }
                    }
                    else
                    {
                        /*  Nothing to send to the decoder.  */
                        readyThreadGroup = false;

//if (cycle > 2500000)
//printf("ShF(%p) %lld => fetching from partition %d batch %d | lastInBatch %d | nextFetchInBatch %d\n",
//    this, cycle, currentBatchPartition, fetchBatch[currentBatchPartition],
//    lastInBatch[currentBatchPartition][fetchBatch[currentBatchPartition]], nextFetchInBatch[currentBatchPartition]);

                        /*  Check if the batch queue is empty.  */
                        if (lastInBatch[currentBatchPartition][fetchBatch[currentBatchPartition]] == 0)
                        {
                            /*  Search for a non empty batch partition.  */
                            if (lastInBatch[VERTEX_PARTITION][fetchBatch[VERTEX_PARTITION]] != 0)
                                currentBatchPartition = VERTEX_PARTITION;
                            else if (lastInBatch[TRIANGLE_PARTITION][fetchBatch[TRIANGLE_PARTITION]] != 0)
                                currentBatchPartition = TRIANGLE_PARTITION;
                            else if (lastInBatch[FRAGMENT_PARTITION][fetchBatch[FRAGMENT_PARTITION]] != 0)
                                currentBatchPartition = FRAGMENT_PARTITION;

                            /*  Stop the fetch and send instruction loop.  */
                            searchReadyGroup = false;
                        }
                        /*  Check if we are waiting for the next group in the batch to be filled or the batch
                            to be closed.  */
                        else if (nextFetchInBatch[currentBatchPartition] == lastInBatch[currentBatchPartition][fetchBatch[currentBatchPartition]])
                        {
                            /*  Check if waiting batch to end.  */
                            if (batchEnding[currentBatchPartition])
                            {

                                /*  Check if the current fetch batch is closed or was restarted because of being already ending.  */
                                if (closedBatch[currentBatchPartition][fetchBatch[currentBatchPartition]])
                                {
//if (cycle > 2500000)
//printf("ShF(%p) %lld => opening partition %d batch %d\n", this, cycle, currentBatchPartition, fetchBatch[currentBatchPartition]);
                                    /*  Open the batch for new inputs.  */
                                    closedBatch[currentBatchPartition][fetchBatch[currentBatchPartition]] = false;

                                    /*  Reset pointer to the last group in the batch.  */
                                    lastInBatch[currentBatchPartition][fetchBatch[currentBatchPartition]] = 0;

                                    /*  Start fetching from the next batch.  */
                                    fetchBatch[currentBatchPartition] = (fetchBatch[currentBatchPartition] + 1) & 0x01;

                                    /*  Check if the next fetch batch is already closed.  */
                                    if (closedBatch[currentBatchPartition][fetchBatch[currentBatchPartition]])
                                    {
                                        /*  Start loading into the next batch.  */
                                        loadBatch[currentBatchPartition] = (loadBatch[currentBatchPartition] + 1) & 0x01;
                                    }
                                }

//if (cycle > 2500000)
//printf("ShF(%p) %lld => A start fetching new partition %d batch %d\n", this, cycle, currentBatchPartition, fetchBatch[currentBatchPartition]);

                                /*  Restart the batch fetching mechanism.  */
                                nextFetchInBatch[currentBatchPartition] = 0;
                                batchEnding[currentBatchPartition] = false;
                            }
                            /*  Check if the batch was finally closed and then wrap to the first group in the batch.  */
                            else if (closedBatch[currentBatchPartition][fetchBatch[currentBatchPartition]])
                            {
                                nextFetchInBatch[currentBatchPartition] = 0;
                            }
                            else
                            {
                                /*  Check if the batch must be closed because the last signal was lost somewhere.  */
                                if (cycle >= (lastLoadCycle[currentBatchPartition] + BATCH_CYCLE_THRESHOLD))
                                {
                                    /*  Close the current batch.  */
                                    if ((loadBatch[currentBatchPartition] == fetchBatch[currentBatchPartition]))
                                    {
//if (cycle > 2500000)
//printf("ShF(%p) %lld => closing (timer) partition %d batch %d\n", this, cycle, currentBatchPartition, loadBatch[currentBatchPartition]);
                                        closedBatch[currentBatchPartition][loadBatch[currentBatchPartition]] = true;

                                        /*  Check if the next load batch is still in use.  */
                                        //if (loadBatch[currentBatchPartition] == fetchBatch[currentBatchPartition])
                                        //{
                                            /*  Start loading the next batch.  */
                                            loadBatch[currentBatchPartition] = (loadBatch[currentBatchPartition] + 1) & 0x01;
                                        //}
                                    }
                                }

                                /*  Skip to a higher priority non empty batch partition.  */
                                if (lastInBatch[VERTEX_PARTITION][fetchBatch[VERTEX_PARTITION]] != 0)
                                    currentBatchPartition = VERTEX_PARTITION;
                                else if (lastInBatch[TRIANGLE_PARTITION][fetchBatch[VERTEX_PARTITION]] != 0)
                                    currentBatchPartition = TRIANGLE_PARTITION;
                                else if (lastInBatch[FRAGMENT_PARTITION][fetchBatch[VERTEX_PARTITION]] != 0)
                                    currentBatchPartition = FRAGMENT_PARTITION;

                                /*  Stop the fetch and send instruction loop.  */
                                searchReadyGroup = false;
                            }
                        }
                        else
                        {
                            /*  Get the next thread group in the batch.  */
                            nextGroup = batchGroupQueue[currentBatchPartition][fetchBatch[currentBatchPartition]][nextFetchInBatch[currentBatchPartition]];
                            nextThread = nextGroup * threadGroup;

                            /*  Check if the thread group is ready.  */
                            readyThreadGroup = threadTable[nextThread].ready && !threadTable[nextThread].free &&
                                (threadTable[nextThread].nextFetchCycle <= cycle);

                            /*  Check that all the threads in the group are ready.  */
                            for(j = 1; (j < threadGroup) && readyThreadGroup; j++)
                            {
                                /*  Determine if the thread in the group is ready.  */
                                readyThreadGroup = readyThreadGroup && (threadTable[GPU_MOD(nextThread + j, numBuffers)].ready &&
                                    !threadTable[GPU_MOD(nextThread + j, numBuffers)].free);
                            }

//if (cycle > 2500000)
//printf("ShF(%p) %lld => nextGroup %d | readyThreadGroup %d\n",
//    this, cycle, nextGroup, readyThreadGroup);

                            /*  Check if the thread group is ready before fetching the group instructions.  */
                            if (readyThreadGroup)
                            {
                                /*  Check that all the threads in the group are ready.  */
                                GPU_ASSERT(
                                    if (readyThreads[nextGroup] < threadGroup)
                                        panic("ShaderFetch", "clock", "Non ready threads in the current thread group.");
                                )

                                /*  Get PC for the current thread group.  */
                                groupPC = threadTable[nextThread].PC;

                                /*  Fetch all the instructions for the group.  */
                                fetchInstructionGroup(cycle, nextThread);

                                /*  If start of the batch store the current batch PC.  */
                                if (nextFetchInBatch[currentBatchPartition] == 0)
                                {
                                    /*  Update batch PC.  */
                                    batchPC[currentBatchPartition] = groupPC;
                                }
                                else
                                {
                                    /*  If not, check for desynchronization in the batch.  */
                                    GPU_ASSERT(
                                        if (batchPC[currentBatchPartition] != groupPC)
                                        {
                                            printf("cycle = %lld | partition = %d | groupPC = %d | batchPC = %d\n",
                                                cycle, currentBatchPartition, groupPC, batchPC[currentBatchPartition]);
                                            panic("ShaderFetch", "clock", "Desynchronized PC inside a batch.");
                                        }
                                    )
                                }
                                /*  Update the pointer to the next group in the batch queue.  */
                                nextFetchInBatch[currentBatchPartition] = nextFetchInBatch[currentBatchPartition] + 1;

                                /*  Wrap to the first group in the batch if last and closed.  */
                                if ((nextFetchInBatch[currentBatchPartition] == lastInBatch[currentBatchPartition][fetchBatch[currentBatchPartition]]) &&
                                    closedBatch[currentBatchPartition][fetchBatch[currentBatchPartition]] &&
                                    !batchEnding[currentBatchPartition])
                                {
                                    nextFetchInBatch[currentBatchPartition] = 0;

                                    /*  Skip to a higher priority non empty batch partition.  */
                                    if (lastInBatch[VERTEX_PARTITION][fetchBatch[VERTEX_PARTITION]] != 0)
                                        currentBatchPartition = VERTEX_PARTITION;
                                    else if (lastInBatch[TRIANGLE_PARTITION][fetchBatch[TRIANGLE_PARTITION]] != 0)
                                        currentBatchPartition = TRIANGLE_PARTITION;
                                    else if (lastInBatch[FRAGMENT_PARTITION][fetchBatch[FRAGMENT_PARTITION]] != 0)
                                        currentBatchPartition = FRAGMENT_PARTITION;
                                }

                                /*  Check that the pointer to the next fetch group in the batch queue doesn't overflows.  */
                                GPU_ASSERT(
                                    if (nextFetchInBatch[currentBatchPartition] > numGroups)
                                        panic("ShaderFetch", "clock", "Pointer to next fetch group in batch queue overflowed.");
                                )
                            }
                            else
                            {
                                /*  NOTE:  SMALL BATCHES SHOULD NOT BE ACCEPTED AND THE CURRENT IMPLEMENTATION
                                           MUST CHANGE TO AVOID THEM.  */

                                /*  Check for REPEAT_LAST desynchroniztion in small batches.  */
                                //if (batchEnding && threadTable[nextThread].end)
                                //{
//printf("ShF (%p) %lld => desynch detected\n", this, cycle);
                                    /*  Update the pointer to the next group in the batch queue.  */
                                //    nextFetchInBatch[currentBatchPartition] = nextFetchInBatch[currentBatchPartition] + 1;
                                //}

                                /*  Skip to a higher priority non empty batch partition.  */
                                if (lastInBatch[VERTEX_PARTITION][fetchBatch[VERTEX_PARTITION]] != 0)
                                    currentBatchPartition = VERTEX_PARTITION;
                                else if (lastInBatch[TRIANGLE_PARTITION][fetchBatch[TRIANGLE_PARTITION]] != 0)
                                    currentBatchPartition = TRIANGLE_PARTITION;
                                else if (lastInBatch[FRAGMENT_PARTITION][fetchBatch[FRAGMENT_PARTITION]] != 0)
                                    currentBatchPartition = FRAGMENT_PARTITION;

                                /*  Stop the fetch and send instruction loop.  */
                                searchReadyGroup = false;
                            }

                            /*  Check if the batch has completed execution.  */
                            if ((nextFetchInBatch[currentBatchPartition] == lastInBatch[currentBatchPartition][fetchBatch[currentBatchPartition]]) &&
                                batchEnding[currentBatchPartition])
                            {
                                /*  Check if current fetch batch is closed or was restarted because the batch is already ending.  */
                                if (closedBatch[currentBatchPartition][fetchBatch[currentBatchPartition]])
                                {
//if (cycle > 2500000)
//printf("ShF(%p) %lld => opening partition %d batch %d\n", this, cycle, currentBatchPartition, fetchBatch[currentBatchPartition]);

                                    /*  Open the batch for new inputs.  */
                                    closedBatch[currentBatchPartition][fetchBatch[currentBatchPartition]] = false;

                                    /*  Reset last group in batch pointer.  */
                                    lastInBatch[currentBatchPartition][fetchBatch[currentBatchPartition]] = 0;

                                    /*  Start fetching from the next batch.  */
                                    fetchBatch[currentBatchPartition] = (fetchBatch[currentBatchPartition] + 1) & 0x01;

                                    /*  Check if the next fetch batch is already closed.  */
                                    if (closedBatch[currentBatchPartition][fetchBatch[currentBatchPartition]])
                                    {
                                        /*  Start loading into the next batch.  */
                                        loadBatch[currentBatchPartition] = (loadBatch[currentBatchPartition] + 1) & 0x01;
                                    }
                                }

//if (cycle > 2500000)
//printf("ShF(%p) %lld => B start fetching new partition %d batch %d\n", this, cycle, currentBatchPartition, fetchBatch[currentBatchPartition]);

                                /*  Restart the batch fetching mechanism.  */
                                nextFetchInBatch[currentBatchPartition] = 0;
                                batchEnding[currentBatchPartition] = false;
                            }
                        }
                    }
                }

                /*  Check if there is a ready group of threads.  */
                if (readyThreadGroup)
                {
                    /*  Send instructions from the current thread to the decoder.  */
                    for(j = 0; j < instrCycle; j++)
                    {
                        /*  Get next instruction for the thread.  */
                        shExecInstr = fetchGroup[nextThreadInGroup][j];

                        /*  Send instruction to the shader decoder.  */
                        sendInstruction(cycle, GPU_MOD(nextThread + nextThreadInGroup, numBuffers), shExecInstr);
                    }

                    /*  Set thread next fetch cycle.  */
                    threadTable[GPU_MOD(nextThread + nextThreadInGroup, numBuffers)].nextFetchCycle = cycle + fetchDelay;

                    /*  Update pointer to the next thread in the current thread group.  */
                    nextThreadInGroup = GPU_MOD(nextThreadInGroup + 1, threadGroup);

                    /*  Update the number of fetched threads.  */
                    fetchedThreads++;

                    /*  Add fetched group to the fetched group queue after fetching ends
                        for all the threads.  */
                    /*  Check for last thread in the group.  */
                    if (threadWindow && (nextThreadInGroup == 0))
                    {
                        /*  Check if fetched groups number.  */
                        GPU_ASSERT(
                            if (numFetchedGroups == numGroups)
                                panic("ShaderFetch", "clock", "Fetched groups queue is full.");
                        )

                        /*  Check if swap thread on block is enabled or disabled.  If disabled the
                            current fetch block is always inserted into the fetched group queue as it
                            won't be active anymore.  If enabled check if the group has been already
                            inserted the first time it became active.  */
                        if (!swapOnBlock)
                        {
                            /*  Add thread group to the fetched groups queue.  */
                            fetchedGroups[nextFreeFetched] = nextGroup;

                            /*  Update pointer to the next free entry in the fetched group queue.  */
                            nextFreeFetched = GPU_MOD(nextFreeFetched + 1, numGroups);

                            /*  Update number of fetched groups.  */
                            numFetchedGroups++;
                       }
                    }
                }
            }
        }
        else
        {
            /*  Execute instructions from N ready threads.  */
            for(i = 0, visited = 0; (i < threadsCycle) && (visited < numBuffers); i++)
            {
                /*  Search for the next ready thread.  */
                while((threadTable[nextThread].free || !threadTable[nextThread].ready) && (visited < numBuffers))
                {
                    /*  Step over a block of threads.  */
                    nextThread = GPU_MOD(nextThread + 1, numBuffers);

                    /*  Update number of visited threads.  */
                    visited++;
                }

                /*  Check if there is a ready group of threads.  */
                if (threadTable[nextThread].ready && !threadTable[nextThread].free)
                {
                    /*  Reset fetched instructions flags for current thread.  */
                    fetchedSIMD = fetchedScalar = false;

                    /*  Execute M instructions from a ready thread.  */
                    for(j = 0; j < instrCycle; j++)
                    {
                        /*  Fetch an instruction from the ready thread.  */
                        fetchInstruction(cycle, nextThread);
                    }

                    /*  Update pointer to next thread.  Round-Robin.  */
                    nextThread = GPU_MOD(nextThread + 1, numBuffers);
                }
            }
        }
    }

    /*  Output Management.  */

    /*  Read consumer state.  This is a permament signal.  */
    if (!consumerSignal->read(cycle, (DynamicObject *&) consumerStateInfo))
    {
        /*  No signal?  Electrons on strike!!!  So we go to strike too :).  */
        panic("ShaderFetch","clock", "No signal received from Shader consumer.");
    }
    else
    {
        /*  Store consumer state.  */
        consumerState = consumerStateInfo->getState();

        /*  Delete received consumer state.  */
        delete consumerStateInfo;
    }


    /*  Check if there is a transmission in progress.  */
    if (transInProgress)
    {
        GPU_DEBUG_BOX(
            printf("ShaderFetch => Transmission in progress Thread = %d Remaining cycles = %d.\n",
                finThreadList[firstFinThread], transCycles);
        )

        /*  Update number of remaining transmission cycles.  */
        transCycles--;

        /*  Check end-of-transmission.  */
        if (transCycles == 0)
        {
            GPU_DEBUG_BOX(
                printf("ShaderFetch => Loading new inputs.\n");
            )

            /*  Mark that the current transmission has finished (for the next cycle.  */
            transInProgress = FALSE;

            /*  Reload the now empty threads with data from an input buffer.
                If there is no input buffer filled with data set as free.  */
            for(i = 0; i < currentOutput; i++)
                reloadThread(cycle, finThreadList[firstFinThread]);
        }
    }
    else
    {

        /*  Check if there are outputs to transmit.  */
        if ((numFinishedThreads >= outputCycle) ||
            ((numFinishedThreads > 0) && (numFreeThreads == (numBuffers - numFinishedThreads))))
        {
            /*  Check if consumer is ready to receive a new output.  */
            if (consumerState == CONS_READY)
            {
                /*  Calculate number of outputs to send back.  */
                currentOutput = GPU_MIN(outputCycle, numFinishedThreads);

                /*  Send N outputs to the consumer.  */
                for(i = 0; i < currentOutput; i++)
                {
                    /*  Get the older finished thread from the list.  */
                    outputThread = finThreadList[GPU_MOD(firstFinThread + i, numThreads)];

                    /* Send output from thread to the consumer.  */
                    sendOutput(cycle, outputThread);
                }

                /*  If multicycle transmission, set transInProgress.  */
                if (transCycles > 1)
                {
                    /*  Multicyle transmission.  Decrease cycle count.  Set transmission in progress on.  */
                    transCycles--;
                    transInProgress = TRUE;
                }
                else
                {
                    GPU_DEBUG_BOX(
                        printf("ShaderFetch => Loading new inputs.\n");
                    )

                    /*  Reload the threads with a new set of inputs.  If no
                        input is available set as free to wait new Inputs.  */
                    for(i = 0; i < currentOutput; i++)
                        reloadThread(cycle, finThreadList[firstFinThread]);
                }
            }
        }
    }

    /*  Update Shader state signal.  */
    if (((!lockStep) && ((numFreeThreads < (2 * inputCycle)) || (freeResources < (2 * inputCycle * maxThreadResources)))) ||
        (lockStep && ((numFreeThreads < (2 * GPU_MAX(inputCycle, threadGroup)) ||
            (freeResources < (2 * GPU_MAX(inputCycle, threadGroup) * maxThreadResources))))))
    {
        GPU_DEBUG_BOX(
            printf("ShaderFetch >> Sending Busy.\n");
        )

        /*  Check if in order batch mode is enabled.   */
        if (!threadWindow)
        {
            /*  Close the current batch.  */
            if ((loadBatch[currentBatchPartition] == fetchBatch[currentBatchPartition])
                && (lastInBatch[currentBatchPartition][loadBatch[currentBatchPartition]] > 0))
            {
//if (cycle > 2500000)
//printf("ShF(%p) %lld => closing (resources) partition %d batch %d\n", this, cycle, currentBatchPartition, loadBatch[currentBatchPartition]);

                closedBatch[currentBatchPartition][loadBatch[currentBatchPartition]] = true;

                /*  Check if the next load batch is still in use.  */
                //if (loadBatch[currentBatchPartition] == fetchBatch[currentBatchPartition])
                //{
                    /*  Start loading the next batch.  */
                    loadBatch[currentBatchPartition] = (loadBatch[currentBatchPartition] + 1) & 0x01;
                //}
            }
        }

        shState = SH_BUSY;
    }
    else if (numFreeThreads == numBuffers)
    {
        GPU_DEBUG_BOX(
            printf("ShaderFetch >> Sending Empty.\n");
        )

        shState = SH_EMPTY;
    }
    else
    {
        GPU_DEBUG_BOX(
            printf("ShaderFetch >> Sending Ready (NumFreeThreads: %d).\n", numFreeThreads);
        )

        shState = SH_READY;
    }

    readySignal->write(cycle, new ShaderStateInfo(shState));

    /*  Update statistics.  */
    nThReadyAvg->incavg(numReadyThreads);
    nThReadyMax->max(numReadyThreads);
    nThReadyMin->min(numReadyThreads);
    nThReady->inc(numReadyThreads);
    nThBlocked->inc(numBuffers - numReadyThreads - numFreeThreads - numFinishedThreads);
    nThFinished->inc(numFinishedThreads);
    nThFree->inc(numFreeThreads);
    nResources->inc(numResources - freeResources);

}

/*  Obtains a free thread to store a new input.  */
s32bit ShaderFetch::getFreeThread(u32bit partition)
{
    u32bit i;
    u32bit requiredResources;

    //  Calculate the number of resources reserved by the thread.
    requiredResources = GPU_MAX(activeInputs[partition], GPU_MAX(threadResources[partition], activeOutputs[partition]));

    /*  Check if there are free threads and thread resources as set for the partition.  */
    if ((numFreeThreads == 0) || (freeResources < requiredResources))
        return -1;

    /*  Check if there are runnable free threads.  */

    i = freeBufferList[firstEmptyBuffer];

    threadTable[i].free = FALSE;

    firstEmptyBuffer = GPU_MOD(firstEmptyBuffer + 1, numBuffers);

    /*  Update free threads counter..  */
    numFreeThreads--;

    /*  Update number of free resources.  */
    freeResources -= requiredResources;

    return i;
}

/*  Frees a thread.  */
void ShaderFetch::addFreeThread(u32bit numThread, u32bit partition)
{
    u32bit requiredResources;

    /*  Check that the thread number is correct.  */
    GPU_ASSERT
    (
        if (numThread > numBuffers)
            panic("ShaderFetch", "addFreeThread", "Invalid thread number.");
    )

    /*  Check free list overflow.  */
    GPU_ASSERT(
        if (numFreeThreads >= numBuffers)
        {
            panic("ShaderFetch", "addFreeThread", "Free Buffer List overflow.");
        }
        if (freeResources >= numResources)
        {
            panic("ShaderFetch", "addFreeThread", "Free resources overflow.");
        }
    )

    /*  Free the thread.  */
    numFreeThreads++;

    //  Calculate the number of resources reserved by the thread.
    requiredResources = GPU_MAX(activeInputs[partition], GPU_MAX(threadResources[partition], activeOutputs[partition]));

    /*  Return the thread resources used by the thread.  */
    freeResources += requiredResources;

    lastEmptyBuffer = GPU_MOD(lastEmptyBuffer + 1, numBuffers);

    freeBufferList[lastEmptyBuffer] = numThread;

    /*  Set it as free.  */
    threadTable[numThread].free = TRUE;
    threadTable[numThread].ready = FALSE;
    threadTable[numThread].end = FALSE;
}

/*  Free the thread from the finished output.  If a there are inputs
    in the buffers load one and reset the thread.  If not set it as
    a free thread. */

void ShaderFetch::reloadThread(u64bit cycle, u32bit nThread)
{
    u32bit newThread;
    u32bit newGroup;
    u32bit partition;
    bool lastInput;
    u32bit i;

    /*  Check shader model.  */
    if (unifiedModel)
    {
        /*  Select shader partition given shader input type.  */
        switch(threadTable[nThread].shInput->getInputMode())
        {
            case SHIM_VERTEX:

                partition = VERTEX_PARTITION;
                break;

            case SHIM_TRIANGLE:

                partition = TRIANGLE_PARTITION;
                break;

            case SHIM_FRAGMENT:

                partition = FRAGMENT_PARTITION;
                break;

            case SHIM_MICROTRIANGLE_FRAGMENT:
                
                partition = FRAGMENT_PARTITION;
                break;

            default:

                panic("ShaderFetch", "reloadThread", "Unsupported shader input mode.");
                break;
        }
    }
    else
    {
        /*  Set unique partition.  */
        partition = PRIMARY_PARTITION;
    }

    /*  Now this thread is free.  So no longer is finished.  */
    numFinishedThreads--;
    firstFinThread = GPU_MOD(firstFinThread + 1, numThreads);

    /*  Add the finished thread to the free buffer list.  */
    addFreeThread(nThread, partition);

    /*  Check if there is a filled input buffer.  If there are less
        or the same number of free buffers as input buffers means
        that the only filled buffers are runnable threads.  So
        we can't load a new thread.  */

    if (numFreeThreads <= numInputBuffers)
    {
        /*  Get a filled input buffer and select it for execution.  */

        /*  Get the older filled buffer.  */
        newThread = freeBufferList[firstFilledBuffer];

        /*  Update pointer to the older filled buffer.  */
        firstFilledBuffer = GPU_MOD(firstFilledBuffer + 1, numBuffers);

        /*  Set the thread/buffer as executable.  */
        threadTable[newThread].ready = TRUE;

        /*  Update number of ready threads.  */
        numReadyThreads++;

        /*  Check shader model.  */
        if (unifiedModel)
        {
            /*  Select shader partition given shader input type.  */
            switch(threadTable[newThread].shInput->getInputMode())
            {
                case SHIM_VERTEX:

                    partition = VERTEX_PARTITION;
                    break;

                case SHIM_TRIANGLE:

                    partition = TRIANGLE_PARTITION;
                    break;

                case SHIM_FRAGMENT:

                    partition = FRAGMENT_PARTITION;
                    break;

                case SHIM_MICROTRIANGLE_FRAGMENT:
                    
                    partition = FRAGMENT_PARTITION;
                    break;

                default:

                    panic("ShaderFetch", "reloadThread", "Unsupported shader input mode.");
                    break;
            }
        }
        else
        {
            /*  Set unique partition.  */
            partition = PRIMARY_PARTITION;
        }

        /*  Calculate the group for the thread.  */
        newGroup = newThread / threadGroup;

//printf("ShF(%p) %lld => reloading old thread %d | firstFilledBuffer %d | newThread %d partition %d \n",
//    this, cycle, nThread, firstFilledBuffer, partition, newThread, partition);

        /*  Update the number of ready threads in the thread group.  */
        readyThreads[newGroup]++;

        /*  Check if thread window mode is enabled.  */
        if (threadWindow)
        {
            /******    THIS SECTION IS FOR THE OUT OF ORDER THREAD WINDOW MODE    ******/

            /*  Check if the whole thread group is ready.  */
            if ((readyThreads[newGroup] == threadGroup) && groupBlocked[newGroup])
            {
                /*  Check ready queue.  */
                GPU_ASSERT(
                    if (numReadyGroups == numGroups)
                        panic("ShaderFetch", "reloadThread", "Ready group queue full.");
                )

//printf("ShF (reload) => adding ready group %d at %d\n", newGroup, nextFreeReady);

                /*  Add thread group to the ready queue.  */
                readyGroups[nextFreeReady] = newGroup;

                /*  Set group as stored in the ready queue.  */
                groupBlocked[newGroup] = false;

                /*  Update pointer to the next free entry.  */
                nextFreeReady = GPU_MOD(nextFreeReady + 1, 2 * numGroups);

                /*  Update number of ready groups.  */
                numReadyGroups++;
            }
        }
        else
        {
            /******    THIS SECTION IS FOR THE IN ORDER BATCH QUEUE MODE    ******/

            /*  Check if the whole thread group is ready.  */
            if (readyThreads[newGroup] == threadGroup)
            {
                /*  Check that the batch queue isn't already full.  */
                GPU_ASSERT(
                    if (lastInBatch[partition][loadBatch[partition]] == numGroups)
                        panic("ShaderFetch", "reloadThread", "Batch group queue is full.");
                )

                /*  Check that the batch is not closed.  */
                GPU_ASSERT(
                    if ((fetchBatch[partition] == loadBatch[partition]) && closedBatch[partition][loadBatch[partition]])
                        panic("ShaderFetch", "reloadThread", "Load batch is already closed.");
                )

//if (cycle > 2500000)
//printf("ShF(%p) %lld => adding (reload) new group %d into partition %d batch %d position %d | lastInBatch (fetch) %d | nextFetchInBatch %d\n",
//    this, cycle, newGroup, partition, loadBatch[partition], lastInBatch[partition][loadBatch[partition]], lastInBatch[partition][fetchBatch[partition]], nextFetchInBatch[partition]);

                /*  Add the new group to the batch queue.  */
                batchGroupQueue[partition][loadBatch[partition]][lastInBatch[partition][loadBatch[partition]]] = newGroup;

                /*  Update pointer to the next free entry in the batch queue.  */
                lastInBatch[partition][loadBatch[partition]]++;

                /*  Update last cycle a group was loaded into a partition batch.  */
                lastLoadCycle[partition] = cycle;

                /*  Check the whole group for an input marked as last one.  */
                for(i = 0, lastInput = false; i < threadGroup; i++)
                    lastInput |= threadTable[newGroup * threadGroup + i].shInput->isLast();

                /*  Check if the batch must be closed.  */
                if ((lastInput) || (lastInBatch[partition][loadBatch[partition]] == numGroups))
                {
//if (cycle > 2500000)
//printf("ShF(%p) %lld => closing (last in reload) partition %d batch %d\n",
//    this, cycle, partition, loadBatch[partition]);

                    closedBatch[partition][loadBatch[partition]] = true;

                    /*  Check if next load batch is still in use.  */
                    if (loadBatch[partition] == fetchBatch[partition])
                    {
                        /*  Start loading the next batch.  */
                        loadBatch[partition] = (loadBatch[partition] + 1) & 0x01;
                    }
                }
            }
        }

//printf("ShF => (reload) numReadyThreads %d\n", numReadyThreads);

    }
}


/*  Marks a thread as finished and adds it to the list of
    finished threads.  */

void ShaderFetch::finishThread(u32bit numThread)
{
    u32bit lastThread;
    u32bit partition;

    /*  Check shader model.  */
    if (unifiedModel)
    {
        /*  Select shader partition given shader input type.  */
        switch(threadTable[numThread].shInput->getInputMode())
        {
            case SHIM_VERTEX:

                partition = VERTEX_PARTITION;
                break;

            case SHIM_TRIANGLE:

                partition = TRIANGLE_PARTITION;
                break;

            case SHIM_FRAGMENT:

                partition = FRAGMENT_PARTITION;
                break;

            case SHIM_MICROTRIANGLE_FRAGMENT:
                
                partition = FRAGMENT_PARTITION;
                break;

            default:

                panic("ShaderFetch", "finishThread", "Unsupported shader input mode.");
                break;
        }

    }
    else
    {
        /*  Set unique partition.  */
        partition = PRIMARY_PARTITION;
    }

    /*  DOESN'T CHECK THAT THE TABLE OVERFLOWS (SAME THREAD ADDED 2+ TIMES).  */

    /*  Set thread as finished.  Send output to next unit.  */
    if (!threadTable[numThread].end)
    {
        threadTable[numThread].end = TRUE;
        threadTable[numThread].ready = FALSE;
        threadTable[numThread].blocked = false;
        threadTable[numThread].zexported = false;
        
        if (threadWindow)
        {
            u32bit group = numThread / threadGroup;
            
            //  Update the number of threads finished in a group.
            finThreads[group]++;
            
            //  Check if all the threads in the group have finished.
            if (finThreads[group] == threadGroup)
            {
                //  Queue all the threads in the finished thread list.
                for(u32bit t = 0; t < threadGroup; t++)
                {
                    numFinishedThreads++;
                    finThreadList[lastFinThread] = group * threadGroup + t;
                    lastFinThread = GPU_MOD(lastFinThread + 1, numThreads);
                }
                
                //  Reset the finished thread counter for the group.
                finThreads[group] = 0;
            }
        }
        else
        {
            numFinishedThreads++;

            /*  Add to the finished thread list.  */
            finThreadList[lastFinThread] = numThread;
            lastFinThread = GPU_MOD(lastFinThread + 1, numThreads);
        }
        
        if (!threadWindow)
        {
            if (!batchEnding[partition])
            {
                /*  Check if the current batch is ending. */
                batchEnding[partition] |= (numThread == (batchGroupQueue[partition][fetchBatch[partition]][0] * threadGroup))
                    && !((loadBatch[partition] == fetchBatch[partition]) && (lastInBatch[partition][fetchBatch[partition]] == 0));

                /*  Check if the batch must be closed.  */
                if (batchEnding[partition] && !closedBatch[partition][fetchBatch[partition]])
                {
//printf("ShF(%p) => closing (ending inputs) partition %d batch %d\n", this, partition, fetchBatch[partition]);
                    /*  Close the batch.  */
                    closedBatch[partition][fetchBatch[partition]] = true;

                    /*  Check if the other batch is being used.  */
                    if (loadBatch[partition] == fetchBatch[partition])
                    {
                        /*  Start loading the next batch.  */
                        loadBatch[partition] = (loadBatch[partition] + 1) & 0x01;
                    }
                }
            }

            /*  Check if the current batch can reopen.  */

            /*  Get the identifier for the last thread in the batch.  */
            lastThread = batchGroupQueue[partition][fetchBatch[partition]][lastInBatch[partition][fetchBatch[partition]] - 1] * threadGroup + threadGroup - 1;

//printf("ShF(%p) => finishing partition %d threadGroup %d thread %d lastThread %d firstThread %d\n", this, partition, numThread/threadGroup, numThread, lastThread,
//    (batchGroupQueue[partition][fetchBatch[partition]][0] * threadGroup));

            if ((lastThread == numThread) && batchEnding[partition])
            {
                /*  Check if the current fetch batch is closed or was already restarted.  */
                if (closedBatch[partition][fetchBatch[partition]])
                {
//printf("ShF(%p) => opening partition %d batch %d\n", this, partition, fetchBatch[partition]);
                    /*  Open the batch for new inputs.  */
                    closedBatch[partition][fetchBatch[partition]] = false;

                    /*  Reset the pointer to the last group in the batch.  */
                    lastInBatch[partition][fetchBatch[partition]] = 0;

                    /*  Start fetching from the next batch.  */
                    fetchBatch[partition] = (fetchBatch[partition] + 1) & 0x01;

                    /*  Check if the next fetch batch is already closed.  */
                    if (closedBatch[partition][fetchBatch[partition]])
                    {
                        /*  Start loading into the next batch.  */
                        loadBatch[partition] = (loadBatch[partition] + 1) & 0x01;
                    }
                 }

//printf("ShF(%p) => C start fetching new batch %d\n", this, fetchBatch[partition]);

                /*  Restart the batch fetching mechanism.  */
                nextFetchInBatch[partition] = 0;
                batchEnding[partition] = false;
            }
        }
    }
}

/*  Process a Shader Command.  */
void ShaderFetch::processShaderCommand(ShaderCommand *command, u32bit partition)
{
    u32bit i, firstAddr, lastAddr;
    QuadFloat *values;

    switch(command->getCommandType())
    {
        case LOAD_PROGRAM:
            /*  New Shader Program must be loaded */

            /*  NOTE: REMOVED FOR UNIFIED MODEL AS CURRENTLY THE SHADER HAS NO WAY TO KNOW THAT VERTEX
                PROCESSING HAS ENDED.  */
            /*  All previous inputs (threads + buffers) should have finished already.  */
            GPU_ASSERT(
                if ((!unifiedModel) && (numFreeThreads != numBuffers))
                    panic("ShaderFetch", "processShaderCommand", "Shader is still processing inputs.");
            )

            //  Load program into the Shader emulator.
            //shEmul.loadShaderProgram(command->getProgramCode(), command->getLoadPC(), command->getProgramCodeSize());
            shEmul.loadShaderProgramLight(command->getProgramCode(), command->getLoadPC(), command->getProgramCodeSize());

            GPU_DEBUG_BOX(
                printf("ShaderFetch >> LOAD_PROGRAM Size %d PC %x\n",
                    command->getProgramCodeSize(), command->getLoadPC());
            )

            break;

        case PARAM_WRITE:
            /*  New Shader Parameters must be loaded */

            /*  NOTE: REMOVED FOR UNIFIED MODEL AS CURRENTLY THE SHADER HAS NO WAY TO KNOW THAT VERTEX
                PROCESSING HAS ENDED.  */
            /*  All previous inputs (thread + buffers) should have finished.  */
            /*  Command Processor should now that the shader is not ready.  */
            GPU_ASSERT(
                if ((!unifiedModel) && (numFreeThreads != numBuffers))
                    panic("ShaderFetch", "processShaderCommand", "Shader is still processing inputs.");
            )

            /*  !!!!!! ONE BY ONE WRITE.  I SHOULD CHANGE SHADEREMULATOR
                INTERFACE TO SUPPORT THIS WITH A SINGLE CALL.  */

            /*  Add constant partition offset as required.  */
            firstAddr = command->getParamFirstAddress() + partition * UNIFIED_CONSTANT_NUM_REGS;
            lastAddr = command->getParamFirstAddress() + command->getNumValues() + partition * UNIFIED_CONSTANT_NUM_REGS;

            values = command->getValues();

            //  Write a single value in the constant/parameter memory.  Thread field is unused for constant memory.
            shEmul.loadShaderState(0, PARAM, values, firstAddr, command->getNumValues());

            GPU_DEBUG_BOX(
                for (u32bit i = firstAddr; i < lastAddr; i ++)
                    printf("ShaderFetch >> PARAM_WRITE %d = {%f, %f, %f, %f}\n",
                        i, values[i - firstAddr][0], values[i - firstAddr][1],
                        values[i - firstAddr][2],values[i - firstAddr][3]);
            )

            break;

        case SET_INIT_PC:
        
            /*  New shader program initial PC.  */

            /*  NOTE: REMOVED FOR UNIFIED MODEL AS CURRENTLY THE SHADER HAS NO WAY TO KNOW THAT VERTEX
                PROCESSING HAS ENDED.  */
            /*  All previous inputs (thread + buffers) should have finished.  */
            /*  Command Processor should now that the shader is not ready.  */
            GPU_ASSERT(
                if ((!unifiedModel) && (numFreeThreads != numBuffers))
                    panic("ShaderFetch", "processShaderCommand", "Shader is still processing inputs.");
                    
                if ((partition == VERTEX_PARTITION) && (command->getShaderTarget() != VERTEX_TARGET))
                    panic("ShaderFetch", "processShaderCommand", "Commands to vertex shader can only be for the vertex shader target.\n");
                
            )

            GPU_DEBUG_BOX(
                printf("ShaderFetch => SET_INIT_PC[%d] = %x.\n", command->getShaderTarget(), command->getInitPC());
            )

            /*  Set initial PC.  */
            initPC[command->getShaderTarget()] = command->getInitPC();

            break;

        case SET_THREAD_RES:
            /*  New per shader thread resource usage.  */

            /*  NOTE: REMOVED FOR UNIFIED MODEL AS CURRENTLY THE SHADER HAS NO WAY TO KNOW THAT VERTEX
                PROCESSING HAS ENDED.  */
            /*  All previous inputs (thread + buffers) should have finished.  */
            /*  Command Processor should now that the shader is not ready.  */
            GPU_ASSERT(
                if ((!unifiedModel) && (numFreeThreads != numBuffers))
                    panic("ShaderFetch", "processShaderCommand", "Shader is still processing inputs.");

                if ((partition == VERTEX_PARTITION) && (command->getShaderTarget() != VERTEX_TARGET))
                    panic("ShaderFetch", "processShaderCommand", "Commands to vertex shader can only be for the vertex shader target.\n");
            )

            //  Check shader partition.
            if (unifiedModel)
            {
                //  Set per thread resource usage.  */
                threadResources[command->getShaderTarget()] = command->getThreadResources();
            }
            else
            {
                //  Set per thread resource usage.
                threadResources[PRIMARY_PARTITION] = command->getThreadResources();
            }

            //  Recalculate max thread resources.
            maxThreadResources = computeMaxThreadResources();

            GPU_DEBUG_BOX(
                char buffer[256];
                switch(command->getShaderTarget())
                {
                    case VERTEX_TARGET:
                        sprintf(buffer, "VERTEX_TARGET");
                        break;
                    case FRAGMENT_TARGET:
                        sprintf(buffer, "FRAGMENT_TARGET");
                        break;
                    case TRIANGLE_TARGET:
                        sprintf(buffer, "TRIANGLE_TARGET");
                        break;
                    case MICROTRIANGLE_TARGET:
                        sprintf(buffer, "MICROTRIANGLE_TARGET");
                        break;
                    default:
                        panic("ShaderFetch", "processShaderCommand", "Undefined shader target.");
                        break;
                }
                printf("ShaderFetch => threadResources[%s] = %d, maxResources =  %d\n",
                    buffer, threadResources[command->getShaderTarget()], maxThreadResources);
            )

            break;

        case SET_IN_ATTR:

            /*  Set which shader input registers are enabled for the current shader program.  */

            /*  All previous inputs (thread + buffers) should have finished.  */
            /*  Command Processor should now that the shader is not ready.  */
            /*  NOTE: REMOVED FOR UNIFIFED MODEL AS CURRENTLY THE SHADER HAS NO WAY TO KNOW THAT VERTEX
                PROCESSING HAS ENDED.  */
            GPU_ASSERT(
                if ((!unifiedModel) && (numFreeThreads != numBuffers))
                    panic("ShaderFetch", "processShaderCommand", "Shader is still processing inputs.");
            )

            /*  Check if the shader output register identifier.  */
            GPU_ASSERT(
                if (command->getAttribute() >= MAX_VERTEX_ATTRIBUTES)
                    panic("ShaderFetch", "processShaderCommand", "Out of range shader input register identifier.");
            )

            GPU_DEBUG_BOX(
                printf("ShaderFetch => Setting input attribute %d as %s for partition %s\n", command->getAttribute(),
                    command->isAttributeActive()?"ACTIVE":"INACTIVE",
                    (partition == VERTEX_PARTITION)?"VERTEX":((partition == FRAGMENT_PARTITION)?"FRAGMENT":"TRIANGLE"));
            )

            /*  Check if the command is changing the current state.  */
            if (input[partition][command->getAttribute()] != command->isAttributeActive())
            {
                /*  Update active inputs counter.  */
                activeInputs[partition] = (command->isAttributeActive())?activeInputs[partition] + 1: activeInputs[partition] - 1;
            }

//printf("ShFetch => activeInputs[%s] = %d\n",
//    (partition == VERTEX_PARTITION)?"VERTEX":((partition == FRAGMENT_PARTITION)?"FRAGMENT":"TRIANGLE"),
//    activeInputs[partition]);

            /*  Configure shader output.  */
            input[partition][command->getAttribute()] = command->isAttributeActive();

            //  Recalculate max thread resources.
            maxThreadResources = computeMaxThreadResources();

            break;

        case SET_OUT_ATTR:
            /*  Configure which shader output registers are written by the
                current shader program and sent back.  */

            /*  All previous inputs (thread + buffers) should have finished.  */
            /*  Command Processor should now that the shader is not ready.  */
            /*  NOTE: REMOVED FOR UNIFIFED MODEL AS CURRENTLY THE SHADER HAS NO WAY TO KNOW THAT VERTEX
                PROCESSING HAS ENDED.  */
            GPU_ASSERT(
                if ((!unifiedModel) && (numFreeThreads != numBuffers))
                    panic("ShaderFetch", "processShaderCommand", "Shader is still processing inputs.");
            )

            /*  Check if the shader output register identifier.  */
            GPU_ASSERT(
                if (command->getAttribute() >= MAX_VERTEX_ATTRIBUTES)
                    panic("ShaderFetch", "processShaderCommand", "Out of range shader output register identifier.");
            )

            GPU_DEBUG_BOX(
                printf("ShaderFetch => Setting output attribute %d as %s for partition %s\n", command->getAttribute(),
                    command->isAttributeActive()?"ACTIVE":"INACTIVE",
                    (partition == VERTEX_PARTITION)?"VERTEX":((partition == FRAGMENT_PARTITION)?"FRAGMENT":"TRIANGLE"));
            )

            /*  Check if the command is changing the current state.  */
            if (output[partition][command->getAttribute()] != command->isAttributeActive())
            {
                /*  Update active outputs counter.  */
                activeOutputs[partition] = (command->isAttributeActive())?activeOutputs[partition] + 1: activeOutputs[partition] - 1;
            }

//printf("ShFetch => activeOutputs[%d] = %d\n", partition, activeOutputs[partition]);

            /*  Configure shader output.  */
            output[partition][command->getAttribute()] = command->isAttributeActive();

            //  Recalculate max thread resources.
            maxThreadResources = computeMaxThreadResources();

            break;

        case SET_MULTISAMPLING:
         
            /*  Set multisampling enabled/disabled.  */
             
            GPU_DEBUG_BOX(
                printf("ShaderFetch => Setting multisampling register: %s\n", command->multisamplingEnabled()?"ENABLED":"DISABLED");
            )
 
            /*  Set register.  */
            multisampling = command->multisamplingEnabled();
 
            break;
 
        case SET_MSAA_SAMPLES:
          
            /*  Set the MSAA number of samples.  */
 
            GPU_DEBUG_BOX(
                printf("ShaderFetch => Setting MSAA samples register: %d\n", command->samplesMSAA()); 
            )
 
            /*  Set register.  */
            msaaSamples = command->samplesMSAA();
 
            break;

        default:
            /*  ERROR.  Unknow command code.*/

            panic("ShaderFetch", "clock", "Unsupported command type.");

            break;
    }

    /*  Delete shader command.  */
    delete command;
}

/*  Process a new input vertex.  */
void ShaderFetch::processShaderInput(u64bit cycle, ShaderInput *input)
{
    u32bit newThread;
    u32bit newGroup;
    u32bit partition;
    u32bit i;
    bool lastInput;
    char buff[128];

    /*  A new shader input has arrived */

    /*  Check shader model.  */
    if (unifiedModel)
    {
        /*  Select shader partition given shader input type.  */
        switch(input->getInputMode())
        {
            case SHIM_VERTEX:

                partition = VERTEX_PARTITION;
                break;

            case SHIM_TRIANGLE:

                partition = TRIANGLE_PARTITION;
                break;

            case SHIM_FRAGMENT:

                partition = FRAGMENT_PARTITION;
                break;

            case SHIM_MICROTRIANGLE_FRAGMENT:
                
                partition = FRAGMENT_PARTITION;
                break;

            default:
                panic("ShaderFetch", "processShaderInput", "Unsupported shader input mode.");
                break;
        }
    }
    else
    {
        /*  Select unique partition.  */
        partition = PRIMARY_PARTITION;
    }

    /*  Get a new thread for the new input.  */
    newThread = getFreeThread(partition);

    /*  Check if there is a free thread.  */
    /*  This shouldn't happen.  Command Processor shouldn't know
        that the Shader has no empty buffers/threads.  */
    GPU_ASSERT(
        if (newThread == -1)
            panic("ShaderFetch", "clock", "No free buffer/threads.");
    )

    /*  Reset the state of the thread.  */
    shEmul.resetShaderState(newThread);

    //  Load the shader input into the input register bank according to the shader input type
    switch(input->getInputMode())
    {
        case SHIM_VERTEX:
            //  Load the input attributes for the shader thread element in the shader emulator.
            shEmul.loadShaderState(newThread, IN, input->getAttributes(), 0, MAX_VERTEX_ATTRIBUTES);
            break;

        case SHIM_TRIANGLE:
            //  Load the input attributes for the shader thread element in the shader emulator.
            shEmul.loadShaderState(newThread, IN, input->getAttributes(), 0, MAX_TRIANGLE_ATTRIBUTES);
            break;

        case SHIM_FRAGMENT:
            //  Load the input attributes for the shader thread element in the shader emulator.
            shEmul.loadShaderState(newThread, IN, input->getAttributes(), 0, MAX_FRAGMENT_ATTRIBUTES);
            break;

        case SHIM_MICROTRIANGLE_FRAGMENT:
            //  Load the input attributes for the shader thread element in the shader emulator.
            shEmul.loadShaderState(newThread, IN, input->getAttributes(), 0, MAX_MICROFRAGMENT_ATTRIBUTES);
            break;

        default:
            panic("ShaderFetch", "processShaderInput", "Unsupported shader input mode.");
            break;
    }

    /*  Check shader model.  */
    if (unifiedModel)
    {
        /*  Set initial PC.  Add partition offset.  */
        threadTable[newThread].PC = initPC[partition];
    }
    else
    {
        /*  Set initial PC.  */
        threadTable[newThread].PC = initPC[PRIMARY_PARTITION];
    }

    /*  Set thread PC in the shader emulator.  */
    shEmul.setThreadPC(newThread, threadTable[newThread].PC);

    /*  Reset thread instruction counter.  */
    threadTable[newThread].instructionCount = 0;

    /*  Set thread vertex.  */
    threadTable[newThread].shInput = input;

//if (cycle > 2500000)
//printf("ShF(%p) %lld => new shader input for partition %d stored as thread %d\n",
//    this, cycle, partition, newThread);

    /*  If there is a runnable set the thread state as ready to execute.  */
    if (numFreeThreads >= numInputBuffers)
    {
        threadTable[newThread].ready = TRUE;

        /*  Update number of ready threads.  */
        numReadyThreads++;

        /*  Calculate the group for the thread.  */
        newGroup = newThread / threadGroup;

        /*  Update the number of ready threads in the thread group.  */
        readyThreads[newGroup]++;

        /*  Check wether thread windor or in order batch mode are enabled.  */
        if (threadWindow)
        {
            /******    THIS SECTION IS FOR THE OUT OF ORDER THREAD WINDOW MODE    ******/

            /*  Check if the whole thread group is ready.  */
            if ((readyThreads[newGroup] == threadGroup) && groupBlocked[newGroup])
            {
                /*  Check ready queue.  */
                GPU_ASSERT(
                    if (numReadyGroups == numGroups)
                        panic("ShaderFetch", "processShaderInput", "Ready group queue full.");
                )

                /*  Add thread group to the ready queue.  */
                readyGroups[nextFreeReady] = newGroup;

//printf("ShF (shInput) => adding ready group %d at %d\n", newGroup, nextFreeReady);

                /*  Set group as stored in the ready queue.  */
                groupBlocked[newGroup] = false;

                /*  Update pointer to the next free entry.  */
                nextFreeReady = GPU_MOD(nextFreeReady + 1, 2 * numGroups);

                /*  Update number of ready groups.  */
                numReadyGroups++;
            }
        }
        else
        {
            /******    THIS SECTION IS FOR THE IN ORDER BATCH QUEUE MODE    ******/

            /*  Check if the whole thread group is ready.  */
            if (readyThreads[newGroup] == threadGroup)
            {
                /*  Check that the batch queue isn't already full.  */
                GPU_ASSERT(
                    if (lastInBatch[partition][loadBatch[partition]] == numGroups)
                        panic("ShaderFetch", "processShaderInput", "Batch group queue is full.");
                )

                /*  Check that the batch is not closed.  */
                GPU_ASSERT(
                    if ((fetchBatch[partition] == loadBatch[partition]) && closedBatch[partition][loadBatch[partition]])
                    {
                        sprintf(buff, "Partition %d load batch %d is already closed | nextFreeInBatch %d.", partition, loadBatch[partition], lastInBatch[partition][loadBatch[partition]]);
                        panic("ShaderFetch", "processShaderInput", buff);
                    }
                )

//if (cycle > 2500000)
//printf("ShF(%p) %lld => adding new group %d into partition %d batch %d position %d | lastInBatch(fetch) %d | nextFetchInBatch %d\n",
//    this, cycle, newGroup, partition, loadBatch[partition], lastInBatch[partition][loadBatch[partition]], lastInBatch[partition][fetchBatch[partition]], nextFetchInBatch[partition]);

                /*  Add the new group to the batch queue.  */
                batchGroupQueue[partition][loadBatch[partition]][lastInBatch[partition][loadBatch[partition]]] = newGroup;

                /*  Update pointer to the next free entry in the batch queue.  */
                lastInBatch[partition][loadBatch[partition]]++;

                /*  Update last cycle a group was loaded into a partition batch.  */
                lastLoadCycle[partition] = cycle;

                /*  Check the whole group for an input marked as last one.  */
                for(i = 0, lastInput = false; i < threadGroup; i++)
                    lastInput |= threadTable[newGroup * threadGroup + i].shInput->isLast();

                /*  Check if the batch must be closed.  */
               if ((lastInput) || (lastInBatch[partition][loadBatch[partition]] == numGroups))
                {
//if (cycle > 2500000)
//printf("ShF(%p) %lld => closing (last in shInput) partition %d batch %d\n",
//    this, cycle, partition, loadBatch[partition]);

                    closedBatch[partition][loadBatch[partition]] = true;

                    /*  Only start loading the next batch if it's not being used (not fetch).  */
                    if (loadBatch[partition] == fetchBatch[partition])
                    {
                        /*  Start loading inputs in the next batch.  */
                        loadBatch[partition] = (loadBatch[partition] + 1) & 0x01;
                    }
                }
            }
        }

//printf("ShF => (shInput) numReadyThreads %d\n", numReadyThreads);

        /*  This buffer is going to start execution now, so skip as a  filled buffer.  */
        firstFilledBuffer = GPU_MOD(firstFilledBuffer + 1, numBuffers);
    }

    /*  Update statistics.  */
    inputs->inc();
    inputInAttribs->inc(activeInputs[partition]);
    inputRegisters->inc(threadResources[partition]);
    if (unifiedModel)
    {
        inputOutAttribs->inc(activeOutputs[partition]);
    }
    else
    {
        inputOutAttribs->inc(GPU_MAX(u32bit(1), activeOutputs[PRIMARY_PARTITION]));
    }

    GPU_DEBUG_BOX(
        if (unifiedModel)
            printf("ShaderFetch >> New Shader Input Thread = %d for partition %s\n", newThread,
                partition==VERTEX_PARTITION?"VERTEX":(partition==FRAGMENT_PARTITION)?"FRAGMENT":"TRIANGLE");
        else
            printf("ShaderFetch >> New Shader Input Thread = %d\n", newThread);
    )
}

/*  Processes a control command from Decode stage.  */
void ShaderFetch::processDecodeCommand(ShaderDecodeCommand *decodeCommand)
{
    u32bit numThread;
    u32bit group;
    u32bit pc;
    u32bit i;

    numThread = decodeCommand->getNumThread();
    pc = decodeCommand->getNewPC();

    /*  Check the thread number is correct.  */
    GPU_ASSERT(
        if (numThread > numBuffers)
            panic("ShaderFetch", "processDecodeCommand", "Illegal thread number.");
    )

    switch(decodeCommand->getCommandType())
    {
        case UNBLOCK_THREAD:

            GPU_DEBUG_BOX(
                printf("ShaderFetch(%p) => UNBLOCK_THREAD Thread = %d PC = %x\n",
                    this, numThread, pc);
            )

            /*  Unblock a blocked thread.  */
            threadTable[numThread].ready = TRUE;
            threadTable[numThread].blocked = false;

            /*  Update number of ready threads.  */
            numReadyThreads++;

            /*  Calculate the group for the thread.  */
            group = numThread / threadGroup;

            /*  Update the number of ready threads in the thread group.  */
            readyThreads[group]++;

            /*  Check if the whole thread group is ready.  */
            if ((readyThreads[group] == threadGroup) && groupBlocked[group])
            {
                /*  Check ready queue.  */
                GPU_ASSERT(
                    if (numReadyGroups == numGroups)
                        panic("ShaderFetch", "processDecodeCommand", "Ready group queue full.");
                )

                /*  Set group as stored in the ready queue.  */
                groupBlocked[group] = false;

                /*  Add thread group to the ready queue.  */
                readyGroups[nextFreeReady] = group;

//printf("ShF (unblock) => adding ready group %d at %d\n", group, nextFreeReady);

                /*  Update pointer to the next free entry.  */
                nextFreeReady = GPU_MOD(nextFreeReady + 1, 2 * numGroups);

                /*  Update number of ready groups.  */
                numReadyGroups++;
            }

//printf("ShF => (unblock) numReadyThreads %d\n", numReadyThreads);

            /*  Update statistics.  */
            unblocks->inc();

            break;

        case BLOCK_THREAD:

            GPU_DEBUG_BOX(
                printf("ShaderFetch(%p) => BLOCK_THREAD Thread = %d PC = %x\n", this, numThread, pc);
            )

            /*  Block a thread.  */
            threadTable[numThread].ready = FALSE;
            threadTable[numThread].blocked = true;

            /*  Set PC for blocked thread: instruction after the one that produced the blockage.  */
            threadTable[numThread].PC = pc + 1;

            /*  Update number of ready threads.  */
            numReadyThreads--;

            /*  Calculate the group for the thread.  */
            group = numThread / threadGroup;

            /*  Update the number of ready threads in the thread group.  */
            readyThreads[group]--;

//printf("ShF => (block) numReadyThreads %d\n", numReadyThreads);

            /*  Update statistics.  */
            blocks->inc();


            break;

        case END_THREAD:

            /*  Finish the thread.  Mark as finished.  Add to finished list.  */
            finishThread(numThread);

            GPU_DEBUG_BOX(
                printf("ShaderFetch(%p) => END_THREAD Thread = %d PC = %x\n", this, numThread, pc);
            )

            break;


        case REPEAT_LAST:

            GPU_DEBUG_BOX(
                printf("ShaderFetch(%p) => REPEAT_LAST Thread = %d PC = %x\n", this, numThread, pc);
            )

            /*  Check lock step mode.  */
            if (lockStep)
            {
                /*  Change the state of all the threads in the group.  */
                for(i = 0; i < threadGroup; i++)
                {
                    /*  Fetch again the last instruction fetched for the thread.  */
                    threadTable[GPU_MOD(numThread + i, numBuffers)].PC = pc;

                    /*  Mark that next instruction will be a repeated one.  */
                    threadTable[GPU_MOD(numThread + i, numBuffers)].repeat = true;

                    /*  Set thread PC in the shader emulator.  */
                    shEmul.setThreadPC(GPU_MOD(numThread + i, numBuffers), pc);

                    /*  Update statistics.  */
                    reFetched->inc();
                }
            }
            else
            {
                /*  Fetch again the last instruction fetched for the thread.  */
                threadTable[numThread].PC = pc;

                /*  Mark that next instruction will be a repeated one.  */
                threadTable[numThread].repeat = true;

                /*  Set thread PC in the shader emulator.  */
                shEmul.setThreadPC(numThread, threadTable[numThread].PC);

                /*  Update statistics.  */
                reFetched->inc();
            }


            break;

        case NEW_PC:

            /*  Change thread PC.  */
            threadTable[numThread].PC = pc;

            GPU_DEBUG_BOX(
                printf("ShaderFetch => NEW_PC Thread = %d PC = %x\n", numThread, pc);
            )

            break;

        case ZEXPORT_THREAD:

            GPU_DEBUG_BOX(
                printf("ShaderFetch => ZEXPORT_THREAD Thread = %d PC = %x\n", numThread, pc);
            )

            /*  Check if the thread was ready.  */
            GPU_ASSERT(
                if (!threadTable[numThread].ready)
                    panic("ShaderFetch", "processDecodeCommand", "Thread wasn't in runnable (ready) state.");
            )

            /*  Set thread's Z exported state.  */
            threadTable[numThread].zexported = true;

            /*  Increment pending z exports.  */
            pendingZExports++;

            break;

        default:
            panic("ShaderFetch", "clock", "Unknown decode command type.");
            break;
    }

    /*  Delete NewPC command from decoder.  */
    delete decodeCommand;
}

/*  Fetches a shader instruction.  */
void ShaderFetch::fetchInstruction(u64bit cycle, u32bit threadID, ShaderExecInstruction *&shExecInstr)
{
    ShaderInstruction::ShaderInstructionDecoded *shInstrDec;
    ShaderInstruction *shInstr;
    bool fakeFetch;
    char buffer[256];
    u32bit partition;

    /*  Check shader model.  */
    if (unifiedModel)
    {
        /*  Select shader partition given shader input type.  */
        switch(threadTable[threadID].shInput->getInputMode())
        {
            case SHIM_VERTEX:

                partition = VERTEX_PARTITION;
                break;

            case SHIM_TRIANGLE:

                partition = TRIANGLE_PARTITION;
                break;

            case SHIM_FRAGMENT:

                partition = FRAGMENT_PARTITION;
                break;

            case SHIM_MICROTRIANGLE_FRAGMENT:

                partition = FRAGMENT_PARTITION;
                break;

            default:

                panic("ShaderFetch", "fetchInstruction", "Unsupported shader input mode.");
                break;
        }

    }
    else
    {
        /*  Set unique partition.  */
        partition = PRIMARY_PARTITION;
    }

    /*  Fetch instruction from ShaderEmulator.  */
    //shInstrDec = shEmul.fetchShaderInstruction(threadID, threadTable[threadID].PC);
    shInstrDec = shEmul.fetchShaderInstructionLight(threadID, threadTable[threadID].PC, partition);
    shInstr = shInstrDec->getShaderInstruction();

    /*  Check instruciton.  */
    GPU_ASSERT(
        if (shInstr == NULL)
        {
            sprintf(buffer, "Fetching instruction from unloaded address.  Thread %d pc %d\n",
                threadID, threadTable[threadID].PC);
            panic("ShaderFetch", "fetchInstruction", buffer);
        }
    )

    /*  Check if SIMD + scalar fetch is enabled.  */
    if (scalarALU)
    {
        /*  Check if a SIMD instruction was already fetched for the thread.  */
        if (fetchedSIMD && (!shInstr->isScalar()))
        {
            GPU_DEBUG_BOX(
                printf("ShaderFetch => Ignoring second SIMD instruction fetch.\n");
            )

            /*  Create a fake dynamic instruction.  */
            shExecInstr = new ShaderExecInstruction(*shInstrDec, threadTable[threadID].PC, cycle,
                threadTable[threadID].repeat, true);

            /*  Copy cookies from shader input and add a new cookie.  */
            shExecInstr->copyParentCookies(*threadTable[threadID].shInput);
            shExecInstr->addCookie();

            return;
        }

        /*  Check if a scalar instruction was already fetched for the thread.  */
        if (fetchedScalar && shInstr->isScalar())
        {
            GPU_DEBUG_BOX(
                printf("ShaderFetch => Ignoring second scalar instruction fetch.\n");
            )

            /*  Create a fake dynamic instruction.  */
            shExecInstr = new ShaderExecInstruction(*shInstrDec, threadTable[threadID].PC, cycle,
                threadTable[threadID].repeat, true);

            /*  Copy cookies from shader input and add a new cookie.  */
            shExecInstr->copyParentCookies(*threadTable[threadID].shInput);
            shExecInstr->addCookie();

            return;
        }

        /*  Update fetched instruction flags for the current thread.  */
        fetchedSIMD = fetchedSIMD || !shInstr->isScalar();
        fetchedScalar = fetchedScalar || shInstr->isScalar();
    }

    /*  Create a new dynamic instruction.  */
    shExecInstr = new ShaderExecInstruction(*shInstrDec, threadTable[threadID].PC, cycle,
        threadTable[threadID].repeat, false);

    /*  Copy cookies from shader input and add a new cookie.  */
    shExecInstr->copyParentCookies(*threadTable[threadID].shInput);
    shExecInstr->addCookie();

    /*  Update instruction count.  */
    threadTable[threadID].instructionCount++;

    /*  Check if the thread has executed too many instructions.  */
    if (threadTable[threadID].instructionCount > MAXSHADERTHREADINSTRUCTIONS)
    {
        /*  Finish thread if has reached the execution cycle limit.  */
        finishThread(threadID);
    }

    /*  Check if thread is still ready (may become blocked while the thread group is waiting for fetch).   */
    if (threadTable[threadID].ready)
    {
        /*  Update thread PC.  */
        threadTable[threadID].PC++;
    }

    /*  Reset repeated instruction flag.  */
    threadTable[threadID].repeat = false;

    /*  Update statistics.  */
    fetched->inc();
}

/*  Fetch a shader instruction and sends it to decode stage.  */
void ShaderFetch::fetchInstruction(u64bit cycle, u32bit threadID)
{
    ShaderExecInstruction *shExecInstr;

    /*  Fetch an instruction for the thread.  */
    fetchInstruction(cycle, threadID, shExecInstr);

    /*  Send the instruction to Shader Decode Execute.  */
    sendInstruction(cycle, threadID, shExecInstr);
}

/*  Fetches all the instruction for a thread group.  */
void ShaderFetch::fetchInstructionGroup(u64bit cycle, u32bit threadID)
{
    ShaderExecInstruction *shExecInstr;
    u32bit i;
    u32bit j;

    /*  Fetch from all the threads in a group.  */
    for(i = 0; i < threadGroup; i++)
    {
        /*  Reset fetched instructions flags for current thread.  */
        fetchedSIMD = fetchedScalar = false;

        /*  Fetch all the instructions required for a thread.  */
        for(j = 0; j < instrCycle; j++)
        {
            /*  Fetch and store instruction for the thread.  */
            fetchInstruction(cycle, GPU_MOD(threadID + i, numBuffers), fetchGroup[i][j]);
        }

        /*  A group of shader instructions must share a cookie.  */
        //for(j = 1; j < instrCycle; j++)
        //{
        //    fetchGroup[i][j]->copyParentCookies(*fetchGroup[i][0]);
        //    fetchGroup[i][j]->removeCookie();
        //    fetchGroup[i][j]->removeCookie();
        //    fetchGroup[i][j]->addCookie();
        //}
        //fetchGroup[i][0]->removeCookie();
        //fetchGroup[i][0]->removeCookie();
        //fetchGroup[i][0]->addCookie();
    }
}

/*  Sends an instruction to the shader decoder.  */
void ShaderFetch::sendInstruction(u64bit cycle, u32bit threadID, ShaderExecInstruction *shExecInstr)
{
    char dis[256];

    /*  Send instruction to decode/execute box.  */
    instructionSignal->write(cycle, shExecInstr);

    GPU_DEBUG_BOX(
        shExecInstr->getShaderInstruction().getShaderInstruction()->disassemble(&dis[0]);

        printf("ShaderFetch (%p) => Instruction Fetched Thread = %d PC = %x : %s\n",
            this, threadID, shExecInstr->getPC(), dis);
    )
}


/*  Send shader output to consumer.  */
void ShaderFetch::sendOutput(u64bit cycle, u32bit outputThread)
{
    ShaderInput *shOutput;
    u32bit i;
    bool killed;

    /*  Start output transmission.  */

    /*  Read Output data from ShaderEmulator.  */
    shOutput = threadTable[outputThread].shInput;

    u32bit partition;
    
    switch(shOutput->getInputMode())
    {
        case SHIM_VERTEX:
            partition = VERTEX_PARTITION;
            break;
        case SHIM_TRIANGLE:
            partition = TRIANGLE_PARTITION;
            break;
        case SHIM_FRAGMENT:
            partition = FRAGMENT_PARTITION;
            break;
        case SHIM_MICROTRIANGLE_FRAGMENT:
            partition = FRAGMENT_PARTITION;
            break;
        default:
            panic("ShaderFetch", "sendOutput", "Undefined shader input mode.");
            break;
    }
    
    //  Calculate the number of cycles for output transmission (only vertices).  */
    transCycles = (u32bit) ceil(((double) activeOutputs[partition]) * OUTPUT_TRANSMISSION_LATENCY);

    /*  Read the result output registers and write them into the shader output array.  */
    shEmul.readShaderState(outputThread, OUT, shOutput->getAttributes());

    /*  Check if the only sample for the shader thread was aborted/killed.  */
    if (shEmul.threadKill(outputThread, 0))
        shOutput->setKilled();

    /*  Send data to consumer.  */
    outputSignal->write(cycle, shOutput, transCycles + OUTPUT_DELAY_LATENCY);

    GPU_DEBUG_BOX(
        printf("ShaderFetch => Output sent\n");
        printf("vo[0] = {%f %f %f %f}\n",
            shOutput->getAttributes()[0][0],
            shOutput->getAttributes()[0][1],
            shOutput->getAttributes()[0][2],
            shOutput->getAttributes()[0][3]);

        printf("vo[1] = {%f %f %f %f}\n",
            shOutput->getAttributes()[1][0],
            shOutput->getAttributes()[1][1],
            shOutput->getAttributes()[1][2],
            shOutput->getAttributes()[1][3]);
    )

    /*  Update statistics.  */
    outputs->inc();
}

//  Compute the maximum number of resources required for a new thread.
u32bit ShaderFetch::computeMaxThreadResources()
{
    u32bit maxResources = 1;
    
    //  Check shader partition.
    if (unifiedModel)
    {
        for(u32bit target = 0; target < 3; target++)
            maxResources = GPU_MAX(maxResources, threadResources[target]);
            
        for(u32bit target = 0; target < 3; target++)
            maxResources = GPU_MAX(maxResources, activeInputs[target]);

        for(u32bit target = 0; target < 3; target++)
            maxResources = GPU_MAX(maxResources, activeOutputs[target]);        
    }
    else
    {
        maxResources = GPU_MAX(threadResources[PRIMARY_PARTITION], GPU_MAX(GPU_MAX(activeInputs[PRIMARY_PARTITION], threadResources[PRIMARY_PARTITION]),
            GPU_MAX(u32bit(1), activeOutputs[PRIMARY_PARTITION])));
    }

    return maxResources;
}

void ShaderFetch::getState(string &stateString)
{
    stringstream stateStream;

    stateStream.clear();

    stateStream << " state = ";

    switch(shState)
    {
        case SH_EMPTY:
            stateStream << "SH_EMPTY";
            break;
        case SH_READY:
            stateStream << "SH_READY";
            break;
        case SH_BUSY:
            stateStream << "SH_BUSY";
            break;
        default:
            stateStream << "undefined";
            break;
    }

    stateStream << " | Ready Thread Group = " << readyThreadGroup;
    stateStream << " | Next Thread = " << nextThread;

    stateString.assign(stateStream.str());
}



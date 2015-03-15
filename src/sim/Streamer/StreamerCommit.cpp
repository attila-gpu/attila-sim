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
 * $RCSfile: StreamerCommit.cpp,v $
 * $Revision: 1.18 $
 * $Author: vmoya $
 * $Date: 2006-12-12 17:41:18 $
 *
 * Streamer Commit box implementation file.
 *
 */


/**
 *
 *  @file StreamerCommit.cpp
 *
 *  This file implements the Streamer Commit box.
 *
 *
 */

#include "StreamerCommit.h"
#include "ConsumerStateInfo.h"
#include "StreamerStateInfo.h"
#include "PrimitiveAssemblyRequest.h"
#include "PrimitiveAssemblyInput.h"
#include "support.h"
#include "GPUMath.h"

using namespace gpu3d;

/*  Streamer Commit constructor.  */
StreamerCommit::StreamerCommit(u32bit idxCycle, u32bit oFIFOSize, u32bit omSize, u32bit nShaders,
    u32bit maxOutLat, u32bit vertCycle, u32bit attrCycle, u32bit outLat, char **shPrefixArray,
    char *name, Box *parent) :

    indicesCycle(idxCycle), outputFIFOSize(oFIFOSize), outputMemorySize(omSize), numShaders(nShaders),
    shMaxOutLat(maxOutLat), verticesCycle(vertCycle), attributesCycle(attrCycle), outputBusLat(outLat),
    Box(name, parent)

{
    u32bit i;
    DynamicObject *defaultStreamerState[1];
    DynamicObject *defaultConsumerState[1];

   /*  Check shaders and shader signals prefixes.  */
    GPU_ASSERT(
        /*  There must at least 1 Shader.  */
        if (numShaders == 0)
            panic("StreamerCommit", "StreamerCommit", "Invalid number of Shaders (must be >0).");

        /*  The shader prefix pointer array must be not null.  */
        if (shPrefixArray == NULL)
            panic("StreamerCommit", "StreamerCommit", "The shader prefix array is null.");

        /*  Check if the prefix arrays point to null.  */
        for(i = 0; i < numShaders; i++)
        {
            if (shPrefixArray[i] == NULL)
                panic("StreamerCommit", "StreamerCommit", "Shader prefix array component is null.");
        }
    )

    /*  Allocate signal arrays for the shader signals.  */
    shConsumerSignal = new Signal*[numShaders];
    shOutputSignal = new Signal*[numShaders];

    /*  Create signals from/to the shaders.  */
    for(i = 0; i < numShaders; i++)
    {
        /*  Streamer state signals to the shaders.  */
        shConsumerSignal[i] = newOutputSignal("ConsumerState", 1, 1, shPrefixArray[i]);

        /*  Build initial signal data.  */
        defaultConsumerState[0] = new ConsumerStateInfo(CONS_READY);

        /*  Set default streamer state signal to the shaders.  */
        shConsumerSignal[i]->setData(defaultConsumerState);

        /*  Output signal from the shaders.  */
        shOutputSignal[i] = newInputSignal("ShaderOutput", 1, shMaxOutLat, shPrefixArray[i]);
    }

    /*  Create commad signal from the main Streamer box.  */
    streamerCommitCommand = newInputSignal("StreamerCommitCommand", 1, 1, NULL);

    /*  Create state signal to the Streamer main box.  */
    streamerCommitState = newOutputSignal("StreamerCommitState", 1, 1, NULL);

    /*  Build initial signal state.  */
    defaultStreamerState[0] = new StreamerStateInfo(ST_RESET);

    /*  Set initial signal value.  */
    streamerCommitState->setData(defaultStreamerState);

    /*  Create the output signal to the Primitive Assembly unit.  */
    assemblyOutputSignal = newOutputSignal("PrimitiveAssemblyInput", verticesCycle,
        outputBusLat + MAX_VERTEX_ATTRIBUTES / attributesCycle , NULL);

    /*  Create new index signal from the Streamer Output Cache.  */
    streamerCommitNewIndex = newInputSignal("StreamerCommitNewIndex", indicesCycle, 1, NULL);

    /*  Create deallocation signal to the Streamer Fetch.  */
    streamerCommitDeAlloc = newOutputSignal("StreamerCommitDeAlloc", 4 * verticesCycle, 1, NULL);

    /*  Create deallocation signal to the Streamer Output Cache.  */
    streamerCommitDeAllocOC = newOutputSignal("StreamerCommitDeAllocOC", 2 * verticesCycle, 1);

    /*  Create the request signal from the Primitive Assembly unit.  */
    assemblyRequestSignal = newInputSignal("PrimitiveAssemblyRequest", 1, 1, NULL);

    /*  Allocate memory for the output memory.  */
    outputMemory = new QuadFloat*[outputMemorySize];

    /*  Check memory allocation.  */
    GPU_ASSERT(
        if (outputMemory == NULL)
            panic("StreamerCommit", "StreamerCommit", "Error allocating memory for the output memory.");
    )

    /*  Allocate memory for the output memory lines.  */
    for(i = 0; i < outputMemorySize; i++)
    {
        outputMemory[i] = new QuadFloat[MAX_VERTEX_ATTRIBUTES];

        /*  Check memory allocation.  */
        GPU_ASSERT(
            if (outputMemory[i] == NULL)
                panic("StreamerCommit", "StreamerCommit", "Error allocating memory for the output memory lines.");
        )
    }

    /*  Allocate memory for the last use table.  */
    lastUseTable = new u32bit[outputMemorySize];

    /*  Check memory allocation.  */
    GPU_ASSERT(
        if (lastUseTable == NULL)
            panic("StreamerCommit", "StreamerCommit", "Error allocating memory for the last use table.");
    )

    /*  Allocate memory for the Output FIFO.  */
    outputFIFO = new OFIFO[outputFIFOSize];

    /*  Check memory allocation.  */
    GPU_ASSERT(
        if (outputFIFO == NULL)
            panic("StreamerCommit", "StreamerCommit", "Error allocating memory for the output FIFO.");
    )

    /*  Allocate array of counter storing the remaining cycles for transmiting vertices to Primitive Assembly.  */
    transCycles = new u32bit[verticesCycle];

    //  Check allocation.
    GPU_ASSERT(
        if (transCycles == NULL)
            panic("StreamerCommit", "StreamerCommit", "Error allocating the vertex transmission cycle counters.");
    )

    /*  Allocate the array storing the deallocated indices of the last cycle that in turn deallocated its corresponding memory lines.  */
    outputMemoryDeAlloc = new u32bit*[verticesCycle];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (outputMemoryDeAlloc == NULL)
            panic("StreamerCommit", "StreamerCommit", "Error allocating the array of the deallocated indices.");
    )

    for(i = 0; i < verticesCycle; i++)
    {
        outputMemoryDeAlloc[i] = new u32bit[3];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (outputMemoryDeAlloc[i] == NULL)
                panic("StreamerCommit", "StreamerCommit", "Error allocating the array of the deallocated indices.");
        )
    }

    /*  Allocate the table of hits into the array of deallocated output memory lines.  */
    outputMemoryDeAllocHit = new bool[verticesCycle];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (outputMemoryDeAllocHit == NULL)
            panic("StreamerCommit", "StreamerCommit", "Error allocating the table of hits in deallocated output memory lines.");
    )

    /*  Allocate the array of streamer control commands of deallocated indices.  */
    outputMemoryDeAllocCCom = new StreamerControlCommand*[verticesCycle];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (outputMemoryDeAllocCCom == NULL)
            panic("StreamerCommit", "StreamerCommit", "Error allocating the array of streamer control commands of deallocated indices.");
    )

    /*  Initialize the array of streamer control commands of deallocated indices.  */
    for (i = 0; i < verticesCycle; i++)
    {
        outputMemoryDeAllocCCom[i] = NULL;
    }


    /*  Set default vertex output attributes configuration.  */
    for (i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
        outputAttribute[i] = FALSE;

    /*  Create statistics.  */
    indices = &getSM().getNumericStatistic("Indices", u32bit(0), "StreamerCommit", "StC");
    shOutputs = &getSM().getNumericStatistic("ShaderOutputs", u32bit(0), "StreamerCommit", "StC");
    paOutputs = &getSM().getNumericStatistic("Outputs", u32bit(0), "StreamerCommit", "StC");
    paRequests = &getSM().getNumericStatistic("Requests", u32bit(0), "StreamerCommit", "StC");
    outAttributes = &getSM().getNumericStatistic("OutputAttributes", u32bit(0), "StreamerCommit", "StC");
    avgVtxShadeLat = &getSM().getNumericStatistic("AvgShadeLatency", u32bit(0), "StreamerCommit", "StC");

    /*  Reset active vertex output attributes counter.  */
    activeOutputs = 0;

    /*  Reset vertex transmission cycles counter.  */
    for(i = 0; i < verticesCycle; i++)
        transCycles[i] = 0;

    //  Set validation mode flag to disabed.
    validationMode = false;
    
    //  Reset next log pointers.
    nextReadLog = 3;
    nextWriteLog = 0;

    /*  Create dummy last streamer command.  */
    lastStreamCom = new StreamerCommand(STCOM_RESET);

    /*  Set initial state to reset.  */
    state = ST_RESET;
}


/*  Streamer Commit simulation rutine.  */
void StreamerCommit::clock(u64bit cycle)
{
    StreamerCommand *streamCom;
    StreamerControlCommand *streamCCom;
    StreamerControlCommand *streamAuxCCom;
    PrimitiveAssemblyRequest *assemblyRequest;
    QuadFloat *oaux;
    ShaderInput *output;
    PrimitiveAssemblyInput *assemblyInput;
    u32bit oMemLine;
    u32bit i;
    u32bit j;
    u32bit nextVertexBus;
    bool addedToDeAllocTable;

    GPU_DEBUG_BOX( printf("StreamerCommit => Clock %lld\n", cycle);)

    /*  Get state from the Primitive Assembly..  */
    while (assemblyRequestSignal->read(cycle, (DynamicObject *&) assemblyRequest))
    {
        /*  Update primitive assembly requested vertex counter.  */
        requestVertex += assemblyRequest->getRequest();

        GPU_DEBUG_BOX(
            printf("StreamerCommit => Vertex request from Primitive Assembly.  Requested %d\n",
                assemblyRequest->getRequest());
        )

        /*  Update statistics.  */
        paRequests->inc(assemblyRequest->getRequest());

        /*  Delete Primitive Assembly state signal.  */
        delete assemblyRequest;
    }

/*if ((cycle > 5024880) && (GPU_MOD(cycle, 1000) == 0))
{
printf("StC %lld => state ", cycle);
switch(state)
{
    case ST_RESET: printf("RESET\n"); break;
    case ST_READY: printf("READY\n"); break;
    case ST_STREAMING: printf("STREAMING\n"); break;
    case ST_FINISHED: printf("FINISHED\n"); break;

}
printf("StC %lld => requestVertex %d freeOFIFOEntries %d sentOutputs %d\n",
cycle, requestVertex, freeOFIFOEntries, sentOutputs);
printf("StC %lld => transCycles : ");
for(i=0; i < verticesCycle; i++)
    printf(" %d ", transCycles[i]);
printf("\n");
printf("StC => nextOutput %d shaded? %s index %d omLine %d stCCom %p\n",
nextOutput, outputFIFO[nextOutput].isShaded?"true":"false", outputFIFO[nextOutput].index, outputFIFO[nextOutput].omLine,
outputFIFO[nextOutput].stCCom);
}*/


    /*  Simulate current cycle.  */
    switch(state)
    {
        case ST_RESET:
            /*  Reset state.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => RESET state.\n"); )

            //  Initialize registers.
            streamCount = 0;
            streamInstances = 1;
            for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                outputAttribute[a] = false;
            activeOutputs = 0;
            
            /*  Initialize the number of deallocs for current cycle.  */
            outputMemoryDeAllocs = 0;

            //  WARNING: REMOVE THOSE STUPID FLAGS AND RELATED CODE AS SOON AS POSSIBLE.
            //  Reset the flags on reset.
            firstOutput = false;
            lastOutputSent = false;

            /*  Change state to ready.  */
            state = ST_READY;

            break;

        case ST_READY:
            /*  Ready state.  Wait for a start command from
                the streamer main box.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => READY state.\n"); )

            /*  Get commands from the Streamer main box.  */
            if (streamerCommitCommand->read(cycle, (DynamicObject *&) streamCom))
            {
                /*  Process streamer command.  */
                processStreamerCommand(streamCom);
            }

            break;

        case ST_STREAMING:
            /*  Streaming state.  Receive New Index from the Output Cache.
                Receive shaded outputs from the Shaders.  Send the outputs
                to the Rasterizer.  Deallocate output FIFO and output
                memory entries.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => STREAMING state.\n"); )

            /*  Read new index command from the Streamer Output Cache.  */
            while (streamerCommitNewIndex->read(cycle, (DynamicObject *&) streamCCom))
            {
                GPU_ASSERT(
                    if (freeOFIFOEntries == 0)
                        panic("StreamerCommit", "clock", "No free output FIFO entries.");
                )

                /*  Look up table of deallocated output memory lines of the previous cycle.  */
                for (i = 0; i < outputMemoryDeAllocs; i++)
                {
                    if ((outputMemoryDeAlloc[i][0] == streamCCom->getIndex()) &&
                        (outputMemoryDeAlloc[i][1] == streamCCom->getInstanceIndex()))
                    {
                        /*  The OM line of the new index was deallocated right on the previous cycle.  */
                        outputMemoryDeAllocHit[i] = true;
                    }
                }

                /*  Add index to the output FIFO.  */
                outputFIFO[streamCCom->getOFIFOEntry()].index = streamCCom->getIndex();
                outputFIFO[streamCCom->getOFIFOEntry()].instance = streamCCom->getInstanceIndex();
                outputFIFO[streamCCom->getOFIFOEntry()].omLine = streamCCom->getOMLine();
                outputFIFO[streamCCom->getOFIFOEntry()].stCCom = streamCCom;
                outputFIFO[streamCCom->getOFIFOEntry()].isShaded = streamCCom->isAHit() && (!streamCCom->isLast());
                outputFIFO[streamCCom->getOFIFOEntry()].startCycle = cycle;

                /*  Update free output FIFO entries counter.  */
                freeOFIFOEntries--;

                /*  Check if the output memory line is already reserved.  */
                if (lastUseTable[streamCCom->getOMLine()] != outputFIFOSize)
                {
                    /*  This output doesn't use an additional output memory line, so send
                        an already allocated command to Streamer Fetch.  */

                    /*  Create already allocated command.  */
                    streamAuxCCom = new StreamerControlCommand(STRC_OM_ALREADY_ALLOC, streamCCom->getOMLine());

                    /*  Copy cookies from the original new index command.  */
                    streamAuxCCom->copyParentCookies(*streamCCom);

                    /*  Add a new cookie.  */
                    streamAuxCCom->addCookie();

                    /*  Send already allocated command to the Streamer Fetch.  */
                    streamerCommitDeAlloc->write(cycle, streamAuxCCom);
                }

                /*  Update last use table.  */
                lastUseTable[streamCCom->getOMLine()] = streamCCom->getOFIFOEntry();

                /*  Set first output flag.  */
                firstOutput = (sentOutputs == 0);

                /*  Update statistics.  */
                indices->inc();
            }

            /*  Process deallocated OM lines on the previous cycle.  */
            for (i = 0; i < outputMemoryDeAllocs; i++)
            {
                /*  Check if any new index this cycle matched the indices deallocated in the previous cycle.  */
                if (!outputMemoryDeAllocHit[i])
                {
                    /*  The corresponding output memory line can now be safely deallocated (confirm deallocation).  */
                    oMemLine = outputMemoryDeAlloc[i][2];

                    /*  Create deallocation confirmation command.  */
                    streamCCom = new StreamerControlCommand(STRC_DEALLOC_OM_CONFIRM, oMemLine);

                    /*  Copy cookies from the original new index command.  */
                    streamCCom->copyParentCookies(*outputMemoryDeAllocCCom[i]);

                    /*  Add a new cookie.  */
                    streamCCom->addCookie();

                    /*  Send output memory deallocation to the Streamer Fetch.  */
                    streamerCommitDeAlloc->write(cycle, streamCCom);

                    /*  Create deallocation command.  */
                    streamCCom = new StreamerControlCommand(STRC_DEALLOC_OM_CONFIRM, oMemLine);

                    /*  Copy cookies from the original new index command.  */
                    streamCCom->copyParentCookies(*outputMemoryDeAllocCCom[i]);

                    /*  Add a new cookie.  */
                    streamCCom->addCookie();

                    /*  Send output memory deallocation to the Streamer Output Cache.  */
                    streamerCommitDeAllocOC->write(cycle, streamCCom);

                    /*  Delete new index streamer control command of deallocated output memory line.  */
                    delete outputMemoryDeAllocCCom[i];
                }
                else
                {
                    /*  A new index this cycle matches with an index deallocated on the previous cycle.
                        The corresponding OM line is returned back to life and no dealloc confirmation is sent.
                        Additionally, Since this output will not use an additional output memory line, an already allocated 
                        streamer command is sent to Streamer Fetch.  */

                    /*  Create already allocated command.  */
                    streamCCom = new StreamerControlCommand(STRC_OM_ALREADY_ALLOC, outputMemoryDeAlloc[i][1]);

                    /*  Copy cookies from the original new index command.  */
                    streamCCom->copyParentCookies(*outputMemoryDeAllocCCom[i]);

                    /*  Add a new cookie.  */
                    streamCCom->addCookie();

                    /*  Send already allocated command to the Streamer Fetch.  */
                    streamerCommitDeAlloc->write(cycle, streamCCom);
                }
            }

            /*  Check if there is an output transmission to Primitive Assembly.  */
            for(i = 0; i < verticesCycle; i++)
            {
                if (transCycles[i] > 0)
                {
                    /*  Update remaining transmission cycles counter.  */
                    transCycles[i]--;
                }
            }

            /*  Search for a free vertex bus to Primitive Assembly.  */
            for(nextVertexBus = 0;(nextVertexBus < verticesCycle) && (transCycles[nextVertexBus] > 0); nextVertexBus++);

            /*  Initialize the counter of deallocated output lines for the current cycle.  */
            outputMemoryDeAllocs = 0;

            /*  Send outputs to the Primitive Assembly unit.  */
            for(i = 0; (i < verticesCycle) && (nextVertexBus < verticesCycle); i++)
            {
                /*  Check the next vertex in order in the output FIFO is already shaded and
                    there are no pending vertex transmission to Primitive Assembly.  */
                if (outputFIFO[nextOutput].isShaded && (transCycles[nextVertexBus] == 0))
                {
                    /*  Check if the rasterizer is ready to receive a vertex.  */
                    if (requestVertex > 0)
                    {
                        /*  Allocate memory for output.  */
                        oaux = new QuadFloat[MAX_VERTEX_ATTRIBUTES];

                        /*  Check memory allocation.  */
                        GPU_ASSERT(
                            if (oaux == NULL)
                                panic("StreamerCommit", "clock", "Primitive Assembly input buffer could not be allocated.");
                        )

                        /*  Get the output memory line where the output for the current index is stored.  */
                        oMemLine = outputFIFO[nextOutput].omLine;

                        /*  Copy the next output vertex to the buffer.  */
                        for(j = 0; j < MAX_VERTEX_ATTRIBUTES; j++)
                        {
                            oaux[j] = outputMemory[oMemLine][j];
                        }

                        /*  Calculate transmission cycle for the full vertex.  */
                        transCycles[nextVertexBus] = (u32bit) ceil(((f32bit) activeOutputs) / (f32bit) attributesCycle);

                        /*  Create a Primitive Assembly input object to carry the output.  */
                        assemblyInput = new PrimitiveAssemblyInput(outputFIFO[nextOutput].index, oaux);

                        /*  Copy cookies from the source new index streamer control command.  */
                        assemblyInput->copyParentCookies(*outputFIFO[nextOutput].stCCom);

                        /*  Send the next output to the rasterizer.  */
                        assemblyOutputSignal->write(cycle, assemblyInput, transCycles[nextVertexBus] + outputBusLat);

                        GPU_DEBUG_BOX(
                            printf("StreamerCommit => Sending output with index %d instance %d to Primitive Assembly.\n",
                                outputFIFO[nextOutput].index, outputFIFO[nextOutput].instance);
                            printf("v[0] = {%f, %f, %f, %f}\n",
                                oaux[0][0], oaux[0][1], oaux[0][2], oaux[0][3]);
                            printf("v[1] = {%f, %f, %f, %f}\n",
                                oaux[1][0], oaux[1][1], oaux[1][2], oaux[1][3]);
                        )

                        /*  Check in the last use table if the output memory line must be deallocated.  */
                        if(lastUseTable[oMemLine] == nextOutput)
                        {
                            /*  Set last use table entry to a non output FIFO entry identifier.  */
                            lastUseTable[oMemLine] = outputFIFOSize;

                            /*  Add deallocated output memory line into the table.  */
                            outputMemoryDeAlloc[outputMemoryDeAllocs][0] = outputFIFO[nextOutput].index;
                            outputMemoryDeAlloc[outputMemoryDeAllocs][1] = outputFIFO[nextOutput].instance;
                            outputMemoryDeAlloc[outputMemoryDeAllocs][2] = oMemLine;

                            /*  Initialize the table of hits of deallocated indices for the next cycle.  */
                            outputMemoryDeAllocHit[outputMemoryDeAllocs] = false;

                            /*  Add the streamer control command of the deallocated index to the table.  */
                            outputMemoryDeAllocCCom[outputMemoryDeAllocs] = outputFIFO[nextOutput].stCCom;

                            /*  Update the number of deallocated lines  */
                            outputMemoryDeAllocs++;

                            /*  This determines next if the streamer control command must be deleted.  */
                            addedToDeAllocTable = true;

                            /*  Create deallocation command.  */
                            streamCCom = new StreamerControlCommand(STRC_DEALLOC_OM, oMemLine);

                            /*  Copy cookies from the original new index command.  */
                            streamCCom->copyParentCookies(*outputFIFO[nextOutput].stCCom);

                            /*  Add a new cookie.  */
                            streamCCom->addCookie();

                            /*  Send output memory deallocation to the Streamer Fetch.  */
                            streamerCommitDeAlloc->write(cycle, streamCCom);

                            /*  Create deallocation command.  */
                            streamCCom = new StreamerControlCommand(STRC_DEALLOC_OM, oMemLine);

                            /*  Copy cookies from the original new index command.  */
                            streamCCom->copyParentCookies(*outputFIFO[nextOutput].stCCom);

                            /*  Add a new cookie.  */
                            streamCCom->addCookie();

                            /*  Send output memory deallocation to the Streamer Output Cache.  */
                            streamerCommitDeAllocOC->write(cycle, streamCCom);
                        }
                        else
                        {
                            /*  This determines next if the streamer control command must be deleted.  */
                            addedToDeAllocTable = false;
                        }

                        /*  Create deallocation command.  */
                        streamCCom = new StreamerControlCommand(STRC_DEALLOC_OFIFO, nextOutput);

                        /*  Copy cookies from the original new index command.  */
                        streamCCom->copyParentCookies(*outputFIFO[nextOutput].stCCom);

                        /*  Add a new cookie.  */
                        streamCCom->addCookie();

                        /*  Send output FIFO deallocation signal to the Streamer Fetch.  */
                        streamerCommitDeAlloc->write(cycle, streamCCom);

                        /*  Check if the index streamer control command is no longer needed.  */
                        if (!addedToDeAllocTable)
                        {
                            /*  Delete new index streamer control command for this index.  */
                            delete outputFIFO[nextOutput].stCCom;
                        }

                        /*  Set entry as not shaded.  */
                        outputFIFO[nextOutput].isShaded = FALSE;

                        /*  Update next output pointer.  */
                        nextOutput = GPU_MOD(nextOutput + 1, outputFIFOSize);

                        /*  Update free output FIFO entries counter.  */
                        freeOFIFOEntries++;

                        /*  Update requested vertex counter.  */
                        requestVertex--;

                        /*  Update sent outputs counter.  */
                        sentOutputs++;

                        /*  Update statistics.  */
                        paOutputs->inc();

                        /*  Check if this was the last output.  */
                        if (sentOutputs == (streamCount * streamInstances))
                        {
                            /*  Change state to streaming finished.  */
                            state = ST_FINISHED;

                            /*  Set last output flag.  */  
                            lastOutputSent = true;
                        }
                    }
                }

                /*  Search for a free vertex bus to Primitive Assembly.  */
                for(nextVertexBus = 0;(nextVertexBus < verticesCycle) && (transCycles[nextVertexBus] > 0); nextVertexBus++);
            }

            /*  Read outputs from the Shader.  */

            /*  Check if there is a vertex ouput from the shaders.  */
            for(i = 0; i < numShaders; i++)
            {
                /*  Check if there is an output from this shader.  */
                if (shOutputSignal[i]->read(cycle, (DynamicObject *&) output))
                {
                    /*  Mark output FIFO entry as shaded. */
                    outputFIFO[output->getEntry()].isShaded = TRUE;

                    /*  Compute shading latency.  */
                    outputFIFO[output->getEntry()].shadingLatency = (cycle - outputFIFO[output->getEntry()].startCycle);

                    /*  Update statistics.  */
                    avgVtxShadeLat->incavg(outputFIFO[output->getEntry()].shadingLatency);

                    /*  Check if killed vertex output (last output).  Discard output if it is the case.  */
                    if (!output->getKill())
                    {
                        /*  Get output memory line where to store the output.  */
                        oMemLine = outputFIFO[output->getEntry()].omLine;

                        /*  Copy output data to the output memory.  */
                        for(j = 0; j < MAX_VERTEX_ATTRIBUTES; j++)
                        {
                            /*  Copy output attribute to the output memory.  */
                            outputMemory[oMemLine][j] = output->getAttributes()[j];
                        }

                        /*  Clamp vertex attribute values.  */

                        /*  Clamp color (attribute 1) to [0.0, 1.0].  */
                        outputMemory[oMemLine][COLOR_ATTRIBUTE][0] =
                            GPU_CLAMP(outputMemory[oMemLine][COLOR_ATTRIBUTE][0], 0.0f, 1.0f);
                        outputMemory[oMemLine][COLOR_ATTRIBUTE][1] =
                            GPU_CLAMP(outputMemory[oMemLine][COLOR_ATTRIBUTE][1], 0.0f, 1.0f);
                        outputMemory[oMemLine][COLOR_ATTRIBUTE][2] =
                            GPU_CLAMP(outputMemory[oMemLine][COLOR_ATTRIBUTE][2], 0.0f, 1.0f);
                        outputMemory[oMemLine][COLOR_ATTRIBUTE][3] =
                            GPU_CLAMP(outputMemory[oMemLine][COLOR_ATTRIBUTE][3], 0.0f, 1.0f);

                        GPU_DEBUG_BOX(
                            printf("StreamerCommit => Receiving output with index %d from shader.\n",
                                output->getID());
                            for(j = 0; j < MAX_VERTEX_ATTRIBUTES; j++)
                                if (outputAttribute[j])
                                    printf("o[%d] = {%f, %f, %f, %f}\n", j,
                                        output->getAttributes()[j][0],
                                        output->getAttributes()[j][1],
                                        output->getAttributes()[j][2],
                                        output->getAttributes()[j][3]);
                        )

                        //  Check if validation is enabled.
                        if (validationMode)
                        {
                            //  Add vertex to the log.
                            ShadedVertexInfo vInfo;
                            
                            ShaderInputID shInID = output->getShaderInputID();
                            vInfo.vertexID.index = shInID.id.vertexID.index;
                            vInfo.vertexID.instance = shInID.id.vertexID.instance;
                            QuadFloat *vAttr = outputMemory[oMemLine];
                            for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                                vInfo.attributes[a] = vAttr[a];
                            
                            ShadedVertexMap::iterator it;
                            
                            //  Check if the vertex was already shaded.
                            it = shadedVertexLog[nextWriteLog].find(vInfo.vertexID);
                            
                            if (it != shadedVertexLog[nextWriteLog].end())
                            {
                                //  If already shaded check if the values are different.
                                bool attributesAreDifferent = false;
                                
                                for(u32bit a = 0; (a < MAX_VERTEX_ATTRIBUTES) && !attributesAreDifferent; a++)
                                {
                                    //  Evaluate if the attribute values are different.
                                    attributesAreDifferent = (vAttr[a][0] != it->second.attributes[a][0]) ||
                                                             (vAttr[a][1] != it->second.attributes[a][1]) ||
                                                             (vAttr[a][2] != it->second.attributes[a][2]) ||
                                                             (vAttr[a][3] != it->second.attributes[a][3]);
                                }
                                
                                vInfo.differencesBetweenShading = attributesAreDifferent;
                                vInfo.timesShaded = it->second.timesShaded + 1;
                            }
                            else
                            {
                                vInfo.differencesBetweenShading = false;
                                vInfo.timesShaded = 1;
                            }

                            //  Log the shaded vertex.    
                            shadedVertexLog[nextWriteLog].insert(make_pair(vInfo.vertexID, vInfo));
                        }
                    }
                    else
                    {
                        GPU_ASSERT(
                            if (!output->isLast())
                                panic("StreamerCommit", "clock", "Output for a hit index but not last index.");
                        )
                    }

                    /*  Update statistics.  */
                    shOutputs->inc();

                    /*  Delete vertex.  */
                    delete[] output->getAttributes();
                    delete output;
                }
            }

            break;

        case ST_FINISHED:
            /*  Finished state.  Wait for END command from the Streamer
                main box.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => FINISHED state.\n"); )

            /*  Receive a command from the Streamer main box */
            if (streamerCommitCommand->read(cycle, (DynamicObject *&) streamCom))
            {
                processStreamerCommand(streamCom);
            }

            break;

        default:
            panic("StreamerCommit", "clock", "State not supported.");
            break;
    }

    /*  Send current state to the Streamer main box.  */
    streamerCommitState->write(cycle, new StreamerStateInfo(state));

    /*  Send consumer state to the Shader.  */
    for(i = 0; i < numShaders; i++)
    {

        /*  Check for this unexpected situation.  */
        GPU_ASSERT(
           if (lastOutputSent && firstOutput)
               panic("StreamerCommit", "clock", "Unexpected first output received and last output sent in the same cycle.");
        )

        /*  Streamer state signals to the shaders.  */
        if (lastOutputSent)
        {
            /*  Send last output sent signal to Fragment FIFO.  */
            shConsumerSignal[i]->write(cycle, new ConsumerStateInfo(CONS_LAST_VERTEX_COMMIT));

            /*  Reset last output sent state.  */
            lastOutputSent = false;
        }
        else if (firstOutput)
        {
            /*  Send first output sent signal to Fragment FIFO.  */
            shConsumerSignal[i]->write(cycle, new ConsumerStateInfo(CONS_FIRST_VERTEX_IN));

            /*  Reset last output sent state.  */
            firstOutput = false;
        }
        else
        {
            /*  Send ready state to shaders.  */
            shConsumerSignal[i]->write(cycle, new ConsumerStateInfo(CONS_READY)); 
        }
    }

}

/*  Processes a stream command.  */
void StreamerCommit::processStreamerCommand(StreamerCommand *streamCom)
{
    u32bit i;

    /*  Delete last streamer command.  */
    delete lastStreamCom;

    /*  Store as last streamer command (to keep object info).  */
    lastStreamCom = streamCom;

    /*  Process stream command.  */
    switch(streamCom->getCommand())
    {
        case STCOM_RESET:
            /*  Reset command from the Command Processor.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => RESET command.\n"); )

            /*  Do whatever to do.  */

            /*  Change state to reset.  */
            state = ST_RESET;

            break;

        case STCOM_REG_WRITE:
            /*  Write to register command.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => REG_WRITE command.\n"); )

            /*  Check Streamer Commit state.  */
            GPU_ASSERT(
                if (state != ST_READY)
                    panic("StreamerCommit", "processStreamerCommand", "Command not allowed in this state.");
            )

            processGPURegisterWrite(streamCom->getStreamerRegister(),
                streamCom->getStreamerSubRegister(),
                streamCom->getRegisterData());

            break;

        case STCOM_REG_READ:
            /*  Read from register command.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => REG_READ command.\n"); )

            /*  Not supported.  */
            panic("StreamerCommit", "processStreamerCommand", "STCOM_REG_READ not supported.");

            break;

        case STCOM_START:
            /*  Start streaming (drawing) command.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => START command.\n"); )

            /*  Check Streamer state.  */
            GPU_ASSERT(
                if (state != ST_READY)
                    panic("StreamerCommit", "processStreamerCommand", "Command not allowed in this state.");
            )

            /*  Reset the last use table.  Use outputFIFOSize (max
                output FIFO entry identifier + 1) as a
                special identifier saying that the output memory line
                isn't storing any valid data yet.  */
            for(i = 0; i < outputMemorySize; i++)
                lastUseTable[i] = outputFIFOSize;

            /*  Reset the output FIFO.  */
            for(i = 0; i < outputFIFOSize; i++)
                outputFIFO[i].isShaded = FALSE;

            /*  Reset the outputFIFO next output pointer.  */
            nextOutput = 0;

            /*  Reset free output FIFO entry counter.  */
            freeOFIFOEntries = outputFIFOSize;

            /*  Reset outputs sent to the rasterizer counter.  */
            sentOutputs = 0;

            /*  Reset the requested vertex counter from Primitive Assembly.  */
            requestVertex = 0;

            /*  Reset vertex transmission cycles counter.  */
            for(i = 0; i < verticesCycle; i++)
                transCycles[i] = 0;

            /*  Update statistics.  */
            outAttributes->inc(activeOutputs);

            /*  Set streamer state to streaming.  */
            state = ST_STREAMING;

            break;

        case STCOM_END:
            /*  End streaming (drawing) command.  */

            GPU_DEBUG_BOX( printf("StreamerCommit => END command.\n"); )

            /*  Check Streamer state.  */
            GPU_ASSERT(
                if (state != ST_FINISHED)
                    panic("StreamerCommit", "processStreamerCommand", "Command not allowed in this state.");
            )

            //  Check if validation mode is enabled.
            if (validationMode)
            {
                //  Update the pointer to the next write log.
                nextWriteLog = (nextWriteLog + 1) & 0x03;
            }   

            /*  Change state to ready.  */
            state = ST_READY;

            break;


        default:
            panic("StreamerCommit", "processStreamerCommand", "Undefined streamer command.");
            break;
    }
}

/*  Process a register write.  */
void StreamerCommit::processGPURegisterWrite(GPURegister gpuReg, u32bit gpuSubReg,
     GPURegData gpuData)
{

    /*  Process Register.  */
    switch(gpuReg)
    {
        case GPU_VERTEX_OUTPUT_ATTRIBUTE:

            /*  Check vertex output attribute identifier.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("StreamerCommit", "processGPURegisterWrite", "Illegal vertex output attribute identifier.");
            )

            /*  Check if there is an state change.  */
            if (outputAttribute[gpuSubReg] != gpuData.booleanVal)
            {
                /*  Update active vertex output attributes counter.  */
                activeOutputs = gpuData.booleanVal?
                    activeOutputs + 1 : activeOutputs - 1;
            }

            /*  Store new configuration for the vertex output attribute.  */
            outputAttribute[gpuSubReg] = gpuData.booleanVal;

            break;

        case GPU_STREAM_COUNT:
            /*  Streaming vertex count.  */

            streamCount = gpuData.uintVal;

            GPU_DEBUG_BOX( printf("StreamerCommit => GPU_STREAM_COUNT = %d.\n", gpuData.uintVal); )

            break;

        case GPU_STREAM_INSTANCES:
            
            //  Streaming instance count.

            streamInstances = gpuData.uintVal;

            GPU_DEBUG_BOX( printf("StreamerCommit => GPU_STREAM_INSTANCES = %d.\n", gpuData.uintVal); )

            break;

        default:
            panic("StreamerCommit", "processGPURegisterWrite", "Not a Streamer register.");
            break;
    }
}

void StreamerCommit::setValidationMode(bool enable)
{
    validationMode = enable;
}

ShadedVertexMap &StreamerCommit::getShadedVertexInfo()
{
    nextReadLog = (nextReadLog + 1) & 0x03;
    
    return shadedVertexLog[nextReadLog];
}


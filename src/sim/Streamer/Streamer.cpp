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
 * $RCSfile: Streamer.cpp,v $
 * $Revision: 1.16 $
 * $Author: vmoya $
 * $Date: 2007-11-23 10:06:23 $
 *
 * Streamer class implementation file.
 *
 */

#include "Streamer.h"
#include "ShaderStateInfo.h"
#include "StreamerStateInfo.h"
#include "RasterizerStateInfo.h"
#include "ConsumerStateInfo.h"
#include "Rasterizer.h"


using namespace gpu3d;

/*  Table with the size in bytes of each Stream data type.  */
u32bit gpu3d::streamDataSize[] =
{
    1,      /**<  SD_UNORM8.  */
    1,      /**<  SD_SNORM8.  */
    2,      /**<  SD_UNORM16.  */
    2,      /**<  SD_SNORM16.  */
    4,      /**<  SD_UNORM32.  */
    4,      /**<  SD_SNORM32.  */
    2,      /**<  SD_FLOAT16.  */
    4,      /**<  SD_FLOAT32.  */
    1,      /**<  SD_UINT8.  */
    1,      /**<  SD_SINT8.  */
    2,      /**<  SD_UINT16.  */
    2,      /**<  SD_SINT16.  */
    4,      /**<  SD_UINT32.  */
    4       /**<  SD_SINT32.  */
};

/*  Streamer box constructor.  */
Streamer::Streamer(u32bit idxCycle, u32bit idxBufferSz, u32bit outFIFOSz, u32bit outMemSz,
    u32bit numSh, u32bit shOutLat, u32bit vertCycle, u32bit paAttrCycle, u32bit outLat,
    u32bit sLoaderUnits, u32bit slIdxCycle, u32bit slInputReqQSize, u32bit slFillAttrCycle, u32bit slInCacheLines,
    u32bit slInCacheLineSize, u32bit slInCachePortWidth, u32bit slInCacheReqQSz, u32bit slInCacheInQSz,
    bool slForceAttrLoadBypass,
    char **slPrefixArray, char **shPrefixArray, char *name, Box *parent):

    indicesCycle(idxCycle), indexBufferSize(idxBufferSz), outputFIFOSize(outFIFOSz), outputMemorySize(outMemSz),
    numShaders(numSh), maxShOutLat(shOutLat), verticesCycle(vertCycle), paAttributes(paAttrCycle), outputBusLat(outLat),
    streamerLoaderUnits(sLoaderUnits), slIndicesCycle(slIdxCycle), slInputRequestQueueSize(slInputReqQSize),
    slFillAttributes(slFillAttrCycle), slInputCacheLines(slInCacheLines), slInputCacheLineSize(slInCacheLineSize),
    slInputCachePortWidth(slInCachePortWidth), slInputCacheReqQSz(slInCacheReqQSz), slInputCacheInQSz(slInCacheInQSz),
    slForceAttrLoadBypass(slForceAttrLoadBypass),
    Box(name, parent)
{
    u32bit i;
    u32bit numShadersPerSL;
    char **sLoaderName;

    DynamicObject *defaultStreamerState[1];
    //DynamicObject *defaultConsumerState[1];

    /*  Check shaders and shader signals prefixes.  */
    GPU_ASSERT(
        /*  There must at least 1 Vertex Shader.  */
        if (numShaders == 0)
            panic("Streamer", "Streamer", "Invalid number of Shaders (must be >0).");

        if (numShaders < streamerLoaderUnits)
           panic("Streamer", "Streamer", "Greater or equal number of Shaders than Streamer Loader units are required to make the binding");
 
        if (GPU_MOD(numShaders, streamerLoaderUnits) != 0)
           panic("Streamer", "Streamer", "Number of Shaders and Streamer Loader units need to be multiples to make the binding");

        /*  The shader prefix pointer array must be not null.  */
        if (shPrefixArray == NULL)
            panic("Streamer", "Streamer", "The shader prefix array is null.");

        /*  Check if the prefix arrays point to null.  */
        for(i = 0; i < numShaders; i++)
        {
            if (shPrefixArray[i] == NULL)
                panic("Streamer", "Streamer", "Shader prefix array component is null.");
        }
    )

    /*  Create control signal from the Command Processor.  */
    streamCtrlSignal = newInputSignal("StreamerControl", 1, 1, NULL);

    /*  Create state signal to the Command Processor.  */
    streamStateSignal = newOutputSignal("StreamerState", 1, 1, NULL);

    /*  Build initial signal data.  */
    defaultStreamerState[0] = new StreamerStateInfo(ST_READY);

    /*  Set initial signal value.  */
    streamStateSignal->setData(defaultStreamerState);

    /*  Create command signal to the Streamer Fetch.  */
    streamerFetchCom = newOutputSignal("StreamerFetchCommand", 1, 1);

    /*  Create state signal from the Streamer Fetch.  */
    streamerFetchState = newInputSignal("StreamerFetchState", 1, 1);

    /*  Create command signal to the Streamer Output Cache.  */
    streamerOCCom = newOutputSignal("StreamerOutputCacheCommand", 1, 1);

    /*  Create command signal to the Streamer Commit.  */
    streamerCommitCom = newOutputSignal("StreamerCommitCommand", 1, 1);

    /*  Create state signal from the Streamer Commit.  */
    streamerCommitState = newInputSignal("StreamerCommitState", 1, 1);

    /*  Create streamer sub boxes.  */

    /*  Create Streamer Fetch box.  */
    streamerFetch = new StreamerFetch(
        indicesCycle,                                     /*  Indices to fetch/generate per cycle.  */
        indexBufferSize,                                  /*  Size in bytes of the index buffer.  */
        outputFIFOSize,                                   /*  Entries in the output FIFO (indices).  */
        outputMemorySize,                                 /*  Size of the output memory (vertices) (Vertex Cache).  */
        slInputRequestQueueSize,                          /*  Number of entries in each Input Request Queues of the Streamer Loader units (indices).  */
        verticesCycle,                                    /*  Number of vertices commited per cycle at Streamer Commit.  */
        slPrefixArray,                                    /*  Array of Streamer Loader units signal prefixes.  */
        streamerLoaderUnits,                              /*  Number of Streamer Loader units.  */
        slIndicesCycle,                                   /*  Indices per cycle processed per Streamer Loader Unit.  */
        "StreamerFetch", this);

    /*  Create Streamer Output Cache box.  */
    streamerOutputCache = new StreamerOutputCache(
        indicesCycle,               /*  Indices received/searched per cycle.  */
        outputMemorySize,           /*  Size of the output memory (vertices) (Vertex Cache).  */
        verticesCycle,              /*  Number of vertices commited per cycle at Streamer Commit.  */
        streamerLoaderUnits,        /*  Number of Streamer Loader Units. */
        slIndicesCycle,             /*  Indices consumed each cycle per Streamer Loader Unit. */
        "StreamerOutputCache", this);

    streamerLoader = new StreamerLoader*[streamerLoaderUnits];

    sLoaderName = new char*[streamerLoaderUnits];

    streamerLoaderCom = new Signal*[streamerLoaderUnits];

    /*  Compute how many shaders attached to each StreamerLoader unit.  */
    numShadersPerSL = numShaders / streamerLoaderUnits;

    for (i = 0; i < streamerLoaderUnits; i++)
    {
        sLoaderName[i] = new char[18];

        sprintf(sLoaderName[i], "StreamerLoader%d",i);

printf("Creating StreamerLoader Unit %d\n", i); 

        /*  Create Streamer Loader boxes.  */
        streamerLoader[i] = new StreamerLoader(
                i,
                slIndicesCycle,                       /*  Number of indices received per cycle.  */
                slInputRequestQueueSize,              /*  Entries in the input request queue (indices).  */
                slFillAttributes,                     /*  Number of vertex attributes filled per cycle.  */
                slInputCacheLines,                    /*  Number of lines of the input cache.  */
                slInputCacheLineSize,                 /*  Size of the input cache line in bytes.  */
                slInputCachePortWidth,                /*  Width in bytes of the input cache ports.  */
                slInputCacheReqQSz,                   /*  Entries in the input cache Request queue.  */
                slInputCacheInQSz,                    /*  Entries in the input cache input queue.  */
                slForceAttrLoadBypass,                /*  Force attribute load bypass.  */
                numShadersPerSL,                      /*  Number of vertex shaders attached to each Streamer Loader Unit.  */
                &shPrefixArray[i * numShadersPerSL],  /*  Array of prefixes for the vertex shaders.  */
                sLoaderName[i],                       /*  Streamer Loader Unit name.  */
                slPrefixArray[i],                     /*  Streamer Loader Unit prefix. */
                this);

        /*  Create command signal to the Streamer Loader.  */
        streamerLoaderCom[i] = newOutputSignal("StreamerLoaderCommand", 1, 1, slPrefixArray[i]);

        delete[] sLoaderName[i];
    }

    delete[] sLoaderName;

    /*  Create Streamer Commit box.  */
    streamerCommit = new StreamerCommit(
        indicesCycle,               /*  Number of indices received per cycle.  */
        outputFIFOSize,             /*  Number of entries (indices) in the output FIFO.  */
        outputMemorySize,           /*  Number of entries (vertices) in the output Memory (Vertex Cache).  */
        numShaders,                 /*  Number of vertex attached to Streamer Commit.  */
        shOutLat,                   /*  Maximum latency of the shader output signal.  */
        verticesCycle,              /*  Number of vertices sent to Primitive Assembly per cycle.  */
        paAttributes,               /*  Vertex attributes sent to Primitive Assembly per cycle.  */
        outputBusLat,               /*  Latency of the vertex bus with Primitive Assembly.  */
        shPrefixArray,              /*  Array of prefixes for the vertex shaders.  */
        "StreamerCommit", this);

    /*  Set initial state.  */
    state = ST_RESET;

    /*  Set last streamer command to a dummy streamer command.  */
    lastStreamCom = new StreamerCommand(STCOM_RESET);
}

/*  Simulation rutine.  */
void Streamer::clock(u64bit cycle)
{
    //MemoryTransaction *memTrans;
    StreamerCommand *streamCom;
    StreamerStateInfo *streamState;
    StreamerState streamCommit;
    StreamerState streamFetch;

    int i;

    GPU_DEBUG( printf("Streamer => Clock %lld\n", cycle);)

    /*  Clock all Streamer subunits.  */

    /*  Clock Streamer Fetch.  */
    streamerFetch->clock(cycle);

    /*  Clock Streamer Output Cache.  */
    streamerOutputCache->clock(cycle);

    /*  Clock Streamer Loader boxes.  */
    for (u32bit i = 0; i < streamerLoaderUnits; i++)
    {    
       streamerLoader[i]->clock(cycle);
    }

    /*  Clock Streamer Commit.  */
    streamerCommit->clock(cycle);

    /*  Get state from the Streamer Fetch.  */
    if(streamerFetchState->read(cycle, (DynamicObject *&) streamState))
    {
        /*  Keep state info.  */
        streamFetch = streamState->getState();

        /*  Delete streamer state info object.  */
        delete streamState;
    }
    else
    {
        /*  Missing state signal.  */
        panic("Streamer", "clock", "No state signal from the Streamer Fetch.");
    }

    /*  Get state from the Streamer Commit.  */
    if (streamerCommitState->read(cycle, (DynamicObject *&) streamState))
    {
        /*  Keep state info.  */
        streamCommit = streamState->getState();

        /*  Delete streamer state info object.  */
        delete streamState;
    }
    else
    {
        /*  Missing signal.  */
        panic("Streamer", "clock", "No state signal from the Streamer Commit.");
    }

    /*  Streamer tasks.  */
    switch(state)
    {
        case ST_RESET:
            /*  Streamer is resetting.  */

            GPU_DEBUG( printf("Streamer => RESET state.\n"); )

            /*  Issue reset signals to all the Streamer units.  */


            /*  Send reset to the Streamer Fetch.  */

            /*  Create reset streamer command.  */
            streamCom = new StreamerCommand(STCOM_RESET);

            /*  Copy cookies from original reset command.  */
            streamCom->copyParentCookies(*lastStreamCom);

            /*  Add a new cookie.  */
            streamCom->addCookie();

            /*  Send signal to the Streamer Fetch.  */
            streamerFetchCom->write(cycle, streamCom);


            /*  Send reset to the Streamer Output Cache.  */

            /*  Create reset streamer command.  */
            streamCom = new StreamerCommand(STCOM_RESET);

            /*  Copy cookies from original reset command.  */
            streamCom->copyParentCookies(*lastStreamCom);

            /*  Add a new cookie.  */
            streamCom->addCookie();

            /*  Send signal to the Streamer Output Cache.  */
            streamerOCCom->write(cycle, streamCom);

            /*  Send reset to the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create reset streamer command.  */
                streamCom = new StreamerCommand(STCOM_RESET);

                /*  Copy cookies from original reset command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                /*  Add a new cookie.  */
                streamCom->addCookie();

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            /*  Send reset to the Streamer Commit.  */

            /*  Create reset streamer command.  */
            streamCom = new StreamerCommand(STCOM_RESET);

            /*  Copy cookies from original reset command.  */
            streamCom->copyParentCookies(*lastStreamCom);

            /*  Add a new cookie.  */
            streamCom->addCookie();

            /*  Send signal to the Streamer Commit.  */
            streamerCommitCom->write(cycle, streamCom);

            /*  Set initial index stream.  */
            indexBuffer = 0;

            /*  Change state.  */
            state = ST_READY;

            break;

        case ST_READY:
            /*  Streamer can receive new commands and state changes from
                the Command Processor.  */

            GPU_DEBUG( printf("Streamer => READY state.\n"); )

            /*  Get commands from the Command Processor.  */
            if (streamCtrlSignal->read(cycle, (DynamicObject *&) streamCom))
            {
                /*  Process streamer command.  */
                processStreamerCommand(cycle, streamCom);
            }

            break;

        case ST_STREAMING:
            /*  Streamer is streaming data to the Shaders.  */

            /*  Check if the Streamer Fetch and the Streamer Commit have
                finished.  */
            if ((streamFetch == ST_FINISHED) && (streamCommit == ST_FINISHED))
            {
                /*  Change Streamer state to finished.  */
                state = ST_FINISHED;
            }

            break;

        case ST_FINISHED:
            /*  Streamer has finished streaming data to the Shader.
                Waiting for Command Processor response.  */

            GPU_DEBUG( printf("Streamer => FINISHED state.\n"); )

            /*  Receive a command from the Command Processor */
            if (streamCtrlSignal->read(cycle, (DynamicObject *&) streamCom))
            {
                processStreamerCommand(cycle, streamCom);
            }

            break;

        default:
            panic("Streamer", "clock", "Undefined Streamer state.");
            break;
    }

    /*  Send Streamer state to the Command Processor.  */
    streamStateSignal->write(cycle, new StreamerStateInfo(state));
}

/*  Processes a stream command.  */
void Streamer::processStreamerCommand(u64bit cycle, StreamerCommand *streamCom)
{
    StreamerCommand *auxStCom;

    /*  Delete last streamer command.  */
    delete lastStreamCom;

    /*  Store as last streamer command (to keep object info).  */
    lastStreamCom = streamCom;

    /*  Process stream command.  */
    switch(streamCom->getCommand())
    {
        case STCOM_RESET:
            /*  Reset command from the Command Processor.  */

            GPU_DEBUG(
                printf("Streamer => RESET command.\n");
            )

            /*  Do whatever to do.  */

            /*  Change state to reset.  */
            state = ST_RESET;

            break;

        case STCOM_REG_WRITE:
            /*  Write to register command.  */

            GPU_DEBUG(
                printf("Streamer => REG_WRITE command.\n");
            )

            /*  Check Streamer state.  */
            GPU_ASSERT(
                if (state != ST_READY)
                    panic("Streamer", "processStreamerCommand", "Command not allowed in this state.");
            )

            processGPURegisterWrite(streamCom->getStreamerRegister(),
                streamCom->getStreamerSubRegister(),
                streamCom->getRegisterData(), cycle);

            break;

        case STCOM_REG_READ:
            /*  Read from register command.  */

            GPU_DEBUG(
                printf("Streamer => REG_READ command.\n");
            )

            /*  Not supported.  */
            panic("Streamer", "processStreamerCommand", "STCOM_REG_READ not supported.");

            break;

        case STCOM_START:
            {
                /*  Start streaming (drawing) command.  */

                GPU_DEBUG(
                    printf("Streamer => START command.\n");
                )

                /*  Check Streamer state.  */
                GPU_ASSERT(
                    if (state != ST_READY)
                        panic("Streamer", "processStreamerCommand", "Command not allowed in this state.");
                )

                u32bit firstActiveAttribute = 0;
                
                /*  Search for the first active attribute.  */
                for(; (firstActiveAttribute < MAX_VERTEX_ATTRIBUTES) &&
                    (attributeMap[firstActiveAttribute] == ST_INACTIVE_ATTRIBUTE); firstActiveAttribute++);

                /*  Check if there is an active attribute (SHOULD BE!!!).  */
                GPU_ASSERT(
                    if (firstActiveAttribute == MAX_VERTEX_ATTRIBUTES)
                        panic("Streamer", "processStreamerCommand", "No active attributes.");
                )

                /*  Check the number of vertex to draw.  */
                GPU_ASSERT(
                    if (count == 0)
                        panic("Streamer", "processStreamerCommand", "Trying to draw an empty vertex batch.");
                )

                /*  Send START command to all Streamer units.  */

                /*  Send START command to the Streamer Fetch.  */

                /*  Create START command.  */
                auxStCom = new StreamerCommand(STCOM_START);

                /*  Copy parent command cookies.  */
                auxStCom->copyParentCookies(*lastStreamCom);

                /*  Send command to the Streamer Fetch.  */
                streamerFetchCom->write(cycle, auxStCom);


                /*  Send START command to the Streamer Output Cache.  */

                /*  Create START command.  */
                auxStCom = new StreamerCommand(STCOM_START);

                /*  Copy parent command cookies.  */
                auxStCom->copyParentCookies(*lastStreamCom);

                /*  Send command to the Streamer Output Cache.  */
                streamerOCCom->write(cycle, auxStCom);


                /*  Send START command to the Streamer Loader Units.  */
                for (u32bit i = 0; i < streamerLoaderUnits; i++)
                {
                    /*  Create START command.  */
                    auxStCom = new StreamerCommand(STCOM_START);

                    /*  Copy parent command cookies.  */
                    auxStCom->copyParentCookies(*lastStreamCom);

                    streamerLoaderCom[i]->write(cycle, auxStCom);
                }

                /*  Send START command to the Streamer Commit.  */

                /*  Create START command.  */
                auxStCom = new StreamerCommand(STCOM_START);

                /*  Copy parent command cookies.  */
                auxStCom->copyParentCookies(*lastStreamCom);

                /*  Send command to the Streamer Commit.  */
                streamerCommitCom->write(cycle, auxStCom);


                /*  Set streamer state to streaming.  */
                state = ST_STREAMING;

            }
            
            break;

        case STCOM_END:
            /*  End streaming (drawing) command.  */

            GPU_DEBUG(
                printf("Streamer => END command.\n");
            )

            /*  Check Streamer state.  */
            GPU_ASSERT(
                if (state != ST_FINISHED)
                    panic("Streamer", "processStreamerCommand", "Command not allowed in this state.");
            )

            /*  Send END command to all Streamer Units.  */

            /*  Send END command to the Streamer Fetch.  */

            /*  Create END command.  */
            auxStCom = new StreamerCommand(STCOM_END);

            /*  Copy parent command cookies.  */
            auxStCom->copyParentCookies(*lastStreamCom);

            /*  Send command to the Streamer Fetch.  */
            streamerFetchCom->write(cycle, auxStCom);


            /*  Send END command to the Streamer Output Cache.  */

            /*  Create END command.  */
            auxStCom = new StreamerCommand(STCOM_END);

            /*  Copy parent command cookies.  */
            auxStCom->copyParentCookies(*lastStreamCom);

            /*  Send command to the Streamer Output Cache.  */
            streamerOCCom->write(cycle, auxStCom);

            
            /*  Send END command to the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create END command.  */
                auxStCom = new StreamerCommand(STCOM_END);

                /*  Copy parent command cookies.  */
                auxStCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, auxStCom);
            }


            /*  Send END command to the Streamer Commit.  */

            /*  Create END command.  */
            auxStCom = new StreamerCommand(STCOM_END);

            /*  Copy parent command cookies.  */
            auxStCom->copyParentCookies(*lastStreamCom);

            /*  Send command to the Streamer Commit.  */
            streamerCommitCom->write(cycle, auxStCom);


            /*  Change state to ready.  */
            state = ST_READY;

            break;


        default:
            panic("Streamer", "processStreamerCommand", "Undefined streamer command.");
            break;
    }
}

/*  Process a register write.  */
void Streamer::processGPURegisterWrite(GPURegister gpuReg, u32bit gpuSubReg,
     GPURegData gpuData, u64bit cycle)
{
    StreamerCommand *streamCom;
    int i;

    /*  Process Register.  */
    switch(gpuReg)
    {
        case GPU_VERTEX_OUTPUT_ATTRIBUTE:
            /*  Vertex output attribute active/written.  */

            GPU_DEBUG(
                printf("Streamer => GPU_VERTEX_OUTPUT_ATTRIBTE[%d] = %s.\n",
                    gpuSubReg, gpuData.booleanVal?"TRUE":"FALSE");
            )

            /*  Check the vertex attribute ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("Streamer", "processGPURegisterWrite", "Illegal vertex attribute ID.");
            )

            /*  Update the register.  */
            outputAttribute[gpuSubReg] = gpuData.booleanVal;

            /*  Send register write to the Streamer Commit.  */

            /*  Create streamer register write.  */
            streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original streamer command.  */
            streamCom->copyParentCookies(*lastStreamCom);

            /*  Write commadn to the Streamer Loader.  */
            streamerCommitCom->write(cycle, streamCom);

            break;


        case GPU_VERTEX_ATTRIBUTE_MAP:
            /*  Mapping between stream buffers and vertex input parameters.  */

            GPU_DEBUG(
                printf("Streamer => GPU_VERTEX_ATTRIBUTE_MAP[%d] = %d.\n",
                    gpuSubReg, gpuData.uintVal);
            )

            /*  Check the vertex attribute ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("Streamer", "processGPURegisterWrite", "Illegal vertex attribute ID.");
            )

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if ((gpuData.uintVal >= MAX_STREAM_BUFFERS) && (gpuData.uintVal != ST_INACTIVE_ATTRIBUTE))
                    panic("Streamer", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Write the register.  */
            attributeMap[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            break;

        case GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE:
            /*  Mapping between stream buffers and vertex input parameters.  */

            GPU_DEBUG(
                printf("Streamer => GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE[%d] = {%f, %f, %f, %f}.\n",
                    gpuSubReg, gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            /*  Check the vertex attribute ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("Streamer", "processGPURegisterWrite", "Illegal vertex attribute ID.");
            )

            /*  Write the register.  */
            attrDefValue[gpuSubReg][0] = gpuData.qfVal[0];
            attrDefValue[gpuSubReg][1] = gpuData.qfVal[1];
            attrDefValue[gpuSubReg][2] = gpuData.qfVal[2];
            attrDefValue[gpuSubReg][3] = gpuData.qfVal[3];


            /*  Send register write to all the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            break;

        case GPU_STREAM_ADDRESS:
            /*  Stream buffer address.  */

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("Streamer", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Check start address.  */


            /*  Write the register.  */
            bufferAddress[gpuSubReg] = gpuData.uintVal;

            GPU_DEBUG(
                printf("Streamer => GPU_STREAM_ADDRESS[%d] = %x.\n",
                    gpuSubReg, gpuData.uintVal);
            )

            /*  Send register write to the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            /*  Check if it is the index stream.  */
            if (gpuSubReg == indexBuffer)
            {
                /*  Send register write to the Streamer Fetch.  */

                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                /*  Write commadn to the Streamer Fetch.  */
                streamerFetchCom->write(cycle, streamCom);
            }

            break;

        case GPU_STREAM_STRIDE:
            /*  Stream buffer stride.  */

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("Streamer", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Write the register.  */
            bufferStride[gpuSubReg] = gpuData.uintVal;

            GPU_DEBUG(
                printf("Streamer => GPU_STREAM_STRIDE[%d] = %d.\n",
                    gpuSubReg, gpuData.uintVal);
            )

            /*  Send register write to the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            break;

        case GPU_STREAM_DATA:
            /*  Stream buffer data type.  */

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("Streamer", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Write the register.  */
            bufferData[gpuSubReg] = gpuData.streamData;

            GPU_DEBUG(
                printf("Streamer => GPU_STREAMDATA[%d] = ", gpuSubReg);
                switch(gpuData.streamData)
                {
                    case SD_UNORM8:
                        printf("SD_UNORM8");
                        break;
                    case SD_SNORM8:
                        printf("SD_SNORM8");
                        break;
                    case SD_UNORM16:
                        printf("SD_UNORM16");
                        break;
                    case SD_SNORM16:
                        printf("SD_SNORM16");
                        break;
                    case SD_UNORM32:
                        printf("SD_UNORM32");
                        break;
                    case SD_SNORM32:
                        printf("SD_SNORM32");
                        break;
                    case SD_FLOAT16:
                        printf("SD_FLOAT16");
                        break;                    
                    case SD_FLOAT32:
                        printf("SD_FLOAT32");
                        break;
                    case SD_UINT8:
                        printf("SD_UINT8");
                        break;
                    case SD_SINT8:
                        printf("SD_SINT8");
                        break;
                    case SD_UINT16:
                        printf("SD_UINT16");
                        break;
                    case SD_SINT16:
                        printf("SD_SINT16");
                        break;
                    case SD_UINT32:
                        printf("SD_UINT32");
                        break;
                    case SD_SINT32:
                        printf("SD_SINT32");
                        break;
                    default:
                        panic("Streamer", "processGPURegisterWrite", "Undefined format.");
                        break;                        
                }
            )

            /*  Send register write to the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            /*  Check if it is the index stream.  */
            if (gpuSubReg == indexBuffer)
            {
                /*  Send register write to the Streamer Fetch.  */

                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                /*  Write command to the Streamer Fetch.  */
                streamerFetchCom->write(cycle, streamCom);
            }

            break;

        case GPU_STREAM_ELEMENTS:
            /*  Stream buffer elements (of any stream data type).  */

            /*  Check the stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("Streamer", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            /*  Write the register.  */
            bufferElements[gpuSubReg] = gpuData.uintVal;

            GPU_DEBUG(
                printf("Streamer => GPU_STREAM_ELEMENTS[%d] = %d.\n",
                    gpuSubReg, gpuData.uintVal);
            )

            /*  Send register write to the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            break;

        case GPU_STREAM_FREQUENCY:
            
            //  Stream buffer update/read frequency (indices/vertices).

            //  Check the stream buffer ID.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("Streamer", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            //  Write the register.
            bufferFrequency[gpuSubReg] = gpuData.uintVal;

            GPU_DEBUG(
                printf("Streamer => GPU_STREAM_FREQUENCY[%d] = %d.\n",
                    gpuSubReg, gpuData.uintVal);
            )

            //  Send register write to the Streamer Loader Units.
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                //  Create streamer register write.
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original streamer command.
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            break;

        case GPU_STREAM_START:
            /*  Streaming start position.  */

            start = gpuData.uintVal;

            GPU_DEBUG(
                printf("Streamer => GPU_STREAM_START = %d.\n", gpuData.uintVal);
            )

            /*  Send register write to the Streamer Fetch.  */

            /*  Create streamer register write.  */
            streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original streamer command.  */
            streamCom->copyParentCookies(*lastStreamCom);

            /*  Write command to the Streamer Fetch.  */
            streamerFetchCom->write(cycle, streamCom);

            /*  Send register write to the Streamer Loader Units.  */
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                /*  Create streamer register write.  */
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original streamer command.  */
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            break;

        case GPU_STREAM_COUNT:
            /*  Streaming vertex count.  */

            count = gpuData.uintVal;

            GPU_DEBUG(
                printf("Streamer => GPU_STREAM_COUNT = %d.\n", gpuData.uintVal);
            )

            /*  Send register write to the Streamer Fetch.  */

            /*  Create streamer register write.  */
            streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original streamer command.  */
            streamCom->copyParentCookies(*lastStreamCom);

            /*  Write command to the Streamer Fetch.  */
            streamerFetchCom->write(cycle, streamCom);

            /*  Send register write to the Streamer Commit.  */

            /*  Create streamer register write.  */
            streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original streamer command.  */
            streamCom->copyParentCookies(*lastStreamCom);

            /*  Write command to the Streamer Commit.  */
            streamerCommitCom->write(cycle, streamCom);

            break;

        case GPU_STREAM_INSTANCES:
            
            //  Streaming instance count.

            instances = gpuData.uintVal;

            GPU_DEBUG(
                printf("Streamer => GPU_STREAM_INSTANCES = %d.\n", gpuData.uintVal);
            )

            //  Send register write to the Streamer Fetch.

            //  Create streamer register write.
            streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original streamer command.
            streamCom->copyParentCookies(*lastStreamCom);

            //  Write command to the Streamer Fetch.
            streamerFetchCom->write(cycle, streamCom);

            //  Send register write to the Streamer Commit.

            //  Create streamer register write.
            streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original streamer command.
            streamCom->copyParentCookies(*lastStreamCom);

            //  Write command to the Streamer Commit.
            streamerCommitCom->write(cycle, streamCom);

            break;

        case GPU_INDEX_MODE:
            /*  Indexed streaming mode enabled/disabled.  */

            indexedMode = gpuData.booleanVal;

            GPU_DEBUG(
                printf("Streamer => GPU_INDEX_MODE = %s.\n",
                    gpuData.booleanVal?"TRUE":"FALSE");
            )

            /*  Send register write to the Streamer Fetch.  */

            /*  Create streamer register write.  */
            streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original streamer command.  */
            streamCom->copyParentCookies(*lastStreamCom);

            /*  Write commadn to the Streamer Loader.  */
            streamerFetchCom->write(cycle, streamCom);

            break;

        case GPU_INDEX_STREAM:
            /*  Index stream buffer ID.  */

            /*  Check stream buffer ID.  */
            GPU_ASSERT(
                if (gpuData.uintVal >= MAX_STREAM_BUFFERS)
                    panic("Streamer", "processGPURegisterWrite", "Illegal stream buffer ID.");
            )

            indexBuffer = gpuData.uintVal;

            GPU_DEBUG(
                printf("Streamer => GPU_INDEX_STREAM = %d.\n", gpuData.uintVal);
            )

            break;

        case GPU_D3D9_COLOR_STREAM:
    
            //  Sets if the color stream has be read using the component order defined by D3D9.

            /*  Check stream buffer ID.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("Streamer", "processGPURegisterWrite", "Stream buffer ID out of range.");
            )

            d3d9ColorStream[gpuSubReg] = gpuData.booleanVal;

            GPU_DEBUG(
                printf("Streamer => GPU_D3D9_COLOR_STREAM[%d] = %s.\n", gpuSubReg, gpuData.booleanVal ? "TRUE" : "FALSE");
            )

            //  Send register write to the Streamer Loader Units.
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                //  Create streamer register write.
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original streamer command.
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }
            
            break;

        case GPU_ATTRIBUTE_LOAD_BYPASS:
    
            //  Sets if the attribute load stage is bypassed to implement attribute load in the shader.
            attributeLoadBypass = gpuData.booleanVal;

            GPU_DEBUG(
                printf("Streamer => GPU_ATTRIBUTE_LOAD_BYPASS = %s.\n", gpuData.booleanVal ? "TRUE" : "FALSE");
            )

            //  Send register write to the Streamer Loader Units.
            for (u32bit i = 0; i < streamerLoaderUnits; i++)
            {
                //  Create streamer register write.
                streamCom = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original streamer command.
                streamCom->copyParentCookies(*lastStreamCom);

                streamerLoaderCom[i]->write(cycle, streamCom);
            }

            break;

        default:
            panic("Streamer", "processGPURegisterWrite", "Not a Streamer register.");
            break;
    }
}

void Streamer::setValidationMode(bool enable)
{
    streamerCommit->setValidationMode(enable);
    for(u32bit u = 0; u < streamerLoaderUnits; u++)
        streamerLoader[u]->setValidationMode(enable);
}

ShadedVertexMap &Streamer::getShadedVertexInfo()
{
    return streamerCommit->getShadedVertexInfo();
}

VertexInputMap &Streamer::getVertexInputInfo(u32bit unit)
{
    return streamerLoader[unit]->getVertexInputInfo();
}





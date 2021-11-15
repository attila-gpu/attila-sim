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
 * $RCSfile: MemoryController.cpp,v $
 * $Revision: 1.33 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:47 $
 *
 * Memory Controller class implementation file.
 *
 */

/**
 *
 *  @file MemoryController.cpp
 *
 *  Memory Controller implementation file.  The Memory Controller class defines the box
 *  controlling the access to GPU and system memory and servicing memory requests
 *  from the other GPU units.
 *
 */

#include "MemoryController.h"
#include <string.h>
#include <stdio.h>
#include "GPUMath.h"
#include "MemoryTransaction.h"
#include "MemoryControllerCommand.h"

#include <fstream>

using namespace gpu3d;
using namespace std;

// Patch for windows
#ifdef WIN32
    #define  U64FMT  "%I64d"
#else
    #define  U64FMT  "%lld"
#endif

/*  Memory Controller box constructor.  */
MemoryController::MemoryController(u32bit memSize, u32bit clockMult, u32bit memFreq, u32bit memBusWidth, u32bit numBuses,
    bool sharedBanks, u32bit bankGranurality, u32bit burstLength, u32bit readLat, u32bit writeLat, u32bit writeToReadLat,
    u32bit pageSize, u32bit memOpenPages, u32bit pageOpenLat,
    u32bit consReads, u32bit consWrites, u32bit comProcBus, u32bit streamerFetchBus, u32bit streamerLoaderBus,
    u32bit zStencilBus, u32bit colorWriteBus, u32bit dacBus, u32bit textUnitBus,
    u32bit systemMem, u32bit readLines, u32bit writeLines, u32bit reqQSize, u32bit servQSize,
    u32bit stampUnits, const char **suPrefixArray, u32bit numTxUnits, const char **tuPrefixArray,
    u32bit streamLoadUnits, const char **slPrefixArray, char *name, Box *parent):
    gpuMemorySize(memSize), baseClockMultiplier(clockMult), gpuMemoryFrequency(memFreq), gpuMemoryBuses(numBuses),
    gpuMemoryBusWidth(memBusWidth), gpuSharedBanks(sharedBanks), gpuBankGranurality(bankGranurality),
    gpuMemoryBurstLength(burstLength), gpuMemReadLatency(readLat), gpuMemWriteLatency(writeLat),
    gpuMemWriteToReadLatency(writeToReadLat), gpuMemPageSize(pageSize), gpuMemOpenPages(memOpenPages),
    gpuMemPageOpenLat(pageOpenLat), maxConsecutiveReads(consReads), maxConsecutiveWrites(consWrites),
    mappedMemorySize(systemMem), readBufferSize(readLines),
    writeBufferSize(writeLines), requestQueueSize(reqQSize), serviceQueueSize(servQSize),
    numStampUnits(stampUnits), numTextUnits(numTxUnits), streamerLoaderUnits(streamLoadUnits),
    _lastCycle(0),

/*  Memory Controller box parameter initalization.  */
    Box(name, parent)

{
    DynamicObject *defValue[2];
    MemoryTransaction *memTrans;
    char busName[128];
    char signalName[128];
    u32bit i;
    u32bit j;
    u32bit busQueues;

    // Init auxiliar busNames array
    busNames[COMMANDPROCESSOR] = MemoryTransaction::getBusName(COMMANDPROCESSOR);
    busNames[STREAMERFETCH] = MemoryTransaction::getBusName(STREAMERFETCH);
    busNames[STREAMERLOADER] = MemoryTransaction::getBusName(STREAMERLOADER);
    busNames[ZSTENCILTEST] = MemoryTransaction::getBusName(ZSTENCILTEST);
    busNames[COLORWRITE] = MemoryTransaction::getBusName(COLORWRITE);
    busNames[DACB] = MemoryTransaction::getBusName(DACB);
    busNames[TEXTUREUNIT] = MemoryTransaction::getBusName(TEXTUREUNIT);
    busNames[MEMORYMODULE] = MemoryTransaction::getBusName(MEMORYMODULE);
    busNames[SYSTEM] = MemoryTransaction::getBusName(SYSTEM);

    /*  Check number of memory buses.  */
    GPU_ASSERT(
        if (gpuMemoryBuses == 0)
            panic("MemoryController", "MemoryController", "At least one memory bus required.");
    )

    /*  Check limit of buses per GPU unit.  */
    GPU_ASSERT(
        if (baseClockMultiplier == 0)
            panic("MemoryController", "MemoryController", "Base clock multiplier must be at least 1.");
        if (gpuMemoryFrequency == 0)
            panic("MemoryController", "MemoryController", "Memory frequency (in reference clock cycles) must be at least 1.");
        if (gpuMemoryBuses == 0)
            panic("MemoryController", "MemoryController", "At least one memory bus required to work.");
        if (gpuMemReadLatency == 0)
            panic("MemoryController", "MemoryController", "GPU memory Read Latency must be at least 1 cycle.");
        if (gpuMemWriteLatency == 0)
            panic("MemoryController", "MemoryController", "GPU memory Write latency must be at least 1 cycle.");
        if (gpuMemWriteLatency > gpuMemReadLatency)
            panic("MemoryController", "MemoryController", "GPU memory Write latency must be smaller or equal than read latency.");
        if (gpuMemPageSize < MAX_TRANSACTION_SIZE)
            panic("MemoryController", "MemoryController", "A page must be larger than the maximum memory transaction.");
        if (gpuBankGranurality > gpuMemPageSize)
            panic("MemoryController", "MemoryController", "The bus interleaving granurality must be equal or smaller than the page size.");
        if (gpuMemPageOpenLat == 0)
            panic("MemoryController", "MemoryController", "The latency for opening a page must be at least 1 cycle.");
        if (readBufferSize == 0)
            panic("MemoryController", "MemoryController", "Minimum number of read buffer lines required is 1.");
        if (writeBufferSize == 0)
            panic("MemoryController", "MemoryController", "Minimum number of write buffer lines required is 1.");
        if (requestQueueSize < ((numTextUnits + numStampUnits + 4)*2))
            panic("MemoryController", "MemoryController", "Minimum number of request queue entries is: (numTextUnits + numStampsUnits + 4) * 2.");
        if (serviceQueueSize < 1)
            panic("MemoryController", "MemoryController", "Minimum number of service queue entries is: 1.");
        if (numTextUnits > MAX_GPU_UNIT_BUSES)
            panic("MemoryController", "MemoryController", "Number of Texture Unit excedes GPU unit buses limit.");
        if (numStampUnits > MAX_GPU_UNIT_BUSES)
            panic("MemoryController", "MemoryController", "Number of stamp units excedes GPU unit buses limit.");
        if (streamerLoaderUnits > MAX_GPU_UNIT_BUSES)
            panic("MemoryController", "MemoryController", "Number of Streamer Loader units excedes GPU unit buses limit.");
    )

    /*  Initialize the GPU memory buffer.  */
    gpuMemory = new u8bit[memSize];

    GPU_ASSERT(
        if (gpuMemory == NULL)
            panic("MemoryController", "MemoryController", "Error allocation GPU memory.");
    )

    /*  Mark memory as empty.  */
    for(i = 0; i < gpuMemorySize; i = i + 4)
        *((u32bit *) &gpuMemory[i]) = 0xDEADCAFE;

    /*  Initialize the mapped system memory buffer.  */
    mappedMemory = new u8bit[mappedMemorySize];

    GPU_ASSERT(
        if (mappedMemory == NULL)
            panic("MemoryController", "MemoryController", "Error allocation mapped system memory.");
    )

    /*  Mark memory as empty.  */
    for(i = 0; i < mappedMemorySize; i = i + 4)
        *((u32bit *) &mappedMemory[i]) = 0xDEADCAFE;

    /* Command signal (to receive commands from command processor) */
    mcCommSignal = newInputSignal("MemoryControllerCommand", 1, 1, 0);

    /*  Read signal from the Memory Controller to the Command Processor.  */
    commProcReadSignal = newOutputSignal("CommProcMemoryRead", 2, 1, NULL);

    /*  Build default signal value array.  */
    memTrans = new MemoryTransaction(MS_BOTH);
    defValue[0] = memTrans;
    memTrans = new MemoryTransaction(MS_BOTH);
    defValue[1] = memTrans;

    /*  Set default signal value array.  */
    commProcReadSignal->setData(defValue);

    /*  Write signal from the Command Processor to the Memory Controller.  */
    commProcWriteSignal = newInputSignal("CommProcMemoryWrite", 1, 1, NULL);

    /*  Check number of attached Streamer Loader units and their prefixes.  */
    GPU_ASSERT(
        if (streamerLoaderUnits == 0)
            panic("MemoryController", "MemoryController", "There should be at least one Streamer Loader unit.");

        if (slPrefixArray == NULL)
            panic("MemoryController", "MemoryController", "The Streamer Loader unit signal name prefix array can not be null.");

        for(i = 0; i < streamerLoaderUnits; i++)
        {
            if (slPrefixArray[i] == NULL)
                panic("MemoryController", "MemoryController", "Streamer Loader unit signal name prefix is null.");
        }
    )

    /*  Allocate signals with Streamer Loader units.  */
    streamLoadDataSignal = new Signal*[streamerLoaderUnits];
    streamLoadReqSignal = new Signal*[streamerLoaderUnits];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (streamLoadDataSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating Streamer Loader data signal array.");

        if (streamLoadReqSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating Streamer Loader request signal array.");
    )

    for (i = 0; i < streamerLoaderUnits; i++)
    {
        /*  Read signal from the Memory Controller to the Streamer Loader.  */
        streamLoadDataSignal[i] = newOutputSignal("StreamerLoaderMemoryData", 2, 1, slPrefixArray[i]);

        /*  Build default signal value array.  */
        memTrans = new MemoryTransaction(MS_BOTH);
        defValue[0] = memTrans;
        memTrans = new MemoryTransaction(MS_BOTH);
        defValue[1] = memTrans;

        /*  Set default signal value array.  */
        streamLoadDataSignal[i]->setData(defValue);
        
        /*  Command signal from the Streamer Loader unit to the memory Controller.  */
        streamLoadReqSignal[i] = newInputSignal("StreamerLoaderMemoryRequest", 1, 1, slPrefixArray[i]);
    }

    /*  Data signal from the Memory Controller to the Streamer Fetch.  */
    streamFetchDataSignal = newOutputSignal("StreamerFetchMemoryData", 2, 1, NULL);

    /*  Build default signal value array.  */
    memTrans = new MemoryTransaction(MS_BOTH);
    defValue[0] = memTrans;
    memTrans = new MemoryTransaction(MS_BOTH);
    defValue[1] = memTrans;

    /*  Set default signal value array.  */
    streamFetchDataSignal->setData(defValue);

    /*  Request signal from the Streamer Fetch to the Memory Controller.  */
    streamFetchReqSignal = newInputSignal("StreamerFetchMemoryRequest", 1, 1, NULL);

    /*  Check number of attached stamp units and their prefixes.  */
    GPU_ASSERT(
        if (numStampUnits == 0)
            panic("MemoryController", "MemoryController", "There should be at least a stamp pipe.");

        if (suPrefixArray == NULL)
            panic("MemoryController", "MemoryController", "The stamp unit signal name prefix array can not be null.");

        for(i = 0; i < numStampUnits; i++)
        {
            if (suPrefixArray[i] == NULL)
                panic("MemoryController", "MemoryController", "Stamp unit signal name prefix is null.");
        }
    )

    /*  Allocate signals with the stamp units.  */
    zStencilDataSignal = new Signal*[numStampUnits];
    zStencilReqSignal = new Signal*[numStampUnits];
    colorWriteDataSignal = new Signal*[numStampUnits];
    colorWriteReqSignal = new Signal*[numStampUnits];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (zStencilDataSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating Z Stencil data signal array.");

        if (zStencilReqSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating Z Stencil request signal array.");

        if (colorWriteDataSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating Color Write data signal array.");

        if (colorWriteReqSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating Color Write request signal array.");
    )

    /*  Create signals with the stamp units.  */
    for(i = 0; i < numStampUnits; i++)
    {

        /*  Data signal from the Memory Controller to the Z Stencil Test unit.  */
        zStencilDataSignal[i] = newOutputSignal("ZStencilTestMemoryData", 2, 1, suPrefixArray[i]);

        /*  Build default signal value array.  */
        memTrans = new MemoryTransaction(MS_BOTH);
        defValue[0] = memTrans;
        memTrans = new MemoryTransaction(MS_BOTH);
        defValue[1] = memTrans;

        /*  Set default signal value array.  */
        zStencilDataSignal[i]->setData(defValue);

        /*  Request signal from the Z Stencil Test unit to the Memory Controller.  */
        zStencilReqSignal[i] = newInputSignal("ZStencilTestMemoryRequest", 1, 1, suPrefixArray[i]);


        /*  Data signal from the Memory Controller to the Color Write unit.  */
        colorWriteDataSignal[i] = newOutputSignal("ColorWriteMemoryData", 2, 1, suPrefixArray[i]);

        /*  Build default signal value array.  */
        memTrans = new MemoryTransaction(MS_BOTH);
        defValue[0] = memTrans;
        memTrans = new MemoryTransaction(MS_BOTH);
        defValue[1] = memTrans;

        /*  Set default signal value array.  */
        colorWriteDataSignal[i]->setData(defValue);

        /*  Request signal from the ColorWrite unit to the Memory Controller.  */
        colorWriteReqSignal[i] = newInputSignal("ColorWriteMemoryRequest", 1, 1, suPrefixArray[i]);
    }


    /*  Data signal from the Memory Controller to the DAC unit.  */
    dacDataSignal = newOutputSignal("DACMemoryData", 2, 1, NULL);

    /*  Build default signal value array.  */
    memTrans = new MemoryTransaction(MS_BOTH);
    defValue[0] = memTrans;
    memTrans = new MemoryTransaction(MS_BOTH);
    defValue[1] = memTrans;

    /*  Set default signal value array.  */
    dacDataSignal->setData(defValue);

    /*  Request signal from the DAC unit to the Memory Controller.  */
    dacReqSignal = newInputSignal("DACMemoryRequest", 1, 1, NULL);


    /*  Check number of attached Texture Units and Texture Unit prefixes.  */
    GPU_ASSERT(
        if (numTextUnits == 0)
            panic("MemoryController", "MemoryController", "There should be at least a Texture Unit.");

        if (tuPrefixArray == NULL)
            panic("MemoryController", "MemoryController", "The Texture Unit signal name prefix array can not be null.");

        for(i = 0; i < numTextUnits; i++)
        {
            if (tuPrefixArray[i] == NULL)
                panic("MemoryController", "MemoryController", "Texture Unit signal name prefix is null.");
        }
    )

    /*  Allocate Texture Unit signals.  */
    tuDataSignal = new Signal*[numTextUnits];
    tuReqSignal = new Signal*[numTextUnits];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (tuDataSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating Texture Unit data signal array.");

        if (tuReqSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating Texture Unit request signal array.");
    )

    /*  Create signals with the Texture Units.  */
    for(i = 0; i < numTextUnits; i++)
    {
        /*  Data signal from the Memory Controller to a Texture Unit.  */
        tuDataSignal[i] = newOutputSignal("TextureMemoryData", 2, 1, tuPrefixArray[i]);

        /*  Build default signal value array.  */
        memTrans = new MemoryTransaction(MS_BOTH);
        defValue[0] = memTrans;
        memTrans = new MemoryTransaction(MS_BOTH);
        defValue[1] = memTrans;

        /*  Set default signal value array.  */
        tuDataSignal[i]->setData(defValue);

        /*  Request signal from the Texture Unit to the Memory Controller.  */
        tuReqSignal[i] = newInputSignal("TextureMemoryRequest", 1, 1, tuPrefixArray[i]);
    }

    /*  Allocate the signals for the memory module buses.  */
    memoryModuleRequestSignal = new Signal*[gpuMemoryBuses];
    memoryModuleDataSignal = new Signal*[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (memoryModuleRequestSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating memory bus request signal.");
        if (memoryModuleDataSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating memory bus data signal.");
    )

    /*  Create the memory module signals.  */
    for(i = 0; i < gpuMemoryBuses; i++)
    {
        /*  Name of the module signal.  */
        sprintf(signalName, "MemoryModule%02d", i);

        /*  Memory module request signal.  */
        memoryModuleRequestSignal[i] = newOutputSignal(signalName, 1, gpuMemReadLatency, NULL);

        /*  Memory module data signal.  */
        memoryModuleDataSignal[i] = newInputSignal(signalName, 1, gpuMemReadLatency, NULL);
    }

    /*  Allocate the signals for the system memory buses.  */
    mappedMemoryRequestSignal = new Signal*[MAPPED_MEMORY_BUSES];
    mappedMemoryDataSignal = new Signal*[MAPPED_MEMORY_BUSES];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (mappedMemoryRequestSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating system memory bus request signal.");
        if (mappedMemoryDataSignal == NULL)
            panic("MemoryController", "MemoryController", "Error allocating system memory bus data signal.");
    )

    /*  Create the system memory signals.  */
    for(i = 0; i < MAPPED_MEMORY_BUSES; i++)
    {
        /*  Name of the signal for mapped system memory.  */
        sprintf(signalName, "SystemMemory%02d", i);

        /*  System memory request signal.  */
        mappedMemoryRequestSignal[i] = newOutputSignal(signalName, 1, MAPPED_MEMORY_LATENCY, NULL);

        /*  System memory data signal.  */
        mappedMemoryDataSignal[i] = newInputSignal(signalName, 1, MAPPED_MEMORY_LATENCY, NULL);
    }

    /*  Create bandwidth signals.  */
    for(i = 0; i < LASTGPUBUS; i++)
    {
        switch(i)
        {
            case COMMANDPROCESSOR:
            case STREAMERFETCH:
            case STREAMERLOADER:
            case DACB:

                sprintf(busName, "%sBus", busNames[i]);
                memBusIn[i][0] = newInputSignal(busName, 1, 1, NULL);
                memBusOut[i][0] = newOutputSignal(busName, 1, 1, NULL);

                break;

            case TEXTUREUNIT:

                for(j = 0; j < numTextUnits; j++)
                {
                        sprintf(busName, "%sBus%02d", busNames[i], j);
                        memBusIn[i][j] = newInputSignal(busName, 1, 1, NULL);
                        memBusOut[i][j] = newOutputSignal(busName, 1, 1, NULL);
                }

                break;

            case ZSTENCILTEST:
            case COLORWRITE:

                for(j = 0; j < numStampUnits; j++)
                {
                        sprintf(busName, "%sBus%02d", busNames[i], j);
                        memBusIn[i][j] = newInputSignal(busName, 1, 1, NULL);
                        memBusOut[i][j] = newOutputSignal(busName, 1, 1, NULL);
                }

                break;

            case MEMORYMODULE:

                for(j = 0; j < gpuMemoryBuses; j++)
                {
                        sprintf(busName, "%sBus%02d", busNames[i], j);
                        memBusIn[i][j] = newInputSignal(busName, 1, 1, NULL);
                        memBusOut[i][j] = newOutputSignal(busName, 1, 1, NULL);
                }

                break;

            case SYSTEM:

                for(j = 0; j < MAPPED_MEMORY_BUSES; j++)
                {
                        sprintf(busName, "%sBus%02d", busNames[i], j);
                        memBusIn[i][j] = newInputSignal(busName, 1, 1, NULL);
                        memBusOut[i][j] = newOutputSignal(busName, 1, 1, NULL);
                }

                break;

            default:

                panic("MemoryController", "MemoryController", "Unknown bus name.");
                break;
        }
    }

    /*  Set to zero all the memory buses remaining cycle counters.  */
    memoryCycles = 0;

    /*  Sets to zero the sum of system memory buses cycles.  */
    systemCycles = 0;

    /*  Allocate array to store the current transaction for each memory bus.  */
    moduleTrans = new MemoryTransaction*[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (moduleTrans == NULL)
            panic("MemoryController", "MemoryController", "Error allocating array of current transactions in memory buses.");
    )

    /*  Allocate array storing the current commands in the memory module buses.  */
    lastCommand = new MemTransCom[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (lastCommand == NULL)
            panic("MemoryController", "MemoryController", "Error allocating array of last issued commands in memory buses.");
    )

    /*  Allocate array storing the last cycle a transaction was issued in the memory module buses.  */
    lastCycle = new u64bit[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (lastCycle == NULL)
            panic("MemoryController", "MemoryController", "Error allocating array of last cycle a transaction was issued in the memory buses.");
    )

    /*  Allocate array storing the pointer to the current request being processed in each memory bus.  */
    currentBusRequest = new u32bit[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (currentBusRequest == NULL)
            panic("MemoryController", "MemoryController", "Error allocating array storing the pointer to the request being processed in the memory buses.");
    )

    /*  Allocate array storing the number of memory requests being processed in each memory bus.  */
    activeBusRequests = new u32bit[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (activeBusRequests == NULL)
            panic("MemoryController", "MemoryController", "Error allocating array storing the number of memory requests request being processed in the memory buses.");
    )

    /*  Allocate array storing the identifiers for the pages open per bus.  */
    currentPages = new u32bit*[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (currentPages == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per bus array of open pages.");
    )

    /*  Allocating page identifier for each bus.  */
    for(i = 0; i < gpuMemoryBuses; i++)
    {
        /*  Allocate open page identifiers for a bus.  */
        currentPages[i] = new u32bit[gpuMemOpenPages];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (currentPages[i] == NULL)
                panic("MemoryController", "MemoryController", "Error allocating array of open pages for a bus.");
        )
    }

    /*  Reset free buffers counter.  */
    freeReadBuffers = readBufferSize;
    freeWriteBuffers = writeBufferSize;

    /*  Set bus width with the GPU units.  */
    /*
    busWidth[COMMANDPROCESSOR] = comProcBus;
    busWidth[STREAMERFETCH] = streamerFetchBus;
    busWidth[STREAMERLOADER] = streamerLoaderBus;
    busWidth[ZSTENCILTEST] = zStencilBus;
    busWidth[COLORWRITE] = colorWriteBus;
    busWidth[DACB] = dacBus;
    busWidth[TEXTUREUNIT] = textUnitBus;
    */

    MemoryTransaction::setBusWidth(COMMANDPROCESSOR,comProcBus);
    MemoryTransaction::setBusWidth(STREAMERFETCH,streamerFetchBus);
    MemoryTransaction::setBusWidth(STREAMERLOADER,streamerLoaderBus);
    MemoryTransaction::setBusWidth(ZSTENCILTEST,zStencilBus);
    MemoryTransaction::setBusWidth(COLORWRITE,colorWriteBus);
    MemoryTransaction::setBusWidth(DACB,dacBus);
    MemoryTransaction::setBusWidth(TEXTUREUNIT,textUnitBus);
    
    /*  Allocate the memory request buffer.  */
    requestBuffer = new MemoryRequest[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (requestBuffer == NULL)
            panic("MemoryController", "MemoryController", "Error allocating the memory request buffer.");
    )

    /*  Allocate the memory request buffer free entry queue.  */
    freeRequestQ = new u32bit[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (freeRequestQ == NULL)
            panic("MemoryController", "MemoryController", "Error allocating the request buffer free entry queue.");
    )

    /*  Check if shared memory banks is enabled.  */
    if (gpuSharedBanks)
    {
        /*  Use a shared bank request queue.  */
        busQueues = 1;
    }
    else
    {
        /*  One request queue per bank/bus.  */
        busQueues = gpuMemoryBuses;
    }

    /*  Allocate structures for the per bus request queues.  */
    readRequestBusQ = new u32bit*[busQueues];
    writeRequestBusQ = new u32bit*[busQueues];
    numReadBusRequests = new u32bit[busQueues];
    numWriteBusRequests = new u32bit[busQueues];
    numBusRequests = new u32bit[busQueues];
    firstReadReq = new u32bit[busQueues];
    lastReadReq = new u32bit[busQueues];
    firstWriteReq = new u32bit[busQueues];
    lastWriteReq = new u32bit[busQueues];
    consecutiveReadReqs = new u32bit[busQueues];
    consecutiveWriteReqs = new u32bit[busQueues];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (readRequestBusQ == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus read request queues.");
        if (readRequestBusQ == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus write request queues.");
        if (numReadBusRequests == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus read request counters.");
        if (numWriteBusRequests == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus write request counters.");
        if (numBusRequests == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus request counters.");
        if (firstReadReq == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus pointers to the first read request in the read request queues.");
        if (lastReadReq == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus pointers to the last read request in the read request queues.");
        if (firstWriteReq == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus pointers to the first write request in the write request queues.");
        if (lastWriteReq == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus pointers to the last write request in the write request queues.");
        if (consecutiveReadReqs == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus counters for consecutive read requests processed.");
        if (consecutiveWriteReqs == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per GPU memory bus counters for consecutive write requests processed.");
    )

    /*  Allocate the per GPU memory bus request queues.  */
    for(i = 0; i < busQueues; i++)
    {
        /*  Allocate the read and write request queues for the GPU memory bus.  */
        readRequestBusQ[i] = new u32bit[requestQueueSize];
        writeRequestBusQ[i] = new u32bit[requestQueueSize];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (readRequestBusQ[i] == NULL)
                panic("MemoryController", "MemoryController", "Error allocating the per GPU memory bus read request queue.");
            if (writeRequestBusQ[i] == NULL)
                panic("MemoryController", "MemoryController", "Error allocating the per GPU memory bus write request queue.");
        )
    }

    /*  Allocate array to store the current transaction for each system memory bus.  */
    systemTrans = new MemoryTransaction*[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (systemTrans == NULL)
            panic("MemoryController", "MemoryController", "Error allocating array of current transactions in system memory buses.");
    )

    /*  Allocate the system memory request queue.  */
    mappedQueue = new MemoryRequest[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (mappedQueue == NULL)
            panic("MemoryController", "MemoryController", "Error allocating the system memory request queue.");
    )

    /*  Allocate the system memory request queue free list.  */
    freeMapped = new u32bit[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (freeMapped == NULL)
            panic("MemoryController", "MemoryController", "Error allocating the system memory request queue free entry list.");
    )

    /*  Allocate the system memory request ready list.  */
    readyMapped = new u32bit[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (readyMapped == NULL)
            panic("MemoryController", "MemoryController", "Error allocating the system memory request queue ready entries list.");
    )


    /*  Allocate service queues.  */
    serviceQueue = new MemoryRequest[serviceQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (serviceQueue == NULL)
            panic("MemoryController", "MemoryController", "Error allocating the service queue.");
    )

    /*  Set initial dynamic objects used to generate bus bandwidth signals.  */
    for(i = 0; i < LASTGPUBUS; i++)
        for(j = 0; j < MAX_GPU_UNIT_BUSES; j++)
            elemSelect[i][j] = 0;

    /*  Initialize statistics.  */
    for(i = 0; i < LASTGPUBUS; i++)
    {
        sprintf(busName,"%sTransactions", busNames[i]);
        transactions[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"%sReadTransactions", busNames[i]);
        readTrans[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"%sWriteTransactions", busNames[i]);
        writeTrans[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"%sReadBytes", busNames[i]);
        readBytes[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"%sWriteBytes", busNames[i]);
        writeBytes[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");
    }

    preloadTrans = &getSM().getNumericStatistic("PreloadTransactions", u32bit(0), "MemoryController", "MC");
    unusedCycles = &getSM().getNumericStatistic("UnusedCycles", u32bit(0), "MemoryController", "MC");

    dataCycles = new GPUStatistics::Statistic*[gpuMemoryBuses];
    rtowCycles = new GPUStatistics::Statistic*[gpuMemoryBuses];
    wtorCycles = new GPUStatistics::Statistic*[gpuMemoryBuses];
    openPageCycles = new GPUStatistics::Statistic*[gpuMemoryBuses];
    openPages = new GPUStatistics::Statistic*[gpuMemoryBuses];
    gpuBusReadBytes = new GPUStatistics::Statistic*[gpuMemoryBuses];
    gpuBusWriteBytes = new GPUStatistics::Statistic*[gpuMemoryBuses];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (dataCycles == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per gpu memory bus data cycles statistic.");
        if (rtowCycles == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per gpu memory bus read to write penalty cycles statistic.");
        if (wtorCycles == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per gpu memory bus write to read penalty cycles statistic.");
        if (openPageCycles == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per gpu memory bus opening page penalty cycles statistic.");
        if (openPages == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per gpu memory bus number of new open pages statistic.");
        if (gpuBusReadBytes == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per gpu memory bus read bytes statistic.");
        if (gpuBusWriteBytes == NULL)
            panic("MemoryController", "MemoryController", "Error allocating per gpu memory bus write bytes statistic.");
    )

    for(i = 0; i < gpuMemoryBuses; i++)
    {
        sprintf(busName,"DataCycles%02d", i);
        dataCycles[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"ReadToWritePenalty%02d", i);
        rtowCycles[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"WriteToReadPenalty%02d", i);
        wtorCycles[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"OpenPagePenalty%02d", i);
        openPageCycles[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"OpenPages%02d", i);
        openPages[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"ReadBytesMemoryBus%02d", i);
        gpuBusReadBytes[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");

        sprintf(busName,"WriteBytesMemoryBus%02d", i);
        gpuBusWriteBytes[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");
    }

    sysBusReadBytes = &getSM().getNumericStatistic("ReadBytesSystemBus", u32bit(0), "MemoryController", "MC");;
    sysBusWriteBytes = &getSM().getNumericStatistic("WriteBytesSystemBus", u32bit(0), "MemoryController", "MC");;

    for(i = 0; i < MAPPED_MEMORY_BUSES; i++)
    {
        sprintf(busName,"SystemDataCycles%02d", i);
        sysDataCycles[i] = &getSM().getNumericStatistic(busName, u32bit(0), "MemoryController", "MC");
    }

    /*  Precalculate shift for memory buses.  */
    busShift = GPUMath::calculateShift(gpuBankGranurality);

    /*  Precalculate offset mask for memory bus access.  */
    busOffsetMask = GPUMath::buildMask(gpuBankGranurality);

    /*  Precalculate shift for memory page identifier.  */
    pageShift = GPUMath::calculateShift(gpuMemOpenPages * gpuMemoryBuses * gpuMemPageSize);

    /*  Precalculate the shift to get to the memory page bank identifier.  */
    pageBankShift = GPUMath::calculateShift(gpuMemoryBuses * gpuMemPageSize);

    /*  Precalculate the mask to get the memory page bank identifier.  */
    pageBankMask = GPUMath::buildMask(gpuMemOpenPages);

    /*  Reset the memory controller.  */
    reset();
}

void MemoryController::processCommand(u64bit cycle)
{
    DynamicObject* dynObj;
    if ( mcCommSignal->read(cycle, dynObj) )
    {
        MemoryControllerCommand* mcComm = (MemoryControllerCommand *)dynObj;
        switch ( mcComm->getCommand() ) {
            case MCCOM_LOAD_MEMORY:
                loadMemory();
                delete dynObj;
                break;
            case MCCOM_SAVE_MEMORY:
                saveMemory();
                delete dynObj;
                break;
            default:
                panic("MemoryController", "processCommand", "Unsupported command received by Legacy memory controller");
        }
    }
}

/*  Memory Controller simulation function.  */
void MemoryController::clock(u64bit cycle)
{
    _lastCycle = cycle;

    MemoryTransaction *memTrans;
    DynamicObject *busElementAux;
    u32bit i;
    u32bit j;
    //u32bit address;
    //u32bit elemSize;
    //u32bit stride;
    //u8bit *data;
    //u32bit req;
    MemState state;
    //MemState unitState;
    //u32bit currentUnit;

    GPU_DEBUG_BOX( printf("MemoryController => Clock " U64FMT "\n", cycle);)

    // Process memory controller commands coming from CP
    processCommand(cycle);

    /*  Read and dump bus usage signals.  */
    for(i = 0; i < LASTGPUBUS; i++)
    {
        switch(i)
        {
            case COMMANDPROCESSOR:
            case STREAMERFETCH:
            case STREAMERLOADER:
            case DACB:

                memBusOut[i][0]->read(cycle, (DynamicObject *&) busElementAux);

                break;

            case TEXTUREUNIT:

                for(j = 0; j < numTextUnits; j++)
                    memBusOut[i][j]->read(cycle, (DynamicObject *&) busElementAux);

                break;

            case ZSTENCILTEST:
            case COLORWRITE:

                for(j = 0; j < numStampUnits; j++)
                    memBusOut[i][j]->read(cycle, (DynamicObject *&) busElementAux);

                break;

            case MEMORYMODULE:

                for(j = 0; j < gpuMemoryBuses; j++)
                    memBusOut[i][j]->read(cycle, (DynamicObject *&) busElementAux);

                break;

            case SYSTEM:

                for(j = 0; j < MAPPED_MEMORY_BUSES; j++)
                    memBusOut[i][j]->read(cycle, (DynamicObject *&) busElementAux);

                break;

            default:

                panic("MemoryController", "MemoryController", "Unknown bus name.");
                break;
        }
    }

/*if ((cycle > 5024880) && (GPU_MOD(cycle, 1000) == 0))
{
printf("MC " U64FMT " => gpu mem requests %d mapped mem requests %d free read buffers %d free write buffers %d\n",
cycle, numRequests, numReadyMapped, freeReadBuffers, freeWriteBuffers);
for(i = 0; i < gpuMemoryBuses; i++)
printf("MC => mem bus %d requests %d read %d write %d\n", i, numBusRequests[i], numReadBusRequests[i], numWriteBusRequests[i]);
printf("MC => pending services %d\n", pendingServices);
}*/

    /*  Cycle through the Memory controller write connections
        for new commands and data.  */

    /*  Read New Memory Transactions.  */

    /*  Read new memory requests from Command Processor.  */
    if (commProcWriteSignal->read(cycle, (DynamicObject *&) memTrans))
    {
        GPU_DEBUG_BOX(
            printf("MemoryController => Request from the Command Processor.\n");
        )

        /*  Add the memory transaction to the request queue.  */
        receiveTransaction(memTrans);
    }

    /*  Read new memory request from the Streamer Loader Units.  */
    for (i = 0; i < streamerLoaderUnits; i++)
    {
        if (streamLoadReqSignal[i]->read(cycle, (DynamicObject *&) memTrans))
        {
            GPU_DEBUG_BOX(
                printf("MemoryController => Request from the Streamer Loader unit %d. \n", i);
            )

            /*  Add the memory transaction to the request queue.  */
            receiveTransaction(memTrans);
        }
    }

    /*  Read new memory request from Streamer Fetch.  */
    if (streamFetchReqSignal->read(cycle, (DynamicObject *&) memTrans))
    {
        GPU_DEBUG_BOX(
            printf("MemoryController => Request from the Streamer Fetch. \n");
        )

        /*  Add the memory transaction to the request queue.  */
        receiveTransaction(memTrans);
    }

    /*  Read new memory request from Texture Units.  */
    for(i = 0; i < numStampUnits; i++)
    {
        /*  Read new memory request from Z Stencil Test unit.  */
        if (zStencilReqSignal[i]->read(cycle, (DynamicObject *&) memTrans))
        {
            GPU_DEBUG_BOX(
                printf("MemoryController => Request from Z Stencil Test unit %d. \n", i);
            )

            /*  Add the memory transaction to the request queue.  */
            receiveTransaction(memTrans);
        }

        /*  Read new memory request from Color Write unit.  */
        if (colorWriteReqSignal[i]->read(cycle, (DynamicObject *&) memTrans))
        {
            GPU_DEBUG_BOX(
                printf("MemoryController => Request from Color Write unit %d. \n", i);
            )

            /*  Add the memory transaction to the request queue.  */
            receiveTransaction(memTrans);
        }
    }

    /*  Read new memory request from DAC unit.  */
    if (dacReqSignal->read(cycle, (DynamicObject *&) memTrans))
    {
        GPU_DEBUG_BOX( printf("MemoryController "  U64FMT " => Request from DAC unit. \n", cycle); )

        /*  Add the memory transaction to the request queue.  */
        receiveTransaction(memTrans);
    }

    /*  Read new memory request from Texture Units.  */
    for(i = 0; i < numTextUnits; i++)
    {
        if (tuReqSignal[i]->read(cycle, (DynamicObject *&) memTrans))
        {
            GPU_DEBUG_BOX(
                printf("MemoryController => Request from Texture Unit %d. \n", i);
            )

            /*  Add the memory transaction to the request queue.  */
            receiveTransaction(memTrans);
        }
    }

    /*  Update GPU unit buses reserved cycles counters.  */
    for(i = 0; i < (LASTGPUBUS - 2); i++)
    {
        for(j = 0; j < MAX_GPU_UNIT_BUSES; j++)
        {
            /*  Check there is a transmission in progress.  */
            if (busCycles[i][j] > 0)
            {
                GPU_DEBUG_BOX(
                    printf("MemoryController => Bus %s [%d] remaining cycles %d.\n",
                        busNames[i], j, busCycles[i][j]);
                )

                /*  Send bus usage signal.  */
                memBusIn[i][j]->write(cycle, &busElement[i][j][elemSelect[i][j]]);

                /*  Update counter.  */
                busCycles[i][j]--;

                /*  Check end of transmission.  */
                if (busCycles[i][j] == 0)
                {
                    /*  Check if the transmission was a service (READ).  */
                    if (service[i][j])
                    {
                        /*  Liberate service queue entry.  */
                        freeServices++;

                        /*  Clear service in the bus unit.  */
                        service[i][j] = FALSE;

                        /*  Liberate buffer.  */
                        freeReadBuffers++;
                    }
                    else
                    {
                        /*  Current transmission is a request (WRITE).  */

                        /*  Check if the write transaction is for mapped system memory or GPU memory.  */
                        if (mappedTrans[i][j])
                        {
                            /*  Set transmission as finished.  */
                            mappedQueue[currentTrans[i][j]].state = MRS_READY;
                        }
                        else
                        {
                            /*  Set transmission as finished.  */
                            requestBuffer[currentTrans[i][j]].state = MRS_READY;
                        }
                    }
                }
            }
        }
    }

    /*  Send data to GPU buses.  */


    /*  Serve the next request to the GPU units.  */
    serveRequest(cycle);

    /*  Reserve bus for the next transaction to be served.  */
    reserveGPUBus();


    /*  Receive memory data from the memory modules signals. */
    for(i = 0; i < gpuMemoryBuses; i++)
    {
        /*  Receive transactions from a memory module bus.  */
        moduleTransaction(cycle, i);
    }

    /*  Check state and update ongoing memory transactions in the memory buses.  */
    for(i = 0; (i < gpuMemoryBuses) && (memoryCycles > 0); i++)
    {
        updateMemoryBus(cycle, i);
    }

    /*  Start new transactions to the memory modules..  */
    for(i = 0; (i < gpuMemoryBuses) && (numRequests > 0); i++)
    {
        /*  Check if a new transaction can be started.  */
        if (gpuSharedBanks)
        {
            issueTransaction(cycle, i);
        }
        else if (numBusRequests[i] > 0)
        {
            issueTransaction(cycle, i);
        }
        else
        {
            /*  Check if memory modules aren't doing anything.  */
            if (memoryCycles == 0)
            {
                /*  Update statistics.  */
                unusedCycles->inc();
            }
        }
    }

    /*  Receive memory data from the system memory signals. */
    for(i = 0; i < MAPPED_MEMORY_BUSES; i++)
    {
        /*  Receive transactions from a system memory bus.  */
        mappedTransaction(cycle, i);
    }

    /*  Check state and update ongoing memory transactions in the system memory buses.  */
    for(i = 0; (i < MAPPED_MEMORY_BUSES) && (systemCycles > 0); i++)
    {
        updateSystemBus(cycle, i);
    }

    /*  Start new transactions to the memory modules..  */
    for(i = 0; i < MAPPED_MEMORY_BUSES; i++)
    {
        /*  Check if a new transaction can be started.  */
        if (numReadyMapped > 0)
        {
            issueSystemTransaction(cycle);
        }
        else
        {
            /*  Check if system memory modules aren't doing anything.  */
            if (systemCycles == 0)
            {
                /*  Update statistics.  */
                //unusedCycles->inc();
            }
        }
    }

    /*  Send state transaction to the GPU units.  */

    /*  NOTE:  The number of free entries that must be available for the Memory Controller to
        accept transactions is the (latency of the request signal + 1) for each GPU unit that
        doesn't support replication and (the latency + 1) x (number of replicated units) for the
        GPU units that support replication.  In the current implementation only the Texture Unit
        supports replication.  This may change and the formula below will have to be changed.  */

    /*  Determines the Memory Controller state.  If there are enough free entries in the memory request
        queue the Memory Controller can receive new memory requests and may be memory writes.  */
    if ((numFreeRequests >= (2 * (LASTGPUBUS - 5) + 2 * 2 * numStampUnits + 2 * numTextUnits)) &&
        (numFreeMapped >= (2 * (LASTGPUBUS - 5) + 2 * 2 * numStampUnits + 2 * numTextUnits)))
        state = MS_READ_ACCEPT; 
    else
        state = MS_NONE;

    /*  Send status signal to Command Processor.  */
    sendBusState(cycle, COMMANDPROCESSOR, state, commProcReadSignal);

    /*  Send status signal to all the Streamer Loader units.  */
    sendBusState(cycle, STREAMERLOADER, state, streamLoadDataSignal, streamerLoaderUnits);

    /*  Send status signal to Streamer Fetch.  */
    sendBusState(cycle, STREAMERFETCH, state, streamFetchDataSignal);

    /*  Send status signal to all Z Stencil Test units.  */
    sendBusState(cycle, ZSTENCILTEST, state, zStencilDataSignal, numStampUnits);

    /*  Send status signal to all Color Write units.  */
    sendBusState(cycle, COLORWRITE, state, colorWriteDataSignal, numStampUnits);

    /*  Send status signal to DAC.  */
    sendBusState(cycle, DACB, state, dacDataSignal);

    /*  Send bus state to all Texture Units.  */
    sendBusState(cycle, TEXTUREUNIT, state, tuDataSignal, numTextUnits);

}

/*  Determines the state of the data bus with a GPU unit.  */
void MemoryController::sendBusState(u64bit cycle, GPUUnit unit, MemState state, Signal *dataSignal)
{
    MemoryTransaction *memTrans;
    MemState unitState;

    /*  Receives the state of the Memory Controller request queue, which determines if
        read requests can be received from the GPU units.  Then it calculates if there are
        enough buffers to receive a write transaction and if the data bus with the unit is
        not already in use.  The number of required buffers is the latency of the request
        signal of the unit + 1 for all GPU units.  */

    /*  NOTE:  We consider only the units that can write to memory when calculating if there
        are enough free buffers.  Currently Command Processor, Z Stencil Test and Color Write
        are the only units that are allowed to write.  This may change in future implementations
        and my require a change in the implementation of this function.  */

    /*  Determines if in the next cycle the unit bus will be free (only 1 busy cycle remaining).  */
    if ((freeWriteBuffers >= (2 + 2 * 2 * numStampUnits)) && (busCycles[unit][0] < 2)
        && (!reserveBus[unit][0]))
        unitState = (state == MS_NONE)?MS_NONE:MS_BOTH;
    else
        unitState = state;


    /*  Create state transaction for the unit.  */
    memTrans = new MemoryTransaction(unitState);

    /*  Send status signal to GPU unit.  */
    dataSignal->write(cycle, memTrans);
}

/*  Determines the state of the data bus with a GPU unit.  Version for replicated units.  */
void MemoryController::sendBusState(u64bit cycle, GPUUnit unit, MemState state, Signal **dataSignal, u32bit units)
{
    MemoryTransaction *memTrans;
    MemState unitState;
    u32bit i;

    /*  Receives the state of the Memory Controller request queue, which determines if
        read requests can be received from the GPU units.  Then it calculates if there are
        enough buffers to receive a write transaction and if the data bus with the unit is
        not already in use.  */

    /*  NOTE:  We consider only the units that can write to memory when calculating if there
        are enough free buffers.  Currently Command Processor, Z Stencil Test and Color Write
        are the only units that are allowed to write.  This may change in future implementations
        and my require a change in the implementation of this function.  */

    for(i = 0; i < units; i++)
    {
        /*  Determines if in the next cycle the unit bus will be free (only 1 busy cycle remaining).  */
        if ((freeWriteBuffers >= (2 + 2 * 2 * numStampUnits)) && (busCycles[unit][i] < 2)
            && (!reserveBus[unit][i]))
            unitState = (state == MS_NONE)?MS_NONE:MS_BOTH;
        else
            unitState = state;

        memTrans = new MemoryTransaction(unitState);

        /*  Send status signal to DAC.  */
        dataSignal[i]->write(cycle, memTrans);
    }
}



/*  Receive memory transaction from a bus.  */
void MemoryController::receiveTransaction(MemoryTransaction *memTrans)
{
    u32bit req;
    GPUUnit unit;
    u32bit unitID;
    u32bit address;
    u32bit size;
    u8bit *data;

    /*  Get the memory transaction address.  */
    address = memTrans->getAddress();

    switch(memTrans->getCommand())
    {
        case MT_READ_REQ:

            /*  Read request received.  */

            /*  Determine the address space of the transaction.  */
            switch(address & ADDRESS_SPACE_MASK)
            {
                case GPU_ADDRESS_SPACE:

                    GPU_DEBUG_BOX(
                        printf("MC => Received READ transaction address %x size %d unit %s id %d\n",
                            memTrans->getAddress(), memTrans->getSize(), busNames[memTrans->getRequestSource()],
                            memTrans->getUnitID());
                    )

                    /*  Add to the request queue.  */
                    addRequest(memTrans);

                    break;

                case SYSTEM_ADDRESS_SPACE:

                    GPU_DEBUG_BOX(
                        printf("MC => Received READ transaction address %x size %d unit %s id %d\n",
                            memTrans->getAddress(), memTrans->getSize(), busNames[memTrans->getRequestSource()],
                            memTrans->getUnitID());
                    )

                    /*  Add to the mapped memory request queue.  */
                    addMappedRequest(memTrans);

                    break;

                default:

                    panic("MemoryController", "receiveTransaction", "Unsupported address space.");
                    break;

            }

            /*  Update statistics.  */
            readTrans[memTrans->getRequestSource()]->inc();

            break;

        case MT_WRITE_DATA:

            /*  Write received.  */

            /*  NOTE:  SUPPORT FOR MEMORY WRITES FROM REPLICATED UNITS IS NOT IMPLEMENTED YET.
                REQUIRES AN ADDITIONAL TRANSACTION TO ACKNOWLEDGE OR REQUEST AGAIN THE SEND.  */

            /*  Check bus is available.  */
            GPU_ASSERT(
                if (busCycles[memTrans->getRequestSource()][memTrans->getUnitID()] != 0)
                    panic("MemoryController", "receiveTransaction", "Write not allowed, GPU unit bus is busy.");
            )

            /*  Check buffer is available.  */
            GPU_ASSERT(
                if (freeWriteBuffers == 0)
                    panic("MemoryController", "receiveTransaction", "No free write buffer available.");
            )

            /*  Get transaction source unit.  */
            unit = memTrans->getRequestSource();

            /*  Get the source unit identifier.  */
            unitID = memTrans->getUnitID();

            /*  Reserve GPU unit bus.  */
            busCycles[unit][unitID] = memTrans->getBusCycles();

            /*  Reserve buffer.  */
            freeWriteBuffers--;

            /*  Determine the address space of the transaction.  */
            switch(address & ADDRESS_SPACE_MASK)
            {
                case GPU_ADDRESS_SPACE:

                    GPU_DEBUG_BOX(
                        printf("MC => Received WRITE transaction address %x size %d unit %s id %d\n",
                            memTrans->getAddress(), memTrans->getSize(), busNames[memTrans->getRequestSource()],
                            memTrans->getUnitID());
                    )

                    /*  Add to the request queue.  */
                    req = addRequest(memTrans);

                    /*  Mark request memory transaction as still not finished.  */
                    requestBuffer[req].state = MRS_TRANSMITING;

                    /*  Set as a write to GPU memory.  */
                    mappedTrans[unit][unitID] = FALSE;

                    break;


                case SYSTEM_ADDRESS_SPACE:

                    GPU_DEBUG_BOX(
                        printf("MC => Received WRITE transaction address %x size %d unit %s id %d\n",
                            memTrans->getAddress(), memTrans->getSize(), busNames[memTrans->getRequestSource()],
                            memTrans->getUnitID());
                    )

                    /*  Add to the mapped system memory request queue.  */
                    req = addMappedRequest(memTrans);

                    /*  Mark request memory transaction as still not finished.  */
                    mappedQueue[req].state = MRS_TRANSMITING;

                    /*  Set as a write to mapped system memory.  */
                    mappedTrans[unit][unitID] = TRUE;

                    break;

                default:

                    panic("MemoryController", "receiveTransaction", "Unsupported address space.");
                    break;
            }

            /*  Store pointer to the current transaction in the request queue.  */
            currentTrans[unit][unitID] = req;

            /*  Set as a request transaction (not a service transaction).  */
            service[unit][unitID] = FALSE;

            /*  Update dynamic object to use for the unit for bus bandwidth signals.  */
            elemSelect[unit][unitID] = (elemSelect[unit][unitID] + 1) & 0x01;

            /*  Copy cookies from the current transaction to the bus usage object.  */
            busElement[unit][unitID][elemSelect[unit][unitID]].copyParentCookies(*memTrans);

            /*  Update statistics.  */
            writeTrans[memTrans->getRequestSource()]->inc();

            break;

        case MT_PRELOAD_DATA:

            /*  Preload received.  */

            /*  Get read operation size (amount of elements to read).  */
            size = memTrans->getSize();

            /*  Get pointer to the data buffer.  */
            data = memTrans->getData();

            /*  Check data buffer pointer not null.  */
            GPU_ASSERT(
                if (data == NULL)
                    panic("MemoryController", "receiveTransaction", "Undefined data buffer.");
            )

            GPU_DEBUG_BOX(
                printf("MemoryController => Processed MT_PRELOAD_DATA (%x, %d).\n", address, size);
            )

            /*  Determine the address space of the transaction.  */
            switch(memTrans->getAddress() & ADDRESS_SPACE_MASK)
            {
                case GPU_ADDRESS_SPACE:

                    /*  Check memory operation correctness.  */
                    GPU_ASSERT(
                        if (((address & SPACE_ADDRESS_MASK) >= gpuMemorySize) ||
                            (((address & SPACE_ADDRESS_MASK) + size) > gpuMemorySize))
                            panic("MemoryController", "receiveTransaction", "GPU memory operation out of range.");
                    )

                    /*  Copy data to GPU memory.  */
                    memcpy(&gpuMemory[address & SPACE_ADDRESS_MASK], data, size);
                    break;


                case SYSTEM_ADDRESS_SPACE:

                    /*  Check memory operation correctness.  */
                    GPU_ASSERT(
                        if (((address & SPACE_ADDRESS_MASK) >= mappedMemorySize) ||
                            (((address & SPACE_ADDRESS_MASK) + size) > mappedMemorySize))
                            panic("MemoryController", "receiveTransaction", "System Memory operation out of range.");
                    )

                    /*  Copy data to mapped system memory.  */
                    memcpy(&mappedMemory[address & SPACE_ADDRESS_MASK], data, size);

                    break;

                default:

                    panic("MemoryController", "receiveTransaction", "Unsupported address space.");
                    break;
            }

            /*
            cout << "MC => Preload memory: ";
            cout << "HC = " << memTrans->getHashCode(false) << " -> ";
            cout << " HCd = " << memTrans->getHashCode(true) << " -> ";
            memTrans->dump(false);
            cout << "\n";
            */

            /*  Delete the transaction.  */
            delete memTrans;

            /*  Update statistics.  */
            preloadTrans->inc();

            break;

        default:
            panic("MemoryController", "receiveTransaction", "Unsupported transaction type.");
            break;
    }

    /*  Update statistics.  */
    transactions[memTrans->getRequestSource()]->inc();
}


/*  Adds a new request to the request queue.  */
u32bit MemoryController::addRequest(MemoryTransaction *memTrans)
{
    u32bit nextRequest;
    u32bit bus;
    u32bit i;
    u32bit searchReq;
    u32bit searchRangeStart;
    u32bit searchRangeEnd;
    u32bit currentRangeStart;
    u32bit currentRangeEnd;
    bool foundDep;

    /*  Check if request buffer is full.  */
    GPU_ASSERT(
        if (numFreeRequests == 0)
            panic("MemoryController", "addRequest", "Request buffer full.");
    )

    GPU_DEBUG_BOX(
        printf("MemoryController => Added new request.\n");
    )

    /*  Update free request buffer entry counter.  */
    numFreeRequests--;

    /*  Get the next free memory request entry.  */
    nextRequest = freeRequestQ[numFreeRequests];

    /*  Add the new request.  */
    requestBuffer[nextRequest].memTrans = memTrans;
    requestBuffer[nextRequest].state = MRS_READY;
    requestBuffer[nextRequest].waitCycles = 0;
    requestBuffer[nextRequest].transCycles = 0;
    requestBuffer[nextRequest].dependency = false;
    requestBuffer[nextRequest].wakeUpWaitingTrans = false;

    /*  Determine the bus for the transaction.  */
    if (gpuSharedBanks)
        bus = 0;
    else
        bus = address2Bus(memTrans->getAddress());

    /*  Determine the transaction type (read or write).  */
    switch(memTrans->getCommand())
    {
        case MT_READ_REQ:

            /*  Update point to the last entry in the GPU memory bus read request queue.  */
            lastReadReq[bus] = GPU_MOD(lastReadReq[bus] + 1, requestQueueSize);

            /*  Add request to the GPU memory bus read request queue. */
            readRequestBusQ[bus][lastReadReq[bus]] = nextRequest;

//printf("MC => adding read request (%p) for bus %d at %d\n", memTrans, bus, nextRequest);

            /*  Update per GPU memory bus read request counter.  */
            numReadBusRequests[bus]++;

            /*  Get the search range.  */
            searchRangeStart = memTrans->getAddress();
            searchRangeEnd = searchRangeStart + memTrans->getSize() - 1;

            /*  Search the write request bus queue for a previous write to the same address range.  */
            for(i = 0, foundDep = false; (i < numWriteBusRequests[bus]) && (!foundDep); i++)
            {
                /*  Get the next transaction in the write request bus queue (newer transactions first).  */
                searchReq = writeRequestBusQ[bus][GPU_PMOD(lastWriteReq[bus] - i, requestQueueSize)];
                currentRangeStart = requestBuffer[searchReq].memTrans->getAddress();
                currentRangeEnd = currentRangeStart + requestBuffer[searchReq].memTrans->getSize() - 1;

                /*  Check if the transaction */
                if (((currentRangeStart >= searchRangeStart) && (currentRangeStart <= searchRangeEnd)) ||
                    ((currentRangeEnd >= searchRangeStart) && (currentRangeEnd <= searchRangeEnd)) ||
                    ((searchRangeStart >= currentRangeStart) && (searchRangeStart <= currentRangeEnd)) ||
                    ((searchRangeEnd >= currentRangeStart) && (searchRangeEnd <= currentRangeEnd)))
                {

//printf("MC => Adding depending read trans (bank %d) at %d waiting for write transaction %d\n",
//bus, nextRequest, searchReq);
//printf("currentRange [%x, %x] searchRange [%x %x]\n", currentRangeStart, currentRangeEnd, searchRangeStart,
//searchRangeEnd);
                    /*  Set read request as dependant from a pending write request to the same bus.  */
                    requestBuffer[nextRequest].dependency = true;
                    requestBuffer[nextRequest].waitForTrans = searchReq;

                    /*  Set write request to wake up dependant transactions.  */
                    requestBuffer[searchReq].wakeUpWaitingTrans = true;
                }
            }

            break;

        case MT_WRITE_DATA:

            /*  Update point to the last entry in the GPU memory bus read request queue.  */
            lastWriteReq[bus] = GPU_MOD(lastWriteReq[bus] + 1, requestQueueSize);

            /*  Add request to the GPU memory bus read request queue. */
            writeRequestBusQ[bus][lastWriteReq[bus]] = nextRequest;

//printf("MC => adding write request (%p) for bus %d at %d\n", memTrans, bus, nextRequest);

            /*  Update per GPU memory bus write request counter.  */
            numWriteBusRequests[bus]++;

            /*  Get the search range.  */
            searchRangeStart = memTrans->getAddress();
            searchRangeEnd = searchRangeStart + memTrans->getSize() - 1;

            /*  Search the read request bus queue for a previous read to the same address range.  */
            for(i = 0, foundDep = false; (i < numReadBusRequests[bus]) && (!foundDep); i++)
            {
                /*  Get the next transaction in the read request bus queue (newer transactions first).  */
                searchReq = readRequestBusQ[bus][GPU_PMOD(lastReadReq[bus] - i, requestQueueSize)] ;
                currentRangeStart = requestBuffer[searchReq].memTrans->getAddress();
                currentRangeEnd = currentRangeStart + requestBuffer[searchReq].memTrans->getSize() - 1;

                /*  Check if the transaction */
                if (((currentRangeStart >= searchRangeStart) && (currentRangeStart <= searchRangeEnd)) ||
                    ((currentRangeEnd >= searchRangeStart) && (currentRangeEnd <= searchRangeEnd)) ||
                    ((searchRangeStart >= currentRangeStart) && (searchRangeStart <= currentRangeEnd)) ||
                    ((searchRangeEnd >= currentRangeStart) && (searchRangeEnd <= currentRangeEnd)))
                {
//printf("MC => Adding depending write trans (bank %d) at %d waiting for read transaction %d\n",
//bus, nextRequest, searchReq);
//printf("currentRange [%x, %x] searchRange [%x %x]\n", currentRangeStart, currentRangeEnd, searchRangeStart,
//searchRangeEnd);
                    /*  Set write request as dependant from a pending read request to the same bus.  */
                    requestBuffer[nextRequest].dependency = true;
                    requestBuffer[nextRequest].waitForTrans = searchReq;

                    /*  Set read request to wake up dependant transactions.  */
                    requestBuffer[searchReq].wakeUpWaitingTrans = true;
                }
            }

            break;
    }

    /*  Update per GPU memory bus request counter.  */
    numBusRequests[bus]++;

    /*  Update global pending requests counter.  */
    numRequests++;

    return nextRequest;
}

/*  Adds a new request to the request queue.  */
u32bit MemoryController::addMappedRequest(MemoryTransaction *memTrans)
{
    u32bit nextFreeEntry;

    /*  Check if queue is full.  */
    GPU_ASSERT(
        if (numFreeMapped == 0)
            panic("MemoryController", "addMappedRequest", "Mapped memory request queue full.");
    )

    GPU_DEBUG_BOX( printf("MemoryController => Added new mapped request.\n"); )

    /*  Update free request entries counter.  */
    numFreeMapped--;

    /*  Get the next free memory request entry.  */
    nextFreeEntry = freeMapped[numFreeMapped];

    /*  Add the new request.  */
    mappedQueue[nextFreeEntry].memTrans = memTrans;
    mappedQueue[nextFreeEntry].state = MRS_READY;
    mappedQueue[nextFreeEntry].waitCycles = 0;
    mappedQueue[nextFreeEntry].transCycles = 0;

    /*  Update last request pointer.  */
    lastMapped = GPU_MOD(lastMapped + 1, requestQueueSize);

    /*  Add request to the FIFO. */
    readyMapped[lastMapped] = nextFreeEntry;

    /*  Update ready requests counter.  */
    numReadyMapped++;

    return nextFreeEntry;
}

/*  Wake up read transactions for a memory bus when a write transaction source of dependencies is processed.  */
void MemoryController::wakeUpReadRequests(u32bit bus, u32bit writeTrans)
{
    u32bit i;
    u32bit wakeUpReq;

//printf("wake up read requests bus %d writeTrans %d\n", bus, writeTrans);

    /*  Search for read transactions waiting for the write transaction.  */
    for(i = 0; i < numReadBusRequests[bus]; i++)
    {
        /*  Get the next transaction in the read request bus queue (newer transactions first).  */
        wakeUpReq = readRequestBusQ[bus][GPU_PMOD(lastReadReq[bus] - i, requestQueueSize)];

        /*  Check if the current read transaction was waiting for the write transaction to be processed.  */
        if (requestBuffer[wakeUpReq].dependency && (requestBuffer[wakeUpReq].waitForTrans == writeTrans))
        {
//printf("waking up write trans %d\n", wakeUpReq);
            /*  Set read transaction as ready.  */
            requestBuffer[wakeUpReq].dependency = false;
        }
    }
}

/*  Wake up write transactions for a memory bus when a read transaction source of dependencies is processed.  */
void MemoryController::wakeUpWriteRequests(u32bit bus, u32bit readTrans)
{
    u32bit i;
    u32bit wakeUpReq;

//printf("wake up write requests bus %d readTrans %d\n", bus, readTrans);

    /*  Search for write transactions waiting for the read transaction.  */
    for(i = 0; i < numWriteBusRequests[bus]; i++)
    {
        /*  Get the next transaction in the write request bus queue (newer transactions first).  */
        wakeUpReq = writeRequestBusQ[bus][GPU_PMOD(lastWriteReq[bus] - i, requestQueueSize)];

        /*  Check if the current write transaction was waiting for the read transaction to be processed.  */
        if (requestBuffer[wakeUpReq].dependency && (requestBuffer[wakeUpReq].waitForTrans == readTrans))
        {
//printf("waking up write trans %d\n", wakeUpReq);
            /*  Set write transaction as ready.  */
            requestBuffer[wakeUpReq].dependency = false;
        }
    }
}

/*  Removes a request from thequeue.  */
void MemoryController::removeRequest(u32bit req)
{
//printf("MC => removing request (%p) at %d\n", requestBuffer[req].memTrans, req);

    /*  Add request buffer entry to the free request queue.  */
    freeRequestQ[numFreeRequests] = req;

    /*  Update free request buffer entry counter.  */
    numFreeRequests++;
}

/*  Removes a request from the queue.  */
void MemoryController::removeMappedRequest(u32bit req)
{
    /*  Add request entry to the free request entries list.  */
    freeMapped[numFreeMapped] = req;

    /*  Update free entries counter.  */
    numFreeMapped++;
}

/*  Selects the next memory request to process for a GPU memory bus.  */
u32bit MemoryController::selectNextRequest(u32bit bus)
{
    u32bit req;
    bool readyReadReq;
    bool readyWriteReq;

    /*  Check if there are pending requests for the GPU memory bus.  */
    GPU_ASSERT(
        if (numBusRequests[bus] == 0)
            panic("MemoryController", "selectNextRequest", "No pending requests for the GPU memory bus.");
    )

    /*  Check if there are pending read requests for the GPU memory bus.  */
    if (numReadBusRequests[bus] > 0)
    {
        /*  Determine if the next read request is ready.  */
        req = readRequestBusQ[bus][firstReadReq[bus]];
        readyReadReq = !requestBuffer[req].dependency;
    }
    else
    {
        /*  No ready read requests for the bus.  */
        readyReadReq = false;
    }

    /*  Check if there are pending write requests for the GPU memory bus.  */
    if (numWriteBusRequests[bus] > 0)
    {
        /*  Determine if the next write request is ready.  */
        req = writeRequestBusQ[bus][firstWriteReq[bus]];
        readyWriteReq = !requestBuffer[req].dependency;
    }
    else
    {
        /*  No ready write requests for the bus.  */
        readyWriteReq = false;
    }

    /*  Check if there are ready read transactions in the bus request queue.  */
    if (readyReadReq)
    {
        /*  Check number of consecutive read requests processed and if there are pending write requests.  */
        if (readyWriteReq &&
            (((consecutiveWriteReqs[bus] > 1) && (consecutiveWriteReqs[bus] < maxConsecutiveWrites)) ||
            (consecutiveReadReqs[bus] == maxConsecutiveReads)))
        {
            /*  Select the next write request for the bus.  */
            req = writeRequestBusQ[bus][firstWriteReq[bus]];

//printf("MC => selecting write request (%p) for bus %d at %d\n", requestBuffer[req].memTrans, bus, req);
        }
        else
        {
            /*  Select the next read request for the bus.  */
            req = readRequestBusQ[bus][firstReadReq[bus]];

//printf("MC => selecting read request (%p) for bus %d at %d\n", requestBuffer[req].memTrans, bus, req);
        }
    }
    else if (readyWriteReq)
    {
        /*  Check if there are pending write requests for the GPU memory bus.  */
        GPU_ASSERT(
            if (numWriteBusRequests[bus] == 0)
            {
                panic("MemoryController", "selectNextRequest", "No write or read pendeing request for the GPU memory bus.");
            }
        )

        /*  Select the next write request for the bus.  */
        req = writeRequestBusQ[bus][firstWriteReq[bus]];

//printf("MC => selecting write request (%p) for bus %d at %d\n", requestBuffer[req].memTrans, bus, req);
    }
    else
    {
        //printf("Deadlock? Bus %d Read Requests %d Write Requests %d\n", bus, numReadBusRequests[bus], numWriteBusRequests[bus]);

        GPU_ASSERT(
            if ((activeBusRequests[bus] == 0)
                && (numWriteBusRequests[bus] > 0) && (numReadBusRequests[bus] > 0))
            {
                panic("MemoryController", "selectNextTrans", "Deadlock, both request queues are blocked for the bus.");
            }
        )

        req = requestQueueSize;
    }

    return req;
}

/*  Updates the read request queue pointers and counters for a GPU memory bus after the selected
    read request has been processed.  */
void MemoryController::updateReadRequests(u32bit bus)
{
    /*  Update first read request pointer.  */
    firstReadReq[bus] = GPU_MOD(firstReadReq[bus] + 1, requestQueueSize);

    /*  Update number of read request for the GPU memory bus.  */
    numReadBusRequests[bus]--;

    /*  Update number of requests for the GPU memory bus.  */
    numBusRequests[bus]--;

    /*  Update global request counter.  */
    numRequests--;

    /*  Reset number of consecutive write requests processed for the GPU memory bus.  */
    consecutiveWriteReqs[bus] = 0;

    /*  Update the number of consecutive read requests processed for the GPU memory bus.  */
    consecutiveReadReqs[bus]++;
}

/*  Updates the write request queue pointers and counters for a GPU memory bus after the selected
    write request has been processed.  */
void MemoryController::updateWriteRequests(u32bit bus)
{
    /*  Update pointer to the first write request for the GPU memory bus.  */
    firstWriteReq[bus] = GPU_MOD(firstWriteReq[bus] + 1, requestQueueSize);

    /*  Update number of pending write requests for the GPU memory bus.  */
    numWriteBusRequests[bus]--;

    /*  Update number of pending requests for the GPU memory bus.  */
    numBusRequests[bus]--;

    /*  Update global request counter.  */
    numRequests--;

    /*  Reset number of consecutive read requests processed for the GPU memory bus.  */
    consecutiveReadReqs[bus] = 0;

    /*  Update the number of consecutive write requests processed for the GPU memory bus.  */
    consecutiveWriteReqs[bus]++;
}

/*  Tries to issue a transaction to memory.  */
void MemoryController::issueTransaction(u64bit cycle, u32bit bus)
{
    u32bit req;
    u32bit address;
    u32bit size;
    u8bit *data;
    u32bit i;
    MemoryTransaction *memTrans;
    MemoryTransaction *auxMemTrans;
    u32bit source, dest;
    u32bit *mask;
    u32bit bank;
    bool pageHit;   

    GPU_ASSERT(
        if (bus >= gpuMemoryBuses)
            panic("MemoryController", "issueTransaction", "Memory bus identifier out of range.");
    )

    GPU_DEBUG_BOX( printf("MemoryController => Issue a new request.\n"); )

    /*  Determine if shared gpu memory banks mode is disabled.  */
    if (!gpuSharedBanks)
    {
        /*  Set the bank for the current gpu memory bus.  */
        bank = bus;
    }
    else
    {
        /*  Set shared bank.  */
        bank = 0;
    }

    /*  Select the next request f*/
    req = selectNextRequest(bank);

    /*  Check if there is any ready transaction for the bus.  */
    if (req == requestQueueSize)
        return;

    /*  Get memory transaction.  */
    memTrans = requestBuffer[req].memTrans;

    /*  Check memory transaction bank.  */
    GPU_ASSERT(
        if (!gpuSharedBanks && (bank != address2Bus(memTrans->getAddress())))
            panic("MemoryController", "issueTransaction", "Memory transaction from a wrong bank.");
        if (!gpuSharedBanks && (((memTrans->getAddress() & busOffsetMask) + memTrans->getSize()) > gpuBankGranurality))
        {
printf("MC " U64FMT " > address %x size %d usOffset %x unit %s id %d\n",
    cycle, memTrans->getAddress(), memTrans->getSize(), (memTrans->getAddress() & busOffsetMask),
    busNames[memTrans->getRequestSource()], memTrans->getUnitID());
            panic("MemoryController", "issueTransaction", "Memory transaction crosses bank boundary.");
        }
    )

    /*  Search the page in current open memory page directory for the memory bus.  */
    pageHit = searchPage(memTrans->getAddress(), bank);

    /*  Check transaction type.  */
    switch(memTrans->getCommand())
    {
        case MT_READ_REQ:

        /*  Memory read request.  */

        /*  Check if there is a write-to-read transaction penalty.  */
        if ((pageHit || ((cycle - lastCycle[bus]) >= gpuMemPageOpenLat)) &&
            (((lastCommand[bus] == MT_READ_REQ) && ((cycle - lastCycle[bus]) >= TRANSACTION_CYCLES)) ||
            ((lastCommand[bus] == MT_WRITE_DATA) &&
            ((cycle - lastCycle[bus]) >= (TRANSACTION_CYCLES + gpuMemWriteLatency + gpuMemWriteToReadLatency)))))
        {
            /*  Check if there are enough resources to start the transaction.  */
            if  ((freeReadBuffers > 0) && (freeServices > 0))
            {

                /*  Get start address.  */
                address = memTrans->getAddress();

                /*  Get read operation size (amount of elements to read).  */
                size = memTrans->getSize();

                /*  Get pointer to the data buffer.  */
                data = memTrans->getData();

                GPU_DEBUG_BOX
                (
                    printf("MemoryController " U64FMT " => Issuing MT_READ_REQ (%x, %d) from unit %s id %d to bus %d.\n", cycle, address, size,
                       busNames[memTrans->getRequestSource()], memTrans->getUnitID(), bus);
                )

                /*  Check memory operation correctness.  */
                GPU_ASSERT(
                    if ((address >= gpuMemorySize) ||
                        ((address + size) > gpuMemorySize))
                    {
                        printf("MC " U64FMT " => checking error\n", cycle);

                        printf("MT_READ_REQ address %x size %d unit %s unit ID %d\n",
                            address, size, busNames[memTrans->getRequestSource()], memTrans->getUnitID());
                        printf("GPUMemory: %x\n", gpuMemorySize);

                        panic("MemoryController", "issueTransaction", "GPU Memory operation out of range.");
                    }
                )

                /*  Check for unallocated memory.  */
                //GPU_ASSERT(
                //    if (*((u32bit *) &gpuMemory[address & SPACE_ADDRESS_MASK]) == 0xDEADCAFE)
                //    {
                //        printf("MC " U64FMT " => Warning.  Unallocated memory -> MT_READ_REQ : address %x size %d unit %s unitID %d to bus %d\n", cycle, address, size,
                //            busNames[memTrans->getRequestSource()], memTrans->getUnitID(), bus);
                //        //panic("MemoryController", "issueTransaction", "Reading unallocated memory.");
                //    }
                //)

                /*  Read the data into the data buffer.  */
                memcpy(data, &gpuMemory[address & SPACE_ADDRESS_MASK], size);

                /*  Create a new memory transaction with the read data.  */
                auxMemTrans = new MemoryTransaction(memTrans);

                /*  Set the request entry for the memory transaction.  */
                auxMemTrans->setRequestID(req);

                /*  Simulate memory module access latency writting through a signal.  */
                memoryModuleRequestSignal[bus]->write(cycle, auxMemTrans, gpuMemReadLatency);

                /*  Reserve a read buffer for the read transaction.  */
                freeReadBuffers--;

                /*  Reserve a service queue entry for the processed request.  */
                freeServices--;

                /*  Update the number of active requests in the bus.  */
                activeBusRequests[bank]++;

                /*  Update the GPU memory bus read request queue.  */
                updateReadRequests(bank);

                /*  Delete request memory transaction.  */
                delete memTrans;

                /*  If no hit on the open page directory open the new page. */
                if (!pageHit)
                {
                    /*  Open the new page.  */
                    openPage(address, bank);

                    /*  Update statistics.  */
                    openPages[bus]->inc();
                }

                /*  Set last command.  */
                lastCommand[bus] = MT_READ_REQ;

                /*  Set last cycle.  */
                lastCycle[bus] = cycle;
            }
        }
        else
        {
            /*  Check if a transaction could have been issued because of data transmission latency.  */
            if ((cycle - lastCycle[bus]) >= TRANSACTION_CYCLES)
            {
                /*  Check if a READ transaction couldn't be issued because latency with a previous
                    WRITE transaction.  */
                if ((lastCommand[bus] == MT_WRITE_DATA) &&
                    ((cycle - lastCycle[bus]) < (TRANSACTION_CYCLES + gpuMemWriteLatency + gpuMemWriteToReadLatency)))
                {
                    /*  Update statistics.  */
                    //wtorCycles[bus]->inc();
                    wtorCycles[address2Bus(memTrans->getAddress())]->inc();
                }
                else
                {
                    /*  Next transaction READ transaction could have been issued but there
                        was a page miss.  */

                    /*  Update statistics.  */
                    openPageCycles[address2Bus(memTrans->getAddress())]->inc();
                }
            }
        }

        break;

    case MT_WRITE_DATA:

        /*  Memory write request.  */

        /*  Check if the transaction can be started.  */
        if (requestBuffer[req].state == MRS_READY)
        {
            /*  Check if there is a read-to-write penalty.  */
            if ((pageHit || ((cycle - lastCycle[bus]) >= gpuMemPageOpenLat)) &&
                (((lastCommand[bus] == MT_WRITE_DATA) && ((cycle - lastCycle[bus]) >= TRANSACTION_CYCLES)) ||
                ((lastCommand[bus] == MT_READ_REQ) && ((cycle - lastCycle[bus]) >= (TRANSACTION_CYCLES + gpuMemReadLatency)))))
            {

                /*  Get start address.  */
                address = memTrans->getAddress();

                /*  Get read operation size (amount of elements to read).  */
                size = memTrans->getSize();

                /*  Get pointer to the data buffer.  */
                data = memTrans->getData();

                /*  Check memory operation correctness.  */
                GPU_ASSERT(
                    if ((address >= gpuMemorySize) ||
                        ((address + size) > gpuMemorySize))
                    {
                        printf("MC " U64FMT " => checking error\n", cycle);

                        printf("MT_WRITE_DATA address %x size %d unit %s unit ID %d\n",
                            address, size, busNames[memTrans->getRequestSource()], memTrans->getUnitID());
                        printf("GPUMemory: %x\n", gpuMemorySize);

                        panic("MemoryController", "issueTransaction", "GPU Memory operation out of range.");
                    }
                )

                /*  Check data buffer pointer not null.  */
                GPU_ASSERT(
                    if (data == NULL)
                        panic("MemoryController", "issueTransaction", "Undefined data buffer.");
                )

                GPU_DEBUG_BOX(
                    printf("MemoryController " U64FMT " => Issuing MT_WRITE (%x, %d)"
                    "from unit %s id %d to bus %d.\n", cycle, address, size,
                        busNames[memTrans->getRequestSource()], memTrans->getUnitID(), bus);
                )

                /*  Check if it is a masked write.  */
                if (memTrans->isMasked())
                {
                    GPU_DEBUG_BOX(
                        printf("MemoryController => Masked write.\n");
                    )

                    /*  Get write mask.  */
                    mask = memTrans->getMask();

                    /*  Write only unmasked bytes.  */
                    for(i = 0; i < size; i = i + 4)
                    {
                        /*  Get source data.  */
                        source = *((u32bit *) &gpuMemory[(address & SPACE_ADDRESS_MASK) + i]);

                        /*  Delete unmasked bytes.  */
                        source = source & ~mask[i >> 2];

                        /*  Get destination data.  */
                        dest = *((u32bit *) &data[i]);

                        /*  Delete masked bytes.  */
                        dest = dest & mask[i >> 2];

                        /*  Write to memory.  */
                        *((u32bit *) &gpuMemory[(address & SPACE_ADDRESS_MASK) + i]) = source | dest;
                    }
                }
                else
                {
                    /*  Copy data to memory.  */
                    memcpy(&gpuMemory[address & SPACE_ADDRESS_MASK], data, size);
                }


                /*  Set the request entry for the memory transaction.  */
                memTrans->setRequestID(req);

                /*  Simulate memory module access latency.  */
                memoryModuleRequestSignal[bus]->write(cycle, memTrans, gpuMemWriteLatency);

                /*  Update the number of active requests in the bus.  */
                activeBusRequests[bank]++;

                /*  Advance write request queue for the bus.  */
                updateWriteRequests(bank);

                /*  If no hit on the open page directory open the new page. */
                if (!pageHit)
                {
                    /*  Open the new page.  */
                    openPage(address, bank);

                    /*  Update statistics.  */
                    openPages[bus]->inc();
                }

                /*  Set last command.  */
                lastCommand[bus] = MT_WRITE_DATA;

                /*  Set last cycle.  */
                lastCycle[bus] = cycle;

            }
            else
            {
                /*  Check if a transaction could have been issued because of data transmission latency.  */
                if ((cycle - lastCycle[bus]) >= TRANSACTION_CYCLES)
                {
                    /*  Check if the next WRITE transaction couldn't be issued because latency
                        with a previous READ transaction.  */
                    if ((lastCommand[bus] == MT_READ_REQ) && ((cycle - lastCycle[bus]) < (TRANSACTION_CYCLES + gpuMemReadLatency)))
                    {
                        /*  Update statistics.  */
                        //rtowCycles[bus]->inc();
                        rtowCycles[address2Bus(memTrans->getAddress())]->inc();
                    }
                    else
                    {
                        /*  Next WRITE transaction could have been issued but there was a page miss.  */

                        /*  Update statistics.  */
                        openPageCycles[address2Bus(memTrans->getAddress())]->inc();
                    }
                }
            }
        }

        break;

    default:
        panic("MemoryController", "issueTransaction", "Unsupported memory transaction.");
        break;

    }

}

/*  Tries to issue a transaction to system memory.  */
void MemoryController::issueSystemTransaction(u64bit cycle)
{
    u32bit req;
    u32bit address;
    u32bit size;
    u8bit *data;
    u32bit i;
    MemoryTransaction *memTrans;
    MemoryTransaction *auxMemTrans;
    u32bit source, dest;
    u32bit *mask;

    /*  NOTE:  Two unidirectional buses, one for writes and one for reads.
        In current implementation bus 0 is used for reads and bus 1 for writes.  */

    GPU_DEBUG_BOX( printf("MemoryController => Issue a new request.\n"); )

    /*  Get first request from the ready request FIFO.  */
    req = readyMapped[firstMapped];

    /*  Get memory transaction.  */
    memTrans = mappedQueue[req].memTrans;

    /*  Check transaction type.  */
    switch(memTrans->getCommand())
    {
        case MT_READ_REQ:

        /*  Memory read request.  */

        /*  Check if there are enough resources to start the transaction.  */
        if  (((cycle - systemBus[0]) >= MAPPED_TRANSACTION_CYCLES) && (freeReadBuffers > 0) &&
            (freeServices > 0))
        {
            /*  Get start address.  */
            address = memTrans->getAddress();

            /*  Get read operation size (amount of elements to read).  */
            size = memTrans->getSize();

            /*  Get pointer to the data buffer.  */
            data = memTrans->getData();

            GPU_DEBUG_BOX(
//if (cycle > 1769000)
                printf("MemoryController => Issuing MT_READ_REQ (%x, %d) to system memory bus %d.\n", address, size, 0);
            )

            /*  Check memory operation correctness.  */
            GPU_ASSERT(
                if (((address & SPACE_ADDRESS_MASK) >= mappedMemorySize) ||
                    (((address & SPACE_ADDRESS_MASK) + size) > mappedMemorySize))
                {
                    printf("MC " U64FMT " => checking error\n", cycle);

                    printf("MT_READ_REQ address %x size %d unit %s unit ID %d\n",
                        address, size, busNames[memTrans->getRequestSource()], memTrans->getUnitID());

                    panic("MemoryController", "issueSystemTransaction", "Mapped memory operation out of range.");
                }
            )

            /*  Check for unallocated memory.  */
            //GPU_ASSERT(
                //if (*((u32bit *) &mappedMemory[address & SPACE_ADDRESS_MASK]) == 0xDEADCAFE)
                //{
                //   printf("MC " U64FMT " =>  Warning.  Unallocated memory -> MT_READ_REQ : address %x size %d unit %s unitID %d\n", cycle, address, size,
                //        busNames[memTrans->getRequestSource()], memTrans->getUnitID());
                //   panic("MemoryController", "issueSystemTransaction", "Reading unallocated memory.");
                //}
            //)

            /*  Read the data into the data buffer.  */
            memcpy(data, &mappedMemory[address & SPACE_ADDRESS_MASK], size);

            /*  Create a new memory transaction with the read data.  */
            auxMemTrans = new MemoryTransaction(memTrans);

            /*  Simulate system memory access latency writting through a signal.  */
            mappedMemoryRequestSignal[0]->write(cycle, auxMemTrans, MAPPED_MEMORY_LATENCY);

//if (cycle > 1769000)
//printf("MC " U64FMT " => memTrans %p\n", cycle, auxMemTrans);

            /*  Set bus as not available.  */
            systemBus[0] = cycle;

            /*  Reserve a read buffer for the read transaction.  */
            freeReadBuffers--;

            /*  Reserve a service queue entry for the processed read request.  */
            freeServices--;

            /*  Update first request pointer.  */
            firstMapped = GPU_MOD(firstMapped + 1, requestQueueSize);

            /*  Update ready request counter.  */
            numReadyMapped--;

            /*  Remove the transaction from the request queue.  */
            removeMappedRequest(req);

            /*  Delete request memory transaction.  */
            delete memTrans;
        }

        break;

    case MT_WRITE_DATA:

        /*  Memory write request.  */

        /*  Check if the transaction can be started.  */
        if (((cycle - systemBus[1]) >= MAPPED_TRANSACTION_CYCLES) && (mappedQueue[req].state == MRS_READY))
        {
            /*  Get start address.  */
            address = memTrans->getAddress();

            /*  Get read operation size (amount of elements to read).  */
            size = memTrans->getSize();

            /*  Get pointer to the data buffer.  */
            data = memTrans->getData();

            /*  Check memory operation correctness.  */
            GPU_ASSERT(
                if (((address & SPACE_ADDRESS_MASK) >= mappedMemorySize) ||
                    (((address & SPACE_ADDRESS_MASK) + size) > mappedMemorySize))
                {
                    printf("MC " U64FMT " => checking error\n", cycle);
                    printf("MT_WRITE_DATA address %x size %d unit %s unit ID %d\n",
                        address, size, busNames[memTrans->getRequestSource()], memTrans->getUnitID());

                    panic("MemoryController", "issueSystemTransaction", "Mapped system memory operation out of range.");
                }
            )

            /*  Check data buffer pointer not null.  */
            GPU_ASSERT(
                if (data == NULL)
                    panic("MemoryController", "issueSystemTransaction", "Undefined data buffer.");
            )

            GPU_DEBUG_BOX(
//if (cycle > 1769000)
                printf("MemoryController => Issuing MT_WRITE (%x, %d) to system memory bus %d.\n", address, size, 0);
            )

            /*  Check if it is a masked write.  */
            if (memTrans->isMasked())
            {
                GPU_DEBUG_BOX(
                    printf("MemoryController => Masked write.\n");
                )

                /*  Get write mask.  */
                mask = memTrans->getMask();

                /*  Write only unmasked bytes.  */
                for(i = 0; i < size; i = i + 4)
                {
                    /*  Get source data.  */
                    source = *((u32bit *) &mappedMemory[(address & SPACE_ADDRESS_MASK) + i]);

                    /*  Delete unmasked bytes.  */
                    source = source & ~mask[i >> 2];

                    /*  Get destination data.  */
                    dest = *((u32bit *) &data[i]);

                    /*  Delete masked bytes.  */
                    dest = dest & mask[i >> 2];

                    /*  Write to memory.  */
                    *((u32bit *) &mappedMemory[(address & SPACE_ADDRESS_MASK) + i]) = source | dest;
                }
            }
            else
            {
                /*  Copy data to memory.  */
                memcpy(&mappedMemory[address & SPACE_ADDRESS_MASK], data, size);
            }

            /*  Simulate system memory access latency.  */
            mappedMemoryRequestSignal[1]->write(cycle, memTrans, MAPPED_MEMORY_LATENCY);

//if (cycle > 1769000)
//printf("MC " U64FMT " => memTrans %p\n", cycle, auxMemTrans);

            /*  Set system bus as not available.  */
            systemBus[1] = cycle;

            /*  Update first request pointer.  */
            firstMapped = GPU_MOD(firstMapped + 1, requestQueueSize);

            /*  Update ready request counter.  */
            numReadyMapped--;

            /*  Remove the transaction from the request queue.  */
            removeMappedRequest(req);
        }

        break;

    default:
        panic("MemoryController", "issueSystemTransaction", "Unsupported memory transaction.");
        break;

    }
}


/*  Reset the Memory Controller state.  */
void MemoryController::reset()
{
    u32bit i;
    u32bit j;
    u32bit buses;

    /*  Check shared banks.  */
    if (gpuSharedBanks)
        buses = 1;
    else
        buses = gpuMemoryBuses;

    /*  Reset per GPU memory bus request queue pointers and counters.  */
    for(i = 0; i < buses; i++)
    {
        /*  Initialize the GPU memory queue pointers.  */
        firstReadReq[i] = 1;
        lastReadReq[i] = 0;
        firstWriteReq[i] = 1;
        lastWriteReq[i] = 0;
        numReadBusRequests[i] = 0;
        numWriteBusRequests[i] = 0;
        numBusRequests[i] = 0;
        consecutiveReadReqs[i] = 0;
        consecutiveWriteReqs[i] = 0;
    }

    numRequests = 0;

    /*  Initialize the GPU memory free request entry list.  */
    numFreeRequests = requestQueueSize;

    /*  Reset the free GPU memory request buffer entry queue.  */
    for(i = 0; i < requestQueueSize; i++)
        freeRequestQ[i] = i;

    /*  Initialize the system memory queue pointers.  */
    firstMapped = 1;
    lastMapped = 0;
    numReadyMapped = 0;

    /*  Initialize the GPU memory free request entry list.  */
    numFreeMapped = requestQueueSize;

    /*  Reset the free GPU memory request queue list.  */
    for(i = 0; i < requestQueueSize; i++)
        freeMapped[i] = i;

    /*  Initialize bus reserved cycles counters.  */
    for(i = 0; i < LASTGPUBUS; i++)
    {
        for(j = 0; j < MAX_GPU_UNIT_BUSES; j++)
            busCycles[i][j] = 0;
    }

    /*  Reset memory bus state.  */
    for(i = 0; i < gpuMemoryBuses; i++)
    {
        /*  Reset last issued transaction command.  */
        lastCommand[i] = MT_READ_REQ;

        /*  Reset last issued transaction cycle.  */
        lastCycle[i] = 0;

        /*  No requests in the buses.  */
        currentBusRequest[i] = requestQueueSize;
        activeBusRequests[i] = 0;

        /*  Reset open page directory.  */
        for(j = 0; j < gpuMemOpenPages; j++)
            currentPages[i][j] = 0;
    }

    /*  Reset gpu memory cycles.  */
    memoryCycles = 0;

    /*  Reset system memory cycles.  */
    systemCycles = 0;

    /*  Reset system buses availability flags.  */
    for(i = 0; i < MAPPED_MEMORY_BUSES; i++)
        systemBus[i] = 0;

    /*  Reset service queue free entries counter.  */
    freeServices = serviceQueueSize;

    /*  Reset pending services counter.  */
    pendingServices = 0;

    /*  Reset service queue next free entry pointer.  */
    nextFreeService = 0;

    /*  Reset service queue next entry pointer.  */
    nextService = 0;

    /*  Reset service queue structures.  */
    for(i = 0; i < (LASTGPUBUS - 2); i++)
    {
        /*  Reset all the buses in the unit.  */
        for(j = 0; j < MAX_GPU_UNIT_BUSES; j++)
        {
            /*  Reset bus service flag.  */
            service[i][j] = FALSE;

            /*  Reset reserved bus flag.  */
            reserveBus[i][j] = FALSE;
        }
    }
}

/*  Service memory request.  */
void MemoryController::serveRequest(u64bit cycle)
{
    MemoryTransaction *memTrans;
    GPUUnit unit;
    u32bit unitID;

    /*  Check if there are pending services in the queue.  */
    if ((pendingServices > 0) /*&& (serviceQueue[nextService].state == MRS_WAITING)*/)
    {
        /*  Get the pending memory transaction to serve.  */
        memTrans = serviceQueue[nextService].memTrans;

        /*  Get the GPU unit that is going to be served.  */
        unit = memTrans->getRequestSource();
        unitID = memTrans->getUnitID();

        /*  Check if the unit bus is free and there is a service in the queue.  */
        if (busCycles[unit][unitID] == 0)
        {
            /*  NOTE:  ONLY WORKS FOR 1 CYCLE STATE SIGNAL LATENCY.  IF LARGER
                LATENCY IS TO BE USED A COUNTER (NUMBER OF WRITE ACCEPTS ISSUED)
                IS NEEDED TO WAIT N CYCLES OR UNTIL A WRITE REQUEST IS RECEIVED
                BEFORE THOSE N CYCLES.  */

            /*  Check if bus was reserved.  */
            if (reserveBus[unit][unitID])
            {
                /*  Send the data received from the memory modules to the GPU
                    unit that issued the read request..  */

                /*  Set service as in transmission.  */
                serviceQueue[nextService].state = MRS_TRANSMITING;
            
                /*  Send data to the unit using the corresponding signal.  */
                switch(unit)
                {
                    case COMMANDPROCESSOR:

                        commProcReadSignal->write(cycle, memTrans, 1);
                        break;

                    case STREAMERLOADER:

                        streamLoadDataSignal[unitID]->write(cycle, memTrans, 1);
                        break;

                    case DACB:

                        dacDataSignal->write(cycle, memTrans, 1);
                        break;

                    case STREAMERFETCH:

                        streamFetchDataSignal->write(cycle, memTrans, 1);
                        break;

                    case ZSTENCILTEST:

                        zStencilDataSignal[unitID]->write(cycle, memTrans, 1);
                        break;

                    case COLORWRITE:

                        colorWriteDataSignal[unitID]->write(cycle, memTrans, 1);
                        break;

                    case TEXTUREUNIT:

                        tuDataSignal[unitID]->write(cycle, memTrans, 1);
                        break;
                }

                GPU_DEBUG_BOX(
                    printf("MemoryController %I64d => Response %d to the GPU Unit %s[%d].  Bus cycles %d.\n",
                        cycle, memTrans->getID(), busNames[unit], unitID, memTrans->getBusCycles());
                )

                /*  Reserve unit bus cycles.  */
                busCycles[unit][unitID] = memTrans->getBusCycles();

                /*  Update dynamic object to use for the unit for bus bandwidth signals.  */
                elemSelect[unit][unitID] = (elemSelect[unit][unitID] + 1) & 0x01;

                /*  Copy cookies from the memory transaction to the bus usage object.  */
                busElement[unit][unitID][elemSelect[unit][unitID]].copyParentCookies(*memTrans);

                /*  Set service in unit bus.  */
                service[unit][unitID] = TRUE;

                /*  Update number of ready services in the unit.  */
                pendingServices--;

                /*  Unreserve bus.  */
                reserveBus[unit][unitID] = FALSE;

                /*  Update next service pointer.  */
                nextService = GPU_MOD(nextService + 1, serviceQueueSize);
            }
        }
    }
}

/*  Reserves a GPU unit bus for the next service.  */
void MemoryController::reserveGPUBus()
{
    MemoryTransaction *memTrans;
    GPUUnit unit;
    u32bit unitID;

    /*  Check if there are pending services in the unit queue.  */
    if (pendingServices > 0)
    {
        /*  Get the next transaction to be served.  */
        memTrans = serviceQueue[nextService].memTrans;

        /*  Get the unit identifier.  */
        unit = memTrans->getRequestSource();
        unitID = memTrans->getUnitID();

        /*  Check bus is busy until the next cycle.  */
        if (busCycles[unit][unitID] <= 1)
        {
            /*  Reserve the bus for the next cycle.  */
            reserveBus[unit][unitID] = TRUE;
        }
    }
}

/*  Processes a transaction received back from the memory module.  */
void MemoryController::moduleTransaction(u64bit cycle, u32bit bus)
{
    MemoryTransaction *memTrans;

    GPU_ASSERT(
        if (bus >= gpuMemoryBuses)
            panic("MemoryController", "moduleTransaction", "Memory bus identifier out of range.");
    )

    /*  Checks if there is a transaction coming from the module bus.  */
    if (memoryModuleDataSignal[bus]->read(cycle, (DynamicObject *&) memTrans))
    {
        /*  Check if the memory bus is still busy.  */
        GPU_ASSERT(
            if (busCycles[MEMORYMODULE][bus] > 0)
                panic("MemoryController", "moduleTransaction", "Memory bus busy.");
        )

        GPU_DEBUG_BOX(
            printf("MemoryController => Transaction from memory module %d.\n", bus);
        )

        /*  Set memory bus reserved cycles.  */
        busCycles[MEMORYMODULE][bus] = TRANSACTION_CYCLES;
        memoryCycles += TRANSACTION_CYCLES;

        /*  Set the current transaction in the bus.  */
        moduleTrans[bus] = memTrans;

        /*  Set current request being processed in the bus.  */
        currentBusRequest[bus] = memTrans->getRequestID();

        /*  Process transaction.  */
        switch(memTrans->getCommand())
        {
            case MT_READ_DATA:

                /*  Add to service queue moved to the end of the transmission in updateMemory().  */

                break;

            case MT_WRITE_DATA:

                /*  Nothing to do after the transaction ends.  */

                break;

            default:
                panic("MemoryController", "moduleTransaction", "Unsupported memory transaction.");
                 break;
        }
    }
}

/*  Updates the state of a memory module bus.  */
void MemoryController::updateMemoryBus(u64bit cycle, u32bit bus)
{
    GPUUnit currentUnit;
    u32bit bank;

    GPU_ASSERT(
        if (bus >= gpuMemoryBuses)
            panic("MemoryController", "updateMemoryBus", "Memory bus identifier out of range.");
    )

    /*  Check if there is a data transmission in the bus.  */
    if (busCycles[MEMORYMODULE][bus] > 0)
    {
        /*  Copy cookies from the memory transaction object in the bus.  */
        busElement[MEMORYMODULE][bus][0].copyParentCookies(*moduleTrans[bus]);

        /*  Send bus usage signal.  */
        memBusIn[MEMORYMODULE][bus]->write(cycle, &busElement[MEMORYMODULE][bus][0]);

        /*  Decrement remaining cycle counter for the current transaction.  */
        memoryCycles--;
        busCycles[MEMORYMODULE][bus]--;

        /*  Update statistics.  */
        //dataCycles[bus]->inc();
        dataCycles[address2Bus(moduleTrans[bus]->getAddress())]->inc();

        GPU_DEBUG_BOX(
            printf("MemoryController => Transmiting data bus %d.  Remaining cycles %d.\n", bus, busCycles[MEMORYMODULE][bus]);
        )

        currentUnit = moduleTrans[bus]->getRequestSource();

        /*  Check end of transaction.  */
        if (busCycles[MEMORYMODULE][bus] == 0)
        {
            GPU_DEBUG_BOX( printf("MemoryController => End of transmission.\n"); )

            /*  Determine the bank for the transaction.  */
            if (gpuSharedBanks)
                bank = 0;
            else
                bank = bus;

            /*  Terminate current transaction.  */
            switch(moduleTrans[bus]->getCommand())
            {
                case MT_READ_DATA:

                    /*  Check if there is a free service entry.  */
                    GPU_ASSERT(
                        if (freeServices < 0)
                            panic("MemoryController", "updateMemoryBus", "No free service entry available.");
                    )

                    /*  Get current unit.  */
                    currentUnit = moduleTrans[bus]->getRequestSource();

                    GPU_DEBUG_BOX(
                        printf("MemoryController => Storing transaction for unit %d service queue entry %d.\n",
                            currentUnit, nextFreeService);
                    )

                    /*  Adds the memory transaction to the service queue.  */
                    serviceQueue[nextFreeService].memTrans = moduleTrans[bus];
                    serviceQueue[nextFreeService].state = MRS_WAITING;
                    serviceQueue[nextFreeService].waitCycles = 0;
                    serviceQueue[nextFreeService].transCycles = 0;

                    /*  Update free service pointer.  */
                    nextFreeService = GPU_MOD(nextFreeService + 1, serviceQueueSize);

                    /*  Update ready service entry counter.  */
                    pendingServices++;

                    /*  Check if the read request was a source of dependencies.  */
                    if (requestBuffer[currentBusRequest[bus]].wakeUpWaitingTrans)
                    {
                        /*  Wake Up write transactions waiting for the read transaction.  */
                        wakeUpWriteRequests(bank, currentBusRequest[bus]);
                    }

//printf("MC " U64FMT " => remove MT_READ_DATA request %p bus %d\n", cycle, moduleTrans[bus], currentBusRequest[bus]);

                    /*  Remove request.  */
                    removeRequest(currentBusRequest[bus]);

                    /*  No request in the bus.  */
                    currentBusRequest[bus] = requestQueueSize;

                    /*  Update number of active requests.  */
                    activeBusRequests[bus]--;

                    /*  Update statistics.  */
                    readBytes[currentUnit]->inc(moduleTrans[bus]->getSize());
                    //gpuBusReadBytes[bus]->inc(moduleTrans[bus]->getSize());
                    gpuBusReadBytes[address2Bus(moduleTrans[bus]->getAddress())]->inc(moduleTrans[bus]->getSize());

                    break;

                case MT_WRITE_DATA:

                    /*  Liberate buffer.  */
                    freeWriteBuffers++;

                    /*  Check if the write request was a source of dependencies.  */
                    if (requestBuffer[currentBusRequest[bus]].wakeUpWaitingTrans)
                    {
                        /*  Wake Up read transactions waiting for the write transaction.  */
                        wakeUpReadRequests(bank, currentBusRequest[bus]);
                    }

//printf("MC " U64FMT " => remove MT_WRITE_DATA request %p bus %d\n", cycle, moduleTrans[bus], currentBusRequest[bus]);

                    /*  Remove request.  */
                    removeRequest(currentBusRequest[bus]);

                    /*  No request in the bus.  */
                    currentBusRequest[bus] = requestQueueSize;

                    /*  Update number of active requests.  */
                    activeBusRequests[bus]--;

                    /*  Update statistics.  */
                    writeBytes[currentUnit]->inc(moduleTrans[bus]->getSize());
                    //gpuBusWriteBytes[bus]->inc(moduleTrans[bus]->getSize());
                    gpuBusWriteBytes[address2Bus(moduleTrans[bus]->getAddress())]->inc(moduleTrans[bus]->getSize());

                    /*  Delete transaction.  */
                    delete moduleTrans[bus];

                    break;

                default:
                    panic("MemoryController", "updateMemoryBus", "Unsupported memory transaction.");
                    break;
            }
        }
    }
}


/*  Updates the state of a system memory bus.  */
void MemoryController::updateSystemBus(u64bit cycle, u32bit bus)
{
    GPUUnit currentUnit;

    GPU_ASSERT(
        if (bus >= MAPPED_MEMORY_BUSES)
            panic("MemoryController", "updateSystemBus", "System memory bus identifier out of range.");
    )

    /*  Check if there is a data transmission in the bus.  */
    if (busCycles[SYSTEM][bus] > 0)
    {
        /*  Copy cookies from the memory transaction object in the bus.  */
        busElement[SYSTEM][bus][0].copyParentCookies(*systemTrans[bus]);

        /*  Send bus usage signal.  */
        memBusIn[SYSTEM][bus]->write(cycle, &busElement[SYSTEM][bus][0]);

        /*  Decrement remaining cycle counter for the current transaction.  */
        systemCycles--;
        busCycles[SYSTEM][bus]--;

        /*  Update statistics.  */
        sysDataCycles[bus]->inc();

        GPU_DEBUG_BOX(
            printf("MemoryController => Transmiting system data bus %d.  Remaining cycles %d.\n", bus, busCycles[SYSTEM][bus]);
        )

        currentUnit = systemTrans[bus]->getRequestSource();

//if (cycle > 1769740)
//printf("MC " U64FMT " => update -> memTrans %p\n", cycle, systemTrans[bus]);

        /*  Check end of transaction.  */
        if (busCycles[SYSTEM][bus] == 0)
        {
            GPU_DEBUG_BOX( printf("MemoryController => End of transmission.\n"); )

            /*  Terminate current transaction.  */
            switch(systemTrans[bus]->getCommand())
            {
                case MT_READ_DATA:

                    /*  Check if there is a free service entry.  */
                    GPU_ASSERT(
                        if (freeServices < 0)
                            panic("MemoryController", "updateSystemBus", "No free service entry available.");
                    )

                    GPU_DEBUG_BOX(
                        printf("MemoryController => Storing system transaction for unit %d in service queue entry %d.\n",
                            systemTrans[bus]->getRequestSource(), nextFreeService);
                    )

                    /*  Adds the memory transaction to the service queue.  */
                    serviceQueue[nextFreeService].memTrans = systemTrans[bus];
                    serviceQueue[nextFreeService].state = MRS_MEMORY;
                    serviceQueue[nextFreeService].waitCycles = 0;
                    serviceQueue[nextFreeService].transCycles = 0;

                    /*  Update free service pointer.  */
                    nextFreeService = GPU_MOD(nextFreeService + 1, serviceQueueSize);

                    /*  Update ready service entry counter.  */
                    pendingServices++;

                    /*  Update statistics.  */
                    readBytes[currentUnit]->inc(systemTrans[bus]->getSize());
                    sysBusReadBytes->inc(systemTrans[bus]->getSize());

                    break;

                case MT_WRITE_DATA:

                    /*  Liberate buffer.  */
                    freeWriteBuffers++;

                    /*  Update statistics.  */
                    writeBytes[currentUnit]->inc(systemTrans[bus]->getSize());
                    sysBusWriteBytes->inc(systemTrans[bus]->getSize());

                    /*  Delete transaction.  */
                    delete systemTrans[bus];

                    break;


                default:
printf(" MC "  U64FMT  " >> transaction %p command %d data %d\n", cycle, systemTrans[bus], systemTrans[bus]->getCommand(),
*((u32bit *) systemTrans[bus]));
                    panic("MemoryController", "updateSystemBus", "Unsupported memory transaction.");
                    break;
            }
        }

        /*  Update statistics.  */
        //dataCycles->inc();
    }
}


/*  Processes a transaction received back from the mapped system memory bus.  */
void MemoryController::mappedTransaction(u64bit cycle, u32bit bus)
{
    MemoryTransaction *memTrans;
    GPUUnit currentUnit;

    GPU_ASSERT(
        if (bus >= MAPPED_MEMORY_BUSES)
            panic("MemoryController", "mappedTransaction", "Memory bus identifier out of range.");
    )

    /*  Checks if there is a transaction coming from the system memory bus.  */
    if (mappedMemoryDataSignal[bus]->read(cycle, (DynamicObject *&) memTrans))
    {
        /*  Check if the system bus is still busy.  */
        GPU_ASSERT(
            if (busCycles[SYSTEM][bus] > 0)
                panic("MemoryController", "mappedTransaction", "System memory bus busy.");
        )

        GPU_DEBUG_BOX(
//if (cycle > 1769000)
            printf("MemoryController " U64FMT " => Transaction %p from system memory bus %d.\n", cycle, memTrans, bus);
        )

        /*  Set system memory bus reserved cycles.  */
        busCycles[SYSTEM][bus] = MAPPED_TRANSACTION_CYCLES;
        systemCycles += MAPPED_TRANSACTION_CYCLES;

        /*  Get current unit.  */
        currentUnit = memTrans->getRequestSource();

        /*  Store current memory transaction in the memory bus.  */
        systemTrans[bus] = memTrans;

        /*  Process transaction.  */
        switch(memTrans->getCommand())
        {
            case MT_READ_DATA:

                /*  Add to service queue moved to the end of the transmission in updateMemory().  */
                break;

            case MT_WRITE_DATA:

                /*  Nothing to do after the transaction ends.  */

                break;

            default:
                panic("MemoryController", "mappedTransaction", "Unsupported memory transaction.");
                break;
        }
    }
}

/*  Search the open page table for bus.  */
bool MemoryController::searchPage(u32bit address, u32bit bus)
{
    u32bit pageID;
    u32bit pageBankID;
    bool hit;

    /*  Get the page bank identifier.  */
    pageBankID = (address >> pageBankShift) & pageBankMask;

    /*  Get the page identifier.  */
    pageID = address >> pageShift;

    /*  Check if the page for the requested address is open in the page bank.  */
    hit = (currentPages[bus][pageBankID] == pageID);

//printf(">>> searchPage => bus %d address %x pageID %x hit %s dirPos %d dirPageID %x pageShift %d\n",
//    bus, address, pageID, hit?"T":"F", pageBankID, currentPages[bus][pageBankID], pageShift);

    return hit;
}

/*  Update per bus open page table with a new page.  */
void MemoryController::openPage(u32bit address, u32bit bus)
{
    u32bit pageID;
    u32bit pageBankID;

    /*  Get the page bank identifier.  */
    pageBankID = (address >> pageBankShift) & pageBankMask;

    /*  Get the page identifier.  */
    pageID = address >> pageShift;

    /*  Set the new page in the LRU position.  */
    currentPages[bus][pageBankID] = pageID;
}

/*  Calculates the gpu memory bus accessed for a given gpu space memory address.  */
u32bit MemoryController::address2Bus(u32bit address)
{
    return GPU_MOD((address & SPACE_ADDRESS_MASK) >> busShift, gpuMemoryBuses);
}

//  List the debug commands supported by the Command Processor
void MemoryController::getCommandList(std::string &commandList)
{
    commandList.append("savememory  - Saves GPU and system memory to snapshot files.\n");
    commandList.append("loadmemory  - Loads GPU and system memmory from snapshot files.\n");
    commandList.append("_savememory - Saves GPU and system memory to snapshot files (silent).\n");
    commandList.append("_loadmemory - Loads GPU and system memmory from snapshot files (silent).\n");
}

//  Execute a debug command
void MemoryController::execCommand(stringstream &commandStream)
{
    string command;

    commandStream >> ws;
    commandStream >> command;

    if (!command.compare("savememory"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "savememory" << endl;
        }
        else
        {
            cout << " Saving memory to file.\n";
            saveMemory();
        }
    }
    else if (!command.compare("loadmemory"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "loadmemory" << endl;
        }
        else
        {
            cout << " Loading memory from file.\n";
            loadMemory();
        }
    }            
    else if (!command.compare("savememory"))
    {
        saveMemory();
    }
    else if (!command.compare("loadmemory"))
    {
        loadMemory();
    }
}

//  Save GPU and system memory into a file.
void MemoryController::saveMemory()
{
    ofstream out;

    //  Open/create snapshot file for the gpu memory.
    out.open("gpumem.snapshot", ios::binary);
    
    //  Check if file was open/created correctly.
    if (!out.is_open())
    {
        panic("MemoryController", "saveMemory", "Error creating gpu memory snapshot file.");
    }
    
    //  Dump the gpu memory content into the file.
    out.write((char *) gpuMemory, gpuMemorySize);

    //  Close the file.
    out.close();


    //  Open/create snapshot file for the system memory.
    out.open("sysmem.snapshot", ios::binary);
    
    //  Check if file was open/created correctly.
    if (!out.is_open())
    {
        panic("MemoryController", "saveMemory", "Error creating system memory snapshot file.");
    }
    
    //  Dump the system content into the file.
    out.write((char *) mappedMemory, mappedMemorySize);

    //  Close the file.
    out.close();
}

//  Load GPU and system memory from a file.
void MemoryController::loadMemory()
{
    ifstream in;

    //  Open snapshot file for the gpu memory.
    in.open("gpumem.snapshot", ios::binary);
    
    //  Check if file was open/created correctly.
    if (!in.is_open())
    {
        panic("MemoryController", "loadMemory", "Error opening gpu memory snapshot file.");
    }
    
    //  Load the gpu memory content from the file.
    in.read((char *) gpuMemory, gpuMemorySize);

    //  Close the file.
    in.close();


    //  Open snapshot file for the system memory.
    in.open("sysmem.snapshot", ios::binary);
    
    //  Check if file was open correctly.
    if (!in.is_open())
    {
        panic("MemoryController", "loadMemory", "Error opening system memory snapshot file.");
    }
    
    //  Load the system content from the file.
    in.read((char *) mappedMemory, mappedMemorySize);

    //  Close the file.
    in.close();
}


/*  GPU Unit to Memory Controller data bus width (default values).  */
/*
u32bit gpu3d::busWidth[] =
{
    8,      //  Data bus with the Command Processor (AGP)
    16,     //  Data bus with the Streamer Fetch unit (indexes)
    16,     //  Data bus with the Streamer Loader unit (attributes)
    32,     //  Data bus with the Z Stencil Test unit (pixels depth/stencil)
    32,     //  Data bus with the Color Write unit (pixels color)
    8,      //  Data bus with the DAC unit (pixels color)
    16,     //  Data bus with the Texture Unit
    16,     //  Data bus with the memory modules
    8,      //  Data bus with system memory
    0       //  Not used
};
*/

/*  Names of the GPU units for each of the Memory Controller buses.  */
/*
char* gpu3d::busNames[] =
{
    "CommandProcessor",
    "StreamerFetch",
    "StreamerLoader",
    "ZStencilTest",
    "ColorWrite",
    "DAC",
    "TextureUnit",
    "Memory",
    "System",
    "LAST BUS"
};
*/

void MemoryController::getDebugInfo(string &debugInfo) const
{
    stringstream ss;
    ss << 
        "MEMORY CONTROLLER LEGACY (debug info) \n"
        "   Current cycle: " << _lastCycle << "\n"
        "   Request buffer entries in use (max: " << requestQueueSize << "): " << requestQueueSize - numFreeRequests << "\n"
        "   System request buffer entries in use (max: " << requestQueueSize << ") : " << requestQueueSize - numFreeMapped << "\n"
        "   Service queue entries in use (max: " << serviceQueueSize << "): " << pendingServices << "\n"
        "   Free services: " << freeServices << "\n"
        "   Free read buffers: " << freeReadBuffers << "\n"
        "   Free write buffers: " << freeWriteBuffers << "\n"
        "";

    ss << "   BUS RESERVATION STATE:\n";
    for ( u32bit i = 0; i < LASTGPUBUS; ++i ) 
    {
        bool busReserved = false;
        for ( u32bit j = 0; j < MAX_GPU_UNIT_BUSES; ++j ) 
        {
            if ( reserveBus[i][j] ) {
                if ( !busReserved ) {
                    ss << "    " << MemoryTransaction::getBusName(static_cast<GPUUnit>(i)) << "\n";
                    busReserved = true;
                }
                ss << "    " << "  [" << j << "] = RESERVED\n";
            }
        }
    }
    
    debugInfo = ss.str();
}


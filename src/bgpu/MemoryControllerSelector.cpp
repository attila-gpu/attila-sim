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
 */

#include "MemoryControllerSelector.h"
#include "MemoryController.h"
#include "MemoryControllerV2.h"
#include <iostream>
#include "GPUMemorySpecs.h"


using namespace gpu3d;

Box* gpu3d::createMemoryController(SimParameters& simP, 
                            const char** tuPrefixes, 
                            const char** suPrefix,
                            const char** slPrefixes,
                            const char* memoryControllerName,
                            Box* parentBox)
{
    using std::cout;
    using std::endl;
    
    u32bit gpuMemSize;
    u32bit mappedMemSize;
    
    GPU_ASSERT(
        if (simP.mem.memSize > 2048)
            panic("gpu3d", "createMemoryController", "Maximum GPU memory size allowed is 2 GBs.");
        if (simP.mem.mappedMemSize > 2048)
            panic("gpu3d", "createMemoryController", "Maximum mapped (system) memory size allowed is 2 GBs.");
    )
    
    //  The memory size coming from the configuration files is in MBs but the Memory Controller expects it in
    //  bytes (for now).
    gpuMemSize = simP.mem.memSize * 1024 * 1024;
    mappedMemSize = simP.mem.mappedMemSize * 1024 * 1024;
    
    Box* memController;
    if ( !simP.mem.memoryControllerV2 )
    {
        // Select legacy memory controller
        cout << "Using legacy Memory Controller..." << endl;
        memController = new MemoryController(
            gpuMemSize,                 /*  GPU Memory Size.  */
            simP.mem.clockMultiplier,   /*  Frequency multiplier applied to the GPU clock to be used as the reference memory frequency.  */
            simP.mem.memoryFrequency,   /*  Memory frequency in cycles from the reference memory frequency.  */
            simP.mem.busWidth,          /*  Bus width in bits.  */
            simP.mem.memBuses,          /*  Number of buses to the memory modules.  */
            simP.mem.sharedBanks,       /*  Flag that enables shared access to all memory banks from all gpu buses.  */
            simP.mem.bankGranurality,   /*  Access granurality to gpu memory banks.  */
            simP.mem.burstLength,       /*  Number of words (bus width) transfered per memory access in a gpu bus.  */
            simP.mem.readLatency,       /*  Cycles from read command to data from GPU memory.  */
            simP.mem.writeLatency,      /*  Cycles from write command to data towards GPU memory.  */
            simP.mem.writeToReadLat,    /*  Cycles from last written data to GPU memory to next read command.  */
            simP.mem.memPageSize,       /*  Size in bytes of a GPU memory page.  */
            simP.mem.openPages,         /*  Number of pages open per memory bus.  */
            simP.mem.pageOpenLat,       /*  Latency of opening a new page.  */
            simP.mem.maxConsecutiveReads,   /*  Number of consecutive read transactions before the next write transaction for a bus.  */
            simP.mem.maxConsecutiveWrites,  /*  Number of consecutive write transactions before the next read transaction for a bus.  */
            simP.mem.comProcBus,        /*  Bus with the Command Processor (bytes).  */
            simP.mem.strFetchBus,       /*  Bus with the Streamer Fetch (bytes).   */
            simP.mem.strLoaderBus,      /*  Bus with the Streamer Loader (bytes).  */
            simP.mem.zStencilBus,       /*  Bus with Z Stencil Test (bytes).  */
            simP.mem.cWriteBus,         /*  Bus with Color Write (bytes).  */
            simP.mem.dacBus,            /*  Bus with DAC (bytes).  */
            simP.mem.textUnitBus,       /*  Texture Unit bus (bytes).  */
            mappedMemSize,              /*  Amount of system memory mapped into the GPU address space.  */
            simP.mem.readBufferLines,   /*  Memory buffer lines for read transactions.  */
            simP.mem.writeBufferLines,  /*  Memory buffer lines for write transactions.  */
            simP.mem.reqQueueSz,        /*  Request queue size.  */
            simP.mem.servQueueSz,       /*  Service queue size.  */
            simP.gpu.numStampUnits,     /*  Number of Stamp Units attached.  */
            suPrefix,                   /*  Array of prefixes for the Stamp Units.  */
            simP.gpu.numFShaders * simP.fsh.textureUnits,   /*  Number of Texture Units attached.  */
            tuPrefixes,                 /*  Array of prefixes for the Texture Units.  */
            simP.str.streamerLoaderUnits,/*  Number of Streamer Loader units.  */
            slPrefixes,                 /*  Array of prefixes for the Streamer Loader units.  */
            "MemoryController",
            NULL);
    }
    else
    {
        cout << "Using Memory Controller Version 2..." << endl;

        using memorycontroller::MemoryControllerParameters;

        MemoryControllerParameters::SchedulerType schedType;
        MemoryControllerParameters::SchedulerPagePolicy schedPagePolicy;

        schedType = (MemoryControllerParameters::SchedulerType)simP.mem.v2ChannelScheduler;

        if ( simP.mem.v2PagePolicy == 0 )
            schedPagePolicy = MemoryControllerParameters::ClosePage;
        else if ( simP.mem.v2PagePolicy == 1 )
            schedPagePolicy = MemoryControllerParameters::OpenPage;

        memorycontroller::MemoryControllerParameters params;

        // default to 0
        memset(&params, '\0', sizeof(memorycontroller::MemoryControllerParameters));

        string memtype;
        if ( simP.mem.v2MemoryType == 0 ) {
            cout << "MCv2 -> No V2MemoryType defined, defaulting to GDDR3" << endl;
            memtype = "gddr3";            
        }
        else {
            memtype = simP.mem.v2MemoryType;
        }        
        
        if ( memtype == "gddr3" ) {  
            if ( simP.mem.v2GDDR_Profile == 0 ) {
                cout << "MCv2 -> No V2GDDR_Profile found for GDDR3 memory, defaulting to V2GDDR_Profile=\"600\"" << endl;
                params.memSpecs = new memorycontroller::GDDR3Specs600MHz();
            }
            else {
                string profile(simP.mem.v2GDDR_Profile);
                if ( profile == "perfect" ) {
                    cout << "MCv2 -> Using 'PERFECT MEMORY' profile (NO DELAY)\n";
                    params.memSpecs = new memorycontroller::GDDR3SpecsZeroDelay();
                }
                else if ( profile == "custom" ) {
                    params.memSpecs = new memorycontroller::GDDR3SpecsCustom( simP.mem.v2GDDR_tRRD,
                                                                              simP.mem.v2GDDR_tRCD,
                                                                              simP.mem.v2GDDR_tWTR,
                                                                              simP.mem.v2GDDR_tRTW,
                                                                              simP.mem.v2GDDR_tWR,
                                                                              simP.mem.v2GDDR_tRP,
                                                                              simP.mem.v2GDDR_CAS,
                                                                              simP.mem.v2GDDR_WL );
                }
                else if ( profile == "600" ) {
                    params.memSpecs = new memorycontroller::GDDR3Specs600MHz();
                }
                else {
                    stringstream ss;
                    ss << "Memory profile \"" << profile << "\" not supported";
                    panic("MemoryControllerSelector", "createMemoryController", ss.str().c_str());
                }
            }
        }
        else {
            stringstream ss;
            ss << "Memory type \"" << memtype << "\" not supported";
            panic("MemoryControllerSelector", "createMemoryController", ss.str().c_str());
        }

        params.burstLength = simP.mem.burstLength;
        params.comProcBus = simP.mem.comProcBus;
        params.streamerFetchBus = simP.mem.strFetchBus;
        params.streamerLoaderBus = simP.mem.strLoaderBus;
        params.zStencilTestBus = simP.mem.zStencilBus;
        params.colorWriteBus = simP.mem.cWriteBus;
        params.dacBus = simP.mem.dacBus;
        params.textUnitBus = simP.mem.textUnitBus;
        params.gpuMemorySize = gpuMemSize;
        params.systemMemorySize = mappedMemSize;
        params.systemMemoryBuses = memorycontroller::SYSTEM_MEMORY_BUSES;
        params.systemMemoryReadLatency = memorycontroller::SYSTEM_MEMORY_READ_LATENCY;
        params.systemMemoryWriteLatency = memorycontroller::SYSTEM_MEMORY_WRITE_LATENCY;
        params.readBuffers = simP.mem.readBufferLines;
        params.writeBuffers = simP.mem.writeBufferLines;
        params.requestQueueSize = simP.mem.reqQueueSz;
        params.serviceQueueSize = simP.mem.servQueueSz;
        params.numStampUnits = simP.gpu.numStampUnits;
        params.numTexUnits = simP.gpu.numFShaders * simP.fsh.textureUnits;
        params.streamerLoaderUnits = simP.str.streamerLoaderUnits;

        params.perBankChannelQueues = simP.mem.v2UseChannelRequestFIFOPerBank;
        params.perBankChannelQueuesSelection = simP.mem.v2ChannelRequestFIFOPerBankSelection;

        params.memoryChannels = simP.mem.v2MemoryChannels;
        params.banksPerMemoryChannel = simP.mem.v2BanksPerMemoryChannel;
        params.memoryRowSize = simP.mem.v2MemoryRowSize;
        params.burstBytesPerCycle = simP.mem.v2BurstBytesPerCycle;
        params.channelInterleaving = simP.mem.v2ChannelInterleaving;
        params.bankInterleaving = simP.mem.v2BankInterleaving;

        // second channel interleaving configuration
        params.enableSecondInterleaving = simP.mem.v2SecondInterleaving;
        params.secondChannelInterleaving = simP.mem.v2SecondChannelInterleaving;
        params.secondBankInterleaving = simP.mem.v2SecondBankInterleaving;

        params.splitterType = simP.mem.v2SplitterType;

        params.channelInterleavingMask = new string(
            (simP.mem.v2ChannelInterleavingMask ? simP.mem.v2ChannelInterleavingMask : "") );

        params.bankInterleavingMask =  new string(
            (simP.mem.v2BankInterleavingMask ? simP.mem.v2BankInterleavingMask : "") );

        params.secondChannelInterleavingMask = new string(
            (simP.mem.v2SecondChannelInterleavingMask ? simP.mem.v2SecondChannelInterleavingMask : "") );

        params.secondBankInterleavingMask = new string(
            (simP.mem.v2SecondBankInterleavingMask ? simP.mem.v2SecondBankInterleavingMask : "") );

        // params.fifo_maxChannelTransactions = simP.mem.v2MaxChannelTransactions;
        // params.rwfifo_maxReadChannelTransactions = simP.mem.v2MaxChannelTransactions;
        // params.rwfifo_maxWriteChannelTransactions = simP.mem.v2MaxChannelTransactions;
        params.maxChannelTransactions = simP.mem.v2MaxChannelTransactions;
        params.dedicatedChannelReadTransactions = simP.mem.v2DedicatedChannelReadTransactions;

        params.rwfifo_maxConsecutiveReads = simP.mem.maxConsecutiveReads;
        params.rwfifo_maxConsecutiveWrites = simP.mem.maxConsecutiveWrites;

        params.schedulerType = schedType;

        if ( simP.mem.v2BankSelectionPolicy )
            params.bankfifo_bankSelectionPolicy = new string(simP.mem.v2BankSelectionPolicy);
        else 
            params.bankfifo_bankSelectionPolicy = new string("OLDEST_FIRST RANDOM"); // default

        params.switchModePolicy = simP.mem.v2SwitchModePolicy;
        params.bankfifo_activeManagerMode = simP.mem.v2ActiveManagerMode;
        params.bankfifo_prechargeManagerMode = simP.mem.v2PrechargeManagerMode;

        params.bankfifo_disableActiveManager = simP.mem.v2DisableActiveManager;
        params.bankfifo_disablePrechargeManager = simP.mem.v2DisablePrechargeManager;
        params.bankfifo_managerSelectionAlgorithm = simP.mem.v2ManagerSelectionAlgorithm;

        params.schedulerPagePolicy = schedPagePolicy;

        // Enable/disable memory trace dump file
        params.memoryTrace = simP.mem.v2MemoryTrace;

        params.debugString = ( simP.mem.v2DebugString ? new string(simP.mem.v2DebugString) : new string("") );

        params.useClassicSchedulerStates = simP.mem.v2UseClassicSchedulerStates;

        params.useSplitRequestBufferPerROP = simP.mem.v2UseSplitRequestBufferPerROP;

        memController = 
            new memorycontroller::MemoryController( params,
                                                    (const char**)tuPrefixes, 
                                                    (const char**)suPrefix,
                                                    (const char**)slPrefixes,
                                                    "MemoryControllerV2", 
                                                    true, // create inner signals
                                                    0);

        delete params.memSpecs;
    }

    return memController;

}

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

#include "MemoryControllerTest.h"
#include "MemoryControllerSelector.h"
#include "MemoryControllerTestBase.h"
#include "ConfigLoader.h"
#include "OptimizedDynamicMemory.h"
#include <iostream>
#include "MCTConsole.h"
// #include <fstream>
#include "zfstream.h"
#include "MemoryControllerProxy.h"
#include "GPUMath.h"


using namespace gpu3d;
using namespace std;

GPUStatistics::Statistic *cyclesCounter;    //  Counts cycles (main clock domain!).

int main(int argc, char **argv)
{   
    // Create statistic to count cycles (GPU clock domain!)
    cyclesCounter = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("CycleCounter", u32bit(0), "GPU", 0);

    SimParameters simP;

    // Get signal binder
    SignalBinder &sigBinder = SignalBinder::getBinder();

    ConfigLoader cl("bGPU.ini");
    cl.getParameters(&simP);
    
    OptimizedDynamicMemory::initialize(simP.objectSize0, simP.bucketSize0, simP.objectSize1, simP.bucketSize1,
        simP.objectSize2, simP.bucketSize2);

    
    //  Allocate pointer array for the texture unit prefixes.
    char** tuPrefixes = new char*[simP.gpu.numFShaders * simP.fsh.textureUnits];
    //  Create prefixes for the fragment shaders.
    for(u32bit i = 0; i < simP.gpu.numFShaders; i++)
    {
        for(u32bit j = 0; j < simP.fsh.textureUnits; j++)
        {
            //  Allocate prefix.
            tuPrefixes[i * simP.fsh.textureUnits + j] = new char[10];

            GPU_ASSERT(
                if (tuPrefixes[i * simP.fsh.textureUnits + j] == NULL)
                    panic("bGPU", "main", "Error allocating texture unit prefix.");
            )

            //  Write prefix.
            sprintf(tuPrefixes[i * simP.fsh.textureUnits + j], "FS%dTU%d", i, j);
        }
    }

    /*  Allocate pointer array for streamer loader prefixes.  */
    char** slPrefixes = new char*[simP.str.streamerLoaderUnits];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (slPrefixes == NULL)
            panic("bGPU", "main", "Error allocating streamer loader prefix array.");
    )

    /*  Create prefixes for the streamer loader units.  */
    for(u32bit i = 0; i < simP.str.streamerLoaderUnits; i++)
    {
        /*  Allocate prefix.  */
        slPrefixes[i] = new char[6];

        GPU_ASSERT(
            if (slPrefixes[i] == NULL)
                panic("bGPU", "main", "Error allocating streamer loader prefix.");
        )

        /*  Write prefix.  */
        sprintf(slPrefixes[i], "SL%d", i);
    }

    //  Allocate pointer array for the stamp unit prefixes.
    char** suPrefix = new char*[simP.gpu.numStampUnits];
   
    //  Create prefixes for the stamp units. 
    for(u32bit i = 0; i < simP.gpu.numStampUnits; i++)
    {
        //  Allocate prefix.  
        suPrefix[i] = new char[6];

        GPU_ASSERT(
            if (suPrefix[i] == NULL)
                panic("bGPU", "main", "Error allocating stamp unit prefix.");
        )

        //  Write prefix. 
        sprintf(suPrefix[i], "SU%d", i);
    }

    u64bit cycle = 0;
    
    Box* mc = createMemoryController(simP, (const char**)tuPrefixes, (const char**)suPrefix, (const char**)slPrefixes, 
                                     "MemoryController", 0);

    MemoryControllerProxy mcProxy(mc);

    // MemoryControllerTestBase* mctest = new MyMCTest(simP, (const char**)tuPrefixes, (const char**)suPrefix, "MyMCTest", 0);
    MemoryControllerTestBase* mctest = new MCTConsole(simP, (const char**)tuPrefixes, (const char**)suPrefix, 
                                     (const char**)slPrefixes, "MyMCTest", mcProxy, 0);

    vector<string> params;
    if ( argc > 1 ) {
        for ( int i = 1; i < argc; ++i ) {
            params.push_back(argv[i]);
        }
    }

    mctest->parseParams(params);

    gzofstream sigTraceFile;
    // ofstream sigTraceFile;

    gzofstream out;
    // ofstream out;

    if ( !sigBinder.checkSignalBindings() ) {
        sigBinder.dump(true); // true = show only signals not properly bound
        panic("MemoryControllerTest", "main", "Signal connections undefined");
    }

    if ( simP.dumpSignalTrace ) {
        sigTraceFile.open(simP.signalDumpFile, ios::out | ios::binary);
        if ( !sigTraceFile.is_open() )
            panic("MemoryControllerTest", "main", "Error opening signal trace file");
        sigBinder.initSignalTrace(&sigTraceFile);
        cout << "MCT info: !!! GENERATING SIGNAL TRACE !!!" << endl;
    }

    if ( simP.statistics ) {
        /*  Initialize the statistics manager.  */
        out.open(simP.statsFile, ios::out | ios::binary);

        if ( !out.is_open() )
            panic("MemoryControllerTest", "main", "Error opening per cycle statistics file");

        GPUStatistics::StatisticsManager::instance().setDumpScheduling(0, simP.statsRate);
        GPUStatistics::StatisticsManager::instance().setOutputStream(out);
    }

    bool multiClock = (simP.gpu.gpuClock != simP.gpu.memoryClock);

    if ( !simP.mem.memoryControllerV2 && multiClock )
        panic("MemoryControllerTest", "main", "Multi clock domain is only supported with Memory Controller V2");

    u64bit gpuCycle = 0; //  Cycle counter for GPU clock domain.
    u64bit memoryCycle = 0; //  Cycle counter for memory clock domain.
    const u32bit gpuClockPeriod = (u32bit) (1E6 / (f32bit) simP.gpu.gpuClock);
    const u32bit memoryClockPeriod = (u32bit) (1E6 / (f32bit) simP.gpu.memoryClock);
    u32bit nextGPUClock = gpuClockPeriod; //  Stores the picoseconds (ps) to next gpu clock.
    u32bit nextMemoryClock = memoryClockPeriod;//  Stores the picoseconds (ps) to next memory clock.


    u32bit WAITING_CYCLES = 500; // Wait for pending replies from MC
    u32bit i = 0;
    while ( !mctest->finished() || i < WAITING_CYCLES ) 
    {
        if ( mctest->finished() )
            ++i;        
        
        if ( !multiClock ) // Unified clock domain clocking
        {
            if ( simP.dumpSignalTrace ) // Dump the signals
                sigBinder.dumpSignalTrace(cycle);
            cyclesCounter->inc();
            mc->clock(cycle);
            mctest->clock(cycle);
            if ( simP.statistics ) // Update GPU Statistics
                GPUStatistics::StatisticsManager::instance().clock(cycle);
            cycle ++;

            if ( cycle % 1000 == 0 ) {
                cout << ".";
                cout.flush();
            }
        }
        else // Multi clock domain clocking
        {
            u32bit nextStep = GPU_MIN(nextGPUClock, nextMemoryClock);
            nextGPUClock -= nextStep;
            nextMemoryClock -= nextStep;
            if ( nextGPUClock == 0 ) 
            {
                cyclesCounter->inc();
                mctest->clock(gpuCycle);
                static_cast<memorycontroller::MemoryController*>(mc)->clock(GPU_CLOCK_DOMAIN, gpuCycle);
                if ( simP.statistics ) // Update GPU Statistics
                    GPUStatistics::StatisticsManager::instance().clock(gpuCycle);

                if ( gpuCycle % 1000 == 0 ){
                    cout << ".";
                    cout.flush();
                }
                ++gpuCycle;
                nextGPUClock = gpuClockPeriod;
            }
            if ( nextMemoryClock == 0 )
            {
                static_cast<memorycontroller::MemoryController*>(mc)->clock(MEMORY_CLOCK_DOMAIN, memoryCycle);
                ++memoryCycle;
                nextMemoryClock = memoryClockPeriod;
            }
        }
    }

    // Signal trace finished
    if ( simP.dumpSignalTrace ) 
        sigBinder.endSignalTrace();

    //OptimizedDynamicMemory::usage();
    //OptimizedDynamicMemory::dumpDynamicMemoryState();

    OptimizedDynamicMemory::usage();
    GPUStatistics::StatisticsManager::instance().finish();

    return 0;
}

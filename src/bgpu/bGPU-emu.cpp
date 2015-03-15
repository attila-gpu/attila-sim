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
 * bGPU emulator implementation file.
 *
 */

/**
 *
 *  @file bGPU-emu.cpp
 *
 *  This file implements the ATTILA emulator.
 *
 */

//  bGPU emulator definitions and declarations.
#include "bGPU-emu.h"
#include "ConfigLoader.h"
#include "support.h"

#include "OptimizedDynamicMemory.h"

#include "AGPTraceDriver.h"
#include "GLTraceDriver.h"
#include "D3DTraceDriver.h"

#include "ShaderArchitectureParameters.h"

//#define ENABLE_GLOBAL_PROFILER

#include "GPUEmulator.h"

#include "GlobalProfiler.h"

#include <ctime>
#include <new>
#include <csignal>
#include <cstdlib>

using namespace gpu3d;

//  Trace reader+driver.
TraceDriverInterface *trDriver;
AGPTraceDriver *agpTraceDriver;
bool trDriverForACD;

 //  Simulator parameters.
SimParameters simP;

//  Emulator.
GPUEmulator *gpuEmu;

#ifdef WIN32
int gpu3d::abortSignalHandler(int s)
{
    if (s == CTRL_C_EVENT)
    {
        printf("SignalHandler => CTRL+C received.\n");
        gpuEmu->setAbortEmulation();
        
    }

    return 1;
}

#else
//  Signal handler for abort signal mode
void gpu3d::abortSignalHandler(int s)
{
    if (s == SIGINT)
    {
        printf("SignalHandler => CTRL+C received.\n");
        gpuEmu->setAbortEmulation();
    }
}

#endif

bool segFaultAlreadyReceived = false;

void gpu3d::segFaultSignalHandler(int s)
{
    if (s == SIGSEGV)
    {
        if (segFaultAlreadyReceived)
        {
            signal(SIGSEGV, SIG_DFL);
        }
        else
        {
            segFaultAlreadyReceived = true;
            printf("\n");
            printf("***! Segmentation Fault detected at frame %d batch %d triangle %d !***\n",
                gpuEmu->getFrameCounter(), gpuEmu->getBatchCounter(), gpuEmu->getTriangleCounter());
            fflush(stdout);
            signal(SIGSEGV, SIG_DFL);
        }
    }
}

void out_of_memory()
{
    cerr << "bGPU-emu::out_of_memory -> Memory exhausted. Aborting" << endl;
    exit(-1);
}

bool gpu3d::checkAGPTrace(AGPTraceFileHeader *agpTraceHeader)
{
    string str = agpTraceHeader->signature;

    return !str.compare(AGPTRACEFILE_SIGNATURE);
}


bool gpu3d::has_extension(const string file_name, const string extension) {
    size_t pos = file_name.find_last_of(".");
    if(pos == file_name.npos)
        return false;
    else {
        string file_extension = file_name.substr(pos + 1);
        string extension_uc = extension;
        for(int i=0; i!=file_extension.length(); ++i)
            extension_uc[i] = toupper(extension[i]);
        for(int i=0; i!=file_extension.length(); ++i)
            file_extension[i] = toupper(file_extension[i]);
        return file_extension.compare(extension_uc) == 0;
    }
}

bool gpu3d::checkAGPTraceParameters(AGPTraceFileHeader *agpTraceHeader)
{
    bool allParamsOK = (simP.mem.memSize == agpTraceHeader->parameters.memSize) &&
                       (simP.mem.mappedMemSize == agpTraceHeader->parameters.mappedMemSize) &&
                       (simP.fsh.textBlockDim == agpTraceHeader->parameters.textBlockDim) &&
                       (simP.fsh.textSBlockDim == agpTraceHeader->parameters.textSBlockDim) &&
                       (simP.ras.scanWidth == agpTraceHeader->parameters.scanWidth) &&
                       (simP.ras.scanHeight == agpTraceHeader->parameters.scanHeight) &&
                       (simP.ras.overScanWidth == agpTraceHeader->parameters.overScanWidth) &&
                       (simP.ras.overScanHeight == agpTraceHeader->parameters.overScanHeight) &&
                       (simP.doubleBuffer == agpTraceHeader->parameters.doubleBuffer) &&
                       (simP.fsh.fetchRate == agpTraceHeader->parameters.fetchRate) &&
                       (simP.mem.memoryControllerV2 == agpTraceHeader->parameters.memoryControllerV2) &&
                       (simP.mem.v2SecondInterleaving == agpTraceHeader->parameters.v2SecondInterleaving);

    return allParamsOK;
}

#ifdef WIN32
    #define parse_cycles(a) _atoi64(a)
#else
    #define parse_cycles(a) atoll(a)
#endif

//  Main Function.
int main(int argc, char *argv[])
{
    GLOBALPROFILER_ENTERREGION("other", "", "")
    set_new_handler(out_of_memory);

    ConfigLoader *cl;

    // Arguments parsing and configuration loading.
    char *configFile = "bGPU.ini";
    bool debugMode = false;

    u64bit data;

    int argIndex = 1;
    int argPos = 0;
    int argCount = 0;
    char **argList = new char*[argc];

    // First pass: search for config file and generate argument list for the 2nd pass.
    while (argIndex < argc) {
        if (strcmp(argv[argIndex], "--config") == 0) {
            if (++argIndex < argc) {
                configFile = new char[strlen(argv[argIndex]) + 1];
                strcpy(configFile, argv[argIndex]);
            }
        }
        else
            argList[argCount++] = argv[argIndex++];
    }

    //  Open the configuration file.
    cl = new ConfigLoader(configFile);

    //  Get all the simulator parameters from the configuration file.
    cl->getParameters(&simP);

    //  Delete the configuration loader.
    delete cl;

    // Second pass: parse the rest of arguments and override configuration.
    argIndex = 0;
    argPos = 0;

    while (argIndex < argCount) {
        if (strcmp(argList[argIndex], "--debug") == 0)
            debugMode = true;
        else if (strcmp(argList[argIndex], "--start") == 0 && ++argIndex < argCount)
            simP.startFrame = atoi(argList[argIndex]);
        else if (strcmp(argList[argIndex], "--frames") == 0 && ++argIndex < argCount)
            simP.simFrames = atoi(argList[argIndex]);
        else if (strcmp(argList[argIndex], "--cycles") == 0 && ++argIndex < argCount)
            simP.simCycles = parse_cycles(argList[argIndex]);
        else if (strcmp(argList[argIndex], "--trace") == 0 && ++argIndex < argCount) {
            simP.inputFile = new char[strlen(argList[argIndex]) + 1];
            strcpy(simP.inputFile, argList[argIndex]);
        }
        else { // traditional arguments style
            switch (argPos) {
                case 0: // trace file
                    simP.inputFile = new char[strlen(argList[argIndex]) + 1];
                    strcpy(simP.inputFile, argList[argIndex]);
                    break;
                case 1: // num frames/cycles
                    data = parse_cycles(argList[argIndex]);
                    if (data >= 10000) {
                        simP.simFrames = 0;
                        simP.simCycles = data;
                    }
                    else {
                        simP.simFrames = u32bit(data);
                        simP.simCycles = 0;
                    }
                    break;
                case 2: // start frame
                    simP.startFrame = atoi(argList[argIndex]);
                    break;
                default:
                    printf("Illegal argument at position %i: %s", argIndex, argList[argIndex]);
                    exit(-1);
            }
            argPos++;
        }
        argIndex++;
    }

    //  Check if the vector alu configuration is scalar (SOA).
    string aluConf(simP.fsh.vectorALUConfig);
    bool vectorScalarALU = simP.fsh.useVectorShader && (aluConf.compare("scalar") == 0);

    u32bit instructionsToSchedulePerCycle = 0;

    //  Compute the number of shader instructions to schedule per cycle that is passed to the driver.
    if (simP.fsh.useVectorShader)
    {
        //  Only the simd4+scalar architecture requires scheduling more than 1 instruction per cycle.
        if (aluConf.compare("simd4+scalar") == 0)
            instructionsToSchedulePerCycle = 2;
        else
            instructionsToSchedulePerCycle = 1;
    }
    else
    {
        //  Use the 'FetchRate' (instructions fetched per cycle) parameter for the old shader model.
        instructionsToSchedulePerCycle = simP.fsh.fetchRate;
    }

    //
    // Configure GPUDriver
    //
    GPUDriver::getGPUDriver()->setGPUParameters(
                                    simP.mem.memSize * 1024,
                                    simP.mem.mappedMemSize * 1024,
                                    simP.fsh.textBlockDim,
                                    simP.fsh.textSBlockDim,
                                    simP.ras.scanWidth,
                                    simP.ras.scanHeight,
                                    simP.ras.overScanWidth,
                                    simP.ras.overScanHeight,
                                    simP.doubleBuffer,
                                    simP.forceMSAA,
                                    simP.msaaSamples,
                                    simP.forceFP16ColorBuffer,
                                    instructionsToSchedulePerCycle,
                                    simP.mem.memoryControllerV2,
                                    simP.mem.v2SecondInterleaving,
                                    simP.fsh.vAttrLoadFromShader,
                                    vectorScalarALU,
                                    simP.enableDriverShTrans,
                                    (simP.ras.useMicroPolRast && simP.ras.microTrisAsFragments)
                    );

    if (simP.fsh.fixedLatencyALU)
    {
        if (simP.fsh.useVectorShader && vectorScalarALU)
            ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("FixedLatSOA");
        else
            ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("FixedLatAOS");
    }
    else
    {
        if (simP.fsh.useVectorShader && vectorScalarALU)
            ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("VarLatSOA");
        else
            ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("VarLatAOS");
    }

    //  Print loaded parameters.
    printf("Simulator Parameters.\n");

    printf("--------------------\n\n");

    printf("Input File = %s\n", simP.inputFile);
    printf("Signal Trace File = %s\n", simP.signalDumpFile);
    printf("Statistics File = %s\n", simP.statsFile);
    printf("Statistics (Per Frame) File = %s\n", simP.statsFilePerFrame);
    printf("Statistics (Per Batch) File = %s\n", simP.statsFilePerBatch);
    printf("Simulation Cycles = %lld\n", simP.simCycles);
    printf("Simulation Frames = %d\n", simP.simFrames);
    printf("Simulation Start Frame = %d\n", simP.startFrame);
    printf("Signal Trace Dump = %s\n", simP.dumpSignalTrace?"enabled":"disabled");
    printf("Signal Trace Start Cycle = %lld\n", simP.startDump);
    printf("Signal Trace Dump Cycles = %lld\n", simP.dumpCycles);
    printf("Statistics Generation = %s\n", simP.statistics?"enabled":"disabled");
    printf("Statistics (Per Cycle) Generation = %s\n", simP.perCycleStatistics?"enabled":"disabled");
    printf("Statistics (Per Frame) Generation = %s\n", simP.perFrameStatistics?"enabled":"disabled");
    printf("Statistics (Per Batch) Generation = %s\n", simP.perBatchStatistics?"enabled":"disabled");
    printf("Statistics Rate = %d\n", simP.statsRate);
    printf("Dectect Stalls = %s\n", simP.detectStalls?"enabled":"disabled");
    printf("EnableDriverShaderTranslation = %s\n", simP.enableDriverShTrans ? "true" : "false");
    printf("VertexAttributeLoadFromShader = %s\n", simP.fsh.vAttrLoadFromShader ? "true" : "false");
    printf("VectorALUConfig = %s\n", simP.fsh.vectorALUConfig);

    //  Initializations.

    //  Initialize the optimized dynamic memory system.
    OptimizedDynamicMemory::initialize(simP.objectSize0, simP.bucketSize0, simP.objectSize1, simP.bucketSize1,
        simP.objectSize2, simP.bucketSize2);

    //  Create and initialize the Trace Driver.
    gzifstream agpTraceFile;

    if (has_extension(simP.inputFile, "pixrun") || has_extension(simP.inputFile, "pixrunz"))
    {
        trDriver = new D3DTraceDriver(simP.inputFile, simP.startFrame);
        
        cout << "Using Direct3D Trace File as simulation input." << endl;

        cout << "Using Attila Common Driver (ACD) Library." << endl;
    }
    else
    {
        //  Check if the input trace file is an AGP transaction trace file.
        agpTraceFile.open(simP.inputFile, ios::in | ios::binary);

        //  Check if the input file is open
        if (!agpTraceFile.is_open())
            panic("bGPU-emu", "main", "Error opening input trace file.");

        AGPTraceFileHeader agpTraceHeader;

        //  Read the agp trace header.
        agpTraceFile.read((char *) &agpTraceHeader, sizeof(agpTraceHeader));

        if (checkAGPTrace(&agpTraceHeader))
        {
            cout << "Using AGP Trace File as simulation input." << endl;

            //  Check parameters of the AGP transaction trace file.
            if (!checkAGPTraceParameters(&agpTraceHeader))
                cout << "Warning!!! Current parameters and the parameters of the trace file differ!!!!" << endl << endl;

            //  Initialize a trace driver for an AGP transaction trace file.
            trDriver = agpTraceDriver = new AGPTraceDriver(&agpTraceFile, simP.startFrame, agpTraceHeader.parameters.startFrame);

            //  The start frame an offset to the first frame in the AGP trace.
            simP.startFrame += agpTraceHeader.parameters.startFrame;
        }
        else
        {
            //  Close tracefile.
            agpTraceFile.close();

            cout << "Using OpenGL Trace File as simulation input." << endl;

            if (simP.useACD)
                cout << "Using Attila Common Driver (ACD) Library." << endl;
            else
                cout << "Using Legacy Library." << endl;

            //  Initialize a trace for an OpenGL trace file
            trDriver = new GLTraceDriver(simP.inputFile,
                                         simP.useACD, // False => Legacy, True => Uses ACD new implementation
                                         GPUDriver::getGPUDriver(),
                                         simP.ras.shadedSetup,
                                         simP.startFrame);

            //  No AGP Trace Driver available.
            agpTraceDriver = NULL;
        }
    }

    //  Create GPU emulator.
    gpuEmu = new GPUEmulator(simP, trDriver);

#ifdef WIN32
    SetConsoleCtrlHandler(PHANDLER_ROUTINE(abortSignalHandler), true);
#else
    signal(SIGINT, &abortSignalHandler);
#endif

    signal(SIGSEGV, &segFaultSignalHandler);

    //  Warning.
    if (simP.startFrame != 0)
        printf("Note: The first frame producing output (start frame) is frame %d\n", simP.startFrame);

    //  Check for debug mode to enable the interactive debug loop.
    if (debugMode)
    {

        cout << "Starting simulator debug mode ---- " << endl << endl;

        cout << "Exiting simulator debug mode ---- " << endl;
    }
    else
    {
        printf("Simulating %d frames (1 dot : 10K cycles).\n\n", simP.simFrames);

        time_t startTime = time(NULL);

        //  Call the emulation main loop.
        gpuEmu->emulationLoop();

        f64bit elapsedTime = time(NULL) - startTime;
        cout << "\nSimulation clock time = " << elapsedTime << " seconds" << endl;

        //  Close input file
        if (agpTraceFile.is_open())
            agpTraceFile.close();

        //  Print end message.
        printf("\n\n");
        printf("End of simulation\n");
        printf("\n\n");
    }
    
    GLOBALPROFILER_EXITREGION()
    GLOBALPROFILER_GENERATEREPORT("profile.txt")

    return 0;
}


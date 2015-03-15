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
 * bGPU implementation file (unified shader version).
 *
 */

/**
 *
 *  @file bGPU-Unified.cpp
 *
 *  This file implements the simulator main loop and
 *  support functions (unified shader version).
 *
 *
 */

/*  bGPU definitions and declarations.  */
#include "bGPU-Unified.h"

#include "GPUSimulator.h"

#include "ConfigLoader.h"
#include "LineReader.h"

#include "AGPTraceDriver.h"
#include "GLTraceDriver.h"
#include "D3DTraceDriver.h"

#include <ctime>
#include <new>
#include <signal.h>

using namespace std;
using namespace gpu3d;

//  Trace reader+driver.
TraceDriverInterface *trDriver;
AGPTraceDriver *agpTraceDriver;

bool d3d9Trace = false;
bool oglTrace = false;
bool agpTrace = false;

SimParameters simP;     //  Simulator parameters.

bool multiClock;            //  Stores if the simulated GPU architecture implements multiple clock domains.
bool shaderClockDomain;     //  Stores if the simulated GPU architecture implements a shader clock domain.
bool memoryClockDomain;     //  Stores if the simulated GPU architecture implements a memory clock domain.
u32bit gpuClockPeriod;      //  Period of the GPU clock in picoseconds (ps).
u32bit shaderClockPeriod;   //  Period of the shader clock in picoseconds (ps):
u32bit memoryClockPeriod;   //  Period of the memory clock in picoseconds (ps):

GPUSimulator *gpuSimulator;

//  Signal handler for abort signal in debug mode
void gpu3d::abortSignalHandler(int s)
{
    gpuSimulator->abortSimulation();
    signal(SIGINT, &abortSignalHandler);        
}

u32bit segFaultAlreadyReceived = 0;

void gpu3d::segFaultSignalHandler(int s)
{
    if (s == SIGSEGV)
    {
        if (segFaultAlreadyReceived == 2)
        {
            signal(SIGSEGV, SIG_DFL);
        }
        else if (segFaultAlreadyReceived == 1)
        {
            segFaultAlreadyReceived++;
            printf("\n");
            printf("***! Segmentation Fault detected on segmentation fault handler !***\n");
            fflush(stdout);
            signal(SIGSEGV, SIG_DFL);
        }
        else
        {
            u32bit frameCounter;
            u32bit frameBatch;
            u32bit batchCounter;
            
            gpuSimulator->getCounters(frameCounter, frameBatch, batchCounter);
            
            u64bit gpuCycle;
            u64bit shaderCycle;
            u64bit memCycle;
            
            gpuSimulator->getCycles(gpuCycle, shaderCycle, memCycle);
            
            segFaultAlreadyReceived++;
            printf("\n");            
            printf("***! Segmentation Fault detected !***\n");            
            printf(" Frame = %d Batch = %d Total Batches = %d\n", frameCounter, frameBatch, batchCounter);
            if (multiClock)
            {
                printf(" GPU Cycle = %lld\n", gpuCycle);
                if (shaderClockDomain)
                    printf(" Shader Cycle = %lld\n", shaderCycle);
                if (memoryClockDomain)
                    printf(" Memory Cycle = %lld\n", memCycle);
            }
            else
                printf(" Cycle = %lld\n", gpuCycle);
            
            GPUSimulator::createSnapshot();
                                    
            printf("***!        END OF REPORT        !***\n");
            fflush(stdout);
            exit(-1);
            signal(SIGSEGV, SIG_DFL);
        }
    }  
}

void out_of_memory()
{
    cerr << "bGPU-Unified::out_of_memory -> Memory exhausted. Aborting" << endl;
    exit(-1);
}

bool gpu3d::checkAGPTrace(AGPTraceFileHeader *agpTraceHeader)
{
    string str(agpTraceHeader->signature, strlen(AGPTRACEFILE_SIGNATURE));

    bool signatureFound = (str.compare(AGPTRACEFILE_SIGNATURE) == 0);
    
    return signatureFound;
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
    set_new_handler(out_of_memory);

    ConfigLoader *cl;

    // Arguments parsing and configuration loading.
    char *configFile = "bGPU.ini";
    bool debugMode = false;
    bool validationMode = false;

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
        else if (strcmp(argList[argIndex], "--valid") == 0)
            validationMode = true;
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
    
    //  Check configuration parameters.
    if (simP.fsh.vAttrLoadFromShader && !simP.enableDriverShTrans)
    {
        panic("bGPU-Unified", "main", "Vertex attribute load from shader requires driver shader program translation to be enabled.");
    }
    if (simP.fsh.useVectorShader && vectorScalarALU && !simP.enableDriverShTrans)
    {
        panic("bGPU-Unified", "main", "Vector Shader SOA architecture requires driver shader program translation to be enabled.");
    }
    
    //  Determine clock operation mode.
    shaderClockDomain = (simP.gpu.gpuClock != simP.gpu.shaderClock);
    memoryClockDomain = (simP.gpu.gpuClock != simP.gpu.memoryClock);
    multiClock = shaderClockDomain || memoryClockDomain;
    
    //  Compute clock period for multiple clock domain mode.
    if (multiClock)
    {
        gpuClockPeriod = (u32bit) (1E6 / (f32bit) simP.gpu.gpuClock);
        shaderClockPeriod = (u32bit) (1E6 / (f32bit) simP.gpu.shaderClock);
        memoryClockPeriod = (u32bit) (1E6 / (f32bit) simP.gpu.memoryClock);
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
    
    //  Set the shader architecture to use.
#ifdef UNIFIEDSHADER
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
#else
    if (simP.fsh.fixedLatencyALU)
        ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("FixedLatAOS");
    else
        ShaderArchitectureParameters::getShaderArchitectureParameters()->selectArchitecture("VarLatAOS");
#endif    

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
    if (multiClock)
    {
        printf("GPU Clock Period = %d ps\n", gpuClockPeriod);
        if (shaderClockDomain)
            printf("Shader Clock Period = %d ps\n", shaderClockPeriod);
        if (memoryClockDomain)
            printf("Memory Clock Period = %d ps\n", memoryClockPeriod);
    }

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
            
            //  The simulator input is a D3D9 PIXRun trace.  
            d3d9Trace = true;
    }
    else
    {
        //  Check if the input trace file is an AGP transaction trace file.
        agpTraceFile.open(simP.inputFile, ios::in | ios::binary);

        //  Check if the input file is open
        if (!agpTraceFile.is_open())
            panic("bGPU", "main", "Error opening input trace file.");

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
            
            //  The simulator input is an AGP Transaction trace.  
            agpTrace = true;
            
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

            //  The simulator input is an GLInterceptor OpenGL trace.  
            oglTrace = true;
        }
    }
   
#ifdef UNIFIEDSHADER 
    gpuSimulator = new GPUSimulator(simP, trDriver, true, d3d9Trace, oglTrace, agpTrace);
#else
    gpuSimulator = new GPUSimulator(simP, trDriver, false, d3d9Trace, oglTrace, agpTrace);
#endif
    
    //  Warning.    
    if (simP.startFrame != 0)
        printf("Note: The first frame producing output (start frame) is frame %d\n", simP.startFrame);

    //  Add a segmentation fault handler to print some information.
    signal(SIGSEGV, &segFaultSignalHandler);
    
    //  Define the call back for the panic function to save a snapshot on simulator errors.
    panicCallback = &GPUSimulator::createSnapshot;
    
    //  Check for debug mode to enable the interactive debug loop.
    if (debugMode || validationMode)
    {
        signal(SIGINT, &abortSignalHandler);        

        cout << "Starting simulator debug mode ---- " << endl << endl;

        gpuSimulator->debugLoop(validationMode);
        
        cout << "Exiting simulator debug mode ---- " << endl;
    }
    else
    {
        if (simP.simCycles != 0)
            printf("Simulating %lld cycles (1 dot : 10K cycles).\n\n", simP.simCycles);
        else
            printf("Simulating %d frames (1 dot : 10K cycles).\n\n", simP.simFrames);

        time_t startTime = time(NULL);

        //  Call the simulation main loop.
        if (!multiClock)
            gpuSimulator->simulationLoop();
        else
            gpuSimulator->multiClockSimLoop();

        f64bit elapsedTime = time(NULL) - startTime;
        cout << "\nSimulation clock time = " << elapsedTime << " seconds" << endl;

        //  Close input file
        if (agpTraceFile.is_open())
            agpTraceFile.close();

        delete gpuSimulator;
        
    }

    return 0;
}


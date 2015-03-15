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
 * $RCSfile: gl2atila.cpp,v $
 * $Revision: 1.6 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:33:58 $
 *
 * gl2atila implementation file
 *
 */


/**
 *
 *  @file gl2atila.cpp
 *
 *  This file contains the implementation code for the preprocessor tool that translates a trace of OpenGL API calls
 *  in a trace of AGP transactions for the ATILA simulator.
 *
 */


#include <cstdio> 
#include <ctime>
#include <iostream>
//#ifdef _DEBUG
//#define _CRTDBG_MAP_ALLOC
//#include <crtdbg.h>
//#endif
#include <new>
#include <cstdlib>
#include <cstring>

#include "support.h"
#include "zfstream.h"
#include "ConfigLoader.h"
#include "OptimizedDynamicMemory.h"
#include "GLTraceDriver.h"
#include "D3DTraceDriver.h"

#include "AGPTraceFile.h"
#include "ShaderArchitectureParameters.h"

using namespace std;
using namespace gpu3d;

//  Trace reader+driver.
TraceDriverInterface *trDriver;

//  Simulator parameters.
SimParameters simP;

//  Counts the frames (swap commands).
u32bit frameCounter;

//  Stores the number of batches rendered.
u32bit batchCounter;

//  Out of memory handler function.
void out_of_memory()
{
    cerr << "bGPU-Unified::out_of_memory -> Memory exhausted. Aborting" << endl;
    exit(-1);
}


bool has_extension(const string file_name, const string extension)
{
    size_t pos = file_name.find_last_of(".");
    if(pos == file_name.npos)
        return false;
    else
    {
        string file_extension = file_name.substr(pos + 1);
        string extension_uc = extension;
        for(int i=0; i!=file_extension.length(); ++i)
            extension_uc[i] = toupper(extension[i]);
        for(int i=0; i!=file_extension.length(); ++i)
            file_extension[i] = toupper(file_extension[i]);
        return file_extension.compare(extension_uc) == 0;
    }
}


//  Main function
int main(int argc, char *argv[])
{
    //  Set function handles for out of memory event.    
    set_new_handler(out_of_memory);

    ConfigLoader *cl;

    //  Open the configuration file.
    cl = new ConfigLoader("bGPU.ini");

    //  Get all the simulator from the configuration file.
    cl->getParameters(&simP);

    //  Delete the loader.
    delete cl;

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
        panic("gl2attila", "main", "Vertex attribute load from shader requires driver shader program translation to be enabled.");
    }
    if (simP.fsh.useVectorShader && vectorScalarALU && !simP.enableDriverShTrans)
    {
        panic("gl2attila", "main", "Vector Shader SOA architecture requires driver shader program translation to be enabled.");
    }

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
    
    // Configure GPUDriver memory size in Kbytes and block and sblock sizes
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
                                    //simP.enableDriverShTrans  // Disabled when creating/processing AGPTrans traces
                                    false,
                                    //(simP.ras.useMicroPolRast && simP.ras.microTrisAsFragments) // Disabled when creating/processing AGPTrans traces
                                    false
                    );


    //  Obtain command line parameters.
    //  First parameters is the trace filename. 
    //  Second is the number of GPU frames or cycles to simulate. 
    //  Third is the start frame for simulation.
    
    //  First parameter present?
    if(argc > 1)
    {
        //  First parameter is the trace filename.
        simP.inputFile = new char[strlen(argv[1]) + 1];

        //  Read parameter.
        strcpy(simP.inputFile, argv[1]);
    }

    u64bit secondParam;
    
    //  Second parameter present?
    if(argc > 2)
    {
        //  Second parameter is frames to convert from OpenGL to ATILA AGP commands

        //  Read second parameter from the arguments.
#ifdef WIN32
            secondParam = _atoi64(argv[2]);
#else
            secondParam = atoll(argv[2]);
#endif

        //  As frames to simulate
        simP.simCycles = 0;
        simP.simFrames = u32bit(secondParam);
    }

    //  Third parameter present?
    if (argc > 3)
    {
        //  Read start simulation frame.
        simP.startFrame = atoi(argv[3]);
    }

    //  Print loaded parameters.
    printf("Simulator Parameters.\n");

    printf("--------------------\n\n");

    printf("Input File = %s\n", simP.inputFile);
    printf("Simulation Frames = %d\n", simP.simFrames);
    printf("Simulation Start Frame = %d\n", simP.startFrame);


    //  Initialize the optimized dynamic memory system.
    OptimizedDynamicMemory::initialize(512,
                                       1024,
                                       1024,
                                       1024,
                                       2048,
                                       1024);
                                       
    //  Create and initialize the Trace Driver.
    if(has_extension(simP.inputFile, "pixrun"))
    {
        trDriver = new D3DTraceDriver(simP.inputFile, simP.startFrame);
        cout << "Using Direct3D Trace File as simulation input." << endl;

        cout << "Using Attila Common Driver (ACD) Library." << endl;
    }
    else
    {
        trDriver = new GLTraceDriver(simP.inputFile, simP.useACD, GPUDriver::getGPUDriver(), simP.ras.shadedSetup, simP.startFrame);
    }

    //  Set frame counter as start frame.
    frameCounter = simP.startFrame;

    if ( simP.startFrame != 0 )
        printf("Note: The first frame producing output (start frame) is frame %d\n", simP.startFrame);

    printf("Converting %d frames.\n\n", simP.simFrames);

    //  Reset batch counter
    batchCounter = 0;

    gzofstream outFile;
    
    //  Open output file
    outFile.open("attila.tracefile.gz", ios::out | ios::binary);
    
    //  Check if output file was correctly created.
    if (!outFile.is_open())
    {
        panic("gl2atila", "main", "Error opening output AGP transaction trace file.");
    }
    
    AGPTraceFileHeader agpTraceHeader;
    
    //  Create header for the AGP Trace file.
    
    //  Write file type stamp
    //agpTraceHeader.signature = AGPTRACEFILE_SIGNATURE;
    for(u32bit i = 0; i < sizeof(AGPTRACEFILE_SIGNATURE); i++)
        agpTraceHeader.signature[i] = AGPTRACEFILE_SIGNATURE[i];
        
    agpTraceHeader.version = AGPTRACEFILE_CURRENT_VERSION;
    
    agpTraceHeader.parameters.startFrame = simP.startFrame;
    agpTraceHeader.parameters.traceFrames = simP.simFrames;
    agpTraceHeader.parameters.memSize = simP.mem.memSize;
    agpTraceHeader.parameters.mappedMemSize = simP.mem.mappedMemSize;
    agpTraceHeader.parameters.textBlockDim = simP.fsh.textBlockDim;
    agpTraceHeader.parameters.textSBlockDim = simP.fsh.textSBlockDim;
    agpTraceHeader.parameters.scanWidth = simP.ras.scanWidth;
    agpTraceHeader.parameters.scanHeight = simP.ras.scanHeight;
    agpTraceHeader.parameters.overScanWidth = simP.ras.overScanWidth;
    agpTraceHeader.parameters.overScanHeight = simP.ras.overScanHeight;
    agpTraceHeader.parameters.doubleBuffer = simP.doubleBuffer;
    agpTraceHeader.parameters.fetchRate = simP.fsh.fetchRate;
    agpTraceHeader.parameters.memoryControllerV2 = simP.mem.memoryControllerV2;
    agpTraceHeader.parameters.v2SecondInterleaving = simP.mem.v2SecondInterleaving;

    //  Write header (configuration parameters related with the OpenGL to ATTILA AGP commands translation).
    outFile.write((char *) &agpTraceHeader, sizeof(agpTraceHeader));    
  
    clock_t startTime = clock();

    bool end = false;
    
    //  Start TraceDriver.
    trDriver->startTrace();
    
    while(!end)
    {
        AGPTransaction *nextAGPTransaction;
        
        nextAGPTransaction = trDriver->nextAGPTransaction();
        
        //  Check if the end of the OpenGL trace has been reached.
        if (nextAGPTransaction != NULL)
        {
            //  Check transaction type.
            if (nextAGPTransaction->getAGPCommand() == AGP_COMMAND)
            {
                if (nextAGPTransaction->getGPUCommand() == GPU_DRAW)
                    batchCounter++;
                    
                if (nextAGPTransaction->getGPUCommand() == GPU_SWAPBUFFERS)
                {
                    frameCounter++;
//#ifdef _DEBUG                    
//                    _CrtDumpMemoryLeaks();                    
//#endif                    
                }
            }
            
            //  Save AGP Transaction in the output file.
            nextAGPTransaction->save(&outFile);
            
            //  Delete AGP transaction
            delete nextAGPTransaction;
        }
        else
        {
            end = true;
        }
        
        //  Determine end of conversion process.
        if (simP.simFrames != 0)
        {
            //  End if the number of requested frames has been rendered.
            //  If the simulation didn't start with the first frame it must count the
            //  false frame rendered (only swap command) to display the cycle at which
            //  frame skipping was disabled
            end = end || ((frameCounter - simP.startFrame) == (simP.simFrames + ((simP.startFrame == 0)?0:1)));
        }
    }

    f64bit elapsedTime = ((f64bit)(clock() - startTime))/ CLOCKS_PER_SEC;

    cout << "\nSimulation clock time = " << elapsedTime << " seconds" << endl;

    //  Close output file.
    outFile.close();
    
    //  Print end message.
    printf("\n\n");
    printf("End of converion\n");
    printf("\n\n");
    
    return 0;
}









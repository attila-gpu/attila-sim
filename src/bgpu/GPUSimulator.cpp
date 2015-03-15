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
 * GPU simulator implementation file.
 *
 */


/**
 *
 *  @file GPUSimulator.cpp
 *
 *  This file contains the implementation of functions for the ATTILA GPU simulator.
 *
 */


#include "GPUSimulator.h"
#include "LineReader.h"
#include "AGPTraceDriver.h"
#include "ColorCompressorEmulator.h"
#include "DepthCompressorEmulator.h"
#include "StatisticsManager.h"
#include "support.h"
#include <ctime>

using namespace std;

namespace gpu3d
{

bool isNaN(f32bit f)
{
    u32bit fr = *((u32bit *) &f);
    return ((fr & 0x7F800000) == 0x7F800000) && ((fr & 0x007FFFFF) != 0);
}

GPUSimulator *GPUSimulator::current = NULL;

//  Constructor.
GPUSimulator::GPUSimulator(SimParameters simP, TraceDriverInterface *trDriver, bool unified,
                           bool d3d9Trace, bool oglTrace, bool agpTrace) :

    simP(simP), trDriver(trDriver), unifiedShader(unified), d3d9Trace(d3d9Trace), oglTrace(oglTrace), agpTrace(agpTrace),
    sigBinder(SignalBinder::getBinder()), simulationStarted(false)

{
    char **vshPrefix;
    char **fshPrefix;
    char **suPrefix;
    char **tuPrefixes;
    char tuPrefix[256];
    char boxName[128];
    u32bit i;
    u32bit j;
    char **slPrefixes;

    // Configure compressors.
    ColorCompressorEmulator::configureCompressor(simP.cwr.comprAlgo);
    DepthCompressorEmulator::configureCompressor(simP.zst.comprAlgo);

    // Notify boxes when the specific signal trace code is required.
    // Allows optimization in boxes with several inner signals only used to
    // show STV information
    Box::setSignalTracing(simP.dumpSignalTrace, simP.startDump, simP.dumpCycles);

    //  Initializations.

    // Create statistic to count cycles.
    cyclesCounter = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("CycleCounter", u32bit(0), "GPU", 0);

    //  Allocate pointer array for streamer loader prefixes.
    slPrefixes = new char*[simP.str.streamerLoaderUnits];

    //  Check allocation.
    GPU_ASSERT(
        if (slPrefixes == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating streamer loader prefix array.");
    )

    //  Create prefixes for the streamer loader units.
    for(i = 0; i < simP.str.streamerLoaderUnits; i++)
    {
        //  Allocate prefix.
        slPrefixes[i] = new char[6];

        GPU_ASSERT(
            if (slPrefixes[i] == NULL)
                panic("GPUSimulator", "GPUSimulator", "Error allocating streamer loader prefix.");
        )

        //  Write prefix.
        sprintf(slPrefixes[i], "SL%d", i);
   }

    //  Allocate pointer array for the vertex shader prefixes.
    if (unifiedShader)
        vshPrefix = new char*[(simP.gpu.numFShaders > simP.gpu.numVShaders)?simP.gpu.numFShaders:simP.gpu.numVShaders];
    else
        vshPrefix = new char*[simP.gpu.numVShaders];

    //  Check allocation.
    GPU_ASSERT(
        if (vshPrefix == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating vertex shader prefix array.");
    )

    u32bit numVShaders;
    if (unifiedShader)
        numVShaders = (simP.gpu.numFShaders > simP.gpu.numVShaders) ? simP.gpu.numFShaders : simP.gpu.numVShaders;
    else
        numVShaders = simP.gpu.numVShaders;
        
    //  Create prefixes for the virtual vertex shaders.
    for(i = 0; i < numVShaders; i++)
    {
        //  Allocate prefix.
        vshPrefix[i] = new char[6];

        GPU_ASSERT(
            if (vshPrefix[i] == NULL)
                panic("GPUSimulator", "GPUSimulator", "Error allocating vertex shader prefix.");
        )

        //  Write prefix.
        sprintf(vshPrefix[i], "VS%d", i);
    }

    //  Allocate pointer array for the fragment shader prefixes.
    fshPrefix = new char*[simP.gpu.numFShaders];

    //  Check allocation.
    GPU_ASSERT(
        if (fshPrefix == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating fragment shader prefix array.");
    )

    //  Create prefixes for the fragment shaders.
    for(i = 0; i < simP.gpu.numFShaders; i++)
    {
        //  Allocate prefix.
        fshPrefix[i] = new char[6];

        GPU_ASSERT(
            if (fshPrefix[i] == NULL)
                panic("GPUSimulator", "GPUSimulator", "Error allocating fragment shader prefix.");
        )

        //  Write prefix.
        sprintf(fshPrefix[i], "FS%d", i);
    }

    //  Allocate pointer array for the texture unit prefixes.
    tuPrefixes = new char*[simP.gpu.numFShaders * simP.fsh.textureUnits];

    //  Check allocation.
    GPU_ASSERT(
        if (tuPrefixes == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating texture unit prefix array.");
    )

    //  Create prefixes for the fragment shaders.
    for(i = 0; i < simP.gpu.numFShaders; i++)
    {
        for(j = 0; j < simP.fsh.textureUnits; j++)
        {
            //  Allocate prefix.
            tuPrefixes[i * simP.fsh.textureUnits + j] = new char[10];

            GPU_ASSERT(
                if (tuPrefixes[i * simP.fsh.textureUnits + j] == NULL)
                    panic("GPUSimulator", "GPUSimulator", "Error allocating texture unit prefix.");
            )

            //  Write prefix.
            sprintf(tuPrefixes[i * simP.fsh.textureUnits + j], "FS%dTU%d", i, j);
        }
    }

    //  Allocate pointer array for the stamp unit prefixes.
    suPrefix = new char*[simP.gpu.numStampUnits];

    //  Check allocation.
    GPU_ASSERT(
        if (suPrefix == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating stamp unit prefix array.");
    )

    //  Create prefixes for the stamp units.
    for(i = 0; i < simP.gpu.numStampUnits; i++)
    {
        //  Allocate prefix.
        suPrefix[i] = new char[6];

        GPU_ASSERT(
            if (suPrefix[i] == NULL)
                panic("GPUSimulator", "GPUSimulator", "Error allocating stamp unit prefix.");
        )

        //  Write prefix.
        sprintf(suPrefix[i], "SU%d", i);
    }

    //  Clear the box vector array.
    boxArray.clear();

    //  Determine clock operation mode.
    shaderClockDomain = (simP.gpu.gpuClock != simP.gpu.shaderClock);
    memoryClockDomain = (simP.gpu.gpuClock != simP.gpu.memoryClock);
    multiClock = shaderClockDomain || memoryClockDomain;
    
    //  In multi clock domain mode use multiple arrays.
    if (multiClock)
    {
        gpuDomainBoxes.clear();
        shaderDomainBoxes.clear();
        memoryDomainBoxes.clear();
    }

    //  Compute clock period for multiple clock domain mode.
    if (multiClock)
    {
        gpuClockPeriod = (u32bit) (1E6 / (f32bit) simP.gpu.gpuClock);
        shaderClockPeriod = (u32bit) (1E6 / (f32bit) simP.gpu.shaderClock);
        memoryClockPeriod = (u32bit) (1E6 / (f32bit) simP.gpu.memoryClock);
    }

    //  Shader clock domain is only supported with Vector Shader.
    if (shaderClockDomain && !simP.fsh.useVectorShader)
    {
        panic("GPUSimulator", "GPUSimulator", "Shader clock domain only supported by Vector Shader model.");
    }

    //  Memory clock domain is only supported with MemoryControllerV2
    if (memoryClockDomain && !simP.mem.memoryControllerV2)
    {
        panic("GPUSimulator", "GPUSimulator", "Memory clock domain only supported with MemoryControllerV2.");
    }

    numVShaders = unifiedShader ? simP.gpu.numFShaders : simP.gpu.numVShaders;
    
    //  Create and initialize the command processor.
    commProc = new CommandProcessor(
        trDriver,                                       //  Pointer to a Trace Drive to provide AGP transactions.
        numVShaders,                                    //  Number of vertex shader units (virtual for unified model) in the GPU.
        vshPrefix,                                      //  Prefixes for the vertex shader units.
        simP.gpu.numFShaders,                           //  Number of fragment shader units in the GPU.
        fshPrefix,                                      //  Prefixes for the fragment shader units.
        simP.gpu.numFShaders * simP.fsh.textureUnits,   //  Number of texture units in the GPU.
        tuPrefixes,                                     //  Prefixes for the Texture units.
        simP.gpu.numStampUnits,                         //  Number of stamp units (Z Stencil + Color Write) in the GPU.
        suPrefix,                                       //  Prefixes for the stamp (Z Stencil + Color Write) units.
        simP.mem.memSize,                               //  GPU Memory Size.
        simP.com.pipelinedBatches,                      //  Enable/disable pipelined batch rendering.
        simP.com.dumpShaderPrograms,                    //  Enable/disable dumping shader programs to files.
        "CommandProc", NULL);

    //  Add Command Processor box to the box array.
    boxArray.push_back(commProc);
    if (multiClock)
        gpuDomainBoxes.push_back(commProc);

    // Select a memory controller based on simulator parameters
    memController = createMemoryController(simP, (const char**) tuPrefixes, (const char**) suPrefix, (const char**) slPrefixes,
                                           "MemoryController", 0);

    //  Add Memory Controller box to the box array.
    boxArray.push_back(memController);
    if (multiClock && !memoryClockDomain)
        gpuDomainBoxes.push_back(memController);
    else if (multiClock && memoryClockDomain)
        memoryDomainBoxes.push_back((MultiClockBox *) memController);        
        

    //  NOTE:  INPUT CACHE IS FULLY ASSOCIATIVE AND MUST HAVE AT
    //  LEAST AS MANY LINES AS INPUT ATTRIBUTES OR IT WILL BECOME
    //  DEADLOCKED WHILE FETCHING ATTRIBUTES (CONFLICTS!!).

    /*  Create and initialize the streamer unit.  */
    streamer = new Streamer(
        simP.str.indicesCycle,        //  Indices processed per cycle.
        simP.str.idxBufferSize,       //  Index buffer size in bytes.
        simP.str.outputFIFOSize,      //  Output FIFO size.
        simP.str.outputMemorySize,    //  Output memory size.
        simP.gpu.numVShaders,         //  Number of attached vertex shaders.
        simP.vsh.outputLatency,       //  Maximum latency of the shader output signal.
        simP.str.verticesCycle,       //  Vertices sent to Primitive Assembly per cycle.
        simP.str.attrSentCycle,       //  Vertex attributes sent per cycle to Primitive Assembly.
        simP.pas.inputBusLat,         //  Latency of the vertex bus with Primitive Assembly.
        simP.str.streamerLoaderUnits, //  Number of Streamer Loader units.
        simP.str.slIndicesCycle,      //  Indices per cycle processed per Streamer Loader unit.
        simP.str.slInputReqQueueSize, //  Input request queue size (Streamer Loader unit).
        simP.str.slAttributesCycle,   //  Shader input attributes filled per cycle.
        simP.str.slInCacheLines,      //  Input cache lines.
        simP.str.slInCacheLineSz,     //  Input cache line size.
        simP.str.slInCachePortWidth,  //  Input cache port width in bytes.
        simP.str.slInCacheReqQSz,     //  Input cache request queue size.
        simP.str.slInCacheInputQSz,   //  Input cache read memory request queue size.
        simP.fsh.vAttrLoadFromShader, //  Force attribute load bypass.  Vertex attribute load is performed in the shader.
        slPrefixes,                   //  Streamer Loader prefixes.
        vshPrefix,                    //  Vertex Shader Prefixes.
        "Streamer",
        NULL);

    //  Add Streamer box to the box array.
    boxArray.push_back(streamer);
    if (multiClock)
        gpuDomainBoxes.push_back(streamer);
        
    //  Check non unified shader model before allocating the vertex shader boxes.
    if (!unifiedShader)
    {
        //  Allocate arrays of vertex shader boxes and emulator. 
        vshEmu = new ShaderEmulator*[simP.gpu.numVShaders];
        vshFetch = new ShaderFetch*[simP.gpu.numVShaders];
        vshDecExec = new ShaderDecodeExecute*[simP.gpu.numVShaders];

        //  Check allocation.
        GPU_ASSERT(
            if (vshEmu == NULL)
                panic("GPUSimulator", "GPUSimulator", "Error allocating vertex shader emulator array.");
            if (vshFetch == NULL)
                panic("GPUSimulator", "GPUSimulator", "Error allocating vertex shader fetch box array.");
            if (vshDecExec == NULL)
                panic("GPUSimulator", "GPUSimulator", "Error allocating vertex shader decode execute box array.");
        )


        //  Create the vertex shaders.
        for(i = 0; i < simP.gpu.numVShaders; i++)
        {
            //  Create Emulator name.
            sprintf(boxName,"VertSh%dEmulator", i);

            //  Create and initialize shader emulation object for VS1.
            vshEmu[i] = new ShaderEmulator(
                boxName,                                        //  Shader Emulator name.
                UNIFIED,                                        //  Shader Model.
                simP.vsh.numThreads + simP.vsh.numInputBuffers, //  Threads supported by the shader.
                false,                                          //  Store decoded instructions.
                NULL,                                           //  No texture emulator attached to the shader.
                1,                                              //  Fragments per stamp (for texture accesses).
                simP.ras.subPixelPrecision                      //  subpixel precision for shader fixed point operations.
                );

            //  Create Emulator name.
            sprintf(boxName,"VertexShaderFetch%d", i);

            //  Create and initialize shader fetch box for VS1.
            vshFetch[i] = new ShaderFetch(
                *vshEmu[i],                 //  Vertex Shader emulator.
                FALSE,                      //  Non unified shader model.
                simP.vsh.numThreads,        //  Number of threads in the shader.
                simP.vsh.numInputBuffers,   //  Number of input buffers in the shader.
                simP.vsh.numResources,      //  Number of thread resources (registers?).
                simP.vsh.threadRate,        //  Thread from which to fetch per cycle.
                simP.vsh.fetchRate,         //  Instructions to fetch per thread and cycle cycle.
                simP.vsh.threadGroup,       //  Number of threads that form a group.
                simP.vsh.lockedMode,        //  Normal per thread execution mode.
                simP.vsh.scalarALU,         //  Support fetching a scalar ALU in parallel with the SIMD ALU.
                simP.vsh.threadWindow,      //  Enable searching for a ready thread in the thread window.
                simP.vsh.fetchDelay,        //  Delay in cycles until the next fetch for a shader thread.
                simP.vsh.swapOnBlock,       //  Enable swap thread on block or swap thread always modes.
                0,                          //  No texture units.
                simP.vsh.inputsCycle,       //  Inputs that can be received per cycle.
                simP.vsh.outputsCycle,      //  Outputs that can be sent per cycle.
                simP.vsh.outputLatency,     //  Maximum output latency for the shader output signal.
                false,                      //  Microtriangle not mode not supported in non unified model.
                boxName,
                vshPrefix[i],
                NULL,
                NULL);

            //  Add Streamer box to the box array.
            boxArray.push_back(vshFetch[i]);

            //  Create Emulator name.
            sprintf(boxName,"VertexShaderDecExec%d", i);

            //  Create and initialize shader decode/execute box for VS1.
            vshDecExec[i] = new ShaderDecodeExecute(
                *vshEmu[i],                                     //  Vertex Shader emulator.
                FALSE,                                          //  Non unified shader model.
                simP.vsh.numThreads + simP.vsh.numInputBuffers, //  Number of threads supported.
                simP.vsh.threadRate,                            //  Thread from which to fetch per cycle.
                simP.vsh.fetchRate,                             //  Instructions to fetch per thread and cycle cycle.
                simP.vsh.threadGroup,                           //  Number of threads per group.
                simP.vsh.lockedMode,                            //  Normal per thread execution mode.
                0,                                              //  No texture unit attached to the shader.
                0,
                0,
                simP.vsh.scalarALU,                             //  Support a scalar ALU in parallel with the SIMD ALU.
                boxName,
                vshPrefix[i],
                NULL,
                NULL);

            //  Add Streamer box to the box array.
            boxArray.push_back(vshDecExec[i]);
        }
    }

    //  Create and initialize the Primitive Assembly unit.
    primAssem = new PrimitiveAssembly(
        simP.pas.verticesCycle,     //  Vertices per cycle received from the Streamer.
        simP.pas.trianglesCycle,    //  Triangles per cycle sent to the Clipper.
        simP.str.attrSentCycle,     //  Number of vertex attributes received per cycle from Streamer.
        simP.pas.inputBusLat,       //  Latency of the vertex bus from the Streamer.
        simP.pas.paQueueSize,       //  Number of vertices in the assembly queue.
        simP.clp.startLatency,      //  Start latency of the clipper test units.
        "PrimitiveAssembly", NULL);

    //  Add Primitive Assembly box to the box array.
    boxArray.push_back(primAssem);
    if (multiClock)
        gpuDomainBoxes.push_back(primAssem);        

    //  Create and initialize the Clipper.
    clipper = new Clipper(
        simP.clp.trianglesCycle,    //  Triangles received from Primitive Assembly per cycle.
        simP.clp.clipperUnits,      //  Number of clipper test units.
        simP.clp.startLatency,      //  Start latency for triangle clipping.
        simP.clp.execLatency,       //  Triangle clipping latency.
        simP.clp.clipBufferSize,    //  Clipped triangle buffer size.
        simP.ras.setupStartLat,     //  Start latency of the triangle setup units.
        simP.ras.trInputLat,        //  Latency of the triangle bus with Triangle Setup.
        "Clipper", NULL);

    //  Add Clipper box to the box array.
    boxArray.push_back(clipper);
    if (multiClock)
        gpuDomainBoxes.push_back(clipper);        

    //  Create and initialize a rasterizer emulator for Rasterizer.
    rastEmu = new RasterizerEmulator(
        simP.ras.emuStoredTriangles,    //  Active triangles.
        MAX_FRAGMENT_ATTRIBUTES,        //  Attributes per fragment.
        simP.ras.scanWidth,             //  Scan tile width in fragments.
        simP.ras.scanHeight,            //  Scan tile height in fragments.
        simP.ras.overScanWidth,         //  Over scan tile width in scan tiles.
        simP.ras.overScanHeight,        //  Over scan tile height in scan tiles.
        simP.ras.genWidth,              //  Generation tile width in fragments.
        simP.ras.genHeight,             //  Generation tile width in fragments.
        simP.ras.useBBOptimization,     //  Use the Bounding Box optimization pass (micropolygon rasterizer).
        simP.ras.subPixelPrecision      //  Precision bits for the decimal part of subpixel operations (micropolygon rasterizer).
        );
    
    u32bit threadGroup;  //  Set to the number of shader elements processed together and that must
                         //  also be issued or received to/from the shader processors as a group.
                         
    //  Check if vector shader model has been enabled.
    if (simP.fsh.useVectorShader)
    {
        threadGroup = simP.fsh.vectorLength;
    }
    else
    {
        threadGroup = simP.fsh.threadGroup;
    }
    
    //  Check if the micropolygon Rasterizer has been selected.
    if (simP.ras.useMicroPolRast)
    {
        panic("GPUSimulator", "GPUSimulator", "Micropolygon rasterizer not implemented.");
    }
    else
    {
        cout << "Using the Classic Rasterizer Unit." << endl;

        //  Create and initialize the classic rasterizer unit.
        rast = new Rasterizer(
            *rastEmu,                       //  Rasterizer emulator.
            simP.ras.trianglesCycle,        //  Number of triangles received from PA/CLP per cycle.
            simP.ras.setupFIFOSize,         //  Triangle Setup FIFO.
            simP.ras.setupUnits,            //  Number of triangle setup units.
            simP.ras.setupLat,              //  Latency of the triangle setup units.
            simP.ras.setupStartLat,         //  Start latency of the triangle setup units.
            simP.ras.trInputLat,            //  Latency of the triangle input bus (from PA/CLP).
            simP.ras.trOutputLat,           //  Latency of the triangle output bus (between TSetup and TTraversal).
            unifiedShader && simP.ras.shadedSetup,  //  Triangle setup performed in the unified shader unit.
            simP.ras.triangleShQSz,         //  Triangle shader queue in Triangle Setup.
            simP.ras.stampsCycle,           //  Stamps per cycle.
            simP.ras.samplesCycle,          //  MSAA samples per fragment and cycle that can be generated.
            simP.ras.overScanWidth,         //  Over scan tile width (scan tiles).
            simP.ras.overScanHeight,        //  Over scan tile height (scan tiles).
            simP.ras.scanWidth,             //  Scan tile width (pixels).
            simP.ras.scanHeight,            //  Scan tile height (pixels).
            simP.ras.genWidth,              //  Generation tile width (pixels).
            simP.ras.genHeight,             //  Generation tile height (pixels).
            simP.ras.rastBatch,             //  Triangles per rasterization batch (recursive traversal).
            simP.ras.batchQueueSize,        //  Triangles in the batch queue (triangle traversal).
            simP.ras.recursive,             //  Rasterization method (T: recursive, F: scanline).
            simP.ras.disableHZ,             //  Disables Hierarchical Z.
            simP.ras.stampsHZBlock,         //  Stamps (quads) per HZ block.
            simP.ras.hzBufferSize,          //  Size of the HZ Buffer Level 0.
            simP.ras.hzCacheLines,          //  Lines in the HZ cache.
            simP.ras.hzCacheLineSize,       //  Blocks in a HZ cache line.
            simP.ras.earlyZQueueSz,         //  Size of the early HZ test stamp queue.
            simP.ras.hzAccessLatency,       //  Access latency to the HZ Buffer Level 0.
            simP.ras.hzUpdateLatency,       //  Hierarchical Update signal latency.
            simP.ras.hzBlockClearCycle,     //  Clear HZ blocks per cycle.
            simP.ras.numInterpolators,      //  Interpolator units available.
            simP.ras.shInputQSize,          //  Shader input queue size (per shader unit).
            simP.ras.shOutputQSize,         //  Shader output queue size (per shader unit).
            simP.ras.shInputBatchSize,      //  Number of consecutive shader inputs assigned to a shader unit work batch.
            simP.ras.tiledShDistro,         //  Enable/disable distribution of fragment inputs to the shader units on a tile basis.
            unifiedShader,                              //  Unified shader architecture.
            unifiedShader ? simP.gpu.numVShaders : 0,   //  Number of virtual vertex shaders simulated by the Rasterizer.
            unifiedShader ? vshPrefix : NULL,           //  Prefixes for the virtual vertex shader units.
            simP.gpu.numFShaders,           //  Number of fragment shaders attached to the Rasterizer.
            fshPrefix,                      //  Prefixes for the fragment shader units.
            simP.fsh.inputsCycle,           //  Shader inputs per cycle and shader.
            simP.fsh.outputsCycle,          //  Shader outputs per cycle and shader.
            threadGroup,                    //  Threads in a shader processing group.
            simP.fsh.outputLatency,         //  Shader output signal maximum latency.
            unifiedShader ? simP.ras.vInputQSize : 0,   //  Vertex input queue size.
            unifiedShader ? simP.ras.vShadedQSize : 0,  //  Shaded vertex queue size.
            unifiedShader ? simP.ras.trInputQSize : 0,  //  Triangle input queue size.
            unifiedShader ? simP.ras.trOutputQSize : 0, //  Triangle output queue size.
            simP.ras.genStampQSize,         //  Stamps in the generated stamp queue (Fragment FIFO) (per stamp unit).
            simP.ras.testStampQSize,        //  Stamps in the early z tested stamp queue (Fragment FIFO).
            simP.ras.intStampQSize,         //  Stamps in the interpolated stamp queue (Fragment FIFO).
            simP.ras.shadedStampQSize,      //  Stamps in the shaded stamp queue (Fragment FIFO) (per stamp unit).
            simP.gpu.numStampUnits,         //  Number of stamp units attached to the Rasterizer.
            suPrefix,                       //  Prefixes for the stamp units.
            "Rasterizer_Unified", NULL);

        //  Add Rasterizer box to the box array.
        boxArray.push_back(rast);
        if (multiClock)
            gpuDomainBoxes.push_back(rast);
    }

    GPU_ASSERT(
        if (simP.fsh.textureUnits == 0)
            panic("GPUSimulator", "GPUSimulator", "The unified shader requires at least one texture unit attached to it.");
    )

    //  Allocate emulator and box arrays for the fragment shaders.
    texEmu = new TextureEmulator*[simP.gpu.numFShaders];
    fshEmu = new ShaderEmulator*[simP.gpu.numFShaders];
    fshFetch = new ShaderFetch*[simP.gpu.numFShaders];
    fshDecExec = new ShaderDecodeExecute*[simP.gpu.numFShaders];
    vecShFetch = new VectorShaderFetch*[simP.gpu.numFShaders];
    vecShDecExec = new VectorShaderDecodeExecute*[simP.gpu.numFShaders];
    textUnit = new TextureUnit*[simP.gpu.numFShaders * simP.fsh.textureUnits];

    //  Check allocation.
    GPU_ASSERT(
        if (texEmu == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating texture emulator array.");
        if (fshEmu == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating fragment shader emulator array.");
        if (fshFetch == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating fragment shader fetch array.");
        if (fshDecExec == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating fragment shader decode execute array.");
        if (vecShFetch == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating vector shader fetch array.");
        if (vecShDecExec == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating vector shader decode execute array.");
        if (textUnit == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating texture unit array.");
    )

    //  Create the fragment shader boxes and emulators.
    for(i = 0; i < simP.gpu.numFShaders; i++)
    {
        //  Create texture emulator for FS1.
        texEmu[i] = new TextureEmulator(
            STAMP_FRAGMENTS,                //  Fragments per stamp.
            simP.fsh.textBlockDim,          //  Texture block dimension (texels): 2^n x 2^n.
            simP.fsh.textSBlockDim,         //  Texture superblock dimension (blocks): 2^m x 2^m.
            simP.fsh.anisoAlgo,             //  Anisotropy algorithm selected.
            simP.fsh.forceMaxAniso,         //  Force the maximum anisotropy from the configuration file for all textures.
            simP.fsh.maxAnisotropy,         //  Maximum anisotropy allowed for any texture.
            simP.fsh.triPrecision,          //  Trilinear precision.
            simP.fsh.briThreshold,          //  Brilinear threshold.
            simP.fsh.anisoRoundPrec,        //  Aniso ratio rounding precision.
            simP.fsh.anisoRoundThres,       //  Aniso ratio rounding threshold.
            simP.fsh.anisoRatioMultOf2,     //  Aniso ratio must be multiple of two.
            simP.ras.overScanWidth,         //  Over scan tile width (scan tiles).
            simP.ras.overScanHeight,        //  Over scan tile height (scan tiles).
            simP.ras.scanWidth,             //  Scan tile width (pixels).
            simP.ras.scanHeight,            //  Scan tile height (pixels).
            simP.ras.genWidth,              //  Generation tile width (pixels).
            simP.ras.genHeight              //  Generation tile height (pixels).
            );

        //  Creat Emulator name.
        sprintf(boxName,"FragmentShaderEmulator%d", i);

        //  Check if vector shader model has been enabled.
        if (simP.fsh.useVectorShader)
        {
            //  Vector Shader model is only supported for the unified shader model.
            if (!unifiedShader)
                panic("GPUSimulator", "GPUSimulator", "Vector Shader model can only be used with unified shader model.");
            
            //  Create and initialize shader emulation object for FS1.
            fshEmu[i] = new ShaderEmulator(
                boxName,                                                //  Shader name.
                simP.ras.microTrisAsFragments? UNIFIED_MICRO : UNIFIED, //  Shader model. 
                simP.fsh.vectorThreads * simP.fsh.vectorLength,         //  Threads supported by the shader.
                false,                                                  //  Store decoded instructions.
                texEmu[i],                                              //  Pointer to the texture emulator attached to the shader.
                STAMP_FRAGMENTS,                                        //  Fragments per stamp for texture accesses.
                simP.ras.subPixelPrecision                              //  subpixel precision for shader fixed point operations. 
                );
                
            //  Creat Emulator name.
            sprintf(boxName,"VectorShaderFetch%d", i);

            //  Create and initialize vector shader fetch stage box.
            vecShFetch[i] = new VectorShaderFetch(
                *fshEmu[i],                     //  Fragment Shader emulator.
                simP.fsh.vectorThreads,          //  Number of shader threads.
                simP.fsh.vectorResources,       //  Number of thread resources.
                simP.fsh.vectorLength,          //  Vector length.
                simP.fsh.vectorALUWidth,        //  Vector ALU width.
                simP.fsh.vectorALUConfig,       //  Configuration of the vector array ALUs.
                simP.fsh.fetchDelay,            //  Delay in cycles until the next fetch for a shader thread.
                simP.fsh.swapOnBlock,           //  Enable swap thread on block or swap thread always modes.
                simP.fsh.textureUnits,          //  Texture Units attached to the shader.
                simP.fsh.inputsCycle,           //  Inputs that can be received per cycle.
                simP.fsh.outputsCycle,          //  Ouputs that can be sent per cycle.
                simP.fsh.outputLatency,         //  Shader output signal maximum latency.
                simP.ras.useMicroPolRast && simP.ras.microTrisAsFragments,  //  Process microtriangle fragment shader inputs and export z values (MicroPolygon Rasterizer).
                boxName,
                fshPrefix[i],
                vshPrefix[i],
                NULL);

            //  Add VectorShaderFetch box to the box array.
            boxArray.push_back(vecShFetch[i]);
            if (multiClock && shaderClockDomain)
                shaderDomainBoxes.push_back(vecShFetch[i]);
            else if (multiClock && !shaderClockDomain)
                gpuDomainBoxes.push_back(vecShFetch[i]);

            //  Creat Emulator name.
            sprintf(boxName,"VectorShaderDecExec%d", i);

            //  Compute the ratio between the texture unit clock and the shader clock.
            u32bit textureClockRatio = shaderClockDomain ? ((u32bit) ceilf((f32bit) simP.gpu.gpuClock / (f32bit) simP.gpu.shaderClock)) : 1;
            
            //  Create and initialize vector shader decode/execute stages box.
            vecShDecExec[i] = new VectorShaderDecodeExecute(
                *fshEmu[i],
                simP.fsh.vectorThreads,         //  Number of supported threads. 
                simP.fsh.vectorLength,          //  Vector length.
                simP.fsh.vectorALUWidth,        //  Vector ALU width.
                simP.fsh.vectorALUConfig,       //  Configuration of the vector array ALUs.
                simP.fsh.vectorWaitOnStall,     //  Flag that defines if an instruction waits until stall conditions are cleared or is discarded.
                simP.fsh.vectorExplicitBlock,   //  Flag that defines if threads are blocked by an explicit trigger in the instruction stream or automatically on texture load.  */
                textureClockRatio,              //  Ratio between the texture unit clock and the shader clock.  Used for buffering between Texture Unit and Shader Decode Execute.
                simP.fsh.textureUnits,          //  Texture Units attached to the shader.
                simP.fsh.textRequestRate,       //  Texture request rate.  
                simP.fsh.textRequestGroup,      //  Size of a group of texture requests sent to a texture unit.
                boxName,
                fshPrefix[i],
                vshPrefix[i],
                NULL);

            //  Add VectorShaderDecodeExecute box to the box array.
            boxArray.push_back(vecShDecExec[i]);
            if (multiClock && shaderClockDomain)
                shaderDomainBoxes.push_back(vecShDecExec[i]);
            else if (multiClock && !shaderClockDomain)
                gpuDomainBoxes.push_back(vecShDecExec[i]);                
        }
        else
        {
            //  Create and initialize shader emulation object for FS1.
            fshEmu[i] = new ShaderEmulator(
                boxName,                                                //  Shader name.
                simP.ras.microTrisAsFragments? UNIFIED_MICRO : UNIFIED, //  Shader model.
                simP.fsh.numThreads + simP.fsh.numInputBuffers,         //  Threads supported by the shader.
                false,                                                  //  Store decoded instructions.
                texEmu[i],                                              //  Pointer to the texture emulator attached to the shader.
                STAMP_FRAGMENTS,                                        //  Fragments per stamp for texture accesses.
                simP.ras.subPixelPrecision                              //  Subpixel precision for shader fixed point operations.
                );

            //  Creat Emulator name.
            sprintf(boxName,"FragmentShaderFetch%d", i);

            //  Create and initialize shader fetch box for FS1.
            fshFetch[i] = new ShaderFetch(
                *fshEmu[i],                     //  Fragment Shader emulator.
                unifiedShader,                  //  Unified shader model.
                simP.fsh.numThreads,            //  Number of shader threads.
                simP.fsh.numInputBuffers,       //  Number of input buffers.
                simP.fsh.numResources,          //  Number of thread resources.
                simP.fsh.threadRate,            //  Thread from where to fetch per cycle.
                simP.fsh.fetchRate,             //  Instructions to fetch per cycle and thread.
                simP.fsh.threadGroup,           //  Number of threads that form a group.
                simP.fsh.lockedMode,            //  Lock step execution mode of a group of threads.
                simP.fsh.scalarALU,             //  Support fetching a scalar ALU in parallel with the SIMD ALU.
                simP.fsh.threadWindow,          //  Enable searching for a ready thread in the thread window.
                simP.fsh.fetchDelay,            //  Delay in cycles until the next fetch for a shader thread.
                simP.fsh.swapOnBlock,           //  Enable swap thread on block or swap thread always modes.
                simP.fsh.textureUnits,          //  Texture Units attached to the shader.
                simP.fsh.inputsCycle,           //  Inputs that can be received per cycle.
                simP.fsh.outputsCycle,          //  Ouputs that can be sent per cycle.
                simP.fsh.outputLatency,         //  Shader output signal maximum latency.
                simP.ras.useMicroPolRast && simP.ras.microTrisAsFragments,  //  Process microtriangle fragment shader inputs and export z values (MicroPolygon Rasterizer). 
                boxName,
                fshPrefix[i],
                unifiedShader ? vshPrefix[i] : NULL,
                NULL);

            //  Add ShaderFetch box to the box array.
            boxArray.push_back(fshFetch[i]);

            //  Creat Emulator name.
            sprintf(boxName,"FragmentShaderDecExec%d", i);

            //  Create and initialize shader decode/execute box for FS1.
            fshDecExec[i] = new ShaderDecodeExecute(
                *fshEmu[i],
                unifiedShader,                                  //  Unified shader model.
                simP.fsh.numThreads + simP.fsh.numInputBuffers, //  Number of supported threads.
                simP.fsh.threadRate,                            //  Thread from where to fetch per cycle.
                simP.fsh.fetchRate,                             //  Instructions to fetch per cycle and thread.
                simP.fsh.threadGroup,                           //  Number of threads per group.
                simP.fsh.lockedMode,                            //  Execute in lock step mode a group of threads.
                simP.fsh.textureUnits,                          //  Texture Units attached to the shader.
                simP.fsh.textRequestRate,                       //  Texture request rate.
                simP.fsh.textRequestGroup,                      //  Size of a group of texture requests sent to a texture unit.
                simP.fsh.scalarALU,                             //  Support a scalar ALU in parallel with the SIMD ALU.
                boxName,
                fshPrefix[i],
                unifiedShader ? vshPrefix[i] : NULL,
                NULL);

            //  Add ShaderDecodeExecute box to the box array.
            boxArray.push_back(fshDecExec[i]);
        }
        
        //  Create the attached texture units.
        for(j = 0; j < simP.fsh.textureUnits; j++)
        {
            //  Creat Emulator name.
            sprintf(boxName, "TextureUnitFSh%d-%d", i, j);
            sprintf(tuPrefix, "%sTU%d", fshPrefix[i], j);

            //  Create and initialize texture unit box for FS1.
            textUnit[i * simP.fsh.textureUnits + j] = new TextureUnit(
                *texEmu[i],                     //  Texture Emulator.
                STAMP_FRAGMENTS,                //  Fragments per stamp.
                simP.fsh.textRequestRate,       //  Texture request rate.
                simP.fsh.textReqQSize,          //  Texture Access Request queue size.
                simP.fsh.textAccessQSize,       //  Texture Access queue size.
                simP.fsh.textResultQSize,       //  Texture Result queue size.
                simP.fsh.textWaitWSize,         //  Texture Wait Read window size.
                simP.fsh.twoLevelCache,         //  Enable two level or single level texture cache version.
                simP.fsh.txCacheLineSz,         //  Size of the Texture Cache lines in bytes (L0 for two level version).
                simP.fsh.txCacheWays,           //  Number of ways in the Texture Cache (L0 for two level version).
                simP.fsh.txCacheLines,          //  Number of lines in the Texture Cache (L0 for two level version).
                simP.fsh.txCachePortWidth,      //  Size of the Texture Cache ports.
                simP.fsh.txCacheReqQSz,         //  Size of the Texture Cache request queue.
                simP.fsh.txCacheInQSz,          //  Size of the Texture Cache input request queue (L0 for two level version).
                simP.fsh.txCacheMissCycle,      //  Missses per cycle supported by the Texture Cache.
                simP.fsh.txCacheDecomprLatency, //  Compressed texture line decompression latency.
                simP.fsh.txCacheLineSzL1,       //  Size of the Texture Cache lines in bytes (L1 for two level version).
                simP.fsh.txCacheWaysL1,         //  Number of ways in the Texture Cache (L1 for two level version).
                simP.fsh.txCacheLinesL1,        //  Number of lines in the Texture Cache (L1 for two level version).
                simP.fsh.txCacheInQSzL1,        //  Size of the Texture Cache input request queue (L1 for two level version).
                simP.fsh.addressALULat,         //  Latency of the address ALU.
                simP.fsh.filterLat,             //  Latency of the filter unit.
                boxName,                        //  Unit name.
                tuPrefix,                       //  Unit prefix.
                NULL                            //  Parent box.
                );

            //  Add Texture Unit box to the box array.
            boxArray.push_back(textUnit[i * simP.fsh.textureUnits + j]);
            if (multiClock)
                gpuDomainBoxes.push_back(textUnit[i * simP.fsh.textureUnits + j]);
        }
    }

    //  Allocate emulator and box arrays for the stamp units.
    fragEmu = new FragmentOpEmulator*[simP.gpu.numStampUnits];
    zStencilV2 = new ZStencilTestV2*[simP.gpu.numStampUnits];
    colorWriteV2 = new ColorWriteV2*[simP.gpu.numStampUnits];

    //  Check allocation.
    GPU_ASSERT(
        if (fragEmu == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating fragment emulator array.");
        if (zStencilV2 == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating Z Stencil Test array.");
        if (colorWriteV2 == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating Color Write array.");
    )


    //  Create the fragment shader boxes and emulators.
    for(i = 0; i < simP.gpu.numStampUnits; i++)
    {
        //  Create fragment operations emulator.
        fragEmu[i] = new FragmentOpEmulator(
            STAMP_FRAGMENTS                 //  Fragments per stamp.
        );

        //  Creat Emulator name.
        sprintf(boxName,"ZStencilTest%d", i);

        //  Create Z Stencil Test box.
        zStencilV2[i] = new ZStencilTestV2(
            simP.zst.stampsCycle,           //  Stamps per cycle.
            simP.ras.overScanWidth,         //  Over scan tile width (scan tiles).
            simP.ras.overScanHeight,        //  Over scan tile height (scan tiles).
            simP.ras.scanWidth,             //  Scan tile width (pixels).
            simP.ras.scanHeight,            //  Scan tile height (pixels).
            simP.ras.genWidth,              //  Generation tile width (pixels).
            simP.ras.genHeight,             //  Generation tile height (pixels).
            simP.zst.disableCompr,          //  Disables Z compression.
            simP.zst.zCacheWays,            //  Z cache set associativity.
            simP.zst.zCacheLines,           //  Cache lines (per way).
            simP.zst.zCacheLineStamps,      //  Stamps per cache line.
            simP.zst.zCachePortWidth,       //  Width in bytes of the Z cache ports.
            simP.zst.extraReadPort,         //  Additional read port in the cache.
            simP.zst.extraWritePort,        //  Additional write port in the cache.
            simP.zst.zCacheReqQSz,          //  Z Cache memory request queue size.
            simP.zst.zCacheInQSz,           //  Z Cache input queue.
            simP.zst.zCacheOutQSz,          //  Z cache output queue.
            simP.gpu.numStampUnits,         //  Number of stamp units in the GPU.
            simP.zst.blockStateMemSz,       //  Z blocks in state memory (max res 2048 x 2048 with 64 pixel lines.
            simP.zst.blockClearCycle,       //  State memory blocks cleared per cycle.
            simP.zst.comprLatency,          //  Z block compression cycles.
            simP.zst.decomprLatency,        //  Z block decompression cycles.
            simP.zst.inputQueueSize,        //  Input stamps queue.
            simP.zst.fetchQueueSize,        //  Fetched stamp queue.
            simP.zst.readQueueSize,         //  Read stamp queue.
            simP.zst.opQueueSize,           //  Operated stamp queue.
            simP.zst.writeQueueSize,        //  Written stamp queue.
            simP.zst.zTestRate,             //  Z test rate for stamps (cycles between stamps).
            simP.zst.zOpLatency,            //  Z operation latency.
            simP.ras.disableHZ,             //  Disables HZ.
            simP.ras.hzUpdateLatency,       //  Hierarchical Z update signal latency.
            simP.dac.blockUpdateLat,        //  Z Stencil block update signal latency.
            *fragEmu[i],                    //  Reference to the Fragment Operation Emulator.
            *rastEmu,                       //  Reference to the Rasterizer Emulator.
            boxName,                        //  Box name.
            suPrefix[i],                    //  Unit prefix.
            NULL                            //  Box parent box.
            );

        //  Add Z Stencil Test box to the box array.
        boxArray.push_back(zStencilV2[i]);
        if (multiClock)
            gpuDomainBoxes.push_back(zStencilV2[i]);
            
        // Creat Emulator name.
        sprintf(boxName,"ColorWrite%d", i);

        //  Create color write box.
        colorWriteV2[i] = new ColorWriteV2(
            simP.cwr.stampsCycle,           //  Stamps per cycle.
            simP.ras.overScanWidth,         //  Over scan tile width (scan tiles).
            simP.ras.overScanHeight,        //  Over scan tile height (scan tiles).
            simP.ras.scanWidth,             //  Scan tile width (pixels).
            simP.ras.scanHeight,            //  Scan tile height (pixels).
            simP.ras.genWidth,              //  Generation tile width (pixels).
            simP.ras.genHeight,             //  Generation tile height (pixels).
            simP.cwr.disableCompr,          //  Disables Color compression.
            simP.cwr.cCacheWays,            //  Color cache set associativity.
            simP.cwr.cCacheLines,           //  Cache lines (per way).
            simP.cwr.cCacheLineStamps,      //  Stamps per cache line.
            simP.cwr.cCachePortWidth,       //  Width in bytes of the Color cache ports.
            simP.cwr.extraReadPort,         //  Additional read port in the cache.
            simP.cwr.extraWritePort,        //  Additional write port in the cache.
            simP.cwr.cCacheReqQSz,          //  Color Cache memory request queue size.
            simP.cwr.cCacheInQSz,           //  Color Cache input queue.
            simP.cwr.cCacheOutQSz,          //  Color cache output queue.
            simP.gpu.numStampUnits,         //  Number of stamp units in the GPU.
            simP.cwr.blockStateMemSz,       //  Color blocks in state memory (max res 2048 x 2048 with 64 pixel lines.
            simP.cwr.blockClearCycle,       //  State memory blocks cleared per cycle.
            simP.cwr.comprLatency,          //  Color block compression cycles.
            simP.cwr.decomprLatency,        //  Color block decompression cycles.
            simP.cwr.inputQueueSize,        //  Input stamps queue.
            simP.cwr.fetchQueueSize,        //  Fetched stamp queue.
            simP.cwr.readQueueSize,         //  Read stamp queue.
            simP.cwr.opQueueSize,           //  Operated stamp queue.
            simP.cwr.writeQueueSize,        //  Written stamp queue.
            simP.cwr.blendRate,             //  Rate for stamp blending (cycles between stamps).
            simP.cwr.blendOpLatency,        //  Blend operation latency.
            simP.dac.blockUpdateLat,        //  Color block update signal latency.
            simP.fragmentMap ? simP.fragmentMapMode : DISABLE_MAP,  //  Fragment map mode.
            *fragEmu[i],                    //  Reference to the Fragment Operation Emulator.
            boxName,                        //  Box name.
            suPrefix[i],                    //  Unit prefix.
            NULL                            //  Box parent box.
            );

        //  Add Color Write box to the box array.
        boxArray.push_back(colorWriteV2[i]);
        if (multiClock)
            gpuDomainBoxes.push_back(colorWriteV2[i]);
    }

    //  Create DAC box.
    dac = new DAC(
        simP.ras.overScanWidth,         //  Over scan tile width (scan tiles).
        simP.ras.overScanHeight,        //  Over scan tile height (scan tiles).
        simP.ras.scanWidth,             //  Scan tile width (pixels).
        simP.ras.scanHeight,            //  Scan tile height (pixels).
        simP.ras.genWidth,              //  Generation tile width (pixels).
        simP.ras.genHeight,             //  Generation tile height (pixels).
        simP.fsh.textBlockDim,          //  Texture block dimension (texels): 2^n x 2^n.
        simP.fsh.textSBlockDim,         //  Texture superblock dimension (blocks): 2^m x 2^m.
        simP.dac.blockSize,             //  Size of an uncompressed block in bytes.
        simP.dac.blockUpdateLat,        //  Block state update signal latency.
        simP.dac.blocksCycle,           //  Block states updated per cycle.
        simP.dac.blockReqQSz,           //  Number of block request queue entries.
        simP.dac.decomprLatency,        //  Block decompression latency.
        simP.gpu.numStampUnits,         //  Number of stamp units attached to the DAC.
        suPrefix,                       //  Prefixes for the stamp units.
        simP.startFrame,                //  Number of the first frame to dump.
        simP.dac.refreshRate,           //  Frame refresh/dumping in cycles.
        simP.dac.synchedRefresh,        //  Swap synchronized with frame refresh/dumping.
        simP.dac.refreshFrame,          //  Enable frame refresh/dump.
        simP.dac.saveBlitSource,        //  Save blit source data to a PPM file.
        "DAC",                          //  Box name.
        NULL                            //  Parent box.
    );

    //  Add DAC box to the box array.
    boxArray.push_back(dac);
    if (multiClock)
        gpuDomainBoxes.push_back(dac);
        
    //  Check that all the signals are well defined.
    if (!sigBinder.checkSignalBindings())
    {
        //  Dump content of the signal binder.
        sigBinder.dump();

        panic("GPUSimulator", "GPUSimulator", "Signals Undefined.");
    }

    //  Check if signal trace dump is enabled.
    if (simP.dumpSignalTrace)
    {
        //  Signal tracing not supported with multiple clock domains.
        if (multiClock)
            panic("GPUSimulator", "GPUSimulator", "Signal Trace Dump not supported with multiple clock domains.");
        
        //  Try to open the signal trace file.
        sigTraceFile.open(simP.signalDumpFile, ios::out | ios::binary);

        if (!sigTraceFile.is_open())
        {
            panic("GPUSimulator", "GPUSimulator", "Error opening signal trace file.");
        }

        /*  Initialize the signal tracer.  */
        sigBinder.initSignalTrace(&sigTraceFile);

        printf("!!! GENERATING SIGNAL TRACE !!!\n");
    }

    //  Check if statistics generation is enabled.
    if (simP.statistics)
    {
        //  Set statistics rate 
        GPUStatistics::StatisticsManager::instance().setDumpScheduling(0, simP.statsRate);

        //  Check if per cycle statistics are enabled
        if (simP.perCycleStatistics)
        {
            //  Initialize the statistics manager.
            out.open(simP.statsFile, ios::out | ios::binary);

            if (!out.is_open())
                panic("GPUSimulator", "GPUSimulator", "Error opening per cycle statistics file");

            GPUStatistics::StatisticsManager::instance().setOutputStream(out);
        }

        //  Check if per frame statistics are enabled.
        if (simP.perFrameStatistics)
        {
            outFrame.open(simP.statsFilePerFrame, ios::out | ios::binary);

            if (!outFrame.is_open())
                panic("GPUSimulator", "GPUSimulator", "Error opening per frame statistics file");

            GPUStatistics::StatisticsManager::instance().setPerFrameStream(outFrame);
        }

        //  Check if per batch statistics are enabled.
        if (simP.perBatchStatistics)
        {
            outBatch.open(simP.statsFilePerBatch, ios::out | ios::binary);

            if (!outBatch.is_open())
                panic("GPUSimulator", "GPUSimulator", "Error opening per batch statistics file");

            GPUStatistics::StatisticsManager::instance().setPerBatchStream(outBatch);
        }
    }

    //  Allocate the pointers for the fragment latency maps.
    latencyMap = new u32bit*[simP.gpu.numStampUnits];

    //  Check allocation.
    GPU_ASSERT(
        if (latencyMap == NULL)
            panic("GPUSimulator", "GPUSimulator", "Error allocating latency map array.");
    )

    //  Initialize the latency map pointers.
    for(i = 0; i < simP.gpu.numStampUnits; i++)
        latencyMap[i] = NULL;

    //  Set frame counter as start frame.
    frameCounter = simP.startFrame;
    
    //  Reset batch counters.
    batchCounter = 0;
    frameBatch = 0;
    
    //  Auto snapshot variables.
    pendingSaveSnapshot = false;
    autoSnapshotEnable = false;    
    snapshotFrequency = 1;
}

GPUSimulator::~GPUSimulator()
{
    //   Close all output files.
    if (sigTraceFile.is_open())
        sigTraceFile.close();
    if (out.is_open())
        out.close();
    if (outFrame.is_open())
        outFrame.close();
    if (outBatch.is_open())
        outBatch.close();

    //  Print end message.
    printf("\n\n");
    printf("End of simulation\n");
    printf("\n\n");

    for(u32bit i = 0; i < boxArray.size(); i++)
        delete boxArray[i];
}

void GPUSimulator::createSnapshot()
{
    // Check if the simulation started.
    if (current->simulationStarted)
    {    
        //  Create a directory for the snapshot files.
        u32bit snapshotID = 0;
        bool directoryCreated = false;
        bool tooManySnapshots = false;
        bool error = false;
        char directoryName[100];
        while (!directoryCreated && !error && !tooManySnapshots)
        {
            sprintf(directoryName, "ATTILAsnapshot%02d", snapshotID);
            
            s32bit res = createDirectory(directoryName);                
            
            if (res == 0)
                directoryCreated = true;
            else if (res == DIRECTORY_ALREADY_EXISTS)
            {
                if (snapshotID < 99)
                    snapshotID++;
                else
                    tooManySnapshots = true;
            }
            else
                error = true;         
        }
        
        //  Check errors creating the snapshot directory.
        if (error)
            printf(" Error creating snapshot directory: %s\n", directoryName);
        
        if (tooManySnapshots)
            printf(" Too many snapshots already present in the current working directory\n");

        if (directoryCreated)
        {
            char workingDirectory[1024];

            if (getCurrentDirectory(workingDirectory, 1024) != 0)
                panic("GPUSimulator", "createSnapshot", "Error obtaining current working directory.");
                
            //  Change working directory to the snapshot directory.            
            if (changeDirectory(directoryName) == 0)
            {
                printf(" Saving a simulator snapshot in %s directory\n", directoryName);
                 
                stringstream comstream;

                printf(" Saving simulator state.\n");
                current->saveSimState();
                
                printf(" Saving configuration.\n");
                current->saveSimConfig();
                
                printf(" Saving GPU Registers snapshot.\n");
                comstream.clear();
                comstream.str("_saveregisters");                    
                current->commProc->execCommand(comstream);
                
                printf(" Saving Hierarchical Z Buffer snapshot.\n");
                current->rast->saveHZBuffer();

                for(u32bit i = 0; i < current->simP.gpu.numStampUnits; i++)
                {
                    printf(" Saving Z Stencil Cache %d Block State Memory snapshot\n", i);
                    current->zStencilV2[i]->saveBlockStateMemory();
                    printf(" Saving Color Cache %d Block State Memory snapshot\n", i);
                    current->colorWriteV2[i]->saveBlockStateMemory();
                }

                printf(" Saving Memory snapshot\n");
                comstream.clear();
                comstream.str("_savememory");
                current->memController->execCommand(comstream);


                if (changeDirectory(workingDirectory) != 0)
                    panic("GPUSimulator", "createSnapshot", "Error changing back to working directory.");
            }
            else
            {
                printf(" Error changing working directory to snapshot directory.\n");
            }
        }        
    }
}

void GPUSimulator::saveSimState()
{
    ofstream out;
    
    out.open("state.snapshot");
    
    if (!out.is_open())
        panic("GPUSimulator", "saveSimState", "Error creating file for simulator state snapshot.");
        
    out << "Frame = " << frameCounter << endl;
    out << "Batch = " << batchCounter << endl;
    out << "Frame Batch = " << frameBatch << endl; 

    if (multiClock)
    {
        out << "MultiClock" << endl;
        out << "GPU Cycle = " << gpuCycle << endl;
        if (shaderClockDomain)
            out << "Shader Cycle = " << shaderCycle << endl;
        if (memoryClockDomain)
            out << "Memory Cycle = " << memoryCycle << endl;
    }
    else
        out << "Cycle = " << cycle << endl;

    if (d3d9Trace)
    {
        out << "D3D9 PIXRun tracefile" << endl;
        
        if (simP.useACD)
            out << "Using ACD" << endl;
            
        out << "Last processed EID = " << trDriver->getTracePosition();
    }
    
    if (oglTrace)
    {
        out << "GLInterceptor OpenGL tracefile" << endl;

        if (simP.useACD)
            out << "Using ACD" << endl;

        out << "Last processed line = " << trDriver->getTracePosition();
    }    

    if (agpTrace)
    {    
        out << "AGP Transaction tracefile" << endl;
        out << "Last processed transaction = " << trDriver->getTracePosition();
    }
    
    out.close();    
}

void GPUSimulator::saveSimConfig()
{
    ofstream out;
    
    out.open("config.snapshot");
    
    if (!out.is_open())
        panic("GPUSimulator", "saveSimConfig", "Error creating file for simulator config snapshot.");

    out.write((char *) &simP, sizeof(simP));
    out.close();    
}


// Simulator debug loop.
void GPUSimulator::debugLoop(bool validate)
{
    //  Check if validation mode is enabled.
    if (validate)
    {
        validationMode = true;
        
        skipValidation = false;
        
        //  Create the GPU emulator.
        gpuEmulator = new GPUEmulator(simP, trDriver);
        
        //  Reset the GPU emulator.
        gpuEmulator->resetState();
        
        //  Enable validation mode.
        gpuEmulator->setValidationMode(true);
        
        //  Enable the validation mode in the simulator Command Processor.
        commProc->setValidationMode(true);
        
        //  Enable the validation mode in the Streamer.
        streamer->setValidationMode(true);
        
        //  Enable validation mode in the Z Stencil Test and Color Write units.
        for(u32bit rop = 0; rop < simP.gpu.numStampUnits; rop++)
        {
            zStencilV2[rop]->setValidationMode(true);
            colorWriteV2[rop]->setValidationMode(true);
        }
    }
    
    string line;
    string command;
    stringstream lineStream;
    LineReader *lr = &LineReader::instance();

    endDebug = false;
    simulationStarted = true;

    lr->initialize();

    current = this;

    //  Initialize clock state.
    if (multiClock)
    {
        nextGPUClock = gpuClockPeriod;
        nextShaderClock = shaderClockPeriod;
        nextMemoryClock = memoryClockPeriod;
        gpuCycle = 0;
        shaderCycle = 0;
        memoryCycle = 0;
    }
    else
    {
        cycle = 0;
    }
    
    while(!endDebug)
    {
        lr->readLine("gpu> ", line);

        lineStream.clear();
        lineStream.str(line);

        command.clear();
        lineStream >> ws;
        lineStream >> command;

        if (!command.compare("quit"))
            endDebug = true;
        else if (!command.compare("run"))
            runCommand(lineStream);
        else if (!command.compare("runbatch"))
            runBatchCommand(lineStream);
        else if (!command.compare("runframe"))
            runFrameCommand(lineStream);
        else if (!command.compare("runcom"))
            runComCommand(lineStream);
        else if (!command.compare("skipbatch"))
            skipBatchCommand(lineStream);
        else if (!command.compare("skipframe"))
            skipFrameCommand(lineStream);
        else if (!command.compare("state"))
            stateCommand(lineStream);
        else if (!command.compare("help"))
            helpCommand(lineStream);
        else if (!command.compare("listboxes"))
            listBoxesCommand(lineStream);
        else if (!command.compare("listcommands"))
            listBoxCommCommand(lineStream);
        else if (!command.compare("execcommand"))
            execBoxCommCommand(lineStream);
        else if (!command.compare("debugmode"))
            debugModeCommand(lineStream);
        else if (!command.compare("emulatortrace"))
            emulatorTraceCommand(lineStream);
        else if (!command.compare("tracevertex"))
            traceVertexCommand(lineStream);
        else if (!command.compare("tracefragment"))
            traceFragmentCommand(lineStream);
        else if (!command.compare("memoryUsage"))
            driverMemoryUsageCommand(lineStream);
        else if (!command.compare("listMDs"))
            driverListMDsCommand(lineStream);
        else if (!command.compare("infoMD"))
            driverInfoMDCommand(lineStream);
        else if (!command.compare("dumpAGPBuffer"))
            driverDumpAGPBufferCommand(lineStream);
        else if (!command.compare("memoryAlloc"))
            driverMemAllocCommand(lineStream);
        //else if (!command.compare("glcontext"))
        //    libraryGLContextCommand(lineStream);
        //else if (!command.compare("dumpStencil"))
        //    libraryDumpStencilCommand(lineStream);
        else if (!command.compare("savesnapshot"))
            saveSnapshotCommand();
        else if (!command.compare("loadsnapshot"))
            loadSnapshotCommand(lineStream);
        else if (!command.compare("autosnapshot"))
            autoSnapshotCommand(lineStream);
        else
            cout << "Unsupported command." << endl;
    }

    lr->restore();
}

//  Displays the simulator debug inline help
void GPUSimulator::helpCommand(stringstream &comStream)
{
    cout << "Supported commands: " << endl << endl;

    cout << "help          - Displays information about the simulator debug commands" << endl;
    cout << "listboxes     - List the names of all the simulation boxes" << endl;
    cout << "listcommands  - List the commands available for a box" << endl;
    cout << "execcommand   - Executes a box command" << endl;
    cout << "debugmode     - Sets the debug mode in a box" << endl;
    cout << "emulatortrace - Enables/disables emulator trace logs (validation mode only)" << endl;
    cout << "tracevertex   - Enables/disables tracing the execution of a defined vertex in the shader processors" << endl;
    cout << "tracefragment - Enables/disables tracing the execution of a defined fragment in the shader processors" << endl;
    cout << "run           - Starts the simulation of n cycles" << endl;
    cout << "runframe      - Starts the simulation of n frames" << endl;
    cout << "runbatch      - Starts the simulation of n batches" << endl;
    cout << "runcom        - Starst the simulation of n AGP commands" << endl;
    cout << "skipframe     - Skips the simulation of n frames (DRAW commands ignored)" << endl;
    cout << "skipbatch     - Skips the simulation of n batches (DRAW commands ignored)" << endl;
    cout << "state         - Displays the current simulator state" << endl;
    cout << "              - Displays information about the simulator boxes state" << endl;
    cout << "savesnapshot  - Saves a snapshot of the GPU state to disk" << endl;
    cout << "loadsnapshot  - Loads a snapshot of the GPU state from disk" << endl;
    cout << "autosnapshot  - Sets automatic snapshot saves" << endl;
    cout << "memoryUsage   - Displays information about the memory used" << endl;
    cout << "listMDs       - Lists the memory descriptors in the GPU Driver" << endl;
    cout << "infoMD        - Displays the information about a memory descriptor" << endl;
    cout << "dumpAGPBuffer - Dumps the content of the AGP Buffer in the GPU Driver" << endl;
    cout << "memoryAlloc   - Dumps the content of the memory allocation structure in the GPU Driver" << endl;
    //cout << "glcontext     - Displays the state of the OpenGL context" << endl;
    //cout << "dumpStencil   - Dumps the stencil buffer as the next frame using a OpenGL library hack" << endl;
    cout << endl;
}

void GPUSimulator::listBoxesCommand(stringstream &comStream)
{
    Box::dumpNameBoxes();
    cout << endl;
}

//  Simulator debug run command function.  Simulates n cycles
void GPUSimulator::runCommand(stringstream &comStream)
{
    bool endOfBatch = false;
    bool endOfFrame = false;
    bool endOfTrace = false;
    bool gpuStalled = false;
    bool validationError = false;
    u64bit simCycles = 0;

    comStream >> ws;

    //  Check if there is a parameter after the command.
    if (comStream.eof())
    {
        simCycles = 1;
    }
    else
    {
        //  Read the parameter as a 64-bit integer.
        comStream >> simCycles;

        //  Check if the parameter was correctly read.
        if (comStream.fail())
            simCycles = 0;
    }

    //  Check if there cycles to simulate
    if (simCycles == 0)
        cout << "Usage: run <simulation cycles>" << endl;
    else
    {
            cout << "-> Simulating " << simCycles << " cycles" << endl;

        //  Reset abort flag
        abortDebug = false;

        bool end = false;
        u64bit firstCycle = multiClock ? gpuCycle : cycle;
        
        //  Simulate n cycles
        while(!end && !endOfTrace && !abortDebug)
        {       
            // Clock all the boxes.
            advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);
            
            //  Check if all the requested cycles were simulated.
            end = ( multiClock && (gpuCycle == (firstCycle + simCycles))) ||
                  (!multiClock && (cycle == (firstCycle + simCycles)));
            
                
            //  End the simulation if a stall was detected.                  
            end = end || gpuStalled || validationError;
        }

        cout << endl;
    }
}

//  Simulator debug runframe command function.  Simulates n frames
void GPUSimulator::runFrameCommand(stringstream &comStream)
{
    bool endOfBatch = false;
    bool endOfFrame = false;
    bool endOfTrace = false;
    bool gpuStalled = false;
    bool validationError = false;
    u32bit simFrames = 0;

    //  Skip white spaces
    comStream >> ws;

    //  Check if there is a parameter for the command.
    if (comStream.eof())
    {
        simFrames = 1;
    }
    else
    {
        //  Read the parameter as a 32-bit integer.
        comStream >> simFrames;

        //  Check if the parameter was correctly read.
        if (comStream.fail())
        {
            simFrames = 0;
        }
    }

    //  Check if there are frames to simulate.
    if (simFrames == 0)
        cout << "Usage: runframe <simulation frames>" << endl;
    else
    {
        cout << "-> Simulating " << simFrames << " frames" << endl;

        //  Reset abort flag
        abortDebug = false;

        bool end = false;
        
        //  Simulate n frames.
        while((simFrames > 0) && (!end) && (!endOfTrace) && (!abortDebug))
        {
            advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);

            //  Check of end of frame.
            if (endOfFrame)
            {
                //  Update the number of simulated frames.
                simFrames--;
                
                //  Reset end of frame flag.
                endOfFrame = false;
            }

            //  End the simulation if a stall was detected.                  
            end = end || gpuStalled || validationError;
        }

        cout << endl;
    }
}


//  Simulator debug runbatch command function.  Simulates n batches
void GPUSimulator::runBatchCommand(stringstream &comStream)
{
    bool endOfBatch = false;
    bool endOfFrame = false;
    bool endOfTrace = false;
    bool gpuStalled = false;
    bool validationError = false;
    u32bit simBatches = 0;

    //  Skip white spaces
    comStream >> ws;

    //  Check if there is a parameter for the command.
    if (comStream.eof())
    {
        simBatches = 1;
    }
    else
    {
        //  Read the parameter as a 32-bit integer.
        comStream >> simBatches;

        //  Check if the parameter was correctly read.
        if (comStream.fail())
        {
            simBatches = 0;
        }
    }

    //  Check if there are frames to simulate.
    if (simBatches == 0)
        cout << "Usage: runbatch <simulation batches>" << endl;
    else
    {
        cout << "-> Simulating " << simBatches << " batches" << endl;

        //  Reset abort flag
        abortDebug = false;

        bool end = false;
        
        //  Simulate n batches.
        while((simBatches > 0) && (!end) && (!endOfTrace) && (!abortDebug))
        {
            advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);
          
            //  Check for end of batch.
            if (endOfBatch)
            {
                //  Reset the end of batch flag.
                endOfBatch = false;
                
                //  Update the number of batches to simulate.
                simBatches--;
            }

            //  End the simulation if a stall was detected.                  
            end = end || gpuStalled || validationError;
        }

        cout << endl;
    }
}

//  Simulator debug runcom command function.  Simulates n AGP commands.
void GPUSimulator::runComCommand(stringstream &comStream)
{
    bool endOfBatch = false;
    bool endOfFrame = false;
    bool endOfTrace = false;
    bool gpuStalled = false;
    bool validationError = false;
    u32bit simCommands = 0;

    //  Skip white spaces
    comStream >> ws;

    //  Check if there is a parameter for the command.
    if (comStream.eof())
    {
        simCommands = 1;
    }
    else
    {
        //  Read the parameter as a 32-bit integer.
        comStream >> simCommands;

        //  Check if the parameter was correctly read.
        if (comStream.fail())
        {
            simCommands = 0;
        }
    }

    //  Check if there are frames to simulate.
    if (simCommands == 0)
        cout << "Usage: runcom <simulation AGP commands>" << endl;
    else
    {
        cout << "-> Simulating " << simCommands << " AGP commands" << endl;

        //  Reset abort flag
        abortDebug = false;
        
        bool end = false;

        //  Simulate n batches.
        while((simCommands > 0) && (!end) && (!endOfTrace) && (!abortDebug))
        {
            advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);

            //  Check if it's the end of a AGP command.
            if (commProc->endOfCommand())
            {
                //  Update the counter of simulated commands.
                simCommands--;
            }
            
            //  End the simulation if a stall was detected.                  
            end = end || gpuStalled || validationError;
        }

        cout << endl;
    }
}


//  Simulator debug skipframe command function.  Skips n frames
void GPUSimulator::skipFrameCommand(stringstream &comStream)
{
    bool endOfBatch = false;
    bool endOfFrame = false;
    bool endOfTrace = false;
    bool gpuStalled = false;
    bool validationError = false;
    u32bit simFrames = 0;

    //  Skip white spaces
    comStream >> ws;

    //  Check if there is a parameter for the command.
    if (comStream.eof())
    {
        simFrames = 1;
    }
    else
    {
        //  Read the parameter as a 32-bit integer.
        comStream >> simFrames;

        //  Check if the parameter was correctly read.
        if (comStream.fail())
        {
            simFrames = 0;
        }
    }

    //  Check if there are frames to simulate.
    if (simFrames == 0)
        cout << "Usage: skipframe <frames to skip>" << endl;
    else
    {
        cout << "-> Skipping " << simFrames << " frames" << endl;

        //  Reset abort flag
        abortDebug = false;

        bool end = false;
        
        //  Set the skip draw command flag in the Command Processor.
        commProc->setSkipFrames(true);

        //  Simulate n frames.
        while((simFrames > 0) && (!end) && (!endOfTrace) && (!abortDebug))
        {
            advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);

            //  Check if the current frame has finished.
            if (endOfFrame)
            {
                //  Reset the end of frame flag.
                endOfFrame = false;
                
                //  Update number of frames to skip.
                simFrames--;
            }

            //  End the simulation if a stall was detected.                  
            end = end || gpuStalled || validationError;
        }

        //  Unset the skip draw command flag in the Command Processor.
        commProc->setSkipFrames(false);

        cout << endl;
    }
}

//  Simulator debug skipbatch command function.  Skips the simulation of n batches
void GPUSimulator::skipBatchCommand(stringstream &comStream)
{
    bool endOfBatch = false;
    bool endOfFrame = false;
    bool endOfTrace = false;
    bool gpuStalled = false;
    bool validationError = false;
    u64bit simCycles = 0;
    u32bit simFrames = 0;
    u32bit simBatches = 0;

    //  Skip white spaces
    comStream >> ws;

    //  Check if there is a parameter for the command.
    if (comStream.eof())
    {
        simBatches = 1;
    }
    else
    {
        //  Read the parameter as a 32-bit integer.
        comStream >> simBatches;

        //  Check if the parameter was correctly read.
        if (comStream.fail())
        {
            simBatches = 0;
        }
    }

    //  Check if there are frames to simulate.
    if (simBatches == 0)
        cout << "Usage: skipbatch <batches to skip>" << endl;
    else
    {
        cout << "-> Skipping " << simBatches << " batches" << endl;

        //  Reset abort flag
        abortDebug = false;
        
        bool end = false;

        //  Set the skip draw command flag in the Command Processor.
        commProc->setSkipDraw(true);

        //  Simulate n batches.
        while((simBatches > 0) && (!end) && (!endOfTrace) && (!abortDebug))
        {
            advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);

            //  Check if the current batch has finished
            if (endOfBatch)
            {
                //  Reset the end of batch flag.
                endOfBatch = false;
                
                //  Update number of batches to skip.
                simBatches--;
            }

            //  End the simulation if a stall was detected.                  
            end = end || gpuStalled || validationError;
        }

        //  Unset the skip draw command flag in the Command Processor.
        commProc->setSkipDraw(false);

        cout << endl;
    }
}

//  Simulator debug state command function.  Returns the state of the simulator and
//  the simulator boxes.
void GPUSimulator::stateCommand(stringstream &comStream)
{
    string boxName;
    Box *stateBox;
    string stateLine;

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (comStream.eof())
    {
        cout << "Usage: state <boxname> | all" << endl;

        if (!multiClock)
            cout << "Simulated cycles = " << cycle;
        else
        {
            cout << "Simulated GPU cycles = " << gpuCycle << " | Simulated Shader cycles = " << shaderCycle;
            cout << " Simulated Memory cycles = " << memoryCycle;
        }
        cout << " | Simulated batches = " << batchCounter;
        cout << " | Simulated frames = " << frameCounter;
        cout << " | Simulated batches in frame = " << frameBatch << endl << endl;
    }
    else
    {
        //  Read the parameter as a string.
        comStream >> boxName;

        if (!multiClock)
            cout << "Simulated cycles = " << cycle;
        else
        {
            cout << "Simulated GPU cycles = " << gpuCycle << " | Simulated Shader cycles = " << shaderCycle;
            cout << " Simulated Memory cycles = " << memoryCycle;
        }
        cout << " | Simulated batches = " << batchCounter;
        cout << " | Simulated frames = " << frameCounter;
        cout << " | Simulated batches in frame = " << frameBatch << endl << endl;       

        if (!boxName.compare("all"))
        {
            cout << "all option not implemented yet" << endl;
        }
        else
        {
            //  Try to get the box using the provided name.
            stateBox = Box::getBox(boxName.c_str());

            //  Check if the box was found.
            if (stateBox != NULL)
            {
                //  Get state line from the box.
                stateBox->getState(stateLine);
                cout << "Box " << boxName << " state -> " << stateLine << endl;
            }
            else
            {
                cout << "Box " << boxName << " not found." << endl;
            }
        }
    }
}

//  Simulator debug mode command function.  Sets the debug mode flag in a box.
void GPUSimulator::debugModeCommand(stringstream &comStream)
{
    string boxName;
    string modeS;
    bool modeB;
    Box *stateBox;
    string stateLine;

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (comStream.eof())
    {
        cout << "Usage: debugmode <boxname | all> <on | off>" << endl;
    }
    else
    {
        //  Read the parameter as a string.
        comStream >> boxName;

        //  Read the debug mode to set.
        comStream >> modeS;

        //  Check if requested to change all boxes.
        if (!boxName.compare("all"))
        {
            bool error = false;
            
            //  Convert the debug mode string
            if (!modeS.compare("on"))
                modeB = true;
            else if (!modeS.compare("off"))
                modeB = false;
            else
            {
                cout << "Usage: debugmode <boxname | all> <on | off>" << endl;
                error = true;
            }
            
            if (!error)
            {
                for(u32bit b = 0; b < boxArray.size(); b++)
                    boxArray[b]->setDebugMode(modeB);
            }
        }
        else
        {                
            //  Try to get the box using the provided name.
            stateBox = Box::getBox(boxName.c_str());

            //  Check if the box was found.
            if (stateBox != NULL)
            {
                bool error = false;
                
                //  Convert the debug mode string
                if (!modeS.compare("on"))
                    modeB = true;
                else if (!modeS.compare("off"))
                    modeB = false;
                else
                {
                    error = true;
                    cout << "Usage: debugmode <boxname | all> <on | off>" << endl;
                }
                
                if (!error)
                {
                    //  Get state line from the box.
                    stateBox->setDebugMode(modeB);
                    cout << "Box " << boxName << " setting debug mode to -> " << modeS << endl;
                }
            }
            else
            {
                cout << "Box " << boxName << " not found." << endl;
            }
        }
    }
}

//  Simultor debug mode execute box command command.  Executes a box command.
void GPUSimulator::execBoxCommCommand(stringstream &comStream)
{
    string boxName;
    Box *box;
    string commandLine;

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (comStream.eof())
    {
        cout << "Usage: execcommand <boxname>" << endl;
    }
    else
    {
        //  Read the parameter as a string.
        comStream >> boxName;

        //  Try to get the box using the provided name.
        box = Box::getBox(boxName.c_str());

        //  Check if the box was found.
        if (box != NULL)
        {
            //  Issue the command to the box
            box->execCommand(comStream);
        }
        else
        {
            cout << "Box " << boxName << " not found." << endl;
        }
    }
}

//  Simulator debug mode list box commands command.  Lists the commands available for a given box.
void GPUSimulator::listBoxCommCommand(stringstream &comStream)
{
    string boxName;
    string commandList;
    Box *box;

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (comStream.eof())
    {
        cout << "Usage: list <boxname>" << endl;
    }
    else
    {
        //  Read the parameter as a string.
        comStream >> boxName;

        //  Try to get the box using the provided name.
        box = Box::getBox(boxName.c_str());

        //  Check if the box was found.
        if (box != NULL)
        {
            //  Get the list of box commands.
            box->getCommandList(commandList);
            cout << "Box " << boxName << " command list : " << endl << commandList.c_str() << endl;
        }
        else
        {
            cout << "Box " << boxName << " not found." << endl;
        }
    }
}

//  Simulator emulator trace command function.  Enable/disable emulator trace logs in validation mode.
void GPUSimulator::emulatorTraceCommand(stringstream &comStream)
{
    string traceMode;
    string traceEnableS;
    bool traceEnable;

    if (!validationMode)
    {
        cout << "Command only available in validation mode" << endl;
    }
    else
    {
        // Skip white spaces.
        comStream >> ws;

        // Check if there is a parameter.
        if (comStream.eof())
        {
            cout << "Usage: emulatortrace <all | batch | vertex | pixel> <on | off> [<index>] [<x>] [<y]" << endl;
        }
        else
        {
            //  Read the parameter as a string.
            comStream >> traceMode;

            // Skip white spaces.
            comStream >> ws;
            
            if (comStream.eof())
            {
                cout << "Usage: emulatortrace <all | batch | vertex | pixel> <on | off> [<index>] [<x>] [<y]" << endl;
            }
            else
            {
                //  Read the enable/disable parameter.
                comStream >> traceEnableS;

                bool error = false;
                
                //  Convert the enable/disable parameter string
                if (!traceEnableS.compare("on"))
                    traceEnable = true;
                else if (!traceEnableS.compare("off"))
                    traceEnable = false;
                else
                {
                    error = true;
                    cout << "Usage: emulatortrace <all | batch | vertex | pixel> <on | off> [<index>] [<x>] [<y]" << endl;
                }
                
                if (!error)
                {
                    //  Check the emulator trace mode to disable/enable.
                    if (!traceMode.compare("all"))
                    {
                        if (traceEnable)
                            cout << "Enabling full emulator trace log." << endl;
                        else
                            cout << "Disabling full emulator trace log." << endl;
                            
                        //  Enable/disable full emulator trace log.
                        gpuEmulator->setFullTraceLog(traceEnable);
                    }
                    else if (!traceMode.compare("batch"))
                    {
                        u32bit batchIndex;
                        
                        comStream >> ws;
                        
                        if (comStream.eof())
                        {
                            cout << "Usage: emulatortrace <all | batch | vertex | pixel> <on | off> [<index>] [<x>] [<y]" << endl;
                        }
                        else
                        {
                            //  Read the batch index as an unsigned integer.
                            comStream >> batchIndex;
                            
                            if (traceEnable)
                            {
                                cout << "Enabling emulator batch trace log for batch " << batchIndex << "." << endl;
                            }
                            else
                            {
                                cout << "Disabling emulator batch trace log." << endl; 
                            }

                            //  Enable/disable emulator trace log for a defined vertex.
                            gpuEmulator->setBatchTraceLog(traceEnable, batchIndex);
                        }
                    }
                    else if (!traceMode.compare("vertex"))
                    {
                        u32bit vIndex;
                        
                        comStream >> ws;
                        
                        if (comStream.eof())
                        {
                            cout << "Usage: emulatortrace <all | batch | vertex | pixel> <on | off> [<index>] [<x>] [<y]" << endl;
                        }
                        else
                        {
                            //  Read the vertex index as an unsigned integer.
                            comStream >> vIndex;
                            
                            if (traceEnable)
                            {
                                cout << "Enabling emulator vertex trace log for index " << vIndex << "." << endl;
                            }
                            else
                            {
                                cout << "Disabling emulator vertex trace log." << endl; 
                            }
                            
                            //  Enable/disable emulator trace log for a defined vertex.
                            gpuEmulator->setVertexTraceLog(traceEnable, vIndex);
                        }
                    }
                    else if (!traceMode.compare("pixel"))
                    {
                        u32bit xPos;
                        u32bit yPos;
                        
                        comStream >> ws;
                        
                        if (comStream.eof())
                        {
                            cout << "Usage: emulatortrace <all | batch | vertex | pixel> <on | off> [<index>] [<x>] [<y]" << endl;
                        }
                        else
                        {
                            //  Read the vertex index as an unsigned integer.
                            comStream >> xPos;
                            
                            comStream >> ws;
                            
                            if (comStream.eof())
                            {
                                cout << "Usage: emulatortrace <all | batch | vertex | pixel> <on | off> [<index>] [<x>] [<y]" << endl;
                            }
                            else
                            {
                                comStream >> yPos;
                                
                                if (traceEnable)
                                {
                                    cout << "Enabling emulator pixel trace log for coordinates (" << xPos << ", " << yPos << ")." << endl;
                                }
                                else
                                {
                                    cout << "Disabling emulator pixel trace log." << endl; 
                                }
                                
                                //  Enable/disable emulator trace log for a defined pixel.
                                gpuEmulator->setPixelTraceLog(traceEnable, xPos, yPos);
                            }
                        }
                    }
                }
            }
        }
    }
}

//  Simulator simulator vertex trace command function.  Enable/disable simulator vertex trace logs in validation mode.
void GPUSimulator::traceVertexCommand(stringstream &comStream)
{
    string enableParam;
    bool traceEnable;

    if (!simP.fsh.useVectorShader)
    {
        cout << "Command only available when vector shader is enabled." << endl;
    }
    else
    {
        // Skip white spaces.
        comStream >> ws;

        // Check if there is a parameter.
        if (comStream.eof())
        {
            cout << "Usage: tracevertex <on | off> <index>" << endl;
        }
        else
        {
            //  Read the parameter as a string.
            comStream >> enableParam;

            // Skip white spaces.
            comStream >> ws;
            
            bool error = false;
            
            //  Convert the enable/disable parameter string
            if (!enableParam.compare("on"))
                traceEnable = true;
            else if (!enableParam.compare("off"))
                traceEnable = false;
            else
                error = true;
            
            if (error || comStream.eof())
            {
                cout << "Usage: tracevertex <on | off> <index>" << endl;
            }
            else
            {
                u32bit vIndex;
                
                comStream >> ws;
                
                if (comStream.eof())
                {
                    cout << "Usage: tracevertex <on | off> <index>" << endl;
                }
                else
                {
                    //  Read the vertex index as an unsigned integer.
                    comStream >> vIndex;
                    
                    if (traceEnable)
                    {
                        cout << "Enabling vertex trace in the shader processors for index " << vIndex << "." << endl;
                    }
                    else
                    {
                        cout << "Disabling vertex trace in the shader processors." << endl; 
                    }
                    
                    for(u32bit u = 0; u < simP.gpu.numFShaders; u++)
                    {
                        stringstream commStr;
                        
                        commStr << "tracevertex " << enableParam << " " << vIndex;
                                                
                        vecShFetch[u]->execCommand(commStr);
                    }
                }
            }
        }
    }
}

//  Simulator simulator fragment trace command function.  Enable/disable simulator fragment trace logs in validation mode.
void GPUSimulator::traceFragmentCommand(stringstream &comStream)
{
    string enableParam;
    bool traceEnable;

    if (!simP.fsh.useVectorShader)
    {
        cout << "Command only available when vector shader is enabled." << endl;
    }
    else
    {
        // Skip white spaces.
        comStream >> ws;

        // Check if there is a parameter.
        if (comStream.eof())
        {
            cout << "Usage: tracefragment <on | off> <x> <y>" << endl;
        }
        else
        {
            //  Read the parameter as a string.
            comStream >> enableParam;

            // Skip white spaces.
            comStream >> ws;
            
            bool error = false;
            
            //  Convert the enable/disable parameter string.
            if (!enableParam.compare("on"))
                traceEnable = true;
            else if (!enableParam.compare("off"))
                traceEnable = false;
            else
                error = true;
            
            if (error || comStream.eof())
            {
                cout << "Usage: tracefragment <on | off> <x> <y>" << endl;
            }
            else
            {
                u32bit fragX;
                u32bit fragY;
                
                comStream >> ws;
                
                if (comStream.eof())
                {
                    cout << "Usage: tracefragment <on | off> <x> <y>" << endl;
                }
                else
                {
                    //  Read the fragment x coordinate index as an unsigned integer.
                    comStream >> fragX;

                    comStream >> ws;
                                        
                    if (comStream.eof())
                    {                  
                        cout << "Usage: tracefragment <on | off> <x> <y>" << endl;
                    }
                    else
                    {
                        comStream >> fragY;
                        
                        if (traceEnable)
                        {
                            cout << "Enabling fragment trace in the shader processors at coordinate (" << fragX << ", " << fragY << ")." << endl;
                        }
                        else
                        {
                            cout << "Disabling fragment trace in the shader processors." << endl; 
                        }
                        
                        for(u32bit u = 0; u < simP.gpu.numFShaders; u++)
                        {
                            stringstream commStr;
                            
                            commStr << "tracefragment " << enableParam << " " << fragX << " " << fragY;
                                                    
                            vecShFetch[u]->execCommand(commStr);
                        }
                        
                        for(u32bit u = 0; u < simP.gpu.numStampUnits; u++)
                        {
                            stringstream commStr;
                            
                            commStr << "tracefragment " << enableParam << " " << fragX << " " << fragY;
                            zStencilV2[u]->execCommand(commStr);
                            
                            commStr.clear();
                            commStr << "tracefragment " << enableParam << " " << fragX << " " << fragY;
                            colorWriteV2[u]->execCommand(commStr);
                        }
                        
                    }
                }
            }
        }
    }
}

void GPUSimulator::driverMemoryUsageCommand(stringstream &comStream)
{
    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (!comStream.eof())
    {
        cout << "Usage: memoryUsage" << endl;
    }
    else
    {
        cout << "GPU Driver => Memory Usage : " << endl;
        GPUDriver::getGPUDriver()->printMemoryUsage();
        cout << endl;
    }
}

void GPUSimulator::driverListMDsCommand(stringstream &comStream)
{
    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (!comStream.eof())
    {
        cout << "Usage: listMDs" << endl;
    }
    else
    {
        cout << "GPU Driver => Memory Descriptors List: " << endl;
        GPUDriver::getGPUDriver()->dumpMDs();
        cout << endl;
    }
}

void GPUSimulator::driverInfoMDCommand(stringstream &comStream)
{
    u32bit md;

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (comStream.eof())
    {
        cout << "Usage: infoMD <md identifier>" << endl;
    }
    else
    {
        comStream >> md;

        comStream >> ws;

        if (!comStream.eof())
        {
            cout << "Usage: infoMD <md identifier>" << endl;
        }
        else
        {
            cout << "GPU Driver => Info Memory Descriptor " << md << endl;
            GPUDriver::getGPUDriver()->dumpMD(md);
            cout << endl;
        }
    }
}

void GPUSimulator::driverDumpAGPBufferCommand(stringstream &comStream)
{
    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (!comStream.eof())
    {
        cout << "Usage: dumpAGPBuffer" << endl;
    }
    else
    {
        cout << "GPU Driver => AGP Buffer Content: " << endl;
        GPUDriver::getGPUDriver()->dumpAGPBuffer();
        cout << endl;
    }
}

void GPUSimulator::driverMemAllocCommand(stringstream &comStream)
{
    string boolParam;
    bool dumpContent;

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (comStream.eof())
    {
        cout << "GPU Driver => Memory Allocation (simple) " << endl;
        GPUDriver::getGPUDriver()->dumpMemoryAllocation(false);
        cout << endl;
    }
    else
    {
        comStream >> boolParam;

        if (!boolParam.compare("true"))
            dumpContent = true;
        else if (!boolParam.compare("false"))
            dumpContent = false;
        else
        {
            cout << "Usage: memoryAlloc [true|false]" << endl;
            return;
        }

        comStream >> ws;

        if (!comStream.eof())
        {
            cout << "Usage: memoryAlloc [true|false]" << endl;
        }
        else
        {
            cout << "GPU Driver => Memory Allocation " << (dumpContent?"(full)":"(simple)") << endl;
            GPUDriver::getGPUDriver()->dumpMemoryAllocation(dumpContent);
            cout << endl;
        }
    }
}

/*
  DEPRECATED
  
void GPUSimulator::libraryGLContextCommand(stringstream &comStream)
{
    libgl::GLContext *ctx;

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (!comStream.eof())
    {
        cout << "Usage: glcontext" << endl;
    }
    else
    {
        cout << "GPU Driver => AGP Buffer Content: " << endl;
        ctx = (libgl::GLContext *) GPUDriver::getGPUDriver()->getContext();
        ctx -> dump();
        cout << endl;
    }
}

void GPUSimulator::libraryDumpStencilCommand(stringstream &comStream)
{
    libgl::GLContext *ctx;

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (!comStream.eof())
    {
        cout << "Usage: dumpStencil" << endl;
    }
    else
    {
        cout << "GPU Driver => Dumping stencil buffer " << endl;
        ctx = (libgl::GLContext *) GPUDriver::getGPUDriver()->getContext();
        libgl::afl::dumpStencilBuffer(ctx);
    }
}*/

//  Simulate until a force command finishes.
bool GPUSimulator::simForcedCommand()
{
    bool endOfBatch = false;
    bool endOfFrame = false;    
    bool endOfTrace = false;
    bool gpuStalled = false;
    bool validationError = false;
    bool endOfCommand = false;

    //  Simulate until the end of the initialization phase.
    while(!endOfCommand && !endOfTrace && !gpuStalled && !validationError)
    {
        advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);

        //  Check if the forced command has finished.
        endOfCommand = commProc->endOfForcedCommand();
    }

    return !endOfTrace;
}

void GPUSimulator::saveSnapshotCommand()
{
    pendingSaveSnapshot = true;
    
    //  Create a directory for the snapshot files.
    u32bit snapshotID = 0;
    bool directoryCreated = false;
    bool tooManySnapshots = false;
    bool error = false;
    char directoryName[100];
    while (!directoryCreated && !error && !tooManySnapshots)
    {
        sprintf(directoryName, "ATTILAsnapshot%02d", snapshotID);
        
        s32bit res = createDirectory(directoryName);                
        
        if (res == 0)
            directoryCreated = true;
        else if (res == DIRECTORY_ALREADY_EXISTS)
        {
            if (snapshotID < 99)
                snapshotID++;
            else
                tooManySnapshots = true;
        }
        else
            error = true;         
    }
    
    //  Check errors creating the snapshot directory.
    if (error)
    {
        printf(" ERROR: Coul not create snapshot directory: %s\n", directoryName);
        return;
    }
    
    if (tooManySnapshots)
    {
        printf(" ERROR: Too many snapshots already present in the current working directory\n");
        return;
    }
    
    //  Disable create snapshot on call back.
    panicCallback = NULL;
    
    if (directoryCreated)
    {
        char workingDirectory[1024];

        if (getCurrentDirectory(workingDirectory, 1024) != 0)
            panic("GPUSimulator", "saveSnapshotCommand", "Error obtaining current working directory.");

        //  Change working directory to the snapshot directory.            
        if (changeDirectory(directoryName) == 0)
        {

            stringstream commandStream;
            
            printf(" Saving a simulator snapshot in %s directory\n", directoryName);
             
            //  Flush color caches.
            commandStream.clear();
            commandStream.str("flushcolor");
            commProc->execCommand(commandStream);

            if (!simForcedCommand())
                return;

            //  Flush z and stencil caches.
            commandStream.clear();
            commandStream.str("flushzst");
            commProc->execCommand(commandStream);

            //  Check for end of trace.
            if (!simForcedCommand())
                return;
                
            saveSimState();
            saveSimConfig();
            
            commandStream.clear();
            commandStream.str("_saveregisters");                    
            commProc->execCommand(commandStream);
            
            AGPTraceDriver *agpTraceDriver = dynamic_cast<AGPTraceDriver*>(trDriver);
            
            //  Dump AGP Trace Driver state.
            if (agpTrace)
            {
                fstream out;

                out.open("agptracedriver.snapshot", ios::binary | ios::out);

                if (!out.is_open())
                    panic("GPUSimulator", "saveSnapshotCommand", "Error creating AGP Trace Driver snapshot file.");

                agpTraceDriver->saveTracePosition(&out);

                out.close();
            }

            rast->saveHZBuffer();

            for(u32bit i = 0; i < simP.gpu.numStampUnits; i++)
            {
                zStencilV2[i]->saveBlockStateMemory();
                colorWriteV2[i]->saveBlockStateMemory();
            }

            commandStream.clear();
            commandStream.str("_savememory");
            memController->execCommand(commandStream);
        
            if (validationMode)
                gpuEmulator->saveSnapshot();
            
            if (changeDirectory(workingDirectory) != 0)
                panic("GPUSimulator", "saveSnapshotCommand", "Error changing back to working directory.");
        }
        else
        {
            printf(" ERROR: Could not access working directory to snapshot directory.\n");
        }
    }

    //  Renable create snapshot on panic.
    panicCallback = &createSnapshot;

    pendingSaveSnapshot = false;    
}

void GPUSimulator::loadSnapshotCommand(stringstream &comStream)
{
    u32bit snapshotID;

    //  Check if a snapshot can be loaded.
    if ((!multiClock && (cycle != 0)) || (multiClock && ((gpuCycle != 0) || (shaderCycle != 0) || (memoryCycle != 0))))
    {
        cout << "ERROR : loadsnapshot can only be used at cycle 0." << endl;
        return;
    }

    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    if (comStream.eof())
    {
        cout << "Usage: loadsnapshot <snapshot identifier>" << endl;
    }
    else
    {
        comStream >> snapshotID;
        comStream >> ws;

        if (!comStream.eof())
        {
            cout << "Usage: loadsnapshot <snapshot identifier>" << endl;
        }
        else
        {
            //  Disable save snapshot on panic.
            panicCallback = NULL;
            
            //  Change to the snapshot directory.
            char directoryName[100];
            sprintf(directoryName, "ATTILAsnapshot%02d", snapshotID);
            
            char workingDirectory[1024];
            char snapshotDirectory[1024];

            if (getCurrentDirectory(workingDirectory, 1024) != 0)
                panic("GPUSimulator", "loadSnapshotCommand", "Error obtaining current working directory.");
            
            printf(" Current working directory path: %s\n", workingDirectory);
            printf(" Loading a simulator snapshot from %s directory\n", directoryName);

            //  Change working directory to the snapshot directory.            
            if (changeDirectory(directoryName) == 0)
            {

                if (getCurrentDirectory(snapshotDirectory, 1024) != 0)
                    panic("GPUSimulator", "loadSnapshotCommand", "Error obtaining snapshot directory.");
                    
                printf(" Snapshot directory path: %s\n", snapshotDirectory);

                //  Check the type of tracefile being used.
                if (agpTrace)
                {
                    //  Load AGP trace snapshot file
                    fstream in;

                    in.open("agptracedriver.snapshot", ios::binary | ios::in);

                    if (!in.is_open())
                        panic("GPUSimulator", "loadSnapshotCommand", "Error opening AGP Trace Driver snapshot file.");

                    AGPTraceDriver *agpTraceDriver = dynamic_cast<AGPTraceDriver*>(trDriver);

                    //  Load the AGP trace start position information from the snapshot file.
                    agpTraceDriver->loadStartTracePosition(&in);

                    in.close();

                    bool endOfBatch = false;
                    bool endOfFrame = false;
                    bool endOfTrace = false;
                    bool gpuStalled = false;
                    bool validationError = false;

                    if (changeDirectory(workingDirectory) != 0)
                        panic("bGPU-Unified", "loadSnapshotCommand", "Error changing back to working directory.");
                    
                    //  Simulate until the end of the initialization phase.
                    while(!commProc->endOfInitialization() && !endOfTrace)
                    {
                        advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);
                    }

                    if (changeDirectory(directoryName) != 0)
                        panic("GPUSimulator", "loadSnapshotCommand", "Error changing to snapshot directory.");

                }
                
                if (oglTrace || d3d9Trace)
                {
                    //  Read the frame and batches to skip from the state snapshot file.
                    ifstream in;
                    
                    in.open("state.snapshot");
                    
                    u32bit savedFrame;
                    u32bit savedBatch;
                    u32bit savedFrameBatch;
                    
                    in.ignore(8);       //  Skip "Frame = ".
                    in >> savedFrame;   //  Read the saved frame.
                    in.ignore();        //  Skip newline.
                    in.ignore(8);       //  Skip "Batch = ".
                    in >> savedBatch;   //  Read the saved batch.
                    in.ignore();        //  Skip newline.
                    in.ignore(14);      //  Skip "Frame Batch = "
                    in >> savedFrameBatch;  //  Read the saved frame batch.
                    in.close(); 
                                                            
                    bool endOfBatch = false;
                    bool endOfFrame = false;
                    bool endOfTrace = false;
                    bool gpuStalled = false;
                    bool validationError = false;

                    cout << " Skipping up to frame " << savedFrame << " batch " << savedBatch << " frame batch " << savedFrameBatch << endl;
                    
                    //  Skip rendering until reaching the saved frame and batch.
                    commProc->setSkipFrames(true);
                    
                    if (validationMode)
                        gpuEmulator->setSkipBatch(true);
                    
                    if (changeDirectory(workingDirectory) != 0)
                        panic("GPUSimulator", "loadSnapshotCommand", "Error changing back to working directory.");

                    //  Skip validation until reaching the point where the snapshot was captured.
                    skipValidation = true;

                    //  Simulate until the end of the initialization phase.
                    while(((frameCounter != savedFrame) || ((batchCounter != savedBatch)) && (frameBatch != savedFrameBatch)) && !endOfTrace)
                    {
                        advanceTime(endOfBatch, endOfFrame, endOfTrace, gpuStalled, validationError);
                    }
                    
                    //  Reenable validation.
                    skipValidation = false;
                    
                    //  Restart rendering.
                    commProc->setSkipFrames(false);

                    if (validationMode)
                        gpuEmulator->setSkipBatch(false);

                    if (changeDirectory(directoryName) != 0)
                        panic("GPUSimulator", "loadSnapshotCommand", "Error changing to snapshot directory.");
                }            

                cout << " Loading state from snapshot" << endl;
                
                //  Load the Hierarchical Z Buffer content from the snapshot file.
                rast->loadHZBuffer();

                //  Load block state memory from the snapshot files.
                for(u32bit i = 0; i < simP.gpu.numStampUnits; i++)
                {
                    zStencilV2[i]->loadBlockStateMemory();
                    colorWriteV2[i]->loadBlockStateMemory();
                }

                stringstream commandStream;
                
                //  Load memory from the snapshot file.
                commandStream.clear();
                commandStream.str("_loadmemory");
                memController->execCommand(commandStream);

                if (validationMode)
                {
                    //  Load emulator snapshot.
                    gpuEmulator->loadSnapshot();
                }
                                
                if (changeDirectory(workingDirectory) != 0)
                    panic("GPUSimulator", "loadSnapshotCommand", "Error changing back to working directory.");
            }            
            else
            {
                printf(" ERROR: Could not access working directory to snapshot directory.\n");
            }

            //  Re-enable save snapshot on panic.
            panicCallback = &createSnapshot;
        }
    }
}

void GPUSimulator::autoSnapshotCommand(stringstream &comStream)
{
    bool errorInParsing = false;
    
    bool enableParam;
    u32bit freqParam;
    
    // Skip white spaces.
    comStream >> ws;

    // Check if there is a parameter.
    errorInParsing = comStream.eof();

    if (!errorInParsing)
    {
        string paramStr;
        
        comStream >> paramStr;

        if (!paramStr.compare("on"))
            enableParam = true;
        else if (!paramStr.compare("off"))
            enableParam = false;
        else
            errorInParsing = true;

        if (!errorInParsing)
        {
            comStream >> ws;
        
            errorInParsing = comStream.eof();
            
            if (!errorInParsing)
            {
                comStream >> freqParam;
                
                comStream >> ws;
                
                errorInParsing = !comStream.eof();
                
                if (!errorInParsing)
                {
                    if (freqParam == 0)
                    {
                        cout << "Auto snapshot frequency must be at leat 1 minute" << endl;
                        errorInParsing = true;
                    }
                    
                    if (!errorInParsing)
                    {
                        if (enableParam)
                            cout << "Setting auto snapshot with a frequency of " << freqParam << " minutes" << endl;
                        else
                            cout << "Disabling auto snapshot." << endl;
                            
                        autoSnapshotEnable = enableParam;
                        snapshotFrequency = freqParam;
                        
                        startTime = time(NULL);
                    }
                }
            }
        }
    }
    
    if (errorInParsing)
    {
        cout << "Usage: autosnapshot <on|off> <frequency in minutes>" << endl;
    }
}

void GPUSimulator::advanceTime(bool &endOfBatch, bool &endOfFrame, bool &endOfTrace, bool &gpuStalled, bool &validationError)
{
    current = this;

    bool checkStalls;
    
    if (!multiClock)
    {
        //  Increment the statistics cycle counter.
        cyclesCounter->inc();
        
        //  Issue a clock for all the simulation boxes.
        for(u32bit box = 0; box < boxArray.size(); box++)
            boxArray[box]->clock(cycle);
        
        //  Update the simulator cycle counter.
        cycle++;
        
        //  Evaluate if stall detection must be performed.
        checkStalls = simP.detectStalls && (GPU_MOD(cycle, 100000) == 99999);
    }
    else
    {
        bool end = false;
        
        //  Run at least one cycle of the main (GPU) clock.
        while(!end)
        {
            //  Determine the picoseconds to the next clock.
            u32bit nextStep = GPU_MIN(GPU_MIN(nextGPUClock, nextShaderClock), nextMemoryClock);
            
            //printf("Clock -> Next GPU Clock = %d | Next Shader Clock = %d | Next Memory Clock = %d| Next Step = %d | GPU Cycle = %lld | Shader Cycle = %lld | Memory Cycle = %lld\n",
            //    nextGPUClock, nextShaderClock, nextStep, gpuCycle, shaderCycle, memoryCycle);
                            
            //  Update step counters for both clock domains.
            nextGPUClock -= nextStep;
            nextShaderClock -= nextStep;
            nextMemoryClock -= nextStep;
            
            //  Check if a gpu domain tick has to be issued.
            if (nextGPUClock == 0)
            {
                GPU_DEBUG(
                    printf("GPU Domain. Cycle %lld ----------------------------\n", gpuCycle);
                )
                
                // Clock all the boxes in the GPU Domain.
                for(u32bit box = 0; box < gpuDomainBoxes.size(); box++)
                    gpuDomainBoxes[box]->clock(gpuCycle);
                    
                //  Clock boxes with multiple domains.
                for(u32bit box = 0; box < shaderDomainBoxes.size(); box++)
                    shaderDomainBoxes[box]->clock(GPU_CLOCK_DOMAIN, gpuCycle);
                    
                for(u32bit box = 0; box < memoryDomainBoxes.size(); box++)
                    memoryDomainBoxes[box]->clock(GPU_CLOCK_DOMAIN, gpuCycle);

                //  Update GPU domain clock state.
                gpuCycle++;
                nextGPUClock = gpuClockPeriod;
                
                //  One cycle of the GPU clock was simulated.
                end = true;
            }
            
            //  Check if a shader domain tick must be issued.
            if (nextShaderClock == 0)
            {
                if (shaderClockDomain)
                {
                    GPU_DEBUG(
                        printf("Shader Domain. Cycle %lld ----------------------------\n", shaderCycle);
                    )

                    //  Clock boxes with multiple domains.
                    for(u32bit box = 0; box < shaderDomainBoxes.size(); box++)
                        shaderDomainBoxes[box]->clock(SHADER_CLOCK_DOMAIN, shaderCycle);

                    //  Update shader domain clock and step counter.
                    shaderCycle++;
                }
                
                nextShaderClock = shaderClockPeriod;
            }

            //  Check if a memory domain tick must be issued.
            if (nextMemoryClock == 0)
            {
                if (memoryClockDomain)
                {
                    GPU_DEBUG(
                        printf("Memory Domain. Cycle %lld ----------------------------\n", memoryCycle);
                    )

                    //  Clock boxes with multiple domains.
                    for(u32bit box = 0; box < memoryDomainBoxes.size(); box++)
                        memoryDomainBoxes[box]->clock(MEMORY_CLOCK_DOMAIN, memoryCycle);

                    //  Update memory domain clock and step counter.
                    memoryCycle++;
                }
                
                nextMemoryClock = memoryClockPeriod;
            }
        }

        //  Evaluate if stall detection must be performed.
        checkStalls = simP.detectStalls && (GPU_MOD(gpuCycle, 100000) == 99999);
    }

    //  Check if the current batch has finished
    if (commProc->endOfBatch())
    {
        //  Update rendered batches counter.
        batchCounter++;
        
        //  Update rendered batches in the current frame.
        frameBatch++;
        
        //  Set the end batch flag.
        endOfBatch = true;
        
        //  Check if validation mode is enabled.
        if (validationMode)
        {
            //  Get the log of AGP Transactions processed by the Command Processor.
            vector<AGPTransaction *> &agpTransLog = commProc->getAGPTransactionLog();
            
            //  Issue the AGP transactions to the emulator.
            for(u32bit trans = 0; trans < agpTransLog.size(); trans++)
            {
                gpuEmulator->emulateCommandProcessor(agpTransLog[trans]);
            }
            
            //  Clear the AGP Transaction log.
            agpTransLog.clear();
                       
            //  Check if validation must be performed.
            if (!skipValidation)
            {
                VertexInputMap simVertexInputLog;
                
                //  Get the read vertex log from the simulator.            
                for(u32bit u = 0; u < simP.str.streamerLoaderUnits; u++)
                {
                    //  Get the read vertex log from one of the Streamer Loader units.
                    VertexInputMap &simVertexInputStLLog = streamer->getVertexInputInfo(u);
                    
                    //  Rebuild as a single map.
                    VertexInputMap::iterator itStL;
                    for(itStL = simVertexInputStLLog.begin(); itStL != simVertexInputStLLog.end(); itStL++)
                    {
                        //  Search in the rebuild map.
                        VertexInputMap::iterator itGlobal;
                        itGlobal = simVertexInputLog.find(itStL->second.vertexID);
                        
                        if (itGlobal != simVertexInputLog.end())
                        {
                            //  Check and update.
                            itGlobal->second.timesRead += itStL->second.timesRead;
                            
                            //  Compare the two versions.
                            bool attributesAreDifferent = false;
                            
                            for(u32bit a = 0; (a < MAX_VERTEX_ATTRIBUTES) && !attributesAreDifferent; a++)
                            {
                                //  Evaluate if the attribute values are different.
                                attributesAreDifferent = (itGlobal->second.attributes[a][0] != itGlobal->second.attributes[a][0]) ||
                                                         (itGlobal->second.attributes[a][1] != itGlobal->second.attributes[a][1]) ||
                                                         (itGlobal->second.attributes[a][2] != itGlobal->second.attributes[a][2]) ||
                                                         (itGlobal->second.attributes[a][3] != itGlobal->second.attributes[a][3]);
                            }
                            
                            itGlobal->second.differencesBetweenReads = itGlobal->second.differencesBetweenReads || 
                                                                       itStL->second.differencesBetweenReads ||
                                                                       attributesAreDifferent;
                        }
                        else
                        {
                            //  Add a new read vertex to the rebuild map.
                            simVertexInputLog.insert(make_pair(itStL->second.vertexID, itStL->second));                        
                        }
                    }
                    
                    //  Clear the map from the Streamer Loader unit.
                    simVertexInputStLLog.clear();
                }
                
                //  Get the read vertex log from the emulator.
                VertexInputMap &emuVertexInputLog = gpuEmulator->getVertexInputLog();

//printf("Validation >> Simulator Read Vertices = %d | Emulator Read Vertices = %d\n", simVertexInputLog.size(),
//emuVertexInputLog.size());

                //  Compare the two lists.
                if (simVertexInputLog.size() != emuVertexInputLog.size())
                {
                    printf("Validation => Number of read vertices is different: emulator = %d simulator = %d\n",
                        emuVertexInputLog.size(), simVertexInputLog.size());
                        
                    validationError = true;               
                    
/*VertexInputMap::iterator itVertexInput;
printf("Validation => Emulator read vertex list\n");
for(itVertexInput = emuVertexInputLog.begin(); itVertexInput != emuVertexInputLog.end(); itVertexInput++)
printf("Validation => Index %d Instance %d\n", itVertexInput->second.vertexID.index, itVertexInput->second.vertexID.instance);
printf("Validation => Simulator read vertex list\n");
for(itVertexInput = simVertexInputLog.begin(); itVertexInput != simVertexInputLog.end(); itVertexInput++)
printf("Validation => Index %d Instance %d TimesRead %d Diffs Between Reads %s\n",
itVertexInput->second.vertexID.index, itVertexInput->second.vertexID.instance,
itVertexInput->second.timesRead, itVertexInput->second.differencesBetweenReads ? "T" : "F");*/
                }
                
                VertexInputMap::iterator itVertexInputEmu;
                
                for(itVertexInputEmu = emuVertexInputLog.begin(); itVertexInputEmu != emuVertexInputLog.end(); itVertexInputEmu++)
                {
                    //  Find index in the simulator log.
                    VertexInputMap::iterator itVertexInputSim;
                    
                    itVertexInputSim = simVertexInputLog.find(itVertexInputEmu->second.vertexID);
                    
                    //  Check if the index was found in the simulator log.
                    if (itVertexInputSim != simVertexInputLog.end())
                    {
                        //  Compare all the attributes.
                        for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                        {
                            bool attributeIsEqual =                             
                                ((itVertexInputEmu->second.attributes[a][0] == itVertexInputSim->second.attributes[a][0]) ||
                                 (isNaN(itVertexInputEmu->second.attributes[a][0]) && isNaN(itVertexInputSim->second.attributes[a][0]))) &&
                                ((itVertexInputEmu->second.attributes[a][1] == itVertexInputSim->second.attributes[a][1]) ||
                                 (isNaN(itVertexInputEmu->second.attributes[a][1]) && isNaN(itVertexInputSim->second.attributes[a][1]))) &&
                                ((itVertexInputEmu->second.attributes[a][2] == itVertexInputSim->second.attributes[a][2]) ||
                                 (isNaN(itVertexInputEmu->second.attributes[a][2]) && isNaN(itVertexInputSim->second.attributes[a][2]))) &&
                                ((itVertexInputEmu->second.attributes[a][3] == itVertexInputSim->second.attributes[a][3]) ||
                                 (isNaN(itVertexInputEmu->second.attributes[a][3]) && isNaN(itVertexInputSim->second.attributes[a][3])));

                            if (!attributeIsEqual)
                            {
                                f32bit attrib[4];

                                attrib[0] = itVertexInputEmu->second.attributes[a][0];
                                attrib[1] = itVertexInputEmu->second.attributes[a][1];
                                attrib[2] = itVertexInputEmu->second.attributes[a][2];
                                attrib[3] = itVertexInputEmu->second.attributes[a][3];
                                
                                printf("Validation => For read vertex at instance %d with index %d attribute %d is different:\n",
                                    itVertexInputEmu->second.vertexID.instance, itVertexInputEmu->second.vertexID.index, a);
                                printf("   emuAttrib[%d] = {%f, %f, %f, %f} | (%08x, %08x, %08x, %08x)\n", a,
                                    itVertexInputEmu->second.attributes[a][0], itVertexInputEmu->second.attributes[a][1],
                                    itVertexInputEmu->second.attributes[a][2], itVertexInputEmu->second.attributes[a][3],
                                    ((u32bit *)attrib)[0], ((u32bit *)attrib)[1], ((u32bit *)attrib)[2], ((u32bit *)attrib)[3]);

                                attrib[0] = itVertexInputSim->second.attributes[a][0];
                                attrib[1] = itVertexInputSim->second.attributes[a][1];
                                attrib[2] = itVertexInputSim->second.attributes[a][2];
                                attrib[3] = itVertexInputSim->second.attributes[a][3];

                                printf("   simAttrib[%d] = {%f, %f, %f, %f} | (%08x, %08x, %08x, %08x)\n", a,
                                    itVertexInputSim->second.attributes[a][0], itVertexInputSim->second.attributes[a][1],
                                    itVertexInputSim->second.attributes[a][2], itVertexInputSim->second.attributes[a][3],
                                    ((u32bit *)attrib)[0], ((u32bit *)attrib)[1], ((u32bit *)attrib)[2], ((u32bit *)attrib)[3]);
                                
                                validationError = true;
                            }
                        }
                    }
                    else
                    {
                        printf("Validation => Instance %d index %d was not found in the simulator read vertex log.\n",
                            itVertexInputEmu->second.vertexID.instance, itVertexInputEmu->second.vertexID.index);

                        validationError = true;
                    }
                }
                
                if (simVertexInputLog.size() > emuVertexInputLog.size())
                {
                    VertexInputMap::iterator itVertexInputSim;
                    
                    for(itVertexInputSim = simVertexInputLog.begin(); itVertexInputSim != simVertexInputLog.end(); itVertexInputSim++)
                    {
                        //  Find index in the emulator log.
                        itVertexInputEmu = emuVertexInputLog.find(itVertexInputSim->second.vertexID);
                        
                        //  Check if the index was found in the emulator log.
                        if (itVertexInputEmu == emuVertexInputLog.end())
                        {
                            printf("Validation => Instance %d index %d was not found in the emulator read vertex log.\n",
                                itVertexInputSim->second.vertexID.instance, itVertexInputSim->second.vertexID.index);

                            validationError = true;
                        }
                    }
                }
                
                //  Clear the log maps.
                emuVertexInputLog.clear();
                simVertexInputLog.clear();
            }
            else
            {
                //  Get the read vertex log from the emulator.
                VertexInputMap &emuVertexInputLog = gpuEmulator->getVertexInputLog();
                
                //  Clear the log maps.
                emuVertexInputLog.clear();
            }
            

            //  Check if validation must be performed.
            if (!skipValidation)
            {
                //  Get the shaded vertex log from the simulator.
                ShadedVertexMap &simShadedVertexLog = streamer->getShadedVertexInfo();
                
                //  Get the shaded vertex log from the emulator.
                ShadedVertexMap &emuShadedVertexLog = gpuEmulator->getShadedVertexLog();
                
                
//printf("Validation >> Simulator Shaded Vertices = %d | Emulator Shaded Vertices = %d\n", simShadedVertexLog.size(),
//emuShadedVertexLog.size());

                //  Compare the two lists.
                if (simShadedVertexLog.size() != emuShadedVertexLog.size())
                {
                    printf("Validation => Number of shaded vertices is different: emulator = %d simulator = %d\n",
                        emuShadedVertexLog.size(), simShadedVertexLog.size());
                    validationError = true;
                }

                ShadedVertexMap::iterator itEmu;
                
                for(itEmu = emuShadedVertexLog.begin(); itEmu != emuShadedVertexLog.end(); itEmu++)
                {
                    //  Find index in the simulator log.
                    ShadedVertexMap::iterator itSim;
                    
                    itSim = simShadedVertexLog.find(itEmu->second.vertexID);
                    
                    //  Check if the index was found in the simulator log.
                    if (itSim != simShadedVertexLog.end())
                    {
                        //  Compare all the attributes.
                        for(u32bit a = 0; a < MAX_VERTEX_ATTRIBUTES; a++)
                        {
                            bool attributeIsEqual =
                                ((itEmu->second.attributes[a][0] == itSim->second.attributes[a][0]) ||
                                 (isNaN(itEmu->second.attributes[a][0]) && isNaN(itSim->second.attributes[a][0]))) &&
                                ((itEmu->second.attributes[a][1] == itSim->second.attributes[a][1]) ||
                                 (isNaN(itEmu->second.attributes[a][1]) && isNaN(itSim->second.attributes[a][1]))) &&
                                ((itEmu->second.attributes[a][2] == itSim->second.attributes[a][2]) ||
                                 (isNaN(itEmu->second.attributes[a][2]) && isNaN(itSim->second.attributes[a][2]))) &&
                                ((itEmu->second.attributes[a][3] == itSim->second.attributes[a][3]) ||
                                 (isNaN(itEmu->second.attributes[a][3]) && isNaN(itSim->second.attributes[a][3])));

                            if (!attributeIsEqual)
                            {
                                f32bit attrib[4];

                                attrib[0] = itEmu->second.attributes[a][0];
                                attrib[1] = itEmu->second.attributes[a][1];
                                attrib[2] = itEmu->second.attributes[a][2];
                                attrib[3] = itEmu->second.attributes[a][3];
                                
                                printf("Validation => For shaded vertex at instance %d index %d attribute %d is different:\n",
                                    itEmu->second.vertexID.instance, itEmu->second.vertexID.index, a);
                                printf("   emuAttrib[%d] = {%f, %f, %f, %f} | (%08x, %08x, %08x, %08x)\n", a,
                                    itEmu->second.attributes[a][0], itEmu->second.attributes[a][1],
                                    itEmu->second.attributes[a][2], itEmu->second.attributes[a][3],
                                    ((u32bit *)attrib)[0], ((u32bit *)attrib)[1], ((u32bit *)attrib)[2], ((u32bit *)attrib)[3]);
                                printf("   simAttrib[%d] = {%f, %f, %f, %f} | (%08x, %08x, %08x, %08x)\n", a,
                                    itSim->second.attributes[a][0], itSim->second.attributes[a][1],
                                    itSim->second.attributes[a][2], itSim->second.attributes[a][3],
                                    ((u32bit *)attrib)[0], ((u32bit *)attrib)[1], ((u32bit *)attrib)[2], ((u32bit *)attrib)[3]);

                                validationError = true;
                            }
                        }
                    }
                    else
                    {
                        printf("Validation => Instance %d Index %d was not found in the simulator shaded vertex log.\n",
                            itEmu->second.vertexID.instance, itEmu->second.vertexID.index);

                        validationError = true;
                    }
                }
                
                if (simShadedVertexLog.size() > emuShadedVertexLog.size())
                {
                    ShadedVertexMap::iterator itSim;
                    
                    for(itSim = simShadedVertexLog.begin(); itSim != simShadedVertexLog.end(); itSim++)
                    {
                        //  Find index in the emulator log.
                        itEmu = emuShadedVertexLog.find(itSim->second.vertexID);
                        
                        //  Check if the index was found in the emulator log.
                        if (itEmu == emuShadedVertexLog.end())
                        {
                            printf("Validation => Instance %d Index %d was not found in the emulator shaded vertex log.\n",
                                itSim->second.vertexID.instance, itSim->second.vertexID.index);

                            validationError = true;
                        }
                    }
                }
                
                
                //  Clear the log maps.
                emuShadedVertexLog.clear();
                simShadedVertexLog.clear();

                //  Get the z stencil memory update log from the emulator.
                FragmentQuadMemoryUpdateMap &emuZStencilUpdateLog = gpuEmulator->getZStencilUpdateMap();
                               
                FragmentQuadMemoryUpdateMap simZStencilUpdateLog;
                
                //  Get the z stencil memory update logs from the simulator Z Stencil Test units.
                for(u32bit rop = 0; rop < simP.gpu.numStampUnits; rop++)
                {
                    //  Get the z stencil memory update log from the z stencil test unit.
                    FragmentQuadMemoryUpdateMap &simZStencilUpdateROPLog = zStencilV2[rop]->getZStencilUpdateMap();
                    
                    FragmentQuadMemoryUpdateMap::iterator it;

                    //  Retrieve the updates from the z stencil unit.
                    for(it = simZStencilUpdateROPLog.begin(); it != simZStencilUpdateROPLog.end(); it++)
                        simZStencilUpdateLog.insert(make_pair(it->first, it->second));
                    
                    //  Clear the z stencil update log.
                    simZStencilUpdateROPLog.clear();
                }
                
//printf("Validation >> Simulator ZST Updates = %d | Emulator ZST Updates = %d\n", simZStencilUpdateLog.size(),
//emuZStencilUpdateLog.size());

                //  Check the number of updates.
                if (simZStencilUpdateLog.size() != emuZStencilUpdateLog.size())
                {
                    printf("Validation => Number of updates to Z Stencil Buffer differs: emulator %d simulator %d\n",
                        emuZStencilUpdateLog.size(), simZStencilUpdateLog.size());
                
                    validationError = true;
                }
                
                
                FragmentQuadMemoryUpdateMap::iterator itZStencilUpdateEmu;
                
                u32bit maxDifferences = 30;
                
                u32bit checkedQuads = 0;
                
                //  Traverse the whole update log and search differences.
                for(itZStencilUpdateEmu = emuZStencilUpdateLog.begin();
                    itZStencilUpdateEmu != emuZStencilUpdateLog.end() && (maxDifferences > 0); itZStencilUpdateEmu++)
                {
                    FragmentQuadMemoryUpdateMap::iterator itZStencilUpdateSim;
                    
                    //  Search the update in the simulator log.
                    itZStencilUpdateSim = simZStencilUpdateLog.find(itZStencilUpdateEmu->first);
                    
                    if (itZStencilUpdateSim == simZStencilUpdateLog.end())
                    {
                        printf("Validation => Z Stencil memory update on quad at (%d, %d) triangle %d not found in the simulator log.\n",   
                            itZStencilUpdateEmu->first.x, itZStencilUpdateEmu->first.y, itZStencilUpdateEmu->first.triangleID);
                        
                        maxDifferences--;
                    }
                    else
                    {
                        //  Compare the two updates.
                        
                        bool diffInReadData = false;                        
                        u32bit bytesPixel = 4;
                        
                        //  Compare read data.
                        for(u32bit qw = 0; (qw < ((bytesPixel * STAMP_FRAGMENTS)) >> 3) && !diffInReadData; qw++)
                            diffInReadData = (((u64bit *) itZStencilUpdateEmu->second.readData)[qw] != ((u64bit *) itZStencilUpdateSim->second.readData)[qw]);
                        
                        bool diffInWriteData = false;
                        
                        //  Compare write data.
                        for(u32bit qw = 0; (qw < ((bytesPixel * STAMP_FRAGMENTS)) >> 3) && !diffInWriteData; qw++)
                            diffInWriteData = (((u64bit *) itZStencilUpdateEmu->second.writeData)[qw] != ((u64bit *) itZStencilUpdateSim->second.writeData)[qw]);
                    
                        if (diffInReadData)
                            printf("Validation => Difference found in z stencil read data for the update on quad at (%d, %d) triangle %d\n",
                                itZStencilUpdateEmu->first.x, itZStencilUpdateEmu->first.y, itZStencilUpdateEmu->first.triangleID);

                        if (diffInWriteData)
                            printf("Validation => Difference found in z stencil write data for the update on quad at (%d, %d) triangle %d\n",
                                itZStencilUpdateEmu->first.x, itZStencilUpdateEmu->first.y, itZStencilUpdateEmu->first.triangleID);
                                
                        if (diffInReadData || diffInWriteData)
                        {
                            printf("  Emulator Update : \n");
                            printf("    Read Data : ");
                            for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                            {
                                printf("  %08x", ((u32bit *) itZStencilUpdateEmu->second.readData)[dw]);
                            }
                            printf("\n");
                            printf("    Input Data : ");
                            for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                            {
                                printf("  %08x", ((u32bit *) itZStencilUpdateEmu->second.inData)[dw]);
                            }
                            printf("\n");
                            printf("    Write Data : ");
                            for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                            {
                                printf("  %08x", ((u32bit *) itZStencilUpdateEmu->second.writeData)[dw]);
                            }
                            printf("\n");
                            printf("    Write Mask : ");
                            for(u32bit b = 0; b < (bytesPixel * STAMP_FRAGMENTS); b++)
                            {
                                printf(" %c", itZStencilUpdateEmu->second.writeMask[b] ? 'W' : '_');
                            }
                            printf("\n");
                            printf("    Cull Mask : ");
                            for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                            {
                                printf(" %c", itZStencilUpdateEmu->second.cullMask[f] ? 'T' : 'F');
                            }
                            printf("\n");
                            
                            printf("  Simulator Update : \n");
                            printf("    Read Data : ");
                            for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                            {
                                printf("  %08x", ((u32bit *) itZStencilUpdateSim->second.readData)[dw]);
                            }
                            printf("\n");
                            printf("    Input Data : ");
                            for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                            {
                                printf("  %08x", ((u32bit *) itZStencilUpdateSim->second.inData)[dw]);
                            }
                            printf("\n");
                            printf("    Write Data : ");
                            for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                            {
                                printf("  %08x", ((u32bit *) itZStencilUpdateSim->second.writeData)[dw]);
                            }
                            printf("\n");
                            printf("    Write Mask : ");
                            for(u32bit b = 0; b < (bytesPixel * STAMP_FRAGMENTS); b++)
                            {
                                printf(" %c", itZStencilUpdateSim->second.writeMask[b] ? 'W' : '_');
                            }
                            printf("\n");
                            printf("    Cull Mask : ");
                            for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                            {
                                printf(" %c", itZStencilUpdateSim->second.cullMask[f] ? 'T' : 'F');
                            }
                            printf("\n");

                            maxDifferences--;
                            
                            validationError = true;
                        }
                    }
                    
                    if (maxDifferences == 0)
                        printf("-- Reached maximum difference log limit  --\n");
                        
                    checkedQuads++;
                }
                
                //printf("Validation => Checked %d quads at Z Stencil stage\n", checkedQuads);

                //  Check the number of updates.
                if (simZStencilUpdateLog.size() > emuZStencilUpdateLog.size())
                {
                    FragmentQuadMemoryUpdateMap::iterator itZStencilUpdateSim;
                    
                    u32bit maxDifferences = 30;
                    
                    u32bit checkedQuads = 0;
                    
                    //  Traverse the whole update log and search differences.
                    for(itZStencilUpdateSim = simZStencilUpdateLog.begin();
                        itZStencilUpdateSim != simZStencilUpdateLog.end() && (maxDifferences > 0); itZStencilUpdateSim++)
                    {
                        //  Search the update in the emulator log.
                        itZStencilUpdateEmu = emuZStencilUpdateLog.find(itZStencilUpdateSim->first);
                        
                        if (itZStencilUpdateEmu == emuZStencilUpdateLog.end())
                        {
                            printf("Validation => Z Stencil memory update on quad at (%d, %d) triangle %d not found in the emulator log.\n",   
                                itZStencilUpdateSim->first.x, itZStencilUpdateSim->first.y, itZStencilUpdateSim->first.triangleID);
                            
                            maxDifferences--;
                        }
                    }
                }                
                
                //  Clear the z stencil memory update log.
                emuZStencilUpdateLog.clear();
                simZStencilUpdateLog.clear();
                
                for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
                {
                    //  Get the color memory update log from the emulator.
                    FragmentQuadMemoryUpdateMap &emuColorUpdateLog = gpuEmulator->getColorUpdateMap(rt);
                    
                    FragmentQuadMemoryUpdateMap simColorUpdateLog;

                    //  Get the color memory update logs from the simulator Color Write units.
                    for(u32bit rop = 0; rop < simP.gpu.numStampUnits; rop++)
                    {
                        //  Get the color memory update log from the color test unit.
                        FragmentQuadMemoryUpdateMap &simColorUpdateROPLog = colorWriteV2[rop]->getColorUpdateMap(rt);
                        
                        FragmentQuadMemoryUpdateMap::iterator it;

                        //  Retrieve the updates from the color write unit.
                        for(it = simColorUpdateROPLog.begin(); it != simColorUpdateROPLog.end(); it++)
                            simColorUpdateLog.insert(make_pair(it->first, it->second));
                        
                        //  Clear the z stencil update log.
                        simColorUpdateROPLog.clear();
                    }
                                       
//printf("Validation >> Render Target = %d | Simulator CW Updates = %d | Emulator CW Updates = %d\n", rt, simColorUpdateLog.size(),
//emuColorUpdateLog.size());

                    //  Check the number of updates.
                    if (simColorUpdateLog.size() != emuColorUpdateLog.size())
                    {
                        printf("Validation => Number of updates to Color Write Buffer differs: emulator %d simulator %d\n",
                            emuColorUpdateLog.size(), simColorUpdateLog.size());
                    
                        validationError = true;
                    }
                    
                    FragmentQuadMemoryUpdateMap::iterator itColorUpdateEmu;
                    
                    u32bit maxDifferences = 30;
                    
                    u32bit checkedQuads = 0;
                    
                    TextureFormat rtFormat = gpuEmulator->getRenderTargetFormat(rt);
                    
                    //  Traverse the whole update log and search differences.
                    for(itColorUpdateEmu = emuColorUpdateLog.begin();
                        itColorUpdateEmu != emuColorUpdateLog.end() && (maxDifferences > 0); itColorUpdateEmu++)
                    {
                        FragmentQuadMemoryUpdateMap::iterator itColorUpdateSim;
                        
                        //  Search the update in the simulator log.
                        itColorUpdateSim = simColorUpdateLog.find(itColorUpdateEmu->first);
                        
                        if (itColorUpdateSim == simColorUpdateLog.end())
                        {
                            printf("Validation => Color memory update on quad at (%d, %d) triangle %d not found in the simulator log.\n",   
                                itColorUpdateEmu->first.x, itColorUpdateEmu->first.y, itColorUpdateEmu->first.triangleID);
                        
                            //  Compare the two updates.
                            
                            u32bit bytesPixel;
                            
                            switch(rtFormat)
                            {
                                case GPU_RGBA8888:
                                case GPU_RG16F:
                                case GPU_R32F:
                                    bytesPixel = 4;
                                    break;
                                case GPU_RGBA16:
                                case GPU_RGBA16F:
                                    bytesPixel = 8;
                                    break;
                                default:
                                    panic("GPUSimulator", "advanceTime", "Unsupported render target format\n");
                            }
                            
                            printf("  Emulator Update : \n");
                            printf("    Read Data : ");
                            for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                            {
                                printf("  %08x", ((u32bit *) itColorUpdateEmu->second.readData)[dw]);
                            }
                            printf("\n");
                            printf("    Input Data : ");
                            for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                            {
                                printf("  (%f, %f, %f, %f) ", ((f32bit *) itColorUpdateEmu->second.inData)[f * 4 + 0],
                                                              ((f32bit *) itColorUpdateEmu->second.inData)[f * 4 + 1],
                                                              ((f32bit *) itColorUpdateEmu->second.inData)[f * 4 + 2],
                                                              ((f32bit *) itColorUpdateEmu->second.inData)[f * 4 + 3]);
                            }
                            printf("\n");
                            printf("    Write Data : ");
                            if (rtFormat == GPU_RGBA8888)
                            {
                                for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                                {
                                    printf("  %08x", ((u32bit *) itColorUpdateEmu->second.writeData)[dw]);
                                }
                            }
                            printf("\n");
                            printf("    Write Mask : ");
                            for(u32bit b = 0; b < (bytesPixel * STAMP_FRAGMENTS); b++)
                            {
                                printf(" %c", itColorUpdateEmu->second.writeMask[b] ? 'W' : '_');
                            }
                            printf("\n");
                            printf("    Cull Mask : ");
                            for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                            {
                                printf(" %c", itColorUpdateEmu->second.cullMask[f] ? 'T' : 'F');
                            }
                            printf("\n");
                            
                            maxDifferences--;
                        }
                        else
                        {
                            //  Compare the two updates.
                            
                            u32bit bytesPixel;
                            
                            switch(rtFormat)
                            {
                                case GPU_RGBA8888:
                                case GPU_RG16F:
                                case GPU_R32F:
                                    bytesPixel = 4;
                                    break;
                                case GPU_RGBA16:
                                case GPU_RGBA16F:
                                    bytesPixel = 8;
                                    break;
                                default:
                                    panic("GPUSimulator", "advanceTime", "Unsupported render target format\n");
                            }

                            bool diffInReadData = false;
                            
                            //  Compare read data.
                            for(u32bit qw = 0; (qw < ((bytesPixel * STAMP_FRAGMENTS)) >> 3) && !diffInReadData; qw++)
                                diffInReadData = (((u64bit *) itColorUpdateEmu->second.readData)[qw] != ((u64bit *) itColorUpdateSim->second.readData)[qw]);
                                                        
                            bool diffInWriteData = false;
                            
                            //  Compare write data.
                            for(u32bit qw = 0; (qw < ((bytesPixel * STAMP_FRAGMENTS)) >> 3) && !diffInWriteData; qw++)
                                diffInWriteData = (((u64bit *) itColorUpdateEmu->second.writeData)[qw] != ((u64bit *) itColorUpdateSim->second.writeData)[qw]);
                        
                            if (diffInReadData)
                                printf("Validation => Difference found in color read data for the update on quad at (%d, %d) triangle %d\n",
                                    itColorUpdateEmu->first.x, itColorUpdateEmu->first.y, itColorUpdateEmu->first.triangleID);

                            if (diffInWriteData)
                                printf("Validation => Difference found in color write data for the update on quad at (%d, %d) triangle %d\n",
                                    itColorUpdateEmu->first.x, itColorUpdateEmu->first.y, itColorUpdateEmu->first.triangleID);
                                    
                            if (diffInReadData || diffInWriteData)
                            {
                                printf("  Emulator Update : \n");
                                printf("    Read Data : ");
                                for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                                {
                                    printf("  %08x", ((u32bit *) itColorUpdateEmu->second.readData)[dw]);
                                }
                                printf("\n");
                                printf("    Input Data : ");
                                for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                                {
                                    printf("  (%f, %f, %f, %f) ", ((f32bit *) itColorUpdateEmu->second.inData)[f * 4 + 0],
                                                                  ((f32bit *) itColorUpdateEmu->second.inData)[f * 4 + 1],
                                                                  ((f32bit *) itColorUpdateEmu->second.inData)[f * 4 + 2],
                                                                  ((f32bit *) itColorUpdateEmu->second.inData)[f * 4 + 3]);
                                }
                                printf("\n");
                                printf("    Write Data : ");
                                if (rtFormat == GPU_RGBA8888)
                                {
                                    for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                                    {
                                        printf("  %08x", ((u32bit *) itColorUpdateEmu->second.writeData)[dw]);
                                    }
                                }
                                printf("\n");
                                printf("    Write Mask : ");
                                for(u32bit b = 0; b < (bytesPixel * STAMP_FRAGMENTS); b++)
                                {
                                    printf(" %c", itColorUpdateEmu->second.writeMask[b] ? 'W' : '_');
                                }
                                printf("\n");
                                printf("    Cull Mask : ");
                                for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                                {
                                    printf(" %c", itColorUpdateEmu->second.cullMask[f] ? 'T' : 'F');
                                }
                                printf("\n");
                                
                                printf("  Simulator Update : \n");
                                printf("    Read Data : ");
                                if (rtFormat == GPU_RGBA8888)
                                {
                                    for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                                    {
                                        printf("  %08x", ((u32bit *) itColorUpdateSim->second.readData)[dw]);
                                    }
                                }
                                printf("\n");
                                printf("    Input Data : ");
                                for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                                {
                                    printf("  (%f, %f, %f, %f) ", ((f32bit *) itColorUpdateSim->second.inData)[f * 4 + 0],
                                                                  ((f32bit *) itColorUpdateSim->second.inData)[f * 4 + 1],
                                                                  ((f32bit *) itColorUpdateSim->second.inData)[f * 4 + 2],
                                                                  ((f32bit *) itColorUpdateSim->second.inData)[f * 4 + 3]);
                                }
                                printf("\n");
                                printf("    Write Data : ");
                                if (rtFormat == GPU_RGBA8888)
                                {
                                    for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                                    {
                                        printf("  %08x", ((u32bit *) itColorUpdateSim->second.writeData)[dw]);
                                    }
                                }
                                printf("\n");
                                printf("    Write Mask : ");
                                for(u32bit b = 0; b < (bytesPixel * STAMP_FRAGMENTS); b++)
                                {
                                    printf(" %c", itColorUpdateSim->second.writeMask[b] ? 'W' : '_');
                                }
                                printf("\n");
                                printf("    Cull Mask : ");
                                for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                                {
                                    printf(" %c", itColorUpdateSim->second.cullMask[f] ? 'T' : 'F');
                                }
                                printf("\n");

                                maxDifferences--;
                                
                                validationError = true;
                            }
                        }
                        
                        checkedQuads++;
                    
                        if (maxDifferences == 0)
                            printf("-- Reached maximum difference log limit  --\n");
                    }
                    
                    //  Check the number of updates.
                    if (simColorUpdateLog.size() > emuColorUpdateLog.size())
                    {
                        FragmentQuadMemoryUpdateMap::iterator itColorUpdateSim;
                        
                        u32bit maxDifferences = 30;
                        
                        u32bit checkedQuads = 0;
                        
                        TextureFormat rtFormat = gpuEmulator->getRenderTargetFormat(rt);
                        
                        //  Traverse the whole update log and search differences.
                        for(itColorUpdateSim = simColorUpdateLog.begin();
                            itColorUpdateSim != simColorUpdateLog.end() && (maxDifferences > 0); itColorUpdateSim++)
                        {
                            //  Search the update in the emulator log.
                            itColorUpdateEmu = emuColorUpdateLog.find(itColorUpdateSim->first);
                            
                            if (itColorUpdateEmu == emuColorUpdateLog.end())
                            {
                                printf("Validation => Color memory update on quad at (%d, %d) triangle %d not found in the emulator log.\n",   
                                    itColorUpdateSim->first.x, itColorUpdateSim->first.y, itColorUpdateSim->first.triangleID);
                            
                                //  Compare the two updates.
                                
                                u32bit bytesPixel;
                                
                                switch(rtFormat)
                                {
                                    case GPU_RGBA8888:
                                    case GPU_RG16F:
                                    case GPU_R32F:
                                        bytesPixel = 4;
                                        break;
                                    case GPU_RGBA16:
                                    case GPU_RGBA16F:
                                        bytesPixel = 8;
                                        break;
                                    default:
                                        panic("GPUSimulator", "advanceTime", "Unsupported render target format\n");
                                }
                                
                                printf("  Simulator Update : \n");
                                printf("    Read Data : ");
                                for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                                {
                                    printf("  %08x", ((u32bit *) itColorUpdateSim->second.readData)[dw]);
                                }
                                printf("\n");
                                printf("    Input Data : ");
                                for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                                {
                                    printf("  (%f, %f, %f, %f) ", ((f32bit *) itColorUpdateSim->second.inData)[f * 4 + 0],
                                                                  ((f32bit *) itColorUpdateSim->second.inData)[f * 4 + 1],
                                                                  ((f32bit *) itColorUpdateSim->second.inData)[f * 4 + 2],
                                                                  ((f32bit *) itColorUpdateSim->second.inData)[f * 4 + 3]);
                                }
                                printf("\n");
                                printf("    Write Data : ");
                                if (rtFormat == GPU_RGBA8888)
                                {
                                    for(u32bit dw = 0; dw < ((bytesPixel * STAMP_FRAGMENTS) >> 2); dw++)
                                    {
                                        printf("  %08x", ((u32bit *) itColorUpdateSim->second.writeData)[dw]);
                                    }
                                }
                                printf("\n");
                                printf("    Write Mask : ");
                                for(u32bit b = 0; b < (bytesPixel * STAMP_FRAGMENTS); b++)
                                {
                                    printf(" %c", itColorUpdateSim->second.writeMask[b] ? 'W' : '_');
                                }
                                printf("\n");
                                printf("    Cull Mask : ");
                                for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
                                {
                                    printf(" %c", itColorUpdateSim->second.cullMask[f] ? 'T' : 'F');
                                }
                                printf("\n");
                                
                                maxDifferences--;
                            }
                        }
                    }
                    
                    //printf("Validation => Checked %d quads at Color Write stage\n", checkedQuads);
                    
                    //  Clear the color memory update log.
                    emuColorUpdateLog.clear();
                    simColorUpdateLog.clear();
                }
            }
            else
            {
                //  Get the shaded vertex log from the emulator.
                ShadedVertexMap &emuShadedVertexLog = gpuEmulator->getShadedVertexLog();
                
                //  Clear the log maps.
                emuShadedVertexLog.clear();

                //  Get the z stencil memory update log from the emulator.
                FragmentQuadMemoryUpdateMap &emuZStencilUpdateLog = gpuEmulator->getZStencilUpdateMap();
                
                //  Clear the z stencil memory update log.
                emuZStencilUpdateLog.clear();
                
                for(u32bit rt = 0; rt < MAX_RENDER_TARGETS; rt++)
                {
                    //  Get the color memory update log from the emulator.
                    FragmentQuadMemoryUpdateMap &emuColorUpdateLog = gpuEmulator->getColorUpdateMap(rt);
                    
                    //  Clear the color memory ypdate log.
                    emuColorUpdateLog.clear();
                }
            }                          
        }
        
        //  Check if auto snapshot is enabled and there is no pending snapshot.
        if (autoSnapshotEnable && !pendingSaveSnapshot)
        {
            //  Get new time.
            u64bit newTime = time(NULL);
            
            //  Check if enough time has passed to save a new snapshot.
            if ((newTime - startTime) >= (snapshotFrequency * 60))
            {
            
                printf("Autosaving at time %d.  Auto save frequency %d minutes.  Times since last save in seconds %d\n",
                    newTime, snapshotFrequency, newTime - startTime);
                    
                //  Save a new snapshot.
                saveSnapshotCommand();
                
                //  Update start time.
                startTime = time(NULL);
            }
        }
    }
    
    //  Check if the current frame has finished.
    if (commProc->isSwap())
    {
        //  Update rendered frames counter.
        frameCounter++;

        //  Reset batch counter for the current batch.
        frameBatch = 0;
        
        //  Set the end of frame flag.
        endOfFrame = true;

        //  In validation mode process all the transactions up to the swap.
        if (validationMode)
        {
            //  Get the log of AGP Transactions processed by the Command Processor.
            vector<AGPTransaction *> &agpTransLog = commProc->getAGPTransactionLog();
            
            //  Issue the AGP transactions to the emulator.
            for(u32bit trans = 0; trans < agpTransLog.size(); trans++)
            {
                gpuEmulator->emulateCommandProcessor(agpTransLog[trans]);
            }
            
            //  Clear the AGP Transaction log.
            agpTransLog.clear();
        }
    }
    
    //  Check if trace has finished
    if(!endOfTrace && commProc->isEndOfTrace())
    {
        //  Set the end of trace flag.
        endOfTrace = true;
        cout << "--- end of simulation trace ---" << endl;

        //  In validation mode process all the transactions up to the end of the trace.
        if (validationMode)
        {
            //  Get the log of AGP Transactions processed by the Command Processor.
            vector<AGPTransaction *> &agpTransLog = commProc->getAGPTransactionLog();
            
            //  Issue the AGP transactions to the emulator.
            for(u32bit trans = 0; trans < agpTransLog.size(); trans++)
            {
                gpuEmulator->emulateCommandProcessor(agpTransLog[trans]);
            }
            
            //  Clear the AGP Transaction log.
            agpTransLog.clear();
            
            cout << "--- end of emulation trace ---" << endl;
        }
    }

    
    //  Check stalls every 100K cycles.
    if (checkStalls)
    {
        gpuStalled = false;
        
        if (!multiClock)
        {
            for(u32bit b = 0; b < boxArray.size() && !gpuStalled; b++)
            {
                bool stallDetectionEnabled;
                bool boxStalled;

                boxArray[b]->detectStall(cycle, stallDetectionEnabled, boxStalled);
                
                gpuStalled = gpuStalled || (stallDetectionEnabled && boxStalled);
                
                if (gpuStalled)
                {
                    printf("Stall detected on box %s at cycle %lld\n", boxArray[b]->getName(), cycle);
                }
            }
        }
        else
        {
                for(u32bit box = 0; box < gpuDomainBoxes.size() && !gpuStalled; box++)
                {
                    bool stallDetectionEnabled;
                    bool boxStalled;
                       
                    gpuDomainBoxes[box]->detectStall(gpuCycle, stallDetectionEnabled, boxStalled);
                    
                    gpuStalled = gpuStalled || (stallDetectionEnabled && boxStalled);
                    
                    if (gpuStalled)
                    {
                        printf("Stall detected on box %s at cycle %lld\n", gpuDomainBoxes[box]->getName(), gpuCycle);
                    }
                }
                   
                //for(u32bit box = 0; box < shaderDomainBoxes.size(); box++)
                //    shaderDomainBoxes[box]->clock(GPU_CLOCK_DOMAIN, gpuCycle);
                   
                //for(u32bit box = 0; box < memoryDomainBoxes.size(); box++)
                //    memoryDomainBoxes[box]->clock(GPU_CLOCK_DOMAIN, gpuCycle);
        }
                
        if (gpuStalled)
        {
            ofstream reportFile;
            
            reportFile.open("StallReport.txt");                               
            
            if (!reportFile.is_open())
                cout << "ERROR: Couldn't create report file." << endl;
            
            cout << "Reporting stall for all boxes : " << endl;
            
            if (reportFile.is_open())
                reportFile <<  "Reporting stall for all boxes : " << endl;
            
            if (!multiClock)
            {
                //  Dump stall reports for all boxes.
                for(u32bit b = 0; b < boxArray.size(); b++)
                {
                    string report;
                    boxArray[b]->stallReport(cycle, report);
                                        
                    cout << report << endl;
                    
                    if (reportFile.is_open())
                        reportFile << report << endl;
                }
            }
            else
            {
                for(u32bit box = 0; box < gpuDomainBoxes.size() && !gpuStalled; box++)
                {
                    string report;
                    gpuDomainBoxes[box]->stallReport(gpuCycle, report);
                                        
                    cout << report << endl;
                    
                    if (reportFile.is_open())
                        reportFile << report << endl;
                }
            }
                    
            cout << "Stall report written into StallReport.txt file." << endl;    
            
            reportFile.close();
            
            createSnapshot();
        }
    }

    /*  Update progress counter.  */
    dotCount++;

    /*  Display progress dots.  */
    if (dotCount == 10000)
    {
        dotCount = 0;
        cout << '.';
        cout << flush;
    }
}


void GPUSimulator::simulationLoop()
{
    u32bit width, height;
    u32bit i;
    bool end;

    current = this;

    simulationStarted = true;
    
    //  Simulation loop.
    for(cycle = 0, end = false, dotCount = 0; !end; cycle++)
    {
        GPU_DEBUG( printf("Cycle %ld ----------------------------\n", cycle); )

        //  Dump the signals.
        if (simP.dumpSignalTrace && (cycle >= simP.startDump) && (cycle <= (simP.startDump + simP.dumpCycles)))
            sigBinder.dumpSignalTrace(cycle);

        //  Update cycle counter statistic.
        cyclesCounter->inc();
        
        // Clock all the boxes.
        for(i = 0; i < boxArray.size(); i++)
            boxArray[i]->clock(cycle);

        //  Check if statistics generation is active.
        if (simP.statistics)
        {
            //  Update statistics.
            GPUStatistics::StatisticsManager::instance().clock(cycle);

            //  Check if current batch has finished.
            if (commProc->endOfBatch() && simP.perBatchStatistics)
            {
                //  Update statistics.
                GPUStatistics::StatisticsManager::instance().batch();
            }

            //  Check if statistics and per frame statistics are enabled.
            if (commProc->isSwap() && simP.perFrameStatistics)
            {
                //  Update statistics.
                GPUStatistics::StatisticsManager::instance().frame(frameCounter);
            }
        }

        //  Check end of batch event.
        if (commProc->endOfBatch())
        {
            //  Update rendered batches counter.
            batchCounter++;
            
            //  Update rendered batches in the current frame.
            frameBatch++;
        }
        
        //  Check if color buffer swap has started and fragment map is enabled.
        if (commProc->isSwap())
        {
            //  Check if fragment map dumping is enabled.
            if  (simP.fragmentMap)
            {
                //  Get the latency maps from the color write units.
                for(i = 0; i < simP.gpu.numStampUnits; i++)
                    latencyMap[i] = colorWriteV2[i]->getLatencyMap(width, height);

                //  Dump the latency map.
                dumpLatencyMap(width, height);
            }

            //  Update frame counter.
            frameCounter++;

            //  Reset batch counter for the current batch.
            frameBatch = 0;
            
            //  Determine end of simulation.
            if (simP.simFrames != 0)
            {
                //  End if the number of requested frames has been rendered.
                end = end || ((frameCounter - simP.startFrame) == simP.simFrames);
            }
        }
        
        //  Update progress counter.
        dotCount++;

        //  Display progress dots.
        if (dotCount == 10000)
        {
            dotCount = 0;
            putchar('.');
            fflush(stdout);

//OptimizedDynamicMemory::usage();

            //  Check if trace has finished.
            end = end || commProc->isEndOfTrace();
        }

        //  Determine end of simulation.
        if (simP.simCycles != 0)
        {
            end = end || ((cycle + 1) == simP.simCycles);
        }
        
        //  Check stalls every 100K cycles.
        if (simP.detectStalls && (GPU_MOD(cycle, 100000) == 99999))
        {
            bool gpuStalled = false;
            
            for(i = 0; i < boxArray.size() && !gpuStalled; i++)
            {
                bool stallDetectionEnabled;
                bool boxStalled;

                boxArray[i]->detectStall(cycle, stallDetectionEnabled, boxStalled);
                               
                gpuStalled = gpuStalled || (stallDetectionEnabled && boxStalled);

                if (gpuStalled)
                {
                    printf("Stall detected on box %s at cycle %lld\n", boxArray[i]->getName(), cycle);
                }
            }
            
            if (gpuStalled)
            {
                ofstream reportFile;
                
                reportFile.open("StallReport.txt");                               
                
                if (!reportFile.is_open())
                    cout << "ERROR: Couldn't create report file." << endl;
                
                cout << "Reporting stall for all boxes : " << endl;
                
                if (reportFile.is_open())
                    reportFile <<  "Reporting stall for all boxes : " << endl;
                
                //  Dump stall reports for all boxes.
                for(i = 0; i < boxArray.size(); i++)
                {
                    string report;
                    boxArray[i]->stallReport(cycle, report);
                                        
                    cout << report << endl;
                    
                    if (reportFile.is_open())
                        reportFile << report << endl;
                }
            
                cout << "Stall report written into StallReport.txt file." << endl;    
                
                reportFile.close();
                
                createSnapshot();
            }
            
            //  End the simulation if a stall was detected.                  
            end = end || gpuStalled;
        }
    }

    //GPU_DEBUG(
        printf("\nEND Cycle %lld ----------------------------\n", cycle);
    //)

    //  Check signal trace dump enabled.  */
    if (simP.dumpSignalTrace)
    {
        /*  End signal tracing.  */
        sigBinder.endSignalTrace();
    }

    OptimizedDynamicMemory::usage();
    GPUStatistics::StatisticsManager::instance().finish();

    //OptimizedDynamicMemory::dumpDynamicMemoryState(FALSE, FALSE);
}

void GPUSimulator::multiClockSimLoop()
{
    u32bit width, height;
    u32bit i;
    bool end;

    current = this;

    // Initialize clock, step counters, etc.
    gpuCycle = 0;
    shaderCycle = 0;
    memoryCycle = 0;
    nextGPUClock = gpuClockPeriod;
    nextShaderClock = shaderClockPeriod;
    nextMemoryClock = memoryClockPeriod;
    end = false;
    
    simulationStarted = true;

    while(!end)
    {
        //
        //  WARNING: Signal Trace Dump not supported yet with multi clock domain.
        //
        //  Dump the signals.
        //if (simP.dumpSignalTrace && (cycle >= simP.startDump) && (cycle <= (simP.startDump + simP.dumpCycles)))
        //    sigBinder.dumpSignalTrace(cycle);

        //  Determine the picoseconds to the next clock.
        u32bit nextStep = GPU_MIN(GPU_MIN(nextGPUClock, nextShaderClock), nextMemoryClock);
        
        //printf("Clock -> Next GPU Clock = %d | Next Shader Clock = %d | Next Memory Clock = %d | Next Step = %d | GPU Cycle = %lld | Shader Cycle = %lld | MemoryCycle = %lld\n",
        //    nextGPUClock, nextShaderClock, nextMemoryClock, nextStep, gpuCycle, shaderCycle, memoryCycle);
                        
        //  Update step counters for both clock domains.
        nextGPUClock -= nextStep;
        nextShaderClock -= nextStep;
        nextMemoryClock -= nextStep;
        
        //  Check if a gpu domain tick has to be issued.
        if (nextGPUClock == 0)
        {
            GPU_DEBUG(
                printf("GPU Domain. Cycle %lld ----------------------------\n", gpuCycle);
            )

            // Clock all the boxes in the GPU Domain.
            for(i = 0; i < gpuDomainBoxes.size(); i++)
                gpuDomainBoxes[i]->clock(gpuCycle);
                
            //  Clock boxes with multiple domains.
            for(i = 0; i < shaderDomainBoxes.size(); i++)
                shaderDomainBoxes[i]->clock(GPU_CLOCK_DOMAIN, gpuCycle);

            for(i = 0; i < memoryDomainBoxes.size(); i++)
                memoryDomainBoxes[i]->clock(GPU_CLOCK_DOMAIN, gpuCycle);

            //  Update cycle counter statistic.
            cyclesCounter->inc();
            
            //  Check if statistics generation is active.
            if (simP.statistics)
            {
                //  Update statistics.
                GPUStatistics::StatisticsManager::instance().clock(gpuCycle);

                //  Check if current batch has finished.
                if (commProc->endOfBatch() && simP.perBatchStatistics)
                {
                    //  Update statistics.
                    GPUStatistics::StatisticsManager::instance().batch();
                }


                //  Check if statistics and per frame statistics are enabled.
                if (commProc->isSwap() && simP.perFrameStatistics)
                {
                    /*  Update statistics.  */
                    GPUStatistics::StatisticsManager::instance().frame(frameCounter);
                }
            }
            
            //  Check end of batch event.
            if (commProc->endOfBatch())
            {
                //  Update rendered batches counter.
                batchCounter++;
                
                //  Update rendered batches in the current frame.
                frameBatch++;
            }

            //  Check if color buffer swap has started and fragment map is enabled.
            if (commProc->isSwap())
            {
                //  Check if fragment map dumping is enabled.
                if  (simP.fragmentMap)
                {
                    //  Get the latency maps from the color write units.
                    for(i = 0; i < simP.gpu.numStampUnits; i++)
                        latencyMap[i] = colorWriteV2[i]->getLatencyMap(width, height);

                    //  Dump the latency map.
                    dumpLatencyMap(width, height);
                }

                //  Update frame counter.
                frameCounter++;
                
                //  Reset batch counter for the current batch.
                frameBatch = 0;

                //  Determine end of simulation.
                if (simP.simFrames != 0)
                {
                    //  End if the number of requested frames has been rendered.
                    end = end || ((frameCounter - simP.startFrame) == simP.simFrames);
                }
            }
            
            //  Update progress counter.
            dotCount++;

            //  Display progress dots.
            if (dotCount == 10000)
            {
                dotCount = 0;
                putchar('.');
                fflush(stdout);

//OptimizedDynamicMemory::usage();

                //  Check if trace has finished.
                end = end || commProc->isEndOfTrace();
            }

            //  Determine end of simulation.
            if (simP.simCycles != 0)
            {
                end = end || ((gpuCycle + 1) == simP.simCycles);
            }
            
            //  Check stalls every 100K cycles.
            if (simP.detectStalls && (GPU_MOD(gpuCycle, 100000) == 99999))
            {
                bool gpuStalled = false;
                            
                for(i = 0; i < gpuDomainBoxes.size() && !gpuStalled; i++)
                {
                    bool stallDetectionEnabled;
                    bool boxStalled;
                 
                    gpuDomainBoxes[i]->detectStall(gpuCycle, stallDetectionEnabled, boxStalled);
                    
                    gpuStalled |= stallDetectionEnabled && boxStalled;
                    
                    if (gpuStalled)
                    {
                        printf("Stall detected on box %s at cycle (GPU Domain) %lld\n", gpuDomainBoxes[i]->getName(), gpuCycle);
                    }
                }

                for(i = 0; i < shaderDomainBoxes.size() && !gpuStalled; i++)
                {
                    bool stallDetectionEnabled;
                    bool boxStalled;
                 
                    shaderDomainBoxes[i]->detectStall(shaderCycle, stallDetectionEnabled, boxStalled);
                    
                    gpuStalled |= stallDetectionEnabled && boxStalled;
                    
                    if (gpuStalled)
                    {
                        printf("Stall detected on box %s at cycle (Shader Domain) %lld\n", shaderDomainBoxes[i]->getName(), shaderCycle);
                    }
                }
                
                if (gpuStalled)
                {
                    ofstream reportFile;
                    
                    reportFile.open("StallReport.txt");                               
                    
                    if (!reportFile.is_open())
                        cout << "ERROR: Couldn't create report file." << endl;
                    
                    cout << "Reporting stall for all boxes : " << endl;
                    
                    if (reportFile.is_open())
                        reportFile <<  "Reporting stall for all boxes : " << endl;
                    
                    //  Dump stall reports for all boxes.
                    for(i = 0; i < boxArray.size(); i++)
                    {
                        string report;
                        boxArray[i]->stallReport(gpuCycle, report);
                                            
                        //cout << report << endl;
                        
                        if (reportFile.is_open())
                            reportFile << report << endl;
                    }
                
                    cout << "Stall report written into StallReport.txt file." << endl;    
                    
                    reportFile.close();
                }
                
                //  End the simulation if a stall was detected.                  
                end = end || gpuStalled;
            }
            
            //  Update GPU domain clock and step counter.
            gpuCycle++;
            nextGPUClock = gpuClockPeriod;
        }
        
        //  Check if a shader domain tick must be issued.
        if (nextShaderClock == 0)
        {
            if (shaderClockDomain)
            {
                GPU_DEBUG(
                    printf("Shader Domain. Cycle %lld ----------------------------\n", shaderCycle);
                )

                //  Clock boxes with multiple domains.
                for(i = 0; i < shaderDomainBoxes.size(); i++)
                    shaderDomainBoxes[i]->clock(SHADER_CLOCK_DOMAIN, shaderCycle);

                //  Update shader domain clock and step counter.
                shaderCycle++;
            }
            
            nextShaderClock = shaderClockPeriod;
        }        
        
        //  Check if a memory domain tick must be issued.
        if (nextMemoryClock == 0)
        {
            if (memoryClockDomain)
            {
                GPU_DEBUG(
                    printf("Memory Domain. Cycle %lld ----------------------------\n", memoryCycle);
                )
                
                //  Clock boxes with multiple domains.
                for(i = 0; i < memoryDomainBoxes.size(); i++)
                    memoryDomainBoxes[i]->clock(MEMORY_CLOCK_DOMAIN, memoryCycle);

                //  Update memory domain clock and step counter.
                memoryCycle++;
            }
            
            nextMemoryClock = memoryClockPeriod;            
        }
    }
    
    //GPU_DEBUG(
        printf("\n");
        printf("GPU Domain. END Cycle %lld ----------------------------\n", gpuCycle);
        if (shaderClockDomain)
            printf("Shader Domain. END Cycle %lld ----------------------------\n", shaderCycle);
        if (memoryClockDomain)
            printf("Memory Domain. END Cycle %lld ----------------------------\n", memoryCycle);
        printf("\n");
    //)

    //
    //  WARNING:  Signal trace dump not supported with multiple clock domains.
    //
    
    //  Check signal trace dump enabled.  */
    //if (simP.dumpSignalTrace)
    //{
        //  End signal tracing.  */
    //    sigBinder.endSignalTrace();
    //}

    OptimizedDynamicMemory::usage();
    GPUStatistics::StatisticsManager::instance().finish();

    //OptimizedDynamicMemory::dumpDynamicMemoryState(FALSE, FALSE);
}

void GPUSimulator::getCounters(u32bit &frame, u32bit &batch, u32bit &totalBatches)
{
    frame = frameCounter;
    batch = frameBatch;
    totalBatches = batchCounter;
}

void GPUSimulator::getCycles(u64bit &_gpuCycle, u64bit &_shaderCycle, u64bit &_memCycle)
{
    if (multiClock)
    {
        _gpuCycle = gpuCycle;
        if (shaderClockDomain)
            _shaderCycle = shaderCycle;
        else
            _shaderCycle = 0;
        if (memoryClockDomain)
            _memCycle = memoryCycle;
        else
            _memCycle = 0;
    }
    else
    {
        _gpuCycle = cycle;
        _shaderCycle = 0;
        _memCycle = 0;
    }
}

void GPUSimulator::abortSimulation()
{
    abortDebug = true;
}


/*  Dumps the latency maps.  */
void GPUSimulator::dumpLatencyMap(u32bit w, u32bit h)
{
    FILE *fout;
    char filename[30];
    s32bit x, y;
    u32bit stampLatency;
    u32bit i;

    /*  Create current frame filename.  */
    sprintf(filename, "latencyMap%04d.ppm", frameCounter);

    /*  Open/Create the file for the current frame.  */
    fout = fopen(filename, "wb");

    /*  Check if the file was correctly created.  */
    GPU_ASSERT(
        if (fout == NULL)
            panic("GPUSimulator", "dumpLatencyMap", "Error creating latency map output file.");
    )

    /*  Write file header.  */

    /*  Write magic number.  */
    fprintf(fout, "P6\n");

    /*  Write latency map size.  */
    fprintf(fout, "%d %d\n", w, h);

    /*  Write color component maximum value.  */
    fprintf(fout, "255\n");

    /* Do this for the whole picture now */
    for (y = h - 1; y >= 0; y--)
    {
        for (x = 0; x < s32bit(w); x++)
        {
            /*  Calculate the latency for the stamp.  Does a join of the
                latency maps per color write unit (stamp pipe).  A stamp should
                have latency > 0 only for one of the color write units.  */
            for(i = 0, stampLatency = 0; i < simP.gpu.numStampUnits; i++)
            {
                if ((stampLatency != 0) && (latencyMap[i][y * w + x] != 0))
                    printf("stamp written by two units!!! at %d %d => ", x, y);
                stampLatency += latencyMap[i][y * w + x];
            }

            //stampLatency = applyColorKey(stampLatency);

            fputc(char(stampLatency & 0xff), fout);
            fputc(char((stampLatency >> 8) & 0xff), fout);
            fputc(char((stampLatency >> 16) & 0xff), fout);
        }
    }

    fclose(fout);
}

u32bit GPUSimulator::applyColorKey(u32bit p)
{
    if (p == 0)
        return 0x00000000;
    if ((p > 0) && (p <= 50))
        return 0x00000020;
    if ((p > 50) && (p <= 100))
        return 0x00000040;
    if ((p > 100) && (p <= 150))
        return 0x00000080;
    if ((p > 150) && (p <= 200))
        return 0x000000ff;
    if ((p > 200) && (p <= 300))
        return 0x00002000;
    if ((p > 300) && (p <= 400))
        return 0x00004000;
    if ((p > 400) && (p <= 500))
        return 0x00008000;
    if ((p > 500) && (p <= 600))
        return 0x0000ff00;
    if ((p > 600) && (p <= 800))
        return 0x00200000;
    if ((p > 800) && (p <= 1000))
        return 0x00400000;
    if ((p > 1000) && (p <= 1200))
        return 0x00800000;
    if ((p > 1200) && (p <= 1400))
        return 0x00ff0000;
    if ((p > 1400) && (p <= 1800))
        return 0x00ff0020;
    if ((p > 1800) && (p <= 2200))
        return 0x00ff0040;
    if ((p > 2200) && (p <= 2600))
        return 0x00ff0080;
    if ((p > 2600) && (p <= 3000))
        return 0x00ff00ff;
    if ((p > 3000) && (p <= 3800))
        return 0x00ff2000;
    if ((p > 3800) && (p <= 4600))
        return 0x00ff4000;
    if ((p > 4600) && (p <= 5400))
        return 0x00ff8000;
    if ((p > 5400) && (p <= 6200))
        return 0x00ffff00;
    if ((p > 6200) && (p <= 0xffffffff))
        return 0x00ffffff;

    return 0x00ffffff;
}

}   // namespace gpu3d


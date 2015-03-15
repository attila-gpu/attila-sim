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
 * GPU simulator definition file.
 *
 */

/**
 *
 *  @file GPUSimulator.h
 *
 *  This file contains definitions and includes for the ATTILA GPU simulator.
 *
 */

#ifndef _GPUSIMULATOR_

#define _GPUSIMULATOR_

#include "ConfigLoader.h"

#include "TraceDriverInterface.h"

#include "GPU.h"

#include "StatisticsManager.h"
#include "SignalBinder.h"
#include "OptimizedDynamicMemory.h"

//  Emulators.
#include "ShaderEmulator.h"
#include "RasterizerEmulator.h"
#include "TextureEmulator.h"
#include "FragmentOpEmulator.h"
#include "ColorCompressorEmulator.h"
#include "DepthCompressorEmulator.h"

#include "GPUEmulator.h"

//  Simulator boxes.
#include "ShaderFetch.h"
#include "ShaderDecodeExecute.h"
#include "VectorShaderFetch.h"
#include "VectorShaderDecodeExecute.h"
#include "CommandProcessor.h"
#include "MemoryController.h"
#include "MemoryControllerSelector.h"
#include "Streamer.h"
#include "PrimitiveAssembly.h"
#include "Clipper.h"
#include "Rasterizer.h"
#include "TextureUnit.h"
#include "ZStencilTestV2.h"
#include "ColorWriteV2.h"
#include "DAC.h"

#include "zfstream.h"

#include <vector>

namespace gpu3d
{

/**
 *
 *  GPU Simulator class.
 *
 *  The class defines structures and attributes to implement a GPU simulator.
 *
 */

class GPUSimulator
{
private:

    SimParameters simP;             /**<  Structure storing the simulator configuration parameters.  */
    bool unifiedShader;             /**<  Flag storing if the simulated architecture has unified shaders.  */
    bool d3d9Trace;                 /**<  Flag that stores if the input trace is a D3D9 API trace (PIX).  */
    bool oglTrace;                  /**<  Flag that stores if the input trace is a OGL API trace.  */
    bool agpTrace;                  /**<  Flag that stores if the input trace is an AGP Transaction trace.  */
    
    //  Emulators.
    ShaderEmulator **vshEmu;        /**<  Pointer to the array of pointers to vertex shader emulators.  */
    ShaderEmulator **fshEmu;        /**<  Pointer to the array of pointers to fragment shader emulators.  */
    TextureEmulator **texEmu;       /**<  Pointer to the array of pointers to texture emulators.  */
    RasterizerEmulator *rastEmu;    /**<  Pointer to a rasterizer emulator.  */
    FragmentOpEmulator **fragEmu;   /**<  Pointer to the array of pointers to fragment operation emulators.  */

    //  Simulation boxes.
    ShaderFetch **vshFetch;                     /**<  Pointer to the array of pointers to ShaderFetch boxes for the simulated vertex shaders.  */
    ShaderDecodeExecute **vshDecExec;           /**<  Pointer to the array of pointers to ShaderDecodeExecute boxes for the simulated vertex shaders.  */
    CommandProcessor *commProc;                 /**<  Pointer to the CommandProcessor box.  */
    Box* memController;                         /**<  Pointer to the MemoryController box (top level).  */
    Streamer *streamer;                         /**<  Pointer to the Streamer box (top level).  */
    PrimitiveAssembly *primAssem;               /**<  Pointer to the PrimitiveAssembly box.  */
    Clipper *clipper;                           /**<  Pointer to the Clipper box.  */
    Rasterizer *rast;                           /**<  Pointer to the Rasterizer box (top level).  */
    ShaderFetch **fshFetch;                     /**<  Pointer to the array of pointers to ShaderFetch boxes for the simulated unified/fragment shaders.  */
    ShaderDecodeExecute **fshDecExec;           /**<  Pointer to the array of pointers to ShaderDecodeExecute boxes for the simulated unified/fragment shaders.  */
    VectorShaderFetch **vecShFetch;             /**<  Pointer to the array of pointers to VectorShaderFetch boxes for the simulated unified shaders.  */
    VectorShaderDecodeExecute **vecShDecExec;   /**<  Pointer to the array of pointers to VectorShaderDecodeExecute boxes for the simulated unified shaders.  */
    TextureUnit **textUnit;                     /**<  Pointer to the array of pointers to TextureUnit boxes.  */
    ZStencilTestV2 **zStencilV2;                /**<  Pointer to the array of pointers to ZStencilTest boxes.  */
    ColorWriteV2 **colorWriteV2;                /**<  Pointer to the array of pointers to ColorWrite boxes.  */
    DAC *dac;                                   /**<  Pointer to the DAC box.  */

    //  Trace reader+driver.
    TraceDriverInterface *trDriver;     /**<  Pointer to a TraceDriverInterface object that will provide the AGP Transactions to simulate.  */

    SignalBinder &sigBinder;            /**<  Reference to the SignalBinder that tracks all signals between and in the simulator boxes.  */

    u64bit cycle;               /**<  Cycle counter for GPU clock domain (unified clock architecture).  */
    u64bit gpuCycle;            /**<  Cycle counter for GPU clock domain.  */
    u64bit shaderCycle;         /**<  Cycle counter for shader clock domain.  */
    u64bit memoryCycle;         /**<  Cycle counter for memory clock domain.  */
    u32bit gpuClockPeriod;      /**<  Period of the GPU clock in picoseconds (ps).  */
    u32bit shaderClockPeriod;   /**<  Period of the shader clock in picoseconds (ps).  */
    u32bit memoryClockPeriod;   /**<  Period of the memory clock in picoseconds (ps).  */
    u32bit nextGPUClock;        /**<  Stores the picoseconds (ps) to next gpu clock.  */
    u32bit nextShaderClock;     /**<  Stores the picoseconds (ps) to next shader clock.  */
    u32bit nextMemoryClock;     /**<  Stores the picoseconds (ps) to next memory clock.  */
    bool shaderClockDomain;     /**<  Flag that stores if the simulated architecture implements a shader clock domain.  */
    bool memoryClockDomain;     /**<  Flag that stores if the simulated architecture implements a memory clock domain.  */
    bool multiClock;            /**<  Flag that stores if the simulated architecture implements a multiple clock domains.  */

    bool simulationStarted;     /**<  Flag that stores if the simulation was started.  */
    u32bit dotCount;            /**<  Progress dot displayer counter.  */
    u32bit frameCounter;        /**<  Counts the frames (swap commands).  */
    u32bit batchCounter;        /**<  Stores the number of batches rendered.  */
    u32bit frameBatch;          /**<  Stores the batches rendered in the current batch.  */
    u32bit **latencyMap;        /**<  Arrays for the signal maps.  */
    bool abortDebug;            /**<  Flag that stores if the abort signal has been received.  */
    bool endDebug;              /**<  Flag that stores if the simulator debugger must end.  */

    std::vector<Box*> boxArray;                     /**<  Stores a pointer to all the boxes in the simulated architecture.  */

    std::vector<Box*> gpuDomainBoxes;               /**<  Stores a pointer to all the boxes in the GPU clock domain (unified clock).  */
    std::vector<MultiClockBox*> shaderDomainBoxes;  /**<  Stores a pointer to all the boxes with GPU and shader clock domain (multiple domains).  */
    std::vector<MultiClockBox*> memoryDomainBoxes;  /**<  Stores a pointer to all the boxes with GPU and memory clock domain (multiple domains).  */

    GPUStatistics::Statistic *cyclesCounter;    /**<  Pointer to GPU statistic used to count the number of simulated cycles (main clock domain!).  */

    gzofstream out;             /**<  Compressed stream output file for statistics.  */
    gzofstream outFrame;        /**<  Compressed stream output file for per frame statistics.  */
    gzofstream outBatch;        /**<  Compressed stream output file for per batch statistics.  */
    gzofstream sigTraceFile;    /**<  Compressed stream output file for signal dump trace.  */


    static GPUSimulator* current;       /**<  Stores the pointer to the currently executing GPU simulator instance.  */
    
    bool pendingSaveSnapshot;   /**<  Flag that stores if there is pending snapshot in process. */
    bool autoSnapshotEnable;    /**<  Flag that stores if auto snapshot is enabled.  */
    u32bit snapshotFrequency;   /**<  Stores the snapshot frequency in minutes.  */
    u64bit startTime;           /**<  Stores the time since the previous snapshot.  */
    
    //  Debug/Validation.
    bool validationMode;        /**<  Stores if the validation mode is enabled.  */
    bool skipValidation;        /**<  Used to skip validation when loading a snapshot.  */
    GPUEmulator *gpuEmulator;   /**<  Pointer to the associated GPU emulator for validation purposes.  */
    
    /**
     *
     *  Saves the simulator state to the 'state.snapshot' file.
     *
     *  The simulator state includes current frame and draw call, current simulation cycle(s),
     *  trace information and current trace position.
     *
     */
    void saveSimState();
    
    /**
     *
     *  Saves the simulator configuration parameters (SimParameters structure) to the 'config.snapshot' file.
     *
     */
    
    void saveSimConfig();
    
public:

    /**
     *
     *  GPUSimulator constructor.
     *
     *  Creates and initializes the objects and state required to simulate a GPU.
     *
     *  The emulation objects and and simulation boxes are created.
     *  The statistics and signal dump state and files are initialized and created if the simulation
     *  as required by the simulator parameters.
     *
     *  @param simP Configuration parameters.
     *  @param trDriver Pointer to the driver that provides AGP transactions for simulation.
     *  @param unified Boolean value that defines if the simulated architecture implements unified shaders.
     *  @param d3d9Trace Boolean value that defines if the input trace is a D3D9 API trace (PIX).
     *  @param oglTrace Boolean value that defines if the input trace is an OpenGL API trace.
     *  @param agpTrace Boolean value that defines if the input trace is an AGP Transaction trace.
     *
     */

    GPUSimulator(SimParameters simP, TraceDriverInterface *trDriver, bool unified, bool d3d9Trace, bool oglTrace, bool agpTrace);

    /**
     *
     *
     *  GPUSimulator destructor.
     *
     */
     
    ~GPUSimulator();
    
    /**
     *
     *  Fire-and-forget simulation loop for a single clock architecture.
     *
     */
     
    void simulationLoop();

    /**
     *
     *  Fire-and-forget simulation loop for a multi-clock architecture.
     *
     */

    void multiClockSimLoop();            

    /**
     *
     *  Fire-and-forget simulation loop with integrated debugger.
     *
     */
     
    void debugLoop(bool validate);

    /**
     *
     *  Implements the 'debugmode' command of the GPU simulator integrated debugger.
     *
     *  The 'debugmode' command is used to enable or disable verbose output for a specific simulator box.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */
     
    void debugModeCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'listboxes' command of the GPU simulator integrated debugger.
     *
     *  The 'listboxes' command is used to output a list the boxes instantiated for the simulated GPU architecture.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void listBoxesCommand(stringstream &streamCom);
    
    /**
     *
     *  Implements the 'listcommands' command of the GPU simulator integrated debugger.
     *
     *  The 'listcommands' command is used to output a list of debug commands supported by a given simulator box.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */
    
    void listBoxCommCommand(stringstream &streamCom);
    
    /**
     *
     *  Implements the 'execcommand' command of the GPU simulator integrated debugger.
     *
     *  The 'execcommand' command is used to execute a command in the defined simulator box.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */
    
    void execBoxCommCommand(stringstream &streamCom);
    
    /**
     *
     *  Implements the 'help' command of the GPU simulator integrated debugger.
     *
     *  The 'help' command outputs a list of all the command supported by the integrated simulator debugger.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void helpCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'run' command of the GPU simulator integrated debugger.
     *
     *  The 'run' command simulates the defined number of cycles (main clock).
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void runCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'runframe' command of the GPU simulator integrated debugger.
     *
     *  The 'run' command simulates the defined number of frames.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void runFrameCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'runbatch' command of the GPU simulator integrated debugger.
     *
     *  The 'runbatch' command simulates the defined number of batches (draw calls).
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void runBatchCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'runcom' command of the GPU simulator integrated debugger.
     *
     *  The 'runcom' command simulated the defined number of AGP commands.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */
     
    void runComCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'skipframe' command of the GPU simulator integrated debugger.
     *
     *  The 'skipframe' command skips (only library state is updated) the defined number of frames from the input trace.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void skipFrameCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'skipbatch' command of the GPU simulator integrated debugger.
     *
     *  The 'skipbatch' command skips (only library state is updated) the defined number of batches (draw calls) from the input trace.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void skipBatchCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'state' command of the GPU simulator integrated debugger.
     *
     *  The 'state' command outputs information about the state of the simulator or a specific simulator box.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void stateCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'savesnapshot' command of the GPU simulator integrated debugger.
     *
     *  The 'savesnapshot' command saves a snapshot of the current simulator state (memory, caches, registers, trace) into
     *  a newly created snapshot directory.
     *
     */

    void saveSnapshotCommand();

    /**
     *
     *  Implements the 'loadsnapshot' command of the GPU simulator integrated debugger.
     *
     *  The 'loadsnapshot' command loads a snapshot with simulator state (memory, caches, registers, trace) into
     *  from the defined snapshot directory.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void loadSnapshotCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'autosnapshot' command of the GPU simulator integrated debugger.
     *
     *  The 'autosnapshot' command sets to simulator to automatically save every n minutes a snapshot of the current
     *  simulator state (memory, caches, registers, trace) into a newly created snapshot directory.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void autoSnapshotCommand(stringstream &streamCom);

    //  Debug commands associated with the GPU Driver

    /**
     *
     *  Implements the 'memoryUsage' command of the GPU simulator integrated debugger.
     *
     *  The 'memoryUsage' outputs information about GPU memory utilization from the GPU driver.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */
     
    void driverMemoryUsageCommand(stringstream &streamCom);
    
    /**
     *
     *  Implements the 'listMDs' command of the GPU simulator integrated debugger.
     *
     *  The 'listMDs' outputs the list of defined memory descriptors from the GPU driver.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */
     
    void driverListMDsCommand(stringstream &streamCom);
    
    /**
     *
     *  Implements the 'infoMD' command of the GPU simulator integrated debugger.
     *
     *  The 'infoMD' outputs information about a memory descriptor from the GPU driver.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void driverInfoMDCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'dumpAGPBuffer' command of the GPU simulator integrated debugger.
     *
     *  The 'dumpAGPBuffer' outputs the current content of the AGP Transaction buffer in the GPU driver.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */
   
    void driverDumpAGPBufferCommand(stringstream &streamCom);
    
    /**
     *
     *  Implements the 'memoryAlloc' command of the GPU simulator integrated debugger.
     *
     *  The 'memoryAlloc' outputs information about the GPU memory allocation from the GPU driver.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void driverMemAllocCommand(stringstream &streamCom);

    /**
     *
     *  Implements the 'glcontext' command of the GPU simulator integrated debugger.
     *
     *  The 'glcontext' outputs information from the legacy OpenGL library (deprecated).
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    //void libraryGLContextCommand(stringstream &lineStream);
    
    /**
     *
     *  Implements the 'dumpStencil' command of the GPU simulator integrated debugger.
     *
     *  The 'dumpStencil' forces the legacy OpenGL library to dump the content of the stencil buffer (deprecated).
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    //void libraryDumpStencilCommand(stringstream &lineStream);

    /**
     *
     *  In the integrated simulator debugger runs the simulation until the execution of a command forced from
     *  the debugger into the simulated GPU finishes.
     *
     *  @return If the simulation of the input trace has finished.
     *
     */
     
    bool simForcedCommand();

    /**
     *
     *  Implements the 'emulatortrace' command of the GPU simulator integrated debugger.
     *
     *  The 'emulatortrace' command is used to enable/disable the trace log from the GPU emulator
     *  instantiated for the validation mode.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void emulatorTraceCommand(stringstream &lineStream);

    /**
     *
     *  Implements the 'tracevertex' command of the GPU simulator integrated debugger.
     *
     *  The 'tracevertex' command is used to enable/disable the trace of the execution of a defined
     *  vertex in the Shader Processors.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void traceVertexCommand(stringstream &lineStream);

    /**
     *
     *  Implements the 'tracefragment' command of the GPU simulator integrated debugger.
     *
     *  The 'tracefragment' command is used to enable/disable the trace of the execution of a defined
     *  fragment in the Shader Processors.
     *
     *  @param streamCom A reference to a stringstream object storing the line with the debug command and parameters.
     *
     */

    void traceFragmentCommand(stringstream &lineStream);

    /**
     *
     *  Simulates at least one clock of the main GPU clock before returning.
     *
     *  Used by the integrated GPU simulator debugger run commands.
     *
     *  @param endOfBatch Reference to a boolean variable where to store if a draw call finished.
     *  @param endOfFrame Reference to a boolean variable where to store if a frame finished.
     *  @param endOfTrace Reference to a boolean variable where to store if the trace finished.
     *  @param gpuStalled Reference to a boolean variable where to store if a stall was detected.
     *  @param validationError Reference to a boolean variable where to store if a validation error was detected.
     *
     */

    void advanceTime(bool &endOfBatch, bool &endOfFrame, bool &endOfTrace, bool &gpuStalled, bool &validationError);

    /**
     *
     *  Creates a snapshot of the simulated state.
     *
     *  Creates a new directory for the snapshot.  Saves simulator state and configuration to files.
     *  Saves GPU registers to a file.  Saves the Hierarchical Z buffer to a file.  Saves the z/stencil and
     *  color cache block state buffer to files.  Saves GPU and system memory to files.
     *
     */

    static void createSnapshot();
    
    /**
     *
     *  Get current frame and draw call counters from the simulator.
     *
     *  @param frameCounter Reference to an integer variable where to store the current frame counter.
     *  @param frameBatch Reference to an integer variable where to store the current frame batch counter.
     *  @param batchCounter Reference to an integer variable where to store the current batch counter.
     *
     */
     
    void getCounters(u32bit &frameCounter, u32bit &frameBatch, u32bit &batchCounter);
    
    /**
     *
     *  Get the current simulation cycles.
     *
     *  @param gpuCycle Reference to an integer variable where to store the current GPU main clock cycle.
     *  @param shaderCycle Reference to an integer variable where to store the current shader domain clock cycle.
     *  @param memCycle Reference to an integer variable where to store the current memory domain clock cycle.
     *
     */
     
    void getCycles(u64bit &gpuCycle, u64bit &shaderCycle, u64bit &memCycle);

    /**
     *
     *  Aborts the simulation.
     *
     */
     
    void abortSimulation();
    
    /**
     *
     *  Dumps the content of the quad/latency map obtained from the ColorWrite boxes as a PPM image file.
     * 
     *  @param width Width of the quad/latency map in pixels (1 pixel -> 2x2 pixels in the framebuffer).
     *  @param width Height of the quad/latency map in pixels (1 pixel -> 2x2 pixels in the framebuffer).
     *
     */
     
    void dumpLatencyMap(u32bit width, u32bit height);
    
    /**
     *
     *  Converts a value to a color.
     *
     *  @param p Value to convert.
     *
     *  @return A 32-bit RGBA color.
     *
     */
     
    u32bit applyColorKey(u32bit p);

};  // class GPUSimulator


};  //  namespace gpu3d

#endif

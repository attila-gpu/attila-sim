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
 * Vector Shader Unit Fetch Box.
 *
 */

/**
 *  @file VectorShaderFetch.h
 *
 *  This file defines the VectorShaderFetch Box class.
 *
 *  This box implements the simulation of the Fetch stage in a GPU Shader unit.
 *
 */

#ifndef _VECTORSHADERFETCH_

#define _VECTORSHADERFETCH_

#include "support.h"
#include "GPUTypes.h"
#include "MultiClockBox.h"
#include "GPU.h"
#include "toolsQueue.h"
#include "ShaderCommon.h"
#include "ShaderEmulator.h"
#include "ShaderState.h"
#include "ShaderInput.h"
#include "ShaderCommand.h"
#include "ShaderDecodeCommand.h"
#include "ShaderExecInstruction.h"

namespace gpu3d
{

/***
 *
 *  This type stores the state of a vector shader thread.
 *
 */

struct VectorThreadState
{
    bool ready;                 /**<  Vector thread ready for execution.  */
    bool blocked;               /**<  Vector thread has been blocked.  */
    bool free;                  /**<  Vector thread is free (no data, no execution).  */
    bool end;                   /**<  The vector thread has finished the program (waiting for output read).  */
    bool zexported;             /**<  Vector thread has executed the z export instruction.  */
    u32bit PC;                  /**<  Thread PC (only for active threads).  */
    u32bit instructionCount;    /**<  Number of executed dynamic instructions for this vector thread.  */
    u32bit partition;           /**<  Shader partition/target to which the vector thread is assigned (vertex, fragment, triangle).  */
    ShaderInput **shInput;      /**<  Pointer to the array of shader inputs assigned to the vector thread.  */
    bool *traceElement;         /**<  Flag that stores if the execution of one element of the vector must be logged.  */
};

/**
 *
 *  VectorShaderFetch implements the Fetch stage of a Vector Shader.
 *
 *  Inherits from Box class.
 *
 */

class VectorShaderFetch : public MultiClockBox
{

private:

    //  Defines the number of shader attributes (same for input and output).
    static const u32bit SHADER_ATTRIBUTES = 48;

    //  Defines the number of shader partitions (per shader input type).
    static const u32bit SHADER_PARTITIONS = 4;

    //  Renames the shader partitions.
    static const u32bit PRIMARY_PARTITION = 0;
    static const u32bit SECONDARY_PARTITION = 1;

    //  Patched address for the triangle setup program.
    static const u32bit TRIANGLE_SETUP_PROGRAM_PC = 384;

    /*  Shader signals.  */
    Signal *commProcSignal[2];  /**<  Command signal from the Command Processor.  */
    Signal *commandSignal;      /**<  Command signal from Streamer.  */
    Signal *readySignal;        /**<  Ready signal to the Streamer.  */
    Signal *instructionSignal;  /**<  Instruction signal to Decode/Execute  */
    Signal *newPCSignal;        /**<  New PC signal from Decode/Execute.  */
    Signal *decodeStateSignal;  /**<  Decoder state signal from Decode/Execute.  */
    Signal *outputSignal;       /**<  Shader output signal to a consumer.  */
    Signal *consumerSignal;     /**<  Consumer readyness state to receive Shader output.  */

    //  Shader State.
    bool transInProgress;       /**<  Shader Output transmission in progress.  */
    u32bit transCycles;         /**<  Remaining Shader Output transmission cycles.  */
    
    ShaderState shState;        /**<  State of the Shader.  */

    ShaderDecodeState decoderState;     /**<  State of the decode stage.  */
    
    u32bit activeOutputs[SHADER_PARTITIONS];    /**<  Number shader output attributes active for the current shade program per shader target.  */
    u32bit activeInputs[SHADER_PARTITIONS];     /**<  Number of shader input attributes active for the current shader program per shader target .  */

    bool fetchedSIMD;           /**<  Stores if a SIMD instruction was fetched for the current thread.  */
    bool fetchedScalar;         /**<  Stores if a scalar instruction was fetched for the current thread.  */

    //  Shader structures.
    VectorThreadState *threadArray;         /**<  Array storing the state of the vector threads in the Shader.  */
    tools::Queue<u32bit> freeThreadFIFO;    /**<  Stores pointers to the free entries in the vector thread array.  */
    tools::Queue<u32bit> endThreadFIFO;     /**<  Stores pointers to threads finished in the vector thread array.  */
    u32bit inputThread;                     /**<  Stores the pointer to the current input thread in the vector thread array.  */
    u32bit nextInputElement;                /**<  Stores the index of the next element (shader input) in the input thread to load.  */
    u32bit outputThread;                    /**<  Stores the pointer to the current output thread in the vector thread array.  */
    u32bit nextOutputElement;               /**<  Stores the index of the next element (shader output) in the output thread to send back.  */
    u32bit outputResources;                 /**<  Number of resources to be liberated by the current output thread.  */
    u32bit currentActiveOutputs;            /**<  Number of active outputs for the current output thread.  */
    u32bit readyThreads;                    /**<  Counter that stores the number of runnable threads (ready state).  */
    u32bit blockedThreads;                  /**<  Counter that stores the number of blocked threads (blocked state).  */
    u32bit scanThread;                      /**<  Stores the pointer to the next vector thread to scan for ready threads.  */

    u32bit cyclesToNextFetch;               /**<  Counter that stores the number of cycles until the next fetch.  */
    u32bit cyclesBetweenFetches;            /**<  Stores the precomputed number of cycles to wait between two consecutive vector instruction fetches.  */

    //  Shader Z Export state.
    bool zExportInProgress;                 /**<  Stores if z exports of any vector thread are currently being sent.  */
    u32bit currentZExportedThread;          /**<  Stores the index of the current vector thread in the thread array for which z exports are currently being sent.  */
    u32bit nextZExportedElement;            /**<  Stores the index of the next element in the current vector thread whose z export is currently being sent.  */

    //  Resource counter.
    u32bit freeResources;                   /**<  Number of free thread resources.  */

    //  Shader instructions for a vector thread.
    ShaderExecInstruction ***vectorFetch;   /**<  Stores the fetched instructions for a vector thread (per element).  */

    //  Shader registers.
    u32bit startPC[MAX_SHADER_TARGETS];         /**<  Shader program start PC for each target/partition.  */
    u32bit resourceAlloc[MAX_SHADER_TARGETS];   /**<  Resources to allocate per thread for each target/partition.  */
    bool activeOutAttr[SHADER_PARTITIONS][SHADER_ATTRIBUTES];   /**<  Defines the active output attributes fir each target/partition.  Active outputs are sent to the shader output consumer.  */
    bool activeInAttr[SHADER_PARTITIONS][SHADER_ATTRIBUTES];    /**<  Defines the active input attributes for each target/partition.  Active inputs are received from the shader input producer.  */

    //  Shader derived pseudoconstants.
    u32bit maxThreadResources;  /**<  Maximum per thread resources of both partitions.  */
 
    //  Shader Parameters.
    ShaderEmulator &shEmul;     /**<  Reference to a ShaderEmulator object.  */
    u32bit numThreads;          /**<  Number of vector threads supported by the Shader  */
    u32bit numResources;        /**<  Number of vector thread resources (equivalent to temporal storage registers).  */
    u32bit vectorLength;        /**<  Elements (each element corresponds with a shader input) per vector.  */
    u32bit vectorALUWidth;      /**<  Number of ALUs in the vector ALU array.  */
    char* vectorALUConfig;      /**<  Configuration of the vector array ALUs.  */
    u32bit fetchDelay;          /**<  Number of cycles until an instruction from a vector thread can be fetched again.  */
    bool switchThreadOnBlock;   /**<  Flag that sets when the fetch thread is switched: after every fetch (false), or only after an instruction is blocked (true).  */
    u32bit textureUnits;        /**<  Number of texture units attached to the shader.  */
    u32bit inputCycle;          /**<  Shader inputs that can be received per cycle.  */
    u32bit outputCycle;         /**<  Shader outputs that can be sent per cycle.  */
    u32bit maxOutLatency;       /**<  Maximum latency of the output signal from the shader to the consumer.  */
    bool multisampling;         /**<  Flag that stores if MSAA is enabled.  */
    u32bit msaaSamples;         /**<  Number of MSAA Z samples generated per fragment when MSAA is enabled.  */

    //  Derived from the parameters.
    u32bit instrCycle;              /**<  Instructions fetched/decoded/executed per cycle.  */
    bool scalarALU;                 /**<  Architecture supports an extra scalar OP per cycle.  */
    bool soaALU;                    /**<  SOA architecture (scalar only vector ALU).  */

    //  Statistics.
    GPUStatistics::Statistic *fetched;          /**<  Number of fetched instructions.  */
    GPUStatistics::Statistic *reFetched;        /**<  Number of refetched instructions.  */
    GPUStatistics::Statistic *inputs;           /**<  Number of shader inputs.  */
    GPUStatistics::Statistic *outputs;          /**<  Number of shader outputs.  */
    GPUStatistics::Statistic *inputInAttribs;   /**<  Number of input attributes active per input.  */
    GPUStatistics::Statistic *inputOutAttribs;  /**<  Number of output attributes active per input.  */
    GPUStatistics::Statistic *inputRegisters;   /**<  Number of temporal registers used per input.  */
    GPUStatistics::Statistic *blocks;           /**<  Number of block command received.  */
    GPUStatistics::Statistic *unblocks;         /**<  Number of unblocking command received.  */
    GPUStatistics::Statistic *nThReady;         /**<  Number of ready threads.  */
    GPUStatistics::Statistic *nThReadyAvg;      /**<  Number of ready threads (average).  */
    GPUStatistics::NumericStatistic<u32bit> *nThReadyMax;  /**<  Number of ready threads (maximun).  */
    GPUStatistics::NumericStatistic<u32bit> *nThReadyMin;  /**<  Number of ready threads (minimun).  */
    GPUStatistics::Statistic *nThBlocked;       /**<  Number of blocked threads.  */
    GPUStatistics::Statistic *nThFinished;      /**<  Number of finished threads.  */
    GPUStatistics::Statistic *nThFree;          /**<  Number of free threads.  */
    GPUStatistics::Statistic *nResources;       /**<  Number of used resources.  */
    GPUStatistics::Statistic *emptyCycles;      /**<  Counts the cycles there is no work to be done.  */
    GPUStatistics::Statistic *fetchCycles;      /**<  Counts the cycles instructions are fetched.  */
    GPUStatistics::Statistic *noReadyCycles;    /**<  Counts the cycles there is no ready thread from which to fetch instructions.  */

    //  Debug/Log.
    bool traceVertex;       /**<  Flag that enables/disables a trace log for the defined vertex.  */
    u32bit watchIndex;      /**<  Defines the vertex index to trace log.  */
    bool traceFragment;     /**<  Flag that enables/disables a trace log for the defined fragment.  */
    u32bit watchFragmentX;  /**<  Defines the fragment x coordinate to trace log.  */
    u32bit watchFragmentY;  /**<  Defines the fragment y coordinate to trace log.  */
    
    //  Private functions.

    /**
     *
     *  Reserves a vector thread for the new set of shader inputs and allocates all the resources required by the
     *  vector thread.
     *
     *  @param cycle Current simulation cycle.
     *  @param partition The partition (vertex/fragment) for which the vector thread is reserved.
     *  @param newThread Reference to a variable where to return the reserved vector thread pointer.
     *
     */

    void reserveVectorThread(u64bit cycle, u32bit partition, u32bit &newThread);

    /**
     *
     *  Ends a vector thread and sets it as waiting for the shader outputs to be delivered to the consumer unit.
     *
     *  @param nThread The identifier of the vector thread that has ended.
     *
     */

    void endVectorThread(u32bit threadID);

    /**
     *
     *  Sets a vector thread as free and deallocate the resources used by the thread.
     *
     *  @param cycle Current simulation cycle.
     *  @param threadID Identifer of the vector thread.
     *  @param resources Number of resources to liberate for the thread.
     *
     */

    void freeVectorThread(u64bit cycle, u32bit threadID, u32bit resources);

    /**
     *
     *  Processes a Shader Command.
     *
     *  @param shComm The ShaderCommand to process.
     *  @param partition Identifies command for vertex or fragment partition in the unified shader model.
     *
     */

    void processShaderCommand(ShaderCommand *shComm, u32bit partition);

    /**
     *
     *  Processes a new Shader Input.
     *
     *  @param cycle Current simulation cycle.
     *  @param input The new shader input.
     *
     */

    void processShaderInput(u64bit cycle, ShaderInput *input);

    /**
     *
     *  Processes a control command from Decode stage.
     *
     *  @param decodeCommand A control command received from the decode stage.
     *
     */

    void processDecodeCommand(ShaderDecodeCommand *decodeCommand);

    /**
     *
     *  Fetches a shader instruction for a vector thread element from the Shader Emulator.
     *
     *  @param cycle Current simulation cycle.
     *  @param threadID Identifier of the vector thread.
     *  @param element Index to the element in the vector thread.
     *  @param instruction Instruction to fetch (multiple instructions per cycle), added to the current thread PC.
     *  @param shEmuElemID Identifier in the Shader Emulator of the vector thread element.
     *  @param shExecInstr Reference to a pointer where to store the dynamic shader instruction fetched.
     *
     */

    void fetchElementInstruction(u64bit cycle, u32bit threadID, u32bit element, u32bit shEmuElemID, u32bit instruction,
        ShaderExecInstruction *& shExecInstr);

    /**
     *
     *  Fetches the instructions for all the elements of a vector thread from the Shader Emulator.
     *
     *  @param cycle Current simulation cycle.
     *  @param threadID Identifier of the vector thread.
     *
     */

    void fetchVectorInstruction(u64bit cycle, u32bit threadID);

    /**
     *
     *  Sends vector thread instructions to the shader decode stage.
     *
     *  @param cycle Current simulation cycle.
     *  @param threadID Identifier of the vector thread.
     *
     */

    void sendVectorInstruction(u64bit cycle, u32bit threadID);

    /**
     *
     *  Receive new shader inputs from a producer unit.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void receiveInputs(u64bit cycle);


    /**
     *
     *  Recieve updates from the Command Processor.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void updatesFromCommandProcessor(u64bit cycle);

    /*
     *
     *  Receive state and updates from the decode stage.
     *
     *  @param cycle Current simulation cycle.
     *
     */
    
    void updatesFromDecodeStage(u64bit cycle);

    /*
     *
     *  Update fetch and shader input/output management statistics
     *
     */
     
    void updateFetchStatistics();
     
    /**
     *
     *  Output transmission stage.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void processOutputs(u64bit cycle);

    /**
     *
     *  Sends the shader output corresponding to a element of a vector thread thread to the shader consumer unit.
     *
     *  @param cycle Current simulation cycle.
     *  @param outputThread The vector thread for which to send the output to the consumer unit.
     *  @param element Index to the element in the vector thread to send to the consumer unit.
     *
     */

    void sendOutput(u64bit cycle, u32bit outputThread, u32bit element);

    /**
     *
     *  Simulates the fetch stage: updates fetch stage state and fetchs and issues instructions to the
     *  decode stage.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void fetchStage(u64bit cycle);
    
    /**
     *
     *  Computes and sends back preassure state to the consumer unit.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendBackPreassure(u64bit cycle);

    /*
     *  Simulate and update the state of the GPU domain section of the vector shader fetch box.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void updateGPUDomain(u64bit cycle);

    /*
     *  Simulate and update the state of the shader domain section of the vector shader fetch box.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void updateShaderDomain(u64bit cycle);     

    /**
     *
     *  Sends shader z exports of vector threads to the Shader Work Distributor unit.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void sendZExports(u64bit cycle);

    /**
     *
     *  Computes the maximum number of resources required for a new shader thread taking into account
     *  all the shader targets.
     *
     *  @return The maximum number of resources required fo a new shader thread.
     *
     */
     
    u32bit computeMaxThreadResources();

public:

    /**
     *
     *  Constructor for VectorShaderFetch.
     *
     *  Creates a new VectorShaderFetch object ready for start the simulation.
     *
     *  @param shEmu Reference to a ShaderEmulator object that will be used to emulate the Shader Unit.
     *  @param nThreads Number of vector threads in the Vector Shader Unit.
     *  @param resources Number of thread resources available (equivalent to registers).
     *  @param vectorLength Elements per vector (correspond to shader inputs).
     *  @param vectorALUWidth Number of ALUs in the vector ALU array.
     *  @param vectorALUConfig Configuration of the vector array ALUs.
     *  @param fetchDelay Cycles until a thread can be fetched again.
     *  @param switchThreadOnBlock Flag that sets if the fetch thread is switched after each instruction (false) or only when the thread becomes blocked.
     *  @param texUnits Number of texture units attached to the shader.
     *  @param inCycle Shader inputs received per cycle.
     *  @param outCycle Shader outputs sent per cycle.
     *  @param outLatency Maximum latency of the output signal to the consumer.
     *  @param microTrisAsFrag Process microtriangle fragment shader inputs and export z values (when MicroPolygon Rasterizer enabled only).
     *  @param name Name of the ShaderFetch box.
     *  @param shPrefix Primary shader prefix.   Used as fragment shader prefix (signals from the Command Processor).
     *  @param sh2Prefix Secondary shader prefix.  Used as vertex shader prefix (signasl from the Command Processor).
     *  @param parent Pointer to the parent box for this box.
     *
     *  @return A new VectorShaderFetch object initializated.
     *
     */

    VectorShaderFetch(ShaderEmulator &shEmu, u32bit nThreads, u32bit resources, u32bit vectorLength,
        u32bit vectorALUWidth, char *vectorALUConfig, u32bit fetchDelay, bool switchThreadOnBlock,
        u32bit texUnits, u32bit inCycle, u32bit outCycle, u32bit outLatency, bool microTrisAsFrag,
        char *name, char *shPrefix = 0, char *sh2Prefix = 0, Box* parent = 0);

    /**
     *
     *  Clock rutine.
     *
     *  Carries the simulation of the Shader Fetch.  It is called
     *  each clock to perform the simulation.
     *
     *  @param cycle  Cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Multi clock domain update rutine.
     *
     *  Updates the state of one of the clock domains implemented in the Vector Shader Fetch box.
     *
     *  @param domain Clock domain to update.
     *  @param cycle Cycle to simulate.
     *
     */

    void clock(u32bit domain, u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Shader Fetch box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(string &stateString);

    /**
     *
     *  Detects stall conditions in the Vector Shader Fetch box.
     *
     *  @param cycle Current simulation cycle.
     *  @param active Reference to a boolean variable where to return if the stall detection logic is enabled/implemented.
     *  @param stalled Reference to a boolean variable where to return if the Vector Shader Fetch has been detected to be stalled.
     *
     */
     
    void detectStall(u64bit cycle, bool &active, bool &stalled);

    /**
     *
     *  Writes into a string a report about the stall condition of the box.
     *
     *  @param cycle Current simulation cycle.
     *  @param stallReport Reference to a string where to store the stall state report for the box.
     *
     */
     
    void stallReport(u64bit cycle, string &stallReport);

    /**
     *
     *  Get the list of debug commands supported by the Vector Shader Fetch box.
     *
     *  @param commandList Reference to a string variable where to store the list debug commands supported
     *  by Vector Shader Fetch.
     *
     */
     
    void getCommandList(std::string &commandList);

    /**
     *
     *  Executes a debug command on the Vector Shader Fetch box.
     *
     *  @param commandStream A reference to a stringstream variable from where to read
     *  the debug command and arguments.
     *
     */    
     
    void execCommand(stringstream &commandStream);

    /**
     *  Displays the disassembled current shader program in the defined partition.
     *
     *  @param partition The partition for which to display the current shader program.
     *
     */
     
    void listProgram(u32bit partition);

};

} // namespace gpu3d

#endif

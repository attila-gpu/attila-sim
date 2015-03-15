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
 * $RCSfile: ShaderFetch.h,v $
 * $Revision: 1.20 $
 * $Author: vmoya $
 * $Date: 2008-01-03 09:56:19 $
 *
 * Shader Unit Fetch Box.
 *
 */

/**
 *  @file ShaderFetch.h
 *
 *  This file defines the ShaderFetch Box class (unified shader version).
 *
 *  This box implements the simulation of the Fetch stage in a
 *  GPU Shader unit.
 *
 */

#ifndef _SHADERFETCH_

#define _SHADERFETCH_

#include "support.h"
#include "GPUTypes.h"
#include "Box.h"
#include "GPU.h"
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
 *  This type stores information about a shader thread.
 *
 */

struct ThreadTable
{
    bool ready;         /**<  Thread ready for execution (active threads only).  */
    bool blocked;       /**<  Thread has been blocked.  */
    bool free;          /**<  Thread is free (no data, no execution).  */
    bool end;           /**<  The thread has finished the program (waiting for output read).  */
    bool repeat;        /**<  Next instruction is a repeated instruction.  */
    bool zexported;     /**<  Thread has executed the z export instruction.  */
    u32bit PC;          /**<  Thread PC (only for active threads).  */
    u32bit instructionCount;    /**<  Number of executed dynamic instructions for this thread.  */
    ShaderInput *shInput;       /**<  Pointer to the thread shader input.  */
    u64bit nextFetchCycle;      /**<  Marks the next cycle this thread will be allowed to be fetched.  */
};


/**
 *
 *  ShaderFetch implements the Fetch stage of a Shader.
 *
 *  Inherits from Box class.
 *
 */

class ShaderFetch : public Box
{

private:

    /*  Renames the shader partitions.  */
    static const u32bit PRIMARY_PARTITION = 0;
    static const u32bit SECONDARY_PARTITION = 1;

    /*  Patched address for the triangle setup program.  */
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
    Signal *shaderZExport;      /**<  Z export signal to the ShaderWorkDistributor (MicroPolygon Rasterizer).  */

    /*  Shader State.  */
    u32bit currentThread;       /**<  Pointer to the current thread from where to fetch an instruction.  */
    u32bit nextThread;          /**<  Pointer to the next thread from where to fetch an instruction.  */
    u32bit nextThreadInGroup;   /**<  Pointer to the next thread in a group from which to fetch instructions.  */
    bool readyThreadGroup;      /**<  Stores if there is a ready thread group.  */
    u32bit groupPC;             /**<  Keeps the PC for the current thread group.  */
    bool transInProgress;       /**<  Shader Output transmission in progress.  */
    u32bit transCycles;         /**<  Remaining Shader Output transmission cycles.  */
    u32bit currentOutput;       /**<  Number of shader outputs being currently transmited.  */
    ShaderState shState;        /**<  State of the Shader.  */
    u32bit activeOutputs[3];    /**<  Number outputs currently active (written and send back) per shader target.  */
    u32bit activeInputs[3];     /**<  Number of shader inputs active for the current shader program per shader target .  */
    bool fetchedSIMD;           /**<  Stores if a SIMD instruction was fetched for the current thread.  */
    bool fetchedScalar;         /**<  Stores if a scalar instruction was fetched for the current thread.  */

    /*  Shader structures.  */
    ThreadTable *threadTable;   /**<  Table with information about the state of the threads in the Shader.  */
    u32bit *freeBufferList;     /**<  List of free shader buffer/threads.  */
    u32bit numFreeThreads;      /**<  Number of free threads.  */
    u32bit numReadyThreads;     /**<  Number of ready threads.  */
    u32bit firstFilledBuffer;   /**<  Pointer to the entry in the free buffer list with the first (FIFO order) filled buffer.  */
    u32bit firstEmptyBuffer;    /**<  Pointer to the entry in the free buffer list with the first empty buffer.  */
    u32bit lastEmptyBuffer;     /**<  Pointer to the entry in the free buffer list with the last empty buffer.  */
    u32bit numFinishedThreads;  /**<  Number of threads that have already finished and are waiting for their output to be trasmited.  */
    u32bit *finThreadList;      /**<  List of finished threads.  */
    u32bit firstFinThread;      /**<  Pointer to the first (FIFO) finished thread in the finished thread list.  */
    u32bit lastFinThread;       /**<  Pointer to the last (FIFO) finished thread in the finished thread list.  */
    u32bit freeResources;       /**<  Number of free thread resources.  */

    /*  FIFO execution mode state variables.  */
    bool batchEnding[3];            /**<  Flag that stores if the current batch is just waiting for all the groups to finish.  */
    bool closedBatch[3][4];         /**<  Two flags store if the current and next shader input batch are closed and don't accepts new shader inputs.  */
    u32bit *batchGroupQueue[3][4];  /**<  Two queues storing the thread groups that form the current and next shader input batch.  */
    u32bit fetchBatch[3];           /**<  Pointers to the shader input batch from which instructions can be fetch.  */
    u32bit loadBatch[3];            /**<  Pointers to the shader input batch to which new groups are being loaded.  */
    u32bit nextFetchInBatch[3];     /**<  Pointers to the next fetch entry in the fetch batch queues.  */
    u32bit lastInBatch[3][2];       /**<  Pointers to the last group in the batch queues.  */
    u64bit lastLoadCycle[3];        /**<  Last cycle a group was loaded into a batch queue.  */
    u32bit batchPC[3];              /**<  Current PC of a shader input batch.  */
    u32bit currentBatchPartition;   /**<  Pointer to the shader input batch partition currently active.  */

    /*  Thread Window execution mode structures and state variables.  */
    u32bit *readyThreads;       /**<  Counter per thread group storing the number of ready threads in the group.  */
    u32bit *finThreads;         /**<  Counter per thread group storing the number of finished threads in the group.  */
    u32bit *readyGroups;        /**<  Queue of ready thread groups.  */
    bool *groupBlocked;         /**<  Flag storing if a group blocked and out of any of the group queues.  */
    u32bit numGroups;           /**<  Number of thread groups in the shader.  */
    u32bit numReadyGroups;      /**<  Number of ready groups.  */
    u32bit nextReadyGroup;      /**<  Pointer to the next ready group in the queue.  */
    u32bit nextFreeReady;       /**<  Pointer to the next free entry in the ready groups queue.  */
    u32bit nextGroup;           /**<  Stores the group from which instructions are being fetched.  */

    u32bit *fetchedGroups;      /**<  Queue with the thread groups that have been fetched in the last cycles.  */
    u32bit numFetchedGroups;    /**<  Number of fetched groups in the queue.  */
    u32bit nextFetchedGroup;    /**<  Pointer to the next fetched group.  */
    u32bit nextFreeFetched;     /**<  Pointer to the next free entry in the fetched group queue.  */

    /*  Shader Z Export state.  */
    u32bit pendingZExports;           /**<  Number of pending z exports to be sent.  */
    u32bit currentZExportThread;  /**<  Stores the index of the current thread being processed for z exports.  */

    /*  Shader instruction group buffer.  */
    ShaderExecInstruction ***fetchGroup;/**<  Stores the fetched instructions for a thread group.  */

    /*  Shader registers.  */
    u32bit initPC[MAX_SHADER_TARGETS];           /**<  Shader program initial PC per partition.  */
    u32bit threadResources[3];  /**<  Per thread resource usage per partition.  */
    bool output[3][MAX_VERTEX_ATTRIBUTES];  /**<  Which shader output registers (written outputs) are going to be sent back per target/partition.  */
    bool input[3][MAX_VERTEX_ATTRIBUTES];   /**<  Which shader input registers are enabled for the current program and target/partition.  */

    /*  Shader derived pseudoconstants.  */
    u32bit maxThreadResources;  /**<  Maximum per thread resources of both partitions.  */

    /*  Shader Parameters.  */
    ShaderEmulator &shEmul;     /**<  Reference to a ShaderEmulator object.  */
    bool unifiedModel;          /**<  Simulate an unified shader model.  */
    u32bit threadsCycle;        /**<  Threads from which to fetch instructions per cycle.  */
    u32bit instrCycle;          /**<  Number of instructions to fetch each cycle per thread. */
    u32bit threadGroup;         /**<  Number of threads that form a group.  */
    bool lockStep;              /**<  Enables/disables lock step execution mode where a thread group executes in lock step (same instructions fetched per cycle).  */
    bool scalarALU;             /**<  Enable SIMD + scalar fetch.  */
    u32bit numThreads;          /**<  Number of threads supported by the Shader  */
    u32bit numInputBuffers;     /**<  Number of Shader Input buffers.  */
    u32bit numBuffers;          /**<  Number of shader buffers (input buffers + thread state).  */
    u32bit numResources;        /**<  Number of thread resources (registers?).  */
    bool threadWindow;          /**<  Flag that enables to search the thread queue for ready threads as it was a window.  */
    u32bit fetchDelay;          /**<  Number of cycles until a thread can be fetched again.  */
    bool swapOnBlock;           /**<  Swap the current active thread on blocks only.  */
    u32bit textureUnits;        /**<  Number of texture units attached to the shader.  */
    u32bit inputCycle;          /**<  Shader inputs that can be received per cycle.  */
    u32bit outputCycle;         /**<  Shader outputs that can be sent per cycle.  */
    u32bit maxOutLatency;       /**<  Maximum latency of the output signal from the shader to the consumer.  */
    bool multisampling;         /**<  Flag that stores if MSAA is enabled.  */
    u32bit msaaSamples;         /**<  Number of MSAA Z samples generated per fragment when MSAA is enabled.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *fetched;      /**<  Number of fetched instructions.  */
    GPUStatistics::Statistic *reFetched;    /**<  Number of refetched instructions.  */
    GPUStatistics::Statistic *inputs;       /**<  Number of shader inputs.  */
    GPUStatistics::Statistic *outputs;      /**<  Number of shader outputs.  */
    GPUStatistics::Statistic *inputInAttribs;   /**<  Number of input attributes active per input.  */
    GPUStatistics::Statistic *inputOutAttribs;  /**<  Number of output attributes active per input.  */
    GPUStatistics::Statistic *inputRegisters;   /**<  Number of temporal registers used per input.  */
    GPUStatistics::Statistic *blocks;       /**<  Number of block command received.  */
    GPUStatistics::Statistic *unblocks;     /**<  Number of unblocking command received.  */
    GPUStatistics::Statistic *nThReady;     /**<  Number of ready threads.  */
    GPUStatistics::Statistic *nThReadyAvg;  /**<  Number of ready threads.  */
    GPUStatistics::NumericStatistic<u32bit> *nThReadyMax;  /**<  Number of ready threads.  */
    GPUStatistics::NumericStatistic<u32bit> *nThReadyMin;  /**<  Number of ready threads.  */
    GPUStatistics::Statistic *nThBlocked;   /**<  Number of blocked threads.  */
    GPUStatistics::Statistic *nThFinished;  /**<  Number of finished threads.  */
    GPUStatistics::Statistic *nThFree;      /**<  Number of free threads.  */
    GPUStatistics::Statistic *nResources;   /**<  Number of used resources.  */
    GPUStatistics::Statistic *emptyCycles;  /**<  Counts the cycles there is no work to be done.  */
    GPUStatistics::Statistic *fetchCycles;  /**<  Counts the cycles instructions are fetched.  */
    GPUStatistics::Statistic *noReadyCycles;/**<  Counts the cycles there is no ready thread from which to fetch instructions.  */

    /*  Private functions.  */

    /**
     *
     *  Returns the number of a free thread if available.
     *
     *  @param partition The partition (vertex/fragment) for which the new thread
     *  is allocated.
     *
     *  @return The identifier of a free thread or -1 if there is no
     *  free threads available.
     *
     */

    s32bit getFreeThread(u32bit partition);

    /**
     *
     *  Sets a thread free.
     *
     *  @param numThread Number of the thread to set free.
     *  @param partition The partition (vertex/fragment) for which the thread
     *  is deallocated.
     *
     */

    void addFreeThread(u32bit numThread, u32bit partition);

    /**
     *
     *  Reloads a finished thread that which output has been already
     *  transmitted with new input data.
     *
     *  @param cycle Current simulation cycle.
     *  @param nThread A thread that has finished its ouput transmission.
     *
     */

    void reloadThread(u64bit cycle, u32bit nThread);

    /**
     *  Finish a thread.
     *
     *  Marks a thread as finished and adds the thread to the list of
     *  finished threads.
     *
     *  @param nThread A thread that has finished.
     *
     */

    void finishThread(u32bit nThread);

    /**
     *
     *  Processes a Shader Command.
     *
     *  @param shComm The ShaderCommand to process.
     *  @param partition Identifies command for vertex or fragment partition in the
     *  unified shader model.
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
     *  Fetches a shader instruction for a thread.
     *
     *  @param cycle Current simulation cycle.
     *  @param threadID Thread from which to fetch the instruction.
     *  @param shExecInstr Reference to a pointer where to store the
     *  dynamic shader instruction fetched.
     *
     */

    void fetchInstruction(u64bit cycle, u32bit threadID, ShaderExecInstruction *& shExecInstr);

    /**
     *
     *  Fetches a shader instruction and sends it to decode stage.
     *
     *  @param cycle Current simulation cycle.
     *  @param threadID Thread from which to fetch the instruction.
     *
     */

    void fetchInstruction(u64bit cycle, u32bit threadID);

    /**
     *
     *  Fetches all the instructions for a shader group and fetch cycle.
     *
     *  @param cycle Current simulation cycle.
     *  @param threadID First thread in the group from which to fetch the instructions.
     *
     */

    void fetchInstructionGroup(u64bit cycle, u32bit threadID);

    /**
     *
     *  Sends a shader instruction it to the shader decode stage.
     *
     *  @param cycle Current simulation cycle.
     *  @param threadID Thread identifier of the instruction to send.
     *  @param shExecInstr Dynamic shader instruction to send to shader decode.
     *
     */

    void sendInstruction(u64bit cycle, u32bit threadID, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Sends the shader output for a thread to the shader consumer unit.
     *
     *  @param cycle Current simulation cycle.
     *  @param outputThread The thread for which to send the output to the consumer unit.
     *
     */

    void sendOutput(u64bit cycle, u32bit outputThread);


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
     *  Constructor for ShaderFetch.
     *
     *  Creates a new ShaderFetch object ready for start the simulation.
     *
     *  @param shEmu Reference to a ShaderEmulator object that will be
     *  used to emulate the Shader Unit.
     *  @param unified Simulate an unified shader model.
     *  @param nThreads  Number of threads in the Shader Unit.
     *  @param nInputBuffers  Number of Input Buffers in the Shader Unit.
     *  @param resources Number of thread resources (registers?) available.
     *  @param threadsPerCycle Number of threads from which to fetch instructions per cycle.
     *  @param instrPerCycle Number of instructions fetched per thread and cycle.
     *  @param threadsGroup Threads per group.
     *  @param lockStepMode If the lock step execution mode is enabled.
     *  @param scalarALU Enables to fetch one SIMD + scalar instruction per cycle.
     *  @param thWindow Enables to search ready threads between all the available threads.
     *  @param fetchDelay Cycles until a thread can be fetched again.
     *  @param swapOnBlock Swap the active thread only when it becomes blocked.
     *  @param texUnits Number of texture units attached to the shader.
     *  @param inCycle Shader inputs received per cycle.
     *  @param outCycle Shader outputs sent per cycle.
     *  @param outLatency Maximum latency of the output signal to the consumer.
     *  @param microTrisAsFrag Process microtriangle fragment shader inputs and export z values (when MicroPolygon Rasterizer enabled only).
     *  @param name Name of the ShaderFetch box.
     *  @param shPrefix Primary shader prefix.   For non unified model is the prefix for the
     *  shader fetch box.  Used as fragment shader prefix (signals from the Command Processor)
     *  for the unified shader model.
     *  @param sh2Prefix Primary shader prefix.  Not used for the non unified model.  For unified shader
     *  model is the virtual vertex shader prefix (signals from command processor).
     *  @param parent Pointer to the parent box for this box.
     *
     *  @return A new ShaderFetch object initializated.
     *
     */

    ShaderFetch(ShaderEmulator &shEmu, bool unified, u32bit nThreads, u32bit nInputBuffers, u32bit resources,
        u32bit threadsPerCycle, u32bit instrPerCycle, u32bit threadsGroup, bool lockStepMode, bool scalarALU,
        bool thWindow, u32bit fetchDelay, bool swapOnBlock, u32bit texUnits, u32bit inCycle, u32bit outCycle,
        u32bit outLatency, bool microTrisAsFrag, char *name, char *shPrefix = 0, char *sh2Prefix = 0,
        Box* parent = 0);

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
     *  Returns a single line string with state and debug information about the
     *  Shader Fetch box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(string &stateString);

};

} // namespace gpu3d

#endif

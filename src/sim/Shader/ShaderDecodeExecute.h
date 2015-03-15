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
 * $RCSfile: ShaderDecodeExecute.h,v $
 * $Revision: 1.16 $
 * $Author: vmoya $
 * $Date: 2006-11-05 21:20:12 $
 *
 * Shader Unit Decode and Execute Box (unified shader version).
 *
 */

/*
 *  VertexShaderDecodeExecute definitions.
 *
 */

#ifndef _SHADERDECODEXECUTE_

#define _SHADERDECODEXECUTE_

#include "Box.h"
#include "ShaderCommon.h"
#include "ShaderState.h"
#include "ShaderInstruction.h"
#include "ShaderArchitectureParameters.h"
#include "ShaderDecodeCommand.h"
#include "ShaderDecodeStateInfo.h"
#include "ShaderExecInstruction.h"
#include "ShaderCommand.h"
#include "ShaderEmulator.h"
#include "ShaderFetch.h"

namespace gpu3d
{

/**
 *
 *  Defines a structure that contains information about a shader thread
 *  in the decode and execution stages.
 *
 */

struct ThreadInfo
{
    bool tempBankDep[MAX_TEMP_BANK_REGS];       /**<  Stores pending writes in the temporal register bank.  */
    bool addrBankDep[MAX_ADDR_BANK_REGS];       /**<  Stores pending writes in the address register bank.  */
    bool outpBankDep[MAX_OUTP_BANK_REGS];       /**<  Stores pending writes in the output register bank.  */
    bool predBankDep[MAX_PRED_BANK_REGS];       /**<  Stores pending writes in the predicate register bank.  */
    u64bit tempBankWrite[MAX_TEMP_BANK_REGS];   /**<  Stores the cycle a temporal register will be written.  */
    u64bit addrBankWrite[MAX_ADDR_BANK_REGS];   /**<  Stores the cycle an address register will be written.  */
    u64bit outpBankWrite[MAX_OUTP_BANK_REGS];   /**<  Stores the cycle an output register will be written.  */
    u64bit predBankWrite[MAX_PRED_BANK_REGS];   /**<  Stores the cycle an predicate register will be written.  */
    u32bit pending;                             /**<  Stores the number of instructions that have not finished yet execution.  */
    bool ready;                                 /**<  Stores if the thread is active, not blocked by a previous instruction.  */
    bool waitTexture;                           /**<  Stores if the thread is waiting for the result of a texture instruction.  */
    bool waitRepeated;                          /**<  Stores if a thread must ignore any instruction until it receives an instruction marked as repeated.  */
    bool end;                                   /**<  Stores if the thread has received the END instruction and is waiting for the last instructions to end execution.  */
    bool zexport;                               /**<  Stores if the thread has executed the z exported instruction.  */
};

/**
 *
 *  This class implements the Box that simulates the Decode, Execute and
 *  WriteBack stages of a Shader pipeline (based in a stream programming
 *  model).
 *
 */

class ShaderDecodeExecute: public Box
{

private:

    /*  Renames the shader partitions.  */
    static const u32bit PRIMARY_PARTITION = 0;
    static const u32bit SECONDARY_PARTITION = 1;

    /*  Shader Decode Execute parameters.  */
    ShaderEmulator &shEmul;         /**<  Reference to a ShaderEmulator object that implements the emulation of the Shader Unit.  */
    bool unifiedModel;              /**<  Simulate an unified shader model.  */
    u32bit numThreads;              /**<  Number of thread runnable threads supported by the Shader Unit.  */
    u32bit threadsCycle;            /**<  Number of threads from which instructions can be started/decoded and executed per cycle.  */
    u32bit instrCycle;              /**<  Number of instructions per thread that can be decoded and executed per cycle.  */
    u32bit threadGroup;             /**<  Number of threads per group.  */
    bool scalarALU;                 /**<  Enables a scalar ALU.  */
    bool lockStep;                  /**<  Lock step execution mode (intrCycle threads executing the same instructions in lock step mode) flag.  */
    u32bit textureUnits;            /**<  Number of texture units attached to the shader.  */
    u32bit textureRequestRate;      /**<  Number of texture requests send to a texture unit per cycle.  */
    u32bit requestsPerTexUnit;      /**<  Number of texture request to send to each texture unit before skipping to the next one.  */
    u32bit instrBufferSize;         /**<  Number of entries in the instruction buffer, an entry stores threadsCycle * instrCycle instructions.  */

    u32bit groupsCycle;             /**<  Number of thread groups that can be decoded and executed per cycle.  */

    /*  Shader Decode Execute signals.  */
    Signal *commandSignal[2];       /**<  Command signal from the Command Processor.  */
    Signal *instructionSignal;      /**<  Input signal from the ShaderFetch box.  Carries new instructions.  */
    Signal *controlSignal;          /**<  Output signal to the ShaderFetch box.  Carries control flow changes.  */
    Signal *executionStartSignal;   /**<  Output signal to the ShaderDecodeExecute box.  Starts an instruction execution.  */
    Signal *executionEndSignal;     /**<  Input Signal from the ShadeDecodeExecute box.  End of an instruction execution.  */
    Signal *decodeStateSignal;      /**<  Decoder state signal to the ShaderFetch box.  */
    Signal **textRequestSignal;     /**<  Texture access request signal to the Texture Unit.  */
    Signal **textResultSignal;      /**<  Texture result signal from the Texture Unit.  */
    Signal **textUnitStateSignal;   /**<  State signal from the Texture Unit.  */

    /*  State of Decode Execute stages.  */
    ShaderState state;              /**<  Current state of the decode execute shader stages.  */
    ThreadInfo *threadInfo;         /**<  Table with information about the shader threads in the decode execute stage.  */
    u32bit regWrites[MAX_EXEC_LAT]; /**<  Stores the number of register writes for the next MAX_EXEC_LAT cycles.  */
    u32bit nextRegWrite;            /**<  Points to the entry in the register writes table for the current cycle.  */
    u32bit groupThread;             /**<  Counts the thread inside the current group.  */
    u32bit instrGroup;              /**<  Instructions to execute per cycle for the current thread group (determined by first thread).  */
    u32bit instrThread;             /**<  Current fetch instruction counter for the current thread.  */
    bool reserveSIMDALU;            /**<  Reserves (one ALU per supported thread rate) the SIMD ALU for an instruction to be initiated in the current cycle.  */
    bool reserveScalarALU;          /**<  Reserves (one ALU per supported thread rate) the scalar ALU for an instruction to be initiated in the current cycle.  */
    u32bit *textureTickets;         /**<  Number of texture tickets available that allow to send a texture request to the Texture Unit.  */
    u32bit nextRequestTU;           /**<  Pointer to the next texture unit where to send texture requests.  */
    u32bit tuRequests;              /**<  Number of texture requests send to the current texture unit.  */

    /*  Instruction buffer.  */
    ShaderExecInstruction ***instrBuffer;   /**<  Stores instructions received from fetch.  */
    u32bit nextInstrCycle;                  /**<  Pointer to the next entry in the instruction buffer for the current cycle.  */
    u32bit nextInstr;                       /**<  Pointer to the next entry in the instruction buffer.  */
    u32bit nextFreeInstrCycle;              /**<  Pointer to the next free entry in the instruction buffer for the current cycle.  */
    u32bit nextFreeInstr;                   /**<  Pointer to the next free entry in the instruction buffer.  */
    u32bit freeInstr;                       /**<  Free entries in the instruction buffer.  */

    /*  Aux structures.  */
    u32bit *textThreads;            /**<  Auxiliary array to retrieve the number of the threads that terminate a texture access.  */

    //  Helper classes.
    ShaderArchitectureParameters *shArchParams; /**<  Pointer to the Shader Architecture Parameters singleton.  */
    
    /*  Statistics.  */
    GPUStatistics::Statistic *execInstr;    /**<  Number of executed instructions.  */
    GPUStatistics::Statistic *blockedInstr; /**<  Number of instructions blocked at decode.  */
    GPUStatistics::Statistic *removedInstr; /**<  Number of instructions removed without being executed at decode.  */
    GPUStatistics::Statistic *fakedInstr;   /**<  Number of faked instructions ignored at decode.  */
    GPUStatistics::Statistic *blocks;       /**<  Block commands sent to Shader Fetch.  */
    GPUStatistics::Statistic *unblocks;     /**<  Unblock commands sent to Shader Fetch.  */
    GPUStatistics::Statistic *ends;         /**<  End commands sent to Shader Fetch.  */
    GPUStatistics::Statistic *replays;      /**<  Replay commands sent to Shader Fetch.  */
    GPUStatistics::Statistic *textures;     /**<  Texture accesses sent to Texture Unit.  */
    GPUStatistics::Statistic *zexports;     /**<  Number of executed Z export instructions.  */

    /**
     *
     *  Sends a replay last instruction command to Shader Fetch.
     *
     *  @param cycle  Current simulation cycle.
     *  @param numThread  Thread of the instruction.
     *  @param PC PC of the instruction.
     *  @param shExecInstr Pointer to the dynamicc instruction.
     *
     */

    void replayInstruction(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Send a block thread command to Shader Fetch.
     *
     *  @param cycle  Current simulation cycle.
     *  @param numThread  Thread of the instruction.
     *  @param PC PC of the instruction.
     *  @param shExecInstr Pointer to the dynamicc instruction.
     *
     */

    void blockThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Send an unblock thread command to Shader Fetch.
     *
     *  @param cycle  Current simulation cycle.
     *  @param numThread  Thread of the instruction.
     *  @param PC PC of the  instruction.
     *  @param shExecInstr Pointer to the dynamicc instruction.
     *
     */

    void unblockThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Send an end thread command to Shader Fetch.
     *
     *  @param cycle  Current simulation cycle.
     *  @param numThread  Thread of the instruction.
     *  @param PC PC of the instruction.
     *  @param shExecInstr Pointer to the dynamicc instruction.
     *
     */

    void endThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Sends a thread's z export executed command to Shader Fetch.
     *
     *  @param cycle Current simulation cycle.
     *  @param numThread  Thread of the instruction.
     *  @param PC PC of the instruction.
     *  @param shExecInstr Pointer to the dynamicc instruction.
     *
     */

    void zExportThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Processes a command from the Command Processor unit.
     *
     *  @param shComm Pointer to a Shader Command receives from the Command Processor.
     *
     */

    void processCommand(ShaderCommand *shCom);


    /**
     *
     *  Processes the end of the execution of shader instructions.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void endExecution(u64bit cycle);

    /**
     *
     *  Starts the execution of a shader instruction.
     *
     *  @param cycle The current simulation cycle.
     *  @param shExecInstr The shader instruction to execute.
     *
     */

    void startExecution(u64bit cycle, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Decodes a shader instruction.
     *
     *  Processes instructions that must be repeated.  Sends a repeat command
     *  to the shader fetch unit.
     *
     *  @param cycle The current simulation cycle.
     *  @param shExecInstr The shader instruction to repeat.
     *
     */

    void repeatInstruction(u64bit cycle, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Decodes a shader instruction.
     *
     *  Determines the dependences of the incoming shader instruction with
     *  the shader instructions being executed.  Blocks and ignores instructions
     *  with dependences, instructions received for a blocked thread or a thread
     *  that has already received the END instruction.
     *
     *  @param cycle The current simulation cycle.
     *  @param shExecInstr The shader instruction to decode.
     *  @param repeat Reference to a boolean variable where to store if the instruction
     *  must be repeated or blocked.
     *  @param block The thread/instruction remains blocked rather than wait for a repetition.
     *
     *  @return Returns if the shader instruction must be executed this cycle
     *  (TRUE: initiate instruction execution, FALSE: block or ignore instruction
     *  execution).
     *
     */

    bool decodeInstruction(u64bit cycle, ShaderExecInstruction *shExecInstr, bool &repeat,
        bool block);

    /**
     *
     *  Checks if the register bank write ports are available for the instruction.
     *
     *  @param cycle The current simulation cycle.
     *  @param shExecInstr The shader instruction to decode.
     *  @param repeat Reference to a boolean variable where to store if the instruction
     *  must be repeated.
     *
     *  @return Returns if the shader instruction has available register bank write ports
     *  and can be started in the current cycle.
     *
     */

    bool checkWritePorts(u64bit cycle, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Determines if there is a RAW dependence for a register.
     *
     *  @param numThread Thread identifier of the instruction for which to check RAW dependences.
     *  @param bank Bank of the register to check for RAW.
     *  @param reg Register to check for RAW.
     *
     *  @return TRUE if there is a RAW dependence for the specified register and thread
     *  with a previous instruction that has not finished its execution yet.  FALSE if there is
     *  no dependence.
     *
     */

    bool checkRAWDependence(u32bit numThread, Bank bank, u32bit reg);

    /**
     *
     *  Clears register write dependences.   Sets to false the dependence flag for
     *  the instruction result register after the instruction has finished execution
     *  or the instructions has been removed.
     *
     *  @param cycle The current simulation cycle.
     *  @param shInstr Instruction for which to clear write dependences.
     *  @param numThread Thread for which to clear the write dependences.
     *
     */

    void clearDependences(u64bit cycle, ShaderInstruction *shInstr, u32bit numThread);


public:

    /**
     *
     *  Constructor for the class.
     *
     *  Creates a new initializated ShaderDecodeExecute box that implements
     *  the simulation of the Decode/Read/Executed/WriteBack stages of a
     *  Shader Unit.
     *
     *  @param shEmul Reference to a ShaderEmulator object that emulates the Shader Unit.
     *  @param unified Simulate an unified shader model.
     *  @param numThreads  Number of runnable threads supported by the Shader Unit.
     *  @param threadsCycle Number of threads from which to receive instructions per cycle.
     *  @param instrCycle  Number of instructions that can be decoded and launched (start execution) per cycle and thread.
     *  @param groupThreads Number of threads per group.
     *  @param lockStepMode Enables or disables lock step execution mode where threads must execute the
     *  same instructions in lock step at the same rate.
     *  @param textUnits Number of texture units attached to the shader.
     *  @param txReqCycle Texture requests sent to a texture unit per cycle.
     *  @param txReqUnit Groups of consecutive texture requests send to a texture unit.
     *  @param scalarALU Enables a scalar ALU to operate in parallel with the SIMD ALU.
     *  @param name Name of the box.
     *  @param shPrefix String used to prefix the box signals names to differentiate between multiple
     *  instances of the ShaderDecodeExecute class.  In non unified shader model is the prefix used.  In
     *  unified shader model is the fragment partition prefix.
     *  @param shPrefix String used to prefix the box signals names to differentiate between multiple
     *  instances of the ShaderDecodeExecute class.  Not used for non unified shader model.  In unified
     *  shader model is the vertex partition prefix.
     *  @param parent  Pointer to the parent box.
     *  @return A new ShaderDecodeExecute object.
     *
     */

    ShaderDecodeExecute(ShaderEmulator &shEmul, bool unified, u32bit numThreads, u32bit threadCycle,
        u32bit instrCycle,  u32bit groupThreads, bool lockStepMode, u32bit textUnits, u32bit txReqCycle,
        u32bit txReqUnit, bool scalarALU, char *name, char *shPrefix, char *s2hPrefix, Box *parent);

    /**
     *
     *  Carries the simulation cycle a cycle of the Shader Decode/Execute box.
     *
     *  Simulates the behaviour of Shader Decode/Execute for a given cycle.
     *  Receives intructions that have finished their execution latency,
     *  executes them, updates dependencies and sends feedback to Shader Fetch.
     *  Receives instructions from Shader Fetch, checks dependencies, sends
     *  refetch commands, sends instructions to execute.
     *
     *  @param cycle  The cycle that is going to be simulated.
     *
     */

    void clock(u64bit cycle);
};

} // namespace gpu3d

#endif

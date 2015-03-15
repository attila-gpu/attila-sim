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
 * Vector Shader Unit Decode and Execute Box.
 *
 */

/**
 *  @file VectorShaderDecodeExecute.h
 *
 *  This file defines a VectorShaderDecodeExecute Box.
 *
 *  Defines and implements a simulator Box for the Decode,
 *  Execute and WriteBack stages of a Shader pipeline.
 *
 */

#ifndef _VECTORSHADERDECODEXECUTE_

#define _VECTORSHADERDECODEXECUTE_

#include "MultiClockBox.h"
#include "ShaderCommon.h"
#include "ShaderState.h"
#include "ShaderInstruction.h"
#include "ShaderDecodeCommand.h"
#include "ShaderDecodeStateInfo.h"
#include "ShaderExecInstruction.h"
#include "ShaderCommand.h"
#include "ShaderEmulator.h"
#include "VectorShaderFetch.h"
#include "toolsQueue.h"

namespace gpu3d
{

/**
 *
 *  Defines a structure that contains information about a shader thread
 *  in the decode and execution stages.
 *
 */

struct VectorThreadInfo
{
    bool tempBankDep[MAX_TEMP_BANK_REGS];       /**<  Stores pending writes in the temporary register bank.  */
    bool addrBankDep[MAX_ADDR_BANK_REGS];       /**<  Stores pending writes in the address register bank.  */
    bool outpBankDep[MAX_OUTP_BANK_REGS];       /**<  Stores pending writes in the output register bank.  */
    bool predBankDep[MAX_PRED_BANK_REGS];       /**<  Stores pending writes in the predicate register bank.  */
    u64bit tempBankWrite[MAX_TEMP_BANK_REGS];   /**<  Stores the cycle a temporary register will be written.  */
    u64bit addrBankWrite[MAX_ADDR_BANK_REGS];   /**<  Stores the cycle an address register will be written.  */
    u64bit outpBankWrite[MAX_OUTP_BANK_REGS];   /**<  Stores the cycle an output register will be written.  */
    u64bit predBankWrite[MAX_PRED_BANK_REGS];   /**<  Stores the cycle an predicate register will be written.  */
    u32bit pendingInstructions;                 /**<  Stores the number of instructions that have not finished yet execution.  */
    u32bit pendingTexElements;                  /**<  Counter that stores the number of texture fetches pending for elements in the vector thread.  */
    bool ready;                                 /**<  Stores if the thread is active, not blocked by a previous instruction.  */
    bool waitTexture;                           /**<  Stores if the thread is waiting for the result of a texture instruction.  */
    bool end;                                   /**<  Stores if the thread has received the END instruction and is waiting for the last instructions to end execution.  */
    bool pendingJump;                           /**<  Stores if the thread has received a JMP instruction and is waiting until it is executed.  */
    bool zexport;                               /**<  Stores if the thread has executed the z exported instruction.  */
};

/**
 *
 *  This class implements the Box that simulates the Decode, Execute and WriteBack stages of a Shader pipeline.
 *
 */

class VectorShaderDecodeExecute: public MultiClockBox
{

private:

    //  Renames the shader partitions.
    static const u32bit PRIMARY_PARTITION = 0;
    static const u32bit SECONDARY_PARTITION = 1;

    //  Shader Decode Execute parameters.
    ShaderEmulator &shEmul;         /**<  Reference to a ShaderEmulator object that implements the emulation of the Shader Unit.  */
    u32bit vectorThreads;           /**<  Number of vector threads supported by the Shader Unit.  */
    u32bit vectorLength;            /**<  Number of elements in a shader vector.  */
    u32bit vectorALUWidth;          /**<  Number of ALUs in the vector ALU array.  Each ALU executes the instruction for an element of the input vector.  */
    char* vectorALUConfig;          /**<  Configuration of the ALUs in the vector array.  */
    bool waitOnStall;               /**<  Flag that defines if when a vector shader instruction must be stalled the instruction waits in the decode stage or it's discarded and repeated (refetched).  */
    bool explicitBlock;             /**<  Flag that defines if threads are blocked by an explicit trigger in the instruction stream or automatically on texture load.  */
    u32bit textureClockRatio;       /**<  Ratio between texture unit clock and shader clock.  Used for the texture blocked thread wakeup queue.  */
    u32bit textureUnits;            /**<  Number of texture units attached to the shader.  */
    u32bit textureRequestRate;      /**<  Number of texture requests send to a texture unit per cycle.  */
    u32bit requestsPerTexUnit;      /**<  Number of texture request to send to each texture unit before skipping to the next one.  */
    u32bit instrBufferSize;         /**<  Number of entries in the instruction buffer, an entry stores threadsCycle * instrCycle instructions.  */

    //  Derived from the parameters.
    u32bit instrCycle;              /**<  Instructions fetched/decoded/executed per cycle.  */
    bool scalarALU;                 /**<  Architecture supports an extra scalar OP per cycle.  */
    bool soaALU;                    /**<  SOA architecture (scalar only vector ALU).  */
    u32bit execCyclesPerVector;     /**<  Number of execution iterations/cycles for a shader vector (vectorLength/vectorALUWidth).  */

    //  Shader Decode Execute signals.
    Signal *commandSignal[2];       /**<  Command signal from the Command Processor.  */
    Signal *instructionSignal;      /**<  Input signal from the ShaderFetch box.  Carries new instructions.  */
    Signal *controlSignal;          /**<  Output signal to the ShaderFetch box.  Carries control flow changes.  */
    Signal *executionStartSignal;   /**<  Output signal to the ShaderDecodeExecute box.  Starts an instruction execution.  */
    Signal *executionEndSignal;     /**<  Input Signal from the ShadeDecodeExecute box.  End of an instruction execution.  */
    Signal *decodeStateSignal;      /**<  Decoder state signal to the ShaderFetch box.  */
    Signal **textRequestSignal;     /**<  Texture access request signal to the Texture Unit.  */
    Signal **textResultSignal;      /**<  Texture result signal from the Texture Unit.  */
    Signal **textUnitStateSignal;   /**<  State signal from the Texture Unit.  */

    //  State of Decode Execute stages.
    ShaderState state;              /**<  Current state of the decode execute shader stages.  */
    VectorThreadInfo *threadInfo;   /**<  Table with information about the shader threads in the decode execute stage.  */
    u32bit regWrites[MAX_EXEC_LAT]; /**<  Stores the number of register writes for the next MAX_EXEC_LAT cycles.  */
    u32bit nextRegWrite;            /**<  Points to the entry in the register writes table for the current cycle.  */
    bool reserveSIMDALU;            /**<  Reserves (one ALU per supported thread rate) the SIMD ALU for an instruction to be initiated in the current cycle.  */
    bool reserveScalarALU;          /**<  Reserves (one ALU per supported thread rate) the scalar ALU for an instruction to be initiated in the current cycle.  */
    u32bit *textureTickets;         /**<  Number of texture tickets available that allow to send a texture request to the Texture Unit.  */
    u32bit nextRequestTU;           /**<  Pointer to the next texture unit where to send texture requests.  */
    u32bit tuRequests;              /**<  Number of texture requests send to the current texture unit.  */
    
    bool vectorFetchAvailable;      /**<  A vector fetch was received from the fetch stage and is available for decode/execution.  */
    u32bit execInstructions;        /**<  Stores the number of instructions that can be executed after performing the decode.  */
    bool executingVector;           /**<  Flag that stores if a vector instruction is on execution.  */
    u32bit execElement;             /**<  Index to the next element of the vector to be executed.  */
    u32bit endExecElement;          /**<  Index to the next element of the vector to end the execution.  */
    u32bit currentRepeatRate;       /**<  Stores the repeat rate (cycles between element execution start) for the current decoded instruction fetch.  */
    u32bit cyclesBetweenFetches;    /**<  Counter that stores the number of cycles expected between two vector fetches.  */
    u32bit cyclesToNextFetch;       /**<  Counter that stores the number of cycles until the next fetch is expected.  */
    u32bit cyclesToNextExec;        /**<  Counter that stores the number of cycles until the next execution of shader elements can be started.  */

    tools::Queue<u32bit> threadWakeUpQ;     /**<  Stores the identifier of threads pending from being awakened by Texture Unit.  */
    
    ShaderExecInstruction ***vectorFetch;   /**<  Array that stores a vector instruction fetch.  Includes the Shader Emulator instructions for all the elements in the vector.  */
    
    //  Aux structures.
    u32bit *textThreads;            /**<  Auxiliary array to retrieve the number of the threads that terminate a texture access.  */

    //  Helper classes.
    ShaderArchitectureParameters *shArchParams; /**<  Pointer to the Shader Architecture Parameters singleton.  */

    //  Statistics.
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
     *  Sends a new PC command to Shader Fetch.
     *
     *  @param cycle Current simulation cycle.
     *  @param numThread  Thread for which to change the PC.
     *  @param PC New PC for the thread.
     *  @param shExecInstr Pointer to the dynamic instruction.
     *
     */

    void changePC(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Sends a replay last instruction command to Shader Fetch.
     *
     *  @param cycle  Current simulation cycle.
     *  @param numThread  Thread of the instruction.
     *  @param PC PC of the instruction.
     *  @param shExecInstr Pointer to the dynamic instruction.
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
     *  @param shExecInstr Pointer to the dynamic instruction.
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
     *  @param shExecInstr Pointer to the dynamic instruction.
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
     *  @param shExecInstr Pointer to the dynamic instruction.
     *
     */

    void endThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Sends a thread's z export executed command to Shader Fetch.
     *
     *  @param cycle  Current simulation cycle.
     *  @param numThread  Thread of the instruction.
     *  @param PC PC of the instruction.
     *  @param shExecInstr Pointer to the dynamic instruction.
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
     *  Resets the vector shader decode and execute stages state.
     *
     *
     */
        
    void reset();

    /**
     *
     *  Receives a vector instruction fetch from the vector shader fetch stage.
     *
     *  @param cycle The current simulation cycle.
     *
     */

    void receiveVectorInstruction(u64bit cycle);

    /**
     *
     *  Updates the state at the decode state and decodes shader instructions.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void decodeStage(u64bit cycle);
    
    /**
     *
     *  Updates the state at the execute (start of the pipeline) stage and executes instructions.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void executeStage(u64bit cycle);
   

    /**
     *
     *  Updates the state at the write back/execution pipeline end stage.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void writeBackStage(u64bit cycle);

    /**
     *
     *  Send decode stage state to the fetch stage.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void sendDecodeState(u64bit cycle);
    
    /**
     *
     *  Sends requests to the texture units.
     *
     *  @param cycle Current simulation cycle.
     *
     */
    
    void sendRequestsToTextureUnits(u64bit cycle);
    
    /**
     *
     *  Receive results from the texture units.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void receiveResultsFromTextureUnits(u64bit cycle);

    /**
     *
     *  Receive state from the Texture units.
     *
     *  @param cycle Current simulation cycle.
     *
     */
    
    void receiveStateFromTextureUnits(u64bit cycle);    

    /**
     *
     *  Wake up threads pending from texture unit.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void wakeUpTextureThreads(u64bit cycle);

    /**
     *
     *  Receive updates from the Command Processor.
     *
     *  @param cycle Current simulation cycle.
     *
     */
    
    void updatesFromCommandProcessor(u64bit cycle);

    /**
     *
     *  Starts the execution of a shader instruction.
     *
     *  @param cycle The current simulation cycle.
     *  @param elem The element index in the vector being executed.
     *  @param shExecInstr The shader instruction to execute.
     *
     */

    void startExecution(u64bit cycle, u32bit instruction, ShaderExecInstruction *shExecInstr);

    /**
     *
     *  Decodes a shader instruction.
     *
     *  Processes instructions that must be repeated.  Sends a repeat command
     *  to the shader fetch unit.
     *
     *  @param cycle The current simulation cycle.
     *  @param instruction The instruction in the vector instruction fetch that is stalled (repeated).
     *
     */

    void repeatInstruction(u64bit cycle, u32bit instruction);

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
     *  @param instruction The instruction in the vector instruction fetch that is going to be decoded.
     *  @param exec Reference to a boolean variable where to store if the instruction can be executed.
     *  @param stall Reference to a boolean variable where to store if the instruction has to be stalled due to a dependence.
     *
     */

    void decodeInstruction(u64bit cycle, u32bit instruction, bool &exec, bool &stall);

    /**
     *
     *  Update decode stage structures (dependence checking, write port limitations).
     *
     *  @param cycle The current simulation cycle.
     *  @param shExecInstr The shader instruction for which to update the decode stage.
     *
     */
     
    void updateDecodeStage(u64bit cycle, ShaderExecInstruction *shExecInstr);

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


    /*
     *  Simulate and update the state of the GPU domain section of the vector shader decode execute box.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void updateGPUDomain(u64bit cycle);

    /*
     *  Simulate and update the state of the shader domain section of the vector shader decode execute box.
     *
     *  @param cycle Current simulation cycle.
     *
     */
     
    void updateShaderDomain(u64bit cycle);

    
    /**
     *
     *  Prints the values of the operands for the decoded shaded instruction.
     *  Should be called before the execution of the instruction.
     *
     *  @param shInstrDec Pointer to a ShaderInstructionDecoded object.
     *
     */
     
    void printShaderInstructionOperands(ShaderInstruction::ShaderInstructionDecoded *shInstrDec);
       
    /**
     *
     *  Prints the values of the results for the decoded shaded instruction.
     *  Should be called after the execution of the instruction.
     *
     *  @param shInstrDec Pointer to a ShaderInstructionDecoded object.
     *
     */
     
    void printShaderInstructionResult(ShaderInstruction::ShaderInstructionDecoded *shInstrDec);

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
     *  @param vectorThreads  Number of vector threads that can be on execution on the Vector Shader.
     *  @param vectorLength Number of elements in a shader vector.
     *  @param vectorALUWidth Number of ALUs in the vector ALU.  Each ALU executes instructions for a vector element.
     *  @param vectorALUConfig Configuration of the vector array ALUs.
     *  @param waitOnStall Flag that is used to determine if a vector shader instruction waits until the stall conditions
     *  are cleared at the decode stage or the instruction is discarded and a request is send to VectorShaderFetch to
     *  refetch (repeat) the instruction.
     *  @param explicitBlock Flag that defines if threads are blocked by an explicit trigger in the instruction stream or
     *  automatically on texture load.
     *  @param textClockRatio Texture Unit clock ratio with shader clock.
     *  @param textUnits Number of texture units attached to the shader.
     *  @param txReqCycle Texture requests sent to a texture unit per cycle.
     *  @param txReqUnit Groups of consecutive texture requests send to a texture unit.
     *  @param name Name of the box.
     *  @param shPrefix String used to prefix the fragment partition related box signals names to differentiate
     *  between multiple instances of the ShaderDecodeExecute class.
     *  @param sh2Prefix String used to prefix the vertex partition related box signals names to differentiate
     *  between multiple instances of the ShaderDecodeExecute class. 
     *  shader model is the vertex partition prefix.
     *  @param parent Pointer to the parent box.
     *  @return A new VectorShaderDecodeExecute object.
     *
     */

    VectorShaderDecodeExecute(ShaderEmulator &shEmul, u32bit vectorThreads, u32bit vectorLength, u32bit vectorALUWidth, char *vectorALUConfig,
        bool waitOnStall,  bool explicitBlock, u32bit textClockRatio, u32bit textUnits, u32bit txReqCycle, u32bit txReqUnit,
        char *name, char *shPrefix, char *sh2Prefix, Box *parent);

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

    /**
     *
     *  Carries the simulation cycle a cycle of the Shader Decode/Execute box.
     *  Multi clock domain version.
     *
     *  Simulates the behaviour of Shader Decode/Execute for a given cycle.
     *  Receives intructions that have finished their execution latency,
     *  executes them, updates dependencies and sends feedback to Shader Fetch.
     *  Receives instructions from Shader Fetch, checks dependencies, sends
     *  refetch commands, sends instructions to execute.
     *
     *  @param domain Clock domain to update.
     *  @param cycle The cycle that is going to be simulated.
     *
     */

    void clock(u32bit domain, u64bit cycle);
       
};

} // namespace gpu3d

#endif

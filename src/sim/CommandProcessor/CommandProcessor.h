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
 * $RCSfile: CommandProcessor.h,v $
 * $Revision: 1.29 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:46 $
 *
 * Command Processor class definition file.
 *
 */

/**
 *
 *  @file CommandProcessor.h
 *
 *  This file defines the Command Processor box.
 *
 *  The Command Processor box simulates the main control
 *  unit of a GPU.
 *
 */


#ifndef _COMMANDPROCESSOR_

#define _COMMANDPROCESSOR_


#include "GPUTypes.h"
#include "support.h"
#include "Box.h"
#include "GPU.h"
#include "ShaderFetch.h"
#include "Streamer.h"
#include "TraceDriverInterface.h"
#include "RasterizerStateInfo.h"

namespace gpu3d
{

/**
 *
 *  Command Processor class.
 *
 *  This class implements a simple GPU command processor for the
 *  Shader Simulator.  Gets command (vertex, program, param) from
 *  a file and sends it to one or more shader units.
 *
 */

class CommandProcessor: public Box
{

private:

    /**
     *
     *  Structure for storing buffered GPU register state changes.
     *
     */

    struct RegisterUpdate
    {
        GPURegister reg;    /**<  GPU register to be updated.  */
        u32bit subReg;      /**<  GPU register subregister identifier.  */
        GPURegData data;    /**<  Data to write in the register.  */
    };

    static const u32bit MAX_REGISTER_UPDATES = 512; /**<  Defines the maximum number of register updates to store.  */
    static const u32bit GEOM_REG = 0;   /**<  Defines identifier for geometry phase registers.  */
    static const u32bit FRAG_REG = 1;   /**<  Defines identifier for geometry phase registers.  */
    static const u32bit CONSECUTIVE_DRAW_DELAY = 2; /**<  Defines the number of cycles between consecutive draw calls.  */
    
    /*  Command Processor parameters.  */
    u32bit numVShaders;         /**<  Number of vertex shaders controlled by the Command Processor.  */
    u32bit numFShaders;         /**<  Number of fragment shader units controlled by the Command Processor.  */
    u32bit numTextureUnits;     /**<  Number of texture units.  */
    u32bit numStampUnits;       /**<  Number of stamp units (Z Stencil + Color Write) controlled by the Command Processor.  */
    u32bit memorySize;          /**<  Size of the GPU memory in bytes.  */
    bool pipelinedBatches;      /**<  Enables/disables pipelined rendering of batches.  */
    bool dumpShaderPrograms;    /**<  Dumps shaders loaded to a file.  */

    /*  Command Processor signals.  */
    Signal **vshFCommSignal;    /**<  Array of the Shader Command signals to the Vertex Shader (Fetch) Units.  */
    Signal **vshDCommSignal;    /**<  Array of the Shader Command signals to the Vertex Shader (Decode) Units.  */
    Signal *streamCtrlSignal;   /**<  Control signal to the Streamer.  */
    Signal *streamStateSignal;  /**<  State signal from the Streamer.  */
    Signal *paCommSignal;       /**<  Command signal to Primitive Assembly.  */
    Signal *paStateSignal;      /**<  State signal from Primitive Assembly.  */
    Signal *clipCommSignal;     /**<  Command signal to the Clipper.  */
    Signal *clipStateSignal;    /**<  State signal from the Clipper.  */
    Signal *rastCommSignal;     /**<  Command Processor Signal to the Rasterizer.  */
    Signal *rastStateSignal;    /**<  Rasterizer state signal to the Command Processor.  */
    Signal **fshFCommandSignal; /**<  Array of command signals to the Fragment Shader Units (Fetch).  */
    Signal **fshDCommandSignal; /**<  Array of command signals to the Fragment Shader Units (Decode).  */
    Signal **tuCommandSignal;   /**<  Array of command signals to the Texture Units.  */
    Signal **zStencilCommSignal;    /**<  Array of command signals to the Z Stencil Test unit.  */
    Signal **zStencilStateSignal;   /**<  Array of state signals from the Color Write unit.  */
    Signal **colorWriteCommSignal;  /**<  Array of command signals to the Color Write unit.  */
    Signal **colorWriteStateSignal; /**<  Array of state signals from the Color Write unit.  */
    Signal *dacCommSignal;      /**<  Command signal to the DAC unit.  */
    Signal *dacStateSignal;     /**<  State signal from the DAC unit.  */
    Signal *readMemorySignal;   /**<  Read signal from the GPU local memory controller.  */
    Signal *writeMemorySignal;  /**<  Write Signal to the GPU local memory controller.  */
    Signal* mcCommSignal; /** Command signal to the memory controller */

    /*  Command Processor state.  */
    GPUState state;             /**<  GPU state and registers.  */
    TraceDriverInterface *driver;        /**<  Pointer to the trace driver from where to get AGP Transactions.  */
    StreamerState streamState;  /**<  Current Streamer unit state.  */
    AGPTransaction *lastAGPTrans;   /**<  Pointer to the AGP Transaction being processed.  */
    u8bit *programCode;             /**<  Pointer to a buffer with the shader program being read.  */
    RasterizerState *zStencilState;     /**<  Array for storing the state of the Z Stencil units.  */
    RasterizerState *colorWriteState;   /**<  Array for storing the state of the Color Write units.  */
    GPUStatus stateStack;               /**<  Store the previous (stacked) GPU state.  For memory read/write pipelining support.  */
    bool processNewTransaction;         /**<  Flag storing if a new AGP transaction must be requested and processed.  */
    bool geometryStarted;               /**<  Flag storing if the geometry phase of a batch rendering has started.  */
    bool traceEnd;                      /**<  Determines if the trace has finished (no more AGP transactions available).  */
    AGPTransaction *loadFragProgram;    /**<  Buffers a load fragment program transaction.  */
    bool storedLoadFragProgram;         /**<  Flag storing if there is a load fragment program transaction buffered.  */
    AGPTransaction *backupAGPTrans;     /**<  Keeps the last AGP transaction while processing the buffered load fragment program transaction.  */
    bool backupProcNewTrans;            /**<  Keeps the last state of the process new transaction flag while processing the buffered load fragment program transaction.  */
    u32bit batch;                       /**<  Current batch.  */
    bool skipDraw;                      /**<  Flag that stores if the Command Processor must ignore the next DRAW commands.  */
    bool skipFrames;                    /**<  Flag that stores if the Command Processor must ignore GPU commands because it is skipping frames.  */
    bool forceTransaction;              /**<  Flag that stores if an external AGP transaction is forced into the GPU at the end of the current batch.  */
    AGPTransaction *forcedTransaction;  /**<  Stores the transaction that is forced on the GPU by an external agent.  */
    bool colorWriteEnd;                 /**<  END command already sent to color write (swap).  */
    bool zStencilTestEnd;               /**<  END command already sent to Z Stencil Test units (dump buffer).  */   
    RastComm dumpBufferCommand;         /**<  Stores the dump buffer command to issue to the DAC.  */
    bool swapReceived;                  /**<  Stores if a swap command has been received in the last cycle.  */
    bool batchEnd;                      /**<  Flag that stores if the batch has finished in the last cycle.  */
    bool initEnd;                       /**<  Flag that stores if the initialization end transaction has been received.  */
    bool commandEnd;                    /**<  Flag that stores if the last GPU command finished in the last cycle.  */
    bool forcedCommand;                 /**<  Flag that stores if a forced command is being executed.  */
    u32bit flushDelayCycles;            /**<  Cycles to wait for HZ updates to finish.  */
    u64bit lastEventCycle[GPU_NUM_EVENTS];  /**<  Stores the last cycle events were signaled.  */
    u32bit drawCommandDelay;            /**<  Cycles to wait between consecutive draw commands.  */

    u32bit vshProgID;                   /**<  Vertex program identifier. */
    u32bit fshProgID;                   /**<  Fragment program identifier.  */
    u32bit shProgID;                    /**<  Shader program identifier.  */
     
    RegisterUpdate updateBuffer[2][MAX_REGISTER_UPDATES];   /**<  Buffers GPU register writes (for geometry and fragment registers).  */
    u32bit freeUpdates[2];      /**<  Number of free entries in the register update buffer.  */
    u32bit regUpdates[2];       /**<  Number of register updates stored in the update buffer.  */
    u32bit nextUpdate[2];       /**<  Pointer to the next register update in the update buffer.  */
    u32bit nextFreeUpdate[2];   /**<  Pointer to the next free entry in the register update buffer.  */

    /*  Memory access state.  */
    MemState memoryState;       /**<  Stores current memory state.  */
    u32bit transCycles;         /**<  Stores the remaining cycles for the end of the current AGP Transaction.  */
    u32bit currentTicket;       /**<  Current ticket/id for the memory transactions.  */
    u32bit freeTickets;         /**<  Number of free tickets.  */
    u32bit requested;           /**<  Bytes requested to the Memory Controller.  */
    u32bit received;            /**<  Bytes received from the Memory Controller.  */
    u32bit sent;                /**<  Bytes sent to the Memory Controller.  */
    u32bit lastSize;            /**<  Size of the last memory transaction (read) received.  */

    /*  Command processor statistics.  */
    GPUStatistics::Statistic &regWrites;    /**<  Number of register writes.  */
    GPUStatistics::Statistic &bytesWritten; /**<  Bytes written to memory.  */
    GPUStatistics::Statistic &bytesRead;    /**<  Bytes read from memory.  */
    GPUStatistics::Statistic &writeTrans;   /**<  Write transactions.  */
    GPUStatistics::Statistic &batches;      /**<  Number of batches processed.  */
    GPUStatistics::Statistic &frames;       /**<  Number of frames processed (SWAP commands).  */
    GPUStatistics::Statistic &bitBlits;     /**<  Number of Bit blit operations. */
    GPUStatistics::Statistic &readyCycles;  /**<  Cycles in ready state.  */
    GPUStatistics::Statistic &drawCycles;   /**<  Cycles in draw state.  */
    GPUStatistics::Statistic &endGeomCycles;/**<  Cycles in end geometry state.  */
    GPUStatistics::Statistic &endFragCycles;/**<  Cycles in end fragment state.  */
    GPUStatistics::Statistic &clearCycles;  /**<  Cycles in clear state.  */
    GPUStatistics::Statistic &swapCycles;   /**<  Cycles in swap state.  */
    GPUStatistics::Statistic &flushCycles;   /**<  Cycles in swap state.  */
    GPUStatistics::Statistic &saveRestoreStateCycles;   /**<  Cycles in save/restore state for z/color state.  */
    GPUStatistics::Statistic &bitBlitCycles;   /**<  Cycles in framebuffer blitting state. */
    GPUStatistics::Statistic &memReadCycles;    /**<  Cycles in memory read state.  */
    GPUStatistics::Statistic &memWriteCycles;   /**<  Cycles in memory write state.  */
    GPUStatistics::Statistic &memPreLoadCycles; /**<  Cycles in memory preload state.  */

    //  Debug/Validation.
    bool enableValidation;                          /**<  Enables the validation mode.  */
    u32bit nextReadLog;                             /**<  Pointer to the next log where to read from.  */
    u32bit nextWriteLog;                            /**<  Pointer to the next log where to write from.  */
    vector<AGPTransaction*> agpTransactionLog[4];   /**<  Logs the AGP transactions received.  */
    
    
    /*  Private functions.  */

    /**
     *
     *  Starts the processing of a new AGP transaction.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void processNewAGPTransaction(u64bit cycle);

    /**
     *  Continues the processing of the last AGP transaction.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void processLastAGPTransaction(u64bit cycle);

    /**
     *
     *  Processes an AGP_REG_WRITE transaction.
     *
     *  @param cycle Current simulation cycle.
     *  @param gpuReg GPU register to write.
     *  @param gpuSubReg GPU subregister to write.
     *  @param gpuData Data to write to the GPU register.
     *
     */

    void processGPURegisterWrite(u64bit cycle, GPURegister gpuReg, u32bit gpuSubReg, GPURegData gpuData);

    /**
     *
     *  Processes an AGP_REG_READ transaction.
     *
     *  @param gpuReg The GPU register to read from.
     *  @param gpuSubReg The GPU subregister to read from.
     *  @param gpuData A reference to variable where to store
     *  the data read from the GPU register.
     *
     */

    void processGPURegisterRead(GPURegister gpuReg, u32bit gpuSubReg, GPURegData &gpuData);

    /**
     *
     *  Processes a AGP_COMMAND transaction.
     *
     *  @param cycle Current simulation cycle.
     *  @param gpuComm The GPU command to execute.
     *
     */

    void processGPUCommand(u64bit cycle, GPUCommand gpuComm);

    /**
     *
     *  Processes a AGP_EVENT transaction.
     *
     *  @param cycle Current simulation cycle.
     *  @param gpuEvent The event signaled to the GPU.
     *  @param eventMsg A string with a message associated with the event.
     *
     */
     
    void processGPUEvent(u64bit cycle, GPUEvent gpuEvent, std::string eventMsg);
         
    /**
     *
     *  Processes a Memory Transaction from the Memory Controller.
     *
     *  Checks the Memory Controller state and accepts requested
     *  data from the Memory Controller.
     *
     *  @param memTrans The Memory Transaction to process.
     *
     */

    void processMemoryTransaction(MemoryTransaction *memTrans);

public:

    /**
     *
     *  Command Processor constructor.
     *
     *  This function initializes a CommandProcessor object.  Creates and initializes the
     *  Command Processor internal state.  Creates the signals to the shaders.
     *
     *  @param driver Pointer to the Trace Driver from where to get AGP Transactions.
     *  @param numVShaders Number of Vertex Shader Units controlled by the command processor.
     *  @param vshPrefixArray An array of strings with the prefix for each of the numVShader Vertex shader signals.
     *  @param numFShaders Number of Fragment Shader Units controlled by the Command Processor.
     *  @param fshPrefixArray Array of strings with the prefix of each Fragment Shader.
     *  @param numTextureUnits Number of texture units.
     *  @param tuPrefixArray Array of strings with the prefix of each Texture Unit.
     *  @param nStampUnits Number of stamp units (Z Stencil Test + Color Write) controlled by the Command Processor.
     *  @param suPrefixes Array of strings with the prefixes for each stamp unit.
     *  @param memorySize Size of the GPU memory in bytes.
     *  @param pipelinedBatches Enables/Disables pipelined rendering of batches.
     *  @param dumpShaders Enables/Disables the dumping of the shader programs being loaded to files.
     *  @param name Name of the Command Processor Box.
     *  @param parent Pointer to a parent box.
     *  @return An initialized Command Processor.
     *
     */

    CommandProcessor(TraceDriverInterface *driver, u32bit numVShaders, char **vshPrefixArray,
        u32bit numFShader, char **fshPrefixArray, u32bit numTextureUnits, char **tuPrefixArray,
        u32bit nStampUnits, char **suPrefixes,
        u32bit memorySize, bool pipelinedBatches, bool dumpShaders, char *name, Box *parent = 0);


    /**
     *
     *  Performs the simulation of the Command Processor.
     *
     *  This function implements the simulation of the Command Processor
     *  for the given time cycle.
     *
     *  @param cycle Cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Resets the GPU state.
     *
     */
     
    void reset();
    
    /**
     *
     *  Returns if the simulation of the trace has finished.
     *
     *  @return TRUE if all the trace has been simulated, FALSE otherwise.
     *
     */

    bool isEndOfTrace();

    /**
     *
     *  Returns if a swap command has been received.  Only valid for the
     *  cycle the command has been received.
     *
     *  @return TRUE if a swap command has been recieved in the last simulated
     *  cycle.
     *
     */

     bool isSwap();

    /**
     *
     *  Returns if the current batch has finished.  Only valid for the cycle the
     *  batch ends (END_FRAGMENT -> READY transition).
     *
     *  @return TRUE if the current batch has finished.
     *
     */

    bool endOfBatch();

    /**
     *
     *  Sets the skip draw command flag that enables/disables the execution of draw and swap commands
     *  by the Command Processor.
     *
     *  @param skip Boolean value to be assigned to the skip draw command flag.
     *
     */

    void setSkipDraw(bool skip);

    /**
     *
     *  Sets the skip frames command flag that enables/disables the execution of GPU commands while frames
     *  are being skipped.
     *
     *  @param skip Boolean value to be assigned to the skip frames command flag.
     *
     */

    void setSkipFrames(bool skip);

    /**
     *
     *  Returns a single line string with basic state and debug information about
     *  the Command Processor.
     *
     *  @param stateString A reference to a string where to store the state line.
     *
     */

    void getState(string &stateString);

    /** 
     *
     *  Returns a list of the debug commands supported by the Command Processor.
     *
     *  @param commandList Reference to a string where to store the list of debug commands supported by 
     *  the Command Processor.
     *
     */

    void getCommandList(string &commandList);

    /** 
     *
     *  Executes a debug command.
     *
     *  @param commandStream A string stream with the command and parameters to execute.
     *
     */

    void execCommand(stringstream &commandStream);


    /**
     *
     *  Returns if the an end of initialization phase transaction has been received.
     *
     *  @return TRUE if the end of initialization transaction has been received.
     *
     */
     
    bool endOfInitialization();

    /**
     *
     *  Returns if in the last cycle a GPU command finished it's execution.
     *
     *  @return TRUE a command finished in the last cycle.
     *
     */
     
    bool endOfCommand();

    /**
     *
     *  Returns if in the last cycle a forced GPU command finished it's execution.
     *
     *  @return TRUE a forced command finished in the last cycle.
     *
     */
     
    bool endOfForcedCommand();

    /**
     *
     *  Saves a snapshot of the current values of the GPU registers stored in the
     *  Command Processor to a file.
     *
     */
     
    void saveRegisters();
    
    /**
     *
     *  Get the AGP Transaction log.
     *
     *  @return A reference to the AGP Transaction log.
     *
     */
     
    vector<AGPTransaction*> &getAGPTransactionLog();

    /**
     *
     *  Enable/disable the validation mode in the Command Processor.
     *
     *  @param enable Boolean value that defines if the validation mode in the Command Processor is set to enabled or disabled.
     *
     */
     
    void setValidationMode(bool enable);
        
};

} // namespace gpu3d

#endif

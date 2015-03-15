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
 * $RCSfile: ZStencilTestV2.h,v $
 * $Revision: 1.7 $
 * $Author: vmoya $
 * $Date: 2007-11-18 20:13:56 $
 *
 * Z Stencil test box definition file.
 *
 */

/**
 *
 *  @file ZStencilTestV2.h
 *
 *  This file defines the Z and Stencil Test box.
 *
 *  This class implements the Z and stencil tests stages of the GPU pipeline.
 *
 */


#ifndef _ZSTENCILTESTV2_

#define _ZSTENCILTESTV2_

#include "GPUTypes.h"
#include "support.h"
#include "Box.h"
#include "ZCacheV2.h"
#include "RasterizerCommand.h"
#include "RasterizerState.h"
#include "GPU.h"
#include "FragmentInput.h"
#include "FragmentOpEmulator.h"
#include "RasterizerEmulator.h"
#include "GenericROP.h"
#include "ValidationInfo.h"

namespace gpu3d
{

/**
 *
 *  This class implements the simulation of the Z and Stencil
 *  test stages of a GPU pipeline.
 *
 *  This class inherits from the Generic ROP class that implements a generic
 *  ROP pipeline
 *
 */

class ZStencilTestV2 : public GenericROP
{
private:

    //  Z Stencil Test signals.
    Signal *hzUpdate;           /**<  Update signal to the Hierarchical Z box.  */
    Signal *blockStateDAC;      /**<  Signal to send the color buffer block state to the DAC.  */

    //  Z Stencil test registers.
    u32bit clearDepth;                  /**<  Current clear depth value.  */
    u32bit depthPrecission;             /**<  Depth bit precission.  */
    u8bit clearStencil;                 /**<  Current clear stencil value.  */
    bool modifyDepth;                   /**<  Flag that stores if the fragment shader has modified the fragment depth component.  */
    bool zTest;                         /**<  Flag storing if Z test is enabled.  */
    CompareMode depthFunction;          /**<  Depth test comparation function.  */
    bool depthMask;                     /**<  If depth buffer update is enabled or disabled.  */
    bool stencilTest;                   /**<  Flag storing if Stencil test is enabled.  */
    CompareMode stencilFunction;        /**<  Stencil test comparation function.  */
    u8bit stencilReference;             /**<  Stencil reference value for the test.  */
    u8bit stencilTestMask;              /**<  Stencil mask for the stencil operands test.  */
    u8bit stencilUpdateMask;            /**<  Stencil mask for the stencil update.  */
    StencilUpdateFunction stencilFail;  /**<  Update function when stencil test fails.  */
    StencilUpdateFunction depthFail;    /**<  Update function when depth test fails.  */
    StencilUpdateFunction depthPass;    /**<  Update function when depth test passes.  */
    u32bit zBuffer;                     /**<  Address in the GPU memory of the current Z buffer.  */

    //  Z Stencil parameters.
    u32bit zCacheWays;          /**<  Z cache set associativity.  */
    u32bit zCacheLines;         /**<  Number of lines in the Z cache per way.  */
    u32bit stampsLine;          /**<  Number of stamps per Z cache line.  */
    u32bit cachePortWidth;      /**<  Width of the Z cache ports in bytes.  */
    bool extraReadPort;         /**<  Add an additional read port to the color cache.  */
    bool extraWritePort;        /**<  Add an additional write port to the color cache.  */
    u32bit zQueueSize;          /**<  Z queue size.  */
    bool disableZCompression;   /**<  Disables Z compression (and HZ update!).  */
    bool disableHZUpdate;       /**<  Disables HZ Update.  */
    u32bit hzUpdateLatency;     /**<  Latency of the hierarchizal Z update signal.  */
    u32bit blockStateLatency;   /**<  Latency of the block state update signal to the DAC.  */
    u32bit numStampUnits;       /**<  Number of stamp units in the GPU pipeline.  */
    u32bit blocksCycle;         /**<  State of z stencil block states copied to the DAC per cycle.  */

    //  Z Write state.
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */

    //  Z cache.
    ZCacheV2 *zCache;           /**<  Pointer to the Z cache.  */
    u32bit bytesLine;           /**<  Bytes per Z cache line.  */
    u32bit lineShift;           /**<  Logical shift for a Z cache line.  */
    u32bit lineMask;            /**<  Logical mask for a Z cache line.  */

    //  Block state copy state.
    u32bit copyStateCycles;     /**<  Number of cycles remaining for the copy of the block state memory.  */

    //  Rasterizer Emulator associated with the stage.
    RasterizerEmulator &rastEmu;/**<  Reference to the rasterizer emulator objecter.  */

    GPUStatistics::Statistic *outputs;      /**<  Output fragments.  */
    GPUStatistics::Statistic *tested;       /**<  Tested fragments.  */
    GPUStatistics::Statistic *failed;       /**<  Fragments that failed the tests.  */
    GPUStatistics::Statistic *passed;       /**<  Fragments that passed the tests.  */
    GPUStatistics::Statistic *updatesHZ;    /**<  Updates to Hierarchical Z.  */

    //  Debug/validation.
    bool validationMode;                    /**<  Flag that stores if validation mode is enabled.  */
    FragmentQuadMemoryUpdateMap zstencilMemoryUpdateMap;    /**<  Map indexed by fragment identifier storing updates to the z stencil buffer.  */

    //  Debug/Log.
    bool traceFragment;     /**<  Flag that enables/disables a trace log for the defined fragment.  */
    u32bit watchFragmentX;  /**<  Defines the fragment x coordinate to trace log.  */
    u32bit watchFragmentY;  /**<  Defines the fragment y coordinate to trace log.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param command The rasterizer command to process.
     *  @param cycle Current simulation cycle.
     *
     */

    void processCommand(RasterizerCommand *command, u64bit cycle);

    /**
     *
     *  Processes a register write.
     *
     *  @param reg The Interpolator register to write.
     *  @param subreg The register subregister to writo to.
     *  @param data The data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);


    /**
     *
     *  Performs any remaining tasks after the stamp data has been written.
     *  The function should read, process and remove the stamp at the head of the
     *  written stamp queue.
     *
     *  @param cycle Current simulation cycle.
     *  @param stamp Pointer to a stamp that has been written to the ROP data buffer
     *  and needs processing.
     *
     */

    void postWriteProcess(u64bit cycle, ROPQueue *stamp);


    /**
     *
     *  To be called after calling the update/clock function of the ROP Cache.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void postCacheUpdate(u64bit cycle);

    /**
     *
     *  This function is called when the ROP stage is in the the reset state.
     *
     */

    void reset();

    /**
     *
     *  This function is called when the ROP stage is in the flush state.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void flush(u64bit cycle);
    
    /**
     *
     *  This function is called when the ROP stage is in the swap state.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void swap(u64bit cycle);

    /**
     *
     *  This function is called when the ROP stage is in the clear state.
     *
     */

    void clear();

    /**
     *
     *  This function is called when a stamp is received at the end of the ROP operation
     *  latency signal and before it is queued in the operated stamp queue.
     *
     *  @param cycle Current simulation cycle.
     *  @param stamp Pointer to the Z queue entry for the stamp that has to be operated.
     *
     */

    void operateStamp(u64bit cycle, ROPQueue *stamp);

    /**
     *
     *  This function is called when all the stamps but the last one have been processed.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void endBatch(u64bit cycle);

public:

    /**
     *
     *  Z Stencil Test box constructor.
     *
     *  Creates and initializes a new Z Stencil Test box object.
     *
     *  @param stampsCycle Number of stamps per cycle.
     *  @param overW Over scan tile width in scan tiles (may become a register!).
     *  @param overH Over scan tile height in scan tiles (may become a register!).
     *  @param scanW Scan tile width in fragments.
     *  @param scanH Scan tile height in fragments.
     *  @param genW Generation tile width in fragments.
     *  @param genH Generation tile height in fragments.
     *  @param disableZCompr Disables Z compression (and HZ update!).
     *  @param cacheWays Z cache set associativity.
     *  @param cacheLines Number of lines in the Z cache per way/way.
     *  @param stampsLine Numer of stamps per Z cache line (less than a tile!).
     *  @param portWidth Width of the Z cache ports in bytes.
     *  @param extraReadPort Adds an extra read port to the color cache.
     *  @param extraWritePort Adds an extra write port to the color cache.
     *  @param cacheReqQueueSize Size of the Z cache memory request queue.
     *  @param inputRequests Number of read requests and input buffers in the Z cache.
     *  @param outputRequests Number of write requests and output buffers in the Z cache.
     *  @param numStampUnits Number of stamp units in the GPU.
     *  @param maxBlocks Maximum number of sopported Z blocks in the Z cache state memory.
     *  @param blocksCycle Number of state block entries that can be cleared per cycle.
     *  @param compCycles Z block compression cycles.
     *  @param decompCycles Z block decompression cycles.
     *  @param zInQSz Z input stamp queue size (entries/stamps).
     *  @param zFetchQSz Z fetched stamp queue size (entries/stamps).
     *  @param zReadQSz Z read stamp queue size (entries/stamps).
     *  @param zOpQSz Z operated stamp queue size (entries/stamps).
     *  @param writeOpQSz Z written stamp queue size (entries/stamps).
     *  @param zTestRate Rate at which stamp are tested (cycles between two stamps to be tested).
     *  @param zTestLatency Z and stencil test latency.
     *  @param disableHZUpdate Disables HZ update.
     *  @param hzUpdateLatency Latency to send updates to Hierarchical Z buffer.
     *  @param blockStateLatency Latency to send the z stencil block state to the DAC.
     *  @param frEmu Reference to the Fragment Operation Emulator object.
     *  @param rastEmu Reference to the Rasterizer Emulator object to be used by the box.
     *  @param name The box name.
     *  @param prefix String used to prefix the box signals names.
     *  @param parent The box parent box.
     *
     *  @return A new Z Stencil Test object.
     *
     */

    ZStencilTestV2(u32bit stampsCycle, u32bit overW, u32bit overH, u32bit scanW, u32bit scanH,
        u32bit genW, u32bit genH,
        bool disableZCompr, u32bit cacheWays, u32bit cacheLines, u32bit stampsLine, u32bit portWidth,
        bool extraReadPort, bool extraWritePort, u32bit cacheReqQueueSize, u32bit inputRequests,
        u32bit outputRequests, u32bit numStampUnits, u32bit maxBlocks, u32bit blocksCycle,
        u32bit compCycles, u32bit decompCycles,
        u32bit zInQSz, u32bit zFetchQSz, u32bit zReadQSz, u32bit zOpQSz, u32bit zWriteQSz,
        u32bit zTestRate, u32bit zTestLatency, bool disableHZUpdate, u32bit hzUpdateLatency,
        u32bit blockStateLatency,
        FragmentOpEmulator &frOp, RasterizerEmulator &rastEmu,
        char *name, char *prefix = 0, Box* parent = 0);

    /**
     *
     *  Saves the block state memory into a file.
     *
     */
     
    void saveBlockStateMemory();
    
    /**
     *
     *  Loads the block state memory from a file.
     *
     */
     
    void loadBlockStateMemory();

    /**
     *
     *  Get the list of debug commands supported by the Color Write box.
     *
     *  @param commandList Reference to a string variable where to store the list debug commands supported
     *  by the Color Write box.
     *
     */
     
    void getCommandList(std::string &commandList);

    /**
     *
     *  Executes a debug command on the Color Write box.
     *
     *  @param commandStream A reference to a stringstream variable from where to read
     *  the debug command and arguments.
     *
     */    
     
    void execCommand(stringstream &commandStream);

    /** 
     *
     *  Set Z Stencil Test unit validation mode.
     *
     *  @param enable Boolean value that defines if the validation mode is enabled.
     *
     */
     
    void setValidationMode(bool enable);

    /**
     *
     *  Get the z stencil buffer memory update map for the current draw call.
     *
     *  @return A reference to the z stencil buffer memory update map for the current draw call.
     *
     */
     
    FragmentQuadMemoryUpdateMap &getZStencilUpdateMap();
    
};

} // namespace gpu3d

#endif


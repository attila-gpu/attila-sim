/**************************************************************************
 *
 * Copyright (c) 2002 - 2004 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: ZStencilTest.h,v $
 * $Revision: 1.12 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:47 $
 *
 * Z Stencil test box definition file.
 *
 */

/**
 *
 *  @file ZStencilTest.h
 *
 *  This file defines the Z and Stencil Test box.
 *
 *  This class implements the Z and stencil tests
 *  stages of the GPU pipeline.
 *
 */


#ifndef _ZSTENCILTEST_

#define _ZSTENCILTEST_

#include "GPUTypes.h"
#include "support.h"
#include "Box.h"
#include "Signal.h"
#include "ZCache.h"
#include "RasterizerCommand.h"
#include "RasterizerState.h"
#include "GPU.h"
#include "FragmentInput.h"
#include "FragmentOpEmulator.h"
#include "RasterizerEmulator.h"

namespace gpu3d
{

/**
 *
 *  Defines a Z Queue entry.
 *
 *  A Z queue entry stores fragments and z values
 *  for a stamp being tested.
 *
 */


struct ZQueue
{
    u32bit address;     /**<  Adddres in the Z buffer where to store the stamp fragments.  */
    u32bit way;         /**<  Way of the color cache where to store the stamp.  */
    u32bit line;        /**<  Line of the color cache where to store the stamp.  */
    u32bit offset;      /**<  Offset inside the color cache line where to store the stamp.  */
    bool lastStamp;     /**<  Stores if the stored stamp is marked as last stamp.  */
    u32bit *inDepth;    /**<  Pointer to an unsigned integer array where to store the stamp Z values.  */
    u32bit *zData;      /**<  Z and stencil data for the stamp for/from the Z buffer.  */
    bool *culled;       /**<  Pointer to a boolean array where to store the stamp culled bits.  */
    FragmentInput **stamp;      /**<  Pointer to the fragments in the stamp.  */
};


/**
 *
 *  Defines the state reported by the Z Stencil Test unit to the Fragment FIFO.
 *
 */

enum ZStencilTestState
{
    ZST_READY,      /**<  Z Stencil test can receive stamps.  */
    ZST_BUSY        /**<  Z Stencil test can not receive stamps.  */
};

/**
 *
 *  This class implements the simulation of the Z and Stencil
 *  test stages of a GPU pipeline.
 *
 *  This class inherits from the Box class that offers basic
 *  simulation support.
 *
 */

class ZStencilTest : public Box
{
private:

    /**<  Defines where the tested fragments must be sent.  */
    enum
    {
        COLOR_WRITE = 0,        /*  Early Z disabled, fragments are sent to Color Write.  */
        FRAGMENT_FIFO = 1       /*  Early Z enabled, fragments are sent to Fragment FIFO.  */
    };

    /*  Z Stencil Test signals.  */
    Signal *rastCommand;        /**<  Command signal from the Rasterizer main box.  */
    Signal *rastState;          /**<  State signal to the Rasterizer main box.  */
    Signal *fragments;          /**<  Fragment signal from the Fragment FIFO box.  */
    Signal *zStencilState;      /**<  State signal to the Fragment FIFO box.  */
    Signal *colorWrite[2];      /**<  Stamp output signal to the Color Write or Fragment FIFO box.  */
    Signal *colorWriteState[2]; /**<  State signal from the Color Write or Fragment FIFO box.  */
    Signal *memRequest;         /**<  Memory request signal to the Memory Controller.  */
    Signal *memData;            /**<  Memory data signal from the Memory Controller.  */
    Signal *testStart;          /**<  Signal that simulates the start of the test operation latency.  */
    Signal *testEnd;            /**<  Signal that simulates the end of the test operation.  */
    Signal *hzUpdate;           /**<  Update signal to the Hierarchical Z box.  */

    /*  Z Stencil test registers.  */
    u32bit hRes;                /**<  Display horizontal resolution.  */
    u32bit vRes;                /**<  Display vertical resolution.  */
    s32bit startX;              /**<  Viewport initial x coordinate.  */
    s32bit startY;              /**<  Viewport initial y coordinate.  */
    u32bit width;               /**<  Viewport width.  */
    u32bit height;              /**<  Viewport height.  */
    u32bit clearDepth;          /**<  Current clear depth value.  */
    u32bit depthPrecission;     /**<  Depth bit precission.  */
    u8bit clearStencil;         /**<  Current clear stencil value.  */
    bool earlyZ;                /**<  Flag that enables or disables early Z testing (Z Stencil before shading).  */
    bool modifyDepth;           /**<  Flag that stores if the fragment shader has modified the fragment depth component.  */
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
    u32bit zBuffer;             /**<  Address in the GPU memory of the current Z buffer.  */

    /*  Color Write parameters.  */
    u32bit stampsCycle;         /**<  Number of stamps that can be received per cycle.  */
    u32bit overW;               /**<  Over scan tile width in scan tiles.  */
    u32bit overH;               /**<  Over scan tile height in scan tiles.  */
    u32bit scanW;               /**<  Scan tile width in generation tiles.  */
    u32bit scanH;               /**<  Scan tile height in generation tiles.  */
    u32bit genW;                /**<  Generation tile width in stamps.  */
    u32bit genH;                /**<  Generation tile height in stamps.  */
    u32bit bytesPixel;          /**<  Depth bytes per pixel.  */
    u32bit zCacheWays;          /**<  Z cache set associativity.  */
    u32bit zCacheLines;         /**<  Number of lines in the Z cache per way/way.  */
    u32bit stampsLine;          /**<  Number of stamps per Z cache line.  */
    u32bit cachePortWidth;      /**<  Width of the Z cache ports in bytes.  */
    bool extraReadPort;         /**<  Add an additional read port to the color cache.  */
    bool extraWritePort;        /**<  Add an additional write port to the color cache.  */
    u32bit zQueueSize;          /**<  Z queue size.  */
    bool disableZCompression;   /**<  Disables Z compression (and HZ update!).  */
    u32bit testRate;            /**<  Rate at which stamp are tested (cycles between two stamp to be tested).  */
    u32bit testLatency;         /**<  Z and Stencil test latency.  */
    bool disableHZUpdate;       /**<  Disables HZ Update.  */
    u32bit hzUpdateLatency;     /**<  Latency of the hierarchizal Z update signal.  */
    FragmentOpEmulator &frEmu;  /**<  Reference to the fragment operation emulator object.  */
    RasterizerEmulator &rastEmu;/**<  Reference to the rasterizer emulator objecter.  */

    /*  Z Write state.  */
    RasterizerState state;      /**<  Current box state.  */
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */
    u32bit currentTriangle;     /**<  Identifier of the current triangle being processed (used to count triangles).  */
    u32bit triangleCounter;     /**<  Number of processed triangles.  */
    u32bit fragmentCounter;     /**<  Number of fragments processed in the current batch.  */
    u32bit frameCounter;        /**<  Counts the number of rendered frames.  */
    bool lastFragment;          /**<  Last batch fragment flag.  */
    bool receivedFragment;      /**<  If a fragment has been received in the current cycle.  */
    MemState memoryState;       /**<  Current memory controller state.  */
    bool endFlush;              /**<  Flag that signals the end of the flush of the Z cache.  */
    u32bit fragmentDestination; /**<  Stores which is the current destination of stamps (FragmentFIFO or Color Write).  */
    u32bit testCycles;          /**<  Cycles until the next stamp can be tested.  */

    /*  Z cache.  */
    ZCache *zCache;             /**<  Pointer to the Z cache.  */
    u32bit bytesLine;           /**<  Bytes per Z cache line.  */
    u32bit lineShift;           /**<  Logical shift for a Z cache line.  */
    u32bit lineMask;            /**<  Logical mask for a Z cache line.  */
    u32bit stampMask;           /**<  Logical mask for the offset of a stamp inside a Z cache line.  */

    /*  Z Queue structures.  */
    ZQueue *zQueue;         /**<  The blend queue.  Stores stamps to be blended or written.  */
    u32bit fetchZ;          /**<  Entry of the Z queue for which to fetch Z and stencil data.  */
    u32bit readZ;           /**<  Entry of the Z queue for which to read Z data.  */
    u32bit writeZ;          /**<  Entry of the Z queue for which to write Z and stencil data.  */
    u32bit colorZ;          /**<  Entry of the Z queue to be sent next to Color Write.  */
    u32bit freeZ;           /**<  Next free entry in the Z queue.  */
    u32bit testStamps;      /**<  Stamps stored in the Z queue.  */
    u32bit fetchStamps;     /**<  Stamps waiting in the Z queue to fetch Z data.  */
    u32bit readStamps;      /**<  Stamps waiting in the Z queue to read Z data.  */
    u32bit writeStamps;     /**<  Stamps waiting in the Z queue to write Z data.  */
    u32bit colorStamps;     /**<  Stamps waiting in the Z queue to be sent to Color Write.  */
    u32bit freeStamps;      /**<  Free entries in the Z queue.  */

    /*  Configurable buffers.  */
    FragmentInput **stamp;      /**<  Stores last receveived stamp.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *inputs;       /**<  Input fragments.  */
    GPUStatistics::Statistic *outputs;      /**<  Output fragments.  */
    GPUStatistics::Statistic *tested;       /**<  Tested fragments.  */
    GPUStatistics::Statistic *failed;       /**<  Fragments that failed the tests.  */
    GPUStatistics::Statistic *passed;       /**<  Fragments that passed the tests.  */
    GPUStatistics::Statistic *outside;      /**<  Outside triangle/viewport fragments.  */
    GPUStatistics::Statistic *culled;       /**<  Culled for non test related reasons.  */
    GPUStatistics::Statistic *readTrans;    /**<  Read transactions to memory.  */
    GPUStatistics::Statistic *writeTrans;   /**<  Write transactions to memory.  */
    GPUStatistics::Statistic *readBytes;    /**<  Bytes read from memory.  */
    GPUStatistics::Statistic *writeBytes;   /**<  Bytes written to memory.  */
    GPUStatistics::Statistic *fetchOK;      /**<  Succesful fetch operations.  */
    GPUStatistics::Statistic *fetchFail;    /**<  Failed fetch operations.  */
    GPUStatistics::Statistic *readOK;       /**<  Sucessful read operations.  */
    GPUStatistics::Statistic *readFail;     /**<  Failed read operations.  */
    GPUStatistics::Statistic *writeOK;      /**<  Sucessful write operations.  */
    GPUStatistics::Statistic *writeFail;    /**<  Failed write operations.  */
    GPUStatistics::Statistic *updatesHZ;    /**<  Updates to Hierarchical Z.  */
    GPUStatistics::Statistic *rawDep;       /**<  Blocked read accesses because of read after write dependence between stamps.  */

    /*  Z and stencil Clear state.  */
    //u8bit clearBuffer[MAX_TRANSACTION_SIZE];    /**<  Clear buffer, stores the clear values for a full transaction.  */
    //u32bit clearAddress;        /**<  Current color clear address.  */
    //u32bit endClearAddress;     /**<  End of the memory region to clear.  */
    //u32bit busCycles;           /**<  Remaining memory bus cycles.  */
    //u32bit ticket;              /**<  Memory ticket.  */
    //u32bit freeTickets;         /**<  Number of free memory tickets.  */

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
     *  Processes a received stamp.
     *
     */

    void processStamp();

    /**
     *
     *  Processes a memory transaction.
     *
     *  @param cycle The current simulation cycle.
     *  @param memTrans The memory transaction to process
     *
     */

    void processMemoryTransaction(u64bit cycle, MemoryTransaction *memTrans);


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
     *  @param bytesPixel Number of bytes to be used per pixel (should be a register!).
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
     *  @param maxBlocks Maximum number of sopported Z blocks in the Z cache state memory.
     *  @param blocksCycle Number of state block entries that can be cleared per cycle.
     *  @param compCycles Z block compression cycles.
     *  @param decompCycles Z block decompression cycles.
     *  @param zQueueSize Z queue size (in entries/stamps).
     *  @param zTestRate Rate at which stamp are tested (cycles between two stamps to be tested).
     *  @param zTestLatency Z and stencil test latency.
     *  @param disableHZUpdate Disables HZ update.
     *  @param hzUpdateLatency Latency to send updates to Hierarchical Z buffer.
     *  @param frEmu Reference to the Fragment Operation Emulator object.
     *  @param rastEmu Reference to the Rasterizer Emulator object to be used by the box.
     *  @param name The box name.
     *  @param prefix String used to prefix the box signals names.
     *  @param parent The box parent box.
     *
     *  @return A new Z Stencil Test object.
     *
     */

    ZStencilTest(u32bit stampsCycle, u32bit overW, u32bit overH, u32bit scanW, u32bit scanH,
        u32bit genW, u32bit genH, u32bit bytesPixel,
        bool disableZCompr, u32bit cacheWays, u32bit cacheLines, u32bit stampsLine, u32bit portWidth,
        bool extraReadPort, bool extraWritePort, u32bit cacheReqQueueSize, u32bit inputRequests,
        u32bit outputRequests, u32bit maxBlocks, u32bit blocksCycle,
        u32bit compCycles, u32bit decompCycles,
        u32bit zQueueSize, u32bit zTestRate, u32bit zTestLatency, bool disableHZUpdate, u32bit hzUpdateLatency,
        FragmentOpEmulator &frOp, RasterizerEmulator &rastEmu,
        char *name, char *prefix = 0, Box* parent = 0);

    /**
     *
     *  Z Stencil Test simulation function.
     *
     *  Simulates a cycle of the Z Stencil Test box.
     *
     *  @param cycle The cycle to simulate of the Z Stencil Test box.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Z Stencil Test box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(string &stateString);
};

} // namespace gpu3d

#endif


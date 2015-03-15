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
 * $RCSfile: ColorWrite.h,v $
 * $Revision: 1.13 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:45 $
 *
 * Color Write box definition file.
 *
 */

/**
 *
 *  @file ColorWrite.h
 *
 *  This file defines the Color Write box.
 *
 *  This class implements the final color blend and color write
 *  stages of the GPU pipeline.
 *
 */


#ifndef _COLORWRITE_

#define _COLORWRITE_

#include "GPUTypes.h"
#include "support.h"
#include "Box.h"
#include "Signal.h"
#include "ColorCache.h"
#include "RasterizerCommand.h"
#include "RasterizerState.h"
#include "GPU.h"
#include "FragmentInput.h"
#include "FragmentOpEmulator.h"

namespace gpu3d
{

/**
 *
 *  Defines a Blend Queue entry.
 *
 *  A blend queue entry stores the colors of an incoming stamp
 *  of fragments (2x2) while waiting for the color of the
 *  pixels to be written (blended) by those fragments to be
 *  read from the color buffer (from the color cache or video
 *  memory).
 *
 *  NOTE:  The number of bytes per color should change
 *  to support other color buffer formats (float point?).
 *
 */


struct BlendQueue
{
    u32bit x;           /**<  First stamp fragment x coordinate.  */
    u32bit y;           /**<  First stamp fragment y coordinate.  */
    u64bit startCycle;  /**<  Stores the stamp start cycle (same for all fragments).  */
    u32bit shaderLatency;   /**<  Latency of the samp in the shader units (same for all fragments).  */
    u32bit address;     /**<  Adddres in the color buffer where to store the stamp fragments.  */
    u32bit way;         /**<  Way of the color cache where to store the stamp.  */
    u32bit line;        /**<  Line of the color cache where to store the stamp.  */
    u32bit offset;      /**<  Offset inside the color cache line where to store the stamp.  */
    QuadFloat *inColor; /**<  Pointer to a QuadFloat array where to store the stamp colors.  */
    u8bit *outColor;    /**<  Pointer to a byte array for the final stamp colors.  */
    QuadFloat *destColorF;  /**<  Pointer to a QuadFloat array with the destination colors.  */
    u8bit *destColor;   /**<  Pointer to a byte array where to store the destination color.  */
    bool *mask;         /**<  Stores the write mask for the stamp. */
    DynamicObject *stampCookies;    /**<  Stores the stamp cookies.  */
};

/**
 *
 *  Defines the fragment map modes.
 *
 */
enum FragmentMapMode
{
    COLOR_MAP = 0,
    OVERDRAW_MAP = 1,
    FRAGMENT_LATENCY_MAP = 2,
    SHADER_LATENCY_MAP = 3
};

/**
 *
 *  Defines the state reported by the color write unit to the Z test box.
 *
 */

enum ColorWriteState
{
    CWS_READY,      /**<  Color Write can receive stamps.  */
    CWS_BUSY        /**<  Color Write can not receive stamps.  */
};

/**
 *
 *  This class implements the simulation of the Color Blend,
 *  Logical Operation and Color Write stages of a GPU pipeline.
 *
 *  This class inherits from the Box class that offers basic
 *  simulation support.
 *
 */

class ColorWrite : public Box
{
private:

    /**<  Defines from where the fragments are received.  */
    enum
    {
        Z_STENCIL_TEST = 0,     /*  Early Z disabled, fragments come from Z Stencil Test.  */
        FRAGMENT_FIFO = 1       /*  Early Z enabled, fragments come from Fragment FIFO.  */
    };

    /*  Color Write signals.  */
    Signal *rastCommand;        /**<  Command signal from the Rasterizer main box.  */
    Signal *rastState;          /**<  State signal to the Rasterizer main box.  */
    Signal *fragments[2];       /**<  Fragment signal from Z Stencil Test and Fragment FIFO.  */
    Signal *colorWriteState[2]; /**<  State signal to Z Stencil Test and Fragment FIFO.  */
    Signal *memRequest;         /**<  Memory request signal to the Memory Controller.  */
    Signal *memData;            /**<  Memory data signal from the Memory Controller.  */
    Signal *blendStart;         /**<  Signal that simulates the start of the blend operation latency.  */
    Signal *blendEnd;           /**<  Signal that simulates the end of the blend operation.  */
    Signal *blockStateDAC;      /**<  Signal to send the color buffer block state to the DAC.  */

    /*  Color Write registers.  */
    u32bit hRes;                /**<  Display horizontal resolution.  */
    u32bit vRes;                /**<  Display vertical resolution.  */
    s32bit startX;              /**<  Viewport initial x coordinate.  */
    s32bit startY;              /**<  Viewport initial y coordinate.  */
    u32bit width;               /**<  Viewport width.  */
    u32bit height;              /**<  Viewport height.  */
    u32bit clearColor;          /**<  Current clear color for the framebuffer.  */
    bool earlyZ;                /**<  Flag that enables or disables early Z testing (Z Stencil before shading).  */
    bool blend;                 /**<  Flag storing if color blending is active.  */
    BlendEquation equation;     /**<  Current blending equation mode.  */
    BlendFunction srcRGB;       /**<  Source RGB weight funtion.  */
    BlendFunction dstRGB;       /**<  Destination RGB weight function.  */
    BlendFunction srcAlpha;     /**<  Source alpha weight function.  */
    BlendFunction dstAlpha;     /**<  Destination alpha weight function.  */
    QuadFloat constantColor;    /**<  Blend constant color.  */
    bool writeR;                /**<  Write mask for red color component.  */
    bool writeG;                /**<  Write mask for green color component.  */
    bool writeB;                /**<  Write mask for blue color component.  */
    bool writeA;                /**<  Write mask for alpha color channel.  */
    bool logicOperation;        /**<  Flag storing if logical operation is active.  */
    LogicOpMode logicOpMode;    /**<  Current logic operation mode.  */
    u32bit frontBuffer;         /**<  Address in the GPU memory of the front (primary) buffer.  */
    u32bit backBuffer;          /**<  Address in the GPU memory of the back (secondary) buffer.  */

    /*  Color Write parameters.  */
    u32bit stampsCycle;         /**<  Number of stamps that can be received per cycle.  */
    u32bit overW;               /**<  Over scan tile width in scan tiles.  */
    u32bit overH;               /**<  Over scan tile height in scan tiles.  */
    u32bit scanW;               /**<  Scan tile width in generation tiles.  */
    u32bit scanH;               /**<  Scan tile height in generation tiles.  */
    u32bit genW;                /**<  Generation tile width in stamps.  */
    u32bit genH;                /**<  Generation tile height in stamps.  */
    u32bit bytesPixel;          /**<  Bytes per pixel.  */
    bool disableColorCompr;     /**<  Disables color compression.  */
    u32bit colorCacheWays;      /**<  Color cache set associativity.  */
    u32bit colorCacheLines;     /**<  Number of lines in the color cache per way/way.  */
    u32bit stampsLine;          /**<  Number of stamps per color cache line.  */
    u32bit cachePortWidth;      /**<  Width of the color cache ports in bytes.  */
    bool extraReadPort;         /**<  Add an additional read port to the color cache.  */
    bool extraWritePort;        /**<  Add an additional write port to the color cache.  */
    u32bit blocksCycle;         /**<  State of color block copied to the DAC per cycle.  */
    u32bit blendQueueSize;      /**<  Blend queue size.  */
    u32bit blendRate;           /**<  Rate at which blending is performed on stamps (cycles between stamps).  */
    u32bit blendLatency;        /**<  Blending latency.  */
    u32bit blockStateLatency;   /**<  Latency of the block state update signal to the DAC.  */
    u32bit fragmentMapMode;     /**<  Fragment map mode.  */
    FragmentOpEmulator &frEmu;  /**<  Reference to the fragment operation emulator object.  */

    /*  Color Write state.  */
    RasterizerState state;      /**<  Current box state.  */
    RasterizerCommand *lastRSCommand;   /**<  Stores the last Rasterizer Command received (for signal tracing).  */
    u32bit currentTriangle;     /**<  Identifier of the current triangle being processed (used to count triangles).  */
    u32bit triangleCounter;     /**<  Number of processed triangles.  */
    u32bit fragmentCounter;     /**<  Number of fragments processed in the current batch.  */
    u32bit frameCounter;        /**<  Counts the number of rendered frames.  */
    bool lastFragment;          /**<  Last batch fragment flag.  */
    bool receivedFragment;      /**<  If a fragment has been received in the current cycle.  */
    MemState memoryState;       /**<  Current memory controller state.  */
    bool endFlush;              /**<  Flag that signals the end of the flush of the color cache.  */
    u32bit fragmentSource;      /**<  Stores from which unit the fragments are received (Z Stencil Test or Fragment FIFO).  */
    u32bit blendCycles;         /**<  Cycles remaining until the next stamp can be blended.  *&

    /*  Color cache.  */
    ColorCache *colorCache;     /**<  Pointer to the color cache/write buffer.  */
    u32bit bytesLine;           /**<  Bytes per color cache line.  */
    u32bit lineShift;           /**<  Logical shift for a color cache line.  */
    u32bit lineMask;            /**<  Logical mask for a color cache line.  */
    u32bit stampMask;           /**<  Logical mask for the offset of a stamp inside a color cache line.  */

    /*  Blend Queue structures.  */
    BlendQueue *blendQueue;     /**<  The blend queue.  Stores stamps to be blended or written.  */
    u32bit fetchBlend;          /**<  Entry of the blend queue from which to fetch color data.  */
    u32bit readBlend;           /**<  Entry of the blend queue from which to read color data.  */
    u32bit writeBlend;          /**<  Entry of the blend queue from which to write color data.  */
    u32bit freeBlend;           /**<  Next free entry in the blend queue.  */
    u32bit blendStamps;         /**<  Stamps stored in the blend queue.  */
    u32bit fetchStamps;         /**<  Stamps waiting in the blend queue to fetch color data.  */
    u32bit readStamps;          /**<  Stamps waiting in the blend queue to read color data.  */
    u32bit writeStamps;         /**<  Stamps waiting in the blend queue to write color data.  */
    u32bit freeStamps;          /**<  Free entries in the blend queue.  */

    /*  Configurable buffers.  */
    FragmentInput **stamp;      /**<  Stores last receveived stamp.  */

    /*  Color Clear state.  */
    u32bit copyStateCycles;     /**<  Number of cycles remaining for the copy of the block state memory.  */
    //u8bit clearBuffer[MAX_TRANSACTION_SIZE];    /**<  Clear buffer, stores the clear values for a full transaction.  */
    //u32bit clearAddress;        /**<  Current color clear address.  */
    //u32bit endClearAddress;     /**<  End of the memory region to clear.  */
    //u32bit busCycles;           /**<  Remaining memory bus cycles.  */
    //u32bit ticket;              /**<  Memory ticket.  */
    //u32bit freeTickets;         /**<  Number of free memory tickets.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *inputs;       /**<  Input fragments.  */
    GPUStatistics::Statistic *blended;      /**<  Blended fragments.  */
    GPUStatistics::Statistic *logoped;      /**<  Logical operation fragments.  */
    GPUStatistics::Statistic *outside;      /**<  Outside triangle/viewport fragments.  */
    GPUStatistics::Statistic *culled;       /**<  Culled fragments.  */
    GPUStatistics::Statistic *readTrans;    /**<  Read transactions to memory.  */
    GPUStatistics::Statistic *writeTrans;   /**<  Write transactions to memory.  */
    GPUStatistics::Statistic *readBytes;    /**<  Bytes read from memory.  */
    GPUStatistics::Statistic *writeBytes;   /**<  Bytes written to memory.  */
    GPUStatistics::Statistic *fetchOK;      /**<  Succesful fetch operations.  */
    GPUStatistics::Statistic *fetchFail;    /**<  Failed fetch operations.  */
    GPUStatistics::Statistic *allocateOK;   /**<  Succesful allocate operations.  */
    GPUStatistics::Statistic *allocateFail; /**<  Failed allocate operations.  */
    GPUStatistics::Statistic *readOK;       /**<  Sucessful read operations.  */
    GPUStatistics::Statistic *readFail;     /**<  Failed read operations.  */
    GPUStatistics::Statistic *writeOK;      /**<  Sucessful write operations.  */
    GPUStatistics::Statistic *writeFail;    /**<  Failed write operations.  */
    GPUStatistics::Statistic *rawDep;       /**<  Blocked read accesses because of read after write dependence between stamps.  */

    /*  Latency map.  */
    u32bit *latencyMap;                     /**<  Stores the fragment latency map for the Color Write unit.  */
    bool clearLatencyMap;                   /**<  Clear the latency map.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param command The rasterizer command to process.
     *
     */

    void processCommand(RasterizerCommand *command);

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

    /**
     *
     *  Converts an array of color values in integer format to float
     *  point format.
     *
     *  @param in The input color array in integer format.
     *  @param out The output color array in float point format.
     *
     */

    void colorInt2Float(u8bit *in, QuadFloat *out);

    /**
     *
     *  Converts an array of color values in integer format to float
     *  point format.
     *
     *  @param in The input color array in float point format.
     *  @param out The output color array in integer point format.
     *
     */

    void colorFloat2Int(QuadFloat *in, u8bit *out);


public:

    /**
     *
     *  Color Write box constructor.
     *
     *  Creates and initializes a new Color Write box object.
     *
     *  @param stampsCycle Number of stamps per cycle.
     *  @param overW Over scan tile width in scan tiles (may become a register!).
     *  @param overH Over scan tile height in scan tiles (may become a register!).
     *  @param scanW Scan tile width in fragments.
     *  @param scanH Scan tile height in fragments.
     *  @param genW Generation tile width in fragments.
     *  @param genH Generation tile height in fragments.
     *  @param bytesPixel Number of bytes to be used per pixel (should be a register!).
     *  @param disableColorCompr Disables color compression.
     *  @param cacheWays Color cache set associativity.
     *  @param cacheLines Number of lines in the color cache per way/way.
     *  @param stampsLine Numer of stamps per color cache line (less than a tile!).
     *  @param portWidth Width of the color cache ports in bytes.
     *  @param extraReadPort Adds an extra read port to the color cache.
     *  @param extraWritePort Adds an extra write port to the color cache.
     *  @param cacheReqQueueSize Size of the color cache memory request queue.
     *  @param inputRequests Number of read requests and input buffers in the color cache.
     *  @param outputRequests Number of write requests and output buffers in the color cache.
     *  @param maxBlocks Maximum number of sopported color blocks in the
     *  color cache state memory.
     *  @param blocksCycle Number of state block entries that can be
     *  modified (cleared), read and sent per cycle.
     *  @param compCycles Color block compression cycles.
     *  @param decompCycles Color block decompression cycles.
     *  @param blendQueueSize Blend queue size (in entries).
     *  @param blendRate Rate at which stamps are blended (cycles between stamps).
     *  @param blendLatency Blending latency.
     *  @param blockStateLatency Latency to send the color block state to the DAC.
     *  @param fragMapMode Fragment Map Mode.
     *  @param frEmu Reference to the Fragment Operation Emulator object.
     *  @param name The box name.
     *  @param prefix String used to prefix the box signals names.
     *  @param parent The box parent box.
     *
     *  @return A new Color Write object.
     *
     */

    ColorWrite(u32bit stampsCycle, u32bit overW, u32bit overH, u32bit scanW, u32bit scanH,
        u32bit genW, u32bit genH, u32bit bytesPixel, bool disableColorCompr,
        u32bit cacheWays, u32bit cacheLines, u32bit stampsLine, u32bit portWidth, bool extraReadPort,
        bool extraWritePort, u32bit cacheReqQueueSize, u32bit inputRequests,
        u32bit outputRequests, u32bit maxBlocks, u32bit blocksCycle,
        u32bit compCycles, u32bit decompCycles,
        u32bit blendQueueSize, u32bit blendRate, u32bit blendLatency, u32bit blockStateLatency,
        u32bit fragMapMode, FragmentOpEmulator &frOp, char *name, char *prefix = 0, Box* parent = 0);

    /**
     *
     *  Color Write simulation function.
     *
     *  Simulates a cycle of the Color Write box.
     *
     *  @param cycle The cycle to simulate of the Color Write box.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Get pointer to the fragment latency map.
     *
     *  @param width Width of the latency map in stamps.
     *  @param height Height of the latency map in stamps.
     *
     *  @return A pointer to the latency map stored in the Color Write unit.
     *
     */

    u32bit *getLatencyMap(u32bit &width, u32bit &height);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Color Write box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(string &stateString);

};

} // namespace gpu3d

#endif


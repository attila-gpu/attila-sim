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
 * $RCSfile: HierarchicalZ.h,v $
 * $Revision: 1.14 $
 * $Author: vmoya $
 * $Date: 2007-09-21 14:56:34 $
 *
 * Hierarchical Z class definition file.
 *
 */

/**
 *
 *  @file HierarchicalZ.h
 *
 *  This file defines the Hierarchical Z class.
 *
 *  This class implements a Hierarchical Z simulation box.
 *
 */


#ifndef _HIERARCHICALZ_

#define _HIERARCHICALZ_

#include "Box.h"
#include "GPUTypes.h"
#include "support.h"
#include "GPU.h"
#include "FragmentInput.h"
#include "RasterizerState.h"
#include "RasterizerCommand.h"
#include "PixelMapper.h"
#include "toolsQueue.h"

namespace gpu3d
{

/**
 *
 *  Defines the Hierarchical Z states for the Triangle Traversal.
 *
 */

enum HZState
{
    HZST_READY,         /**<  The Hierarchical Z Early test can receive new stamps.  */
    HZST_BUSY           /**<  The Hierarchical Z Early test can not receive new stamps.  */
};

/**
 *
 *  This structure defines a line in the Hierachical Z Buffer Cache.
 *
 */

struct HZCacheLine
{
    u32bit block;       /**<  Identifier of the line of blocks stored in the cache.  */
    u32bit *z;          /**<  The z value stored for each block in the line.  */
    u32bit reserves;    /**<  Number of reserves for the cache entry.  */
    bool read;          /**<  The cache entry has received the line from the HZ buffer.  */
    bool valid;         /**<  If the cache line is storing valid data.  */
};

/**
 *
 *  Defines the maximum number of HZ blocks that can be required to evaluate a stamp tile.
 *
 */
const u32bit MAX_STAMP_BLOCKS = 4;

/**
 *
 *  Defines an entry in the stamp queue for the Hierarchical Early
 *  Z Test.
 *
 */
struct HZQueue
{
    FragmentInput **stamp;          /**<  Stores a fragment stamp.  */
    u32bit numBlocks;               /**<  Number of HZ blocks that are required to evaluate the stamp.  */
    u32bit block[MAX_STAMP_BLOCKS]; /**<  Hierarchical Z block for the stamp.  */
    u32bit stampZ;                  /**<  Minimum Z value of the stamp.  */
    u32bit currentBlock;            /**<  Pointer to the block that is being processed.  */
    u32bit cache[MAX_STAMP_BLOCKS]; /**<  Pointer to the cache entry where the stamp block Z value will be stored.  */
    u32bit blockZ;                  /**<  Value of the blocks z.  */
    bool culled;                    /**<  Stores if the stamp has been culled.  */
};

/**
 *
 *  This class implements the Hierarchical Z box.
 *
 *  The Hierarchical Z box simulates a Hierarchical Z buffer
 *  for a GPU.
 *
 *  Inherits from the Box class that offers basic simulation
 *  support.
 *
 */


class HierarchicalZ : public Box
{
    //  Defines a entry of the microStampQueue.
    struct StampInput
    {
        FragmentInput* fragment[STAMP_FRAGMENTS];
    };

private:

    /*  Hierarchical Z Signals.  */
    Signal *inputStamps;        /**<  Input stamp signal from Triangle Traversal.  */
    Signal *outputStamps;       /**<  Output stamp signal to Interpolator.  */
    Signal *fFIFOState;         /**<  State signal from the Fragment FIFO box.  */
    Signal *hzTestState;        /**<  State signal to the Triangle Traversal box.  */
    Signal *hzCommand;          /**<  Command signal from the main rasterizer box.  */
    Signal *hzState;            /**<  Command signal to the main rasterizer box.  */
    Signal **hzUpdate;          /**<  Array of update signals from the Z Test box.  */
    Signal *hzBufferWrite;      /**<  Write signal for the HZ Buffer Level 0.  */
    Signal *hzBufferRead;       /**<  Read signal for the HZ Buffer Level 0.  */

    /*  Hierarchical Z parameters.  */
    u32bit stampsCycle;              /**<  Number of stamps per cycle.  */
    u32bit overW;                    /**<  Over scan tile width in scan tiles.  */
    u32bit overH;                    /**<  Over scan tile height in scan tiles.  */
    u32bit scanW;                    /**<  Scan tile width in generation tiles.  */
    u32bit scanH;                    /**<  Scan tile height in generation tiles.  */
    u32bit genW;                     /**<  Generation tile width in stamps.  */
    u32bit genH;                     /**<  Generation tile height in stamps.  */
    bool disableHZ;                  /**<  Disables HZ support.  */
    u32bit blockStamps;              /**<  Stamps per HZ block.  */
    u32bit hzBufferSize;             /**<  Size of the Hierarchical Z Buffer (number of blocks stored).  */
    u32bit hzCacheLines;             /**<  Number of lines in the fast access Hierarchical Z Buffer cache.  */
    u32bit hzCacheLineSize;          /**<  Number of blocks per HZ Cache line.  */
    u32bit hzQueueSize;              /**<  Size of the stamp queue for the Early test.  */
    u32bit hzBufferLatency;          /**<  Access latency to the HZ Buffer Level 0.  */
    u32bit hzUpdateLatency;          /**<  Update signal latency (from Z Stencil Test).  */
    u32bit clearBlocksCycle;         /**<  Number of blocks that can be cleared per cycle.  */
    u32bit numStampUnits;            /**<  Number of stamp units attached to the Rasterizer.  */

    /*  Hierarchical Z constants.  */
    u32bit blockShift;          /**<  Bits of a fragment address corresponding not related to the block.  */
    u32bit blockSize;           /**<  Size in depth elements (32 bit) of a HZ block.  */
    u32bit blockSizeMask;       /**<  Bit mask for the block size.  */

    /*  Hierarchical Z registers.  */
    u32bit hRes;                /**<  Display horizontal resolution.  */
    u32bit vRes;                /**<  Display vertical resolution.  */
    s32bit startX;              /**<  Viewport initial x coordinate.  */
    s32bit startY;              /**<  Viewport initial y coordinate.  */
    u32bit width;               /**<  Viewport width.  */
    u32bit height;              /**<  Viewport height.  */
    bool scissorTest;           /**<  Enables scissor test.  */
    s32bit scissorIniX;         /**<  Scissor initial x coordinate.  */
    s32bit scissorIniY;         /**<  Scissor initial y coordinate.  */
    u32bit scissorWidth;        /**<  Scissor width.  */
    u32bit scissorHeight;       /**<  Scissor height.  */
    bool hzEnabled;             /**<  Early Z test enable flag.  */
    u32bit clearDepth;          /**<  Current clear depth value.  */
    u32bit zBufferPrecission;   /**<  Z buffer depth bit precission.  */
    bool modifyDepth;           /**<  Flag to disable Hierarchical Z if the fragment shader stage modifies the fragment depth component.  */
    CompareMode depthFunction;  /**<  Depth test comparation function.  */
    bool multisampling;         /**<  Flag that stores if MSAA is enabled.  */
    u32bit msaaSamples;         /**<  Number of MSAA Z samples generated per fragment when MSAA is enabled.  */

    s32bit scissorMinX;         /**<  Scissor bounding box.  */
    s32bit scissorMaxX;         /**<  Scissor bounding box.  */
    s32bit scissorMinY;         /**<  Scissor bounding box.  */
    s32bit scissorMaxY;         /**<  Scissor bounding box.  */

    //  Pixel Mapper
    PixelMapper pixelMapper;    /**<  Maps pixels to addresses and processing units.  */

    /*  Hierarchical Z state.  */
    RasterizerState state;      /**<  Current Hierarchical Z box state.  */
    RasterizerCommand *lastRSCommand;   /**<  Last rasterizer command received.  */
    u32bit fragmentCounter;     /**<  Number of received fragments.  */
    bool lastFragment;          /**<  Stores if the last fragment has been received.  */
    bool dataBus;               /**<  Stores if the HZ Level 0 data bus is busy.  */
    u32bit clearCycles;         /**<  Remaining HZ clear cycles.  */

    /*  Hierarchical Z structures.  */
    u32bit *hzLevel0;           /**<  Level 0 of the Hierachical Z buffer.  */
    HZCacheLine *hzCache;       /**<  Hierarchical Z cache.  */
    HZQueue *hzQueue;           /**<  Hierarchical Z Early Test queue.  */
    u32bit nextFree;            /**<  Next free HZ queue entry.  */
    u32bit nextRead;            /**<  Next stamp for which to read the block Z value.  */
    u32bit nextTest;            /**<  Next stamp to be tested.  */
    u32bit nextSend;            /**<  Next stamp to send to the Interpolator box.  */
    u32bit freeHZQE;            /**<  Number of free entries in the HZ queue.  */
    u32bit readHZQE;            /**<  Number of read entries in the HZ queue.  */
    u32bit testHZQE;            /**<  Number of test entries in the HZ queue.  */
    u32bit sendHZQE;            /**<  Number of send entries in the HZ queue.  */
    u32bit hzLineMask;          /**<  Precalculated mask for HZ cache lines.  */
    u32bit hzBlockMask;         /**<  Precalculated mask for the blocks inside a HZ cache line.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *inputs;         /**<  Input fragments.  */
    GPUStatistics::Statistic *outputs;        /**<  Output fragments.  */
    GPUStatistics::Statistic *outTriangle;    /**<  Fragments outside triangle.  */
    GPUStatistics::Statistic *outViewport;    /**<  Fragments outside viewport.  */
    GPUStatistics::Statistic *culled;         /**<  Culled fragments.  */
    GPUStatistics::Statistic *cullOutside;    /**<  Fragments outside triangle or viewport culled.  */
    GPUStatistics::Statistic *cullHZ;         /**<  Fragments culled by HZ test.  */
    GPUStatistics::Statistic *misses;         /**<  Misses to HZ Cache.  */
    GPUStatistics::Statistic *hits;           /**<  Hits to HZ Cache.  */
    GPUStatistics::Statistic *reads;          /**<  Read operations to the HZ Buffer.  */
    GPUStatistics::Statistic *writes;         /**<  Write operations to the HZ Buffer.  */


    /*  Private functions.  */

    /**
     *
     *  Receives new stamps from Fragment Generation.
     *
     *  @param cycle The current simulation cycle.
     *
     */

    void receiveStamps(u64bit cycle);

    /**
     *
     *  Calculates the minimum depth between two Z values
     *  (24 bit format).
     *
     *  @param a First input z value.
     *  @param b Second input z value.
     *
     *  @return The minimum of the two values.
     *
     */

    u32bit minimumDepth(u32bit a, u32bit b);

    /**
     *
     *  Calculates the address of the block for a stamp or fragment inside
     *  the Hierarchical Z buffer.
     *
     *  @param x Horizontal position of the stamp/fragment.
     *  @param y Vertical position of the stamp/fragment.
     *  @param blocks Pointer to a MAX_STAMP_BLOCKS array where to store the addresses of
     *  the blocks required to evaluate the stamp.
     *
     *  @return The number of block address required to evaluate the stamp.
     *
     */

    u32bit stampBlocks(s32bit x, s32bit y, u32bit *blocks);

    /**
     *
     *  Search the Hierarchical Z Cache for a given Hierarchical Z block.
     *
     *  u32bit block Block (address) to search in the cache.
     *  u32bit cacheEntry Reference to a variable where to store which
     *  cache entry stores the block if found inside the cache.
     *
     *  @return If the block was found in the HZ Cache.
     *
     */

    bool searchHZCache(u32bit block, u32bit &cacheEntry);

    /**
     *
     *  Inserts a new block in the Hierarchical Z Cache.
     *
     *  @param block Block (address) to insert in the HZ Cache.
     *  @param cacheEntry Reference to a variable where to store which
     *  cache entry will store the new block in the HZ Cache.
     *
     *  @return If the block could be added to the HZ cache.
     *  Fails if there are no free HZ cache entries.
     *
     */

    bool insertHZCache(u32bit block, u32bit &cacheEntry);


    /**
     *
     *  Processes a rasterizer command.
     *
     *  @param command The rasterizer command to process.
     *  @param cycle  Current simulation cycle.
     *
     */

    void processCommand(RasterizerCommand *command);

    /**
     *
     *  Processes a register write.
     *
     *  @param reg The Triangle Traversal register to write.
     *  @param subreg The register subregister to writo to.
     *  @param data The data to write to the register.
     *
     */

    void processRegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);

    /**
     *
     *  Compares a fragment and its corresponding hierarchical z values.
     *
     *  @param fragZ Fragment depth.
     *  @param hzZ Corresponding depth stored in the Hierarchical Z buffer.
     *
     *  @return The result of the compare operation.
     *
     */

    bool hzCompare(u32bit fragZ, u32bit hzZ);

public:

    /**
     *
     *  Hierarchical Z class constructor.
     *
     *  Creates and initializes a Hierarchical Z box.
     *
     *  @param stampCyle Stamps received per cycle.
     *  @param overW Over scan tile width in scan tiles (may become a register!).
     *  @param overH Over scan tile height in scan tiles (may become a register!).
     *  @param scanW Scan tile width in fragments.
     *  @param scanH Scan tile height in fragments.
     *  @param genW Generation tile width in fragments.
     *  @param genH Generation tile height in fragments.
     *  @param disableHZ Disables the Hierarchical Z support.
     *  @param stampBlocks Stamps per Hierarchical Z block.
     *  @param hzBufferSize Size of the Hierarchical Z Buffer (number of stored blocks).
     *  @param hzCacheLines Number of lines in the Hierarchical Z Buffer cache.
     *  @param hzCacheLineSize Number of blocks in a line of the Hierarchical Z Buffer cache .
     *  @param hzQueueSize Size of the stamp queue for the early test.
     *  @param hzBufferLatency Access latency to the HZ Buffer level 0.
     *  @param hzUpdateLatency Update signal latency with Z Test.
     *  @param clearBlocksCycle Number of blocks cleared per cycle in the HZ Buffer level 0.
     *  @param nStampUnits Number of stamp units attached to HZ.
     *  @param suPrefixes Array of stamp unit prefixes.
     *  @param microTrisAsFragments Process micro triangles directly as fragments (MicroPolygon Rasterizer)
     *  @param microTriSzLimit 0: 1-pixel bound triangles only, 1: 1-pixel and 1-stamp bound triangles, 2: Any triangle bound to 2x2, 1x4 or 4x1 stamps (MicroPolygon Rasterizer).
     *  @param name Name of the box.
     *  @param parent Pointer to the box parent box.
     *
     *  @return A new initialized Hierarchical Z box.
     *
     */

    HierarchicalZ(u32bit stampCycle, u32bit overW, u32bit overH, u32bit scanW, u32bit scanH,
        u32bit genW, u32bit genH, bool disableHZ, u32bit stampBlocks, u32bit hZBufferSize, u32bit hzCacheLines,
        u32bit hzCacheLineSize, u32bit hzQueueSize, u32bit hzBufferLatency, u32bit hzUpdateLatency,
        u32bit clearBlocksCycle, u32bit nStampUnits, char **suPrefixes, bool microTrisAsFrag,
        u32bit microTriSzLimit, char *name, Box* parent);

    /**
     *
     *  Hierarchical Z class simulation function.
     *
     *  Simulates a cycle of the Hierarchical Z box.
     *
     *  @param cycle The cycle to simulate in the Hierarchical Z box.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Returns a single line string with state and debug information about the
     *  Hierarchical Z box.
     *
     *  @param stateString Reference to a string where to store the state information.
     *
     */

    void getState(string &stateString);


    /**
     *
     *  Saves the content of the Hierarchical Z buffer into a file.
     *
     */

    void saveHZBuffer();

    /**
     *
     *  Loads the content of the Hierarchical Z buffer from a file.
     */

    void loadHZBuffer();

};

} // namespace gpu3d

#endif

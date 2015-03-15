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
 * $RCSfile: ZCacheV2.h,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:17 $
 *
 * Z Cache class definition file.
 *
 */

/**
 *
 *  @file ZCacheV2.h
 *
 *  Defines the Z Cache class.  This class defines the cache used
 *  to access the depth and stencil buffer in a GPU.
 *
 */


#ifndef _ZCACHEV2_

#define _ZCACHEV2_

#include "GPUTypes.h"
#include "FetchCache.h"
#include "ROPCache.h"
#include "MemoryTransaction.h"
#include "DepthCompressorEmulator.h"

namespace gpu3d
{

/**
 *
 *  This class describes and implements the behaviour of the cache
 *  used to access the Z buffer in a GPU.
 *  The Z cache is used in the Z Write GPU unit.
 *
 *  This classes derives from the ROP Cache interface class.
 *
 *  This class uses the Fetch Cache.
 *
 */

class ZCacheV2 : public ROPCache
{

private:

    //  Z cache identification.
    static u32bit cacheCounter;     /**<  Class variable used to count and create identifiers for the created Z Caches.  */

    //  Z cache registers.
    u32bit clearDepth;      /**<  Depth clear value.  */
    u8bit clearStencil;     /**<  Stencil clear value.  */
    
    // Z cache state
    u32bit wrBlockMaxVal;   /**<  Maximum value the elements of ROP data buffer block that was written to memory .  */
    
    //static const int compressorBlockSizes[] = {64, 128, 192};

protected:
    virtual void processNextWrittenBlock(u8bit* outputBuffer, u32bit size);
    
    virtual CompressorEmulator& getCompressor() {
        return DepthCompressorEmulator::getInstance();
    }
    
public:

    /**
     *
     *  Z Cache constructor.
     *
     *  Creates and initializes a Z Cache object.
     *
     *  @param numWays Number of ways in the cache.
     *  @param numLines Number of lines in the cache.
     *  @param lineSize Size of the cache line in bytes.
     *  @param readPorts Number of read ports.
     *  @param writePorts Number of write ports.
     *  @param portWidth Width in bytes of the write and read ports.
     *  @param reqQueueSize Z cache request to memory queue size.
     *  @param inputRequests Number of read requests and input buffers.
     *  @param outputRequests Number of write requests and output buffers.
     *  @param disableCompr Disables Z block compression.
     *  @param numStampUnits Number of stamp units in the GPU.
     *  @param stampUnitStride Stride in blocks for the blocks that are assigned to a stamp unit.
     *  @param maxBlocks Maximum number of sopported color blocks in the Z buffer.
     *  @param blocksCycle Number of state block entries that can be modified (cleared) per cycle.
     *  @param compCycles Compression cycles.
     *  @param decompCycles Decompression cycles.
     *  @param postfix The postfix for the z cache statistics.
     *
     *  @return A new initialized cache object.
     *
     */

    ZCacheV2(u32bit numWays, u32bit numLines, u32bit lineSize,
        u32bit readPorts, u32bit writePorts, u32bit portWidth, u32bit reqQueueSize,
        u32bit inputRequests, u32bit outputRequests, bool disableCompr, u32bit numStampUnits,
        u32bit stampUnitStride, u32bit maxBlocks,
        u32bit blocksCycle, u32bit compCycles, u32bit decompCycles, char *postfix);

    /**
     *
     *  Clears the Z buffer.
     *
     *  @param clearDepth The depth value with which to clear the depth buffer.
     *  @param clearStencil The stencil value with which to clear the stencil buffer.
     *
     *  @return If the clear process has finished.
     *
     */

    bool clear(u32bit clearDepth, u8bit clearStencil);

    /**
     *
     *  Checks if there is an update for the Hierarchical Z ready
     *  and returns it.
     *
     *  @param block A reference to a variable were to store the Hierarchical Z
     *  block to be updated.
     *  @param z A reference to a variable where to store the new Z for the updated
     *  Hierarchical Z block.
     *
     *  @return If there is an update to the Hierarchical Z ready.
     *
     */

    bool updateHZ(u32bit &block, u32bit &z);

    /**
     *
     *  Copies the block state memory.
     *
     *  @param buffer Pointer to a ROP Block State buffer.
     *  @param blocks Number of blocks to copy.
     *
     */

    void copyBlockStateMemory(ROPBlockState *buffer, u32bit blocks);

};

} // namespace gpu3d

#endif

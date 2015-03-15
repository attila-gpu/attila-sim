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
 * $RCSfile: ColorCacheV2.h,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:16 $
 *
 * Color Cache class definition file.
 *
 */

/**
 *
 *  @file ColorCacheV2.h
 *
 *  Defines the Color Cache class.  This class defines the cache used
 *  to access the color buffer in a GPU.
 *
 */


#ifndef _COLORCACHEV2_

#define _COLORCACHEV2_

#include "GPUTypes.h"
#include "FetchCache.h"
#include "ROPCache.h"
#include "MemoryTransaction.h"
#include "ColorCompressorEmulator.h"

namespace gpu3d
{

/**
 *
 *  This class describes and implements the behaviour of the cache
 *  used to access the color buffer in a GPU.
 *  The color cache is used in the Color Write GPU unit.
 *
 *  This classes derives from the ROPCache interface class.
 *
 *  This class uses the Fetch Cache.
 *
 */

class ColorCacheV2 : public ROPCache
{

private:

    static u32bit cacheCounter;     /**<  Class variable used to count and create identifiers for the created Color Caches.  */

protected:
    //virtual void processNextWrittenBlock(u8bit* outputBuffer, u32bit size);
    
    virtual CompressorEmulator& getCompressor() {
        return ColorCompressorEmulator::getInstance();
    }

public:

    /**
     *
     *  Color Cache constructor.
     *
     *  Creates and initializes a Color Cache object.
     *
     *  @param numWays Number of ways in the cache.
     *  @param numLines Number of lines in the cache.
     *  @param lineSize Size of the cache line in bytes.
     *  @param bytesStamp Number of bytes per stamp.
     *  @param readPorts Number of read ports.
     *  @param writePorts Number of write ports.
     *  @param portWidth Width in bytes of the write and read ports.
     *  @param reqQueueSize Color cache request to memory queue size.
     *  @param inputRequests Number of read requests and input buffers.
     *  @param outputRequests Number of write requests and output buffers.
     *  @param disableCompr Disables color compression.
     *  @param numStampUnits Number of stamp units in the GPU.
     *  @param stampUnitStride Stride in blocks for the blocks that are assigned to a stamp unit.
     *  @param maxBlocks Maximum number of sopported color blocks in the color buffer.
     *  @param blocksCycle Number of state block entries that can be modified (cleared) per cycle.
     *  @param compCycles Compression cycles.
     *  @param decompCycles Decompression cycles.
     *  @param postfix The postfix for the color cache statistics.
     *
     *  @return A new initialized cache object.
     *
     */

    ColorCacheV2(u32bit numWays, u32bit numLines, u32bit lineSize,
        u32bit readPorts, u32bit writePorts, u32bit portWidth, u32bit reqQueueSize,
        u32bit inputRequests, u32bit outputRequests, bool disableCompr, u32bit numStampUnits,
        u32bit stampUnitStride, u32bit maxBlocks, u32bit blocksCycle, u32bit compCycles, u32bit decompCycles, char *postfix);

    /**
     *
     *  Clears the color buffer.
     *
     *  @param clearColor Color value with which to
     *  clear the color buffer.
     *
     *  @return If the clear process has finished.
     *
     */

    bool clear(u8bit *clearColor);

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

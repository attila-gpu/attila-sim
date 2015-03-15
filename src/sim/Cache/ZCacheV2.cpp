/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not b;e disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: ZCacheV2.cpp,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:17 $
 *
 * Z Cache class implementation file.
 *
 */

/**
 *
 * @file ZCacheV2.cpp
 *
 * Implements the Z Cache class.  This class the cache used for access to the Z buffer in a GPU.
 *
 */

#include "ZCacheV2.h"
#include "GPUMath.h"
#include "FragmentOpEmulator.h"

using namespace gpu3d;

//  Z Cache class counter.  Used to create identifiers for the created Z Caches
//  that are then used to access the Memory Controller.
u32bit ZCacheV2::cacheCounter = 0;


//  Z cache constructor.
ZCacheV2::ZCacheV2(u32bit ways, u32bit lines, u32bit lineSz,
        u32bit readP, u32bit writeP, u32bit pWidth, u32bit reqQSize, u32bit inReqs,
        u32bit outReqs, bool zComprDisabled, u32bit numStampUnits, u32bit stampUnitStride, u32bit maxZBlocks, u32bit blocksPerCycle,
        u32bit compCycles, u32bit decompCycles, char *postfix) :

        ROPCache(ways, lines, lineSz, readP, writeP, pWidth, reqQSize,
            inReqs, outReqs, zComprDisabled, numStampUnits, stampUnitStride, maxZBlocks, blocksPerCycle, compCycles,
            decompCycles, ZSTENCILTEST, "ZCache", postfix)

{
    //  Get the Z Cache identifier.
    cacheID = cacheCounter;

    //  Update the number of created Z Caches.
    cacheCounter++;

    //  Set reset value for clear.
    for(u32bit i = 0; i < (MAX_BYTES_PER_PIXEL >> 2); i++)
        ((u32bit *) clearResetValue)[i] = 0x00ffffff;

}

//  Clears the Z cache.
bool ZCacheV2::clear(u32bit depth, u8bit stencil)
{
    //  Reset the cache.
    if (clearMode)
    {
        //  Check clear cycles remaining.
        if (clearCycles > 0)
        {
            //  Update clear cycles.
            clearCycles--;

            //  Check if end of clear.
            if (clearCycles == 0)
            {
                //  Set the clear value registers.
                clearDepth = depth;
                clearStencil = stencil;

                //  Set the ROP data clear value
                ((u32bit *) clearROPValue)[0] = (clearStencil << 24) | (clearDepth & 0x00ffffff);

                /*  Unset reset mode.  */
                clearMode = FALSE;
            }
        }
    }
    else
    {
        //  NOTE:  SHOULD TAKE INTO ACCOUNT THE RESOLUTION SO NOT ALL
        //  BLOCKS HAD TO BE CLEARED EVEN IF UNUSED AT CURRENT RESOLUTION.

        //  Set clear cycles.
        clearCycles = (u32bit) ceil((f32bit) maxBlocks / (f32bit) blocksCycle);

        //  Set clear mode.
        clearMode = TRUE;

        //  Reset the cache.
        resetMode = TRUE;
    }

    return clearMode;
}

//  Check HZ updates.
bool ZCacheV2::updateHZ(u32bit &block, u32bit &z)
{
    //  Check if there is an updated block.
    if (blockWasWritten)
    {
        //  Return block identifier and block Z.
        block = writtenBlock;
        z = wrBlockMaxVal;

        //  Reset updated HZ block flag.
        blockWasWritten = false;

        return true;
    }
    else
        return false;
}

void ZCacheV2::processNextWrittenBlock(u8bit* outputBuffer, u32bit size)
{
    u32bit* data = (u32bit*) outputBuffer;
    u32bit dataSize = size / sizeof(u32bit);
    
    u32bit maxZ;
    
    //  Calculate the maximum depth/Z.
    FragmentOpEmulator::blockMaxZ(data, dataSize, maxZ);

    // Store for later use
    wrBlockMaxVal = maxZ;
}

//  Copies the block state memory.
void ZCacheV2::copyBlockStateMemory(ROPBlockState *buffer, u32bit blocks)
{
    GPU_ASSERT(
        if (blocks > maxBlocks)
            panic("ZCache", "copyBlockSateMemory", "More blocks to copy than blocks in the state memory.");
    )

    //  Copy the block states.
    memcpy(buffer, blockState, sizeof(ROPBlockState) * blocks);
}


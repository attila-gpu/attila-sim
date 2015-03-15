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
 * $RCSfile: CacheReplacement.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-06-28 10:23:28 $
 *
 * Cache replacement policy class implementation file.
 *
 */

/**
 *
 * @file CacheReplacement.cpp
 *
 * Defines the Cache Replace Policy classes.  This classes define how a line
 * can be selected for replacing in a cache.
 *
 */

#include "CacheReplacement.h"
#include "GPUMath.h"

using namespace gpu3d;


/*  Generic cache replacement policy constructor.  */
CacheReplacementPolicy::CacheReplacementPolicy(u32bit bias, u32bit lines) :
    numBias(bias), numLines(lines)
{
    /*  Check number of lines.  */
    GPU_ASSERT(
        if (numLines == 0)
            panic("CacheReplacementPolicy", "CacheReplacementPolicy", "At least a line per bia is required.");
    )

    /*  Check number of bias.  */
    GPU_ASSERT(
        if (numBias == 0)
            panic("CacheReplacementPolicy", "CacheReplacementPolicy", "At least a bia is required.");
    )

}

/*  LRU replacemente policy constructor.  */
LRUPolicy::LRUPolicy(u32bit bias, u32bit lines) :
    CacheReplacementPolicy(bias, lines)
{
    u32bit i, j;

    /*  Allocate pointers to the access order list per line index.  */
    accessOrder = new u32bit*[numLines];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (accessOrder == NULL)
            panic("LRUPolicy", "LRUPolicy", "Could not allocate the pointers to the access order lists per line index.");
    )

    /*  Allocate the access order list per line line index.  */
    for(i = 0; i < numLines; i++)
    {
        /*  Allocate access order list for line index.  */
        accessOrder[i] = new u32bit[numBias];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (accessOrder[i] == NULL)
                panic("LRUPolicy", "LRUPolicy", "Could not allocate the access order list for the line index.");
        )

        /*  Reset access order.  */
        for (j = 0; j < numBias; j++)
            accessOrder[i][j] = j;
    }

}

/*  Access to a line.  */
void LRUPolicy::access(u32bit bia, u32bit line)
{
    u32bit i;
    u32bit ti, to;
    bool found;

    /*  Check bia range.  */
    GPU_ASSERT(
        if (bia >= numBias)
            panic("LRUPolicy", "access", "Out of range bia number.");
    )

    /*  Check line range.  */
    GPU_ASSERT(
        if (line >= numLines)
            panic("LRUPolicy", "access", "Out of range line number.");
    )

    /*  Update access order list for the line index.  */

    /*  Check if the accessed bia is already the first in the
        access order list.  */
    if (accessOrder[line][0] != bia)
    {
        /*  Insert the accessed bia in the first position in
            the list.  */
        to = bia;

        /*  Move the accessed bia to first position in the list.  */
        for(i = 0, found = FALSE; (i < numBias) && !found; i++)
        {
            /*  Already found the last */
            found = (accessOrder[line][i] == bia);

            /*  Keep the bia in the current position.  */
            ti = accessOrder[line][i];

            /*  Insert previous bia in the list.  */
            accessOrder[line][i] = to;

            /*  Exchange.  */
            to = ti;
        }
    }
}

/*  Select a victim line for a line index.  */
u32bit LRUPolicy::victim(u32bit line)
{
    /*  The last element in the access order list for a line index
        is the least recently used line.  */
    return accessOrder[line][numBias - 1];
}


/*  FIFO replacement policy constructor.  */
FIFOPolicy::FIFOPolicy(u32bit bias, u32bit lines) :
    CacheReplacementPolicy(bias, lines)
{
    u32bit i;

    /*  Allocate next victim per line index array.  */
    next = new u32bit[numLines];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (next == NULL)
            panic("FIFOPolicy", "FIFOPolicy", "Could not allocate the next victim array.");
    )

    /*  Reset the victim array.  */
    for(i = 0; i < numLines; i++)
        next[i] = 0;
}

/*  Access to a cache line and bia.  */
void FIFOPolicy::access(u32bit bia, u32bit line)
{
    /*  Check bia range.  */
    GPU_ASSERT(
        if (bia >= numBias)
            panic("FIFOPolicy", "access", "Out of range bia number.");
    )

    /*  Check line range.  */
    GPU_ASSERT(
        if (line >= numLines)
            panic("FIFOPolicy", "access", "Out of range line number.");
    )

    /*  Does not takes into account invalidations nor access
        so nothing to do.  */
}

/*  Select a victim line.  */
u32bit FIFOPolicy::victim(u32bit line)
{
    u32bit victimLine;

    /*  Check line range.  */
    GPU_ASSERT(
        if (line >= numLines)
            panic("FIFOPolicy", "access", "Out of range line number.");
    )

    /*  Get victim.  */
    victimLine = next[line];

    /*  Update next victim.  */
    next[line] = GPU_MOD(next[line] + 1, numBias);

    /*  Return the next victim*/
    return victimLine;
}

/*  PseudoLRU replacement policy constructor.  */
PseudoLRUPolicy::PseudoLRUPolicy(u32bit bias, u32bit lines) :
    CacheReplacementPolicy(bias, lines)
{
    u32bit i;

    /*  Check number of bias.  */
    GPU_ASSERT(
        if ((bias != 2) && (bias != 4) && (bias != 8)
            && (bias != 16) && (bias != 32))
            panic("PseudoLRUPolicy", "PseudoLRUPolicy", "Allowed number of bias/ways are 2, 4, 8, 16 and 32.");
    )

    /*  Allocate partitioned LRU table.  */
    lineState = new u32bit[numLines];

    /*  Reset partitioned LRU table.  */
    for (i = 0; i < numLines; i++)
        lineState[i] = 0;


    /*  Caculate bia shift.  */
    biaShift = GPUMath::calculateShift(numBias);

    /*  Check allocation.  */
    GPU_ASSERT(
        if (lineState == NULL)
            panic("PseudoLRUPolicy", "PseudoLRUPolicy", "Error allocating the partitioned LRU table.");
    )

}

/* Access to a bia and line.  */
void PseudoLRUPolicy::access(u32bit bia, u32bit line)
{
    u32bit mask;
    u32bit b;
    int i, j, k;

    /*  Check bia range.  */
    GPU_ASSERT(
        if (bia >= numBias)
            panic("PseudoLRUPolicy", "access", "Out of range bia number.");
    )

    /*  Check line range.  */
    GPU_ASSERT(
        if (line >= numLines)
            panic("PseudoLRUPolicy", "access", "Out of range line number.");
    )

    /*  Update pseudo LRU partition state for the line index.  */
    for(mask = numBias >> 1, i = numBias - 1, j = 1,k = 0; mask > 0;
        mask = mask >> 1)
    {
        /*  Calculate partition bit.  */
        b = ((bia & mask) == 0)?1:0;

        /*  Update partition bit.  */
        lineState[line] = lineState[line] & ~(1 << (i - 1)) | (b << (i - 1));

        /*  Update index inside the partition level.  */
        k = (k << 1) + ((b == 0)?1:0);

        /*  Update index inside the partition.  */
        i = i - j - k;

        /*  Update index to the partition level.  */
        j = j << 1;
    }
}


u32bit PseudoLRUPolicy::victim(u32bit line)
{
    u32bit victimLine;
    u32bit b;
    u32bit i, j, k;

    /*  Check line range.  */
    GPU_ASSERT(
        if (line >= numLines)
            panic("PseudoLRUPolicy", "access", "Out of range line number.");
    )

    /*  Calculate the victim line from the pseudo LRU partition state.  */
    for(i = numBias - 1, j = 1, k = 0; j < numBias;)
    {
        /*  Get bit from partition.  */
         b = (lineState[line] >> (i - 1)) & 0x01;

        /*  Update index inside the partition level.  */
        k = (k << 1) + b;

        /*  Update index inside the partition.  */
        i = i - j - k;

        /*  Update index to the partition level.  */
        j = j << 1;
    }

    /*  Get victim line.  */
    victimLine = k;

    /*  Return the victim line.  */
    return victimLine;
}


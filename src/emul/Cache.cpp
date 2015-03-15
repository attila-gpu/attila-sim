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
 * $RCSfile: Cache.cpp,v $
 * $Revision: 1.10 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:15 $
 *
 * Cache class implementation file.
 *
 */

/**
 *
 * @file Cache.cpp
 *
 * Implements the Cache class.  This class implements a generic
 * and configurable memory cache.
 *
 */


#include "Cache.h"
#include "GPUMath.h"
#include <cstdio>
#include <cstring>

using namespace gpu3d;

/*  Cache Constructor.  */
Cache::Cache(u32bit ways, u32bit lines, u32bit bytesLine,
    CacheReplacePolicy repPolicy) :

    numWays(ways), numLines(lines), lineSize(bytesLine),
    replacePolicy(repPolicy)
{
    u32bit i, j;

    /*  Check number of lines.  */
    GPU_ASSERT(
        if (numLines == 0)
            panic("Cache", "Cache", "At least a line per way is required.");
    )

    /*  Check number of ways.  */
    GPU_ASSERT(
        if (numWays == 0)
            panic("Cache", "Cache", "At least a way is required.");
    )

    /*  Check line size.  */
    GPU_ASSERT(
        if (lineSize == 0)
            panic("Cache", "Cache", "At least a byte per line is required.");
    )

    /*  Allocate cache ways.  */
    cache = new u8bit**[numWays];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (cache == NULL)
            panic("Cache", "Cache", "Error allocating pointer to the ways.");
    )

    /*  Allocate tag file for the ways.  */
    tags = new u32bit*[numWays];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (tags == NULL)
            panic("Cache", "Cache", "Error allocating pointers for the ways tag files.");
    )

    /*  Allocate the valid bit for ways.  */
    valid = new bool*[numWays];

    GPU_ASSERT(
        if (valid == NULL)
            panic("Cache", "Cache", "Error allocating the pointers for the ways valid bits.");
    )

    /*  Allocate cache memory, tags and valid bits.  */
    for (i = 0; i < numWays; i++)
    {
        /*  Allocate the way cache lines.  */
        cache[i] = new u8bit*[numLines];

        /*  Cehck allocation.  */
        GPU_ASSERT(
            if (cache[i] == NULL)
                panic("Cache", "Cache", "Error allocating line pointers for the way.");
        )

        /*  Allocate tag file for the way lines.  */
        tags[i] = new u32bit[numLines];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (tags[i] == NULL)
                panic("Cache", "Cache", "Error allocating tag file for the lines in a way.");
        )

        /*  Allocate the valid bits  for the way lines.  */
        valid[i] = new bool[numLines];

        GPU_ASSERT(
            if (valid[i] == NULL)
                panic("Cache", "Cache", "Error allocating the valid bits for the lines in a way.");
        )

        /*  Allocate cache lines.  */
        for (j = 0; j < numLines; j++)
        {
            /*  Allocate line.  */
            cache[i][j] = new u8bit[lineSize];

            /*  Check memory allocation.  */
            GPU_ASSERT(
                if (cache[i][j] == NULL)
                    panic("Cache", "Cache", "Error allocating cache line.");
            )

            /*  Reset valid bit for the line.  */
            valid[i][j] = FALSE;
        }
    }

    /*  Calculate masks and shifts.  */
    byteMask = GPUMath::buildMask(lineSize);
    lineMask = GPUMath::buildMask(numLines);
    lineShift = GPUMath::calculateShift(lineSize);

    /*  Check for fully associative cache.  */
    if (numLines == 1)
        tagShift = lineShift;
    else
        tagShift = GPUMath::calculateShift(numLines) + lineShift;

    /*  Create replacemente policy object.  */
    switch(repPolicy)
    {
        case CACHE_DIRECT:
            panic("Cache", "Cache", "Not Implemented.");
            break;

        case CACHE_LRU:

            /*  LRU replacemente policy.  */
            policy = new LRUPolicy(numWays, numLines);
            break;

        case CACHE_PSEUDOLRU:

            /*  Pseudo LRU replacement policy.  */
            policy = new PseudoLRUPolicy(numWays, numLines);
            break;

        case CACHE_FIFO:

            /*  FIFO replacement policy.  */
            policy = new FIFOPolicy(numWays, numLines);
            break;

        case CACHE_NONE:

            /*  No replacement policy defined.  */
            policy = NULL;

            break;

        default:
            panic("Cache", "Cache", "Unsupported cache replacemente policy.");
            break;
    }
}

/*  Returns the line tag for the given address.  */
u32bit Cache::tag(u32bit address)
{
    return (address >> tagShift);
}

/*  Returns the offset inside a line for a given address.  */
u32bit Cache::offset(u32bit address)
{
    return (address & byteMask);
}

/*  Calculates the address of a line stored in the cache.  */
u32bit Cache::line2address(u32bit way, u32bit line)
{
    return (((tags[way][line] << (tagShift - lineShift)) + line) << lineShift);
}

/*  Search if a requested address is in the cache.  */
bool Cache::search(u32bit address, u32bit &line, u32bit &way)
{
    u32bit wayAux;
    bool found;

    /*  Check for fully associative cache.  */
    if (numLines == 1)
    {
        /*  Fully associative cache, no line index used.  */
        line = 0;
    }
    else
    {
        /*  Get the line index from the address.  */
        line = (address >> lineShift) & lineMask;
    }

//if ((address >> tagShift) == (0x00ae6000 >> tagShift))
//printf("Cache (%p) => search address %x in line %d\n", this, address, line);

    /*  Search the line in the different ways for the line index.  */
    for(wayAux = 0, found = FALSE; (wayAux < numWays) && !found; wayAux++)
    {
//if ((address >> tagShift) == (0x00ae6000 >> tagShift))
//printf("searching -> wayAux = %d tag = %x addressTag = %x valid = %d\n", wayAux, tags[wayAux][line], address >> tagShift, valid[wayAux][line]);
        found = ((tags[wayAux][line] == (address >> tagShift)) && valid[wayAux][line]);
    }
    
    /*  Adjust way.  */
    way = wayAux - 1;

    /*  Check hit.  */
    return found;
}

/*  Cache read function.  */
bool Cache::read(u32bit address, u32bit &data)
{
    u32bit line;
    u32bit way;

    /*  Check hit.  */
    if (search(address, line, way))
    {
        /*  Read the data.  */
        data = *((u32bit *) &cache[way][line][address & byteMask]);

        /*  Check if replacement policy is defined.  */
        if (policy != NULL)
        {
            /*  Update replacemente algorithm.  */
            policy->access(way, line);
        }

        /*  Return a hit.  */
        return TRUE;
    }
    else
    {
        /*  Return a miss.  */
        return FALSE;
    }
}

/*  Cache write function.  */
bool Cache::write(u32bit address, u32bit &data)
{
    u32bit line;
    u32bit way;

    /*  Check hit.  */
    if (search(address, line, way))
    {
        /*  Write the data.  */
        *((u32bit *) &cache[way][line][address & byteMask]) = data;

        /*  Check if replacement policy defined.  */
        if (policy != NULL)
        {
            /*  Update replacemente algorithm.  */
            policy->access(way, line);
        }

        /*  Return a hit.  */
        return TRUE;
    }
    else
    {
        /*  Return a miss.  */
        return FALSE;
    }

}

/*  Cache victim line selection function.  */
u32bit Cache::selectVictim(u32bit address)
{
    u32bit line;
    u32bit way;

    /*  Check for fully associative cache.  */
    if (numLines == 1)
    {
        /*  Fully associative cache, no line index used.  */
        line = 0;
    }
    else
    {
        /*  Get the line index from the address.  */
        line = (address >> lineShift) & lineMask;
    }

    /*  First search the ways in the line index for an invalid line.  */
    for(way = 0;(way < numWays) && valid[way][line]; way++);

    /*  Check if an invalid line was found for the line index.  */
    if (way == numWays)
    {
        /*  Check if replacement policy is defined.  */
        if (policy != NULL)
        {
            /*  Ask the replacement protocol for a line.  */
            return policy->victim(line);
        }
        else
        {
            panic("Cache", "selectVictim", "No replacement policy defined.");
        }
    }
    // else /*  Use the invalid line.  */
    return way;
}

/*  Cache line replace function.  */
void Cache::replace(u32bit address, u32bit way, u8bit *data)
{
    u32bit line;

    GPU_ASSERT(
        if (way >= numWays)
            panic("Cache", "replace", "Out of range cache way number.");
    )

    /*  Check for fully associative cache.  */
    if (numLines == 0)
    {
        /*  There is only a cache line per way.  */
        line = 0;
    }
    else
    {
        /*  Get the line in the way from the address.  */
        line = (address >> lineShift) & lineMask;
    }

    /*  Copy the data.  */
    memcpy(cache[way][line], data, lineSize);

    /*  Set the tag for the line.  */
    tags[way][line] = (address >> tagShift);

    /*  Set the valid bit for the line.  */
    valid[way][line] = TRUE;

    /*  Check if replacement policy is defined.  */
    if (policy != NULL)
    {
        /*  Update replacement algorithm.  */
        policy->access(way, line);
    }
}

/*  Cache line replace function.  This version does not fill the line.  */
void Cache::replace(u32bit address, u32bit way)
{
    u32bit line;

    GPU_ASSERT(
        if (way >= numWays)
            panic("Cache", "replace", "Out of range cache way number.");
    )

    /*  Check for fully associative cache.  */
    if (numLines == 0)
    {
        /*  There is only a cache line per way.  */
        line = 0;
    }
    else
    {
        /*  Get the line in the way from the address.  */
        line = (address >> lineShift) & lineMask;
    }

    /*  Set the tag for the line.  */
    tags[way][line] = (address >> tagShift);

    /*  Set the valid bit for the line.  */
    valid[way][line] = TRUE;

    /*  Check if replacement policy is defined.  */
    if (policy != NULL)
    {
        /*  Update replacement algorithm.  */
        policy->access(way, line);
    }
}

/*  Fill a cache line.  Does not updates the replacement algorithm.  */
void Cache::fill(u32bit address, u8bit *data)
{
    u32bit line;
    u32bit way;

    /*  Check hit.  */
    if (search(address, line, way))
    {
        /*  Copy the data.  */
        memcpy(cache[way][line], data, lineSize);
    }
    else
        panic("Cache", "fill", "Trying to fill a non allocated line.");
}

/*  Invalidates a line if found in the cache.  */
void Cache::invalidate(u32bit address)
{
    u32bit line;
    u32bit way;

    /*  Check hit.  */
    if (search(address, line, way))
    {
        /*  Invalidate line.  */
        valid[way][line] = FALSE;
    }
}

/*  Resets the cache.  Invalidates all the cache lines.  */
void Cache::reset()
{
    u32bit i, j;

    for(i = 0; i < numWays; i++)
    {
        for(j = 0; j < numLines; j++)
        {
            tags[i][j] = 0;
            valid[i][j] = FALSE;
        }
    }
}

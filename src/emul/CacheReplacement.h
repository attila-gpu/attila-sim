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
 * $RCSfile: CacheReplacement.h,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-06-28 10:23:28 $
 *
 * Cache replacement policy class definition file.
 *
 */

/**
 *
 * @file CacheReplacement.h
 *
 * Defines the Cache Replace Policy classes.  This classes define how a line
 * can be selected for replacing in a cache.
 *
 */


#include "support.h"
#include "GPUTypes.h"

#ifndef _CACHEREPLACEMENT_

#define _CACHEREPLACEMENT_

namespace gpu3d
{

/**
 *
 *  Defines the replacement policies for the cache.
 *
 */

enum CacheReplacePolicy
{
    CACHE_DIRECT,       /**<  Direct mapping.  */
    CACHE_FIFO,         /**<  FIFO replacement policy.  */
    CACHE_LRU,          /**<  LRU replacement policy.  */
    CACHE_PSEUDOLRU,    /**<  Pseudo LRU replacement policy.  */
    CACHE_NONE          /**<  No predefined replacement policy.  */
};



/**
 *
 *  This abstract class defines classes that implement
 *  replacement policies for a cache.
 *
 *
 */

class CacheReplacementPolicy
{
protected:

    u32bit numBias;     /**<  Number of bias/ways in the cache.  */
    u32bit numLines;    /**<  Number of lines per bia/way in the cache.  */

public:

    /**
     *
     *  Constructor.
     *
     *  @param numBias Number of bias in the cache.
     *  @param numLines Number of lines per bia/way in the cache.
     *
     *  @return An initialized Cache Replacement Policy object.
     *
     */

    CacheReplacementPolicy(u32bit numBias, u32bit numLines);

    /**
     *
     *  Virtual function.
     *
     *  Updates the replacement policy state with a new
     *  access to a line and bia.
     *
     *  @param bia The bia that was accessed.
     *  @param line The line in the bia that was accessed.
     *
     */

    virtual void access(u32bit bia, u32bit line) = 0;

    /**
     *
     *  Virtual function.
     *
     *  Updates the replacement policy state and returns
     *  the next victim for a line.
     *
     *  @param line The line index for which to select a
     *  victim
     *
     *  @return The bia for the line index to be replaced.
     *
     */

    virtual u32bit victim(u32bit line) = 0;

};

/*  Replacement policy classes.  */

/**
 *
 *  This class implements a LRU replacement policy for a
 *  cache.
 *
 */

class LRUPolicy : public CacheReplacementPolicy
{
private:

    u32bit **accessOrder;    /**<  Stores the bias for a line index sorted by last access.  */

public:

    /**
     *
     *  Creates a LRU policy object.
     *
     *  @param numBias Number of bias in the cache.
     *  @param numLines Number of lines per bia/way in the cache.
     *
     *  @return An initialized LRU Policy object.
     *
     */

    LRUPolicy(u32bit numBias, u32bit numLines);

    /**
     *
     *  Updates the replacement policy state with a new
     *  access to a line and bia.
     *
     *  @param bia The bia that was accessed.
     *  @param line The line in the bia that was accessed.
     *
     */

    void access(u32bit bia, u32bit line);

    /**
     *
     *  Updates the replacement policy state and returns
     *  the next victim for a line.
     *
     *  @param line The line index for which to select a
     *  victim
     *
     *  @return The bia for the line index to be replaced.
     *
     */

    u32bit victim(u32bit line);
};

/**
 *
 *  This class implements a Pseudo LRU replacement policy for a
 *  cache.
 *
 */

class PseudoLRUPolicy : public CacheReplacementPolicy
{
private:

    u32bit *lineState;      /**<  Current state per line index.  */
    u32bit biaShift;        /**<  Shift for the bias, or number of bits that define a bia.  */

public:

    /**
     *
     *  Creates a Pseudo LRU policy object.
     *
     *  @param numBias Number of bias in the cache.
     *  @param numLines Number of lines per bia/way in the cache.
     *
     *  @return An initialized LRU Policy object.
     *
     */

    PseudoLRUPolicy(u32bit numBias, u32bit numLines);

    /**
     *
     *  Updates the replacement policy state with a new
     *  access to a line and bia.
     *
     *  @param bia The bia that was accessed.
     *  @param line The line in the bia that was accessed.
     *
     */

    void access(u32bit bia, u32bit line);

    /**
     *
     *
     *  Updates the replacement policy state and returns
     *  the next victim for a line.
     *
     *  @param line The line index for which to select a
     *  victim
     *
     *  @return The bia for the line index to be replaced.
     *
     */

    u32bit victim(u32bit line);
};

/**
 *
 *  This class implements a FIFO replacement policy for a
 *  cache.
 *
 */

class FIFOPolicy : public CacheReplacementPolicy
{
private:

    u32bit *next;       /**<  Next victim per line index.  */

public:

    /**
     *
     *  Creates a FIFO policy object.
     *
     *  @param numBias Number of bias in the cache.
     *  @param numLines Number of lines per bia/way in the cache.
     *
     *  @return An initialized LRU Policy object.
     *
     */

    FIFOPolicy(u32bit numBias, u32bit numLines);

    /**
     *
     *  Updates the replacement policy state with a new
     *  access to a line and bia.
     *
     *  @param bia The bia that was accessed.
     *  @param line The line in the bia that was accessed.
     *
     */

    void access(u32bit bia, u32bit line);

    /**
     *
     *
     *  Updates the replacement policy state and returns
     *  the next victim for a line.
     *
     *  @param line The line index for which to select a
     *  victim
     *
     *  @return The bia for the line index to be replaced.
     *
     */

    u32bit victim(u32bit line);
};

} // namespace gpu3d

#endif

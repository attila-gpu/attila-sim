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
 * $RCSfile: Cache64.h,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2006-05-11 18:10:39 $
 *
 * Cache class definition file.
 *
 */

/**
 *
 * @file Cache64.h
 *
 * Defines the Cache class.  This class defines a generic and configurable
 * memory cache model.  Version for 64 bit addresses.
 *
 */


#include "support.h"
#include "GPUTypes.h"
#include "CacheReplacement.h"

#ifndef _CACHE64_

#define _CACHE64_


namespace gpu3d
{

/**
 *
 *  The Cache class defines and implements a generic and
 *  configurable cache model.  Version for 64 bit addresses.
 *
 */

class Cache64
{

private:

    /*  Cache parameters.  */
    CacheReplacePolicy replacePolicy;   /**<  Replacement policy for the cache.  */

    /*  Cache masks and shifts.  */
    u32bit byteMask;        /**<  Address line byte mask.  */
    u32bit lineMask;        /**<  Address line mask.  */
    u32bit lineShift;       /**<  Address line shift.  */
    u32bit tagShift;        /**<  Address tag shift.  */

    /*  Cache replacement mechanism.  */
    CacheReplacementPolicy *policy;      /**<  Cache replacement policy.  */

protected:

    /*  Cache parameters.  */
    u32bit numWays;         /**<  Number of ways in the cache.  */
    u32bit numLines;        /**<  Number of lines in the cache (total).  */
    u32bit lineSize;        /**<  Size of a cache line in bytes.  */

    /*  Cache structures.  */
    u8bit ***cache;         /**<  Cache memory.  */
    bool **valid;           /**<  Valid line bit.  */
    u64bit **tags;          /**<  Cache tag file.  */


    /**
     *
     *  Search if a requested address is in the cache.
     *
     *  @param address Address to search in the cache tag file.
     *  @param line Reference to a variable where to store the
     *  cache line for the address.
     *  @param way Reference to a variable where to store the
     *  cache way for the address.
     *
     *  @return If the address was found in the cache tag file.
     *
     */

    bool search(u64bit address, u32bit &line, u32bit &way);


    /**
     *
     *  Calculates the line tag for an address.
     *
     *  @param address The address for which to calcultate the tag.
     *
     *  @return The calculated tag.
     *
     */

    u64bit tag(u64bit address);

    /**
     *
     *  Retrieves the address of the first byte of the specified line.
     *
     *  @param way The way of the cache line for which to get the memory
     *  address.
     *  @param line The line of the cache line for which to get the memory
     *  address.
     *
     *  @return The memory address of the line stored in the
     *  cache line specified.
     *
     */

    u64bit line2address(u32bit way, u32bit line);

    /**
     *
     *  Calculates the offset inside of a cache line for a given address.
     *
     *  @param address Address for which to calculate the line offset.
     *
     *  @return The calculated line offset.
     *
     */

    u32bit offset(u64bit address);

public:

    /**
     *
     *  Cache constructor.
     *
     *  Creates and initializes a Cache object.
     *
     *  @param numWays Number of ways in the cache.
     *  @param numLines Number of lines in the cache.
     *  @param bytesLine Line size in bytes.
     *  @param replacePolicy Replacement policy for the cache.
     *
     *  @return A new initialized cache object.
     *
     */

    Cache64(u32bit numWays, u32bit numLines, u32bit bytesLine,
        CacheReplacePolicy replacePolicy);

    /**
     *
     *  Reads a 32 bit word from the cache.
     *
     *  @param address Address (64 bits) of the word to be read
     *  from the cache.
     *  @param data Reference to the variable where to store the
     *  read 32 bit word.
     *
     *  @return TRUE if the address was found (hit) in the cache and
     *  data was returned, FALSE if the address was not found in the
     *  cache (miss).
     *
     */

    bool read(u64bit address, u32bit &data);

    /**
     *
     *  Writes a 32 bit word to the cache.
     *
     *  @param address Address (64 bits) of the word to be written
     *  in the cache.
     *  @param data Reference to the variable where is the data
     *  to be written to the cache.
     *
     *  @return TRUE if the address was in the cache (hit), FALSE
     *  if the address was not in the cache (miss).
     *
     */

     bool write(u64bit address, u32bit &data);


    /**
     *
     *  Selects a way for the line associated to the address
     *  as a victim for replacement.
     *
     *  @param address Address (64 bits) for which a line
     *  is needed.
     *
     *  @return The victim way for the line defined by the
     *  input address.
     *
     */

    u32bit selectVictim(u64bit address);

    /**
     *
     *  Replaces and fills a cache line.
     *
     *  @param address Address of the new line being loaded in the
     *  cache.
     *  @param way Way for the line defined by the address in which
     *  the data is going to be stored.
     *  @param data Pointer to a byte array with the new line data.
     *
     */

    void replace(u64bit address, u32bit way, u8bit *data);

    /**
     *
     *  Replaces a cache line.
     *
     *  Only updates the tag file, valid bit and replacement
     *  policy algorithm.  The cache line is not filled with
     *  the replacing line data.
     *
     *  @param address Address of the line replacing the cache
     *  line.
     *  @param way Way index for the cache line being replaced
     *  (for n-associative caches).
     *
     */

    void replace(u64bit address, u32bit way);

    /**
     *
     *  Fills a cache line.
     *
     *  Only fills the cache line with data, tag file, valid bit
     *  and replacement policy algorithm are not changed.
     *  The line being filled must already exist in the cache, if
     *  not the function causes a panic exit.
     *
     *  @param address Address of the line to be filled with data
     *  in the cache.
     *  @param data Pointer to a byte array of the size of a cache
     *  line with the data for filling the cache line.
     *
     */

    void fill(u64bit address, u8bit *data);

    /**
     *
     *  Invalidates the cache line that contains the parameter
     *  address.
     *
     *  @param address An address inside the line to invalidate.
     *
     */

    void invalidate(u64bit address);

    /**
     *
     *  Resets the cache.
     *
     *  Invalidates all the cache lines.
     *
     */

    void reset();

};

} // namespace gpu3d

#endif

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
 * $RCSfile: TextureCacheGen.h,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:44 $
 *
 * Texture Cache class definition file.
 *
 */

/**
 *
 *  @file TextureCacheGen.h
 *
 *  Defines the Texture Cache class.  This class defines the cache used
 *  to access texture data.
 *
 */


#ifndef _TEXTURECACHEGEN_

#define _TEXTURECACHEGEN_

#include "GPUTypes.h"
#include "MemoryTransaction.h"
#include <string>

namespace gpu3d
{

/**
 *
 *  This class describes and implements the behaviour of the cache
 *  used to get texture data from memory.
 *  The texture cache is used in the Texture Unit GPU unit.
 *
 *
 */

class TextureCacheGen
{

private:


public:

    /**
     *
     *  Texture Cache constructor.
     *
     *  Creates and initializes a Texture Cache object.
     *
     *  @return A new initialized cache object.
     *
     */

    TextureCacheGen();

    /**
     *
     *  Reserves and fetches (if the line is not already available) the cache line for the requested address.
     *
     *  Pure virtual.  The derived class must implement this function.
     *
     *  @param address The address in GPU memory of the data to read.
     *  @param way Reference to a variable where to store the way in which the data for the
     *  fetched address will be stored.
     *  @param line Reference to a variable where to store the cache line where the fetched data will be stored.
     *  @param tag Reference to a variable where to store the line tag to wait for the fetched address.
     *  @param ready Reference to a variable where to store if the data for the address is already available.
     *  @param source Pointer to a DynamicObject that is the source of the fetch request.
     *
     *  @return If the line for the address could be reserved and fetched (ex. all line are already reserved).
     *
     */

    virtual bool fetch(u64bit address, u32bit &way, u32bit &line, u64bit &tag, bool &ready, DynamicObject *source) = 0;

   /**
     *
     *  Reads texture data data from the texture cache.
     *  The line associated with the requested address must have been previously fetched, if not an error
     *  is produced.
     *
     *  Pure virtual.  The derived class must implement this function.
     *
     *  @param address Address of the data in the texture cache.
     *  @param way Way where the data for the address is stored.
     *  @param line Cache line where the data for the address is stored.
     *  @param size Amount of bytes to read from the texture cache.
     *  @param data Pointer to an array where to store the read color data for the stamp.
     *
     *  @return If the read could be performed (ex. line not yet received from memory).
     *
     */

    virtual bool read(u64bit address, u32bit way, u32bit line, u32bit size, u8bit *data) = 0;

    /**
     *
     *  Unreserves a cache line.
     *
     *  Pure virtual.  The derived class must implement this function.
     *
     *  @param way The way of the cache line to unreserve.
     *  @param line The cache line to unreserve.
     *
     */

    virtual void unreserve(u32bit way, u32bit line) = 0;

    /**
     *
     *  Resets the Input Cache structures.
     *
     *  Pure virtual.  The derived class must implement this function.
     *
     *
     */

    virtual void reset() = 0;

    /**
     *
     *  Process a received memory transaction from the Memory Controller.
     *
     *  Pure virtual.  The derived class must implement this function.
     *
     *  @param memTrans Pointer to a memory transaction.
     *
     */

    virtual void processMemoryTransaction(MemoryTransaction *memTrans) = 0;

    /**
     *
     *  Updates the state of the memory request queue.
     *
     *  Pure virtual.  The derived class must implement this function.
     *
     *  @param cycle Current simulation cycle.
     *  @param memoryState Current state of the Memory Controller.
     *  @param filled Reference to a boolean variable where to store if a cache line
     *  was filled this cycle.
     *  @param tag Reference to a variable where to store the tag for the cache line
     *  filled in the current cycle.
     *
     *  @return A new memory transaction to be issued to the
     *  memory controller.
     *
     */

    virtual MemoryTransaction *update(u64bit cycle, MemState memoryState, bool &filled, u64bit &tag) = 0;

    /**
     *
     *  Simulates a cycle of the texture cache.
     *
     *  Pure virtual.  The derived class must implement this function.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    virtual void clock(u64bit cycle) = 0;

    /**
     *
     *  Writes into a string a report about the stall condition of the box.
     *
     *  @param cycle Current simulation cycle.
     *  @param stallReport Reference to a string where to store the stall state report for the box.
     *
     */
     
    virtual void stallReport(u64bit cycle, std::string &stallReport) = 0;

    /**
     *
     *  Enables or disables debug output for the texture cache.
     *
     *
     *  @param enable Boolean variable used to enable/disable debug output for the TextureCache.
     *
     */

    virtual void setDebug(bool enable) = 0;
};

} // namespace gpu3d

#endif

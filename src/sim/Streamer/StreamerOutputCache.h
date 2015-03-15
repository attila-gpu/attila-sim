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
 * $RCSfile: StreamerOutputCache.h,v $
 * $Revision: 1.7 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:54 $
 *
 * Streamer Output Cache class definition file.
 *
 */


/**
 *
 *  @file StreamerOutputCache.h
 *
 *  This file contains the definition of the Streamer Output Cache
 *  box.
 *
 */


#ifndef _STREAMEROUTPUTCACHE_

#define _STREAMEROUTPUTCACHE_

namespace gpu3d
{
    class StreamerOutputCache;
} // namespace gpu3d;


#include "GPUTypes.h"
#include "Box.h"
#include "Streamer.h"
#include "StreamerCommand.h"
#include "StreamerControlCommand.h"

namespace gpu3d
{

/**
 *
 *  This class implements the Streamer Output Cache box.
 *
 *  The Streamer Output Cache receives new input indexes from
 *  the Streamer Fetch and look for them in the output cache.
 *  Then sends the new index, a hit/miss flag and the output
 *  memory line allocated for the index to the Streamer Commit
 *  and Streamer Loader.
 *
 *  This class inherits from the Box class that offers basic
 *  simulation support.
 *
 */

class StreamerOutputCache : public Box
{

private:

    /*  Streamer Output Cache signals.  */
    Signal *streamerCommand;        /**<  Command signal from the Streamer main box.  */
    Signal *streamerFetchNewIndex;  /**<  New Index signal from the Streamer Fetch.  */
    Signal *streamerCommitNewIndex; /**<  New Index signal to the Streamer Commit.  */
    Signal **streamerLoaderNewIndex;/**<  New Index signal to the Streamer Loader Units.  */
    Signal *streamerCommitDeAlloc;  /**<  Deallocation signal from the Streamer Commit.  */
    Signal *streamerOCUpdateWrite;  /**<  Streamer Output Cache update signal to the Streamer Output Cache.  */
    Signal *streamerOCUpdateRead;   /**<  Streamer Output Cache update signal from the Streamer Output Cache.  */

    /*  Streamer Output Cache parameters.  */
    u32bit indicesCycle;            /**<  Indices received per cycle from Streamer Fetch.  */
    u32bit outputMemorySize;        /**<  Size in lines (outputs) of the output memory.  */
    u32bit verticesCycle;           /**<  Vertices per cycle commited and sent to Primitive Assembly by Streamer Commit.  */
    u32bit streamerLoaderUnits;     /**<  Number of Streamer Loader Units. */
    u32bit slIndicesCycle;          /**<  Indices consumed each cycle per Streamer Loader Unit. */

    /*  Streamer Output Cache state.  */
    StreamerState state;                /**<  State of the streamer output cache.  */
    StreamerCommand *lastStreamCom;     /**<  Last streamer command received.  For signal tracing.  */
    StreamerControlCommand **indexReqQ; /**<  Array storing the indices requested in the current cycle by Streamer Fetch.  */
    u32bit indicesRequested;            /**<  Number of indices requested to the cache in the current cycle.  */
    u32bit **missOutputMLines;          /**<  Array storing the output cache line allocated for index misses already processed in the current cycle.  */
    u32bit solvedMisses;                /**<  Number of misses already solved (excluded repeated misses).  */

    /*  Streamer Output Cache structures.  */
    u32bit *outputCacheTagsIndex;       /**<  Output cache tags (index).  */
    u32bit *outputCacheTagsInstance;    /**<  Output cache tags (instance index).  */
    bool *outputCacheValidBit;          /**<  Output cache valid bit.  */
    u32bit *outputMemoryFreeList;       /**<  List of free output memory lines.  */
    u32bit freeOutputMemoryLines;       /**<  Number of free output memory lines.  */
    u32bit nextFreeOMLine;              /**<  Pointer to the next free output memory line in the output memory free list.  */
    u32bit **unconfirmedOMLineDeAllocs; /**<  Array of deallocated output memory lines from Streamer Commit pending of confirmation.  */
    u32bit *unconfirmedDeAllocCounters; /**<  Number of deallocated output memory lines from Streamer Commit pending of confirmation.  */
    bool *deAllocConfirmed;             /**<  Array of confirmation status of deallocated lines.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *hits;     /**<  Number of hits in the Streamer Output Cache.  */
    GPUStatistics::Statistic *misses;   /**<  Number of misses in the Streamer Output Cache.  */
    GPUStatistics::Statistic *indices;  /**<  Number of indices sent to Streamer Output Cache.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a streamer command.
     *
     *  @param streamComm The streamer command to process.
     *
     */

    void processStreamerCommand(StreamerCommand *streamCom);

    /**
     *
     *  Searches in the output cache tag table for an index.
     *
     *  @param index The index to search.
     *  @param instance The instance index corresponding to the index to search.
     *  @param omLine Reference to an u32bit variable where to store
     *  the output memory line where the index output is stored
     *  if the index was found.
     *
     *  @return TRUE if the index was found in the tag table,
     *  FALSE otherwise.
     *
     */

    bool outputCacheSearch(u32bit index, u32bit instance, u32bit &omLine);


public:

    /**
     *
     *  Streamer Output Cache box constructor.
     *
     *  @param idxCycle Indices received per cycle from Streamer Fetch.
     *  @param omSize The output memory size in lines.
     *  @param vertCycle Vertices per cycle that Streamer Commit can commit and send to Primitive Assembly.
     *  @param sLoaderUnits Number of Streamer Loader Units.
     *  @param slIdxCycle Indices consumed each cycle per Streamer Loader Unit.
     *  @param name The box name.
     *  @param parent The box parent box.
     *
     *  @return An initialized Streamer Output Cache object.
     *
     */

    StreamerOutputCache(u32bit idxCycle, u32bit omSize, u32bit vertCycle, u32bit sLoaderUnits, 
      u32bit slIdxCycle, char *name, Box *parent);

    /**
     *
     *  Simulation rutine for the Streamer Output Cache box.
     *
     *  @param cycle Cycle to simulate.
     *
     *
     */

    void clock(u64bit cycle);

};

} // namespace gpu3d

#endif

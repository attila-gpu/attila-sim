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
 * $RCSfile: InputCache.h,v $
 * $Revision: 1.9 $
 * $Author: cgonzale $
 * $Date: 2006-06-21 11:43:45 $
 *
 * Input Cache class definition file.
 *
 */

/**
 *
 *  @file InputCache.h
 *
 *  Defines the Input Cache class.  This class defines the cache used
 *  by the Streamer to load vertex inputs from memory.
 *
 */


#ifndef _INPUTCACHE_

#define _INPUTCACHE_

#include "GPUTypes.h"
#include "FetchCache.h"
//#include "MemoryController.h"
#include "MemoryTransaction.h"
#include "StatisticsManager.h"
#include "toolsQueue.h"

namespace gpu3d
{

/*  Defines a block read request to memory.  */
struct InputCacheReadRequest
{
    u32bit address;     /**<  Address to read from.  */
    u32bit size;        /**<  Size of the read request.  */
    u32bit requested;   /**<  Bytes requested to memory.  */
    u32bit received;    /**<  Bytes received from memory.  */
    CacheRequest *request;  /**<  Pointer to the cache request.  */
    u32bit requestID;   /**<  Identifier of the cache request.  */
};

/**
 *
 *  This class describes and implements the behaviour of the cache
 *  used to get vertex input data from memory.
 *  The input cache is used in the Streamer Loader GPU unit.
 *
 *  This class uses the Fetch Cache.
 *
 */

class InputCache
{

private:

    u32bit cacheId;         /**<  The input cache identifier.  */

    /*  Input cache parameters.  */
    u32bit lineSize;        /**<  Size of a cache line/block.  */
    u32bit numPorts;        /**<  Number of read/write ports.  */
    u32bit portWidth;       /**<  Size of the cache read/write ports in bytes.  */
    u32bit inputRequests;   /**<  Number of read requests and input buffers.  */

    /*  Input cache structures.  */
    FetchCache *cache;      /**<  Pointer to the Input Cache fetch cache.  */

    /*  Input cache state.  */
    bool resetMode;                     /**<  Reset mode flag.  */
    CacheRequest *cacheRequest;         /**<  Last cache request received.  */
    u32bit requestID;                   /**<  Current cache request identifier.  */
    MemState memoryState;               /**<  Current state of the memory controller.  */
    u32bit lastSize;                    /**<  Size of the last memory read transaction received.  */
    u32bit readTicket;                  /**<  Ticket of the memory read transaction being received.  */
    bool memoryRead;                    /**<  There is a memory read transaction in progress.  */
    MemoryTransaction *nextTransaction; /**<  Stores the pointer to the new generated memory transaction.  */
    bool writingLine;                   /**<  A full line is being written to the cache.  */

    /*  Memory Structures.  */
    InputCacheReadRequest *readQueue;   /**<  Read request queue.  */
    u32bit inputs;              /**<  Number of input requests.  */
    u32bit nextInput;           /**<  Next input request.  */
    u32bit readInputs;          /**<  Number of inputs already read from the cache.  */
    u32bit nextRead;            /**<  Pointer to the next read input.  */
    u32bit freeReads;           /**<  Number of free entries in the read queue.  */
    u32bit nextFreeRead;        /**<  Pointer to the next free read queue entry.  */
    u32bit inputsRequested;     /**<  Number of input requests requested to memory.  */
    u32bit readsWriting;        /**<  Number of blocks being written to a cache line.  */
    u8bit **inputBuffer;        /**<  Buffer for the cache line being written to the cache.  */
    u32bit memoryRequest[MAX_MEMORY_TICKETS];   /**<  Associates memory tickets (identifiers) to a read queue entry.  */
    tools::Queue<u32bit> ticketList;            /**<  List with the memory tickets available to generate requests to memory.  */
    u32bit freeTickets;                         /**<  Number of free memory tickets.  */

    /*  Timing.  */
    u32bit writeCycles;     /**<  Remaining cycles in the cache write port.  */
    u32bit *readCycles;     /**<  Remaining cycles the cache read port is reserved.  */
    u32bit nextReadPort;    /**<  Next read port.  */
    u32bit memoryCycles;    /**<  Remaining cycles the memory bus is used/reserved.  */

    /*  Statistics.  */
    //GPUStatistics::Statistic *readTrans;    /**<  Read transactions to memory.  */
    //GPUStatistics::Statistic *writeTrans;   /**<  Write transactions to memory.  */
    //GPUStatistics::Statistic *readMem;      /**<  Read bytes from memory.  */
    //GPUStatistics::Statistic *writeMem;     /**<  Write bytes to memory.  */
    //GPUStatistics::Statistic *fetches;      /**<  Fetch operations.  */
    //GPUStatistics::Statistic *fetchOK;      /**<  Successful fetch operations.  */
    //GPUStatistics::Statistic *fetchFail;    /**<  Failed fetch operation.  */
    //GPUStatistics::Statistic *allocOK;      /**<  Successful allocation operation.  */
    //GPUStatistics::Statistic *allocFail;    /**<  Failed allocation operation.  */
    //GPUStatistics::Statistic *readOK;       /**<  Succesful read operation.  */
    //GPUStatistics::Statistic *readFail;     /**<  Failed read operation.  */
    //GPUStatistics::Statistic *writeOK;      /**<  Succesful write operation.  */
    //GPUStatistics::Statistic *writeFail;    /**<  Failed write operation.  */

    /*  Private functions.  */

    /**
     *
     *  Generates a memory transaction requesting data to
     *  the memory controller for the read queue entry.
     *
     *  @param readReq The read queue entry for which to generate
     *  the memory transaction.
     *
     *  @return The memory transaction generated.  If it could not
     *  be generated returns NULL.
     *
     */

    MemoryTransaction *requestBlock(u32bit readReq);

    /**
     *
     *  Generates a write memory transaction for the write queue
     *  entry.
     *
     *  @param writeReq Write queue entry for which to generate
     *  the memory transaction.
     *
     *  @return If a memory transaction could be generated, the
     *  transaction generated, otherwise NULL.
     *
     */

    MemoryTransaction *writeBlock(u32bit writeReq);


public:

    /**
     *
     *  Input Cache constructor.
     *
     *  Creates and initializes a Input Cache object.
     *
     *  @param cacheId The input cache identifier.
     *  @param numWays Number of ways in the cache.
     *  @param numLines Number of lines in the cache.
     *  @param lineSize Bytes per cache line.
     *  @param ports Number of read/write ports.
     *  @param portWidth Width in bytes of the write and read ports.
     *  @param reqQueueSize Input cache request to memory queue size.
     *  @param inputRequests Number of read requests and input buffers.
     *
     *  @return A new initialized cache object.
     *
     */

    InputCache(u32bit cacheId, u32bit numWays, u32bit numLines, u32bit lineSize,
        u32bit ports, u32bit portWidth, u32bit reqQueueSize, u32bit inputRequests);

    /**
     *
     *  Reserves and fetches (if the line is not already available)
     *  the cache line for the requested address.
     *
     *  @param address The address in GPU memory of the data to read.
     *  @param way Reference to a variable where to store the
     *  way in which the data for the fetched address will be stored.
     *  @param line Reference to a variable where to store the
     *  cache line where the fetched data will be stored.
     *  @param source Pointer to a Dynamic Object that is the source of the cache access.
     *
     *
     *  @return If the line for the address could be reserved and
     *  fetched (ex. all line are already reserved).
     *
     */

    bool fetch(u32bit address, u32bit &way, u32bit &line, DynamicObject *source);

   /**
     *
     *  Reads vertex input data data from the input cache.
     *  The line associated with the requested address
     *  must have been previously fetched, if not an error
     *  is produced.
     *
     *  @param address Address of the data in the input cache.
     *  @param way Way where the data for the address is stored.
     *  @param line Cache line where the data for the address
     *  is stored.
     *  @param size Amount of bytes to read from the input cache.
     *  @param data Pointer to an array where to store the read
     *  color data for the stamp.
     *
     *  @return If the read could be performed (ex. line not
     *  yet received from memory).
     *
     */

    bool read(u32bit address, u32bit way, u32bit line, u32bit size, u8bit *data);

    /**
     *
     *  Unreserves a cache line.
     *
     *  @param way The way of the cache line to unreserve.
     *  @param line The cache line to unreserve.
     *
     */

    void unreserve(u32bit way, u32bit line);

    /**
     *
     *  Resets the Input Cache structures.
     *
     *
     */

    void reset();

    /**
     *
     *  Process a received memory transaction from the
     *  Memory Controller.
     *
     *  @param memTrans Pointer to a memory transaction.
     *
     */

    void processMemoryTransaction(MemoryTransaction *memTrans);

    /**
     *
     *  Updates the state of the memory request queue.
     *
     *  @param cycle Current simulation cycle.
     *  @param memoryState Current state of the Memory Controller.
     *
     *  @return A new memory transaction to be issued to the
     *  memory controller.
     *
     */

    MemoryTransaction *update(u64bit cycle, MemState memoryState);

    /**
     *
     *  Simulates a cycle of the input cache.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void clock(u64bit cycle);

};

} // namespace gpu3d

#endif

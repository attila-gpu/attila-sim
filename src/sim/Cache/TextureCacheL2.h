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
 * $RCSfile: TextureCacheL2.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2006-06-21 11:43:45 $
 *
 * Texture Cache class definition file.
 *
 */

/**
 *
 *  @file TextureCacheL2.h
 *
 *  Defines the Texture Cache class.  This class defines the cache used
 *  to access texture data.
 *
 */


#ifndef _TEXTURECACHE2_

#define _TEXTURECACHE2_

#include "GPUTypes.h"
#include "FetchCache64.h"
#include "FetchCache.h"
//#include "MemoryController.h"
#include "MemoryTransaction.h"
#include "StatisticsManager.h"
#include "TextureCacheGen.h"
#include "toolsQueue.h"
#include <string>

namespace gpu3d
{

/*  Defines a block read request to memory.  */
struct TextureCacheL2ReadRequest
{
    u64bit address;     /**<  Address to read from in the texture address space.  */
    u32bit memAddress;  /**<  Address in the GPU memory address space.  */
    u32bit size;        /**<  Size of the read request.  */
    u32bit requested;   /**<  Bytes requested to memory.  */
    u32bit received;    /**<  Bytes received from memory.  */
    Cache64Request *request;    /**<  Pointer to the cache request.  */
    CacheRequest *requestL1;    /**<  Pointer to the cache request.  */
    u32bit requestID;   /**<  Identifier of the cache request.  */
    u32bit way;         /**<  Way in the L1 cache for the request data.  */
    u32bit line;        /**<  Line (inside way) in the L1 cache for the request data.  */
};

/**
 *
 *  This class describes and implements the behaviour of the cache
 *  used to get texture data from memory.
 *  The texture cache is used in the Texture Unit GPU unit.
 *
 *  This class uses the Fetch Cache.
 *
 */

class TextureCacheL2 : public TextureCacheGen
{

private:

    /*  Texture cache identification.  */
    static u32bit cacheCounter;     /**<  Class variable used to count and create identifiers for the created Texture Caches.  */
    u32bit cacheID;                 /**<  Identifier of the texture cache.  */

    /*  Texture cache parameters.  */
    u32bit lineSizeL0;      /**<  Size of a cache line/block in L0.  */
    u32bit portWidth;       /**<  Size of the cache read/write ports in bytes.  */
    u32bit inputRequestsL0; /**<  Number of pending read requests and input buffers for L0 cache.  */
    u32bit banks;           /**<  Number of supported banks.  */
    u32bit maxAccesses;     /**<  Number of accesses allowed to a bank in a cycle.  */
    u32bit bankWidth;       /**<  Width in bytes of each bank word.  */
    u32bit maxMisses;       /**<  Number of misses allowed per cycle.  */
    u32bit decomprLatency;  /**<  Cycles it takes to decompress a block.  */
    u32bit lineSizeL1;      /**<  L1 cache line size.  */
    u32bit inputRequestsL1; /**<  Number of pending read requests supported for L1 cache.  */

    /*  Texture cache structures.  */
    FetchCache64 *cacheL0;  /**<  Pointer to the L0 Texture Cache fetch cache.  */
    FetchCache *cacheL1;    /**<  Pointer to the L1 Texture Cache fetch cache.  */

    /*  Texture cache state.  */
    bool resetMode;                 /**<  Reset mode flag.  */
    Cache64Request *cacheRequestL0; /**<  Last cache request for L0.  */
    CacheRequest *cacheRequestL1;   /**<  Last cache request for L1.  */
    u32bit requestIDL0;             /**<  Last cache request identifier for L0.  */
    u32bit requestIDL1;             /**<  Last cache request identifier for L1.  */
    MemState memoryState;           /**<  Current state of the memory controller.  */
    u32bit lastSize;                /**<  Size of the last memory read transaction received.  */
    u32bit readTicket;              /**<  Ticket of the memory read transaction being received.  */
    bool memoryRead;                /**<  There is a memory read transaction in progress.  */
    MemoryTransaction *nextTransaction;     /**<  Stores the pointer to the new generated memory transaction.  */
    bool writingLine;               /**<  A full line is being written to the cache.  */
    bool wasLineFilled;             /**<  Stores if a line was filled in the current cycle.  */
    u64bit notifyTag;               /**<  Stores the next tag to notify as filled.  */
    u64bit lastFilledLineTag;       /**<  Stores the tag for the last line filled.  */

    /*  Memory Structures.  */
    TextureCacheL2ReadRequest *readQueueL0; /**<  Read request queue for L0.  */
    u32bit uncompressedL0;      /**<  Number of compressed requests.  */
    u32bit nextUncompressedL0;  /**<  Pointer to the next compressed request.  */
    u32bit fetchInputsL0;       /**<  Number of L0 input requests waiting for L1 fetch.  */
    u32bit nextFetchInputL0;    /**<  Next L0 input request to be fetched from L1.  */
    u32bit waitReadInputsL0;    /**<  Number of L0 input requests waiting for L1 read.  */
    u32bit nextReadInputL0;     /**<  Next L0 input request to be read from L1.  */
    u32bit readInputsL0;        /**<  Number of L0 inputs already read from cache L1.  */
    u32bit nextReadL0;          /**<  Pointer to the next L0 read input.  */
    u32bit freeReadsL0;         /**<  Number of free entries in the L0 read queue.  */
    u32bit nextFreeReadL0;      /**<  Pointer to the next free L0 read queue entry.  */
    u32bit uncompressingL0;     /**<  Number of blocks being uncompressed.  */
    u32bit readsWritingL0;      /**<  Number of blocks being written to a cache line.  */
    u8bit **inputBufferL0;      /**<  Buffer for the cache line being written to the L0 cache.  */
    u8bit *comprBuffer;         /**<  Auxiliary buffer from where to decompress compressed texture lines.  */

    u32bit memoryRequest[MAX_MEMORY_TICKETS];   /**<  Associates memory tickets (identifiers) to a read queue entry.  */
    u64bit memRStartCycle[MAX_MEMORY_TICKETS];  /**<  Cycle at which a memory request with a given ticket (identifier) was sent to the memory controller.  */
    tools::Queue<u32bit> ticketList;            /**<  List with the memory tickets available to generate requests to memory.  */
    u32bit freeTickets;                         /**<  Number of free memory tickets.  */

    TextureCacheL2ReadRequest *readQueueL1; /**<  L1 read request queue.  */
    u32bit inputsL1;            /**<  Number of L1 input requests.  */
    u32bit nextInputL1;         /**<  Pointer to the next L1 request to be request from memory.  */
    u32bit readInputsL1;        /**<  Number of L1 requests already received from memory.  */
    u32bit inputsRequestedL1;   /**<  Number of L0 input requests requested to memory.  */
    u32bit nextReadInputL1;     /**<  Pointer to the next L1 request to fill L1 cache.  */
    u32bit nextFreeReadL1;      /**<  Pointer to the next free L1 read queue entry.  */
    u32bit freeReadsL1;         /**<  Number of free entries in the L1 read queue.  */
    u8bit **inputBufferL1;      /**<  Buffer the cache lines being written to the L1 cache.  */

    /*  Resource limits.  */
    u32bit *dataBankAccess;     /**<  Number of accesses in the current cycle to each texture cache memory bank.  */
    u32bit *tagBankAccess;      /**<  Number of accesses in the current cycle to each texture cache tag bank.  */
    u32bit cycleMisses;         /**<  Number of misses in the current cycle.  */
    u64bit **fetchAccess;       /**<  Stores the line addresses fetched in a cycle for each texture cache bank, used to eliminate redundant fetches.  */
    u64bit **readAccess;        /**<  Stores the addresses read in a cycle for each texture cache bank, used for data bypasses.  */

    /*  Precalculate values.  */
    u32bit lineShift;           /**<  Binary shift to obtain the line from an address.  */
    u32bit bankShift;           /**<  Binary shift to obtain the bank from an address.  */
    u32bit bankMask;            /**<  Binary mask to obtain the mask from an address.  */
    u32bit readPorts;           /**<  Number of read ports in the Texture Cache.  */

    /*  Timing.  */
    u32bit writeCycles;         /**<  Remaining cycles in the cache write port.  */
    u32bit *readCycles;         /**<  Remaining cycles the cache read port is reserved.  */
    u32bit memoryCycles;        /**<  Remaining cycles the memory bus is used/reserved.  */
    u32bit uncompressCycles;    /**<  Remaining cycles for the compression of the current block.  */

    //  Debug.
    bool debugMode;             /**<  Flag that stores if debug output is enabled.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *fetchBankConflicts;   /**<  Counts bank conflicts when fetching.  */
    GPUStatistics::Statistic *readBankConflicts;    /**<  Counts bank conflicts when reading.  */
    GPUStatistics::Statistic *memRequests;          /**<  Memory requests served by the Memory Controller.  */
    GPUStatistics::Statistic *memReqLatency;        /**<  Latency of the memory requests served by the Memory Controller.  */
    GPUStatistics::Statistic *pendingRequests;      /**<  Number of memory requests issued to the Memory Controller and not served.  */
    GPUStatistics::Statistic *pendingRequestsAvg;   /**<  Average number of memory requests issued to the Memory Controller and not served.  */

    /*  Private functions.  */

    /**
     *
     *  Generates a memory transaction requesting data to the memory controller for the read queue entry.
     *
     *  @param readReq The read queue entry for which to generate the memory transaction.
     *
     *  @return The memory transaction generated.  If it could not be generated returns NULL.
     *
     */

    MemoryTransaction *requestBlock(u32bit readReq);

    /**
     *
     *  Generates a write memory transaction for the write queue entry.
     *
     *  @param writeReq Write queue entry for which to generate the memory transaction.
     *
     *  @return If a memory transaction could be generated, the transaction generated, otherwise NULL.
     *
     */

    MemoryTransaction *writeBlock(u32bit writeReq);


public:

    /**
     *
     *  Texture Cache constructor.
     *
     *  Creates and initializes a Texture Cache object.
     *
     *  @param numWaysL0 Number of ways in the L0 cache.
     *  @param numLinesL0 Number of lines in the L0 cache.
     *  @param lineSizeL0 Bytes per L0 cache line.
     *  @param portWidth Width in bytes of the write and read ports.
     *  @param reqQueueSize Input cache request to memory queue size.
     *  @param inputRequestsL0 Number of read requests and input buffers for L0 cache.
     *  @param numBanks Number of banks in the texture cache.
     *  @param maxAccess Maximum number of accesses to a bank in a cycle.
     *  @param bWidth Width in bytes of each cache bank.
     *  @param maxMiss Maximum number of misses in a cycle.
     *  @param decomprLat Decompression latency for texture compressed blocks.
     *  @param numWaysL1 Number of ways in the L1 cache.
     *  @param numLinesL1 Number of lines in the L1 cache.
     *  @param lineSizeL1 Bytes per L1 cache line.
     *  @param inputRequestsL1 Number of pending read requests for L1 cache.
     *  @param postfix The postfix for the texture cache statistics.
     *
     *  @return A new initialized cache object.
     *
     */

    TextureCacheL2(u32bit numWaysL0, u32bit numLinesL0, u32bit lineSizeL0,
        u32bit portWidth, u32bit reqQueueSize, u32bit inputRequests,
        u32bit numBanks, u32bit numAccess, u32bit bWidth, u32bit maxMiss,
        u32bit decomprLat, u32bit numWaysL1, u32bit numLinesL1, u32bit lineSizeL1,
        u32bit inputRequestsL1, char *postfix);

    /**
     *
     *  Reserves and fetches (if the line is not already available) the cache line for the requested address.
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

    bool fetch(u64bit address, u32bit &way, u32bit &line, u64bit &tag, bool &ready, DynamicObject *source);

   /**
     *
     *  Reads texture data data from the texture cache.
     *  The line associated with the requested address must have been previously fetched, if not an error
     *  is produced.
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

    bool read(u64bit address, u32bit way, u32bit line, u32bit size, u8bit *data);

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
     *  Process a received memory transaction from the Memory Controller.
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
     *  @param filled Reference to a boolean variable where to store if a cache line
     *  was filled this cycle.
     *  @param tag Reference to a variable where to store the tag for the cache line
     *  filled in the current cycle.
     *
     *  @return A new memory transaction to be issued to the
     *  memory controller.
     *
     */

    MemoryTransaction *update(u64bit cycle, MemState memoryState, bool &filled, u64bit &tag);

    /**
     *
     *  Simulates a cycle of the texture cache.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Writes into a string a report about the stall condition of the box.
     *
     *  @param cycle Current simulation cycle.
     *  @param stallReport Reference to a string where to store the stall state report for the box.
     *
     */
     
    void stallReport(u64bit cycle, std::string &stallReport);

    /**
     *
     *  Enables or disables debug output for the texture cache.
     *
     *  @param enable Boolean variable used to enable/disable debug output for the TextureCache.
     *
     */

    void setDebug(bool enable);
};

} // namespace gpu3d

#endif

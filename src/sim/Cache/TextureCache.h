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
 * $RCSfile: TextureCache.h,v $
 * $Revision: 1.12 $
 * $Author: cgonzale $
 * $Date: 2006-06-21 11:43:45 $
 *
 * Texture Cache class definition file.
 *
 */

/**
 *
 *  @file TextureCache.h
 *
 *  Defines the Texture Cache class.  This class defines the cache used
 *  to access texture data.
 *
 */


#ifndef _TEXTURECACHE_

#define _TEXTURECACHE_

#include "GPUTypes.h"
#include "FetchCache64.h"
//#include "MemoryController.h"
#include "MemoryTransaction.h"
#include "StatisticsManager.h"
#include "TextureCacheGen.h"
#include "toolsQueue.h"
#include <string>

namespace gpu3d
{

/*  Defines a block read request to memory.  */
struct TextureCacheReadRequest
{
    u64bit address;     /**<  Address to read from in the texture address space.  */
    u32bit memAddress;  /**<  Address in the GPU memory address space.  */
    u32bit size;        /**<  Size of the read request.  */
    u32bit requested;   /**<  Bytes requested to memory.  */
    u32bit received;    /**<  Bytes received from memory.  */
    Cache64Request *request;    /**<  Pointer to the cache request.  */
    u32bit requestID;   /**<  Identifier of the cache request.  */
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

class TextureCache : public TextureCacheGen
{

private:

    /*  Texture cache identification.  */
    static u32bit cacheCounter;     /**<  Class variable used to count and create identifiers for the created Texture Caches.  */
    u32bit cacheID;                 /**<  Identifier of the texture cache.  */

    /*  Texture cache parameters.  */
    u32bit lineSize;        /**<  Size of a cache line/block.  */
    u32bit portWidth;       /**<  Size of the cache read/write ports in bytes.  */
    u32bit inputRequests;   /**<  Number of read requests and input buffers.  */
    u32bit banks;           /**<  Number of supported banks.  */
    u32bit maxAccesses;     /**<  Number of accesses allowed to a bank in a cycle.  */
    u32bit bankWidth;       /**<  Width in bytes of each bank word.  */
    u32bit maxMisses;       /**<  Number of misses allowed per cycle.  */
    u32bit decomprLatency;  /**<  Cycles it takes to decompress a block.  */

    /*  Texture cache structures.  */
    FetchCache64 *cache;    /**<  Pointer to the Texture Cache fetch cache.  */

    /*  Texture cache state.  */
    bool resetMode;         /**<  Reset mode flag.  */
    Cache64Request *cacheRequest;   /**<  Last cache request received.  */
    u32bit requestID;       /**<  Current cache request identifier.  */
    MemState memoryState;   /**<  Current state of the memory controller.  */
    u32bit lastSize;        /**<  Size of the last memory read transaction received.  */
    u32bit readTicket;      /**<  Ticket of the memory read transaction being received.  */
    bool memoryRead;        /**<  There is a memory read transaction in progress.  */
    MemoryTransaction *nextTransaction;     /**<  Stores the pointer to the new generated memory transaction.  */
    bool writingLine;       /**<  A full line is being written to the cache.  */
    bool wasLineFilled;     /**<  Stores if a line was filled in the current cycle.  */
    u64bit notifyTag;       /**<  Stores the next tag to notify as filled.  */
    u64bit lastFilledLineTag;   /**<  Stores the tag for the last line filled.  */

    /*  Memory Structures.  */
    TextureCacheReadRequest *readQueue; /**<  Read request queue.  */
    u32bit uncompressed;        /**<  Number of compressed requests.  */
    u32bit nextUncompressed;    /**<  Pointer to the next compressed request.  */
    u32bit inputs;              /**<  Number of input requests.  */
    u32bit nextInput;           /**<  Next input request.  */
    u32bit readInputs;          /**<  Number of inputs already read from the cache.  */
    u32bit nextRead;            /**<  Pointer to the next read input.  */
    u32bit freeReads;           /**<  Number of free entries in the read queue.  */
    u32bit nextFreeRead;        /**<  Pointer to the next free read queue entry.  */
    u32bit inputsRequested;     /**<  Number of input requests requested to memory.  */
    u32bit uncompressing;       /**<  Number of blocks being uncompressed.  */
    u32bit readsWriting;        /**<  Number of blocks being written to a cache line.  */
    u8bit **inputBuffer;        /**<  Buffer for the cache line being written to the cache.  */
    u32bit memoryRequest[MAX_MEMORY_TICKETS];   /**<  Associates memory tickets (identifiers) to a read queue entry.  */
    u64bit memRStartCycle[MAX_MEMORY_TICKETS];  /**<  Cycle at which a memory request with a given ticket (identifier) was sent to the memory controller.  */
    tools::Queue<u32bit> ticketList;            /**<  List with the memory tickets available to generate requests to memory.  */
    u32bit freeTickets;                         /**<  Number of free memory tickets.  */
    u8bit *comprBuffer;         /**<  Auxiliary buffer from where to decompress compressed texture lines.  */

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
     *  @param numWays Number of ways in the cache.
     *  @param numLines Number of lines in the cache.
     *  @param lineSize Bytes per cache line.
     *  @param portWidth Width in bytes of the write and read ports.
     *  @param reqQueueSize Input cache request to memory queue size.
     *  @param inputRequests Number of read requests and input buffers.
     *  @param numBanks Number of banks in the texture cache.
     *  @param maxAccess Maximum number of accesses to a bank in a cycle.
     *  @param bWidth Width in bytes of each cache bank.
     *  @param maxMiss Maximum number of misses in a cycle.
     *  @param decomprLat Decompression latency for texture compressed blocks.
     *  @param postfix The postfix for the texture cache statistics.
     *
     *  @return A new initialized cache object.
     *
     */

    TextureCache(u32bit numWays, u32bit numLines, u32bit lineSize,
        u32bit portWidth, u32bit reqQueueSize, u32bit inputRequests,
        u32bit numBanks, u32bit numAccess, u32bit bWidth, u32bit maxMiss,
        u32bit decomprLat, char *postfix);

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

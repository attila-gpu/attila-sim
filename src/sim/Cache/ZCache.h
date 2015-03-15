/**************************************************************************
 *
 * Copyright (c) 2002 - 2004 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: ZCache.h,v $
 * $Revision: 1.11 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:44 $
 *
 * Z Cache class definition file.
 *
 */

/**
 *
 *  @file ZCache.h
 *
 *  Defines the Z Cache class.  This class defines the cache used
 *  to access the depth and stencil buffer in a GPU.
 *
 */


#ifndef _ZCACHE_

#define _ZCACHE_

#include "GPUTypes.h"
#include "FetchCache.h"
#include "ROPCache.h"
#include "MemoryTransaction.h"

namespace gpu3d
{

/*  Defines a block memory write request.  */
struct ZCacheWriteRequest
{
    u32bit address;     /**<  Address to write to.  */
    u32bit block;       /**<  Block for the address.  */
    u32bit size;        /**<  Size of the write request.  */
    u32bit written;     /**<  Bytes already written to memory.  */
    CacheRequest *request;  /**<  Pointer to the cache request.  */
    u32bit requestID;   /**<  Identifier of the cache request.  */
    u32bit readWaiting; /**<  Read queue entry waiting for the write queue to read from the same cache line.  */
    bool isReadWaiting; /**<  Stores if a read queue entry is waiting for this write to finish.  */
};

/*  Defines a block read request to memory.  */
struct ZCacheReadRequest
{
    u32bit address;     /**<  Address to read from.  */
    u32bit block;       /**<  Block for the address.  */
    u32bit size;        /**<  Size of the read request.  */
    u32bit requested;   /**<  Bytes requested to memory.  */
    u32bit received;    /**<  Bytes received from memory.  */
    CacheRequest *request;  /**<  Pointer to the cache request.  */
    u32bit requestID;   /**<  Identifier of the cache request.  */
    bool writeWait;     /**<  The read request must wait for a write request to read the cache line before replacing the cache line itself.  */
};

/*  Defines the states of a Z buffer block.  */
enum ZBlockState
{
    Z_BLOCK_CLEAR,              /**<  The block is in clear state (use clear depth and stencil values).  */
    Z_BLOCK_COMPRESSED_BEST,    /**<  The block is in best compression level.  */
    Z_BLOCK_COMPRESSED_NORMAL,  /**<  The block is in normal compression level.  */
    Z_BLOCK_UNCOMPRESSED        /**<  The block is uncompressed.  */
};


/**
 *
 *  This class describes and implements the behaviour of the cache
 *  used to access the Z buffer in a GPU.
 *  The Z cache is used in the Z Write GPU unit.
 *
 *  This classes derives from the ROP Cache interface class.
 *
 *  This class uses the Fetch Cache.
 *
 */

class ZCache : public ROPCache
{

private:

    /*  Z cache identification.  */
    static u32bit cacheCounter;     /**<  Class variable used to count and create identifiers for the created Z Caches.  */
    u32bit cacheID;                 /**<  Identifier of the Z cache.  */

    /*  Color cache parameters.  */
    u32bit stampBytes;      /**<  Z bytes per stamp.  */
    u32bit lineStamps;      /**<  Stamps per cache line.  */
    u32bit readPorts;       /**<  Number of read ports.  */
    u32bit writePorts;      /**<  Number of write ports.  */
    u32bit portWidth;       /**<  Size of the cache read/write ports in bytes.  */
    u32bit inputRequests;   /**<  Number of read requests and input buffers.  */
    u32bit outputRequests;  /**<  Number of write requests and output buffers.  */
    bool disableCompr;      /**<  Disables compression of the Z blocks.  */
    u32bit maxBlocks;       /**<  Maximun number of supported blocks (cache lines) in the Z buffer.  */
    u32bit blocksCycle;     /**<  Blocks modified (cleared) per cycle.  */
    u32bit comprLatency;    /**<  Cycles it takes to compress a block.  */
    u32bit decomprLatency;  /**<  Cycles it takes to decompress a block.  */

    /*  Z cache derived parameters.  */
    u32bit lineSize;        /**<  Size of a cache line/block.  */
    u32bit blockShift;      /**<  Number of bits to removed from a Z buffer address to retrieve the block number.  */

    /*  Z cache registers.  */
    u32bit zBufferAddress;  /**<  Start address of the color buffer in video memory.  */
    u32bit clearDepth;      /**<  Depth clear value.  */
    u8bit clearStencil;     /**<  Stencil clear value.  */

    /*  Z cache structures.  */
    FetchCache *cache;      /**<  Pointer to the Z Cache fetch cache.  */
    ZBlockState *blockState;    /**<  Stores the current state of the blocks in the Z buffer.  */

    /*  Color cache state.  */
    bool flushRequest;      /**<  Flag for starting the flush.  */
    bool flushMode;         /**<  If the cache is in flush mode.  */
    bool resetMode;         /**<  Reset mode flag.  */
    bool clearMode;         /**<  Clear mode flag.  */
    CacheRequest *cacheRequest; /**<  Last cache request received.  */
    u32bit requestID;       /**<  Current cache request identifier.  */
    MemState memoryState;   /**<  Current state of the memory controller.  */
    u32bit lastSize;        /**<  Size of the last memory read transaction received.  */
    u32bit readTicket;      /**<  Ticket of the memory read transaction being received.  */
    bool memoryRead;        /**<  There is a memory read transaction in progress.  */
    bool memoryWrite;       /**<  There is a memory write transaction in progress.  */
    MemoryTransaction *nextTransaction;     /**<  Stores the pointer to the new generated memory transaction.  */
    bool writingLine;       /**<  A full line is being written to the cache.  */
    bool readingLine;       /**<  A full line is being read from the cache.  */
    bool fetchPerformed;    /**<  Stores if a fetch was performed in the current cycle.  */
    u32bit writeLinePort;   /**<  Port being used to write a cache line.  */
    u32bit readLinePort;    /**<  Port being used to read a cache line.  */

    /*  Memory Structures.  */
    ZCacheWriteRequest *writeQueue; /**<  Write request queue.  */
    ZCacheReadRequest *readQueue;   /**<  Read request queue.  */
    u32bit compressed;          /**<  Number of compressed requests.  */
    u32bit uncompressed;        /**<  Number of uncompressed requests.  */
    u32bit nextCompressed;      /**<  Pointer to the next compressed request.  */
    u32bit nextUncompressed;    /**<  Pointer to the next uncompressed request.  */
    u32bit inputs;              /**<  Number of input requests.  */
    u32bit outputs;             /**<  Number of outputs requests.  */
    u32bit nextInput;           /**<  Next input request.  */
    u32bit nextOutput;          /**<  Next output request.  */
    u32bit readInputs;          /**<  Number of inputs already read from the cache.  */
    u32bit writeOutputs;        /**<  Number of outputs to write to cache.  */
    u32bit nextRead;            /**<  Pointer to the next read input.  */
    u32bit nextWrite;           /**<  Pointer to the next write output.  */
    u32bit freeWrites;          /**<  Number of free entries in the write queue.  */
    u32bit freeReads;           /**<  Number of free entries in the read queue.  */
    u32bit nextFreeRead;        /**<  Pointer to the next free read queue entry.  */
    u32bit nextFreeWrite;       /**<  Pointet to the next free write queue entry.  */
    u32bit inputsRequested;     /**<  Number of input requests requested to memory.  */
    u32bit uncompressing;       /**<  Number of blocks being uncompressed.  */
    u32bit readsWriting;        /**<  Number of blocks being written to a cache line.  */
    u8bit **inputBuffer;        /**<  Buffer for the cache line being written to the cache.  */
    u8bit **outputBuffer;       /**<  Buffer for the cache line being read out of the cache.  */
    u32bit **maskBuffer;        /**<  Mask buffer for masked cache lines.  */
    u32bit memoryRequest[MAX_MEMORY_TICKETS];   /**<  Associates memory tickets (identifiers) to a read queue entry.  */
    u32bit nextTicket;          /**<  Next memory ticket.  */
    u32bit freeTickets;         /**<  Number of free memory tickets.  */


    /*  Timing.  */
    u32bit nextReadPort;    /**<  Pointer to the next read port to be used.  */
    u32bit nextWritePort;   /**<  Pointer to the next write port to be used.  */
    u32bit *writeCycles;    /**<  Remaining cycles in the cache write port.  */
    u32bit *readCycles;     /**<  Remaining cycles the cache read port is reserved.  */
    u32bit memoryCycles;    /**<  Remaining cycles the memory bus is used/reserved.  */
    u32bit compressCycles;  /**<  Remaining cycles for the compression of the current block.  */
    u32bit uncompressCycles;/**<  Remaining cycles for the compression of the current block.  */
    u32bit clearCycles;     /**<  Remaining cycles for the clear of the block state memory.  */

    /*  Hierarchical Z update state.  */
    bool updatedHZ;         /**<  This flag stores if there is a new update for the Hierarchical Z.  */
    u32bit hzBlock;         /**<  Hierarchical Z block to update.  */
    u32bit hzBlockZ;        /**<  Hierarchical Z block new Z.  */

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

    /**
     *
     *  Translates a cache line address to a block address in the
     *  block state memory of the Z Cache.
     *
     *  @param address Address of the cache line to translate.
     *
     *  @return The address in the state memory for the block.
     *
     */

    u32bit address2block(u32bit address);

public:

	/*  Defines the size of a compressed block using best compression level.  */
	static const u32bit COMPRESSED_BLOCK_SIZE_BEST = 64;

	/*  Defines the size of a compressed block using normal compression level.  */
	static const u32bit COMPRESSED_BLOCK_SIZE_NORMAL = 128;

	/*  Defines the size of an uncompressed block.  */
	static const u32bit UNCOMPRESSED_BLOCK_SIZE = 256;

	/*  Defines compression masks and shift values.  */
	static const u32bit COMPRESSION_HIMASK_NORMAL = 0xffffe000L;
	static const u32bit COMPRESSION_HIMASK_BEST = 0xffffffe0L;
	static const u32bit COMPRESSION_LOSHIFT_NORMAL = 13;
	static const u32bit COMPRESSION_LOSHIFT_BEST = 5;
	static const u32bit COMPRESSION_LOMASK_NORMAL = 0x00001fffL;
	static const u32bit COMPRESSION_LOMASK_BEST = 0x0000001fL;


    /**
     *
     *  Z Cache constructor.
     *
     *  Creates and initializes a Z Cache object.
     *
     *  @param numWays Number of ways in the cache.
     *  @param numLines Number of lines in the cache.
     *  @param stampsLine Number of stamps per cache line.
     *  @param bytesStamp Number of bytes per stamp.
     *  @param readPorts Number of read ports.
     *  @param writePorts Number of write ports.
     *  @param portWidth Width in bytes of the write and read ports.
     *  @param reqQueueSize Z cache request to memory queue size.
     *  @param inputRequests Number of read requests and input buffers.
     *  @param outputRequests Number of write requests and output buffers.
     *  @param disableCompr Disables Z block compression.
     *  @param maxBlocks Maximum number of sopported color blocks in the Z buffer.
     *  @param blocksCycle Number of state block entries that can be modified (cleared) per cycle.
     *  @param compCycles Compression cycles.
     *  @param decompCycles Decompression cycles.
     *  @param postfix The postfix for the z cache statistics.
     *
     *  @return A new initialized cache object.
     *
     */

    ZCache(u32bit numWays, u32bit numLines, u32bit stampsLine,
        u32bit bytesStamp, u32bit readPorts, u32bit writePorts, u32bit portWidth, u32bit reqQueueSize,
        u32bit inputRequests, u32bit outputRequests, bool disableCompr, u32bit maxBlocks,
        u32bit blocksCycle, u32bit compCycles, u32bit decompCycles, char *postfix);

    /**
     *
     *  Reserves and fetches (if the line is not already available)
     *  the cache line for the requested stamp defined by the
     *  Z buffer address.
     *
     *  @param address The address of the first byte of the stamp
     *  in the Z buffer.
     *  @param way Reference to a variable where to store the
     *  cache way in which the data for the fetched address will be stored.
     *  @param line Reference to a variable where to store the
     *  cache line where the fetched data will be stored.
     *  @param source Pointer to a Dynamic Object that is the source of the
     *  cache access.
     *
     *
     *  @return If the line for the stamp could be reserved and
     *  fetched (ex. all line are already reserved).
     *
     */

    bool fetch(u32bit address, u32bit &way, u32bit &line, DynamicObject *source);

    /**
     *
     *  Reserves and fetches (if the line is not already available)
     *  the cache line for the requested stamp defined by the
     *  Z buffer address.
     *
     *  @param address The address of the first byte of the stamp
     *  in the Z buffer.
     *  @param way Reference to a variable where to store the
     *  way in which the data for the fetched address will be stored.
     *  @param line Reference to a variable where to store the
     *  cache line where the fetched data will be stored.
     *  @param source Pointer to a Dynamic Object that is the source of the
     *  cache access.
     *
     *  @return If the line for the stamp could be reserved and
     *  fetched (ex. all line are already reserved).
     *
     */

    bool allocate(u32bit address, u32bit &way, u32bit &line, DynamicObject *source);

    /**
     *
     *  Reads stamp depth and stencil data from the Z cache.
     *  The line associated with the requested address/stamp
     *  must have been previously fetched, if not an error
     *  is produced.
     *
     *  @param address Address of the stamp in the Z buffer.
     *  @param way Way where the data for the address is stored.
     *  @param line Cache line where the data for the address
     *  is stored.
     *  @param data Pointer to an array where to store the read
     *  depth and stencil data for the stamp.
     *
     *  @return If the read could be performed (ex. line not
     *  yet received from video memory).
     *
     */

    bool read(u32bit address, u32bit way, u32bit line, u8bit *data);

    /**
     *
     *  Writes with mask a stamp depth and stencil data to the Z cache
     *  and unreserves the associated Z cache line.
     *
     *  @param address The address of the first byte of the stamp
     *  in the Z buffer.
     *  @param way Way where the data for the address is stored.
     *  @param line Cache line where the data for the address
     *  is stored.
     *  @param data Pointer to a buffer where the stamp color data
     *  to write is stored.
     *  @param mask Write mask (per byte).
     *
     *  @return If the write could be performed (ex. conflict using
     *  the write bus).
     *
     */

    bool write(u32bit address, u32bit way, u32bit line, u8bit *data, bool *mask);

    /**
     *
     *  Writes a stamp depth and stencil data to the Z cache and unreserves
     *  the associated Z cache line.
     *
     *  @param address The address of the first byte of the stamp
     *  in the Z buffer.
     *  @param way Way where the data for the address is stored.
     *  @param line Cache line where the data for the address
     *  is stored.
     *  @param data Pointer to a buffer where the stamp depth and stencil data
     *  to write is stored.
     *
     *  @return If the write could be performed (ex. conflict using
     *  the write bus).
     *
     */

    bool write(u32bit address, u32bit way, u32bit line, u8bit *data);

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
     *  Resets the Z Cache structures.
     *
     *
     */

    void reset();

    /**
     *
     *  Writes back to memory the valid Z cache lines.
     *
     *  @return If all the valid lines have been written
     *  to memory.
     *
     */

    bool flush();

    /**
     *
     *  Clears the Z buffer.
     *
     *  @param clearDepth The depth value with which to
     *  clear the depth buffer.
     *  @param clearStencil The stencil value with which
     *  to clear the stencil buffer.
     *
     *  @return If the clear process has finished.
     *
     */

    bool clear(u32bit clearDepth, u8bit clearStencil);

    /**
     *
     *  Signals a swap in the Z buffer address.
     *
     *  @param address New address of the Z buffer.
     *
     */

    void swap(u32bit address);

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
     *  Checks if there is an update for the Hierarchical Z ready
     *  and returns it.
     *
     *  @param block A reference to a variable were to store the Hierarchical Z
     *  block to be updated.
     *  @param z A reference to a variable where to store the new Z for the updated
     *  Hierarchical Z block.
     *
     *  @return If there is an update to the Hierarchical Z ready.
     *
     */

    bool updateHZ(u32bit &block, u32bit &z);

    /**
     *
     *  Simulates a cycle of the Z cache.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Copies the block state memory.
     *
     *  @param buffer Pointer to a Z Block State buffer.
     *  @param blocks Number of blocks to copy.
     *
     */

    void copyBlockStateMemory(ZBlockState *buffer, u32bit blocks);

};

} // namespace gpu3d

#endif

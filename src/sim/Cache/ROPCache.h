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
 * $RCSfile: ROPCache.h,v $
 * $Revision: 1.8 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:17 $
 *
 * ROP Cache class definition file.
 *
 */

/**
 *
 *  @file ROPCache.h
 *
 *  Defines the ROP Cache class.  This class defines the cache used
 *  to access buffer data from a Generic ROP box.
 *
 */


#ifndef _ROPCACHE_

#define _ROPCACHE_

#include "GPUTypes.h"
#include "MemoryTransaction.h"
#include "FetchCache.h"
#include "toolsQueue.h"
#include "FragmentOpEmulator.h"
#include "CompressorEmulator.h"
#include "toolsQueue.h"

namespace gpu3d
{

//  Defines a block memory write request.
class WriteRequest
{
public:

    u32bit address;     /**<  Address to write to.  */
    u32bit block;       /**<  Block ID for the address for the stamp unit.  */
    u32bit blockFB;     /**<  Block ID for the address for the framebuffer.  */
    u32bit size;        /**<  Size of the write request.  */
    u32bit written;     /**<  Bytes already written to memory.  */
    CacheRequest *request;  /**<  Pointer to the cache request.  */
    u32bit requestID;   /**<  Identifier of the cache request.  */
    u32bit readWaiting; /**<  Read queue entry waiting for the write queue to read from the same cache line.  */
    bool isReadWaiting; /**<  Stores if a read queue entry is waiting for this write to finish.  */
};

//  Defines a block read request to memory.
class ReadRequest
{
public:

    u32bit address;     /**<  Address to read from.  */
    u32bit block;       /**<  Block for the address.  */
    u32bit size;        /**<  Size of the read request.  */
    u32bit requested;   /**<  Bytes requested to memory.  */
    u32bit received;    /**<  Bytes received from memory.  */
    CacheRequest *request;  /**<  Pointer to the cache request.  */
    u32bit requestID;   /**<  Identifier of the cache request.  */
    bool writeWait;     /**<  The read request must wait for a write request to read the cache line before replacing the cache line itself.  */
    bool spillWait;     /**<  The read request must wait for a write request to write memory before requesting the line from memory (spill must complete).  */
};

//  Defines the states of a ROP data buffer block.
class ROPBlockState {
public:
    enum State
    {
        CLEAR,              /**<  The block is in clear state (use clear value).  */
        UNCOMPRESSED,       /**<  The block is uncompressed.  */
        COMPRESSED          /**<  The block is compressed.  */
    };
    
    State state;
    u32bit comprLevel;      /**< If ROP_BLOCK_COMPRESSED, the compression level */
    
public:
    ROPBlockState() : state(CLEAR) {}
    ROPBlockState(State state) : state(state) {}

    operator State() const
    {
        return state;
    }
    
    State getState() const
    {
        return state;
    }
    
    bool isCompressed() const
    {
        return state == COMPRESSED; 
    }
    
    u32bit getComprLevel() const
    {
        return comprLevel;
    }
};

/**
 *
 *  This class describes and implements the behaviour of the cache
 *  used to access the buffer data from a Generic ROP stage in a GPU.
 *
 *  This class uses the Fetch Cache.
 *
 */

class ROPCache
{

private:

    //  ROP cache identification.
    char *ropCacheName;             /**<  Name of the ROP cache.  */
    GPUUnit gpuUnitID;              /**<  Stores the ROP cache GPU unit type identifier for the Memory Controller.  */

    //  ROP cache parameters.  */
    u32bit lineSize;        /**<  Size of the cache line in bytes.  */
    u32bit readPorts;       /**<  Number of read ports.  */
    u32bit writePorts;      /**<  Number of write ports.  */
    u32bit portWidth;       /**<  Size of the cache read/write ports in bytes.  */
    u32bit inputRequests;   /**<  Number of read requests and input buffers.  */
    u32bit outputRequests;  /**<  Number of write requests and output buffers.  */
    bool disableCompr;      /**<  Disables ROP data block compression.  */
    u32bit comprLatency;    /**<  Cycles it takes to compress a block.  */
    u32bit decomprLatency;  /**<  Cycles it takes to decompress a block.  */

    //  ROP cache derived parameters.
    u32bit blockShift;      /**<  Number of bits to removed from a ROP data buffer buffer address to retrieve the block number.  */

    //  ROP cache registers.
    u32bit ropBufferAddress;/**<  Start address of the ROP data buffer in GPU memory.  */
    u32bit ropStateAddress; /**<  Start address of the ROP block state buffer in GPU memory.  */
    bool compression;       /**<  Flag that enables or disables compression.  */
    u32bit bytesPixel;      /**<  Bytes per pixel for the current pixel data format.  */
    u32bit msaaSamples;     /**<  Number of MSAA samples.  */

    //  ROP cache structures.
    FetchCache *cache;          /**<  Pointer to the ROP Cache fetch cache.  */

    //  ROP cache state.
    bool flushRequest;      /**<  Flag for starting the flush.  */
    bool flushMode;         /**<  If the cache is in flush mode.  */
    bool saveStateMode;     /**<  Saving block state info into block state buffer in memory.  */
    bool restoreStateMode;  /**<  Restoring block state info from block state buffer in memory.  */
    bool saveStateRequest;  /**<  Flag for starting a save block state info request.  */
    bool restoreStateRequest;   /**<  Flag for starting a restore block state info request.  */
    bool resetStateMode;        /**<  Resetting the block state info to uncompressed mode.  */
    bool resetStateRequest;     /**<  Flag for starting a reset block state info request.  */    
    u32bit resetStateCycles;    /**<  Cycles until the end of the reset block state process.  */
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
    bool fetchPerformed;    /**<  Stores if a fetch/allocate operation was performed in the current cycle.  */
    u32bit writeLinePort;   /**<  Port being used to write a cache line.  */
    u32bit readLinePort;    /**<  Port being used to read a cache line.  */

    
    //  Memory Structures.
    WriteRequest *writeQueue;   /**<  Write request queue.  */
    ReadRequest *readQueue;     /**<  Read request queue.  */
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
    tools::Queue<u32bit> ticketList;            /**<  List with the memory tickets available to generate requests to memory.  */
    u32bit freeTickets;                         /**<  Number of free memory tickets.  */
    u32bit nextWriteTicket;                     /**<  Write ticket counter.  */
    
    // Block state save/restore structures.
    u32bit savedBlocks;         /**<  Number of blocks saved to the block state buffer in memory.  */
    u32bit requestedBlocks;     /**<  Number of blocks requested to memory.  */
    u32bit restoredBlocks;      /**<  Number of blocks restored from the block state buffer in memory.  */
    u8bit *blockStateBuffer;    /**<  Storage for the block state into (binary encoded).  */

    //  Timing.
    u32bit nextReadPort;    /**<  Pointer to the next read port to be used.  */
    u32bit nextWritePort;   /**<  Pointer to the next write port to be used.  */
    u32bit *writeCycles;    /**<  Remaining cycles in the cache write port.  */
    u32bit *readCycles;     /**<  Remaining cycles the cache read port is reserved.  */
    u32bit memoryCycles;    /**<  Remaining cycles the memory bus is used/reserved.  */
    u32bit compressCycles;  /**<  Remaining cycles for the compression of the current block.  */
    u32bit uncompressCycles;/**<  Remaining cycles for the compression of the current block.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *noRequests;       /**<  Counts cycles with no new cache requests.  */
    GPUStatistics::Statistic *writeQFull;       /**<  Counts cycles with write request queue full.  */
    GPUStatistics::Statistic *readQFull;        /**<  Counts cycles with read request queue full.  */
    GPUStatistics::Statistic *rwQFull;          /**<  Counts cycles with write or read request queues full.  */
    GPUStatistics::Statistic *waitWriteStall;   /**<  Counts cycles with a read request stalled waiting for a write request to finish.  */
    GPUStatistics::Statistic *writeReqQueued;   /**<  Counts cycles with a new write request accepted.  */
    GPUStatistics::Statistic *readReqQueued;    /**<  Counts cycles with a new read request accepted.  */
    GPUStatistics::Statistic *readInQEmpty;     /**<  Counts cycles with the read input queue empty.  */
    GPUStatistics::Statistic *decomprBusy;      /**<  Counts cycles with the decompressor unit busy.  */
    GPUStatistics::Statistic *waitReadStall;    /**<  Counts cycles with the head of the read input queue waiting for a read transaction to finish.  */
    GPUStatistics::Statistic *comprBusy;        /**<  Counts cycles with the compressor unit busy.  */
    GPUStatistics::Statistic *writeOutQEmpty;   /**<  Counts cycles with the write output queue empty.  */
    GPUStatistics::Statistic *memReqStall;      /**<  Counts cycles with the memory request stalled due to the request bus to memory controller busy.  */
    GPUStatistics::Statistic *warStall;         /**<  Counts cycles with a cache line write stalled due to a dependence with a preceding cache line read.  */
    GPUStatistics::Statistic *dataBusBusy;      /**<  Counts cycles with the memory data bus busy.  */
    GPUStatistics::Statistic *writePortStall;   /**<  Counts cycles with a cache line write stalled due to the cache write port being busy.  */
    GPUStatistics::Statistic *readPortStall;    /**<  Counts cycles with a cache line read stalled due to the cache read port being busy.  */
    
    GPUStatistics::Statistic *comprBlocks64;    /**< Counts the number of blocks compressed to size 64. */
    GPUStatistics::Statistic *comprBlocks128;    /**< Counts the number of blocks compressed to size 128. */
    GPUStatistics::Statistic *comprBlocks192;    /**< Counts the number of blocks compressed to size 192. */
    GPUStatistics::Statistic *comprBlocks256;    /**< Counts the number of blocks compressed to size 256 (uncompressed). */
    
    //  Private functions.

    /**
     *
     *  Generates a memory transaction requesting data to
     *  the memory controller for the read queue entry.
     *
     *  @param cycle Current simulation cycle.
     *  @param readReq The read queue entry for which to generate
     *  the memory transaction.
     *
     *  @return The memory transaction generated.  If it could not
     *  be generated returns NULL.
     *
     */

    MemoryTransaction *requestBlock(u64bit cycle, u32bit readReq);

    /**
     *
     *  Generates a write memory transaction for the write queue
     *  entry.
     *
     *  @param cycle Current simulation cycle.
     *  @param writeReq Write queue entry for which to generate
     *  the memory transaction.
     *
     *  @return If a memory transaction could be generated, the
     *  transaction generated, otherwise NULL.
     *
     */

    MemoryTransaction *writeBlock(u64bit cycle, u32bit writeReq);

    /**
     *
     *  Translates a cache line address to a block address in the block state memory of the ROP Cache.
     *
     *  @param address Address of the cache line to translate.
     *
     *  @return The address in the state memory for the block.
     *
     */

    u32bit addressToBlockSU(u32bit address);

    /**
     *
     *  Translates a cache line address to a block address in the block state memory of the ROP Cache.
     *
     *  @param address Address of the cache line to translate.
     *
     *  @return The address in the state memory for the block.
     *
     */

    u32bit addressToBlock(u32bit address);

    /**
     *
     *  Decode the block state.
     *
     *  @param state Encoded block state.
     *
     *  @return Decoded block state.
     *
     */
     
    ROPBlockState::State decodeState(u32bit state);

    /**
     *
     *  Reset the ROP cache state.
     *
     */

    void performeReset();

    /**
     *
     *  Updates the save block state info machine.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void updateSaveState(u64bit cycle);

    /**
     *
     *  Updates the restore block state info machine.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void updateRestoreState(u64bit cycle);

    /**
     *
     *  Updates the reset block state info machine.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void updateResetState(u64bit cycle);

protected:

    //  ROP cache parameters
    u32bit numStampUnits;   /**<  Number of stamp units in the GPU pipeline.  */
    u32bit stampUnitStride; /**<  Stride in blocks between blocks assigned to a stamp unit.  */
    u32bit maxBlocks;       /**<  Maximun number of supported blocks (cache lines) in the ROP data buffer.  */
    u32bit blocksCycle;     /**<  Blocks modified (cleared) per cycle.  */

    //  ROP cache identification to the Memory Controller
    u32bit cacheID;                 /**<  Identifier of the specific object ROP cache.  */

    //  ROP Cache registers.
    u8bit clearROPValue[MAX_BYTES_PER_PIXEL];   /**<  The current clear data value (32 bits only).  */

    //  ROP Cache structures.
    ROPBlockState *blockState;  /**<  Stores the current state of the blocks in the ROP data buffer.  */

    //  ROP Cache state.
    bool resetMode;         /**<  Reset mode flag.  */
    bool clearMode;         /**<  Clear mode flag.  */

    //  ROP cache constant.
    u8bit clearResetValue[MAX_BYTES_PER_PIXEL];     /**<  Value taken as clear value on cache reset. */

    //  ROP cache timing.
    u32bit clearCycles;     /**<  Remaining cycles for the clear of the block state memory.  */

    //  ROP cache block written state
    bool blockWasWritten;   /**<  This flag stores if a ROP data buffer block was written into memory in the current cycle.  */
    u32bit writtenBlock;    /**<  Identifier/number of the ROP data buffer block that was written to memory.  */

protected:
    virtual void processNextWrittenBlock(u8bit* outputBuffer, u32bit size) {}
    
    virtual CompressorEmulator& getCompressor() = 0;
    
public:

    /*  Defines the size of a compressed block using best compression level.  */
    static const u32bit COMPRESSED_BLOCK_SIZE_BEST = 64;

    /*  Defines the size of a compressed block using normal compression level.  */
    static const u32bit COMPRESSED_BLOCK_SIZE_NORMAL = 128;

    /*  Defines the size of an uncompressed block.  */
    static const u32bit UNCOMPRESSED_BLOCK_SIZE = 256;

    /**
     *
     *  ROP Cache constructor.
     *
     *  Creates and initializes a ROP Cache object.
     *
     *  @param numWays Number of ways in the cache.
     *  @param numLines Number of lines in the cache.
     *  @param lineSize Size of the cache line in bytes.
     *  @param readPorts Number of read ports.
     *  @param writePorts Number of write ports.
     *  @param portWidth Width in bytes of the write and read ports.
     *  @param reqQueueSize ROP cache request to memory queue size.
     *  @param inputRequests Number of read requests and input buffers.
     *  @param outputRequests Number of write requests and output buffers.
     *  @param disableCompr Disables ROP data block compression.
     *  @param numStampUnits Number of stamp units in the GPU pipeline.
     *  @param stampUnitStride Stride in blocks for the blocks that are assigned to a stamp unit.
     *  @param maxBlocks Maximum number of sopported ROP data blocks in the ROP data buffer.
     *  @param blocksCycle Number of state block entries that can be modified (cleared) per cycle.
     *  @param compCycles Compression cycles.
     *  @param decompCycles Decompression cycles.
     *  @param maxBlocks Number of block state items that can be modified per cycle.
     *  @param gpuUnit The Memory Controller GPU unit type identifier for this instance of the ROP cache.
     *  @param ropCacheName Name of the ROP cache.
     *  @param postfix The postfix for the ROP cache statistics.
     *
     *  @return A new initialized ROP cache object.
     *
     */

    ROPCache(u32bit numWays, u32bit numLines, u32bit lineSize,
        u32bit readPorts, u32bit writePorts, u32bit portWidth, u32bit reqQueueSize,
        u32bit inputRequests, u32bit outputRequests, bool disableCompr,
        u32bit numStampUnits, u32bit stampUnitStride, u32bit maxBlocks,
        u32bit blocksCycle, u32bit compCycles, u32bit decompCycles,
        GPUUnit gpuUnit, char *ropCacheName, char *postfix);

    /**
     *
     *  Reserves and fetches (if the line is not already available)
     *  the cache line for the requested stamp defined by the
     *  ROP buffer address.
     *
     *  @param address The address of the first byte of the stamp in the ROP buffer.
     *  @param way Reference to a variable where to store the cache way in which the data
     *  for the fetched address will be stored.�
     *  @param line Reference to a variable where to store the cache line where the fetched
     *  data will be stored.
     *  @param source Pointer to a Dynamic Object that is the source of the cache access.
     *  @param reserves Number of reserve slots (read/write operations) that this fetch will take.
     *
     *
     *  @return If the line for the stamp could be reserved and fetched (ex. all line
     *          are already reserved).
     *
     */

    bool fetch(u32bit address, u32bit &way, u32bit &line, DynamicObject *source = NULL, u32bit reserves = 1);

    /**
     *
     *  Reserves and fetches (if the line is not already available) the cache line for the
     *  requested stamp defined by the ROP data buffer address.
     *
     *  @param address The address of the first byte of the stamp in the ROP data buffer.
     *  @param way Reference to a variable where to store the way in which the data for
     *  the fetched address will be stored.
     *  @param line Reference to a variable where to store the cache line where the
     *  fetched data will be stored.
     *  @param source Pointer to a Dynamic Object that is the source of the cache access.
     *  @param reserves Number of reserve slots (read/write operations) that this fetch will take.
     *
     *
     *  @return If the line for the stamp could be reserved and fetched (ex. all line are already reserved).
     *
     */

    bool allocate(u32bit address, u32bit &way, u32bit &line, DynamicObject *source = NULL, u32bit reserves = 1);

    /**
     *
     *  Reads stamp data from the ROP cache.  The line associated with the requested
     *  address/stamp must have been previously fetched, if not an error
     *  is generated.
     *
     *  @param address Address of the stamp in the ROP data buffer.
     *  @param way Way where the data for the address is stored.
     *  @param line Cache line where the data for the address is stored.
     *  @param size Bytes to read from the cache line.
     *  @param data Pointer to an array where to store the read data for the stamp.
     *
     *  @return If the read could be performed (ex. line not yet received from video memory).
     *
     */

    bool read(u32bit address, u32bit way, u32bit line, u32bit size, u8bit *data);

    /**
     *
     *  Writes with mask data to the ROP cache and unreserves the associated ROP cache line.
     *
     *  @param address The address of the first byte of the stamp in the ROP data buffer.
     *  @param way Way where the data for the address is stored.
     *  @param line Cache line where the data for the address is stored.
     *  @param size Bytes to write to the cache line.
     *  @param data Pointer to a buffer where the data to write is stored.
     *  @param mask Write mask (per byte).
     *
     *  @return If the write could be performed (ex. conflict using the write bus).
     *
     */

    bool write(u32bit address, u32bit way, u32bit line, u32bit size, u8bit *data, bool *mask);

    /**
     *
     *  Writes a stamp data to the ROP cache and unreserves the associated ROP cache line.
     *
     *  @param address The address of the first byte of the stamp in the ROP data buffer.
     *  @param way Way where the data for the address is stored.
     *  @param line Cache line where the data for the address is stored.
     *  @param size Bytes to write to the cache line.
     *  @param data Pointer to a buffer where the stamp data to write is stored.
     *
     *  @return If the write could be performed (ex. conflict using the write bus).
     *
     */

    bool write(u32bit address, u32bit way, u32bit line, u32bit size, u8bit *data);

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
     *  Resets the ROP Cache structures.
     *
     */

    void reset();

    /**
     *
     *  Writes back to memory the valid ROP cache lines.
     *
     *  @return If all the valid lines have been written to memory.
     *
     */

    bool flush();

    /**
     *
     *  Save the block state info to the block state buffer in memory.
     *
     *  @return If the block state buffer has been saved into memory.
     *
     */

    bool saveState();

    /**
     *
     *  Reset the block state info to uncompressed mode.
     *
     *  @return If all the block state info has been reset.
     */
     
    bool resetState();
    
    /**
     *
     *  Restore the block state info from the block state buffer in memory.
     *
     *  @return If the block state buffer has been restored from memory.
     *
     */

    bool restoreState();

    /**
     *
     *  Signals a swap in the ROP data buffer address.  Sets the address of the ROP data buffer in memory.
     *
     *  @param address New address of the ROP data buffer.
     *
     */

    void swap(u32bit address);

    /**
     *
     *  Sets the start address of the block state buffer in memory.
     *
     */

    void setStateAddress(u32bit address);

    /**
     *
     *  Sets the bytes per pixel data.
     *
     *  @param The number of bytes for the current pixel data format.
     *
     */
     
    void setBytesPerPixel(u32bit bytesPixel);

    /**
     *
     *  Set the number of samples for multisampling antialiasing.
     *
     *  @param Number of MSAA samples.
     *
     */
  
    void setMSAASamples(u32bit msaaSamples);
                  
    /**
     *
     *  Set the compression flag.  Used to enable or disable compression of the buffer.
     *
     *  @param The new value of the compression.
     *
     */
     
    void setCompression(bool compr);     
     

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
     *
     *  @return A new memory transaction to be issued to the  memory controller.
     *
     */

    MemoryTransaction *update(u64bit cycle, MemState memoryState);

    /**
     *
     *  Simulates a cycle of the color cache.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void clock(u64bit cycle);

    /**
     *
     *  Saves the block state memory into a file.
     *
     */
     
    void saveBlockStateMemory();
    
    /**
     *
     *  Loads the block state memory from a file.
     *
     */
     
    void loadBlockStateMemory();


    /**
     *
     *  Encodes block state data into its real binary encoding.
     *
     *  @param data Pointer to a data array where to store the binary encoding of the block state data.
     *  @param blocks Number of block state elements to encode.
     *
     */    
    
    void encodeBlocks(u8bit *data, u32bit blocks);
    
    /**
     *
     *  Decodes block state data from its real binary encoding and stores it on the block state memory.
     *
     *  @param data Pointer to a data array with the binary encoded block state data that has to be decoded to fill the
     *              block state memory.
     *  @param blocks Number of block state elements to decode.
     *
     */    
    
    void decodeAndFillBlocks(u8bit *data, u32bit blocks);

    /**
     *
     *  Writes into a string a report about the stall condition of the box.
     *
     *  @param cycle Current simulation cycle.
     *  @param stallReport Reference to a string where to store the stall state report for the box.
     *
     */
     
    void stallReport(u64bit cycle, string &stallReport);
    
};

} // namespace gpu3d

#endif

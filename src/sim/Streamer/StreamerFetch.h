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
 * $RCSfile: StreamerFetch.h,v $
 * $Revision: 1.12 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:54 $
 *
 * Streamer Fetch class definition file.
 *
 */

/**
 *
 *  @file StreamerFetch.h
 *
 *  This file defines the StreamerFetch box class.
 *
 */

#ifndef _STREAMERFETCH_

#define _STREAMERFETCH_


namespace gpu3d
{
    class StreamerFetch;
} // namespace gpu3d


#include "GPUTypes.h"
#include "Box.h"
#include "Streamer.h"
#include "StreamerControlCommand.h"
#include "StreamerCommand.h"
// #include "MemoryController.h"
#include "MemoryTransaction.h"
#include "MemoryControllerDefs.h"
#include "GPU.h"

namespace gpu3d
{

/**
 *
 *  Defines the size of the memory request reorder queue for the Streamer
 *  Fetch index buffer.
 */
static const u32bit MAX_MEMORY_REQUESTS = 64;

/**
 *
 *  This class implements the Streamer Fetch box.
 *
 *  This box simulates the load of new indexes from the
 *  memory and the fetch and allocations of new input indexes.
 *  Inherits from the Box class that provides basic simulation
 *  support.
 *
 */

class StreamerFetch : public Box
{

private:

    /*  Streamer Fetch Registers.  */
    bool indexedMode;               /**<  Indexed mode enabled.  */
    u32bit indexStreamAddress;      /**<  The index stream address.  */
    StreamData indexStreamData;     /**<  The index stream data format.  */
    u32bit streamStart;             /**<  Start index position (non indexed mode).  */
    u32bit streamCount;             /**<  Stream count (number of indexes/inputs) to fetch.  */
    u32bit streamInstances;         /**<  Number of instances of the current stream to process.  */

    /*  Streamer Fetch Signals.  */
    Signal *streamerFetchMemReq;    /**<  Request signal to the Memory Controller.  */
    Signal *streamerFetchMemData;   /**<  Data signal from the Memory Controller.  */
    Signal *streamerFetchCommand;   /**<  Streamer Command signal from the Streamer box.  */
    Signal *streamerFetchState;     /**<  State signal to the Streamer.  */
    Signal *streamerOutputCache;    /**<  Signal to the streamer output cache.  */
    Signal **streamerLoaderDeAlloc; /**<  Deallocation signal from the streamer loader units.  */
    Signal *streamerCommitDeAlloc;  /**<  Deallocation signal from the streamer commit.  */

    /*  Streamer Fetch parameters.  */
    u32bit indicesCycle;            /**<  Number of indices to fetch per cycle.  */
    u32bit indexBufferSize;         /**<  Size in bytes of the index buffer for the index data to fetch.  */
    u32bit outputFIFOSize;          /**<  Number of output/indexes in the output FIFO.  */
    u32bit outputMemorySize;        /**<  Number of lines/outputs in the output memory.  */
    u32bit inputRequestQueueSize;   /**<  Number of inputs/indexes in the input request queue.  */
    u32bit verticesCycle;           /**<  Number of vertices that Streamer Commit can deallocate per cycle.  */
    u32bit streamerLoaderUnits;     /**<  Number of Streamer Loader Units.  */
    u32bit slIndicesCycle;          /**<  Number of indices per cycle processed by each Streamer Loader Unit.  */

    /*  Index buffer structures.  */
    u8bit *indexBuffer;             /**<  The index data buffer.  */
    u32bit nextIndex;               /**<  Next index pointer/counter.  */
    u32bit nextFreeIndex;           /**<  Pointer to next index to fetch from the index buffer.  */
    u32bit requestedIndex;          /**<  Pointer to next free position in the index buffer.  */
    u32bit indexData;               /**<  Index data (in bytes) stored in the index buffer.  */
    u32bit requestedData;           /**<  Data in the index buffer requested to the memory controller.  */
    u32bit fetchedIndexes;          /**<  Total number of indexes already fetched.  */
    u32bit requestedIndexData;      /**<  Total number of index data bytes requested to memory.  */
    u32bit streamPaddingBytes;      /**<  Number of padding bytes at the start of the index stream to align to the memory transaction size.  */
    u32bit bufferPaddingBytes;      /**<  Number of padding bytes to align the index buffer read pointer to the memory transaction size.  */
    bool skipPaddingBytes;          /**<  Flag that stores if before the next index read padding bytes must be skipped.  */
    u32bit readInstance;            /**<  Current instance being read from memory.  */
    u32bit fetchInstance;           /**<  Current instance begin fetched.  */
    
    /*  Streamer Fetch state.  */
    u32bit freeOutputFIFOEntries;       /**<  Number of free output FIFO entries.  */
    u32bit freeOutputMemoryLines;       /**<  Number of free ouput memory lines.  */
    u32bit* freeIRQEntries;             /**<  Number of free input request queue entries each Streamer Loader unit.  */
    u32bit* indicesSentThisCycle;       /**<  Number of indices sent the current cycle to each Streamer Loader unit.  */
    u32bit nextFreeOFIFOEntry;          /**<  Next free output FIFO entry.  */
    StreamerCommand *lastStreamCom;     /**<  Keeps the last streamer command received for signal tracing.  */
    StreamerState state;                /**<  State of the streamer.  */
    u32bit **unconfirmedOMLineDeAllocs; /**<  Array of deallocated output memory lines from Streamer Commit pending of confirmation.  */
    u32bit *unconfirmedDeAllocCounters; /**<  Number of deallocated output memory lines from Streamer Commit pending of confirmation.  */

    /*  Memory access structures.  */
    u32bit currentTicket;           /**<  Current memory ticket.  */
    u32bit freeTickets;             /**<  Number of free memory tickets.  */
    u32bit busCycles;               /**<  Remaining cycles of the current memory bus transmission.  */
    u32bit lastSize;                /**<  Size of the last memory transaction received.  */
    u32bit readTicket;              /**<  Ticket of the last memory transaction received.  */
    MemState memoryState;           /**<  Current state of the Memory Controller.  */
    u32bit memoryRequest[MAX_MEMORY_TICKETS];   /**<  Translates tickets into reorder queue entries.  */
    u32bit reorderQueue[MAX_MEMORY_REQUESTS];   /**<  Reorder queue for memory requests.  */
    u32bit nextCommit;              /**<  Pointer to the next queue entry in the reorder queue to commit.  */
    u32bit nextRequest;             /**<  Pointer to the next queue entry in the reorder queue where to store a new request.  */
    u32bit activeRequests;          /**<  Number of active memory requests.  */

    /*  Statistics.  */
    GPUStatistics::Statistic *bytesRead;    /**<  Bytes read from memory.  */
    GPUStatistics::Statistic *transactions; /**<  Number of transactions with the Memory Controller.  */
    GPUStatistics::Statistic *sentIdx;      /**<  Number of indices sent to Output Cache.  */

    /*  Private functions.  */

    /**
     *
     *  Processes a memory transaction received from the Memory
     *  Controller.
     *
     *  @param memTrans The memory transaction to process.
     *
     */

    void processMemoryTransaction(MemoryTransaction *memTrans);

    /**
     *
     *  Processes a streamer command from the Streamer main box.
     *
     *  @param streamCom Pointer to the Streamer Command to process.
     *
     *
     */

    void processStreamerCommand(StreamerCommand *streamCom);

    /**
     *
     *  Processes a register write to a streamer fetch register.
     *
     *  @param reg Register to write.
     *  @param subreg Register subregister to write.
     *  @param data Register data to write.
     *
     */

    void processGPURegisterWrite(GPURegister reg, u32bit subreg, GPURegData data);


    /**
     *
     *  Process a streamer control command from the streamer commit
     *  and streamer loader units.
     *
     *  @param cycle The simulation cycle
     *  @param streamCCom The streamer control command to process.
     *
     */

    void processStreamerControlCommand(u32bit cycle, StreamerControlCommand *streamCCom);

    /**
     *
     *  Converts from a stream buffer data format to a
     *  32 bit unsigned integer value.
     *
     *  @param format Stream buffer data format.
     *  @param data pointer to the data to convert.
     *
     *  @return The converted 32 bit unsigned integer.
     *
     */

    u32bit indexDataConvert(StreamData format, u8bit *data);

public:

    /**
     *
     *  Streamer Fetch constructor.
     *
     *  Creates and initializes a Streamer Fetch box.
     *
     *  @param idxCycle Indices to fetch/generate per cycle.
     *  @param indexBufferSize Size of the index buffer size (in bytes).
     *  @param outputFIFOSize Size of the output FIFO (entries).
     *  @param outputMemorySize Size of the output memory (lines).
     *  @param inputRequestQueueSize Size of the input request queue (IRQ) (entries).
     *  @param vertCycle Vertices per cycle that Streamer Commit can deallocate per cycle.
     *  @param slPrefixArray Array with Streamer Loader Units signal prefixes.
     *  @param streamLoadUnits Number of Streamer Loader Units.
     *  @param slIdxCycle Indices per cycle processed per Streamer Loader unit.
     *  @param name Name of the box.
     *  @param parent Parent box.
     *
     *  @return An initialized Streamer Fetch box.
     *
     */

    StreamerFetch(u32bit idxCycle, u32bit indexBufferSize, u32bit outputFIFOSize,
        u32bit outputMemorySize, u32bit inputRequestQueueSize, u32bit vertCycle,
        char **slPrefixArray, u32bit streamLoadUnits, u32bit slIdxCycle,
        char *name, Box* parent);

    /**
     *
     *  Simulates a cycle in the Streamer Fetch box.
     *
     *  @param cycle The cycle to simulate.
     *
     */

    void clock(u64bit cycle);

};

} // namespace gpu3d

#endif

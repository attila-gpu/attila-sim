/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not b;e disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: ROPCache.cpp,v $
 * $Revision: 1.8 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:17 $
 *
 * ROP Cache class implementation file.
 *
 */

/**
 *
 * @file ROPCache.cpp
 *
 * Implements the ROP Cache class.  This class the cache used for
 * accessing the ROP data buffer from a Generic ROP box.
 *
 */

#include "GPUMath.h"
#include "ROPCache.h"
using gpu3d::tools::Queue;

#include <fstream>
#include <sstream>

using namespace std;

using namespace gpu3d;

//  ROP cache constructor.
ROPCache::ROPCache(u32bit ways, u32bit lines, u32bit lineSz,
    u32bit readP, u32bit writeP, u32bit pWidth, u32bit reqQSize, u32bit inReqs,
    u32bit outReqs, bool comprDisabled, u32bit numStampUnits_, u32bit stampUnitStride_,
    u32bit maxColorBlocks, u32bit blocksPerCycle,
    u32bit compCycles, u32bit decompCycles, GPUUnit gpuUnit, char *nameCache, char *postfix) :

    readPorts(readP), writePorts(writeP),
    portWidth(pWidth), inputRequests(inReqs), outputRequests(outReqs),
    disableCompr(comprDisabled), numStampUnits(numStampUnits_), stampUnitStride(stampUnitStride_),
    maxBlocks(maxColorBlocks), blocksCycle(blocksPerCycle),
    comprLatency(compCycles), decomprLatency(decompCycles), lineSize(lineSz),
    gpuUnitID(gpuUnit), ropCacheName(nameCache)

{
    u32bit i;

    //  Check parameters.
    GPU_ASSERT(
        if (ways == 0)
            panic(ropCacheName, ropCacheName, "At least one ways required.");
        if (lines == 0)
            panic(ropCacheName, ropCacheName, "At least one cache line per ways required.");
        if (readPorts == 0)
            panic(ropCacheName, ropCacheName, "At least one read port required.");
        if (writePorts == 0)
            panic(ropCacheName, ropCacheName, "At least one read port required.");
        if (portWidth == 0)
            panic(ropCacheName, ropCacheName, "Port width must be at least one byte.");
        if (inputRequests == 0)
            panic(ropCacheName, ropCacheName, "Input request queue must have at least one entry.");
        if (outputRequests == 0)
            panic(ropCacheName, ropCacheName, "Output request queue must have at least one entry.");
        if (numStampUnits == 0)
            panic(ropCacheName, ropCacheName, "The number of stamp units must be at least 1.");
        if (stampUnitStride == 0)
            panic(ropCacheName, ropCacheName, "The cache block stride per stamp unit must be at least 1.");
        //if (maxBlocks < ((MAX_DISPLAY_RES_X * MAX_DISPLAY_RES_Y) / (lineSize >> 2)))
        //    panic(ropCacheName, ropCacheName, "Block state memory too small for the highest supported resolution.");
        if (blocksCycle == 0)
            panic(ropCacheName, ropCacheName, "At lest one block state entry should be cleared per cycle.");
        if (comprLatency == 0)
            panic(ropCacheName, ropCacheName, "Compression unit latency must be at least 1.");
        if (decomprLatency == 0)
            panic(ropCacheName, ropCacheName, "Decompression unit latency must be at least 1.");
    )

    //  Create statistics.
    noRequests = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("NoNewRequests", u32bit(0), "RopCache", postfix);
    writeQFull = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("WriteReqQFull", u32bit(0), "RopCache", postfix);
    readQFull = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadReqQFull", u32bit(0), "RopCache", postfix);
    rwQFull = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("RWReqFull", u32bit(0), "RopCache", postfix);
    waitWriteStall = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("WaitWriteStall", u32bit(0), "RopCache", postfix);
    writeReqQueued = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("WriteReqAccepted", u32bit(0), "RopCache", postfix);
    readReqQueued = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadReqAccepted", u32bit(0), "RopCache", postfix);
    readInQEmpty = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadInputQEmpty", u32bit(0), "RopCache", postfix);
    decomprBusy = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("DecompressorBusy", u32bit(0), "RopCache", postfix);
    waitReadStall = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadInputWaitReadStall", u32bit(0), "RopCache", postfix);
    writeOutQEmpty = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("WriteOutQEmpty", u32bit(0), "RopCache", postfix);
    comprBusy = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("CompressorBusy", u32bit(0), "RopCache", postfix);
    memReqStall = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MemReqStall", u32bit(0), "RopCache", postfix);
    warStall = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("CacheLineWARStall", u32bit(0), "RopCache", postfix);
    dataBusBusy = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("DataBusBusy", u32bit(0), "RopCache", postfix);
    writePortStall = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("WriteLinePortStall", u32bit(0), "RopCache", postfix);
    readPortStall = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadLinePortStall", u32bit(0), "RopCache", postfix);

    comprBlocks64 = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ComprBlock64", u32bit(0), "RopCache", postfix);
    comprBlocks128 = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ComprBlock128", u32bit(0), "RopCache", postfix);
    comprBlocks192 = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ComprBlock192", u32bit(0), "RopCache", postfix);
    comprBlocks256 = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ComprBlock256", u32bit(0), "RopCache", postfix);

    //  Create the fetch cache object.
    cache = new FetchCache(ways, lines, lineSize, reqQSize, postfix);

    //  Create the input buffer.
    inputBuffer = new u8bit*[inputRequests];

    //  Check allocation.
    GPU_ASSERT(
        if (inputBuffer == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating input buffers.");
    )

    //  Create the output buffer.
    outputBuffer = new u8bit*[outputRequests];

    //  Check allocation.
    GPU_ASSERT(
        if (outputBuffer == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating output buffers.");
    )

    //  Allocate the mask buffers.
    maskBuffer = new u32bit*[outputRequests];

    //  Check allocation.
    GPU_ASSERT(
        if (maskBuffer == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating mask buffers.");
    )

    //  Create read request queue.
    readQueue = new ReadRequest[inputRequests];

    //  Check allocation.
    GPU_ASSERT(
        if (readQueue == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating the read queue.");
    )

    //  Create write request queue.
    writeQueue = new WriteRequest[outputRequests];

    //  Check allocation.
    GPU_ASSERT(
        if (writeQueue == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating the write queue.");
    )

    //  Allocate individual buffers.
    for(i = 0; i < inputRequests; i++)
    {
        //  Allocate input buffer.
        inputBuffer[i] = new u8bit[lineSize];

        //  Check allocation.
        GPU_ASSERT(
            if (inputBuffer[i] == NULL)
                panic(ropCacheName, ropCacheName, "Error allocating input buffer.");
        )
    }

    //  Allocate individual buffers.
    for(i = 0; i < outputRequests; i++)
    {
        //  Allocate output buffer.
        outputBuffer[i] = new u8bit[lineSize];

        //  Check allocation.
        GPU_ASSERT(
            if (outputBuffer[i] == NULL)
                panic(ropCacheName, ropCacheName, "Error allocating output buffer.");
        )

        //  Allocate mask buffer.
        maskBuffer[i] = new u32bit[lineSize];

        //  Check allocation.
        GPU_ASSERT(
            if (maskBuffer[i] == NULL)
                panic(ropCacheName, ropCacheName, "Error allocating mask buffer.");
        )
    }

    //  Allocate the ROP data buffer block state table.
    blockState = new ROPBlockState[maxBlocks];

    //  Check allocation.
    GPU_ASSERT(
        if (blockState == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating the block state information.");
    )

    //  Allocate the block state buffer that stores block state info in binary encoded format (4 bits per block).
    blockStateBuffer = new u8bit[maxBlocks >> 1];

    //  Check allocation.
    GPU_ASSERT(
        if (blockStateBuffer == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating the block state buffer (encoded data).");
    )

    //  Allocate the write port cycle counters.
    writeCycles = new u32bit[writePorts];

    //  Check allocation.
    GPU_ASSERT(
        if (writeCycles == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating write port cycle counters.");
    )

    //  Allocate the read port cycle counters.
    readCycles = new u32bit[readPorts];

    //  Check allocation.
    GPU_ASSERT(
        if (writeCycles == NULL)
            panic(ropCacheName, ropCacheName, "Error allocating read port cycle counters.");
    )

    //  Reset clear color vale.
    for(u32bit i = 0; i < MAX_BYTES_PER_PIXEL; i++)
        clearResetValue[i] = 0x00;
        
    //  Calculate the shift bits for block addresses.
    blockShift = GPUMath::calculateShift(lineSize);

    //  Set clear mode to false.
    clearMode = false;

    //  Set flush mode to false.
    flushMode = false;

    //  Set save block state info mode to false.
    saveStateMode = false;

    //  Set restore block state info mode to false.
    restoreStateMode = false;

    //  Reset the ROP cache.
    resetMode = true;

    //  No fetch performed yet (avoid warnings).
    fetchPerformed = false;

    //  Set pointers to the next ports (avoid warnings).
    nextReadPort = nextWritePort = 0;

    //  Set the ticket list to the maximum number of memory tickets.
    ticketList.resize(MAX_MEMORY_TICKETS);
    
    string queueName;
    queueName.clear();
    queueName = "ReadTicketQueue";
    queueName.append(postfix);
    ticketList.setName(queueName);    
}

//  Fetches a cache line.
bool ROPCache::fetch(u32bit address, u32bit &way, u32bit &line, DynamicObject *source, u32bit reserves)
{
    //  Check if a fetch was already performed.
    if (fetchPerformed)
        return false;

    //  Set fetch performed in this cycle.
    fetchPerformed = true;

    //  Try to fetch the address in the Fetch Cache.
    return cache->fetch(address, way, line, source, reserves);
}

//  Allocates a cache/buffer line.
bool ROPCache::allocate(u32bit address, u32bit &way, u32bit &line, DynamicObject *source, u32bit reserves)
{
    u32bit i;
    u32bit e;
    u32bit block;
    bool wait;

    //  Check if a fetch/allocate was already performed.
    if (fetchPerformed)
        return false;

    //  Set fetch/allocate performed in this cycle.
    fetchPerformed = true;

    //  Check if compression is enabled.
    if (compression)
    {
        //  Calculate block for allocation address.
        block = addressToBlockSU(address);

        //  Wait for the block to be written and compressed before allocate it.
        wait = false;

        //  Search the block inside the write queue.
        for(i = 0, e = GPU_MOD(nextFreeWrite + freeWrites, outputRequests);
            ((i < (outputRequests - freeWrites)) && (!wait));
            i++, e = GPU_MOD(e + 1, outputRequests))
        {
            //  Check if it is the same block.
            if (writeQueue[e].block == block)
            {
                //  Set wait write.
                wait = true;
                GPU_DEBUG(
                    printf("%s%02d => Block %d for address %x is on the write queue.\n",
                        ropCacheName, cacheID, block, address);
                    printf("%s%02d => Wait before allocation.\n", ropCacheName, cacheID);
                )
            }
        }

        //  Check if the allocation must wait.
        if (wait)
            return false;

        //  Check the state of the block.
        switch(blockState[block].state)
        {
            case ROPBlockState::CLEAR:
            case ROPBlockState::UNCOMPRESSED:

                //  The line is not required to be loaded, just try to make an allocation
                //  in the Fetch Cache.
                return cache->allocate(address, way, line, source, reserves);

            case ROPBlockState::COMPRESSED:
            
                //  The line must be load from memory, try to fetch the address in the Fetch Cache.
                return cache->fetch(address, way, line, source, reserves);
        }
    }
    else
    {
        //  When compression is disabled block state is not accessed and all requests are
        //  for uncompressed data.

        //  The line is not required to be loaded, just try to make an allocation in the Fetch Cache.
        return cache->allocate(address, way, line, source, reserves);
    }
    
    return false;
}

//  Reads ROP data for a stamp.
bool ROPCache::read(u32bit address, u32bit way, u32bit line, u32bit size, u8bit *data)
{
    //  Search for the next free write port.
    for(; (nextReadPort < readPorts) && (readCycles[nextReadPort] > 0); nextReadPort++);

    //  Check if cache read port is busy.
    if ((nextReadPort < readPorts) && (readCycles[nextReadPort] == 0))
    {
        //  Try to read fetched data from the Fetch Cache.
        if (cache->read(address, way, line, size, data))
        {
            //  Update read cycles counter.
            readCycles[nextReadPort] += (u32bit) ceil((f32bit) size / (f32bit) portWidth);

            //  Update pointer to next read port.
            nextReadPort++;

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

//  Writes masked ROP data from a stamp (write buffer mode).
bool ROPCache::write(u32bit address, u32bit way, u32bit line, u32bit size, u8bit *data, bool *mask)
{
    //  Search for the next free write port.
    for(; (nextWritePort < writePorts) && (writeCycles[nextWritePort] > 0); nextWritePort++);

    //  Check if write port is busy.
    if ((nextWritePort < writePorts) && (writeCycles[nextWritePort] == 0))
    {
        //  Tries to perform a masked write to the fetch cache.
        if (cache->write(address, way, line, size, data, mask))
        {
            //  Update write cycles counter.
            writeCycles[nextWritePort] += (u32bit) ceil((f32bit) size / (f32bit) portWidth);

            //  Update pointer to the next write port.
            nextWritePort++;

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}


//  Writes ROP data from a stamp.
bool ROPCache::write(u32bit address, u32bit way, u32bit line, u32bit size, u8bit *data)
{
    //  Search for the next free write port.
    for(; (nextWritePort < writePorts) && (writeCycles[nextWritePort] > 0); nextWritePort++);

    //  Check if the cache write port is busy.
    if ((nextWritePort < writePorts) && (writeCycles[nextWritePort] == 0))
    {
        //  Tries to perform a write to the fetch cache.
        if (cache->write(address, way, line, size, data))
        {
            //  Update write cycles counter.
            writeCycles[nextWritePort] += (u32bit) ceil((f32bit) size / (f32bit) portWidth);

            //  Update pointer to the next write port.
            nextWritePort++;

            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
}

//  Unreserves a cache line after use.
void ROPCache::unreserve(u32bit way, u32bit line)
{
    //  Unreserve fetch cache line.
    cache->unreserve(way, line);
}

//  Resets the ROP cache structures.
void ROPCache::reset()
{
    //  Unset clear mode.
    clearMode = false;

    //  Reset the color cache.
    resetMode = true;
}

//  Process a memory transaction.
void ROPCache::processMemoryTransaction(MemoryTransaction *memTrans)
{
    //  Process memory transaction.
    switch(memTrans->getCommand())
    {
        case MT_READ_DATA:

            //  Check if memory bus is in use.
            GPU_ASSERT(
                if (memoryCycles > 0)
                    panic(ropCacheName, "processMemoryTransaction", "Memory bus still busy.");
            )

            //  Get ticket of the transaction.
            readTicket = memTrans->getID();

            //  Keep transaction size.
            lastSize = memTrans->getSize();

            GPU_DEBUG(
                printf("%s%02d => MT_READ_DATA ticket %d address %x size %d\n",
                    ropCacheName, cacheID, readTicket, memTrans->getAddress(), lastSize);
            )

//if (memTrans->getAddress() == 0x001cc480)
//{
//printf("%s%02d => MT_READ_DATA ticket %d address %x size %d\n",
//ropCacheName, cacheID, readTicket, memTrans->getAddress(), lastSize);
//printf("%s%02d => data : ", ropCacheName, cacheID);
//u8bit *p = memTrans->getData();
//for(u32bit dw = 0; dw < (lastSize >> 2); dw++)
//printf("%08x ", ((u32bit *) p)[dw]);
//printf("\n");
//}
            //  Set cycles the memory bus will be in use.
            memoryCycles = memTrans->getBusCycles();

            //  Set memory read in progress flag.
            memoryRead = true;

            break;

        default:

            panic(ropCacheName, "processMemoryTransaction", "Unsopported transaction type.");
            break;
    }
}


//  Update the memory request queue.  Generate new memory transactions.
MemoryTransaction *ROPCache::update(u64bit cycle, MemState memState)
{
    //  Set memory controller state.
    memoryState = memState;

    //  Simulate a ROP cache cycle.
    clock(cycle);

    //  Return the generated (if any) memory transaction.
    return nextTransaction;
}

//  Copy back all the valid cache lines to memory.
bool ROPCache::flush()
{
    //  Check if the cache has been requested to be flushed.
    if (!flushRequest)
    {
        //  Cache request received.
        flushRequest = true;

        //  Enable flush mode.
        flushMode = true;
    }
    else
    {
        //  Check if cache flush has finished.
        if (!flushMode)
        {
            //  End of cache flush request.
            flushRequest = false;
        }
    }

    return flushMode;
}

//  Save the block state info into the block state buffer in memory.
bool ROPCache::saveState()
{
    //  Check if there is a pending request for saving the block state info into memory.
    if (!saveStateRequest)
    {
        GPU_DEBUG(
            printf("%s%02d => Starting Save State.\n", ropCacheName, cacheID);
        )
        
        //  Flag that the request has been received.
        saveStateRequest = true;

        //  Enable saving block state info mode.
        saveStateMode = true;
    }
    else
    {
        //  Check if the process of saving the block state info into memory has finished.
        if (!saveStateMode)
        {
            GPU_DEBUG(
                printf("%s%02d => Finishing Save State.\n", ropCacheName, cacheID);
            )
            
            //  Set the request flag to false.
            saveStateRequest = false;
        }
    }

    return saveStateMode;
}

//  Restore the block state info from the block state buffer in memory.
bool ROPCache::restoreState()
{
    //  Check if there is a pending request for restoring the block state info from memory.
    if (!restoreStateRequest)
    {
        GPU_DEBUG(
            printf("%s%02d => Starting Restore State.\n", ropCacheName, cacheID);
        )
        
        //  Flag that the request has been received.
        restoreStateRequest = true;

        //  Enable restoring block state info mode.
        restoreStateMode = true;
    }
    else
    {
        //  Check if the process of restoring the block state info from memory has finished.
        if (!restoreStateMode)
        {
            GPU_DEBUG(
                printf("%s%02d => Finishing Restore State.\n", ropCacheName, cacheID);
            )
            
            //  Set the request flag to false.
            restoreStateRequest = false;
        }
    }

    return restoreStateMode;
}

//  Check and initiate the reset state block info mode.
bool ROPCache::resetState()
{
    //  Check if the reset state mode has been already requested.
    if (!resetStateRequest)
    {
        GPU_DEBUG(
            printf("%s%02d => Starting Reset State.\n", ropCacheName, cacheID);
        )        
        
        //  Not requested.  Start the reset state mode.
        resetStateRequest = true;
        resetStateMode = true;

        //  NOTE:  SHOULD TAKE INTO ACCOUNT THE RESOLUTION SO NOT ALL
        //  BLOCKS HAD TO BE CLEARED EVEN IF UNUSED AT CURRENT RESOLUTION.  */

        //  Compute cycles required to reset all the block state info.
        resetStateCycles = (u32bit) ceil((f32bit) maxBlocks / (f32bit) blocksCycle);
    }
    else
    {
        //  Alreayd requested.  Check if already finished.
        if (!resetStateMode)
        {
            GPU_DEBUG(
                printf("%s%02d => Finishing Reset State.\n", ropCacheName, cacheID);
            )
            
            //  Finished.  Remove the reset state request.
            resetStateRequest = false;
        }
    }
    
    return resetStateMode;
}

//  Signals a swap in the ROP data buffer address.
void ROPCache::swap(u32bit bufferAddress)
{
    //  Sets the ROP data buffer addresss.
    ropBufferAddress = bufferAddress;
}

//  Sets the bytes per pixel for the current pixel data format.
void ROPCache::setBytesPerPixel(u32bit bytes)
{
    //  Set the bytes per pixel.
    bytesPixel = bytes;
}

//  Set the number of MSAA samples for the current framebuffer.
void ROPCache::setMSAASamples(u32bit samples)
{
    msaaSamples = samples;
}

//  Sets the compression flag.  Used to enable or disable buffer compression.
void ROPCache::setCompression(bool compr)
{
    //  Set the compression enable/disable flag.
    compression = compr;

    char fullName[128];
}

//  Sets the address of the block state buffer in memory.
void ROPCache::setStateAddress(u32bit address)
{
    //  Set the block state buffer address.
    ropStateAddress = address;
}


//  ROP cache simulation rutine.
void ROPCache::clock(u64bit cycle)
{
    u32bit i, e;
    u32bit readEndReq;
    MemoryTransaction *memTrans;
    u32bit fillReadRequest;
    bool waitWrite;
    u8bit comprBuffer[UNCOMPRESSED_BLOCK_SIZE];

    GPU_DEBUG(
        printf("%s%02d => Cycle %lld\n", ropCacheName, cacheID, cycle);
    )

    //  Receive memory transactions and state from Memory Controller.

    //  Check if flushing.
    if (flushMode)
    {
        GPU_DEBUG(
            printf("%s%02d => Flush.\n", ropCacheName, cacheID);
        )

        //  Continue flushing the cache lines.
        flushMode = !cache->flush();
    }

    //  Check if the ROP cache is in reset mode.
    if (resetMode)
    {
        //  Reset the ROP cache state.
        performeReset();
    }

    //  NOTE:  TO BE IMPLEMENTED WHEN THE CLASS IS CONVERTED TO A
    //  BOX AND COMMUNICATION GOES THROUGH SIGNALS.

    //  Process cache commands.
    //  Process cache reads.
    //  Process cache writes.

    //  Unset fetch performed for the current cycle.
    fetchPerformed = false;

    //  Update read port cycle counters.
    for(i = 0; i < readPorts; i++)
    {
        //  Check if there is line being read from the cache.
        if (readCycles[i] > 0)
        {
            GPU_DEBUG(
                printf("%s%02d => Remaining cache read cycles %d.\n", ropCacheName, cacheID, readCycles[i]);
            )

            //  Update read cycles counter.
            readCycles[i]--;

            //  Check if read has finished.
            if (readCycles[i] == 0)
            {
                //  Reset pointer to the next read port to the first free read port.
                if (i < nextReadPort)
                    nextReadPort = i;

                GPU_DEBUG(
                    printf("%s%02d => Cache read end.\n", ropCacheName, cacheID);
                )

                //  Check if a line was being read.
                if (readingLine && (readLinePort == i))
                {
                    //  Update number of write blocks read from the cache.
                    writeOutputs++;

                    //  Unset reading line flag.
                    readingLine = false;
                }
            }
        }
        else
        {
            //  Reset pointer to the next read port to the first free read port.
            if (i < nextReadPort)
                nextReadPort = i;
        }
    }

    //  Update write port cycle counters.
    for(i = 0; i < writePorts; i++)
    {
        //  Check if there is a line being written to the cache.
        if (writeCycles[i] > 0)
        {
            GPU_DEBUG(
                printf("%s%02d => Cache write cycles %d\n", ropCacheName, cacheID, writeCycles[i]);
            )

            //  Update write port cycles.
            writeCycles[i]--;

            //  Check if write has finished.
            if (writeCycles[i] == 0)
            {
                //  Reset pointer to the next write port to the first free write port.
                if (i < nextWritePort)
                    nextWritePort = i;

                //  Check if there was a line being written.
                if (writingLine && (writeLinePort == i))
                {
                    GPU_DEBUG(
                        printf("%s%02d => End of cache write.\n", ropCacheName, cacheID);
                    )

                    //  Free fill cache request.
                    cache->freeRequest(readQueue[nextUncompressed].requestID, false, true);

                    //  Clear cache request pointer.
                    readQueue[nextUncompressed].request = NULL;

                    //  Update pointer to the next uncompressed block.
                    nextUncompressed = GPU_MOD(nextUncompressed + 1, inputRequests);

                    //  Update free inputs counter.
                    freeReads++;

                    //  Update the number of input requests writing to a cache line.
                    readsWriting--;

                    //  Unset writing line flag.
                    writingLine = false;
                }
            }
        }
        else
        {
            //  Reset pointer to the next write port to the first free write port.
            if (i < nextWritePort)
                nextWritePort = i;
        }
    }

    //  No memory transaction generated yet.
    memTrans = NULL;

    //  Check if there is no cache request to process.
    if (cacheRequest == NULL)
    {
        //  Receive request from the cache.
        cacheRequest = cache->getRequest(requestID);
    }

    //  Check if there is a cache request being served.
    if ((cacheRequest != NULL) && (freeWrites > 0) && (freeReads > 0))
    {

        //  Do not wait for a write request.
        waitWrite = false;

        //  Set read request that will be assigned to the fill request.
        fillReadRequest = nextFreeRead;

        //  If it is a cache spill add to the write queue.
        if (cacheRequest->spill)
        {
            GPU_ASSERT(
                //  Search in the read request queue for the address to write.
                for(i = 0, e = GPU_MOD(nextFreeRead - 1, inputRequests); i < (inputs + readInputs + uncompressed);
                    i++, e = GPU_MOD(e - 1, inputRequests))
                {
                    //  Check if it is the same line.
                    if (readQueue[e].address == cacheRequest->outAddress)
                    {
                        printf("e %d i %d inputs %d readInputs %d uncompressed %d\n",
                            e, i, inputs, readInputs, uncompressed);
                        printf("nextInput %d nextRead %d nextUncompressed %d nextFreeRead %d\n",
                            nextInput, nextRead, nextUncompressed, nextFreeRead);
                        printf("Request address %x ReadQueue address %x\n",
                            cacheRequest->outAddress, readQueue[e].address);
                        printf("Request line set = %d way = %d inAddress = %x\n", cacheRequest->way, cacheRequest->line,
                            cacheRequest->inAddress);
                        panic(ropCacheName, "clock", "Writing a line not fully read from the cache.");
                    }
                }
            )

//if (cacheRequest->outAddress == 0x001CC400)
//printf("%s%02d (%lld) => Adding spill for address %08x (%d, %d)\n", ropCacheName, cacheID, cycle,
//cacheRequest->outAddress, cacheRequest->way, cacheRequest->line);

            //  Add to the next free write queue entry.
            writeQueue[nextFreeWrite].address = cacheRequest->outAddress;
            writeQueue[nextFreeWrite].block = addressToBlockSU(cacheRequest->outAddress);
            writeQueue[nextFreeWrite].blockFB = addressToBlock(cacheRequest->outAddress);
            writeQueue[nextFreeWrite].size = 0;
            writeQueue[nextFreeWrite].written = 0;
            writeQueue[nextFreeWrite].request = cacheRequest;
            writeQueue[nextFreeWrite].requestID = requestID;

            //  Check if the request is also doing a fill.
            if (cacheRequest->fill)
            {
                //  Set that the paired fill read request has a dependency with this write request.
                writeQueue[nextFreeWrite].isReadWaiting = true;

                //  Set the read queue entry waiting for this write request.
                writeQueue[nextFreeWrite].readWaiting = fillReadRequest;
            }
            else
            {
                //  Only spill request.  No dependency with a read request.
                writeQueue[nextFreeWrite].isReadWaiting = false;
            }
        }

        //  Add a read request to the queue if the cache request is a fill.
        if ((cacheRequest->fill) && (freeReads > 0))
        {
            GPU_DEBUG(
                printf("%s%02d => Adding read (fill) request %d line (%d, %d) to address %x\n",
                    ropCacheName, cacheID, nextFreeRead, cacheRequest->way, cacheRequest->line,
                    cacheRequest->inAddress);
            )

            //  Search in the write request queue for the address to read.
            for(i = 0, e = GPU_MOD(nextFreeWrite + freeWrites, outputRequests);
                ((i < (outputRequests - freeWrites)) && (!waitWrite));
                i++, e = GPU_MOD(e + 1, outputRequests))
            {
                //  Check if it is the same line.
                if (writeQueue[e].address == cacheRequest->inAddress)
                {
                    //  Set wait write.
                    waitWrite = true;
                    GPU_DEBUG(
                        printf("%s%02d => Read request address %x found at write queue entry %d.\n",
                            ropCacheName, cacheID, cacheRequest->inAddress, e);
                        printf("%s%02d => Waiting for end of write request.\n", ropCacheName, cacheID);
                    )
                }
            }

            //  NOTE: THIS IS A CLEARLY A CONFLICT MISS AND IT SHOULDN'T BE THAT
            //  FREQUENT UNLESS THE REPLACING POLICY IS NOT WORKING WELL.

            //  Check if the read request must wait to a write request to end.
            if (!waitWrite)
            {

//if (cacheRequest->inAddress == 0x001CC400)
//printf("%s%02d (%lld) => Adding fill for address %08x (%d, %d)\n", ropCacheName, cacheID, cycle,
//cacheRequest->outAddress, cacheRequest->way, cacheRequest->line);

                //  Add to the next free read queue entry.
                readQueue[nextFreeRead].address = cacheRequest->inAddress;
                readQueue[nextFreeRead].block = addressToBlockSU(cacheRequest->inAddress);
                readQueue[nextFreeRead].size = 0;
                readQueue[nextFreeRead].requested = 0;
                readQueue[nextFreeRead].received = 0;
                readQueue[nextFreeRead].request = cacheRequest;
                readQueue[nextFreeRead].requestID = requestID;

                //  Set wait flag for a write queue to read from the same cache line.
                readQueue[nextFreeRead].writeWait = cacheRequest->spill;
                
                //  Set wait falg for a write queues to complete a spill if requesting
                //  spilling and filling from the same address..
                readQueue[nextFreeRead].spillWait = cacheRequest->spill && (cacheRequest->inAddress == cacheRequest->outAddress);

//if (cacheRequest->inAddress == 0x001CC400)
//if (readQueue[nextFreeRead].spillWait)
//printf("%s%02d (%lld) => inAddress = %08x outAddress = %08x nextFreeRead = %d spillWait = %s\n", ropCacheName, cacheID, cycle,
//cacheRequest->inAddress, cacheRequest->outAddress, nextFreeRead, readQueue[nextFreeRead].spillWait ? "T" : "F");

                //  Update inputs counter.
                inputs++;

                //  Update free read queue entries counter.
                freeReads--;

                //  Update pointer to the next free read queue entry.
                nextFreeRead = GPU_MOD(nextFreeRead + 1, inputRequests);
                
                //  Update statistics.
                readReqQueued->inc();
            }
        }

        if (!waitWrite && cacheRequest->spill)
        {
            GPU_DEBUG(
                printf("%s%02d => Adding write (spill) request %d line (%d, %d) to address %x\n",
                    ropCacheName, cacheID, nextFreeWrite, cacheRequest->way, cacheRequest->line,
                    cacheRequest->outAddress);
            )

            //  Update outputs counter.
            outputs++;

            //  Update free write queue entries counter.
            freeWrites--;

            //  Update pointer to the next free write queue entry.
            nextFreeWrite = GPU_MOD(nextFreeWrite + 1, outputRequests);
            
            //  Update statistics.
            writeReqQueued->inc();
        }

        //  Check if we could add the request to the queues.
        if (!waitWrite)
        {
            //  Release the cache request.
            cacheRequest = NULL;
        }
        else
        {
            // Update statistics.
            waitWriteStall->inc();
        }
    }
    else
    {
        // Update statistics.
        if (cacheRequest == NULL)
        {
            noRequests->inc();
        }
        else
        {
            if (freeWrites == 0)
            {
                writeQFull->inc();
                rwQFull->inc();
            }
            
            if (freeReads == 0)
            {
                readQFull->inc();
                rwQFull->inc();
            }
        }
    }


    //  Check if there is a memory read transaction being received.
    if (memoryCycles > 0)
    {
        GPU_DEBUG(
            printf("%s%02d => Remaining memory cycles %d\n", ropCacheName, cacheID, memoryCycles);
        )

        //  Update memory bus busy cycles counter.
        memoryCycles--;

        //  Check if transmission has finished.
        if (memoryCycles == 0)
        {
            //  Check if it was a read transaction.
            if(memoryRead)
            {
                //  Check for restore state mode.
                if (!restoreStateMode)
                {
                    //  Search ticket in the memory request table.
                    readEndReq = memoryRequest[readTicket];

                    GPU_DEBUG(
                        printf("%s%02d => End of read transaction for read request queue %d\n",
                            ropCacheName, cacheID, readEndReq);
                    )

                    //  Update read queue entry.
                    readQueue[readEndReq].received += lastSize;

                    //  Check if the full block was received.
                    if (readQueue[readEndReq].received == readQueue[readEndReq].size)
                    {
                        GPU_DEBUG(
                            printf("%s%02d => Input block %d fully read.\n",
                                ropCacheName, cacheID, readEndReq);
                        )

                        //  Update number of read input blocks.
                        readInputs++;

                        //  Update the number of inputs requested to memory.
                        inputsRequested--;

                    }
                }
                else
                {
                    //  Update the number of blocks restored from memory.
                    restoredBlocks += lastSize << 1;

                    GPU_DEBUG(
                        printf("%s%02d (%lld) => End of read transaction for restore block state info.  Restored blocks %d.\n",
                            ropCacheName, cacheID, cycle, restoredBlocks);
                    )

                }

                //  Add ticket to read ticket list.
                ticketList.add(readTicket);
                
                //  Unset read on progress flag.
                memoryRead = false;
            }

            //  Check if it was a write transaction.
            if (memoryWrite)
            {
                //  Check for restore state mode.
                if (!saveStateMode)
                {
                    GPU_DEBUG(
                        printf("%s%02d => End of write transaction for write request queue %d\n",
                            ropCacheName, cacheID, nextCompressed);
                    )

                    //  Check if current block has been fully written.
                    if (writeQueue[nextCompressed].written == writeQueue[nextCompressed].size)
                    {
                        GPU_DEBUG(
                            printf("%s%02d => Output block %d fully written.\n",
                                ropCacheName, cacheID, nextUncompressed);
                        )

                        //  Free spill cache request.
                        cache->freeRequest(writeQueue[nextCompressed].requestID, true, false);

//printf("%s%02d (%lld) => Finishing spill nextCompressed = %d isReadWaiting = %s readWaiting = %d\n", ropCacheName, cacheID, cycle,
//nextCompressed, writeQueue[nextCompressed].isReadWaiting ? "T" : "F", writeQueue[nextCompressed].readWaiting);

                        //  Signal waiting read request that the cache line has been read and written to memory (spill complete).
                        if (writeQueue[nextCompressed].isReadWaiting &&
                            (readQueue[writeQueue[nextCompressed].readWaiting].request == writeQueue[nextCompressed].request))
                        {
//if (writeQueue[nextCompressed].address == 0x001CC400)
//printf("%s%02d (%lld) => Clearing spill wait flag for read queue entry %d\n", ropCacheName, cacheID, cycle,
//writeQueue[nextCompressed].readWaiting);
                        
                            //  Clear write wait flag for the read queue.
                            readQueue[writeQueue[nextCompressed].readWaiting].spillWait = false;
                        }

                        //  Clear cache request pointer.
                        writeQueue[nextCompressed].request = NULL;

                        //  Update pointer to next compressed block to write.
                        nextCompressed = GPU_MOD(nextCompressed + 1, outputRequests);

                        //  Update number of compressed blocks.
                        compressed--;

                        //  Update free outputs counter.
                        freeWrites++;
                    }
                }
                else
                {
                    GPU_DEBUG(
                        printf("%s%02d => End of write transaction for save block state info\n", ropCacheName, cacheID);
                    )
                }
                
                //  Unset write on progress flag.
                memoryWrite = false;
            }

            //  Update number of free tickets.
            freeTickets++;
        }
        
        // Update statistics.
        dataBusBusy->inc();
    }

    //  Request input blocks to memory.
    if (inputs > 0)
    {
    
//if (readQueue[nextInput].spillWait)
//printf("%s%02d (%lld) => Waiting for spill to finish nextInput = %d\n", ropCacheName, cacheID, cycle, nextInput);

        //  Check if the read request must wait before writing the cache line.
        if (!readQueue[nextInput].spillWait)
        {
            const ROPBlockState& block = (compression && (readQueue[nextInput].block < maxBlocks)) ? blockState[readQueue[nextInput].block] :
                                                                                                     ROPBlockState(ROPBlockState::UNCOMPRESSED);
            
//printf("%s%02d => Block state -> address %08x block %d maxBlocks %d compression %s compressedOrig %s compressedFinal %s\n",
//ropCacheName, cacheID, readQueue[nextInput].address, readQueue[nextInput].block, maxBlocks, compression ? "T" : "F",
//((blockState[readQueue[nextInput].block].state == ROPBlockState::CLEAR) ? "CLEAR" : 
//    ((blockState[readQueue[nextInput].block].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM")),
//((block.state == ROPBlockState::CLEAR) ? "CLEAR" : 
//    ((block.state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM")));
    
            //  Check compression state for the current block.
            switch(block.state)
            {
                case ROPBlockState::CLEAR:

                    //  Block is cleared so it shouldn't be requested to memory.

                    GPU_DEBUG(
                        printf("%s%02d => Input Request %d is for CLEAR block %d.\n",
                            ropCacheName, cacheID, nextInput, readQueue[nextInput].block);
                    )

                    //  Set block size and received bytes.
                    readQueue[nextInput].size = readQueue[nextInput].received = lineSize;

                    //  Update pointer to next input.
                    nextInput = GPU_MOD(nextInput + 1, inputRequests);

                    //  Update number of inputs.
                    inputs--;

                    //  Update number of inputs read from memory.
                    readInputs++;

                    break;

                case ROPBlockState::UNCOMPRESSED:

                    //  Block is uncompressed in memory.  Request the full block.

                    GPU_DEBUG(
                        printf("%s%02d => Input Request %d is for UNCOMPRESSED block %d.\n",
                            ropCacheName, cacheID, nextInput, readQueue[nextInput].block);
                    )

                    //  If there is no previous request for the entry.
                    if (readQueue[nextInput].size == 0)
                    {
                        //  Set uncompressed block size.
                        readQueue[nextInput].size = lineSize;
                    }

                    //  Request data to memory.
                    memTrans = requestBlock(cycle, nextInput);

                    //  Copy memory transaction cookies.

                    //  Send memory transaction to the memory controller.

                    //  Check if current input block has been fully requested.
                    if (readQueue[nextInput].requested == readQueue[nextInput].size)
                    {

                        GPU_DEBUG(
                            printf("%s%02d => Input request %d fully requested to memory.\n",
                                ropCacheName, cacheID, nextInput);
                        )

                        //  Update pointer to the next input.
                        nextInput = GPU_MOD(nextInput + 1, inputRequests);

                        //  Update waiting inputs counter.
                        inputs--;

                        //  Update the number of inputs requested to memory.
                        inputsRequested++;
                    }

                    break;

                case ROPBlockState::COMPRESSED:

                    //  Request a compressed block.

                    GPU_DEBUG(
                        printf("%s%02d => Input Request %d is for COMPRESSED block %d.\n",
                            ropCacheName, cacheID, nextInput, readQueue[nextInput].block);
                    )

                    //  If there is no previous request for the entry.
                    if (readQueue[nextInput].size == 0)
                    {
                        //  Set compressed block size.
                        readQueue[nextInput].size = 
                            getCompressor().getLevelBlockSize(block.comprLevel);
                    }

                    //  Request data to memory.
                    memTrans = requestBlock(cycle, nextInput);

                    //  Copy memory transaction cookies.

                    //  Send memory transaction to the memory controller.

                    //  Check if current input block has been fully requested.
                    if (readQueue[nextInput].requested == readQueue[nextInput].size)
                    {

                        GPU_DEBUG(
                            printf("%s%02d => Input request %d fullyrequested to memory.\n",
                                ropCacheName, cacheID, nextInput);
                        )

                        //  Update pointer to the next input.
                        nextInput = GPU_MOD(nextInput + 1, inputRequests);

                        //  Update waiting inputs counter.
                        inputs--;

                        //  Update the number of inputs requested to memory.
                        inputsRequested++;
                    }

                    break;

                default:

                    panic(ropCacheName, "clock", "Unsupported ROP data block state mode.");
            }
        }
    }

    //  Request output blocks to the cache.
    if ((outputs > 0) && (!readingLine) &&  (nextReadPort < readPorts) && (readCycles[nextReadPort] == 0))
    {
        //  Try to read the cache line for the write request.
        if (cache->readLine(writeQueue[nextOutput].request->way,
            writeQueue[nextOutput].request->line, outputBuffer[nextOutput]))
        {
            GPU_DEBUG(
                printf("%s%02d => Reading output block %d from cache.\n", ropCacheName, cacheID, nextOutput);
            )

            //  Check if it is a masked write.
            if (writeQueue[nextOutput].request->masked)
            {
                //  Get the mask for the cache line.
                cache->readMask(writeQueue[nextOutput].request->way,
                    writeQueue[nextOutput].request->line, maskBuffer[nextOutput]);
            }
                    
            //  Reset the cache line mask.
            cache->resetMask(writeQueue[nextOutput].request->way, writeQueue[nextOutput].request->line);

            //  Set cycles until the line is fully received.
            readCycles[nextReadPort] += (u32bit) ceil( (f32bit) lineSize/ (f32bit) portWidth);

            //  Set port the line read.
            readLinePort = nextReadPort;

            //  Update pointer to the next read port.
            nextReadPort++;

            //  Set reading line flag.
            readingLine = true;

            //  Signal waiting read request that the cache line has been read and written to memory (spill complete).
            if (writeQueue[nextOutput].isReadWaiting &&
                (readQueue[writeQueue[nextOutput].readWaiting].request == writeQueue[nextOutput].request))
            {
                //  Unset write wait flag for the read queue.
                readQueue[writeQueue[nextOutput].readWaiting].writeWait = false;
            }

            //  Update pointer to next output.
            nextOutput = GPU_MOD(nextOutput + 1, outputRequests);

            //  Update write requests counter.
            outputs--;
        }
    }
    else
    {
        // Update statistics.
        if (outputs > 0)
            readPortStall->inc();
    }

    //  Uncompress input blocks.
    if (uncompressCycles > 0)
    {
        GPU_DEBUG(
            printf("%s%02d => Remaining cycles to decompress block %d\n", ropCacheName, cacheID,
                uncompressCycles);
        )

        //  Update uncompress cycles.
        uncompressCycles--;

        //  Check if uncompression has finished.
        if (uncompressCycles == 0)
        {
            GPU_DEBUG(
                printf("%s%02d => End of block decompression.\n", ropCacheName, cacheID);
            )

            //  Update number of uncompressed inputs.
            uncompressed++;

            //  Update the number of blocks being uncompressed.
            uncompressing--;
        }

        //  Update statistics.
        decomprBusy->inc();
    }

    //  Check if there are blocks to uncompress and the uncompressor is ready.
    if ((readInputs > 0) && (uncompressCycles == 0))
    {
        //  Check if the next read input has been fully received from memory.
        if (readQueue[nextRead].size == readQueue[nextRead].received)
        {
            //  Set uncompression cycles.
            uncompressCycles = decomprLatency;

            const ROPBlockState& nextReadBlock = (compression && (readQueue[nextRead].block < maxBlocks)) ? blockState[readQueue[nextRead].block] :
                                                                                                            ROPBlockState(ROPBlockState::UNCOMPRESSED);

            //  Check block state.
            switch(nextReadBlock.state)
            {
                case ROPBlockState::CLEAR:

                    GPU_DEBUG(
                        printf("%s%02d => Filling with clear color input request %d.\n",
                            ropCacheName, cacheID, nextRead);
                    )

                    //  Fill the block/line with the clear ROP value.
                    for(i = 0; i < lineSize; i += bytesPixel)
                        for(u32bit j = 0; j < (bytesPixel >> 2); j++)
                            *((u32bit *) &inputBuffer[nextRead][i + j * 4]) = ((u32bit *) clearROPValue)[j];

                    break;

                case ROPBlockState::UNCOMPRESSED:

                    //  Nothing to do.

                    GPU_DEBUG(
                        printf("%s%02d => Uncompressed block for input request %d.\n",
                            ropCacheName, cacheID, nextRead);
                    )

                    break;

                case ROPBlockState::COMPRESSED:
                    
                    GPU_DEBUG(
                        printf("%s%02d => Compressed block for input request %d.\n",
                            ropCacheName, cacheID, nextRead);
                    );

                    // Copy the compressed input to the compression buffer.
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize); 
                    //TODO:getCompressor().getLevelBlockSize(nextReadBlock.compressionLevel));

                    // Uncompress the block.
                    getCompressor().uncompress(
                            comprBuffer, 
                            (u32bit *) inputBuffer[nextRead], 
                            lineSize >> 2, 
                            nextReadBlock.getComprLevel());

                    break;

                default:

                    panic(ropCacheName, "clock", "Unsupported block mode.");
                    break;
            }

            //  Update pointer to the input to decompress.
            nextRead = GPU_MOD(nextRead + 1, inputRequests);

            //  Update number of inputs to decompress.
            readInputs--;

            //  Update the number of blocks being uncompressed.
            uncompressing++;
        }
        else
        {
            //  Update statistics.
            waitReadStall->inc();
        }
    }
    else
    {
        //  Update statistics;
        if (readInputs == 0)
            readInQEmpty->inc();
    }

    //  Compress output blocks.
    if (compressCycles > 0)
    {
        GPU_DEBUG(
            printf("%s%02d => Block compression remaining cycles %d\n", ropCacheName, cacheID,
                compressCycles);
        )

        //  Update compress cycles.
        compressCycles--;

        //  Check if compression has finished.
        if (compressCycles == 0)
        {
            GPU_DEBUG(
                printf("%s%02d => Block compression end.\n", ropCacheName, cacheID);
            )

            //  Update number of compressed outputs.
            compressed++;
        }

        // Update statistics.
        comprBusy->inc();
    }

    //  Check if there are blocks to compress and the compressor is ready.
    if ((writeOutputs > 0) && (compressCycles == 0))
    {
        GPU_DEBUG(
            printf("%s%02d => Compressing block for output request %d\n", ropCacheName, cacheID, nextWrite);
        )

        //  Set compression cycles.
        compressCycles = comprLatency;

        ROPBlockState forceUncompressed(ROPBlockState::UNCOMPRESSED);
        
        //  Get the state of the block to be written.
        ROPBlockState& nextWriteBlock = (compression && (writeQueue[nextWrite].block < maxBlocks)) ? 
                                        blockState[writeQueue[nextWrite].block] : forceUncompressed;

        //  For clear blocks the non masked positions must be written too with the clear ROP value.
        if ((nextWriteBlock.state == ROPBlockState::CLEAR) && (writeQueue[nextWrite].request->masked))
        {
if (writeQueue[nextWrite].address == 0x00259f00)
{
printf("%s%02d => Block marked as CLEAR and masked.\n", ropCacheName, cacheID);
}
            //  Clear masked pixels
            for(i = 0; i < lineSize; i += bytesPixel)
            {
                for(u32bit j = 0; j < (bytesPixel >> 2); j++)
                {
                    //  Write clear ROP value on unmasked writes.
                    *((u32bit *) &outputBuffer[nextWrite][i + j * 4]) =
                        ((*((u32bit *) &outputBuffer[nextWrite][i + j * 4])) & maskBuffer[nextWrite][(i >> 2) + j]) |
                        (((u32bit *) clearROPValue)[j] & ~maskBuffer[nextWrite][(i >> 2) + j]);

                    //  Set pixel mask to true.
                    maskBuffer[nextWrite][(i >> 2) + j] = 0xffffffff;
                }
            }
        }

        //  Set state for the block written to memory
        blockWasWritten = true;
        writtenBlock = writeQueue[nextWrite].blockFB;
                
        // This call will be overriden by ZCache to calculate maxZ value
        processNextWrittenBlock(outputBuffer[nextWrite], lineSize);

        CompressorInfo comprInfo;
        
//printf("%s%02d => Compressing address %08x block %d -> disableCompr = %s compression = %s nextWriteBlock.state = %s\n",
//ropCacheName, cacheID, writeQueue[nextWrite].address, writeQueue[nextWrite].block,
//disableCompr ? "T" : "F", compression ? "T" : "F", 
//(nextWriteBlock.state == ROPBlockState::CLEAR) ? "CLEAR" : (nextWriteBlock.state == ROPBlockState::COMPRESSED ? "COMPR" : "UNCOMPR"));

        //  Check if it is an uncompressed block.
        if ((!disableCompr) && compression && (nextWriteBlock.state != ROPBlockState::UNCOMPRESSED) && (writeQueue[nextWrite].block < maxBlocks))
        {
            //  Copy uncompressed output to the compression buffer.
            memcpy(comprBuffer, outputBuffer[nextWrite], lineSize);

            //  Compress block/line.
            comprInfo = getCompressor().compress(
                    comprBuffer, 
                    outputBuffer[nextWrite], 
                    lineSize >> 2);
        }
        else
        {
            //  Do not recompress uncompressed blocks (not read in write only mode!!!).
            comprInfo = CompressorInfo(false, 0, lineSize);
        }
        
        // Update compressor statistics
        switch (comprInfo.size) {
        case 64: comprBlocks64->inc(); break;
        case 128: comprBlocks128->inc(); break;
        case 192: comprBlocks192->inc(); break;
        case 256: comprBlocks256->inc(); break;
        default: panic(ropCacheName, "clock", "Unsupported compressor block size."); break;
        }

        // Set new block state.
        nextWriteBlock.state = comprInfo.success ?  ROPBlockState::COMPRESSED : ROPBlockState::UNCOMPRESSED;
        nextWriteBlock.comprLevel = comprInfo.success ?  comprInfo.level : nextWriteBlock.comprLevel;

        // Set compressed block size.
        writeQueue[nextWrite].size = comprInfo.size;
        
        if (comprInfo.success)
        {
            GPU_DEBUG(
                printf("%s%02d => Address %08x Block %d compressed as COMPRESSED level %i.\n", ropCacheName, cacheID,
                    writeQueue[nextWrite].address, writeQueue[nextWrite].block, comprInfo.level);
            )

            // Compressed blocks are not masked.
            writeQueue[nextWrite].request->masked = false;
        }
        else
        {
            GPU_DEBUG(
                printf("%s%02d => Address %08x Block %d compressed as UNCOMPRESSED.\n", ropCacheName, cacheID,
                    writeQueue[nextWrite].address, writeQueue[nextWrite].block);
            )
        }

        //  Update pointer to the next output to compress.
        nextWrite = GPU_MOD(nextWrite + 1, outputRequests);

        //  Update number of write blocks to compress.
        writeOutputs--;
    }
    else
    {
        // Update statistics.
        if (writeOutputs == 0)
            writeOutQEmpty->inc();
    }

    //  Write compressed blocks to memory.
    if (compressed > 0)
    {
        //  Check if a memory transaction has been already generated.
        if (memTrans == NULL)
        {
            GPU_DEBUG(
                printf("%s%02d => Writing output request %d to memory.\n", ropCacheName, cacheID,
                    nextCompressed);
            )

            //  Send write to memory.
            memTrans = writeBlock(cycle, nextCompressed);

            //  If a transaction has been generated.
            if (memTrans == NULL)
            {
                //  Check if current block has been fully written.
                if (writeQueue[nextCompressed].written == writeQueue[nextCompressed].size)
                {
                    GPU_DEBUG(
                        printf("%s%02d => Output block %d fully written.\n",
                            ropCacheName, cacheID, nextCompressed);
                    )

                    //  Free spill cache request.
                    cache->freeRequest(writeQueue[nextCompressed].requestID, true, false);

//printf("%s%02d (%lld) => Finishing spill nextCompressed = %d isReadWaiting = %s readWaiting = %d\n", ropCacheName, cacheID, cycle,
//nextCompressed, writeQueue[nextCompressed].isReadWaiting ? "T" : "F", writeQueue[nextCompressed].readWaiting);

                    //  Signal waiting read request that the cache line has been read and written to memory (spill complete).
                    if (writeQueue[nextCompressed].isReadWaiting &&
                        (readQueue[writeQueue[nextCompressed].readWaiting].request == writeQueue[nextCompressed].request))
                    {
//if (writeQueue[nextCompressed].address == 0x001CC400)
//printf("%s%02d (%lld) => Clearing spill wait flag for read queue entry %d\n", ropCacheName, cacheID, cycle,
//writeQueue[nextCompressed].readWaiting);
                    
                        //  Clear write wait flag for the read queue.
                        readQueue[writeQueue[nextCompressed].readWaiting].spillWait = false;
                    }

                    //  Clear cache request pointer.
                    writeQueue[nextCompressed].request = NULL;

                    //  Update pointer to next compressed block to write.
                    nextCompressed = GPU_MOD(nextCompressed + 1, outputRequests);

                    //  Update number of compressed blocks.
                    compressed--;

                    //  Update free outputs counter.
                    freeWrites++;
                }
            }
            
            //  Send transaction to the memory controller.
            
        }
        else
        {
            //  Update statistics.
            memReqStall->inc();
        }
    }

    //  Write uncompressed block/line to the cache.
    if ((uncompressed > 0) && (!writingLine) && (nextWritePort < writePorts) && (writeCycles[nextWritePort] == 0))
    {
        //  Check if the read request must wait before writing the cache line.
        if (!readQueue[nextUncompressed].writeWait)
        {
            //  Try to write the cache line.
            if (cache->writeLine(readQueue[nextUncompressed].request->way,
                readQueue[nextUncompressed].request->line,
                inputBuffer[nextUncompressed]))
            {
                GPU_DEBUG(
                    printf("%s%02d => Writing input request %d to cache\n", ropCacheName, cacheID,
                        nextUncompressed);
                )

                //  Update number of uncompressed blocks.
                uncompressed--;

                //  Update number of input requests writing to a cache line.
                readsWriting++;

                //  Set cache write port cycles.
                writeCycles[nextWritePort] += (u32bit) ceil((f32bit) lineSize / (f32bit) portWidth);

                //  Set port for the line write.
                writeLinePort = nextWritePort;

                //  Update pointer to the next write port.
                nextWritePort++;

                //  Set writing line flag.
                writingLine = true;
            }
        }
        else
        {
            // Update statistics.
            warStall->inc();
        }
    }
    else
    {
        //  Update statistics.
        if (uncompressed > 0)
            writePortStall->inc();
    }

    //  Set the new transaction generated.
    nextTransaction = memTrans;

    //  Check save state mode.
    if (saveStateMode)
    {
        GPU_DEBUG(
            printf("%s%02d => Save state.\n", ropCacheName, cacheID);
        )

        //  Update save state machine.
        updateSaveState(cycle);
    }

    //  Check restore state mode.
    if (restoreStateMode)
    {
        GPU_DEBUG(
            printf("%s%02d => Restore state.\n", ropCacheName, cacheID);
        )

        //  Update restore state machine.
        updateRestoreState(cycle);
    }
    
    if (resetStateMode)
    {
        GPU_DEBUG(
            printf("%s%02d => Reset state.\n", ropCacheName, cacheID);
        )
        
        //  Update reset state machine.
        updateResetState(cycle);        
    }
}


//  Requests a compressed block/line of data.
MemoryTransaction *ROPCache::requestBlock(u64bit cycle, u32bit readReq)
{
    u32bit size;
    u32bit offset;
    MemoryTransaction *memTrans;

    //  Check if there is no write or read transaction in progress
    //  (bus busy), there are free memory tickets and the memory controller
    //  accepts read requests.
    if ((!memoryWrite) && (!memoryRead) && (freeTickets > 0)
        && ((memoryState & MS_READ_ACCEPT) != 0))
    {
        //  Calculate transaction size.
        size = GPU_MIN(MAX_TRANSACTION_SIZE,
            readQueue[readReq].size - readQueue[readReq].requested);

        //  Get parameters.
        offset = readQueue[readReq].requested;

        //  Get the next read ticket.
        u32bit nextReadTicket = ticketList.pop();
        
        //  Keep the transaction ticket associated with the request.
        memoryRequest[nextReadTicket] = readReq;

        //  Create the new memory transaction.
        memTrans = new MemoryTransaction(
            MT_READ_REQ,
            readQueue[readReq].address + offset,
            size,
            &inputBuffer[readReq][offset],
            gpuUnitID,
            cacheID,
            nextReadTicket);

        //  Check if request has an associated Dynamic Object.
        if (readQueue[readReq].request->source != NULL)
        {
            //  Copy cookies from the source object.
            memTrans->copyParentCookies(*readQueue[readReq].request->source);
            memTrans->addCookie();
        }

        GPU_DEBUG(
            printf("%s%02d (%lld) => MT_READ_REQ addr %x size %d.\n", ropCacheName, cacheID, cycle,
                readQueue[readReq].address + offset, size);
        )

        //  Update requested block bytes counter.
        readQueue[readReq].requested += size;

        //  Update number of free tickets.
        freeTickets--;
    }
    else
    {
        //  No memory transaction generated.
        memTrans = NULL;
    }

    return memTrans;
}

//  Requests a compressed block/line of data.
MemoryTransaction *ROPCache::writeBlock(u64bit cycle, u32bit writeReq)
{
    u32bit size;
    u32bit offset;
    MemoryTransaction *memTrans;

    //  Check if there is no write or read transaction in progress
    //  (bus busy), there are free memory tickets and the memory controller
    //  accepts read requests.
    if ((!memoryWrite) && (!memoryRead) && (freeTickets > 0)
        && ((memoryState & MS_WRITE_ACCEPT) != 0))
    {
        //  Calculate transaction size.
        size = GPU_MIN(MAX_TRANSACTION_SIZE,
            writeQueue[writeReq].size - writeQueue[writeReq].written);

        //  Get parameters.
        offset = writeQueue[writeReq].written;

        //  NOTE:  MASKED WRITES CAN ONLY BE USED WITH UNCOMPRESSED
        //  BLOCKS.  AS THE CURRENT COLOR CACHE DOES NOT SUPPORT
        //  COMPRESSED BLOCKS (OTHER THAN CLEAR, THAT ARE NEVER
        //  WRITTEN) THIS IS ALLOWED.


        //  Check if the cache line is a masked write.
        if (writeQueue[writeReq].request->masked)
        {
            //  Masked write.
            
            //  Check if the whole block is masked.
            bool wholeBlockMasked = true;
            
            for (u32bit dw = 0; (dw < (size >> 2)) && wholeBlockMasked; dw ++)
                wholeBlockMasked = (maskBuffer[writeReq][(offset >> 2) + dw] == 0);

//if ((writeQueue[writeReq].address + offset) == 0x00259f00)
//printf("%s0%02d (%lld) => Write Masked -> wholeBlockMasked = %s\n", ropCacheName, cacheID, cycle,
//wholeBlockMasked ? "T" : "F");                

            //  Skip the write transaction if the whole block is masked.
            if (!wholeBlockMasked)
            {
                //  Create the new memory transaction.
                memTrans = new MemoryTransaction(
                    writeQueue[writeReq].address + offset,
                    size,
                    &outputBuffer[writeReq][offset],
                    &maskBuffer[writeReq][offset >> 2],
                    gpuUnitID,
                    cacheID,
                    nextWriteTicket++);

                GPU_DEBUG(
                    printf("%s%02d (%lld) => MT_WRITE_DATA (masked) addr %x size %d.\n", ropCacheName, cacheID, cycle,
                        writeQueue[writeReq].address + offset, size);
                )
/*if ((writeQueue[writeReq].address + offset) == 0x00259f00)
{
printf("%s%02d (%lld) => MT_WRITE_DATA (masked) addr %x size %d.\n", ropCacheName, cacheID, cycle,
    writeQueue[writeReq].address + offset, size);
printf("data = ");
for(u32bit dw = 0; dw < (size >> 2); dw++)
printf("%08x ", ((u32bit *) &outputBuffer[writeReq][offset])[dw]);
printf("\n");
printf("mask = ");
for(u32bit dw = 0; dw < (size >> 2); dw++)
printf("%08x ", maskBuffer[writeReq][(offset >> 2) + dw]);
printf("\n");
}*/
            }            
            else
            {
                GPU_DEBUG(
                    printf("%s%02d (%lld) => Skipping MT_WRITE_DATA because the whole block is masked for addr %08x size %d.\n",
                        ropCacheName, cacheID, cycle, writeQueue[writeReq].address + offset, size);
                )

                //  No memory write transaction generated.
                memTrans = NULL;
            }
        }
        else
        {
            //  Unmasked write.

            //  Create the new memory transaction.
            memTrans = new MemoryTransaction(
                MT_WRITE_DATA,
                writeQueue[writeReq].address + offset,
                size,
                &outputBuffer[writeReq][offset],
                gpuUnitID,
                cacheID,
                nextWriteTicket++);

            GPU_DEBUG(
                printf("%s%02d (%lld) => MT_WRITE_DATA addr %x size %d.\n", ropCacheName, cacheID, cycle,
                   writeQueue[writeReq].address + offset, size);
            )
/*if ((writeQueue[writeReq].address + offset) == 0x00259f00)
{
printf("%s%02d (%lld) => MT_WRITE_DATA addr %x size %d.\n", ropCacheName, cacheID, cycle,
   writeQueue[writeReq].address + offset, size);
for(u32bit dw = 0; dw < (size >> 2); dw++)
printf("%08x ", ((u32bit *) &outputBuffer[writeReq][offset])[dw]);
printf("\n");
}*/
        }

        //  Check if a memory transaction transaction was generated.
        if (memTrans != NULL)
        {
            //  Check if request has an associated Dynamic Object.
            if (writeQueue[writeReq].request->source != NULL)
            {
                //  Copy cookies from the source object.
                memTrans->copyParentCookies(*writeQueue[writeReq].request->source);
                memTrans->addCookie();
            }

            //  Set write in progress flag.
            memoryWrite = true;

            //  Set memory write cycles.
            memoryCycles = memTrans->getBusCycles();

            //  Update number of free tickets.
            freeTickets--;
        }

        //  Update requested block bytes counter.
        writeQueue[writeReq].written += size;
    }
    else
    {
        //  No memory transaction generated.
        memTrans = NULL;
    }

    return memTrans;
}

//  Translates an address to the start of a block into a block number.
u32bit ROPCache::addressToBlockSU(u32bit address)
{
    //  Check if compression is enabled.
    if (compression)
    {
        u32bit block;
        u32bit blockStampUnit;

        //  Translate start block/line address to a block address inside the framebuffer.
        block = (address - ropBufferAddress) >> blockShift;

        //  Translate the block address to a block assigned to this stamp.
        blockStampUnit = (block / (stampUnitStride * numStampUnits)) * stampUnitStride + GPU_MOD(block, stampUnitStride); 

        return blockStampUnit;
    }
    else
    {
        //  When compression is disabled block state is not accessed.
        return 0;
    }
}

//  Translates a block identifier from this stamp unit into a block identifier into the framebuffer.
u32bit ROPCache::addressToBlock(u32bit address)
{
    //  Check if compression is enabled.
    if (compression)
    {
        //  Translate start block/line address to a block address inside the framebuffer.
        return (address - ropBufferAddress) >> blockShift;
    }
    else
    {
        //  When compression is disabled block state is not accessed.
        return 0;
    }
}


//  Save the cache block state memory into a file.
void ROPCache::saveBlockStateMemory()
{
    char filename[255];
    ofstream out;

    //  Create name for the snapshot file.
    sprintf(filename, "%s-%02d.snapshot", ropCacheName, cacheID);
    
    //  Open/create snapshot file for the cache block state memory.
    out.open(filename, ios::binary);
    
    //  Check if file was open/created correctly.
    if (!out.is_open())
    {
        panic(ropCacheName, "saveBlockStateMemory", "Error creating cache block state memory snapshot file.");
    }
    
    //  Dump the block state memory content into the file.
    out.write((char *) blockState, maxBlocks * sizeof(ROPBlockState));

    //  Close the file.
    out.close();
}

//  Load the cache block state memory from a file.
void ROPCache::loadBlockStateMemory()
{
    char filename[255];
    ifstream in;

    //  Create name for the snapshot file.
    sprintf(filename, "%s-%02d.snapshot", ropCacheName, cacheID);
    
    //  Open snapshot file for the cache block state memory.
    in.open(filename, ios::binary);
    
    //  Check if file was created correctly.
    if (!in.is_open())
    {
        panic(ropCacheName, "loadBlockStateMemory", "Error opening cache block state memory snapshot file.");
    }
    
    //  Load the block state memory content into the file.
    in.read((char *) blockState, maxBlocks * sizeof(ROPBlockState));

    //  Close the file.
    in.close();
}


//  Binary encode the block state data.
void ROPCache::encodeBlocks(u8bit *data, u32bit blocks)
{
    u32bit encodedBlockState[8];
    u32bit encodedBlocks;
    
    //  The block state information is encoded as two fields: 2-bits encode the state and 2-bits encode the compression level.
    //  The destination data buffer must be padded to 32-bit words.  
    
    for(u32bit i = 0; i < (blocks >> 3); i++)
    {
        //  Encode the state in the higher 2 bits and the compression level in the lower 2-bits.
        encodedBlockState[0] =  ((blockState[i * 8 + 0].state & 0x03) << 2) | (blockState[i * 8 + 0].comprLevel & 0x03);
        encodedBlockState[1] =  ((blockState[i * 8 + 1].state & 0x03) << 2) | (blockState[i * 8 + 1].comprLevel & 0x03);
        encodedBlockState[2] =  ((blockState[i * 8 + 2].state & 0x03) << 2) | (blockState[i * 8 + 2].comprLevel & 0x03);
        encodedBlockState[3] =  ((blockState[i * 8 + 3].state & 0x03) << 2) | (blockState[i * 8 + 3].comprLevel & 0x03);
        encodedBlockState[4] =  ((blockState[i * 8 + 4].state & 0x03) << 2) | (blockState[i * 8 + 4].comprLevel & 0x03);
        encodedBlockState[5] =  ((blockState[i * 8 + 5].state & 0x03) << 2) | (blockState[i * 8 + 5].comprLevel & 0x03);
        encodedBlockState[6] =  ((blockState[i * 8 + 6].state & 0x03) << 2) | (blockState[i * 8 + 6].comprLevel & 0x03);
        encodedBlockState[7] =  ((blockState[i * 8 + 7].state & 0x03) << 2) | (blockState[i * 8 + 7].comprLevel & 0x03);

        //  Pack the encoded block state data.
        encodedBlocks = (encodedBlockState[7] << 28) | (encodedBlockState[6] << 24) | (encodedBlockState[5] << 20) |
                        (encodedBlockState[4] << 16) | (encodedBlockState[3] << 12) | (encodedBlockState[2] << 8)  |
                        (encodedBlockState[1] << 4)  |  encodedBlockState[0];

        //  Store the packed block state data.
        ((u32bit *) data)[i] = encodedBlocks;
    }
    
    u32bit i;
    
    //  Fill the last word.
    for(i = 0; i < (blocks & 0x07); i++)
    {
        //  Encode the blocks for the last 32-bit word.
        encodedBlockState[i] = ((blockState[(blocks & 0xfffffff8) + i].state & 0x03) << 2) |
                                (blockState[(blocks & 0xfffffff8) + i].comprLevel & 0x03);
    }
    
    //  Check if an extra word is required.
    if ((blocks & 0x07) != 0)
    {
        //  Fill the remaining bits with zeros.
        for(; i < 8; i++)
        {
            encodedBlockState[i] = 0;
        }
        
        // Pack the last word
        encodedBlocks = (encodedBlockState[7] << 28) | (encodedBlockState[6] << 24) | (encodedBlockState[5] << 20) |
                        (encodedBlockState[4] << 16) | (encodedBlockState[3] << 12) | (encodedBlockState[2] << 8)  |
                        (encodedBlockState[1] << 4)  |  encodedBlockState[0];
                        
        //  Store the packed block state data.
        ((u32bit *) data)[(blocks >> 3) + 1] = encodedBlocks;
    }    
}


//  Binary decode and fill the block state data.
void ROPCache::decodeAndFillBlocks(u8bit *data, u32bit blocks)
{
    //printf("%s%02d => Restoring compression state blocks = %d\n", ropCacheName, cacheID, blocks);
    u32bit packedBlockData;
    for(u32bit i = 0; i < (blocks >> 3); i++)
    {
        //  Read packed block state data.
        packedBlockData = ((u32bit *) data)[i];
        
        //  Decode the block data.
        blockState[i * 8 + 0].state = decodeState((packedBlockData >> 2) & 0x03);
        blockState[i * 8 + 0].comprLevel = packedBlockData & 0x03;
        blockState[i * 8 + 1].state = decodeState((packedBlockData >> 6) & 0x03);
        blockState[i * 8 + 1].comprLevel = (packedBlockData >> 4) & 0x03;
        blockState[i * 8 + 2].state = decodeState((packedBlockData >> 10) & 0x03);
        blockState[i * 8 + 2].comprLevel = (packedBlockData >> 8) & 0x03;
        blockState[i * 8 + 3].state = decodeState((packedBlockData >> 14) & 0x03);
        blockState[i * 8 + 3].comprLevel = (packedBlockData >> 12) & 0x03;
        blockState[i * 8 + 4].state = decodeState((packedBlockData >> 18) & 0x03);
        blockState[i * 8 + 4].comprLevel = (packedBlockData >> 16) & 0x03;
        blockState[i * 8 + 5].state = decodeState((packedBlockData >> 22) & 0x03);
        blockState[i * 8 + 5].comprLevel = (packedBlockData >> 20) & 0x03;
        blockState[i * 8 + 6].state = decodeState((packedBlockData >> 26) & 0x03);
        blockState[i * 8 + 6].comprLevel = (packedBlockData >> 24) & 0x03;
        blockState[i * 8 + 7].state = decodeState((packedBlockData >> 30) & 0x03);
        blockState[i * 8 + 7].comprLevel = (packedBlockData >> 28) & 0x03;
        
/*printf("   block %06x - %06x => (%s %d) (%s %d) (%s %d) (%s %d) (%s %d) (%s %d) (%s %d) (%s %d)\n",
    i * 8, i * 8 + 7,
    (blockState[i * 8 + 0].state == ROPBlockState::CLEAR) ? "CLEAR " :
        ((blockState[i * 8 + 0].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
    blockState[i * 8 + 0].comprLevel,
    (blockState[i * 8 + 1].state == ROPBlockState::CLEAR) ? "CLEAR " :
        ((blockState[i * 8 + 1].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
    blockState[i * 8 + 1].comprLevel,
    (blockState[i * 8 + 2].state == ROPBlockState::CLEAR) ? "CLEAR " :
        ((blockState[i * 8 + 2].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
    blockState[i * 8 + 2].comprLevel,
    (blockState[i * 8 + 3].state == ROPBlockState::CLEAR) ? "CLEAR " :
        ((blockState[i * 8 + 3].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
    blockState[i * 8 + 3].comprLevel,
    (blockState[i * 8 + 4].state == ROPBlockState::CLEAR) ? "CLEAR " :
        ((blockState[i * 8 + 4].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
    blockState[i * 8 + 4].comprLevel,
    (blockState[i * 8 + 5].state == ROPBlockState::CLEAR) ? "CLEAR " :
        ((blockState[i * 8 + 5].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
    blockState[i * 8 + 5].comprLevel,
    (blockState[i * 8 + 6].state == ROPBlockState::CLEAR) ? "CLEAR " :
        ((blockState[i * 8 + 6].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
    blockState[i * 8 + 6].comprLevel,
    (blockState[i * 8 + 7].state == ROPBlockState::CLEAR) ? "CLEAR " :
        ((blockState[i * 8 + 7].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
    blockState[i * 8 + 7].comprLevel);*/
    }
    
    //  Decode an extra 32-bit word if required.
    if ((blocks & 0x07) != 0)
    {
        ROPBlockState lastBlocks[8];
        
        //  Read last 32-bit word with packed block state data.
        packedBlockData = ((u32bit *) data)[(blocks >> 3) + 1];

        //  Decode the block state data from the last 32-bit word.
        lastBlocks[0].state = decodeState(packedBlockData >> 2);
        lastBlocks[0].comprLevel = packedBlockData & 0x03;
        lastBlocks[1].state = decodeState(packedBlockData >> 6);
        lastBlocks[1].comprLevel = (packedBlockData >> 4) & 0x03;
        lastBlocks[2].state = decodeState(packedBlockData >> 10);
        lastBlocks[2].comprLevel = (packedBlockData >> 8) & 0x03;
        lastBlocks[3].state = decodeState(packedBlockData >> 14);
        lastBlocks[3].comprLevel = (packedBlockData >> 12) & 0x03;
        lastBlocks[4].state = decodeState(packedBlockData >> 18);
        lastBlocks[4].comprLevel = (packedBlockData >> 16) & 0x03;
        lastBlocks[5].state = decodeState(packedBlockData >> 22);
        lastBlocks[5].comprLevel = (packedBlockData >> 20) & 0x03;
        lastBlocks[6].state = decodeState(packedBlockData >> 26);
        lastBlocks[6].comprLevel = (packedBlockData >> 24) & 0x03;
        lastBlocks[7].state = decodeState(packedBlockData >> 30);
        lastBlocks[7].comprLevel = (packedBlockData >> 28) & 0x03;
        
/*printf("   block %06x - %06x => (%s %d) (%s %d) (%s %d) (%s %d) (%s %d) (%s %d) (%s %d) (%s %d)\n",
        (blocks & 0xfffffff8), (blocks & 0xfffffff8) + 7,
        (lastBlocks[0].state == ROPBlockState::CLEAR) ? "CLEAR " :
            ((lastBlocks[0].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
        lastBlocks[0].comprLevel,
        (lastBlocks[1].state == ROPBlockState::CLEAR) ? "CLEAR " :
            ((lastBlocks[1].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
        lastBlocks[1].comprLevel,
        (lastBlocks[2].state == ROPBlockState::CLEAR) ? "CLEAR " :
            ((lastBlocks[2].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
        lastBlocks[2].comprLevel,
        (lastBlocks[3].state == ROPBlockState::CLEAR) ? "CLEAR " :
            ((lastBlocks[3].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
        lastBlocks[3].comprLevel,
        (lastBlocks[4].state == ROPBlockState::CLEAR) ? "CLEAR " :
            ((lastBlocks[4].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
        lastBlocks[4].comprLevel,
        (lastBlocks[5].state == ROPBlockState::CLEAR) ? "CLEAR " :
            ((lastBlocks[5].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
        lastBlocks[5].comprLevel,
        (lastBlocks[6].state == ROPBlockState::CLEAR) ? "CLEAR " :
            ((lastBlocks[6].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
        lastBlocks[6].comprLevel,
        (lastBlocks[7].state == ROPBlockState::CLEAR) ? "CLEAR " :
            ((lastBlocks[7].state == ROPBlockState::COMPRESSED) ? "COMPR" : "UNCOM"),
        lastBlocks[7].comprLevel);*/

        //  Set the last blocks.
        for(u32bit i = 0; i < (blocks & 0x07); i++)
        {
            blockState[(blocks & 0xfffffff8) + i] = lastBlocks[i];
        }
    }
}

//  Decode the block state.
ROPBlockState::State ROPCache::decodeState(u32bit state)
{
    switch(state)
    {
        case ROPBlockState::CLEAR        : return ROPBlockState::CLEAR;
        case ROPBlockState::UNCOMPRESSED : return ROPBlockState::UNCOMPRESSED;
        case ROPBlockState::COMPRESSED   : return ROPBlockState::COMPRESSED;
        default:
            panic("ROPCache", "decodedState", "Undefined block state.");
            return ROPBlockState::UNCOMPRESSED;  //  To avoid compiler warning
            break;
    }
    
   
}

void ROPCache::performeReset()
{
    GPU_DEBUG(
        if (clearMode)
            printf("%s%02d => Clear (partial state reset).\n", ropCacheName, cacheID);
        else
            printf("%s%02d => Reset.\n", ropCacheName, cacheID);
    )

    //  Reset the cache.
    cache->reset();

    //  Check if it is clear mode.
    if (!clearMode)
    {
        //  Set default value of the ROP data buffer address register.
        ropBufferAddress = 0x200000;

        //  Set default value of the clear ROP value register.
        for(u32bit i = 0; i < MAX_BYTES_PER_PIXEL; i++)
            clearROPValue[i] = clearResetValue[i];
            
        //  Reset the state of the ROP data block state table.
        for(u32bit i = 0; i < maxBlocks; i++)
        {
            //  Set block as uncompressed at reset.
            blockState[i].state = ROPBlockState::UNCOMPRESSED;
        }

        //  Reset registers.
        bytesPixel = 4;
        msaaSamples = 1;  
    }
    else
    {
        //  Reset the state of the ROP data block state table.
        for(u32bit i = 0; i < maxBlocks; i++)
        {
            //  Set block as clear at clear.
            blockState[i].state = ROPBlockState::CLEAR;
        }
    }

    //  Reset free memory tickets counter.
    freeTickets = MAX_MEMORY_TICKETS;

    //  Fill the ticket list.
    ticketList.clear();
    for(u32bit i = 0; i < MAX_MEMORY_TICKETS; i++)
        ticketList.add(i);
        
    //  Reset write ticket counter.
    nextWriteTicket = 0;

    //  Reset memory bus cycles.
    memoryCycles = 0;

    //  Reset cache write port cycles.
    for(u32bit i = 0; i < writePorts; i++)
        writeCycles[i] = 0;

    //  Reset cache read port cycles.
    for(u32bit i = 0; i < readPorts; i++)
        readCycles[i] = 0;

    //  Reset compress cycles.
    compressCycles = 0;

    //  Reset uncompress cycles.
    uncompressCycles = 0;

    //  Reset compressed/uncompressed buffer counters.
    compressed = uncompressed = 0;

    //  Reset compressed/uncompressed buffer pointers.
    nextCompressed = nextUncompressed = 0;

    //  Reset input/output buffer counters.
    inputs = outputs = 0;

    //  Reset input/output buffer pointers.
    nextInput = nextOutput = 0;

    //  Reset free read/write queue entry counters.
    freeReads = inputRequests;
    freeWrites = outputRequests;

    //  Reset free queue entries pointers.
    nextFreeRead = nextFreeWrite = 0;

    //  Reset counter of read/write inputs/outputs.
    readInputs = writeOutputs = 0;

    //  Reset pointers to the read/write inputs/outputs.
    nextRead = nextWrite = 0;

    //  Reset number of input requests requested to memory.
    inputsRequested = 0;

    //  Reset the number of blocks being uncompressed.
    uncompressing = 0;

    //  Reset the number of read queue entries writing to a cache line.
    readsWriting = 0;

    //  Unset memory read and memory write flags.
    memoryRead = memoryWrite = false;

    //  Unset reading and writing line flags.
    readingLine = writingLine = false;

    //  Reset read and write line ports.
    readLinePort = readPorts;
    writeLinePort = writePorts;

    //  No cache request on process right now.
    cacheRequest = NULL;

    //  Reset block written flag.
    blockWasWritten = false;

    //  Disable flush mode.
    flushMode = false;

    //  No request to flush received.
    flushRequest = false;

    //  Disable save/restore modes and reset request flags.
    saveStateMode = false;
    resetStateMode = false;
    restoreStateMode = false;
    saveStateRequest = false;
    restoreStateRequest = false;
    resetStateMode = false;
    resetStateRequest = false;

    //  Reset save/restore mode state.
    savedBlocks = 0;
    requestedBlocks = 0;
    restoredBlocks = 0;

    //  End of the reset mode.
    resetMode = false;
}

//  Update save state machine.
void ROPCache::updateSaveState(u64bit cycle)
{
    u32bit size;
    u32bit address;
    MemoryTransaction *memTrans;

    //  Check if all the blocks have been saved to memory.
    if (savedBlocks != maxBlocks)
    {
        //  Check the memory state.  If memory is ready to receive a write request start process.
        //  Check if there is no write or read transaction in progress (bus busy), there are free memory tickets
        //  and the memory controller accepts read requests.
        if ((!memoryWrite) && (!memoryRead) && (freeTickets > 0) && ((memoryState & MS_WRITE_ACCEPT) != 0))
        {
            //  If no blocks have been requested yet encode the block state info into the internal buffer.
            if (savedBlocks == 0)
            {
                encodeBlocks(blockStateBuffer, maxBlocks);
            }

            //  Calculate transaction size.
            size = GPU_MIN(MAX_TRANSACTION_SIZE, (maxBlocks - savedBlocks) >> 1);

            //  Calculate address in memory where to store the block state info.  Pad area reserved for each ROP unit to the maximum memory
            //  transaction size to avoid problems.  Block state info encoded as 4 bits per block.
            u32bit padding = MAX_TRANSACTION_SIZE - GPU_MOD((maxBlocks >> 1), MAX_TRANSACTION_SIZE);
            padding = (padding == MAX_TRANSACTION_SIZE) ? 0 : padding;
            address = ropStateAddress + (cacheID * ((maxBlocks >> 1) + padding)) + (savedBlocks >> 1);

            //  Create the new memory transaction.
            memTrans = new MemoryTransaction(
                MT_WRITE_DATA,
                address,
                size,
                blockStateBuffer + (savedBlocks >> 1),
                gpuUnitID,
                cacheID,
                nextWriteTicket++);

            GPU_DEBUG(
                printf("%s%02d => MT_WRITE_DATA addr %x size %d.\n", ropCacheName, cacheID, address, size);
                printf(" => blockStateBuffer %x offset %d maxblocks %d\n", blockStateBuffer, (savedBlocks >> 1), (maxBlocks >> 1));
            )

            //  Update the number of saved blocks (encoded as 4 bits per block).
            savedBlocks += size << 1;

            //  Update number of free tickets.
            freeTickets--;

            //  Set write in progress flag.
            memoryWrite = true;

            //  Set memory write cycles.
            memoryCycles = memTrans->getBusCycles();

            //  Set next transaction.
            nextTransaction = memTrans;
        }
    }
    else
    {
        //  All blocks saved to the buffer in memory.  Leave save state mode.
        saveStateMode = false;

        //  Reset save block state machine.
        savedBlocks = 0;
    }
}

//  Update restore state machine.
void ROPCache::updateRestoreState(u64bit cycle)
{
    u32bit size;
    u32bit address;
    MemoryTransaction *memTrans;

    //  Check if all the blocks have been restored from memory.
    if (restoredBlocks != maxBlocks)
    {
        //  Check if all the blocks have been requested from memory.
        if (requestedBlocks != maxBlocks)
        {

//printf("%s%02d (%lld) => Restore State : requestedBlocks = %d maxBlocks %d\n",
//ropCacheName, cacheID, cycle, requestedBlocks, maxBlocks);

            //  Check the memory state.  If the memory is ready request block state info from memory.
            //  Check if there is no write or read transaction in progress (bus busy), there are free memory tickets
            //  and the memory controller accepts read requests.
            if ((!memoryWrite) && (!memoryRead) && (freeTickets > 0) && ((memoryState & MS_READ_ACCEPT) != 0))
            {
                //  Calculate transaction size.
                size = GPU_MIN(MAX_TRANSACTION_SIZE, (maxBlocks - requestedBlocks) >> 1);

                //  Calculate address in memory where the block state info is stored.  Pad area reserved for each ROP unit to the maximum memory
                //  transaction size to avoid problems.  Block state info encoded as 4 bits per block.
                u32bit padding = MAX_TRANSACTION_SIZE - GPU_MOD((maxBlocks >> 1), MAX_TRANSACTION_SIZE);
                padding = (padding == MAX_TRANSACTION_SIZE) ? 0 : padding;
                address = ropStateAddress + (cacheID * ((maxBlocks >> 1) + padding)) + (requestedBlocks >> 1);

                //  Create the new memory transaction.
                memTrans = new MemoryTransaction(
                    MT_READ_REQ,
                    address,
                    size,
                    blockStateBuffer + (requestedBlocks >> 1),
                    gpuUnitID,
                    cacheID,
                    ticketList.pop());

                GPU_DEBUG(
                    printf("%s%02d (%lld) => MT_READ_REQ addr %x size %d.\n", ropCacheName, cacheID, cycle, address, size);
                )
                
                //  Update requested blocks wounter.
                requestedBlocks += size << 1;

                //  Update number of free tickets.
                freeTickets--;
                
                //  Set next transaction.
                nextTransaction = memTrans;
            }
        }
    }
    else
    {
        //  Decode the block state info.
        decodeAndFillBlocks(blockStateBuffer, maxBlocks);

        //  All blocks restored frome the buffer in memory.  Leave restore state mode.
        restoreStateMode = false;

//printf("%s%02d (%lld) => End of restore state.\n", ropCacheName, cacheID, cycle);
        //  Reset restore state machine.
        restoredBlocks = 0;
        requestedBlocks = 0;
    }
}

void ROPCache::updateResetState(u64bit cycle)
{
    //  Check reset state block info cycles remaining.
    if (resetStateCycles > 0)
    {
        //  Update reset state block info cycles.
        resetStateCycles--;

        //  Check end of reset state block info.
        if (resetStateCycles == 0)
        {
            //  Reset the state of the ROP data block state table.
            for(u32bit i = 0; i < maxBlocks; i++)
            {            
                //  Set block as clear at reset.
                blockState[i].state = ROPBlockState::UNCOMPRESSED;
            }

            //  Unset reset state block info mode.
            resetStateMode = false;
        }
    }
}

void ROPCache::stallReport(u64bit cycle, string &stallReport)
{
    stringstream reportStream;
    
    char fullName[128];
    sprintf(fullName, "%s-%02d", ropCacheName, cacheID);
    
    reportStream << fullName << " stall report for cycle " << cycle << endl;
    reportStream << "------------------------------------------------------------" << endl;


    reportStream << " Fill pipeline : " << endl;
    reportStream << "   inputs = " << inputs << " | inputsRequested = " << inputsRequested << " | readInputs = " << readInputs;
    reportStream << " | uncompressing = " << uncompressing << " | uncompressed = " << uncompressed << " | readsWriting = " << readsWriting << endl;
    reportStream << " Spill pipeline " << endl;
    reportStream << "   outputs = " << outputs << " | writeOutputs = " << writeOutputs << " | compressed = " << compressed << endl;
    reportStream << " Cache state : " << endl;
    for(u32bit port = 0; port < readPorts; port ++)
        reportStream << "    Read port " << port << " -> readCycles = " << readCycles[port] << endl;
    for(u32bit port = 0; port < writePorts; port ++)
        reportStream << "    Write port " << port << " -> writeCycles = " << writeCycles[port] << endl;
    reportStream << "    readingLine = " << readingLine << " | writingLine = " << writingLine << endl;
    reportStream << " Memory state : " << endl;
    reportStream << "    memoryCycles = " << memoryCycles << " | memoryRead = " << memoryRead;
    reportStream << " | memoryWrite = " << memoryWrite << " | freeTickets = " << freeTickets << endl;
    reportStream << " Compressor state : " << endl;
    reportStream << "    uncompressCycles = " << uncompressCycles << " | compressCycles = " << compressCycles << endl;    
        
    stallReport.assign(reportStream.str());
}



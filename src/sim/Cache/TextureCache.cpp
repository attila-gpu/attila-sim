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
 * $RCSfile: TextureCache.cpp,v $
 * $Revision: 1.17 $
 * $Author: vmoya $
 * $Date: 2006-05-11 18:10:40 $
 *
 * Texture Cache class implementation file.
 *
 */

/**
 *
 * @file TextureCache.cpp
 *
 * Implements the Texture Cache class.  This class implements the cache used to
 * read texels from texture data.
 *
 */

#include "TextureCache.h"
#include "TextureEmulator.h"
#include "GPUMath.h"

#include <sstream>

using gpu3d::tools::Queue;

using namespace gpu3d;

/*  Calculates the tag bank for a texture cache address.  */
#define TAGBANK(address) ((address >> bankShift) & bankMask)

/*  Calculates the data bank for a texture cache address.  */
#define DATABANK(address) ((address >> bankShift) & bankMask)

/*  Calculates the line for a given address.  */
#define LINE(a) ((a) >> (lineShift))

/*  Calculates the word (32 bits) for a given address.  */
#define WORD(a) ((a) >> 2)

/*  Texture Cache class counter.  Used to create identifiers for the created Texture Caches
    that are then used to access the Memory Controller.  */
u32bit TextureCache::cacheCounter = 0;


/*  Texture cache constructor.  */
TextureCache::TextureCache(u32bit ways, u32bit lines, u32bit bytesLine,
        u32bit pWidth, u32bit reqQSize, u32bit inReqs, u32bit numBanks,
        u32bit maxAccess, u32bit bWidth, u32bit maxMiss, u32bit decompLat, char *postfix) :

    portWidth(pWidth), inputRequests(inReqs), lineSize(bytesLine), banks(numBanks),
    maxAccesses(maxAccess), bankWidth(bWidth), maxMisses(maxMiss), decomprLatency(decompLat),
    debugMode(false), TextureCacheGen()

{
    u32bit i;

    /*  Get the Texture Cache identifier.  */
    cacheID = cacheCounter;

    /*  Update the number of created Texture Caches.  */
    cacheCounter++;

    /*  Create the fetch cache object.  */
    cache = new FetchCache64(ways, lines, lineSize, reqQSize, postfix);

    /*  Create statistics.  */
    fetchBankConflicts = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("FetchBankConflicts", u32bit(0), "TextureCache", postfix);
    readBankConflicts = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadBankConflicts", u32bit(0), "TextureCache", postfix);
    memRequests = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MemoryRequests", u32bit(0), "TextureCache", postfix);
    memReqLatency = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MemoryRequestLatency", u32bit(0), "TextureCache", postfix);

    /*  Create the input buffer.  */
    inputBuffer = new u8bit*[inputRequests];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (inputBuffer == NULL)
            panic("TextureCache", "TextureCache", "Error allocating input buffers.");
    )

    /*  Create read request queue.  */
    readQueue = new TextureCacheReadRequest[inputRequests];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (readQueue == NULL)
            panic("TextureCache", "TextureCache", "Error allocating the read queue.");
    )

    /*  Allocate individual buffers.  */
    for(i = 0; i < inputRequests; i++)
    {
        /*  Allocate input buffer.  */
        inputBuffer[i] = new u8bit[lineSize];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (inputBuffer[i] == NULL)
                panic("TextureCache", "TextureCache", "Error allocating input buffer.");
        )
    }

    /*  Allocate space for the bank access counters.  */

    dataBankAccess = new u32bit[banks];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (dataBankAccess == NULL)
            panic("TextureCache", "TextureCache", "Error allocating data bank access counters.");
    )

    tagBankAccess = new u32bit[banks];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (tagBankAccess == NULL)
            panic("TextureCache", "TextureCache", "Error allocating tag bank access counters.");
    )

    /*  Allocate array storing the address fetched and read per bank and cycle.  */
    fetchAccess = new u64bit*[banks];
    readAccess = new u64bit*[banks];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (fetchAccess == NULL)
            panic("TextureCache", "TextureCache", "Error allocating fetch access address array (redundant fetches).");
        if (readAccess == NULL)
            panic("TextureCache", "TextureCache", "Error allocating read access address array (bypasses).");
    )

    /*  Allocate the per bank address arrays.  */
    for(i = 0; i < banks; i++)
    {
        /*  Allocate array storing the fetch and read access addresses per bank and cycle.  */
        fetchAccess[i] = new u64bit[maxAccesses];
        readAccess[i] = new u64bit[maxAccesses];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (fetchAccess[i] == NULL)
                panic("TextureCache", "TextureCache", "Error allocating fetch access address array (redundant fetch) for a bank.");
            if (readAccess[i] == NULL)
                panic("TextureCache", "TextureCache", "Error allocating read access address array (bypasses) for a bank.");
        )
    }

    /*  Calculate number of read ports for the Texture Cache.  */
    readPorts = banks * maxAccesses;

    /*  Allocate read port bandwidth cycle counters.  */
    readCycles = new u32bit[readPorts];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (readCycles == NULL)
            panic("TextureCache", "TextureCache", "Error allocating read port bandwidth cycle counters.");
    )


    /*  Allocate space for the decompression buffer.  */
    comprBuffer = new u8bit[lineSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (comprBuffer == NULL)
            panic("TextureCache", "TextureCache", "Error allocating buffer for texture line decompression.");
    )

    /*  Calculate line shift.  */
    lineShift = GPUMath::calculateShift(lineSize);
    bankShift = GPUMath::calculateShift(bankWidth);
    bankMask = GPUMath::buildMask(banks);

    /*  Reset the texture cache.  */
    resetMode = TRUE;

    //  Set the ticket list to the maximum number of memory tickets.
    ticketList.resize(MAX_MEMORY_TICKETS);
    
    string queueName;
    queueName = "ReadTicketQueue";
    queueName.append(postfix);
    ticketList.setName(queueName);    
}

/*  Fetches a cache line.  */
bool TextureCache::fetch(u64bit address, u32bit &way, u32bit &line, u64bit &tag, bool &ready, DynamicObject *source)
{
    bool isMiss;
    bool fetchResult;
    u32bit i;
    u64bit bank;
    u64bit lineA;

    /*  Check for black texel address (sampling outside the texture).  */
    if (address == BLACK_TEXEL_ADDRESS)
    {
        /*  Set some way and line to avoid problems when unreserving.  */
        way = 0x80000000;
        line = 0x80000000;
        tag = 0;
        ready = true;

        return TRUE;
    }

    /*  Check redundant fetches (same line requested to the same cache bank).  */
    for(i = 0, bank = TAGBANK(address), lineA = LINE(address);
        (i < tagBankAccess[bank]) && (fetchAccess[bank][i] != lineA); i++);

    /*  Check if the line was not already requested.  */
    if (i == tagBankAccess[bank])
    {
        /*  Check accesses to the tag banks this cycle.  */
        if (tagBankAccess[bank] == maxAccesses)
        {
            /*  Update statistics.  */
            fetchBankConflicts->inc();

            tag = 0;
            ready = false;

            /*  No more accesses allowed to this tag bank.  */
            return FALSE;
        }

        /*  Store the line address fetched.  */
        fetchAccess[bank][i] = lineA;

        /*  Update accesses to the tag bank.  */
        tagBankAccess[bank]++;
    }

    /*  Initialize to if maximum number of misses reached.  */
    isMiss = (cycleMisses == maxMisses);

    /*  Try to fetch the address in the Fetch Cache.  */
    fetchResult = cache->fetch(address, way, line, isMiss, ready, source);

//printf("TC => fetching %llx fetched? %s way %d line %d isMiss? %s ready? %s\n",
//    address, fetchResult?"T":"F", way, line, isMiss?"T":"F", ready?"T":"F");

    /*  Calculate line address.  */
    tag = LINE(address) << lineShift;

    /*  Check if fetch returns a miss.  */
    if (isMiss && (cycleMisses < maxMisses))
    {
        /*  Update number of misses in the current cycles.  */
        cycleMisses++;
    }

    return fetchResult;
}

/*  Reads vertex data.  */
bool TextureCache::read(u64bit address, u32bit way, u32bit line, u32bit size, u8bit *data)
{
    u32bit port;
    u32bit i;
    u64bit bank;
    u64bit word;

    /*  Check for black texel address (sampling outside the texture).  */
    if (address == BLACK_TEXEL_ADDRESS)
    {
        /*  Return 0.  */
        for(i = 0; i < size; i++)
            data[i] = 0;

        return TRUE;
    }

    /*  Check redundant reads (same 32 bit word requested to the same bank, data is bypassed).  */
    for(i = 0, bank = TAGBANK(address), word = WORD(address);
        (i < dataBankAccess[bank]) && (readAccess[bank][i] != word); i++);

    /*  Check if the address was not found.  */
    if (i == dataBankAccess[bank])
    {
        /*  Check accesses to the data banks this cycle.  */
        if (dataBankAccess[DATABANK(address)] == maxAccesses)
        {
            /*  Update statistics.  */
            readBankConflicts->inc();

            /*  No more accesses allowed to this tag bank.  */
            return FALSE;
        }

        /*  Calculate read port being used.  */
        port = static_cast<u32bit>(DATABANK(address) * maxAccesses + dataBankAccess[DATABANK(address)]);

        /*  Store the read word address for the bank.  */
        readAccess[bank][i] = word;

        /*  Update accesses to the data bank.  */
        dataBankAccess[DATABANK(address)]++;

        /*  Check if cache read port is busy.  */
        if (readCycles[port] > 0)
        {
            return FALSE;
        }
        else
        {
            /*  Try to read fetched data from the Fetch Cache.  */
            if (cache->read(address, way, line, size, data))
            {
                /*  Update read cycles counter.  */
                readCycles[port] += (u32bit) ceil((f32bit) size / (f32bit) portWidth);

                return TRUE;
            }
            else
            {
                return FALSE;
            }
        }
    }
    else
    {
        /*  The address was found, use bypass, don't count additional bandwidth usage.  */

        /*  Try to read the data.  */
        return cache->read(address, way, line, size, data);
    }
}

/*  Unreserves a cache line after use.  */
void TextureCache::unreserve(u32bit way, u32bit line)
{
    /*  Check unreserve of black texel (sampling outside the texture).  */
    if ((way != 0x80000000) || (line != 0x80000000))
    {
        /*  Unreserve fetch cache line.  */
        cache->unreserve(way, line);
    }
}

/*  Resets the Texture cache structures.  */
void TextureCache::reset()
{
    /*  Reset the texture cache.  */
    resetMode = TRUE;
}

/*  Process a memory transaction.  */
void TextureCache::processMemoryTransaction(MemoryTransaction *memTrans)
{
    /*  Process memory transaction.  */
    switch(memTrans->getCommand())
    {
        case MT_READ_DATA:

            /*  Check if memory bus is in use.  */
            GPU_ASSERT(
                if (memoryCycles > 0)
                    panic("TextureCache", "processMemoryTransaction", "Memory bus still busy.");
            )

            /*  Get ticket of the transaction.  */
            readTicket = memTrans->getID();

            /*  Keep transaction size.  */
            lastSize = memTrans->getSize();

            GPU_DEBUG(
                printf("TextureCache => MT_READ_DATA ticket %d address %x size %d\n",
                    readTicket, memTrans->getAddress(), lastSize);
            )

            /*  Set cycles the memory bus will be in use.  */
            memoryCycles = memTrans->getBusCycles();

            /*  Set memory read in progress flag.  */
            memoryRead = TRUE;

            break;

        default:

            panic("TextureCache", "processMemoryTransaction", "Unsopported transaction type.");
            break;
    }
}


/*  Update the memory request queue.  Generate new memory transactions.  */
MemoryTransaction *TextureCache::update(u64bit cycle, MemState memState, bool &filled, u64bit &tag)
{
    /*  Set memory controller state.  */
    memoryState = memState;

    /*  Simulate a input cache cycle.  */
    clock(cycle);

    /*  Get information about filled lines.  */
    filled = wasLineFilled;
    tag = notifyTag;

    /*  Check if there is a standing memory transaction .  */
    if (nextTransaction != NULL)
    {
        /*  Set the cycle at which the request was issued to memory.  */
        memRStartCycle[GPU_MOD(nextTransaction->getID(), MAX_MEMORY_TICKETS)] = cycle;
    }

    /*  Return the generated (if any) memory transaction.  */
    return nextTransaction;
}


/*  Input cache simulation rutine.  */
void TextureCache::clock(u64bit cycle)
{
    u32bit i;
    u32bit readEndReq;
    MemoryTransaction *memTrans;

    GPU_DEBUG(
        printf("TextureCache => Cycle %lld\n", cycle);
    )

    /*  Receive memory transactions and state from Memory Controller.  */

    /*  Reset line filled flag.  */
    wasLineFilled = false;

    /*  Reset resource limit counters each cycle.  */

    /*  Reset number of misses in this cycle.  */
    cycleMisses = 0;

    /*  Reset number of bank accesses this cycle.  */
    for(i = 0; i < banks; i++)
    {
        dataBankAccess[i] = 0;
        tagBankAccess[i] = 0;
    }

    /*  Check if the texture cache is in reset mode.  */
    if (resetMode)
    {
        GPU_DEBUG(
            printf("TextureCache => Reset.\n");
        )

        /*  Reset the cache.  */
        cache->reset();

        /*  Reset free memory tickets counter.  */
        freeTickets = MAX_MEMORY_TICKETS;

        //  Fill the ticket list.
        ticketList.clear();
        for(u32bit i = 0; i < MAX_MEMORY_TICKETS; i++)
            ticketList.add(i);

        /*  Reset memory bus cycles.  */
        memoryCycles = 0;

        /*  Reset cache write port cycles.  */
        writeCycles = 0;

        /*  Reset uncompress cycles.  */
        uncompressCycles = 0;

        /*  Reset compressed/uncompressed buffer counters.  */
        uncompressed = 0;

        /*  Reset compressed/uncompressed buffer pointers.  */
        nextUncompressed = 0;


        /*  Reset cache read port cycles.  */
        for(u32bit i = 0; i < readPorts; i++)
            readCycles[i] = 0;

        /*  Reset input buffer counters.  */
        inputs = 0;

        /*  Reset input buffer pointers.  */
        nextInput = 0;

        /*  Reset free read queue entry counters.  */
        freeReads = inputRequests;

        /*  Reset free queue entries pointers.  */
        nextFreeRead = 0;

        /*  Reset counter of read inputs.  */
        readInputs = 0;

        /*  Reset pointers to the read inputs.  */
        nextRead = 0;

        /*  Reset number of input requests requested to memory.  */
        inputsRequested = 0;

        /*  Reset the number of read queue entries writing to a cache line.  */
        readsWriting = 0;

        /*  Unset memory read flags.  */
        memoryRead = FALSE;

        /*  Unset writing line flags.  */
        writingLine = FALSE;

        /*  No cache request on process right now.  */
        cacheRequest = NULL;

        /*  End of the reset mode.  */
        resetMode = FALSE;
    }

    /*  NOTE:  TO BE IMPLEMENTED WHEN THE CLASS IS CONVERTED TO A
        BOX AND COMMUNICATION GOES THROUGH SIGNALS.  */

    /*  Process cache commands.  */
    /*  Process cache reads.  */
    /*  Process cache writes.  */

    /*  Check if there is line being read from the cache.  */
    for(i = 0; i < readPorts; i++)
    {
        if (readCycles[i] > 0)
        {
            GPU_DEBUG(
                printf("TextureCache => Remaining cache port %d read cycles %d.\n",
                    i, readCycles[i]);
            )

            /*  Update read cycles counter.  */
            readCycles[i]--;

            /*  Check if read has finished.  */
            if (readCycles[i] == 0)
            {
                GPU_DEBUG(
                    printf("TextureCache => Cache read end.\n");
                )
            }
        }
    }

    /*  Check if there is a line being written to the cache.  */
    if (writeCycles > 0)
    {
        GPU_DEBUG(
            printf("TextureCache => Cache write cycles %d\n",
                writeCycles);
        )

        /*  Update write port cycles.  */
        writeCycles--;

        /*  Check if write has finished.  */
        if (writeCycles == 0)
        {
            /*  Check if there was a line being written.  */
            if (writingLine)
            {
                GPU_DEBUG(
                    printf("TextureCache => End of cache write.\n");
                )

                /*  Free fill cache request.  */
                cache->freeRequest(readQueue[nextUncompressed].requestID, FALSE, TRUE);

                /*  Clear cache request pointer.  */
                readQueue[nextUncompressed].request = NULL;

                /*  Update pointer to the next uncompressed block.  */
                nextUncompressed = GPU_MOD(nextUncompressed + 1, inputRequests);

                /*  Update free inputs counter.  */
                freeReads++;

                /*  Update the number of input requests writing to a cache line.  */
                readsWriting--;

                /*  Unset writing line flag.  */
                writingLine = FALSE;

                /*  A line was filled this cycle.  */
                wasLineFilled = true;
                notifyTag = lastFilledLineTag;
            }
        }
    }

    /*  No memory transaction generated yet.  */
    memTrans = NULL;

    /*  Check if there is no cache request to process.  */
    if (cacheRequest == NULL)
    {
        /*  Receive request from the cache.  */
        cacheRequest = cache->getRequest(requestID);
    }

    /*  Check if there is a cache request being served.  */
    if ((cacheRequest != NULL) && (freeReads > 0))
    {
        /*  Add a read request to the queue if the cache request is a fill.  */
        if ((cacheRequest->fill) && (freeReads > 0))
        {
            GPU_DEBUG(
                printf("TextureCache(%p) %lld => Adding read (fill) request %d line (%d, %d) to address %llx\n",
                    this, cycle,
                    nextFreeRead, cacheRequest->way, cacheRequest->line,
                    cacheRequest->inAddress);
            )

            /*  Add to the next free read queue entry.  */
            readQueue[nextFreeRead].address = cacheRequest->inAddress;
            readQueue[nextFreeRead].size = 0;
            readQueue[nextFreeRead].requested = 0;
            readQueue[nextFreeRead].received = 0;
            readQueue[nextFreeRead].request = cacheRequest;
            readQueue[nextFreeRead].requestID = requestID;

            /*  Update inputs counter.  */
            inputs++;

            /*  Update free read queue entries counter.  */
            freeReads--;

            /*  Update pointer to the next free read queue entry.  */
            nextFreeRead = GPU_MOD(nextFreeRead + 1, inputRequests);
        }

        /*  Release the cache request.  */
        cacheRequest = NULL;
    }

    /*  Check if there is a memory read transaction being received.  */
    if (memoryCycles > 0)
    {
        GPU_DEBUG(
            printf("TextureCache => Remaining memory cycles %d\n", memoryCycles);
        )

        /*  Update memory bus busy cycles counter.  */
        memoryCycles--;

        /*  Check if transmission has finished.  */
        if (memoryCycles == 0)
        {
            /*  Search ticket in the memory request table.  */
            readEndReq = memoryRequest[MAX_MEMORY_TICKETS];

            GPU_DEBUG(
                printf("TextureCache => End of read transaction for read request queue %d\n",
                    readEndReq);
            )

            /*  Update read queue entry.  */
            readQueue[readEndReq].received += lastSize;

            /*  Check if the full block was received.  */
            if (readQueue[readEndReq].received == readQueue[readEndReq].size)
            {
                GPU_DEBUG(
                     printf("TextureCache => Input block %d fully read.\n", readEndReq);
                )

                /*  Update number of read input blocks.  */
                readInputs++;

                /*  Update the number of inputs requested to memory.  */
                inputsRequested--;

            }

            //  Add ticket to the ticket list.
            ticketList.add(readTicket);
            
            //  Update number of free tickets.
            freeTickets++;

            /*  Unset memory read on progress flag.  */
            memoryRead = FALSE;

            /*  Update statistics.  */
            memRequests->inc();
            memReqLatency->inc(cycle - memRStartCycle[readTicket]);
        }
    }

    /*  Request input blocks to memory.  */
    if (inputs > 0)
    {
        GPU_DEBUG(
            printf("TextureCache => Requesting to memory Input Request %d.\n",
                nextInput);
        )

        /*  If there is no previous request for the entry.  */
        if (readQueue[nextInput].size == 0)
        {
            /*  Determine the kind of compression used by the texture line to request.  */
            switch(readQueue[nextInput].address & TEXTURE_ADDRESS_SPACE_MASK)
            {
                case UNCOMPRESSED_TEXTURE_SPACE:

                    /*  Request an uncompressed texture line .  */

                    GPU_DEBUG(
                        printf("TextureCache => Input Request %d is for UNCOMPRESSED texture line %llx.\n",
                            nextInput, readQueue[nextInput].address);
                    )

                    /*  Set uncompressed block size.  */
                    readQueue[nextInput].size = lineSize;

                    /*  Convert texture space address to a GPU memory address.  */
                    readQueue[nextInput].memAddress = (u32bit) (readQueue[nextInput].address & 0xffffffff);
//if (cycle > 4913000)
//{
//printf("TxC %lld > Read queue %d Converting uncompressed %llx > %x\n", cycle, nextInput,
//readQueue[nextInput].address, readQueue[nextInput].memAddress);
//}

                    break;

                case COMPRESSED_TEXTURE_SPACE_DXT1_RGB:
                case COMPRESSED_TEXTURE_SPACE_DXT1_RGBA:

                    /*  Request a compressed DXT1 RGB or DXT1 RGBA texture line (ratio 1:8).  */

                    GPU_DEBUG(
                        printf("TextureCache => Input Request %d is for COMPRESSED RATIO 1:8 texture line %llx.\n",
                            nextInput, readQueue[nextInput].address);
                    )

                    /*  Set uncompressed block size.  */
                    readQueue[nextInput].size = lineSize >> DXT1_SPACE_SHIFT;

                    /*  Convert texture space address to a GPU memory address.  */
                    readQueue[nextInput].memAddress = (u32bit) ((readQueue[nextInput].address >> DXT1_SPACE_SHIFT) &
                        0xffffffff);

//if (cycle > 4913000)
//{
//printf("TxC %lld > Read queue %d Converting DXT1 %llx > %x.  Size to read %d\n", cycle, nextInput,
//readQueue[nextInput].address, readQueue[nextInput].memAddress, readQueue[nextInput].size);
//}

                    break;

                case COMPRESSED_TEXTURE_SPACE_DXT3_RGBA:
                case COMPRESSED_TEXTURE_SPACE_DXT5_RGBA:

                    /*  Request a compressed DXT3 or DXT5 RGBA texture line (ratio 1:4).  */

                    GPU_DEBUG(
                        printf("TextureCache => Input Request %d is for COMPRESSED RATIO 1:4 texture line %llx.\n",
                            nextInput, readQueue[nextInput].address);
                    )


                    /*  Set uncompressed block size.  */
                    readQueue[nextInput].size = lineSize >> DXT3_DXT5_SPACE_SHIFT;

                    /*  Convert texture space address to a GPU memory address.  */
                    readQueue[nextInput].memAddress = (u32bit) ((readQueue[nextInput].address >> DXT3_DXT5_SPACE_SHIFT) &
                         0xffffffff);

//if (cycle > 4913000)
//{
//printf("TxC %lld > Read queue %d Converting DXT3/DXT5 %llx > %x\n", cycle, nextInput,
//readQueue[nextInput].address, readQueue[nextInput].memAddress);
//}

                    break;

                case COMPRESSED_TEXTURE_SPACE_LATC1:
                case COMPRESSED_TEXTURE_SPACE_LATC2:
                case COMPRESSED_TEXTURE_SPACE_LATC1_SIGNED:
                case COMPRESSED_TEXTURE_SPACE_LATC2_SIGNED:
                
                    //  Request a compressed LATC1/LATC2 texture line (ratio 1:2).

                    GPU_DEBUG(
                        printf("TextureCache => Input Request %d is for COMPRESSED RATIO 1:2 texture line %llx.\n",
                            nextInput, readQueue[nextInput].address);
                    )


                    //  Set uncompressed block size.
                    readQueue[nextInput].size = lineSize >> LATC1_LATC2_SPACE_SHIFT;

                    //  Convert texture space address to a GPU memory address.
                    readQueue[nextInput].memAddress = (u32bit) ((readQueue[nextInput].address >> LATC1_LATC2_SPACE_SHIFT) &
                         0xffffffff);

//if (cycle > 4913000)
//{
//printf("TxC %lld > Read queue %d Converting LATC1/LATC2 %llx > %x\n", cycle, nextInput,
//readQueue[nextInput].address, readQueue[nextInput].memAddress);
//}
                    break;
                    
                default:

                    panic("TextureCache", "clock", "Unsupported texture address space.");

                    break;
            }
        }

        /*  Request data to memory.  */
        memTrans = requestBlock(nextInput);

        /*  Copy memory transaction cookies.  */

        /*  Send memory transaction to the memory controller.  */

        /*  Check if current input block has been fully requested.  */
        if (readQueue[nextInput].requested == readQueue[nextInput].size)
        {

            GPU_DEBUG(
                printf("TextureCache => Input request %d fully requested to memory.\n", nextInput);
            )

            /*  Update pointer to the next input.  */
            nextInput = GPU_MOD(nextInput + 1, inputRequests);

            /*  Update waiting inputs counter.  */
            inputs--;

            /*  Update the number of inputs requested to memory.  */
            inputsRequested++;
        }
    }

    /*  Uncompress input blocks.  */
    if (uncompressCycles > 0)
    {
        GPU_DEBUG(
            printf("ZCache => Remaining cycles to decompress block %d\n",
                uncompressCycles);
        )

        /*  Update uncompress cycles.  */
        uncompressCycles--;

        /*  Check if uncompression has finished.  */
        if (uncompressCycles == 0)
        {
            GPU_DEBUG(
                printf("ZCache => End of block decompression.\n");
            )

            /*  Update number of uncompressed inputs.  */
            uncompressed++;

            /*  Update the number of blocks being uncompressed.  */
            uncompressing--;
        }
    }

    /*  Check if there are blocks to uncompress and the uncompressor is ready.  */
    if ((readInputs > 0) && (uncompressCycles == 0))
    {
        /*  Check if the next read input has been fully received from memory.  */
        if (readQueue[nextRead].size == readQueue[nextRead].received)
        {
            /*  Set uncompression cycles.  */
            uncompressCycles = decomprLatency;

            /*  Check block state.  */
            switch(readQueue[nextRead].address & TEXTURE_ADDRESS_SPACE_MASK)
            {
                case UNCOMPRESSED_TEXTURE_SPACE:

                    /*  Nothing to do.  */

                    GPU_DEBUG(
                        printf("TextureCache => Uncompressed texture line for input request %d.\n",
                            nextRead);
                    )

                    break;

                case COMPRESSED_TEXTURE_SPACE_DXT1_RGB:

                    GPU_DEBUG(
                        printf("TextureCache => DXT1 RGB compressed texture line for input request %d.\n",
                            nextRead);
                    )

                    /*  Copy the compressed input to the compression buffer.  */
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize >> DXT1_SPACE_SHIFT);

                    if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                    {
                        printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                            cycle, nextRead, readQueue[nextRead].address, readQueue[nextRead].memAddress,
                            readQueue[nextRead].size, readQueue[nextRead].requested, readQueue[nextRead].received);

                        panic("TextureCache", "clock", "Decompresing uninitialized memory block (DXT1-RGB).");
                    }

                    /*  Uncompress the block.  */
                    TextureEmulator::decompressDXT1RGB(comprBuffer, inputBuffer[nextRead], lineSize >> DXT1_SPACE_SHIFT);

                    break;

                case COMPRESSED_TEXTURE_SPACE_DXT1_RGBA:

                    GPU_DEBUG(
                        printf("TextureCache => DXT1 RGBA compressed texture line for input request %d.\n",
                            nextRead);
                    )

                    /*  Copy the compressed input to the compression buffer.  */
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize >> DXT1_SPACE_SHIFT);

                    if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                    {
                        printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                            cycle, nextRead, readQueue[nextRead].address, readQueue[nextRead].memAddress,
                            readQueue[nextRead].size, readQueue[nextRead].requested, readQueue[nextRead].received);

                        panic("TextureCache", "clock", "Decompresing uninitialized memory block (DXT1-RGBA).");
                    }

//printf("Decompressing DXT1 RGBA block:\n");
//for(int iii = 0; iii < (lineSize >> DXT1_SPACE_SHIFT); iii++)
//printf("%02x ", comprBuffer[iii]);
//printf("\n");
                    /*  Uncompress the block.  */
                    TextureEmulator::decompressDXT1RGBA(comprBuffer, inputBuffer[nextRead], lineSize >> DXT1_SPACE_SHIFT);

                    break;

                case COMPRESSED_TEXTURE_SPACE_DXT3_RGBA:

                    GPU_DEBUG(
                        printf("TextureCache => DXT3 RGBA compressed texture line for input request %d.\n",
                            nextRead);
                    )

                    /*  Copy the compressed input to the compression buffer.  */
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize >> DXT3_DXT5_SPACE_SHIFT);

                    if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                    {
                        printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                            cycle, nextRead, readQueue[nextRead].address, readQueue[nextRead].memAddress,
                            readQueue[nextRead].size, readQueue[nextRead].requested, readQueue[nextRead].received);

                        panic("TextureCache", "clock", "Decompresing uninitialized memory block (DXT3-RGBA).");
                    }

                    /*  Uncompress the block.  */
                    TextureEmulator::decompressDXT3RGBA(comprBuffer, inputBuffer[nextRead],
                        lineSize >> DXT3_DXT5_SPACE_SHIFT);

                    break;

                case COMPRESSED_TEXTURE_SPACE_DXT5_RGBA:

                    GPU_DEBUG(
                        printf("TextureCache => DXT5 RGBA compressed texture line for input request %d.\n",
                            nextRead);
                    )

                    /*  Copy the compressed input to the compression buffer.  */
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize >> DXT3_DXT5_SPACE_SHIFT);

                    if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                    {
                        printf("TC %lld => nextRead %d address %016lx (%08x) size %d requested %d received %d\n",
                            cycle, nextRead, readQueue[nextRead].address, readQueue[nextRead].memAddress,
                            readQueue[nextRead].size, readQueue[nextRead].requested, readQueue[nextRead].received);

                        panic("TextureCache", "clock", "Decompresing uninitialized memory block (DXT5-RGBA).");
                    }

                    /*  Uncompress the block.  */
                    TextureEmulator::decompressDXT5RGBA(comprBuffer, inputBuffer[nextRead],
                        lineSize >> DXT3_DXT5_SPACE_SHIFT);

                    break;

                case COMPRESSED_TEXTURE_SPACE_LATC1:

                    GPU_DEBUG(
                        printf("TextureCache => LATC1 compressed texture line for input request %d.\n",
                            nextRead);
                    )

                    //  Copy the compressed input to the compression buffer.
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize >> LATC1_LATC2_SPACE_SHIFT);

                    //  Undefined access memory check.
                    if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                    {
                        printf("TC %lld => nextRead %d address %016lx (%08x) size %d requested %d received %d\n",
                            cycle, nextRead, readQueue[nextRead].address, readQueue[nextRead].memAddress,
                            readQueue[nextRead].size, readQueue[nextRead].requested, readQueue[nextRead].received);

                        panic("TextureCache", "clock", "Decompresing uninitialized memory block (LATC1).");
                    }

                    //  Uncompress the block.
                    TextureEmulator::decompressLATC1(comprBuffer, inputBuffer[nextRead], lineSize >> LATC1_LATC2_SPACE_SHIFT);

                    break;

                case COMPRESSED_TEXTURE_SPACE_LATC1_SIGNED:

                    GPU_DEBUG(
                        printf("TextureCache => LATC1_SIGNED compressed texture line for input request %d.\n",
                            nextRead);
                    )

                    //  Copy the compressed input to the compression buffer.
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize >> LATC1_LATC2_SPACE_SHIFT);

                    //  Undefined access memory check.
                    if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                    {
                        printf("TC %lld => nextRead %d address %016lx (%08x) size %d requested %d received %d\n",
                            cycle, nextRead, readQueue[nextRead].address, readQueue[nextRead].memAddress,
                            readQueue[nextRead].size, readQueue[nextRead].requested, readQueue[nextRead].received);

                        panic("TextureCache", "clock", "Decompresing uninitialized memory block (LATC1_SIGNED).");
                    }

                    //  Uncompress the block.
                    TextureEmulator::decompressLATC1Signed(comprBuffer, inputBuffer[nextRead], lineSize >> LATC1_LATC2_SPACE_SHIFT);

                    break;

                case COMPRESSED_TEXTURE_SPACE_LATC2:

                    GPU_DEBUG(
                        printf("TextureCache => LATC2 compressed texture line for input request %d.\n",
                            nextRead);
                    )

                    //  Copy the compressed input to the compression buffer.
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize >> LATC1_LATC2_SPACE_SHIFT);

                    //  Undefined access memory check.
                    if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                    {
                        printf("TC %lld => nextRead %d address %016lx (%08x) size %d requested %d received %d\n",
                            cycle, nextRead, readQueue[nextRead].address, readQueue[nextRead].memAddress,
                            readQueue[nextRead].size, readQueue[nextRead].requested, readQueue[nextRead].received);

                        panic("TextureCache", "clock", "Decompresing uninitialized memory block (LATC2).");
                    }

                    //  Uncompress the block.
                    TextureEmulator::decompressLATC2(comprBuffer, inputBuffer[nextRead], lineSize >> LATC1_LATC2_SPACE_SHIFT);

                    break;

                case COMPRESSED_TEXTURE_SPACE_LATC2_SIGNED:

                    GPU_DEBUG(
                        printf("TextureCache => LATC2_SIGNED compressed texture line for input request %d.\n",
                            nextRead);
                    )

                    //  Copy the compressed input to the compression buffer.
                    memcpy(comprBuffer, inputBuffer[nextRead], lineSize >> LATC1_LATC2_SPACE_SHIFT);

                    //  Undefined access memory check.
                    if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                    {
                        printf("TC %lld => nextRead %d address %016lx (%08x) size %d requested %d received %d\n",
                            cycle, nextRead, readQueue[nextRead].address, readQueue[nextRead].memAddress,
                            readQueue[nextRead].size, readQueue[nextRead].requested, readQueue[nextRead].received);

                        panic("TextureCache", "clock", "Decompresing uninitialized memory block (LATC2_SIGNED).");
                    }

                    //  Uncompress the block.
                    TextureEmulator::decompressLATC2Signed(comprBuffer, inputBuffer[nextRead], lineSize >> LATC1_LATC2_SPACE_SHIFT);

                    break;

                default:

                    panic("TextureCache", "clock", "Unsupported texture compression mode.");
                    break;
            }

            /*  Update pointer to the input to decompress.  */
            nextRead = GPU_MOD(nextRead + 1, inputRequests);

            /*  Update number of inputs to decompress.  */
            readInputs--;

            /*  Update the number of blocks being uncompressed.  */
            uncompressing++;
        }
    }

    /*  Write uncompressed block/line to the cache.  */
    if ((uncompressed > 0) && (writeCycles == 0))
    {
        /*  Try to write the cache line.  */
        if (cache->writeLine(readQueue[nextUncompressed].request->way,
            readQueue[nextUncompressed].request->line,
            inputBuffer[nextUncompressed], lastFilledLineTag))
        {
            GPU_DEBUG(
                printf("ZCache => Writing input request %d to cache\n",
                    nextUncompressed);
            )

            /*  Update number of uncompressed blocks.  */
            uncompressed--;

            /*  Update number of input requests writing to a cache line.  */
            readsWriting++;

            /*  Set cache write port cycles.  */
            writeCycles += (u32bit) ceil((f32bit) lineSize / ((f32bit) portWidth * readPorts));

            /*  Set writing line flag.  */
            writingLine = TRUE;
        }
    }

    /*  Set the new transaction generated.  */
    nextTransaction = memTrans;
}


/*  Requests a line of data.  */
MemoryTransaction *TextureCache::requestBlock(u32bit readReq)
{
    u32bit size;
    u32bit offset;
    MemoryTransaction *memTrans;

    /*  Check if there is no write or read transaction in progress
        (bus busy), there are free memory tickets and the memory controller
        accepts read requests.  */
    if ((!memoryRead) && (freeTickets > 0)
        && ((memoryState & MS_READ_ACCEPT) != 0))
    {
        /*  Calculate transaction size.  */
        size = GPU_MIN(MAX_TRANSACTION_SIZE,
            readQueue[readReq].size - readQueue[readReq].requested);

        /*  Get parameters.  */
        offset = readQueue[readReq].requested;

        //  Get the next read ticket.
        u32bit nextReadTicket = ticketList.pop();
        
        /*  Keep the transaction ticket associated with the request.  */
        memoryRequest[nextReadTicket] = readReq;

        /*  Create the new memory transaction.  */
        memTrans = new MemoryTransaction(
            MT_READ_REQ,
            readQueue[readReq].memAddress + offset,
            size,
            &inputBuffer[readReq][offset],
            TEXTUREUNIT,
            cacheID,
            nextReadTicket);

        /*  If the request has a dynamic object as a source copy the cookies.  */
        if (readQueue[readReq].request->source != NULL)
        {
            memTrans->copyParentCookies(*readQueue[readReq].request->source);
            memTrans->addCookie();
        }

        GPU_DEBUG(
            printf("TextureCache => MT_READ_REQ addr %x size %d.\n",
                readQueue[readReq].memAddress + offset, size);
        )

        /*  Update requested block bytes counter.  */
        readQueue[readReq].requested += size;

        /*  Update number of free tickets.  */
        freeTickets--;
    }
    else
    {
        /*  No memory transaction generated.  */
        memTrans = NULL;
    }

    return memTrans;
}

void TextureCache::stallReport(u64bit cycle, string &stallReport)
{
    stringstream reportStream;
    
    char fullName[128];
    sprintf(fullName, "TextureCacheL1-%02d", cacheID);
    
    reportStream << fullName << " stall report for cycle " << cycle << endl;
    reportStream << "------------------------------------------------------------" << endl;


    reportStream << " Fill pipeline : " << endl;
    reportStream << "   inputs = " << inputs << " | inputsRequested = " << inputsRequested << " | readInputs = " << readInputs;
    reportStream << " | uncompressing = " << uncompressing << " | uncompressed = " << uncompressed << " | readsWriting = " << readsWriting << endl;
    reportStream << " Cache state : " << endl;
    for(u32bit port = 0; port < readPorts; port ++)
        reportStream << "    Read port " << port << " -> readCycles = " << readCycles[port] << endl;
    for(u32bit port = 0; port < 1; port ++)
        reportStream << "    Write port " << port << " -> writeCycles = " << writeCycles << endl;
    reportStream << "    writingLine = " << writingLine << endl;
    reportStream << " Memory state : " << endl;
    reportStream << "    memoryCycles = " << memoryCycles << " | memoryRead = " << memoryRead << " | freeTickets = " << freeTickets << endl;
    reportStream << " Compressor state : " << endl;
    reportStream << "    uncompressCycles = " << uncompressCycles << endl;    
        
    stallReport.assign(reportStream.str());
}

void TextureCache::setDebug(bool enable)
{
    debugMode = enable;
    cache->setDebug(enable);
}


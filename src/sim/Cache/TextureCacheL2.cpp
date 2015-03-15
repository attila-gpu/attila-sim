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
 * $RCSfile: TextureCacheL2.cpp,v $
 * $Revision: 1.2 $
 * $Author: vmoya $
 * $Date: 2007-12-17 19:54:57 $
 *
 * Texture Cache class implementation file.
 *
 */

/**
 *
 * @file TextureCacheL2.cpp
 *
 * Implements the Texture Cache class.  This class implements the cache used to
 * read texels from texture data.
 *
 */

#include "TextureCacheL2.h"
#include "TextureEmulator.h"
#include "GPUMath.h"
#include <sstream>

using gpu3d::tools::Queue;

using namespace gpu3d;

#undef GPU_DEBUG
#define GPU_DEBUG(expr) if (debugMode) { expr }

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
u32bit TextureCacheL2::cacheCounter = 0;


/*  Texture cache constructor.  */
TextureCacheL2::TextureCacheL2(u32bit waysL0, u32bit linesL0, u32bit bytesLineL0,
        u32bit pWidth, u32bit reqQSize, u32bit inReqsL0, u32bit numBanks,
        u32bit maxAccess, u32bit bWidth, u32bit maxMiss, u32bit decompLat,
        u32bit waysL1, u32bit linesL1, u32bit bytesLineL1, u32bit inReqsL1,
        char *postfix) :

    portWidth(pWidth), inputRequestsL0(inReqsL0), lineSizeL0(bytesLineL0), banks(numBanks),
    maxAccesses(maxAccess), bankWidth(bWidth), maxMisses(maxMiss), decomprLatency(decompLat),
    lineSizeL1(bytesLineL1), inputRequestsL1(inReqsL1), debugMode(false),
    TextureCacheGen()

{
    u32bit i;

    /*  Get the Texture Cache identifier.  */
    cacheID = cacheCounter;

    /*  Update the number of created Texture Caches.  */
    cacheCounter++;

    /*  Create the L0 fetch cache object.  */
    cacheL0 = new FetchCache64(waysL0, linesL0, lineSizeL0, reqQSize, postfix);

    /*  Create the L1 fetch cache object.  */
    cacheL1 = new FetchCache(waysL1, linesL1, lineSizeL1, reqQSize, postfix);

    /*  Create statistics.  */
    fetchBankConflicts = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("FetchBankConflicts", u32bit(0), "TextureCache", postfix);
    readBankConflicts = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadBankConflicts", u32bit(0), "TextureCache", postfix);
    memRequests = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MemoryRequests", u32bit(0), "TextureCache", postfix);
    memReqLatency = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MemoryRequestLatency", u32bit(0), "TextureCache", postfix);
    pendingRequests = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("PendingMemoryRequests", u32bit(0), "TextureCache", postfix);
    pendingRequestsAvg = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("PendingMemoryRequestsAvg", u32bit(0), "TextureCache", postfix);

    /*  Create the L0 input buffer.  */
    inputBufferL0 = new u8bit*[inputRequestsL0];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (inputBufferL0 == NULL)
            panic("TextureCache", "TextureCache", "Error allocating L0 input buffers.");
    )

    /*  Create the L1 input buffer.  */
    inputBufferL1 = new u8bit*[inputRequestsL1];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (inputBufferL1 == NULL)
            panic("TextureCache", "TextureCache", "Error allocating L1 input buffers.");
    )

    /*  Create L0 read request queue.  */
    readQueueL0 = new TextureCacheL2ReadRequest[inputRequestsL0];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (readQueueL0 == NULL)
            panic("TextureCache", "TextureCache", "Error allocating the L0 read queue.");
    )

    /*  Create L1 read request queue.  */
    readQueueL1 = new TextureCacheL2ReadRequest[inputRequestsL1];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (readQueueL1 == NULL)
            panic("TextureCache", "TextureCache", "Error allocating the L1 read queue.");
    )

    /*  Allocate individual buffers.  */
    for(i = 0; i < inputRequestsL0; i++)
    {
        /*  Allocate L0 input buffer.  */
        inputBufferL0[i] = new u8bit[lineSizeL0];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (inputBufferL0[i] == NULL)
                panic("TextureCache", "TextureCache", "Error allocating L0 input buffer.");
        )
    }

    /*  Allocate individual buffers.  */
    for(i = 0; i < inputRequestsL1; i++)
    {
        /*  Allocate L1 input buffer.  */
        inputBufferL1[i] = new u8bit[lineSizeL1];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (inputBufferL1[i] == NULL)
                panic("TextureCache", "TextureCache", "Error allocating L0 input buffer.");
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
    comprBuffer = new u8bit[lineSizeL0];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (comprBuffer == NULL)
            panic("TextureCache", "TextureCache", "Error allocating buffer for texture line decompression.");
    )

    /*  Calculate line shift.  */
    lineShift = GPUMath::calculateShift(lineSizeL0);
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
bool TextureCacheL2::fetch(u64bit address, u32bit &way, u32bit &line, u64bit &tag, bool &ready, DynamicObject *source)
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
    fetchResult = cacheL0->fetch(address, way, line, isMiss, ready, source);

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
bool TextureCacheL2::read(u64bit address, u32bit way, u32bit line, u32bit size, u8bit *data)
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
            if (cacheL0->read(address, way, line, size, data))
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
        return cacheL0->read(address, way, line, size, data);
    }
}

/*  Unreserves a cache line after use.  */
void TextureCacheL2::unreserve(u32bit way, u32bit line)
{
    /*  Check unreserve of black texel (sampling outside the texture).  */
    if ((way != 0x80000000) || (line != 0x80000000))
    {
        /*  Unreserve fetch cache line.  */
        cacheL0->unreserve(way, line);
    }
}

/*  Resets the Texture cache structures.  */
void TextureCacheL2::reset()
{
    /*  Reset the texture cache.  */
    resetMode = TRUE;
}

/*  Process a memory transaction.  */
void TextureCacheL2::processMemoryTransaction(MemoryTransaction *memTrans)
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
                printf("TextureCacheL2-%02d => MT_READ_DATA ticket %d address %x size %d\n",
                    cacheID, readTicket, memTrans->getAddress(), lastSize);
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
MemoryTransaction *TextureCacheL2::update(u64bit cycle, MemState memState, bool &filled, u64bit &tag)
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
void TextureCacheL2::clock(u64bit cycle)
{
    u32bit i;
    u32bit readEndReq;
    MemoryTransaction *memTrans;
    bool fakeMiss;
    bool fakeReady;

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
        cacheL0->reset();
        cacheL1->reset();

        /*  Reset free memory tickets counter.  */
        freeTickets = MAX_MEMORY_TICKETS;

        //  Fill the ticket list.
        ticketList.clear();
        for(u32bit i = 0; i < MAX_MEMORY_TICKETS; i++)
            ticketList.add(i);

        /*  Reset cycles counters.  */
        memoryCycles = writeCycles =  uncompressCycles = 0;

        /*  Reset cache read port cycles.  */
        for(u32bit i = 0; i < readPorts; i++)
            readCycles[i] = 0;


        /*  L0 requests pointers and counters.  */
        uncompressedL0 = nextUncompressedL0 = uncompressingL0 =
        fetchInputsL0 = nextFetchInputL0 = waitReadInputsL0 = nextReadInputL0 =
        nextFreeReadL0 = readInputsL0 = nextReadL0 = readsWritingL0 = 0;
        freeReadsL0 = inputRequestsL0;

        /*  L1 requests pointers and counters.  */
        inputsL1 = nextInputL1 = readInputsL1 = nextReadInputL1 = nextFreeReadL1 =
        inputsRequestedL1 = 0;
        freeReadsL1 = inputRequestsL1;

        /*  Unset memory read flags.  */
        memoryRead = FALSE;

        /*  Unset writing line flags.  */
        writingLine = FALSE;

        /*  No cache request on process right now.  */
        cacheRequestL0 = NULL;
        cacheRequestL1 = NULL;

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
                    printf("TextureCache => End of L0 cache write.\n");
                )

                /*  Free fill cache request.  */
                cacheL0->freeRequest(readQueueL0[nextUncompressedL0].requestID, FALSE, TRUE);

                /*  Clear cache request pointer.  */
                readQueueL0[nextUncompressedL0].request = NULL;

                /*  Update pointer to the next uncompressed block.  */
                nextUncompressedL0 = GPU_MOD(nextUncompressedL0 + 1, inputRequestsL0);

                /*  Update free inputs counter.  */
                freeReadsL0++;

                /*  Update the number of input requests writing to a cache line.  */
                readsWritingL0--;

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

    /*****************************************************************/
    /*****************************************************************/
    /*****************************************************************/

    /*  Check if there is no L0 cache request to process.  */
    if (cacheRequestL0 == NULL)
    {
        /*  Receive request from the cache.  */
        cacheRequestL0 = cacheL0->getRequest(requestIDL0);
    }

    /*  Check if there is a L0 cache request being served.  */
    if ((cacheRequestL0 != NULL) && (freeReadsL0 > 0))
    {
        /*  Add a read request to the queue if the cache request is a fill.  */
        if ((cacheRequestL0->fill) && (freeReadsL0 > 0))
        {
            GPU_DEBUG(
                printf("TextureCacheL2-%02d %lld => Adding read (fill) request %d line (%d, %d) to address %llx for L0 cache\n",
                    cacheID, cycle,
                    nextFreeReadL0, cacheRequestL0->way, cacheRequestL0->line,
                    cacheRequestL0->inAddress);
            )

            /*  Add to the next free read queue entry.  */
            readQueueL0[nextFreeReadL0].address = cacheRequestL0->inAddress;
            readQueueL0[nextFreeReadL0].size = 0;
            readQueueL0[nextFreeReadL0].requested = 0;
            readQueueL0[nextFreeReadL0].received = 0;
            readQueueL0[nextFreeReadL0].request = cacheRequestL0;
            readQueueL0[nextFreeReadL0].requestID = requestIDL0;

            /*  Update fetch inputs counter.  */
            fetchInputsL0++;

            /*  Update free read queue entries counter.  */
            freeReadsL0--;

            /*  Update pointer to the next free read queue entry.  */
            nextFreeReadL0 = GPU_MOD(nextFreeReadL0 + 1, inputRequestsL0);
        }

        /*  Release the cache request.  */
        cacheRequestL0 = NULL;
    }

    /*****************************************************************/

    /*  Check if there is no L1 cache request to process.  */
    if (cacheRequestL1 == NULL)
    {
        /*  Receive request from the cache.  */
        cacheRequestL1 = cacheL1->getRequest(requestIDL1);
    }

    /*  Check if there is a L1 cache request being served.  */
    if ((cacheRequestL1 != NULL) && (freeReadsL1 > 0))
    {
        /*  Add a read request to the queue if the cache request is a fill.  */
        if ((cacheRequestL1->fill) && (freeReadsL1 > 0))
        {
            GPU_DEBUG(
                printf("TextureCacheL2-%02d => At cycle %lld.  Adding read (fill) L1 cache request with ID %d to input request entry %d for line (%d, %d) address %lx.\n",
                    cacheID, cycle, requestIDL1, nextFreeReadL1, cacheRequestL1->way, cacheRequestL1->line, cacheRequestL1->inAddress);
            )

            /*  Add to the next free read queue entry.  */
            readQueueL1[nextFreeReadL1].memAddress = cacheRequestL1->inAddress;
            readQueueL1[nextFreeReadL1].size = lineSizeL1;
            readQueueL1[nextFreeReadL1].requested = 0;
            readQueueL1[nextFreeReadL1].received = 0;
            readQueueL1[nextFreeReadL1].requestL1 = cacheRequestL1;
            readQueueL1[nextFreeReadL1].requestID = requestIDL1;

            /*  Update inputs counter.  */
            inputsL1++;

            /*  Update free read queue entries counter.  */
            freeReadsL1--;

            /*  Update pointer to the next free read queue entry.  */
            nextFreeReadL1 = GPU_MOD(nextFreeReadL1 + 1, inputRequestsL1);
        }

        /*  Release the cache request.  */
        cacheRequestL1 = NULL;
    }

    /*****************************************************************/
    /*****************************************************************/

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
            readEndReq = memoryRequest[readTicket];

            GPU_DEBUG(
                printf("TextureCacheL2-%02d => End of read transaction with ticket %d for read request queue %d pending to receive %d\n",
                    cacheID, readTicket, readEndReq, readQueueL1[readEndReq].received);
            )

            /*  Update read queue entry.  */
            readQueueL1[readEndReq].received += lastSize;

            /*  Check if the full block was received.  */
            if (readQueueL1[readEndReq].received == readQueueL1[readEndReq].size)
            {
                GPU_DEBUG(
                     printf("TextureCache-%02d => Input block %d fully read.\n", cacheID, readEndReq);
                )

                /*  Update number of read input blocks.  */
                readInputsL1++;

                /*  Update the number of inputs requested to memory.  */
                inputsRequestedL1--;
            }

            //  Add ticket to the ticket list.
            ticketList.add(readTicket);
            
            /*  Update number of free tickets.  */
            freeTickets++;

            /*  Unset memory read on progress flag.  */
            memoryRead = FALSE;

            /*  Update statistics.  */
            memRequests->inc();
            memReqLatency->inc(cycle - memRStartCycle[readTicket]);
        }
    }

    /*****************************************************************/
    /*****************************************************************/

    /*  Fetch input blocks from L1 cache.  */
    if (fetchInputsL0 > 0)
    {
        /*  If there is no previous request for the entry.  */
        if (readQueueL0[nextFetchInputL0].size == 0)
        {
            /*  Determine the kind of compression used by the texture line to request.  */
            switch(readQueueL0[nextFetchInputL0].address & TEXTURE_ADDRESS_SPACE_MASK)
            {
                case UNCOMPRESSED_TEXTURE_SPACE:

                    /*  Fetch an uncompressed texture line .  */

                    GPU_DEBUG(
                        printf("TextureCache => Input Request %d is for UNCOMPRESSED texture line %llx.\n",
                            nextFetchInputL0, readQueueL0[nextFetchInputL0].address);
                    )

                    /*  Set uncompressed block size.  */
                    readQueueL0[nextFetchInputL0].size = lineSizeL0;

                    /*  Convert texture space address to a GPU memory address.  */
                    readQueueL0[nextFetchInputL0].memAddress = (u32bit) (readQueueL0[nextFetchInputL0].address & 0xffffffff);
//if (cycle > 4913000)
//{
//printf("TxC %lld > Read queue %d Converting uncompressed %llx > %x\n", cycle, nextFetchInputL0,
//readQueueL0[nextFetchInputL0].address, readQueueL0[nextFetchInputL0].memAddress);
//}

                    break;

                case COMPRESSED_TEXTURE_SPACE_DXT1_RGB:
                case COMPRESSED_TEXTURE_SPACE_DXT1_RGBA:

                    /*  Request a compressed DXT1 RGB or DXT1 RGBA texture line (ratio 1:8).  */

                    GPU_DEBUG(
                        printf("TextureCache => Input Request %d is for COMPRESSED RATIO 1:8 texture line %llx.\n",
                            nextFetchInputL0, readQueueL0[nextFetchInputL0].address);
                    )

                    /*  Set uncompressed block size.  */
                    readQueueL0[nextFetchInputL0].size = lineSizeL0 >> DXT1_SPACE_SHIFT;

                    /*  Convert texture space address to a GPU memory address.  */
                    readQueueL0[nextFetchInputL0].memAddress = (u32bit) ((readQueueL0[nextFetchInputL0].address >> DXT1_SPACE_SHIFT) &
                        0xffffffff);

//if (cycle > 4913000)
//{
//printf("TxC %lld > Read queue %d Converting DXT1 %llx > %x.  Size to read %d\n", cycle, nextFetchInputL0,
//readQueueL0[nextFetchInputL0].address, readQueueL0[nextFetchInputL0].memAddress, readQueueL0[nextFetchInputL0].size);
//}

                    break;

                case COMPRESSED_TEXTURE_SPACE_DXT3_RGBA:
                case COMPRESSED_TEXTURE_SPACE_DXT5_RGBA:

                    /*  Request a compressed DXT3 or DXT5 RGBA texture line (ratio 1:4).  */

                    GPU_DEBUG(
                        printf("TextureCache => Input Request %d is for COMPRESSED RATIO 1:4 texture line %llx.\n",
                            nextFetchInputL0, readQueueL0[nextFetchInputL0].address);
                    )


                    /*  Set uncompressed block size.  */
                    readQueueL0[nextFetchInputL0].size = lineSizeL0 >> DXT3_DXT5_SPACE_SHIFT;

                    /*  Convert texture space address to a GPU memory address.  */
                    readQueueL0[nextFetchInputL0].memAddress = (u32bit) ((readQueueL0[nextFetchInputL0].address >> DXT3_DXT5_SPACE_SHIFT) &
                         0xffffffff);

//if (cycle > 4913000)
//{
//printf("TxC %lld > Read queue %d Converting DXT3/DXT5 %llx > %x\n", cycle, nextFetchInputL0,
//readQueueL0[nextFetchInputL0].address, readQueueL0[nextFetchInputL0].memAddress);
//}

                    break;

                case COMPRESSED_TEXTURE_SPACE_LATC1:
                case COMPRESSED_TEXTURE_SPACE_LATC2:
                case COMPRESSED_TEXTURE_SPACE_LATC1_SIGNED:
                case COMPRESSED_TEXTURE_SPACE_LATC2_SIGNED:

                    //  Request a compressed LATC1/LATC2 texture line (ratio 1:2).

                    GPU_DEBUG(
                        printf("TextureCache => Input Request %d is for LATC COMPRESSED RATIO 1:2 texture line %llx.\n",
                            nextFetchInputL0, readQueueL0[nextFetchInputL0].address);
                    )


                    //  Set uncompressed block size.
                    readQueueL0[nextFetchInputL0].size = lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT;

                    //  Convert texture space address to a GPU memory address.
                    readQueueL0[nextFetchInputL0].memAddress = (u32bit) ((readQueueL0[nextFetchInputL0].address >> LATC1_LATC2_SPACE_SHIFT) &
                         0xffffffff);

//if (cycle > 4913000)
//{
//printf("TxC %lld > Read queue %d Converting LATC1/LATC2 %llx > %x\n", cycle, nextFetchInputL0,
//readQueueL0[nextFetchInputL0].address, readQueueL0[nextFetchInputL0].memAddress);
//}

                    break;

                default:
                    {
                        char errorMsg[256];
                        sprintf(errorMsg, "TextureCacheL2-%02d => Error in request to address %016llx : Unsupported texture address space\n",
                            cacheID, readQueueL0[nextFetchInputL0].address);
                        panic("TextureCache", "clock", errorMsg);
                    }
                    
                    break;
            }
        }

        GPU_DEBUG(
            printf("TextureCache => Fetching L1 cache for address %x for L0 input request %d.\n",
                readQueueL0[nextFetchInputL0].memAddress, nextFetchInputL0);
        )


        /*  Fetch line from L1 cache.  */
        if (cacheL1->fetch(readQueueL0[nextFetchInputL0].memAddress,
            readQueueL0[nextFetchInputL0].way, readQueueL0[nextFetchInputL0].line,
            readQueueL0[nextFetchInputL0].request->source))
        {
            GPU_DEBUG(
                printf("TextureCacheL2-%02d => Input request %d fetched from L1 cache line (%d, %d).\n",
                    cacheID, nextFetchInputL0, readQueueL0[nextFetchInputL0].way, readQueueL0[nextFetchInputL0].line);
            )

            /*  Update pointer to the next fetch input for L0 cache.  */
            nextFetchInputL0 = GPU_MOD(nextFetchInputL0 + 1, inputRequestsL0);

            /*  Update waiting fetch inputs counter for L0 cache.  */
            fetchInputsL0--;

            /*  Update the number of inputs fetched from L1 cache.  */
            waitReadInputsL0++;
        }
    }

    /*****************************************************************/

    /*  Request data from memory.  */
    if (inputsL1 > 0)
    {
        GPU_DEBUG(
            printf("TextureCacheL2-%02d => Requesting data from memory for L1 input request entry %d.\n",
                cacheID, nextInputL1);
        )

        /*  Request data to memory.  */
        memTrans = requestBlock(nextInputL1);

        /*  Copy memory transaction cookies.  */

        /*  Send memory transaction to the memory controller.  */

        /*  Check if current input block has been fully requested.  */
        if (readQueueL1[nextInputL1].requested == readQueueL1[nextInputL1].size)
        {

            GPU_DEBUG(
                printf("TextureCache-%02d => L1 cache input request %d fully requested to memory.\n", cacheID, nextInputL1);
            )

            /*  Update pointer to the next input for L1 cache.  */
            nextInputL1 = GPU_MOD(nextInputL1 + 1, inputRequestsL1);

            /*  Update waiting inputs counter for L1 cache.  */
            inputsL1--;

            /*  Update the number of inputs requested to memory for L1 cache.  */
            inputsRequestedL1++;
        }
    }

    /*****************************************************************/
    /*****************************************************************/

    /*  Read input blocks from L1 cache.  */
    if (waitReadInputsL0 > 0)
    {
        GPU_DEBUG(
            printf("TextureCache => Reading from L1 cache line (%d, %d) for L0 input request entry %d.\n",
                readQueueL0[nextReadInputL0].way, readQueueL0[nextReadInputL0].line, nextReadInputL0);
        )

        /*  Fetch line from L1 cache.  */
        if (cacheL1->read(readQueueL0[nextReadInputL0].memAddress,
            readQueueL0[nextReadInputL0].way, readQueueL0[nextReadInputL0].line,
            readQueueL0[nextReadInputL0].size, inputBufferL0[nextReadInputL0]))
        {
            GPU_DEBUG(
                printf("TextureCacheL2-%02d => Input request %d read from L1 cache line (%d, %d) .\n", cacheID, nextReadInputL0,
                    readQueueL0[nextReadInputL0].way, readQueueL0[nextReadInputL0].line);
            )

            /*  Set the line as fully received (no atomic fill not supported).  */
            readQueueL0[nextReadInputL0].received = readQueueL0[nextReadInputL0].size;

            /*  Unreserve fetch cache line.  */
            cacheL1->unreserve(readQueueL0[nextReadInputL0].way, readQueueL0[nextReadInputL0].line);

            /*  Update pointer to the next to read from L1 input for L0 cache.  */
            nextReadInputL0 = GPU_MOD(nextReadInputL0 + 1, inputRequestsL0);

            /*  Update waiting read inputs counter for L0 cache.  */
            waitReadInputsL0--;

            /*  Update the number of inputs read from L1 cache.  */
            readInputsL0++;
        }
    }

    /*****************************************************************/
    /*****************************************************************/

    /*  Uncompress input blocks.  */
    if (uncompressCycles > 0)
    {
        GPU_DEBUG(
            printf("TextureCache => Remaining cycles to decompress block %d\n",
                uncompressCycles);
        )

        /*  Update uncompress cycles.  */
        uncompressCycles--;

        /*  Check if uncompression has finished.  */
        if (uncompressCycles == 0)
        {
            GPU_DEBUG(
                printf("TextureCache => End of block decompression.\n");
            )

            /*  Update number of uncompressed inputs.  */
            uncompressedL0++;

            /*  Update the number of blocks being uncompressed.  */
            uncompressingL0--;
        }
    }

    /*  Check if there are blocks to uncompress and the uncompressor is ready.  */
    if ((readInputsL0 > 0) && (uncompressCycles == 0))
    {
        /*  Set uncompression cycles.  */
        uncompressCycles = decomprLatency;

        /*  Check block state.  */
        switch(readQueueL0[nextReadL0].address & TEXTURE_ADDRESS_SPACE_MASK)
        {
            case UNCOMPRESSED_TEXTURE_SPACE:

                /*  Nothing to do.  */

                GPU_DEBUG(
                    printf("TextureCache => Uncompressed texture line for input request %d.\n",
                        nextReadL0);
                )

//printf("TextureCache => Read uncompressed texture line at %016llx\n", readQueueL0[nextReadL0].address);
//for(u32bit w = 0; w < (lineSizeL0 >> 2); w++)
//printf("%08x ", ((u32bit*) inputBufferL0[nextReadL0])[w]);
//printf("\n");


                break;

            case COMPRESSED_TEXTURE_SPACE_DXT1_RGB:

                GPU_DEBUG(
                    printf("TextureCache => DXT1 RGB compressed texture line for input request %d.\n",
                        nextReadL0);
                )

                /*  Copy the compressed input to the compression buffer.  */
                memcpy(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> DXT1_SPACE_SHIFT);

                if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                {
                    printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                        cycle, nextReadL0, readQueueL0[nextReadL0].address, readQueueL0[nextReadL0].memAddress,
                        readQueueL0[nextReadL0].size, readQueueL0[nextReadL0].requested, readQueueL0[nextReadL0].received);

                    panic("TextureCache", "clock", "Decompresing uninitialized memory block (DXT1-RGB).");
                }

                /*  Uncompress the block.  */
                TextureEmulator::decompressDXT1RGB(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> DXT1_SPACE_SHIFT);

                break;

            case COMPRESSED_TEXTURE_SPACE_DXT1_RGBA:

                GPU_DEBUG(
                    printf("TextureCache => DXT1 RGBA compressed texture line for input request %d.\n",
                        nextReadL0);
                )

                /*  Copy the compressed input to the compression buffer.  */
                memcpy(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> DXT1_SPACE_SHIFT);

                if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                {
                    printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                        cycle, nextReadL0, readQueueL0[nextReadL0].address, readQueueL0[nextReadL0].memAddress,
                        readQueueL0[nextReadL0].size, readQueueL0[nextReadL0].requested, readQueueL0[nextReadL0].received);

                    panic("TextureCache", "clock", "Decompresing uninitialized memory block (DXT1-RGBA).");
                }

//printf("Decompressing DXT1 RGBA block:\n");
//for(int iii = 0; iii < (lineSizeL0 >> DXT1_SPACE_SHIFT); iii++)
//printf("%02x ", comprBuffer[iii]);
//printf("\n");
                /*  Uncompress the block.  */
                TextureEmulator::decompressDXT1RGBA(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> DXT1_SPACE_SHIFT);

                break;

            case COMPRESSED_TEXTURE_SPACE_DXT3_RGBA:

                GPU_DEBUG(
                    printf("TextureCache => DXT3 RGBA compressed texture line for input request %d.\n",
                        nextReadL0);
                )

                /*  Copy the compressed input to the compression buffer.  */
                memcpy(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> DXT3_DXT5_SPACE_SHIFT);

                if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                {
                    printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                        cycle, nextReadL0, readQueueL0[nextReadL0].address, readQueueL0[nextReadL0].memAddress,
                        readQueueL0[nextReadL0].size, readQueueL0[nextReadL0].requested, readQueueL0[nextReadL0].received);

                    panic("TextureCache", "clock", "Decompresing uninitialized memory block (DXT3-RGBA).");
                }

                /*  Uncompress the block.  */
                TextureEmulator::decompressDXT3RGBA(comprBuffer, inputBufferL0[nextReadL0],
                    lineSizeL0 >> DXT3_DXT5_SPACE_SHIFT);

                break;

            case COMPRESSED_TEXTURE_SPACE_DXT5_RGBA:

                GPU_DEBUG(
                    printf("TextureCache => DXT5 RGBA compressed texture line for input request %d.\n",
                        nextReadL0);
                )

                /*  Copy the compressed input to the compression buffer.  */
                memcpy(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> DXT3_DXT5_SPACE_SHIFT);

                if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                {
                    printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                        cycle, nextReadL0, readQueueL0[nextReadL0].address, readQueueL0[nextReadL0].memAddress, 
                        readQueueL0[nextReadL0].size, readQueueL0[nextReadL0].requested, readQueueL0[nextReadL0].received);

                    panic("TextureCache", "clock", "Decompresing uninitialized memory block (DXT5-RGBA).");
                }

                /*  Uncompress the block.  */
                TextureEmulator::decompressDXT5RGBA(comprBuffer, inputBufferL0[nextReadL0],
                    lineSizeL0 >> DXT3_DXT5_SPACE_SHIFT);

                break;

            case COMPRESSED_TEXTURE_SPACE_LATC1:

                GPU_DEBUG(
                    printf("TextureCache => LATC1 compressed texture line for input request %d.\n",
                        nextReadL0);
                )

                //  Copy the compressed input to the compression buffer.
                memcpy(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT);

                //  Check uninitialized memory access.
                if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                {
                    printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                        cycle, nextReadL0, readQueueL0[nextReadL0].address, readQueueL0[nextReadL0].memAddress, 
                        readQueueL0[nextReadL0].size, readQueueL0[nextReadL0].requested, readQueueL0[nextReadL0].received);

                    panic("TextureCache", "clock", "Decompresing uninitialized memory block (LATC1).");
                }

                //  Uncompress the block.
                TextureEmulator::decompressLATC1(comprBuffer, inputBufferL0[nextReadL0],
                    lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT);

                break;
                
            case COMPRESSED_TEXTURE_SPACE_LATC1_SIGNED:

                GPU_DEBUG(
                    printf("TextureCache => LATC1_SIGNED compressed texture line for input request %d.\n",
                        nextReadL0);
                )

                //  Copy the compressed input to the compression buffer.
                memcpy(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT);

                //  Check uninitialized memory access.
                if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                {
                    printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                        cycle, nextReadL0, readQueueL0[nextReadL0].address, readQueueL0[nextReadL0].memAddress, 
                        readQueueL0[nextReadL0].size, readQueueL0[nextReadL0].requested, readQueueL0[nextReadL0].received);

                    panic("TextureCache", "clock", "Decompresing uninitialized memory block (LATC1_SIGNED).");
                }

                //  Uncompress the block.
                TextureEmulator::decompressLATC1Signed(comprBuffer, inputBufferL0[nextReadL0],
                    lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT);

                break;

            case COMPRESSED_TEXTURE_SPACE_LATC2:

                GPU_DEBUG(
                    printf("TextureCache => LATC2 compressed texture line for input request %d.\n",
                        nextReadL0);
                )

                //  Copy the compressed input to the compression buffer.
                memcpy(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT);

                //  Check uninitialized memory access.
                if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                {
                    printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                        cycle, nextReadL0, readQueueL0[nextReadL0].address, readQueueL0[nextReadL0].memAddress, 
                        readQueueL0[nextReadL0].size, readQueueL0[nextReadL0].requested, readQueueL0[nextReadL0].received);

                    panic("TextureCache", "clock", "Decompresing uninitialized memory block (LATC2).");
                }

                //  Uncompress the block.
                TextureEmulator::decompressLATC2(comprBuffer, inputBufferL0[nextReadL0],
                    lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT);

                break;
                
            case COMPRESSED_TEXTURE_SPACE_LATC2_SIGNED:

                GPU_DEBUG(
                    printf("TextureCache => LATC2_SIGNED compressed texture line for input request %d.\n",
                        nextReadL0);
                )

                //  Copy the compressed input to the compression buffer.
                memcpy(comprBuffer, inputBufferL0[nextReadL0], lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT);

                //  Check uninitialized memory access.
                if (*((u32bit *) comprBuffer) == 0xDEADCAFE)
                {
                    printf("TC %lld => nextRead %d address %016llx (%08x) size %d requested %d received %d\n",
                        cycle, nextReadL0, readQueueL0[nextReadL0].address, readQueueL0[nextReadL0].memAddress, 
                        readQueueL0[nextReadL0].size, readQueueL0[nextReadL0].requested, readQueueL0[nextReadL0].received);

                    panic("TextureCache", "clock", "Decompresing uninitialized memory block (LATC2_SIGNED).");
                }

                //  Uncompress the block.
                TextureEmulator::decompressLATC2Signed(comprBuffer, inputBufferL0[nextReadL0],
                    lineSizeL0 >> LATC1_LATC2_SPACE_SHIFT);

                break;

            default:

                panic("TextureCache", "clock", "Unsupported texture compression mode.");
                break;
        }

        /*  Update pointer to the input to decompress.  */
        nextReadL0 = GPU_MOD(nextReadL0 + 1, inputRequestsL0);

        /*  Update number of inputs to decompress.  */
        readInputsL0--;

        /*  Update the number of blocks being uncompressed.  */
        uncompressingL0++;
    }

    /*****************************************************************/
    /*****************************************************************/

    /*  Write uncompressed block/line to the cache.  */
    if ((uncompressedL0 > 0) && (writeCycles == 0))
    {
        /*  Try to write the cache line.  */
        if (cacheL0->writeLine(readQueueL0[nextUncompressedL0].request->way,
            readQueueL0[nextUncompressedL0].request->line,
            inputBufferL0[nextUncompressedL0], lastFilledLineTag))
        {
            GPU_DEBUG(
                printf("TextureCache => Writing input request %d to L0 cache\n",
                    nextUncompressedL0);
            )

            /*  Update number of uncompressed blocks.  */
            uncompressedL0--;

            /*  Update number of input requests writing to a cache line.  */
            readsWritingL0++;

            /*  Set cache write port cycles.  */
            writeCycles += (u32bit) ceil((f32bit) lineSizeL0 / ((f32bit) portWidth * readPorts));

            /*  Set writing line flag.  */
            writingLine = TRUE;
        }
    }


    /*****************************************************************/
    /*****************************************************************/

    /*  Check if there are pending read inputs for L1 cache.  */
    if (readInputsL1 > 0)
    {
        /*  Check if the whole line was recieved.  */
        if (readQueueL1[nextReadInputL1].received == readQueueL1[nextReadInputL1].size)
        {
            /*  Try to write the cache line.  */
            if (cacheL1->writeLine(readQueueL1[nextReadInputL1].requestL1->way,
                readQueueL1[nextReadInputL1].requestL1->line,
                inputBufferL1[nextReadInputL1]))
            {
                GPU_DEBUG(
                    printf("TextureCacheL2-%02d => Writing input request %d to L1 cache line (%d, %d)\n",
                        cacheID, nextReadInputL1, readQueueL1[nextReadInputL1].requestL1->way, readQueueL1[nextReadInputL1].requestL1->line);
                )

                GPU_DEBUG(
                    printf("TextureCache => End of L1 cache write.\n");
                )

                /*  Free fill cache request.  */
                cacheL1->freeRequest(readQueueL1[nextReadInputL1].requestID, FALSE, TRUE);

                /*  Clear cache request pointer.  */
                readQueueL1[nextReadInputL1].requestL1 = NULL;

                /*  Update number of uncompressed blocks.  */
                readInputsL1--;

                /*  Update pointer to the next read input for L1 cache.  */
                nextReadInputL1 = GPU_MOD(nextReadInputL1 + 1, inputRequestsL1);

                /*  Update number of free entries in the L1 cache request queue.  */
                freeReadsL1++;

                /*  Update number of input requests writing to a cache line.  */
                //readsWritingL1++;

                /*  Set cache write port cycles.  */
                //writeCycles += (u32bit) ceil((f32bit) lineSize / ((f32bit) portWidth * readPorts));

                /*  Set writing line flag.  */
                //writingLine = TRUE;
            }
        }
    }

    /*  Set the new transaction generated.  */
    nextTransaction = memTrans;

    //  Update statistics.
    pendingRequests->inc(MAX_MEMORY_TICKETS - freeTickets);
    pendingRequestsAvg->incavg(MAX_MEMORY_TICKETS - freeTickets);
}


/*  Requests a line of data.  */
MemoryTransaction *TextureCacheL2::requestBlock(u32bit readReq)
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
            readQueueL1[readReq].size - readQueueL1[readReq].requested);

        /*  Get parameters.  */
        offset = readQueueL1[readReq].requested;

        //  Get the next read ticket.
        u32bit nextReadTicket = ticketList.pop();
        
        /*  Keep the transaction ticket associated with the request.  */
        memoryRequest[nextReadTicket] = readReq;

        /*  Create the new memory transaction.  */
        memTrans = new MemoryTransaction(
            MT_READ_REQ,
            readQueueL1[readReq].memAddress + offset,
            size,
            &inputBufferL1[readReq][offset],
            TEXTUREUNIT,
            cacheID,
            nextReadTicket);

        /*  If the request has a dynamic object as a source copy the cookies.  */
        if (readQueueL1[readReq].requestL1->source != NULL)
        {
            memTrans->copyParentCookies(*readQueueL1[readReq].requestL1->source);
            memTrans->addCookie();
        }

        GPU_DEBUG(
            printf("TextureCacheL2-%02d => MT_READ_REQ addr %x size %d ticket %d.\n",
                cacheID, readQueueL1[readReq].memAddress + offset, size, nextReadTicket);
        )

        /*  Update requested block bytes counter.  */
        readQueueL1[readReq].requested += size;

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

void TextureCacheL2::stallReport(u64bit cycle, string &stallReport)
{
    stringstream reportStream;
    
    char fullName[128];
    sprintf(fullName, "TextureCacheL2-%02d", cacheID);
    
    reportStream << fullName << " stall report for cycle " << cycle << endl;
    reportStream << "------------------------------------------------------------" << endl;


    reportStream << " Fill pipeline (L0) : " << endl;
    reportStream << "   fetchInputs = " << fetchInputsL0 << " | waitReadInputs = " << waitReadInputsL0 << " | readInputs = " << readInputsL0;
    reportStream << " | uncompressing = " << uncompressingL0 << " | uncompressed = " << uncompressedL0 << " | readsWriting = " << readsWritingL0 << endl;
    reportStream << " Cache state (L0) : " << endl;
    for(u32bit port = 0; port < readPorts; port ++)
        reportStream << "    Read port " << port << " -> readCycles = " << readCycles[port] << endl;
    for(u32bit port = 0; port < 1; port ++)
        reportStream << "    Write port " << port << " -> writeCycles = " << writeCycles << endl;
    reportStream << "    writingLine = " << writingLine << endl;
    reportStream << " Compressor state (L0) : " << endl;
    reportStream << "    uncompressCycles = " << uncompressCycles << endl;    
    reportStream << " Fill pipeline (L1) : " << endl;
    reportStream << "    inputs = " << inputsL1 << " | inputsRequested = " << inputsRequestedL1 << " | readInputs = " << readInputsL1 << endl;
    reportStream << " Memory state (L1) : " << endl;
    reportStream << "    memoryCycles = " << memoryCycles << " | memoryRead = " << memoryRead << " | freeTickets = " << freeTickets << endl;

    reportStream << " Waiting read at L0 -> memAddress = " << hex << readQueueL0[nextReadInputL0].memAddress << dec;
    reportStream << " | way = " << readQueueL0[nextReadInputL0].way << " | line = " << readQueueL0[nextReadInputL0].line << endl;
    
    stallReport.assign(reportStream.str());
}

void TextureCacheL2::setDebug(bool enable)
{
    debugMode = enable;
    cacheL0->setDebug(enable);
    cacheL1->setDebug(enable);
}


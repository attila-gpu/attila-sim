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
 * $RCSfile: FetchCache64.cpp,v $
 * $Revision: 1.14 $
 * $Author: vmoya $
 * $Date: 2006-07-11 07:16:55 $
 *
 * Fetch Cache class implementation file.  Version for 64 bit addresses.
 *
 */

/**
 *
 * @file FetchCache.cpp
 *
 * Implements the Fetch Cache class.  This class implements a type of
 * cache that supports fetching lines before using them.  Version for 64 bit addresses.
 *
 */

#include "FetchCache64.h"
#include "GPUMath.h"
#include <cstdio>
#include <cstring>

using namespace gpu3d;

#undef GPU_DEBUG
#define GPU_DEBUG(expr) if (debugMode) { expr }

/*
 *
 *  This macro is temporaly used to mask statistic gathering for all those fetch caches
 *  that have not been updated to provide the name/postfix parameter.
 *
 */
#define UPDATE_STATS( instr ) { if (name != NULL) { instr } }

/*  Fetch cache constructor.  */
FetchCache64::FetchCache64(u32bit ways, u32bit lines, u32bit lineBytes, u32bit reqQSize,
    char *fetchCacheName) :

    requestQueueSize(reqQSize), debugMode(false),
    Cache64(ways, lines, lineBytes, CACHE_NONE)
{
    u32bit i;
    u32bit j;

    /*  Check if a name has been provided.  */
    if (fetchCacheName == NULL)
    {
        /*  The fetch cache has no name.  */
        name = NULL;
    }
    else
    {
        /*  Allocate space for the fetch cache name.  */
        name = new char[strlen(fetchCacheName) + 1];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (name == NULL)
                panic("FetchCache64", "FetchCache64", "Error allocating fetch cache name.");
        )

        /*  Copy name.  */
        strcpy(name, fetchCacheName);

        /*  Create statistics.  */
        fetchMisses = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissesFetch_FC64", u32bit(0), "FetchCache", name);
        fetchHits = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("HitsFetch_FC64", u32bit(0), "FetchCache", name);
        fetchMissOK = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissOKFetch_FC64", u32bit(0), "FetchCache", name);
        fetchMissFail = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissFailFetch_FC64", u32bit(0), "FetchCache", name);
        fetchMissFailReqQ = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissFailReqQueueFetch_FC64", u32bit(0), "FetchCache", name);
        fetchMissFailRes = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissFailReserveFetch_FC64", u32bit(0), "FetchCache", name);
        fetchMissFailMiss = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissFailMissFetch_FC64", u32bit(0), "FetchCache", name);
        allocMisses = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissesAlloc_FC64", u32bit(0), "FetchCache", name);
        allocHits = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("HitsAlloc_FC64", u32bit(0), "FetchCache", name);
        allocMissOK = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissOKAlloc_FC64", u32bit(0), "FetchCache", name);
        allocMissFail = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissFailAlloc_FC64", u32bit(0), "FetchCache", name);
        allocMissFailReqQ = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissFailReqQueueAlloc_FC64", u32bit(0), "FetchCache", name);
        allocMissFailRes = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissFailReserveAlloc_FC64", u32bit(0), "FetchCache", name);
        allocMissFailRes = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("MissFailMissAlloc_FC64", u32bit(0), "FetchCache", name);
        readOK = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadsOK_FC64", u32bit(0), "FetchCache", name);
        readFail = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadsFail_FC64", u32bit(0), "FetchCache", name);
        writeOK = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("WritesOK_FC64", u32bit(0), "FetchCache", name);
        writeFail = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("WritesFail_FC64", u32bit(0), "FetchCache", name);
        readBytes = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("ReadBytes_FC64", u32bit(0), "FetchCache", name);
        writeBytes = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("WriteBytes_FC64", u32bit(0), "FetchCache", name);
        unreserves = &GPUStatistics::StatisticsManager::instance().getNumericStatistic("Unreserves_FC64", u32bit(0), "FetchCache", name);
    }


    /*  Allocate the line reserve counters.  */
    reserve = new u32bit*[numWays];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (reserve == NULL)
            panic("FetchCache64", "FetchCache64", "Error allocating line reserve counters.");
    )

    /*  Allocate replace bit.  */
    replaceLine = new bool*[numWays];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (replaceLine == NULL)
            panic("FetchCache64", "FetchCache64", "Error allocating replace bit.");
    )

    /*  Allocate dirty bit (per way).  */
    dirty = new bool*[numWays];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (dirty == NULL)
            panic("FetchCache64", "FetchCache64", "Error allocating dirty bit (per way).");
    )

    /*  Allocated masked bit (per way).  */
    masked = new bool*[numWays];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (masked == NULL)
            panic("FetchCache64", "FetchCache64", "Error allocating masked bit (per way).");
    )

    /*  Allocate line write mask.  */
    writeMask = new bool**[numWays];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (writeMask == NULL)
            panic("FetchCache64", "FetchCache64", "Error allocating line write mask.");
    )

    /*  Allocate reserve counters annd replace bits per way.  */
    for(i = 0; i < numWays; i++)
    {
        /*  Allocate reserve counters for a way.  */
        reserve[i] = new u32bit[numLines];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (reserve[i] == NULL)
                panic("FetchCache64", "FetchCache64", "Error allocating line reserve counters for a way.");
        )

        /*  Allocate replace bit for a way.  */
        replaceLine[i] = new bool[numLines];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (replaceLine[i] == NULL)
                panic("FetchCache64", "FetchCache64", "Error allocating replace bit for way.");
        )

        /*  Allocate dirty bit (per line).  */
        dirty[i] = new bool[numLines];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (dirty[i] == NULL)
                panic("FetchCache64", "FetchCache64", "Error allocating dirty bit (per line).");
        )

        /*  Allocate masked bit (per line).  */
        masked[i] = new bool[numLines];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (masked[i] == NULL)
                panic("FetchCache64", "FetchCache64", "Error allocating masked bit (per line).");
        )

        /*  Allocate line write mask for a way.  */
        writeMask[i] = new bool*[numLines];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (writeMask[i] == NULL)
                panic("FetchCache64", "FetchCache64", "Error allocating line write mask for way.");
        )

        /*  Allocate line write masks.  */
        for(j = 0; j < numLines; j++)
        {
            /*  Allocate line write mask.  */
            writeMask[i][j] = new bool[lineSize];

            /*  Check allocation.  */
            GPU_ASSERT(
                if (writeMask[i][j] == NULL)
                    panic("FetchCache64", "FetchCache64", "Error allocating line write mask.");
            )
        }
    }

    /*  Set number of entries in the victim list.  */
    maxLRU = GPU_MIN(u32bit(MAX_LRU), numWays);

    /*  Allocate victim lists for the cache lines.  */
    victim = new u32bit*[numLines];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (victim == NULL)
            panic("FetchCache64", "FetchCache64", "Error allocating victim lists.");
    )

    for(i = 0; i < numLines; i++)
    {
        /*  Allocate victim list for each line.  */
        victim[i] = new u32bit[maxLRU];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (victim[i] == NULL)
                panic("FetchCache64", "FetchCache64", "Error allocating victim list for line");
        )
    }


    /*  Allocate the memory request queue.  */
    requestQueue = new Cache64Request[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (requestQueue == NULL)
            panic("FetchCache64", "FetchCache64", "Memory request queue could not be allocated.");
    )

    /*  Allocate the free memory request entry list.  */
    freeRequestList = new u32bit[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (freeRequestList == NULL)
            panic("FetchCache64", "FetchCache64", "Error allocating free memory request list.");
    )

    /*  Allocate the active memory request entry list.  */
    activeList = new u32bit[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (activeList == NULL)
            panic("FetchCache64", "FetchCache64", "Error allocating active request list.");
    )

    /*  Reset the fetch cache.  */
    reset();
}


/*  Fetches a cache line.  */
bool FetchCache64::fetch(u64bit address, u32bit &way, u32bit &line, DynamicObject *source)
{
    u32bit freeRequest;
    u64bit oldAddress;
    bool hit;
    u32bit i;

    /*  Search the address in the fetch cache tag file.  */
    hit = search(address, line, way);

    /*  Check if the line is a masked (partial) write.  */
    if (hit && masked[way][line])
    {
        /*  Check if all the bytes in the line have been written.  */
        for(i = 0; (i < lineSize) && hit; i++)
            hit = hit && writeMask[way][line][i];
    }

    /*  Check if there is a hit.  */
    if (hit)
    {
        GPU_DEBUG(
            printf("%s => Fetch hit address %016llx.\n", name, address);
        )

        /*  Hit.  Just update the reserve counter for the line.  */
        reserve[way][line]++;

        /*  Update statistics.  */
        UPDATE_STATS(
            fetchHits->inc();
        )

        /*  Line was reserved.  */
        return TRUE;
    }
    else
    {
        GPU_DEBUG(
            printf("%s => Fetch miss address %016llx.\n", name, address);
        )

        /*  Update statistics.  */
        UPDATE_STATS(
            fetchMisses->inc();
        )

        /*  Miss.  Search for a line to reserve.  */
        way = nextVictim(line);

        /*  Check if there is an unreserved line.  */
        if (reserve[way][line] == 0)
        {
            /*  Check if there is a free entry in the memory request queue.  */
            if (freeRequests > 0)
            {
                /*  Get next free request queue entry.  */
                freeRequest = freeRequestList[nextFreeRequest];

                /*  Get address of the line to be replaced.  */
                oldAddress = line2address(way, line);

                /*  Set the new tag for the fetch cache line.  */
                tags[way][line] = tag(address);

                /*  Check if the current data in the line is valid.  */
                if (valid[way][line] && dirty[way][line])
                {
                    /*  Add a write request to write back the fetch cache
                        line to memory.  */

                    /*  Add new request to memory request queue.  */
                    requestQueue[freeRequest].inAddress = line2address(way, line);
                    requestQueue[freeRequest].outAddress = oldAddress;
                    requestQueue[freeRequest].line = line;
                    requestQueue[freeRequest].way = way;
                    requestQueue[freeRequest].spill = TRUE;
                    requestQueue[freeRequest].fill = TRUE;
                    requestQueue[freeRequest].masked = masked[way][line];
                    requestQueue[freeRequest].free = FALSE;
                    requestQueue[freeRequest].source = source;
                }
                else
                {

                    /*  Add a read request to read the new data for the line.  */
                    requestQueue[freeRequest].inAddress = line2address(way, line);
                    requestQueue[freeRequest].outAddress = oldAddress;
                    requestQueue[freeRequest].line = line;
                    requestQueue[freeRequest].way = way;
                    requestQueue[freeRequest].spill = FALSE;
                    requestQueue[freeRequest].fill = TRUE;
                    requestQueue[freeRequest].free = FALSE;
                    requestQueue[freeRequest].source = source;

                    /*  Set line write mask to false.  */
                    //for(i = 0; i < lineSize; i++)
                    //    writeMask[way][line][i] = FALSE;
                }

                /*  Update pointer to next free memory request entry.  */
                nextFreeRequest = GPU_MOD(nextFreeRequest + 1, requestQueueSize);

                /*  Update free memory request entries counter.  */
                freeRequests--;

                /*  Add the new request to the active request list.  */
                activeList[GPU_MOD(nextRequest + activeRequests, requestQueueSize)] = freeRequest;

                /*  Update active memory request entries counter.  */
                activeRequests++;

                /*  Set line as reserved.  */
                reserve[way][line]++;

                /*  Set line as marked for replacing.  */
                replaceLine[way][line] = TRUE;

                /*  Mark line as valid.  */
                valid[way][line] = TRUE;

                /*  Mark as not masked line (normal mode).  */
                masked[way][line] = FALSE;

                /*  Mark as not a dirty line.  */
                dirty[way][line] = FALSE;

                /*  Update statistics.  */
                UPDATE_STATS(
                    fetchMissOK->inc();
                )

                /*  Line was fetched and reserved.  */
                return TRUE;
            }
            else
            {
                GPU_DEBUG(
                    printf("%s => No free entry in the memory request queue.\n", name);
                )

                /*  Update statistics.  */
                UPDATE_STATS(
                    fetchMissFail->inc();
                    fetchMissFailReqQ->inc();
                )

                /*  No free entry in the memory request queue.  */
                return FALSE;
            }
        }
        else
        {
            GPU_DEBUG(
                printf("%s => All cache lines are reserved.\n", name);
            )

            /*  Update statistics.  */
            UPDATE_STATS(
                fetchMissFail->inc();
                fetchMissFailRes->inc();
            )

            /*  No unreserved line available for the the address. Data cannot be fetched.  */
            return FALSE;
        }
    }
}

/*  Fetches a cache line (cover for out of order reading).  */
bool FetchCache64::fetch(u64bit address, u32bit &way, u32bit &line, bool &miss, bool &ready, DynamicObject *source)
{
    bool fetched;

    fetched = fetch(address, way, line, miss, source);

    ready = !replaceLine[way][line] && fetched;

    return fetched;
}

/*  Fetches a cache line.  */
bool FetchCache64::fetch(u64bit address, u32bit &way, u32bit &line, bool &miss, DynamicObject *source)
{
    u32bit freeRequest;
    u64bit oldAddress;
    bool hit;
    u32bit i;

    /*  Search the address in the fetch cache tag file.  */
    hit = search(address, line, way);

    /*  Check if the line is a masked (partial) write.  */
    if (hit && masked[way][line])
    {
        /*  Check if all the bytes in the line have been written.  */
        for(i = 0; (i < lineSize) && hit; i++)
            hit = hit && writeMask[way][line][i];
    }

    /*  Check if there is a hit.  */
    if (hit)
    {
        GPU_DEBUG(
            printf("%s => Fetch hit address %016llx.\n", name, address);
        )

        /*  Hit.  Just update the reserve counter for the line.  */
        reserve[way][line]++;

        /*  Update statistics.  */
        UPDATE_STATS(
            fetchHits->inc();
        )

        /*  Line was reserved.  */
        return TRUE;
    }
    else
    {
        /*  Update statistics.  */
        UPDATE_STATS(
            fetchMisses->inc();
        )

        /*  Check if the Fetch Allowed to allocate the line on a miss.  */
        if (miss)
        {
            /*  Line produced a miss.  */
            miss = TRUE;

            /*  Update statistics.  */
            UPDATE_STATS(
                fetchMissFail->inc();
                fetchMissFailMiss->inc();
            )

            /*  No fetch allowed on miss.  */
            return FALSE;
        }

        /*  Line produced a miss.  */
        miss = TRUE;

        GPU_DEBUG(
            printf("%s => Fetch miss address %016llx.\n", name, address);
        )

        /*  Miss.  Search for a line to reserve.  */
        way = nextVictim(line);

        /*  Check if there is an unreserved line.  */
        if (reserve[way][line] == 0)
        {
            /*  Check if there is a free entry in the memory request queue.  */
            if (freeRequests > 0)
            {
                /*  Get next free request queue entry.  */
                freeRequest = freeRequestList[nextFreeRequest];

                /*  Get address of the line to be replaced.  */
                oldAddress = line2address(way, line);

                /*  Set the new tag for the fetch cache line.  */
                tags[way][line] = tag(address);

                /*  Check if the current data in the line is valid.  */
                if (valid[way][line] && dirty[way][line])
                {
                    /*  Add a write request to write back the fetch cache
                        line to memory.  */

                    /*  Add new request to memory request queue.  */
                    requestQueue[freeRequest].inAddress = line2address(way, line);
                    requestQueue[freeRequest].outAddress = oldAddress;
                    requestQueue[freeRequest].line = line;
                    requestQueue[freeRequest].way = way;
                    requestQueue[freeRequest].spill = TRUE;
                    requestQueue[freeRequest].fill = TRUE;
                    requestQueue[freeRequest].masked = masked[way][line];
                    requestQueue[freeRequest].free = FALSE;
                    requestQueue[freeRequest].source = source;
                }
                else
                {
                    /*  Add a read request to read the new data for the line.  */
                    requestQueue[freeRequest].inAddress = line2address(way, line);
                    requestQueue[freeRequest].outAddress = oldAddress;
                    requestQueue[freeRequest].line = line;
                    requestQueue[freeRequest].way = way;
                    requestQueue[freeRequest].spill = FALSE;
                    requestQueue[freeRequest].fill = TRUE;
                    requestQueue[freeRequest].free = FALSE;
                    requestQueue[freeRequest].source = source;

                    /*  Set line write mask to false.  */
                    //for(i = 0; i < lineSize; i++)
                    //    writeMask[way][line][i] = FALSE;
                }

                /*  Update pointer to next free memory request entry.  */
                nextFreeRequest = GPU_MOD(nextFreeRequest + 1, requestQueueSize);

                /*  Update free memory request entries counter.  */
                freeRequests--;

                /*  Add the new request to the active request list.  */
                activeList[GPU_MOD(nextRequest + activeRequests, requestQueueSize)] = freeRequest;

                /*  Update active memory request entries counter.  */
                activeRequests++;

                /*  Set line as reserved.  */
                reserve[way][line]++;

                /*  Set line as marked for replacing.  */
                replaceLine[way][line] = TRUE;

                /*  Mark line as valid.  */
                valid[way][line] = TRUE;

                /*  Mark as not masked line (normal mode).  */
                masked[way][line] = FALSE;

                /*  Mark as not a dirty line.  */
                dirty[way][line] = FALSE;

                /*  Update statistics.  */
                UPDATE_STATS(
                    fetchMissOK->inc();
                )

                /*  Line was fetched and reserved.  */
                return TRUE;
            }
            else
            {
                GPU_DEBUG(
                    printf("%s => No free entry in the memory request queue.\n", name);
                )

                /*  Update statistics.  */
                UPDATE_STATS(
                    fetchMissFail->inc();
                    fetchMissFailReqQ->inc();
                )

                /*  No free entry in the memory request queue.  */
                return FALSE;
            }
        }
        else
        {
            GPU_DEBUG(
                printf("%s => All cache lines are reserved.\n", name);
                for(u32bit w = 0; w < numWays; w++)
                {
                    printf(" Set %d Way %d -> reserved? %s | tag = %016llx\n", line, w, reserve[w][line]? "Yes" : "No", tags[w][line]);
                }
            )

            /*  Update statistics.  */
            UPDATE_STATS(
                fetchMissFail->inc();
                fetchMissFailRes->inc();
            )

            /*  No unreserved line available for the the address. Data cannot be fetched.  */
            return FALSE;
        }
    }
}

/*  Fetches a cache line.  */
bool FetchCache64::allocate(u64bit address, u32bit &way, u32bit &line, DynamicObject *source)
{
    u32bit i;
    u32bit freeRequest;
    u64bit oldAddress;

    /*  Search the address in the fetch cache tag file.  */
    if (search(address, line, way))
    {
        GPU_DEBUG(
            printf("%s => Allocate hit address %016llx.\n", name, address);
        )

        /*  Hit.  Just update the reserve counter for the line.  */
        reserve[way][line]++;

        /*  Update statistics.  */
        UPDATE_STATS(
            allocHits->inc();
        )

        /*  Line was reserved.  */
        return TRUE;
    }
    else
    {
        GPU_DEBUG(
            printf("%s => Allocate miss address %016llx at line %d.\n", name, address, line);
        )

        /*  Update statistics.  */
        UPDATE_STATS(
            allocMisses->inc();
        )

        /*  Works as a write buffer, just flush the line if it was modified.  */

        /*  Search for a line to reserve.  */
        way = nextVictim(line);

        /*  Check if there is an unreserved line.  */
        if (reserve[way][line] == 0)
        {
            /*  Check if the current data in the line is valid.  */
            if (valid[way][line] && dirty[way][line])
            {
                GPU_DEBUG(
                    printf("%s => Valid line.\n", name);
                )

                /*  Check if there is a free entry in the memory request queue.  */
                if (freeRequests > 0)
                {
                    /*  Add a write request to write back the fetch cache
                        line to memory.  */

                    /*  Get address of the line to be replaced.  */
                    oldAddress = line2address(way, line);

                    /*  Set the new tag for the fetch cache line.  */
                    tags[way][line] = tag(address);

                    GPU_DEBUG(
                        printf("%s => Flush line at address %016llx and allocate line at address %016llx\n",
                            name, oldAddress, line2address(way, line));
                    )

                    /*  Get next free request queue entry.  */
                    freeRequest = freeRequestList[nextFreeRequest];

                    GPU_DEBUG(
                        printf("%s => Adding request %d\n", name, freeRequest);
                    )

                    /*  Add new request to memory request queue.  */
                    requestQueue[freeRequest].inAddress = 0;
                    requestQueue[freeRequest].outAddress = oldAddress;
                    requestQueue[freeRequest].line = line;
                    requestQueue[freeRequest].way = way;
                    requestQueue[freeRequest].spill = TRUE;
                    requestQueue[freeRequest].fill = FALSE;
                    requestQueue[freeRequest].masked = masked[way][line];
                    requestQueue[freeRequest].free = FALSE;
                    requestQueue[freeRequest].source = source;

                    /*  Update pointer to next free memory request entry.  */
                    nextFreeRequest = GPU_MOD(nextFreeRequest + 1, requestQueueSize);

                    /*  Update free memory request entries counter.  */
                    freeRequests--;

                    /*  Add the new request to the active request list.  */
                    activeList[GPU_MOD(nextRequest + activeRequests, requestQueueSize)] = freeRequest;

                    /*  Update active memory request entries counter.  */
                    activeRequests++;

                    /*  Set line as marked for replacing.  */
                    replaceLine[way][line] = TRUE;

                    /*  Set line as reserved.  */
                    reserve[way][line]++;

                    /*  Mark line as valid.  */
                    valid[way][line] = TRUE;

                    /*  Mark as a masked line (write buffer mode!!).  */
                    masked[way][line] = TRUE;

                    /*  Marks as not dirty line.  */
                    dirty[way][line] = FALSE;

                    /*  Update statistics.  */
                    UPDATE_STATS(
                        allocMissOK->inc();
                    )

                    return TRUE;
                }
                else
                {
                    /*  Update statistics.  */
                    UPDATE_STATS(
                        allocMissFail->inc();
                        allocMissFailReqQ->inc();
                    )

                    /*  No free entry in the memory request queue.  */
                    return FALSE;
                }
            }
            else
            {
                /*  If invalid line just update tags and set as valid.  Wait
                    for next cycle before allowing the write.  */

                /*  Set the new tag for the fetch cache line.  */
                tags[way][line] = tag(address);

                GPU_DEBUG(
                    printf("%s => Invalid line found. Just clear write mask.\n", name);
                )

                /*  Set line write mask to false.  */
                for(i = 0; i < lineSize; i++)
                    writeMask[way][line][i] = FALSE;

                /*  Set line as reserved.  */
                reserve[way][line]++;

                /*  Mark line as valid.  */
                valid[way][line] = TRUE;

                /*  Mark as a masked line (write buffer mode!!!).  */
                masked[way][line] = TRUE;

                /*  Set as not dirty.  */
                dirty[way][line] = FALSE;

                /*  Update statistics.  */
                UPDATE_STATS(
                    allocMissOK->inc();
                )

                /*  Line allocated.  */
                return TRUE;
            }
        }
        else
        {
            /*  Update statistics.  */
            UPDATE_STATS(
                allocMissFail->inc();
                allocMissFailRes->inc();
            )

            /*  No line available to allocate and reserve.  */
            return FALSE;
        }
    }

}

/*  Reads data from the cache.  */
bool FetchCache64::read(u64bit address, u32bit way, u32bit line, u32bit size, u8bit *data)
{
    u32bit i;
    u32bit off;

    /*  NOTE:  All accesses are aligned to 4 bytes, so the two less signficant bits
        of the address are dropped.  */

    /*  Check size is a multiple of 4 bytes.  */
    GPU_ASSERT(
        if ((size & 0x03) != 0)
            panic("FetchCache64", "read", "Size of the data to read must be a multiple of 4.");
    )

    /*  Check if size is larger than the line size.  */
    GPU_ASSERT(
        if (size > lineSize)
            panic("FetchCache64", "read", "Trying to read more than a cache line.");
    )

    /*  Check the amount of data to read does not overflows the line.  */
    GPU_ASSERT(
        if (((offset(address) & 0xfffffffffffffffcULL) + size) > lineSize)
            panic("FetchCache64", "read", "Trying to read beyond the cache line.");
    )

    /*  Check if the address was previously fetched.  */
    GPU_ASSERT(
        if (tags[way][line] != tag(address))
            panic("FetchCache64", "read", "Trying to read an unfetched address.");
    )

    /*  Check if the line is available.  */
    if (!replaceLine[way][line])
    {
        /*  Get the requested amount of data.  */
        for(i = 0, off = (offset(address) & 0xfffffffffffffffcULL); i < size; i = i + 4, off = off + 4)
        {
            /*  Copy a data byte.  */
            *((u32bit *) &data[i]) = *((u32bit *) &cache[way][line][off]);
        }

        /*  Update replacement policy.  */
        access(way, line);

        /*  Update statistics.  */
        UPDATE_STATS(
            readOK->inc();
            readBytes->inc(size);
        )

        /*  Data was ready.  */
        return TRUE;
    }
    else
    {
        /*  Update statistics.  */
        UPDATE_STATS(
            readFail->inc();
        )

        /*  Line still has not been retrieved from memory.  */
        return FALSE;
    }
}

/*  Writes data to the fetch cache.  */
bool FetchCache64::write(u64bit address, u32bit way, u32bit line, u32bit size,
    u8bit *data)
{
    u32bit i;
    u32bit off;

    /*  NOTE:  All accesses are aligned to 4 bytes, so the two less signficant bits
        of the address are dropped.  */

   /*  Check if size is larger than the line size.  */
    GPU_ASSERT(
        if (size > lineSize)
            panic("FetchCache64", "write", "Trying to write more than a cache line.");
    )

    /*  Check the amount of data to write does not overflows the line.  */
    GPU_ASSERT(
        if (((offset(address) & 0xfffffffffffffffcULL) + size) > lineSize)
            panic("FetchCache64", "write", "Trying to write beyond the cache line.");
    )

    /*  Check size is a multiple of 4 bytes.  */
    GPU_ASSERT(
        if ((size & 0x03) != 0)
            panic("FetchCache64", "write", "Size of the data to write must be a multiple of 4.");
    )

    /*  Check if the address was previously fetched.  */
    GPU_ASSERT(
        if (tags[way][line] != tag(address))
            panic("FetchCache64", "write", "Trying to write an unfetched address.");
    )

    /*  Check if the line data is available.  */
    if (!replaceLine[way][line])
    {
        /*  Write data to the fetch cache.  */
        for(i = 0, off = (offset(address) & 0xfffffffffffffffcULL); i < size; i = i + 4, off = off + 4)
        {
            /*  Copy a data byte.  */
            *((u32bit *) &cache[way][line][off]) = *((u32bit *) &data[i]);

            /*
             *  NOTE:  MASKED WRITES AND UNMASKED WRITES SHOULDN'T BE
             *  MIXED.  IF THEY NEED TO BE MIXED THE WRITE MASK SHOULD
             *  BE SET AS SHOWN BELOW.
             *
             */

            /*  Set write mask.  */
            //writeMask[way][line][off] = TRUE;
            //writeMask[way][line][off + 1] = TRUE;
            //writeMask[way][line][off + 2] = TRUE;
            //writeMask[way][line][off + 3] = TRUE;
        }

        /*  Set line as dirty.  */
        dirty[way][line] = TRUE;

        /*  Free reserve.  */
        if (reserve[way][line] > 0)
            reserve[way][line]--;

        /*  Update replacement policy.  */
        access(way, line);

        /*  Update statistics.  */
        UPDATE_STATS(
            writeOK->inc();
            writeBytes->inc(size);
        )

        /*  Data was ready.  */
        return TRUE;
    }
    else
    {
        /*  Update statistics.  */
        UPDATE_STATS(
            writeFail->inc();
        )

        /*  Line still not ready.  */
        return FALSE;
    }
}

/*  Writes masked data to the fetch cache.  */
bool FetchCache64::write(u64bit address, u32bit way, u32bit line, u32bit size,
    u8bit *data, bool *mask)
{
    u32bit i;
    u32bit off;
    bool anyWrite;

    /*  NOTE:  All accesses are aligned to 4 bytes, so the two less signficant bits
        of the address are dropped.  */

    /*  Check if size is larger than the line size.  */
    GPU_ASSERT(
        if (size > lineSize)
            panic("FetchCache64", "write", "Trying to write more than a cache line.");
    )

    /*  Check the amount of data to write does not overflows the line.  */
    GPU_ASSERT(
        if (((offset(address) & 0xfffffffffffffffcULL) + size) > lineSize)
            panic("FetchCache64", "write", "Trying to write beyond the cache line.");
    )

     /*  Check if the address was previously fetched.  */
    GPU_ASSERT(
        if (tags[way][line] != tag(address))
            panic("FetchCache64", "write", "Trying to write an unfetched address.");
    )

    /*  There are no writes, yet.  */
    anyWrite = FALSE;

    /*  Check if the line data is available.  */
    if (!replaceLine[way][line])
    {
        /*  Write data to the fetch cache.  */
        for(i = 0, off = (offset(address) & 0xfffffffffffffffcULL); i < size; i++, off++)
        {
            /*  Check if write is enabled for the bytes.  */
            if (mask[i])
            {
                /*  Copy a data byte.  */
                cache[way][line][off] = data[i];

                /*  Set write mask.  */
                writeMask[way][line][off] = TRUE;

                /*  Set line as dirty.  */
                anyWrite = TRUE;
            }
        }

        /*  Set dirty bit.  */
        dirty[way][line] = dirty[way][line] || anyWrite;

        /*  Free reserve.  */
        if (reserve[way][line] > 0)
            reserve[way][line]--;

        /*  Update replacement policy.  */
        access(way, line);

        /*  Update statistics.  */
        UPDATE_STATS(
            writeOK->inc();
            writeBytes->inc(size);
        )

        /*  Data was ready.  */
        return TRUE;
    }
    else
    {
        /*  Update statistics.  */
        UPDATE_STATS(
            writeFail->inc();
        )

        /*  Line still not ready.  */
        return FALSE;
    }

}

/*  Reads a full cache line.  */
bool FetchCache64::readLine(u32bit way, u32bit line, u8bit *linedata)
{
    u32bit i;

    /*  Get the requested amount of data.  */
    for(i = 0; i < lineSize; i = i + 4)
    {
        /*  Copy a data byte.  */
        *((u32bit *) &linedata[i]) = *((u32bit *) &cache[way][line][i]);
    }

    /*  Update statistics.  */
    UPDATE_STATS(
        readBytes->inc(lineSize);
    )

    /*  Data was ready.  */
    return TRUE;
}

/*  Reads the line mask for a cache line.  */
void FetchCache64::readMask(u32bit way, u32bit line, u32bit *mask)
{
    u32bit i;

    /*  Build the write mask.  */
    for(i = 0; i < lineSize; i = i + 4)
    {
        ((u8bit *) mask)[i + 0] = (writeMask[way][line][i + 0] ? 0xff : 0x00);
        ((u8bit *) mask)[i + 1] = (writeMask[way][line][i + 1] ? 0xff : 0x00);
        ((u8bit *) mask)[i + 2] = (writeMask[way][line][i + 2] ? 0xff : 0x00);
        ((u8bit *) mask)[i + 3] = (writeMask[way][line][i + 3] ? 0xff : 0x00);
    }
}

//  Reset the line mask for the cache line.
void FetchCache64::resetMask(u32bit way, u32bit line)
{
    for(u32bit b = 0; b < lineSize; b++)
        writeMask[way][line][b] = false;
}

/*  Writes and replaces a full cache line.  */
bool FetchCache64::writeLine(u32bit way, u32bit line, u8bit *linedata, u64bit &tag)
{
    u32bit i;

    /*  Write data to the fetch cache.  */
    for(i = 0; i < lineSize; i = i + 4)
    {
        /*  Copy a data byte.  */
        *((u32bit *) &cache[way][line][i]) = *((u32bit *) &linedata[i]);

        /*  Set write mask.  */
        //writeMask[way][line][i] = FALSE;
        //writeMask[way][line][i + 1] = FALSE;
        //writeMask[way][line][i + 2] = FALSE;
        //writeMask[way][line][i + 3] = FALSE;
    }

    /*  Set line as not dirty.  */
    dirty[way][line] = FALSE;

    /*  Update statistics.  */
    UPDATE_STATS(
        writeBytes->inc(lineSize);
    )

    /*  Return the tag for the line.  */
    tag = line2address(way, line);

    /*  Data was ready.  */
    return TRUE;
}


/*  Unreserves a cache line.  */
void FetchCache64::unreserve(u32bit way, u32bit line)
{
    /*  Liberate reserve.  */
    if (reserve[way][line] > 0)
        reserve[way][line]--;

    /*  Update statistics.  */
    UPDATE_STATS(
        unreserves->inc();
    )
}

/*  Resets the fetch cache structures.  */
void FetchCache64::reset()
{
    u32bit i, j;

    GPU_DEBUG(
        printf("%s => Reset.\n", name);
    )

    /*  Reset valid bits.  */
    /*  Reset reserve counters.  */
    /*  Reset replace bits.  */
    for(i = 0; i < numWays; i++)
    {
        for (j = 0; j < numLines; j++)
        {
            /*  Reset reserve counter.  */
            reserve[i][j] = 0;

            /*  Reset tags.  */
            tags[i][j] = 0;

            /*  Reset valid bits.  */
            valid[i][j] = FALSE;

            /*  Reset replace bit.  */
            replaceLine[i][j] = FALSE;

            /*  Reset dirty bit.  */
            dirty[i][j] = FALSE;

            /*  Reset masked bit.  */
            masked[i][j] = FALSE;
        }
    }

    /*  Reset victim lists.  */
    for(i = 0; i < numLines; i++)
    {
        for(j = 0; j < maxLRU; j++)
        {
            victim[i][j] = j;
        }
    }

    /*  Set first search way.  */
    firstWay = 0;

    /*  Reset free request counter.  */
    freeRequests = requestQueueSize;

    /*  Reset active request counter.  */
    activeRequests = 0;

    /*  Reset next free memory request entry pointer.  */
    nextFreeRequest = 0;

    /*  Reset pointer to the next request.  */
    nextRequest = 0;

    /*  Reset free request list.  */
    for(i = 0; i < requestQueueSize; i++)
    {
        /*  Default value for the memory request list.  */
        freeRequestList[i] = i;
    }
}

/*  Copy back all the valid cache lines to memory.  */
bool FetchCache64::flush()
{
    u32bit way;
    u32bit line;
    u32bit freeRequest;
    u64bit oldAddress;

    /*  Search for valid cache lines.  */
    for(way = 0; (way < numWays) && (freeRequests > 0); way++)
    {
        for(line = 0; (line < numLines) && (freeRequests > 0); line++)
        {
            /*  Check there are free request entry*/
            if (valid[way][line])
            {
                /*  Add a write request to write back the fetch cache
                    line to memory.  */

                /*  Get next free request queue entry.  */
                freeRequest = freeRequestList[nextFreeRequest];

                /*  Get address of the line to be replaced.  */
                oldAddress = line2address(way, line);

                /*  Add new request to memory request queue.  */
                requestQueue[freeRequest].outAddress = oldAddress;
                requestQueue[freeRequest].line = line;
                requestQueue[freeRequest].way = way;
                requestQueue[freeRequest].fill = FALSE;
                requestQueue[freeRequest].spill = TRUE;
                requestQueue[freeRequest].masked = masked[way][line];
                requestQueue[freeRequest].free = FALSE;

                /*  Update pointer to next free memory request entry.  */
                nextFreeRequest = GPU_MOD(nextFreeRequest + 1, requestQueueSize);

                /*  Update free memory request entries counter.  */
                freeRequests--;

                /*  Add the new request to the active request list.  */
                activeList[GPU_MOD(nextRequest + activeRequests, requestQueueSize)] = freeRequest;

                /*  Update active memory request entries counter.  */
                activeRequests++;

                /*  Mark line as valid.  */
                valid[way][line] = FALSE;
            }
        }
    }

    /*  Wait until all the requests have been finished.  */

    return (freeRequests == requestQueueSize);
}

/*  Returns the current cache request.  */
Cache64Request *FetchCache64::getRequest(u32bit &cacheRequest)
{
    Cache64Request *request;

    /*  Check if there is a request in the queue.  */
    if (activeRequests > 0)
    {
        /*  Get the next requests in the queue.  */
        request = &requestQueue[activeList[nextRequest]];

        /*  Keep the request queue entry pointer.  */
        cacheRequest = activeList[nextRequest];

        /*  Update next request pointeer.  */
        nextRequest = GPU_MOD(nextRequest + 1, requestQueueSize);

        /*  Update active request counter.  */
        activeRequests--;

        return request;
    }
    else
    {
        return NULL;
    }
}

/*  Update cache request pointer.  */
void FetchCache64::freeRequest(u32bit cacheRequest, bool freeSpill, bool freeFill)
{
    u32bit way;
    u32bit line;
    u32bit i;

    /*  Free spill request.  */
    requestQueue[cacheRequest].spill = requestQueue[cacheRequest].spill && !freeSpill;

    /*  Free fill request.  */
    requestQueue[cacheRequest].fill = requestQueue[cacheRequest].fill && !freeFill;

    /*  Check if both spill and fill have been liberated.  */
    if ((!requestQueue[cacheRequest].spill) && (!requestQueue[cacheRequest].fill))
    {
        /*  Get request way and line.  */
        way = requestQueue[cacheRequest].way;
        line = requestQueue[cacheRequest].line;

        /*  Set line for the cache request as replaced.  */
        replaceLine[way][line] = FALSE;

        /*  Set line write mask to false.  */
        for(i = 0; i < lineSize; i++)
            writeMask[way][line][i] = FALSE;

        /*  Set as free.  */
        requestQueue[cacheRequest].free = TRUE;

        /*  Add request to the free request list.  */
        freeRequestList[GPU_MOD(nextFreeRequest + freeRequests, requestQueueSize)] = cacheRequest;

        /*  Update free requests counter.  */
        freeRequests++;
    }
}

/*  Updates the victim list with a new access.  */
void FetchCache64::access(u32bit way, u32bit line)
{
    s32bit i;
    u32bit aux;
    u32bit aux2;

    /*  NOTE:  WE DO NOT CHECK LINE AND WAY!!!  */

    /*  Implementing a LRU policy (SHOULDN'T BE USED WITH MORE THAN 4 WAYS!!!!).  */

    /*  First element in the list is the next victim.  Last element is the last access.  */

    /*  Write the current access at the tail of the list.  */
    aux = victim[line][maxLRU - 1];
    victim[line][maxLRU - 1] = way;

    /*  Move all the old accesses.  */
    for(i = (maxLRU - 2); (i >= 0) && (aux != way); i--)
    {
        /*  Store way at current position.  */
        aux2 = victim[line][i];

        /*  Set new more recent in current position.  */
        victim[line][i] = aux;

        /*  Set as more recent to update.  */
        aux = aux2;
    }
}

/*  Returns the next victim for a line.  */
u32bit FetchCache64::nextVictim(u32bit line)
{
    u32bit i;
    u32bit j;


    /*  Takes into account the access order of the last MAX_LRU accesses and the reserve bits.
        The first element in the victim list is the older accessed way that we still remember.  */

    /*  Update from which way to start searching.  */
    firstWay = GPU_MOD(firstWay + 1, numWays);

    /*  First search if any of the not last MAX_LRU accessed ways is not reserved.  */
    for(i = firstWay; i < numWays; i++)
    {
        /*  Check if the way is not reserved.  */
        if (reserve[i][line] == 0)
        {
            /*  Search the way inside the last accesses list.  */
            for(j = 0; (j < maxLRU) && (victim[line][j] != i); j++);

            /*  Check if the way is not one of the last accesses.  */
            if (j == maxLRU)
            {
                /*  Use this way as victim.  */
                return i;
            }
        }
    }

    /*  Now search for the older not reserved in the last MAX_LRU accesses.  */
    for(i = 0; i < maxLRU; i++)
    {
        /*  Check if the victim is reserved.  */
        if (reserve[victim[line][i]][line] == 0)
            return victim[line][i];
    }

    /*  Doesn't matter what we return as all are reserved.  Reserved bit must be always rechecked
        before selecting the new line.  */
    return 0;
}

void FetchCache64::setDebug(bool enable)
{
    debugMode = enable;
}



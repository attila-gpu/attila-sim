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
 * $RCSfile: TextureUnit.cpp,v $
 * $Revision: 1.37 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:17 $
 *
 * Texture Unit Box.
 *
 */

/**
 *
 *  @file TextureUnit.cpp
 *
 *  This file implements the TextureUnit Box Class.
 *
 *  This box simulates the texture unit attached to a shader for
 *  performing memory accesses.
 *
 */

#include "TextureUnit.h"
#include "TextureUnitStateInfo.h"
#include "FilterOperation.h"
#include "GPUMath.h"
#include <cmath>
#include <cstdio>
#include <sstream>

using namespace gpu3d;


/*  Constructor for the Texture Unit BOX class.  */

TextureUnit::TextureUnit(TextureEmulator &textEmu, u32bit stFrags, u32bit reqCycle, u32bit reqQueueSz,
    u32bit accQueueSz, u32bit resQueueSz, u32bit readWaitWSz, bool twoLevel, u32bit cacheLineSzL0,
    u32bit cacheWaysL0, u32bit cacheLinesL0, u32bit portWidth, u32bit reqQSz, u32bit inputReqQSzL0,
    u32bit missesCycle, u32bit decomprLat, u32bit cacheLineSzL1, u32bit cacheWaysL1, u32bit cacheLinesL1,
    u32bit inputReqQSzL1, u32bit addrALULat, u32bit filterLat, char *name, char *prefix, Box* parent):

    /*  Pre-initializations.  */
    Box(name, parent), txEmul(textEmu), stampFragments(stFrags), requestsCycle(reqCycle), requestQueueSize(reqQueueSz),
    accessQueueSize(accQueueSz), resultQueueSize(resQueueSz), waitReadWindowSize(readWaitWSz), useTwoLevelCache(twoLevel),
    textCacheLineSizeL0(cacheLineSzL0), textCacheWaysL0(cacheWaysL0), textCacheLinesL0(cacheLinesL0), textCachePortWidth(portWidth),
    textCacheReqQSize(reqQSz), textCacheInReqSizeL0(inputReqQSzL0), textCacheMaxMiss(missesCycle),
    textCacheDecomprLatency(decomprLat), textCacheLineSizeL1(cacheLineSzL1), textCacheWaysL1(cacheWaysL1),
    textCacheLinesL1(cacheLinesL1), textCacheInReqSizeL1(inputReqQSzL1),
    addressALULatency(addrALULat), filterLatency(filterLat)

{
    DynamicObject *defValue[1];
    char fullName[64];
    char postfix[32];

    /*  Check parameters.  */
    GPU_ASSERT(
        if (stampFragments != 4)
            panic("TextureUnit", "TextureUnit", "Only implemented support for 4 fragment stamps.");
        if (requestsCycle == 0)
            panic("TextureUnit", "TextureUnit", "Texture requests per cycle must be at least 1.");
        if (requestQueueSize < (2 * requestsCycle))
            panic("TextureUnit", "TextureUnit", "Request queue requires at least twice the request rate entries.");
        if (accessQueueSize < 1)
            panic("TextureUnit", "TextureUnit", "Texture access queue requires at least an entry.");
        if (waitReadWindowSize < 1)
            panic("TextureUnit", "TextureUnit", "Read wait window requires at least one entry.");
        if (resultQueueSize < 1)
            panic("TextureUnit", "TextureUnit", "Result queue requires at least an entry.");
        if (textCacheLineSizeL0 == 0)
            panic("TextureUnit", "TextureUnit", "Texture cache line size can't be zero.");
        if (textCacheWaysL0 == 0)
            panic("TextureUnit", "TextureUnit", "Texture cache requires at least a way.");
        if (textCacheLinesL0 == 0)
            panic("TextureUnit", "TextureUnit", "Texture cache requires at least a line.");
        if (textCachePortWidth == 0)
            panic("TextureUnit", "TextureUnit", "Texture cache port must be at least a byte wide.");
        if (textCacheReqQSize == 0)
            panic("TextureUnit", "TextureUnit", "Texture cache request queue requires at least an entry.");
        if (textCacheInReqSizeL0 == 0)
            panic("TextureUnit", "TextureUnit", "Texture cache input request queue requires at least an entry.");
        if (textCacheMaxMiss == 0)
            panic("TextureUnit", "TextureUnit", "Texture cache maximum misses per cycles can't be zero.");
        if (addressALULatency < 1)
            panic("TextureUnit", "TextureUnit", "Address ALU latency must be at least 1.");
        if (filterLatency < 1)
            panic("TextureUnit", "TextureUnit", "Filter ALU latency must be at least 1.");
        if (useTwoLevelCache && (textCacheLineSizeL1 == 0))
            panic("TextureUnit", "TextureUnit", "Texture cache L1 line size can't be zero.");
        if (useTwoLevelCache && (textCacheWaysL1 == 0))
            panic("TextureUnit", "TextureUnit", "Texture cache L1 requires at least a way.");
        if (useTwoLevelCache && (textCacheLinesL1 == 0))
            panic("TextureUnit", "TextureUnit", "Texture cache L1 requires at least a line.");
        if (useTwoLevelCache && (textCacheInReqSizeL1 == 0))
            panic("TextureUnit", "TextureUnit", "Texture cache L1 input request queue requires at least an entry.");
    )

    /*  Create the full name and postfix for the statistics.  */
    sprintf(fullName, "%s::%s", prefix, name);
    sprintf(postfix, "TU-%s", prefix);

    GPU_TEX_TRACE(
        char filename[128];
        sprintf(filename, "texTrace-%s.txt.gz", prefix);
        texTrace.open(filename, fstream::out);
        sprintf(filename,"texAddrTrace-%s.txt.gz", prefix);
        texAddrTrace.open(filename, fstream::out);
        texTrace << hex;
        texAddrTrace << hex;
    )

    /*  Create statistics.  */
    txRequests = &getSM().getNumericStatistic("TextureRequests", u32bit(0), fullName, postfix);
    results = &getSM().getNumericStatistic("TextureResults", u32bit(0), fullName, postfix);
    fetchOK = &getSM().getNumericStatistic("FetchesOK", u32bit(0), fullName, postfix);
    fetchFail = &getSM().getNumericStatistic("FetchesFailed", u32bit(0), fullName, postfix);
    fetchSkip = &getSM().getNumericStatistic("FetchesSkiped", u32bit(0), fullName, postfix);
    fetchStallFetch = &getSM().getNumericStatistic("FetchStallFetch", u32bit(0), fullName, postfix);
    fetchStallAddress = &getSM().getNumericStatistic("FetchStallAddress", u32bit(0), fullName, postfix);
    fetchStallWaitRead = &getSM().getNumericStatistic("FetchStallWaitRead", u32bit(0), fullName, postfix);
    fetchStallReadyRead = &getSM().getNumericStatistic("FetchStallReadyRead", u32bit(0), fullName, postfix);
    readOK = &getSM().getNumericStatistic("ReadsOK", u32bit(0), fullName, postfix);
    readFail = &getSM().getNumericStatistic("ReadsFailed", u32bit(0), fullName, postfix);
    readBytes = &getSM().getNumericStatistic("ReadBytesMemory", u32bit(0), fullName, postfix);
    readTrans = &getSM().getNumericStatistic("ReadTransactions", u32bit(0), fullName, postfix);
    readyReadLevel = &getSM().getNumericStatistic("ReadyReadQueueOccupation", u32bit(0), fullName, postfix);
    waitReadLevel = &getSM().getNumericStatistic("WaitReadWindowOccupation", u32bit(0), fullName, postfix);
    resultLevel = &getSM().getNumericStatistic("ResultQueueOccupation", u32bit(0), fullName, postfix);
    accessLevel = &getSM().getNumericStatistic("AccessQueueOccupation", u32bit(0), fullName, postfix);
    requestLevel = &getSM().getNumericStatistic("RequestQueueOccupation", u32bit(0), fullName, postfix);
    addrALUBusy = &getSM().getNumericStatistic("AddressALUBusyCycles", u32bit(0), fullName, postfix);
    filterALUBusy = &getSM().getNumericStatistic("FilterALUBusyCycles", u32bit(0), fullName, postfix);
    textResLat = &getSM().getNumericStatistic("TextureResultLatency", u32bit(0), fullName, postfix);
    bilinearSamples = &getSM().getNumericStatistic("BilinearSamples", u32bit(0), fullName, postfix);
    anisoRatio = &getSM().getNumericStatistic("AnisotropyRatio", u32bit(0), fullName, postfix);
    addrCalcFinished  = &getSM().getNumericStatistic("AddressCalculationFinished", u32bit(0), fullName, postfix);

    //  Histogram for anisotropy samples taken per request.
    for(u32bit aratio = 0; aratio <= MAX_ANISOTROPY; aratio++)
    {
        char statsName[256];
        if (aratio == 0)
            sprintf(statsName, "HistoAnisoRatioNoAniso");
        else
            sprintf(statsName, "HistoAnisoRatio%02d", aratio);
        anisoRatioHisto[aratio] = &getSM().getNumericStatistic(statsName, u32bit(0), fullName, postfix);
    }
    
    //  Histogram for bilinear samples taken per request.
    for(u32bit bl = 1; bl <= (MAX_ANISOTROPY * 2); bl++)
    {
        char statsName[256];
        sprintf(statsName, "HistoBilinears%02d", bl);
        bilinearsHisto[bl - 1] = &getSM().getNumericStatistic(statsName, u32bit(0), fullName, postfix);
    }

    magnifiedPixels = &getSM().getNumericStatistic("MagnifiedPixels", u32bit(0), fullName, postfix);

    mipsSampled[0] = &getSM().getNumericStatistic("MipsSampled1", u32bit(0), fullName, postfix);
    mipsSampled[1] = &getSM().getNumericStatistic("MipsSampled2", u32bit(0), fullName, postfix);

    pointSampled = &getSM().getNumericStatistic("PointSampled", u32bit(0), fullName, postfix);
    
    /*  Create Box Signals.  */

    /*  Signal from the Command Processor.  */
    commProcSignal = newInputSignal("TextureUnitCommand", 1, 1, prefix);

    /*  Texture request signal from the Shader Decoder/Execute Box.  */
    textRequestSignal = newInputSignal("TextureRequest", requestsCycle, 1, prefix);

    /*  State signal to the Shader Decode Execute box.  */
    readySignal = newOutputSignal("TextureUnitState", 1, 1, prefix);

    /*  Build initial signal state.  */
    //defValue[0] = new TextureUnitStateInfo(TU_READY);

    /*  Set default state of Texture Unit state signal.  */
    //readySignal->setData(defValue);

    /*  Texture data signal to Shader Decode Execute box.  */
    textDataSignal = newOutputSignal("TextureData", 1, 1, prefix);


    /*  Create input signal for the address ALU latency simulator.  */
    addressALUInput = newInputSignal("TextureAddressALU", 1, addressALULatency + MAX_FILTER_LOOPS, prefix);

    /*  Create output signal for the address ALU latency simulator.  */
    addressALUOutput = newOutputSignal("TextureAddressALU", 1, addressALULatency + MAX_FILTER_LOOPS, prefix);


    /*  Create input signal for the filter latency simulator.  */
    filterInput = newInputSignal("TextureFilter", 1, filterLatency + MAX_FILTER_LOOPS, prefix);

    /*  Create output signal for the filter latency simulator.  */
    filterOutput = newOutputSignal("TextureFilter", 1, filterLatency + MAX_FILTER_LOOPS, prefix);


    /*  Create memory data signal from the Memory Controller.  */
    memDataSignal = newInputSignal("TextureMemoryData", 2, 1, prefix);

    /*  Create request signal to the Memory Controller.  */
    memRequestSignal = newOutputSignal("TextureMemoryRequest", 1, 1, prefix);

    /*  Check which version of Texture Cache is to be used.  */
    if (!useTwoLevelCache)
    {
        /*  Create the texture cache attached to the texture unit.  */
        textCacheL1 = new TextureCache(
            textCacheWaysL0,            /*  Number of ways in the cache.  */
            textCacheLinesL0,           /*  Number of lines (per way) in the cache.  */
            textCacheLineSizeL0,        /*  Size in bytes of each line.  */
            textCachePortWidth,         /*  Width of the read and write ports in bytes.  */
            textCacheReqQSize,          /*  Size of the request queue size.  */
            textCacheInReqSizeL0,       /*  Size of the input request queue.  */
            4,                          /*  Number of banks in the texture cache.  */
            stampFragments,             /*  Accesses supported per bank and cycle.  */
            4,                          /*  Width in bytes of each texture cache bank.  */
            textCacheMaxMiss,           /*  Number of misses supported per cycle.  */
            textCacheDecomprLatency,    /*  Compressed texture line decompression latency.  */
            postfix                     /*  Postfix for the cache statistics.  */
            );

        textCache = textCacheL1;
    }
    else
    {
        textCacheL2 = new TextureCacheL2(
            textCacheWaysL0,            /*  Number of ways in the L0 cache.  */
            textCacheLinesL0,           /*  Number of lines (per way) in the L0 cache.  */
            textCacheLineSizeL0,        /*  Size in bytes of each L0 cache line.  */
            textCachePortWidth,         /*  Width of the read and write ports in bytes.  */
            textCacheReqQSize,          /*  Size of the request queue size.  */
            textCacheInReqSizeL0,       /*  Size of the input request queue for L0 cache.  */
            4,                          /*  Number of banks in the texture cache.  */
            stampFragments,             /*  Accesses supported per bank and cycle.  */
            4,                          /*  Width in bytes of each texture cache bank.  */
            textCacheMaxMiss,           /*  Number of misses supported per cycle.  */
            textCacheDecomprLatency,    /*  Compressed texture line decompression latency.  */
            textCacheWaysL1,            /*  Number of ways in the L1 cache.  */
            textCacheLinesL1,           /*  Number of lines (per way) in the L1 cache.  */
            textCacheLineSizeL1,        /*  Size in bytes of each L1 cache line.  */
            textCacheInReqSizeL1,       /*  Size of the input request queue for L1 cache.  */
            postfix                     /*  Postfix for the cache statistics.  */
            );

        textCache = textCacheL2;
    }

    /*  Allocate texture request queue.  */
    textRequestsQ = new TextureRequest*[requestQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (textRequestsQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate texture request queue.");
    )

    /*  Allocate texture access queue.  */
    texAccessQ = new TextureAccess*[accessQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (texAccessQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate texture access queue.");
    )

    /*  Allocate texture access request queue.  */
    texAccessReqQ = new TextureRequest*[accessQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (texAccessReqQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate texture access request queue.");
    )

    /*  Allocate free texture access entry queue.  */
    freeTexAccessQ = new u32bit[accessQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (freeTexAccessQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate texture access free entries queue.");
    )

    /*  Allocate address calculation texture access entry queue.  */
    calcTexAccessQ = new u32bit[accessQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (calcTexAccessQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate address calculation queue.");
    )

    /*  Allocate fetch texture access entry queue.  */
    fetchTexAccessQ = new u32bit[accessQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (fetchTexAccessQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate texture access fetch entries queue.");
    )
    /*  Allocate texture result queue.  */
    textResultQ = new TextureResult*[resultQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (textResultQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate texture result queue.");
    )

    /*  Allocate ready read queue.  */
    readyReadQ = new TrilinearElement[accessQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (readyReadQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate ready read queue.");
    )

    /*  Allocate wait read window.  */
    waitReadW = new TrilinearElement[waitReadWindowSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (waitReadW == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate wait read window.");
    )

    /*  Allocate free wait read entries queue.  */
    freeWaitReadQ = new u32bit[waitReadWindowSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (freeWaitReadQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate wait read window free entries queue.");
    )

    /*  Allocate trilinear elements waiting in the wait read window queue.  */
    waitReadQ = new u32bit[waitReadWindowSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (waitReadQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate wait read queue.");
    )

    /*  Allocate trilinear filter queue.  */
    filterQ = new TrilinearElement[accessQueueSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (filterQ == NULL)
            panic("TextureUnit", "TextureUnit", "Unable to allocate trilinear filter queue.");
    )

    /*  Set initial texture unit state.  */
    txState = TU_READY;

    /*  Set shader state to reset.  */
    state = SH_RESET;

    /*  Initial state.  */
    pendingMoveToReady = 0;
}

TextureUnit::~TextureUnit()
{
    GPU_TEX_TRACE(
        texTrace.close();
        texAddrTrace.close();
    )
}

/*  Clock function for Texture Unit Box.  Drives the time of the simulation.  */
void TextureUnit::clock(u64bit cycle)
{
    TextureRequest *txRequest;
    ShaderCommand *command;
    MemoryTransaction *memTrans;
    bool lineFilled;
    u64bit filledLineTag;
    bool allReadsReady;
    u32bit i, j, k;
    u32bit waitAccess;
    u32bit waitTrilinear;
    u32bit moveWait;
    u32bit auxWaitReads;


    /*  Process new commands from the Command Processor.
        Only one command per cycle can be recieved.  The Command
        Processor shouldn't send commands if the Texture Unit is busy.  */

    GPU_DEBUG_BOX(
        printf("%s >> Clock %lld.\n", getName(), cycle);
    )

    /*  Receive state from Memory Controller.  */
    while(memDataSignal->read(cycle, (DynamicObject *&) memTrans))
        processMemoryTransaction(cycle, memTrans);

    /*  Give clock signal to the texture cache.  */
    memTrans = textCache->update(cycle, memoryState, lineFilled, filledLineTag);

    /*  Check if a memory transaction was generated.  */
    if (memTrans != NULL)
    {
        /*  Copy cookies from the last DRAW command from the
            Command Processor.  */
        //memTrans->copyParentCookies(*lastRSCommand);

        /*  Add a cookie level.  */
        //memTrans->addCookie();

        /*  Send memory transaction to the Memory Controller.  */
        memRequestSignal->write(cycle, memTrans);

        /*  Update statistics.  */
        readTrans->inc();
    }

    /*  Check if a line was filled in the texture cache.  */
    if (lineFilled || (pendingMoveToReady > 0))
    {
        GPU_DEBUG_BOX(
            printf("%s %lld => Received line filled for tag %llx.\n", getName(), cycle, filledLineTag);
        )

        /*  Reset pending moves to ready read queue counter.  */
        pendingMoveToReady = 0;

        /*  Search the wait read window and update ready flags.  */
        for(moveWait = 0, auxWaitReads = numWaitReads, i = 0; i < auxWaitReads; i++)
        {
            /*  Get pointers to waiting texture access and trilinear element.  */
            waitAccess = waitReadW[waitReadQ[i]].access;
            waitTrilinear = waitReadW[waitReadQ[i]].trilinear;

            /*  Check and update ready flags for the trilinear element.  */
            for(allReadsReady = true, j = 0; j < stampFragments; j++)
            {
                for(k = 0; k < (texAccessQ[waitAccess]->trilinear[waitTrilinear]->texelsLoop[j] *
                    texAccessQ[waitAccess]->trilinear[waitTrilinear]->loops[j]); k++)
                {
                    /*  Check tag being waited.  */
                    if (lineFilled && (texAccessQ[waitAccess]->trilinear[waitTrilinear]->tag[j][k] == filledLineTag))
                    {
//printf("TU %lld => Awakening access %d trilinear %d frag %d texel %d stored at position %d (order %d)\n", cycle,
//waitAccess, waitTrilinear, j, k, waitReadQ[i], i);

                        texAccessQ[waitAccess]->trilinear[waitTrilinear]->ready[j][k] = true;
                    }

                    /*  Check if texel is ready to be read.  */
                    allReadsReady = allReadsReady && texAccessQ[waitAccess]->trilinear[waitTrilinear]->ready[j][k];
                }
            }

            /*  Check if trilinear element can be awaken.  */
            if (allReadsReady)
            {
                /*  Check if the ready read queue has a free entry.  */
                if (numFreeReadyReads > 0)
                {
                    /*  Move processing element to the ready read queue.  */
                    readyReadQ[nextFreeReadyRead].access = waitAccess;
                    readyReadQ[nextFreeReadyRead].trilinear = waitTrilinear;

                    GPU_DEBUG_BOX(
                        printf("%s => Moving trilinear element (%d, %d) from wait read window %d (order %d) to ready read queue %d\n",
                            getName(), waitAccess, waitTrilinear, waitReadQ[i], i, nextFreeReadyRead);
                    )

                    /*  Add wait read entry to the free wait read entry queue.  */
                    freeWaitReadQ[lastFreeWaitRead] = waitReadQ[i];

                    /*  Update pointer to the next free ready read queue entry.  */
                    nextFreeReadyRead = GPU_MOD(nextFreeReadyRead + 1, accessQueueSize);

                    /*  Updata number of free ready read queue entries.  */
                    numFreeReadyReads--;

                    /*  Update number of ready read queue entries.  */
                    numReadyReads++;

                    /*  Update pointer to the last entry in the free wait read window entry queue.  */
                    lastFreeWaitRead = GPU_MOD(lastFreeWaitRead + 1, waitReadWindowSize);

                    /*  Update number of free wait read entries.  */
                    numFreeWaitReads++;

                    /*  Update number of wait read entries.  */
                    numWaitReads--;

                    /*  Update number of positions to move back the next wait read element.  */
                    moveWait++;
                }
                else
                {
                    /*  Update the number of pending of moves to ready read queue.  */
                    pendingMoveToReady++;

                    /*  Check if the waiting trilinear element must be moved.  */
                    if (moveWait > 0)
                    {
                        waitReadQ[GPU_PMOD(i - moveWait, waitReadWindowSize)] = waitReadQ[i];
                    }
                }
            }
            else
            {
                /*  Check if the waiting trilinear element must be moved.  */
                if (moveWait > 0)
                {
                    waitReadQ[GPU_PMOD(i - moveWait, waitReadWindowSize)] = waitReadQ[i];
                }
            }
        }

        /*  Awake already fetched texels in the current texture access being fetch.  */
        if (numFetchTexAcc > 0)
        {
            /*  Get the current texture access being fetched.  */
            waitAccess = fetchTexAccessQ[nextFetchTexAcc];

            /*  Search for fetched texels with the received tag.  */
            for(i = 0; i < texAccessQ[waitAccess]->anisoSamples; i++)
            {
                for(j = 0; j < stampFragments; j++)
                {
                    for(k = 0; k < (texAccessQ[waitAccess]->trilinear[i]->texelsLoop[j] *
                        texAccessQ[waitAccess]->trilinear[i]->loops[j]); k++)
                    {
                        /*  Check tag being waited.  */
                        if (lineFilled && texAccessQ[waitAccess]->trilinear[i]->fetched[j][k] &&
                            (texAccessQ[waitAccess]->trilinear[i]->tag[j][k] == filledLineTag))
                        {
//printf("TU %lld => Awakening access %d trilinear %d frag %d texel %d in fetch stage.\n", cycle,
//waitAccess, i, j, k);

                            texAccessQ[waitAccess]->trilinear[i]->ready[j][k] = true;
                        }
                    }
                }
            }
        }
    }

/*if (numFreeTexAcc < accessQueueSize)
{
printf("FreeTexAcc[%d]: ", numFreeTexAcc);
for(i = 0; i < numFreeTexAcc; i++)
printf(" %d, ", freeTexAccessQ[GPU_MOD(nextFreeTexAcc + i, accessQueueSize)]);
printf("\n");
printf("CalcTexAcc[%d]: ", numCalcTexAcc);
for(i = 0; i < numCalcTexAcc; i++)
printf(" %d, ", calcTexAccessQ[GPU_MOD(nextCalcTexAcc + i, accessQueueSize)]);
printf("\n");
printf("FetchTexAcc[%d]: ", numFetchTexAcc);
for(i = 0; i < numFetchTexAcc; i++)
printf(" %d, ", fetchTexAccessQ[GPU_MOD(nextFetchTexAcc + i, accessQueueSize)]);
printf("\n");
}
if (GPU_MOD(cycle, 10000) == 0)
{
printf("TU %lld => numFreeReadyReads %d numReadyReads %d numFreeWaitReads %d numWaitReads %d\n",
cycle, numFreeReadyReads, numReadyReads, numFreeWaitReads, numWaitReads);
printf("TU %lld => numFreeFilters %d numToFilter %d numFiltered %d\n",
cycle, numFreeFilters, numToFilter, numFiltered);
}*/

    /*  Check texture requests from the Shader Decode Execute box.  */
    while (textRequestSignal->read(cycle, (DynamicObject *&) txRequest))
        processTextureRequest(txRequest);

    /*  Check commands from the Command Processor.  */
    if (commProcSignal->read(cycle, (DynamicObject *&) command))
        processShaderCommand(cycle, command);

    /*  Check if shader state is reset.  */
    if (state == SH_RESET)
    {
        GPU_DEBUG_BOX( printf("%s => RESET state.\n", getName()); )

        /*  Set default register values.  */
        for (i = 0; i < MAX_TEXTURES; i++)
        {
            textureEnabled[i] = FALSE;
            textureMode[i] = GPU_TEXTURE2D;
            textureWidth[i] = 0;
            textureHeight[i] = 0;
            textureDepth[i] = 0;
            textureWidth2[i] = 0;
            textureHeight2[i] = 0;
            textureDepth2[i] = 0;
            textureBorder[i] = 0;
            textureFormat[i] = GPU_RGBA8888;
            textureReverse[i] = false;
            textD3D9ColorConv[i] = false;
            textD3D9VInvert[i] = false;
            textureCompr[i] = GPU_NO_TEXTURE_COMPRESSION;
            textureBlocking[i] = GPU_TXBLOCK_TEXTURE;
            textBorderColor[i][0] = 0.0f;
            textBorderColor[i][1] = 0.0f;
            textBorderColor[i][2] = 0.0f;
            textBorderColor[i][3] = 1.0f;
            textureWrapS[i] = GPU_TEXT_CLAMP;
            textureWrapT[i] = GPU_TEXT_CLAMP;
            textureWrapR[i] = GPU_TEXT_CLAMP;
            textureNonNormalized[i] = false;
            textureMinFilter[i] = GPU_NEAREST;
            textureMagFilter[i] = GPU_NEAREST;
            textureEnableComparison[i] = false;
            textureComparisonFunction[i] = GPU_LEQUAL;
            textureSRGB[i] = false;
            textureMinLOD[i] = 0.0f;
            textureMaxLOD[i] = 12.0f;
            textureLODWays[i] = 0.0f;
            textureMinLevel[i] = 0;
            textureMaxLevel[i] = 12;
            textureUnitLODWays[i] = 0.0f;
            maxAnisotropy[i] = 1;
        }

        //  Set default values to vertex attribute and stream registers.
        for(i = 0; i < MAX_VERTEX_ATTRIBUTES; i++)
        {
            attributeMap[i] = ST_INACTIVE_ATTRIBUTE;
            attrDefValue[i][0] = 0.0f;
            attrDefValue[i][1] = 0.0f;
            attrDefValue[i][2] = 0.0f;
            attrDefValue[i][3] = 1.0f;
        }
        for(i = 0; i < MAX_STREAM_BUFFERS; i++)
        {
            streamAddress[i] = 0;
            streamStride[i] = 0;
            streamData[i] = SD_U32BIT;
            streamElements[i] = 0;
            d3d9ColorStream[i] = false;
        }
        
        /*  Reset texture emulator.  */
        txEmul.reset();

        /*  Set default state.  */

        /*  Reset the number of offered texture tickets to the shader.  */
        textureTickets = 0;

        /*  Initialize request queue counters and pointers.  */
        nextRequest = 0;
        nextFreeRequest = 0;
        requests = 0;
        freeRequests = requestQueueSize;

        /*  Initialize texture access queues, counters and pointers.  */
        for(i = 0; i < accessQueueSize; i++)
        {
            freeTexAccessQ[i] = calcTexAccessQ[i] = fetchTexAccessQ[i] = i;
        }
        nextFreeTexAcc = lastFreeTexAcc = 0;
        numFreeTexAcc = accessQueueSize;
        nextCalcTexAcc = lastCalcTexAcc = 0;
        numCalcTexAcc = 0;
        nextFetchTexAcc = lastFetchTexAcc = 0;
        numFetchTexAcc = 0;

        /*  Initialize texture result counters and pointers.  */
        nextResult = 0;
        nextFreeResult = 0;
        numResults = 0;
        freeResults = resultQueueSize;

        /*  Initialize trilinear ready read queue counters and pointers.  */
        nextFreeReadyRead = nextReadyRead = 0;
        numFreeReadyReads = accessQueueSize;
        numReadyReads = 0;

        /*  Initialize trilinear wait read window queues, counters and pointers.  */
        for(i = 0; i < waitReadWindowSize; i++)
        {
            freeWaitReadQ[i] = waitReadQ[i] = i;
        }
        nextFreeWaitRead = lastFreeWaitRead = 0;
        numFreeWaitReads = waitReadWindowSize;
        numWaitReads = 0;

        /*  Initialize trilinear filter queue counters and pointers.  */
        nextFreeFilter = nextToFilter = nextFiltered = 0;
        numFreeFilters = accessQueueSize;
        numToFilter = numFiltered = 0;

        /*  Initialize ALU cycle counters.  */
        addressALUCycles = 0;
        filterCycles = 0;

        /*  Reset counter with the number of pending moves to the ready read queue.  */
        pendingMoveToReady = 0;

        /*  Reset the texture cache.  */
        textCache->reset();

        /*  Change state to ready shader state.  */
        state = SH_READY;
    }
    else
    {
        GPU_DEBUG_BOX( printf("%s => READY state.\n", getName()); )

        /*  Send back texture accesses result to the Shader Decode Execute unit.  */
        sendTextureSample(cycle);

        /*  Filter texture samples.  */
        filter(cycle);

        /*  Read texture data.  */
        read(cycle);

        /*  Fetch texture data.  */
        fetch(cycle);

        /*  Calculate sample addresses.  */
        calculateAddress(cycle);

        /*  Generate texture accesses.  */
        genTextureAccess();
    }

//if (cycle > 71000)
//printf("TU (%p) %lld => textureTickets %d requests %d requestQueueSize %d\n",
//this, cycle, textureTickets, requests, requestQueueSize);

    /*  Update texture unit state.  */
    if ((requests + textureTickets) < requestQueueSize)
    {

        /*  Update Texture Unit state signal.  */
        readySignal->write(cycle, new TextureUnitStateInfo(TU_READY));

        /*  Update number of offered texture tickets.  */
        textureTickets++;
    }

    /*  Send shader state to Command Processor.  */

    /*  Update statistics.  */
    readyReadLevel->inc(numReadyReads);
    waitReadLevel->inc(numWaitReads);
    resultLevel->inc(numResults);
    accessLevel->inc(numCalcTexAcc + numFetchTexAcc);
    requestLevel->inc(requests);
}

/*  Calculates texel addresses.  */
void TextureUnit::calculateAddress(u64bit cycle)
{
    u32bit frag;
    u32bit filterLoops;
    FilterOperation *aluOp;
    u32bit addressAccess;

    /*  Check if there are accesses in the texture access queue waiting for their texel address to be calculated.
        Check that the address ALU can receive a new operation.
        Check that the trilinear fetch queue has an empty entry. */
    if ((numCalcTexAcc > 0) && (addressALUCycles == 0))
    {

        /*  Get pointer to the next texture access for which to calculate addresses.  */
        addressAccess = calcTexAccessQ[nextCalcTexAcc];

        GPU_DEBUG_BOX(
            printf("%s => Starting address calculation of trilinear access %d for texture access at %d\n",
                getName(), texAccessQ[addressAccess]->currentAnisoSample, addressAccess);
        )

        /*  Calculate address for the next trilinear access.  */
        txEmul.calculateAddress(texAccessQ[addressAccess]);

        /*  Search the maximum loop for the fragments in the stamp.  */
        for(frag = 0, filterLoops = 0; frag < stampFragments; frag++)
        {
            filterLoops = GPU_MAX(filterLoops,
                texAccessQ[addressAccess]->trilinear[(texAccessQ[addressAccess]->currentAnisoSample - 1)]->loops[frag]);
        }

        /*  Create address ALU operation.  */
        aluOp = new FilterOperation(addressAccess);

        /*  Copy cookies from the original request.  */
        aluOp->copyParentCookies(*texAccessReqQ[addressAccess]);

        /*  Send address ALU operation through the address ALU pipeline.  */
        addressALUInput->write(cycle, aluOp, addressALULatency + filterLoops);

        /*  Set cycles to wait until the address ALU is free.  */
        addressALUCycles = filterLoops;

        /*  Update pointer to the next trilinear sample for which to calculate the address.  */
        texAccessQ[addressAccess]->currentAnisoSample++;

        /*  Check if all the trilinear samples for the texture address processed.  */
        if (texAccessQ[addressAccess]->currentAnisoSample > texAccessQ[addressAccess]->anisoSamples)
        {
            /*  Add texture access to the waiting fetch queue.  */
            fetchTexAccessQ[lastFetchTexAcc] = addressAccess;

            /*  Update number of texture accesses for which addresses must be calculated.  */
            numCalcTexAcc--;

            /*  Update number of texture accesses waiting to be fetched.  */
            numFetchTexAcc++;

            /*  Update pointer to the next texture access entry for which to calculate the texel addresses.  */
            nextCalcTexAcc = GPU_MOD(nextCalcTexAcc + 1, accessQueueSize);

            /*  Update pointer to the next texture access waiting to be fetched.  */
            lastFetchTexAcc = GPU_MOD(lastFetchTexAcc + 1, accessQueueSize);

            GPU_DEBUG_BOX(
                printf("%s => Texture Address calculated : \n", getName());
                printTextureAccessInfo(texAccessQ[addressAccess]);
            )
            
            /*  Update statistics.  */
            addrCalcFinished->inc();
            bilinearSamples->inc(filterLoops * texAccessQ[addressAccess]->anisoSamples);
            anisoRatio->inc(texAccessQ[addressAccess]->anisoSamples);
            if (maxAnisotropy[texAccessQ[addressAccess]->textUnit] == 1)
                anisoRatioHisto[0]->inc();
            else
                anisoRatioHisto[texAccessQ[addressAccess]->anisoSamples]->inc();
            bilinearsHisto[(filterLoops * texAccessQ[addressAccess]->anisoSamples) - 1]->inc();
            magnifiedPixels->inc(texAccessQ[addressAccess]->magnified);
            for(u32bit f = 0; f < STAMP_FRAGMENTS; f++)
            {
                if (texAccessQ[addressAccess]->filter[f] == GPU_NEAREST)
                    pointSampled->inc();
            }
            if (textureMode[texAccessQ[addressAccess]->textUnit] != GPU_TEXTURE3D)
                mipsSampled[filterLoops - 1]->inc();
        }
    }

    /*  Read address ALU operations at the end of the address ALU pipeline.  */
    if (addressALUOutput->read(cycle, (DynamicObject *&) aluOp))
    {

        GPU_DEBUG_BOX(
            printf("%s => End of trilinear sample address calculation.\n", getName());
        )

        /*  Get texture access queue entry for the address ALU operation.  */
        addressAccess = aluOp->getFilterEntry();

        /*  Check if all the trilinear samples for the texture address processed.  */
        if (texAccessQ[addressAccess]->currentAnisoSample > texAccessQ[addressAccess]->anisoSamples)
        {
            /*  Mark that all the texel addresses have been calculated.  */
            texAccessQ[addressAccess]->addressCalculated = true;
        }

        /*  Delete filter operation object.  */
        delete aluOp;
    }

    /*  Update cycles to wait.  */
    if (addressALUCycles > 0)
    {
        addressALUCycles--;

        /*  Update statistics.  */
        addrALUBusy->inc();
    }

}


/*  Fetches texture data from the texture cache.  */
void TextureUnit::fetch(u64bit cycle)
{
    u32bit frag;
    u32bit i;
    u32bit start;
    u32bit nextTrilinear;
    bool allFetched;
    bool lastLoop;
    u32bit fetchAccess;
    bool allTexelsReady;
    u32bit waitReadEntry;

    /*  All texels in the current loop have been fetched.  */
    allFetched = TRUE;

    /*  Get next texture access for which to fetch texel addresses.  */
    fetchAccess = fetchTexAccessQ[nextFetchTexAcc];

    /*  Check if there are accesses in the texture access queue waiting to be fetched.  */
    if ((numFetchTexAcc > 0) && (texAccessQ[fetchAccess]->addressCalculated) &&
        (numFreeReadyReads > 0) && (numFreeWaitReads > 0))
    {
        /*  Get pointer to the next trilinear sample for the texture access to fetch.  */
        nextTrilinear =  texAccessQ[fetchAccess]->nextTrilinearFetch;

        GPU_DEBUG_BOX(
            printf("%s => Fetching data for trilinear element %d of texture access at %d\n", getName(), nextTrilinear, fetchAccess);
        )

        /*  For each of the fragments in a stamp.  */
        for(frag = 0; frag < stampFragments; frag++)
        {
            /*  Calculate first texel for this loop.  */
            start = texAccessQ[fetchAccess]->trilinear[nextTrilinear]->fetchLoop[frag] *
                texAccessQ[fetchAccess]->trilinear[nextTrilinear]->texelsLoop[frag];
                
            //  Check if all the texel loops for the fragment were fetched.
            bool fetchTexels = (texAccessQ[fetchAccess]->trilinear[nextTrilinear]->fetchLoop[frag] <
                                texAccessQ[fetchAccess]->trilinear[nextTrilinear]->loops[frag]);

            /*  Fetch as many texels as required.  */
            for(i = 0; fetchTexels && (i < texAccessQ[fetchAccess]->trilinear[nextTrilinear]->texelsLoop[frag]); i++)
            {
                /*  Check if the texel has been already fetched.  */
                if (!texAccessQ[fetchAccess]->trilinear[nextTrilinear]->fetched[frag][start + i])
                {
//if (cycle == 300000)
//    textCache->setDebug(true);
//if (cycle > 300000)
//printf("%s (%lld) => fetchAccess %d nextTrilinear %d frag %d texel %d -> address %llx\n", getName(), cycle, fetchAccess, nextTrilinear,
//frag, start + i, texAccessQ[fetchAccess]->trilinear[nextTrilinear]->address[frag][start + i]);
                    /*  Try to fetch next texel.  */
                    if (textCache->fetch(
                        texAccessQ[fetchAccess]->trilinear[nextTrilinear]->address[frag][start + i],
                        texAccessQ[fetchAccess]->trilinear[nextTrilinear]->way[frag][start + i],
                        texAccessQ[fetchAccess]->trilinear[nextTrilinear]->line[frag][start + i],
                        texAccessQ[fetchAccess]->trilinear[nextTrilinear]->tag[frag][start + i],
                        texAccessQ[fetchAccess]->trilinear[nextTrilinear]->ready[frag][start + i],
                        texAccessReqQ[fetchAccess]
                        ))
                    {
GPU_TEX_TRACE(
    u64bit address64 = texAccessQ[fetchAccess]->trilinear[nextTrilinear]->address[frag][start + i];
    u32bit address32;

    switch(address64 & TEXTURE_ADDRESS_SPACE_MASK)
    {
        case UNCOMPRESSED_TEXTURE_SPACE:

            address32 = (u32bit) (address64 & 0xffffffff);
            break;

        case COMPRESSED_TEXTURE_SPACE_DXT1_RGB:
        case COMPRESSED_TEXTURE_SPACE_DXT1_RGBA:
            address32 = (u32bit) ((address64 >> DXT1_SPACE_SHIFT) & 0xffffffff);
            break;

        case COMPRESSED_TEXTURE_SPACE_DXT3_RGBA:
        case COMPRESSED_TEXTURE_SPACE_DXT5_RGBA:

            address32 = (u32bit) ((address64 >> DXT3_DXT5_SPACE_SHIFT) & 0xffffffff);

            break;

    }
    texAddrTrace << address32 << endl;
)

                        /*  Set texel as fetched.  */
                        texAccessQ[fetchAccess]->trilinear[nextTrilinear]->fetched[frag][start + i] = TRUE;

                        /*  Update statistics.  */
                        fetchOK->inc();
                    }
                    else
                    {
                        /*  Not all texels in the fetch loop could be fetched.  */
                        allFetched = FALSE;

                        /*  Update statistics.  */
                        fetchFail->inc();
                    }
                }
                else
                {
                    /*  Update statistics.  */
                    fetchSkip->inc();
                }
            }
        }

        /*  Check if all texels in the loop were fetched.  */
        if (allFetched)
        {
GPU_TEX_TRACE(
    texAddrTrace << "EndLoop" << endl;
)
            /*  Last loop?  */
            lastLoop = TRUE;

            /*  Update current count loop.  */
            for(frag = 0; frag < stampFragments; frag++)
            {
                //  Check there was a texel loop to be fetched for this fragment.
                if (texAccessQ[fetchAccess]->trilinear[nextTrilinear]->fetchLoop[frag] <
                    texAccessQ[fetchAccess]->trilinear[nextTrilinear]->loops[frag])
                    texAccessQ[fetchAccess]->trilinear[nextTrilinear]->fetchLoop[frag]++;

                /*  Last loop?  */
                lastLoop = lastLoop &&
                    (texAccessQ[fetchAccess]->trilinear[nextTrilinear]->fetchLoop[frag] ==
                        texAccessQ[fetchAccess]->trilinear[nextTrilinear]->loops[frag]);
            }

            /*  Check if all the loops have been finished.  */
            if (lastLoop)
            {
GPU_TEX_TRACE(
    texAddrTrace << "EndTrilinear" << endl;
)
                /*  A whole trilinear element has been fetched.  */

                GPU_DEBUG_BOX(
                    printf("%s => End of fetch for trilinear sample %d for texture access at %d\n",
                        getName(), nextTrilinear, fetchAccess);
                )


                /*  Determine if all the texels in the trilinear element are ready to be read.  */
                for(allTexelsReady = true, frag = 0; (frag < stampFragments) && allTexelsReady; frag++)
                    for(i = 0; (i < (texAccessQ[fetchAccess]->trilinear[nextTrilinear]->texelsLoop[frag] *
                        texAccessQ[fetchAccess]->trilinear[nextTrilinear]->loops[frag])) && allTexelsReady ; i++)
                            allTexelsReady = allTexelsReady && texAccessQ[fetchAccess]->trilinear[nextTrilinear]->ready[frag][i];

                /*  Check if the trilinear element can be read.  */
                if (allTexelsReady)
                {
                    /*  Add trilinear to the trilinear ready read queue.  */
                    readyReadQ[nextFreeReadyRead].access = fetchAccess;
                    readyReadQ[nextFreeReadyRead].trilinear = nextTrilinear;

                    GPU_DEBUG_BOX(
                        printf("%s => Adding trilinear element to the ready read queue entry %d\n",
                            getName(), nextFreeReadyRead);
                    )

                    /*  Update pointer to the next free trilinear ready read entry.  */
                    nextFreeReadyRead = GPU_MOD(nextFreeReadyRead + 1, accessQueueSize);

                    /*  Update number of free entries in the trilinear ready read queue.  */
                    numFreeReadyReads--;

                    /*  Update number of trilinears waiting to be read in the ready read queue.  */
                    numReadyReads++;
                }
                else
                {
                    /*  Get the next free entry in the wait read window.  */
                    waitReadEntry = freeWaitReadQ[nextFreeWaitRead];

                    GPU_DEBUG_BOX(
                        printf("%s => Adding trilinear element to be wait read window entry %d (order %d)\n",
                            getName(), waitReadEntry, numWaitReads);
                    )

                    /*  Add trilinear to the trilinear wait read window.  */
                    waitReadW[waitReadEntry].access = fetchAccess;
                    waitReadW[waitReadEntry].trilinear = nextTrilinear;
                    waitReadQ[numWaitReads] = waitReadEntry;

                    /*  Update pointer to the next free wait read window free entry queue.  */
                    nextFreeWaitRead = GPU_MOD(nextFreeWaitRead + 1, waitReadWindowSize);

                    /*  Update number of free entries in the wait read window.  */
                    numFreeWaitReads--;

                    /*  Update number of trilinear elements waiting in the wait read window.  */
                    numWaitReads++;
                }

                /*  Update pointer for the next trilinear sample in the texture access to fetch.  */
                texAccessQ[fetchAccess]->nextTrilinearFetch++;

                /*  Determine if all trilinear samples fetched for the texture access.  */
                if (texAccessQ[fetchAccess]->nextTrilinearFetch == texAccessQ[fetchAccess]->anisoSamples)
                {
GPU_TEX_TRACE(
    texAddrTrace << "EndTexAccess" << endl;
)
                    GPU_DEBUG_BOX(
                        printf("%s => End of fetch for for texture access at %d\n", getName(), fetchAccess);
                    )

                    /*  Update number of texture access waiting to be fetched.  */
                    numFetchTexAcc--;

                    /*  Update pointer to the next texture access to be fetched in the queue.  */
                    nextFetchTexAcc = GPU_MOD(nextFetchTexAcc + 1, accessQueueSize);
                }
            }
        }
    }
    else
    {
        /*  Update stall statistics.  */
        if (numFetchTexAcc > 0)
        {
            if  (!texAccessQ[fetchAccess]->addressCalculated)
            {
                fetchStallAddress->inc();
            }
            else
            {
                if (numFreeReadyReads == 0)
                {
                    fetchStallReadyRead->inc();
                }
                else
                {
                    if(numFreeWaitReads == 0)
                    {
                        fetchStallWaitRead->inc();
                    }
                    else
                    {
                        panic("TextureUnit", "fetch", "Impossible!");
                    }
                }
            }
        }
        else
        {
            fetchStallFetch->inc();
        }
    }
}

/*  Performs a texel read loop for all the fragments in a stamp.  */
void TextureUnit::read(u64bit cycle)
{
    u32bit frag;
    u32bit i;
    u32bit start;
    u32bit nextTrilinear;
    u8bit data[MAX_TEXEL_DATA];
    bool allRead;
    bool lastLoop;
    u32bit readAccess;

    /*  All texels in the current loop have been read.  */
    allRead = TRUE;

    /*  Check if there are accesses in the texture access queue waiting to be read.  */
    if ((numReadyReads > 0) && (numFreeFilters > 0))
    {
        /*  Get pointer to the next texture access to read.  */
        readAccess = readyReadQ[nextReadyRead].access;

        /*  Get pointer to the next trilinear sample to read in the texture access.  */
        nextTrilinear = readyReadQ[nextReadyRead].trilinear;

        GPU_DEBUG_BOX(
            printf("%s => Reading data for trilinear element %d from texture access at %d.\n",
                getName(), nextTrilinear, readAccess);
        )

        /*  For each of the fragments in a stamp.  */
        for(frag = 0; frag < stampFragments; frag++)
        {
            /*  Calculate first texel for this loop.  */
            start = texAccessQ[readAccess]->trilinear[nextTrilinear]->readLoop[frag] *
                texAccessQ[readAccess]->trilinear[nextTrilinear]->texelsLoop[frag];

            //  Check if all the texel loops for the fragment were fetched.
            bool readTexels = (texAccessQ[readAccess]->trilinear[nextTrilinear]->readLoop[frag] <
                                texAccessQ[readAccess]->trilinear[nextTrilinear]->loops[frag]);

            /*  Read as many texels as required.  */
            for(i = 0; readTexels && (i < texAccessQ[readAccess]->trilinear[nextTrilinear]->texelsLoop[frag]); i++)
            {
                /*  Check if the texel has been fetched.  */
                if (texAccessQ[readAccess]->trilinear[nextTrilinear]->fetched[frag][start + i])
                {
                    /*  Check if the texel has been already read.  */
                    if (!texAccessQ[readAccess]->trilinear[nextTrilinear]->read[frag][start + i])
                    {
//if (cycle > 300000)
//printf("%s %lld => readAccess %d nextTrilinear %d frag %d texel %d => address %llx way %d line %d\n",
//getName(), cycle, readAccess, nextTrilinear, frag, start + i,
//texAccessQ[readAccess]->trilinear[nextTrilinear]->address[frag][start + i],
//texAccessQ[readAccess]->trilinear[nextTrilinear]->way[frag][start + i],
//texAccessQ[readAccess]->trilinear[nextTrilinear]->line[frag][start + i]);
                        /*  Try to fetch next texel.  */
                        if (textCache->read(
                            texAccessQ[readAccess]->trilinear[nextTrilinear]->address[frag][start + i],
                            texAccessQ[readAccess]->trilinear[nextTrilinear]->way[frag][start + i],
                            texAccessQ[readAccess]->trilinear[nextTrilinear]->line[frag][start + i],
                            texAccessQ[readAccess]->texelSize[frag],
                            data
                            ))
                        {
                            /*  Convert and store read data.  */
                            txEmul.convertFormat(*texAccessQ[readAccess], nextTrilinear, frag, start + i, data);

                            /*  Set texel as read.  */
                            texAccessQ[readAccess]->trilinear[nextTrilinear]->read[frag][start + i] = TRUE;

                            /*  Unreserve the cache line.  */
                            textCache->unreserve(texAccessQ[readAccess]->trilinear[nextTrilinear]->way[frag][start + i],
                                texAccessQ[readAccess]->trilinear[nextTrilinear]->line[frag][start + i]);

                            /*  Update statistics.  */
                            readOK->inc();
                        }
                        else
                        {
                            //panic("TextureUnit", "read", "Ready read address couldn't be read.");
                            /*  Not all texels in the current loop could be read.  */
                            allRead = FALSE;

                            /*  Update statistics.  */
                            readFail->inc();
                        }
                    }
                }
                else
                {
                    panic("TextureUnit", "read", "All texels should be already fetched.");
                    /*  Not all texels in the current loop could be read.  */
                    //allRead = FALSE;

                    /*  Update statistics.  */
                    //readFail->inc();
                }
            }
        }

        /*  Check if all texels in the loop were read.  */
        if (allRead)
        {
            /*  Last loop?  */
            lastLoop = TRUE;

            /*  Update current count loop.  */
            for(frag = 0; frag < stampFragments; frag++)
            {
                //  Check if not all texel loops for the fragment were read.
                if (texAccessQ[readAccess]->trilinear[nextTrilinear]->readLoop[frag] < 
                    texAccessQ[readAccess]->trilinear[nextTrilinear]->loops[frag])
                    texAccessQ[readAccess]->trilinear[nextTrilinear]->readLoop[frag]++;

                /*  Last loop?  */
                lastLoop = lastLoop &&
                    (texAccessQ[readAccess]->trilinear[nextTrilinear]->readLoop[frag] ==
                        texAccessQ[readAccess]->trilinear[nextTrilinear]->loops[frag]);
            }

            /*  Check if all the loops have been finished.  */
            if (lastLoop)
            {
                GPU_DEBUG_BOX(
                    printf("%s => End of read for trilinear sample %d for texture access at %d\n",
                        getName(), nextTrilinear, readAccess);
                )

                /*  Whole trilinear element has been read.  */

                /*  Add trilinear element to the trilinear filter queue.  */
                filterQ[nextFreeFilter].access = readAccess;
                filterQ[nextFreeFilter].trilinear = nextTrilinear;

                /*  Update pointer to the next trilinear ready read queue entry.  */
                nextReadyRead = GPU_MOD(nextReadyRead + 1, accessQueueSize);

                /*  Update number of trilinear elements waiting to be read.  */
                numReadyReads--;

                /*  Update free ready read queue  entries counter.  */
                numFreeReadyReads++;

                /*  Update pointer to the next free entry in the trilinear filter queue.  */
                nextFreeFilter = GPU_MOD(nextFreeFilter + 1, accessQueueSize);

                /*  Update number of trilinear elements waiting to be filtered.  */
                numToFilter++;

                /*  Update free trilinear filter queue entries counter.  */
                numFreeFilters--;
            }
        }
    }
}

/*  Filters a stamp.  */
void TextureUnit::filter(u64bit cycle)
{
    u32bit frag;
    u32bit filterLoops;
    FilterOperation *filterOp;
    TextureResult *txRes;
    u32bit nextTrilinear;
    u32bit filterAccess;

    /*  Check if there are filtered trilinear elements in the queue.  */
    if (numFiltered > 0)
    {
        filterAccess = filterQ[nextFiltered].access;

        /*  Check if texture access fully sampled.  */
        if (texAccessQ[filterAccess]->trilinearFiltered == texAccessQ[filterAccess]->anisoSamples)
        {
            /*  Texture access fully sampled.  */

            /*  Check there are free entries in the result queue.  */
            if (freeResults > 0)
            {
                GPU_DEBUG_BOX(
                    printf("%s => Adding texture access %d as texture result at %d\n", getName(), filterAccess, nextFreeResult);
                )

                /*  Create texture result object.  */
                txRes = new TextureResult(texAccessQ[filterAccess]->accessID, texAccessQ[filterAccess]->sample,
                    stampFragments, texAccessQ[filterAccess]->cycle);

                /*  Copy cookies from the original texture request object.  */
                txRes->copyParentCookies(*texAccessReqQ[filterAccess]);

                /*  Add texture result to the queue.  */
                textResultQ[nextFreeResult] = txRes;

                /*  Update number of texture results.  */
                numResults++;

                /*  Update pointer to the next free texture result queue entry.  */
                nextFreeResult = GPU_MOD(nextFreeResult + 1, resultQueueSize);

                /*  Update number of free texture result queue entries.  */
                freeResults--;

                /*  Update number of filtered trilinear elements.  */
                numFiltered--;

                /*  Update number of free entries in the trilinear filter queue.  */
                numFreeFilters++;

                /*  Update pointer to the next filtered trilinear element.  */
                nextFiltered = GPU_MOD(nextFiltered + 1, accessQueueSize);

                /*  Add texture access entry to the free texture access entry queue.  */
                freeTexAccessQ[lastFreeTexAcc] = filterAccess;

                /*  Update number of free texture access queue entries.  */
                numFreeTexAcc++;

                /*  Update pointer to the next free entry in the free texture access queue.  */
                lastFreeTexAcc = GPU_MOD(lastFreeTexAcc + 1, accessQueueSize);

                /*  Delete texture access object.  */
                delete texAccessQ[filterAccess];

                /*  Delete texture request object.  */
                delete texAccessReqQ[filterAccess];
            }
        }
        else
        {
            /*  Update number of filtered trilinear elements.  */
            numFiltered--;

            /*  Update number of free entries in the trilinear filter queue.  */
            numFreeFilters++;

            /*  Update pointer to the next filtered trilinear element.  */
            nextFiltered = GPU_MOD(nextFiltered + 1, accessQueueSize);
        }
    }

    /*  Check if there are trilinear elements waiting to be filtered.  */
    if ((numToFilter > 0) && (filterCycles == 0))
    {
        /*  Get trilinear element texture access queue pointer and trilinear pointer.  */
        filterAccess = filterQ[nextToFilter].access;
        nextTrilinear = filterQ[nextToFilter].trilinear;

        /*  Search the maximum loop for the fragments in the stamp.  */
        for(frag = 0, filterLoops = 0; frag < stampFragments; frag++)
        {
            filterLoops = GPU_MAX(filterLoops,
                texAccessQ[filterAccess]->trilinear[nextTrilinear]->loops[frag]);
        }

        GPU_DEBUG_BOX(
            printf("%s (%lld) => Starting filtering of texture access at %d trilinear element %d of %d\n",
                getName(), cycle, filterAccess, nextTrilinear, texAccessQ[filterAccess]->anisoSamples);
        )

        /*  Filter all the stamp fragment texels.  */
        txEmul.filter(*texAccessQ[filterAccess], nextTrilinear);

        /*  Create filter operation.  */
        filterOp = new FilterOperation(filterAccess);

        /*  Copy cookies from the original request.  */
        filterOp->copyParentCookies(*texAccessReqQ[filterAccess]);

        /*  Send filter operation through the filter pipeline.  */
        filterInput->write(cycle, filterOp, filterLatency + filterLoops);

        /*  Set cycles to wait until the filter pipeline is free.  */
        filterCycles = filterLoops;

        /*  Update number of trilinear elements to filter.  */
        numToFilter--;

        /*  Update pointer to the next trilinear element to filter.  */
        nextToFilter = GPU_MOD(nextToFilter + 1, accessQueueSize);
    }

    /*  Read filter operations at the end of the filter pipeline.  */
    if (filterOutput->read(cycle, (DynamicObject *&) filterOp))
    {

        GPU_DEBUG_BOX(
            printf("%s => End of texture access filtering.\n", getName());
        )

        /*  Update number of trilinear elements filtered.  */
        numFiltered++;

        /*  Update pointer to next trilinear sample sampled.  */
        texAccessQ[filterOp->getFilterEntry()]->trilinearFiltered++;

        /*  Delete filter operation object.  */
        delete filterOp;
    }

    /*  Update cycles to wait.  */
    if (filterCycles > 0)
    {
        filterCycles--;

        /*  Update statistics.  */
        filterALUBusy->inc();
    }
}

/*  Send texture samples back to the shader.  */
void TextureUnit::sendTextureSample(u64bit cycle)
{
    /*  Check if there is a texture access to be sent.  */
    if (numResults > 0)
    {
        GPU_DEBUG_BOX(
            printf("%s (%lld) => Sending texture result at %d to Shader\n", getName(), cycle, nextResult);
        )

        GPU_DEBUG_BOX(
            QuadFloat *samples;
            samples = textResultQ[nextResult]->getTextSamples();
            printf("%s => Sample TxID %d :\n", getName(), textResultQ[nextResult]->getTextAccessID());
            printf("%s => { %f, %f, %f, %f }\n", getName(), samples[0][0], samples[0][1], samples[0][2], samples[0][3]);
            printf("%s => { %f, %f, %f, %f }\n", getName(), samples[1][0], samples[1][1], samples[1][2], samples[1][3]);
            printf("%s => { %f, %f, %f, %f }\n", getName(), samples[2][0], samples[2][1], samples[2][2], samples[2][3]);
            printf("%s => { %f, %f, %f, %f }\n", getName(), samples[3][0], samples[3][1], samples[3][2], samples[3][3]);
        )

        /*  Send texture samples back to the shader.  */
        textDataSignal->write(cycle, textResultQ[nextResult]);

        /*  Update statistics.  */
        results->inc();
        textResLat->inc(cycle - textResultQ[nextResult]->getStartCycle());

        /*  Update pointer to the next texture result.  */
        nextResult = GPU_MOD(nextResult + 1, resultQueueSize);

        /*  Update number of texture results.  */
        numResults--;

        /*  Update free texture result queue entries counter.  */
        freeResults++;
    }
}

/*  Process a new texture request.  */
void TextureUnit::processTextureRequest(TextureRequest *txRequest)
{
    /*  Check if the queue has free entries.  */
    GPU_ASSERT(
        if (freeRequests == 0)
            panic("TextureUnit", "processTextureRequest", "No free entry in the texture request queue.");
    )

    GPU_DEBUG_BOX(
        printf("%s => Adding texture request at %d\n", getName(), nextFreeRequest);
    )

    /*  Add request to the queue.  */
    textRequestsQ[nextFreeRequest] = txRequest;

    /*  Update number of free requests.  */
    freeRequests--;

    /*  Update number of requests waiting in the texture request queue.  */
    requests++;

    /*  Update number of offered texture tickets.  */
    textureTickets--;

    /*  Update pointer to next free entry in the request queue.  */
    nextFreeRequest = GPU_MOD(nextFreeRequest + 1, requestQueueSize);

    /*  Update statistics.  */
    txRequests->inc();
}

/*  Generates a new texture access from the texture request queue.  */
void TextureUnit::genTextureAccess()
{
    u32bit freeAccess;

    /*  Check if there are free entries in the texture access queue and
        texture requests waiting in the request queue.  */
    if ((numFreeTexAcc > 0) && (requests > 0))
    {

GPU_TEX_TRACE(
    TextureAccess *texAccess;
    texAccess = textRequestsQ[nextRequest]->getTextAccess();
    for(int i = 0; i < STAMP_FRAGMENTS; i++)
    {
        for(int j = 0; j < 4; j++)
            texTrace << *((u32bit *) &(texAccess->originalCoord[i][j])) << " ";
    };
    texTrace << texAccess->textUnit;
    texTrace << endl;
)

        /*  Get next free entry in the texture access queue.  */
        freeAccess = freeTexAccessQ[nextFreeTexAcc];

        GPU_DEBUG_BOX(
            printf("%s => Evaluating texture request at %d to be stored in texture queue at %d\n",
                getName(), nextRequest, freeAccess);
        )

        /*  Get the texture access from the texture request.  */
        texAccessQ[freeAccess] = textRequestsQ[nextRequest]->getTextAccess();
        texAccessReqQ[freeAccess] = textRequestsQ[nextRequest];

        /*  Add texture access to the waiting for address calcualtion queue.  */
        calcTexAccessQ[lastCalcTexAcc] = freeAccess;

        /*  Update number of free entries in the texture access queue.  */
        numFreeTexAcc--;

        /*  Update number of requests waiting in the texture request queue.  */
        requests--;

        /*  Update number of free entries in the texture request queue.  */
        freeRequests++;

        /*  Update the number of texture acceses for which the texel addresses must be calculated.  */
        numCalcTexAcc++;

        /*  Update pointer to the next free texture access entry.  */
        nextFreeTexAcc = GPU_MOD(nextFreeTexAcc + 1, accessQueueSize);

        /*  Update pointer to the last texture access for which to calculate texel addresses.  */
        lastCalcTexAcc = GPU_MOD(lastCalcTexAcc + 1, accessQueueSize);

        /*  Update pointer to the next request in the texture request queue.  */
        nextRequest = GPU_MOD(nextRequest + 1, requestQueueSize);
    }
}

/*  Process a Shader Command.  */
void TextureUnit::processShaderCommand(u64bit cycle, ShaderCommand *command)
{
    switch(command->getCommandType())
    {
        case RESET:

            /*  Change to reset state.  */
            state = SH_RESET;

            GPU_DEBUG_BOX(
                printf("%s => RESET command received.\n", getName());
            )

            break;

        case TX_REG_WRITE:

            /*  Write a texture unit register.  */
            processRegisterWrite(cycle, command->getRegister(), command->getSubRegister(), command->getRegisterData());

            break;

        default:
            /*  ERROR.  Unknow command code.*/

            panic("TextureUnit", "clock", "Unsupported command type.");

            break;
    }

    /*  Delete shader command.  */
    delete command;
}


/*  Processes a register write.  */
void TextureUnit::processRegisterWrite(u64bit cycle, GPURegister reg, u32bit subReg, GPURegData data)
{
    u32bit textUnit;
    u32bit mipmap;
    u32bit cubemap;

    switch(reg)
    {
        case GPU_TEXTURE_ENABLE:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture enable register.  */
            textureEnabled[subReg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_ENABLE[%d] = %s\n",
                    getName(), subReg, data.booleanVal?"TRUE":"FALSE");
            )

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_ENABLE " << data.booleanVal << endl;)

            break;

        case GPU_TEXTURE_MODE:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture mode register.  */
            textureMode[subReg] = data.txMode;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_MODE[%d] = ", getName(), subReg);
                switch(textureMode[subReg])
                {
                    case GPU_TEXTURE1D:
                        printf("TEXTURE1D\n");
                        break;

                    case GPU_TEXTURE2D:
                        printf("TEXTURE2D\n");
                        break;

                    case GPU_TEXTURE3D:
                        printf("TEXTURE3D\n");
                        break;

                    case GPU_TEXTURECUBEMAP:
                        printf("TEXTURECUBEMAP\n");
                        break;

                    default:
                        panic("TextureUnit", "processRegisterWrite", "Unsupported texture mode.");
                }

            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_MODE " << data.txMode << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_ADDRESS:

            /*  WARNING:  As the current simulator only support an index per register we must
                decode the texture unit, mipmap and cubemap image.  To do so the cubemap images of
                a mipmap are stored sequentally and then the mipmaps for a texture unit
                are stored sequentially and then the mipmap addresses of the other texture units
                are stored sequentially.  */


            textUnit = subReg / (MAX_TEXTURE_SIZE * CUBEMAP_IMAGES);
            mipmap = GPU_MOD(subReg / CUBEMAP_IMAGES, MAX_TEXTURE_SIZE);
            cubemap = GPU_MOD(subReg, CUBEMAP_IMAGES);

            /*  Check texture unit index.  */

            GPU_ASSERT(
               if (textUnit >= MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Check mipmap index.  */
            GPU_ASSERT(
                if (mipmap >= MAX_TEXTURE_SIZE)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit Mipmap index overflow.");
            )

            /*  Check cubemap index.  */
            GPU_ASSERT(
                if (cubemap >= CUBEMAP_IMAGES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit cubemap index overflow.");
            )

            /*  Write texture address register (per mipmap and texture unit).  */
            textureAddress[textUnit][mipmap][cubemap] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_ADDRESS[%d][%d][%d] = %x\n",
                    getName(), textUnit, mipmap, cubemap, textureAddress[textUnit][mipmap][cubemap]);
            )

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_WIDTH:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture width (first mipmap).  */
            textureWidth[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_WIDTH[%d] = %d\n",
                    getName(), subReg, textureWidth[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_WIDTH " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_HEIGHT:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture height (first mipmap).  */
            textureHeight[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_HEIGHT[%d] = %d\n",
                    getName(), subReg, textureHeight[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_HEIGHT " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_DEPTH:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture width (first mipmap).  */
            textureDepth[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_DEPTH[%d] = %d\n",
                    getName(), subReg, textureDepth[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_DEPTH " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_WIDTH2:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture width (log of 2 of the first mipmap).  */
            textureWidth2[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_WIDTH2[%d] = %d\n",
                    getName(), subReg, textureWidth2[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_WIDTH2 " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_HEIGHT2:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture height (log 2 of the first mipmap).  */
            textureHeight2[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_HEIGHT2[%d] = %d\n",
                    getName(), subReg, textureHeight2[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_HEIGHT2 " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_DEPTH2:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture depth (log of 2 of the first mipmap).  */
            textureDepth2[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_DEPTH2[%d] = %d\n",
                    getName(), subReg, textureDepth2[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_DEPTH2 " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_BORDER:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture border register.  */
            textureBorder[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_BORDER[%d] = %d\n",
                    getName(), subReg, textureBorder[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_BORDER " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;


        case GPU_TEXTURE_FORMAT:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture format register.  */
            textureFormat[subReg] = data.txFormat;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_FORMAT[%d] = ", getName(), subReg);

                switch(textureFormat[subReg])
                {
                    case GPU_ALPHA8:
                        printf("GPU_ALPHA8\n");
                        break;
                    case GPU_ALPHA12:
                        printf("GPU_ALPHA12\n");
                        break;
                    
                    case GPU_ALPHA16:
                        printf("GPU_ALPHA16\n");
                        break;
                    
                    case GPU_DEPTH_COMPONENT16:
                        printf("GPU_DEPTH_COMPONENT16\n");
                        break;
                    
                    case GPU_DEPTH_COMPONENT24:
                        printf("GPU_DEPTH_COMPONENT24\n");
                        break;

                    case GPU_DEPTH_COMPONENT32:
                        printf("GPU_DEPTH_COMPONENT32\n");
                        break;
                    
                    case GPU_LUMINANCE8:
                        printf("GPU_LUMINANCE8\n");
                        break;

                    case GPU_LUMINANCE8_SIGNED:
                        printf("GPU_LUMINANCE8_SIGNED\n");
                        break;

                    case GPU_LUMINANCE12:
                        printf("GPU_LUMINANCE12\n");
                        break;

                    case GPU_LUMINANCE16:
                        printf("GPU_LUMINANCE16\n");
                        break;
                    
                    case GPU_LUMINANCE4_ALPHA4:
                        printf("GPU_LUMINANCE4_ALPHA4\n");
                        break;
                        
                    case GPU_LUMINANCE6_ALPHA2:
                        printf("GPU_LUMINANCE6_ALPHA2\n");
                        break;
                        
                    case GPU_LUMINANCE8_ALPHA8:
                        printf("GPU_LUMINANCE8_ALPHA8\n");
                        break;
                    
                    case GPU_LUMINANCE8_ALPHA8_SIGNED:
                        printf("GPU_LUMINANCE8_ALPHA8_SIGNED\n");
                        break;

                    case GPU_LUMINANCE12_ALPHA4:
                        printf("GPU_LUMINANCE12_ALPHA4\n");
                        break;
                    
                    case GPU_LUMINANCE12_ALPHA12:
                        printf("GPU_LUMINANCE12_ALPHA12\n");
                        break;

                    case GPU_LUMINANCE16_ALPHA16:
                        printf("GPU_LUMINANCE16_ALPHA16\n");
                        break;

                    case GPU_INTENSITY8:
                        printf("GPU_INTENSITY8\n");
                        break;
                    
                    case GPU_INTENSITY12:
                        printf("GPU_INTENSITY12\n");
                        break;
                    
                    case GPU_INTENSITY16:
                        printf("GPU_INTENSITY16\n");
                        break;
                    case GPU_RGB332:
                        printf("GPU_RGB332\n");
                        break;

                    case GPU_RGB444:
                        printf("GPU_RGB444\n");
                        break;
                    
                    case GPU_RGB555:
                        printf("GPU_RGB555\n");
                        break;

                    case GPU_RGB565:
                        printf("GPU_RGB565\n");
                        break;

                    case GPU_RGB888:
                        printf("GPU_RGB888\n");
                        break;
                    
                    case GPU_RGB101010:
                        printf("GPU_RGB101010\n");
                        break;
                    case GPU_RGB121212:
                        printf("GPU_RGB121212\n");
                        break;
                    
                    case GPU_RGBA2222:
                        printf("GPU_RGBA2222\n");
                        break;
                    
                    case GPU_RGBA4444:
                        printf("GPU_RGBA4444\n");
                        break;
                        
                    case GPU_RGBA5551:
                        printf("GPU_RGBA5551\n");
                        break;
                    
                    case GPU_RGBA8888:
                        printf("GPU_RGBA8888\n");
                        break;
                    
                    case GPU_RGBA1010102:
                        printf("GPU_RGB1010102\n");
                        break;
                        
                    case GPU_R16:
                        printf("GPU_R16\n");
                        break;
                        
                    case GPU_RG16:
                        printf("GPU_RG16\n");
                        break;
                    
                    case GPU_RGBA16:
                        printf("GPU_RGBA16\n");
                        break;

                    case GPU_R16F:
                        printf("GPU_R16F\n");
                        break;
                        
                    case GPU_RG16F:
                        printf("GPU_RG16F\n");
                        break;
                    
                    case GPU_RGBA16F:
                        printf("GPU_RGBA16F\n");
                        break;

                    case GPU_R32F:
                        printf("GPU_R32F\n");
                        break;
                        
                    case GPU_RG32F:
                        printf("GPU_RG32F\n");
                        break;
                    
                    case GPU_RGBA32F:
                        printf("GPU_RGBA32F\n");
                        break;

                    default:
                        printf("unknown format with code %0X\n", subReg);
                        break;
                }
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_FORMAT " << data.txFormat << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_REVERSE:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture reverse register.  */
            textureReverse[subReg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_REVERSE[%d] = %s\n", getName(), subReg,
                    textureReverse[subReg]?"TRUE":"FALSE");
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_REVERSE " << data.booleanVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_D3D9_COLOR_CONV:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture d3d9 color component order register.  */
            textD3D9ColorConv[subReg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_D3D9_COLOR_CONVERSION[%d] = %s\n", getName(), subReg,
                    textD3D9ColorConv[subReg]?"TRUE":"FALSE");
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_D3D9_COLOR_CONV " << data.booleanVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_D3D9_V_INV:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture d3d9 u coordinate invert register.  */
            textD3D9VInvert[subReg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_D3D9_V_INV[%d] = %s\n", getName(), subReg,
                    textD3D9VInvert[subReg]?"TRUE":"FALSE");
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_D3D9_V_INV " << data.booleanVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_COMPRESSION:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture compression mode register.  */
            textureCompr[subReg] = data.txCompression;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_COMPRESSION[%d] = ", getName(), subReg);

                switch(textureCompr[subReg])
                {
                    case GPU_NO_TEXTURE_COMPRESSION:
                        printf("GPU_NO_TEXTURE_COMPRESSION.\n");
                        break;

                    case GPU_S3TC_DXT1_RGB:
                        printf("GPU_S3TC_DXT1_RGB.\n");
                        break;

                    case GPU_S3TC_DXT1_RGBA:
                        printf("GPU_S3TC_DXT1_RGBA.\n");
                        break;

                    case GPU_S3TC_DXT3_RGBA:
                        printf("GPU_S3TC_DXT3_RGBA.\n");
                        break;

                    case GPU_S3TC_DXT5_RGBA:
                        printf("GPU_S3TC_DXT5_RGBA.\n");
                        break;
                        
                    case GPU_LATC1:
                        printf("GPU_LATC1.\n");
                        break;                        

                    case GPU_LATC1_SIGNED:
                        printf("GPU_LATC1_SIGNED.\n");
                        break;                        

                    case GPU_LATC2:
                        printf("GPU_LATC2.\n");
                        break;                        

                    case GPU_LATC2_SIGNED:
                        printf("GPU_LATC2_SIGNED.\n");
                        break;                        

                    default:
                        panic("TextureUnit", "processRegisterWrite", "Unsupported texture compression mode.");
                        break;
                }
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_COMPRESSION " << data.txCompression << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_BLOCKING:

            //  Check texture unit index.
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            //  Write texture blocking mode register.
            textureBlocking[subReg] = data.txBlocking;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_BLOCKING[%d] = ", getName(), subReg);

                switch(textureBlocking[subReg])
                {
                    case GPU_TXBLOCK_TEXTURE:
                        printf("GPU_TXBLOCK_TEXTURE.\n");
                        break;

                    case GPU_TXBLOCK_FRAMEBUFFER:
                        printf("GPU_TXBLOCK_FRAMEBUFFER.\n");
                        break;

                    default:
                        panic("TextureUnit", "processRegisterWrite", "Unsupported texture blocking mode.");
                        break;
                }
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_BLOCKING " << data.txBlocking << endl;)

            //  Write register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_BORDER_COLOR:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture border color register.  */
            textBorderColor[subReg][0] = data.qfVal[0];
            textBorderColor[subReg][1] = data.qfVal[1];
            textBorderColor[subReg][2] = data.qfVal[2];
            textBorderColor[subReg][3] = data.qfVal[3];

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_BORDER_COLOR[%d] = {%d, %d, %d, %d}\n",
                    getName(), subReg, textBorderColor[subReg][0], textBorderColor[subReg][1], textBorderColor[subReg][2],
                    textBorderColor[subReg][3]);
            )

            GPU_TEX_TRACE(
                texTrace << "UPD " << subReg << " TEX_BORDER_COLOR ";
                texTrace << *((u32bit *) &data.qfVal[0]) << " ";
                texTrace << *((u32bit *) &data.qfVal[1]) << " ";
                texTrace << *((u32bit *) &data.qfVal[2]) << " ";
                texTrace << *((u32bit *) &data.qfVal[3]);
                texTrace << endl;
            )

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_WRAP_S:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture wrap in s dimension register.  */
            textureWrapS[subReg] = data.txClamp;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_WRAP_S[%d] = ", getName(), subReg);

                switch(textureWrapS[subReg])
                {

                    case GPU_TEXT_CLAMP:
                        printf("CLAMP\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_EDGE:
                        printf("CLAMP_TO_EDGE\n");
                        break;

                    case GPU_TEXT_REPEAT:
                        printf("REPEAT\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_BORDER:
                        printf("CLAMP_TO_BORDER\n");
                        break;

                    case GPU_TEXT_MIRRORED_REPEAT:
                        printf("MIRRORED_REPEAT\n");
                        break;

                    default:
                        panic("TextureUnit", "processRegisterWrite", "Unsupported texture clamp mode.");
                        break;
                }
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_WRAP_S " << data.txClamp << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_WRAP_T:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture wrap in t dimension register.  */
            textureWrapT[subReg] = data.txClamp;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_WRAP_T[%d] = ", getName(), subReg);

                switch(textureWrapT[subReg])
                {

                    case GPU_TEXT_CLAMP:
                        printf("CLAMP\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_EDGE:
                        printf("CLAMP_TO_EDGE\n");
                        break;

                    case GPU_TEXT_REPEAT:
                        printf("REPEAT\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_BORDER:
                        printf("CLAMP_TO_BORDER\n");
                        break;

                    case GPU_TEXT_MIRRORED_REPEAT:
                        printf("MIRRORED_REPEAT\n");
                        break;

                    default:
                        panic("TextureUnit", "processRegisterWrite", "Unsupported texture clamp mode.");
                        break;
                }
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_WRAP_T " << data.txClamp << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_WRAP_R:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture wrap in r dimension register.  */
            textureWrapR[subReg] = data.txClamp;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_WRAP_R[%d] = ", getName(), subReg);

                switch(textureWrapR[subReg])
                {

                    case GPU_TEXT_CLAMP:
                        printf("CLAMP\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_EDGE:
                        printf("CLAMP_TO_EDGE\n");
                        break;

                   case GPU_TEXT_REPEAT:
                        printf("REPEAT\n");
                        break;

                    case GPU_TEXT_CLAMP_TO_BORDER:
                        printf("CLAMP_TO_BORDER\n");
                        break;

                    case GPU_TEXT_MIRRORED_REPEAT:
                        printf("MIRRORED_REPEAT\n");
                        break;

                    default:
                        panic("TextureUnit", "processRegisterWrite", "Unsupported texture clamp mode.");
                        break;
                }
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_WRAP_R " << data.txClamp << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_NON_NORMALIZED:

            //  Check texture unit index.
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            //  Write texture non normalized texture coordinates.
            textureNonNormalized[subReg] = data.booleanVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_NON_NORMALIZED[%d] = ", getName(), subReg, data.booleanVal ? "T" : "F");
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_NON_NORMALIZED " << data.booleanVal << endl;)

            //  Write register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_MIN_FILTER:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture minification filter register.  */
            textureMinFilter[subReg] = data.txFilter;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_MIN_FILTER[%d] = ", getName(), subReg);

                switch(textureMinFilter[subReg])
                {
                    case GPU_NEAREST:
                        printf("NEAREST\n");
                        break;

                    case GPU_LINEAR:
                        printf("LINEAR\n");
                        break;

                    case GPU_NEAREST_MIPMAP_NEAREST:
                        printf("NEAREST_MIPMAP_NEAREST\n");
                        break;

                    case GPU_NEAREST_MIPMAP_LINEAR:
                        printf("NEAREST_MIPMAP_LINEAR\n");
                        break;

                    case GPU_LINEAR_MIPMAP_NEAREST:
                        printf("LINEAR_MIPMAP_NEAREST\n");
                        break;

                    case GPU_LINEAR_MIPMAP_LINEAR:
                        printf("LINEAR_MIPMAP_LINEAR\n");
                        break;

                    default:
                        panic("TextureUnit", "processRegisterWrite", "Unsupported texture minification filter mode.");
                        break;
                }
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_MIN_FILTER " << data.txFilter << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_MAG_FILTER:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            GPU_ASSERT(
                if ((data.txFilter != GPU_NEAREST) && (data.txFilter != GPU_LINEAR))
                    panic("TextureUnit", "processRegisterWrite", "Unsupported magnification filter.");
            )

            /*  Write texture magnification filter register.  */
            textureMagFilter[subReg] = data.txFilter;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_MAG_FILTER[%d] = ", getName(), subReg);

                switch(textureMagFilter[subReg])
                {
                    case GPU_NEAREST:
                        printf("NEAREST\n");
                        break;

                    case GPU_LINEAR:
                        printf("LINEAR\n");
                        break;

                    case GPU_NEAREST_MIPMAP_NEAREST:
                        printf("NEAREST_MIPMAP_NEAREST\n");
                        break;

                    case GPU_NEAREST_MIPMAP_LINEAR:
                        printf("NEAREST_MIPMAP_LINEAR\n");
                        break;

                    case GPU_LINEAR_MIPMAP_NEAREST:
                        printf("LINEAR_MIPMAP_NEAREST\n");
                        break;

                    case GPU_LINEAR_MIPMAP_LINEAR:
                        printf("LINEAR_MIPMAP_LINEAR\n");
                        break;

                    default:
                        panic("TextureUnit", "processRegisterWrite", "Unsupported texture magnification filter mode.");
                        break;
                }
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_MAG_FILTER " << data.txFilter << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;


        case GPU_TEXTURE_ENABLE_COMPARISON:

            //  Check texture unit index.
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            //  Write texture enable comparison filter (PCF) register.
            textureEnableComparison[subReg] = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("%s => Write GPU_TEXTURE_ENABLE_COMPARISON[%d] = %s.\n", getName(), subReg, data.booleanVal ? "T" : "F");
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " ENABLE_COMPARISON " << data.booleanVal << endl;)

            //  Write register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;
            
        case GPU_TEXTURE_COMPARISON_FUNCTION:

            //  Check texture unit index.
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )
            
            //  Write texture comparison function (PCF) register.
            textureComparisonFunction[subReg] = data.compare;

            GPU_DEBUG_BOX(
                printf("%s => Write GPU_TEXTURE_COMPARISON_FUNCTION[%d] = ", getName(), subReg);
                switch(data.compare)
                {
                    case GPU_NEVER:
                        printf("NEVER.\n");
                        break;

                    case GPU_ALWAYS:
                        printf("ALWAYS.\n");
                        break;

                    case GPU_LESS:
                        printf("LESS.\n");
                        break;

                    case GPU_LEQUAL:
                        printf("LEQUAL.\n");
                        break;

                    case GPU_EQUAL:
                        printf("EQUAL.\n");
                        break;

                    case GPU_GEQUAL:
                        printf("GEQUAL.\n");
                        break;

                    case GPU_GREATER:
                        printf("GREATER.\n");
                        break;

                    case GPU_NOTEQUAL:
                        printf("NOTEQUAL.\n");

                    default:
                        panic("bGPU-emu", "processRegisterWrite", "Undefined compare function mode");
                        break;
                }
            )
            
            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " COMPARISON_FUNCTION " << data.compare << endl;)

            //  Write register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);
            
            break;
            
        case GPU_TEXTURE_SRGB:

            //  Check texture unit index.
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            //  Write texture sRGB space to linear space conversion register.
            textureSRGB[subReg] = data.booleanVal;
            
            GPU_DEBUG_BOX(
                printf("%s => Write GPU_TEXTURE_SRGB[%d] = %s.\n", getName(), subReg, data.booleanVal ? "T" : "F");
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " SRGB " << data.booleanVal << endl;)

            //  Write register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;
            
        case GPU_TEXTURE_MIN_LOD:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture minimum lod register.  */
            textureMinLOD[subReg] = data.f32Val;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_MIN_LOD[%d] = %f\n",
                    getName(), subReg, textureMinLOD[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_MIN_LOD " << *((u32bit *) &data.f32Val) << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_MAX_LOD:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture maximum lod register.  */
            textureMaxLOD[subReg] = data.f32Val;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_MAX_LOD[%d] = %f\n",
                    getName(), subReg, textureMaxLOD[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_MAX_LOD " << *((u32bit *) &data.f32Val) << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_LOD_BIAS:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture lod ways register.  */
            textureLODWays[subReg] = data.f32Val;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_LOD_BIAS[%d] = %f\n",
                    getName(), subReg, textureLODWays[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_LOD_BIAS " << *((u32bit *) &data.f32Val) << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_MIN_LEVEL:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture minimum mipmap level register.  */
            textureMinLevel[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_MIN_LEVEL[%d] = %d\n",
                    getName(), subReg, textureMinLevel[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_MIN_LEVEL " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_MAX_LEVEL:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture maximum mipmap level register.  */
            textureMaxLevel[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_MAX_LEVEL[%d] = %d\n",
                    getName(), subReg, textureMaxLevel[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_MAX_LEVEL " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;


        case GPU_TEXT_UNIT_LOD_BIAS:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Write texture unit lod ways register.  */
            textureUnitLODWays[subReg] = data.f32Val;
 
            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXT_UNIT_LOD_BIAS[%d] = %f\n",
                    getName(), subReg, textureUnitLODWays[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_UNIT_LOD_BIAS " << *((u32bit *) &data.f32Val) << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_TEXTURE_MAX_ANISOTROPY:

            /*  Check texture unit index.  */
            GPU_ASSERT(
                if (subReg > MAX_TEXTURES)
                    panic("TextureUnit", "processRegisterWrite", "Texture Unit index overflow.");
            )

            /*  Check maximum supported anisotropy.  */
            GPU_ASSERT(
                if (data.uintVal == 0)
                    panic("TextureUnit", "processRegisterWrite", "Maximum anisotropy for a texture must be at least 1.");
                if (data.uintVal > MAX_ANISOTROPY)
                    panic("TextureUnit", "processRegisterWrite", "Maximum anisotropy for the texture too large.");
            )

            /*  Write texture unit max anisotropy register.  */
            maxAnisotropy[subReg] = data.uintVal;

            GPU_DEBUG_BOX(
                printf("%s => Writing register GPU_TEXTURE_MAX_ANISOTROPY[%d] = %d\n",
                    getName(), subReg, maxAnisotropy[subReg]);
            )

            GPU_TEX_TRACE(texTrace << "UPD " << subReg << " TEX_MAX_ANISO " << data.uintVal << endl;)

            /*  Write register in the Texture Emulator.  */
            txEmul.writeRegister(reg, subReg, data);

            break;

        case GPU_VERTEX_ATTRIBUTE_MAP :
        
            //  Vertex attribute to stream buffer map register.
            
            GPU_DEBUG_BOX(
                printf("%s (%lld) => Writing register VERTEX_ATTRIBUTE_MAP[%d] = %d\n",
                    getName(), cycle, subReg, data.uintVal);
            )

            GPU_ASSERT(
                if (subReg >= MAX_VERTEX_ATTRIBUTES)                
                    panic("TextureUnit", "processRegisterWrite", "Vertex attribute identifier our of range.");
                if ((data.uintVal >= MAX_STREAM_BUFFERS) && (data.uintVal != ST_INACTIVE_ATTRIBUTE))
                    panic("TextureUnit", "processRegisterWrite", "Illegal stream buffer ID.");
            )
            
            //  Write vertex attribute to stream buffer map register.
            attributeMap[subReg] = data.uintVal;
            
            //  Update the register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);
            
            break;
            
        case GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE:
        
            //  Vertex attribute default value register.

            GPU_DEBUG_BOX(
                printf("%s (%lld) => GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE[%d] = {%f, %f, %f, %f}.\n",
                    getName(), cycle, subReg, data.qfVal[0], data.qfVal[1], data.qfVal[2], data.qfVal[3]);
            )

            //  Check the vertex attribute ID.
            GPU_ASSERT(
                if (subReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("TextureUnit", "processRegisterWrite", "Illegal vertex attribute ID.");
            )

            //  Write the vertex attribute default value register.
            attrDefValue[subReg][0] = data.qfVal[0];
            attrDefValue[subReg][1] = data.qfVal[1];
            attrDefValue[subReg][2] = data.qfVal[2];
            attrDefValue[subReg][3] = data.qfVal[3];

            //  Update the register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;
            
        case GPU_STREAM_ADDRESS:
        
            //  Stream buffer address.

            GPU_DEBUG_BOX(
                printf("%s (%lld) => GPU_STREAM_ADDRESS[%d] = %x.\n", getName(), cycle, subReg, data.uintVal);
            )
            
            //  Check the stream buffer ID.
            GPU_ASSERT(
                if (subReg >= MAX_STREAM_BUFFERS)
                    panic("TextureUnit", "processRegisterWrite", "Illegal stream buffer ID.");
            )

            //  Write the stream buffer address register.
            streamAddress[subReg] = data.uintVal;
            
            //  Update the register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;
            
        case GPU_STREAM_STRIDE:
        
            //  Stream buffer stride.

            GPU_DEBUG_BOX(
               printf("%s (%lld) => GPU_STREAM_STRIDE[%d] = %d.\n", getName(), cycle, subReg, data.uintVal);
            )

            //  Check the stream buffer ID.
            GPU_ASSERT(
                if (subReg >= MAX_STREAM_BUFFERS)
                    panic("TextureUnit", "processRegisterWrite", "Illegal stream buffer ID.");
            )

            //  Write stream buffer stride register.
            streamStride[subReg] = data.uintVal;

            //  Update the register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;
            
        case GPU_STREAM_DATA:
        
            //  Stream buffer data type.

            GPU_DEBUG_BOX(
                printf("%s (%lld) => GPU_STREAMDATA[%d] = ", getName(), cycle, subReg);
                switch(data.streamData)
                {
                    case SD_UNORM8:
                        printf("SD_UNORM8.\n");
                        break;
                    case SD_SNORM8:
                        printf("SD_SNORM8.\n");
                        break;
                    case SD_UNORM16:
                        printf("SD_UNORM16.\n");
                        break;
                    case SD_SNORM16:
                        printf("SD_SNORM16.\n");
                        break;
                    case SD_UNORM32:
                        printf("SD_UNORM32.\n");
                        break;
                    case SD_SNORM32:
                        printf("SD_SNORM32.\n");
                        break;
                    case SD_FLOAT16:
                        printf("SD_FLOAT16.\n");
                        break;                    
                    case SD_FLOAT32:
                        printf("SD_FLOAT32.\n");
                        break;
                    case SD_UINT8:
                        printf("SD_UINT8.\n");
                        break;
                    case SD_SINT8:
                        printf("SD_SINT8.\n");
                        break;
                    case SD_UINT16:
                        printf("SD_UINT16.\n");
                        break;
                    case SD_SINT16:
                        printf("SD_SINT16.\n");
                        break;
                    case SD_UINT32:
                        printf("SD_UINT32.\n");
                        break;
                    case SD_SINT32:
                        printf("SD_SINT32.\n");
                        break;
                    default:
                        panic("TextureUnit", "processRegisterWrite", "Undefined format.");
                        break;                        
                }
            )

            //  Check the stream buffer ID.
            GPU_ASSERT(
                if (subReg >= MAX_STREAM_BUFFERS)
                    panic("TextureUnit", "processRegisterWrite", "Illegal stream buffer ID.");
            )

            //  Write stream buffer data type register.
            streamData[subReg] = data.streamData;

            //  Update the register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;
            
        case GPU_STREAM_ELEMENTS:
        
            //  Stream buffer elements.

            GPU_DEBUG_BOX(
                printf("%s (%lld) => GPU_STREAM_ELEMENTS[%d] = %d.\n", getName(), cycle, subReg, data.uintVal);
            )
            
            //  Check the stream buffer ID.
            GPU_ASSERT(
                if (subReg >= MAX_STREAM_BUFFERS)
                    panic("TextureUnit", "processRegisterWrite", "Illegal stream buffer ID.");
            )

            //  Check the number of elements to read.
            GPU_ASSERT(
                if (data.uintVal > 4)
                    panic("TextureUnit", "processRegisterWrite", "Maximum 4 elements per stream read allowed (SIMD4).");
            )
            
            //  Write stream buffer elements register.
            streamElements[subReg] = data.uintVal;

            //  Update the register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;
            
        case GPU_D3D9_COLOR_STREAM:
        
            //  Sets if the color stream must be read using the component order defined by D3D9.
            
            GPU_DEBUG_BOX(
                printf("%s (%lld) => GPU_D3D9_COLOR_STREAM[%d] = %s.\n", getName(), cycle, subReg, data.booleanVal? "TRUE" : "FALSE");
            )

            //  Check the stream buffer ID.
            GPU_ASSERT(
                if (subReg >= MAX_STREAM_BUFFERS)
                    panic("TextureUnit", "processRegisterWrite", "Illegal stream buffer ID.");
            )

            //  Write D3D9 color stream mode register.
            d3d9ColorStream[subReg] = data.booleanVal;
            
            //  Update the register in the Texture Emulator.
            txEmul.writeRegister(reg, subReg, data);

            break;

        default:

            panic("TextureUnit", "processRegisterWrite", "Unsupported texture unit register.");
            break;
    }

}


/*  Processes a memory transaction.  */
void TextureUnit::processMemoryTransaction(u64bit cycle, MemoryTransaction *memTrans)
{
    /*  Get transaction type.  */
    switch(memTrans->getCommand())
    {
        case MT_STATE:

            /*  Received state of the Memory controller.  */
            memoryState = memTrans->getState();

            GPU_DEBUG_BOX(
                printf("%s => Memory Controller State = %d\n", getName(), memoryState);
            )

            break;

        default:

            GPU_DEBUG_BOX(
                printf("%s => Memory Transaction to the Texture Cache.\n", getName());
            )

            /*  Let the texture cache process any other transaction.  */
            textCache->processMemoryTransaction(memTrans);

            /*  Update statistics.  */
            readBytes->inc(memTrans->getSize());

            break;
    }

    /*  Delete processed memory transaction.  */
    delete memTrans;
}

void TextureUnit::setDebugMode(bool mode)
{
    debugMode = mode;
    textCache->setDebug(mode);
}

void TextureUnit::getState(string &stateString)
{
    stringstream stateStream;

    stateStream.clear();

    stateStream << " state = ";

    switch(state)
    {
        case SH_RESET:
            stateStream << "SH_RESET";
            break;
        case SH_READY:
            stateStream << "SH_READY";
            break;
        default:
            stateStream << "undefined";
            break;
    }

    stateStream << " | Texture Tickets = " << textureTickets;
    stateStream << " | Texture Requests = " << requests;
    stateStream << " | Texture Results = " << numResults;
    stateStream << " | Address TAs = " << numCalcTexAcc;
    stateStream << " | Fetch TAs = " << numFetchTexAcc;
    stateStream << " | To Filter TAs = " << numToFilter;
    stateStream << " | Filtered TAs  " << numFiltered;

    stateString.assign(stateStream.str());
}

void TextureUnit::stallReport(u64bit cycle, string &stallReport)
{
    stringstream reportStream;
    
    reportStream << getName() << " stall report for cycle " << cycle << endl;
    reportStream << "------------------------------------------------------------" << endl;
    reportStream << " Stall Threashold = " << STALL_CYCLE_THRESHOLD << endl;
    
    reportStream << " Texture Tickets = " << textureTickets;
    reportStream << " | Texture Requests = " << requests;
    reportStream << " | Texture Results = " << numResults;
    reportStream << " | Address TAs = " << numCalcTexAcc;
    reportStream << " | Fetch TAs = " << numFetchTexAcc;
    reportStream << " | To Filter TAs = " << numToFilter;
    reportStream << " | Filtered TAs  " << numFiltered << endl;

    reportStream << " Num Ready Reads = " << numReadyReads << " | Num Wait Reads = " << numWaitReads << endl;
    reportStream << " Num Free Ready Reads = " << numFreeReadyReads << " | Num Free Wait Reads = " << numFreeWaitReads << endl;
    
    reportStream << " List of requests waiting : " << endl;
    
    for(u32bit w = 0; w < numWaitReads; w++)
    {
        u32bit waitAccess = waitReadW[waitReadQ[w]].access;
        u32bit waitTrilinear = waitReadW[waitReadQ[w]].trilinear;
        
        reportStream << "  Waiting request " << w << " access queue entry " << waitAccess << " trilinear " << waitTrilinear << endl;
        for(u32bit f = 0; f < stampFragments; f++)
        {            
            u32bit texelsToRequest = texAccessQ[waitAccess]->trilinear[waitTrilinear]->texelsLoop[f] * texAccessQ[waitAccess]->trilinear[waitTrilinear]->loops[f];
            reportStream << "    Fragment " << f << " texels to read " << texelsToRequest << endl;
            for(u32bit t = 0; t < texelsToRequest; t++)
            {
                reportStream << "      Texel " << t << " ready " << (texAccessQ[waitAccess]->trilinear[waitTrilinear]->ready[f][t] ? "Y" : "N");
                reportStream << " tag = " << hex << texAccessQ[waitAccess]->trilinear[waitTrilinear]->tag[f][t] << dec << endl;
            }
        }
    }

    string cacheReport;
    
    textCache->stallReport(cycle, cacheReport);
    
    reportStream << "Texture Cache report : " << endl;
    reportStream << cacheReport << endl;
    
    stallReport.assign(reportStream.str());
}

void TextureUnit::printTextureAccessInfo(TextureAccess *texAccess)
{
    printf("Texture Access Info\n");
    printf("-------------------\n");
    printf("  Texture Unit = %d\n", texAccess->textUnit);
    printf("  Width = %d\n", textureWidth[texAccess->textUnit]);
    printf("  Height = %d\n", textureHeight[texAccess->textUnit]);
    if (textureMode[texAccess->textUnit] == GPU_TEXTURE3D)
        printf("  Depth = %d\n", textureDepth[texAccess->textUnit]);
    printf("  Format = %d\n", textureFormat[texAccess->textUnit]);
    printf("  Type = ");    
    switch(textureMode[texAccess->textUnit])
    {
        case GPU_TEXTURE1D:
            printf("1D\n");
            break;
        case GPU_TEXTURE2D:
            printf("2D\n");
            break;
        case GPU_TEXTURECUBEMAP:
            printf("Cube\n");
            break;
        case GPU_TEXTURE3D:
            printf("3D\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
    printf("  Blocking = ");
    switch(textureBlocking[texAccess->textUnit])
    {
        case GPU_TXBLOCK_TEXTURE:
            printf("TEXTURE\n");
            break;
        case GPU_TXBLOCK_FRAMEBUFFER:
            printf("FRAMEBUFFER\n");
            break;
        default:
            printf("Unknown\n");
    }
    printf("  Magnification filter = ");
    switch(textureMagFilter[texAccess->textUnit])
    {
        case GPU_NEAREST:
            printf("NEAREST\n");
            break;

        case GPU_LINEAR:
            printf("LINEAR\n");
            break;

        case GPU_NEAREST_MIPMAP_NEAREST:
            printf("NEAREST_MIPMAP_NEAREST\n");
            break;

        case GPU_NEAREST_MIPMAP_LINEAR:
            printf("NEAREST_MIPMAP_LINEAR\n");
            break;

        case GPU_LINEAR_MIPMAP_NEAREST:
            printf("LINEAR_MIPMAP_NEAREST\n");
            break;

        case GPU_LINEAR_MIPMAP_LINEAR:
            printf("LINEAR_MIPMAP_LINEAR\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
    printf("  Minification filter = ");
    switch(textureMinFilter[texAccess->textUnit])
    {
        case GPU_NEAREST:
            printf("NEAREST\n");
            break;

        case GPU_LINEAR:
            printf("LINEAR\n");
            break;

        case GPU_NEAREST_MIPMAP_NEAREST:
            printf("NEAREST_MIPMAP_NEAREST\n");
            break;

        case GPU_NEAREST_MIPMAP_LINEAR:
            printf("NEAREST_MIPMAP_LINEAR\n");
            break;

        case GPU_LINEAR_MIPMAP_NEAREST:
            printf("LINEAR_MIPMAP_NEAREST\n");
            break;

        case GPU_LINEAR_MIPMAP_LINEAR:
            printf("LINEAR_MIPMAP_LINEAR\n");
            break;
        default:
            printf("Unknown\n");
            break;
    }
    
    
    for(u32bit p = 0; p < STAMP_FRAGMENTS; p++)
    {
        printf(" Fragment %d\n", p);
        printf("------------\n");
        printf("  LOD = %f\n", texAccess->lod[p]);
        printf("  LOD0 = %d\n", texAccess->level[p][0]);
        printf("  LOD1 = %d\n", texAccess->level[p][1]);
        f32bit w = texAccess->lod[p] - static_cast<f32bit>(GPU_FLOOR(texAccess->lod[p]));
        printf("  WeightLOD = %f\n", w);
        printf("  Samples = %d\n", texAccess->anisoSamples);
        if (textureMode[texAccess->textUnit] == GPU_TEXTURECUBEMAP)
        {
            printf("Pixel %d Coord (orig) = (%f, %f, %f, %f)\n", p,
                texAccess->originalCoord[p][0], texAccess->originalCoord[p][1],
                texAccess->originalCoord[p][2], texAccess->originalCoord[p][3]);
            printf("Pixel %d Coord (post cube) = (%f, %f, %f, %f)\n", p,
                texAccess->coordinates[p][0], texAccess->coordinates[p][1],
                texAccess->coordinates[p][2], texAccess->coordinates[p][3]);
            printf("Face = %d (", texAccess->cubemap);
            switch(texAccess->cubemap)
            {
                case GPU_CUBEMAP_NEGATIVE_X:
                    printf("-X)\n");
                    break;
                case GPU_CUBEMAP_POSITIVE_X:
                    printf("+X)\n");
                    break;
                case GPU_CUBEMAP_NEGATIVE_Y:
                    printf("-Y)\n");
                    break;
                case GPU_CUBEMAP_POSITIVE_Y:
                    printf("+Y)\n");
                    break;
                case GPU_CUBEMAP_NEGATIVE_Z:
                    printf("-Z)\n");
                    break;
                case GPU_CUBEMAP_POSITIVE_Z:
                    printf("+Z)\n");
                    break;
                default:
                    printf("Unknown face)\n");
                    break;
            }
        }
        else
        {
            printf("Pixel %d Coord = (%f, %f, %f, %f)\n", p,
                texAccess->coordinates[p][0], texAccess->coordinates[p][1],
                texAccess->coordinates[p][2], texAccess->coordinates[p][3]);
        }
     
        for(u32bit s = 0; s < texAccess->anisoSamples; s++)
        {
            printf("  Sample from two mips = %s\n",
                texAccess->trilinear[s]->sampleFromTwoMips[p] ? "YES":"NO");
            /*printf("  LOD0\n");
            printf("    WeightA = %f\n", texAccess->trilinear[s]->a[p][0]);
            printf("    WeightB = %f\n", texAccess->trilinear[s]->b[p][0]);
            printf("    WeightC = %f\n", texAccess->trilinear[s]->c[p][0]);
            printf("  LOD1\n");
            printf("    WeightA = %f\n", texAccess->trilinear[s]->a[p][1]);
            printf("    WeightB = %f\n", texAccess->trilinear[s]->b[p][1]);
            printf("    WeightC = %f\n", texAccess->trilinear[s]->c[p][1]);
            printf("  Loops = %d\n", texAccess->trilinear[s]->loops[p]);*/
            for(u32bit b = 0; b < texAccess->trilinear[s]->loops[p]; b++)
            {
                u32bit texelsLoop = texAccess->trilinear[s]->texelsLoop[p];
                printf("Loop %d\n", b);
                printf("-------------\n");
                printf("  Texels/Loop = %d\n", texelsLoop);
                for(u32bit t = 0; t < texelsLoop; t++)
                {
                    printf("  Texel = (%d, %d)\n",
                    texAccess->trilinear[s]->i[p][b * texelsLoop + t],
                    texAccess->trilinear[s]->j[p][b * texelsLoop + t]);
                    if (textureMode[texAccess->textUnit] == GPU_TEXTURE3D)
                        printf("  Slice = %d\n", texAccess->trilinear[s]->k[p][b * texelsLoop + t]);
                    printf("  Address = %16llx\n",
                        texAccess->trilinear[s]->address[p][b * texelsLoop + t]);
                    /*printf("         Value = {%f, %f, %f, %f} [%02x, %02x, %02x, %02x]\n",
                        texAccess->trilinear[s]->texel[p][b * texelsLoop + t][0],
                        texAccess->trilinear[s]->texel[p][b * texelsLoop + t][1],
                        texAccess->trilinear[s]->texel[p][b * texelsLoop + t][2],
                        texAccess->trilinear[s]->texel[p][b * texelsLoop + t][3],
                        u32bit(255.0f * texAccess->trilinear[s]->texel[p][b * texelsLoop + t][0]),
                        u32bit(255.0f * texAccess->trilinear[s]->texel[p][b * texelsLoop + t][1]),
                        u32bit(255.0f * texAccess->trilinear[s]->texel[p][b * texelsLoop + t][2]),
                        u32bit(255.0f * texAccess->trilinear[s]->texel[p][b * texelsLoop + t][3]));*/
                }
            }
        }
        /*printf("         Result = {%f, %f, %f, %f}\n",
            texAccess->sample[p][0], texAccess->sample[p][1],
            texAccess->sample[p][2], texAccess->sample[p][3]);*/
    }

    printf("-------------\n");
}


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
 */

#ifndef FIFOSCHEDULER_H
    #define FIFOSCHEDULER_H

#include <queue>
#include <utility>
#include "FifoSchedulerBase.h"

namespace gpu3d
{
namespace memorycontroller
{

class GPUMemorySpecs;

/**
 * Basic fifo-scheduler with a single-unified read/write queue
 */
class FifoScheduler : public FifoSchedulerBase
{

public:

    FifoScheduler(const char* name,
                  const char* prefix,
                  Box* parent,
                  const CommonConfig& config );
                  // u32bit maxTransactions );

private:

    GPUStatistics::Statistic& closePageActivationsCount; // Counts how many times the close page algorithm is activated

    std::vector<u32bit> pendingBankAccesses;

    bool useInnerSignals;

    std::queue<ChannelTransaction*> transQ; ///< Read and write transaction queue
    u32bit maxSizeQ; ///< Maximum number of in-flight channel transactions

    // transQ + currentTrans
    Signal* queueSignal; // bw = transQ + 1
    std::list<ChannelTransaction*> queueTrack;

    void processStateQueueSignals(u64bit cycle);

protected:
    
    void receiveRequest(u64bit cycle, ChannelTransaction* request);
    ChannelTransaction* selectNextTransaction(u64bit cycle);

    void handler_ddrCommandNotSent(const DDRCommand* ddrCmd, u64bit cycle);

    // calls directly processStateQueueSignals
    void handler_endOfClock(u64bit cycle);
    
};
} // namespace memorycontroller
} // namespace gpu3d

#endif // FIFOSCHEDULER_H


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

#ifndef RWFIFOSCHEDULER_H
    #define RWFIFOSCHEDULER_H

#include "FifoSchedulerBase.h"
#include "DependencyQueue.h"
#include "SwitchOperationMode.h"

namespace gpu3d
{
namespace memorycontroller
{

class GPUMemorySpecs;

class RWFifoScheduler : public FifoSchedulerBase
{
public:

    RWFifoScheduler( const char* name,
                     const char* prefix,
                     Box* parent,
                     const CommonConfig& config
                     //u32bit maxReadTransactions,
                     //u32bit maxWriteTransaction,
                     // u32bit maxConsecutiveReads,
                     // u32bit maxConsecutiveWrites 
                     );


protected:

    void receiveRequest(u64bit cycle, ChannelTransaction* request);

    ChannelTransaction* selectNextTransaction(u64bit cycle);

    // Use this handler to implement page policy
    void handler_ddrCommandNotSent(const DDRCommand* ddrCmd, u64bit cycle);

private:

    GPUStatistics::Statistic& closePageActivationsCount; // Counts how many times the close page algorithm is activated


    // ChannelTransaction* _selectNextTransaction_Counters(u64bit cycle);
    // ChannelTransaction* _selectNextTransaction_old(u64bit cycle);

    std::vector<u32bit> pendingBankAccesses;

    // SwitchModePolicy switchMode;
    // SwitchModeCounters* smpCounters;
    SwitchOperationModeBase* som;


    const u32bit maxReadTransactions;
    const u32bit maxWriteTransactions;
    // const u32bit maxConsecutiveReads;
    // const u32bit maxConsecutiveWrites;

    u32bit consecutiveReads;
    u32bit consecutiveWrites;

    bool createInnerSignals;

    DependencyQueue readQ;
    DependencyQueue writeQ;

    void handler_endOfClock(u64bit cycle);

    void _coreDump() const;

};

} // namespace memorycontroller
} // namespace gpu3d

#endif // RWFIFOSCHEDULER_H

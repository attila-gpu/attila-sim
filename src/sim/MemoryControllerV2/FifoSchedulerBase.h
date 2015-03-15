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

#ifndef FIFOSCHEDULERBASE_H
    #define FIFOSCHEDULERBASE_H

#include "ChannelScheduler.h"
#include <queue>
#include <list>
#include "toolsQueue.h"

namespace gpu3d
{
namespace memorycontroller
{

class GPUMemorySpecs;

/**
 * Parent class of all schedulers using a Fifo-like strategy
 */
class FifoSchedulerBase : public ChannelScheduler
{
protected:


    FifoSchedulerBase(const char* name, const char* prefix, Box* parent, const ChannelScheduler::CommonConfig& config);

    // Implemented for all fifo-like schedulers
    void schedulerClock(u64bit cycle);

    /**
     * Gets the current in progress transaction
     */
    ChannelTransaction* getCurrentTransaction();

    /**
     * This method implements the transaction allocation in the fifo structure
     * implemented in the scheduler
     */ 
    virtual void receiveRequest(u64bit cycle, ChannelTransaction* request)=0;
    
    /**
     * Must be implemented in each subclass
     * This method must implement the queue strategy selection
     */
    virtual ChannelTransaction* selectNextTransaction(u64bit cycle) = 0;

    /**
     * Called when a DDRCommand in the command buffer cannot be sent
     * due to any memory constraint
     *
     * This method is provided to allow sending another command if the command
     * in the command buffer cannot be sent
     * (for example a precharge or active to another bank)
     *
     * @param ddrCmd The command not sent
     */
    virtual void handler_ddrCommandNotSent(const DDRCommand* /*ddrCmd*/, u64bit /*cycle*/) 
    { /* empty by default */ }
    virtual void handler_ddrCommandSent(const DDRCommand* /*ddrCmd*/, u64bit /*cycle*/) 
    { /* empty by default */ }

    /**
     * This method is called at the end of the current cycle simulation
     *
     * Usually used to include specific code to be performed each cycle
     *
     * (for example: managing STV feedback signals)
     */
    virtual void handler_endOfClock(u64bit /*cycle*/) { /* empty by default */ }

    /**
     * Called each time a channel transaction has been resolved (completed)
     */
    virtual void handler_transactionCompleted(const ChannelTransaction* /*ct*/, u64bit /*cycle*/)
    { /* empty by default */ }

    // Implemented here for all Fifo-like schedulers
    // (ie. automatically called by all fifo schedulers)
    void receiveData(u64bit cycle, DDRBurst* data);


private:

    enum CShedState
    {
        CSS_Idle,
        CSS_WaitPrevCmd,
        CSS_WaitPrevPageClose,
        CSS_WaitOpeningPage,
        CSS_AccessingData
    };

    CShedState cSchedState;

    struct CTAccessInfo
    {
        u64bit startCycle; ///< When a transaction starts putting/getting data to/from datapins
        u32bit remainingTransferCycles; ///< Cycles remaing for complete the transaction
        bool isRead; /// type of transaction
    };

    u32bit readDelay;
    u32bit writeDelay;
    const u32bit BytesPerCycle;
    const u32bit CyclesPerBurst;

    /**
     * Used to keep track of when data is in the datapins and be able to detect when data is being read/written
     *
     * This queue should have data only when cSchedState is equal to CSS_AccessingData
     */
    gpu3d::tools::Queue<CTAccessInfo> ongoingAccessesQueue; 

    GPUStatistics::Statistic& cSched_IdleCycles;
    GPUStatistics::Statistic& cSched_PrevCmdWaitCycles;
    GPUStatistics::Statistic& cSched_PrevPageCloseCycles;
    GPUStatistics::Statistic& cSched_OpeningPageCycles;
    GPUStatistics::Statistic& cSched_AccessingReadDelayCycles; // CAS or WL latency cycles
    GPUStatistics::Statistic& cSched_AccessingWriteDelayCycles;
    GPUStatistics::Statistic& cSched_AccessingReadDataCycles;
    GPUStatistics::Statistic& cSched_AccessingWriteDataCycles;

    // This signal is written with each time a new transaction is selected
    // This signal is read every clock
    Signal* selectedTransactionSignal;

    // Cycles in which no DDR commands are available (idle control)
    GPUStatistics::Statistic& ctrlIdleCyclesStat;

    // Cycles sending a command in the control bus
    GPUStatistics::Statistic& ctrlUsedCyclesStat;

    // Time constraint stats
    GPUStatistics::Statistic& atorStat; // Active to read penalty cycles
    GPUStatistics::Statistic& atowStat; // Active to write cycles penalty cycles
    GPUStatistics::Statistic& atoaStat; // Active to Active penalty cycles
    GPUStatistics::Statistic& ptoaStat; // Precharge to active penalty cycles
    GPUStatistics::Statistic& wtorStat; // Write to read penalty cycles
    GPUStatistics::Statistic& rtowStat; // Read to write penalty cycles
    GPUStatistics::Statistic& wtopStat; // Write to precharge penalty cycles
    GPUStatistics::Statistic& atopStat; // Active to precharge penalty cycles
    GPUStatistics::Statistic& rtopStat; // Read to precharge penalty cycles
    GPUStatistics::Statistic& rtorStat; // Read to read "penalty" cycles
    GPUStatistics::Statistic& wtowStat; // Write to write "penalty" cycles

    bool lastTransactionWasRead;
    GPUStatistics::Statistic& switchModeCount; // Statistic counting how much switches from r2w and w2r happen

    bool waitingForFirstTransactionAccess;
    u64bit cycleLastTransactionSelected;
    GPUStatistics::Statistic& sumPreAct2AccessCycles; // sum of cycles lost between 


    //GPUStatistics::Statistic& otherLostCyclesStat;

    void updateUnusedCyclesStats(DDRModuleState::IssueConstraint ic);

    /**
     * Generate all the required DDRCommands to satisfy the channeltransaction
     *
     * Return the number of burst (reads or writes) required to satisfy the transaction
     */
    u32bit fillCommandBuffer(const ChannelTransaction* ct); // , PagePolicy policy);
    
    void processNextDDRCommand(u64bit cycle);
    void processReadReply(u64bit cycle);


    u32bit BurstBytes; /// constant equivalent to BurstSize * 4

    // Pending DDRCommand to solve the last channel transaction processed
    std::list<DDRCommand*> commandBuffer;
    
    // Transaction that has generated the current commands in the command buffer
    ChannelTransaction* currentTrans;

    u32bit pendingWriteBursts; // Counter to known how many write burst remains
    // This counter is only available if the currentTrans is a write

    std::list<std::pair<ChannelTransaction*,u32bit> > inProgressReads;
    std::list<u32bit> inProgressReadBursts;

    std::queue<ChannelTransaction*> replyQ;
};

} // namespace memorycontroller
} // namespace gpu3d

#endif

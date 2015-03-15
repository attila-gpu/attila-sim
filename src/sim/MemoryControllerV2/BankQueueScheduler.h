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

#ifndef BANKQUEUESCHEDULER_H
    #define BANKQUEUESCHEDULER_H

#include "FifoSchedulerBase.h"
#include "BankSelectionPolicy.h"
#include <list>
#include <vector>
#include "SwitchOperationMode.h"

namespace gpu3d
{
namespace memorycontroller
{

class GPUMemorySpecs;

// Helper class implementing a queue with some "special" added features
class TQueue
{
public:

    TQueue();

    u32bit getConsecutiveAccesses(bool writes) const;

    bool empty() const;

    u32bit size() const;

    u64bit getTimestamp() const;

    void enqueue(ChannelTransaction* ct, u64bit timestamp);

    ChannelTransaction* front() const;

    void pop();

    void setName(const std::string& name);

    const std::string& getName() const;

private:

    struct QueueEntry
    {
        ChannelTransaction* ct;
        u64bit timestamp;
        QueueEntry(ChannelTransaction* ct, u64bit timestamp) : ct(ct), timestamp(timestamp)
        {}
        QueueEntry() : ct(0), timestamp(0) {}
    };

    typedef std::list<QueueEntry> Queue;

    Queue q;
    u32bit qSize; // to increase efficiency in implementations with STL list.size() O(n)
    std::string qName;

};

class BankQueueScheduler : public FifoSchedulerBase
{
public:

    BankQueueScheduler( const char* name,
                        const char* prefix,
                        Box* parent,
                        const CommonConfig& config
                        // u32bit maxConsecutiveReads,
                        // u32bit maxConsecutiveWrites 
                        );

protected:

    void receiveRequest(u64bit cycle, ChannelTransaction* request);
    
    // Reimplement diferent policies
    ChannelTransaction* selectNextTransaction(u64bit cycle);

    // use this handler to precharge or active a row when a DDRCommand
    // can not be issued
    void handler_ddrCommandNotSent(const DDRCommand* ddrCmd, u64bit cycle);

    // sets state
    void handler_endOfClock(u64bit cycle);

private:

    GPUStatistics::Statistic& closePageActivationsCount; // Counts how many times the close page algorithm is activated

    bool banksShareState;

    // SwitchModeCounters* smpCounters;
    SwitchOperationModeBase* som;

    enum AdvancedActiveMode
    {
        AAM_Conservative = 0,
        AAM_Agressive = 1
    };

    AdvancedActiveMode aam;

    // import types
    typedef BankSelectionPolicy::BankInfo BankInfo;
    typedef BankSelectionPolicy::BankInfoArray BankInfoArray;  

    std::vector<BankInfo> queueBankInfos;
    BankInfoArray queueBankPointers;

    BankSelectionPolicy* bankSelector;

    bool disableActiveManager;
    bool disablePrechargeManager;
    u32bit managerSelectionAlgorithm;

    bool useInnerSignals;
    const u32bit bankQueueSz;

    std::vector<TQueue> bankQ; // one queue per bank    

    u32bit lastSelected;

    void _coreDump() const;

    struct Candidates
    {
        u32bit read;
        u32bit write;
        bool readIsHit;
        bool writeIsHit;
    };

    bool _countHits(u32bit bankIgnored, u32bit& readHits, u32bit& writeHits) const;

    Candidates _findCandidateTransactions(BankInfoArray& queueBankPointers);

    std::vector<u32bit> getBankPriority(BankInfoArray& bia) const;

    bool _tryAdvancedPrecharge(const DDRCommand* notSentCommand, u64bit cycle);

    bool _tryAdvancedActive(const DDRCommand* notSentCommand, u64bit cycle);

    // Generic
    bool _tryAdvancedActive(const DDRCommand* notSentCommand, u64bit cycle, bool tryRead, const std::vector<u32bit>& bankOrder);


    // DEPRECATED METHODS
    // ChannelTransaction* _selectNextTransaction_Counters(u64bit cycle);
    // bool _tryAdvancedPrecharge_Counters(const DDRCommand* notSentCommand, u64bit cycle);
    // bool _tryAdvancedActive_Counters(const DDRCommand* notSentCommand, u64bit cycle);



};

} // namespace memorycontroller
} // namespace gpu3d

#endif // BANKQUEUESCHEDULER_H

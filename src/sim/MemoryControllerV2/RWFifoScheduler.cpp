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

#include "RWFifoScheduler.h"
#include "ChannelTransaction.h"

using namespace gpu3d::memorycontroller;
using namespace std;

RWFifoScheduler::RWFifoScheduler(const char* name,
                    const char* prefix,
                    Box* parent,
                    const CommonConfig& config
                    // u32bit maxConsecutiveReads_,
                    // u32bit maxConsecutiveWrites_
                    ) :
FifoSchedulerBase(name, prefix, parent, config),
maxReadTransactions( config.dedicatedChannelReadTransactions == 0 ? (config.maxChannelTransactions / 2) + 1 : 
                                                                    config.dedicatedChannelReadTransactions + 1),
maxWriteTransactions(config.dedicatedChannelReadTransactions == 0 ? (config.maxChannelTransactions / 2) + 1 : 
                                                                    (config.maxChannelTransactions - config.dedicatedChannelReadTransactions) + 1),
// maxConsecutiveReads(config.smpMaxConsecutiveReads), maxConsecutiveWrites(maxConsecutiveWrites_),
consecutiveReads(0), consecutiveWrites(0),
createInnerSignals(config.createInnerSignals),
pendingBankAccesses(config.moduleBanks, 0),
closePageActivationsCount(getSM().getNumericStatistic("ClosePageActivationCount", u32bit(0), "FifoSchedulerBase", prefix))
{
    GPU_ASSERT(
        if ( config.dedicatedChannelReadTransactions >= config.maxChannelTransactions ) {
            stringstream ss;
            ss << "The number of dedicated read transactions (" << config.dedicatedChannelReadTransactions 
               << ") must be strictly less than MaxChannelTransactions (" << config.maxChannelTransactions << ")";
            panic("RWFifoScheduler", "ctor", ss.str().c_str());
        }
    )

        som = SwitchOperationModeSelector::create(config);

    /*
    if ( config.switchModePolicy == SMP_Counters )
        smpCounters = new SwitchModeCounters(config.smpMaxConsecutiveReads, config.smpMaxConsecutiveReads);
    else
        panic("RWFifoScheduler", "ctor", "Only SMP_Counters(=0) supported for now");
    */

    // create inner signals here
}


void RWFifoScheduler::handler_endOfClock(u64bit cycle)
{
    // Set state according to queue occupation
    bool readAccept = readQ.size() < maxReadTransactions - 1 /*getRequestBandwidth()*/;
    bool writeAccept =  writeQ.size() < maxWriteTransactions - 1 /*getRequestBandwidth()*/;
    if ( readAccept && writeAccept )
        setState(new SchedulerState(SchedulerState::AcceptBoth));
    else if ( readAccept )
        setState(new SchedulerState(SchedulerState::AcceptRead));
    else if ( writeAccept )
        setState(new SchedulerState(SchedulerState::AcceptWrite));
    else
        setState(new SchedulerState(SchedulerState::AcceptNone));
}

void RWFifoScheduler::receiveRequest(u64bit cycle, ChannelTransaction* request)
{
    const u32bit bank = request->getBank();

    if ( bank >= moduleState().banks() ) {
        _coreDump();
        panic("RWFifoScheduler", "receiveRequest", "Bank ID too high");
    }

    // keep track of pending bank accesses
    pendingBankAccesses[bank]++;

    if ( request->isRead() )
    {
        if ( readQ.size() == maxReadTransactions )
        {
            _coreDump();
            panic("RWFifoScheduler", "receiveRequest", "Read fifo queue full");
        }
        // Compute RAW dependency
        ChannelTransaction* dependency = writeQ.findDependency(request);
        readQ.enqueue(request, dependency);

    }
    else
    {
        if ( writeQ.size() == maxWriteTransactions )
        {
            _coreDump();
            panic("RWFifoScheduler", "receiveRequest", "Write fifo queue full");
        }
        // compute WAR depedency
        ChannelTransaction* dependency = readQ.findDependency(request);
        writeQ.enqueue(request, dependency);
    }
}
/*
ChannelTransaction* RWFifoScheduler::_selectNextTransaction_Counters(u64bit cycle)
{
   if ( readQ.empty() && writeQ.empty() )
        return 0;

    bool readReady = false;
    bool writeReady = false;

    if ( !readQ.empty() )
        readReady = readQ.ready();

    if ( !writeQ.empty() )
        writeReady = writeQ.ready();

    if ( !readReady && !writeReady ) // DEADLOCK detected (should not happen)
    {
        _coreDump();
        panic("RWFifoScheduler", "selectNextTransaction", "Deadlock. Read and write queue blocked");
    }

   
    if ( smpCounters->reading() ) {
        if ( !readReady || ( smpCounters->moreConsecutiveOpsAllowed() == 0 && writeReady ) )
            smpCounters->switchMode();
    }
    else {
        if ( !writeReady || ( smpCounters->moreConsecutiveOpsAllowed() == 0 && readReady ) )
            smpCounters->switchMode();
    }

    ChannelTransaction* trans;
    if ( smpCounters->reading() ) {
        trans = readQ.front();
        readQ.pop();
        writeQ.wakeup(trans);
    }
    else {
        trans = writeQ.front();
        writeQ.pop();
        readQ.wakeup(trans);
    }
    smpCounters->newOp();

    return trans;
}
*/


ChannelTransaction* RWFifoScheduler::selectNextTransaction(u64bit cycle)
{
    // ChannelTransaction* ct = _selectNextTransaction_Counters(cycle);    

   if ( readQ.empty() && writeQ.empty() )
        return 0;

    bool readReady = false;
    bool writeReady = false;
    bool readIsHit = false;
    bool writeIsHit = false;

    if ( !readQ.empty() ) {
        const ChannelTransaction* ct = readQ.front();
        readReady = readQ.ready();
        readIsHit = moduleState().getActiveRow(ct->getBank()) == ct->getRow() && readReady;
    }

    if ( !writeQ.empty() ) {
        const ChannelTransaction* ct = writeQ.front();
        writeReady = writeQ.ready();
        writeIsHit = moduleState().getActiveRow(ct->getBank()) == ct->getRow() && writeReady;
        
    }

    GPU_ASSERT(
        if ( !readReady  && !writeReady ) {
            _coreDump();
            panic("RWFifoScheduler", "selectNextTransaction", "Reads or writes exist but any is ready => DEADLOK");
        }
    )

    som->update(readReady, writeReady, readIsHit, writeIsHit); 

    GPU_ASSERT(
        if ( !readReady && som->reading() ) {
            cout << "readReady=" << readReady << " writeReady=" << writeReady << "  readHit=" << readIsHit << " writeHit=" << writeIsHit << endl;
            panic("RWFifoScheduler", "selectNextTransaction", "No ready read transaction and switch mode set to READ!");
        }
        if ( !writeReady && !som->reading() ) {
            cout << "readReady=" << readReady << " writeReady=" << writeReady << "  readHit=" << readIsHit << " writeHit=" << writeIsHit << endl;
            panic("RWFifoScheduler", "selectNextTransaction", "No ready write transaction and switch mode set to WRITE!");
        }
    
    )

    ChannelTransaction* ct;

    if ( som->reading() ) {
        ct = readQ.front();
        readQ.pop();
        writeQ.wakeup(ct);
    }
    else {
        ct = writeQ.front();
        writeQ.pop();
        readQ.wakeup(ct);
    }

    
    GPU_ASSERT(
        if ( pendingBankAccesses[ct->getBank()] == 0 ) {
            _coreDump();
            panic("RWFifoScheduler", "selectNextTransaction", "Internal error: pendingBankAccesses counter failed!!!");
        }
    )
    --pendingBankAccesses[ct->getBank()];    

    return ct;
}


void RWFifoScheduler::handler_ddrCommandNotSent(const DDRCommand* notSentCommand, u64bit cycle)
{
    // Apply page policy
    if ( getPagePolicy() == ClosePage ) {
        const u32bit Banks = moduleState().banks();
        u32bit bank = ( notSentCommand ? notSentCommand->getBank() : Banks );
        for ( u32bit i = 0; i < Banks; ++i ) {
            if ( bank == i )
                continue; // skip -> bank used by current transaction
            if ( pendingBankAccesses[i] == 0 && moduleState().getActiveRow(i) != DDRModuleState::NoActiveRow )
            {
                // close page as soon as no requests pending to this row/page are available
                DDRCommand* preCommand = createPrecharge(i);
                if ( notSentCommand )
                    preCommand->setProtocolConstraint(notSentCommand->getProtocolConstraint());
                //string icstr = DDRModuleState::getIssueConstraintStr(moduleState().getIssueConstraint(i, DDRModuleState::C_Precharge));
                if ( !sendDDRCommand(cycle, preCommand, 0) )
                    delete preCommand; // cannot be issued
                else {
                    closePageActivationsCount.inc();
                    return ; // no more commands can be issued this cycle
                }
            }
        }
    }
}

void RWFifoScheduler::_coreDump() const
{
    using std::cout;
    cout << "\n-----------------------------------------\n";
    cout << "Read queue size: " << readQ.size() << "\n";
    if ( !readQ.empty() )
    {
        cout << " Transaction has dependency: "
            << ( !readQ.ready() ? "YES":"NO" ) << "\n";
        readQ.front()->dump();
        cout << "\n";
        if ( readQ.frontDependency() )
        {
            cout << "  Dependency: ";
            readQ.frontDependency()->dump();
            cout << "\n";
        }
    }
    cout << "Write queue size: " << writeQ.size() << "\n";
    if ( !writeQ.empty() )
    {
        cout << " Transaction has dependency: "
            << ( !writeQ.ready() ? "YES":"NO" ) << "\n";
        writeQ.front()->dump();
        cout << "\n";
        if ( writeQ.frontDependency() )
        {
            cout << "  Dependency: ";
            writeQ.frontDependency()->dump();
            cout << "\n";
        }
    }
    //cout << "Consecutive reads counter: " << consecutiveReads << "\n";
    //cout << "Consecutive writes counter: " << consecutiveWrites << "\n";
    cout << "-----------------------------------------\n";
}

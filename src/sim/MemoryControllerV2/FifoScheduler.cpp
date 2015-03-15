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

#include "FifoScheduler.h"

// temporary include
#include "MemoryRequest.h"

#include <sstream>

using namespace std;
using namespace gpu3d::memorycontroller;

void FifoScheduler::processStateQueueSignals(u64bit cycle)
{
    DynamicObject* dynObj;

    // read and delete previous inner dynamic objects
    while ( queueSignal->read(cycle, dynObj) )
        delete dynObj;

    u32bit color = 0;
    
    ChannelTransaction* currentTrans = getCurrentTransaction();

    // Use signals for monitoring queue state
    if ( currentTrans != 0 )
    {
        // Queue colors based on the queue size
        // 0 -> 100 % queue full
        // 1 -> up to 25 %
        // 2 -> up to 50 %
        // 3 -> up to 75 %
        // 4 -> up to 100 % (full not included)
        if ( queueTrack.size() != maxSizeQ ) 
        {
            f32bit col = (1.0f + (f32bit)queueTrack.size()) / ((f32bit)maxSizeQ + 1.0f);
            u32bit col2 = (u32bit)(col * 100);
            if ( col2 <= 25 )
                color = 1;
            else if ( col2 <= 50 )
                color = 2;
            else if ( col2 <= 75 )
                color = 3;
            else // (col2 < 100)
                color = 4;
        }
        // else color = 0

        dynObj = new DynamicObject;
        dynObj->setColor(color); // selected transaction (current)
        dynObj->copyParentCookies(*currentTrans);
        stringstream ss;
        ss << currentTrans->toString() << " (queue size = " << queueTrack.size() + 1 << ")";
        strcpy((char *)dynObj->getInfo(), ss.str().c_str());
        queueSignal->write(cycle, dynObj);
    }

    // code not included within the previous IF to find possible programming errors
    for ( list<ChannelTransaction*>::iterator it = queueTrack.begin();
          it != queueTrack.end(); it++ )
    {
        GPU_ASSERT(
            if ( currentTrans == 0 )
                panic("FifoScheduler", "processStateQueueSignals", "Queue inconsistency");
        )
        dynObj = new DynamicObject;
        dynObj->setColor(color); // ready transaction in the queue not yet selected
        dynObj->copyParentCookies(**it);
        strcpy((char*)dynObj->getInfo(), (*it)->toString().c_str());
        queueSignal->write(cycle, dynObj);
    }
}


FifoScheduler::FifoScheduler(const char* name, const char* prefix, Box* parent, const CommonConfig& config) : // , u32bit maxTransactions) :
FifoSchedulerBase(name, prefix, parent, config),
maxSizeQ(config.maxChannelTransactions),
useInnerSignals(config.createInnerSignals),
pendingBankAccesses(config.moduleBanks, 0),
closePageActivationsCount(getSM().getNumericStatistic("ClosePageActivationCount", u32bit(0), "FifoSchedulerBase", prefix))
{
    if ( config.createInnerSignals )
    {
        // "currentTrans + maxTransactions" signals
        queueSignal = newInputSignal("FifoEntry", maxSizeQ + 1, 1, prefix);
        queueSignal = newOutputSignal("FifoEntry", maxSizeQ + 1, 1, prefix);
    }
}

void FifoScheduler::handler_ddrCommandNotSent(const DDRCommand* notSentCommand, u64bit cycle)
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
                if ( !sendDDRCommand(cycle, preCommand, 0) ) 
                    delete preCommand; // cannot be issued
                else {
                    closePageActivationsCount.inc();
                    break;
                }
            }
        }
    }
    // nothing to do with OpenPage
}


ChannelTransaction* FifoScheduler::selectNextTransaction(u64bit cycle)
{
	if ( transQ.empty() )
        return 0;

    ChannelTransaction* ct = transQ.front();
    transQ.pop();

    // start STV feedback code
    if ( useInnerSignals && isSignalTracingRequired(cycle) )
        queueTrack.pop_front();
    // end STV feedback code

    GPU_ASSERT(
        if ( pendingBankAccesses[ct->getBank()] == 0 ) {
            panic("FifoScheduler", "selectNextTransaction", "Internal error: pendingBankAccesses counter failed!!!");
        }
    )
    --pendingBankAccesses[ct->getBank()];        

    return ct;
}

void FifoScheduler::receiveRequest(u64bit cycle, ChannelTransaction* request)
{
    if ( transQ.size() == maxSizeQ )
        panic("FifoScheduler", "receiveRequest", "Fifo scheduler queue is full");

    transQ.push(request);

    const u32bit bank = request->getBank();
    if ( bank >= moduleState().banks() ) {
        panic("FifoScheduler", "receiveRequest", "Bank ID too high");
    }
    pendingBankAccesses[bank]++;
    
    // start STV feedback code
    if ( useInnerSignals && isSignalTracingRequired(cycle) )
        queueTrack.push_back(request);
    // end STV feedback code
}

void FifoScheduler::handler_endOfClock(u64bit cycle)
{
    if ( transQ.size() < maxSizeQ - 1 /*getRequestBandwidth()*/ )
        setState(new SchedulerState(SchedulerState::AcceptBoth));
    else
        setState(new SchedulerState(SchedulerState::AcceptNone));

    if ( useInnerSignals && isSignalTracingRequired(cycle) )
        processStateQueueSignals(cycle);
}


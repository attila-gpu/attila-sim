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

#include "FifoSchedulerBase.h"

#include <vector>
#include <iostream>

using std::cout;

using namespace gpu3d::memorycontroller;
using std::make_pair;

FifoSchedulerBase::FifoSchedulerBase(const char* name, const char* prefix, Box* parent, const ChannelScheduler::CommonConfig& config):
    ChannelScheduler(name, prefix, parent, config),
    BurstBytes(config.burstLength*4), currentTrans(0), cSchedState(CSS_Idle),
ongoingAccessesQueue(),
// BytesPerCycle(4 * burstElementsPerCycle),
BytesPerCycle(config.burstBytesPerCycle),
// CyclesPerBurst(burstLength / burstElementsPerCycle),
CyclesPerBurst((4*config.burstLength)/config.burstBytesPerCycle),
cSched_IdleCycles(getSM().getNumericStatistic(prefix, u32bit(0),"ChannelScheduler", "ChSched_IdleCycles")),
cSched_PrevCmdWaitCycles(getSM().getNumericStatistic(prefix, u32bit(0), "ChannelScheduler", "ChSched_PrevCmdWaitCycles")),
cSched_PrevPageCloseCycles(getSM().getNumericStatistic(prefix, u32bit(0),"ChannelScheduler", "ChSched_PrevPageCloseCycles")),
cSched_OpeningPageCycles(getSM().getNumericStatistic(prefix, u32bit(0),"ChannelScheduler", "ChSched_OpeningPageCycles")),
cSched_AccessingReadDelayCycles(getSM().getNumericStatistic(prefix, u32bit(0),"ChannelScheduler", "ChSched_AccessingReadDelayCycles")),
cSched_AccessingReadDataCycles(getSM().getNumericStatistic(prefix, u32bit(0),"ChannelScheduler", "ChSched_AccessingReadDataCycles")),
cSched_AccessingWriteDelayCycles(getSM().getNumericStatistic(prefix, u32bit(0),"ChannelScheduler", "ChSched_AccessingWriteDelayCycles")),
cSched_AccessingWriteDataCycles(getSM().getNumericStatistic(prefix, u32bit(0),"ChannelScheduler", "ChSched_AccessingWriteDataCycles")),

ctrlIdleCyclesStat(getSM().getNumericStatistic("ctrlIdleCycles", u32bit(0),"FifoSchedulerBase", prefix)),

ctrlUsedCyclesStat(getSM().getNumericStatistic("ctrlUsedCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),

atorStat(getSM().getNumericStatistic("act2readCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
atowStat(getSM().getNumericStatistic("act2writeCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
atoaStat(getSM().getNumericStatistic("act2actCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
ptoaStat(getSM().getNumericStatistic("pre2actCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
wtorStat(getSM().getNumericStatistic("write2readCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
rtowStat(getSM().getNumericStatistic("read2writeCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
wtopStat(getSM().getNumericStatistic("write2preCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
atopStat(getSM().getNumericStatistic("act2preCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
rtopStat(getSM().getNumericStatistic("read2preCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
rtorStat(getSM().getNumericStatistic("read2readCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
wtowStat(getSM().getNumericStatistic("write2writeCycles", u32bit(0),
                                              "FifoSchedulerBase", prefix)),
switchModeCount(getSM().getNumericStatistic("SwitchModeCount", u32bit(0), "FifoSchedulerBase", prefix)),
lastTransactionWasRead(false),
cycleLastTransactionSelected(0),
waitingForFirstTransactionAccess(false),
sumPreAct2AccessCycles(getSM().getNumericStatistic("sumPreAct2AccessCycles", u32bit(0), "FifoSchedulerBase", prefix))
//commandBufferTransaction(0)
{
    if ( config.memSpecs.memtype() != GPUMemorySpecs::GDDR3 ) {
        panic("FifoSchedulerBase", "ctor", "Only GDDR3 memory supported now");
    }

    // GDDR3 memory 
    const GDDR3Specs& gddr3Specs = static_cast<const GDDR3Specs&>(config.memSpecs);
    readDelay  = gddr3Specs.CASLatency;
    writeDelay = gddr3Specs.WriteLatency;

    GPU_ASSERT(
        if ( config.burstLength == 0 || (config.burstLength & (config.burstLength - 1)) != 0) {
            stringstream ss;
            ss << "BurstLength has to be a power of two and it is not: BL=" << config.burstLength;
            panic("FifoSchedulerBase", "ctor", ss.str().c_str());
        }
        if ( BurstBytes % config.burstBytesPerCycle != 0 ) {
            stringstream ss;
            ss << "BurstBytes = BL * 4 (" << config.burstLength << "*4=" << (config.burstLength*4) 
               << ") has to be divisible by BytesPerCycle (" << config.burstBytesPerCycle << ")";
            panic("FifoSchedulerBase", "ctor", ss.str().c_str());
        }
    )
    
    ongoingAccessesQueue.resize(std::max(readDelay, writeDelay) + CyclesPerBurst + 1);

    selectedTransactionSignal = newInputSignal("SelectedTransaction", 1, 1, prefix);
    // ignore return value, only selectedTransactionSignal private var used to read and write the signal
    newOutputSignal("SelectedTransaction", 1, 1, prefix);
}


ChannelTransaction* FifoSchedulerBase::getCurrentTransaction()
{
    return currentTrans;
}

void FifoSchedulerBase::schedulerClock(u64bit cycle)
{
    DynamicObject* dummy;
    // ignore return, this signal is used to provide STV feedback only
    selectedTransactionSignal->read(cycle, dummy);

    if ( commandBuffer.empty() ) // check if the command buffer has pendent ddr commands
    {
        currentTrans = selectNextTransaction(cycle);
        if ( currentTrans )
        {
            waitingForFirstTransactionAccess = true;
            cycleLastTransactionSelected = cycle;
            if ( currentTrans->isRead() != lastTransactionWasRead ) {
                switchModeCount.inc();
                lastTransactionWasRead = currentTrans->isRead();
            }            
            

            // Change internal scheduler state
            cSchedState = CSS_WaitPrevCmd;

            // STV feedback
            selectedTransactionSignal->write(cycle, currentTrans);
            // Fill the command buffer with the DDR commands required to satisfy
            // the current channel transaction
            u32bit burstsCount = fillCommandBuffer(currentTrans); //, getPagePolicy());

            GPU_ASSERT( 
                if ( commandBuffer.empty() ) {
                    panic("FifoSchedulerBase", "schedulerClock", "Current transaction has not generated DDR commands!");
                }
            )

            //if ( commandBuffer.front()->which() == DDRCommand::Read || commandBuffer.front()->which() == DDRCommand::Write ) {
                // Start accessing data
                // cSchedState = CSS_AccessingData;
            //}

            if ( currentTrans->isRead() )
            {
                inProgressReads.push_back(make_pair(currentTrans, burstsCount));
                inProgressReadBursts.push_back(0);
            }
            else // currentTrans is a write
                pendingWriteBursts = burstsCount;
        }
        else
            cSchedState = CSS_Idle;
    }


    // try to send the next command
    processNextDDRCommand(cycle);

    // send the next completed read reply
    processReadReply(cycle);

    // Call end of clock handler
    handler_endOfClock(cycle);

    /////////////////////////////////////////
    // Scheduler State statistics handling //
    /////////////////////////////////////////

    if ( !ongoingAccessesQueue.empty() ) {
        // at least we've got one ongoing transmission
        u64bit& startCycle = ongoingAccessesQueue.head().startCycle; // create an alias for startCycle
        u32bit& remainingCycles = ongoingAccessesQueue.head().remainingTransferCycles; // create an alias for remaning burst cycles
        if ( startCycle <= cycle ) { // Starts now or previously started
            if ( ongoingAccessesQueue.head().isRead ) 
                cSched_AccessingReadDataCycles.inc();
            else
                cSched_AccessingWriteDataCycles.inc();
            --remainingCycles;
            if ( remainingCycles == 0 ) {
                // deallocate from ongoing accesses queue
                ongoingAccessesQueue.remove();
            }            
        }
        else { // startCycle > cycle
            if ( ongoingAccessesQueue.head().isRead ) 
                cSched_AccessingReadDelayCycles.inc();
            else
                cSched_AccessingWriteDelayCycles.inc();
        }
    }
    else { // No ongoing data access
        switch ( cSchedState ) {
            case CSS_Idle:
                cSched_IdleCycles.inc();
                break;
            case CSS_WaitPrevCmd:
                cSched_PrevCmdWaitCycles.inc();
                break;
            case CSS_WaitPrevPageClose:
                cSched_PrevPageCloseCycles.inc();
                break;
            case CSS_WaitOpeningPage:
                cSched_OpeningPageCycles.inc();
                break;
            case CSS_AccessingData:
                panic("FifoSchedulerBase", "schedulerClock", "Inconsistency between 'cSchedState' AND 'ongoingAccessesQueue'");
                break;
        }
    }

}


u32bit FifoSchedulerBase::fillCommandBuffer(const ChannelTransaction* currentTrans)
{
    using std::vector;

    // generate DDR Commands required to resolve this transaction
    CommandList commands = transactionToCommands(currentTrans);
    u32bit burstCount = static_cast<u32bit>(commands.size());
    CommandList::const_iterator it = commands.begin();
    u32bit transRow = currentTrans->getRow();

    const u32bit Banks = moduleState().banks();

    vector<u32bit> rowOpen(Banks);
    for ( u32bit i = 0; i < Banks; ++i ) 
        rowOpen[i] = moduleState().getActiveRow(i);

    for ( ; it != commands.end(); ++it ) {
        DDRCommand* cmd = *it;
        u32bit bank = cmd->getBank();
        if ( rowOpen[bank] != transRow ) {
            if ( rowOpen[bank] != DDRModuleState::NoActiveRow )
                commandBuffer.push_back(createPrecharge(bank));
            commandBuffer.push_back(createActive(bank, transRow));
            rowOpen[bank] = transRow;
        }
        commandBuffer.push_back(cmd);
    }

    return burstCount;
}

void FifoSchedulerBase::receiveData(u64bit cycle, DDRBurst* data)
{
    // data received from the attached module
    GPU_ASSERT(
        if ( inProgressReads.empty() )
            panic("FifoSchedulerBase", "receiveData", 
                  "Not in progress transactions (shoudn't happen ever)");
    )

    ChannelTransaction* trans = inProgressReads.front().first;

    u32bit bytes;
    
    if ( trans->bytes() % BurstBytes != 0 
             && inProgressReadBursts.front() + 1 == inProgressReads.front().second )
         bytes = trans->bytes() % BurstBytes;
    else
        bytes = BurstBytes;

    trans->setData(data->getBytes(), bytes, inProgressReadBursts.front() * BurstBytes);
    
    inProgressReadBursts.front()++;

    if ( inProgressReadBursts.front() == inProgressReads.front().second )
    {
        replyQ.push(trans);
        inProgressReads.pop_front();
        inProgressReadBursts.pop_front();

        // call "transaction completed" handler
        handler_transactionCompleted(trans, cycle);
    }

    delete data; // bye bye burst
}


void FifoSchedulerBase::processReadReply(u64bit cycle)
{
    if ( replyQ.empty() )
        return ;

    ChannelTransaction* trans = replyQ.front();
    replyQ.pop();
    sendReply(cycle, trans);

}

void FifoSchedulerBase::processNextDDRCommand(u64bit cycle)
{
    if ( commandBuffer.empty() )
    {
        ctrlIdleCyclesStat.inc();
        handler_ddrCommandNotSent(0, cycle);
        return ; // nothing to be processed
    }

    DDRCommand* cmd = commandBuffer.front();

    // try to send a new DDR Command
    if ( sendDDRCommand(cycle, cmd, currentTrans) )
    {
        // update control bus stats
        ctrlUsedCyclesStat.inc();

        // if the command was successfully sent, remove it from the buffer
        commandBuffer.pop_front();
        switch ( cmd->which() ) 
        {
            case DDRCommand::Write:
                {
                    CTAccessInfo info;
                    info.startCycle = cycle + writeDelay;
                    info.remainingTransferCycles = CyclesPerBurst;
                    info.isRead = false;
                    ongoingAccessesQueue.add(info);
                }

                if ( waitingForFirstTransactionAccess ) {
                    sumPreAct2AccessCycles.inc( cycle - cycleLastTransactionSelected );
                    waitingForFirstTransactionAccess = false;
                }

                cSchedState = CSS_AccessingData;
                pendingWriteBursts--;
                if ( pendingWriteBursts == 0 )
                {
                    replyQ.push(currentTrans); // Write transaction is completed
                    handler_transactionCompleted(currentTrans, cycle); // call "transaction completed" handler
                }
                break;
            case DDRCommand::Read:
                {
                    CTAccessInfo info;
                    info.startCycle = cycle + readDelay;
                    info.remainingTransferCycles = CyclesPerBurst;
                    info.isRead = true;
                    ongoingAccessesQueue.add(info);
                }

                if ( waitingForFirstTransactionAccess ) {
                    sumPreAct2AccessCycles.inc( cycle - cycleLastTransactionSelected );
                    waitingForFirstTransactionAccess = false;
                }

                cSchedState = CSS_AccessingData;
                break;
            case DDRCommand::Active:
                cSchedState = CSS_WaitOpeningPage;
                if ( !commandBuffer.empty() && commandBuffer.front()->which() == DDRCommand::Read ) {
                    cmd->setProtocolConstraint(DDRCommand::PC_a2r);
                }
                else if ( !commandBuffer.empty() && commandBuffer.front()->which() == DDRCommand::Write ) {
                    cmd->setProtocolConstraint(DDRCommand::PC_a2w);
                }
                break;
            case DDRCommand::Precharge:
                cSchedState = CSS_WaitPrevPageClose;
                break;
            default:
                panic("FifoSchedulerBase", "processNextDDRCommand", "Unexpected command");
        }
        handler_ddrCommandSent(cmd, cycle);
    }
    else
    {
        // update lost cycles
        DDRModuleState::CommandID cmdID;
        switch ( cmd->which() )
        {
            case DDRCommand::Active:
                cmdID = DDRModuleState::C_Active;
                break;
            case DDRCommand::Read:
                cmdID = DDRModuleState::C_Read;
                break;
            case DDRCommand::Write:
                cmdID = DDRModuleState::C_Write;
                break;
            case DDRCommand::Precharge:
                cmdID = DDRModuleState::C_Precharge;
                break;
            default:
                cmdID = DDRModuleState::C_Unknown;
                panic("FifoSchedulerBase", "processNextDDRCommand", "Unexpected command");
        }
        
        DDRModuleState::IssueConstraint ic = 
            moduleState().getIssueConstraint(cmd->getBank(), cmdID);       

        // Classify unused cycles
        updateUnusedCyclesStats(ic);


        DDRCommand::ProtocolConstraint pc = DDRCommand::PC_none;
        // Convert issue constraint
        switch ( ic )
        {
            // ACT to command constraints
            case DDRModuleState::CONSTRAINT_ACT_TO_ACT:
                pc = DDRCommand::PC_a2a;
                break;
            case DDRModuleState::CONSTRAINT_ACT_TO_PRE:
                pc = DDRCommand::PC_a2p;
                break;
            case DDRModuleState::CONSTRAINT_ACT_TO_READ:
                pc = DDRCommand::PC_a2r;
                break;
            case DDRModuleState::CONSTRAINT_ACT_TO_WRITE:
                pc = DDRCommand::PC_a2w;
                break;
            // READ to command constraints
            case DDRModuleState::CONSTRAINT_READ_TO_WRITE:
                pc = DDRCommand::PC_r2w;
                break;
            case DDRModuleState::CONSTRAINT_READ_TO_PRE:
                pc = DDRCommand::PC_w2p;
                break;
            // WRITE to command constraints
            case DDRModuleState::CONSTRAINT_WRITE_TO_READ:
                pc = DDRCommand::PC_w2r;
                break;
            case DDRModuleState::CONSTRAINT_WRITE_TO_PRE:
                pc = DDRCommand::PC_w2p;
                break;
            // PRE to command constraints
            case DDRModuleState::CONSTRAINT_PRE_TO_ACT:
                pc = DDRCommand::PC_p2a;
                break;
        }

        cmd->setProtocolConstraint(pc);
        // if a new command is sent from this handler it should copy the protocol constraint of the not-sent command
        handler_ddrCommandNotSent(cmd, cycle);
        cmd->setProtocolConstraint(DDRCommand::PC_none); // reset constraint

        if ( pc != DDRCommand::PC_none ) {
            DDRCommand* dummy = DDRCommand::createDummy(pc);
            if ( !sendDDRCommand(cycle, dummy, currentTrans) )
                delete dummy;
        }
        
    }
}



void FifoSchedulerBase::updateUnusedCyclesStats(DDRModuleState::IssueConstraint ic)
{
    switch ( ic )
    {
        case DDRModuleState::CONSTRAINT_ACT_TO_READ:
            atorStat.inc(); break;
        case DDRModuleState::CONSTRAINT_ACT_TO_WRITE:
            atowStat.inc(); break;
        case DDRModuleState::CONSTRAINT_PRE_TO_ACT:
            ptoaStat.inc(); break;
        case DDRModuleState::CONSTRAINT_ACT_TO_ACT:
            atoaStat.inc(); break;
        case DDRModuleState::CONSTRAINT_WRITE_TO_READ:
            wtorStat.inc(); break;
        case DDRModuleState::CONSTRAINT_READ_TO_WRITE:
            rtowStat.inc(); break;
        case DDRModuleState::CONSTRAINT_WRITE_TO_PRE:
            wtopStat.inc(); break;
        case DDRModuleState::CONSTRAINT_ACT_TO_PRE:
            atopStat.inc(); break;
        case DDRModuleState::CONSTRAINT_READ_TO_PRE:
            rtopStat.inc(); break;
        case DDRModuleState::CONSTRAINT_DATA_BUS_CONFLICT:
            rtorStat.inc(); break; // Using this stat to account for data bus conflicts (stat name should be changed)
        default:
            panic("FifoSchedulerBase", "updateUnusedCyclesStats", "Unexpected CONSTRAINT");            
    }
}

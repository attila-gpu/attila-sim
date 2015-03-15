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

#include "DDRModule.h"
#include "DDRCommand.h"
#include <sstream>
#include <deque>
#include <iostream>
#include "GPUMemorySpecs.h"


#ifdef GPU_DEBUG
    #undef GPU_DEBUG
#endif
//#define GPU_DEBUG(expr) { expr }
#define GPU_DEBUG(expr) {  }

#define ASSERT_DATAPINS_PUSH(funcname, nextLastCycle) \
    GPU_ASSERT(\
        if ( !dataPinsItem.empty() && dataPinsItem.back().first >= nextLastCycle ) {\
            stringstream ss;\
            ss << "Conflict in DDRDataPins. Next last DataPinsItem processed at cycle = " << nextLastCycle << " <= "\
               << "Previous last DataPinsItem at cycle = " << dataPinsItem.back().first;\
                dump(); panic("DDRModule", funcname, ss.str().c_str());\
        }\
    )

using namespace std;
using namespace gpu3d::memorycontroller;

void DDRModule::setModuleParameters( const GPUMemorySpecs& memSpecs, u32bit burstLength_ )
{
    switch ( memSpecs.memtype() ) {
        case GPUMemorySpecs::GDDR3:
            {
                const GDDR3Specs& params = static_cast<const GDDR3Specs&>(memSpecs);
                tRRD = params.tRRD;
                tRCD = params.tRCD;
                tWTR = params.tWTR;
                tRTW = params.tRTW;
                tWR = params.tWR;
                tRP = params.tRP;
                CASLatency = params.CASLatency;
                WriteLatency = params.WriteLatency;
                burstLength = burstLength_;                
            }
            break;
        case GPUMemorySpecs::GDDR4:
            panic("DDRModuleState", "setModuleParameters", "GDDR4 memory not yet supported");
        case GPUMemorySpecs::GDDR5:
            panic("DDRModuleState", "setModuleParameters", "GDDR5 memory not yet supported");
        default:
            panic("DDRModuleState", "setModuleParameters", "Unknown memory");

    }
}


DDRModule::DDRModule(const char* name, const char* prefix, u32bit burstLength, 
                     u32bit nBanks, u32bit bankRows, u32bit bankCols,
                     //u32bit burstElementsPerCycle,
                     u32bit burstBytesPerCycle,
                     const GPUMemorySpecs& memSpecs,
                     Box* parentBox) : 
burstLength(burstLength),
Box(name, parentBox), nBanks(nBanks), bankRows(bankRows), bankCols(bankCols),
lastClock(0), lastCmd(DDRCommand::Unknown),
lastActiveBank(0), lastActiveStart(0), lastActiveEnd(0),
lastReadBank(0), lastReadStart(0), lastReadEnd(0),
lastWriteBank(0), lastWriteStart(0), lastWriteEnd(0),
//burstElementsPerCycle(burstElementsPerCycle),
burstBytesPerCycle(burstBytesPerCycle),
burstTransmissionTime((4*burstLength)/burstBytesPerCycle),
//burstTransmissionTime(burstLength/burstElementsPerCycle),
bypassConstraint(0),
// Statistics creation
transmissionCyclesStat(getSM().getNumericStatistic("DataCycles", u32bit(0), "DDRModule", prefix)),
transmissionBytesStat(getSM().getNumericStatistic("DataBytes", u32bit(0), "DDRModule", prefix)),
idleCyclesStat(getSM().getNumericStatistic("IdleCycles", u32bit(0), "DDRModule", prefix)),
readTransmissionCyclesStat(getSM().getNumericStatistic("ReadDataCycles", u32bit(0), "DDRModule", prefix)),
readTransmissionBytesStat(getSM().getNumericStatistic("ReadDataBytes", u32bit(0), "DDRModule",  prefix)),
writeTransmissionCyclesStat(getSM().getNumericStatistic("WriteDataCycles", u32bit(0), "DDRModule", prefix)),
writeTransmissionBytesStat(getSM().getNumericStatistic("WriteDataBytes", u32bit(0), "DDRModule", prefix)),
casCyclesStat(getSM().getNumericStatistic("WastedCycles_CASLatency", u32bit(0), "DDRModule", prefix)),
wlCyclesStat(getSM().getNumericStatistic("WastedCycles_WriteLatency", u32bit(0), "DDRModule", prefix)),
allBanksPrechargedCyclesStat(getSM().getNumericStatistic("AllBanksPrechargedCycles", u32bit(0), "DDRModule", prefix)),
actCommandsReceived(getSM().getNumericStatistic("ACTCommandsReceived", u32bit(0), "DDRModule", prefix))
{

    for ( u32bit i = DDRCommand::PC_none + 1; i < DDRCommand::PC_count; ++i ) {
        string constraintStatName = "WastedCycles_" + 
                                    DDRCommand::protocolConstraintToString(static_cast<DDRCommand::ProtocolConstraint>(i));
        constraintCyclesStats[i] = &getSM().getNumericStatistic(constraintStatName.c_str(), u32bit(0), "DDRModule", prefix);
    }

    GPU_ASSERT
    (
        if ( nBanks == 0 )
            panic("DDRModule", "ctor", "Number of banks cannot be 0");
        if ( prefix == 0 )
            panic("DDRModule", "ctor", "A prefix must be specified");
    )


    setModuleParameters(memSpecs, burstLength);

    banks.resize(nBanks, DDRBank(bankRows, bankCols));
    bankState = new BankState[nBanks];
    for ( u32bit i = 0; i < nBanks; i++ )
    {
        bankState[i].autoprecharge = false;
        bankState[i].state = BS_Idle;
        bankState[i].lastWriteEnd = 0;
        bankState[i].endCycle = 0;
    }

    // Create interface signals
    cmdSignal = newInputSignal("DDRModuleRequest", 1, 1, prefix);
    replySignal = newOutputSignal("DDRModuleReply", 1, 1, prefix);

    // Create inner signals
    // Input/out signal
    dataPinsSignal = newInputSignal("DDRModuleDataPins", 1, 1, prefix);
    newOutputSignal("DDRModuleDataPins", 1, 1, prefix);

    moduleInfoSignal = newInputSignal("DDRModuleInfo", 1, 1, prefix);
    newOutputSignal("DDRModuleInfo", 1, 1, prefix);
}

void DDRModule::preload(u32bit bankID, u32bit row, u32bit startCol, 
    const u8bit* data, u32bit dataSz, const u32bit* mask)
{
    if ( bankID >= nBanks )
        panic("DDRModule", "preload", "Bank identifier too high");
    
    DDRBank& theBank = banks[bankID];
    u32bit oldActive = theBank.getActive();
    theBank.activate(row);

    // Compute burst bytes (4 bytes read for each burst item)
    const u32bit burstBytes = 4*burstLength;

    // Get the number of bursts required to satisfy the preload invocation
    u32bit burstCount = dataSz / burstBytes;

    for ( u32bit i = 0; i < burstCount; i++ )
    {
        DDRBurst burst(burstLength);
        burst.setData(data + burstBytes*i, burstBytes);
        if ( mask != 0 )
            burst.setMask(mask + burstLength*i, burstLength);
        // Apply instant write
        theBank.write(startCol, &burst);
        startCol += burstLength; // select next start column
    }

    // a last partial burst required
    if ( dataSz % burstBytes != 0 )
    {
        DDRBurst burst(burstLength);
        burst.setData(data + burstBytes*burstCount, dataSz % burstBytes);
        if ( mask != 0 )
            burst.setMask(mask + burstLength*burstCount, (dataSz % burstBytes)/4);
        theBank.write(startCol, &burst); 
    }

    theBank.activate(oldActive); // restore previous activated row
}

bool DDRModule::isAnyBank(BankStateID state) const
{
    for ( u32bit i = 0; i < nBanks; ++i ) {
        if ( bankState[i].state == state )
            return true;
    }
    return false;
}

// Implements automatic (pasive) state transitions
void DDRModule::updateBankState(u64bit cycle, u32bit bankId)
{
    BankState& bstate = bankState[bankId];
    
    if ( bstate.state == BS_Idle || bstate.state == BS_Active )
        return ; // no "pasive" state change is posible
        
    if ( cycle < bstate.endCycle )
        return ; // no state updating required until bstate.endCycle

    // This two vars are used to generate endCycle for autoprecharging
    u64bit extraDelay = 0;
    bool updateEndCycle = false;
    
    // { cycle >= bstate.newStateCycle }
    // State transition completed, change bank state
    switch ( bstate.state )
    {
        case BS_Activating:
            /*
            GPU_DEBUG
            ( 
                cout << "Cycle: " << cycle 
                     << " -> Bank state change: Activating -> Active";
            )
            */
            bstate.state = BS_Active;
            break;
        case BS_Reading:
            /*
            GPU_DEBUG
            (
                cout << "Cycle: " << cycle 
                     << " -> Bank state change: Reading -> ";
            )
            */
            if ( bstate.autoprecharge )
            {
                if ( tRP + 1 <= CASLatency + burstTransmissionTime )
                {
                    // autoprecharge is fully overlapped with the read command
                    //GPU_DEBUG( cout << "Idle"; )
                    bstate.state = BS_Idle;
                }
                else 
                {
                    // { P: tRP + 1 > CASLatency + burstTransmissionTime }
                    extraDelay = tRP + 1 - CASLatency - burstTransmissionTime;
                    // there is additional delay due to autoprecharge
                    if ( cycle >= bstate.endCycle + extraDelay )
                    {
                        //GPU_DEBUG( cout << "Idle"; )
                        bstate.state = BS_Idle;
                    }
                    else
                    {
                        //GPU_DEBUG( cout << "Precharging"; )
                        bstate.state = BS_Precharging;
                        updateEndCycle = true;
                    }
                }
                bstate.autoprecharge = false; // reset autoprecharge bit
            }
            else
            {
                //GPU_DEBUG( cout << "Active"; )
                bstate.state = BS_Active;
            }
            break;
        case BS_Writing:
            /*
            GPU_DEBUG
            (
                cout << "Cycle: " << cycle 
                     << " -> Bank state change: Writing -> ";
            )
            */
            if ( bstate.autoprecharge )
            {
                extraDelay = tWR + tRP;
                if ( cycle >= bstate.endCycle + extraDelay )
                {
                    //GPU_DEBUG( cout << "Idle"; )
                    bstate.state = BS_Idle;
                }
                else
                {
                    //GPU_DEBUG( cout << "Precharging"; )
                    bstate.state = BS_Precharging;
                    updateEndCycle = true;
                }
                bstate.autoprecharge = false; // reset autoprecharge bit
            }
            else
            {
                //GPU_DEBUG( cout << "Active"; )
                bstate.state = BS_Active;
            }
            break;
        case BS_Precharging:
            /*
            GPU_DEBUG
            (
                cout << "Cycle: " << cycle 
                     << " -> Bank state change: Precharging -> Idle";
            )
            */
            bstate.state = BS_Idle;
            banks[bankId].deactivate();
            break;
        default:
            panic("DDRModule", "updateBankState", "Unexpected state");
    }

    if ( updateEndCycle )
        bstate.endCycle += extraDelay;
}

void DDRModule::clock(u64bit cycle)
{
    GPU_DEBUG( printf("%s => Clock "U64FMT".\n", getName(), cycle); )

    bool allBanksPrecharged = true;
    for ( u32bit i = 0; i < nBanks; i++ ) {
        updateBankState(cycle, i);  // Update bank state
        if ( bankState[i].state != BS_Idle )
            allBanksPrecharged = false;
    }

    if ( allBanksPrecharged )
        allBanksPrechargedCyclesStat.inc();

    // Process commands
    DDRCommand* cmd;
    if ( cmdSignal->read(cycle, (DynamicObject*& )cmd) ) { // a new command has been issued in "cycle - 1"
        processCommand(cycle, cmd);
    }

    GPU_DEBUG( 
        cout << getName() << " => Banks state: {";
        for ( u32bit i = 0; i < nBanks; i++ )
        {
            cout << i << "=" << getStateStr(i);
            if ( i < nBanks-1 )
                cout << ",";
        }
        cout << "}\n";
    )

    // send data from banks (if any)
    if ( !readout.empty() )
    {
        u64bit nextCycle = readout.front().first;
        if ( nextCycle < cycle )
        {
            dump();
            stringstream ss;
            ss << "Data not sent in a previous cycle: " << nextCycle 
               << ". Current = " << cycle;
            panic("DDRModule", "clock", ss.str().c_str());
            
        }
        else if ( nextCycle == cycle )
        {
            GPU_DEBUG ( 
                {
                    DDRBurst* b = readout.front().second;
                    cout << getName() << " => Reading out " << (4 * b->getSize())
                         << " bytes.\n";
                }
            )
            replySignal->write(cycle, readout.front().second);
            readout.pop_front();
        }
        // else (No data to be sent)
    }

    if ( !readin.empty() ) {
        u64bit nextCycle = readin.front().first;
        if ( nextCycle < cycle ) {
            dump();
            stringstream ss;
            ss << "Data not written in a previous cycle: " << nextCycle << ". Current cycle = " << cycle;
            panic("DDRModule", "clock", ss.str().c_str());            
        }
        else if ( nextCycle == cycle ) {
            // cout << "Cycle=" << cycle << ". DDRModule::clock() -> Burst written" << endl;
            GPU_DEBUG(
                {
                    DDRBurst* b = readin.front().second;
                    cout << getName() << " => Writing " << (4 * b->getSize()) << " bytes.\n";
                }
            )
            delete readin.front().second; // Get rid of DDRBurst
            readin.pop_front();
        }
        // else (nothing to be done)
    }

    // Write/Read inner signals
    static DataPinItem* prevDynObj = 0;
    DataPinItem* dynObj;
    if ( dataPinsSignal->read(cycle, (DynamicObject*&)dynObj) )
    {
        if ( dynObj->whattype() == DataPinItem::IS_READ )
        {
            transmissionCyclesStat++; // inc data pins transmission stats
            readTransmissionCyclesStat++;
            //readTransmissionBytesStat.inc(4*burstElementsPerCycle);
            readTransmissionBytesStat.inc(burstBytesPerCycle);
        }
        else if ( dynObj->whattype() == DataPinItem::IS_WRITE )
        {
            transmissionCyclesStat++; // inc data pins transmission stats
            writeTransmissionCyclesStat++;
            // writeTransmissionBytesStat.inc(4*burstElementsPerCycle);
            writeTransmissionBytesStat.inc(burstBytesPerCycle);
        }
        else { // dynObj->whatis == DataPinItem::IS_PROTOCOL_CONSTRAINT
            if ( dynObj->whattype() == DataPinItem::IS_CAS_LATENCY )
                casCyclesStat++;
            else if ( dynObj->whattype() == DataPinItem::IS_WRITE_LATENCY )
                wlCyclesStat++;
            else if ( dynObj->whattype() == DataPinItem::IS_PROTOCOL_CONSTRAINT ) {
                constraintCyclesStats[dynObj->_whatis - DataPinItem::STV_COLOR_PROTOCOL_CONSTRAINT_BASE]->inc();
            }
            else {
                dump();
                panic("DDRModule", "clock", "Unknown DataPinItem!");
            }
        }
        // Identificar reads & writes
        if ( dynObj != prevDynObj ) // next burst (destroy previous item)
        {
            delete prevDynObj; // Consume DataPinsElement
            prevDynObj = dynObj; 
        }       
    }
    else
        idleCyclesStat++;

    // Check the pending data pin accesses.If any ready, send it thru dataPinsSignal
    // Return true if something has been sent thru dataPinsSignal, false otherwise
    bool discardConstraint = processDataPins(cycle);

    if ( discardConstraint ) { // No constraint applicable this cycle, data is being sent thru data pins (discard the constraint if exists)
        if ( bypassConstraint ) {
            delete bypassConstraint; // delete/discard the constraint supplied externally 
            bypassConstraint = 0;
        }
    }
    else // Check if a constraint has to be sent thru dataPinsSignal, since data pins cannot be used
        processDataPinsConstraints(cycle);
    
    lastClock = cycle; // record last cycle


    //////////////////////////////////////////////////////
    /// Code used to feed STV with banks state changes ///
    //////////////////////////////////////////////////////
    if ( moduleInfoSignal ) {

        DDRModuleInfo* minfo;
        if ( moduleInfoSignal->read(cycle, (DynamicObject*&)minfo) ) {
            lastModuleInfoString = (const char*)minfo->getInfo();
            delete minfo;
        }

        stringstream inf;
        inf << "(prev. cycle) Banks => ";
        for ( u32bit i = 0; i < nBanks; ++i ) {
            inf << "b[" << i << "]=" << getStateStr(i);
            BankStateID bstate = bankState[i].state;
            switch ( bstate ) {
                case BS_Active:
                case BS_Activating:
                case BS_Reading:
                case BS_Writing:
                    inf << banks[i].getActive();
                    break;
            }
            if ( i < nBanks - 1 )
                inf << " ";
        }

        minfo = new DDRModuleInfo();
        strcpy((char *)minfo->getInfo(), inf.str().c_str());
        if ( inf.str() != lastModuleInfoString )
            minfo->setColor(minfo->getColor() + 1); // Change color to reflect a state change
        moduleInfoSignal->write(cycle, minfo);
    }
}


bool DDRModule::processDataPins(u64bit cycle)
{
    if ( dataPinsItem.empty() )
        return false;
    
    u32bit elemCycle = dataPinsItem.front().first;
    if ( elemCycle == cycle ) {
        dataPinsSignal->write(cycle, dataPinsItem.front().second);
        dataPinsItem.pop_front();
        return true; // dataPinsItem processed
    }
    else if ( elemCycle < cycle ) {
        dump();
        stringstream ss;
        ss << "Cycle=" << cycle << ". DataPinElem LOST (ElemCycle=" << elemCycle << "). " << (char*)dataPinsItem.front().second->getInfo();
        panic("DDRModule", "clock", ss.str().c_str());
    }
    return false; // nothing processed
}

bool DDRModule::processDataPinsConstraints(u64bit cycle)
{
    bool externalSchedulerConstraintsFirst = false;

    if ( externalSchedulerConstraintsFirst ) {
        // Process constraints received from DDRModuleRequest interface signals
        if ( bypassConstraint ) {
            dataPinsSignal->write(cycle, bypassConstraint);
            bypassConstraint = 0;
            return true;
        }
    }

    // Process constraint generated internally by the DDRModule (such as CAS latency and WL)
    // bool breading = isAnyBank(BS_Reading);
    // bool bwriting = isAnyBank(BS_Writing);
    bool breading = lastReadEnd  != 0 && lastReadEnd  > cycle;
    bool bwriting = lastWriteEnd != 0 && lastWriteEnd > cycle;

    // { no data in output-pins (implied when we are here)  }

    if ( breading || bwriting ) { 
        /*
        // This is not a panic anymore, can happen when switching from read to write
        if ( breading && bwriting ) {
            dump();
            panic("DDRModule","processDataPinsConstraints", "reading & writing simultaneously");
        }
        */
        DataPinItem* latencyConstraint = 0;
        if ( bwriting ) { // if bwriting && breading -> This combination can only be seen from read to write, so write is younger and has to be processed first
            latencyConstraint = new DataPinItem(DataPinItem::STV_COLOR_WL);
            latencyConstraint->setColor(DataPinItem::STV_COLOR_WL);
            sprintf((char*)latencyConstraint->getInfo(), "Write Latency");
            if ( readin.empty() ) {
                dump();
                panic("DDRModule", "processDataPinsConstraints", "Inconsistency, seeing Write latency but not to-be-written data is pending!");
            }
            latencyConstraint->copyParentCookies(*readin.front().second);
        }
        else { //  breading 
            latencyConstraint = new DataPinItem(DataPinItem::STV_COLOR_CAS);
            latencyConstraint->setColor(DataPinItem::STV_COLOR_CAS);
            sprintf((char*)latencyConstraint->getInfo(), "CAS Latency");
            if ( readout.empty() ) {
                dump();
                panic("DDRModule", "processDataPinsConstraints", "Inconsistency, seeing CAS latency but not read data is pending!");
            }
            latencyConstraint->copyParentCookies(*readout.front().second);
        }
        dataPinsSignal->write(cycle, latencyConstraint);

        if ( !externalSchedulerConstraintsFirst ) {
            // In this case, we give priority to constraints internally generated by DDRModules over constraints sent by schedulers
            if ( bypassConstraint ) { 
                delete bypassConstraint; // discard the externally supplied constraint
                bypassConstraint = 0;
            }
        }

        return true;
    }
    else {
        // { externalSchedulerConstraintsFirst == false }
        if ( bypassConstraint ) {
            dataPinsSignal->write(cycle, bypassConstraint);
            bypassConstraint = 0;
            return true;
        }
    }

    return false;
}


void DDRModule::processCommand(u64bit cycle, DDRCommand* cmd)
{
    if ( cmd->which() != DDRCommand::Dummy ) {
        // Dummy commands are ignored
        if ( listOfLastestProcessedCommands.size() >= LIST_OF_LATESTS_COMMANDS_MAX_SIZE ) {
            delete listOfLastestProcessedCommands.front().second; // Delete command
            listOfLastestProcessedCommands.pop_front(); // Remove from the list
        }
        listOfLastestProcessedCommands.push_back(make_pair(cycle, cmd));
    }

    preprocessCommand(cycle, cmd);
    switch ( cmd->which() )
    {
        case DDRCommand::Active:
            GPU_DEBUG(
                cout << getName() << " => DDRCommand received. Executing ACTIVE bank=" 
                     << cmd->getBank() << " row=" << cmd->getRow() << ".\n";
            )
            actCommandsReceived.inc(); // Keep a counter of ACT commands (to compute our power model)
            processActiveCommand(cycle, cmd);
            break;
        case DDRCommand::Read:
            GPU_DEBUG(
                cout << getName() << " => DDRCommand received. Executing READ bank=" 
                     << cmd->getBank() << " col=" << cmd->getColumn() << ".\n";
            )
            processReadCommand(cycle, cmd);
            break;
        case DDRCommand::Write:
            GPU_DEBUG(
                cout << getName() << " => DDRCommand received. Executing WRITE bank=" 
                     << cmd->getBank() << " col=" << cmd->getColumn() << ".\n";
            )
            processWriteCommand(cycle, cmd);
            break;
        case DDRCommand::Precharge:
            GPU_DEBUG(
                cout << getName() << " => DDRCommand received. Executing PRECHARGE bank=";
                if ( cmd->getBank() == DDRCommand::AllBanks )
                    cout << "ALL.\n";
                else
                    cout << cmd->getBank() << ".\n";
            )
            processPrechargeCommand(cycle, cmd);
            break;
        case DDRCommand::Dummy:
            GPU_DEBUG(
                cout << getName() << " => DDRCommand received. Processing " << cmd->toString() << "\n"
            )
            processDummyCommand(cycle, cmd);
            break;
        default:
            panic("DDRModule", "processCommand", "Not implemented command");
    }
}


void DDRModule::preprocessCommand(u64bit cycle, const DDRCommand* ddrCommand)
{
    DDRCommand::ProtocolConstraint pc = ddrCommand->getProtocolConstraint();
    if ( ddrCommand->getProtocolConstraint() != DDRCommand::PC_none ) {
        bypassConstraint = new DataPinItem(DataPinItem::DataPinItemColor(DataPinItem::STV_COLOR_PROTOCOL_CONSTRAINT_BASE + pc));
        bypassConstraint->copyParentCookies(*ddrCommand);
        bypassConstraint->setColor(DataPinItem::STV_COLOR_PROTOCOL_CONSTRAINT_BASE + pc);
        sprintf((char*)bypassConstraint->getInfo(), DDRCommand::protocolConstraintToString(pc).c_str());
    }
}


void DDRModule::processDummyCommand(u64bit cycle, DDRCommand* dummyCommand)
{
    // bool breading = isAnyBank(BS_Reading);
    // bool bwriting = isAnyBank(BS_Writing);
    bool breading = lastReadEnd  != 0 && lastReadEnd  > cycle;
    bool bwriting = lastWriteEnd != 0 && lastWriteEnd > cycle;

    if ( breading || bwriting ) {
        // if chip is reading or writing the effect of the constraint will be seen later in the data pins
        // If not reading or writing, then preprocess command will set the external constraint this cycle
        DDRCommand::ProtocolConstraint pc  = dummyCommand->getProtocolConstraint();
        u32bit offset = 0;
        switch ( pc ) {
            case DDRCommand::PC_r2w:
            case DDRCommand::PC_a2w:
                offset = WriteLatency;
                break;
            case DDRCommand::PC_a2r:
                offset = CASLatency;
                break;
            default:
                ; // nothing to do
        }
        if ( offset != 0 ) {
            DataPinItemColor dpic = static_cast<DataPinItemColor>(DataPinItem::STV_COLOR_PROTOCOL_CONSTRAINT_BASE + pc);
            DataPinItem* dynObj = new DataPinItem(dpic);
            dynObj->copyParentCookies(*dummyCommand);
            dynObj->setColor(DataPinItem::STV_COLOR_PROTOCOL_CONSTRAINT_BASE + pc);
            sprintf((char*)dynObj->getInfo(), "%s", DDRCommand::protocolConstraintToString(pc).c_str());
            ASSERT_DATAPINS_PUSH("processDummyCommand", cycle + offset)
            dataPinsItem.push_back(make_pair(cycle + offset, dynObj));
        }
    }    
    delete dummyCommand; // This is the only command that can be delete right here
}

// { P: activeCommand->which() == DDRCOmmand::Active }
void DDRModule::processActiveCommand(u64bit cycle, DDRCommand* activeCommand)
{
    u32bit bankId = activeCommand->getBank();

    GPU_ASSERT
    (
        if ( bankId == DDRCommand::AllBanks )
            panic("DDRModule", "processActiveCommand", "'bank = AllBanks' not accepted with Active command");
        if ( bankId >= nBanks )
            panic("DDRModule", "processActiveCommand", "bank identifier too high");
    )

    if ( bankState[bankId].state != BS_Idle )
    {
        stringstream ss;
        ss << "The bank " << bankId << " is not IDLE, cannot be activated";
        dump();
        panic("DDRModule", "processActiveCommand", ss.str().c_str());

    }

    if ( lastActiveEnd != 0 && lastActiveStart + tRRD > cycle ) {
        dump();
        panic("DDRModule", "processActiveCommand", 
              "tRRD violated between two ACTIVE commands");
    }

    // update global state
    lastActiveBank = bankId;
    lastActiveStart = cycle;
    lastActiveEnd = cycle + tRCD;

    // update local bank state
    banks[bankId].activate(activeCommand->getRow());
    bankState[bankId].state = BS_Activating;
    bankState[bankId].endCycle = lastActiveEnd;

    // delete activeCommand;
}

void DDRModule::processReadCommand(u64bit cycle, DDRCommand* readCommand)
{
    u32bit bankId = readCommand->getBank();

    if ( bankId == DDRCommand::AllBanks )
        panic("DDRModule", "processReadCommand", 
              "'bank = AllBanks' not accepted with Read command");

    if ( bankId >= nBanks )
        panic("DDRModule", "processReadCommand", "bank identifier too high");

    if ( bankState[bankId].state != BS_Active && bankState[bankId].state != BS_Reading ) {
        dump();
        panic("DDRModule", "processReadCommand", 
              "Only in ACTIVE or READING state the bank can be read");
    }

    // check if a previous read/write had autoprecharge enabled
    if ( bankState[bankId].autoprecharge )
        panic("DDRModule", "processReadCommand", "Performing a previous read/write with autoprecharge. Read not accepted");
    
    // check conflict with previous writes (check write to read delay restriction)
    if ( lastWriteEnd + tWTR > cycle ) {
        dump();
        panic("DDRModule", "processReadCommand", "write to read delay violated");
    }

    // check read conflicts with previous in progress reads
    if ( !readout.empty() )
    {
        if ( readout.back().first > cycle + CASLatency ) {
            dump();
            panic("DDRModule", "processReadCommand", 
                  "data readout collision between two reads");
        }
    }
    
    for ( u32bit i = 0; i < burstTransmissionTime; i++ )
    {
        /*
        stringstream ss;        
        u32bit firstCol = readCommand->getColumn() + i * burstElementsPerCycle;
        u32bit firstCol = readCommand->getColumn() + i * burstElementsPerCycle;
        u32bit firstCol = readCommand->getColumn() + i * burstElementsPerCycle;
        if ( burstElementsPerCycle == 1 )
            ss << "col[" << firstCol << "]";
        else
            ss << "cols[" << firstCol << ".." << (firstCol + burstElementsPerCycle - 1) << "]";
        */

        // cout << "Cycle=" << cycle << ". Adding READ Data into dataPinsItem" << endl;
        DataPinItem* dynObj = new DataPinItem(DataPinItem::STV_COLOR_READ); // isRead = true
        dynObj->copyParentCookies(*readCommand);
        dynObj->setColor(DataPinItem::STV_COLOR_READ); // burst reads identified with a 0
        // sprintf((char*)dynObj->getInfo(), "%s", ss.str().c_str());
        ASSERT_DATAPINS_PUSH("processReadCommand", cycle + i + CASLatency)
        dataPinsItem.push_back(make_pair(cycle + i + CASLatency, dynObj));
    }

    // update global state
    lastReadBank = bankId;
    lastReadStart = cycle;
    lastReadEnd = cycle + CASLatency + burstTransmissionTime;

    // update local bank state
    BankState& bstate = bankState[bankId];
    bstate.state = BS_Reading;
    bstate.endCycle = lastReadEnd;
    bstate.autoprecharge = readCommand->autoprechargeEnabled();

    // Perform emulation access
    lastColumnAccessed = readCommand->getColumn();
    DDRBurst* burst = banks[bankId].read(readCommand->getColumn(), burstLength);
    
    // Copy the cookies from the read command generating the read burst
    burst->copyParentCookies(*readCommand);

    // queue the burst into the readout queue
    readout.push_back(make_pair(bstate.endCycle, burst));

    // delete readCommand;
}


void DDRModule::processWriteCommand(u64bit cycle, DDRCommand* writeCommand)
{
    u32bit bankId = writeCommand->getBank();

    if ( bankId == DDRCommand::AllBanks )
        panic("DDRModule", "processWriteCommand", 
              "'bank = AllBanks' not accepted with Write command");
    if ( bankId >= nBanks )
        panic("DDRModule", "processWriteCommand", "bank identifier too high");

    if ( writeCommand->getColumn() >= banks[bankId].columns() )
    {
        cout << "Max. columns = " << banks[bankId].columns() << "\n";
        writeCommand->dump();
        panic("DDRModule", "processWriteCommand", "Column out of bounds");

    }

    if ( lastWriteEnd > cycle + WriteLatency ) {
        dump();
        panic("DDRModule", "processWriteCommand", "Write collision with a previous write (both overlapping the usage of data pins)");
    }

    if ( lastReadEnd > cycle + WriteLatency ) {
        dump();
        panic("DDRModule", "processWriteCommand", "Write collision with a previous read (both overlapping the usage of data pins)");
    }

    // check read to write delay
    if ( cycle + WriteLatency < lastReadEnd + tRTW )
    {
        dump();
        stringstream ss;
        ss << "In a read DQ must be unused for at least " << tRTW << " cycles using DQ pins for writing";
        panic("DDRModule", "processWriteCommand", ss.str().c_str());
    }

    if ( bankState[bankId].state == BS_Activating ) {
        dump();
        panic("DDRModule", "processWriteCommand", "Trying to write data in a row being opened");
    }

    if ( bankState[bankId].state == BS_Idle || bankState[bankId].state == BS_Precharging ) {
        dump();
        panic("DDRModule", "processWriteCommand", "Trying to write data in a bank that is being precharged or is closed");
    }

    if ( bankState[bankId].autoprecharge ) {
        dump();
        panic("DDRModule", "processWriteCommand", "Performing a previous read/write with autoprecharge. Read not accepted");
    }


    // check write after write delay
    if ( !readin.empty() )
    {
        if ( readin.back().first > cycle + WriteLatency ) {
            dump();
            panic("DDRModule", "processWriteCommand", "data readin collision between two writes");
        }
    }

    for ( u32bit i = 0; i < burstTransmissionTime; i++ )
    {
        /*
        stringstream ss;
        u32bit firstCol = writeCommand->getColumn() + i * burstElementsPerCycle;
        if ( burstElementsPerCycle == 1 )
            ss << "col[" << firstCol << "]";
        else
            ss << "cols[" << firstCol << ".." << (firstCol + burstElementsPerCycle - 1) << "]";
        */
        DataPinItem* dynObj = new DataPinItem(DataPinItem::STV_COLOR_WRITE); // isRead = false (it is a write)
        dynObj->copyParentCookies(*writeCommand);
        dynObj->setColor(DataPinItem::STV_COLOR_WRITE); // burst writes identified with a 1
        // sprintf((char*)dynObj->getInfo(), "%s", ss.str().c_str());
        ASSERT_DATAPINS_PUSH("processWriteCommand", cycle + i + WriteLatency)
        dataPinsItem.push_back(make_pair(cycle + i + WriteLatency, dynObj));
    }

    // update global state
    lastWriteBank = bankId;
    lastWriteStart = cycle;
    lastWriteEnd = cycle + WriteLatency + burstTransmissionTime;

    // update local bank state
    BankState& bstate = bankState[bankId];
    bstate.state = BS_Writing;
    bstate.endCycle = lastWriteEnd;
    bstate.lastWriteEnd = lastWriteEnd;
    bstate.autoprecharge = writeCommand->autoprechargeEnabled();

    // Perform emulation access
    lastColumnAccessed = writeCommand->getColumn(); 

    u32bit col = writeCommand->getColumn();

    // We own the burst now so we can cast away
    DDRBurst* burst = const_cast<DDRBurst*>(writeCommand->getData());

    // delete writeCommand; // delete the write command (write command cannot access its former burst anymore)

    banks[bankId].write(writeCommand->getColumn(), burst);

    // Copy the cookies from the read command generating the read burst
    burst->copyParentCookies(*writeCommand);

    // readin.push_back(lastWriteEnd);
    readin.push_back(make_pair(lastWriteEnd, burst));

    // delete writeCommand->getData(); // delete associated burst to the write command
}

void DDRModule::processPrechargeCommand(u64bit cycle, DDRCommand* prechargeCommand)
{
    u32bit bankId = prechargeCommand->getBank();
    if (bankId == DDRCommand::AllBanks )
    {
        // precharge all
        panic("DDRModule", "processPrechargeCommand", "Precharge ALL not implemented yet");
    }

    if ( bankState[bankId].autoprecharge )
    {
        // ignore precharge, previous autoprecharge will be performed
        return;
    }

    BankStateID state = bankState[bankId].state;

    // Check write to precharge delay constraint (also covers state == BS_Writing)
    // This check must be performed before state == BS_Idle || state == BS_Precharging
    if ( bankState[bankId].lastWriteEnd + tWR > cycle )
    {
        cout << "Cycle: " << cycle << "\n";
        cout << "bankState.lastWriteEnd = " << bankState[bankId].lastWriteEnd << "\n";
        cout << "tWR = " << tWR << "\n";
        cout << "bankState.lastWriteEnd + tWR > cycle\n";
        cout << bankState[bankId].lastWriteEnd << " + " << tWR << " > " << cycle << "\n";
        dump();
        panic("DDRModule", "processPrechargeCommand", "write to precharge delay within the same bank violated");
    }

    if ( state == BS_Idle || state == BS_Precharging )
        return; // interpreted as a NOP
    else if ( state == BS_Activating ) {
        dump();
        panic("DDRModule", "processPrechargeCommand", "Bank being activated. Can not be precharged");
    }
    else if ( state == BS_Reading )
    {
        if ( bankState[bankId].endCycle > cycle + tRP )
        {
            cout << "bankState[bankId].endCycle > cycle + tRP\n";
            cout << bankState[bankId].endCycle << " > " << cycle << " + " << tRP << "\n";
            cout << bankState[bankId].endCycle << " > " << cycle + tRP << "\n";
            dump();
            panic("DDRModule", "processPrechargeCommand", 
                  "Read to precharge delay violated");
        }
    }
    // else: writing state check is covered by "write to precharge" delay test

    GPU_ASSERT
    (
        if ( state != BS_Active && state != BS_Reading ) {
            dump();
            panic("DDRModule", "processPrechargeCommand", "Bank Active/Reading states expected");
        }
        
    )

    // { P: state == BS_Active || state == BS_Reading }

    bankState[bankId].state = BS_Precharging;
    bankState[bankId].endCycle = cycle + tRP;

    // delete prechargeCommand;
}


void DDRModule::printOperatingParameters() const
{
    cout << "-----------------------------------------------------\n";
    cout << "DDRModule: " << getName() << " Operating parameters:";
    cout << "tRRD: " << tRRD << " -> Active bank x to Active bank y delay\n";
    cout << "tRCD: " << tRCD << " -> Active to Read/Write delay\n";
    cout << "tWTR: " << tWTR << " -> Write to Read delay\n";
    cout << "tRTW: " << tRTW << " -> Read to Write unused cycles required in the data bus\n";
    cout << "tWR: " << tWR << " -> Write recovery penalty (Write after Precharge)\n";
    cout << "tRP: " << tRP << " -> Row Precharge delay\n";
    cout << "CAS latency: " << CASLatency << "\n";
    cout << "Write latency: " << WriteLatency << "\n";
    cout << "Current burst length accepted: " << burstLength << "\n";
    cout << "-----------------------------------------------------\n";
}

DDRBank& DDRModule::getBank(u32bit bankId)
{
    GPU_ASSERT
    (
        if ( bankId >= nBanks )
            panic("DDRModule", "getBank", "Bank identifier too high");
    )
    return banks[bankId];
}

u32bit DDRModule::countBanks() const
{
    return nBanks;
}

void DDRModule::dump(bool bankContents, bool txtFormat) const
{
    cout << "\n----------------------------------------------\n";
    cout << "DDR module = \"" << getName() << "\" state in cycle: " << lastClock << "\n";
    for ( u32bit i = 0; i < nBanks; i++ )
    {
        cout << " Bank " << i << " -> state: ";
        switch ( bankState[i].state )
        {
            case BS_Idle:
                cout << "Idle\n"; break;
            case BS_Activating:
                cout << "Activating row = " << banks[i].getActive()
                     << " (" << (bankState[i].endCycle - lastClock) 
                     << " remaining cycles)\n";
                break;
            case BS_Active:
                cout << "Active, row = " << banks[i].getActive() << "\n"; break;
            case BS_Reading:
                cout << "Reading" << (bankState[i].autoprecharge ? "A" : "" )
                    << " row = " << banks[i].getActive()
                    << "  column: " << lastColumnAccessed
                    << " (" << (bankState[i].endCycle - lastClock)
                    << " remaining cycles)\n";
                break;
            case BS_Writing:
                cout << "Writing" << ( bankState[i].autoprecharge ? "A" : "" )
                    << " row = " << banks[i].getActive()
                    << "  column: " << lastColumnAccessed
                    << " (" << (bankState[i].endCycle - lastClock)
                    << " remaining cycles)\n";
                break;
            case BS_Precharging:
                cout << "Precharging row = " << banks[i].getActive() << " (" 
                     << (bankState[i].endCycle - lastClock) << " remaining cycles)\n";
                break;
            default:
                panic("DDRModule", "dump", "Unknown state");
        }
        if ( bankContents )
        {
            if ( txtFormat )
                banks[i].dump(DDRBank::txt);
            else
                banks[i].dump(DDRBank::hex);
        }
    }
    cout << " LastRead  [bank=" << lastReadBank << "] -> StartCycle = " << lastReadStart << "   EndCycle = " << lastReadEnd << "\n";
    cout << " LastWrite [bank=" << lastWriteBank << "] -> StartCycle = " << lastWriteStart << "   EndCycle = " << lastWriteEnd << "\n";
    cout << " Latest " << listOfLastestProcessedCommands.size() << " received DDR Commands:\n";
    std::deque<std::pair<u64bit,DDRCommand*> >::const_iterator it = listOfLastestProcessedCommands.begin();
    for ( ; it != listOfLastestProcessedCommands.end(); ++it ) {
        cout << " Cycle: " << it->first << "  Command: " << it->second->toString() << "\n";
    }
    cout << "----------------------------------------------\n";
}

string DDRModule::getStateStr(u32bit bank) const
{
    BankStateID state = bankState[bank].state;
    switch ( state )
    {
        case BS_Idle:
            return string("IDLE");
        case BS_Activating:
            return string("ACTIVATING");
        case BS_Active:
            return string("ACTIVE");
        case BS_Reading:
            return string("READING");
        case BS_Writing:
            return string("WRITING");
        case BS_Precharging:
            return string("PRECHARGING");
    }

    panic("DDRModule", "getStateStr", "Unknown state. Programming error (should not happen)");
    return string("Unknown state!");
}


void DDRModule::readData(u32bit bankId, u32bit row, u32bit startCol, u32bit bytes, std::ostream& outStream) const
{
    GPU_ASSERT(
        if ( bankId >= banks.size() )
            panic("DDRModule", "readData", "Trying to access a bank with identifier too high");
    )
        
    const DDRBank& bank = banks[bankId];
    bank.readData(row, startCol, bytes, outStream);
}

void DDRModule::writeData(u32bit bankId, u32bit row, u32bit startCol, u32bit bytes, std::istream& inStream)
{
    GPU_ASSERT(
        if ( bankId >= banks.size() )
            panic("DDRModule", "writeData", "Trying to access a bank with identifier too high");
    )
    DDRBank& bank = banks[bankId];
    bank.writeData(row, startCol, bytes, inStream);
}

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

#ifndef DDRMODULESTATE_h
    #define DDRMODULESTATE_h

#include "GPUTypes.h"
#include "GPUMemorySpecs.h"

namespace gpu3d
{
namespace memorycontroller
{

/**
 * Class used to query the state of a DDRModule
 */
class DDRModuleState
{
public:

    typedef u32bit CommandMask;

    static const CommandMask ActiveBit = 0x1;
    static const CommandMask PrechargeBit = 0x2;
    static const CommandMask ReadBit = 0x4;
    static const CommandMask WriteBit = 0x8;

    static const u32bit NoActiveRow = 0xFFFFFFFF;
    static const u32bit AllBanks = 0xFFFFFFFE;
    static const u64bit WaitCyclesUnknown = 0xFFFFFFFF;

    enum BankStateID
    {
        BS_Idle, ///< Any page opened (after precharge is completed)
        BS_Activating, ///< Opening a page
        BS_Active, ///< A read or write operation can be issued
        BS_Reading, ///< Performing a read
        BS_Writing, ///< Performing a write
        BS_Precharging ///< Performing a precharge operation
    };

    enum CommandID
    {
        C_Active,
        C_Precharge,
        C_Read,
        C_Write,
        C_Unknown
    };


    /**
     * Return a bit mask with the accepted commands in the current cycle for a bank
     *
     * @warning When the list contains read or/and write commands this commands are accepted if
     *          the target row is the current active row
     */
    CommandMask acceptedCommands(u32bit bank) const;

    /**
     * Returns the wait cycles required before sending the specified command to a bank
     *
     * @return the cycles to wait before the specified commands
     */
    u64bit getWaitCycles(u32bit bank, CommandID cmd) const;

    /**
     * Return the number of cycles required by a write command to be completed
     *
     * This query can be useful to known when a write that is issued now
     * will be finished and the input data can be considered free.
     */
    u32bit writeBurstRequiredCycles() const;

    /**
     * Return the number of cycles requited by a read command to be completed
     *
     * This query can be useful to known when the data will be available for a 
     * read command issued now.
     */
    u32bit readBurstRequiredCycles() const;
    
    /**
     * Check if the specified command can be issued to a given bank
     */ 
    bool canBeIssued(u32bit bank, CommandID cmd) const;

    enum IssueConstraint
    {
        CONSTRAINT_NONE, // No constraint found for the command

        CONSTRAINT_ACT_TO_ACT, // tRRD constraint (ACT interbank dependency)
        CONSTRAINT_ACT_TO_READ, // Active to read required delay constraint
        CONSTRAINT_ACT_TO_WRITE, // Active to write required delay constraint
        CONSTRAINT_ACT_TO_PRE,  // Act to precharge delay constraint

        CONSTRAINT_READ_TO_WRITE, // tRTW constraint
        CONSTRAINT_READ_TO_PRE, // tRP constraint (Precharging before meet TRP)

        CONSTRAINT_WRITE_TO_READ, // tWTR constraint
        CONSTRAINT_WRITE_TO_PRE, // tRW constraint (Precharging before meet tRW)

        CONSTRAINT_PRE_TO_ACT, // Act sent to a bank not yet closed
        // CONSTRAINT_READ_TO_READ, // Max read bandwith exceed constraint
        // CONSTRAINT_WRITE_TO_WRITE,// Max write bandwith exceed constraint

        CONSTRAINT_DATA_BUS_CONFLICT, // The data bus being used

        /// Constraint considered errors (AKA incorrect sequence of commands independently of time)
        CONSTRAINT_NOACT_WITH_WRITE, // ERROR: Trying to write a bank without an open row
        CONSTRAINT_ACT_WITH_OPENROW, // ERROR: Act to a bank with a current already opened row
        CONSTRAINT_NOACT_WITH_READ, // ERROR: Trying to read a bank without an open row
        CONSTRAINT_AUTOP_READ, // ERROR: Autoprecharge enabled, no more reads accepted
        CONSTRAINT_AUTOP_WRITE, // ERROR: Autoprecharge enabled, no more writes accepted

        CONSTRAINT_UNKNOWN // ???
    };

    IssueConstraint getIssueConstraint(u32bit bank, CommandID cmd) const;


    static std::string getIssueConstraintStr(IssueConstraint ic);

    /**
     * Returns the active row in the current cycle, if there is not a current active row it
     * returns NoActiveRow
     */
    u32bit getActiveRow(u32bit bank) const;

    /**
     * Returns the current burst length of the DDRModule
     */
    u32bit getBurstLength() const;

    /**
     * Return the number of available banks
     */
    u32bit banks() const;

    /**
     * Returns the state of the target bank in the current cycle
     */
    BankStateID getState(u32bit bank) const;

    /**
     * Returns a string representation of the BankStateID
     */
    std::string getStateStr(u32bit bank) const;

    static std::string getCommandIDStr(CommandID cmdid);

    /**
     * Returns the number of cycles remaining to change the current state of the current bank
     *
     * @note Returns 0 when the current state won't change passively (ex: ACTIVE state)
     */
    u32bit getRemainingCyclesToChangeState(u32bit bank) const;

private:
    
    u64bit cycle;

    CommandID lastCommand;
    u32bit nBanks;

    /******************************
     * Global timming constraints *
     ******************************/
    u32bit tRRD; ///< Active bank x to active bank y delay
    
    /*******************************
     * Per bank timing constraints *
     *******************************/
    u32bit tRCD; ///< Active to Read/Write delay
    u32bit tWTR; ///< Write time recovery

    /**
     * Write Data cannot be driven onto the DQ bus for 2 clocks after 
     * the READ data is off the bus.
     */
    u32bit tRTW;
    u32bit tWR; ///< Write recovery time (penalty used with write to precharge)
    u32bit tRP; ///< Rwo precharge time
    u32bit CASLatency; ///< CAS latency
    u32bit WriteLatency; ///< Write latency

    u32bit burstLength; ///< Current burst size configuration
    ///< (burstLength/2) OR Zero if setModuleParametersWithDelayZero() is called
    u32bit burstTransmissionTime;

    struct BankState
    {
        BankStateID state; ///< Current state
        u64bit endCycle; ///< Next cycle that the state will change (current op will finish)
        u64bit lastWriteEnd; ///< True is the last operation was a write, false otherwise
        u32bit openRow; ///< Current open row
        /**
         * Perform autoprecharge as soon as possible (after or in parallel) with the 
         * current read/write command
         */
        bool autoprecharge;
    };

    BankState* bankState;

    u64bit lastActiveBank;
    u64bit lastActiveStart;
    u64bit lastActiveEnd; // == 0 indicates that no previous active was issued

    u64bit lastReadBank;
    u64bit lastReadStart;
    u64bit lastReadEnd;

    u64bit lastWriteBank;
    u64bit lastWriteStart;
    u64bit lastWriteEnd;


    // Called from ctor() to configure GPU memory parameters
    void setModuleParameters(const GPUMemorySpecs& memSpecs, u32bit burstLength);


    DDRModuleState(const DDRModuleState&);
    DDRModuleState& operator=(const DDRModuleState&);

    friend class ChannelScheduler;

    DDRModuleState( u32bit nBanks, 
                    u32bit burstLength,
                    u32bit burstBytesPerCycle, // Typical DDR speed (8B/c)
                    const GPUMemorySpecs& memSpecs );

    /**
     * Performs the timing update required by this class
     * ONLY CALLED BY CHANNELSCHEDULER BASE CLASS
     *
     * It must be called before performing any query or update of this class each time 
     * a new cycle of simulation is started
     */
    void updateState(u64bit cycle);
    // update methods (ONLY CALLED BY CHANNELSCHEDULER)
    void postRead(u32bit bank, bool autoprecharge = false);
    void postWrite(u32bit bank, bool autoprecharge = false);
    void postActive(u32bit bank, u32bit row);
    void postPrecharge(u32bit bank);
    void postPrechargeAll();


    // called by updateState...
    void updateBankState(u64bit cycle, u32bit bankId);

    bool isAnyBank(BankStateID state) const;


};
} // namespace memorycontroller
} // namespace gpu3d

#endif // DDRMODULESTATE_h

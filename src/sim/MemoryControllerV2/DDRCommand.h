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

#ifndef DDRCOMMAND_H
    #define DDRCOMMAND_H

#include "DynamicObject.h"
#include "DDRBurst.h"
#include <string>

namespace gpu3d
{
namespace memorycontroller
{

/**
 * DDRCommand class implementing a DDRCommand
 *
 * Implementation details:
 *    When a DDRCommand is a Write has an associated burst, this burst
 *    is owned by the DDRCommand, which means that when the DDRCommand
 *    is deleted the burst is itself deleted
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @ver 1.0
 * @date 06/03/2006
 */
class DDRCommand : public DynamicObject
{
public:

    enum DDRCmd 
    { 
        Deselect, 
        NOP, 
        Active, 
        Read, 
        Write, 
        Precharge, 
        AutoRefresh,
        Dummy, // Fake command used to send information regarding to bandwidth limitations due to protocol overhead
        Unknown 
    };

    ///@see DDRModuleState::IssueConstraint
    enum ProtocolConstraint
    {
        PC_none,
        
        PC_a2a, // = DDRModuleState::CONSTRAINT_ACT_TO_ACT
        PC_a2p, // = DDRModuleState::CONSTRAINT_ACT_TO_PRE
        PC_a2r, // = DDRModuleState::CONSTRAINT_ACT_TO_READ
        PC_a2w, // = DDRModuleState::CONSTRAINT_ACT_TO_WRITE

        PC_r2w, // = DDRModuleState::CONSTRAINT_READ_TO_WRITE
        PC_r2p, // = DDRModuleState::CONSTRAINT_READ_TO_PRE

        PC_w2r, // = DDRModuleState::CONSTRAINT_WRITE_TO_READ
        PC_w2p, // = DDRModuleState::CONSTRAINT_WRITE_TO_PRE

        PC_p2a, // = DDRModuleState::CONSTRAINT_PRE_TO_ACT        
        
        PC_count
    };

    static std::string protocolConstraintToString(ProtocolConstraint pc);

    void setProtocolConstraint(ProtocolConstraint pc) { protocolConstraint = pc; }

    ProtocolConstraint getProtocolConstraint() const { return protocolConstraint; }

    /**
     * Constant that indicates literally "AllBanks". Is can be used with precharge commands
     */
    static const u32bit AllBanks = 0xFFFFFFFF;


    /**
     * Creates a dummy command to show 
     */
    static DDRCommand* createDummy(ProtocolConstraint pc);
    
    /**
     * Creates an ACTIVE command
     *
     * @param bank bank selected
     * @param row target row to be activated (of the selected bank)
     *
     * @return a ACTIVE DDRCommand with the specified fields
     */
    static DDRCommand* createActive(u32bit bank, u32bit row, ProtocolConstraint pc = PC_none);
    
    /**
     * Creates a READ command
     *
     * @param bank bank selected
     * @param column starting column from where to start reading columns to complete a burst
     * @param autoprecharge enables the autoprecharge bit of this command
     *
     * @return a READ DDRCommand with the specified fields
     */
    static DDRCommand* createRead(u32bit bank, u32bit column, bool autoprecharge = false, ProtocolConstraint pc = PC_none);
    
    /**
     * Creates a WRITE command
     *
     * @param bank bank selected
     * @param colum starting column from where to start reading columns to complete a burst
     * @param data to be written
     * @param autoprecharge enables the autoprecharge bit of this command
     *
     * @return a WRITE DDRCommand with the specified fields
     */
    static DDRCommand* createWrite(u32bit bank, u32bit column, DDRBurst* data, bool autoprecharge = false, ProtocolConstraint pc = PC_none);
    
    /**
     * Creates a PRECHARGE command
     *
     * @param bank bank selected. the constant AllBanks is supplied to indicate that this command
     *             will precharge all chip banks
     *
     * @return a PRECHARGE DDRCommand with the specified fields
     */
    static DDRCommand* createPrecharge(u32bit bank, ProtocolConstraint pc = PC_none);

    /**
     * Returns the enumeration value of this command
     *
     * @return the enumaration value corresponding to this DDRCommand
     */
    DDRCmd which() const;

    /**
     * Returns a string representing the DDRCommand with its parameters
     *
     * @return a string representation of the command
     */
    std::string whichStr() const;
    
    /**
     * Get the bank to which this command is addressed
     *
     * @return the target bank of this command or AllBanks if it is a Precharge_all command
     * @note if the command has not a target bank the return value is undefined (usually 0)
     */
    u32bit getBank() const;
    
    /**
     * Gets the row to which this command is addressed
     *
     * @return the target row of this command
     * @note if the command has not a target row the return value is undefined (usually 0)
     */
    u32bit getRow() const;
    
    /**
     * Gets the column to which this command is addressed
     *
     * @return the target column if this command
     * @note if the command has not a target column the return value is undefined (usually 0)
     */
    u32bit getColumn() const;
    
    /**
     * Gets the data associated to this command (write)
     *
     * The burst "cannot" be deleted since it is owned by the DDRCommand
     *
     * @return the data associated to this command if exists, otherwise it returns 0
     */
    const DDRBurst* getData() const;
    
    /**
     * Checks if this command (read/write) has autoprecharge bit enabled
     *
     * @return true if the command has autoprecharge bit enabled, false otherwise
     */
    bool autoprechargeEnabled() const;
    
    /**
     * Dumps internal state information
     */
    void dump() const;

    /**
     * Get a textual representation of the command
     */
    std::string toString() const;

    void setAsAdvancedCommmand() { advanced = true; }
    bool isAdvancedCommmand() const { return advanced; }

    static u32bit countInstances();

    /**
     * DDRBurst destructor deallocate memory associated to this DDR command
     */ 
    ~DDRCommand();


private:

    DDRCommand(DDRCmd cmd, u32bit bank, u32bit row, u32bit column, bool autoprecharge, DDRBurst* data, ProtocolConstraint pc);
    DDRCommand(const DDRCommand&);
    DDRCommand& operator=(const DDRCommand&);

    static u32bit instances;

    bool advanced;

    DDRCmd cmd; ///< Command enumeration value
    u32bit bank; ///< Bank selector
    u32bit row; ///< Row selector
    u32bit column; ///< Column selector
    bool autoprecharge; ///< Autoprecharge bit
    DDRBurst* data; ///< Data associated to this command (write)
    ProtocolConstraint protocolConstraint; ///< Field used to communicate protocol constraints showed later in the Data Bus

};

} // namespace memorycontroller

} // namespace gpu3d


#endif // DDRCOMMAND_H

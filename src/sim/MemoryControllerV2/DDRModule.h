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

#ifndef DDRMODULE_H
    #define DDRMODULE_H

#include "Box.h"
#include "DDRBank.h"
#include "DDRCommand.h"
#include <queue>
#include <vector>
#include <utility>

namespace gpu3d
{
namespace memorycontroller
{

class GPUMemorySpecs;

/**
 * Class implementing a DDR Module chip
 *
 * This class is implemented as a Box so the communication interface is performed
 * through two signals
 *
 * The input signals receives DDRCommand objects and the output signal sends
 * DDRBurst objects
 *
 * - prefix value concatenated with "DDRModuleRequest" is the name of the signal 
 *   that is read by the DDRModule to receive new commands (and data in write case)
 * 
 * - prefix value concatenated with "DDRModuleReply" is the name of the signal 
 *   that is written by DDRModule to send read data
 *
 * @code
 *   // Example:
 *   // Creates a DDR module chip with 8 banks
 *   DDRModule* module = new DDRModule("DDRChip0", "chip0", 8, 0);
 *   // signals after creatial of the module are:
 *   // in: chip0.DDRModuleRequest
 *   // out: chip0.DDRModuleReply
 * @endcode
 */
class DDRModule : public Box
{
private:

    GPUStatistics::Statistic& transmissionCyclesStat; // Cycles transmiting data
    GPUStatistics::Statistic& transmissionBytesStat; // Total bytes transmited
    GPUStatistics::Statistic& readTransmissionCyclesStat; // Total read cycles
    GPUStatistics::Statistic& readTransmissionBytesStat; 
    GPUStatistics::Statistic& writeTransmissionCyclesStat;
    GPUStatistics::Statistic& writeTransmissionBytesStat;

    GPUStatistics::Statistic& actCommandsReceived;
    GPUStatistics::Statistic& allBanksPrechargedCyclesStat;

    GPUStatistics::Statistic& idleCyclesStat; // Data pins unused due to there is nothing to do

    GPUStatistics::Statistic& casCyclesStat;
    GPUStatistics::Statistic& wlCyclesStat;
    GPUStatistics::Statistic* constraintCyclesStats[DDRCommand::PC_count];

    /**
     * Retains the cycle used with the last clock method call
     */
    u64bit lastClock;
    DDRCommand::DDRCmd lastCmd;
    u32bit lastColumnAccessed; ///< last column accessed


    //DDRBank* banks; ///< banks of this DDR module
    std::vector<DDRBank> banks; ///< banks of this DDR module
    u32bit nBanks; ///< Number of banks of this DDR Module
    u32bit bankRows;    ///<  Number of rows in a bank
    u32bit bankCols;    ///<  Number of columns in a row in a bank

    /**
     * Global timing constraints
     */
    u32bit tRRD; ///< Active bank x to active bank y delay

    /**
     * Per bank timing constraints
     */
    u32bit tRCD; ///< Active to Read/Write delay
    u32bit tWTR; ///< Write to Read delay
    /**
     * Write Data cannot be driven onto the DQ bus for 2 clocks after 
     * the READ data is off the bus.
     */
    u32bit tRTW;
    u32bit tWR; ///< Write recovery time (penalty used with write to precharge)
    u32bit tRP; ///< Rwo precharge time
    u32bit CASLatency; ///< CAS latency
    u32bit WriteLatency; ///< Write latency

    ///< DDR ratio 32-bit datums per cycle (tipically 2 in DDR (double data rate) memory)
    ///u32bit burstElementsPerCycle; 
    u32bit burstBytesPerCycle;

    u32bit burstLength; ///< Current burst size configuration
    ///< (burstLength/2) OR Zero if setModuleParametersWithDelayZero() is called
    u32bit burstTransmissionTime; 

    enum BankStateID
    {
        BS_Idle, ///< Any page opened (after precharge is completed)
        BS_Activating, ///< Opening a page
        BS_Active, ///< A read or write operation can be issued
        BS_Reading, ///< Performing a read
        BS_Writing, ///< Performing a write
        BS_Precharging ///< Performing a precharge operation
    };

    struct BankState
    {
        BankStateID state; ///< Current state
        u64bit endCycle; ///< Next cycle that the state will change (current op will finish)
        u64bit lastWriteEnd; ///< True is the last operation was a write, false otherwise
        /**
         * Perform autoprecharge as soon as possible (after or in parallel) with the 
         * current read/write command
         */
        bool autoprecharge;
    };

    BankState* bankState;

    Signal* cmdSignal; ///< Receives commands (and write data included in write commands)
    Signal* replySignal; ///< Transmits DDRBurst objects

    Signal* moduleInfoSignal; ///< Used for debug purposes
    std::string lastModuleInfoString; ///< Used to compare with previous one and set a different color when a state change has occurred

    struct DataPinItem : public DynamicObject
    {
        enum DataPinItemColor
        {
            STV_COLOR_READ  = 0,
            STV_COLOR_WRITE = 1,
            STV_COLOR_PROTOCOL_CONSTRAINT_BASE = 10, // (Base) color = BASE + getProtocolConstraint()
            STV_COLOR_PROTOCOL_CONSTRAINT_MAX  = 100,
            STV_COLOR_CAS   = 101,
            STV_COLOR_WL    = 102
        };

        enum DataPinItemType
        {
            IS_READ,
            IS_WRITE,
            IS_PROTOCOL_CONSTRAINT,
            IS_CAS_LATENCY,
            IS_WRITE_LATENCY
        };
        // bool isRead;

        DataPinItemColor _whatis ;


        DataPinItemType whattype() const
        {
            if ( _whatis == STV_COLOR_READ )
                return IS_READ;
            if ( _whatis == STV_COLOR_WRITE )
                return IS_WRITE;
            if ( STV_COLOR_PROTOCOL_CONSTRAINT_BASE <= _whatis && _whatis < STV_COLOR_PROTOCOL_CONSTRAINT_MAX )
                return IS_PROTOCOL_CONSTRAINT;
            if ( _whatis == STV_COLOR_CAS )
                return IS_CAS_LATENCY;
            if ( _whatis == STV_COLOR_WL )
                return IS_WRITE_LATENCY;
            panic("DataPinItem", "whattype", "Unknown type!");
            return IS_PROTOCOL_CONSTRAINT; // to avoid stupid compiler warnings...
        }

        DataPinItem(DataPinItemColor wi) : _whatis(wi) {}

        std::string toString() const {
            std::string str;
            switch ( whattype() ) {
                case IS_READ:
                    str = str + "IS_READ"; break;
                case IS_WRITE:
                    str = str + "IS_WRITE"; break;
                case IS_PROTOCOL_CONSTRAINT:
                    str = str + "PROTOCOL_CONSTRAINT (";
                    {
                        str = str + DDRCommand::protocolConstraintToString(
                            DDRCommand::ProtocolConstraint(_whatis - STV_COLOR_PROTOCOL_CONSTRAINT_BASE)) +
                            ")";
                    }
                    break;
                case IS_CAS_LATENCY:
                    str = str + "IS_CAS_LATENCY"; break;
                case IS_WRITE_LATENCY:
                    str = str + "IS_WRITE_LATENCY"; break;
                default:
                    str = "UNKNOWN!";

            } 
            return str;
        }
    };

    typedef DataPinItem::DataPinItemColor DataPinItemColor;
    typedef DataPinItem::DataPinItemType DataPinItemType;


    // Used to debug DDRModule state
    struct DDRModuleInfo : public DynamicObject
    {};
    
    // inner signals to visualize data pins transmissions
    Signal* dataPinsSignal;

    DataPinItem* bypassConstraint;

    // This queue is processed each cycle writing in dataPinsSignal if the front
    // cycle of the front pair matches with the current cycle
    std::deque<std::pair<u64bit,DataPinItem*> > dataPinsItem;

    std::deque<std::pair<u64bit, DDRBurst*> > readout; ///< in progress reads
    //std::deque<u64bit> readin; ///< in progress writes
    std::deque<std::pair<u64bit, const DDRBurst*> > readin; ///< in progress writes

    u64bit lastActiveBank;
    u64bit lastActiveStart;
    u64bit lastActiveEnd; // == 0 indicates that no previous active was issued

    u64bit lastReadBank;
    u64bit lastReadStart;
    u64bit lastReadEnd;

    u64bit lastWriteBank;
    u64bit lastWriteStart;
    u64bit lastWriteEnd;

    /// Cannot be 0!!! It will skip the deleting of DDR Commands
    static const u32bit LIST_OF_LATESTS_COMMANDS_MAX_SIZE = 10;
    /// List to keep track of latests DDR Commands received
    std::deque<std::pair<u64bit,DDRCommand*> > listOfLastestProcessedCommands;

    // This attribute is updated by each write command and indicates when a new
    // read will be able to be processed
    
    void updateBankState(u64bit cycle, u32bit bankId);
    

    void preprocessCommand(u64bit cycle, const DDRCommand* ddrCommand); // Called by processCommand()
    void processCommand(u64bit cycle, DDRCommand* cmd); 
    void processActiveCommand(u64bit  cycle, DDRCommand* activeCommand);
    void processPrechargeCommand(u64bit  cycle, DDRCommand* prechargeCommand);
    void processReadCommand(u64bit  cycle, DDRCommand* readCommand);
    void processWriteCommand(u64bit  cycle, DDRCommand* writeCommand);
    void processDummyCommand(u64bit cycle, DDRCommand* dummyCommand);

    bool isAnyBank(BankStateID state) const;

    bool processDataPins(u64bit cycle);
    bool processDataPinsConstraints(u64bit cycle);

    // info methods

    std::string getStateStr(u32bit bank) const;


public:

    /**
     * Method available to implemented PRELOAD DATA into memory modules
     */
    void preload(u32bit bank, u32bit row, u32bit startCol, 
                 const u8bit* data, u32bit dataSz, const u32bit* mask = 0 );

    /**
     * Creates a DDRModule object
     *
     * @param name Box name
     * @param prefix prefix added to the basic Box signal names to create unique signal names
     * @param banks Number of banks within this module
     * @param parentBox Box which is the parent of this memory module
     */
    DDRModule(const char* name, 
              const char* prefix,
              u32bit burstLength,             
              u32bit banks,
              u32bit bankRows,
              u32bit bankCols,
              //u32bit burstElementsPerCycle, // 2 = Typical DDR speed
              u32bit burstBytesPerCycle, // 8 = Typical DDR speed
              const GPUMemorySpecs& memSpecs,
              Box* parentBox = 0
              );

    void setModuleParameters( const GPUMemorySpecs& memSpecs, u32bit burstLength );

    /**
     * Simulates 1 cycle of simulation time of this memory module
     *
     * @param cycle cycle to be simulated
     */
    void clock(u64bit cycle);

    /**
     * Dump the current operating parameters of the memory module
     */
    void printOperatingParameters() const;

    /**
     * Gets a DDRModule bank
     *
     * @param bankId bank identifier
     * @return a reference to the DDRBank identified by bankId
     */
    DDRBank& getBank(u32bit bankId);

    /**
     * Returns the number of banks within this memory module
     *
     * @return number of banks within this memory module
     */
    u32bit countBanks() const;

    /**
     * Dump the memory module internal state
     *
     * @param bankContens true if the data contents of the bank will be printed, 
     *                    false otherwise
     * @param txtFormat true if byte will be printed as characters, false
     *                  if byte contents will be printed as hexadecimal values
     */
    void dump(bool bankContents = false, bool txtFormat = false) const;

    /**
     * Direct access to read data from a bank and place it into an output stream (ie: ofstream)
     *
     * @note this method access data directly without changing chip state at all (any simulation is performed)
     */
    void readData(u32bit bank, u32bit row, u32bit startCol, u32bit bytes, std::ostream& outStream) const;

    /**
     * Direct access to write data into a bank reading the data from an input stream (ie: ifstream)
     *
     * @note this method access data directly without changing chip state at all (any simulation is performed)
     */
    void writeData(u32bit bank, u32bit row, u32bit startCol, u32bit bytes, std::istream& inStream);
    
};

} // namespace memorycontroller

} // namespace gpu3d

#endif // DDRMODULE_H

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

#ifndef CHANNELSCHEDULER_H
    #define CHANNELSCHEDULER_H

#include "Box.h"
#include "DynamicObject.h"
#include "ChannelTransaction.h"
#include "DDRModuleState.h"
#include "DDRBurst.h"
#include "DDRCommand.h"
#include <list>

// included to be able to identify clients
#include "MemoryTransaction.h"

namespace gpu3d
{
namespace memorycontroller
{

class GPUMemorySpecs;

/**
 * @brief Class used to carry channel scheduler states
 *
 * This class is mainly a container to allow specific state enums to travel thru signals
 */
class SchedulerState : public DynamicObject
{
public:

    /**
     * @brief Tokens definining the possible scheduler states
     */
    enum State
    {
        AcceptBoth, /**< Scheduler accepts read and write channel transaction requests */
        AcceptRead, /**< Scheduler only accepts read channel transaction requests */
        AcceptWrite, /**< Scheduler only accepts write channel transaction requests */
        AcceptNone /**< Scheduler does not accept read or write channel transaction requests */
    };

    /**
     * @brief Creates a new scheduler state object based on a State token (all bank resources/queues share the same state)
     * @param state The desired state to be contained into the scheduler state object
     */
    SchedulerState(State state);

    /**
     * @brief Creates a new scheduler state object containing the different per bank states
     * @param bankState vector with the individual bank states
     * @warning The vector pointer passed is owned by SchedulerState from now on
     */
    SchedulerState(std::vector<State>* bankStates); 

    /**
     * @brief Gets the state identifier associated to a specific bank saying what type of requests 
     * to that bank are currently accepted
     *
     * @param bank Bank identifier
     * @return The state saying what type of requests are accepted by the target bank
     */
    State state(u32bit bank) const;

    ~SchedulerState();

private:

    const bool allBanksShareState;
    State sharedSchedState; /**< The state value represented by this scheduler state object */
    std::vector<State>* bankStates;

};

/**
 * @brief Interface for all memory controller schedulers
 */
class ChannelScheduler : public Box
{

public:



    /**
     * Struct used to pass global information about clients to channel schedulers
     * @note Some values are defined as constants since they cannot change (for now)
     */
    struct MemoryClientsInfo
    {
        static const u32bit numCommandProcessorUnits = 1;
        static const u32bit numStreamerFetchUnits = 1;
        static const u32bit numDACUnits = 1;
        u32bit streamerLoaderUnits;
        u32bit numTextureUnits;
        u32bit numStampUnits;
    };

	// Page policy is defined here but implemented in each scheduler
    enum PagePolicy
    {
        ClosePage,
        OpenPage
    };

    enum SwitchModePolicy
    {
        SMP_Counters = 0, ///< Legacy Attila - two counters
        SMP_LoadsOverStores = 1, ///< Priorize reads over writes, only keep writing while writes keep hiting pages
        SMP_SwitchOnMiss, ///< Switch always that a page miss is generated if the 
        /**
        * if reading and miss ->
        *   if next write is hit -> switch to writes (just two cycles penalty in the data bus)
        *   else -> keep reading
        * if writing and miss ->
        *   always switch to reads (part of the latency from ACT will be hidden by tRTW)
        */
        SMP_SwitchOnMiss2,
        SMP_MinSwitches, ///< Drain reads (or writes) and switch (ignore misses)
    };

    /*
     * @param moduleBanks Number of banks available per DRAM chip
     * @param burstLenght Current burst length used by the DRAM chips
     * @param burstBytesPerCycle Bytes of burst transmited per cycle (DDR memories usually = 8)
     * @param perfectMemory Disable all timing constraints of the DRAM chips
     * @param requestBandwidth UNKNOWN
     */
    struct CommonConfig
    {
        const MemoryClientsInfo mci;
        const u32bit moduleBanks;
        const u32bit burstLength;
        const u32bit burstBytesPerCycle;
        bool enableProtocolMonitoring;
        const GPUMemorySpecs& memSpecs;
        const SwitchModePolicy switchModePolicy;
        const u32bit smpMaxConsecutiveReads;
        const u32bit smpMaxConsecutiveWrites;

        const PagePolicy pagePolicy;
        const std::string debugString;
        // These two variables have only meaning with schedulers supporting PAM
        const u32bit activeManagerMode;
        const u32bit prechargeManagerMode;
        bool disableActiveManager;
        bool disablePrechargeManager;        
        const u32bit managerSelectionAlgorithm; 
         // 0 first selects between hits using as a second criteria the bankSelectionPolicy
         // 1 first selects using bankSelectionPolicy, then priorize based on hits (hits first in case of bankSelection tie)
         u32bit transactionSelectionMode;
        const std::string bankSelectionPolicy;
        // Create internal signals used to monitor STV's information
        const bool createInnerSignals;
        const bool useClassicSchedulerStates;
        const u32bit maxChannelTransactions;
        // Used by schedulers having different queues for reads and writes
        // readEntries = dedicatedChannelReadTransactions
        // writeEntries = maxChannelTransactions - dedicatedChannelReadTransactions
        // to ignore this value set it to 0, readEntries = maxChannelTransactions/2 and writeEntries=maxChannelTransactions/2
        const u32bit dedicatedChannelReadTransactions;

        // ctor mandatory to initialize come const vars
        CommonConfig( const MemoryClientsInfo mci,
                      u32bit moduleBanks,
                      u32bit burstLength,
                      u32bit burstBytesPerCycle,
                      bool enableProtocolMonitoring,
                      const GPUMemorySpecs& memSpecs,
                      SwitchModePolicy switchModePolicy,
                      u32bit smpMaxConsecutiveReads,
                      u32bit smpMaxConsecutiveWrites,
                      PagePolicy pagePolicy,
                      u32bit maxChannelTransactions,
                      u32bit dedicatedChannelReadTransactions,
                      const std::string debugString,
                      bool useClassicSchedulerStates,
                      u32bit activeManagerMode,
                      u32bit prechargeManagerMode,
                      bool disableActiveManager,
                      bool disablePrechargeManager,
                      u32bit managerSelectionAlgorithm,
                      u32bit transactionSelectionMode, 
                      const std::string& bankSelectionPolicy,
                      bool createInnerSignals ) :
            mci(mci),
            moduleBanks(moduleBanks),
            burstLength(burstLength),
            burstBytesPerCycle(burstBytesPerCycle),
            enableProtocolMonitoring(enableProtocolMonitoring),
            memSpecs(memSpecs),
            switchModePolicy(switchModePolicy),
            smpMaxConsecutiveReads(smpMaxConsecutiveReads),
            smpMaxConsecutiveWrites(smpMaxConsecutiveWrites),
            pagePolicy(pagePolicy),
            maxChannelTransactions(maxChannelTransactions),
            dedicatedChannelReadTransactions(dedicatedChannelReadTransactions),
            debugString(debugString),
            useClassicSchedulerStates(useClassicSchedulerStates),
            activeManagerMode(activeManagerMode), prechargeManagerMode(prechargeManagerMode),
            disableActiveManager(disableActiveManager),
            disablePrechargeManager(disablePrechargeManager),
            managerSelectionAlgorithm(managerSelectionAlgorithm),
            transactionSelectionMode(transactionSelectionMode),
            bankSelectionPolicy(bankSelectionPolicy),
            createInnerSignals(createInnerSignals)
        {}

    };

    /**
     * Reads input signals and executes schedulerClock() method
     *
     * @warning This method must not be reimplemented (therefore is not virtual). The method
     *          that must be implemented is schedulerClock that is called by clock()
     */
    void clock(u64bit cycle);


private:

    
    ChannelScheduler(const ChannelScheduler&);
    ChannelScheduler& operator=(const ChannelScheduler&);


    //SwitchModeManager* sm;

    /**
     * @warning Pending documentation
     */
    bool setStateCalled;

    std::string prefixStr;

    /**
     * @brief Input signal (with name "ChannelRequest") receiving channel transactions
     * Signal's properties
     *    - Name: "ChannelRequest"
     *    - Type: Input
     *    - Carried object: ChannelTransaction*
     */
    Signal* channelRequest;
    
    /**
     * @brief Signal used to return completed channel transaction ACKS and read data
     * Signal's properties:
     *    - Name: "ChannelReply"
     *    - Type: Output
     *    - Carried object: ChannelTransaction*

     * @note The object returned is interpreted as an ACK, for reads also includes read data
     * @note The block reading from this signal is responsible of eventually deleting the SchedulerState object read
     */
    Signal* channelReply;

    /**
     * @brief Signal returning the current scheduler's state
     * Signal's properties:
     *    - Name: "SchedulerState"
     *    - Type: Output
     *    - Carried object: SchedulerState*
     */
    Signal* schedulerState;

    /**
     * @brief Signal used to send DDR Commands to the channel's attached DDR Chip
     * Signal's properties:
     *    - Name: "DDRModuleRequest"
     *    - Type: Output
     *    - Carried object: DDRCommand*
     */
    Signal* moduleRequest;

    /**
     * Last cycle module request was written
     */
    u64bit moduleRequestLastCycle;

    /**
     * @brief Signal used to receive read data from channel's attached DDR Chip
     * Signal's properties:
     *    - Name: "DDRModuleReply"
     *    - Type: Input
     *    - Carried object: DDRBurst*
     */
    Signal* moduleReply;

    GPUStatistics::Statistic& readBytesStat; /**< Statistic accumulating read bytes by this channel */
    GPUStatistics::Statistic& writeBytesStat; /**< Statistic accumulating written bytes by this channel */

    // Command distribution statistics
    GPUStatistics::Statistic& readCmdStat; /**< Statistic used to count read commands issued to this channel */
    GPUStatistics::Statistic& writeCmdStat; /**< Statistic used to count write commands issued to this channel */
    GPUStatistics::Statistic& activeCmdStat; /**< Statisic used to count active commands issued to this channel */
    GPUStatistics::Statistic& prechargeCmdStat; /**< Statistic used to count precharge commands issued to this channel */

   /**
    * @brief Bitmask indicating whether last DDR command to a specific bank was either a read/write (true) or
    * an act/pre command (false)
    * @note This bit vector is a helper structure used to compute hit/miss page ratio
    */
    std::vector<bool> lastCmdWasRW;

    GPUStatistics::Statistic& totalRowHitStat;       /**< Statistic used to count row/page hits */
    GPUStatistics::Statistic& readRowHitStat;   /**< Statistic used to count read row/page hits */
    GPUStatistics::Statistic& writeRowHitStat;  /**< Statistic used to count write row/page hits */
    GPUStatistics::Statistic& totalRowMissStat;      /**< Statistic used to count row/page misses */
    GPUStatistics::Statistic& readRowMissStat;  /**< Statistic used to count read row/page misses */
    GPUStatistics::Statistic& writeRowMissStat; /**< Statistic used to count write row/page misses */
    
    struct PerClientRowHitRatio
    {
        GPUStatistics::Statistic* readHits;
        GPUStatistics::Statistic* readMisses;
        GPUStatistics::Statistic* writeHits;
        GPUStatistics::Statistic* writeMisses;
        GPUStatistics::Statistic* totalHits;
        GPUStatistics::Statistic* totalMisses;
    };
    
    /**
     * Array containing row hit/miss ratios per client type
     */
    PerClientRowHitRatio perClientRowHit[LASTGPUBUS];

    DDRModuleState modState; /**< The full DDR chip's current state */
    //SchedulerState::State currentState; /**< The current scheduler state */
    SchedulerState* currentState;

    const u32bit BurstLength; /**< Constant containing the current burst length value */
    const u32bit Banks; /**< Constant containing the number of banks available per DDR chip */
    //u32bit requestBandwidth; /**< @warning Documentation pending */

	PagePolicy pagePolicy;

    u32bit numTexUnits;
    u32bit numStampUnits;
    u32bit streamerLoaderUnits;

    std::string _debugString;

protected:

    /**
     * Gets the current switch mode manager used by this channel scheduler
     */
    //SwitchModeManager* getSwitchModeManager();

    /**
     * @brief This method is called automatically each time a channel transaction is received by the channel scheduler
     * @note This method has to be implemented in subclasses to process channel transaction requests
     * @param cycle Current simulation cycle
     * @param request Channel transaction just received
     */
    virtual void receiveRequest(u64bit cycle, ChannelTransaction* request) = 0;

    /**
     * @brief This method is called automatically each time a DDRBurst is received from the attached DDR chip
     * @note This method has to be implemented in subclasses to process incoming read data from DDR chips 
     * @param cycle Current simulation cycle
     * @param data Incoming data from DDR chip
     */
    virtual void receiveData(u64bit cycle, DDRBurst* data) = 0;
    
    /**
     * @brief Method provided to subclasses to send data/acks back to clients of the channel scheduler
     * @note This method should be called (often inside schedulerClock) each time a reply (data/ack) is ready
     */ 
    void sendReply(u64bit cycle, ChannelTransaction* reply);

    /**
     * @brief Method provided to subclasses to send DDR Command to the attached DRAM chip
     *
     * The protocol (currently GDDR3) is checked and if the command can not be sent the method returns
     * immediately with a false value, if the command can be sent returns true and sends
     * the command
     * @note When the command is discarded the channel scheduler owns the DDRCommand object, otherwise the
     *       DDRCommand ownership is of the DRAM chip
     * @param cycle Current simulation cycle
     * @param ddrCmd The command to be sent to the DRAM chip
     * @param ct The associated channel transaction that generates the ddr command
     *           This parameter may be 0 for ACT/PRE comamnds but has to be !=0 for R/W commands
     */
    bool sendDDRCommand(u64bit cycle, DDRCommand* ddrCmd, const ChannelTransaction* ct);

    /**
     * @brief This method must implement the specific scheduler algorithm, it is implicitly called by the
     * base class method clock()
     * @param cycle Current simulation cycle 
     */
    virtual void schedulerClock(u64bit cycle) = 0;

    /**
     * @brief Base constructor that must be called by all subclasses
     *
     * @param name Channel scheduler name (Box name)
     * @param prefix The prefix supported by the base box class
     * @param moduleBanks Number of banks available per DRAM chip
     * @param burstLenght Current burst length used by the DRAM chips
     * @param burstBytesPerCycle Bytes of burst transmited per cycle (DDR memories usually = 8)
     * @param perfectMemory Disable all timing constraints of the DRAM chips
     * @param requestBandwidth UNKNOWN
     * @param parent Box that owns the channel scheduler box
     */
    /*
    ChannelScheduler(const char* name, 
                     const char* prefix,
                     const MemoryClientsInfo& mci,
                     u32bit moduleBanks,
                     u32bit burstLength,
                     // u32bit burstElementsPerCycle,
                     u32bit burstBytesPerCycle,
                     bool enableProtocolMonitoring,
                     const GPUMemorySpecs& memSpecs,
                     u32bit requestBandwidth,
					 PagePolicy pagePolicy,
                     const std::string& debugString,
                     Box* parent);
    */

    /**
     * @brief Base constructor that must be called by all subclasses
     *
     * @param name Channel scheduler name (Box name)
     * @param prefix The prefix supported by the base box class
     * @param parent Box that owns the channel scheduler box
     * @param config Configuration common to all channel schedulers
     */
    ChannelScheduler(const char* name, const char* prefix, Box* parent, const CommonConfig& config);


    /**
     * @brief Obtain the DDRModuleState object that allows to query the attached DRAM chip
     *
     * @note The usage of this object is not mandatory to keep track and query the attached module
     *       however is provided to ease the task of implementing the different schedulers
     * @return The DRAM state object
     */
    const DDRModuleState& moduleState() const;

    /**
     * @brief Convenient command list definition
     */
    typedef std::list<DDRCommand*> CommandList;

    /**
     * @brief Helpper method provided to subclasses to generate the DRAM commands required to satisty a channel transaction
     * @note list.front() is the first commands, list.back() is the last one
     * @param channelTran The channel transaction taken to generate the commands
     * @return the generated list of commands required to satisfy the transaction
     */
    CommandList transactionToCommands( const ChannelTransaction* channelTran );

    /**
     * @brief Helper method provided to subclasses to easily create an active DRAM command
     * @param bank The target bank to be activated/opened
     * @param row The target row/page to be activated/opened
     * @return A new Active DRAM command based on the input parameter values
     */
    DDRCommand* createActive(u32bit bank, u32bit row); ///< To create an active command

    /**
     * @brief Helper method provided to subclasses to easily create a precharge DRAM command
     * @param bank The target bank to be precharged
     * @return A new precharge DRAM command based on the selected bank
     */ 
    DDRCommand* createPrecharge(u32bit bank); ///< To create a precharge command
    
    /**
     * @brief Method provided to subclasses to update the state that will be sent at the end of the current simulation cycle 
     * @param state The new state of the channel scheduler that will be sent to clients
     */
    //void setState(SchedulerState::State state);
    void setState(SchedulerState* state);

    /**
     * @brief Method provided to subclasses to query the current burst length
     */
    u32bit getBurstLength() { return BurstLength; }

    /**
     * @brief Method provided to subclasses to query the number of available banks of the attached DRAM chip
     */
    u32bit getBanks() { return Banks; }


	PagePolicy getPagePolicy() const { return pagePolicy; }

    /**
     * debug string passed from config file to the scheduler to check experimental features without create public parameters
     */
    const std::string& getDebugString() const;
    
};
} // namespace memorycontroller
} // namespace gpu3d

#endif // CHANNELSCHEDULER_H

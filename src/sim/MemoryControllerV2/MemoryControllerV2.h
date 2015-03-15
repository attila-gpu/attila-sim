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

#ifndef MEMORYCONTROLLER_H
    #define MEMORYCONTROLLER_H

#include "MultiClockBox.h"
#include "MemoryTransaction.h"
#include "toolsQueue.h"
#include "ChannelScheduler.h"
#include "DDRModule.h"
#include "MemoryRequest.h"

//  std includes
#include <string>
#include <vector>

namespace gpu3d
{
namespace memorycontroller
{
class GPUMemorySpecs;
class MemoryRequestSplitter;
class MemoryTraceRecorder;

/***  Maximum number of memory transaction ids availables.  */
const u32bit MAX_MEMORY_TICKETS = 256;

/**  Memory transaction transmission cycles (to physical memory).  */
const u32bit TRANSACTION_CYCLES = 4;

/*** Maximum number of buses per GPU unit type.  */
// deprecated
//const u32bit MAX_GPU_UNIT_BUSES = 16;

/**
 *  Defines the latency to system memory in cycles.
 */
const u32bit SYSTEM_MEMORY_READ_LATENCY = 500;
const u32bit SYSTEM_MEMORY_WRITE_LATENCY = 500;

/**
 *  Defines the cycles it takes to transmit a memory transaction from/to system memory.
 */
const u32bit SYSTEM_TRANSACTION_CYCLES = 16;

/**
 *  Defines the number of buses to system memory.
 *  In current implementation: two, one for read and one for writes.
 */
const u32bit SYSTEM_MEMORY_BUSES = 2;

// By now this constant should be used to configure the number of DDR Channels
const u32bit GPU_MEMORY_CHANNELS = 8;

struct MemoryControllerParameters
{
    enum SchedulerType
    {
        Fifo = 0,
        RWFifo = 1,
        BankQueueFifo = 2,
        BankRWQueueFifo = 3,
        LookAhead = 4
    };

    enum SchedulerPagePolicy
    {
        ClosePage,
        OpenPage
    };

    /////////////////
    // Buses width //
    /////////////////
    u32bit comProcBus;
    u32bit streamerFetchBus;
    u32bit streamerLoaderBus;
    u32bit zStencilTestBus;
    u32bit colorWriteBus;
    u32bit dacBus;
    u32bit textUnitBus;

    u32bit gpuMemorySize; ///< GPU local memory size (in bytes) (ignored for now)
    u32bit systemMemorySize; // System memory size (in bytes)
    u32bit numTexUnits;
    u32bit systemMemoryBuses;
    u32bit numStampUnits;
    u32bit streamerLoaderUnits;
    u32bit systemMemoryReadLatency;
    u32bit systemMemoryWriteLatency;
    u32bit requestQueueSize;
    u32bit serviceQueueSize;
    u32bit readBuffers;
    u32bit writeBuffers;
    u32bit systemTransactionCycles;

    bool perBankChannelQueues;
    // 0 means "Round & Robin"
    // != 0 means "Oldest first"
    u32bit perBankChannelQueuesSelection;
    bool perBankSchedulerState;

    bool memoryTrace; ///< Enables/disables memory trace generation
    u32bit memoryChannels; ///< Number of GPU Memory Channels
    u32bit banksPerMemoryChannel;
    u32bit channelInterleaving;
    u32bit bankInterleaving;
    bool enableSecondInterleaving;
    u32bit secondChannelInterleaving;
    u32bit secondBankInterleaving;
    u32bit splitterType;
    std::string* channelInterleavingMask;
    std::string* bankInterleavingMask;
    std::string* secondChannelInterleavingMask;
    std::string* secondBankInterleavingMask;
    //u32bit fifo_maxChannelTransactions;
    //u32bit rwfifo_maxReadChannelTransactions;
    //u32bit rwfifo_maxWriteChannelTransactions;
    u32bit maxChannelTransactions;
    u32bit dedicatedChannelReadTransactions;
    u32bit rwfifo_maxConsecutiveReads;
    u32bit rwfifo_maxConsecutiveWrites;

    u32bit switchModePolicy;
    u32bit bankfifo_activeManagerMode;
    u32bit bankfifo_prechargeManagerMode;

    bool bankfifo_disablePrechargeManager;
    bool bankfifo_disableActiveManager;
    u32bit bankfifo_managerSelectionAlgorithm;

    // This structure defines all the parameters relative to the underlaying memory used by Attila
    // Currently only GDDR-like memory is supported
    u32bit memoryRowSize;
    //u32bit burstElementsPerCycle;
    u32bit burstBytesPerCycle;
    u32bit burstLength;
    GPUMemorySpecs* memSpecs;

    SchedulerType schedulerType;
    std::string* bankfifo_bankSelectionPolicy;
    SchedulerPagePolicy schedulerPagePolicy;

    std::string* debugString;
    bool useClassicSchedulerStates;

    bool useSplitRequestBufferPerROP;
};

class MemoryController : public MultiClockBox
{
private:

    u64bit _lastCycle;
    u64bit _lastCycleMem;

    MemoryTraceRecorder* memoryTrace;

    bool useInnerSignals; // execute code of inner signals

    /********************************
     * Memory controller statistics *
     ********************************/

    // Global counter stats
    GPUStatistics::Statistic* totalTransStat;
    GPUStatistics::Statistic* totalWriteTransStat;
    GPUStatistics::Statistic* totalReadTransStat;
    GPUStatistics::Statistic* totalWriteBytesStat;
    GPUStatistics::Statistic* totalReadBytesStat;

    // Accounts for cycles causing a read or write stall in channelRequests (scheduler cannot accept reads or writes, backpressure)
    GPUStatistics::Statistic* channelReq_ReadStallCyclesStat;
    GPUStatistics::Statistic* channelReq_WriteStallCyclesStat;

    // Per unit-subunit stats struct
    struct UnitStatsGroup
    {
        GPUStatistics::Statistic* trans;
        GPUStatistics::Statistic* writeTrans;
        GPUStatistics::Statistic* readTrans;
        GPUStatistics::Statistic* writeBytes;
        GPUStatistics::Statistic* readBytes;
        GPUStatistics::Statistic* acceptReadCycles;
        GPUStatistics::Statistic* acceptWriteCycles;
        GPUStatistics::Statistic* readServiceAccumTime;
        GPUStatistics::Statistic* completedReadTrans; // different from readTrans -> readTrans is computed when a request arrive to the MC
        GPUStatistics::Statistic* readServiceTimeAvg; // equivalent to readServiceAccumTime/completedReadTrans 

        void initUnitStatGroup(const std::string& prefix);
    };

    // Per-subunit/channel stats struct
    struct UnitChannelStatsGroup
    {
        GPUStatistics::Statistic* readBytes;
        GPUStatistics::Statistic* writeBytes;
        GPUStatistics::Statistic* readChannelTransAccumTime;
        GPUStatistics::Statistic* completedReadChannelTrans;

        void initUnitChannelStatsGroup(const std::string& prefix, const std::string& postfix);
    };


    // Example of use
    // unitStat[CMDPROC][unit].transStat->inc();
    // unitStat[CMDPROC].back().transStat->inc(); // inc stat per unit type (total Accum)
    std::vector<UnitStatsGroup> unitStats[LASTGPUBUS];

    // unitChannelStats[COLOR][unit][channel].readBytes->inc(amountOfBytes);
    std::vector< std::vector<UnitChannelStatsGroup> > unitChannelStats[LASTGPUBUS];

    // preload transaction counter
    GPUStatistics::Statistic* preloadStat;

    // System memory statistics
    GPUStatistics::Statistic* sysBusReadBytesStat;
    GPUStatistics::Statistic* sysBusWriteBytesStat;

    // Updates readServiceTimeStat & completedReadTransStat
    void updateCompletedReadStats(MemoryRequest& mr, u64bit cycle);

    /**
     * @brief Sum of the request buffer size each cycle
     */
    GPUStatistics::Statistic* stat_requestBufferAccumSizeStat;

    /**
     * @brief Statistic keeping the request buffer's average allocated entries
     */
    GPUStatistics::Statistic* stat_avgRequestBufferAllocated;


    GPUStatistics::Statistic* stat_serviceQueueAccumSizeStat;
    /**
     * @brief Statistic keeping the service queue's average requests
     */
    GPUStatistics::Statistic* stat_avgServiceQueueItems;

    // Object used to split memory request objects
    // A second splitter can be created if a second interleaving is provided
    //std::vector<MCSplitter*> splitterArray;
    std::vector<MemoryRequestSplitter*> splitterArray;

    // Register with the start address of the second interleaving
    // (0 means ignore second interleaving)
    u32bit MCV2_2ND_INTERLEAVING_START_ADDR;

    ChannelScheduler** channelScheds;
    DDRModule** ddrModules;
    
    Signal** channelRequest; ///< Output signal matching "ChannelRequest" from ChannelScheduler
    Signal** channelReply; ///< Input signal matching "ChannelReply" from ChannelScheduler
    Signal** schedulerState; ///< Input signal matching  "SchedulerState" from ChannelScheduler
    
    Signal* mcCommSignal; // Command signal from the command processor

    // Input/output signals
    std::vector<Signal*> requestSignals; // example: commProcWriteSignal
    std::vector<std::vector<Signal*> > dataSignals; // example:commProcReadSignal

    /**
     * System memory signals required to simulate system memory access
     */
    std::vector<Signal*> systemMemoryRequestSignal;
    std::vector<Signal*> systemMemoryDataSignal;
    // used to maintain the arrival time of the in-flight sytem transactions (read)
    tools::Queue<u64bit> systemTransactionArrivalTime;
    // Check that the arrival time corresponds to the transaction received from system memory
    tools::Queue<u32bit> systemTransactionArrivalTimeCheckID;

    /**
     *  Signals and structures to generate bandwidth usage information
     */
    std::vector<Signal*> memBusIn[LASTGPUBUS]; ///<  Signals for displaying bandwidth usage
    std::vector<Signal*> memBusOut[LASTGPUBUS]; ///<  Signals for displaying bandwidth usage
    std::vector<std::vector<DynamicObject*> > busElement[LASTGPUBUS]; ///<  Stores the cookies for the current transaction in the unit bus
    std::vector<u32bit> elemSelect[LASTGPUBUS];  ///<  Which bus element is currently active

    /***************************
     * Memory Controller state *
     ***************************/
    u32bit freeReadBuffers; ///< Number of free read buffers (lines)
    u32bit freeWriteBuffers; ///< Number of free write buffers (lines)

    /*****************************************
     * GPU Memory request queues and buffers *
     *****************************************/
    MemoryRequest* requestBuffer; ///< GPU Memory request buffer entries
    tools::Queue<u32bit> freeRequestQueue; ///< Queue storing free entries in the request buffer

    //// Counters to implement a pseudo split Request buffer ///
    std::vector<u32bit> ropCounters;
    bool useRopCounters;

    /************************************************
     * Per GPU bus read request queues and counters *
     ************************************************/
    // Channel transaction + timestamp
    //tools::Queue<std::pair<ChannelTransaction*, u64bit> >* channelQueue; ///< Per GPU memory bus read request queues
    
    bool useIndependentQueuesPerBank;
    u32bit perBankChannelQueuesSelection; // ALgorithm used to select the target bank if independent queues per bank are used
    tools::Queue<std::pair<ChannelTransaction*, u64bit> >** channelQueue; ///< Per GPU memory bus read request queues
    // One Round Robin indicator per channel group of queues
    u32bit* nextBankRR;
    u32bit banksPerMemoryChannel;
    
    ///////////////////////////////
    /// System memory variables ///
    ///////////////////////////////
    
    ///<  Mapped system memory request queue
    ///< Array with the transaction beings transmitted from/to system memory
    MemoryRequest* systemRequestBuffer; 
    
    tools::Queue<u32bit> systemRequestQueue; ///< FIFO with ready requests
    ///< Free list of system memory request entries
    tools::Queue<u32bit> systemFreeRequestQueue;
    ///< Stores the cycle of the last transaction to a system memory bus
    u64bit systemBus[SYSTEM_MEMORY_BUSES];
    ///< Stores the request ID in the systemRequestBuffer of the current memory transaction being processed
    // u32bit systemBusID[SYSTEM_MEMORY_BUSES];
    std::queue<u32bit> systemBusID[SYSTEM_MEMORY_BUSES];

    ///< Array with the transaction beings transmitted from/to system memory
    MemoryTransaction** systemTrans;

    // receive transactions from system memory
    void processSystemMemoryReplies(u64bit cycle);

    //std::vector<bool> service[LASTGPUBUS]; ///< If a service is being served in a bus
    //std::vector<u32bit> currentTrans[LASTGPUBUS]; ///<  Stores a pointer to the current memory transaction in the GPU unit bus
    //std::vector<bool> isSystemTrans[LASTGPUBUS]; ///<  Stores if the current memory transaction in the GPU unit bus is to mapped system memory
    //std::vector<u32bit> busCycles[LASTGPUBUS]; ///<  Number of cycles until the end of the current bus data transmission
    //std::vector<bool> reserveBus[LASTGPUBUS]; ///< Used by the service queues to reserve the unit bus

    /**
     * Structure keeping the state of one Memory Controller's I/O bus
     */
    struct BusState
    {
        bool service;
        bool isSystemTrans;
        u32bit rbEntry; // previously known as 'currentTrans'
        u32bit busCycles;
        bool reserveBus;
        // to debug
        const MemoryTransaction* mt;
    };

    std::vector<BusState> _busState[LASTGPUBUS]; ///< State of all Memory Controller's I/O buses

    tools::Queue<MemoryRequest> serviceQueue; ///< Stores the memory read requests to be served to the GPU units

    u8bit* systemMemory; ///< Pointer to the buffer where the mapped system memory is stored
    u32bit systemMemorySize; ///< Amount of system memory (bytes)

    u32bit gpuMemorySize;
    u32bit gpuBurstLength;
    //u32bit gpuMemPageSize;
    //u32bit gpuMemOpenPages;
    u32bit numTexUnits;
    const char** texUnitsPrefixArray;
    u32bit gpuMemoryChannels;
    u32bit systemMemoryBuses;
    u32bit numStampUnits;
    const char** stampUnitsPrefixArray;
    u32bit numStrLoaderUnits;
    const char** streamerLoaderPrefixArray;
    u32bit memReadLatency;
    u32bit systemMemoryReadLatency;
    u32bit systemMemoryWriteLatency;
    u32bit requestQueueSize;
    u32bit serviceQueueSize;
    // u32bit freeServices;

    u32bit maxConsecutiveReads;
    u32bit maxConsecutiveWrites;
    u32bit readBufferSize;
    u32bit writeBufferSize;

    // MemoryRequestSplitter& selectSplitter(const MemoryTransaction* mt);
    MemoryRequestSplitter& selectSplitter(u32bit address, u32bit size) const;

    void processCommand(u64bit cycle);

    /**
     * Reset contents of internal structures
     */
    void reset();

    ///////////////////////////////////////
    // Memory controller pipeline stages //
    ///////////////////////////////////////
    void stage_readRequests(u64bit cycle);
    void stage_sendToSchedulers(u64bit cycle);
    void stage_receiveFromSchedulers(u64bit cycle);
    void stage_updateCompletedRequests(u64bit cycle);
    void stage_serveRequest(u64bit cycle);

    /// Implements the logic required to keep track of bus occupation
     // and transmission duration
    void updateBusCounters(u64bit cycle);
    /// Implements the logic managing the buses arbitration 
    void reserveGPUBus(u64bit cycle);
    
    // Called inside stage_readRequests for each incoming request
    void processMemoryTransaction(u64bit cycle, MemoryTransaction* memTrans);
    
    // Called by processMemoryTransaction: 
    //    Adds a new memory transaction to the GPU memory request queue 
    u32bit addRequest(MemoryTransaction* memTrans, u64bit cycle);
    // Called by processMemoryTransaction:
    //    Adds a new memory transaction to the GPU memory request queue
    u32bit addSystemMemoryRequest(MemoryTransaction* mt, u64bit cycle);

    // Create signals to keep track of bus bw (feedback to STV)
    void createUsageSignals(const MemoryControllerParameters& params);
    // Create I/O signals (input/output buses)
    void createIOSignals(); 

    void createStatistics();

    // Read all the usage 
    void readUsageBuses(u64bit cycle);

    void writeUsageBuses(u64bit cycle);

    // Implements write the data of a memory transaction into 
    // memory modules directly (without timing overhead)
    void preloadGPUMemory(MemoryTransaction* mt);

    // System memory methods
    // Tries to issue a transaction to system memory
    void issueSystemTransaction(u64bit cycle);
    void updateSystemBuses(u64bit cycle);

    void sendBusStateToClients(u64bit cycle);
    // called over all GPU units by sendBusStateToClients
    void sendBusState(u64bit cycle, GPUUnit unit, MemState state);

    //  Clock/update the GPU clock domain of the Memory Controller.
    void updateGPUDomain(u64bit cycle);
    
    //  Clock/update the Memory clock domain of the Memory Controller.
    void updateMemoryDomain(u64bit cycle);

    static std::string getRangeList(const std::vector<u32bit>& listOfIndices);

public:

    // To initialize the memory controller with a default set of parameters
    static MemoryControllerParameters MemoryControllerDefaultParameters;

    MemoryController(const MemoryControllerParameters& params, 
                     const char** texUnitsPrefixArray,
                     const char** stampUnitsPrefixArray,
                     const char** streamerLoaderPrefixArray,
                     const char* name,
                     bool createInnerSignals = true,
                     Box* parent = 0);

    void clock(u64bit cycle);

    //  Clock update function for multiple clock domain support.
    void clock(u32bit domain, u64bit cycle);

    /** 
     *
     *  Returns a list of the debug commands supported by the Memory Controller.
     *
     *  @param commandList Reference to a string where to store the list of debug commands supported by 
     *  the Memory Controller.
     *
     */

    void getCommandList(string &commandList);

    /** 
     *
     *  Executes a debug command.
     *
     *  @param commandStream A string stream with the command and parameters to execute.
     *
     */

    void execCommand(stringstream &commandStream);

    void saveMemory() const;

    void loadMemory();

    void getDebugInfo(std::string &debugInfo) const;

};


} // namespace memorycontroller
} // namespace gpu3d

#endif // MEMORYCONTROLLER_H

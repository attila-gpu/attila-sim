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
 * $RCSfile: MemoryController.h,v $
 * $Revision: 1.25 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:47 $
 *
 * Memory Controller class definition file.
 *
 */

/**
 *
 *  @file MemoryController.h
 *
 *  Memory Controller definition file.  The Memory Controller class defines the box
 *  controlling the access to GPU and system memory and servicing memory requests
 *  from the other GPU units.
 *
 */

#ifndef _MEMORY_CONTROLLER_
    #define _MEMORY_CONTROLLER_

#include "GPUTypes.h"
#include "support.h"
#include "Box.h"
#include "MemorySpace.h"
#include "MemoryControllerDefs.h"

namespace gpu3d
{
    class MemoryTransaction;

/**
 *
 *  Stores the bus width (in bytes) between the Memory Controller and
 *  the GPU units.
 *
 */
//extern u32bit busWidth[];

/**
 *
 *  Strings with the names of the GPU units connected to the Memory Controller.
 *
 */
//extern char *busNames[];

/**  Defines a memory request queue entry.  */
struct MemoryRequest
{
    MemoryTransaction *memTrans;    /**<  The memory transaction that carries the request.  */
    MemReqState state;              /**<  Current state of the request.  */
    u32bit waitCycles;              /**<  Number of cycles the request has been waiting to start being processed.  */
    u32bit transCycles;             /**<  Number of cycles the request has been transmiting data.  */
    bool dependency;                /**<  Flag that stores if the transaction has a dependency.  */
    u32bit waitForTrans;            /**<  Pointer/identifier to the transaction that must precede the memory request.  */
    bool wakeUpWaitingTrans;        /**<  Flag that stores if the transaction, when served must wake up dependant transactions.  */
};

/***  Maximum number of memory transaction ids availables.  */
//static const u32bit MAX_MEMORY_TICKETS = 256;

/**  Read transaction latency.  */
//static const u32bit READ_LATENCY = 10;

/**  Write transaction latency.  */
//static const u32bit WRITE_LATENCY = 5;

/**  Memory transaction transmission cycles (to physical memory).  */
static const u32bit TRANSACTION_CYCLES = 4;

/*** Maximum number of buses per GPU unit type.  */
static const u32bit MAX_GPU_UNIT_BUSES = 16;

/*** Maximum number of consecutive read requests that can be processed when there are pending write requests.  */
//static const u32bit MAX_CONSECUTIVE_READS = 8;

/*** Maximum number of consecutive write requests that can be processed when there are pending read requests.  */
//static const u32bit MAX_CONSECUTIVE_WRITES = 8;

/**
 *
 *  Defines the latency to system memory in cycles.
 *
 */
static const u32bit MAPPED_MEMORY_LATENCY = 500;

/**
 *
 *  Defines the cycles it takes to transmit a memory transaction from/to system memory.
 *
 */
static const u32bit MAPPED_TRANSACTION_CYCLES = 16;

/**
 *
 *  Defines the number of buses to system memory.
 *
 *  In current implementation: two, one for read and one for writes.
 *
 */
static const u32bit MAPPED_MEMORY_BUSES = 2;

/**
 *
 *  Implements a Memory Controller Box.
 *
 *  Simulates the GPU memory controller.
 *
 */

class MemoryController: public Box
{

private:

    u64bit _lastCycle;

    // Defined here for compatibility reasons
    // each entry is initialized with (example):
    // busNames[COMMANDPROCESSOR] = MemoryTransaction::getBusName(COMMANDPROCESSOR);
    const char* busNames[LASTGPUBUS];

    /*  Memory Controller parameters.  */
    u32bit gpuMemorySize;       /**<  Amount of GPU local memory in bytes.  */
    u32bit baseClockMultiplier; /**<  Frequency multiplier applied to the GPU frequency to be used as the reference memory frequency.  */
    u32bit gpuMemoryFrequency;  /**<  Memory frequency in clock cycles from the reference memory frequency.  */
    u32bit gpuMemoryBusWidth;   /**<  Width in bits of each GPU memory bus.  */
    u32bit gpuMemoryBuses;      /**<  Number of data/control buses to memory.  */
    bool gpuSharedBanks;        /**<  Flag that stores if the gpu memory buses access shared or distributed memory banks.  */
    u32bit gpuBankGranurality;  /**<  Access granurality in bytes for the access to the distributed gpu memory banks.  */
    u32bit gpuMemoryBurstLength;/**<  Length of a gpu memory burst data ccess in words (bus width).  */
    u32bit gpuMemReadLatency;   /**<  Cycles until data is received from GPU memory after a read command.  */
    u32bit gpuMemWriteLatency;  /**<  Cycles until data can be sent to GPU memory after a write command.  */
    u32bit gpuMemWriteToReadLatency;    /**<  Cycles to wait after data was written to the GPU memory before issuing a read command.  */
    u32bit gpuMemPageSize;      /**<  Size of a GPU memory page.  */
    u32bit gpuMemOpenPages;     /**<  Number of pages open in a GPU memory chip.  */
    u32bit gpuMemPageOpenLat;   /**<  Latency for opening a page in a GPU memory chip.  */
    u32bit maxConsecutiveReads; /**<  Number of consecutive read transactions than can be processed before processing a write transaction.  */
    u32bit maxConsecutiveWrites;/**<  Number of consecutive write transactions than can be processed before processing a read transaction.  */
    u32bit mappedMemorySize;    /**<  Amount of system memory mapped system into the GPU address space in bytes.  */
    u32bit readBufferSize;      /**<  Size of the Memory Controller buffer for read transactions in lines.  */
    u32bit writeBufferSize;     /**<  Size of the Memory Controller buffer for write transactions in lines.  */
    u32bit requestQueueSize;    /**<  Number of entries of the memory request queue.  */
    u32bit serviceQueueSize;    /**<  Number of entries in the memory service queues per GPU unit buses.  */
    u32bit numStampUnits;       /**<  Number of stamp pipes (Z Stencil and Color Write) attached to the Memory Controller.  */
    u32bit numTextUnits;        /**<  Number of Texture Units attached to the Memory Controller.  */
    u32bit streamerLoaderUnits; /**<  Number of Streamer Loader units attached to the Memory Controller.  */

    /*  Precalculated constants.  */
    u32bit busShift;            /**<  Bit shift required to get the bank for a given GPU memory address.  */
    u32bit busOffsetMask;       /**<  Mask for the offset inside a gpu memory bank.  */
    u32bit pageShift;           /**<  Bit shift required to get the memory page identifier.  */
    u32bit pageBankShift;       /**<  Bit shift required to get the memory page bank identifier.  */
    u32bit pageBankMask;        /**<  Mask for require to get the memory page bank identifier.  */

    /*  Memory buffers.  */
    u8bit *gpuMemory;       /**<  Pointer to the buffer where the GPU local memory is stored.  */
    u8bit *mappedMemory;    /**<  Pointer to the buffer where the mapped system memory is stored.  */

    /**
     * Command signal from the Command Processor.
     */
    Signal* mcCommSignal;

    /*  Memory Controller signals.  */
    Signal *commProcReadSignal;     /**<  Memory read/status signal to the Command Processor.  */
    Signal *commProcWriteSignal;    /**<  Memory write/request signal from the Command Processor.  */
    Signal **streamLoadDataSignal;  /**<  Memory read/status signal to the Streamer Loader units.  */
    Signal **streamLoadReqSignal;   /**<  Memory request signal to the Streamer Loader units.  */
    Signal *streamFetchDataSignal;  /**<  Memory data signal to the Streamer Fetch unit.  */
    Signal *streamFetchReqSignal;   /**<  Memory request signal from the Streamer Fetch.  */
    Signal **zStencilDataSignal;    /**<  Array of memory data signal to the Z Stencil Test unit.  */
    Signal **zStencilReqSignal;     /**<  Array of memory request signal from Z Stencil Test unit.  */
    Signal **colorWriteDataSignal;  /**<  Array of memory data signal to the Color Write unit.  */
    Signal **colorWriteReqSignal;   /**<  Array of memory request signal from Color Write unit.  */
    Signal *dacDataSignal;          /**<  Memory data signal to the DAC unit.  */
    Signal *dacReqSignal;           /**<  Memory request signal from DAC unit.  */
    Signal **tuDataSignal;          /**<  Array of memory request signals from the Texture Units.  */
    Signal **tuReqSignal;           /**<  Array of memory data signals to the Texture Units.  */
    Signal **memoryModuleRequestSignal; /**<  Request signal to the memory modules (simulates memory access latency).  */
    Signal **memoryModuleDataSignal;    /**<  Data signal from the memory modules (simulates memory access latency).  */
    Signal **mappedMemoryRequestSignal; /**<  Request signal to the mapped system memory (simulates system memory access latency).  */
    Signal **mappedMemoryDataSignal;    /**<  Data signal from the mapped system memory (simulates system memory access latency).  */

    /*  Signals and structures to generate bandwidth usage information.  */
    Signal *memBusIn[LASTGPUBUS][MAX_GPU_UNIT_BUSES];   /**<  Signals for displaying bandwidth usage.  */
    Signal *memBusOut[LASTGPUBUS][MAX_GPU_UNIT_BUSES];  /**<  Signals for displaying bandwidth usage.  */
    DynamicObject busElement[LASTGPUBUS][MAX_GPU_UNIT_BUSES][2];    /**<  Stores the cookies for the current transaction in the unit bus.  */
    u32bit elemSelect[LASTGPUBUS][MAX_GPU_UNIT_BUSES];  /**<  Which bus element is currently active.  */

    /*  Memory Controller state.  */
    u32bit freeReadBuffers;     /**<  Number of free read buffers (lines).  */
    u32bit freeWriteBuffers;    /**<  Number of free write buffers (lines).  */
    u32bit memoryCycles;        /**<  Stores the sum of the cycles until the end of all the current gpu memory bus data transmissions.  */
    u32bit systemCycles;        /**<  Stores the sum of the cycles until the end of all the current system memory bus data transmissions.  */
    u32bit busCycles[LASTGPUBUS][MAX_GPU_UNIT_BUSES];   /**<  Number of cycles until the end of the current bus data transmission.  */
    bool reserveBus[LASTGPUBUS][MAX_GPU_UNIT_BUSES];    /**<  Used by the service queues to reserve the unit bus.  */
    u32bit currentTrans[LASTGPUBUS][MAX_GPU_UNIT_BUSES];/**<  Stores a pointer to the current memory transaction in the GPU unit bus.  */
    bool mappedTrans[LASTGPUBUS][MAX_GPU_UNIT_BUSES];   /**<  Stores if the current memory transaction in the GPU unit bus is to mapped system memory.  */
    u32bit **currentPages;      /**<  Stores the identifiers of the memory pages currently open per bus.  */

    /*  GPU memory request queues and buffers.  */
    MemoryRequest *requestBuffer;   /**<  GPU memory request buffer.  */
    u32bit numRequests;             /**<  Number requests for all the GPU memory buses.  */
    u32bit *freeRequestQ;           /**<  Queue storing free entries in the request buffer.  */
    u32bit numFreeRequests;         /**<  Number of free entries in the request buffer.  */

    /*  Per GPU bus read request queues, pointers and counters.  */
    u32bit *lastReadReq;            /**<  Pointers to the last read request in the read request queues.  */
    u32bit *firstReadReq;           /**<  Pointers to the first read request in the read request queues.  */
    u32bit **readRequestBusQ;       /**<  Per GPU memory bus read request queues.  */
    u32bit *numReadBusRequests;     /**<  Number of read requests per GPU memory bus.  */
    u32bit *consecutiveReadReqs;    /**<  Number of consecutive read requests selected per GPU memory bus.  */

    /*  Per GPU bus read request queues, pointers and counters.  */
    u32bit *lastWriteReq;           /**<  Pointers to the last write request in the write request queues.  */
    u32bit *firstWriteReq;          /**<  Pointers to the first write request in the write request queues.  */
    u32bit **writeRequestBusQ;      /**<  Per GPU memory bus write request queues.  */
    u32bit *numWriteBusRequests;    /**<  Number of write requests per GPU memory bus.  */
    u32bit *consecutiveWriteReqs;   /**<  Number of consecutive write requests selected per GPU memory bus.  */

    u32bit *numBusRequests;         /**<  Number of requests per GPU memory bus.  */

    /*  Per memory bus state for the current memory transference.  */
    MemTransCom *lastCommand;       /**<  Array storing the command/type of the last transaction issued per memory bus.  */
    u64bit *lastCycle;              /**<  Array storing the cycle when the last transaction was issued per memory bus.  */
    MemoryTransaction **moduleTrans;/**<  Array with the transaction being transmitted from/to the memory modules.  */
    u32bit *currentBusRequest;      /**<  Pointer to the current request being served in each GPU memory bus.  */
    u32bit *activeBusRequests;      /**<  Stores the number of memory requests active in the pipeline (signal latency).  */

    /*  Mapped memory request queue variables.  */
    MemoryRequest *mappedQueue;     /**<  Mapped system memory request queue.  */
    u32bit lastMapped;              /**<  Pointer to the last request.  */
    u32bit firstMapped;             /**<  Pointer to the first request.  */
    u32bit *readyMapped;            /**<  Ready request FIFO.  */
    u32bit numReadyMapped;          /**<  Number of ready requests.  */
    u32bit *freeMapped;             /**<  Free request entries list.  */
    u32bit numFreeMapped;           /**<  Number of free request entries.  */
    MemoryTransaction **systemTrans;/**<  Array with the transaction beings transmitted from/to the memory modules.  */
    u64bit systemBus[MAPPED_MEMORY_BUSES];  /**<  Stores the cycle of the last transaction to a system memory bus.  */

    /*  Service queues.  */
    MemoryRequest *serviceQueue;    /**<  Stores the memory read requests to be served to the GPU units.  */
    u32bit nextService;             /**<  Pointer to the next request to serve.  */
    u32bit freeServices;            /**<  Number of free service entries in the service queue.  */
    u32bit pendingServices;         /**<  Number of pending service entries in the service queue.  */
    u32bit nextFreeService;         /**<  Pointer to the next free service entry in the service queue.  */
    bool service[LASTGPUBUS][MAX_GPU_UNIT_BUSES];   /**<  If a service is being served in a bus (not a WRITE!!!).  */

    /*  Statistics.  */

    /*  Per GPU Unit type statistics.  */
    GPUStatistics::Statistic *transactions[LASTGPUBUS]; /**<  Transactions received per GPU unit.  */
    GPUStatistics::Statistic *writeTrans[LASTGPUBUS];   /**<  Write transactions received per GPU unit.  */
    GPUStatistics::Statistic *readTrans[LASTGPUBUS];    /**<  Read transactions received per GPU Unit.  */
    GPUStatistics::Statistic *writeBytes[LASTGPUBUS];   /**<  Bytes written per GPU unit.  */
    GPUStatistics::Statistic *readBytes[LASTGPUBUS];    /**<  Bytes read per GPU unit.  */

    /*  Memory controller statistics.  */
    GPUStatistics::Statistic *unusedCycles;             /**<  Cycles in which the memory module is unused.  */
    GPUStatistics::Statistic *preloadTrans;             /**<  Preload transactions received.  */

    /*  GPU memory statistics.  */
    GPUStatistics::Statistic **gpuBusReadBytes;         /**<  Bytes read from a GPU memory bus.  */
    GPUStatistics::Statistic **gpuBusWriteBytes;        /**<  Bytes written to a GPU memory bus.  */
    GPUStatistics::Statistic **dataCycles;              /**<  Cycles in which the GPU memory bus was transmitting data.  */
    GPUStatistics::Statistic **rtowCycles;              /**<  Cycles in which the GPU memory bus was penalized for read to write transaction change.  */
    GPUStatistics::Statistic **wtorCycles;              /**<  Cycles in which the GPU memory bus was penalized for write to read transaction change.  */
    GPUStatistics::Statistic **openPageCycles;          /**<  Cycles in which the GPU memory bus was penalized for opening a new memory page.  */
    GPUStatistics::Statistic **openPages;               /**<  Number of new pages open per GPU memory bus.  */

    /*  System memory statistics.  */
    GPUStatistics::Statistic *sysBusReadBytes;          /**<  Bytes read from system/mapped memory bus.  */
    GPUStatistics::Statistic *sysBusWriteBytes;         /**<  Bytes written to system/mapped memory bus.  */
    GPUStatistics::Statistic *sysDataCycles[MAPPED_MEMORY_BUSES];   /**<  Cycles in which the system memory bus was transmitting data.  */


    void processCommand(u64bit cycle);

    /*  Private functions.  */

    /**
     *
     *  Resets the memory controller.
     *
     */

    void reset();

    /**
     *
     *  Adds a new memory transaction to the gpu memory request queue.
     *
     *  @param memTrans A pointer to the Memory Transaction to add to the queue.
     *
     *  @return A pointer to the request queue entry where the transaction has been stored.
     *
     */

    u32bit addRequest(MemoryTransaction *memTrans);

    /*
     *  Wake up read transactions for a memory bus when a write transaction source of dependencies is processed.
     *
     *  @param bus The read request bus queue where read transactions must be waken up.
     *  @param writeTrans The pointer/identifier of the write transactions that was finished.
     *
     */

    void wakeUpReadRequests(u32bit bus, u32bit writeTrans);

    /**
     *
     *  Wake up write transactions for a memory bus when a read transaction source of dependencies is processed.
     *
     *  @param bus The write request bus queue where write transactions must be waken up.
     *  @param readTrans The pointer/identifier of the read transactions that was finished.
     *
     */

    void wakeUpWriteRequests(u32bit bus, u32bit readTrans);

    /**
     *
     *  Removes a memory request from the gpu memory queue.
     *
     *  @param req The position inside the memory request queue of the request to remove.
     *
     */

    void removeRequest(u32bit req);

    /**
     *
     *  Adds a new memory transaction to the mapped system memory request queue.
     *
     *  @param memTrans A pointer to the Memory Transaction to add to the queue.
     *
     *  @return A pointer to the request queue entry where the transaction has been stored.
     *
     */

    u32bit addMappedRequest(MemoryTransaction *memTrans);

    /**
     *
     *  Removes a memory request from the mapped system memory queue.
     *
     *  @param req The position inside the memory request queue of the request to remove.
     *
     */

    void removeMappedRequest(u32bit req);


    /**
     *
     *  Selects the next memory request to process for a GPU memory bus.
     *
     *  @param bus GPU memory bus for which to select the memory request.
     *
     *  @return Pointer to the request buffer entry storing the request to process.
     *
     */

    u32bit selectNextRequest(u32bit bus);

    /**
     *
     *  Updates the read request queue pointers and counters for a GPU memory bus after the selected
     *  read request has been processed.
     *
     *  @param bus GPU memory bus for which to select the memory request.
     *
     */

    void updateReadRequests(u32bit bus);

    /**
     *
     *  Updates the read request queue pointers and counters for a GPU memory bus after the selected
     *  read request has been processed.
     *
     *  @param bus GPU memory bus for which to select the memory request.
     *
     */

    void updateWriteRequests(u32bit bus);

    /**
     *
     *  Receives a memory transaction from a GPU bus.
     *
     *  @param memTrans The received memory transaction.
     *
     */

    void receiveTransaction(MemoryTransaction *memTrans);

    /**
     *
     *  Tries to issue a memory transaction to a memory module.
     *
     *  @param cycle Current simulation cycle.
     *  @param bus Bus for which to send a transaction.
     *
     */

    void issueTransaction(u64bit cycle, u32bit bus);

    /**
     *
     *  Tries to issue a memory transaction to system memory.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void issueSystemTransaction(u64bit cycle);

    /**
     *
     *  Tries to serve a memory request to the GPU unit that generated it.
     *
     *  @param cycle Current simulation cycle.
     *
     */

    void serveRequest(u64bit cycle);

    /**
     *
     *  Reserves the GPU unit bus the next request to serve in the service queue.
     *
     */

    void reserveGPUBus();

    /**
     *
     *  Sends a transaction with the state of the data bus with the Memory Controller
     *  to the GPU unit.
     *
     *  @param cycle The current simulation cycle.
     *  @param unit The unit to which send the state update.
     *  @param mcState State of the memory controller (if there are enough request entries
     *  to allow transactions from the units).
     *  @param dataSignal Pointer to the data/state signal with the GPU unit.
     *
     */

    void sendBusState(u64bit cycle, GPUUnit unit, MemState mcState, Signal *dataSignal);

    /**
     *
     *  Sends a transaction with the state of the data bus with the Memory Controller
     *  to the GPU unit.  Version for units that support replication.
     *
     *  @param cycle The current simulation cycle.
     *  @param unit The unit to which send the state update.
     *  @param mcState State of the memory controller (if there are enough request entries
     *  to allow transactions from the units).
     *  @param dataSignal Pointer to the data/state signal with the GPU unit.
     *  @param units Number of replicated units.
     *
     */

    void sendBusState(u64bit cycle, GPUUnit unit, MemState mcState, Signal **dataSignal, u32bit units);

    /**
     *
     *  Processes a transaction received back from the memory module.
     *
     *  @param cycle Current simulation cycle.
     *  @param bus Memory module bus from which to receive transactions.
     *
     */

    void moduleTransaction(u64bit cycle, u32bit bus);

    /**
     *
     *  Updates the state of a gpu memory bus.
     *
     *  @param cycle Current simulation cycle.
     *  @param bus Memory module bus to update.
     *
     */

    void updateMemoryBus(u64bit cycle, u32bit bus);

    /**
     *
     *  Processes a transaction received back from the system memory bus.
     *
     *  @param cycle Current simulation cycle.
     *  @param bus Memory module bus from which to receive transactions.
     *
     */

    void mappedTransaction(u64bit cycle, u32bit bus);

    /**
     *
     *  Updates the state of a system memory bus.
     *
     *  @param cycle Current simulation cycle.
     *  @param bus Memory module bus to update.
     *
     */

    void updateSystemBus(u64bit cycle, u32bit bus);

    /**
     *
     *  Calculates the gpu memory bank being accessed by a gpu memory address
     *  based on the access bank granurality.
     *
     *  @param address GPU memory address for which to determine the bank
     *  accessed.
     *
     *  @return The non shared GPU memory bank associated with a gpu memory bus
     *  that is being accessed by the given address.
     *
     */

    u32bit address2Bus(u32bit address);

    /**
     *
     *  Search the open page table for bus.
     *
     *  @param address Address being accessed
     *  @param bus Which bus is being accessed.
     *
     *  @return If the page for the address to be accessed is in the current open page
     *  directory for the bus.
     *
     */

    bool searchPage(u32bit address, u32bit bus);

    /**
     *
     *  Update per bus open page table with a new page.
     *
     *  @param address Address being accessed
     *  @param bus Which bus is being accessed.
     *
     */

    void openPage(u32bit address, u32bit bus);


public:

    /**
     *
     *  Memory Controller Box constructor.
     *
     *  This function creates and initializes a Memory Controller Box.
     *
     *  @param memSize Amount of GPU memory available for the memory controller.
     *  @param clockMult Frequency multiplier applied to the GPU frequency to be used as the memory reference frequency.
     *  @param memFreq Memory frequency as a number of clock cycles from the memory reference frequency.
     *  @param busWidth Width in bits of each independant GPU memory bus.
     *  @param memBuses Number of independant GPU memory buses.
     *  @param sharedBanks Enables access from the gpu memory buses to a shared memory array.
     *  @param bankGranurality Access granurality when shared access through the gpu memory bus is disabled.
     *  @param burstLength Numbers of words (bus width) transfered with each memory access.
     *  @param readLatency Cycles from read command to data from gpu memory.
     *  @param writeLatency Cycles from write command to data towards gpu memory.
     *  @param writeToReadLatency Cycles after last written data until next read command.
     *  @param pageSize Size of GPU memory page in bytes.
     *  @param openPages Number of open pages in a GPU memory chip.
     *  @param pageOpenLat Latency for opening a page in a GPU memory chip.
     *  @param consecutiveReads Number of consecutive read transactions that can be processed before processing the next write transaction.
     *  @param consecutiveWrites Number of consecutive read transactions that can be processed before processing the next write transaction.
     *  @param comProcBus Command Processor data bus width.
     *  @param streamerFetchBus Streamer Fetch data bus width.
     *  @param streamerLoaderBus Streamer Loader data bus width.
     *  @param zStencilTestBus Z Stencil Tes data bus widht.
     *  @param colorWriteBus Color Write data bus width.
     *  @param dacBus DAC data bus width.
     *  @param systemMem Amount of system memory mapped into the GPU address space.
     *  @param readBufferSize Size of the memory controller write transaction buffer (lines).
     *  @param writeBufferSize Size of the memory controller read transaction buffer (lines).
     *  @param reqQueueSize Number of entries in the memory request queue.
     *  @param serviceQueueSize Number of entries in the service queues for the GPU buses.
     *  @param numStampUnits Number of stamp pipes (Z Stencil and Color Write) attached to the Memory Controller.
     *  @param suPrefixArray Array of the signal name prefixes for the attached stamp units.
     *  @param numTxUnits Number of Texture Units attached to the Memory Controller.
     *  @param tuPrefixArray Array of signal name prefixes for the attached Texture Units.
     *  @param streamerLoaderUnits Number of Streamer Loader Units.
     *  @param slPrefixArray Array of signal name prefixes for the attached Streamer Loader units.
     *  @param name Box name.
     *  @param parent Parent Box.
     *
     *  @return An initialized Memory Controller Box.
     *
     */

    MemoryController(u32bit memSize, u32bit clockMult, u32bit memFreq, u32bit busWidth, u32bit memBuses,
        bool sharedBanks, u32bit bankGranurality, u32bit burstLength, u32bit readLatency, u32bit writeLatency,
        u32bit writeToReadLatency, u32bit pageSize, u32bit openPages, u32bit pageOpenLat,
        u32bit consecutiveReads, u32bit consecutiveWrites,
        u32bit comProcBus, u32bit streamerFetchBus, u32bit streamerLoaderBus, u32bit zStencilTestBus,
        u32bit colorWriteBus, u32bit dacBus, u32bit textUnitBus,
        u32bit systemMem, u32bit readBufferSize, u32bit writeBufferSize,
        u32bit reqQueueSize, u32bit serviceQueueSize,
        u32bit numStampUnits, const char **suPrefixArray, u32bit numTxUnits, const char **tuPrefixArray,
        u32bit streamerLoaderUnits, const char **slPrefixArray, char *name, Box* parent = 0);

    /**
     *
     *  The clock function implements the cycle accurated simulation
     *  of the Memory Controller box.
     *
     *  @param cycle The cycle to simulate.
     *
     */

    void clock(u64bit cycle);

    /** 
     *
     *  Returns a list of the debug commands supported by the Memory Controller.
     *
     *  @param commandList Reference to a string where to store the list of debug commands supported by 
     *  the Memory Controller.
     *
     */

    void getCommandList(std::string &commandList);

    /** 
     *
     *  Executes a debug command.
     *
     *  @param commandStream A string stream with the command and parameters to execute.
     *
     */

    void execCommand(std::stringstream &commandStream);

    /**
     *  
     *  Saves the content of the GPU and system memory to a file.
     *
     */
     
    void saveMemory();

    /**
     *
     *  Loads the content of the GPU and system memory from a file.
     *
     */    
     
    void loadMemory();

    
    void getDebugInfo(std::string &debugInfo) const;
    
    
};

} // namespace gpu3d

#endif

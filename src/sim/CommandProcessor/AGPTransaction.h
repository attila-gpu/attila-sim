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
 * $RCSfile: AGPTransaction.h,v $
 * $Revision: 1.11 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:46 $
 *
 * AGP Transaction.
 *
 */

#ifndef _AGPTRANSACTION_

#define _AGPTRANSACTION_

#include "GPUTypes.h"
#include "DynamicObject.h"
#include "GPU.h"
#include <string>
#include <ostream>
#include <iostream>
#include "zfstream.h"

namespace gpu3d
{

/*  Number of bytes per AGP Packet.  */
static const u32bit AGP_PACKET_SIZE = 32;

/*  Number of bits for a AGP packet */
static const u32bit AGP_PACKET_SHIFT = 5;

/**  Defines the different kind of transactions that go through the
     APG port.  */
enum AGPComm
{
    AGP_WRITE,      /**<  AGP write from system memory (system to local).  */
    AGP_READ,       /**<  AGP read to system memory (local to system).  */
    AGP_PRELOAD,    /**<  Preload data into system or gpu memory.  */
    AGP_REG_WRITE,  /**<  AGP write access to GPU registers.  */
    AGP_REG_READ,   /**<  AGP read access to GPU registers.  */
    AGP_COMMAND,    /**<  AGP control command to the GPU Command Processor.  */
    AGP_INIT_END,   /**<  Marks the end of the initialization phase.  */
    AGP_EVENT       /**<  Signal an event to the GPU Command processor.  */
};


/**
 *
 *  This classes defines a transaction through the AGP port between
 *  the main system (CPU, main memory) and the 3D hardware (GPU, local
 *  memory).
 *
 *  Inherits from the Dynamic Object class that supports fast dynamic
 *  creation and destruction of comunication objects and tracing
 *  capabilities.
 *
 */

class AGPTransaction : public DynamicObject
{

private:

    AGPComm agpTrans;       /**<  Type of this AGP Transaction.  */
    u32bit address;         /**<  AGP transaction destination address.  */
    u32bit size;            /**<  Size in bytes of the transmited data.  */
    u8bit *data;            /**<  Pointer to the transmited data.  */
    GPURegister gpuReg;     /**<  GPU register from which read or write.  */
    GPURegData  regData;    /**<  Data to read/write from the GPU register.  */
    u32bit subReg;          /**<  GPU register subregister address.  */
    GPUCommand gpuCommand;  /**<  GPU control command.  */
    u32bit numPackets;      /**<  Number of AGP packets for this transaction.  */
    bool locked;            /**<  Flag marking the transaction must wait until the next batch has finished.  */
    u32bit md;              /**<  Memory descriptor associated with the register write (Used to help parsing AGP Transaction traces).  */

    GPUEvent gpuEvent;      /**<  GPU event.  */
    std::string eventMsg;   /**<  Event message.  */
    
    std::string debugInfo;

public:

    /**
     *
     *  AGP Transaction constructor function.
     *  AGP Read/Write data from/to GPU local memory.
     *
     *  @param address GPU local memory address from/to which
     *  to read/write the data.
     *  @param size Number of bytes to read from the GPU local memory.
     *  @param data A pointer to the buffer where to write/read the
     *  read/written data from/to the GPU local memory.
     *  @param md Identifier of the memory descriptor (GPU driver) associated with this AGP transaction.
     *  @param isWrite TRUE if it is a write operation (system to local).
     *  @param isLocked TRUE if the write operation must wait until the end of the batch.
     *
     *  @return  An initialized AGP Transaction object.
     *
     */

    AGPTransaction(u32bit address, u32bit size, u8bit *data, u32bit md, bool isWrite, bool isLocked = true);

    /**
     *
     *  AGP Transaction constructor function.
     *  AGP Preload data into GPU local memory or system memory.
     *
     *  @param address GPU or system memory address into which to preload the data.
     *  @param size Number of bytes to preload.
     *  @param data A pointer to the buffer from where to preload the data.
     *  @param md Identifier of the memory descriptor (GPU driver) associated with this AGP transaction.
     *
     *  @return  An initialized AGP Transaction object.
     *
     */

    AGPTransaction(u32bit address, u32bit size, u8bit *data, u32bit md);

    /**
     *
     *  AGP Transaction constructor function.
     *  AGP Write to a GPU register.
     *
     *  @param gpuReg  The register identifier where to write.
     *  @param subReg The subregister number where to write.
     *  @param regData The data to write in the register.
     *  @param md Identifier of the memory descriptor (GPU driver) associated with this AGP transaction.
     *
     *  @return An initialized AGP Transaction object.
     *
     */

    AGPTransaction(GPURegister gpuReg, u32bit subReg, GPURegData regData, u32bit md);

    /**
     *
     *  AGP Transaction constructor function.
     *
     *  AGP Read from a GPU register.
     *
     *  @param gpuReg GPU register identifier from which to read.
     *  @param subReg GPU register subregister number from which to read.
     *
     *  @return An initialized AGP Transaction object.
     *
     */

    AGPTransaction(GPURegister gpuReg, u32bit subReg);

    /**
     *
     *  AGP Transaction constructor function.
     *
     *  AGP GPU command.
     *
     *  @param gpuCommand Control command issued to the GPU Command
     *  processor.
     *
     * @return An initialized AGP Transaction object.
     *
     */

    AGPTransaction(GPUCommand gpuCommand);

    /**
     *
     *  AGP Transaction construction function.
     *
     *  AGP Event to be signaled to the Command Processor.
     *
     *  @param event The event to be signaled to the Command Processor.
     *  @param string A string that will be printed with the event.
     *
     */
     
    AGPTransaction(GPUEvent event, string msg);
     
    /**
     *
     *  AGP Transaction constructor function.
     *
     *  AGP Initialization End.
     *
     *  @return An initialized AGP Transaction object.
     *
     */
    
    AGPTransaction();   
     
    /**
     *
     *  AGP Transaction constructor.
     *
     *  Load AGP Transaction from a AGP Transaction trace file.
     *
     *
     *  @param traceFile Reference to a file stream from where to load the AGP Transaction.
     *
     *  @return An initialized AGP transaction object.
     *
     */
     
    AGPTransaction(gzifstream *traceFile);
    
    /**
     *
     *  AGP Transaction constructor.
     *
     *  Clones the AGP Transaction passed as a parameter.
     *
     *  @param sourceAGPTrans Pointer to the AGPTransaction to clone.
     *
     *  @return An initialized AGP transaction object which is a clone of the passed AGPTransaction.
     *
     */
   
    AGPTransaction(AGPTransaction *sourceAGPTrans);
    
    /**
     *
     *  Sets the GPU Register Data attribute of the transaction.
     *
     *  @param gpuData The GPU Register Data to write in the transaction.
     *
     */

    void setGPURegData(GPURegData gpuData);

    /**
     *
     *  Returns the GPU Register Data attribute from the transaction.
     *
     *  @return The GPU Register Data from the transaction.
     *
     */

    GPURegData getGPURegData();


    /**
     *
     *  Returns the AGP Transaction type.
     *
     *  @return The AGP Transaction type.
     *
     */

    AGPComm getAGPCommand();

    /**
     *
     *  Returns the AGP Transaction source/destination address.
     *
     *  @return The AGP Transaction source/destination address.
     *
     */

    u32bit getAddress();

    /**
     *
     *  Returns the pointer to the buffer for the data to read/write
     *  in the AGP Transaction.
     *
     *  @return AGP Transaction data buffer pointer.
     *
     */

    u8bit *getData();

    /**
     *
     *  Returns the amount of bytes that are being transmited
     *  with the AGP Transaction.
     *
     *  @return Size in bytes of the AGP Transaction data.
     *
     */

    u32bit getSize();

    /**
     *
     *  Returns the GPU register identifier destination of the
     *  AGP transaction.
     *
     *  @return The GPU register identifer.
     *
     */

    GPURegister getGPURegister();

    /**
     *
     *  Returns the GPU register subregister number that is the
     *  destination of the AGP Transaction.
     *
     *  @return The GPU register subregister number.
     *
     */

     u32bit getGPUSubRegister();

    /**
     *
     *  Returns the GPU control command issued with the AGP Transaction.
     *
     *  @return The GPU command issued in the AGP Transaction.
     *
     */

    GPUCommand getGPUCommand();

    /**
     *
     *  Returns the GPU event that was signaled in the AGP Transaction.
     *
     *  @return The GPU event signaled in the AGP Transaction.
     *
     */
     
    GPUEvent getGPUEvent();
    
    /**
     *
     *  Returns the message string for the event signaled in the AGP Transaction.
     *
     *  @return The message string for the event signaled to the GPU.
     *
     */     
     
    std::string getGPUEventMsg();
        
    /**
     *
     *  Returns the number of AGP packets for this AGP Transaction.
     *
     *  @return The number of AGP packets for this AGP Transaction.
     *
     */

    u32bit getNumPackets();

    /**
     *
     *  Returns if the AGP transaction is locked and must wait until the
     *  end of the current batch.
     *
     */

    bool getLocked();

    /**
     *
     *  Returns the memory descriptor associated with the AGP transaction.
     *
     *  @return The memory descriptor identifier associated with the AGP transaction.
     *
     */
     
    u32bit getMD();
     
    /**
     *
     *  Saves the AGP transaction object into a file.
     *
     *  @param outFile Pointer to a stream file where the AGP transaction is going to be saved.
     *
     */
    
    void save(gzofstream *outFile);

    /**
     *
     *  Changes an AGP transaction from AGP_WRITE to AGP_PRELOAD.
     *
     */    
     
    void forcePreload();
     
    /**
     * Dumps AGPTransaction info
     */
    void dump(std::ostream& os = std::cout) const;

    void setDebugInfo(const std::string& debugInfo) { this->debugInfo = debugInfo; }
    std::string getDebugInfo() const { return debugInfo; }


    ~AGPTransaction();
};

} // namespace gpu3d

#endif

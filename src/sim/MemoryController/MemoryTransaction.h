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

#ifndef MEMORYTRANSACTION_H
    #define MEMORYTRANSACTION_H

#include "MemoryControllerDefs.h"
#include "DynamicObject.h"
#include <string>

namespace gpu3d {

/**
 *  This class defines a memory transaction from the Command
 *  Processor or other GPU units to the GPU memory controller.
 *
 *  This class inherits from the DynamicObject class
 *  for efficient dynamic creation and destruction of transactions,
 *  and support for tracing.
 *
 */
class MemoryTransaction : public DynamicObject
{
public:

    static const u32bit MAX_TRANSACION_SIZE = 128;
    static const u32bit MAX_WRITE_SIZE = 16;
        
    /**
     *  Creates an initializated Memory Transaction (read, write, preload)
     *
     *  @param memCom Memory command of the Memory Transaction
     *  @param address GPU memory address where to perform the operation
     *  @param size Amount of data affected by this Memory Transaction
     *  @param data Pointer to the buffer where the data for this Memory Transaction
     *              will be found
     *  @param source GPU source unit for the memory transaction
     *  @param id Transaction identifier
     *
     *  @return A new Memory Transaction object.
     */
    MemoryTransaction(MemTransCom memCom, u32bit address, u32bit size, 
                      u8bit *data, GPUUnit source, u32bit id);

    /**
     *  Creates an initializated Memory Transaction (masked write)
     *
     *  @param address GPU memory address where to perform the operation
     *  @param size Amount of data affected by this Memory Transaction
     *  @param data Pointer to the buffer where the data for this Memory Transaction
     *              will be found
     *  @param mask Write mask (per byte)
     *  @param source GPU source unit for the memory transaction
     *  @param id Transaction identifier
     *
     *  @return A new Memory Transaction object.
     */
    MemoryTransaction(u32bit address, u32bit size, u8bit *data, u32bit *mask,
        GPUUnit source, u32bit id);

    /**
     *  Creates an initializated Memory Transaction (read and write).  For units that
     *  support unit replication (Texture Units, Shaders, ...)
     *
     *  @param memCom Memory command of the Memory Transaction
     *  @param address GPU memory address where to perform the operation
     *  @param size Amount of data affected by this Memory Transaction
     *  @param data Pointer to the buffer where the data for this Memory 
     *              Transaction will be found
     *  @param source GPU source unit for the memory transaction
     *  @param sourceID GPU source unit identifier for the memory transaction
     *  @param id Transaction identifier
     *
     *  @return A new Memory Transaction object.
     */
    MemoryTransaction(MemTransCom memCom, u32bit address, u32bit size, u8bit *data,
        GPUUnit source, u32bit sourceID, u32bit id);

    /**
     *  Creates an initializated Memory Transaction (masked write).  For units that
     *  support unit replication (Texture Units, Shaders, ...)
     *
     *  @param address GPU memory address where to perform the operation
     *  @param size Amount of data affected by this Memory Transaction
     *  @param data Pointer to the buffer where the data for this Memory 
     *              Transaction will be found
     *  @param mask Write mask (per byte)
     *  @param source GPU source unit for the memory transaction
     *  @param sourceID GPU source unit identifier for the memory transaction
     *  @param id Transaction identifier
     *
     *  @return A new Memory Transaction object
     */
    MemoryTransaction(u32bit address, u32bit size, u8bit *data, u32bit *mask,
        GPUUnit source, u32bit sourceID, u32bit id);

    /**
     *  Memory Transaction construtor (copying a read request to a read data
     *  transaction)
     *
     *  @param requestTrans Request transaction for which to create a data
     *  Memory Transaction
     *
     *  @return A new read data MemoryTransaction object
     */
    MemoryTransaction(MemoryTransaction *requestTrans);

    /**
     *  Creates an initialized controller state Memory Transaction
     *  (for MT_STATE)
     *
     *  @return state State of the memory controller
     */
    MemoryTransaction(MemState state);

    ////////////////////////////////////////////////////////////////
    /// Virtual constructors for the different transaction types ///
    ////////////////////////////////////////////////////////////////
    // For compatibility reasons regular ctors are still available
    /**
     * Creates a memory transaction READ_DATA from a previous request transaction REQ_DATA
     */
    static MemoryTransaction* createReadData(MemoryTransaction* reqDataTran);

    // Creates a new state memory transaction
    static MemoryTransaction* createStateTransaction(MemState state);

    /**
     * Returns the Memory Transaction command
     */
    MemTransCom getCommand() const;

    u32bit getAddress() const;
    u32bit getSize() const;
    u8bit* getData() const;
    GPUUnit getRequestSource() const;
    u32bit getUnitID() const;
    u32bit getID() const;   
    u32bit getBusCycles() const;
    MemState getState() const;
    bool isMasked() const;
    u32bit* getMask() const;
    u32bit getRequestID() const;
    void setRequestID(u32bit id);

    std::string getRequestSourceStr(bool compactName = false) const;

    bool isToSystemMemory() const;

    std::string toString(bool compact = true) const;

    void dump(bool dumpData = true) const;

    // Creates a string based on the internal representation of the transaction
    // Two exact (field by field) transaction has the same string, two different
    // one has a different identification string
    std::string getIdentificationStr(bool concatData = true) const;


    u32bit getHashCode(bool useData = true) const;

    ~MemoryTransaction();

    static const char* getBusName(GPUUnit unit);
    static u32bit getBusWidth(GPUUnit unit);
    static void setBusWidth(GPUUnit unit, u32bit newBW);

    static u32bit countInstances();


private:

    static u32bit instances;

    // moved here, in previous version these arrays were extern
    static const char* busNames[LASTGPUBUS];
    static u32bit busWidth[LASTGPUBUS];

    MemTransCom command; ///< The operation of this memory transaction
    MemState state; ///< State of the memory controller
    u32bit address; ///< Address for this memory transaction
    u32bit size; ///< Size of the memory transaction in bytes
    u8bit *readData; ///<  Pointer to the buffer where to store data from a MT_READ_DATA transaction (to avoid a copy)
    u8bit writeData[MAX_TRANSACTION_SIZE]; ///< Buffer where to store write data
    u8bit *preloadData; ///<  Buffer with the preloaded data
    bool masked; ///< It is a mask write
    u32bit mask[WRITE_MASK_SIZE]; ///< Write mask per byte
    u32bit cycles; ///< Number of bus cycles (MC to GPU unit) that the transaction consumes
    GPUUnit sourceUnit; ///<  Memory transaction source unit
    u32bit unitID; ///< Identifies between units of the same type
    u32bit ID; ///< Transaction identifier
    u32bit requestID; ///< Request pointer/identifier for the memory transaction

};

} // namespace gpu3d



#endif // MEMORYTRANSACTION_H

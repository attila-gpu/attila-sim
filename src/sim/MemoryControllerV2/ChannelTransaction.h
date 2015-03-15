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

#ifndef CHANNELTRANSACTION_H
    #define CHANNELTRANSACTION_H

#include <string>
#include "DynamicObject.h"
#include "MemoryControllerDefs.h"


namespace gpu3d
{
namespace memorycontroller
{

// forward declaration
class MemoryRequest;

class ChannelTransaction : public DynamicObject
{
public:

    /**
     * Virtual constructor to create a Read Channel Transaction
     *
     * @param memReq The parent memory request
     * @param bank Destination bank
     * @param row The row destination of this channel transaction
     * @param col the starting column
     * @param bytes bytes to be read or read if the transaction has already been performed
     * @param dataBuffer buffer where the read data will be stored
     */
    static ChannelTransaction* createRead(MemoryRequest* memReq, 
                        u32bit channel, u32bit bank, u32bit row, 
                        u32bit col, u32bit bytes, u8bit* dataBuffer);

    /**
     * Virtual constructor to create a Write Channel Transaction
     *
     * @param memReq The parent memory request
     * @param bank Destination bank
     * @param row The row destination of this channel transaction
     * @param col The starting column of the transaction
     * @param bytes bytes to be read or read if the transaction has already been performed
     * @param dataBuffer data to be written
     */
    static ChannelTransaction* createWrite(MemoryRequest* memReq, 
                            u32bit channel, u32bit bank, u32bit row, 
                            u32bit col, u32bit  bytes, u8bit* dataBuffer);

    MemReqState getState() const;

    // equivalent to "getState() == MRS_READY"
    bool ready() const;

    /**
     * Get the unit requester identifier
     */
    GPUUnit getRequestSource() const;

    /**
     * Gets the sub-unit identifier
     */
    u32bit getUnitID() const;

    /**
     * Gets the request ID of the associated request to this channel transaction
     *
     * @note equivalent to: channelTrans->getRequest()->getID();
     *
     * @return the associated request ID
     */
    u32bit getRequestID() const;

    /**
     * Tells if it is a read transaction
     *
     * @return true if the transaction is a read, false otherwise (it is a write)
     */
    bool isRead() const;

    /**
     * Gets the destination channel
     */
    u32bit getChannel() const;

    /**
     * Gets the destination bank
     *
     * This information is temporary (a transaction can be directed to more than one bank)
     * @see MCSPlitter::split method to known more about...
     */
    u32bit getBank() const;

    /**
     * Gets the row of this channel transaction
     *
     * @return the row of this channel transaction
     */
    u32bit getRow() const;

    /**
     * Gets the start column of this channel transaction
     *
     * @return the first column of this transaction
     */
    u32bit getCol() const;
    
    /**
     * Return the amount of bytes in this transaction
     */
    u32bit bytes() const;

    /** 
     * Gets the data of this channel transaction
     *
     * @param offset used to get the data beginning from that offset
     * @return the data associated to this transaction
     *
     * @warning if the transaction is a read the data is not available until the read transaction is performed
     */
    const u8bit* getData(u32bit offset = 0) const;

    /**
     * Sets new data to this transaction
     *
     * @param data new data
     * @param how many bytes are available in the data buffer
     * @param offset offset bytes used to update partially the contents of the channel transaction
     */
    void setData(const u8bit* data, u32bit bytes, u32bit offset);

    /**
     * Gets the parent memory request
     */
    MemoryRequest* getRequest() const;

    bool overlapsWith(const ChannelTransaction& ct) const;

    /**
     * Tells is the transaction is a masked write
     *
     * @note Read transaction always return false
     */
    bool isMasked() const;
    void setMask(const u32bit* mask);

    // @warning offset is expressed in 32bit items
    const u32bit* getMask(u32bit offset = 0) const;

    std::string toString() const;

    void dump(bool showData = false) const;

    static u32bit countInstances();

    ~ChannelTransaction();

private:

    static u32bit instances;

    // set when the channel transactions arrives to the channel queue
    mutable u64bit arrivalTimestamp;
    // set when the channel transaction initiates a pre/act 
    mutable u64bit startPageSetupTimestamp;
    // set when the channel transaction initiates an act, the value set is cycle + a2r or a2w
    mutable u64bit pageReadyTimestamp;
    // set when the channel transaction is selected by the channel scheduler
    mutable u64bit channelTransactionSelectedTimestamp;
    // set when the first read/write ddr command is issued to the DDR modules
   // mutable u64bit startPageAccessTimestamp;

    /**
     * Forbids creating, copying and assigning ChannelTransaction objects directly
     */
    ChannelTransaction();
    ChannelTransaction(const ChannelTransaction&);
    ChannelTransaction& operator=(const ChannelTransaction&);

    enum { CHANNEL_TRANSACTION_MAX_BYTES = 128 };

    //u8bit data[CHANNEL_TRANSACTION_MAX_BYTES];
    u8bit* dataBuffer; // pointer to the input/output buffer
    const u32bit* mask; // if null, it is not masked
    u32bit size;
    bool readBit;
    u32bit channel;
    u32bit bank;
    u32bit row;
    u32bit col;
    MemoryRequest* req;

}; // class ChannelTransaction


} // namespace memorycontroller

} // namespace gpu3d

#endif // CHANNELTRANSACTION_H

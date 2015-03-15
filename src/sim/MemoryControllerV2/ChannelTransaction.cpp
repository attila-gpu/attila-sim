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

#include "ChannelTransaction.h"
#include "MemoryRequest.h"
#include <iostream>
#include <sstream>
#include <cstring>

using namespace gpu3d::memorycontroller;
using gpu3d::MemReqState;

u32bit ChannelTransaction::instances = 0;

ChannelTransaction::ChannelTransaction()
{
    ++instances;
}

ChannelTransaction::~ChannelTransaction()
{
    --instances;
}

u32bit ChannelTransaction::countInstances()
{
    return instances;
}


MemReqState ChannelTransaction::getState() const
{
    return req->getState();
}

bool ChannelTransaction::ready() const
{
    MemReqState state = req->getState();
    return ( state == MRS_READY || state == MRS_MEMORY );
}


ChannelTransaction* ChannelTransaction::createRead(MemoryRequest* memReq, 
                            u32bit channel, u32bit bank, u32bit row, u32bit col, 
                            u32bit bytes, u8bit* dataBuffer)
{
    GPU_ASSERT
    (
        if ( memReq == 0 )
            panic("ChannelTransaction", "createRead", "Memory Request is NULL");
        if ( dataBuffer == 0 )
            panic("ChannelTransaction", "createRead", "Output data buffer is NULL");
    )

    ChannelTransaction* ct = new ChannelTransaction;
    ct->readBit = true; // it is a read
    ct->req = memReq; // memReq must be a read request
    ct->channel = channel;
    ct->bank = bank;
    ct->row = row;
    ct->col = col;
    ct->size = bytes;
    ct->dataBuffer = dataBuffer;
    ct->mask = 0;
    
    ct->setColor(0); // Use a 0 to identify reads
    
    return ct;
}

ChannelTransaction* ChannelTransaction::createWrite(MemoryRequest* memReq,
                    u32bit channel, u32bit bank, u32bit row, u32bit col, 
                    u32bit  bytes, u8bit* dataBuffer)
{
    GPU_ASSERT
    (
        if ( memReq == 0 )
            panic("ChannelTransaction", "createWrite", "Memory Request is NULL");
        if ( dataBuffer == 0 )
            panic("ChannelTransaction", "createRead", "Input data buffer is NULL");
    )

    ChannelTransaction* ct = new ChannelTransaction;
    ct->readBit = false; // it is a write
    ct->req = memReq;
    ct->channel = channel;
    ct->bank = bank;
    ct->row = row;
    ct->col = col;
    ct->size = bytes;
    ct->dataBuffer = dataBuffer;
    ct->mask = 0;

    ct->setColor(1); // Use a 1 to identify writes

    return ct;
}


u32bit ChannelTransaction::getUnitID() const
{
    return req->getTransaction()->getUnitID();
}


gpu3d::GPUUnit ChannelTransaction::getRequestSource() const
{
    return req->getTransaction()->getRequestSource();
}


u32bit ChannelTransaction::getRequestID() const
{
    return req->getID();
}


bool ChannelTransaction::isRead() const
{
    return readBit;
}

u32bit ChannelTransaction::getChannel() const
{
    return channel;
}

u32bit ChannelTransaction::getBank() const
{
    return bank;
}

u32bit ChannelTransaction::getRow() const
{
    return row;
}

u32bit ChannelTransaction::getCol() const
{
    return col;
}

const u8bit* ChannelTransaction::getData(u32bit offset) const
{
    return (dataBuffer + offset);
}

void ChannelTransaction::setData(const u8bit* data_, u32bit bytes, u32bit offset)
{
    if ( bytes + offset > size )
        panic("ChannelTransaction", "setData", "Channel Transaction overflow");

    std::memcpy(dataBuffer+offset, data_, bytes);
}

MemoryRequest* ChannelTransaction::getRequest() const
{
    return req;
}

u32bit ChannelTransaction::bytes() const
{
    return size;
}

bool ChannelTransaction::isMasked() const
{
    return mask != 0;
}

void ChannelTransaction::setMask(const u32bit* mask)
{
    this->mask = mask;
}

const u32bit* ChannelTransaction::getMask(u32bit offset) const
{
    if ( offset >= size/4 )
        panic("ChannelTransaction", "getMask", 
              "Offset cannot be greater that ChannelTransaction_size / 4");
    return mask + offset;
}

std::string ChannelTransaction::toString() const
{
    std::stringstream ss;
    ss << (readBit?"READ":"WRITE") << " ID=" << req->getID() << " bytes=" << size
       << " (C,B,R,Col)=(" << channel << "," << bank << "," << row << "," << col << ")";
    return ss.str();
}

void ChannelTransaction::dump(bool showData) const
{
    using namespace std;    
    cout << "Memory tran. ID = " << req->getID()
         << " Size: " << size
         << " Type: " << (readBit?"READ":"WRITE")
         << " (C,B,R,Col)=(" << channel << "," << bank << "," << row << "," << col << ")";
    
    if ( showData )
    {
        cout << " Data: ";
        for ( u32bit i = 0; i < size; i++ )
        {
            cout << hex << (u32bit)dataBuffer[i] << dec << ".";
        }
    }
}


bool ChannelTransaction::overlapsWith(const ChannelTransaction& ct) const
{
    if ( bank == ct.bank && row == ct.row )
    {
        u32bit minStart = col << 2;
        u32bit maxStart = ct.col << 2;
        u32bit minEnd;
        if ( minStart > maxStart )
        {
            // Swap mins
            u32bit temp = minStart;
            minStart = maxStart;
            maxStart = temp;
            minEnd = minStart + ct.size;
        }
        else
            minEnd = minStart + size;

        return minEnd >= maxStart;
    }
    return false;
}

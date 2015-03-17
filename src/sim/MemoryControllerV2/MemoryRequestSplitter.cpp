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

#include "MemoryRequestSplitter.h"
#include "MemoryRequest.h"
#include "ChannelTransaction.h"
#include <utility> // to include make_pair
#include <iomanip> // to use std::setw manipulator
#include <iostream>
#include <stdio.h>
#include <bitset>

using namespace gpu3d;
using namespace gpu3d::memorycontroller;
using std::vector;
using std::make_pair;

MemoryRequestSplitter::MemoryRequestSplitter(u32bit burstLength, u32bit channels, 
                                             u32bit channelBanks, u32bit bankRows, 
                                             u32bit bankCols) :
    BURST_BYTES(burstLength*4),
    CHANNELS(channels),
    CHANNEL_BANKS(channelBanks),
    BANK_ROWS(bankRows),
    BANK_COLS(bankCols)
{}

vector<ChannelTransaction*> MemoryRequestSplitter::split(MemoryRequest* mr)
{
    MemoryTransaction* memTrans = mr->getTransaction();
    
    GPU_ASSERT
    (
        if ( memTrans == 0 )
            panic("MCSplitter", "split", "the memory transaction of the request is NULL");
        if ( memTrans->getSize() == 0 )
            panic("MCSPlitter", "split", "Request size can not be 0");
    )

    u32bit nextAddress = memTrans->getAddress();

    GPU_ASSERT(
        if ((memTrans->getAddress() % BURST_BYTES) != 0)
        {
            printf("MemoryRequestSplitter (dumping transaction) Memory Transaction : \n");
            memTrans->dump(false);
            printf("\n");
            panic("MemoryRequestSplitter", "split", "Memory transaction is not aligned to burst length.");
        }
    )
    
    vector<std::pair<u32bit,u32bit> > cts;
    
    u32bit totalBytes = memTrans->getSize(); // remaining bytes to be added
    
    // bytes to add in one step
    u32bit bytes = ( totalBytes < BURST_BYTES ? totalBytes : BURST_BYTES );
    totalBytes -= bytes;

    cts.push_back(make_pair(nextAddress, bytes));   

    AddressInfo prevInfo = extractAddressInfo(nextAddress);

    while ( totalBytes != 0 )
    {
        // Compute next address
        nextAddress += bytes;
        
        // check if the next address pertains to the previous channel transaction
        AddressInfo info = extractAddressInfo(nextAddress);

        bytes = ( totalBytes < BURST_BYTES ? totalBytes : BURST_BYTES );
        totalBytes -= bytes;

        if ( info.channel == prevInfo.channel 
             && info.bank == prevInfo.bank // this condition is temporary
             && info.row == prevInfo.row )
        {
            // The second condition enforces that a transaction pertains to a unique
            // bank (this is temporary)
            cts.back().second += bytes; // Accum access in the previous CT
        }
        else
        {
            cts.push_back(make_pair(nextAddress,bytes));
            prevInfo = info;
        }
    }

    vector<ChannelTransaction*> vtrans;

    // Get data pointer
    u8bit* data = memTrans->getData();
    u32bit offset = 0;
    if ( mr->isRead() )
    {
        for ( u32bit i = 0; i < cts.size(); i++ )
        {
            AddressInfo info = extractAddressInfo(cts[i].first);
            vtrans.push_back( ChannelTransaction::createRead(
                                    mr, // The parent memory request
                                    info.channel, // The channel identifier
                                    info.bank, // The bank indentifier
                                    info.row, // The target row 
                                    info.startCol, // The starting column
                                    cts[i].second, // The size in bytes of the Channel T.
                                    data + offset // Pointer to the portion of data of this CT
                            ));
            assertTransaction(vtrans.back());

            offset += cts[i].second; // Accumulated offset into the pointer to data
        }
    }
    else // Write or Preload
    {
        const u32bit* mask = ( mr->isMasked() ? mr->getMask() : static_cast<const u32bit*>(0));
        for ( u32bit i = 0; i < cts.size(); i++ )
        {
            AddressInfo info = extractAddressInfo(cts[i].first);
            vtrans.push_back( ChannelTransaction::createWrite(
                                    mr, // The parent memory request
                                    info.channel, // The channel identifier
                                    info.bank, // The bank indentifier
                                    info.row, // The target row 
                                    info.startCol, // The starting column
                                    cts[i].second, // The size in bytes of the Channel T.
                                    data + offset // Pointer to the portion of data of this CT
                            ));

            assertTransaction(vtrans.back());
            
            if ( mask != 0 )
                vtrans.back()->setMask(mask+offset/4);

            offset += cts[i].second; // Accumulated offset into the pointer to data
        }
    }

    return vtrans;
}

void MemoryRequestSplitter::assertTransaction(const ChannelTransaction* ct) const
{
  if ( ct->getChannel() >= CHANNELS )
  {
      ct->dump();
        panic("MCSplitter", "assertTransaction", "Channel out of bounds");
 }

 if ( ct->getBank() >= CHANNEL_BANKS )
  {
      ct->dump();
        panic("MCSplitter", "assertTransaction", "Bank out of bounds");
    }

 if ( ct->getRow() >= BANK_ROWS )
   {
      ct->dump();
        panic("MCSplitter", "assertTransaction", "Row out of bounds");
 }

 if ( ct->getCol() >= BANK_COLS )
   {
      ct->dump();
        panic("MCSplitter", "assertTransaction", "Column out of bounds");
  }
}

void MemoryRequestSplitter::printAddress(u32bit address) const
{
    using namespace std;

    AddressInfo inf = extractAddressInfo(address);
    cout << "Address (as integer) : 0x" << std::hex << address << std::dec << "\n";
    cout << "As raw bytes (HEX): ";
    const char* ptr = reinterpret_cast<const char*>(&address);

    char prevFill = cout.fill('0'); 

    cout << std::hex << setw(2) << u32bit((u8bit)ptr[0]) << "." << setw(2) 
        << u32bit((u8bit)ptr[1]) << "." << setw(2) << u32bit((u8bit)ptr[2]) 
        << "." << setw(2) << u32bit((u8bit)ptr[3]) << std::dec << "\n";

    cout.fill(prevFill);

    std::bitset<sizeof(u32bit)> bset(address);
    cout << "As bitset: " << bset;

    cout << "\nChannel = " << inf.channel
         << "\nBank = " << inf.bank
         << "\nRow = " << inf.row
         << "\nColumn = " << inf.startCol;
    cout << "\n------------------" << endl;
}

MemoryRequestSplitter::~MemoryRequestSplitter()
{}


u32bit MemoryRequestSplitter::createMask(u32bit n)
{
    u32bit l;
    u32bit m;

    /*  Value 0 has no mask.  */
    if (n == 0)
        return 0;
    /*  Value 1 translates to mask 0x01.  */
    else if (n == 1)
        return 0x01;
    else
    {
        /*  Calculate the shift for the value.  */
        l = createShift(n);

        /*  Build the start mask.  */
        m = (unsigned int) -1;

        /*  Adjust the mask.  */
        m = m >> (32 - l);
    }

    return m;
}

u32bit MemoryRequestSplitter::createShift(u32bit n)
{
    u32bit l;
    u32bit m;

    /*  Value 0 has no shift.  */
    if (n == 0)
        return 0;
    /*  Value 1 translates to 1 bit shift.  */
    else if (n == 1)
        return 1;
    else
    {
        /*  Calculate shift length for the value.  */
        for (l = 0, m = 1; (l < 32) && (n > m); l++, m = m << 1);

        return l;
    }
}

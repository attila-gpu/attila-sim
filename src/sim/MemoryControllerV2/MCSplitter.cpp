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

#include "MCSplitter.h"
#include "MemoryRequest.h"
#include "ChannelTransaction.h"
#include "support.h"

using namespace gpu3d::memorycontroller;
using namespace std;

MCSplitter::MCSplitter(u32bit burstLength, u32bit channels, u32bit channelBanks, 
            u32bit bankRows, u32bit bankCols,
            u32bit channelInterleaving,
            u32bit bankInterleaving) : 
   MemoryRequestSplitter(burstLength, channels, channelBanks, bankRows, bankCols)
{
    GPU_ASSERT
    (
        if ( channelInterleaving < 4*burstLength )
            panic("MCSPlitter", "ctor", "Channel interleaving must be equal to 4*BurstLength bytes or greater");
        if ( bankInterleaving < 4*burstLength )
            panic("MCSplitter", "ctor", "Bank interleaving must be equal to 4*BurstLength bytes or greater");
        if ( channelInterleaving % 4*burstLength != 0 )
            panic("MCSplitter", "ctor", "Channel interleaving must be multiple of 4*BurstLength");
        if ( bankInterleaving % 4*burstLength != 0 )
            panic("MCSplitter", "ctor", "Bank interleaving must be multiple of 4*BurstLength");
    )

    channelMask = createMask(channels);
    channelShift = createShift(channelInterleaving);
    channelInterleavingMask = createMask(channelInterleaving);
    channelInterleavingShift = createShift(channels);

    bankMask = createMask(channelBanks);
    bankShift = createShift(bankInterleaving);
    bankInterleavingMask = createMask(bankInterleaving);
    bankInterleavingShift = createShift(channelBanks);

    rowMask = createMask(bankRows);
    rowShift = createShift(bankCols) + 2; // see below to understand the "+ 2"

    colMask = createMask(bankCols);
    colShift = 2; // a column refers to 4 bytes thus the last two address bits don't care
}

MCSplitter::AddressInfo MCSplitter::extractAddressInfo(u32bit address) const
{
    AddressInfo inf;
    u32bit temp;

    if ( CHANNELS == 1 )
        inf.channel = 0;
    else { 
        // Extract channel bits selector
        inf.channel = ((address >> channelShift) & channelMask);    
        // remove channel bits selector from address
        temp = ((address >> channelInterleavingShift) & ~channelInterleavingMask);
        address = temp | (address & channelInterleavingMask);
    }

    if (  CHANNEL_BANKS == 1 )
        inf.bank = 0;
    else  { 
        // Extract bank bits selector
        inf.bank = ((address >> bankShift) & bankMask);
        // Remove bank bits selector address
        temp = ((address >> bankInterleavingShift) & ~bankInterleavingMask);
        address = temp | (address & bankInterleavingMask);
    }

    // Extract row and column bits selector
    inf.row = ((address >> rowShift) & rowMask);
    inf.startCol = ((address >> colShift) & colMask);

    return inf;    
}

u32bit MCSplitter::createAddress(AddressInfo addrInfo) const
{
    u32bit address;
    u32bit temp;
    
    address = (addrInfo.row << rowShift) | (addrInfo.startCol << colShift);
    
    address = ((address & ~bankInterleavingMask) << bankInterleavingShift) |
              (addrInfo.bank << bankShift) | (address & bankInterleavingMask);
              
    address = ((address & ~channelInterleavingMask) << channelInterleavingShift) |
              (addrInfo.channel << channelShift) | (address & channelInterleavingMask);
              
    return address;              
}


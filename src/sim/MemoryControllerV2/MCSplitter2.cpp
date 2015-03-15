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

#include "MCSplitter2.h"
#include "support.h"
#include <cmath>
#include <set>
#include <sstream>
#include <bitset>
#include <iostream>

using namespace gpu3d;
using namespace gpu3d::memorycontroller;
using namespace std;


MCSplitter2::MCSplitter2(u32bit burstLength, u32bit channels, u32bit channelBanks,
                    u32bit bankRows, u32bit bankCols,
                    string channelMask, string bankMask ) :
    MemoryRequestSplitter(burstLength, channels, channelBanks, bankRows, bankCols)
{
    bitset<8*sizeof(u32bit)> chBitset(channels);
    bitset<8*sizeof(u32bit)> bBitset(channelBanks);
    if ( channels != 1 && chBitset.count() != 1 )
        panic("MCSplitter2", "ctor", "Only a number of channels power of 2 supported");
    if ( channelBanks != 1 && bBitset.count() != 1 )
        panic("MCSplitter2", "ctor", "Only a number of banks power of 2 supported");

    std::set<u32bit> selectedBits;
    channelBits = getBits(channelMask, selectedBits, channels, "Channel");
    bankBits = getBits(bankMask, selectedBits, channelBanks, "Bank");

    // channelBits.sort();
    // bankBits.sort();

    list<u32bit> channelBitsToRemove, toRemove;
    channelBitsToRemove = channelBits; // Copy channel bit list
    bitsToRemove = bankBits;           // Copy bank bit list
    bitsToRemove.splice(bitsToRemove.begin(), channelBitsToRemove); // merge the two lists
    bitsToRemove.sort(std::greater<u32bit>());
    
    colMask = createMask(bankCols);
    colShift = 2;
    rowMask = createMask(bankRows);
    rowShift = createShift(bankCols) + colShift;
}

list<u32bit> MCSplitter2::getBits(string bitMask, set<u32bit>& selectedBits, 
                                  u32bit selections, string bitsTarget)
{
    list<u32bit> bitPositions;
    std::stringstream ss(bitMask);
    u32bit bit = 32;
    while ( ss >> bit )
    {
        if ( bit > 31 )
        {    
            stringstream msg;
            msg << bitsTarget << " bitmask selecting bit (" << bit << ") greater than 31 or unknown value";
            panic("MCSplitter2", "getBits", msg.str().c_str());
        }
        if ( selectedBits.count(bit) )
        {
            stringstream msg;
            msg << bitsTarget << " bitmask selecting a bit (" << bit << ") already used";
            panic("MCSplitter2", "getBits", msg.str().c_str());
        }
        selectedBits.insert(bit);
        bitPositions.push_front(bit);        
    }

    u32bit expectedBits = (u32bit)ceil(log((f64bit)selections)/log(2.0));
    if ( bitPositions.size() != expectedBits )
    {
        cout << "Bits = " << bitPositions.size() << "  Expected bits = " 
             << expectedBits << "\n";
        stringstream ss;
        ss << bitsTarget << " bitmask with not the exact bits to select the " << bitsTarget;
        panic("MCSplitter2", "getBits", ss.str().c_str());
    }

    return bitPositions;
}


u32bit MCSplitter2::getValue(const list<u32bit>& bits, u32bit address)
{
    std::bitset<sizeof(u32bit)*8> addressbits(address);
    u32bit expvalue = 1;
    u32bit value = 0;
    list<u32bit>::const_iterator it = bits.begin();
    for ( ; it != bits.end(); it++ ) {
        if ( addressbits.test(*it) )
            value += expvalue;
        expvalue <<= 1;
    }
    return value;
}

u32bit MCSplitter2::removeBits(u32bit addr, const list<u32bit> &bitsToRemove)
{
    list<u32bit>::const_iterator it = bitsToRemove.begin();
    for ( ; it != bitsToRemove.end(); it++ )
    {
        u32bit mask = (1 << *it) - 1;
        u32bit low = addr & mask;
        addr = ((addr >> 1) & ~mask) | low;
    }
    return addr;
}
    
MemoryRequestSplitter::AddressInfo MCSplitter2::extractAddressInfo(u32bit address) const
{
    AddressInfo inf;
    inf.channel = getValue(channelBits, address);
    inf.bank = getValue(bankBits, address);
    address = removeBits(address, bitsToRemove); // Remove channel & bank bits
    
    // Extract row and column
    // Last two bits ignored (columns have size multiple of 4)
    // Expected format: @ = R...RRCC..CCxx (bits 1 and 0 ignored)
    inf.row = ((address >> rowShift) & rowMask);
    inf.startCol = ((address >> colShift) & colMask);
    return inf;
}

u32bit MCSplitter2::createAddress(AddressInfo addrInfo) const
{
    u32bit address = 0;

    u32bit bank = addrInfo.bank;
    u32bit channel = addrInfo.channel;

    u32bit tempAddress = (addrInfo.row << rowShift) | (addrInfo.startCol << colShift);

    list<u32bit>::const_iterator itChannel = channelBits.begin();
    list<u32bit>::const_iterator itBank = bankBits.begin();
    
    u32bit nextChannelBit;
    u32bit nextBankBit;
    
    if (itChannel != channelBits.end())    
        nextChannelBit = *itChannel;
    else
        nextChannelBit = 32;
    
    if (itBank != bankBits.end())    
        nextBankBit = *itBank;
    else
        nextBankBit = 32;        
    
    for(u32bit bit = 0; bit < 32; bit++)
    {
        if (bit == nextChannelBit)
        {
            address = address | ((channel & 0x01) << bit);
            channel = channel >> 1;
            
            itChannel++;
            
            if (itChannel != channelBits.end())
                nextChannelBit = *itChannel;
            else
                nextChannelBit = 32;                
        }
        else if (bit == nextBankBit)
        {
            address = address | ((bank & 0x01) << bit);
            bank = bank >> 1;
            
            itBank++;
            
            if (itBank != bankBits.end())
                nextBankBit = *itBank;
            else
                nextBankBit = 32;                
        }
        else if ((bit != nextChannelBit) && (bit != nextBankBit))
        {        
            address = address | ((tempAddress & 0x01) << bit);
            tempAddress = tempAddress >> 1;
        }
        else
        {
            panic("MCSpliter2", "createAddress", "Same bit used for channel and bank.");
        }
    }
    
    return address;   
}

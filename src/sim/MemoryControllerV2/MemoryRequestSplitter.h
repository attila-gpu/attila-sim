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

#ifndef MEMORYREQUESTSPLITTER_H
    #define MEMORYREQUESTSPLITTER_H

#include <vector>
#include "GPUTypes.h" // to include u32bit type

namespace gpu3d 
{
namespace memorycontroller
{

class MemoryRequest;
class ChannelTransaction;

// Interface for Splitters splitting requests
class MemoryRequestSplitter
{
public:

    // Public types
    typedef std::vector<ChannelTransaction*> CTV;
    typedef CTV::iterator CTVIt;
    typedef CTV::const_iterator CTVCIt;

    MemoryRequestSplitter(u32bit burstLength, u32bit channels, u32bit channelBanks,
                          u32bit bankRows, u32bit bankCols);
                          //u32bit channelInterleaving, u32bit bankInterleaving);

    // Called to split a mr in several channel transactions
    CTV split(MemoryRequest* mr);

    // Prints a representation of the address in the standard output
    virtual void printAddress(u32bit address) const;
    
    struct AddressInfo
    {
        u32bit channel;
        u32bit bank;
        u32bit row;
        u32bit startCol;
    };

    // Creates and address given a channel, bank, row and column
    virtual u32bit createAddress(AddressInfo addrInfo) const = 0;

    // Must be implemented by each subclass (is called by split method)
    virtual AddressInfo extractAddressInfo(u32bit address) const = 0;

    virtual ~MemoryRequestSplitter() = 0;

protected:

    // Constants initialized at Splitter creation
    const u32bit BURST_BYTES;
    const u32bit CHANNELS;
    const u32bit CHANNEL_BANKS;
    const u32bit BANK_ROWS;
    const u32bit BANK_COLS;

    // Helper static methods
    static u32bit createMask(u32bit value);
    static u32bit createShift(u32bit value);



private:

    void assertTransaction(const ChannelTransaction* ct) const;

};

} // namespace memorycontroller
} // namespace gpu3d

#endif

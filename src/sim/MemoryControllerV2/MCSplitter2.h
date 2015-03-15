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

#ifndef MCSPLITTER2_H
    #define MCSPLITTER2_H

#include <list>
#include <set>
#include "GPUTypes.h"
#include "MemoryRequestSplitter.h"

namespace gpu3d
{
namespace memorycontroller
{

/**
 * Object used to split memory request into channel transactions
 * This version is enhanced to support arbitrary selection of bits address
 *
 * This object has to take into account the address bits
 */
class MCSplitter2 : public MemoryRequestSplitter
{
public:


    /**
     * ctor
     *
     * @param channelMask string representing the bits that will be selected to compose
     *        the channel identifier
     * @param bankMask string representing the bits that will be selected to compose
     *        the bank identifier
     */
    MCSplitter2(u32bit burstLength, u32bit channels, u32bit channelBanks,
                    u32bit bankRows, u32bit bankCols,
                    std::string channelMask, std::string bankMask );

    virtual u32bit createAddress(AddressInfo addrInfo) const;

    virtual AddressInfo extractAddressInfo(u32bit address) const;

protected:

private:

    std::list<u32bit> channelBits;
    std::list<u32bit> bankBits;
    std::list<u32bit> bitsToRemove;
    u32bit rowMask;
    u32bit rowShift;
    u32bit colMask;
    u32bit colShift;

    static std::list<u32bit> getBits(std::string bitMask, std::set<u32bit>& selected, 
                             u32bit selections, std::string bitsTarget);

    static u32bit removeBits(u32bit addr, const std::list<u32bit> &bitsToRemove);

    static u32bit getValue(const std::list<u32bit>& bits, u32bit address);

}; // class MCSplitter2

}
}

#endif // MCSPLITTER2_H

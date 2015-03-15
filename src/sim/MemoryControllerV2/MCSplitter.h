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

#ifndef MCSPLITTER_H
    #define MCSPLITTER_H

#include <vector>
#include "GPUTypes.h"
#include "MemoryRequestSplitter.h"

namespace gpu3d
{
namespace memorycontroller
{

/**
 * Object used to split memory request into channel transactions
 *
 * This object has to take into account the address bits
 */
class MCSplitter : public MemoryRequestSplitter
{
public:


    MCSplitter(u32bit burstLength, u32bit channels, u32bit channelBanks,
               u32bit bankRows, u32bit bankCols,
               u32bit channelInterleaving, u32bit bankInterleaving );

    virtual u32bit createAddress(AddressInfo addrInfo) const;

    virtual AddressInfo extractAddressInfo(u32bit address) const;

protected:

private:
    
    u32bit channelMask;// = 0x7;
    u32bit bankMask;// = 0x7;
    u32bit rowMask; // = 0xFFF;
    u32bit colMask;// = 0x1FF;  

    u32bit channelShift;
    u32bit bankShift;
    u32bit rowShift;
    u32bit colShift;

    // Auxiliar masks and shifts to remove address bits
    u32bit channelInterleavingMask;
    u32bit channelInterleavingShift;
    u32bit bankInterleavingMask;
    u32bit bankInterleavingShift;

};

} // namespace memorycontroller
} // namespace gpu3d

#endif // MCSPLITTER_H

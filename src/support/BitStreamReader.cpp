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

#include "BitStreamReader.h"

namespace gpu3d
{

u32bit BitStreamReader::read(unsigned int bits)
{
    u32bit r = 0;
    
    bool enoughBits = bits <= validBits;
    int minBits = enoughBits ? bits : validBits;
    int remBits = bits - minBits;

    r = value & mask[minBits];
    
    if (enoughBits) {    
        value >>= bits;
        validBits -= bits;
    }
    else {
        value = *(buffer++);
        r |= (value & mask[remBits]) << validBits;
        value >>= remBits;
        validBits = maxBits - remBits;
    }
    
    readedBits += bits;
    
    // Alternative without if's
    /*r = value & mask[minBits];
    value = enoughBits ? value >> bits : *buffer;
    buffer += enoughBits ? 0 : 1;
    r |= (value & mask[remBits]) << validBits;
    value >>= remBits;
    validBits = enoughBits ? validBits - bits : maxBits - remBits;*/
    return r;
}

}

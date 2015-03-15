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

#ifndef BITSTREAM_H_
#define BITSTREAM_H_

#include "GPUTypes.h"

namespace gpu3d
{

template <size_t N, size_t base=2> 
struct const_log {
    enum { value = 1 + const_log<N/base, base>::value }; 
};

template <size_t base>
struct const_log<1, base> {
    enum { value = 0 };
};

template <size_t base>
struct const_log<0, base> {
    enum { value = 0 };
};

/* Rudimentary bit stream support.
 * See also BitStreamReader and BitStreamWriter. */

class BitStream
{
public:
    BitStream() {}
    virtual ~BitStream() {}
    
protected:
    static const int maxBits = sizeof(u32bit) * 8;
    static const int maxBitsLog2 = const_log<maxBits, 2>::value;
        
    static const u32bit mask[maxBits + 1];
};

}

#endif /*BITSTREAM_H_*/

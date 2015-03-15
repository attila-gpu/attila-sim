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

#ifndef BITSTREAMREADER_H_
#define BITSTREAMREADER_H_

#include "GPUTypes.h"
#include "BitStream.h"

namespace gpu3d
{

class BitStreamReader : public BitStream
{
public:
    BitStreamReader(u32bit* buffer)
        : buffer(buffer), value(0), validBits(0), readedBits(0) {}
    
    virtual ~BitStreamReader() {}
    
    u32bit read(unsigned int bits);
    
    unsigned int getReadedBits() {
        return readedBits;
    }
    
private:
    u32bit* buffer;
    u32bit value;
    unsigned int validBits;
    unsigned int readedBits;
};

}

#endif /*BITSTREAMREADER_H_*/

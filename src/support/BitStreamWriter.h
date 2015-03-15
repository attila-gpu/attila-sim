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

#ifndef BITSTREAMWRITER_H_
#define BITSTREAMWRITER_H_

#include "GPUTypes.h"
#include "BitStream.h"

namespace gpu3d
{

class BitStreamWriter : public BitStream
{
public:
    BitStreamWriter(u32bit *buffer)
        : buffer(buffer), value(0), bitPos(0), writtenBits(0) {}
    
    virtual ~BitStreamWriter() { flush(); }
    
    void write(u32bit data, unsigned int bits);
    
    void flush();
    
    unsigned int getWrittenBits() {
        return writtenBits;
    }
    
private:
    u32bit* buffer;
    u32bit value;
    unsigned int bitPos;
    unsigned int writtenBits;
};

}

#endif /*BITSTREAMWRITER_H_*/

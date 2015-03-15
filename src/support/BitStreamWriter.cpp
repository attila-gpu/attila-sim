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

#include "BitStreamWriter.h"

namespace gpu3d
{

void BitStreamWriter::write(u32bit data, unsigned int bits)
{
    value |= (data & mask[bits]) << bitPos;
    bitPos += bits;
    if (bitPos >= maxBits) {
        bitPos &= (1 << maxBitsLog2) - 1;
        
        *(buffer++) = value;
        
        value = (data >> (bits - bitPos)) & mask[bitPos];
    }
    writtenBits += bits;
}
    
void BitStreamWriter::flush()
{
    if (bitPos > 0)
        *(buffer++) = value;
    value = 0;
    bitPos = 0;
    writtenBits = 0;
}

}

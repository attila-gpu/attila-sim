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

#include "BitStream.h"

namespace gpu3d
{

const u32bit BitStream::mask[BitStream::maxBits + 1] = {
        0x00000000, 0x00000001, 0x00000003, 0x00000007,  
        0x0000000f, 0x0000001f, 0x0000003f, 0x0000007f,  
        0x000000ff, 0x000001ff, 0x000003ff, 0x000007ff,  
        0x00000fff, 0x00001fff, 0x00003fff, 0x00007fff,  
        0x0000ffff, 0x0001ffff, 0x0003ffff, 0x0007ffff,  
        0x000fffff, 0x001fffff, 0x003fffff, 0x007fffff,  
        0x00ffffff, 0x01ffffff, 0x03ffffff, 0x07ffffff,  
        0x0fffffff, 0x1fffffff, 0x3fffffff, 0x7fffffff,  
        0xffffffff };

}

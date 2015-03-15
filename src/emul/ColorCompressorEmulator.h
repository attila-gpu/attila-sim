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

#ifndef COLORCOMPRESSOREMULATOR_H_
#define COLORCOMPRESSOREMULATOR_H_

#include "CompressorEmulator.h"

namespace gpu3d
{

class ColorCompressorEmulator
{
private:
    static CompressorEmulator* instance;
    
    ColorCompressorEmulator();
    virtual ~ColorCompressorEmulator();
    
public:
    static CompressorEmulator& getInstance() {
        return *instance;
    }
    
    static void configureCompressor(int id);
    
    static const int blockSize = 64;
    
    static const int colorMask = 0xffffffff;
};

}

#endif /*COLORCOMPRESSOREMULATOR_H_*/

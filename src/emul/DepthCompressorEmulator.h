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

#ifndef DEPTHCOMPRESSOREMULATOR_H_
#define DEPTHCOMPRESSOREMULATOR_H_

#include "CompressorEmulator.h"

namespace gpu3d
{

class DepthCompressorEmulator
{
private:
    static CompressorEmulator* instance;
    
    DepthCompressorEmulator();
    virtual ~DepthCompressorEmulator();
    
public:
    static CompressorEmulator& getInstance() {
        return *instance;
    }
    
    static void configureCompressor(int id);
    
    static const int levels = 3;
    static const u32bit depthMask = 0xffffffff;
};

}

#endif /*DEPTHCOMPRESSOREMULATOR_H_*/

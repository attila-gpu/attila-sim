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

#include "DepthCompressorEmulator.h"
#include "HiloCompressorEmulator.h"

#include "GPUMath.h"

namespace gpu3d
{

CompressorEmulator* DepthCompressorEmulator::instance = NULL;

DepthCompressorEmulator::DepthCompressorEmulator()
{
}

DepthCompressorEmulator::~DepthCompressorEmulator()
{
}

void DepthCompressorEmulator::configureCompressor(int id) {
    switch (id) {
    case 0: /* hiloz */
        instance = new HiloCompressorEmulator(levels, depthMask, false);
        break;
    
    default:
        panic("DepthCompressorEmulator", "configureCompressor", "Unknown compressor id.");
    }
}

}

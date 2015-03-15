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

#include "ColorCompressorEmulator.h"
#include "HiloCompressorEmulator.h"
#include "MsaaCompressorEmulator.h"

#include "GPUMath.h"

namespace gpu3d
{

CompressorEmulator* ColorCompressorEmulator::instance = NULL;

ColorCompressorEmulator::ColorCompressorEmulator()
{
}

ColorCompressorEmulator::~ColorCompressorEmulator()
{
}

void ColorCompressorEmulator::configureCompressor(int id) {
    switch (id) {
    case 0: /* the old hilo implementation */
        instance = new HiloCompressorEmulator(2, colorMask, false);
        break;
    case 1: /* hilore */
        instance = new HiloCompressorEmulator(3, colorMask, true);
        break;
    case 2: /* msaa */
        instance = new MsaaCompressorEmulator(blockSize);
        break;
        
    default:
        panic("ColorCompressorEmulator", "configureCompressor", "Unknown compressor id.");
    }
}

}

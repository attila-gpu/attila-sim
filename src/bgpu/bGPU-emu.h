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
 * bGPU emulator definition file.
 *
 */


/**
 *
 *  @file bGPU-emu.h
 *
 *  This file contains definitions and includes for the ATTILA emulator.
 *
 */

#ifndef _BGPU_EMU_

#define _BGPU_EMU_

#include <string>

#include "AGPTraceFile.h"

namespace gpu3d
{

//  Helper functions.
bool checkAGPTrace(AGPTraceFileHeader *agpTraceHeader);
bool checkAGPTraceParameters(AGPTraceFileHeader *agpTraceHeader);
bool has_extension(const std::string file_name, const std::string extension);

#ifdef WIN32
    int abortSignalHandler(int s);
#else
    void abortSignalHandler(int s);
#endif

void segFaultSignalHandler(int s);

} // namespace gpu3d;

#endif

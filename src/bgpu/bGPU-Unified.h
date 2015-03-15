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
 * $RCSfile: bGPU-Unified.h,v $
 * $Revision: 1.10 $
 * $Author: csolis $
 * $Date: 2007-08-02 17:41:52 $
 *
 * bGPU definition file (unified shader version).
 *
 */


/**
 *
 *  @file bGPU-Unified.h
 *
 *  This file contains definitions and includes for the
 *  main simulation loop (unified shader version).
 *
 */

#ifndef _BGPU_

#define _BGPU_

#include "AGPTraceFile.h"

#include <string>

namespace gpu3d
{

void abortSignalHandler(int s);

void segFaultSignalHandler(int s);

bool checkAGPTrace(AGPTraceFileHeader *agpTraceHeader);
bool checkAGPTraceParameters(AGPTraceFileHeader *agpTraceHeader);

// case insensitive file extension test
bool has_extension(const std::string file_name, const std::string extension);


} // namespace gpu3d;

#endif

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
 * $RCSfile: MemorySpace.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2006-06-21 11:43:46 $
 *
 * Memory Space definition file.
 *
 */

/**
 *
 *  @file MemorySpace.h
 *
 *  Memory Space definition file.  Defines masks describing how memory is mapped on the
 *  GPU address space.
 *
 */

#ifndef _MEMORY_SPACE_

#define _MEMORY_SPACE_

#include "GPUTypes.h"


namespace gpu3d
{

/**
 *
 *  Address space for the GPU memory.
 *
 */
static const u32bit GPU_ADDRESS_SPACE = 0x00000000;

/**
 *
 *  Address space for the system memory.
 *
 */
static const u32bit SYSTEM_ADDRESS_SPACE = 0x80000000;

/**
 *
 *  Address space mask.
 *
 */
static const u32bit ADDRESS_SPACE_MASK = 0x80000000;

/**
 *
 *  Address space mask.
 *
 */
static const u32bit SPACE_ADDRESS_MASK = 0x7fffffff;

} // namespace gpu3d

#endif

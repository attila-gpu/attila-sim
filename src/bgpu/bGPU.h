/**************************************************************************
 *
 * Copyright (c) 2002, 2003 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: bGPU.h,v $
 * $Revision: 1.14 $
 * $Author: cgonzale $
 * $Date: 2006-06-21 11:43:44 $
 *
 * bGPU definition file.
 *
 */


/**
 *
 *  @file bGPU.h
 *
 *  This file contains definitions and includes for the
 *  main simulation loop.
 *
 */

#ifndef _BGPU_

#define _BGPU_

/*  Library functions.  */
#include <cstdlib>
#include <cstdio>
#include <ostream>
#include <unistd.h>

#include "GPU.h"

/*  Emulators.  */
#include "ShaderEmulator.h"
#include "RasterizerEmulator.h"
#include "TextureEmulator.h"
#include "FragmentOpEmulator.h"

/*  Simulator boxes.  */
#include "ShaderFetch.h"
#include "ShaderDecodeExecute.h"
#include "CommandProcessor.h"
#include "MemoryControllerSelector.h"
//#include "MemoryController.h"
//#include "MemoryControllerV2.h"
#include "Streamer.h"
#include "PrimitiveAssembly.h"
#include "Clipper.h"
#include "Rasterizer.h"
#include "TextureUnit.h"
#include "ZStencilTest.h"
#include "ColorWrite.h"
#include "DAC.h"

/*  Simulation support files.  */
#include "StatisticsManager.h"
#include "SignalBinder.h"
#include "OptimizedDynamicMemory.h"
#include "support.h"

namespace gpu3d
{

void simulationLoop();

void dumpLatencyMap(u32bit width, u32bit height);

u32bit applyColorKey(u32bit p);

} // namespace gpu3d;

#endif

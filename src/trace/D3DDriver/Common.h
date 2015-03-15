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

#ifndef COMMON_H_INCLUDED
#define COMMON_H_INCLUDED

#include <map>
#include <set>
#include <list>
#include <algorithm>
#include <string>

// #include <inttypes.h>
#include <iostream>
#include <fstream>
using namespace std;

#include <d3d9_port.h>
#include <support.h>
#include "ToString.h"

//typedef unsigned int uint32_t;
//typedef uint32_t u32bit;

// Global settings

//#define D3D_DEBUG_ON

/*
 NOTE: Alpha test introduces extra code in pixel shaders. It works fine for all
combinations in small traces, but with HL2 traces the simulation halts with a
panic. The problem is that at some point the simulator tries to load a pixel
shader and it finds garbage in memory.

I've track the problem and is related to the "clear using quad" feature. If
you disable this feature the simulation finishes without problems. This makes
sense because "clear using quad" stores the gpu pixel shader state, then loads
a pixel shader for doing the clear and finally restores the previous shader
loading it again on gpu.
*/
#define D3D_ENABLE_CLEAR_USING_RECTANGLE

// Capabilities
#define MAX_D3D_VS_CONSTANTS 256
#define MAX_D3D_PS_CONSTANTS 256
#define MAX_D3D_SAMPLERS 8
#define MAX_D3D_STREAMS 16

// Macros
#ifdef D3D_DEBUG_ON
    #define D3D_DEBUG(expr) { expr }
#else
    #define D3D_DEBUG(expr) { }
#endif

#ifdef D3D9_CALL_ON
    #define D3D9_CALL(implemented, expr) { cout << expr << endl; }
#else
    #ifdef D3D9_NONIMPLEMENTED_CALL_ON
        #define D3D9_CALL(implemented, expr) { if(!implemented) cout << expr << endl; }
    #else
        #define D3D9_CALL(implemented, expr) { }
    #endif
#endif




#endif


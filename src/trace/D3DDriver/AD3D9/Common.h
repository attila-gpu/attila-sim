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

#ifndef AINTERFACE_COMMON_H_INCLUDED
#define AINTERFACE_COMMON_H_INCLUDED

#include <d3d9_port.h>
#include "GPUTypes.h"

const size_t COVER_BUFFER_SIZE_9 = 1048576;

extern char cover_buffer_9[];

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

#endif // AINTERFACE_COMMON_H_INCLUDED

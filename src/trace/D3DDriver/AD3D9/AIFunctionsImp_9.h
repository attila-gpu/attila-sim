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

#ifndef IFUNCTIONSIMP_H_INCLUDED
#define IFUNCTIONSIMP_H_INCLUDED

#include "d3d9_port.h"

IDirect3D9 * D3D_CALL Direct3DCreate9(UINT SDKVersion);
void D3D_CALL DebugSetMute(UINT SDKVersion);
void D3D_CALL Direct3DShaderValidatorCreate9(UINT SDKVersion);
void D3D_CALL D3DPERF_BeginEvent(UINT SDKVersion);
void D3D_CALL D3DPERF_EndEvent(UINT SDKVersion);
void D3D_CALL D3DPERF_SetMarker(UINT SDKVersion);
void D3D_CALL D3DPERF_SetRegion(UINT SDKVersion);
void D3D_CALL D3DPERF_QueryRepeatFrame(UINT SDKVersion);
void D3D_CALL D3DPERF_SetOptions(UINT SDKVersion);
void D3D_CALL D3DPERF_GetStatus(UINT SDKVersion);

#endif


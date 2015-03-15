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

#include "Common.h"
#include "D3DInterface.h"
#include "AIFunctionsImp_9.h"

IDirect3D9 * D3D_CALL Direct3DCreate9 (UINT SDKVersion )
{
    return D3DInterface::get_acd_root_9()->Direct3DCreate9(SDKVersion);
}

void D3D_CALL DebugSetMute(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  DebugSetMute  NOT IMPLEMENTED" << endl; )
}

void D3D_CALL Direct3DShaderValidatorCreate9(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  Direct3DShaderValidatorCreate9  NOT IMPLEMENTED" << endl; )
}

void D3D_CALL D3DPERF_BeginEvent(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  D3DPERF_BeginEvent  NOT IMPLEMENTED" << endl; )
}

void D3D_CALL D3DPERF_EndEvent(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  D3DPERF_EndEvent  NOT IMPLEMENTED" << endl; )
}
void D3D_CALL D3DPERF_SetMarker(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  D3DPERF_SetMarker  NOT IMPLEMENTED" << endl; )
}

void D3D_CALL D3DPERF_SetRegion(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  D3DPERF_SetRegion  NOT IMPLEMENTED" << endl; )
}
void D3D_CALL D3DPERF_QueryRepeatFrame(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  D3DPERF_QueryRepeatFrame  NOT IMPLEMENTED" << endl; )
}

void D3D_CALL D3DPERF_SetOptions(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  D3DPERF_SetOptions  NOT IMPLEMENTED" << endl; )
}

void D3D_CALL D3DPERF_GetStatus(UINT SDKVersion)
{
    D3D_DEBUG( cout <<"WARNING:  D3DPERF_GetStatus  NOT IMPLEMENTED" << endl; )
}


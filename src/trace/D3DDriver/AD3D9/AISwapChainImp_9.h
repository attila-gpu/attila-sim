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

#ifndef AISWAPCHAINIMP_9_H_INCLUDED
#define AISWAPCHAINIMP_9_H_INCLUDED

class AISwapChainImp9 : public IDirect3DSwapChain9{
public:
    static AISwapChainImp9 &getInstance();
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL Present (  CONST RECT* pSourceRect , CONST RECT* pDestRect , HWND hDestWindowOverride , CONST RGNDATA* pDirtyRegion , DWORD dwFlags );
    HRESULT D3D_CALL GetFrontBufferData (  IDirect3DSurface9* pDestSurface );
    HRESULT D3D_CALL GetBackBuffer (  UINT iBackBuffer , D3DBACKBUFFER_TYPE Type , IDirect3DSurface9** ppBackBuffer );
    HRESULT D3D_CALL GetRasterStatus (  D3DRASTER_STATUS* pRasterStatus );
    HRESULT D3D_CALL GetDisplayMode (  D3DDISPLAYMODE* pMode );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL GetPresentParameters (  D3DPRESENT_PARAMETERS* pPresentationParameters );
private:
    AISwapChainImp9();
};

#endif

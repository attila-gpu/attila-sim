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
#include "AD3D9State.h"
#include "AIDeviceImp_9.h"
#include "AISurfaceImp_9.h"
#include "AISwapChainImp_9.h"


AISwapChainImp9 :: AISwapChainImp9() {}

AISwapChainImp9 & AISwapChainImp9 :: getInstance() 
{
    static AISwapChainImp9 instance;
    return instance;
}

HRESULT D3D_CALL AISwapChainImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AISwapChainImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AISwapChainImp9 :: AddRef ( ) 
{
    D3D9_CALL(false, "AISwapChainImp9::AddRef")
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL AISwapChainImp9 :: Release ( ) 
{
    D3D9_CALL(false, "AISwapChainImp9::Release")
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL AISwapChainImp9 :: Present (  CONST RECT* pSourceRect , CONST RECT* pDestRect , HWND hDestWindowOverride , CONST RGNDATA* pDirtyRegion , DWORD dwFlags ) 
{
    D3D9_CALL(true, "AISwapChainImp9::Present")

    //D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: Present  NOT IMPLEMENTED" << endl; )
    //HRESULT ret = static_cast< HRESULT >(0);
    //return ret;

    AD3D9State::instance().swapBuffers();

    return D3D_OK;
}

HRESULT D3D_CALL AISwapChainImp9 :: GetFrontBufferData (  IDirect3DSurface9* pDestSurface )
{
    D3D9_CALL(false, "AISwapChainImp9::GetFrontBufferData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AISwapChainImp9 :: GetBackBuffer (  UINT iBackBuffer , D3DBACKBUFFER_TYPE Type , IDirect3DSurface9** ppBackBuffer )
{
    D3D9_CALL(true, "AISwapChainImp9::GetBackBuffer")

    //* ppBackBuffer = & AISurfaceImp9 :: getInstance();
    //D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: GetBackBuffer  NOT IMPLEMENTED" << endl; )
    //HRESULT ret = static_cast< HRESULT >(0);
    //return ret;


    //*ppBackBuffer = (IDirect3DSurface9 *) AD3D9State::instance().getRenderTarget(iBackBuffer);
    *ppBackBuffer = (IDirect3DSurface9 *) AD3D9State::instance().getDefaultRenderSurface();
    
    return D3D_OK;
}

HRESULT D3D_CALL AISwapChainImp9 :: GetRasterStatus (  D3DRASTER_STATUS* pRasterStatus ) 
{
    D3D9_CALL(false, "AISwapChainImp9::GetRasterStatus")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AISwapChainImp9 :: GetDisplayMode (  D3DDISPLAYMODE* pMode ) 
{
    D3D9_CALL(false, "AISwapChainImp9::GetDisplayMode")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AISwapChainImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) 
{
    D3D9_CALL(false, "AISwapChainImp9::GetDevice")
    * ppDevice = & AIDeviceImp9 :: getInstance();
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AISwapChainImp9 :: GetPresentParameters (  D3DPRESENT_PARAMETERS* pPresentationParameters ) 
{
    D3D9_CALL(false, "AISwapChainImp9::GetPresentParameters")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

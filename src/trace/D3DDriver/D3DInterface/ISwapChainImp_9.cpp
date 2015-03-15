#include "Common.h"
#include "IDeviceImp_9.h"
#include "ISurfaceImp_9.h"
#include "ISwapChainImp_9.h"

ISwapChainImp9 :: ISwapChainImp9() {}

ISwapChainImp9 & ISwapChainImp9 :: getInstance() {
    static ISwapChainImp9 instance;
    return instance;
}

HRESULT D3D_CALL ISwapChainImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL ISwapChainImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL ISwapChainImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL ISwapChainImp9 :: Present (  CONST RECT* pSourceRect , CONST RECT* pDestRect , HWND hDestWindowOverride , CONST RGNDATA* pDirtyRegion , DWORD dwFlags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: Present  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISwapChainImp9 :: GetFrontBufferData (  IDirect3DSurface9* pDestSurface ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: GetFrontBufferData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISwapChainImp9 :: GetBackBuffer (  UINT iBackBuffer , D3DBACKBUFFER_TYPE Type , IDirect3DSurface9** ppBackBuffer ) {
    * ppBackBuffer = & ISurfaceImp9 :: getInstance();
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: GetBackBuffer  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISwapChainImp9 :: GetRasterStatus (  D3DRASTER_STATUS* pRasterStatus ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: GetRasterStatus  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISwapChainImp9 :: GetDisplayMode (  D3DDISPLAYMODE* pMode ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: GetDisplayMode  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISwapChainImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    * ppDevice = & IDeviceImp9 :: getInstance();
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: GetDevice  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISwapChainImp9 :: GetPresentParameters (  D3DPRESENT_PARAMETERS* pPresentationParameters ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSwapChain9 :: GetPresentParameters  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

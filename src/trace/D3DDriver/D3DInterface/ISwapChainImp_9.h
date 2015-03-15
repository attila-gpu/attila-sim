#ifndef ISWAPCHAINIMP_9_H_INCLUDED
#define ISWAPCHAINIMP_9_H_INCLUDED

class ISwapChainImp9 : public IDirect3DSwapChain9{
public:
    static ISwapChainImp9 &getInstance();
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
    ISwapChainImp9();
};

#endif

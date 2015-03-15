#ifndef ISURFACEIMP_9_H_INCLUDED
#define ISURFACEIMP_9_H_INCLUDED

class IDeviceImp9;

class ISurfaceImp9 : public IDirect3DSurface9{
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static ISurfaceImp9 &getInstance();

    ISurfaceImp9(StateDataNode* s_parent, IDeviceImp9* i_parent, UINT Width, UINT Height,
    DWORD Usage , D3DFORMAT Format, D3DPOOL Pool);

private:
    /// Singleton constructor method
    ISurfaceImp9();

    IDeviceImp9* i_parent;
    StateDataNode* state;
    ULONG ref_count;

public:
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags );
    HRESULT D3D_CALL GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData );
    HRESULT D3D_CALL FreePrivateData (  REFGUID refguid );
    DWORD D3D_CALL SetPriority (  DWORD PriorityNew );
    DWORD D3D_CALL GetPriority ( );
    void D3D_CALL PreLoad ( );
    D3DRESOURCETYPE D3D_CALL GetType ( );
    HRESULT D3D_CALL GetContainer (  REFIID riid , void** ppContainer );
    HRESULT D3D_CALL GetDesc (  D3DSURFACE_DESC * pDesc );
    HRESULT D3D_CALL LockRect (  D3DLOCKED_RECT* pLockedRect , CONST RECT* pRect , DWORD Flags );
    HRESULT D3D_CALL UnlockRect ( );
    HRESULT D3D_CALL GetDC (  HDC * phdc );
    HRESULT D3D_CALL ReleaseDC (  HDC hdc );
};

#endif


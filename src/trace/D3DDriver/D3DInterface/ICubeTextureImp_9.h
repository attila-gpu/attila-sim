#ifndef ICUBETEXTUREIMP_9_H_INCLUDED
#define ICUBETEXTUREIMP_9_H_INCLUDED

class IDeviceImp9;
class ISurfaceImp9;

class ICubeTextureImp9 : public IDirect3DCubeTexture9{
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static ICubeTextureImp9 &getInstance();

    ICubeTextureImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, UINT EdgeLength,
        UINT Levels, DWORD Usage , D3DFORMAT Format, D3DPOOL Pool);

    ~ICubeTextureImp9();
private:
    /// Singleton constructor method
    ICubeTextureImp9();

    IDeviceImp9* i_parent;
    StateDataNode* state;

    set<ISurfaceImp9*> i_surface_childs;

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
    DWORD D3D_CALL SetLOD (  DWORD LODNew );
    DWORD D3D_CALL GetLOD ( );
    DWORD D3D_CALL GetLevelCount ( );
    HRESULT D3D_CALL SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType );
    D3DTEXTUREFILTERTYPE D3D_CALL GetAutoGenFilterType ( );
    void D3D_CALL GenerateMipSubLevels ( );
    HRESULT D3D_CALL GetLevelDesc (  UINT Level , D3DSURFACE_DESC * pDesc );
    HRESULT D3D_CALL GetCubeMapSurface (  D3DCUBEMAP_FACES FaceType , UINT Level , IDirect3DSurface9** ppCubeMapSurface );
    HRESULT D3D_CALL LockRect (  D3DCUBEMAP_FACES FaceType , UINT Level , D3DLOCKED_RECT* pLockedRect , CONST RECT* pRect , DWORD Flags );
    HRESULT D3D_CALL UnlockRect (  D3DCUBEMAP_FACES FaceType , UINT Level );
    HRESULT D3D_CALL AddDirtyRect (  D3DCUBEMAP_FACES FaceType , CONST RECT* pDirtyRect );
};

#endif

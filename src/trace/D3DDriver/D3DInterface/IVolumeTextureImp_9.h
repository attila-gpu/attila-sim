#ifndef IVOLUMETEXTUREIMP_9_H
#define IVOLUMETEXTUREIMP_9_H

class IDeviceImp9;
class IVolumeImp9;

class IVolumeTextureImp9 : public IDirect3DVolumeTexture9{
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static IVolumeTextureImp9 &getInstance();

    IVolumeTextureImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, UINT Width, UINT Height, UINT Depth,
    UINT Levels, DWORD Usage , D3DFORMAT Format, D3DPOOL Pool);
    ~IVolumeTextureImp9();

private:
    /// Singleton constructor method
    IVolumeTextureImp9();

    IDeviceImp9* i_parent;
    StateDataNode* state;

    set<IVolumeImp9*> i_volume_childs;

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
    HRESULT D3D_CALL GetLevelDesc (  UINT Level , D3DVOLUME_DESC * pDesc );
    HRESULT D3D_CALL GetVolumeLevel (  UINT Level , IDirect3DVolume9** ppVolumeLevel );
    HRESULT D3D_CALL LockBox (  UINT Level , D3DLOCKED_BOX* pLockedVolume , CONST D3DBOX* pBox , DWORD Flags );
    HRESULT D3D_CALL UnlockBox (  UINT Level );
    HRESULT D3D_CALL AddDirtyBox (  CONST D3DBOX* pDirtyBox );

};

#endif

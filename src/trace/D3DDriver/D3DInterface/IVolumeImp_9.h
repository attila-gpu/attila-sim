#ifndef IVOLUMEIMP_9_H_INCLUDED
#define IVOLUMEIMP_9_H_INCLUDED

class IDeviceImp9;

class IVolumeImp9 : public IDirect3DVolume9{
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static IVolumeImp9 &getInstance();

   IVolumeImp9(StateDataNode* s_parent, IDeviceImp9* i_parent, UINT Width, UINT Height, UINT Depth,
        DWORD Usage , D3DFORMAT Format, D3DPOOL Pool);
private:
    /// Singleton constructor method
    IVolumeImp9();

    IDeviceImp9* i_parent;
    StateDataNode* state;

public:
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags );
    HRESULT D3D_CALL GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData );
    HRESULT D3D_CALL FreePrivateData (  REFGUID refguid );
    HRESULT D3D_CALL GetContainer (  REFIID riid , void** ppContainer );
    HRESULT D3D_CALL GetDesc (  D3DVOLUME_DESC * pDesc );
    HRESULT D3D_CALL LockBox (  D3DLOCKED_BOX * pLockedVolume , CONST D3DBOX* pBox , DWORD Flags );
    HRESULT D3D_CALL UnlockBox ( );

    virtual DWORD D3D_CALL SetPriority(DWORD);
    virtual DWORD D3D_CALL GetPriority();
    virtual void D3D_CALL PreLoad();
    virtual D3DRESOURCETYPE D3D_CALL GetType();

};


#endif


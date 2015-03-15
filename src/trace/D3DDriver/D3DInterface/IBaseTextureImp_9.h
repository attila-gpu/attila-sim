#ifndef IBASETEXTUREIMP_9_H
#define IBASETEXTUREIMP_9_H

class IBaseTextureImp9 : public IDirect3DBaseTexture9{
public:
    static IBaseTextureImp9 &getInstance();
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
private:
    IBaseTextureImp9();
};

#endif

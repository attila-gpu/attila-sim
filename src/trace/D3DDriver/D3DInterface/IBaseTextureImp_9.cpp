#include "Common.h"
#include "IDeviceImp_9.h"
#include "IBaseTextureImp_9.h"

IBaseTextureImp9 :: IBaseTextureImp9() {}

IBaseTextureImp9 & IBaseTextureImp9 :: getInstance() {
    static IBaseTextureImp9 instance;
    return instance;
}

HRESULT D3D_CALL IBaseTextureImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IBaseTextureImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL IBaseTextureImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL IBaseTextureImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    * ppDevice = & IDeviceImp9 :: getInstance();
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: GetDevice  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IBaseTextureImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: SetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IBaseTextureImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: GetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IBaseTextureImp9 :: FreePrivateData (  REFGUID refguid ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: FreePrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL IBaseTextureImp9 :: SetPriority (  DWORD PriorityNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: SetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL IBaseTextureImp9 :: GetPriority ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: GetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL IBaseTextureImp9 :: PreLoad ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: PreLoad  NOT IMPLEMENTED" << endl; )
}

D3DRESOURCETYPE D3D_CALL IBaseTextureImp9 :: GetType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: GetType  NOT IMPLEMENTED" << endl; )
    D3DRESOURCETYPE ret = static_cast< D3DRESOURCETYPE >(0);
    return ret;
}

DWORD D3D_CALL IBaseTextureImp9 :: SetLOD (  DWORD LODNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: SetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL IBaseTextureImp9 :: GetLOD ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: GetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL IBaseTextureImp9 :: GetLevelCount ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: GetLevelCount  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL IBaseTextureImp9 :: SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: SetAutoGenFilterType  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DTEXTUREFILTERTYPE D3D_CALL IBaseTextureImp9 :: GetAutoGenFilterType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: GetAutoGenFilterType  NOT IMPLEMENTED" << endl; )
    D3DTEXTUREFILTERTYPE ret = static_cast< D3DTEXTUREFILTERTYPE >(0);
    return ret;
}

void D3D_CALL IBaseTextureImp9 :: GenerateMipSubLevels ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DBaseTexture9 :: GenerateMipSubLevels  NOT IMPLEMENTED" << endl; )
}


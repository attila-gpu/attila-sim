#include "Common.h"
#include "IDeviceImp_9.h"
#include "IResourceImp_9.h"

IResourceImp9 :: IResourceImp9() {}

IResourceImp9 & IResourceImp9 :: getInstance() {
    static IResourceImp9 instance;
    return instance;
}

HRESULT D3D_CALL IResourceImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IResourceImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL IResourceImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL IResourceImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    * ppDevice = & IDeviceImp9 :: getInstance();
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: GetDevice  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IResourceImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: SetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IResourceImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: GetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IResourceImp9 :: FreePrivateData (  REFGUID refguid ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: FreePrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL IResourceImp9 :: SetPriority (  DWORD PriorityNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: SetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL IResourceImp9 :: GetPriority ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: GetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL IResourceImp9 :: PreLoad ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: PreLoad  NOT IMPLEMENTED" << endl; )
}

D3DRESOURCETYPE D3D_CALL IResourceImp9 :: GetType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DResource9 :: GetType  NOT IMPLEMENTED" << endl; )
    D3DRESOURCETYPE ret = static_cast< D3DRESOURCETYPE >(0);
    return ret;
}


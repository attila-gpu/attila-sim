#include "Common.h"
#include "IDeviceImp_9.h"
#include "IQueryImp_9.h"

IQueryImp9 :: IQueryImp9() {}

IQueryImp9 & IQueryImp9 :: getInstance() {
    static IQueryImp9 instance;
    return instance;
}

HRESULT D3D_CALL IQueryImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DQuery9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IQueryImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DQuery9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL IQueryImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DQuery9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL IQueryImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    * ppDevice = & IDeviceImp9 :: getInstance();
    D3D_DEBUG( cout <<"WARNING:  IDirect3DQuery9 :: GetDevice  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DQUERYTYPE D3D_CALL IQueryImp9 :: GetType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DQuery9 :: GetType  NOT IMPLEMENTED" << endl; )
    D3DQUERYTYPE ret = static_cast< D3DQUERYTYPE >(0);
    return ret;
}

DWORD D3D_CALL IQueryImp9 :: GetDataSize ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DQuery9 :: GetDataSize  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL IQueryImp9 :: Issue (  DWORD dwIssueFlags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DQuery9 :: Issue  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IQueryImp9 :: GetData (  void* pData , DWORD dwSize , DWORD dwGetDataFlags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DQuery9 :: GetData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}



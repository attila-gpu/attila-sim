#include "Common.h"
#include "IDeviceImp_9.h"
#include "IDirect3DImp_9.h"

IDirect3DImp9 :: IDirect3DImp9() {}

IDirect3DImp9 & IDirect3DImp9 :: getInstance() {
    static IDirect3DImp9 instance;
    return instance;
}


IDirect3DImp9 :: IDirect3DImp9(StateDataNode* s_parent) {
    state = D3DState::create_direct3d_state_9(this);
    s_parent->add_child(state);
}

IDirect3DImp9 :: ~IDirect3DImp9() {
    set< IDeviceImp9* > :: iterator it;
    for(it = i_childs.begin(); it != i_childs.end(); it ++)
        delete *it;

    /// @todo delete state
}


HRESULT D3D_CALL IDirect3DImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: QueryInterface  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IDirect3DImp9 :: AddRef ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: AddRef  NOT IMPLEMENTED" << endl; ) )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL IDirect3DImp9 :: Release ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: Release  NOT IMPLEMENTED" << endl; ) )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: RegisterSoftwareDevice (  void* pInitializeFunction ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: RegisterSoftwareDevice  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

UINT D3D_CALL IDirect3DImp9 :: GetAdapterCount ( ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: GetAdapterCount  NOT IMPLEMENTED" << endl; ) )
    UINT ret = static_cast< UINT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: GetAdapterIdentifier (  UINT Adapter , DWORD Flags , D3DADAPTER_IDENTIFIER9* pIdentifier ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: GetAdapterIdentifier  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

UINT D3D_CALL IDirect3DImp9 :: GetAdapterModeCount (  UINT Adapter , D3DFORMAT Format ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: GetAdapterModeCount  NOT IMPLEMENTED" << endl; ) )
    UINT ret = static_cast< UINT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: EnumAdapterModes (  UINT Adapter , D3DFORMAT Format , UINT Mode , D3DDISPLAYMODE* pMode ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: EnumAdapterModes  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: GetAdapterDisplayMode (  UINT Adapter , D3DDISPLAYMODE* pMode ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: GetAdapterDisplayMode  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: CheckDeviceType (  UINT Adapter , D3DDEVTYPE DevType , D3DFORMAT AdapterFormat , D3DFORMAT BackBufferFormat , BOOL bWindowed ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: CheckDeviceType  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: CheckDeviceFormat (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT AdapterFormat , DWORD Usage , D3DRESOURCETYPE RType , D3DFORMAT CheckFormat ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: CheckDeviceFormat  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: CheckDeviceMultiSampleType (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT SurfaceFormat , BOOL Windowed , D3DMULTISAMPLE_TYPE MultiSampleType , DWORD* pQualityLevels ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: CheckDeviceMultiSampleType  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: CheckDepthStencilMatch (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT AdapterFormat , D3DFORMAT RenderTargetFormat , D3DFORMAT DepthStencilFormat ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: CheckDepthStencilMatch  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: CheckDeviceFormatConversion (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT SourceFormat , D3DFORMAT TargetFormat ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: CheckDeviceFormatConversion  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: GetDeviceCaps (  UINT Adapter , D3DDEVTYPE DeviceType , D3DCAPS9* pCaps ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: GetDeviceCaps  NOT IMPLEMENTED" << endl; ) )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HMONITOR D3D_CALL IDirect3DImp9 :: GetAdapterMonitor (  UINT Adapter ) {
    D3D_DEBUG( D3D_DEBUG( cout <<"WARNING:  IDirect3D9 :: GetAdapterMonitor  NOT IMPLEMENTED" << endl; ) )
    HMONITOR ret = static_cast< HMONITOR >(0);
    return ret;
}

HRESULT D3D_CALL IDirect3DImp9 :: CreateDevice (  UINT Adapter , D3DDEVTYPE DeviceType , HWND hFocusWindow , DWORD BehaviorFlags , D3DPRESENT_PARAMETERS* pPresentationParameters , IDirect3DDevice9** ppReturnedDeviceInterface ) {
    D3D_DEBUG( D3D_DEBUG( D3D_DEBUG( cout << "IDIRECT3D9: CreateDevice" << endl; ) ) )
    /// @note Adapter, DeviceType, FocusWindow and BehaviorFlags are ignored from here on.
    IDeviceImp9* device = new IDeviceImp9(state, this, pPresentationParameters);
    i_childs.insert(device);
    *ppReturnedDeviceInterface = device;
    return D3D_OK;
}

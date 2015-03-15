#ifndef IDIRECT3DIMP_9_H_INCLUDED
#define IDIRECT3DIMP_9_H_INCLUDED

class IDeviceImp9;

class IDirect3DImp9 : public IDirect3D9{

public:
   /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static IDirect3DImp9 &getInstance();
    
    IDirect3DImp9(StateDataNode* s_parent);
    ~IDirect3DImp9();
private:
    /// Singleton constructor method
    IDirect3DImp9();
    
    set< IDeviceImp9* > i_childs;
    StateDataNode *state;
    
public:

//  IDirect3D9 implementation

    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL RegisterSoftwareDevice (  void* pInitializeFunction );
    UINT D3D_CALL GetAdapterCount ( );
    HRESULT D3D_CALL GetAdapterIdentifier (  UINT Adapter , DWORD Flags , D3DADAPTER_IDENTIFIER9* pIdentifier );
    UINT D3D_CALL GetAdapterModeCount (  UINT Adapter , D3DFORMAT Format );
    HRESULT D3D_CALL EnumAdapterModes (  UINT Adapter , D3DFORMAT Format , UINT Mode , D3DDISPLAYMODE* pMode );
    HRESULT D3D_CALL GetAdapterDisplayMode (  UINT Adapter , D3DDISPLAYMODE* pMode );
    HRESULT D3D_CALL CheckDeviceType (  UINT Adapter , D3DDEVTYPE DevType , D3DFORMAT AdapterFormat , D3DFORMAT BackBufferFormat , BOOL bWindowed );
    HRESULT D3D_CALL CheckDeviceFormat (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT AdapterFormat , DWORD Usage , D3DRESOURCETYPE RType , D3DFORMAT CheckFormat );
    HRESULT D3D_CALL CheckDeviceMultiSampleType (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT SurfaceFormat , BOOL Windowed , D3DMULTISAMPLE_TYPE MultiSampleType , DWORD* pQualityLevels );
    HRESULT D3D_CALL CheckDepthStencilMatch (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT AdapterFormat , D3DFORMAT RenderTargetFormat , D3DFORMAT DepthStencilFormat );
    HRESULT D3D_CALL CheckDeviceFormatConversion (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT SourceFormat , D3DFORMAT TargetFormat );
    HRESULT D3D_CALL GetDeviceCaps (  UINT Adapter , D3DDEVTYPE DeviceType , D3DCAPS9* pCaps );
    HMONITOR D3D_CALL GetAdapterMonitor (  UINT Adapter );
    HRESULT D3D_CALL CreateDevice (  UINT Adapter , D3DDEVTYPE DeviceType , HWND hFocusWindow , DWORD BehaviorFlags , D3DPRESENT_PARAMETERS* pPresentationParameters , IDirect3DDevice9** ppReturnedDeviceInterface );

};


#endif

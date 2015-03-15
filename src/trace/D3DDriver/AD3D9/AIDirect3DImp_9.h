/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

#ifndef AIDIRECT3DIMP_9_H_INCLUDED
#define AIDIRECT3DIMP_9_H_INCLUDED

#include <set>

class AIDeviceImp9;

class AIDirect3DImp9 : public IDirect3D9{

public:
   /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static AIDirect3DImp9 &getInstance();
    
    /// Singleton constructor method
    AIDirect3DImp9();

    //AIDirect3DImp9(StateDataNode* s_parent);
    ~AIDirect3DImp9();
private:
    
    std::set< AIDeviceImp9* > i_childs;
    //StateDataNode *state;
    
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

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

#include "Common.h"
#include "AIDeviceImp_9.h"
#include "AIDirect3DImp_9.h"

AIDirect3DImp9 :: AIDirect3DImp9() {}

AIDirect3DImp9 & AIDirect3DImp9 :: getInstance() 
{
    static AIDirect3DImp9 instance;
    return instance;
}


/*AIDirect3DImp9 :: AIDirect3DImp9(StateDataNode* s_parent) {
    state = D3DState::create_direct3d_state_9(this);
    s_parent->add_child(state);
}*/

AIDirect3DImp9 :: ~AIDirect3DImp9() 
{
    set< AIDeviceImp9* > :: iterator it;
    for(it = i_childs.begin(); it != i_childs.end(); it ++)
        delete *it;

    /// @todo delete state
}


HRESULT D3D_CALL AIDirect3DImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AIDeviceImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIDirect3DImp9 :: AddRef ( )
{
    D3D9_CALL(false, "AIDirect3DImp9::AddRef")
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL AIDirect3DImp9 :: Release ( ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::Release")
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: RegisterSoftwareDevice (  void* pInitializeFunction ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::RegisterSoftwareDevice")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

UINT D3D_CALL AIDirect3DImp9 :: GetAdapterCount ( ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::GetAdapterCount")
    UINT ret = static_cast< UINT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: GetAdapterIdentifier (  UINT Adapter , DWORD Flags , D3DADAPTER_IDENTIFIER9* pIdentifier ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::GetAdapterIdentifier")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

UINT D3D_CALL AIDirect3DImp9 :: GetAdapterModeCount (  UINT Adapter , D3DFORMAT Format ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::GetAdapterModeCount")
    UINT ret = static_cast< UINT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: EnumAdapterModes (  UINT Adapter , D3DFORMAT Format , UINT Mode , D3DDISPLAYMODE* pMode ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::EnumAdapterModes")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: GetAdapterDisplayMode (  UINT Adapter , D3DDISPLAYMODE* pMode ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::GetAdapterDisplayMode")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: CheckDeviceType (  UINT Adapter , D3DDEVTYPE DevType , D3DFORMAT AdapterFormat , D3DFORMAT BackBufferFormat , BOOL bWindowed ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::CheckDeviceType")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: CheckDeviceFormat (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT AdapterFormat , DWORD Usage , D3DRESOURCETYPE RType , D3DFORMAT CheckFormat ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::CheckDeviceFormat")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: CheckDeviceMultiSampleType (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT SurfaceFormat , BOOL Windowed , D3DMULTISAMPLE_TYPE MultiSampleType , DWORD* pQualityLevels ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::CheckDeviceMultiSampleType")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: CheckDepthStencilMatch (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT AdapterFormat , D3DFORMAT RenderTargetFormat , D3DFORMAT DepthStencilFormat ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::CheckDepthStencilMatch")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: CheckDeviceFormatConversion (  UINT Adapter , D3DDEVTYPE DeviceType , D3DFORMAT SourceFormat , D3DFORMAT TargetFormat ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::CheckDeviceFormatConversion")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: GetDeviceCaps (  UINT Adapter , D3DDEVTYPE DeviceType , D3DCAPS9* pCaps ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::GetDeviceCaps")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HMONITOR D3D_CALL AIDirect3DImp9 :: GetAdapterMonitor (  UINT Adapter ) 
{
    D3D9_CALL(false, "AIDirect3DImp9::GetAdapterMonitor")
    HMONITOR ret = static_cast< HMONITOR >(0);
    return ret;
}

HRESULT D3D_CALL AIDirect3DImp9 :: CreateDevice (  UINT Adapter , D3DDEVTYPE DeviceType , HWND hFocusWindow , DWORD BehaviorFlags , D3DPRESENT_PARAMETERS* pPresentationParameters , IDirect3DDevice9** ppReturnedDeviceInterface ) 
{
    D3D9_CALL(true, "AIDirect3DImp9::CreateDevice")
    /// @note Adapter, DeviceType, FocusWindow and BehaviorFlags are ignored from here on.
    AIDeviceImp9* device = new AIDeviceImp9(/*state,*/ this, pPresentationParameters);
    i_childs.insert(device);
    *ppReturnedDeviceInterface = device;
    return D3D_OK;
}

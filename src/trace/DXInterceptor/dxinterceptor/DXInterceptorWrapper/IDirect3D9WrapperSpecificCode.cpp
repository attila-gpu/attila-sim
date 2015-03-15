////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDirect3D9InterceptorWrapper.h"
#include "IDirect3DDevice9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3D9InterceptorWrapper::IDirect3D9InterceptorWrapper(REFIID iid, IDirect3D9* original) :
DXInterceptorWrapper(iid),
m_original(original)
{
}

////////////////////////////////////////////////////////////////////////////////

IDirect3D9InterceptorWrapper::~IDirect3D9InterceptorWrapper()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3D9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
{
  if (IsEqualGUID(riid, IID_IUnknown))
  {
    m_original->AddRef();
    *ppvObj = (IUnknown*) this;
    return S_OK;
  }
  
  if (IsEqualGUID(riid, IID_IDirect3D9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3D9*) this;
    return S_OK;
  }
  
  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3D9InterceptorWrapper::RegisterSoftwareDevice_Specific(void* pInitializeFunction)
{
  m_original->RegisterSoftwareDevice(pInitializeFunction);
  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3D9InterceptorWrapper::CreateDevice_Specific(UINT Adapter, D3DDEVTYPE DeviceType, HWND hFocusWindow, DWORD BehaviorFlags, D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DDevice9** ppReturnedDeviceInterface, DXMethodCallPtr call)
{
#ifndef D3D_MULTITHREAD_SUPPORT
  if (BehaviorFlags & D3DCREATE_MULTITHREADED)
  {
    g_logger->Write("WARNING: CreateDevice requested D3DCREATE_MULTITHREADED flag. Ensure that the DLL was compiled with DXTraceManager multithread support or the trace can be corrupted.");
  }
#endif
  
  HRESULT hr = m_original->CreateDevice(Adapter, DeviceType, hFocusWindow, BehaviorFlags, pPresentationParameters, ppReturnedDeviceInterface);

  if (SUCCEEDED(hr))
  {
    *ppReturnedDeviceInterface = new IDirect3DDevice9InterceptorWrapper(IID_IDirect3DDevice9, *ppReturnedDeviceInterface, this);
    call->Push_DXResourceObjectID(((IDirect3DDevice9InterceptorWrapper*) *ppReturnedDeviceInterface)->GetObjectID());
  }
  else
  {
    call->Push_DXNullPointer();
  }
  
  return hr;
}

////////////////////////////////////////////////////////////////////////////////

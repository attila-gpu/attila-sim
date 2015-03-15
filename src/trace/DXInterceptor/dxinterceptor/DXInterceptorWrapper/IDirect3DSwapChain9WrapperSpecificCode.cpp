////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DSurface9InterceptorWrapper.h"
#include "IDirect3DSwapChain9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DSwapChain9InterceptorWrapper::IDirect3DSwapChain9InterceptorWrapper(REFIID iid, IDirect3DSwapChain9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator)
{
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DSwapChain9InterceptorWrapper::~IDirect3DSwapChain9InterceptorWrapper()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSwapChain9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSwapChain9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
{
  if (IsEqualGUID(riid, IID_IUnknown))
  {
    m_original->AddRef();
    *ppvObj = (IUnknown*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DSwapChain9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DSwapChain9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DSwapChain9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DSwapChain9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSwapChain9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSwapChain9InterceptorWrapper::GetBackBuffer_Specific(UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer, DXMethodCallPtr call)
{
  IDirect3DSurface9* pSurface;

  HRESULT hr = m_original->GetBackBuffer(iBackBuffer, Type, &pSurface);

  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }

  IReferenceCounter* pRefCounter;
  DWORD pRefCounterSize = sizeof(IUnknown*);

  if (FAILED(pSurface->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
  {
    *ppBackBuffer = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, 0, pSurface, m_creator);
    pSurface->AddRef();
  }
  else
  {
    *ppBackBuffer = (IDirect3DSurface9InterceptorWrapper*) pRefCounter->GetOwner();
    pRefCounter->Release();
  }

  call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppBackBuffer)->GetObjectID());

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSwapChain9InterceptorWrapper::Present_Specific(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion, DWORD dwFlags)
{
  m_creator->Banner_Draw();
  return m_original->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion, dwFlags);
}

////////////////////////////////////////////////////////////////////////////////

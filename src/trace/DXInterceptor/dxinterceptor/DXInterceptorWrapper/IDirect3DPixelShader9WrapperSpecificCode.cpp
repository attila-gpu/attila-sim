////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DPixelShader9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DPixelShader9InterceptorWrapper::GetInternalResource(IDirect3DPixelShader9** ppRes, DWORD* pIDRes)
{
  if (!*ppRes)
  {
    *pIDRes = 0;
    return S_OK;
  }
  else
  {
    IDirect3DPixelShader9InterceptorWrapper* pResI;
    if (FAILED(((IDirect3DPixelShader9InterceptorWrapper*) *ppRes)->QueryInterfaceSilent(IID_IDirect3DPixelShader9InterceptorWrapper, (LPVOID*) &pResI)))
    {
      *pIDRes = 0;
      return E_FAIL;
    }
    else
    {
      *ppRes  = (IDirect3DPixelShader9*) pResI->GetOriginal();
      *pIDRes = pResI->GetObjectID();
      (*ppRes)->Release();
      return S_OK;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DPixelShader9InterceptorWrapper::IDirect3DPixelShader9InterceptorWrapper(REFIID iid, IDirect3DPixelShader9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator)
{
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DPixelShader9InterceptorWrapper::~IDirect3DPixelShader9InterceptorWrapper()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DPixelShader9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DPixelShader9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
{
  if (IsEqualGUID(riid, IID_IUnknown))
  {
    m_original->AddRef();
    *ppvObj = (IUnknown*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DPixelShader9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DPixelShader9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DPixelShader9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DPixelShader9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DPixelShader9* IDirect3DPixelShader9InterceptorWrapper::GetOriginal()
{
  return m_original;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DPixelShader9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

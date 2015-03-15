////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DSurface9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSurface9InterceptorWrapper::GetInternalResource(IDirect3DSurface9** ppRes, DWORD* pIDRes)
{
  if (!*ppRes)
  {
    *pIDRes = 0;
    return S_OK;
  }
  else
  {
    IDirect3DSurface9InterceptorWrapper* pResI;
    if (FAILED(((IDirect3DSurface9InterceptorWrapper*) *ppRes)->QueryInterfaceSilent(IID_IDirect3DSurface9InterceptorWrapper, (LPVOID*) &pResI)))
    {
      *pIDRes = 0;
      return E_FAIL;
    }
    else
    {
      *ppRes  = (IDirect3DSurface9*) pResI->GetOriginal();
      *pIDRes = pResI->GetObjectID();
      (*ppRes)->Release();
      return S_OK;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DSurface9InterceptorWrapper::IDirect3DSurface9InterceptorWrapper(REFIID iid, UINT level, IDirect3DSurface9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();

  D3DSURFACE_DESC	description;
  m_original->GetDesc(&description);
  m_format = description.Format;
  m_level = level;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DSurface9InterceptorWrapper::~IDirect3DSurface9InterceptorWrapper()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSurface9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSurface9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
{
  if (IsEqualGUID(riid, IID_IUnknown))
  {
    m_original->AddRef();
    *ppvObj = (IUnknown*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DResource9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DResource9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DSurface9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DSurface9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DSurface9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DSurface9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DSurface9* IDirect3DSurface9InterceptorWrapper::GetOriginal()
{
  return m_original;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSurface9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSurface9InterceptorWrapper::LockRect_Specific(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
  HRESULT hr = m_original->LockRect(pLockedRect, pRect, Flags);

  if (FAILED(hr) || (Flags & D3DLOCK_READONLY))
  {
    D3DLOCKED_RECT lockedRect;
    lockedRect.pBits = 0;
    m_lock.SetLockedRect(&lockedRect);
  }
  else
  {
    m_lock.SetLockedRect(pLockedRect);
    m_lock.SetLock(m_original, m_level, pRect, m_format);
  }

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSurface9InterceptorWrapper::UnlockRect_Specific(DXMethodCallPtr call)
{
  DXTexturePtr texture = new DXTexture();

  if (m_lock.WriteLock(texture))
  {
    DXTextureIdentifier texture_id;
    g_traceman->WriteTexture(texture, texture_id);
    call->Push_DXTextureIdentifier(texture_id);
  }
  else
  {
    call->Push_DXNullPointer();
  }
  
  return m_original->UnlockRect();
}

////////////////////////////////////////////////////////////////////////////////

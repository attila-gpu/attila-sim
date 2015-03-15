////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DSurface9InterceptorWrapper.h"
#include "IDirect3DTexture9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DTexture9InterceptorWrapper::GetInternalResource(IDirect3DBaseTexture9** ppRes, DWORD* pIDRes)
{
  if (!*ppRes)
  {
    *pIDRes = 0;
    return S_OK;
  }
  else
  {
    IDirect3DTexture9InterceptorWrapper* pResI;
    if (FAILED(((IDirect3DTexture9InterceptorWrapper*) *ppRes)->QueryInterfaceSilent(IID_IDirect3DTexture9InterceptorWrapper, (LPVOID*) &pResI)))
    {
      *pIDRes = 0;
      return E_FAIL;
    }
    else
    {
      *ppRes  = (IDirect3DBaseTexture9*) pResI->GetOriginal();
      *pIDRes = pResI->GetObjectID();
      (*ppRes)->Release();
      return S_OK;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DTexture9InterceptorWrapper::IDirect3DTexture9InterceptorWrapper(REFIID iid, IDirect3DTexture9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator),
m_locks(NULL)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();

  DWORD levels = m_original->GetLevelCount();
  m_locks = new DXSurfaceLock[levels];

  // Reset the internal objects
  for (unsigned int i=0; i < levels; ++i)
  {
    IDirect3DSurface9* surface;
    if (SUCCEEDED(m_original->GetSurfaceLevel(i, &surface)))
    {
      surface->FreePrivateData(IID_IReferenceCounter);
      surface->Release();
    }
  }
  
  D3DSURFACE_DESC	description;
  m_original->GetLevelDesc(0, &description);
  m_format = description.Format;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DTexture9InterceptorWrapper::~IDirect3DTexture9InterceptorWrapper()
{
  if (m_locks)
  {
    delete[] m_locks;
    m_locks = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DTexture9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DTexture9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
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

  if (IsEqualGUID(riid, IID_IDirect3DBaseTexture9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DBaseTexture9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DTexture9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DTexture9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DTexture9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DTexture9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DTexture9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DTexture9* IDirect3DTexture9InterceptorWrapper::GetOriginal()
{
  return m_original;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DTexture9InterceptorWrapper::GetSurfaceLevel_Specific(UINT Level, IDirect3DSurface9** ppSurfaceLevel, DXMethodCallPtr call)
{
  IDirect3DSurface9* pSurface;

  HRESULT hr = m_original->GetSurfaceLevel(Level, &pSurface);

  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }

  IReferenceCounter* pRefCounter;
  DWORD pRefCounterSize = sizeof(IUnknown*);
  
  if (FAILED(pSurface->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
  {
    *ppSurfaceLevel = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, Level, pSurface, m_creator);
  }
  else
  {
    *ppSurfaceLevel = (IDirect3DSurface9InterceptorWrapper*) pRefCounter->GetOwner();
    pRefCounter->Release();
  }  

  call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppSurfaceLevel)->GetObjectID());
  
  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DTexture9InterceptorWrapper::LockRect_Specific(UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
  HRESULT hr = m_original->LockRect(Level, pLockedRect, pRect, Flags);

  if (FAILED(hr) || (Flags & D3DLOCK_READONLY))
  {
    D3DLOCKED_RECT lockedRect;
    lockedRect.pBits = 0;
    m_locks[Level].SetLockedRect(&lockedRect);
  }
  else
  {
    m_locks[Level].SetLockedRect(pLockedRect);
    m_locks[Level].SetLock(m_original, Level, pRect, m_format);
  }

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DTexture9InterceptorWrapper::UnlockRect_Specific(UINT Level, DXMethodCallPtr call)
{
  DXTexturePtr texture = new DXTexture();

  if (m_locks[Level].WriteLock(texture))
  {
    DXTextureIdentifier texture_id;
    g_traceman->WriteTexture(texture, texture_id);
    call->Push_DXTextureIdentifier(texture_id);
  }
  else
  {
    call->Push_DXNullPointer();
  }

  return m_original->UnlockRect(Level);
}

////////////////////////////////////////////////////////////////////////////////

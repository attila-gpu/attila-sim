////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DSurface9InterceptorWrapper.h"
#include "IDirect3DCubeTexture9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DCubeTexture9InterceptorWrapper::GetInternalResource(IDirect3DCubeTexture9** ppRes, DWORD* pIDRes)
{
  if (!*ppRes)
  {
    *pIDRes = 0;
    return S_OK;
  }
  else
  {
    IDirect3DCubeTexture9InterceptorWrapper* pResI;
    if (FAILED(((IDirect3DCubeTexture9InterceptorWrapper*) *ppRes)->QueryInterfaceSilent(IID_IDirect3DCubeTexture9InterceptorWrapper, (LPVOID*) &pResI)))
    {
      *pIDRes = 0;
      return E_FAIL;
    }
    else
    {
      *ppRes  = (IDirect3DCubeTexture9*) pResI->GetOriginal();
      *pIDRes = pResI->GetObjectID();
      (*ppRes)->Release();
      return S_OK;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DCubeTexture9InterceptorWrapper::IDirect3DCubeTexture9InterceptorWrapper(REFIID iid, IDirect3DCubeTexture9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator),
m_locks(NULL)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();

  DWORD levels = m_original->GetLevelCount();
  m_locks = new DXCubeMapLock[levels];

  // Reset the internal objects
  for (unsigned int i=0; i < levels; ++i)
  {
    IDirect3DSurface9* surface;
    if (SUCCEEDED(m_original->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_X, i, &surface)))
    {
      surface->FreePrivateData(IID_IReferenceCounter);
      surface->Release();
    }
    if (SUCCEEDED(m_original->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_X, i, &surface)))
    {
      surface->FreePrivateData(IID_IReferenceCounter);
      surface->Release();
    }
    if (SUCCEEDED(m_original->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_Y, i, &surface)))
    {
      surface->FreePrivateData(IID_IReferenceCounter);
      surface->Release();
    }
    if (SUCCEEDED(m_original->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_Y, i, &surface)))
    {
      surface->FreePrivateData(IID_IReferenceCounter);
      surface->Release();
    }
    if (SUCCEEDED(m_original->GetCubeMapSurface(D3DCUBEMAP_FACE_POSITIVE_Z, i, &surface)))
    {
      surface->FreePrivateData(IID_IReferenceCounter);
      surface->Release();
    }
    if (SUCCEEDED(m_original->GetCubeMapSurface(D3DCUBEMAP_FACE_NEGATIVE_Z, i, &surface)))
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

IDirect3DCubeTexture9InterceptorWrapper::~IDirect3DCubeTexture9InterceptorWrapper()
{
  if (m_locks)
  {
    delete[] m_locks;
    m_locks = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DCubeTexture9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DCubeTexture9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
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

  if (IsEqualGUID(riid, IID_IDirect3DCubeTexture9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DCubeTexture9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DCubeTexture9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DCubeTexture9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DCubeTexture9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DCubeTexture9* IDirect3DCubeTexture9InterceptorWrapper::GetOriginal()
{
  return m_original;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DCubeTexture9InterceptorWrapper::GetCubeMapSurface_Specific(D3DCUBEMAP_FACES FaceType, UINT Level, IDirect3DSurface9** ppCubeMapSurface, DXMethodCallPtr call)
{
  IDirect3DSurface9* pSurface;

  HRESULT hr = m_original->GetCubeMapSurface(FaceType, Level, &pSurface);

  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }

  IReferenceCounter* pRefCounter;
  DWORD pRefCounterSize = sizeof(IUnknown*);

  if (FAILED(pSurface->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
  {
    *ppCubeMapSurface = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, Level, pSurface, m_creator);
  }
  else
  {
    *ppCubeMapSurface = (IDirect3DSurface9InterceptorWrapper*) pRefCounter->GetOwner();
    pRefCounter->Release();
  }

  call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppCubeMapSurface)->GetObjectID());

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DCubeTexture9InterceptorWrapper::LockRect_Specific(D3DCUBEMAP_FACES FaceType, UINT Level, D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags)
{
  HRESULT hr = m_original->LockRect(FaceType, Level, pLockedRect, pRect, Flags);

  if (FAILED(hr) || (Flags & D3DLOCK_READONLY))
  {
    D3DLOCKED_RECT lockedRect;
    lockedRect.pBits = 0;
    m_locks[Level][FaceType].SetLockedRect(&lockedRect);
  }
  else
  {
    m_locks[Level][FaceType].SetLockedRect(pLockedRect);
    m_locks[Level][FaceType].SetLock(m_original, Level, pRect, m_format);
  }

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DCubeTexture9InterceptorWrapper::UnlockRect_Specific(D3DCUBEMAP_FACES FaceType, UINT Level, DXMethodCallPtr call)
{
  DXTexturePtr texture = new DXTexture();

  if (m_locks[Level][FaceType].WriteLock(texture))
  {
    DXTextureIdentifier texture_id;
    g_traceman->WriteTexture(texture, texture_id);
    call->Push_DXTextureIdentifier(texture_id);
  }
  else
  {
    call->Push_DXNullPointer();
  }

  return m_original->UnlockRect(FaceType, Level);
}

////////////////////////////////////////////////////////////////////////////////

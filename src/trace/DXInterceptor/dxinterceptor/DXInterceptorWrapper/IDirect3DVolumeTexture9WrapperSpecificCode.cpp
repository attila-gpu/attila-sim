////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DSurface9InterceptorWrapper.h"
#include "IDirect3DVolume9InterceptorWrapper.h"
#include "IDirect3DVolumeTexture9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolumeTexture9InterceptorWrapper::GetInternalResource(IDirect3DVolumeTexture9** ppRes, DWORD* pIDRes)
{
  if (!*ppRes)
  {
    *pIDRes = 0;
    return S_OK;
  }
  else
  {
    IDirect3DVolumeTexture9InterceptorWrapper* pResI;
    if (FAILED(((IDirect3DVolumeTexture9InterceptorWrapper*) *ppRes)->QueryInterfaceSilent(IID_IDirect3DVolumeTexture9InterceptorWrapper, (LPVOID*) &pResI)))
    {
      *pIDRes = 0;
      return E_FAIL;
    }
    else
    {
      *ppRes  = (IDirect3DVolumeTexture9*) pResI->GetOriginal();
      *pIDRes = pResI->GetObjectID();
      (*ppRes)->Release();
      return S_OK;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolumeTexture9InterceptorWrapper::IDirect3DVolumeTexture9InterceptorWrapper(REFIID iid, IDirect3DVolumeTexture9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator),
m_locks(NULL)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();

  DWORD levels = m_original->GetLevelCount();
  m_locks = new DXVolumeLock[levels];

  // Reset the internal objects
  for (unsigned int i=0; i < levels; ++i)
  {
    IDirect3DVolume9* volume;
    if (SUCCEEDED(m_original->GetVolumeLevel(i, &volume)))
    {
      volume->FreePrivateData(IID_IReferenceCounter);
      volume->Release();
    }
  }
  
  D3DVOLUME_DESC	description;
  m_original->GetLevelDesc(0, &description);
  m_format = description.Format;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolumeTexture9InterceptorWrapper::~IDirect3DVolumeTexture9InterceptorWrapper()
{
  if (m_locks)
  {
    delete[] m_locks;
    m_locks = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolumeTexture9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolumeTexture9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
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

  if (IsEqualGUID(riid, IID_IDirect3DVolumeTexture9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DVolumeTexture9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DVolumeTexture9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DVolumeTexture9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolumeTexture9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolumeTexture9* IDirect3DVolumeTexture9InterceptorWrapper::GetOriginal()
{
  return m_original;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolumeTexture9InterceptorWrapper::GetVolumeLevel_Specific(UINT Level, IDirect3DVolume9** ppVolumeLevel, DXMethodCallPtr call)
{
  IDirect3DVolume9* pVolume;

  HRESULT hr = m_original->GetVolumeLevel(Level, &pVolume);

  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }

  IReferenceCounter* pRefCounter;
  DWORD pRefCounterSize = sizeof(IUnknown*);
  
  if (FAILED(pVolume->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
  {
    *ppVolumeLevel = new IDirect3DVolume9InterceptorWrapper(IID_IDirect3DVolume9, Level, pVolume, m_creator);
  }
  else
  {
    *ppVolumeLevel = (IDirect3DVolume9InterceptorWrapper*) pRefCounter->GetOwner();
    pRefCounter->Release();
  }

  call->Push_DXResourceObjectID(((IDirect3DVolume9InterceptorWrapper*) *ppVolumeLevel)->GetObjectID());
  
  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolumeTexture9InterceptorWrapper::LockBox_Specific(UINT Level, D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags)
{
  HRESULT hr = m_original->LockBox(Level, pLockedVolume, pBox, Flags);

  if (FAILED(hr) || (Flags & D3DLOCK_READONLY))
  {
    D3DLOCKED_BOX lockedBox;
    lockedBox.pBits = 0;
    m_locks[Level].SetLockedBox(&lockedBox);
  }
  else
  {
    m_locks[Level].SetLockedBox(pLockedVolume);
    m_locks[Level].SetLock(m_original, Level, pBox, m_format);
  }

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolumeTexture9InterceptorWrapper::UnlockBox_Specific(UINT Level, DXMethodCallPtr call)
{
  DXTexturePtr volumeTexture = new DXTexture();
  
  if (m_locks[Level].WriteLock(volumeTexture))
  {
    DXTextureIdentifier texture_id;
    g_traceman->WriteTexture(volumeTexture, texture_id);
    call->Push_DXTextureIdentifier(texture_id);
  }
  else
  {
    call->Push_DXNullPointer();
  }

  return m_original->UnlockBox(Level);
}

////////////////////////////////////////////////////////////////////////////////

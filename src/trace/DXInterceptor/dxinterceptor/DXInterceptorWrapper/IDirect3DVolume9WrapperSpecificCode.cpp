////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DVolume9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolume9InterceptorWrapper::IDirect3DVolume9InterceptorWrapper(REFIID iid, UINT level, IDirect3DVolume9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator),
m_lock(NULL)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();
  
  m_lock = new DXVolumeLock;

  D3DVOLUME_DESC description;
  m_original->GetDesc(&description);
  m_format = description.Format;
  m_level = level;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolume9InterceptorWrapper::~IDirect3DVolume9InterceptorWrapper()
{
  if (m_lock)
  {
    delete m_lock;
    m_lock = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolume9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolume9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
{
  if (IsEqualGUID(riid, IID_IUnknown))
  {
    m_original->AddRef();
    *ppvObj = (IUnknown*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DVolume9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DVolume9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DVolume9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DVolume9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolume9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolume9InterceptorWrapper::LockBox_Specific(D3DLOCKED_BOX* pLockedVolume, CONST D3DBOX* pBox, DWORD Flags)
{
  HRESULT hr = m_original->LockBox(pLockedVolume, pBox, Flags);

  if (FAILED(hr) || (Flags & D3DLOCK_READONLY))
  {
    D3DLOCKED_BOX lockedBox;
    lockedBox.pBits = 0;
    m_lock[0].SetLockedBox(&lockedBox);
  }
  else
  {
    m_lock[0].SetLockedBox(pLockedVolume);
    m_lock[0].SetLock(m_original, 0, pBox, m_format);
  }

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolume9InterceptorWrapper::UnlockBox_Specific(DXMethodCallPtr call)
{
  DXTexturePtr volumeTexture = new DXTexture();

  if (m_lock[0].WriteLock(volumeTexture))
  {
    DXTextureIdentifier texture_id;
    g_traceman->WriteTexture(volumeTexture, texture_id);
    call->Push_DXTextureIdentifier(texture_id);
  }
  else
  {
    call->Push_DXNullPointer();
  }

  return m_original->UnlockBox();
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DIndexBuffer9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DIndexBuffer9InterceptorWrapper::GetInternalResource(IDirect3DIndexBuffer9** ppRes, DWORD* pIDRes)
{
  if (!*ppRes)
  {
    *pIDRes = 0;
    return S_OK;
  }
  else
  {
    IDirect3DIndexBuffer9InterceptorWrapper* pResI;
    if (FAILED(((IDirect3DIndexBuffer9InterceptorWrapper*) *ppRes)->QueryInterfaceSilent(IID_IDirect3DIndexBuffer9InterceptorWrapper, (LPVOID*) &pResI)))
    {
      *pIDRes = 0;
      return E_FAIL;
    }
    else
    {
      *ppRes  = pResI->GetOriginal();
      *pIDRes = pResI->GetObjectID();
      (*ppRes)->Release();
      return S_OK;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DIndexBuffer9InterceptorWrapper::IDirect3DIndexBuffer9InterceptorWrapper(REFIID iid, IDirect3DIndexBuffer9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();

  D3DINDEXBUFFER_DESC description;
  m_original->GetDesc(&description);
  m_shadowBufferSize = description.Size;
  m_shadowBuffer = new BYTE[m_shadowBufferSize];
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DIndexBuffer9InterceptorWrapper::~IDirect3DIndexBuffer9InterceptorWrapper()
{
  if (m_shadowBuffer)
  {
    delete[] m_shadowBuffer;
    m_shadowBuffer = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DIndexBuffer9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DIndexBuffer9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
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

  if (IsEqualGUID(riid, IID_IDirect3DIndexBuffer9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DIndexBuffer9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DIndexBuffer9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DIndexBuffer9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DIndexBuffer9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DIndexBuffer9* IDirect3DIndexBuffer9InterceptorWrapper::GetOriginal()
{
  return m_original;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DIndexBuffer9InterceptorWrapper::Lock_Specific(UINT OffsetToLock, UINT SizeToLock, void** ppbData, DWORD Flags, DXMethodCallPtr call)
{
  //////////////////////////////////////////////////////////////////////////////
  // HACKS
  //////////////////////////////////////////////////////////////////////////////
  
  // Halo

  if (SizeToLock == 4 && OffsetToLock+SizeToLock < m_shadowBufferSize && Flags == 0)
  {
    m_creator->Banner_DrawError("HACK: 'Halo' hack applied", 30000);
    SizeToLock = m_shadowBufferSize - OffsetToLock;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  
  call->Push_UINT(SizeToLock);
  
  HRESULT hr = m_original->Lock(OffsetToLock, SizeToLock, ppbData, Flags);
  
  if (FAILED(hr) || !m_shadowBuffer || (Flags & D3DLOCK_READONLY))
  {
    m_locks.push(DXBufferLock(0, NULL, NULL));
  }
  else
  {
    if (!OffsetToLock && !SizeToLock)
    {
      SizeToLock = m_shadowBufferSize;
    }

    m_locks.push(DXBufferLock(SizeToLock, m_shadowBuffer+OffsetToLock, (BYTE*) *ppbData));
    *ppbData = m_shadowBuffer+OffsetToLock;
  }

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DIndexBuffer9InterceptorWrapper::Unlock_Specific(DXMethodCallPtr call)
{
  DXBufferLock data = m_locks.top();
  m_locks.pop();

  if (data.Size)
  {
    memcpy(data.LockedPtr, data.ShadowPtr, data.Size);

    DXBufferIdentifier buffer_id;
    DXBufferPtr buffer = new DXBuffer();
    buffer->Push_DXRawData((void*) data.ShadowPtr, data.Size);
    g_traceman->WriteBuffer(buffer, buffer_id);
    call->Push_DXBufferIdentifier(buffer_id);
  }
  else
  {
    call->Push_DXNullPointer();
  }

  return m_original->Unlock();
}

////////////////////////////////////////////////////////////////////////////////

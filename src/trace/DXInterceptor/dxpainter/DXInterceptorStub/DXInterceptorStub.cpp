////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "DXInterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

DXInterceptorStub::DXInterceptorStub(DXPainter* painter) :
m_painter(painter)
{
  m_stubID = m_painter->AddStub(this);
}

////////////////////////////////////////////////////////////////////////////////

DXInterceptorStub::~DXInterceptorStub()
{
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXInterceptorStub::GetStubID() const
{
  return m_stubID;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT DXInterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT DXInterceptorStub::D3DResource9_GetPrivateData(IDirect3DResource9* resource, DXMethodCallPtr call)
{
  IID refguid;
  CHECK_CALL(call->Pop_IID(&refguid));

  void* pData = NULL;
  {
    DXIgnoredParameter ignoredParam;
    CHECK_CALL(call->Pop_DXIgnoredParameter(&ignoredParam));
  }

  DWORD pSizeOfData;
  CHECK_CALL(call->Pop_DWORD(&pSizeOfData));
  
  //////////////////////////////////////////////////////////////////////////////
  // Simulate a size consult

  pSizeOfData = 0;

  //////////////////////////////////////////////////////////////////////////////

  HRESULT result = resource->GetPrivateData(refguid, pData, &pSizeOfData);
  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT DXInterceptorStub::D3DResource9_SetPrivateData(IDirect3DResource9* resource, DXMethodCallPtr call)
{
  IID refguid;
  CHECK_CALL(call->Pop_IID(&refguid));

  char* pData;
  DXBufferPtr pDataBuffer;
  {
    if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
    {
      DXNullPointer checkNull;
      call->Pop_DXNullPointer(&checkNull);
      pData = NULL;
    }
    else
    {
      DXBufferIdentifier buffer_id;
      CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
      CHECK_CALL(m_painter->GetBuffer(&pDataBuffer, buffer_id));
      CHECK_CALL(pDataBuffer->Pop_DXRawData(&pData));
    }
  }

  DWORD SizeOfData;
  CHECK_CALL(call->Pop_DWORD(&SizeOfData));

  DWORD Flags;
  CHECK_CALL(call->Pop_DWORD(&Flags));

  //////////////////////////////////////////////////////////////////////////////
  // Ignore if equal to the internal IReferenceCounter IID

  if (refguid == IID_IReferenceCounter)
  {
    return D3D_OK;
  }

  //////////////////////////////////////////////////////////////////////////////
  
  HRESULT result = resource->SetPrivateData(refguid, (CONST void*) pData, SizeOfData, Flags);
  CHECK_CALL_RETURN_VALUE_HRESULT(result);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT DXInterceptorStub::D3DResource9_FreePrivateData(IDirect3DResource9* resource, DXMethodCallPtr call)
{
  IID refguid;
  CHECK_CALL(call->Pop_IID(&refguid));

  //////////////////////////////////////////////////////////////////////////////
  // Ignore if equal to the internal IReferenceCounter IID

  if (refguid == IID_IReferenceCounter)
  {
    return D3D_OK;
  }

  //////////////////////////////////////////////////////////////////////////////
  
  HRESULT result = resource->FreePrivateData(refguid);
  CHECK_CALL_RETURN_VALUE_HRESULT(result);
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorStub.h"
#include "IDirect3DVertexBuffer9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DVertexBuffer9InterceptorStub::IDirect3DVertexBuffer9InterceptorStub(DXPainter* painter, IDirect3DVertexBuffer9* original, IDirect3DDevice9InterceptorStub* creator) : 
DXInterceptorStub(painter),
m_original(original),
m_creator(creator)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();
  
  D3DVERTEXBUFFER_DESC description;
  m_original->GetDesc(&description);
  m_bufferSize = description.Size;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVertexBuffer9InterceptorStub::~IDirect3DVertexBuffer9InterceptorStub()
{
  FREE_D3D_RESOURCE();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVertexBuffer9InterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_QueryInterface:
    {
      IID param1;
      CHECK_CALL(call->Pop_IID(&param1));

      DXIgnoredParameter ignoredParam;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&ignoredParam));
      
      VOID* param2;

      HRESULT hr = m_original->QueryInterface(param1, &param2);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_AddRef:
    {
      ULONG result = m_original->AddRef();
      CHECK_CALL_RETURN_VALUE_ADDREF_RELEASE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_Release:
    {
      ULONG result = m_original->Release();
      if (!result)
      {
        m_painter->RemoveStub(this);
        m_original = NULL;
      }
      CHECK_CALL_RETURN_VALUE_ADDREF_RELEASE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_GetDevice:
    {
      DXResourceObjectID param1ResID;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      IDirect3DDevice9* param1;

      HRESULT hr = m_original->GetDevice(&param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;  
  
  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_Lock:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      VOID* param3;
      DXIgnoredParameter param3dummy;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param3dummy));

      DXFlagsD3DLOCK param4;
      CHECK_CALL(call->Pop_DXFlagsD3DLOCK(&param4));

      HRESULT hr = m_original->Lock(param1, param2, &param3, param4);

      if (param2 == 0)
      {
        param2 = m_bufferSize - param1;
      }

      if (param4 & D3DLOCK_READONLY)
        m_locks.push(DXBufferLock(0, NULL, NULL));
      else
        m_locks.push(DXBufferLock(param2, NULL, (BYTE*) param3));

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_Unlock:
    {
      DXBufferLock data = m_locks.top();
      m_locks.pop();
      
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXBufferIdentifier buffer_id;
        CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
        DXBufferPtr buffer;
        CHECK_CALL(m_painter->GetBuffer(&buffer, buffer_id));
        char* param1;
        CHECK_CALL(buffer->Pop_DXRawData(&param1));

        if (data.LockedPtr)
        {
          memcpy(data.LockedPtr, param1, data.Size);
        }
      }

      HRESULT hr = m_original->Unlock();
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_GetPrivateData:
    return D3DResource9_GetPrivateData(m_original, call);
    break;

  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_SetPrivateData:
    return D3DResource9_SetPrivateData(m_original, call);
    break;

  case DXMethodCallHelper::TOK_IDirect3DVertexBuffer9_FreePrivateData:
    return D3DResource9_FreePrivateData(m_original, call);
    break;
  }

  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

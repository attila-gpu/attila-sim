////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IDirect3D9InterceptorStub.h"
#include "IDirect3DDevice9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3D9InterceptorStub::IDirect3D9InterceptorStub(DXPainter* painter, IDirect3D9* original) : 
DXInterceptorStub(painter),
m_original(original)
{
}

////////////////////////////////////////////////////////////////////////////////

IDirect3D9InterceptorStub::~IDirect3D9InterceptorStub()
{
  FREE_D3D_RESOURCE();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3D9InterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3D9_QueryInterface:
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

  case DXMethodCallHelper::TOK_IDirect3D9_AddRef:
    {
      m_original->AddRef();
      return D3D_OK;
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3D9_Release:
    {
      if (!m_original->Release())
      {
        m_painter->RemoveStub(this);
        m_original = NULL;
      }
      return D3D_OK;
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3D9_RegisterSoftwareDevice:
    return E_NOTIMPL;
    break;
  
  case DXMethodCallHelper::TOK_IDirect3D9_CreateDevice:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      D3DDEVTYPE param2;
      CHECK_CALL(call->Pop_D3DDEVTYPE(&param2));

      HWND param3;
      CHECK_CALL(call->Pop_HWND(&param3));

      DXFlagsD3DCREATE param4;
      CHECK_CALL(call->Pop_DXFlagsD3DCREATE(&param4));

      char* param5 = NULL;
      DXBufferPtr param5Buffer;
      {
        if (!call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXBufferIdentifier buffer_id;
          CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
          CHECK_CALL(m_painter->GetBuffer(&param5Buffer, buffer_id));
          CHECK_CALL(param5Buffer->Pop_D3DPRESENT_PARAMETERS(&param5));
        }
      }

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param6ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param6ResID));
      }
      IDirect3DDevice9* param6 = NULL; 

      //////////////////////////////////////////////////////////////////////////
      
      param3 = m_painter->GetViewportHWND();
      m_painter->CorrectDevicePresentationParameters((D3DPRESENT_PARAMETERS*) param5);
      m_painter->CorrectDeviceFlags(&param4);

      //////////////////////////////////////////////////////////////////////////
      
      HRESULT hr = m_original->CreateDevice(param1, param2, param3, param4, (D3DPRESENT_PARAMETERS*) param5, &param6);
      if (SUCCEEDED(hr))
      {
        m_painter->SetDevice(param6);
        IDirect3DDevice9InterceptorStub* device = new IDirect3DDevice9InterceptorStub(m_painter, param6, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  }
    
  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

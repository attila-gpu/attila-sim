////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorStub.h"
#include "IDirect3DSurface9InterceptorStub.h"
#include "IDirect3DSwapChain9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DSwapChain9InterceptorStub::IDirect3DSwapChain9InterceptorStub(DXPainter* painter, IDirect3DSwapChain9* original, IDirect3DDevice9InterceptorStub* creator) : 
DXInterceptorStub(painter),
m_original(original),
m_creator(creator)
{
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DSwapChain9InterceptorStub::~IDirect3DSwapChain9InterceptorStub()
{
  FREE_D3D_RESOURCE();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSwapChain9InterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DSwapChain9_QueryInterface:
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

  case DXMethodCallHelper::TOK_IDirect3DSwapChain9_AddRef:
    {
      ULONG result = m_original->AddRef();
      CHECK_CALL_RETURN_VALUE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DSwapChain9_Release:
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

  case DXMethodCallHelper::TOK_IDirect3DSwapChain9_GetDevice:
    {
      DXResourceObjectID param1ResID;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      IDirect3DDevice9* param1;

      HRESULT hr = m_original->GetDevice(&param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DSwapChain9_Present:
    {
      RECT pSourceRect;
      RECT* pSourceRectPtr = &pSourceRect;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          pSourceRectPtr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_RECT(pSourceRectPtr));
        }
      }

      RECT pDestRect;
      RECT* pDestRectPtr = &pDestRect;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          pDestRectPtr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_RECT(pDestRectPtr));
        }
      }

      HWND hDestWindowOverride;
      CHECK_CALL(call->Pop_HWND(&hDestWindowOverride));
      if (hDestWindowOverride) hDestWindowOverride = m_painter->GetViewportHWND();

      char* pDirtyRegion;
      DXBufferPtr pDirtyRegionBuffer;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          pDirtyRegion = NULL;
        }
        else
        {
          DXBufferIdentifier buffer_id;
          CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
          CHECK_CALL(m_painter->GetBuffer(&pDirtyRegionBuffer, buffer_id));
          CHECK_CALL(pDirtyRegionBuffer->Pop_ARR_RGNDATA(&pDirtyRegion));
        }
      }

      DXFlagsD3DPRESENT dwFlags;
      CHECK_CALL(call->Pop_DXFlagsD3DPRESENT(&dwFlags));

      HRESULT result = m_original->Present(pSourceRectPtr, pDestRectPtr, hDestWindowOverride, (CONST RGNDATA*) pDirtyRegion, dwFlags);
      
      //////////////////////////////////////////////////////////////////////////

      m_painter->UpdateFrameStatistics();

      //////////////////////////////////////////////////////////////////////////
      
      CHECK_CALL_RETURN_VALUE_HRESULT(result);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DSwapChain9_GetBackBuffer:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      D3DBACKBUFFER_TYPE param2;
      CHECK_CALL(call->Pop_D3DBACKBUFFER_TYPE(&param2));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param3ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param3ResID));
      }
      IDirect3DSurface9* param3;

      HRESULT hr = m_original->GetBackBuffer(param1, param2, &param3);

      if (FAILED(hr))
      {
        CHECK_CALL_RETURN_VALUE_HRESULT(hr);
      }

      IReferenceCounter* pRefCounter;
      DWORD pRefCounterSize = sizeof(IUnknown*);

      if (FAILED(param3->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
      {
        new IDirect3DSurface9InterceptorStub(m_painter, 0, param3, m_creator);
        param3->AddRef();
      }
      else
      {
        pRefCounter->Release();
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  }

  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

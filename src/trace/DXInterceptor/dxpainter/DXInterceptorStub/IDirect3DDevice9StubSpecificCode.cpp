////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3D9InterceptorStub.h"
#include "IDirect3DDevice9InterceptorStub.h"
#include "IDirect3DVertexDeclaration9InterceptorStub.h"
#include "IDirect3DVertexBuffer9InterceptorStub.h"
#include "IDirect3DIndexBuffer9InterceptorStub.h"
#include "IDirect3DTexture9InterceptorStub.h"
#include "IDirect3DCubeTexture9InterceptorStub.h"
#include "IDirect3DVolumeTexture9InterceptorStub.h"
#include "IDirect3DSurface9InterceptorStub.h"
#include "IDirect3DStateBlock9InterceptorStub.h"
#include "IDirect3DPixelShader9InterceptorStub.h"
#include "IDirect3DVertexShader9InterceptorStub.h"
#include "IDirect3DQuery9InterceptorStub.h"
#include "IDirect3DSwapChain9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DDevice9InterceptorStub::IDirect3DDevice9InterceptorStub(DXPainter* painter, IDirect3DDevice9* original, IDirect3D9InterceptorStub* creator) :
DXInterceptorStub(painter),
m_original(original),
m_creator(creator),
m_deviceReferenceCount(0)
{
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DDevice9InterceptorStub::~IDirect3DDevice9InterceptorStub()
{
  FREE_D3D_RESOURCE();
}

////////////////////////////////////////////////////////////////////////////////

void IDirect3DDevice9InterceptorStub::FreeDeviceSurfaces()
{
  // Free correctly the current render target surface
  IDirect3DSurface9* currentRenderTarget = NULL;
  if (SUCCEEDED(m_original->GetRenderTarget(0, &currentRenderTarget)))
  {
    IReferenceCounter* pRefCounter;
    DWORD pRefCounterSize = sizeof(IUnknown*);
    if (SUCCEEDED(currentRenderTarget->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
    {
      IDirect3DSurface9InterceptorStub* myCurrentRenderTarget = (IDirect3DSurface9InterceptorStub*) pRefCounter->GetOwner();
      pRefCounter->Release();
      m_painter->RemoveStub(myCurrentRenderTarget);
      while(currentRenderTarget->Release());
      m_deviceReferenceCount--;
    }
    else
    {
      currentRenderTarget->Release();
    }
  }

  // Free correctly the current depth stencil surface
  IDirect3DSurface9* currentDepthStencil = NULL;
  if (SUCCEEDED(m_original->GetDepthStencilSurface(&currentDepthStencil)))
  {
    IReferenceCounter* pRefCounter;
    DWORD pRefCounterSize = sizeof(IUnknown*);
    if (SUCCEEDED(currentDepthStencil->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
    {
      IDirect3DSurface9InterceptorStub* myCurrentDepthStencil = (IDirect3DSurface9InterceptorStub*) pRefCounter->GetOwner();
      pRefCounter->Release();
      m_painter->RemoveStub(myCurrentDepthStencil);
      while(currentDepthStencil->Release());
      m_deviceReferenceCount--;
    }
    else
    {
      currentDepthStencil->Release();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DDevice9_QueryInterface:
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

  case DXMethodCallHelper::TOK_IDirect3DDevice9_AddRef:
    {
      ULONG result = m_original->AddRef();
      CHECK_CALL_RETURN_VALUE_ADDREF_RELEASE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_Release:
    {
      m_original->AddRef();
      ULONG references = m_original->Release();
      references -= m_deviceReferenceCount;
      if (references == 1)
      {
        FreeDeviceSurfaces();
        references = m_original->Release();
        m_painter->RemoveStub(this);
        m_original = NULL;
      }
      else
      {
        m_original->Release();
        references--;
      }
      CHECK_CALL_RETURN_VALUE_ADDREF_RELEASE(ULONG, references);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetDirect3D:
    {
      DXResourceObjectID param1ResID;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      IDirect3D9* param1;

      HRESULT hr = m_original->GetDirect3D(&param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_Present:
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

      HRESULT result = m_original->Present(pSourceRectPtr, pDestRectPtr, hDestWindowOverride, (CONST RGNDATA*) pDirtyRegion);

      //////////////////////////////////////////////////////////////////////////

      m_painter->UpdateFrameStatistics();

      //////////////////////////////////////////////////////////////////////////

      CHECK_CALL_RETURN_VALUE_HRESULT(result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_Reset:
    {
      FreeDeviceSurfaces();

      char* param1;
      DXBufferPtr param1Buffer;
      DXBufferIdentifier buffer_id;
      CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
      CHECK_CALL(m_painter->GetBuffer(&param1Buffer, buffer_id));
      CHECK_CALL(param1Buffer->Pop_D3DPRESENT_PARAMETERS(&param1));

      //////////////////////////////////////////////////////////////////////////

      m_painter->CorrectDevicePresentationParameters((D3DPRESENT_PARAMETERS*) param1);

      //////////////////////////////////////////////////////////////////////////

      HRESULT hr = m_original->Reset((D3DPRESENT_PARAMETERS*) param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateVertexDeclaration:
    {
      char* param1;
      DXBufferPtr param1Buffer;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param1 = NULL;
        }
        else
        {
          DXBufferIdentifier buffer_id;
          CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
          CHECK_CALL(m_painter->GetBuffer(&param1Buffer, buffer_id));
          CHECK_CALL(param1Buffer->Pop_ARR_D3DVERTEXELEMENT9(&param1));
        }
      }

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
      }
      IDirect3DVertexDeclaration9* param2 = NULL;

      HRESULT hr = m_original->CreateVertexDeclaration((CONST D3DVERTEXELEMENT9*) param1, &param2);
      if (SUCCEEDED(hr))
      {
        new IDirect3DVertexDeclaration9InterceptorStub(m_painter, param2, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateVertexBuffer:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXFlagsD3DUSAGE param2;
      CHECK_CALL(call->Pop_DXFlagsD3DUSAGE(&param2));

      DXFlagsD3DFVF param3;
      CHECK_CALL(call->Pop_DXFlagsD3DFVF(&param3));

      D3DPOOL param4;
      CHECK_CALL(call->Pop_D3DPOOL(&param4));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param5ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param5ResID));
      }
      IDirect3DVertexBuffer9* param5;

      DXNullPointer param6;
      CHECK_CALL(call->Pop_DXNullPointer(&param6));

      HRESULT hr = m_original->CreateVertexBuffer(param1, param2, param3, param4, &param5, NULL);
      if (SUCCEEDED(hr))
      {
        new IDirect3DVertexBuffer9InterceptorStub(m_painter, param5, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateIndexBuffer:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXFlagsD3DUSAGE param2;
      CHECK_CALL(call->Pop_DXFlagsD3DUSAGE(&param2));

      D3DFORMAT param3;
      CHECK_CALL(call->Pop_D3DFORMAT(&param3));

      D3DPOOL param4;
      CHECK_CALL(call->Pop_D3DPOOL(&param4));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param5ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param5ResID));
      }
      IDirect3DIndexBuffer9* param5;

      DXNullPointer param6;
      CHECK_CALL(call->Pop_DXNullPointer(&param6));

      HRESULT hr = m_original->CreateIndexBuffer(param1, param2, param3, param4, &param5, NULL);
      if (SUCCEEDED(hr))
      {
        new IDirect3DIndexBuffer9InterceptorStub(m_painter, param5, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateTexture:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      DWORD param4;
      CHECK_CALL(call->Pop_DWORD(&param4));

      D3DFORMAT param5;
      CHECK_CALL(call->Pop_D3DFORMAT(&param5));

      D3DPOOL param6;
      CHECK_CALL(call->Pop_D3DPOOL(&param6));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param7ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param7ResID));
      }
      IDirect3DTexture9* param7;

      DXNullPointer param8;
      CHECK_CALL(call->Pop_DXNullPointer(&param8));

      HRESULT hr = m_original->CreateTexture(param1, param2, param3, param4, param5, param6, &param7, NULL);
      if (SUCCEEDED(hr))
      {
        new IDirect3DTexture9InterceptorStub(m_painter, param7, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateCubeTexture:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      DWORD param3;
      CHECK_CALL(call->Pop_DWORD(&param3));

      D3DFORMAT param4;
      CHECK_CALL(call->Pop_D3DFORMAT(&param4));

      D3DPOOL param5;
      CHECK_CALL(call->Pop_D3DPOOL(&param5));

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
      IDirect3DCubeTexture9* param6;

      DXNullPointer param7;
      CHECK_CALL(call->Pop_DXNullPointer(&param7));

      HRESULT hr = m_original->CreateCubeTexture(param1, param2, param3, param4, param5, &param6, NULL);
      if (SUCCEEDED(hr))
      {
        new IDirect3DCubeTexture9InterceptorStub(m_painter, param6, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateVolumeTexture:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      UINT param4;
      CHECK_CALL(call->Pop_UINT(&param4));

      DWORD param5;
      CHECK_CALL(call->Pop_DWORD(&param5));

      D3DFORMAT param6;
      CHECK_CALL(call->Pop_D3DFORMAT(&param6));

      D3DPOOL param7;
      CHECK_CALL(call->Pop_D3DPOOL(&param7));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param8ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param8ResID));
      }
      IDirect3DVolumeTexture9* param8;

      DXNullPointer param9;
      CHECK_CALL(call->Pop_DXNullPointer(&param9));

      HRESULT hr = m_original->CreateVolumeTexture(param1, param2, param3, param4, param5, param6, param7, &param8, NULL);
      if (SUCCEEDED(hr))
      {
        new IDirect3DVolumeTexture9InterceptorStub(m_painter, param8, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateDepthStencilSurface:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      D3DFORMAT param3;
      CHECK_CALL(call->Pop_D3DFORMAT(&param3));

      D3DMULTISAMPLE_TYPE param4;
      CHECK_CALL(call->Pop_D3DMULTISAMPLE_TYPE(&param4));

      DWORD param5;
      CHECK_CALL(call->Pop_DWORD(&param5));

      BOOL param6;
      CHECK_CALL(call->Pop_BOOL(&param6));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param7ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param7ResID));
      }
      IDirect3DSurface9* param7;

      DXNullPointer param8;
      CHECK_CALL(call->Pop_DXNullPointer(&param8));

      HRESULT hr = m_original->CreateDepthStencilSurface(param1, param2, param3, param4, param5, param6, &param7, NULL);
      if (SUCCEEDED(hr))
      {
        new IDirect3DSurface9InterceptorStub(m_painter, 0, param7, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateOffscreenPlainSurface:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      D3DFORMAT param3;
      CHECK_CALL(call->Pop_D3DFORMAT(&param3));

      D3DPOOL param4;
      CHECK_CALL(call->Pop_D3DPOOL(&param4));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param5ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param5ResID));
      }
      IDirect3DSurface9* param5;

      DXNullPointer param6;
      CHECK_CALL(call->Pop_DXNullPointer(&param6));

      HRESULT hr = m_original->CreateOffscreenPlainSurface(param1, param2, param3, param4, &param5, NULL);
      if (SUCCEEDED(hr))
      {
        new IDirect3DSurface9InterceptorStub(m_painter, 0, param5, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateRenderTarget:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      D3DFORMAT param3;
      CHECK_CALL(call->Pop_D3DFORMAT(&param3));

      D3DMULTISAMPLE_TYPE param4;
      CHECK_CALL(call->Pop_D3DMULTISAMPLE_TYPE(&param4));

      DWORD param5;
      CHECK_CALL(call->Pop_DWORD(&param5));

      BOOL param6;
      CHECK_CALL(call->Pop_BOOL(&param6));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param7ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param7ResID));
      }
      IDirect3DSurface9* param7;

      DXNullPointer param8;
      CHECK_CALL(call->Pop_DXNullPointer(&param8));

      HRESULT hr = m_original->CreateRenderTarget(param1, param2, param3, param4, param5, param6, &param7, NULL);
      if (SUCCEEDED(hr))
      {
        new IDirect3DSurface9InterceptorStub(m_painter, 0, param7, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetCursorProperties:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      DXResourceObjectID param3;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param3));

      HRESULT hr;
      if (param3 != 0 && m_painter->GetStub(param3))
      {
        hr = m_original->SetCursorProperties(param1, param2, (IDirect3DSurface9*) m_painter->GetStub(param3)->GetIUnknown());
      }
      else
      {
        hr = m_original->SetCursorProperties(param1, param2, NULL);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetFrontBufferData:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXResourceObjectID param2;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param2));

      HRESULT hr;
      if (param2 != 0 && m_painter->GetStub(param2))
      {
        hr = m_original->GetFrontBufferData(param1, (IDirect3DSurface9*) m_painter->GetStub(param2)->GetIUnknown());
      }
      else
      {
        hr = m_original->GetFrontBufferData(param1, NULL);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetVertexDeclaration:
    {
      DXResourceObjectID param1;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1));

      IDirect3DVertexDeclaration9* pVertexDecl = NULL;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        pVertexDecl = (IDirect3DVertexDeclaration9*) m_painter->GetStub(param1)->GetIUnknown();
      }

      HRESULT hr = m_original->SetVertexDeclaration(pVertexDecl);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetStreamSource:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXResourceObjectID param2;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      UINT param4;
      CHECK_CALL(call->Pop_UINT(&param4));

      HRESULT hr;
      if (param2 != 0 && m_painter->GetStub(param2))
      {
        hr = m_original->SetStreamSource(param1, (IDirect3DVertexBuffer9*) m_painter->GetStub(param2)->GetIUnknown(), param3, param4);
      }
      else
      {
        hr = m_original->SetStreamSource(param1, NULL, param3, param4);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetIndices:
    {
      DXResourceObjectID param1;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1));

      HRESULT hr;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        hr = m_original->SetIndices((IDirect3DIndexBuffer9*) m_painter->GetStub(param1)->GetIUnknown());
      }
      else
      {
        hr = m_original->SetIndices(NULL);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetTexture:
    {
      DWORD param1;
      CHECK_CALL(call->Pop_DWORD(&param1));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
      }
      IDirect3DBaseTexture9* param2;

      HRESULT hr = m_original->GetTexture(param1, &param2);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetTexture:
    {
      DWORD param1;
      CHECK_CALL(call->Pop_DWORD(&param1));

      DXResourceObjectID param2 = 0;
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2));
      }

      HRESULT hr;
      if (param2 != 0 && m_painter->GetStub(param2))
      {
        hr = m_original->SetTexture(param1, (IDirect3DBaseTexture9*) m_painter->GetStub(param2)->GetIUnknown());
      }
      else
      {
        hr = m_original->SetTexture(param1, NULL);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_ColorFill:
    {
      DXResourceObjectID param1;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1));

      RECT param2;
      RECT* param2Ptr = &param2;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param2Ptr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_RECT(param2Ptr));
        }
      }

      D3DCOLOR param3;
      CHECK_CALL(call->Pop_D3DCOLOR(&param3));

      HRESULT hr;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        hr = m_original->ColorFill((IDirect3DSurface9*) m_painter->GetStub(param1)->GetIUnknown(), param2Ptr, param3);
      }
      else
      {
        hr = m_original->ColorFill(NULL, param2Ptr, param3);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetBackBuffer:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));

      D3DBACKBUFFER_TYPE param3;
      CHECK_CALL(call->Pop_D3DBACKBUFFER_TYPE(&param3));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param4ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param4ResID));
      }
      IDirect3DSurface9* param4;

      HRESULT hr = m_original->GetBackBuffer(param1, param2, param3, &param4);

      if (FAILED(hr))
      {
        CHECK_CALL_RETURN_VALUE_HRESULT(hr);
      }

      IReferenceCounter* pRefCounter;
      DWORD pRefCounterSize = sizeof(IUnknown*);

      if (FAILED(param4->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
      {
        new IDirect3DSurface9InterceptorStub(m_painter, 0, param4, this);
        param4->AddRef();
        m_deviceReferenceCount++;
      }
      else
      {
        pRefCounter->Release();
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateStateBlock:
    {
      D3DSTATEBLOCKTYPE param1;
      CHECK_CALL(call->Pop_D3DSTATEBLOCKTYPE(&param1));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
      }
      IDirect3DStateBlock9* param2;

      HRESULT hr = m_original->CreateStateBlock(param1, &param2);
      if (SUCCEEDED(hr))
      {
        new IDirect3DStateBlock9InterceptorStub(m_painter, param2, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_EndStateBlock:
    {
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param1ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      }
      IDirect3DStateBlock9* param1;

      HRESULT hr = m_original->EndStateBlock(&param1);
      if (SUCCEEDED(hr))
      {
        new IDirect3DStateBlock9InterceptorStub(m_painter, param1, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateAdditionalSwapChain:
    {
      char* param1 = NULL;
      DXBufferPtr param1Buffer;
      {
        if (!call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXBufferIdentifier buffer_id;
          CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
          CHECK_CALL(m_painter->GetBuffer(&param1Buffer, buffer_id));
          CHECK_CALL(param1Buffer->Pop_D3DPRESENT_PARAMETERS(&param1));
        }
      }

      IDirect3DSwapChain9* param2;

      //////////////////////////////////////////////////////////////////////////

      m_painter->CorrectDevicePresentationParameters((D3DPRESENT_PARAMETERS*) param1);

      //////////////////////////////////////////////////////////////////////////

      HRESULT hr = m_original->CreateAdditionalSwapChain((D3DPRESENT_PARAMETERS*) param1, &param2);

      if (SUCCEEDED(hr))
      {
        new IDirect3DSwapChain9InterceptorStub(m_painter, param2, this);
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetSwapChain:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
      }
      IDirect3DSwapChain9* param2;

      HRESULT hr = m_original->GetSwapChain(param1, &param2);

      if (SUCCEEDED(hr))
      {
        new IDirect3DSwapChain9InterceptorStub(m_painter, param2, this);
        param2->AddRef();
        m_deviceReferenceCount++;
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetPixelShader:
    {
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param1ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      }
      IDirect3DPixelShader9* param1;

      HRESULT hr = m_original->GetPixelShader(&param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetPixelShader:
    {
      DXResourceObjectID param1 = 0;
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        CHECK_CALL(call->Pop_DXResourceObjectID(&param1));
      }

      IDirect3DPixelShader9* pShader = NULL;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        pShader = (IDirect3DPixelShader9*) m_painter->GetStub(param1)->GetIUnknown();
      }

      HRESULT hr = m_original->SetPixelShader(pShader);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetRenderTarget:
    {
      DWORD param1;
      CHECK_CALL(call->Pop_DWORD(&param1));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
      }
      IDirect3DSurface9* param2;

      HRESULT hr = m_original->GetRenderTarget(param1, &param2);

      if (FAILED(hr))
      {
        CHECK_CALL_RETURN_VALUE_HRESULT(hr);
      }

      IReferenceCounter* pRefCounter;
      DWORD pRefCounterSize = sizeof(IUnknown*);

      if (FAILED(param2->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
      {
        new IDirect3DSurface9InterceptorStub(m_painter, 0, param2, this);
        param2->AddRef();
        m_deviceReferenceCount++;
      }
      else
      {
        pRefCounter->Release();
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetRenderTargetData:
    {
      DXResourceObjectID param1;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1));

      DXResourceObjectID param2;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param2));

      IDirect3DSurface9* pSurface1 = NULL;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        pSurface1 = (IDirect3DSurface9*) m_painter->GetStub(param1)->GetIUnknown();
      }

      IDirect3DSurface9* pSurface2 = NULL;
      if (param2 != 0 && m_painter->GetStub(param2))
      {
        pSurface2 = (IDirect3DSurface9*) m_painter->GetStub(param2)->GetIUnknown();
      }

      HRESULT hr = m_original->GetRenderTargetData(pSurface1, pSurface2);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_UpdateSurface:
    {
      DXResourceObjectID param1;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1));

      RECT param2;
      RECT* param2Ptr = &param2;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param2Ptr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_RECT(param2Ptr));
        }
      }

      DXResourceObjectID param3;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param3));

      POINT param4;
      POINT* param4Ptr = &param4;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param4Ptr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_POINT(param4Ptr));
        }
      }

      IDirect3DSurface9* pSurface1 = NULL;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        pSurface1 = (IDirect3DSurface9*) m_painter->GetStub(param1)->GetIUnknown();
      }

      IDirect3DSurface9* pSurface2 = NULL;
      if (param3 != 0 && m_painter->GetStub(param3))
      {
        pSurface2 = (IDirect3DSurface9*) m_painter->GetStub(param3)->GetIUnknown();
      }

      HRESULT hr = m_original->UpdateSurface(pSurface1, param2Ptr, pSurface2, param4Ptr);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_StretchRect:
    {
      DXResourceObjectID param1;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1));

      RECT param2;
      RECT* param2Ptr = &param2;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param2Ptr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_RECT(param2Ptr));
        }
      }

      DXResourceObjectID param3;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param3));

      RECT param4;
      RECT* param4Ptr = &param4;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param4Ptr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_RECT(param4Ptr));
        }
      }

      D3DTEXTUREFILTERTYPE param5;
      CHECK_CALL(call->Pop_D3DTEXTUREFILTERTYPE(&param5));

      IDirect3DSurface9* pSurface1 = NULL;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        pSurface1 = (IDirect3DSurface9*) m_painter->GetStub(param1)->GetIUnknown();
      }

      IDirect3DSurface9* pSurface2 = NULL;
      if (param3 != 0 && m_painter->GetStub(param3))
      {
        pSurface2 = (IDirect3DSurface9*) m_painter->GetStub(param3)->GetIUnknown();
      }

      HRESULT hr = m_original->StretchRect(pSurface1, param2Ptr, pSurface2, param4Ptr, param5);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_UpdateTexture:
    {
      DXResourceObjectID param1;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1));

      DXResourceObjectID param2;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param2));

      IDirect3DTexture9* pTexture1 = NULL;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        pTexture1 = (IDirect3DTexture9*) m_painter->GetStub(param1)->GetIUnknown();
      }

      IDirect3DTexture9* pTexture2 = NULL;
      if (param2 != 0 && m_painter->GetStub(param2))
      {
        pTexture2 = (IDirect3DTexture9*) m_painter->GetStub(param2)->GetIUnknown();
      }

      HRESULT hr = m_original->UpdateTexture(pTexture1, pTexture2);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetRenderTarget:
    {
      DWORD param1;
      CHECK_CALL(call->Pop_DWORD(&param1));

      DXResourceObjectID param2 = 0;
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2));
      }

      IDirect3DSurface9* pRenderTarget = NULL;
      if (param2 != 0 && m_painter->GetStub(param2))
      {
        pRenderTarget = (IDirect3DSurface9*) m_painter->GetStub(param2)->GetIUnknown();
      }

      HRESULT hr = m_original->SetRenderTarget(param1, pRenderTarget);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetVertexShader:
    {
      IDirect3DVertexShader9* pShader;
      HRESULT hr = m_original->GetVertexShader(&pShader);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetVertexShader:
    {
      DXResourceObjectID param1 = 0;
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        CHECK_CALL(call->Pop_DXResourceObjectID(&param1));
      }

      IDirect3DVertexShader9* pShader = NULL;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        pShader = (IDirect3DVertexShader9*) m_painter->GetStub(param1)->GetIUnknown();
      }

      HRESULT hr = m_original->SetVertexShader(pShader);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetDepthStencilSurface:
    {
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param1ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      }
      IDirect3DSurface9* param1 = NULL;

      HRESULT hr = m_original->GetDepthStencilSurface(&param1);

      if (FAILED(hr))
      {
        CHECK_CALL_RETURN_VALUE_HRESULT(hr);
      }

      IReferenceCounter* pRefCounter;
      DWORD pRefCounterSize = sizeof(IUnknown*);

      if (FAILED(param1->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
      {
        new IDirect3DSurface9InterceptorStub(m_painter, 0, param1, this);
        param1->AddRef();
        m_deviceReferenceCount++;
      }
      else
      {
        pRefCounter->Release();
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetDepthStencilSurface:
    {
      DXResourceObjectID param1 = 0;
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        CHECK_CALL(call->Pop_DXResourceObjectID(&param1));
      }

      IDirect3DSurface9* pNewZStencil = NULL;
      if (param1 != 0 && m_painter->GetStub(param1))
      {
        pNewZStencil = (IDirect3DSurface9*) m_painter->GetStub(param1)->GetIUnknown();
      }

      HRESULT hr = m_original->SetDepthStencilSurface(pNewZStencil);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreatePixelShader:
    {
      char* param1;
      DXBufferPtr param1Buffer;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param1 = NULL;
        }
        else
        {
          DXBufferIdentifier buffer_id;
          CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
          CHECK_CALL(m_painter->GetBuffer(&param1Buffer, buffer_id));
          CHECK_CALL(param1Buffer->Pop_ARR_SHADERFUNCTIONTOKEN(&param1));
        }
      }

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
      }
      IDirect3DPixelShader9* param2 = NULL;

      HRESULT hr = m_original->CreatePixelShader((CONST DWORD*) param1, &param2);
      if (SUCCEEDED(hr))
      {
        new IDirect3DPixelShader9InterceptorStub(m_painter, param2, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateVertexShader:
    {
      char* param1;
      DXBufferPtr param1Buffer;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param1 = NULL;
        }
        else
        {
          DXBufferIdentifier buffer_id;
          CHECK_CALL(call->Pop_DXBufferIdentifier(&buffer_id));
          CHECK_CALL(m_painter->GetBuffer(&param1Buffer, buffer_id));
          CHECK_CALL(param1Buffer->Pop_ARR_SHADERFUNCTIONTOKEN(&param1));
        }
      }

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
      }
      IDirect3DVertexShader9* param2 = NULL;

      HRESULT hr = m_original->CreateVertexShader((CONST DWORD*) param1, &param2);
      if (SUCCEEDED(hr))
      {
        new IDirect3DVertexShader9InterceptorStub(m_painter, param2, this);
      }
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_CreateQuery:
    {
      D3DQUERYTYPE param1;
      CHECK_CALL(call->Pop_D3DQUERYTYPE(&param1));

      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);

        HRESULT hr = m_original->CreateQuery(param1, NULL);
        CHECK_CALL_RETURN_VALUE_HRESULT(hr);
      }
      else
      {
        DXResourceObjectID param2ResID;
        CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
        IDirect3DQuery9* param2 = NULL;

        HRESULT hr = m_original->CreateQuery(param1, &param2);
        if (SUCCEEDED(hr))
        {
          new IDirect3DQuery9InterceptorStub(m_painter, param2, this);
        }
        CHECK_CALL_RETURN_VALUE_HRESULT(hr);
      }
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetPixelShaderConstantB:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXIgnoredParameter param2;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      char* param2Ptr = new char[param3*sizeof(BOOL)];

      HRESULT hr = m_original->GetPixelShaderConstantB(param1, (BOOL*) param2Ptr, param3);

      delete[] param2Ptr;

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetPixelShaderConstantF:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXIgnoredParameter param2;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      char* param2Ptr = new char[param3*sizeof(float)*4];

      HRESULT hr = m_original->GetPixelShaderConstantF(param1, (float*) param2Ptr, param3);

      delete[] param2Ptr;

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetPixelShaderConstantI:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXIgnoredParameter param2;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      char* param2Ptr = new char[param3*sizeof(int)*4];

      HRESULT hr = m_original->GetPixelShaderConstantI(param1, (int*) param2Ptr, param3);

      delete[] param2Ptr;

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetVertexShaderConstantB:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXIgnoredParameter param2;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      char* param2Ptr = new char[param3*sizeof(BOOL)];

      HRESULT hr = m_original->GetVertexShaderConstantB(param1, (BOOL*) param2Ptr, param3);

      delete[] param2Ptr;

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetVertexShaderConstantF:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXIgnoredParameter param2;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      char* param2Ptr = new char[param3*sizeof(float)*4];

      HRESULT hr = m_original->GetVertexShaderConstantF(param1, (float*) param2Ptr, param3);

      delete[] param2Ptr;

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_GetVertexShaderConstantI:
    {
      UINT param1;
      CHECK_CALL(call->Pop_UINT(&param1));

      DXIgnoredParameter param2;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param2));

      UINT param3;
      CHECK_CALL(call->Pop_UINT(&param3));

      char* param2Ptr = new char[param3*sizeof(int)*4];

      HRESULT hr = m_original->GetVertexShaderConstantI(param1, (int*) param2Ptr, param3);

      delete[] param2Ptr;

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DDevice9_SetCursorPosition:
    {
      /*
      // PERFER
      int X;
      CHECK_CALL(call->Pop_int(&X));

      int Y;
      CHECK_CALL(call->Pop_int(&Y));

      DWORD Flags;
      CHECK_CALL(call->Pop_DWORD(&Flags));

      m_original->SetCursorPosition(X, Y, Flags);
      */
      return D3D_OK;
    }
    break;
  }

  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

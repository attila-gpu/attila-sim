////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorStub.h"
#include "IDirect3DSurface9InterceptorStub.h"
#include "IDirect3DCubeTexture9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DCubeTexture9InterceptorStub::IDirect3DCubeTexture9InterceptorStub(DXPainter* painter, IDirect3DCubeTexture9* original, IDirect3DDevice9InterceptorStub* creator) : 
DXInterceptorStub(painter),
m_original(original),
m_creator(creator)
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

IDirect3DCubeTexture9InterceptorStub::~IDirect3DCubeTexture9InterceptorStub()
{
  if (m_locks)
  {
    delete[] m_locks;
    m_locks = NULL;
  }

  FREE_D3D_RESOURCE();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DCubeTexture9InterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_QueryInterface:
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

  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_AddRef:
    {
      ULONG result = m_original->AddRef();
      CHECK_CALL_RETURN_VALUE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_Release:
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

  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_GetDevice:
    {
      DXResourceObjectID param1ResID;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      IDirect3DDevice9* param1;
      
      HRESULT hr = m_original->GetDevice(&param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_GetCubeMapSurface:
    {
      D3DCUBEMAP_FACES param1;
      CHECK_CALL(call->Pop_D3DCUBEMAP_FACES(&param1));

      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));
      
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
      IDirect3DSurface9* pSurface;

      HRESULT hr = m_original->GetCubeMapSurface(param1, param2, &pSurface);

      if (FAILED(hr))
      {
        CHECK_CALL_RETURN_VALUE_HRESULT(hr);
      }

      IReferenceCounter* pRefCounter;
      DWORD pRefCounterSize = sizeof(IUnknown*);
      
      if (FAILED(pSurface->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
      {
        new IDirect3DSurface9InterceptorStub(m_painter, param2, pSurface, m_creator);
      }
      else
      {
        pRefCounter->Release();
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_LockRect:
    {
      D3DCUBEMAP_FACES param1;
      CHECK_CALL(call->Pop_D3DCUBEMAP_FACES(&param1));
      
      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));
      
      DXIgnoredParameter param3;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param3));

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

      DXFlagsD3DLOCK param5;
      CHECK_CALL(call->Pop_DXFlagsD3DLOCK(&param5));

      D3DLOCKED_RECT lockedRect;

      HRESULT hr = m_original->LockRect(param1, param2, &lockedRect, param4Ptr, param5);

      if (FAILED(hr) || (param5 & D3DLOCK_READONLY))
      {
        lockedRect.pBits = 0;
        m_locks[param2][param1].SetLockedRect(&lockedRect);
      }
      else
      {
        m_locks[param2][param1].SetLockedRect(&lockedRect);
        m_locks[param2][param1].SetLock(m_original, param2, param4Ptr, m_format);
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_UnlockRect:
    {
      D3DCUBEMAP_FACES param1;
      CHECK_CALL(call->Pop_D3DCUBEMAP_FACES(&param1));
      
      UINT param2;
      CHECK_CALL(call->Pop_UINT(&param2));
      
      if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
      {
        DXNullPointer checkNull;
        call->Pop_DXNullPointer(&checkNull);
      }
      else
      {
        DXTextureIdentifier texture_id;
        CHECK_CALL(call->Pop_DXTextureIdentifier(&texture_id));
        DXTexturePtr texture;
        CHECK_CALL(m_painter->GetTexture(&texture, texture_id));

        if (!m_locks[param2][param1].ReadLock(texture))
        {
          return E_FAIL;
        }
      }

      HRESULT hr = m_original->UnlockRect(param1, param2);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_GetPrivateData:
    return D3DResource9_GetPrivateData(m_original, call);
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_SetPrivateData:
    return D3DResource9_SetPrivateData(m_original, call);
    break;

  case DXMethodCallHelper::TOK_IDirect3DCubeTexture9_FreePrivateData:
    return D3DResource9_FreePrivateData(m_original, call);
    break;
  }

  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorStub.h"
#include "IDirect3DSurface9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DSurface9InterceptorStub::IDirect3DSurface9InterceptorStub(DXPainter* painter, UINT level, IDirect3DSurface9* original, IDirect3DDevice9InterceptorStub* creator) : 
DXInterceptorStub(painter),
m_original(original),
m_creator(creator)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();

  D3DSURFACE_DESC	description;
  m_original->GetDesc(&description);
  m_format = description.Format;
  m_level = level;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DSurface9InterceptorStub::~IDirect3DSurface9InterceptorStub()
{
  FREE_D3D_RESOURCE();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DSurface9InterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DSurface9_QueryInterface:
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

  case DXMethodCallHelper::TOK_IDirect3DSurface9_AddRef:
    {
      ULONG result = m_original->AddRef();
      CHECK_CALL_RETURN_VALUE_ADDREF_RELEASE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DSurface9_Release:
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

  case DXMethodCallHelper::TOK_IDirect3DSurface9_GetDevice:
    {
      DXResourceObjectID param1ResID;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      IDirect3DDevice9* param1;

      HRESULT hr = m_original->GetDevice(&param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DSurface9_LockRect:
    {
      DXIgnoredParameter param1;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param1));

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

      DXFlagsD3DLOCK param3;
      CHECK_CALL(call->Pop_DXFlagsD3DLOCK(&param3));

      D3DLOCKED_RECT lockedRect;
      
      HRESULT hr = m_original->LockRect(&lockedRect, param2Ptr, param3);

      if (FAILED(hr) || (param3 & D3DLOCK_READONLY))
      {
        lockedRect.pBits = 0;
        m_lock.SetLockedRect(&lockedRect);
      }
      else
      {
        m_lock.SetLockedRect(&lockedRect);
        m_lock.SetLock(m_original, m_level, param2Ptr, m_format);
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DSurface9_UnlockRect:
    {
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
        
        if (!m_lock.ReadLock(texture))
        {
          return E_FAIL;
        }
      }
      
      HRESULT hr = m_original->UnlockRect();
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DSurface9_GetPrivateData:
    return D3DResource9_GetPrivateData(m_original, call);
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DSurface9_SetPrivateData:
    return D3DResource9_SetPrivateData(m_original, call);
    break;

  case DXMethodCallHelper::TOK_IDirect3DSurface9_FreePrivateData:
    return D3DResource9_FreePrivateData(m_original, call);
    break;
  }

  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

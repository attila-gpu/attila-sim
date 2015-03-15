////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorStub.h"
#include "IDirect3DVolume9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolume9InterceptorStub::IDirect3DVolume9InterceptorStub(DXPainter* painter, UINT level, IDirect3DVolume9* original, IDirect3DDevice9InterceptorStub* creator) : 
DXInterceptorStub(painter),
m_original(original),
m_creator(creator)
{
  IReferenceCounter* pRefCounter = new IReferenceCounter(this);
  m_original->SetPrivateData(IID_IReferenceCounter, pRefCounter, sizeof(IUnknown*), D3DSPD_IUNKNOWN);
  pRefCounter->Release();

  m_lock = new DXVolumeLock;

  D3DVOLUME_DESC	description;
  m_original->GetDesc(&description);
  m_format = description.Format;
  m_level = level;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVolume9InterceptorStub::~IDirect3DVolume9InterceptorStub()
{
  if (m_lock)
  {
    delete m_lock;
    m_lock = NULL;
  }

  FREE_D3D_RESOURCE();
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVolume9InterceptorStub::DoSpecific(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_IDirect3DVolume9_QueryInterface:
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

  case DXMethodCallHelper::TOK_IDirect3DVolume9_AddRef:
    {
      ULONG result = m_original->AddRef();
      CHECK_CALL_RETURN_VALUE(ULONG, result);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolume9_Release:
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

  case DXMethodCallHelper::TOK_IDirect3DVolume9_GetDevice:
    {
      DXResourceObjectID param1ResID;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param1ResID));
      IDirect3DDevice9* param1;

      HRESULT hr = m_original->GetDevice(&param1);
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolume9_LockBox:
    {
      DXIgnoredParameter param1;
      CHECK_CALL(call->Pop_DXIgnoredParameter(&param1));

      D3DBOX param2;
      D3DBOX* param2Ptr = &param2;
      {
        if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
        {
          DXNullPointer checkNull;
          call->Pop_DXNullPointer(&checkNull);
          param2Ptr = NULL;
        }
        else
        {
          CHECK_CALL(call->Pop_D3DBOX(param2Ptr));
        }
      }

      DXFlagsD3DLOCK param3;
      CHECK_CALL(call->Pop_DXFlagsD3DLOCK(&param3));

      D3DLOCKED_BOX lockedBox;

      HRESULT hr = m_original->LockBox(&lockedBox, param2Ptr, param3);

      if (FAILED(hr) || (param3 & D3DLOCK_READONLY))
      {
        lockedBox.pBits = 0;
        m_lock[0].SetLockedBox(&lockedBox);
      }
      else
      {
        m_lock[0].SetLockedBox(&lockedBox);
        m_lock[0].SetLock(m_original, 0, param2Ptr, m_format);
      }

      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolume9_UnlockBox:
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

        if (!m_lock[0].ReadLock(texture))
        {
          return E_FAIL;
        }
      }

      HRESULT hr = m_original->UnlockBox();
      CHECK_CALL_RETURN_VALUE_HRESULT(hr);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DVolume9_GetPrivateData:
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

      HRESULT result = m_original->GetPrivateData(refguid, pData, &pSizeOfData);
      return D3D_OK;
    }
    break;

  case DXMethodCallHelper::TOK_IDirect3DVolume9_SetPrivateData:
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

      //////////////////////////////////////////////////////////////////////////
      // Ignore if equal to the internal IReferenceCounter IID

      if (refguid == IID_IReferenceCounter)
      {
        return D3D_OK;
      }

      //////////////////////////////////////////////////////////////////////////

      HRESULT result = m_original->SetPrivateData(refguid, (CONST void*) pData, SizeOfData, Flags);
      CHECK_CALL_RETURN_VALUE_HRESULT(result);
    }
    break;
  
  case DXMethodCallHelper::TOK_IDirect3DVolume9_FreePrivateData:
    {
      IID refguid;
      CHECK_CALL(call->Pop_IID(&refguid));

      //////////////////////////////////////////////////////////////////////////
      // Ignore if equal to the internal IReferenceCounter IID

      if (refguid == IID_IReferenceCounter)
      {
        return D3D_OK;
      }

      //////////////////////////////////////////////////////////////////////////
      
      HRESULT result = m_original->FreePrivateData(refguid);
      CHECK_CALL_RETURN_VALUE_HRESULT(result);
    }
    break;
  }

  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

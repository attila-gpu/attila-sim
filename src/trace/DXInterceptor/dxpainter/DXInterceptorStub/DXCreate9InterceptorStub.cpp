////////////////////////////////////////////////////////////////////////////////

#include "IDirect3D9InterceptorStub.h"
#include "DXCreate9InterceptorStub.h"

////////////////////////////////////////////////////////////////////////////////

DXCreate9InterceptorStub::DXCreate9InterceptorStub(DXPainter* painter) :
DXInterceptorStub(painter)
{
}

////////////////////////////////////////////////////////////////////////////////

DXCreate9InterceptorStub::~DXCreate9InterceptorStub()
{
}

////////////////////////////////////////////////////////////////////////////////

IUnknown* DXCreate9InterceptorStub::GetIUnknown() const
{
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT DXCreate9InterceptorStub::HandleCall(DXMethodCallPtr call)
{
  switch (call->GetToken())
  {
  case DXMethodCallHelper::TOK_Direct3DCreate9:
    DWORD param1;
    CHECK_CALL(call->Pop_DWORD(&param1));
    
    if (call->CheckNextPopType(DXTypeHelper::TT_DXNullPointer))
    {
      DXNullPointer checkNull;
      call->Pop_DXNullPointer(&checkNull);
      return D3DERR_INVALIDCALL;
    }
    else
    {
      DXResourceObjectID param2ResID;
      CHECK_CALL(call->Pop_DXResourceObjectID(&param2ResID));
    }
    
    IDirect3D9* d3d9 = Direct3DCreate9(param1);
    
    if (d3d9)
    {
      new IDirect3D9InterceptorStub(m_painter, d3d9);
      return D3D_OK;
    }
    else
    {
      d3d9 = NULL;
      return D3DERR_INVALIDCALL;
    }
    break;
  }

  return E_NOTIMPL;
}

////////////////////////////////////////////////////////////////////////////////

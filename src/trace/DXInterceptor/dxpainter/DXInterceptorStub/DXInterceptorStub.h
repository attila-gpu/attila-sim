////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXTraceManagerHeaders.h"
#include "GeometryTypes.h"
#include "DXPainter.h"

using namespace dxpainter;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

class IDirect3D9InterceptorStub;
class IDirect3DDevice9InterceptorStub;
class IDirect3DVertexDeclaration9InterceptorStub;
class IDirect3DVertexBuffer9InterceptorStub;
class IDirect3DIndexBuffer9InterceptorStub;
class IDirect3DTexture9InterceptorStub;
class IDirect3DCubeTexture9InterceptorStub;
class IDirect3DSurface9InterceptorStub;
class IDirect3DStateBlock9InterceptorStub;
class IDirect3DPixelShader9InterceptorStub;
class IDirect3DVertexShader9InterceptorStub;
class IDirect3DQuery9InterceptorStub;
class IDirect3DSwapChain9InterceptorStub;

////////////////////////////////////////////////////////////////////////////////
// Macro to liberate Direct3D resources and ignore C0000005 exceptions

#define FREE_D3D_RESOURCE() \
  if (m_original) \
  { \
    try \
    { \
      while (m_original->Release()) \
      m_original = NULL; \
    } \
    catch (unsigned) \
    { \
    } \
  }

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define CHECK_CALL(result) \
  if (!(result)) \
  { \
    m_painter->ShowError("ERROR\n\nMethod: %s\nLine: '%s'", __FUNCTION__, #result); \
    return E_FAIL; \
  }
#else
#define CHECK_CALL(result) result
#endif // ifdef DEBUG

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define CHECK_CALL_RETURN_VALUE(tipo, result1) \
  tipo result2; \
  CHECK_CALL(call->Pop_##tipo(&result2)); \
  return (result1 == result2 ? D3D_OK : D3DERR_INVALIDCALL);
#else
#define CHECK_CALL_RETURN_VALUE(tipo, result1) return D3D_OK;
#endif // ifdef _DEBUG

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG // PERFER : activar quan les reproduccions donen problemes
#define CHECK_CALL_RETURN_VALUE_HRESULT(result1) \
  HRESULT result2 = D3D_OK; \
  if (call->GetIsSavedReturnValue()) \
  { \
    CHECK_CALL(call->Pop_HRESULT(&result2)); \
  } \
  return (result1 == result2 ? D3D_OK : result1);
#else
#define CHECK_CALL_RETURN_VALUE_HRESULT(result1) return D3D_OK;
#endif // ifdef _DEBUG

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG // PERFER : activar quan les reproduccions donen problemes
#define CHECK_CALL_RETURN_VALUE_ADDREF_RELEASE(tipo, result1) \
  tipo result2; \
  CHECK_CALL(call->Pop_##tipo(&result2)); \
  if (result1 != result2) \
  { \
    m_painter->ShowError("ERROR\n\nMethod: %s\nIncorrect AddRef/Release\nReaded from trace = %u vs. Reproduced = %u", __FUNCTION__, result2, result1); \
    return D3DERR_INVALIDCALL; \
  } \
  else return D3D_OK;
#else
#define CHECK_CALL_RETURN_VALUE_ADDREF_RELEASE(tipo, result1) return D3D_OK;
#endif // ifdef _DEBUG

////////////////////////////////////////////////////////////////////////////////

class DXInterceptorStub
{
public:

  DXInterceptorStub(DXPainter* painter);
  virtual ~DXInterceptorStub();

  DWORD GetStubID() const;

  virtual IUnknown* GetIUnknown() const = 0;
  virtual HRESULT HandleCall(DXMethodCallPtr call) = 0;
  virtual HRESULT DoSpecific(DXMethodCallPtr call);
  virtual HRESULT D3DResource9_GetPrivateData(IDirect3DResource9* resource, DXMethodCallPtr call);
  virtual HRESULT D3DResource9_SetPrivateData(IDirect3DResource9* resource, DXMethodCallPtr call);
  virtual HRESULT D3DResource9_FreePrivateData(IDirect3DResource9* resource, DXMethodCallPtr call);

protected:
  
  DXPainter* m_painter;

private:

  DWORD m_stubID;

};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3D9InterceptorWrapper.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DVertexDeclaration9InterceptorWrapper.h"
#include "IDirect3DVertexBuffer9InterceptorWrapper.h"
#include "IDirect3DIndexBuffer9InterceptorWrapper.h"
#include "IDirect3DTexture9InterceptorWrapper.h"
#include "IDirect3DCubeTexture9InterceptorWrapper.h"
#include "IDirect3DVolume9InterceptorWrapper.h"
#include "IDirect3DVolumeTexture9InterceptorWrapper.h"
#include "IDirect3DSurface9InterceptorWrapper.h"
#include "IDirect3DStateBlock9InterceptorWrapper.h"
#include "IDirect3DPixelShader9InterceptorWrapper.h"
#include "IDirect3DVertexShader9InterceptorWrapper.h"
#include "IDirect3DQuery9InterceptorWrapper.h"
#include "IDirect3DSwapChain9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

#define INTERCEPT_VERTEXSHADERS
#define INTERCEPT_PIXELSHADERS
#define INTERCEPT_VERTEXBUFFERS
#define INTERCEPT_INDEXBUFFERS
#define INTERCEPT_SURFACESTEXTURES
#define INTERCEPT_STATEBLOCKS
#define INTERCEPT_QUERYS
#define INTERCEPT_SWAPCHAINS

////////////////////////////////////////////////////////////////////////////////

IDirect3DDevice9InterceptorWrapper::IDirect3DDevice9InterceptorWrapper(REFIID iid, IDirect3DDevice9* original, IDirect3D9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator),
m_banner(NULL),
m_deviceReferenceCount(0)
{
  Banner_Create();

#ifndef INTERCEPT_VERTEXSHADERS
  g_logger->Write("INFO: Ignored interception of IDirect3DVertexDeclaration9 in compilation time");
#endif // INTERCEPT_VERTEXSHADERS

#ifndef INTERCEPT_PIXELSHADERS
  g_logger->Write("INFO: Ignored interception of IDirect3DPixelShader9 in compilation time");
#endif // INTERCEPT_PIXELSHADERS

#ifndef INTERCEPT_VERTEXBUFFERS
  g_logger->Write("INFO: Ignored interception of IDirect3DVertexBuffer9 in compilation time");
#endif // INTERCEPT_VERTEXBUFFERS

#ifndef INTERCEPT_INDEXBUFFERS
  g_logger->Write("INFO: Ignored interception of IDirect3DIndexBuffer9 in compilation time");
#endif // INTERCEPT_INDEXBUFFERS

#ifndef INTERCEPT_SURFACESTEXTURES
  g_logger->Write("INFO: Ignored interception of IDirect3DSurface9 in compilation time");
  g_logger->Write("INFO: Ignored interception of IDirect3DTexture9 in compilation time");
  g_logger->Write("INFO: Ignored interception of IDirect3DCubeTexture9 in compilation time");
  g_logger->Write("INFO: Ignored interception of IDirect3DVolumeTexture9 in compilation time");
#endif // INTERCEPT_SURFACESTEXTURES

#ifndef INTERCEPT_STATEBLOCKS
  g_logger->Write("INFO: Ignored interception of IDirect3DStateBlock9 in compilation time");
#endif // INTERCEPT_STATEBLOCKS

#ifndef INTERCEPT_QUERYS
  g_logger->Write("INFO: Ignored interception of IDirect3DQuery9 in compilation time");
#endif // INTERCEPT_QUERYS

#ifndef INTERCEPT_SWAPCHAINS
  g_logger->Write("INFO: Ignored interception of IDirect3DSwapChain9 in compilation time");
#endif // INTERCEPT_SWAPCHAINS
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DDevice9InterceptorWrapper::~IDirect3DDevice9InterceptorWrapper()
{
  Banner_Destroy();
}

////////////////////////////////////////////////////////////////////////////////

void IDirect3DDevice9InterceptorWrapper::Banner_Create()
{
  if (g_options->GetBannerShow())
  {
    m_banner = new DXInterceptorBanner(m_original);
    if (m_banner)
    {
      char cadena[256];
      
      m_banner->Init();      
      m_banner->Message("Started Direct3D Capture Session", D3DCOLOR_RGBA(128, 255, 128, 255), 30000);
      sprintf(cadena, "Data saved in '%s'", g_traceman->GetProjectFileName().c_str());
      m_banner->Message(cadena, D3DCOLOR_RGBA(128, 128, 255, 255), 60000);
      if (g_statman && g_statman->GetCounterToRecordCount())
      {
        sprintf(cadena, "Saving statistics for %u counters", g_statman->GetCounterToRecordCount());
        m_banner->Message(cadena, D3DCOLOR_RGBA(128, 128, 255, 255), 60000);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void IDirect3DDevice9InterceptorWrapper::Banner_Destroy()
{
  if (m_banner)
  {
    delete m_banner;
    m_banner = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

void IDirect3DDevice9InterceptorWrapper::Banner_Draw()
{
  if (m_banner)
  {
    m_original->BeginScene();
    m_banner->Draw();
    m_original->EndScene();
  }
}

////////////////////////////////////////////////////////////////////////////////

void IDirect3DDevice9InterceptorWrapper::Banner_DrawError(const char* cadena, unsigned int timeout)
{
  if (m_banner)
  {
    m_banner->Message(cadena, D3DCOLOR_RGBA(255,0,0,255), timeout);
  }
  g_logger->Write(cadena);
}

////////////////////////////////////////////////////////////////////////////////

void IDirect3DDevice9InterceptorWrapper::FreeDeviceSurfaces()
{
  // Free correctly the current render target surface
  IDirect3DSurface9* currentRenderTarget = NULL;
  if (SUCCEEDED(m_original->GetRenderTarget(0, &currentRenderTarget)))
  {
    IReferenceCounter* pRefCounter;
    DWORD pRefCounterSize = sizeof(IUnknown*);
    if (SUCCEEDED(currentRenderTarget->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
    {
      IDirect3DSurface9InterceptorWrapper* myCurrentRenderTarget = (IDirect3DSurface9InterceptorWrapper*) pRefCounter->GetOwner();
      pRefCounter->Release();
      delete myCurrentRenderTarget;
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
      IDirect3DSurface9InterceptorWrapper* myCurrentDepthStencil = (IDirect3DSurface9InterceptorWrapper*) pRefCounter->GetOwner();
      pRefCounter->Release();
      delete myCurrentDepthStencil;
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

HRESULT IDirect3DDevice9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
{
  if (IsEqualGUID(riid, IID_IUnknown))
  {
    m_original->AddRef();
    *ppvObj = (IUnknown*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DDevice9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DDevice9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DDevice9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DDevice9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

ULONG IDirect3DDevice9InterceptorWrapper::AddRef_Specific()
{
  ULONG references = m_original->AddRef();
  if (m_banner)
  {
    references -= m_banner->GetDeviceReferenceCount();
  }
  return references;
}

////////////////////////////////////////////////////////////////////////////////

ULONG IDirect3DDevice9InterceptorWrapper::Release_Specific()
{
  m_original->AddRef();
  ULONG references = m_original->Release();
  
  references -= m_deviceReferenceCount;
  if (m_banner)
  {
    references -= m_banner->GetDeviceReferenceCount();
  }
  
  if (references == 1)
  {
    FreeDeviceSurfaces();

    if (m_banner)
    {
      m_banner->Close();
    }

    references = m_original->Release();
  }
  else
  {
    m_original->Release();
    references--;
  }

  return references;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetDirect3D_Specific(IDirect3D9** ppD3D9, DXMethodCallPtr call)
{
  m_original->GetDirect3D(ppD3D9);
  *ppD3D9 = m_creator;
  
  call->Push_DXResourceObjectID(m_creator->GetObjectID());
  
  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::Reset_Specific(D3DPRESENT_PARAMETERS* pPresentationParameters)
{
  FreeDeviceSurfaces();

  if (m_banner)
  {
    m_banner->Close();
  }
  
  HRESULT hr = m_original->Reset(pPresentationParameters);
  
  if (SUCCEEDED(hr) && m_banner)
  {
    m_banner->Init();
  }
  
  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetViewport_Specific(CONST D3DVIEWPORT9* pViewport)
{
  HRESULT hr = m_original->SetViewport(pViewport);
  if (SUCCEEDED(hr) && m_banner)
  {
    m_banner->SetViewport(pViewport);
  }
  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::Present_Specific(CONST RECT* pSourceRect, CONST RECT* pDestRect, HWND hDestWindowOverride, CONST RGNDATA* pDirtyRegion)
{
  Banner_Draw();
  return m_original->Present(pSourceRect, pDestRect, hDestWindowOverride, pDirtyRegion);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateVertexDeclaration_Specific(CONST D3DVERTEXELEMENT9* pVertexElements, IDirect3DVertexDeclaration9** ppDecl, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateVertexDeclaration(pVertexElements, ppDecl);

#ifdef INTERCEPT_VERTEXSHADERS

  if (SUCCEEDED(hr))
  {
    *ppDecl = new IDirect3DVertexDeclaration9InterceptorWrapper(IID_IDirect3DVertexDeclaration9, *ppDecl, this);
    call->Push_DXResourceObjectID(((IDirect3DVertexDeclaration9InterceptorWrapper*) *ppDecl)->GetObjectID());
  }
  else
  {
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_VERTEXSHADERS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateVertexBuffer_Specific(UINT Length, DWORD Usage, DWORD FVF, D3DPOOL Pool, IDirect3DVertexBuffer9** ppVertexBuffer, HANDLE* pSharedHandle, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateVertexBuffer(Length, Usage, FVF, Pool, ppVertexBuffer, pSharedHandle);

#ifdef INTERCEPT_VERTEXBUFFERS
  
  if (SUCCEEDED(hr))
  {
    *ppVertexBuffer = new IDirect3DVertexBuffer9InterceptorWrapper(IID_IDirect3DVertexBuffer9, *ppVertexBuffer, this);
    call->Push_DXResourceObjectID(((IDirect3DVertexBuffer9InterceptorWrapper*) *ppVertexBuffer)->GetObjectID());
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXNullPointer();
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_VERTEXBUFFERS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateIndexBuffer_Specific(UINT Length, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DIndexBuffer9** ppIndexBuffer, HANDLE* pSharedHandle, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateIndexBuffer(Length, Usage, Format, Pool, ppIndexBuffer, pSharedHandle);

#ifdef INTERCEPT_INDEXBUFFERS

  if (SUCCEEDED(hr))
  {
    *ppIndexBuffer = new IDirect3DIndexBuffer9InterceptorWrapper(IID_IDirect3DIndexBuffer9, *ppIndexBuffer, this);
    call->Push_DXResourceObjectID(((IDirect3DIndexBuffer9InterceptorWrapper*) *ppIndexBuffer)->GetObjectID());
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXNullPointer();
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_INDEXBUFFERS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateTexture_Specific(UINT Width, UINT Height, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DTexture9** ppTexture, HANDLE* pSharedHandle, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateTexture(Width, Height, Levels, Usage, Format, Pool, ppTexture, pSharedHandle);

#ifdef INTERCEPT_SURFACESTEXTURES

  if (SUCCEEDED(hr))
  {
    *ppTexture = new IDirect3DTexture9InterceptorWrapper(IID_IDirect3DTexture9, *ppTexture, this);
    call->Push_DXResourceObjectID(((IDirect3DTexture9InterceptorWrapper*) *ppTexture)->GetObjectID());
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXNullPointer();
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_SURFACESTEXTURES

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateCubeTexture_Specific(UINT EdgeLength, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DCubeTexture9** ppCubeTexture, HANDLE* pSharedHandle, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateCubeTexture(EdgeLength, Levels, Usage, Format, Pool, ppCubeTexture, pSharedHandle);

#ifdef INTERCEPT_SURFACESTEXTURES

  if (SUCCEEDED(hr))
  {
    *ppCubeTexture = new IDirect3DCubeTexture9InterceptorWrapper(IID_IDirect3DCubeTexture9, *ppCubeTexture, this);
    call->Push_DXResourceObjectID(((IDirect3DCubeTexture9InterceptorWrapper*) *ppCubeTexture)->GetObjectID());
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXNullPointer();
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_SURFACESTEXTURES

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateVolumeTexture_Specific(UINT Width, UINT Height, UINT Depth, UINT Levels, DWORD Usage, D3DFORMAT Format, D3DPOOL Pool, IDirect3DVolumeTexture9** ppVolumeTexture, HANDLE* pSharedHandle, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateVolumeTexture(Width, Height, Depth, Levels, Usage, Format, Pool, ppVolumeTexture, pSharedHandle);

#ifdef INTERCEPT_SURFACESTEXTURES

  if (SUCCEEDED(hr))
  {
    *ppVolumeTexture = new IDirect3DVolumeTexture9InterceptorWrapper(IID_IDirect3DVolumeTexture9, *ppVolumeTexture, this);
    call->Push_DXResourceObjectID(((IDirect3DVolumeTexture9InterceptorWrapper*) *ppVolumeTexture)->GetObjectID());
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXNullPointer();
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_SURFACESTEXTURES

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateDepthStencilSurface_Specific(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Discard, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateDepthStencilSurface(Width, Height, Format, MultiSample, MultisampleQuality, Discard, ppSurface, pSharedHandle);

#ifdef INTERCEPT_SURFACESTEXTURES

  if (SUCCEEDED(hr))
  {
    *ppSurface = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, 0, *ppSurface, this);
    call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppSurface)->GetObjectID());
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXNullPointer();
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_SURFACESTEXTURES

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateOffscreenPlainSurface_Specific(UINT Width, UINT Height, D3DFORMAT Format, D3DPOOL Pool, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateOffscreenPlainSurface(Width, Height, Format, Pool, ppSurface, pSharedHandle);

#ifdef INTERCEPT_SURFACESTEXTURES

  if (SUCCEEDED(hr))
  {
    *ppSurface = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, 0, *ppSurface, this);
    call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppSurface)->GetObjectID());
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXNullPointer();
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_SURFACESTEXTURES

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateRenderTarget_Specific(UINT Width, UINT Height, D3DFORMAT Format, D3DMULTISAMPLE_TYPE MultiSample, DWORD MultisampleQuality, BOOL Lockable, IDirect3DSurface9** ppSurface, HANDLE* pSharedHandle, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateRenderTarget(Width, Height, Format, MultiSample, MultisampleQuality, Lockable, ppSurface, pSharedHandle);

#ifdef INTERCEPT_SURFACESTEXTURES

  if (SUCCEEDED(hr))
  {
    *ppSurface = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, 0, *ppSurface, this);
    call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppSurface)->GetObjectID());
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXNullPointer();
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_SURFACESTEXTURES

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateStateBlock_Specific(D3DSTATEBLOCKTYPE Type, IDirect3DStateBlock9** ppSB, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateStateBlock(Type, ppSB);

#ifdef INTERCEPT_STATEBLOCKS

  if (SUCCEEDED(hr))
  {
    *ppSB = new IDirect3DStateBlock9InterceptorWrapper(IID_IDirect3DStateBlock9, *ppSB, this);
    call->Push_DXResourceObjectID(((IDirect3DStateBlock9InterceptorWrapper*) *ppSB)->GetObjectID());
  }
  else
  {
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_STATEBLOCKS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreatePixelShader_Specific(CONST DWORD* pFunction, IDirect3DPixelShader9** ppShader, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreatePixelShader(pFunction, ppShader);

#ifdef INTERCEPT_PIXELSHADERS

  if (SUCCEEDED(hr))
  {
    *ppShader = new IDirect3DPixelShader9InterceptorWrapper(IID_IDirect3DPixelShader9, *ppShader, this);
    call->Push_DXResourceObjectID(((IDirect3DPixelShader9InterceptorWrapper*) *ppShader)->GetObjectID());
  }
  else
  {
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_PIXELSHADERS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateVertexShader_Specific(CONST DWORD* pFunction, IDirect3DVertexShader9** ppShader, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateVertexShader(pFunction, ppShader);

#ifdef INTERCEPT_VERTEXSHADERS

  if (SUCCEEDED(hr))
  {
    *ppShader = new IDirect3DVertexShader9InterceptorWrapper(IID_IDirect3DVertexShader9, *ppShader, this);
    call->Push_DXResourceObjectID(((IDirect3DVertexShader9InterceptorWrapper*) *ppShader)->GetObjectID());
  }
  else
  {
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_VERTEXSHADERS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetCursorProperties_Specific(UINT XHotSpot, UINT YHotSpot, IDirect3DSurface9* pCursorBitmap, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idCursorBitmap;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pCursorBitmap, &idCursorBitmap)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }
  
  call->Push_DXResourceObjectID(idCursorBitmap);

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->SetCursorProperties(XHotSpot, YHotSpot, pCursorBitmap);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetFrontBufferData_Specific(UINT iSwapChain, IDirect3DSurface9* pDestSurface, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idDestSurface;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pDestSurface, &idDestSurface)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  call->Push_DXResourceObjectID(idDestSurface);

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->GetFrontBufferData(iSwapChain, pDestSurface);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetVertexDeclaration_Specific(IDirect3DVertexDeclaration9* pDecl, DXMethodCallPtr call)
{
#ifdef INTERCEPT_VERTEXSHADERS

  DWORD idDeclaration;
  if (FAILED(IDirect3DVertexDeclaration9InterceptorWrapper::GetInternalResource(&pDecl, &idDeclaration)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DVertexDeclaration9"));
  }

  call->Push_DXResourceObjectID(idDeclaration);

  return m_original->SetVertexDeclaration(pDecl);

#else

  call->Push_DXIgnoredParameter();
  
  return m_original->SetVertexDeclaration(pDecl);

#endif // INTERCEPT_VERTEXSHADERS
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetStreamSource_Specific(UINT StreamNumber, IDirect3DVertexBuffer9* pStreamData, UINT OffsetInBytes, UINT Stride, DXMethodCallPtr call)
{
#ifdef INTERCEPT_VERTEXBUFFERS

  DWORD idStreamData;
  if (FAILED(IDirect3DVertexBuffer9InterceptorWrapper::GetInternalResource(&pStreamData, &idStreamData)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DVertexBuffer9"));
  }

  call->Push_DXResourceObjectID(idStreamData);

#endif // INTERCEPT_VERTEXBUFFERS

  return m_original->SetStreamSource(StreamNumber, pStreamData, OffsetInBytes, Stride);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetIndices_Specific(IDirect3DIndexBuffer9* pIndexData, DXMethodCallPtr call)
{
#ifdef INTERCEPT_INDEXBUFFERS

  DWORD idIndexData;
  if (FAILED(IDirect3DIndexBuffer9InterceptorWrapper::GetInternalResource(&pIndexData, &idIndexData)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DIndexBuffer9"));
  }

  call->Push_DXResourceObjectID(idIndexData);

#endif // INTERCEPT_INDEXBUFFERS

  return m_original->SetIndices(pIndexData);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetTexture_Specific(DWORD Stage, IDirect3DBaseTexture9** ppTexture, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  IDirect3DBaseTexture9* pTexture;

  HRESULT hr = m_original->GetTexture(Stage, &pTexture);

  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }

  IReferenceCounter* pRefCounter;
  DWORD pRefCounterSize = sizeof(IUnknown*);

  switch (pTexture->GetType())
  {
  case D3DRTYPE_TEXTURE:
    if (FAILED(pTexture->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
    {
      *ppTexture = new IDirect3DTexture9InterceptorWrapper(IID_IDirect3DTexture9, (IDirect3DTexture9*) pTexture, this);
      pTexture->AddRef();
      m_deviceReferenceCount++;
    }
    else
    {
      *ppTexture = (IDirect3DTexture9InterceptorWrapper*) pRefCounter->GetOwner();
      pRefCounter->Release();
    }
    call->Push_DXResourceObjectID(((IDirect3DTexture9InterceptorWrapper*) *ppTexture)->GetObjectID());
    break;
  
  case D3DRTYPE_VOLUMETEXTURE:
    if (FAILED(pTexture->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
    {
      *ppTexture = new IDirect3DVolumeTexture9InterceptorWrapper(IID_IDirect3DVolumeTexture9, (IDirect3DVolumeTexture9*) pTexture, this);
      pTexture->AddRef();
      m_deviceReferenceCount++;
    }
    else
    {
      *ppTexture = (IDirect3DVolumeTexture9InterceptorWrapper*) pRefCounter->GetOwner();
      pRefCounter->Release();
    }
    call->Push_DXResourceObjectID(((IDirect3DVolumeTexture9InterceptorWrapper*) *ppTexture)->GetObjectID());
    break;
  
  case D3DRTYPE_CUBETEXTURE:
    if (FAILED(pTexture->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
    {
      *ppTexture = new IDirect3DCubeTexture9InterceptorWrapper(IID_IDirect3DCubeTexture9, (IDirect3DCubeTexture9*) pTexture, this);
      pTexture->AddRef();
      m_deviceReferenceCount++;
    }
    else
    {
      *ppTexture = (IDirect3DCubeTexture9InterceptorWrapper*) pRefCounter->GetOwner();
      pRefCounter->Release();
    }
    call->Push_DXResourceObjectID(((IDirect3DCubeTexture9InterceptorWrapper*) *ppTexture)->GetObjectID());
    break;
  }

  return hr;

#else

  call->Push_DXIgnoredParameter();

  return m_original->GetTexture(Stage, ppTexture);

#endif // INTERCEPT_SURFACESTEXTURES
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetTexture_Specific(DWORD Stage, IDirect3DBaseTexture9* pTexture, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idTexture;
  if (FAILED(IDirect3DTexture9InterceptorWrapper::GetInternalResource(&pTexture, &idTexture)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DTexture9"));
  }

  if (!pTexture)
  {
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXResourceObjectID(idTexture);
  }

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->SetTexture(Stage, pTexture);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::ColorFill_Specific(IDirect3DSurface9* pSurface, CONST RECT* pRect, D3DCOLOR color, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idSurface;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pSurface, &idSurface)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  call->Push_DXResourceObjectID(idSurface);

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->ColorFill(pSurface, pRect, color);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetBackBuffer_Specific(UINT iSwapChain, UINT iBackBuffer, D3DBACKBUFFER_TYPE Type, IDirect3DSurface9** ppBackBuffer, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  IDirect3DSurface9* pSurface;

  HRESULT hr = m_original->GetBackBuffer(iSwapChain, iBackBuffer, Type, &pSurface);

  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }

  IReferenceCounter* pRefCounter;
  DWORD pRefCounterSize = sizeof(IUnknown*);

  if (FAILED(pSurface->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
  {
    *ppBackBuffer = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, 0, pSurface, this);
    pSurface->AddRef();
    m_deviceReferenceCount++;
  }
  else
  {
    *ppBackBuffer = (IDirect3DSurface9InterceptorWrapper*) pRefCounter->GetOwner();
    pRefCounter->Release();
  }

  call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppBackBuffer)->GetObjectID());
  
  return hr;

#else

  call->Push_DXIgnoredParameter();
  
  return m_original->GetBackBuffer(iSwapChain, iBackBuffer, Type, ppBackBuffer);

#endif // INTERCEPT_SURFACESTEXTURES
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::EndStateBlock_Specific(IDirect3DStateBlock9** ppSB, DXMethodCallPtr call)
{
  HRESULT hr = m_original->EndStateBlock(ppSB);

#ifdef INTERCEPT_STATEBLOCKS

  if (SUCCEEDED(hr))
  {
    *ppSB = new IDirect3DStateBlock9InterceptorWrapper(IID_IDirect3DStateBlock9, *ppSB, this);
    call->Push_DXResourceObjectID(((IDirect3DStateBlock9InterceptorWrapper*) *ppSB)->GetObjectID());
  }
  else
  {
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_STATEBLOCKS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateAdditionalSwapChain_Specific(D3DPRESENT_PARAMETERS* pPresentationParameters, IDirect3DSwapChain9** ppSwapChain, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateAdditionalSwapChain(pPresentationParameters, ppSwapChain);

#ifdef INTERCEPT_SWAPCHAINS

  if (SUCCEEDED(hr))
  {
    *ppSwapChain = new IDirect3DSwapChain9InterceptorWrapper(IID_IDirect3DSwapChain9, *ppSwapChain, this);
    call->Push_DXResourceObjectID(((IDirect3DSwapChain9InterceptorWrapper*) *ppSwapChain)->GetObjectID());
  }
  else
  {
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_SWAPCHAINS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetSwapChain_Specific(UINT iSwapChain, IDirect3DSwapChain9** ppSwapChain, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SWAPCHAINS

  IDirect3DSwapChain9* pChain;
  HRESULT hr = m_original->GetSwapChain(iSwapChain, &pChain);
  
  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }
  
  if (SUCCEEDED(hr))
  {
    *ppSwapChain = new IDirect3DSwapChain9InterceptorWrapper(IID_IDirect3DSwapChain9, pChain, this);
    call->Push_DXResourceObjectID(((IDirect3DSwapChain9InterceptorWrapper*) *ppSwapChain)->GetObjectID());
    pChain->AddRef();
    m_deviceReferenceCount++;
  }
  else
  {
    call->Push_DXNullPointer();
  }

  return hr;

#else
  
  call->Push_DXIgnoredParameter();
  
  return m_original->GetSwapChain(iSwapChain, ppSwapChain);

#endif // INTERCEPT_SWAPCHAINS
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetPixelShader_Specific(IDirect3DPixelShader9** ppShader, DXMethodCallPtr call)
{
  HRESULT hr = m_original->GetPixelShader(ppShader);

#ifdef INTERCEPT_PIXELSHADERS

  if (SUCCEEDED(hr))
  {
    DWORD idDeclaration;
    if (FAILED(IDirect3DPixelShader9InterceptorWrapper::GetInternalResource(ppShader, &idDeclaration)))
    {
      Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DPixelShader9"));
    }

    if (*ppShader)
    {
      call->Push_DXResourceObjectID(((IDirect3DPixelShader9InterceptorWrapper*) *ppShader)->GetObjectID());
    }
    else
    {
      call->Push_DXNullPointer();
    }
  }

#endif // INTERCEPT_PIXELSHADERS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetPixelShader_Specific(IDirect3DPixelShader9* pShader, DXMethodCallPtr call)
{
#ifdef INTERCEPT_PIXELSHADERS

  DWORD idDeclaration;
  if (FAILED(IDirect3DPixelShader9InterceptorWrapper::GetInternalResource(&pShader, &idDeclaration)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DPixelShader9"));
  }

  if (!pShader)
  {
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXResourceObjectID(idDeclaration);
  }

  return m_original->SetPixelShader(pShader);

#else

  call->Push_DXIgnoredParameter();
  
  return m_original->SetPixelShader(pShader);

#endif // INTERCEPT_PIXELSHADERS
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetVertexShader_Specific(IDirect3DVertexShader9** ppShader, DXMethodCallPtr call)
{
  HRESULT hr = m_original->GetVertexShader(ppShader);

#ifdef INTERCEPT_VERTEXSHADERS

  if (SUCCEEDED(hr))
  {
    DWORD idDeclaration;
    if (FAILED(IDirect3DVertexShader9InterceptorWrapper::GetInternalResource(ppShader, &idDeclaration)))
    {
      Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DVertexShader9"));
    }

    if (*ppShader)
    {
      call->Push_DXResourceObjectID(((IDirect3DVertexShader9InterceptorWrapper*) *ppShader)->GetObjectID());
    }
    else
    {
      call->Push_DXNullPointer();
    }
  }

#endif // INTERCEPT_VERTEXSHADERS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetVertexShader_Specific(IDirect3DVertexShader9* pShader, DXMethodCallPtr call)
{
#ifdef INTERCEPT_VERTEXSHADERS

  DWORD idDeclaration;
  if (FAILED(IDirect3DVertexShader9InterceptorWrapper::GetInternalResource(&pShader, &idDeclaration)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DVertexShader9"));
  }

  if (!pShader)
  {
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXResourceObjectID(idDeclaration);
  }

  return m_original->SetVertexShader(pShader);

#else

  call->Push_DXIgnoredParameter();
  
  return m_original->SetVertexShader(pShader);

#endif // INTERCEPT_VERTEXSHADERS
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetRenderTarget_Specific(DWORD RenderTargetIndex, IDirect3DSurface9** ppRenderTarget, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  IDirect3DSurface9* pSurface;

  HRESULT hr = m_original->GetRenderTarget(RenderTargetIndex, &pSurface);

  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }

  IReferenceCounter* pRefCounter;
  DWORD pRefCounterSize = sizeof(IUnknown*);

  if (FAILED(pSurface->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
  {
    *ppRenderTarget = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, 0, pSurface, this);
    pSurface->AddRef();
    m_deviceReferenceCount++;
  }
  else
  {
    *ppRenderTarget = (IDirect3DSurface9InterceptorWrapper*) pRefCounter->GetOwner();
    pRefCounter->Release();
  }

  call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppRenderTarget)->GetObjectID());
  
  return hr;

#else

  call->Push_DXIgnoredParameter();
  
  return m_original->GetRenderTarget(RenderTargetIndex, ppRenderTarget);

#endif // INTERCEPT_SURFACESTEXTURES
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetRenderTargetData_Specific(IDirect3DSurface9* pRenderTarget, IDirect3DSurface9* pDestSurface, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idRenderTarget;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pRenderTarget, &idRenderTarget)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  call->Push_DXResourceObjectID(idRenderTarget);

  DWORD idDestSurface;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pDestSurface, &idDestSurface)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  call->Push_DXResourceObjectID(idDestSurface);

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->GetRenderTargetData(pRenderTarget, pDestSurface);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::UpdateSurface_Specific(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestinationSurface, CONST POINT* pDestPoint, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES
  
  DWORD idSourceSurface;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pSourceSurface, &idSourceSurface)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  call->Push_DXResourceObjectID(idSourceSurface);

  call->Push_RECT(pSourceRect);

  DWORD idDestinationSurface;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pDestinationSurface, &idDestinationSurface)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  call->Push_DXResourceObjectID(idDestinationSurface);

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->UpdateSurface(pSourceSurface, pSourceRect, pDestinationSurface, pDestPoint);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::StretchRect_Specific(IDirect3DSurface9* pSourceSurface, CONST RECT* pSourceRect, IDirect3DSurface9* pDestSurface, CONST RECT* pDestRect, D3DTEXTUREFILTERTYPE Filter, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idSourceSurface;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pSourceSurface, &idSourceSurface)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  call->Push_DXResourceObjectID(idSourceSurface);
  
  call->Push_RECT(pSourceRect);
  
  DWORD idDestSurface;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pDestSurface, &idDestSurface)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  call->Push_DXResourceObjectID(idDestSurface);

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->StretchRect(pSourceSurface, pSourceRect, pDestSurface, pDestRect, Filter);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::UpdateTexture_Specific(IDirect3DBaseTexture9* pSourceTexture, IDirect3DBaseTexture9* pDestinationTexture, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idSourceTexture;
  if (FAILED(IDirect3DTexture9InterceptorWrapper::GetInternalResource(&pSourceTexture, &idSourceTexture)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DTexture9"));
  }

  call->Push_DXResourceObjectID(idSourceTexture);

  DWORD idDestinationTexture;
  if (FAILED(IDirect3DTexture9InterceptorWrapper::GetInternalResource(&pDestinationTexture, &idDestinationTexture)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DTexture9"));
  }

  call->Push_DXResourceObjectID(idDestinationTexture);

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->UpdateTexture(pSourceTexture, pDestinationTexture);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetRenderTarget_Specific(DWORD RenderTargetIndex, IDirect3DSurface9* pRenderTarget, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idRenderTarget;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pRenderTarget, &idRenderTarget)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  if (!pRenderTarget)
  {
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXResourceObjectID(idRenderTarget);
  }

#endif // INTERCEPT_SURFACESTEXTURES
  
  return m_original->SetRenderTarget(RenderTargetIndex, pRenderTarget);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::GetDepthStencilSurface_Specific(IDirect3DSurface9** ppZStencilSurface, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  IDirect3DSurface9* pSurface;

  HRESULT hr = m_original->GetDepthStencilSurface(&pSurface);

  if (FAILED(hr))
  {
    call->Push_DXNullPointer();
    return hr;
  }

  IReferenceCounter* pRefCounter;
  DWORD pRefCounterSize = sizeof(IUnknown*);

  if (FAILED(pSurface->GetPrivateData(IID_IReferenceCounter, (LPVOID) &pRefCounter, &pRefCounterSize)))
  {
    *ppZStencilSurface = new IDirect3DSurface9InterceptorWrapper(IID_IDirect3DSurface9, 0, pSurface, this);
    pSurface->AddRef();
    m_deviceReferenceCount++;
  }
  else
  {
    *ppZStencilSurface = (IDirect3DSurface9InterceptorWrapper*) pRefCounter->GetOwner();
    pRefCounter->Release();
  }

  call->Push_DXResourceObjectID(((IDirect3DSurface9InterceptorWrapper*) *ppZStencilSurface)->GetObjectID());

  return hr;

#else

  call->Push_DXIgnoredParameter();
  
  return m_original->GetDepthStencilSurface(ppZStencilSurface);

#endif // INTERCEPT_SURFACESTEXTURES
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::SetDepthStencilSurface_Specific(IDirect3DSurface9* pNewZStencil, DXMethodCallPtr call)
{
#ifdef INTERCEPT_SURFACESTEXTURES

  DWORD idNewZStencil;
  if (FAILED(IDirect3DSurface9InterceptorWrapper::GetInternalResource(&pNewZStencil, &idNewZStencil)))
  {
    Banner_DrawError(__FUNCTION__ TEXT(": Unable QueryInterface IDirect3DSurface9"));
  }

  if (!pNewZStencil)
  {
    call->Push_DXNullPointer();
  }
  else
  {
    call->Push_DXResourceObjectID(idNewZStencil);
  }

#endif // INTERCEPT_SURFACESTEXTURES

  return m_original->SetDepthStencilSurface(pNewZStencil);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DDevice9InterceptorWrapper::CreateQuery_Specific(D3DQUERYTYPE Type, IDirect3DQuery9** ppQuery, DXMethodCallPtr call)
{
  HRESULT hr = m_original->CreateQuery(Type, ppQuery);

#ifdef INTERCEPT_QUERYS

  if (SUCCEEDED(hr))
  {
    if (ppQuery)
    {
      *ppQuery = new IDirect3DQuery9InterceptorWrapper(IID_IDirect3DQuery9, *ppQuery, this);
      call->Push_DXResourceObjectID(((IDirect3DQuery9InterceptorWrapper*) *ppQuery)->GetObjectID());
    }
    else
    {
      call->Push_DXNullPointer();
    }
  }
  else
  {
    call->Push_DXNullPointer();
  }

#endif // INTERCEPT_QUERYS

  return hr;
}

////////////////////////////////////////////////////////////////////////////////

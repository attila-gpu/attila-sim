////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "IReferenceCounter.h"
#include "IDirect3DDevice9InterceptorWrapper.h"
#include "IDirect3DVertexDeclaration9InterceptorWrapper.h"

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVertexDeclaration9InterceptorWrapper::GetInternalResource(IDirect3DVertexDeclaration9** ppRes, DWORD* pIDRes)
{
  if (!*ppRes)
  {
    *pIDRes = 0;
    return S_OK;
  }
  else
  {
    IDirect3DVertexDeclaration9InterceptorWrapper* pResI;
    if (FAILED(((IDirect3DVertexDeclaration9InterceptorWrapper*) *ppRes)->QueryInterfaceSilent(IID_IDirect3DVertexDeclaration9InterceptorWrapper, (LPVOID*) &pResI)))
    {
      *pIDRes = 0;
      return E_FAIL;
    }
    else
    {
      *ppRes  = pResI->GetOriginal();
      *pIDRes = pResI->GetObjectID();
      (*ppRes)->Release();
      return S_OK;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVertexDeclaration9InterceptorWrapper::IDirect3DVertexDeclaration9InterceptorWrapper(REFIID iid, IDirect3DVertexDeclaration9* original, IDirect3DDevice9InterceptorWrapper* creator) :
DXInterceptorWrapper(iid),
m_original(original),
m_creator(creator)
{
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVertexDeclaration9InterceptorWrapper::~IDirect3DVertexDeclaration9InterceptorWrapper()
{
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVertexDeclaration9InterceptorWrapper::QueryInterfaceSilent(REFIID riid, void** ppvObj)
{
  return QueryInterface_Specific(riid, ppvObj);
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVertexDeclaration9InterceptorWrapper::QueryInterface_Specific(REFIID riid, void** ppvObj)
{
  if (IsEqualGUID(riid, IID_IUnknown))
  {
    m_original->AddRef();
    *ppvObj = (IUnknown*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DVertexDeclaration9))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DVertexDeclaration9*) this;
    return S_OK;
  }

  if (IsEqualGUID(riid, IID_IDirect3DVertexDeclaration9InterceptorWrapper))
  {
    m_original->AddRef();
    *ppvObj = (IDirect3DVertexDeclaration9InterceptorWrapper*) this;
    return S_OK;
  }

  *ppvObj = NULL;
  return E_NOINTERFACE;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVertexDeclaration9InterceptorWrapper::GetDevice_Specific(IDirect3DDevice9** ppDevice, DXMethodCallPtr call)
{
  m_original->GetDevice(ppDevice);
  *ppDevice = m_creator;

  call->Push_DXResourceObjectID(m_creator->GetObjectID());

  return D3D_OK;
}

////////////////////////////////////////////////////////////////////////////////

IDirect3DVertexDeclaration9* IDirect3DVertexDeclaration9InterceptorWrapper::GetOriginal()
{
  return m_original;
}

////////////////////////////////////////////////////////////////////////////////

HRESULT IDirect3DVertexDeclaration9InterceptorWrapper::GetDeclaration_Specific(D3DVERTEXELEMENT9* pElement, UINT* pNumElements, DXMethodCallPtr call)
{
  call->Push_UINT(*pNumElements);
  return m_original->GetDeclaration(pElement, pNumElements);
}

////////////////////////////////////////////////////////////////////////////////

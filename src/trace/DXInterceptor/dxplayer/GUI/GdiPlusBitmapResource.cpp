////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXPlayer.h"
#include "GdiPlusBitmapResource.h"

using namespace Gdiplus;

////////////////////////////////////////////////////////////////////////////////

GdiPlusBitmapResource::GdiPlusBitmapResource(void) :
m_pBitmap(NULL),
m_hBuffer(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

GdiPlusBitmapResource::GdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType, HMODULE hInst) :
m_pBitmap(NULL),
m_hBuffer(NULL)
{
  if (!Load(pName, pType, hInst))
  {
    delete this;
  }
}

////////////////////////////////////////////////////////////////////////////////

GdiPlusBitmapResource::GdiPlusBitmapResource(UINT id, LPCTSTR pType, HMODULE hInst) :
m_pBitmap(NULL),
m_hBuffer(NULL)
{
  if (!Load(id, pType, hInst))
  {
    delete this;
  }
}

////////////////////////////////////////////////////////////////////////////////

GdiPlusBitmapResource::GdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst) :
m_pBitmap(NULL),
m_hBuffer(NULL)
{
  if (!Load(id, type, hInst))
  {
    delete this;
  }
}

////////////////////////////////////////////////////////////////////////////////

GdiPlusBitmapResource::~GdiPlusBitmapResource(void)
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

void GdiPlusBitmapResource::Clear()
{
  if (m_pBitmap)
  {
    delete m_pBitmap;
    m_pBitmap = NULL;
  }
  
  if (m_hBuffer)
  {
    ::GlobalUnlock(m_hBuffer);
    ::GlobalFree(m_hBuffer);
    m_hBuffer = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool GdiPlusBitmapResource::Load(LPCTSTR pName, LPCTSTR pType, HMODULE hInst)
{
  Clear();
  
  if (!hInst)
  {
    hInst = theApp.m_hInstance;
  }
  
  HRSRC hResource = ::FindResource(hInst, pName, pType);
  if (!hResource)
  {
    return false;
  }

  DWORD imageSize = ::SizeofResource(hInst, hResource);
  if (!imageSize)
  {
    return false;
  }

  const void* pResourceData = ::LockResource(::LoadResource(hInst, hResource));
  if (!pResourceData)
  {
    return false;
  }

  m_hBuffer = ::GlobalAlloc(GMEM_MOVEABLE, imageSize);
  if (m_hBuffer)
  {
    void* pBuffer = ::GlobalLock(m_hBuffer);
    if (pBuffer)
    {
      CopyMemory(pBuffer, pResourceData, imageSize);
      IStream* pStream = NULL;
      if (::CreateStreamOnHGlobal(m_hBuffer, FALSE, &pStream) == S_OK)
      {
        m_pBitmap = Bitmap::FromStream(pStream);
        pStream->Release();
        if (m_pBitmap)
        { 
          if (m_pBitmap->GetLastStatus() == Gdiplus::Ok)
          {
            return true;
          }
          delete m_pBitmap;
          m_pBitmap = NULL;
        }
      }
      ::GlobalUnlock(m_hBuffer);
    }
    ::GlobalFree(m_hBuffer);
    m_hBuffer = NULL;
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool GdiPlusBitmapResource::Load(UINT id, LPCTSTR pType, HMODULE hInst)
{
  return Load(MAKEINTRESOURCE(id), pType, hInst);
}

////////////////////////////////////////////////////////////////////////////////

bool GdiPlusBitmapResource::Load(UINT id, UINT type, HMODULE hInst)
{
  return Load(MAKEINTRESOURCE(id), MAKEINTRESOURCE(type), hInst);
}

////////////////////////////////////////////////////////////////////////////////

GdiPlusBitmapResource::operator Bitmap * () const
{
  return m_pBitmap;
}

////////////////////////////////////////////////////////////////////////////////

Bitmap* GdiPlusBitmapResource::operator -> () const
{
  return m_pBitmap;
}

////////////////////////////////////////////////////////////////////////////////
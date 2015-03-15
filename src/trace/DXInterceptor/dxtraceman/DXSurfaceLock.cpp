////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXTypeHelper.h"
#include "DXTexture.h"
#include "DXSurfaceLock.h"

using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

DXSurfaceLock::DXSurfaceLock()
{
}

////////////////////////////////////////////////////////////////////////////////

DXSurfaceLock::~DXSurfaceLock()
{
}

////////////////////////////////////////////////////////////////////////////////

D3DLOCKED_RECT DXSurfaceLock::GetLockedRect() const
{
  return m_lockedRect;
}

////////////////////////////////////////////////////////////////////////////////

void DXSurfaceLock::SetLockedRect(D3DLOCKED_RECT* lockedRect)
{
  m_lockedRect = *lockedRect;
}

////////////////////////////////////////////////////////////////////////////////

void DXSurfaceLock::SetLock(IDirect3DSurface9* surface, UINT level, CONST RECT* rect, D3DFORMAT format)
{
  m_level = level;

  D3DSURFACE_DESC	description;
  surface->GetDesc(&description);
  SetLockBase(rect, &description, format);
}

////////////////////////////////////////////////////////////////////////////////

void DXSurfaceLock::SetLock(IDirect3DTexture9* texture, UINT level, CONST RECT* rect, D3DFORMAT format)
{
  m_level = level;

  D3DSURFACE_DESC	description;
  texture->GetLevelDesc(level, &description);
  SetLockBase(rect, &description, format);
}

////////////////////////////////////////////////////////////////////////////////

void DXSurfaceLock::SetLock(IDirect3DCubeTexture9* texture, UINT level, CONST RECT* rect, D3DFORMAT format)
{
  m_level = level;

  D3DSURFACE_DESC	description;
  texture->GetLevelDesc(level, &description);
  SetLockBase(rect, &description, format);
}

////////////////////////////////////////////////////////////////////////////////

void DXSurfaceLock::SetLockBase(CONST RECT* rect, D3DSURFACE_DESC* description, D3DFORMAT format)
{
  if (rect)
  {
    m_bytesPerLine = rect->right - rect->left;
    m_numLines = rect->bottom - rect->top;
  }
  else
  {
    m_bytesPerLine = description->Width;
    m_numLines = description->Height;
  }
  
  m_width = m_bytesPerLine;
  m_height = m_numLines;
  m_format = format;
  
  switch (m_format)
  {
  case D3DFMT_DXT1:
  case D3DFMT_DXT2:
  case D3DFMT_DXT3:
  case D3DFMT_DXT4:
  case D3DFMT_DXT5:
    m_bytesPerLine = m_lockedRect.Pitch;
    m_numLines >>= 2;
    if (!m_numLines) m_numLines = 1;
    break;
  default:
    m_bytesPerLine *= GetBitsPerPixelForFormat(m_format) >> 3;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXSurfaceLock::ReadLock(DXTexturePtr texture)
{
  if (m_lockedRect.pBits)
  {
    if (texture->GetPitch() == m_lockedRect.Pitch)
    {
      unsigned int readedBytes = m_numLines * m_lockedRect.Pitch;
      return texture->TextureData_Read((char*) m_lockedRect.pBits, &readedBytes);
    }
    else
    {
      unsigned int pitch_add = texture->GetPitch()-m_bytesPerLine;
      char* lockedRect = (char*) m_lockedRect.pBits;
      for (unsigned int i=0; i < m_numLines; i++)
      {
        unsigned int readedBytes = m_bytesPerLine;
        if (!texture->TextureData_Read(lockedRect, &readedBytes))
        {
          return false;
        }
        lockedRect += m_lockedRect.Pitch;
        texture->TextureData_SeekRead(pitch_add, true);
      }
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXSurfaceLock::WriteLock(DXTexturePtr texture)
{
  if (m_lockedRect.pBits)
  {
    texture->SetType(DXTypeHelper::TT_DXTexture);
    texture->SetDimensions(m_width, m_height);
    texture->SetFormat(m_format);
    texture->SetLevel(m_level);
    texture->SetPitch(m_lockedRect.Pitch);
    
    return texture->TextureData_Write((char*) m_lockedRect.pBits, m_numLines * m_lockedRect.Pitch);
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXSurfaceLock::GetBitsPerPixelForFormat(D3DFORMAT format)
{
  switch (format)
  {
  // Unsigned Formats
  case D3DFMT_R8G8B8: return 24;
  case D3DFMT_A8R8G8B8: return 32;
  case D3DFMT_X8R8G8B8: return 32;
  case D3DFMT_R5G6B5: return 16;
  case D3DFMT_X1R5G5B5: return 16;
  case D3DFMT_A1R5G5B5: return 16;
  case D3DFMT_A4R4G4B4: return 16;
  case D3DFMT_R3G3B2: return 8;
  case D3DFMT_A8: return 8;
  case D3DFMT_A8R3G3B2: return 16;
  case D3DFMT_X4R4G4B4: return 16;
  case D3DFMT_A2B10G10R10: return 32;
  case D3DFMT_A8B8G8R8: return 32;
  case D3DFMT_X8B8G8R8: return 32;
  case D3DFMT_G16R16: return 32;
  case D3DFMT_A2R10G10B10: return 32;
  case D3DFMT_A16B16G16R16: return 64;
  case D3DFMT_A8P8: return 16;
  case D3DFMT_P8: return 8;
  case D3DFMT_L8: return 8;
  case D3DFMT_L16: return 16;
  case D3DFMT_A8L8: return 16;
  case D3DFMT_A4L4: return 8;
  
  // DXTn Compressed Texture Formats
  case D3DFMT_DXT1: return 4;
  case D3DFMT_DXT2: return 8;
  case D3DFMT_DXT3: return 8;
  case D3DFMT_DXT4: return 8;
  case D3DFMT_DXT5: return 8;
  
  // Buffer Formats
  case D3DFMT_D16_LOCKABLE: return 16;
  case D3DFMT_D32: return 32;
  case D3DFMT_D15S1: return 16;
  case D3DFMT_D24S8: return 32;
  case D3DFMT_D24X8: return 32;
  case D3DFMT_D24X4S4: return 32;
  case D3DFMT_D32F_LOCKABLE: return 32;
  case D3DFMT_D24FS8: return 32;
  case D3DFMT_D16: return 16;
  case D3DFMT_VERTEXDATA: return 0;
  case D3DFMT_INDEX16: return 16;
  case D3DFMT_INDEX32: return 32;

  // Floating-Point Formats
  case D3DFMT_R16F: return 16;
  case D3DFMT_G16R16F: return 32;
  case D3DFMT_A16B16G16R16F: return 64;

  // FOURCC Formats
  case D3DFMT_MULTI2_ARGB8: return 0;
  case D3DFMT_G8R8_G8B8: return 16;
  case D3DFMT_R8G8_B8G8: return 16;
  case D3DFMT_UYVY: return 8;
  case D3DFMT_YUY2: return 8;

  // IEEE Formats
  case D3DFMT_R32F: return 32;
  case D3DFMT_G32R32F: return 64;
  case D3DFMT_A32B32G32R32F: return 128;

  // Mixed Formats
  case D3DFMT_L6V5U5: return 16;
  case D3DFMT_X8L8V8U8: return 32;
  case D3DFMT_A2W10V10U10: return 32;

  // Signed Formats
  case D3DFMT_V8U8: return 16;
  case D3DFMT_Q8W8V8U8: return 32;
  case D3DFMT_V16U16: return 32;
  case D3DFMT_Q16W16V16U16: return 64;
  case D3DFMT_CxV8U8 : return 16;

  // Other
  default:
  case D3DFMT_UNKNOWN: return 0;
  }

  return 0;
};

////////////////////////////////////////////////////////////////////////////////

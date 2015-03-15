////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXTypeHelper.h"
#include "DXTexture.h"
#include "DXVolumeLock.h"

using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

DXVolumeLock::DXVolumeLock()
{
}

////////////////////////////////////////////////////////////////////////////////

DXVolumeLock::~DXVolumeLock()
{
}

////////////////////////////////////////////////////////////////////////////////

D3DLOCKED_BOX DXVolumeLock::GetLockedBox() const
{
  return m_lockedBox;
}

////////////////////////////////////////////////////////////////////////////////

void DXVolumeLock::SetLockedBox(D3DLOCKED_BOX* lockedBox)
{
  m_lockedBox = *lockedBox;
}

////////////////////////////////////////////////////////////////////////////////

void DXVolumeLock::SetLock(IDirect3DVolume9* volume, UINT level, CONST D3DBOX* box, D3DFORMAT format)
{
  m_level = level;

  D3DVOLUME_DESC description;
  volume->GetDesc(&description);
  SetLockBase(box, &description, format);
}

////////////////////////////////////////////////////////////////////////////////

void DXVolumeLock::SetLock(IDirect3DVolumeTexture9* volumeTexture, UINT level, CONST D3DBOX* box, D3DFORMAT format)
{
  m_level = level;

  D3DVOLUME_DESC	description;
  volumeTexture->GetLevelDesc(level, &description);
  SetLockBase(box, &description, format);
}

////////////////////////////////////////////////////////////////////////////////

void DXVolumeLock::SetLockBase(CONST D3DBOX* box, D3DVOLUME_DESC* description, D3DFORMAT format)
{
  if (box)
  {
    m_bytesPerLine = box->Right - box->Left;
    m_numLines = box->Bottom - box->Top;
    m_numSurfaces = box->Back - box->Front;
  }
  else
  {
    m_bytesPerLine = description->Width;
    m_numLines = description->Height;
    m_numSurfaces = description->Depth;
  }
  
  m_width = m_bytesPerLine;
  m_height = m_numLines;
  m_depth = m_numSurfaces;
  m_format = format;
  
  switch (m_format)
  {
  case D3DFMT_DXT1:
  case D3DFMT_DXT2:
  case D3DFMT_DXT3:
  case D3DFMT_DXT4:
  case D3DFMT_DXT5:
    m_bytesPerLine = m_lockedBox.RowPitch;
    m_numLines >>= 2;
    if (!m_numLines) m_numLines = 1;
    break;
  default:
    m_bytesPerLine *= GetBitsPerPixelForFormat(m_format) >> 3;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXVolumeLock::ReadLock(DXTexturePtr volumeTexture)
{
  if (m_lockedBox.pBits)
  {
    if (volumeTexture->GetPitch() == m_lockedBox.RowPitch && volumeTexture->GetSlicePitch() == m_lockedBox.SlicePitch)
    {
      unsigned int readedBytes = m_numSurfaces * m_lockedBox.SlicePitch;
      return volumeTexture->TextureData_Read((char*) m_lockedBox.pBits, &readedBytes);
    }
    else
    {
      unsigned int rowPitch = volumeTexture->GetPitch() - m_bytesPerLine;
      unsigned int levPitch = volumeTexture->GetSlicePitch() - (volumeTexture->GetPitch() * m_numLines);
      unsigned int depthPitch = m_lockedBox.SlicePitch - (m_lockedBox.RowPitch * m_numLines);
      char* lockedBox = (char*) m_lockedBox.pBits;
      for (unsigned int s=0; s < m_numSurfaces; s++)
      {
        for (unsigned int i=0; i < m_numLines; i++)
        {
          unsigned int readedBytes = m_bytesPerLine;
          if (!volumeTexture->TextureData_Read(lockedBox, &readedBytes))
          {
            return false;
          }
          lockedBox += m_lockedBox.RowPitch;
          volumeTexture->TextureData_SeekRead(rowPitch, true);
        }
        lockedBox += depthPitch;
        volumeTexture->TextureData_SeekRead(levPitch, true);
      }
      return true;
    }
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXVolumeLock::WriteLock(DXTexturePtr volumeTexture)
{
  if (m_lockedBox.pBits)
  {
    volumeTexture->SetType(DXTypeHelper::TT_DXVolumeTexture);
    volumeTexture->SetDimensions(m_width, m_height, m_depth);
    volumeTexture->SetFormat(m_format);
    volumeTexture->SetLevel(m_level);
    volumeTexture->SetPitch(m_lockedBox.RowPitch);
    volumeTexture->SetSlicePitch(m_lockedBox.SlicePitch);
    
    return volumeTexture->TextureData_Write((char*) m_lockedBox.pBits, m_numSurfaces * m_lockedBox.SlicePitch);
  }

  return false;
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXVolumeLock::GetBitsPerPixelForFormat(D3DFORMAT format)
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

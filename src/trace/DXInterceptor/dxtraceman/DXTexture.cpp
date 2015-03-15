////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CRC32.h"
#include "DXTypeHelper.h"
#include "DXFileManagerBase.h"
#include "DXTexture.h"

using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

DXTexture::DXTexture() :
m_mustFreeData(false),
m_data(NULL)
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

DXTexture::~DXTexture()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::Clear()
{
  memset(&m_information, 0, sizeof(DXTextureInformation));
  m_information.Type = DXTypeHelper::TT_DummyType;

  m_size = 0;
  m_crc32 = 0x00000000;
  m_readPosition = 0;

  if (m_mustFreeData && m_data)
  {
    delete[] m_data;
    m_data = NULL;
  }

  m_mustFreeData = false;
}

////////////////////////////////////////////////////////////////////////////////

DXTypeHelper::DXTypeType DXTexture::GetType() const
{
  return m_information.Type;
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::SetType(DXTypeHelper::DXTypeType type)
{
  m_information.Type = type;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTexture::GetSize() const
{
  return m_size;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTexture::GetSerializedSize() const
{
  return sizeof(DXTextureInformation) + m_size;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTexture::GetCRC32()
{
  if (!m_crc32 && m_size)
  {
    if (m_mustFreeData)
    {
      m_crc32 = CRC32::Calculate(m_data, sizeof(DXTextureInformation) + m_size);
    }
    else
    {
      m_crc32 = CRC32::Calculate((const char*) &m_information, sizeof(DXTextureInformation));
      m_crc32 = CRC32::Calculate(m_data, m_size, m_crc32);
    }    
  }
  return m_crc32;
}

////////////////////////////////////////////////////////////////////////////////

UINT DXTexture::GetWidth() const
{
  return m_information.Width;
}

////////////////////////////////////////////////////////////////////////////////

UINT DXTexture::GetHeight() const
{
  return m_information.Height;
}

////////////////////////////////////////////////////////////////////////////////

UINT DXTexture::GetDepth() const
{
  return m_information.Depth;
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::SetDimensions(UINT width, UINT height)
{
  m_information.Width  = width;
  m_information.Height = height;
  m_information.Depth  = 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::SetDimensions(UINT width, UINT height, UINT depth)
{
  m_information.Width  = width;
  m_information.Height = height;
  m_information.Depth  = depth;
}

////////////////////////////////////////////////////////////////////////////////

D3DFORMAT DXTexture::GetFormat() const
{
  return m_information.Format;
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::SetFormat(D3DFORMAT format)
{
  m_information.Format = format;
}

////////////////////////////////////////////////////////////////////////////////

UINT DXTexture::GetLevel() const
{
  return m_information.Level;
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::SetLevel(UINT level)
{
  m_information.Level = level;
}

////////////////////////////////////////////////////////////////////////////////

UINT DXTexture::GetPitch() const
{
  return m_information.Pitch;
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::SetPitch(UINT pitch)
{
  m_information.Pitch = pitch;
}

////////////////////////////////////////////////////////////////////////////////

UINT DXTexture::GetSlicePitch() const
{
  return m_information.SlicePitch;
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::SetSlicePitch(UINT pitch)
{
  m_information.SlicePitch = pitch;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTexture::SerializeToFile(DXFileManagerBase* file)
{
#ifdef _DEBUG
  if (m_information.Type == DXTypeHelper::TT_DummyType)
  {
    return false;
  }
#endif // ifdef _DEBUG

  if (!file->Write((char*) &m_information, sizeof(DXTextureInformation)))
  {
    return false;
  }

  return file->Write((m_mustFreeData ? &m_data[sizeof(DXTextureInformation)] : m_data), m_size);
}

////////////////////////////////////////////////////////////////////////////////

bool DXTexture::DeserializeFromFile(DXFileManagerBase* file, unsigned int size)
{
#ifdef _DEBUG
  if (size <= sizeof(DXTextureInformation))
  {
    return false;
  }
#endif // ifdef _DEBUG

  Clear();

  m_data = new char[size];

  if (!m_data)
  {
    return false;
  }

  m_mustFreeData = true;
  m_size = size - sizeof(DXTextureInformation);

  if (file->Read(m_data, size))
  {
    memcpy(&m_information, m_data, sizeof(DXTextureInformation));
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXTexture::TextureData_Read(char* buffer, unsigned int* size)
{
#ifdef _DEBUG
  if (*size > (m_size - m_readPosition))
  {
    *size = m_size - m_readPosition;
  }
#endif // ifdef _DEBUG

  if (*size)
  {
    memcpy(buffer, &m_data[sizeof(DXTextureInformation) + m_readPosition], *size);
    m_readPosition += *size;
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXTexture::TextureData_Write(char* data, unsigned int size)
{
  m_mustFreeData = false;
  m_data = data;
  m_size = size;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXTexture::TextureData_SeekRead(unsigned int position, bool fromCurrentPosition)
{
  if (fromCurrentPosition)
    m_readPosition += position;
  else
    m_readPosition = position;

  if (m_readPosition > m_size)
  {
    m_readPosition = m_size;
  }
}

////////////////////////////////////////////////////////////////////////////////

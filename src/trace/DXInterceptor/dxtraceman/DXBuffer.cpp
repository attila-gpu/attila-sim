////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "CRC32.h"
#include "MD5.h"
#include "DXTypeHelper.h"
#include "DXFileManagerBase.h"
#include "DXBuffer.h"

using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

#define POP_BUFFER_MACRO(type) \
  bool DXBuffer::Pop_##type(char** ptr) \
  { \
    *ptr = &m_data[sizeof(char)]; \
    return true; \
  }

////////////////////////////////////////////////////////////////////////////////

#define PUSH_BUFFER_MACRO(type) \
  bool DXBuffer::Push_##type(const type##* value) \
  { \
    return BufferWrite((const char*) value, sizeof(##type), DXTypeHelper::TT_##type); \
  }

////////////////////////////////////////////////////////////////////////////////

#define BUFFER_MACRO(type) \
  POP_BUFFER_MACRO(type) \
  PUSH_BUFFER_MACRO(type)

////////////////////////////////////////////////////////////////////////////////

DXBuffer::DXBuffer() :
m_mustFreeData(false),
m_data(NULL)
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

DXBuffer::~DXBuffer()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

void DXBuffer::Clear()
{
  m_type = DXTypeHelper::TT_DummyType;
  m_size = 0;

  if (m_mustFreeData && m_data)
  {
    delete[] m_data;
    m_data = NULL;
  }

  m_mustFreeData = false;
}

////////////////////////////////////////////////////////////////////////////////

DXTypeHelper::DXTypeType DXBuffer::GetType() const
{
  return m_type;
}

////////////////////////////////////////////////////////////////////////////////

void DXBuffer::SetType(DXTypeHelper::DXTypeType type)
{
  m_type = type;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXBuffer::GetSize() const
{
  return m_size;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXBuffer::GetSerializedSize() const
{
  return sizeof(char) + m_size;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXBuffer::GetCRC32()
{
  unsigned int crc32 = 0x00000000;
  if (m_mustFreeData)
  {
    crc32 = CRC32::Calculate(m_data, sizeof(char) + m_size);
  }
  else
  {
    crc32 = CRC32::Calculate((const char*) &m_type, sizeof(char));
    crc32 = CRC32::Calculate(m_data, m_size, crc32);
  }
  return crc32;
}

////////////////////////////////////////////////////////////////////////////////

void DXBuffer::GetMD5(unsigned char* md5_hash)
{
  MD5::md5_t md5;
  
  if (m_mustFreeData)
  {
    MD5::Calculate((const unsigned char*) m_data, sizeof(char) + m_size, md5.value);
  }
  else
  {
    MD5::Init();
    MD5::Update((const unsigned char*) &m_type, sizeof(char));
    MD5::Update((const unsigned char*) m_data, m_size);
    MD5::Finalize();
    MD5::GetMD5(md5.value);
  }
  
  memcpy(md5_hash, md5.value, sizeof(md5.value));
}

////////////////////////////////////////////////////////////////////////////////

bool DXBuffer::SerializeToFile(DXFileManagerBase* file)
{
#ifdef _DEBUG
  if (m_type == DXTypeHelper::TT_DummyType)
  {
    return false;
  }
#endif // ifdef _DEBUG

  if (!file->Write((char*) &m_type, sizeof(char)))
  {
    return false;
  }

  return file->Write((m_mustFreeData ? &m_data[sizeof(char)] : m_data), m_size);
}

////////////////////////////////////////////////////////////////////////////////

bool DXBuffer::DeserializeFromFile(DXFileManagerBase* file, unsigned int size)
{
#ifdef _DEBUG
  if (size <= sizeof(char))
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
  m_size = size - sizeof(char);

  if (file->Read(m_data, size))
  {
    memcpy(&m_type, m_data, sizeof(char));
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXBuffer::SerializeToBuffer(char* buffer, unsigned int* size)
{
#ifdef _DEBUG
  if (m_type == DXTypeHelper::TT_DummyType)
  {
    return false;
  }

  if (!buffer || *size < GetSerializedSize())
  {
    return false;
  }
#endif // ifdef _DEBUG
  
  if (m_mustFreeData)
  {
    memcpy(buffer, m_data, GetSerializedSize());
  }
  else
  {
    memcpy(buffer, (const char*) m_type, sizeof(char));
    memcpy(&buffer[sizeof(char)], m_data, m_size);
  }
  *size = GetSerializedSize();
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXBuffer::DeserializeFromBuffer(const char* buffer, unsigned int size)
{
#ifdef _DEBUG
  if (size <= sizeof(DXTypeHelper::DXTypeType))
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
  m_size = size - sizeof(char);

  memcpy(m_data, buffer, size);
  memcpy(&m_type, m_data, sizeof(char));
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXBuffer::BufferRead(char* buffer, unsigned int& size)
{
  if (size)
  {
    memcpy(buffer, (const char*) &m_data[sizeof(char)], m_size);
    size = m_size;
    return true;
  }
  else
  {
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXBuffer::BufferWrite(const char* buffer, unsigned int size, DXTypeHelper::DXTypeType type)
{  
  Clear();
  m_type = type;
  m_data = (char*) buffer;
  m_size = size;
  m_mustFreeData = false;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(DXRawData);

bool DXBuffer::Push_DXRawData(const void* buffer, unsigned int size)
{
  return BufferWrite((const char*) buffer, size, DXTypeHelper::TT_DXRawData);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXBuffer::CalculateSize_ARR_D3DVERTEXELEMENT9(const D3DVERTEXELEMENT9* value)
{
  if (!value)
  {
    return 0;
  }

  D3DVERTEXELEMENT9 end = D3DDECL_END();

  unsigned int size = 0;
  while (memcmp(&value[size], &end, sizeof(end)))
  {
    size++; 
  }

  return sizeof(D3DVERTEXELEMENT9)*(size+1);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_D3DVERTEXELEMENT9);

bool DXBuffer::Push_ARR_D3DVERTEXELEMENT9(const D3DVERTEXELEMENT9* value)
{
  return BufferWrite((const char*) value, CalculateSize_ARR_D3DVERTEXELEMENT9(value), DXTypeHelper::TT_ARR_D3DVERTEXELEMENT9);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXBuffer::CalculateSize_ARR_RGNDATA(const RGNDATA* value)
{
  if (!value)
  {
    return 0;
  }

  RGNDATAHEADER header;
  memcpy(&header, (const char*) value, sizeof(RGNDATAHEADER));
  
  return sizeof(RGNDATAHEADER) + header.nRgnSize;
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_RGNDATA);

bool DXBuffer::Push_ARR_RGNDATA(const RGNDATA* value)
{
  return BufferWrite((const char*) value, CalculateSize_ARR_RGNDATA(value), DXTypeHelper::TT_ARR_RGNDATA);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_D3DRECT);

bool DXBuffer::Push_ARR_D3DRECT(const D3DRECT* value, UINT count)
{
  return BufferWrite((const char*) value, count*sizeof(D3DRECT), DXTypeHelper::TT_ARR_D3DRECT);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXBuffer::CalculateSize_ARR_DRAWPRIMITIVEUP(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, UINT vertexStride)
{
  unsigned int numVertex = 0;
  
  switch (primitiveType)
  {
  case D3DPT_POINTLIST:
    numVertex = primitiveCount;
    break;

  case D3DPT_LINELIST:
    numVertex = primitiveCount*2;
    break;

  case D3DPT_LINESTRIP:
    numVertex = primitiveCount*2 + 1;
    break;

  case D3DPT_TRIANGLELIST:
    numVertex = primitiveCount*3;
    break;

  case D3DPT_TRIANGLESTRIP:
    numVertex = primitiveCount + 2;
    break;

  case D3DPT_TRIANGLEFAN:
    numVertex = primitiveCount + 2;
    break;
  }

  return numVertex * vertexStride;
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_DRAWPRIMITIVEUP);

bool DXBuffer::Push_ARR_DRAWPRIMITIVEUP(const void* value, D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, UINT vertexStride)
{
  return BufferWrite((const char*) value, CalculateSize_ARR_DRAWPRIMITIVEUP(primitiveType, primitiveCount, vertexStride), DXTypeHelper::TT_ARR_DRAWPRIMITIVEUP);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_DRAWINDEXEDPRIMITIVEUPINDICES);

bool DXBuffer::Push_ARR_DRAWINDEXEDPRIMITIVEUPINDICES(const void* value, D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, D3DFORMAT indexDataFormat)
{
  return BufferWrite((const char*) value, CalculateSize_ARR_DRAWPRIMITIVEUP(primitiveType, primitiveCount, (indexDataFormat == D3DFMT_INDEX16 ? 2 : 4)), DXTypeHelper::TT_ARR_DRAWINDEXEDPRIMITIVEUPINDICES);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_DRAWINDEXEDPRIMITIVEUPVERTICES);

bool DXBuffer::Push_ARR_DRAWINDEXEDPRIMITIVEUPVERTICES(const void* value, UINT minVertexIndex, UINT numVertices, UINT vertexStreamZeroStride)
{
  return BufferWrite((const char*) value, (minVertexIndex+numVertices)*vertexStreamZeroStride, DXTypeHelper::TT_ARR_DRAWINDEXEDPRIMITIVEUPVERTICES);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXBuffer::CalculateSize_ARR_SHADERFUNCTIONTOKEN(const DWORD* value)
{
  if (!value)
  {
    return 0;
  }

  DWORD length = 1;
  while (*value != 0x0000FFFF)
  {
    if ((*value & D3DSI_OPCODE_MASK) == D3DSIO_COMMENT)
    {
      DWORD commentLength;
      commentLength = (*value & D3DSI_COMMENTSIZE_MASK) >> D3DSI_COMMENTSIZE_SHIFT;
      length += commentLength;
      value += commentLength;
    }
    
    length++;
    value++;
  }

  return length*4;
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_SHADERFUNCTIONTOKEN);

bool DXBuffer::Push_ARR_SHADERFUNCTIONTOKEN(const DWORD* value)
{
  return BufferWrite((const char*) value, CalculateSize_ARR_SHADERFUNCTIONTOKEN(value), DXTypeHelper::TT_ARR_SHADERFUNCTIONTOKEN);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_SHADERCONSTANTBOOL);

bool DXBuffer::Push_ARR_SHADERCONSTANTBOOL(const BOOL* value, UINT count)
{
  return BufferWrite((const char*) value, count*sizeof(BOOL), DXTypeHelper::TT_ARR_SHADERCONSTANTBOOL);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_SHADERCONSTANTFLOAT);

bool DXBuffer::Push_ARR_SHADERCONSTANTFLOAT(const float* value, UINT count)
{
  return BufferWrite((const char*) value, count*4*sizeof(float), DXTypeHelper::TT_ARR_SHADERCONSTANTFLOAT);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_SHADERCONSTANTINT);

bool DXBuffer::Push_ARR_SHADERCONSTANTINT(const int* value, UINT count)
{
  return BufferWrite((const char*) value, count*4*sizeof(int), DXTypeHelper::TT_ARR_SHADERCONSTANTINT);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_SETCLIPPLANE);

bool DXBuffer::Push_ARR_SETCLIPPLANE(const float* value)
{
  return BufferWrite((const char*) value, 4*sizeof(float), DXTypeHelper::TT_ARR_SETCLIPPLANE);
}

////////////////////////////////////////////////////////////////////////////////

POP_BUFFER_MACRO(ARR_PALETTEENTRY);

bool DXBuffer::Push_ARR_PALETTEENTRY(const PALETTEENTRY* value)
{
  return BufferWrite((const char*) value, 256*sizeof(PALETTEENTRY), DXTypeHelper::TT_ARR_PALETTEENTRY);
}

////////////////////////////////////////////////////////////////////////////////

BUFFER_MACRO(D3DPRESENT_PARAMETERS);
BUFFER_MACRO(D3DLIGHT9);
BUFFER_MACRO(D3DMATRIX);
BUFFER_MACRO(D3DMATERIAL9);
BUFFER_MACRO(D3DVIEWPORT9);
BUFFER_MACRO(D3DGAMMARAMP);

////////////////////////////////////////////////////////////////////////////////

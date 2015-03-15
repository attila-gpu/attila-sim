////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "arraystream.h"
#include "CRC32.h"
#include "DXBasicTypes.h"
#include "DXFileManagerW32.h"
#include "DXFileManagerSTL.h"
#include "DXTypeHelper.h"
#include "DXMethodCallHelper.h"
#include "DXBuffer.h"
#include "DXTexture.h"
#include "DXStatistic.h"
#include "DXMethodCall.h"
#include "DXTraceManager.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
#define POP_PARAM_MACRO(type) \
  bool DXMethodCall::Pop_##type(##type* value) \
  { \
    if \
    ( \
      !m_paramsCount || \
      m_paramsCount <= m_paramsCurrent || \
      m_params[m_paramsCurrent].Type != DXTypeHelper::TT_##type \
    ) \
    { \
      return false; \
    } \
    if (ParamsBufferRead((char*) value, m_params[m_paramsCurrent].Size)) \
    { \
      m_paramsCurrent++; \
      return true; \
    } \
    else \
    { \
      return false; \
    } \
  }
#else
#define POP_PARAM_MACRO(type) \
  bool DXMethodCall::Pop_##type(##type* value) \
  { \
    ParamsBufferRead((char*) value, m_params[m_paramsCurrent++].Size); \
    return true; \
  }
#endif // ifdef DEBUG

////////////////////////////////////////////////////////////////////////////////

#define PUSH_PARAM_MACRO_BY_POINTER(type) \
  bool DXMethodCall::Push_##type(const type## * value) \
  { \
    if (value == NULL) \
    { \
      m_params[m_paramsCount].Type = DXTypeHelper::TT_DXNullPointer; \
      m_params[m_paramsCount].Position = m_positionWrite+1; \
      m_params[m_paramsCount].Size = sizeof(DXNullPointer); \
      if (ParamsBufferWrite((char*) &value, (BYTE) DXTypeHelper::TT_DXNullPointer, m_params[m_paramsCount].Size)) \
      { \
        m_paramsCount++; \
        return true; \
      } \
      else \
      { \
        return false; \
      } \
    } \
    else \
    { \
      m_params[m_paramsCount].Type = DXTypeHelper::TT_##type; \
      m_params[m_paramsCount].Position = m_positionWrite+1; \
      m_params[m_paramsCount].Size = sizeof(##type); \
      if (ParamsBufferWrite((char*) value, (BYTE) DXTypeHelper::TT_##type, m_params[m_paramsCount].Size)) \
      { \
        m_paramsCount++; \
        return true; \
      } \
      else \
      { \
        return false; \
      } \
    } \
  }

////////////////////////////////////////////////////////////////////////////////

#define PUSH_PARAM_MACRO_BY_VALUE(type) \
  bool DXMethodCall::Push_##type(##type value) \
  { \
    m_params[m_paramsCount].Type = DXTypeHelper::TT_##type; \
    m_params[m_paramsCount].Position = m_positionWrite+1; \
    m_params[m_paramsCount].Size = sizeof(##type); \
    if (ParamsBufferWrite((char*) &value, (BYTE) DXTypeHelper::TT_##type, m_params[m_paramsCount].Size)) \
    { \
      m_paramsCount++; \
      return true; \
    } \
    else \
    { \
      return false; \
    } \
  }

////////////////////////////////////////////////////////////////////////////////

#define PARAM_MACRO_BY_POINTER(type) \
  POP_PARAM_MACRO(type) \
  PUSH_PARAM_MACRO_BY_POINTER(type)

////////////////////////////////////////////////////////////////////////////////

#define PARAM_MACRO_BY_VALUE(type) \
  POP_PARAM_MACRO(type) \
  PUSH_PARAM_MACRO_BY_VALUE(type)

////////////////////////////////////////////////////////////////////////////////

DXMethodCall::DXMethodCall()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

DXMethodCall::DXMethodCall(DXMethodCallHelper::DXMethodCallToken token)
{
  Clear();
  SetToken(token);
}

////////////////////////////////////////////////////////////////////////////////

DXMethodCall::DXMethodCall(DXMethodCallHelper::DXMethodCallToken token, DWORD creatorID)
{
  Clear();
  SetToken(token);
  SetCreatorID(creatorID);
}

////////////////////////////////////////////////////////////////////////////////

DXMethodCall::~DXMethodCall()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

void DXMethodCall::Clear()
{
  m_positionRead = 6;
  m_positionWrite = 6;
  m_crc32 = 0;
  m_paramsCount = 0;
  m_paramsCurrent = 0;
  memset(m_buffer, 0, m_positionWrite);
}

////////////////////////////////////////////////////////////////////////////////

DXMethodCallHelper::DXMethodCallToken DXMethodCall::GetToken() const
{
  return (DXMethodCallHelper::DXMethodCallToken) (*((WORD*) m_buffer) >> 6);
}

////////////////////////////////////////////////////////////////////////////////

void DXMethodCall::SetToken(DXMethodCallHelper::DXMethodCallToken token)
{
  *((WORD*) m_buffer) &= 0x003F;
  *((WORD*) m_buffer) |= ((WORD) token << 6);
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXMethodCall::GetNumParams() const
{
  return (unsigned int) (*((WORD*) m_buffer) >> 1) & 0x001F;
}

////////////////////////////////////////////////////////////////////////////////

void DXMethodCall::SetNumParams()
{
  *((WORD*) m_buffer) &= 0xFFC1;
  *((WORD*) m_buffer) |= ((WORD) (m_paramsCount << 1) & 0x003E);
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::GetIsSavedReturnValue() const
{
  return ((*(WORD*) m_buffer) & 0x0001) != 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXMethodCall::SetIsSavedReturnValue(bool isSaved)
{
  *((WORD*) m_buffer) &= 0xFFFE;
  *((WORD*) m_buffer) |= ((WORD) (isSaved ? 1 : 0) & 0x0001);
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXMethodCall::GetCreatorID() const
{
  return *((DWORD*) &m_buffer[sizeof(WORD)]);
}

////////////////////////////////////////////////////////////////////////////////

void DXMethodCall::SetCreatorID(DWORD creatorID)
{
  *((DWORD*) &m_buffer[sizeof(WORD)]) = creatorID;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXMethodCall::GetSize() const
{
  return m_positionWrite - m_paramsCount - 6;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXMethodCall::GetSerializedSize() const
{
  return m_positionWrite;
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXMethodCall::GetCRC32()
{
  if (!m_crc32 && m_positionWrite)
  {
    m_crc32 = CRC32::Calculate(m_buffer, m_positionWrite);
  }
  return m_crc32;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::ParamsBufferRead(char* buffer, BYTE size)
{
#ifdef _DEBUG
  if (!size)
  {
    return false;
  }
#endif // ifdef _DEBUG

#ifdef _DEBUG
  if ((m_positionWrite - m_positionRead) >= (unsigned int) (size + 1))
  {
#endif // ifdef _DEBUG

    memcpy(buffer, &m_buffer[++m_positionRead], size);
    m_positionRead += size;
    return true;

#ifdef _DEBUG
  }
  else
  {
    return false;
  }
#endif // ifdef _DEBUG
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::ParamsBufferWrite(const char* buffer, BYTE type, BYTE size)
{
#ifdef _DEBUG
  if (!size || ((m_positionWrite+size+1) > DXCALL_MAX_SIZE))
  {
    return false;
  }
#endif // ifdef _DEBUG

  memcpy(&m_buffer[m_positionWrite++], &type, sizeof(BYTE)); // Write the param value type
  memcpy(&m_buffer[m_positionWrite], buffer, size); // Write the param value
  m_positionWrite += size;
 
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::SerializeToBuffer(char** buffer, unsigned int* size)
{
  SetNumParams();
  *buffer = m_buffer;
  *size = m_positionWrite;
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::SerializeToString(string& cadena, const DXTypeHelper::StringConversionOptions* options)
{
  ostringstream out(ios::out);
  bool result = SerializeToString(out, options);
  cadena = out.str();
  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::SerializeToString(ostream& stream, const DXTypeHelper::StringConversionOptions* options)
{
  char methodCallBuffer[256];
  arraystream methodCallArrayStream(methodCallBuffer, sizeof(methodCallBuffer));
  ostream methodCallStr(&methodCallArrayStream);
    
  methodCallStr << DXMethodCallHelper::GetClassName(GetToken()) << "[" << GetCreatorID() << "]." << DXMethodCallHelper::GetMethodName(GetToken()) << "(";
  
  if (m_paramsCount)
  {
    unsigned int j = m_paramsCount;
    if (GetIsSavedReturnValue()) j--;
    for (unsigned int i=0; i < j; ++i)
    {
      DXTypeHelper::ToString(methodCallStr, &m_buffer[m_params[i].Position], m_params[i].Type, options);
      if (i+1 < j)
      {
        methodCallStr << ", ";
      }
    }

    if (GetIsSavedReturnValue())
    {
      methodCallStr << ") = ";
      DXTypeHelper::ToString(methodCallStr, &m_buffer[m_params[j].Position], m_params[j].Type, options);
    }
    else
    {
      methodCallStr << ")";
    }
  }
  else
  {
    methodCallStr << ")";
  }
  
  stream << methodCallArrayStream.finalize();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::DeserializeFromBuffer(const char* buffer, unsigned int size)
{
#ifdef _DEBUG
  if (size < 6 || size > DXCALL_MAX_SIZE)
  {
    return false;
  }
#endif // ifdef _DEBUG

  Clear();

  memcpy(m_buffer, buffer, size);
  m_positionWrite = size;
  m_paramsCount = GetNumParams();

  for (unsigned int i=0, k=m_positionRead; i < m_paramsCount; i++)
  {
    m_params[i].Type = (DXTypeHelper::DXTypeType) m_buffer[k++];
    m_params[i].Position = k;
    m_params[i].Size = DXTypeHelper::GetTypeSize(m_params[i].Type);
    k += m_params[i].Size;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::DeserializeFromBufferFastInit(char** buffer, unsigned int size)
{
#ifdef _DEBUG
  if (size < 6 || size > DXCALL_MAX_SIZE)
  {
    return false;
  }
#endif // ifdef _DEBUG

  Clear();
  
  m_positionWrite = size;
  *buffer = m_buffer;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::DeserializeFromBufferFast()
{
  m_paramsCount = GetNumParams();
  
  for (unsigned int i=0, k=m_positionRead; i < m_paramsCount; i++)
  {
    m_params[i].Type = (DXTypeHelper::DXTypeType) m_buffer[k++];
    m_params[i].Position = k;
    m_params[i].Size = DXTypeHelper::GetTypeSize(m_params[i].Type);
    k += m_params[i].Size;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::CheckNextPopType(DXTypeHelper::DXTypeType type)
{
#ifdef _DEBUG
  if (!m_paramsCount || m_paramsCount <= m_paramsCurrent)
  {
    return false;
  }
#endif // ifdef _DEBUG
  
  return m_params[m_paramsCurrent].Type == type;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXMethodCall::GetParamCount() const
{
  return m_paramsCount;
}

////////////////////////////////////////////////////////////////////////////////

DXTypeHelper::DXTypeType DXMethodCall::GetParamType(unsigned int numparam) const
{
#ifdef _DEBUG
  if (!m_paramsCount || numparam >= m_paramsCount)
  {
    return DXTypeHelper::TT_DummyType;
  }
#endif // ifdef _DEBUG

  return m_params[numparam].Type;
}

////////////////////////////////////////////////////////////////////////////////

bool DXMethodCall::GetParam(unsigned int numparam, char* buffer)
{
#ifdef _DEBUG
  if (!m_paramsCount || numparam >= m_paramsCount)
  {
    return false;
  }
#endif // ifdef _DEBUG

  memcpy(buffer, &m_buffer[m_params[numparam].Position], m_params[numparam].Size);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

// basic types
PARAM_MACRO_BY_VALUE(HRESULT);
PARAM_MACRO_BY_VALUE(HMONITOR);
PARAM_MACRO_BY_VALUE(BYTE);
PARAM_MACRO_BY_VALUE(WORD);
PARAM_MACRO_BY_VALUE(DWORD);
PARAM_MACRO_BY_VALUE(INT);
PARAM_MACRO_BY_VALUE(UINT);
PARAM_MACRO_BY_VALUE(ULONG);
PARAM_MACRO_BY_VALUE(BOOL);
PARAM_MACRO_BY_VALUE(int);
PARAM_MACRO_BY_VALUE(HWND);
PARAM_MACRO_BY_VALUE(float);
PARAM_MACRO_BY_VALUE(D3DCOLOR);

// enum types
PARAM_MACRO_BY_VALUE(D3DFORMAT);
PARAM_MACRO_BY_VALUE(D3DDEVTYPE);
PARAM_MACRO_BY_VALUE(D3DSWAPEFFECT);
PARAM_MACRO_BY_VALUE(D3DMULTISAMPLE_TYPE);
PARAM_MACRO_BY_VALUE(D3DRESOURCETYPE);
PARAM_MACRO_BY_VALUE(D3DRENDERSTATETYPE);
PARAM_MACRO_BY_VALUE(D3DPOOL);
PARAM_MACRO_BY_VALUE(D3DQUERYTYPE);
PARAM_MACRO_BY_VALUE(D3DTRANSFORMSTATETYPE);
PARAM_MACRO_BY_VALUE(D3DPRIMITIVETYPE);
PARAM_MACRO_BY_VALUE(D3DSAMPLERSTATETYPE);
PARAM_MACRO_BY_VALUE(D3DTEXTURESTAGESTATETYPE);
PARAM_MACRO_BY_VALUE(D3DBACKBUFFER_TYPE);
PARAM_MACRO_BY_VALUE(D3DSTATEBLOCKTYPE);
PARAM_MACRO_BY_VALUE(D3DTEXTUREFILTERTYPE);
PARAM_MACRO_BY_VALUE(D3DCUBEMAP_FACES);

// struct types
PARAM_MACRO_BY_POINTER(IID);
PARAM_MACRO_BY_POINTER(RECT);
PARAM_MACRO_BY_POINTER(POINT);
PARAM_MACRO_BY_POINTER(D3DBOX);
PARAM_MACRO_BY_POINTER(D3DCLIPSTATUS9);

// special types
PARAM_MACRO_BY_VALUE(DXBufferIdentifier);
PARAM_MACRO_BY_VALUE(DXTextureIdentifier);
PARAM_MACRO_BY_VALUE(DXIgnoredParameter);
PARAM_MACRO_BY_VALUE(DXNullPointer);
PARAM_MACRO_BY_VALUE(DXResourceObjectID);
PARAM_MACRO_BY_VALUE(DXVoid);

// flags types
PARAM_MACRO_BY_VALUE(DXFlagsD3DLOCK);
PARAM_MACRO_BY_VALUE(DXFlagsD3DCREATE);
PARAM_MACRO_BY_VALUE(DXFlagsD3DCLEAR);
PARAM_MACRO_BY_VALUE(DXFlagsD3DFVF);
PARAM_MACRO_BY_VALUE(DXFlagsD3DUSAGE);
PARAM_MACRO_BY_VALUE(DXFlagsD3DSGR);
PARAM_MACRO_BY_VALUE(DXFlagsD3DISSUE);
PARAM_MACRO_BY_VALUE(DXFlagsD3DENUM);
PARAM_MACRO_BY_VALUE(DXFlagsD3DPRESENT);

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXFileManagerBase;
  
  class DXBuffer : public SmartPointer
  {
  public:

    DXBuffer();
    virtual ~DXBuffer();

    void Clear();
    
    DXTypeHelper::DXTypeType GetType() const;
    void SetType(DXTypeHelper::DXTypeType type);
    
    unsigned int GetSize() const;
    unsigned int GetSerializedSize() const;
    unsigned int GetCRC32();
    void GetMD5(unsigned char* md5_hash);

    bool SerializeToFile(DXFileManagerBase* file);
    bool DeserializeFromFile(DXFileManagerBase* file, unsigned int size);

    bool SerializeToBuffer(char* buffer, unsigned int* size);
    bool DeserializeFromBuffer(const char* buffer, unsigned int size);
    
    bool Pop_DXRawData(char** ptr);
    bool Push_DXRawData(const void* buffer, unsigned int size);

    bool Pop_ARR_D3DVERTEXELEMENT9(char** ptr);
    bool Push_ARR_D3DVERTEXELEMENT9(const D3DVERTEXELEMENT9* value);

    bool Pop_ARR_RGNDATA(char** ptr);
    bool Push_ARR_RGNDATA(const RGNDATA* value);

    bool Pop_ARR_D3DRECT(char** ptr);
    bool Push_ARR_D3DRECT(const D3DRECT* value, UINT count = 1);

    bool Pop_ARR_DRAWPRIMITIVEUP(char** ptr);
    bool Push_ARR_DRAWPRIMITIVEUP(const void* value, D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, UINT vertexStride);

    bool Pop_ARR_DRAWINDEXEDPRIMITIVEUPINDICES(char** ptr);
    bool Push_ARR_DRAWINDEXEDPRIMITIVEUPINDICES(const void* value, D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, D3DFORMAT indexDataFormat);

    bool Pop_ARR_DRAWINDEXEDPRIMITIVEUPVERTICES(char** ptr);
    bool Push_ARR_DRAWINDEXEDPRIMITIVEUPVERTICES(const void* value, UINT minVertexIndex, UINT numVertices, UINT vertexStreamZeroStride);

    bool Pop_ARR_SHADERFUNCTIONTOKEN(char** ptr);
    bool Push_ARR_SHADERFUNCTIONTOKEN(const DWORD* value);

    bool Pop_ARR_SHADERCONSTANTBOOL(char** ptr);
    bool Push_ARR_SHADERCONSTANTBOOL(const BOOL* value, UINT count);

    bool Pop_ARR_SHADERCONSTANTFLOAT(char** ptr);
    bool Push_ARR_SHADERCONSTANTFLOAT(const float* value, UINT count);

    bool Pop_ARR_SHADERCONSTANTINT(char** ptr);
    bool Push_ARR_SHADERCONSTANTINT(const int* value, UINT count);

    bool Pop_ARR_SETCLIPPLANE(char** ptr);
    bool Push_ARR_SETCLIPPLANE(const float* value);

    bool Pop_ARR_PALETTEENTRY(char** ptr);
    bool Push_ARR_PALETTEENTRY(const PALETTEENTRY* value);

    bool Pop_D3DPRESENT_PARAMETERS(char** ptr);
    bool Push_D3DPRESENT_PARAMETERS(const D3DPRESENT_PARAMETERS* value);

    bool Pop_D3DLIGHT9(char** ptr);
    bool Push_D3DLIGHT9(const D3DLIGHT9* value);

    bool Pop_D3DMATRIX(char** ptr);
    bool Push_D3DMATRIX(const D3DMATRIX* value);

    bool Pop_D3DMATERIAL9(char** ptr);
    bool Push_D3DMATERIAL9(const D3DMATERIAL9* value);

    bool Pop_D3DVIEWPORT9(char** ptr);
    bool Push_D3DVIEWPORT9(const D3DVIEWPORT9* value);

    bool Pop_D3DGAMMARAMP(char** ptr);
    bool Push_D3DGAMMARAMP(const D3DGAMMARAMP* value);

  protected:

    DXTypeHelper::DXTypeType m_type;
    unsigned int m_size;
    bool m_mustFreeData;
    char* m_data;

    bool BufferRead(char* buffer, unsigned int& size);
    bool BufferWrite(const char* buffer, unsigned int size, DXTypeHelper::DXTypeType type);

    unsigned int CalculateSize_ARR_D3DVERTEXELEMENT9(const D3DVERTEXELEMENT9* value);
    unsigned int CalculateSize_ARR_RGNDATA(const RGNDATA* value);
    unsigned int CalculateSize_ARR_DRAWPRIMITIVEUP(D3DPRIMITIVETYPE primitiveType, UINT primitiveCount, UINT vertexStride);
    unsigned int CalculateSize_ARR_SHADERFUNCTIONTOKEN(const DWORD* value);

  };

  //////////////////////////////////////////////////////////////////////////////

  typedef smart_ptr<DXBuffer> DXBufferPtr;

  //////////////////////////////////////////////////////////////////////////////

}

////////////////////////////////////////////////////////////////////////////////

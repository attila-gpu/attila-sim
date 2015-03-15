////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "SmartPointer.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXFileManagerBase;
  
  class DXTexture : public SmartPointer
  {
  public:

    DXTexture();
    virtual ~DXTexture();

    void Clear();

    void SetType(DXTypeHelper::DXTypeType type);
    DXTypeHelper::DXTypeType GetType() const;
    
    unsigned int GetSize() const;
    unsigned int GetSerializedSize() const;
    unsigned int GetCRC32();

    UINT GetWidth() const;
    UINT GetHeight() const;
    UINT GetDepth() const;
    void SetDimensions(UINT width, UINT height);
    void SetDimensions(UINT width, UINT height, UINT depth);

    D3DFORMAT GetFormat() const;
    void SetFormat(D3DFORMAT format);

    UINT GetLevel() const;
    void SetLevel(UINT level);

    UINT GetPitch() const;
    void SetPitch(UINT pitch);

    UINT GetSlicePitch() const;
    void SetSlicePitch(UINT pitch);
    
    bool SerializeToFile(DXFileManagerBase* file);
    bool DeserializeFromFile(DXFileManagerBase* file, unsigned int size);

    bool TextureData_Read(char* data, unsigned int* size);
    bool TextureData_Write(char* data, unsigned int size);
    void TextureData_SeekRead(unsigned int position, bool fromCurrentPosition);

  protected:

#pragma pack(push)
#pragma pack(1)

    struct DXTextureInformation
    {
      DXTypeHelper::DXTypeType Type;
      UINT Width;
      UINT Height;
      UINT Depth;
      D3DFORMAT Format;
      UINT Level;
      UINT Pitch;
      UINT SlicePitch;
    };

#pragma pack(pop)

    DXTextureInformation m_information;
    unsigned int m_size;
    unsigned int m_crc32;
    unsigned int m_readPosition;
    bool m_mustFreeData;
    char* m_data;

  };

  //////////////////////////////////////////////////////////////////////////////

  typedef smart_ptr<DXTexture> DXTexturePtr;

  //////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////

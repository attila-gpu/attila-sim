////////////////////////////////////////////////////////////////////////////////

#pragma once

//////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  enum IndexType
  {
    IT_MethodCall = 0,
    IT_Buffer     = 1,
    IT_Texture    = 2,
    IT_Statistic  = 3
  };
}

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(4)

namespace dxtraceman
{
  //////////////////////////////////////////////////////////////////////////////
  
  struct MiniIndexInDisk
  {
  public:

    MiniIndexInDisk() :
    TypeSize(0)
    {
    }

    MiniIndexInDisk(IndexType type, DWORD size)
    {
      TypeSize = size;
      TypeSize &= 0x3FFFFFFF;
      TypeSize |= type << 30;
    }

    IndexType GetType()
    {
      return (IndexType) ((TypeSize >> 30) & 0x00000003);
    }

    DWORD GetSize()
    {
      return TypeSize & 0x3FFFFFFF;
    }

    void SetSize(DWORD size)
    {
      TypeSize &= 0xC0000000;
      TypeSize |= (size & 0x3FFFFFFF);
    }

  protected:

    DWORD TypeSize;

  };

  //////////////////////////////////////////////////////////////////////////////

  struct IndexInDisk
  {
  public:

    IndexInDisk() :
    Position(0),
    TypeSize(0)
    {
    }

    IndexInDisk(IndexType type, ULONGLONG position, DWORD size) :
    Position(position)
    {
      TypeSize = size;
      TypeSize &= 0x3FFFFFFF;
      TypeSize |= type << 30;
    }

    IndexType GetType()
    {
      return (IndexType) ((TypeSize >> 30) & 0x00000003);
    }

    ULONGLONG GetPosition()
    {
      return Position;
    }

    DWORD GetSize()
    {
      return TypeSize & 0x3FFFFFFF;
    }

    void SetSize(DWORD size)
    {
      TypeSize &= 0xC0000000;
      TypeSize |= (size & 0x3FFFFFFF);
    }

  protected:

    ULONGLONG Position;
    DWORD TypeSize;

  };

  //////////////////////////////////////////////////////////////////////////////

  struct IndexInMemory
  {
    ULONGLONG Position;
    DWORD Size;
  };

  //////////////////////////////////////////////////////////////////////////////
}

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXTraceManager;

  class IDXIndexManager
  {
  public:

    virtual bool Init() = 0;
    virtual void Close() = 0;
    virtual IndexInMemory* GetIndexMethodCall(unsigned int element_id) = 0;
    virtual IndexInMemory* GetIndexBuffer(unsigned int element_id) = 0;
    virtual IndexInMemory* GetIndexTexture(unsigned int element_id) = 0;
    virtual IndexInMemory* GetIndexStatistic(unsigned int element_id) = 0;

  };
}

////////////////////////////////////////////////////////////////////////////////

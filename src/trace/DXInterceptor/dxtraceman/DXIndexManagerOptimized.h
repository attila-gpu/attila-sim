////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "IDXIndexManager.h"

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(4)

namespace dxtraceman
{
  struct OptimizedIndexBlockInDisk
  {
  public:

    static const std::string HEADER_MAGIC_NUMBER;
    
    BYTE  MagicNumber[8];
    DWORD NumIndexs;

    OptimizedIndexBlockInDisk() :
    NumIndexs(0)
    {
      memset(MagicNumber, 0, sizeof(MagicNumber));
    }

    void SetMagicNumber()
    {
      memcpy(MagicNumber, HEADER_MAGIC_NUMBER.c_str(), sizeof(MagicNumber));
    }

    bool CheckMagicNumber()
    {
      return HEADER_MAGIC_NUMBER.compare(0, HEADER_MAGIC_NUMBER.length(), (const char*) MagicNumber, sizeof(MagicNumber)) == 0;
    }

    void Clear()
    {
      memset(MagicNumber, 0, sizeof(MagicNumber));
      NumIndexs = 0;
    }

  };
}

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXIndexManagerOptimized : public IDXIndexManager
  {
  public:

    DXIndexManagerOptimized(DXTraceManager& traceman);
    virtual ~DXIndexManagerOptimized();

    bool Init();
    void Close();

    IndexInMemory* GetIndexMethodCall(unsigned int element_id);
    IndexInMemory* GetIndexBuffer(unsigned int element_id);
    IndexInMemory* GetIndexTexture(unsigned int element_id);
    IndexInMemory* GetIndexStatistic(unsigned int element_id);

  protected:

    static const unsigned int READ_INDEX_PACKET_MAX_METHODCALLS;
    static const unsigned int READ_INDEX_PACKET_MAX_BUFFERS;
    static const unsigned int READ_INDEX_PACKET_MAX_TEXTURES;
    static const unsigned int READ_INDEX_PACKET_MAX_STATISTICS;
    
    DXTraceManager& m_traceman;

    unsigned int m_currentIndexBlockMethodCalls;
    unsigned int m_currentIndexBlockBuffers;
    unsigned int m_currentIndexBlockTextures;
    unsigned int m_currentIndexBlockStatistics;

    unsigned int m_currentIndexBlockSizeMethodCalls;
    unsigned int m_currentIndexBlockSizeBuffers;
    unsigned int m_currentIndexBlockSizeTextures;
    unsigned int m_currentIndexBlockSizeStatistics;

    ULONGLONG m_filePositionMethodCalls;
    ULONGLONG m_filePositionBuffers;
    ULONGLONG m_filePositionTextures;
    ULONGLONG m_filePositionStatistics;

    IndexInMemory* m_listPositionsMethodCalls;
    IndexInMemory* m_listPositionsBuffers;
    IndexInMemory* m_listPositionsTextures;
    IndexInMemory* m_listPositionsStatistics;

    bool CheckOptimizedIndexBlockHeaders();

    bool LoadMethodCallsIndexBlock(unsigned int numBlock);
    bool LoadBuffersIndexBlock(unsigned int numBlock);
    bool LoadTexturesIndexBlock(unsigned int numBlock);
    bool LoadStatisticsIndexBlock(unsigned int numBlock);

  };
}

////////////////////////////////////////////////////////////////////////////////

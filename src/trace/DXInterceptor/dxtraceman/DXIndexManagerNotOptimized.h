////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "IDXIndexManager.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXIndexManagerNotOptimized : public IDXIndexManager
  {
  public:

    DXIndexManagerNotOptimized(DXTraceManager& traceman);
    virtual ~DXIndexManagerNotOptimized();

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

    ULONGLONG m_lastFilePositionMethodCalls;
    ULONGLONG m_lastFilePositionBuffers;
    ULONGLONG m_lastFilePositionTextures;
    ULONGLONG m_lastFilePositionStatistics;

    IndexInMemory* m_listPositionsMethodCalls;
    IndexInMemory* m_listPositionsBuffers;
    IndexInMemory* m_listPositionsTextures;
    IndexInMemory* m_listPositionsStatistics;

    bool LoadMethodCallsIndexBlock(unsigned int numBlock);
    bool LoadBuffersIndexBlock(unsigned int numBlock);
    bool LoadTexturesIndexBlock(unsigned int numBlock);
    bool LoadStatisticsIndexBlock(unsigned int numBlock);

    bool LoadNextMethodCallsIndexBlock(bool loadFast = false);
    bool LoadNextBuffersIndexBlock(bool loadFast = false);
    bool LoadNextTexturesIndexBlock(bool loadFast = false);
    bool LoadNextStatisticsIndexBlock(bool loadFast = false);

  };
}

////////////////////////////////////////////////////////////////////////////////

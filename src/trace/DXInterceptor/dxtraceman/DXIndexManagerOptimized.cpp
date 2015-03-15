////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXBasicTypes.h"
#include "DXTypeHelper.h"
#include "DXMethodCallHelper.h"
#include "DXMethodCall.h"
#include "DXBuffer.h"
#include "DXTexture.h"
#include "DXStatistic.h"
#include "DXFileManagerW32.h"
#include "DXFileManagerSTL.h"
#include "DXTraceManager.h"
#include "DXIndexManagerOptimized.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

const string OptimizedIndexBlockInDisk::HEADER_MAGIC_NUMBER = "IDXBLOCK";

////////////////////////////////////////////////////////////////////////////////

const unsigned int DXIndexManagerOptimized::READ_INDEX_PACKET_MAX_METHODCALLS = 1000;
const unsigned int DXIndexManagerOptimized::READ_INDEX_PACKET_MAX_BUFFERS     = 1000;
const unsigned int DXIndexManagerOptimized::READ_INDEX_PACKET_MAX_TEXTURES    = 1000;
const unsigned int DXIndexManagerOptimized::READ_INDEX_PACKET_MAX_STATISTICS  = 1000;

////////////////////////////////////////////////////////////////////////////////

DXIndexManagerOptimized::DXIndexManagerOptimized(DXTraceManager& traceman) :
m_traceman(traceman),
m_listPositionsMethodCalls(NULL),
m_listPositionsBuffers(NULL),
m_listPositionsTextures(NULL),
m_listPositionsStatistics(NULL)
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

DXIndexManagerOptimized::~DXIndexManagerOptimized()
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerOptimized::Init()
{
  Close();
  
  if (!m_traceman.IsOptimized())
  {
    return false;
  }

  m_listPositionsMethodCalls = new IndexInMemory[READ_INDEX_PACKET_MAX_METHODCALLS];
  m_listPositionsBuffers = new IndexInMemory[READ_INDEX_PACKET_MAX_BUFFERS];
  m_listPositionsTextures = new IndexInMemory[READ_INDEX_PACKET_MAX_TEXTURES];
  m_listPositionsStatistics = new IndexInMemory[READ_INDEX_PACKET_MAX_STATISTICS];

  if (!m_listPositionsMethodCalls || !m_listPositionsBuffers || !m_listPositionsTextures || !m_listPositionsStatistics)
  {
    Close();
    return false;
  }

  m_filePositionMethodCalls = m_traceman.m_indexBlockPositionMethodCall;
  m_filePositionBuffers     = m_traceman.m_indexBlockPositionBuffer;
  m_filePositionTextures    = m_traceman.m_indexBlockPositionTexture;
  m_filePositionStatistics  = m_traceman.m_indexBlockPositionStatistic;
  
  if (!CheckOptimizedIndexBlockHeaders())
  {
    Close();
    return false;
  }
  
  if (!LoadMethodCallsIndexBlock(0))
  {
    Close();
    return false;
  }

  if (!LoadBuffersIndexBlock(0))
  {
    Close();
    return false;
  }

  if (!LoadTexturesIndexBlock(0))
  {
    Close();
    return false;
  }

  if (!LoadStatisticsIndexBlock(0))
  {
    Close();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerOptimized::CheckOptimizedIndexBlockHeaders()
{
  OptimizedIndexBlockInDisk indexBlock;
  unsigned int readedBytes = sizeof(OptimizedIndexBlockInDisk);
  
  //////////////////////////////////////////////////////////////////////////////
  // check method calls index block signature
  
  m_traceman.m_file.SeekRead(m_filePositionMethodCalls);  
  if (!m_traceman.m_file.Read((char*) &indexBlock, &readedBytes))
  {
    return false;
  }
  else
  {
    if (!indexBlock.CheckMagicNumber() || indexBlock.NumIndexs != m_traceman.GetMethodCallCount())
    {
      return false;
    }
    else
    {
      m_filePositionMethodCalls += sizeof(OptimizedIndexBlockInDisk);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // check buffers index block signature
  
  m_traceman.m_file.SeekRead(m_filePositionBuffers);  
  if (!m_traceman.m_file.Read((char*) &indexBlock, &readedBytes))
  {
    return false;
  }
  else
  {
    if (!indexBlock.CheckMagicNumber() || indexBlock.NumIndexs != m_traceman.GetBufferCount())
    {
      return false;
    }
    else
    {
      m_filePositionBuffers += sizeof(OptimizedIndexBlockInDisk);
    }
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // check textures index block signature

  m_traceman.m_file.SeekRead(m_filePositionTextures);  
  if (!m_traceman.m_file.Read((char*) &indexBlock, &readedBytes))
  {
    return false;
  }
  else
  {
    if (!indexBlock.CheckMagicNumber() || indexBlock.NumIndexs != m_traceman.GetTextureCount())
    {
      return false;
    }
    else
    {
      m_filePositionTextures += sizeof(OptimizedIndexBlockInDisk);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // check statistics index block signature

  m_traceman.m_file.SeekRead(m_filePositionStatistics);  
  if (!m_traceman.m_file.Read((char*) &indexBlock, &readedBytes))
  {
    return false;
  }
  else
  {
    if (!indexBlock.CheckMagicNumber() || indexBlock.NumIndexs != m_traceman.GetStatisticCount())
    {
      return false;
    }
    else
    {
      m_filePositionStatistics += sizeof(OptimizedIndexBlockInDisk);
    }
  }
  
  //////////////////////////////////////////////////////////////////////////////
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXIndexManagerOptimized::Close()
{
  m_currentIndexBlockMethodCalls = 0;
  m_currentIndexBlockBuffers = 0;
  m_currentIndexBlockTextures = 0;
  m_currentIndexBlockStatistics = 0;

  m_currentIndexBlockSizeMethodCalls = 0;
  m_currentIndexBlockSizeBuffers = 0;
  m_currentIndexBlockSizeTextures = 0;
  m_currentIndexBlockSizeStatistics = 0;

  m_filePositionMethodCalls = 0;
  m_filePositionBuffers = 0;
  m_filePositionTextures = 0;
  m_filePositionStatistics = 0;

  if (m_listPositionsMethodCalls)
  {
    delete[] m_listPositionsMethodCalls;
    m_listPositionsMethodCalls = NULL;
  }

  if (m_listPositionsBuffers)
  {
    delete[] m_listPositionsBuffers;
    m_listPositionsBuffers = NULL;
  }

  if (m_listPositionsTextures)
  {
    delete[] m_listPositionsTextures;
    m_listPositionsTextures = NULL;
  }

  if (m_listPositionsStatistics)
  {
    delete[] m_listPositionsStatistics;
    m_listPositionsStatistics = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

IndexInMemory* DXIndexManagerOptimized::GetIndexMethodCall(unsigned int element_id)
{
  unsigned int numBlock = element_id / READ_INDEX_PACKET_MAX_METHODCALLS;
  
  if (numBlock != m_currentIndexBlockMethodCalls)
  {
    if (!LoadMethodCallsIndexBlock(numBlock))
    {
      return NULL;
    }
  }

  element_id -= m_currentIndexBlockMethodCalls * READ_INDEX_PACKET_MAX_METHODCALLS;

#ifdef _DEBUG
  if (element_id >= m_currentIndexBlockSizeMethodCalls)
  {
    return NULL;
  }
#endif // ifdef _DEBUG

  return &m_listPositionsMethodCalls[element_id];
}

////////////////////////////////////////////////////////////////////////////////

IndexInMemory* DXIndexManagerOptimized::GetIndexBuffer(unsigned int element_id)
{
  unsigned int numBlock = element_id / READ_INDEX_PACKET_MAX_BUFFERS;
  
  if (numBlock != m_currentIndexBlockBuffers)
  {
    if (!LoadBuffersIndexBlock(numBlock))
    {
      return NULL;
    }
  }

  element_id -= m_currentIndexBlockBuffers * READ_INDEX_PACKET_MAX_BUFFERS;

#ifdef _DEBUG
  if (element_id >= m_currentIndexBlockSizeBuffers)
  {
    return NULL;
  }
#endif // ifdef _DEBUG

  return &m_listPositionsBuffers[element_id];
}

////////////////////////////////////////////////////////////////////////////////

IndexInMemory* DXIndexManagerOptimized::GetIndexTexture(unsigned int element_id)
{
  unsigned int numBlock = element_id / READ_INDEX_PACKET_MAX_TEXTURES;

  if (numBlock != m_currentIndexBlockTextures)
  {
    if (!LoadTexturesIndexBlock(numBlock))
    {
      return NULL;
    }
  }

  element_id -= m_currentIndexBlockTextures * READ_INDEX_PACKET_MAX_TEXTURES;

#ifdef _DEBUG
  if (element_id >= m_currentIndexBlockSizeTextures)
  {
    return NULL;
  }
#endif // ifdef _DEBUG

  return &m_listPositionsTextures[element_id];
}

////////////////////////////////////////////////////////////////////////////////

IndexInMemory* DXIndexManagerOptimized::GetIndexStatistic(unsigned int element_id)
{
  unsigned int numBlock = element_id / READ_INDEX_PACKET_MAX_STATISTICS;

  if (numBlock != m_currentIndexBlockStatistics)
  {
    if (!LoadStatisticsIndexBlock(numBlock))
    {
      return NULL;
    }
  }

  element_id -= m_currentIndexBlockStatistics * READ_INDEX_PACKET_MAX_STATISTICS;

#ifdef _DEBUG
  if (element_id >= m_currentIndexBlockSizeStatistics)
  {
    return NULL;
  }
#endif // ifdef _DEBUG

  return &m_listPositionsStatistics[element_id];
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerOptimized::LoadMethodCallsIndexBlock(unsigned int numBlock)
{
  m_currentIndexBlockMethodCalls = numBlock;
  m_currentIndexBlockSizeMethodCalls = min(READ_INDEX_PACKET_MAX_METHODCALLS, m_traceman.m_numMethodCalls - (m_currentIndexBlockMethodCalls * READ_INDEX_PACKET_MAX_METHODCALLS));

  m_traceman.m_file.SeekRead(m_filePositionMethodCalls + m_currentIndexBlockMethodCalls*READ_INDEX_PACKET_MAX_METHODCALLS*sizeof(IndexInDisk));
  
  unsigned int readedBytes = m_currentIndexBlockSizeMethodCalls*sizeof(IndexInDisk);
  if (!m_traceman.m_file.Read((char*) m_listPositionsMethodCalls, &readedBytes))
  {
    return false;
  }

  for (unsigned int i=0; i < m_currentIndexBlockSizeMethodCalls; ++i)
  {
    m_listPositionsMethodCalls[i].Size &= 0x3FFFFFFF;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerOptimized::LoadBuffersIndexBlock(unsigned int numBlock)
{
  m_currentIndexBlockBuffers = numBlock;
  m_currentIndexBlockSizeBuffers = min(READ_INDEX_PACKET_MAX_BUFFERS, m_traceman.m_numBuffers - (m_currentIndexBlockBuffers * READ_INDEX_PACKET_MAX_BUFFERS));

  m_traceman.m_file.SeekRead(m_filePositionBuffers + m_currentIndexBlockBuffers*READ_INDEX_PACKET_MAX_BUFFERS*sizeof(IndexInDisk));

  unsigned int readedBytes = m_currentIndexBlockSizeBuffers*sizeof(IndexInDisk);
  if (!m_traceman.m_file.Read((char*) m_listPositionsBuffers, &readedBytes))
  {
    return false;
  }

  for (unsigned int i=0; i < m_currentIndexBlockSizeBuffers; ++i)
  {
    m_listPositionsBuffers[i].Size &= 0x3FFFFFFF;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerOptimized::LoadTexturesIndexBlock(unsigned int numBlock)
{
  m_currentIndexBlockTextures = numBlock;
  m_currentIndexBlockSizeTextures = min(READ_INDEX_PACKET_MAX_TEXTURES, m_traceman.m_numTextures - (m_currentIndexBlockTextures * READ_INDEX_PACKET_MAX_TEXTURES));

  m_traceman.m_file.SeekRead(m_filePositionTextures + m_currentIndexBlockTextures*READ_INDEX_PACKET_MAX_TEXTURES*sizeof(IndexInDisk));

  unsigned int readedBytes = m_currentIndexBlockSizeTextures*sizeof(IndexInDisk);
  if (!m_traceman.m_file.Read((char*) m_listPositionsTextures, &readedBytes))
  {
    return false;
  }

  for (unsigned int i=0; i < m_currentIndexBlockSizeTextures; ++i)
  {
    m_listPositionsTextures[i].Size &= 0x3FFFFFFF;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerOptimized::LoadStatisticsIndexBlock(unsigned int numBlock)
{
  m_currentIndexBlockStatistics = numBlock;
  m_currentIndexBlockSizeStatistics = min(READ_INDEX_PACKET_MAX_STATISTICS, m_traceman.m_numStatistics - (m_currentIndexBlockStatistics * READ_INDEX_PACKET_MAX_STATISTICS));

  m_traceman.m_file.SeekRead(m_filePositionStatistics + m_currentIndexBlockStatistics*READ_INDEX_PACKET_MAX_STATISTICS*sizeof(IndexInDisk));

  unsigned int readedBytes = m_currentIndexBlockSizeStatistics*sizeof(IndexInDisk);
  if (!m_traceman.m_file.Read((char*) m_listPositionsStatistics, &readedBytes))
  {
    return false;
  }

  for (unsigned int i=0; i < m_currentIndexBlockSizeStatistics; ++i)
  {
    m_listPositionsStatistics[i].Size &= 0x3FFFFFFF;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

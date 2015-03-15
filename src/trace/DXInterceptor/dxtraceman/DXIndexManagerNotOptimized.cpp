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
#include "DXIndexManagerNotOptimized.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

const unsigned int DXIndexManagerNotOptimized::READ_INDEX_PACKET_MAX_METHODCALLS = 10000;
const unsigned int DXIndexManagerNotOptimized::READ_INDEX_PACKET_MAX_BUFFERS     = 10000;
const unsigned int DXIndexManagerNotOptimized::READ_INDEX_PACKET_MAX_TEXTURES    = 100;
const unsigned int DXIndexManagerNotOptimized::READ_INDEX_PACKET_MAX_STATISTICS  = 50;

////////////////////////////////////////////////////////////////////////////////

DXIndexManagerNotOptimized::DXIndexManagerNotOptimized(DXTraceManager& traceman) :
m_traceman(traceman),
m_listPositionsMethodCalls(NULL),
m_listPositionsBuffers(NULL),
m_listPositionsTextures(NULL),
m_listPositionsStatistics(NULL)
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

DXIndexManagerNotOptimized::~DXIndexManagerNotOptimized()
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerNotOptimized::Init()
{
  Close();
  
  if (m_traceman.IsOptimized())
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

void DXIndexManagerNotOptimized::Close()
{
  m_currentIndexBlockMethodCalls = 0;
  m_currentIndexBlockBuffers = 0;
  m_currentIndexBlockTextures = 0;
  m_currentIndexBlockStatistics = 0;

  m_currentIndexBlockSizeMethodCalls = 0;
  m_currentIndexBlockSizeBuffers = 0;
  m_currentIndexBlockSizeTextures = 0;
  m_currentIndexBlockSizeStatistics = 0;

  m_lastFilePositionMethodCalls = 0;
  m_lastFilePositionBuffers = 0;
  m_lastFilePositionTextures = 0;
  m_lastFilePositionStatistics = 0;

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

IndexInMemory* DXIndexManagerNotOptimized::GetIndexMethodCall(unsigned int element_id)
{
  if (!LoadMethodCallsIndexBlock(element_id / READ_INDEX_PACKET_MAX_METHODCALLS))
  {
    return NULL;
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

IndexInMemory* DXIndexManagerNotOptimized::GetIndexBuffer(unsigned int element_id)
{
  if (!LoadBuffersIndexBlock(element_id / READ_INDEX_PACKET_MAX_BUFFERS))
  {
    return NULL;
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

IndexInMemory* DXIndexManagerNotOptimized::GetIndexTexture(unsigned int element_id)
{
  if (!LoadTexturesIndexBlock(element_id / READ_INDEX_PACKET_MAX_TEXTURES))
  {
    return NULL;
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

IndexInMemory* DXIndexManagerNotOptimized::GetIndexStatistic(unsigned int element_id)
{
  if (!LoadStatisticsIndexBlock(element_id / READ_INDEX_PACKET_MAX_STATISTICS))
  {
    return NULL;
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

bool DXIndexManagerNotOptimized::LoadMethodCallsIndexBlock(unsigned int numBlock)
{
  bool problems = false;
  
  if (numBlock == m_currentIndexBlockMethodCalls)
  {
    if (!numBlock && !m_lastFilePositionMethodCalls)
    {
      m_currentIndexBlockMethodCalls = 0;
      m_lastFilePositionMethodCalls = 0;
      problems = !LoadNextMethodCallsIndexBlock();
    }
  }
  else if (numBlock == m_currentIndexBlockMethodCalls+1)
  {
    m_currentIndexBlockMethodCalls++;
    problems = !LoadNextMethodCallsIndexBlock();
  }
  else
  {
    m_currentIndexBlockMethodCalls = 0;
    m_lastFilePositionMethodCalls = 0;
    for (unsigned int i=0; i <= numBlock; i++)
    {
      if (!LoadNextMethodCallsIndexBlock(i != numBlock))
      {
        problems = true;
        break;
      }
      
      if (i < numBlock)
      {
        m_currentIndexBlockMethodCalls++;
      }
    }
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerNotOptimized::LoadBuffersIndexBlock(unsigned int numBlock)
{
  bool problems = false;

  if (numBlock == m_currentIndexBlockBuffers)
  {
    if (!numBlock && !m_lastFilePositionBuffers)
    {
      m_currentIndexBlockBuffers = 0;
      m_lastFilePositionBuffers = 0;
      problems = !LoadNextBuffersIndexBlock();
    }
  }
  else if (numBlock == m_currentIndexBlockBuffers+1)
  {
    m_currentIndexBlockBuffers++;
    problems = !LoadNextBuffersIndexBlock();
  }
  else
  {
    m_currentIndexBlockBuffers = 0;
    m_lastFilePositionBuffers = 0;
    for (unsigned int i=0; i <= numBlock; i++)
    {
      if (!LoadNextBuffersIndexBlock(i != numBlock))
      {
        problems = true;
        break;
      }

      if (i < numBlock)
      {
        m_currentIndexBlockBuffers++;
      }
    }
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerNotOptimized::LoadTexturesIndexBlock(unsigned int numBlock)
{
  bool problems = false;

  if (numBlock == m_currentIndexBlockTextures)
  {
    if (!numBlock && !m_lastFilePositionTextures)
    {
      m_currentIndexBlockTextures = 0;
      m_lastFilePositionTextures = 0;
      problems = !LoadNextTexturesIndexBlock();
    }
  }
  else if (numBlock == m_currentIndexBlockTextures+1)
  {
    m_currentIndexBlockTextures++;
    problems = !LoadNextTexturesIndexBlock();
  }
  else
  {
    m_currentIndexBlockTextures = 0;
    m_lastFilePositionTextures = 0;
    for (unsigned int i=0; i <= numBlock; i++)
    {
      if (!LoadNextTexturesIndexBlock(i != numBlock))
      {
        problems = true;
        break;
      }

      if (i < numBlock)
      {
        m_currentIndexBlockTextures++;
      }
    }
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerNotOptimized::LoadStatisticsIndexBlock(unsigned int numBlock)
{
  bool problems = false;

  if (numBlock == m_currentIndexBlockStatistics)
  {
    if (!numBlock && !m_lastFilePositionStatistics)
    {
      m_currentIndexBlockStatistics = 0;
      m_lastFilePositionStatistics = 0;
      problems = !LoadNextStatisticsIndexBlock();
    }
  }
  else if (numBlock == m_currentIndexBlockStatistics+1)
  {
    m_currentIndexBlockStatistics++;
    problems = !LoadNextStatisticsIndexBlock();
  }
  else
  {
    m_currentIndexBlockStatistics = 0;
    m_lastFilePositionStatistics = 0;
    for (unsigned int i=0; i <= numBlock; i++)
    {
      if (!LoadNextStatisticsIndexBlock(i != numBlock))
      {
        problems = true;
        break;
      }

      if (i < numBlock)
      {
        m_currentIndexBlockStatistics++;
      }
    }
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerNotOptimized::LoadNextMethodCallsIndexBlock(bool loadFast)
{
  m_currentIndexBlockSizeMethodCalls = min(READ_INDEX_PACKET_MAX_METHODCALLS, m_traceman.m_numMethodCalls - (m_currentIndexBlockMethodCalls * READ_INDEX_PACKET_MAX_METHODCALLS));

  unsigned int indexsRemaining = m_currentIndexBlockSizeMethodCalls;

  bool problems = false;
  while (indexsRemaining)
  {
    m_traceman.m_file.SeekRead(m_lastFilePositionMethodCalls);
    
    MiniIndexInDisk miniindex;
    unsigned int readedBytes = sizeof(MiniIndexInDisk);
    m_traceman.m_file.Read((char*) &miniindex, &readedBytes);
    if (readedBytes != sizeof(MiniIndexInDisk))
    {
      problems = true;
      break;
    }

    if (miniindex.GetType() == IT_MethodCall)
    {
      if (!loadFast)
      {
        IndexInMemory* index = &m_listPositionsMethodCalls[m_currentIndexBlockSizeMethodCalls - indexsRemaining];
        index->Size = miniindex.GetSize();
        index->Position = m_traceman.m_file.Tell();
      }
      indexsRemaining--;
    }

    m_lastFilePositionMethodCalls = m_traceman.m_file.Tell() + miniindex.GetSize();
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerNotOptimized::LoadNextBuffersIndexBlock(bool loadFast)
{
  m_currentIndexBlockSizeBuffers = min(READ_INDEX_PACKET_MAX_BUFFERS, m_traceman.m_numBuffers - (m_currentIndexBlockBuffers * READ_INDEX_PACKET_MAX_BUFFERS));

  unsigned int indexsRemaining = m_currentIndexBlockSizeBuffers;
  
  bool problems = false;
  while (indexsRemaining)
  {
    m_traceman.m_file.SeekRead(m_lastFilePositionBuffers);

    MiniIndexInDisk miniindex;
    unsigned int readedBytes = sizeof(MiniIndexInDisk);
    m_traceman.m_file.Read((char*) &miniindex, &readedBytes);
    if (readedBytes != sizeof(MiniIndexInDisk))
    {
      problems = true;
      break;
    }
    
    if (miniindex.GetType() == IT_Buffer)
    {
      if (!loadFast)
      {
        IndexInMemory* index = &m_listPositionsBuffers[m_currentIndexBlockSizeBuffers - indexsRemaining];
        index->Size = miniindex.GetSize();
        index->Position = m_traceman.m_file.Tell();
      }
      indexsRemaining--;
    }

    m_lastFilePositionBuffers = m_traceman.m_file.Tell() + miniindex.GetSize();
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerNotOptimized::LoadNextTexturesIndexBlock(bool loadFast)
{
  m_currentIndexBlockSizeTextures = min(READ_INDEX_PACKET_MAX_TEXTURES, m_traceman.m_numTextures - (m_currentIndexBlockTextures * READ_INDEX_PACKET_MAX_TEXTURES));

  unsigned int indexsRemaining = m_currentIndexBlockSizeTextures;
  
  bool problems = false;
  while (indexsRemaining)
  {
    m_traceman.m_file.SeekRead(m_lastFilePositionTextures);
    
    MiniIndexInDisk miniindex;
    unsigned int readedBytes = sizeof(MiniIndexInDisk);
    m_traceman.m_file.Read((char*) &miniindex, &readedBytes);
    if (readedBytes != sizeof(MiniIndexInDisk))
    {
      problems = true;
      break;
    }

    if (miniindex.GetType() == IT_Texture)
    {
      if (!loadFast)
      {
        IndexInMemory* index = &m_listPositionsTextures[m_currentIndexBlockSizeTextures - indexsRemaining];
        index->Size = miniindex.GetSize();
        index->Position = m_traceman.m_file.Tell();
      }
      indexsRemaining--;
    }

    m_lastFilePositionTextures = m_traceman.m_file.Tell() + miniindex.GetSize();
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXIndexManagerNotOptimized::LoadNextStatisticsIndexBlock(bool loadFast)
{
  m_currentIndexBlockSizeStatistics = min(READ_INDEX_PACKET_MAX_STATISTICS, m_traceman.m_numStatistics - (m_currentIndexBlockStatistics * READ_INDEX_PACKET_MAX_STATISTICS));

  unsigned int indexsRemaining = m_currentIndexBlockSizeStatistics;

  bool problems = false;
  while (indexsRemaining)
  {
    m_traceman.m_file.SeekRead(m_lastFilePositionStatistics);

    MiniIndexInDisk miniindex;
    unsigned int readedBytes = sizeof(MiniIndexInDisk);
    m_traceman.m_file.Read((char*) &miniindex, &readedBytes);
    if (readedBytes != sizeof(MiniIndexInDisk))
    {
      problems = true;
      break;
    }

    if (miniindex.GetType() == IT_Statistic)
    {
      if (!loadFast)
      {
        IndexInMemory* index = &m_listPositionsStatistics[m_currentIndexBlockSizeStatistics - indexsRemaining];
        index->Size = miniindex.GetSize();
        index->Position = m_traceman.m_file.Tell();
      }
      indexsRemaining--;
    }

    m_lastFilePositionStatistics = m_traceman.m_file.Tell() + miniindex.GetSize();
  }

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

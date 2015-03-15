////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
#include "DXFileManagerBlockCache.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

struct DXFileManagerBlockCache::CompressedBlockBufferInformation
{
public:

  unsigned int BlockID;
  unsigned int Age;
  char* Buffer;
  unsigned int Size;

  virtual ~CompressedBlockBufferInformation()
  {
    if (Buffer)
    {
      delete[] Buffer;
      Buffer = NULL;
    }
    BlockID = 0;
    Age = 0;
    Size = 0;
  }
};

////////////////////////////////////////////////////////////////////////////////

DXFileManagerBlockCache::DXFileManagerBlockCache(unsigned int cacheSize, unsigned int blockSize) :
m_cacheSize(cacheSize),
m_blockSize(blockSize)
{
  for (unsigned int i=0; i < m_cacheSize; ++i)
  {
    CacheItemType item = new CompressedBlockBufferInformation;
    item->Buffer = new char[m_blockSize];
    m_cacheFree.push_back(item);
  }
}

////////////////////////////////////////////////////////////////////////////////

DXFileManagerBlockCache::~DXFileManagerBlockCache()
{
  for (CacheHashFreeIterator it=m_cacheFree.begin(); it != m_cacheFree.end(); ++it)
  {
    if (*it)
    {
      delete *it;
      *it = NULL;
    }
  }
  m_cacheFree.clear();
  m_cacheData.clear();
  m_cacheTime.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBlockCache::GetBlockBuffer(unsigned int block_id, char** buffer, unsigned int* size)
{
  CacheHashDataIterator it1 = m_cacheData.find(block_id);
  if (it1 != m_cacheData.end())
  {
    CacheHashTimeIterator it2 = m_cacheTime.find((*it1).second->Age);
    if ((*it2).second->BlockID != (*m_cacheTime.rbegin()).second->BlockID)
    {
      m_cacheTime.erase(it2);
      (*it1).second->Age = GetTimeStamp();
      m_cacheTime[(*it1).second->Age] = (*it1).second;
    }
    *buffer = (*it1).second->Buffer;
    if (size) *size = (*it1).second->Size;
    return true;
  }
  else
  {
    if (m_cacheData.size() == m_cacheSize)
    {
      RemoveOldest();
    }
    
    CacheItemType item = m_cacheFree.back();
    m_cacheFree.pop_back();
    
    item->BlockID = block_id;
    item->Age = GetTimeStamp();
    item->Size = m_blockSize;
    
    m_cacheTime[item->Age] = item;
    m_cacheData[item->BlockID] = item;

    *buffer = item->Buffer;
    if (size) *size = item->Size;
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXFileManagerBlockCache::GetBlockBufferSize(unsigned int block_id) const
{
  CacheHashDataIteratorConst it = m_cacheData.find(block_id);
  if (it != m_cacheData.end())
  {
    return (*it).second->Size;
  }
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerBlockCache::SetBlockBufferSize(unsigned int block_id, unsigned int size)
{
  CacheHashDataIterator it = m_cacheData.find(block_id);
  if (it != m_cacheData.end())
  {
    (*it).second->Size = size;
  }
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXFileManagerBlockCache::GetTimeStamp() const
{
  static unsigned int petitionNumber = 0;
  return ++petitionNumber;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerBlockCache::RemoveOldest()
{
  CacheHashTimeIterator it1 = m_cacheTime.begin();
  CacheHashDataIterator it2 = m_cacheData.find((*it1).second->BlockID);
  
  m_cacheFree.push_back((*it1).second);
  m_cacheTime.erase(it1);
  m_cacheData.erase(it2);
}

////////////////////////////////////////////////////////////////////////////////

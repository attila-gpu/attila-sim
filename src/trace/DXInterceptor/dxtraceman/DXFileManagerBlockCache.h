////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXFileManagerBlockCache
  {
  public:

    DXFileManagerBlockCache(unsigned int cacheSize, unsigned int blockSize);
    virtual ~DXFileManagerBlockCache();
    
    bool GetBlockBuffer(unsigned int block_id, char** buffer, unsigned int* size = NULL);
    unsigned int GetBlockBufferSize(unsigned int block_id) const;
    void SetBlockBufferSize(unsigned int block_id, unsigned int size);
  
  protected:

    struct CompressedBlockBufferInformation;
    
    typedef CompressedBlockBufferInformation* CacheItemType;
    typedef std::vector<CacheItemType> CacheHashFree;
    typedef CacheHashFree::iterator CacheHashFreeIterator;
    typedef CacheHashFree::const_iterator CacheHashFreeIteratorConst;
    typedef std::map<unsigned int, CacheItemType> CacheHashTime;
    typedef CacheHashTime::iterator CacheHashTimeIterator;
    typedef CacheHashTime::const_iterator CacheHashTimeIteratorConst;
    typedef std::map<unsigned int, CacheItemType> CacheHashData;
    typedef CacheHashData::iterator CacheHashDataIterator;
    typedef CacheHashData::const_iterator CacheHashDataIteratorConst;

    unsigned int m_cacheSize;
    unsigned int m_blockSize;
    CacheHashFree m_cacheFree;
    CacheHashTime m_cacheTime;
    CacheHashData m_cacheData;

    unsigned int GetTimeStamp() const;
    void RemoveOldest();

  };
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DXBasicTypes.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXFileManagerBlockCache;
  
  class DXFileManagerBase
  {
  public:

    DXFileManagerBase();
    virtual ~DXFileManagerBase();
    
    bool OpenRead(const std::string& filename, bool enableCompression = false, unsigned int endOfRawHeaderPosition = 0);
    bool OpenWrite(const std::string& filename, bool enableCompression = false, unsigned int endOfRawHeaderPosition = 0, unsigned int blockSize = 0);
    void Close();

    bool IsOpened() const;
    bool IsEOF() const;
    dx_uint64 GetSizeUncompressed() const;
    dx_uint64 GetSizeCompressed() const;

    void SeekRead(dx_uint64 position);
    void SeekWrite(dx_uint64 position);
    dx_uint64 Tell() const;

    ////////////////////////////////////////////////////////////////////////////

    inline bool Read(char* buffer, unsigned int size)
    {
      return (*this.*ReadFunction)(buffer, &size);
    }

    inline bool Write(const char* buffer, unsigned int size)
    {
      return (*this.*WriteFunction)(buffer, &size);
    }

    inline bool ReadRaw(dx_uint64 position, char* buffer, unsigned int size)
    {
      return ReadRaw(position, buffer, &size);
    }

    inline bool WriteRaw(dx_uint64 position, const char* buffer, unsigned int size)
    {
      return WriteRaw(position, buffer, &size);
    }

    ////////////////////////////////////////////////////////////////////////////

    inline bool Read(char* buffer, unsigned int* size)
    {
      return (*this.*ReadFunction)(buffer, size);
    }

    inline bool Write(const char* buffer, unsigned int* size)
    {
      return (*this.*WriteFunction)(buffer, size);
    }

    bool ReadRaw(dx_uint64 position, char* buffer, unsigned int* size);
    bool WriteRaw(dx_uint64 position, const char* buffer, unsigned int* size);

    ////////////////////////////////////////////////////////////////////////////

  protected:

    static const unsigned int LZO_DEFAULT_BUFFER_SIZE;

    struct CompressionHeaderInDisk;

    std::string m_filename;
    bool m_isReadOnly;
    bool m_isOpened;
    bool m_compressed;

    dx_uint64 m_positionRawHeaderDisplacement;
    dx_uint64 m_positionUncompressed;
    dx_uint64 m_positionCompressed;
    dx_uint64 m_sizeUncompressed;
    dx_uint64 m_sizeCompressed;

    unsigned int m_bufferUncompressedPosition;
    unsigned int m_bufferUncompressedPositionEnd;
    char* m_bufferUncompressed;
    char* m_bufferCompressed;
    char* m_bufferWork;
    unsigned int m_blockSize;
    unsigned int m_blockSizeCompressed;
    DXFileManagerBlockCache* m_blocksCache;
    std::vector<dx_uint64> m_blocksPositions;

    bool (DXFileManagerBase::*ReadFunction)(char*, unsigned int*);
    bool (DXFileManagerBase::*WriteFunction)(const char*, unsigned int*);

    unsigned int CalculateCompressedBlockBufferSize(unsigned int bufferSize) const;
    
    bool ReadCompressedMetaData();
    bool WriteCompressedMetaData();
    bool ReadCompressedBlocksPositions(dx_uint64 filePosition, unsigned int numBlocks);
    bool WriteCompressedBlocksPositions();

    bool ReadCompressed(char* buffer, unsigned int* size);
    bool WriteCompressed(const char* buffer, unsigned int* size);
    void ReadCompressedMemory(char* buffer, unsigned int size);
    void WriteCompressedMemory(const char* buffer, unsigned int size);
    bool ReadCompressedDisc();
    bool WriteCompressedDisc();
    bool ReadDisc(char* buffer, unsigned int* size);
    bool WriteDisc(const char* buffer, unsigned int* size);

    virtual bool FS_OpenRead(const std::string& filename) = 0;
    virtual bool FS_OpenWrite(const std::string& filename) = 0;
    virtual void FS_Close() = 0;
    virtual dx_uint64 FS_FileSize() = 0;
    virtual void FS_Seek(dx_uint64 position) = 0;
    virtual dx_uint64 FS_Tell() = 0;
    virtual bool FS_Read(char* buffer, unsigned int* size) = 0;
    virtual bool FS_Write(const char* buffer, unsigned int* size) = 0;
  
  };
}

////////////////////////////////////////////////////////////////////////////////

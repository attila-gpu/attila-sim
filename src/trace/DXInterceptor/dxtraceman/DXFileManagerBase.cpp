////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <map>
#include "lzo/lzo1x.h"
#include "lzo/lzo_asm.h"
#include "DXFileManagerBlockCache.h"
#include "DXFileManagerBase.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(4)

struct DXFileManagerBase::CompressionHeaderInDisk
{
public:

  static const string HEADER_MAGIC_NUMBER;

  dx_byte   MagicNumber[8];
  dx_dword  BlockSize;
  dx_uint64 UncompressedFileSize;
  dx_dword  BlockPositionsCount;
  dx_uint64 BlockPositionsStart;

  CompressionHeaderInDisk()
  {
    Clear();
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
    BlockSize = 0;
    UncompressedFileSize = 0;
    BlockPositionsCount = 0;
    BlockPositionsStart = 0;
    ::memset(MagicNumber, 0, sizeof(MagicNumber));
  }

};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

const unsigned int DXFileManagerBase::LZO_DEFAULT_BUFFER_SIZE = 64*1024; // 64 Kb
const string DXFileManagerBase::CompressionHeaderInDisk::HEADER_MAGIC_NUMBER = "FMCMPLZO";

////////////////////////////////////////////////////////////////////////////////

DXFileManagerBase::DXFileManagerBase() :
m_isReadOnly(false),
m_isOpened(false),
m_compressed(false),
m_positionUncompressed(0),
m_positionCompressed(0),
m_sizeUncompressed(0),
m_sizeCompressed(0),
m_positionRawHeaderDisplacement(0),
m_bufferUncompressedPosition(0),
m_bufferUncompressedPositionEnd(0),
m_bufferUncompressed(NULL),
m_bufferCompressed(NULL),
m_bufferWork(NULL),
m_blocksCache(NULL),
ReadFunction(NULL),
WriteFunction(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

DXFileManagerBase::~DXFileManagerBase()
{
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXFileManagerBase::CalculateCompressedBlockBufferSize(unsigned int bufferSize) const
{
  return bufferSize + (bufferSize / 64) + 16 + 3 + sizeof(lzo_uint);
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::ReadCompressedMetaData()
{
  CompressionHeaderInDisk header;

  if (!ReadRaw(m_positionRawHeaderDisplacement, (char*) &header, sizeof(CompressionHeaderInDisk)))
  {
    return false;
  }

  if (!header.CheckMagicNumber())
  {
    return false;
  }

  m_sizeUncompressed = header.UncompressedFileSize;
  m_blockSize = header.BlockSize;

  return ReadCompressedBlocksPositions(header.BlockPositionsStart, header.BlockPositionsCount);
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::WriteCompressedMetaData()
{
  if (!m_compressed)
  {
    return false;
  }

  CompressionHeaderInDisk header;

  header.SetMagicNumber();
  header.BlockSize = m_blockSize;
  header.UncompressedFileSize = m_sizeUncompressed;
  header.BlockPositionsCount = static_cast<unsigned int>(m_blocksPositions.size());
  header.BlockPositionsStart = m_sizeCompressed;

  if (!WriteRaw(m_positionRawHeaderDisplacement, (char*) &header, sizeof(CompressionHeaderInDisk)))
  {
    return false;
  }

  return WriteCompressedBlocksPositions();
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::ReadCompressedBlocksPositions(dx_uint64 filePosition, unsigned int numBlocks)
{
  dx_uint64 blockPosition = 0;
  for (unsigned int i=0; i < numBlocks; ++i)
  {
    if (!ReadRaw(filePosition, (char*) &blockPosition, sizeof(dx_uint64)))
    {
      return false;
    }
    m_blocksPositions.push_back(blockPosition);
    filePosition += sizeof(dx_uint64);
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::WriteCompressedBlocksPositions()
{
  unsigned int numBlocks = static_cast<unsigned int>(m_blocksPositions.size());  
  dx_uint64 filePosition = m_sizeCompressed;
  for (unsigned int i=0; i < numBlocks; ++i)
  {
    if (!WriteRaw(filePosition, (const char*) &m_blocksPositions[i], sizeof(dx_uint64)))
    {
      return false;
    }
    filePosition += sizeof(dx_uint64);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::IsOpened() const
{
  return m_isOpened;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::IsEOF() const
{
  if (!IsOpened() || !m_isReadOnly)
  {
    return false;
  }
  else
  {
    return m_positionUncompressed >= m_sizeUncompressed;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::OpenRead(const string& filename, bool enableCompression, unsigned int endOfRawHeaderPosition)
{
  Close();

  if (!FS_OpenRead(filename))
  {
    return false;
  }
  else
  {
    m_positionRawHeaderDisplacement = endOfRawHeaderPosition;
    m_positionUncompressed = endOfRawHeaderPosition;
    m_positionCompressed = endOfRawHeaderPosition + (enableCompression ? sizeof(CompressionHeaderInDisk) : 0);
    FS_Seek(m_positionCompressed);

    m_sizeUncompressed = FS_FileSize();
    m_sizeCompressed = m_sizeUncompressed;

    if (enableCompression)
    {
      m_compressed = true;

      if (!ReadCompressedMetaData())
      {
        Close();
        return false;
      }

      if (lzo_init() != LZO_E_OK)
      {
        Close();
        return false;
      }

      m_blockSizeCompressed = CalculateCompressedBlockBufferSize(m_blockSize);
      m_bufferCompressed = new char[m_blockSizeCompressed];
      m_blocksCache = new DXFileManagerBlockCache(48, m_blockSize+3); // 3 more bytes for 'lzo1x_decompress_asm_fast'
      m_bufferWork = new char[LZO1X_1_15_MEM_COMPRESS];
      
      if (!m_bufferCompressed || !m_bufferWork)
      {
        Close();
        return false;
      }

      ReadFunction = &DXFileManagerBase::ReadCompressed;
    }
    else
    {
      ReadFunction = &DXFileManagerBase::ReadDisc;
    }
  }
  
  m_filename = filename;
  m_isOpened = true;
  m_isReadOnly = true;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::OpenWrite(const string& filename, bool enableCompression, unsigned int endOfRawHeaderPosition, unsigned int blockSize)
{
  Close();

  if (!FS_OpenWrite(filename))
  {
    return false;
  }
  else
  {
    m_positionRawHeaderDisplacement = endOfRawHeaderPosition;
    m_positionUncompressed = endOfRawHeaderPosition;
    m_positionCompressed = endOfRawHeaderPosition + (enableCompression ? sizeof(CompressionHeaderInDisk) : 0);
    FS_Seek(m_positionCompressed);

    m_sizeUncompressed = m_positionUncompressed;
    m_sizeCompressed = m_positionCompressed;

    if (enableCompression)
    {
      m_compressed = true;

      m_blockSize = blockSize;
      if (!m_blockSize)
      {
        m_blockSize = DXFileManagerBase::LZO_DEFAULT_BUFFER_SIZE;
      }

      if (lzo_init() != LZO_E_OK)
      {
        Close();
        return false;
      }

      m_blockSizeCompressed = CalculateCompressedBlockBufferSize(m_blockSize);
      m_bufferCompressed = new char[m_blockSizeCompressed];
      m_bufferUncompressed = new char[m_blockSize];
      m_bufferWork = new char[LZO1X_1_15_MEM_COMPRESS];

      if (!m_bufferCompressed || !m_bufferUncompressed || !m_bufferWork)
      {
        Close();
        return false;
      }

      ReadFunction = &DXFileManagerBase::ReadCompressed;
      WriteFunction = &DXFileManagerBase::WriteCompressed;
    }
    else
    {
      ReadFunction = &DXFileManagerBase::ReadDisc;
      WriteFunction = &DXFileManagerBase::WriteDisc;
    }
  }

  m_filename = filename;
  m_isOpened = true;
  m_isReadOnly = false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerBase::Close()
{
  if (m_isOpened && !m_isReadOnly)
  {
    WriteCompressedDisc();
    WriteCompressedMetaData();
  }

  FS_Close();

  m_filename = "";
  m_compressed = false;
  m_isOpened = false;
  m_isReadOnly = false;

  m_positionRawHeaderDisplacement = 0;
  m_positionUncompressed = 0;
  m_positionCompressed = 0;
  m_sizeUncompressed = 0;
  m_sizeCompressed = 0;

  m_bufferUncompressedPosition = 0;
  m_bufferUncompressedPositionEnd = 0;

  if (m_blocksCache)
  {
    delete m_blocksCache;
    m_blocksCache = NULL;
    m_bufferUncompressed = NULL;
  }
  
  if (m_bufferUncompressed)
  {
    delete[] m_bufferUncompressed;
    m_bufferUncompressed = NULL;
  }

  if (m_bufferCompressed)
  {
    delete[] m_bufferCompressed;
    m_bufferCompressed = NULL;
  }

  if (m_bufferWork)
  {
    delete[] m_bufferWork;
    m_bufferWork = NULL;
  }

  m_blocksPositions.clear();

  ReadFunction = NULL;
  WriteFunction = NULL;
}

////////////////////////////////////////////////////////////////////////////////

dx_uint64 DXFileManagerBase::GetSizeUncompressed() const
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    return 0;
  }
#endif // ifdef _DEBUG

  return m_sizeUncompressed;
}

////////////////////////////////////////////////////////////////////////////////

dx_uint64 DXFileManagerBase::GetSizeCompressed() const
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    return 0;
  }
#endif // ifdef _DEBUG

  if (m_compressed)
    return m_sizeCompressed + (m_isReadOnly ? 0 : static_cast<unsigned int>(m_blocksPositions.size()) * sizeof(dx_uint64));
  else
    return m_sizeUncompressed;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerBase::SeekRead(dx_uint64 position)
{
  // If don't need to move (sequential reads), returns inmediatly
  if (m_positionUncompressed == position + m_positionRawHeaderDisplacement)
  {
    return;
  }

  // Truncate the desired position if is past the end of the file
  if (position >= m_sizeUncompressed)
  {
    position = m_sizeUncompressed - m_positionRawHeaderDisplacement;
  }

  if (m_compressed)
  {
    // If the file is compressed and current block != wanted block
    if (((m_positionUncompressed - m_positionRawHeaderDisplacement) / m_blockSize) != (position / m_blockSize))
    {
      // Empty the current block to force to load the wanted block in the next
      // read operation.
      m_bufferUncompressedPosition = 0;
      m_bufferUncompressedPositionEnd = 0;
    }
    // If the file is compressed and current block == wanted block
    else if (m_bufferUncompressedPosition != m_bufferUncompressedPositionEnd)
    {
      // Move the offset into the current block
      m_bufferUncompressedPosition = static_cast<unsigned int>(position % m_blockSize);
    }
  }
  else
  {
    FS_Seek(m_positionRawHeaderDisplacement + position);
  }  

  m_positionUncompressed = m_positionRawHeaderDisplacement + position;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerBase::SeekWrite(dx_uint64 position)
{
  if (m_isReadOnly || m_compressed)
  {
    return;
  }

  position += m_positionRawHeaderDisplacement;
  FS_Seek(position);
  m_positionUncompressed = position;
}

////////////////////////////////////////////////////////////////////////////////

dx_uint64 DXFileManagerBase::Tell() const
{
  return m_positionUncompressed - m_positionRawHeaderDisplacement;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::ReadCompressed(char* buffer, unsigned int* size)
{
#ifdef _DEBUG
  if (!IsOpened() || !*size)
  {
    return false;
  }

  if (*size > (m_sizeUncompressed - m_positionUncompressed))
  {
    unsigned int remaining = static_cast<unsigned int>(m_sizeUncompressed - m_positionUncompressed);
    if (!(*size = remaining))
    {
      return false;
    }
  }
#endif // ifdef _DEBUG

  bool problems = false;
  unsigned int readedBytes = 0;
  unsigned int remaining = m_bufferUncompressedPositionEnd - m_bufferUncompressedPosition;
  
  if (!remaining)
  {
    if (!ReadCompressedDisc())
    {
      *size = 0;
      return false;
    }

    m_bufferUncompressedPosition = static_cast<unsigned int>((m_positionUncompressed - m_positionRawHeaderDisplacement) % m_blockSize);
    remaining = m_bufferUncompressedPositionEnd - m_bufferUncompressedPosition;
  }

  while (!problems && remaining < *size)
  {
    ReadCompressedMemory(buffer, remaining);
    problems = !ReadCompressedDisc();

    *size -= remaining;
    buffer += remaining;
    readedBytes += remaining;
    remaining = m_bufferUncompressedPositionEnd;
  }

  if (!problems && *size)
  {
    ReadCompressedMemory(buffer, *size);
    readedBytes += *size;
  }

  *size = readedBytes;
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::WriteCompressed(const char* buffer, unsigned int* size)
{
#ifdef _DEBUG
  if (!IsOpened() || m_isReadOnly || !*size)
  {
    return false;
  }
#endif // ifdef _DEBUG

  bool problems = false;
  unsigned int writedBytes = 0;
  unsigned int remaining = m_blockSize - m_bufferUncompressedPosition;
  
  while (!problems && remaining <= *size)
  {
    WriteCompressedMemory(buffer, remaining);
    problems = !WriteCompressedDisc();

    *size -= remaining;
    buffer += remaining;
    writedBytes += remaining;
    remaining = m_blockSize;
  }

  if (!problems && *size)
  {
    WriteCompressedMemory(buffer, *size);
    writedBytes += *size;
  }

  *size = writedBytes;
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerBase::ReadCompressedMemory(char* buffer, unsigned int size)
{
  memcpy(buffer, &m_bufferUncompressed[m_bufferUncompressedPosition], size);
  m_bufferUncompressedPosition += size;
  m_positionUncompressed += size;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerBase::WriteCompressedMemory(const char* buffer, unsigned int size)
{
  memcpy(&m_bufferUncompressed[m_bufferUncompressedPosition], buffer, size);
  m_bufferUncompressedPosition += size;
  m_positionUncompressed += size;
  m_sizeUncompressed = max(m_sizeUncompressed, m_positionUncompressed);
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::ReadCompressedDisc()
{
  m_bufferUncompressedPosition = 0;
  m_bufferUncompressedPositionEnd = 0;
  
  //////////////////////////////////////////////////////////////////////////////
  
  unsigned int currentBlock = static_cast<unsigned int>((m_positionUncompressed - m_positionRawHeaderDisplacement) / m_blockSize);

  //////////////////////////////////////////////////////////////////////////////
  
  if (m_blocksCache->GetBlockBuffer(currentBlock, &m_bufferUncompressed, &m_bufferUncompressedPositionEnd))
  {
    return true;
  }

  m_positionCompressed = m_blocksPositions[currentBlock];
  FS_Seek(m_positionCompressed);

  //////////////////////////////////////////////////////////////////////////////
  // Read from disk to memory
  //////////////////////////////////////////////////////////////////////////////

  lzo_uint sizeCompressed = 0;
  unsigned int readedBytes = sizeof(lzo_uint);
  if (!ReadDisc((char*) &sizeCompressed, &readedBytes))
  {
    return false;
  }  
  sizeCompressed -= sizeof(lzo_uint);

  readedBytes = sizeCompressed;
  if (!ReadDisc(m_bufferCompressed, &readedBytes))
  {
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Decompress from memory to memory
  //////////////////////////////////////////////////////////////////////////////

  lzo_uint sizeUncompressed = 0;
  int m_error = lzo1x_decompress_asm_fast((const lzo_bytep) m_bufferCompressed, sizeCompressed, (lzo_bytep) m_bufferUncompressed, &sizeUncompressed, m_bufferWork);
  m_bufferUncompressedPosition = 0;
  m_bufferUncompressedPositionEnd = sizeUncompressed;
  m_blocksCache->SetBlockBufferSize(currentBlock, sizeUncompressed);

  //////////////////////////////////////////////////////////////////////////////

  return (m_error == LZO_E_OK);
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::WriteCompressedDisc()
{
  if (!m_bufferUncompressedPosition)
  {
    return true;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Compress from memory to memory
  //////////////////////////////////////////////////////////////////////////////

  lzo_uint* sizeCompressed = (lzo_uint*) &m_bufferCompressed[0];
  *sizeCompressed = 0;
  int m_error = lzo1x_1_15_compress((const unsigned char*) m_bufferUncompressed, m_bufferUncompressedPosition, (unsigned char*) &m_bufferCompressed[sizeof(lzo_uint)], sizeCompressed, m_bufferWork);
  *sizeCompressed += sizeof(lzo_uint);
  
  if (m_error != LZO_E_OK)
  {
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Write from memory to disk
  //////////////////////////////////////////////////////////////////////////////

  unsigned int writedBytes = *sizeCompressed;
  m_blocksPositions.push_back(m_positionCompressed);
  if (!WriteDisc(m_bufferCompressed, &writedBytes))
  {
    m_blocksPositions.pop_back();
    return false;
  }
  m_bufferUncompressedPosition = 0;

  //////////////////////////////////////////////////////////////////////////////
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::ReadDisc(char* buffer, unsigned int* size)
{
#ifdef _DEBUG
  if (!IsOpened() || !*size)
  {
    return false;
  }

  if (*size > (m_compressed ? m_sizeCompressed - m_positionCompressed : m_sizeUncompressed - m_positionUncompressed))
  {
    unsigned int remaining = static_cast<unsigned int>(m_compressed ? m_sizeCompressed - m_positionCompressed : m_sizeUncompressed - m_positionUncompressed);
    if (!(*size = remaining))
    {
      return false;
    }
  }
#endif // ifdef _DEBUG

  bool result = FS_Read(buffer, size);

  if (m_compressed)
  {
    m_positionCompressed += *size;
  }
  else
  {
    m_positionUncompressed += *size;
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::WriteDisc(const char* buffer, unsigned int* size)
{
#ifdef _DEBUG
  if (!IsOpened() || m_isReadOnly || !*size)
  {
    return false;
  }
#endif // ifdef _DEBUG

  bool result = FS_Write(buffer, size);
  
  if (m_compressed)
  {
    m_positionCompressed += *size;
    m_sizeCompressed = max(m_sizeCompressed, m_positionCompressed);
  }
  else
  {
    m_positionUncompressed += *size;
    m_sizeUncompressed = max(m_sizeUncompressed, m_positionUncompressed);
  }

  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::ReadRaw(dx_uint64 position, char* buffer, unsigned int* size)
{
  dx_uint64 lastPosition = FS_Tell();
  FS_Seek(position);
  bool result = FS_Read(buffer, size);
  FS_Seek(lastPosition);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerBase::WriteRaw(dx_uint64 position, const char* buffer, unsigned int* size)
{
  dx_uint64 lastPosition = FS_Tell();
  FS_Seek(position);
  bool result = FS_Write(buffer, size);
  FS_Seek(lastPosition);
  return result;
}

////////////////////////////////////////////////////////////////////////////////

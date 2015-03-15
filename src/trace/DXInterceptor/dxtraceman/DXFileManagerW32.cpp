////////////////////////////////////////////////////////////////////////////////

#include <vector>
#include <string>
#include <shlwapi.h>
#include "DXFileManagerW32.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

DXFileManagerW32::DXFileManagerW32() :
m_fileHandle(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

DXFileManagerW32::~DXFileManagerW32()
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerW32::FS_OpenRead(const string& filename)
{
  m_fileHandle = CreateFileA(filename.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, NULL);
  if (m_fileHandle == INVALID_HANDLE_VALUE)
  {
    m_fileHandle = NULL;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerW32::FS_OpenWrite(const string& filename)
{
  m_fileHandle = CreateFileA(filename.c_str(), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
  if (m_fileHandle == INVALID_HANDLE_VALUE)
  {
    m_fileHandle = NULL;
    return false;
  }
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerW32::FS_Close()
{
  if (m_fileHandle)
  {
    FlushFileBuffers(m_fileHandle);
    CloseHandle(m_fileHandle);
    m_fileHandle = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

dx_uint64 DXFileManagerW32::FS_FileSize()
{
  LARGE_INTEGER filesize;
  if (!GetFileSizeEx(m_fileHandle, &filesize))
  {
    return 0;
  }
  else
  {
    return filesize.QuadPart;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerW32::FS_Seek(dx_uint64 position)
{
  LARGE_INTEGER positionLarge;
  positionLarge.QuadPart = position;
  SetFilePointerEx(m_fileHandle, positionLarge, NULL, FILE_BEGIN);
}

////////////////////////////////////////////////////////////////////////////////

dx_uint64 DXFileManagerW32::FS_Tell()
{
  LARGE_INTEGER movement;
  movement.QuadPart = 0;
  LARGE_INTEGER position;
  SetFilePointerEx(m_fileHandle, movement, &position, FILE_CURRENT);
  return position.QuadPart;
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerW32::FS_Read(char* buffer, unsigned int* size)
{
  DWORD readedBytes;
  if (!ReadFile(m_fileHandle, buffer, *size, &readedBytes, NULL))
  {
    *size = 0;
    return false;
  }
  else
  {
    *size = readedBytes;
    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerW32::FS_Write(const char* buffer, unsigned int* size)
{
  DWORD writedBytes;
  if (!WriteFile(m_fileHandle, buffer, *size, &writedBytes, NULL))
  {
    *size = 0;
    return false;
  }
  else
  {
    *size = writedBytes;
    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////

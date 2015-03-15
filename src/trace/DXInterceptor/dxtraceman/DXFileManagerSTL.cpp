////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <vector>
#include <iostream>
#include "DXFileManagerSTL.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

DXFileManagerSTL::DXFileManagerSTL() :
m_isReadOnly(true)
{
}

////////////////////////////////////////////////////////////////////////////////

DXFileManagerSTL::~DXFileManagerSTL()
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerSTL::FS_OpenRead(const string& filename)
{
  FS_Close();
  m_file.open(filename.c_str(), ios_base::in | ios_base::binary);
  m_isReadOnly = true;
  return m_file.is_open();
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerSTL::FS_OpenWrite(const string& filename)
{
  FS_Close();
  m_file.open(filename.c_str(), ios_base::in | ios_base::out | ios_base::trunc | ios_base::binary);
  m_isReadOnly = false;
  return m_file.is_open();
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerSTL::FS_Close()
{
  if (m_file.is_open())
  {
    m_file.close();
  }
  m_isReadOnly = true;
}

////////////////////////////////////////////////////////////////////////////////

dx_uint64 DXFileManagerSTL::FS_FileSize()
{
  streamoff positionOld = m_file.tellg();
  m_file.seekg(0, ios_base::end);
  dx_uint64 position = m_file.tellg();
  m_file.seekg(positionOld, ios_base::beg);  
  return position;
}

////////////////////////////////////////////////////////////////////////////////

void DXFileManagerSTL::FS_Seek(dx_uint64 position)
{
  m_file.seekg(static_cast<streamoff>(position), ios_base::beg);
  m_file.seekp(static_cast<streamoff>(position), ios_base::beg);
}

////////////////////////////////////////////////////////////////////////////////

dx_uint64 DXFileManagerSTL::FS_Tell()
{
  if (m_isReadOnly)
  {
    return m_file.tellg();
  }
  else
  {
    return m_file.tellp();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerSTL::FS_Read(char* buffer, unsigned int* size)
{
  m_file.read(buffer, *size);
  if (m_file.good())
  {
    *size = m_file.gcount();
    return true;
  }
  else
  {
    *size = 0;
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXFileManagerSTL::FS_Write(const char* buffer, unsigned int* size)
{
  m_file.write(buffer, *size);
  if (m_file.good())
  {
    return true;
  }
  else
  {
    *size = 0;
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

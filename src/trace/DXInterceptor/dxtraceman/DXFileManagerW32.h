////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "DXFileManagerBase.h"

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXFileManagerW32 : public DXFileManagerBase
  {
  public:

    DXFileManagerW32();
    virtual ~DXFileManagerW32();
  
  protected:

    HANDLE m_fileHandle;
    
    bool FS_OpenRead(const std::string& filename);
    bool FS_OpenWrite(const std::string& filename);
    void FS_Close();
    dx_uint64 FS_FileSize();
    void FS_Seek(dx_uint64 position);
    dx_uint64 FS_Tell();
    bool FS_Read(char* buffer, unsigned int* size);
    bool FS_Write(const char* buffer, unsigned int* size);

  };
}

////////////////////////////////////////////////////////////////////////////////

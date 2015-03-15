////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace dxcodegen
{
  namespace Utilities
  {
    class FileSystem
    {
    public:
    
      static bool DirectoryExists(std::string path);
      static bool FilenameExists(std::string filename);
      static bool FilenameReadOnly(std::string filename);
      static bool CreateDirectory(std::string path, bool recursive=false);
      static std::string GetCurrentDirectory();
      static void DirectoryAddBackslash(std::string& path);

    protected:

      static bool IsValid(DWORD flags);
      static bool IsDirectory(DWORD flags);
      static bool IsFilename(DWORD flags);
      static bool IsFilenameReadOnly(DWORD flags);

    };
  }
}

////////////////////////////////////////////////////////////////////////////////
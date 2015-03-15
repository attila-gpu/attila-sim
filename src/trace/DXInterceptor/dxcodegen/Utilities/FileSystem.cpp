////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Utilities/FileSystem.h"

using namespace std;
using namespace dxcodegen::Utilities;

////////////////////////////////////////////////////////////////////////////////

bool FileSystem::DirectoryExists(string path)
{
  DWORD flags = ::GetFileAttributes(path.c_str());
  return IsValid(flags) && IsDirectory(flags);
}

////////////////////////////////////////////////////////////////////////////////

bool FileSystem::FilenameExists(string filename)
{
  DWORD flags = ::GetFileAttributes(filename.c_str());
  return IsValid(flags) && IsFilename(flags);
}

////////////////////////////////////////////////////////////////////////////////

bool FileSystem::FilenameReadOnly(string filename)
{
  DWORD flags = ::GetFileAttributes(filename.c_str());
  return IsValid(flags) && IsFilenameReadOnly(flags);
}

////////////////////////////////////////////////////////////////////////////////

bool FileSystem::CreateDirectory(string path, bool recursive)
{
  return (::SHCreateDirectoryEx(NULL, path.c_str(), NULL) == ERROR_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////

string FileSystem::GetCurrentDirectory()
{
  static char cadena[MAX_PATH];
  string path;
  if (::GetCurrentDirectory(MAX_PATH, cadena))
  {
    path = cadena;
    if (path[path.length()-1] != '\\')
    {
      path += '\\';
    }
  }
  return path;
}

////////////////////////////////////////////////////////////////////////////////

void FileSystem::DirectoryAddBackslash(string& path)
{
  if (!path.empty() && path[path.length()-1] != '\\')
  {
    path += '\\';
  }
}


////////////////////////////////////////////////////////////////////////////////

bool FileSystem::IsValid(DWORD flags)
{
  return (flags != INVALID_FILE_ATTRIBUTES);
}

////////////////////////////////////////////////////////////////////////////////

bool FileSystem::IsDirectory(DWORD flags)
{
  return ((flags & FILE_ATTRIBUTE_DIRECTORY) != 0) && ((flags & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) == 0);
}

////////////////////////////////////////////////////////////////////////////////

bool FileSystem::IsFilename(DWORD flags)
{
  return ((flags & FILE_ATTRIBUTE_DIRECTORY) == 0) && ((flags & (FILE_ATTRIBUTE_ARCHIVE | FILE_ATTRIBUTE_NORMAL)) != 0);
}

////////////////////////////////////////////////////////////////////////////////

bool FileSystem::IsFilenameReadOnly(DWORD flags)
{
  return IsFilename(flags) && ((flags & FILE_ATTRIBUTE_READONLY) != 0);
}

////////////////////////////////////////////////////////////////////////////////

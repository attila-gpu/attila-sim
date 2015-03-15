////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MD5.h"
#include "DXBasicTypes.h"
#include "DXTypeHelper.h"
#include "DXMethodCallHelper.h"
#include "DXMethodCall.h"
#include "DXBuffer.h"
#include "DXTexture.h"
#include "DXStatistic.h"
#include "DXFileManagerW32.h"
#include "DXFileManagerSTL.h"
#include "DXIndexManagerOptimized.h"
#include "DXIndexManagerNotOptimized.h"
#include "DXTraceManager.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

const string DXTraceManager::PROJECT_FILE_EXTENSION = ".DXIntRun";
const unsigned int DXTraceManager::LIBRARY_VERSION = 1000; // 1.000

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(4)

struct DXTraceManager::HeaderInDisk
{
public:

  static const string HEADER_MAGIC_NUMBER;
  
  BYTE   MagicNumber[8];
  DWORD  Version;
  time_t CreationDate;

  HeaderInDisk() :
  Version(0),
  CreationDate(0)
  {
    ::memset(MagicNumber, 0, sizeof(MagicNumber));
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
    ::memset(MagicNumber, 0, sizeof(MagicNumber));
    Version = 0;
    CreationDate = 0;
  }

};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

const string DXTraceManager::HeaderInDisk::HEADER_MAGIC_NUMBER = "DXINTRUN";

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(4)

struct DXTraceManager::ProjectInformationInDisk
{
public:
  
  BYTE GameName[32];
  BYTE Anotations[256];

  ProjectInformationInDisk()
  {
    Clear();
  }

  void Clear()
  {
    ::memset((char*) GameName, 0, sizeof(GameName));
    ::memset((char*) Anotations, 0, sizeof(Anotations));
  }

};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(1)

struct DXTraceManager::OptionsInDisk
{
public:  
  
  bool Compressed : 8;
  bool Optimized : 8;
  ULONGLONG IndexBlockPositionMethodCall : 64;
  ULONGLONG IndexBlockPositionBuffer : 64;
  ULONGLONG IndexBlockPositionTexture : 64;
  ULONGLONG IndexBlockPositionStatistic : 64;

  OptionsInDisk() :
  Compressed(false),
  Optimized(false),
  IndexBlockPositionMethodCall(0),
  IndexBlockPositionBuffer(0),
  IndexBlockPositionTexture(0),
  IndexBlockPositionStatistic(0)
  {
  }

};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

#pragma pack(push)
#pragma pack(4)

struct DXTraceManager::CountersInDisk
{
public:

  DWORD FramesCount;
  DWORD MethodCallsCount;
  DWORD BuffersCount;
  DWORD TexturesCount;
  DWORD StatisticsCount;
  ULONGLONG UncompressedFileSize;
  
  CountersInDisk() :
  FramesCount(0),
  MethodCallsCount(0),
  BuffersCount(0),
  TexturesCount(0),
  StatisticsCount(0),
  UncompressedFileSize(0)
  {
  }

};

#pragma pack(pop)

////////////////////////////////////////////////////////////////////////////////

DXTraceManager::DXTraceManager() :
m_isOpened(false),
m_isReadOnly(true),
m_indexManager(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

DXTraceManager::~DXTraceManager()
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::OpenRead(const string& filename)
{
  if (!Close())
  {
    return false;
  }

  ResetErrorCode();
  
  bool problems = !ReadMetaData(filename);

  if (!problems)
  {
    problems = !OpenFile(filename, true, m_isCompressed);
  }

  if (!problems)
  {
    m_isOpened = true;
    m_isReadOnly = true;
    m_projectFileName = filename;
    
    problems = !InitRead();
  }

  if (problems)
  {
    Close();
    return false;
  }
  else
  {
    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::OpenWrite(const string& destinationPath, const string& projectName, bool compressed)
{
  if (!Close())
  {
    return false;
  }
  
  ResetErrorCode();
  
  string destinationPathTemp = destinationPath;
  CorrectDestinationPath(destinationPathTemp);
  string filename = destinationPathTemp + GetGameNameFromProjectName(projectName) + PROJECT_FILE_EXTENSION;
  
  bool problems = false;
  
  if (!problems)
  {
    problems = !OpenFile(filename, false, compressed);
  }

  if (!problems)
  {
    m_isOpened = true;
    m_isReadOnly = false;
    m_projectFileName = filename;
    m_projectGameName = GetGameNameFromProjectName(projectName);
    m_creationDate = time(NULL);
    m_isCompressed = compressed;
    
    problems = !WriteMetaData();
  }
  
  if (problems)
  {
    Close();
    return false;
  }
  else
  {
    return true;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::Close()
{
  bool problems = false;
  
  if (IsOpened() && !IsReadOnly())
  {
    problems = !WriteCounters();
  }

  m_isOpened = false;
  m_isReadOnly = true;

  m_creationDate = 0;
  m_projectFileName.clear();
  m_projectGameName.clear();
  m_projectAnotations.clear();

  if (m_indexManager)
  {
    m_indexManager->Close();
    delete m_indexManager;
    m_indexManager = NULL;
  }
  
  m_isCompressed = false;
  m_isOptimized = false;
  m_indexBlockPositionMethodCall = 0;
  m_indexBlockPositionBuffer = 0;
  m_indexBlockPositionTexture = 0;
  m_indexBlockPositionStatistic = 0;
  
  m_numFrames = 0;
  m_numMethodCalls = 0;
  m_numBuffers = 0;
  m_numTextures = 0;

  CloseFile();

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTraceManager::GetErrorCode() const
{
  return (unsigned int) m_errorCode;
}

////////////////////////////////////////////////////////////////////////////////

string DXTraceManager::GetErrorMessage(unsigned int code) const
{
  switch ((ErrorType) code)
  {
  case DXTM_NOERROR:
    return "no errors detected";
    break;
  case DXTM_PROJECT_NOT_OPENED:
    return "project not opened";
    break;
  case DXTM_WRITE_IN_READ_MODE:
    return "could'nt write in read only mode";
    break;
  case DXTM_READ_IN_WRITE_MODE:
    return "could'nt read in write only mode";
    break;
  case DXTM_CANT_OPEN_FILE:
    return "could'nt open project file";
    break;
  case DXTM_ERROR_LAST_FILE_OPERATION:
    return "error on last operation in project file";
    break;
  case DXTM_ERROR_LOADING_FILE_INDEXS:
    return "error loading file indexs";
    break;
  case DXTM_ERROR_INVALID_HEADER:
    return "found an invalid file header";
    break;
  case DXTM_ERROR_SERIALIZE_METHODCALL:
    return "error serializing a method call";
    break;
  case DXTM_ERROR_DESERIALIZE_METHODCALL:
    return "error deserializing a method call";
    break;
  case DXTM_ERROR_SERIALIZE_BUFFER:
    return "error serializing a buffer";
    break;
  case DXTM_ERROR_DESERIALIZE_BUFFER:
    return "error deserializing a buffer";
    break;
  case DXTM_ERROR_SERIALIZE_TEXTURE:
    return "error serializing a texture";
    break;
  case DXTM_ERROR_DESERIALIZE_TEXTURE:
    return "error deserializing a texture";
    break;
  case DXTM_ERROR_SERIALIZE_STATISTIC:
    return "error serializing a statistic";
    break;
  case DXTM_ERROR_DESERIALIZE_STATISTIC:
    return "error deserializing a statistic";
    break;
  case DXTM_ERROR_NO_MEMORY:
    return "could'nt allocate memory";
    break;
  default:
    return "unknow error code";
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

string DXTraceManager::GetCurrentErrorMessage() const
{
  return GetErrorMessage(m_errorCode);
}

////////////////////////////////////////////////////////////////////////////////

time_t DXTraceManager::GetCreationDate() const
{
  return m_creationDate;
}

////////////////////////////////////////////////////////////////////////////////

const string& DXTraceManager::GetProjectFileName() const
{
  return m_projectFileName;
}

////////////////////////////////////////////////////////////////////////////////

const string& DXTraceManager::GetProjectGameName() const
{
  return m_projectGameName;
}

////////////////////////////////////////////////////////////////////////////////

const string& DXTraceManager::GetProjectAnotations() const
{
  return m_projectAnotations;
}

////////////////////////////////////////////////////////////////////////////////

string DXTraceManager::GetOptionsString() const
{
  ostringstream str;

  str << endl;
  str << "DXTraceManager Options:" << endl;
  str << "-----------------------" << endl;
  str << "Project Filename: " << m_projectFileName << endl;
  str << "Compressed:       " << (m_isCompressed ? "TRUE" : "FALSE") << endl;
  str << endl;

  return str.str();
}

////////////////////////////////////////////////////////////////////////////////

string DXTraceManager::GetCountersString() const
{
  ostringstream str;

  str << "DXTraceManager Counters:" << endl;
  str << "------------------------" << endl;
  str << "Frames:       " << setw(10) << setfill(' ') << GetFrameCount() << endl;
  str << "Method Calls: " << setw(10) << setfill(' ') << GetMethodCallCount() << endl;
  str << "Buffers:      " << setw(10) << setfill(' ') << GetBufferCount() << endl;
  str << "Textures:     " << setw(10) << setfill(' ') << GetTextureCount() << endl;
  str << "Statistics:   " << setw(10) << setfill(' ') << GetStatisticCount() << endl;
  str << "Size Uncomp.: " << setw(10) << setfill(' ') << fixed << setprecision(2) << (GetFileSize()       >= 1024*1024*1024 ? GetFileSize()       / (1024.0 * 1024.0 * 1024.0) : GetFileSize()       / (1024.0 * 1024.0)) << (GetFileSize()       >= 1024*1024*1024 ? " Gb" : " Mb") << endl;
  str << "Size Comp.:   " << setw(10) << setfill(' ') << fixed << setprecision(2) << (GetFileSizeInDisk() >= 1024*1024*1024 ? GetFileSizeInDisk() / (1024.0 * 1024.0 * 1024.0) : GetFileSizeInDisk() / (1024.0 * 1024.0)) << (GetFileSizeInDisk() >= 1024*1024*1024 ? " Gb (" : " Mb (") << fixed << setprecision(2) << (GetFileSize() ? GetFileSizeInDisk() * 100.0 / GetFileSize() : 0.0) << "%)" << endl;
  str << endl;

  return str.str();
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTraceManager::GetFrameCount() const
{
  return m_numFrames;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTraceManager::GetMethodCallCount() const
{
  return m_numMethodCalls;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTraceManager::GetBufferCount() const
{
  return m_numBuffers;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTraceManager::GetTextureCount() const
{
  return m_numTextures;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXTraceManager::GetStatisticCount() const
{
  return m_numStatistics;
}

////////////////////////////////////////////////////////////////////////////////

ULONGLONG DXTraceManager::GetFileSize() const
{
  return m_file.GetSizeUncompressed();
}

////////////////////////////////////////////////////////////////////////////////

ULONGLONG DXTraceManager::GetFileSizeInDisk() const
{
  return m_file.GetSizeCompressed();
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::IsOpened() const
{
  return m_isOpened;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::IsReadOnly() const
{
  return m_isReadOnly;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::IsCompressed() const
{
  return m_isCompressed;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::IsOptimized() const
{
  return m_isOptimized;
}

////////////////////////////////////////////////////////////////////////////////

void DXTraceManager::ResetErrorCode()
{
  m_errorCode = DXTM_NOERROR;
}

////////////////////////////////////////////////////////////////////////////////

string DXTraceManager::CorrectDestinationPath(const string& destinationPath)
{
  if (!destinationPath.empty())
  {
    TCHAR szDestinationPath[MAX_PATH];
    _tcsncpy(szDestinationPath, destinationPath.c_str(), MAX_PATH);
    ::PathAddBackslash(szDestinationPath);
    return szDestinationPath;
  }
  else
  {
    return destinationPath;
  }
}

////////////////////////////////////////////////////////////////////////////////

string DXTraceManager::GetGameNameFromProjectName(const string& projectName)
{
  TCHAR szGameName[MAX_PATH];
  _tcsncpy(szGameName, projectName.c_str(), MAX_PATH);
  ::PathStripPath(szGameName);
  return string(szGameName);
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::InitRead()
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (!IsReadOnly())
  {
    m_errorCode = DXTM_READ_IN_WRITE_MODE;
    return false;
  }
#endif // ifdef _DEBUG

  if (m_isOptimized)
    m_indexManager = new DXIndexManagerOptimized(*this);
  else
    m_indexManager = new DXIndexManagerNotOptimized(*this);

  if (!m_indexManager)
  {
    m_errorCode = DXTM_ERROR_NO_MEMORY;
    return false;
  }

  if (!m_indexManager->Init())
  {
    m_errorCode = DXTM_ERROR_LOADING_FILE_INDEXS;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::OpenFile(const string& filename, bool readOnly, bool compressed)
{
  unsigned int headersSize = sizeof(HeaderInDisk)+
                             sizeof(ProjectInformationInDisk)+
                             sizeof(OptionsInDisk)+
                             sizeof(CountersInDisk);

  if (readOnly)
  {
    if (!m_file.OpenRead(filename, compressed, headersSize))
    {
      m_errorCode = DXTM_CANT_OPEN_FILE;
      return false;
    }
  }
  else
  {
    if (!m_file.OpenWrite(filename, compressed, headersSize, 64*1024))
    {
      m_errorCode = DXTM_CANT_OPEN_FILE;
      return false;
    }
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXTraceManager::CloseFile()
{
  m_file.Close();
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadMetaData(const string& filename)
{
  DXFileManagerType file;
  
  if (!file.OpenRead(filename))
  {
    m_errorCode = DXTM_CANT_OPEN_FILE;
    return false;
  }
  
  bool problems = false;
  
  if (!ReadHeader(&file)) problems = true;
  if (!ReadProjectInformation(&file)) problems = true;
  if (!ReadOptions(&file)) problems = true;
  if (!ReadCounters(&file)) problems = true;
  
  file.Close();
  
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteMetaData()
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (IsReadOnly())
  {
    m_errorCode = DXTM_WRITE_IN_READ_MODE;
    return false;
  }
#endif // ifdef _DEBUG
  
  if (!WriteHeader()) return false;
  if (!WriteProjectInformation()) return false;
  if (!WriteOptions()) return false;
  if (!WriteCounters()) return false;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadHeader(DXFileManagerType* file)
{
  HeaderInDisk header;

  if (!file->ReadRaw(0, (char*) &header, sizeof(HeaderInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }
  
  if (!header.CheckMagicNumber())
  {
    m_errorCode = DXTM_ERROR_INVALID_HEADER;
    return false;
  }

  if (header.Version > LIBRARY_VERSION)
  {
    m_errorCode = DXTM_ERROR_INVALID_HEADER;
    return false;
  }

  m_creationDate = header.CreationDate;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteHeader()
{
  HeaderInDisk header;

  header.SetMagicNumber();
  header.Version = LIBRARY_VERSION;
  header.CreationDate = m_creationDate;

  if (!m_file.WriteRaw(0, (const char*) &header, sizeof(HeaderInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadProjectInformation(DXFileManagerType* file)
{
  ProjectInformationInDisk projectInfo;
  
  if (!file->ReadRaw(sizeof(HeaderInDisk), (char*) &projectInfo, sizeof(ProjectInformationInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }
  
  int position;
    
  for (position = sizeof(projectInfo.GameName)-1; position >= 0 && projectInfo.GameName[position] == '\0'; position--);
  if (position > 0)
  {
    m_projectGameName.assign((const char*) projectInfo.GameName, position+1);
  }

  for (position = sizeof(projectInfo.Anotations)-1; position >= 0 && projectInfo.Anotations[position] == '\0'; position--);
  if (position > 0)
  {
    m_projectAnotations.assign((char*) projectInfo.Anotations, position+1);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteProjectInformation()
{
  ProjectInformationInDisk projectInfo;

  ::memset(projectInfo.GameName, 0, sizeof(projectInfo.GameName));
  if (m_projectGameName.length())
  {
    ::memcpy(projectInfo.GameName, m_projectGameName.c_str(), m_projectGameName.length() > sizeof(projectInfo.GameName) ? sizeof(projectInfo.GameName) : m_projectGameName.length());
  }

  ::memset(projectInfo.Anotations, 0, sizeof(projectInfo.Anotations));
  if (m_projectAnotations.length())
  {
    ::memcpy(projectInfo.Anotations, m_projectAnotations.c_str(), m_projectAnotations.length() > sizeof(projectInfo.Anotations) ? sizeof(projectInfo.Anotations) : m_projectAnotations.length());
  }

  if (!m_file.WriteRaw(sizeof(HeaderInDisk), (const char*) &projectInfo, sizeof(ProjectInformationInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadOptions(DXFileManagerType* file)
{
  OptionsInDisk options;
  
  if (!file->ReadRaw(sizeof(HeaderInDisk)+sizeof(ProjectInformationInDisk), (char*) &options, sizeof(OptionsInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  m_isCompressed = options.Compressed;
  m_isOptimized = options.Optimized;
  m_indexBlockPositionMethodCall = options.IndexBlockPositionMethodCall;
  m_indexBlockPositionBuffer = options.IndexBlockPositionBuffer;
  m_indexBlockPositionTexture = options.IndexBlockPositionTexture;
  m_indexBlockPositionStatistic = options.IndexBlockPositionStatistic;
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteOptions()
{
  OptionsInDisk options;

  options.Compressed = m_isCompressed;
  options.Optimized = m_isOptimized;
  options.IndexBlockPositionMethodCall = m_indexBlockPositionMethodCall;
  options.IndexBlockPositionBuffer = m_indexBlockPositionBuffer;
  options.IndexBlockPositionStatistic = m_indexBlockPositionStatistic;
  
  if (!m_file.WriteRaw(sizeof(HeaderInDisk)+sizeof(ProjectInformationInDisk), (const char*) &options, sizeof(OptionsInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadCounters(DXFileManagerType* file)
{
  CountersInDisk counters;
  ULONGLONG headerPosition = sizeof(HeaderInDisk)+
                             sizeof(ProjectInformationInDisk)+
                             sizeof(OptionsInDisk);

  if (!file->ReadRaw(headerPosition, (char*) &counters, sizeof(CountersInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  m_numFrames = counters.FramesCount;
  m_numMethodCalls = counters.MethodCallsCount;
  m_numBuffers = counters.BuffersCount;
  m_numTextures = counters.TexturesCount;
  m_numStatistics = counters.StatisticsCount;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteCounters()
{
  CountersInDisk counters;
  ULONGLONG headerPosition = sizeof(HeaderInDisk)+
                             sizeof(ProjectInformationInDisk)+
                             sizeof(OptionsInDisk);

  counters.FramesCount = m_numFrames;
  counters.MethodCallsCount = m_numMethodCalls;
  counters.BuffersCount = m_numBuffers;
  counters.TexturesCount = m_numTextures;
  counters.StatisticsCount = m_numStatistics;
  counters.UncompressedFileSize = m_file.GetSizeUncompressed();

  if (!m_file.WriteRaw(headerPosition, (const char*) &counters, sizeof(CountersInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadMethodCall(DXMethodCallPtr* call, unsigned int methodcall_id)
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (!IsReadOnly())
  {
    m_errorCode = DXTM_READ_IN_WRITE_MODE;
    return false;
  }
  
  if (methodcall_id >= GetMethodCallCount())
  {
    return false;
  }
#endif // ifdef _DEBUG

  *call = new DXMethodCall();
  
  IndexInMemory* index = m_indexManager->GetIndexMethodCall(methodcall_id);
  
  if (!ReadMethodCallFromDisk(*call, index->Position, index->Size))
  {
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteMethodCall(DXMethodCallPtr call)
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (IsReadOnly())
  {
    m_errorCode = DXTM_WRITE_IN_READ_MODE;
    return false;
  }
#endif // ifdef _DEBUG

  if (call->GetToken() == DXMethodCallHelper::TOK_IDirect3DDevice9_Present ||
      call->GetToken() == DXMethodCallHelper::TOK_IDirect3DSwapChain9_Present)
  {
    m_numFrames++;
  }
  
  if (!WriteMethodCallToDisk(call))
  {
    return false;
  }

  m_numMethodCalls++;

#ifdef _DEBUG
  WriteCounters();
#endif // ifdef _DEBUG
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadMethodCallFromDisk(DXMethodCallPtr call, ULONGLONG position, unsigned int size)
{
  char* deserializedMethodCall = NULL;
  
  if (!call->DeserializeFromBufferFastInit(&deserializedMethodCall, size))
  {
    m_errorCode = DXTraceManager::DXTM_ERROR_NO_MEMORY;
    return false;
  }
  
  m_file.SeekRead(position);
  if (!m_file.Read(deserializedMethodCall, size))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  if (!call->DeserializeFromBufferFast())
  {
    m_errorCode = DXTM_ERROR_DESERIALIZE_METHODCALL;
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteMethodCallToDisk(DXMethodCallPtr call)
{
  char* serializedCall;
  unsigned int serializedMethodCallSize;

  if (!call->SerializeToBuffer(&serializedCall, &serializedMethodCallSize))
  {
    m_errorCode = DXTM_ERROR_SERIALIZE_METHODCALL;
    return false;
  }

  MiniIndexInDisk index(IT_MethodCall, serializedMethodCallSize);

  if (!m_file.Write((const char*) &index, sizeof(MiniIndexInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  if (!m_file.Write((const char*) serializedCall, serializedMethodCallSize))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadBuffer(DXBufferPtr* buffer, unsigned int buffer_id)
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (!IsReadOnly())
  {
    m_errorCode = DXTM_READ_IN_WRITE_MODE;
    return false;
  }
  
  if (buffer_id >= GetBufferCount())
  {
    return false;
  }
#endif // ifdef _DEBUG

  *buffer = new DXBuffer();
  
  IndexInMemory* index = m_indexManager->GetIndexBuffer(buffer_id);
  
  if (!ReadBufferFromDisk(*buffer, index->Position, index->Size))
  {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteBuffer(DXBufferPtr buffer, unsigned int& buffer_id)
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (IsReadOnly())
  {
    m_errorCode = DXTM_WRITE_IN_READ_MODE;
    return false;
  }
#endif // ifdef _DEBUG

  if (!WriteBufferToDisk(buffer))
  {
    return false;
  }

  buffer_id = m_numBuffers++;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadBufferFromDisk(DXBufferPtr buffer, ULONGLONG position, unsigned int size)
{
  m_file.SeekRead(position);

  if (!buffer->DeserializeFromFile(&m_file, size))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteBufferToDisk(DXBufferPtr buffer)
{
  MiniIndexInDisk index(IT_Buffer, buffer->GetSerializedSize());

  if (!m_file.Write((const char*) &index, sizeof(MiniIndexInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  if (!buffer->SerializeToFile(&m_file))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadTexture(DXTexturePtr* texture, unsigned int texture_id)
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (!IsReadOnly())
  {
    m_errorCode = DXTM_READ_IN_WRITE_MODE;
    return false;
  }
  
  if (texture_id >= GetTextureCount())
  {
    return false;
  }
#endif // ifdef _DEBUG

  *texture = new DXTexture();

  IndexInMemory* index = m_indexManager->GetIndexTexture(texture_id);
  
  if (!ReadTextureFromDisk(*texture, index->Position, index->Size))
  {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteTexture(DXTexturePtr texture, unsigned int& texture_id)
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (IsReadOnly())
  {
    m_errorCode = DXTM_WRITE_IN_READ_MODE;
    return false;
  }
#endif // ifdef _DEBUG
  
  if (!WriteTextureToDisk(texture))
  {
    return false;
  }

  texture_id = m_numTextures++;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadTextureFromDisk(DXTexturePtr texture, ULONGLONG position, unsigned int size)
{
  m_file.SeekRead(position);
  
  if (!texture->DeserializeFromFile(&m_file, size))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteTextureToDisk(DXTexturePtr texture)
{
  MiniIndexInDisk index(IT_Texture, texture->GetSerializedSize());
  
  if (!m_file.Write((const char*) &index, sizeof(MiniIndexInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }
  
  if (!texture->SerializeToFile(&m_file))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadStatistic(DXStatisticPtr* statistic, unsigned int statistic_id)
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (!IsReadOnly())
  {
    m_errorCode = DXTM_READ_IN_WRITE_MODE;
    return false;
  }

  if (statistic_id >= GetStatisticCount())
  {
    return false;
  }
#endif // ifdef _DEBUG

  *statistic = new DXStatistic();

  IndexInMemory* index = m_indexManager->GetIndexStatistic(statistic_id);

  if (!ReadStatisticFromDisk(*statistic, index->Position, index->Size))
  {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteStatistic(DXStatisticPtr statistic, unsigned int& statistic_id)
{
#ifdef _DEBUG
  if (!IsOpened())
  {
    m_errorCode = DXTM_PROJECT_NOT_OPENED;
    return false;
  }

  if (IsReadOnly())
  {
    m_errorCode = DXTM_WRITE_IN_READ_MODE;
    return false;
  }
#endif // ifdef _DEBUG

  if (!WriteStatisticToDisk(statistic))
  {
    return false;
  }

  statistic_id = m_numStatistics++;

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::ReadStatisticFromDisk(DXStatisticPtr statistic, ULONGLONG position, unsigned int size)
{
  m_file.SeekRead(position);

  if (!statistic->DeserializeFromFile(&m_file, size))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::WriteStatisticToDisk(DXStatisticPtr statistic)
{
  MiniIndexInDisk index(IT_Statistic, statistic->GetSerializedSize());

  if (!m_file.Write((const char*) &index, sizeof(MiniIndexInDisk)))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  if (!statistic->SerializeToFile(&m_file))
  {
    m_errorCode = DXTM_ERROR_LAST_FILE_OPERATION;
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::UpdateProjectInformation(const string& filename, const string& gameName, const string& anotations)
{
  fstream index(filename.c_str(), ios::in | ios::out | ios::binary);
  if (!index.is_open())
  {
    return false;
  }

  bool problems = false;
  ProjectInformationInDisk projectInfo;

  index.seekg(sizeof(HeaderInDisk), ios::beg);
  index.read((char*) &projectInfo, sizeof(ProjectInformationInDisk));
  problems = !index.good();

  if (!problems)
  {
    ::memset(projectInfo.GameName, 0, sizeof(projectInfo.GameName));
    if (gameName.length())
    {
      ::memcpy(projectInfo.GameName, gameName.c_str(), gameName.length() > sizeof(projectInfo.GameName) ? sizeof(projectInfo.GameName) : gameName.length());
    }

    ::memset(projectInfo.Anotations, 0, sizeof(projectInfo.Anotations));
    if (anotations.length())
    {
      ::memcpy(projectInfo.Anotations, anotations.c_str(), anotations.length() > sizeof(projectInfo.Anotations) ? sizeof(projectInfo.Anotations) : anotations.length());
    }

    index.seekp(sizeof(HeaderInDisk), ios::beg);
    index.write((const char*) &projectInfo, sizeof(ProjectInformationInDisk));
    problems = !index.good();
  }

  index.close();

  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXTraceManager::OptimizeProjectFile(const string& filenameSource, const string& filenameDestination)
{
  DXFileManagerW32 fileSource;
  DXFileManagerW32 fileDestination;
  DXFileManagerW32 fileDestinationIndexTempMethodCall;
  DXFileManagerW32 fileDestinationIndexTempBuffer;
  DXFileManagerW32 fileDestinationIndexTempTexture;
  DXFileManagerW32 fileDestinationIndexTempStatistic;
  
  //////////////////////////////////////////////////////////////////////////////
  // Open the source project file
  //////////////////////////////////////////////////////////////////////////////
  
  if (!fileSource.OpenRead(filenameSource))
  {
    return false;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Get project source file metadata
  //////////////////////////////////////////////////////////////////////////////
  
  OptionsInDisk fileOptions;
  if (!fileSource.ReadRaw(sizeof(HeaderInDisk)+sizeof(ProjectInformationInDisk), (char*) &fileOptions, sizeof(OptionsInDisk)))
  {
    return false;
  }

  bool fileCompressed = fileOptions.Compressed;
  bool fileOptimized = fileOptions.Optimized;

  if (fileOptimized)
  {
    return true;
  }

  //////////////////////////////////////////////////////////////////////////////

  CountersInDisk fileCounters;
  if (!fileSource.ReadRaw(sizeof(HeaderInDisk)+sizeof(ProjectInformationInDisk)+sizeof(OptionsInDisk), (char*) &fileCounters, sizeof(CountersInDisk)))
  {
    return false;
  }

  unsigned int fileMethodCallCount = fileCounters.MethodCallsCount;
  unsigned int fileBufferCount     = fileCounters.BuffersCount;
  unsigned int fileTextureCount    = fileCounters.TexturesCount;
  unsigned int fileStatisticCount  = fileCounters.StatisticsCount;

  //////////////////////////////////////////////////////////////////////////////
  // Open the source and destination project file in correct mode
  //////////////////////////////////////////////////////////////////////////////

  unsigned int fileHeadersSize = sizeof(HeaderInDisk)+
                                 sizeof(ProjectInformationInDisk)+
                                 sizeof(OptionsInDisk)+
                                 sizeof(CountersInDisk);

  if (!fileSource.OpenRead(filenameSource, fileCompressed, fileHeadersSize))
  {
    return false;
  }
  
  if (!fileDestination.OpenWrite(filenameDestination, true, fileHeadersSize, 256*1024))
  {
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////

  if (!fileDestinationIndexTempMethodCall.OpenWrite(filenameDestination+".idxtmp1", true, sizeof(OptimizedIndexBlockInDisk), 1024*1024))
  {
    return false;
  }
  else
  {
    OptimizedIndexBlockInDisk indexBlock;
    indexBlock.SetMagicNumber();
    indexBlock.NumIndexs = fileMethodCallCount;
    if (!fileDestinationIndexTempMethodCall.WriteRaw(0, (char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
    {
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  
  if (!fileDestinationIndexTempBuffer.OpenWrite(filenameDestination+".idxtmp2", true, sizeof(OptimizedIndexBlockInDisk), 1024*1024))
  {
    return false;
  }
  else
  {
    OptimizedIndexBlockInDisk indexBlock;
    indexBlock.SetMagicNumber();
    indexBlock.NumIndexs = fileBufferCount;
    if (!fileDestinationIndexTempBuffer.WriteRaw(0, (char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
    {
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  
  if (!fileDestinationIndexTempTexture.OpenWrite(filenameDestination+".idxtmp3", true, sizeof(OptimizedIndexBlockInDisk), 1024*1024))
  {
    return false;
  }
  else
  {
    OptimizedIndexBlockInDisk indexBlock;
    indexBlock.SetMagicNumber();
    indexBlock.NumIndexs = fileTextureCount;
    if (!fileDestinationIndexTempTexture.WriteRaw(0, (char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
    {
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////////////

  if (!fileDestinationIndexTempStatistic.OpenWrite(filenameDestination+".idxtmp4", true, sizeof(OptimizedIndexBlockInDisk), 1024*1024))
  {
    return false;
  }
  else
  {
    OptimizedIndexBlockInDisk indexBlock;
    indexBlock.SetMagicNumber();
    indexBlock.NumIndexs = fileStatisticCount;
    if (!fileDestinationIndexTempStatistic.WriteRaw(0, (char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
    {
      return false;
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  // Copy headers between files
  //////////////////////////////////////////////////////////////////////////////

  char* headers = new char[fileHeadersSize];
  if (!headers)
  {
    return false;
  }
  else
  {
    fileSource.ReadRaw(0, headers, fileHeadersSize);
    fileDestination.WriteRaw(0, headers, fileHeadersSize);
    delete[] headers;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Copy data between files
  //////////////////////////////////////////////////////////////////////////////

  unsigned int numPendingMethodCall = fileMethodCallCount;
  unsigned int numPendingBuffer = fileBufferCount;
  unsigned int numPendingTexture = fileTextureCount;
  unsigned int numPendingStatistic = fileStatisticCount;
  
  unsigned int numReadedMethodCall = 0;
  unsigned int numReadedBuffer = 0;
  unsigned int numReadedTexture = 0;
  unsigned int numReadedStatistic = 0;

  bool problems = false;
  bool discardByCRC = true;
  ULONGLONG lastReadPosition = 0;
  ULONGLONG lastWritePosition = 0;
  
  //////////////////////////////////////////////////////////////////////////////
  
  vector<DXMethodCallPtr> listMethodCalls;
  vector<DXBufferPtr> listBuffers;
  vector<DXTexturePtr> listTextures;
  vector<DXStatisticPtr> listStatistics;

  map<MD5::md5_t, unsigned int> hashBuffers;
  map<unsigned int, unsigned int> hashTextures;
  
  vector<unsigned int> listBuffersRepeated;
  vector<unsigned int> listTexturesRepeated;

  vector<pair<ULONGLONG, unsigned int> > listBuffersPositions;
  vector<pair<ULONGLONG, unsigned int> > listTexturesPositions;
  
  vector<IndexInDisk> listMethodCallsIndexs;
  vector<IndexInDisk> listBuffersIndexs;
  vector<IndexInDisk> listTexturesIndexs;
  vector<IndexInDisk> listStatisticsIndexs;
  
  //////////////////////////////////////////////////////////////////////////////
  
  while (numPendingMethodCall)
  {
    for (unsigned int numReaded=0, numToReadMethodCall=min(10000, numPendingMethodCall); numReaded < numToReadMethodCall;)
    {
      MiniIndexInDisk index;
      
      fileSource.SeekRead(lastReadPosition);
      if (!fileSource.Read((char*) &index, sizeof(MiniIndexInDisk)))
      {
        problems = true;
        break;
      }
      
      switch (index.GetType())
      {
      case IT_MethodCall:
        {
          DXMethodCallPtr call = new DXMethodCall();
          char* deserializedMethodCall = NULL;
          if (!call->DeserializeFromBufferFastInit(&deserializedMethodCall, index.GetSize()))
          {
            problems = true;
            break;
          }
          if (!fileSource.Read(deserializedMethodCall, index.GetSize()))
          {
            problems = true;
            break;
          }
          if (!call->DeserializeFromBufferFast())
          {
            problems = true;
            break;
          }
          listMethodCalls.push_back(call);
          
          numReaded++;
          lastReadPosition += (sizeof(MiniIndexInDisk) + index.GetSize());
          numPendingMethodCall--;
        }
        break;
      
      case IT_Buffer:
        {
          DXBufferPtr buffer = new DXBuffer();
          if (!buffer->DeserializeFromFile(&fileSource, index.GetSize()))
          {
            problems = true;
            break;
          }
          
          if (!discardByCRC)
          {
            listBuffers.push_back(buffer);
          }
          else
          {
            if (buffer->GetSerializedSize() >= 64)
            {
              MD5::md5_t md5_val;
              buffer->GetMD5(md5_val.value);
              map<MD5::md5_t, unsigned int>::iterator md5_pos = hashBuffers.find(md5_val);
              if (md5_pos != hashBuffers.end() && listBuffers[(*md5_pos).second]->GetSize() == buffer->GetSize())
              {
                listBuffersRepeated.push_back((*md5_pos).second);
              }
              else
              {
                listBuffers.push_back(buffer);
                hashBuffers.insert(pair<MD5::md5_t, unsigned int>(md5_val, (unsigned int) listBuffers.size()-1));
                listBuffersRepeated.push_back((unsigned int) listBuffers.size()-1);
              }
            }
            else
            {
              listBuffers.push_back(buffer);
              listBuffersRepeated.push_back((unsigned int) listBuffers.size()-1);
            }
          }
          
          lastReadPosition += (sizeof(MiniIndexInDisk) + index.GetSize());
          numPendingBuffer--;
        }
        break;
      
      case IT_Texture:
        {
          DXTexturePtr texture = new DXTexture();
          if (!texture->DeserializeFromFile(&fileSource, index.GetSize()))
          {
            problems = true;
            break;
          }

          if (!discardByCRC)
          {
            listTextures.push_back(texture);
          }
          else
          {
            if (texture->GetSerializedSize() >= 8*1024)
            {
              unsigned int crc32_val = texture->GetCRC32();
              map<unsigned int, unsigned int>::iterator crc32_pos = hashTextures.find(crc32_val);
              if (crc32_pos != hashTextures.end() && listTextures[(*crc32_pos).second]->GetSerializedSize() == texture->GetSerializedSize())
              {
                listTexturesRepeated.push_back((*crc32_pos).second);
              }
              else
              {
                listTextures.push_back(texture);
                hashTextures.insert(pair<unsigned int, unsigned int>(crc32_val, (unsigned int) listTextures.size()-1));
                listTexturesRepeated.push_back((unsigned int) listTextures.size()-1);
              }
            }
            else
            {
              listTextures.push_back(texture);
              listTexturesRepeated.push_back((unsigned int) listTextures.size()-1);
            }
          }

          lastReadPosition += (sizeof(MiniIndexInDisk) + index.GetSize());
          numPendingTexture--;
        }
        break;
      
      case IT_Statistic:
        {
          DXStatisticPtr statistic = new DXStatistic();
          if (!statistic->DeserializeFromFile(&fileSource, index.GetSize()))
          {
            problems = true;
            break;
          }
          listStatistics.push_back(statistic);

          lastReadPosition += (sizeof(MiniIndexInDisk) + index.GetSize());
          numPendingStatistic--;
        }
        break;
      }
      
      if (problems) break;
    }

    if (problems) break;
    
    ////////////////////////////////////////////////////////////////////////////
    // Write method calls in destination file
    ////////////////////////////////////////////////////////////////////////////

    for (vector<DXMethodCallPtr>::iterator it=listMethodCalls.begin(); it != listMethodCalls.end() && !problems; ++it)
    {
      char* serializedCall;
      unsigned int serializedMethodCallSize;

      if (!(*it)->SerializeToBuffer(&serializedCall, &serializedMethodCallSize))
      {
        problems = true;
        break;
      }

      if (!fileDestination.Write((const char*) serializedCall, serializedMethodCallSize))
      {
        problems = true;
        break;
      }

      listMethodCallsIndexs.push_back(IndexInDisk(IT_MethodCall, lastWritePosition, serializedMethodCallSize));
      lastWritePosition += serializedMethodCallSize;
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // Write buffers in destination file
    ////////////////////////////////////////////////////////////////////////////

    for (vector<DXBufferPtr>::iterator it=listBuffers.begin(); it != listBuffers.end() && !problems; ++it)
    {
      if (!(*it)->SerializeToFile(&fileDestination))
      {
        problems = true;
        break;
      }

      if (discardByCRC)
        listBuffersPositions.push_back(pair<ULONGLONG, unsigned int>(lastWritePosition, (*it)->GetSerializedSize()));
      else
        listBuffersIndexs.push_back(IndexInDisk(IT_Buffer, lastWritePosition, (*it)->GetSerializedSize()));
      
      lastWritePosition += (*it)->GetSerializedSize();
    }
    
    if (discardByCRC)
    {
      for (vector<unsigned int>::iterator it=listBuffersRepeated.begin(); it != listBuffersRepeated.end(); ++it)
      {
        listBuffersIndexs.push_back(IndexInDisk(IT_Buffer, listBuffersPositions[*it].first, listBuffersPositions[*it].second));
      }
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // Write textures in destination file
    ////////////////////////////////////////////////////////////////////////////

    for (vector<DXTexturePtr>::iterator it=listTextures.begin(); it != listTextures.end() && !problems; ++it)
    {
      if (!(*it)->SerializeToFile(&fileDestination))
      {
        problems = true;
        break;
      }

      if (discardByCRC)
        listTexturesPositions.push_back(pair<ULONGLONG, unsigned int>(lastWritePosition, (*it)->GetSerializedSize()));
      else
        listTexturesIndexs.push_back(IndexInDisk(IT_Texture, lastWritePosition, (*it)->GetSerializedSize()));

      lastWritePosition += (*it)->GetSerializedSize();
    }

    if (discardByCRC)
    {
      for (vector<unsigned int>::iterator it=listTexturesRepeated.begin(); it != listTexturesRepeated.end(); ++it)
      {
        listTexturesIndexs.push_back(IndexInDisk(IT_Texture, listTexturesPositions[*it].first, listTexturesPositions[*it].second));
      }
    }
    
    ////////////////////////////////////////////////////////////////////////////
    // Write statistics in destination file
    ////////////////////////////////////////////////////////////////////////////

    for (vector<DXStatisticPtr>::iterator it=listStatistics.begin(); it != listStatistics.end() && !problems; ++it)
    {
      if (!(*it)->SerializeToFile(&fileDestination))
      {
        problems = true;
        break;
      }

      listStatisticsIndexs.push_back(IndexInDisk(IT_Statistic, lastWritePosition, (*it)->GetSerializedSize()));
      lastWritePosition += (*it)->GetSerializedSize();
    }
    
    ////////////////////////////////////////////////////////////////////////////
    
    if (problems) break;
    
    ////////////////////////////////////////////////////////////////////////////
    // Write optimized index block in temporal destination file
    ////////////////////////////////////////////////////////////////////////////

    for (vector<IndexInDisk>::iterator it=listMethodCallsIndexs.begin(); it != listMethodCallsIndexs.end() && !problems; ++it)
    {
      problems = !fileDestinationIndexTempMethodCall.Write((char*) &(*it), sizeof(IndexInDisk));
    }

    for (vector<IndexInDisk>::iterator it=listBuffersIndexs.begin(); it != listBuffersIndexs.end() && !problems; ++it)
    {
      problems = !fileDestinationIndexTempBuffer.Write((char*) &(*it), sizeof(IndexInDisk));
    }

    for (vector<IndexInDisk>::iterator it=listTexturesIndexs.begin(); it != listTexturesIndexs.end() && !problems; ++it)
    {
      problems = !fileDestinationIndexTempTexture.Write((char*) &(*it), sizeof(IndexInDisk));
    }

    for (vector<IndexInDisk>::iterator it=listStatisticsIndexs.begin(); it != listStatisticsIndexs.end() && !problems; ++it)
    {
      problems = !fileDestinationIndexTempStatistic.Write((char*) &(*it), sizeof(IndexInDisk));
    }
    
    ////////////////////////////////////////////////////////////////////////////
    
    if (discardByCRC)
    {
      numReadedMethodCall += static_cast<unsigned int>(listMethodCalls.size());
      numReadedBuffer += static_cast<unsigned int>(listBuffersRepeated.size());
      numReadedTexture += static_cast<unsigned int>(listTexturesRepeated.size());
      numReadedStatistic += static_cast<unsigned int>(listStatistics.size());
    }
    else
    {
      numReadedMethodCall += static_cast<unsigned int>(listMethodCalls.size());
      numReadedBuffer += static_cast<unsigned int>(listBuffers.size());
      numReadedTexture += static_cast<unsigned int>(listTextures.size());
      numReadedStatistic += static_cast<unsigned int>(listStatistics.size());
    }
    
    ////////////////////////////////////////////////////////////////////////////

    listMethodCalls.clear();
    listBuffers.clear();
    listTextures.clear();
    listStatistics.clear();

    hashBuffers.clear();
    hashTextures.clear();

    listBuffersRepeated.clear();
    listTexturesRepeated.clear();

    listBuffersPositions.clear();
    listTexturesPositions.clear();
    
    listMethodCallsIndexs.clear();
    listBuffersIndexs.clear();
    listTexturesIndexs.clear();
    listStatisticsIndexs.clear();
    
    ////////////////////////////////////////////////////////////////////////////
    
    if (problems) break;
  }
  
  if (!problems)
  {
    problems = (numPendingMethodCall != 0 || numPendingBuffer != 0 || numPendingTexture != 0 || numPendingStatistic != 0);
  }
  
  if (!problems)
  {
    ULONGLONG firstIndexBlockPositionMethodCall = 0;
    ULONGLONG firstIndexBlockPositionBuffer     = 0;
    ULONGLONG firstIndexBlockPositionTexture    = 0;
    ULONGLONG firstIndexBlockPositionStatistic  = 0;
   
    ////////////////////////////////////////////////////////////////////////////
    // Dump temporal index contents into destination file (method calls)
    ////////////////////////////////////////////////////////////////////////////
    
    fileDestinationIndexTempMethodCall.Close();
    
    if (!problems && fileDestinationIndexTempMethodCall.OpenRead(filenameDestination+".idxtmp1", true, sizeof(OptimizedIndexBlockInDisk)))
    {
      OptimizedIndexBlockInDisk indexBlock;
      if (!fileDestinationIndexTempMethodCall.ReadRaw(0, (char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
      {
        problems = true;
      }
      else
      {
        if (!fileDestination.Write((const char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
        {
          problems = true;
        }
      }
      
      if (!problems)
      {
        ULARGE_INTEGER fileSize;
        fileSize.QuadPart = fileDestinationIndexTempMethodCall.GetSizeUncompressed();
        unsigned int pendingBytes = fileSize.LowPart - sizeof(OptimizedIndexBlockInDisk);
        char* bufferTemp = new char[64*1024];

        firstIndexBlockPositionMethodCall = lastWritePosition;
        lastWritePosition += pendingBytes + sizeof(OptimizedIndexBlockInDisk);

        while (pendingBytes)
        {
          if (problems) break;

          unsigned int readedBytes = min(pendingBytes, 64*1024);

          if (!fileDestinationIndexTempMethodCall.Read(bufferTemp, &readedBytes))
          {
            problems = true;
            break;
          }

          if (!fileDestination.Write(bufferTemp, readedBytes))
          {
            problems = true;
            break;
          }

          pendingBytes -= readedBytes;
        }

        delete[] bufferTemp;
      }
    }
    else problems = true;

    ////////////////////////////////////////////////////////////////////////////
    // Dump temporal index contents into destination file (buffers)
    ////////////////////////////////////////////////////////////////////////////

    fileDestinationIndexTempBuffer.Close();

    if (!problems && fileDestinationIndexTempBuffer.OpenRead(filenameDestination+".idxtmp2", true, sizeof(OptimizedIndexBlockInDisk)))
    {
      OptimizedIndexBlockInDisk indexBlock;
      if (!fileDestinationIndexTempBuffer.ReadRaw(0, (char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
      {
        problems = true;
      }
      else
      {
        if (!fileDestination.Write((const char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
        {
          problems = true;
        }
      }
      
      if (!problems)
      {
        ULARGE_INTEGER fileSize;
        fileSize.QuadPart = fileDestinationIndexTempBuffer.GetSizeUncompressed();
        unsigned int pendingBytes = fileSize.LowPart - sizeof(OptimizedIndexBlockInDisk);
        char* bufferTemp = new char[64*1024];

        firstIndexBlockPositionBuffer = lastWritePosition;
        lastWritePosition += pendingBytes + sizeof(OptimizedIndexBlockInDisk);

        while (pendingBytes)
        {
          if (problems) break;

          unsigned int readedBytes = min(pendingBytes, 64*1024);
          
          if (!fileDestinationIndexTempBuffer.Read(bufferTemp, &readedBytes))
          {
            problems = true;
            break;
          }
          
          if (!fileDestination.Write(bufferTemp, readedBytes))
          {
            problems = true;
            break;
          }

          pendingBytes -= readedBytes;
        }

        delete[] bufferTemp;
      }
    }
    else problems = true;

    ////////////////////////////////////////////////////////////////////////////
    // Dump temporal index contents into destination file (textures)
    ////////////////////////////////////////////////////////////////////////////

    fileDestinationIndexTempTexture.Close();

    if (!problems && fileDestinationIndexTempTexture.OpenRead(filenameDestination+".idxtmp3", true, sizeof(OptimizedIndexBlockInDisk)))
    {
      OptimizedIndexBlockInDisk indexBlock;
      if (!fileDestinationIndexTempTexture.ReadRaw(0, (char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
      {
        problems = true;
      }
      else
      {
        if (!fileDestination.Write((const char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
        {
          problems = true;
        }
      }

      if (!problems)
      {
        ULARGE_INTEGER fileSize;
        fileSize.QuadPart = fileDestinationIndexTempTexture.GetSizeUncompressed();
        unsigned int pendingBytes = fileSize.LowPart - sizeof(OptimizedIndexBlockInDisk);
        char* bufferTemp = new char[64*1024];

        firstIndexBlockPositionTexture = lastWritePosition;
        lastWritePosition += pendingBytes + sizeof(OptimizedIndexBlockInDisk);

        while (pendingBytes)
        {
          if (problems) break;

          unsigned int readedBytes = min(pendingBytes, 64*1024);

          if (!fileDestinationIndexTempTexture.Read(bufferTemp, &readedBytes))
          {
            problems = true;
            break;
          }

          if (!fileDestination.Write(bufferTemp, readedBytes))
          {
            problems = true;
            break;
          }

          pendingBytes -= readedBytes;
        }

        delete[] bufferTemp;
      }
    }
    else problems = true;

    ////////////////////////////////////////////////////////////////////////////
    // Dump temporal index contents into destination file (statistics)
    ////////////////////////////////////////////////////////////////////////////

    fileDestinationIndexTempStatistic.Close();

    if (!problems && fileDestinationIndexTempStatistic.OpenRead(filenameDestination+".idxtmp4", true, sizeof(OptimizedIndexBlockInDisk)))
    {
      OptimizedIndexBlockInDisk indexBlock;
      if (!fileDestinationIndexTempStatistic.ReadRaw(0, (char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
      {
        problems = true;
      }
      else
      {
        if (!fileDestination.Write((const char*) &indexBlock, sizeof(OptimizedIndexBlockInDisk)))
        {
          problems = true;
        }
      }

      if (!problems)
      {
        ULARGE_INTEGER fileSize;
        fileSize.QuadPart = fileDestinationIndexTempStatistic.GetSizeUncompressed();
        unsigned int pendingBytes = fileSize.LowPart - sizeof(OptimizedIndexBlockInDisk);
        char* bufferTemp = new char[64*1024];

        firstIndexBlockPositionStatistic = lastWritePosition;
        lastWritePosition += pendingBytes + sizeof(OptimizedIndexBlockInDisk);

        while (pendingBytes)
        {
          if (problems) break;

          unsigned int readedBytes = min(pendingBytes, 64*1024);

          if (!fileDestinationIndexTempStatistic.Read(bufferTemp, &readedBytes))
          {
            problems = true;
            break;
          }

          if (!fileDestination.Write(bufferTemp, readedBytes))
          {
            problems = true;
            break;
          }

          pendingBytes -= readedBytes;
        }

        delete[] bufferTemp;
      }
    }
    else problems = true;

    ////////////////////////////////////////////////////////////////////////////
    // Change destination file index header
    ////////////////////////////////////////////////////////////////////////////
    
    if (!problems)
    {
      fileOptions.Compressed = true;
      fileOptions.Optimized = true;
      fileOptions.IndexBlockPositionMethodCall = firstIndexBlockPositionMethodCall;
      fileOptions.IndexBlockPositionBuffer = firstIndexBlockPositionBuffer;
      fileOptions.IndexBlockPositionTexture = firstIndexBlockPositionTexture;
      fileOptions.IndexBlockPositionStatistic = firstIndexBlockPositionStatistic;
      
      problems = !fileDestination.WriteRaw(sizeof(HeaderInDisk)+sizeof(ProjectInformationInDisk), (char*) &fileOptions, sizeof(OptionsInDisk));
    }
  }
  
  //////////////////////////////////////////////////////////////////////////////
  
  fileSource.Close();
  fileDestination.Close();
  fileDestinationIndexTempMethodCall.Close();
  fileDestinationIndexTempBuffer.Close();
  fileDestinationIndexTempTexture.Close();
  fileDestinationIndexTempStatistic.Close();
  
  //////////////////////////////////////////////////////////////////////////////
  
  ::DeleteFile((filenameDestination+".idxtmp1").c_str());
  ::DeleteFile((filenameDestination+".idxtmp2").c_str());
  ::DeleteFile((filenameDestination+".idxtmp3").c_str());
  ::DeleteFile((filenameDestination+".idxtmp4").c_str());

  if (problems)
  {
    ::DeleteFile((filenameDestination).c_str());
  }

  //////////////////////////////////////////////////////////////////////////////
  
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

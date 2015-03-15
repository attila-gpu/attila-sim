////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  //////////////////////////////////////////////////////////////////////////////

  class IDXIndexManager;
  class DXIndexManagerOptimized;
  class DXIndexManagerNotOptimized;

  //////////////////////////////////////////////////////////////////////////////
  
  class DXTraceManager
  {
  public:

    DXTraceManager();
    virtual ~DXTraceManager();

    bool OpenRead(const std::string& filename);
    bool OpenWrite(const std::string& destinationPath, const std::string& projectName, bool compressed);
    bool Close();
    bool IsOpened() const;
    bool IsCompressed() const;
    bool IsOptimized() const;

    unsigned int GetErrorCode() const;
    std::string GetErrorMessage(unsigned int code) const;
    std::string GetCurrentErrorMessage() const;

    time_t GetCreationDate() const;
    const std::string& GetProjectFileName() const;
    const std::string& GetProjectGameName() const;
    const std::string& GetProjectAnotations() const;
    std::string GetOptionsString() const;
    std::string GetCountersString() const;

    unsigned int GetFrameCount() const;
    unsigned int GetMethodCallCount() const;
    unsigned int GetBufferCount() const;
    unsigned int GetTextureCount() const;
    unsigned int GetStatisticCount() const;
    ULONGLONG GetFileSize() const;
    ULONGLONG GetFileSizeInDisk() const;
    
    bool ReadMethodCall(DXMethodCallPtr* call, unsigned int methodcall_id);
    bool WriteMethodCall(DXMethodCallPtr call);

    bool ReadBuffer(DXBufferPtr* buffer, unsigned int buffer_id);
    bool WriteBuffer(DXBufferPtr buffer, unsigned int& buffer_id);

    bool ReadTexture(DXTexturePtr* buffer, unsigned int texture_id);
    bool WriteTexture(DXTexturePtr buffer, unsigned int& texture_id);

    bool ReadStatistic(DXStatisticPtr* statistic, unsigned int statistic_id);
    bool WriteStatistic(DXStatisticPtr statistic, unsigned int& statistic_id);

    static bool UpdateProjectInformation(const std::string& filename, const std::string& gameName, const std::string& anotations = "");
    static bool OptimizeProjectFile(const std::string& filenameSource, const std::string& filenameDestination);

  protected:

    friend class DXIndexManagerOptimized;
    friend class DXIndexManagerNotOptimized;

    //////////////////////////////////////////////////////////////////////////////

    enum ErrorType
    {
      DXTM_NOERROR = 0,
      DXTM_PROJECT_NOT_OPENED,
      DXTM_WRITE_IN_READ_MODE,
      DXTM_READ_IN_WRITE_MODE,
      DXTM_CANT_OPEN_FILE,
      DXTM_ERROR_LAST_FILE_OPERATION,
      DXTM_ERROR_LOADING_FILE_INDEXS,
      DXTM_ERROR_INVALID_HEADER,
      DXTM_ERROR_SERIALIZE_METHODCALL,
      DXTM_ERROR_DESERIALIZE_METHODCALL,
      DXTM_ERROR_SERIALIZE_BUFFER,
      DXTM_ERROR_DESERIALIZE_BUFFER,
      DXTM_ERROR_SERIALIZE_TEXTURE,
      DXTM_ERROR_DESERIALIZE_TEXTURE,
      DXTM_ERROR_SERIALIZE_STATISTIC,
      DXTM_ERROR_DESERIALIZE_STATISTIC,
      DXTM_ERROR_NO_MEMORY
    };

    //////////////////////////////////////////////////////////////////////////////

    struct HeaderInDisk;
    struct ProjectInformationInDisk;
    struct OptionsInDisk;
    struct CountersInDisk;

    typedef DXFileManagerW32 DXFileManagerType;
    
    static const std::string PROJECT_FILE_EXTENSION;
    static const unsigned int LIBRARY_VERSION;

    bool m_isOpened;
    bool m_isReadOnly;
    
    ErrorType m_errorCode;

    time_t m_creationDate;
    std::string m_projectFileName;
    std::string m_projectGameName;
    std::string m_projectAnotations;

    bool m_isCompressed;
    bool m_isOptimized;
    DXFileManagerType m_file;
    IDXIndexManager* m_indexManager;
    ULONGLONG m_indexBlockPositionMethodCall;
    ULONGLONG m_indexBlockPositionBuffer;
    ULONGLONG m_indexBlockPositionTexture;
    ULONGLONG m_indexBlockPositionStatistic;
    
    unsigned int m_numFrames;
    unsigned int m_numMethodCalls;
    unsigned int m_numBuffers;
    unsigned int m_numTextures;
    unsigned int m_numStatistics;
    
    bool IsReadOnly() const;
    void ResetErrorCode();

    std::string CorrectDestinationPath(const std::string& destinationPath);
    std::string GetGameNameFromProjectName(const std::string& projectName);

    bool InitRead();
    
    bool OpenFile(const std::string& filename, bool readOnly, bool compressed);
    void CloseFile();

    bool ReadMetaData(const std::string& filename);
    bool WriteMetaData();

    bool ReadHeader(DXFileManagerType* file);
    bool WriteHeader();

    bool ReadProjectInformation(DXFileManagerType* file);
    bool WriteProjectInformation();

    bool ReadOptions(DXFileManagerType* file);
    bool WriteOptions();

    bool ReadCounters(DXFileManagerType* file);
    bool WriteCounters();

    bool ReadMethodCallFromDisk(DXMethodCallPtr call, ULONGLONG position, unsigned int size);
    bool WriteMethodCallToDisk(DXMethodCallPtr call);

    bool ReadBufferFromDisk(DXBufferPtr buffer, ULONGLONG position, unsigned int size);
    bool WriteBufferToDisk(DXBufferPtr buffer);

    bool ReadTextureFromDisk(DXTexturePtr texture, ULONGLONG position, unsigned int size);
    bool WriteTextureToDisk(DXTexturePtr texture);

    bool ReadStatisticFromDisk(DXStatisticPtr statistic, ULONGLONG position, unsigned int size);
    bool WriteStatisticToDisk(DXStatisticPtr statistic);

  };
}

////////////////////////////////////////////////////////////////////////////////

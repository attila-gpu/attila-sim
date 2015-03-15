////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class DXProfiler
{
public:

  DXProfiler(HANDLE processHandle);
  virtual ~DXProfiler();

  void Reset();
  bool UpdateCounters();
  bool SaveResultsCSV(const std::string& filenameCSV, unsigned int frameRangeMin = 0, unsigned int frameRangeMax = 0);

private:
  
  struct timeval
  {
    long tv_sec;  // seconds
    long tv_usec; // and microseconds
  };
  
  struct CountersInformation
  {
    DWORD instant;
    
    timeval timeUser;
    timeval timeKernel;
    timeval timeLife;

    long cpuAvg;
    long cpuCur;

    long memoryPagesAvg;
    long memoryPagesMax;

    long memoryWorkingSetAvg;
    long memoryWorkingSetMax;

    ULONGLONG ioOpsRead;
    ULONGLONG ioOpsWrite;
    ULONGLONG ioOpsOther;

    long ioOpsReadPerSecond;
    long ioOpsWritePerSecond;
    long ioOpsOtherPerSecond;

    long fpsAvg;
    long fpsCur;
  };

  struct CountersInformationAVG
  {
    ULONGLONG cpuAvg;
    long cpuAvgCount;

    ULONGLONG memoryPagesAvg;
    long memoryPagesAvgCount;

    ULONGLONG memoryWorkingSetAvg;
    long memoryWorkingSetAvgCount;

    ULONGLONG fpsAvg;
    long fpsAvgCount;
  };
  
  typedef unsigned int dx_uint32;
  typedef float dx_float;
  
  static const unsigned int MIN_ELAPSED_TIME = 100;
  
  HANDLE m_processHandle;
  std::vector<CountersInformation> m_counters;
  CountersInformationAVG m_procCountersAVG;
  
  DWORD m_timeStart;
  
  dx_float  m_fps;
  dx_uint32 m_fpsLastTimeStamp;
  dx_uint32 m_fpsNumFramesFromLast;
  dx_uint32 m_fpsTimeElapsedFromLast;
  
  ULARGE_INTEGER m_cpuUserTimeStart;
  ULARGE_INTEGER m_cpuKernelTimeStart;
  DWORD          m_cpuLastMeasureCPU;
  long           m_cpuLastUsageCPU;
  
  ULONGLONG m_ioReadOpsStart;
  DWORD     m_ioLastMeasureReadOps;
  long      m_ioLastReadOpsUsage;

  ULONGLONG m_ioWriteOpsStart;
  DWORD     m_ioLastMeasureWriteOps;
  long      m_ioLastWriteOpsUsage;

  ULONGLONG m_ioOtherOpsStart;
  DWORD     m_ioLastMeasureOtherOps;
  long      m_ioLastOtherOpsUsage;

  void Init();
  void Close();

  void CalculateFPS();
  void CalculateLifeTime(FILETIME* creationTime, FILETIME* lifeTime);
  long CalculateUsageCPU(FILETIME* userTime, FILETIME* kernelTime);
  long CalculateUsageReadOps(ULONGLONG readOps);
  long CalculateUsageWriteOps(ULONGLONG readOps);
  long CalculateUsageOtherOps(ULONGLONG readOps);

  static void filetime_to_timeval(FILETIME* src, timeval* dst);

};

////////////////////////////////////////////////////////////////////////////////

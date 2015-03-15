////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXProfiler.h"

using namespace std;

////////////////////////////////////////////////////////////////////////////////

DXProfiler::DXProfiler(HANDLE processHandle) :
m_processHandle(processHandle)
{
  Init();
}

////////////////////////////////////////////////////////////////////////////////

DXProfiler::~DXProfiler()
{
  Close();
}

////////////////////////////////////////////////////////////////////////////////

void DXProfiler::Init()
{
  ZeroMemory(&m_procCountersAVG, sizeof(CountersInformationAVG));
  m_counters.clear();
  
  m_timeStart = 0;
  
  m_fps = 0.0;
  m_fpsLastTimeStamp = 0;
  m_fpsNumFramesFromLast = 0;
  m_fpsTimeElapsedFromLast = 0;

  m_cpuUserTimeStart.QuadPart = 0;
  m_cpuKernelTimeStart.QuadPart = 0;
  m_cpuLastMeasureCPU = 0;
  m_cpuLastUsageCPU = 0;

  m_ioReadOpsStart = 0;
  m_ioLastMeasureReadOps = 0;
  m_ioLastReadOpsUsage = 0;

  m_ioWriteOpsStart = 0;
  m_ioLastMeasureWriteOps = 0;
  m_ioLastWriteOpsUsage = 0;

  m_ioOtherOpsStart = 0;
  m_ioLastMeasureOtherOps = 0;
  m_ioLastOtherOpsUsage = 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXProfiler::Close()
{
}

////////////////////////////////////////////////////////////////////////////////

void DXProfiler::Reset()
{
  Init();
}

////////////////////////////////////////////////////////////////////////////////

bool DXProfiler::UpdateCounters()
{
  CountersInformation procCounters;
  ZeroMemory(&procCounters, sizeof(CountersInformation));
  
  if (!m_timeStart)
  {
    m_timeStart = timeGetTime();
    procCounters.instant = 0;
  }
  else
  {
    procCounters.instant = timeGetTime() - m_timeStart;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Time usage
  
  FILETIME creationTime;
  FILETIME lifeTime;
  FILETIME exitTime;
  FILETIME kernelTime;
  FILETIME userTime;

  if (!GetProcessTimes(m_processHandle, &creationTime, &exitTime, &kernelTime, &userTime))
  {
    return false;
  }

  CalculateLifeTime(&creationTime, &lifeTime);

  filetime_to_timeval(&lifeTime, &procCounters.timeLife);
  filetime_to_timeval(&userTime, &procCounters.timeUser);
  filetime_to_timeval(&kernelTime, &procCounters.timeKernel);

  //////////////////////////////////////////////////////////////////////////////
  // CPU usage

  long cpuUsage = CalculateUsageCPU(&userTime, &kernelTime);

  m_procCountersAVG.cpuAvg += cpuUsage;
  m_procCountersAVG.cpuAvgCount++;
  
  procCounters.cpuAvg = static_cast<long>(m_procCountersAVG.cpuAvg / m_procCountersAVG.cpuAvgCount);
  procCounters.cpuCur = cpuUsage;

  //////////////////////////////////////////////////////////////////////////////
  // Memory usage

  PROCESS_MEMORY_COUNTERS pmc = { 0 };
  if (!GetProcessMemoryInfo(m_processHandle, &pmc, sizeof(pmc)))
  {
    return false;
  }

  m_procCountersAVG.memoryPagesAvg += (long) pmc.PagefileUsage;
  m_procCountersAVG.memoryPagesAvgCount++;
  
  procCounters.memoryPagesAvg = static_cast<long>(m_procCountersAVG.memoryPagesAvg / m_procCountersAVG.memoryPagesAvgCount);
  procCounters.memoryPagesMax = (long) pmc.PeakPagefileUsage;

  m_procCountersAVG.memoryWorkingSetAvg += (long) pmc.WorkingSetSize;
  m_procCountersAVG.memoryWorkingSetAvgCount++;
  
  procCounters.memoryWorkingSetAvg = static_cast<long>(m_procCountersAVG.memoryWorkingSetAvg / m_procCountersAVG.memoryWorkingSetAvgCount);
  procCounters.memoryWorkingSetMax = (long) pmc.PeakWorkingSetSize;

  //////////////////////////////////////////////////////////////////////////////
  // I/O usage
  
  IO_COUNTERS ioc = { 0 };
  if (!GetProcessIoCounters(m_processHandle, &ioc))
  {
    return false;
  }

  procCounters.ioOpsRead  = ioc.ReadTransferCount;
  procCounters.ioOpsWrite = ioc.WriteTransferCount;
  procCounters.ioOpsOther = ioc.OtherTransferCount;

  procCounters.ioOpsReadPerSecond  = CalculateUsageReadOps(ioc.ReadTransferCount);
  procCounters.ioOpsWritePerSecond = CalculateUsageWriteOps(ioc.WriteTransferCount);
  procCounters.ioOpsOtherPerSecond = CalculateUsageOtherOps(ioc.OtherTransferCount);

  //////////////////////////////////////////////////////////////////////////////
  // FPS usage

  CalculateFPS();

  m_procCountersAVG.fpsAvg += (long) m_fps;
  m_procCountersAVG.fpsAvgCount++;
  
  procCounters.fpsAvg = static_cast<long>(m_procCountersAVG.fpsAvg / m_procCountersAVG.fpsAvgCount);
  procCounters.fpsCur = (long) m_fps;

  //////////////////////////////////////////////////////////////////////////////

  m_counters.push_back(procCounters);
  
  //////////////////////////////////////////////////////////////////////////////
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXProfiler::SaveResultsCSV(const std::string& filenameCSV, unsigned int frameRangeMin, unsigned int frameRangeMax)
{
  if (filenameCSV.empty())
  {
    return false;
  }
  
  ofstream fileCSV(filenameCSV.c_str(), ios::out | ios::trunc);
  
  if (!fileCSV.is_open())
  {
    return false;
  }
  
  fileCSV.imbue(locale("esp"));

  // Write headers

  fileCSV << "frame" << ";";
  fileCSV << "instant" << ";";
  fileCSV << "time_u" << ";";
  fileCSV << "time_k" << ";";
  fileCSV << "time_t" << ";";
  fileCSV << "time_l" << ";";
  fileCSV << "cpu_avg" << ";";
  fileCSV << "cpu_cur" << ";";
  fileCSV << "mem_avg" << ";";
  fileCSV << "mem_max" << ";";
  fileCSV << "pag_avg" << ";";
  fileCSV << "pag_max" << ";";
  fileCSV << "io_read" << ";";
  fileCSV << "io_write" << ";";
  fileCSV << "io_other" << ";";
  fileCSV << "io_read_ps" << ";";
  fileCSV << "io_write_ps" << ";";
  fileCSV << "io_other_ps" << ";";
  fileCSV << "fps_avg" << ";";
  fileCSV << "fps_cur" << endl;
  
  if (frameRangeMax < frameRangeMin)
  {
    unsigned int temp = frameRangeMax;
    frameRangeMax = frameRangeMin;
    frameRangeMin = temp;
  }
  
  if (frameRangeMin == 0 || frameRangeMax == 0 || frameRangeMax > (unsigned int) m_counters.size())
  {
    frameRangeMin = 1;
    frameRangeMax = (unsigned int) m_counters.size();
  }
  
  unsigned int numFrame = 0;
  for (vector<CountersInformation>::iterator it=m_counters.begin(); it != m_counters.end(); ++it)
  {
    numFrame++;
    
    if (frameRangeMin > numFrame || numFrame > frameRangeMax)
    {
      continue;
    }
    
    fileCSV << dec << numFrame << ";";
    fileCSV << dec << (*it).instant << ";";
    fileCSV << dec << ((*it).timeUser.tv_sec*1000) + ((*it).timeUser.tv_usec / 1000) << ";";
    fileCSV << dec << ((*it).timeKernel.tv_sec*1000) + ((*it).timeKernel.tv_usec / 1000) << ";";
    fileCSV << dec << ((*it).timeUser.tv_sec*1000) + ((*it).timeUser.tv_usec / 1000) + ((*it).timeKernel.tv_sec*1000) + ((*it).timeKernel.tv_usec / 1000) << ";";
    fileCSV << dec << ((*it).timeLife.tv_sec*1000) + ((*it).timeLife.tv_usec / 1000) << ";";
    fileCSV << dec << (*it).cpuAvg << ";";
    fileCSV << dec << (*it).cpuCur << ";";
    fileCSV << fixed << setprecision(2) << (*it).memoryWorkingSetAvg / (1024.0*1024.0) << ";";
    fileCSV << fixed << setprecision(2) << (*it).memoryWorkingSetMax / (1024.0*1024.0) << ";";
    fileCSV << fixed << setprecision(2) << (*it).memoryPagesAvg / (1024.0*1024.0) << ";";
    fileCSV << fixed << setprecision(2) << (*it).memoryPagesMax / (1024.0*1024.0) << ";";
    fileCSV << dec << (*it).ioOpsRead << ";";
    fileCSV << dec << (*it).ioOpsWrite << ";";
    fileCSV << dec << (*it).ioOpsOther << ";";
    fileCSV << dec << (*it).ioOpsReadPerSecond << ";";
    fileCSV << dec << (*it).ioOpsWritePerSecond << ";";
    fileCSV << dec << (*it).ioOpsOtherPerSecond << ";";
    fileCSV << dec << (*it).fpsAvg << ";";
    fileCSV << dec << (*it).fpsCur << endl;
  }
  
  fileCSV.close();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXProfiler::CalculateFPS()
{
  m_fpsNumFramesFromLast++;

  if (!m_fpsLastTimeStamp)
  {
    m_fpsLastTimeStamp = timeGetTime();
  }

  dx_uint32 currentTimeStamp = timeGetTime();
  dx_uint32 timeElapsed = currentTimeStamp - m_fpsLastTimeStamp;
  m_fpsLastTimeStamp = currentTimeStamp;
  m_fpsTimeElapsedFromLast += timeElapsed;

  if (m_fpsTimeElapsedFromLast >= MIN_ELAPSED_TIME)
  {
    m_fps = m_fpsNumFramesFromLast * 1000.0f / m_fpsTimeElapsedFromLast;
    m_fpsNumFramesFromLast = 0;
    m_fpsTimeElapsedFromLast = 0;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXProfiler::CalculateLifeTime(FILETIME* creationTime, FILETIME* lifeTime)
{
  ULARGE_INTEGER timeBegInt;
  timeBegInt.LowPart = creationTime->dwLowDateTime;
  timeBegInt.HighPart = creationTime->dwHighDateTime;

  FILETIME timeNow;
  SYSTEMTIME timeNowSys;
  ::GetSystemTime(&timeNowSys);
  ::SystemTimeToFileTime(&timeNowSys, &timeNow);
  ULARGE_INTEGER timeNowInt;
  timeNowInt.LowPart = timeNow.dwLowDateTime;
  timeNowInt.HighPart = timeNow.dwHighDateTime;

  ULARGE_INTEGER timeLife;
  if (timeNowInt.QuadPart > timeBegInt.QuadPart)
  {
    timeLife.QuadPart = timeNowInt.QuadPart - timeBegInt.QuadPart;
  }
  else
  {
    timeLife.QuadPart = 0;
  }

  lifeTime->dwLowDateTime = timeLife.LowPart;
  lifeTime->dwHighDateTime = timeLife.HighPart;
}

////////////////////////////////////////////////////////////////////////////////

long DXProfiler::CalculateUsageCPU(FILETIME* userTime, FILETIME* kernelTime)
{
  if (!m_cpuLastMeasureCPU)
  {
    m_cpuLastMeasureCPU = ::GetTickCount();

    m_cpuUserTimeStart.LowPart = userTime->dwLowDateTime;
    m_cpuUserTimeStart.HighPart = userTime->dwHighDateTime;

    m_cpuKernelTimeStart.LowPart = kernelTime->dwLowDateTime;
    m_cpuKernelTimeStart.HighPart = kernelTime->dwHighDateTime;
  }

  DWORD thisMeasureCPU = ::GetTickCount();
  DWORD deltaTimeMs = thisMeasureCPU - m_cpuLastMeasureCPU;
  if (deltaTimeMs < MIN_ELAPSED_TIME)
  {
    return m_cpuLastUsageCPU;
  }
  m_cpuLastMeasureCPU = thisMeasureCPU;

  ULARGE_INTEGER userTimeInt;
  userTimeInt.LowPart = userTime->dwLowDateTime;
  userTimeInt.HighPart = userTime->dwHighDateTime;

  ULARGE_INTEGER kernelTimeInt;
  kernelTimeInt.LowPart = kernelTime->dwLowDateTime;
  kernelTimeInt.HighPart = kernelTime->dwHighDateTime;

  ULARGE_INTEGER deltaTime;
  deltaTime.QuadPart = userTimeInt.QuadPart + kernelTimeInt.QuadPart - m_cpuUserTimeStart.QuadPart - m_cpuKernelTimeStart.QuadPart;
  m_cpuUserTimeStart = userTimeInt;
  m_cpuKernelTimeStart = kernelTimeInt;

  m_cpuLastUsageCPU = (long) ((deltaTime.QuadPart / deltaTimeMs) / 100);

  return m_cpuLastUsageCPU;
}

////////////////////////////////////////////////////////////////////////////////

long DXProfiler::CalculateUsageReadOps(ULONGLONG readOps)
{
  if (!m_ioLastMeasureReadOps)
  {
    m_ioLastMeasureReadOps = ::GetTickCount();
    m_ioReadOpsStart = readOps;
  }

  DWORD thisMeasureReadOps = ::GetTickCount();
  DWORD deltaMs = thisMeasureReadOps - m_ioLastMeasureReadOps;
  if (deltaMs < MIN_ELAPSED_TIME)
  {
    return m_ioLastReadOpsUsage;
  }
  m_ioLastMeasureReadOps = thisMeasureReadOps;

  ULONGLONG delta = readOps - m_ioReadOpsStart;
  m_ioReadOpsStart = readOps;
  m_ioLastReadOpsUsage = (long) ((delta / (deltaMs / 1000.0)) / 100);

  return m_ioLastReadOpsUsage;
}

////////////////////////////////////////////////////////////////////////////////

long DXProfiler::CalculateUsageWriteOps(ULONGLONG writeOps)
{
  if (!m_ioLastMeasureWriteOps)
  {
    m_ioLastMeasureWriteOps = ::GetTickCount();
    m_ioWriteOpsStart = writeOps;
  }

  DWORD thisMeasureWriteOps = ::GetTickCount();
  DWORD deltaMs = thisMeasureWriteOps - m_ioLastMeasureWriteOps;
  if (deltaMs < MIN_ELAPSED_TIME)
  {
    return m_ioLastWriteOpsUsage;
  }
  m_ioLastMeasureWriteOps = thisMeasureWriteOps;

  ULONGLONG delta = writeOps - m_ioWriteOpsStart;
  m_ioWriteOpsStart = writeOps;
  m_ioLastWriteOpsUsage = (long) ((delta / (deltaMs / 1000.0)) / 100);

  return m_ioLastWriteOpsUsage;
}

////////////////////////////////////////////////////////////////////////////////

long DXProfiler::CalculateUsageOtherOps(ULONGLONG otherOps)
{
  if (!m_ioLastMeasureOtherOps)
  {
    m_ioLastMeasureOtherOps = ::GetTickCount();
    m_ioOtherOpsStart = otherOps;
  }

  DWORD thisMeasureOtherOps = ::GetTickCount();
  DWORD deltaMs = thisMeasureOtherOps - m_ioLastMeasureOtherOps;
  if (deltaMs < MIN_ELAPSED_TIME)
  {
    return m_ioLastOtherOpsUsage;
  }
  m_ioLastMeasureOtherOps = thisMeasureOtherOps;

  ULONGLONG delta = otherOps - m_ioOtherOpsStart;
  m_ioOtherOpsStart = otherOps;
  m_ioLastOtherOpsUsage = (long) ((delta / (deltaMs / 1000.0)) / 100);

  return m_ioLastOtherOpsUsage;
}

////////////////////////////////////////////////////////////////////////////////

void DXProfiler::filetime_to_timeval(FILETIME* src, timeval* dst)
{
  ULARGE_INTEGER x;

  x.LowPart = src->dwLowDateTime;
  x.HighPart = src->dwHighDateTime;

  dst->tv_sec  = static_cast<long>(x.QuadPart / 10000000);
  dst->tv_usec = static_cast<long>((x.QuadPart % 10000000) / 10);
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXPainterHeaders.h"
#include "DXPlayerStatisticsData.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

DXPlayerStatisticsData::DXPlayerStatisticsData() :
m_legend(NULL),
m_abortLoading(false)
{
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerStatisticsData::~DXPlayerStatisticsData()
{
  Clear();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::Clear()
{
  ClearOnTheFly();
  
  m_abortLoading = false;
  m_legend = NULL;
  m_legendInfo.clear();
  m_counters.clear();
  FreePointBuffers();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::ClearOnTheFly()
{
  if (m_legend)
  {
    SetLegend(m_legend);
  }
  else
  {
    m_legendInfo.clear();
    m_counters.clear();
    FreePointBuffers();
  }
  
  m_legendOnTheFly = NULL;
  m_countersOnTheFly.clear();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::SetNotifier(ProgressNotifierPtr function)
{
  m_notifier = function;
}

////////////////////////////////////////////////////////////////////////////////

CXGraph* DXPlayerStatisticsData::GetGraph() const
{
  return m_graph;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::SetGraph(CXGraph* graph)
{
  m_graph = graph;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::AbortLoading()
{
  m_abortLoading = true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::LoadTraceStatistics(const string& traceFilename)
{
  bool problems = false;
  
  if (!m_graph)
  {
    return false;
  }

  Clear();
  m_graph->ResetAll();
  
  NotifyBegin("Loading");
  
  //////////////////////////////////////////////////////////////////////////////
  // Open the trace file
  
  DXTraceManager traceman;
  if (!traceman.OpenRead(traceFilename))
  {
    NotifyEnded();
    return false;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Load all the statistics from the trace
  
  for (unsigned int i=0; i < traceman.GetStatisticCount() && !m_abortLoading; ++i)
  {
    NotifyProgress("Loading", (unsigned int) (i*100.0 / traceman.GetStatisticCount()));

    DXStatisticPtr statistic = NULL;
    if (!traceman.ReadStatistic(&statistic, i))
    {
      problems = true;
      break;
    }
    
    if (!AddStatistic(statistic))
    {
      problems = true;
      break;
    }
  }

  if (problems || m_abortLoading || (GetCounterCount() == 0) || (GetStatisticsCount() == 0))
  {
    Clear();
    m_graph->ResetAll();
    NotifyEnded();
    return false;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Link data series to the graph
  
  if (!FillPointBuffers())
  {
    Clear();
    m_graph->ResetAll();
    NotifyEnded();
    return false;
  }
  
  for (unsigned int i=0; i < GetCounterCount() && !m_abortLoading; ++i)
  {
    TDataPoint* pointBuffer = NULL;
    if (GetPointBuffer(i, &pointBuffer) && pointBuffer)
    {
      m_graph->SetData(pointBuffer, GetStatisticsCount(), i);
      m_graph->GetCurve(i).SetLabel(GetLegendName(i));
    }
    else
    {
      problems = true;
      break;
    }
  }
  
  if (problems || m_abortLoading)
  {
    Clear();
    m_graph->ResetAll();
    NotifyEnded();
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Setup graph axis
  
  m_graph->GetXAxis(0).SetLabel(_T("Frame"));
  m_graph->GetYAxis(0).SetLabel(_T(""));
  m_graph->GetXAxis(0).SetDisplayFmt(_T("%5.0f"));
  m_graph->GetYAxis(0).SetDisplayFmt(_T("%5.0f"));
  
  //////////////////////////////////////////////////////////////////////////////
  
  m_abortLoading = false;
  
  NotifyEnded();
  
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::LoadTraceStatisticsOnTheFly(const std::string& traceFilename, dxplugin::DXIntPluginManager& plugman)
{
  bool problems = false;

  if (!m_graph)
  {
    return false;
  }

  ClearOnTheFly();
  m_graph->ResetAll();

  NotifyBegin("Loading");

  //////////////////////////////////////////////////////////////////////////////
  // Open the trace file

  DXTraceManager traceman;
  if (!traceman.OpenRead(traceFilename))
  {
    NotifyEnded();
    return false;
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Load all the statistics from the trace
  
  plugman.BeginExperiment();

  DXStatisticPtr statistic = new DXStatistic();
  if (!plugman.GetStatisticsLegend(statistic))
  {
    plugman.EndExperiment();
    NotifyEnded();
    return false;
  }
  
  if (!AddStatisticOnTheFly(statistic))
  {
    plugman.EndExperiment();
    NotifyEnded();
    return false;
  }
  
  for (unsigned int i=0; i < traceman.GetMethodCallCount(); ++i)
  {
    NotifyProgress("Loading", (unsigned int) (i*100.0 / traceman.GetMethodCallCount()));

    DXMethodCallPtr call = NULL;
    if (!traceman.ReadMethodCall(&call, i))
    {
      problems = true;
      break;
    }
    else
    {
      plugman.ProcessCall(call);
      
      if (call->GetToken() == DXMethodCallHelper::TOK_IDirect3DDevice9_Present ||
          call->GetToken() == DXMethodCallHelper::TOK_IDirect3DSwapChain9_Present)
      {
        DXStatisticPtr statistic = new DXStatistic();
        if (!plugman.GetStatisticsFrame(statistic))
        {
          problems = true;
          break;
        }

        if (!AddStatisticOnTheFly(statistic))
        {
          problems = true;
          break;
        }
      }
    }
  }

  plugman.EndExperiment();
  
  if (problems || (GetCounterCount() == 0) || (GetStatisticsCount() == 0))
  {
    Clear();
    m_graph->ResetAll();
    NotifyEnded();
    return false;
  } 
  
  //////////////////////////////////////////////////////////////////////////////
  // Link new data series to the graph

  if (!FillPointBuffers())
  {
    Clear();
    m_graph->ResetAll();
    NotifyEnded();
    return false;
  }
  
  for (unsigned int i=0; i < GetCounterCount(); ++i)
  {
    TDataPoint* pointBuffer = NULL;
    if (GetPointBuffer(i, &pointBuffer) && pointBuffer)
    {
      m_graph->SetData(pointBuffer, GetStatisticsCount(), i);
      m_graph->GetCurve(i).SetLabel(GetLegendName(i));
    }
    else
    {
      problems = true;
      break;
    }
  }

  if (problems)
  {
    Clear();
    m_graph->ResetAll();
    NotifyEnded();
    return false;
  }

  //////////////////////////////////////////////////////////////////////////////
  // Setup graph axis

  m_graph->GetXAxis(0).SetLabel(_T("Frame"));
  m_graph->GetYAxis(0).SetLabel(_T(""));
  m_graph->GetXAxis(0).SetDisplayFmt(_T("%5.0f"));
  m_graph->GetYAxis(0).SetDisplayFmt(_T("%5.0f"));
  
  //////////////////////////////////////////////////////////////////////////////

  NotifyEnded();

  return true;
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXPlayerStatisticsData::GetCounterCount() const
{
  return (unsigned int) m_legendInfo.size();
}

////////////////////////////////////////////////////////////////////////////////

unsigned int DXPlayerStatisticsData::GetStatisticsCount() const
{
  if (m_counters.size() == 0)
  {
    return (unsigned int) m_countersOnTheFly.size();
  }
  else
  {
    return (unsigned int) m_counters.size();
  }
}

////////////////////////////////////////////////////////////////////////////////

char* DXPlayerStatisticsData::GetLegendName(unsigned int position) const
{
  if (position < (unsigned int) m_legendInfo.size())
  {
    return m_legendInfo[position]->Name;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

char* DXPlayerStatisticsData::GetLegendDescription(unsigned int position) const
{
  if (position < (unsigned int) m_legendInfo.size())
  {
    return m_legendInfo[position]->Description;
  }
  return NULL;
}

////////////////////////////////////////////////////////////////////////////////

DXStatistic::StatisticDataType DXPlayerStatisticsData::GetLegendType(unsigned int position) const
{
  if (position < (unsigned int) m_legendInfo.size())
  {
    return m_legendInfo[position]->DataType;
  }
  return DXStatistic::SDT_UINT32;
}

////////////////////////////////////////////////////////////////////////////////

char* DXPlayerStatisticsData::GetLegendTypeText(unsigned int position) const
{
  switch (GetLegendType(position))
  {
  default:
  case DXStatistic::SDT_UINT32:
    return "DXICDT_UINT32";
  case DXStatistic::SDT_UINT64:
    return "DXICDT_UINT64";
  case DXStatistic::SDT_FLOAT:
    return "DXICDT_FLOAT";
  case DXStatistic::SDT_DOUBLE:
    return "DXICDT_DOUBLE";
  case DXStatistic::SDT_STRING:
    return "DXICDT_STRING";
  }
}

////////////////////////////////////////////////////////////////////////////////

double DXPlayerStatisticsData::GetPointMIN(unsigned int position) const
{
  if (position < (unsigned int) m_pointsMinMax.size())
  {
    return m_pointsMinMax[position].fXVal;
  }
  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////

double DXPlayerStatisticsData::GetPointMAX(unsigned int position) const
{
  if (position < (unsigned int) m_pointsMinMax.size())
  {
    return m_pointsMinMax[position].fYVal;
  }
  return 0.0;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::AddStatistic(DXStatisticPtr statistic)
{
  switch (statistic->GetType())
  {
  case DXStatistic::ST_LEGEND:
    return SetLegend(statistic);
    break;

  case DXStatistic::ST_COUNTER:
    m_counters.push_back(statistic);
    return true;
    break;

  default:
    return false;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::AddStatisticOnTheFly(DXStatisticPtr statistic)
{
  switch (statistic->GetType())
  {
  case DXStatistic::ST_LEGEND:
    return SetLegendOnTheFly(statistic);
    break;

  case DXStatistic::ST_COUNTER:
    m_countersOnTheFly.push_back(statistic);
    return true;
    break;

  default:
    return false;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::GetPointBuffer(unsigned int position, TDataPoint** buffer)
{
  if (!buffer)
  {
    return false;
  }
  else
  {
    *buffer = NULL;
  }
  
  if (position < (unsigned int) m_points.size())
  {
    *buffer = m_points[position];
    return true;
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::FillPointBuffers()
{
  bool problems = false;
  
  NotifyBegin("Filling Graph");
  
  if (!CreatePointBuffers())
  {
    return false;
  }

  dx_uint32 type1;
  dx_uint64 type2;
  dx_float  type3;
  dx_double type4;

  for (unsigned int i=0; i < GetStatisticsCount(); ++i)
  {
    NotifyProgress("Filling Graph", (unsigned int) (i*100.0 / GetStatisticsCount()));
    
    for (unsigned int j=0; j < GetCounterCount(); ++j)
    {
      DXStatisticPtr counter = NULL;
      unsigned int counter_id = 0;
      if (j < ((bool) m_legend ? m_legend->GetLegendCount() : 0))
      {
        counter = m_counters[i];
        counter_id = j;
      }
      else
      {
        counter = m_countersOnTheFly[i];
        counter_id = j - ((bool) m_legend ? m_legend->GetLegendCount() : 0);
      }
      
      m_points[j][i].fXVal = static_cast<double>(i+1);

      switch (m_legendInfo[j]->DataType)
      {
      case DXStatistic::SDT_UINT32:
        if (!counter->GetCounterData(counter_id, &type1))
        {
          problems = true;
          break;
        }
        else
        {
          m_points[j][i].fYVal = static_cast<double>(type1);
        }
        break;
      
      case DXStatistic::SDT_UINT64:
        if (!counter->GetCounterData(counter_id, &type2))
        {
          problems = true;
          break;
        }
        else
        {
          m_points[j][i].fYVal = static_cast<double>(type2);
        }
        break;
      
      case DXStatistic::SDT_FLOAT:
        if (!counter->GetCounterData(counter_id, &type3))
        {
          problems = true;
          break;
        }
        else
        {
          m_points[j][i].fYVal = static_cast<double>(type3);
        }
        break;
      
      case DXStatistic::SDT_DOUBLE:
        if (!counter->GetCounterData(counter_id, &type4))
        {
          problems = true;
          break;
        }
        else
        {
          m_points[j][i].fYVal = static_cast<double>(type4);
        }
        break;
      
      case DXStatistic::SDT_STRING:
        m_points[j][i].fYVal = 0.0;
        break;
      }
      
      m_pointsMinMax[j].fXVal = min(m_pointsMinMax[j].fXVal, m_points[j][i].fYVal); // save X as min
      m_pointsMinMax[j].fYVal = max(m_pointsMinMax[j].fYVal, m_points[j][i].fYVal); // save Y as min
    }

    if (problems)
    {
      break;
    }
  }

  NotifyEnded();
  
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::CreatePointBuffers()
{
  bool problems = false;

  FreePointBuffers();
  
  for (unsigned int i=0; i < GetCounterCount() && !m_abortLoading; ++i)
  {
    TDataPoint* pointBuffer = new TDataPoint[GetStatisticsCount()];
    if (!pointBuffer)
    {
      problems = true;
      break;
    }
    else
    {
      memset((char*) pointBuffer, 0, sizeof(TDataPoint) * GetStatisticsCount());
      m_points.push_back(pointBuffer);
      m_pointsMinMax.push_back(TDataPoint(0.0, 0.0));
    }
  }

  if (problems || m_abortLoading)
  {
    FreePointBuffers();
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::FreePointBuffers()
{
  for (PointBufferVector::iterator it=m_points.begin(); it != m_points.end(); ++it)
  {
    if (*it)
    {
      delete[] *it;
      *it = NULL;
    }
  }
  m_points.clear();
  m_pointsMinMax.clear();
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::SetLegend(DXStatisticPtr statistic)
{
  m_legend = statistic;
  m_legendInfo.clear();
  
  for (unsigned int j=0; j < m_legend->GetLegendCount(); ++j)
  {
    DXStatistic::DXStatisticLegendInformation* legendInfo;
    if (m_legend->GetLegend(j, &legendInfo))
    {
      m_legendInfo.push_back(legendInfo);
    }
  }
  
  return (GetCounterCount() > 0);
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::SetLegendOnTheFly(DXStatisticPtr statistic)
{
  m_legendOnTheFly = statistic;

  for (unsigned int j=0; j < m_legendOnTheFly->GetLegendCount(); ++j)
  {
    DXStatistic::DXStatisticLegendInformation* legendInfo;
    if (m_legendOnTheFly->GetLegend(j, &legendInfo))
    {
      strncat(legendInfo->Name, " (dynamic)", min(10, sizeof(legendInfo->Name)-strlen(legendInfo->Name)-1));
      m_legendInfo.push_back(legendInfo);
    }
  }

  return (GetCounterCount() > 0);
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticsData::ExportStatisticsToCSV(const CString& fileName) const
{
  bool problems = false;

  if (!m_points.size())
  {
    return false;
  }

  NotifyBegin("Exporting");
  
  //////////////////////////////////////////////////////////////////////////////
  // Create output file

  ofstream fileCSV(fileName.GetString(), ios::out | ios::trunc);
  
  if (!fileCSV.is_open())
  {
    return false;
  }

  fileCSV.imbue(std::locale("esp"));
  
  //////////////////////////////////////////////////////////////////////////////
  // Export Table Header

  fileCSV << "\"Frame\"";
  
  for (unsigned int j=0; j < GetCounterCount(); ++j)
  {
    fileCSV << ";";
    ExportaStatisicsToCSVWriteString(fileCSV, m_legendInfo[j]->Name);
  }

  fileCSV << endl;
  
  //////////////////////////////////////////////////////////////////////////////
  // Export the table contents
  
  dx_uint32 type1;
  dx_uint64 type2;
  dx_float  type3;
  dx_double type4;
  char*     type5;
  
  for (unsigned int i=0; i < GetStatisticsCount(); ++i)
  {
    NotifyProgress("Exporting", (unsigned int) (i*100.0 / GetStatisticsCount()));
    
    fileCSV << i+1;
    
    for (unsigned int j=0; j < GetCounterCount(); ++j)
    {
      DXStatisticPtr counter = NULL;
      unsigned int counter_id = 0;
      if (j < ((bool) m_legend ? m_legend->GetLegendCount() : 0))
      {
        counter = m_counters[i];
        counter_id = j;
      }
      else
      {
        counter = m_countersOnTheFly[i];
        counter_id = j - ((bool) m_legend ? m_legend->GetLegendCount() : 0);
      }
      
      fileCSV << ";";
      
      switch (m_legendInfo[j]->DataType)
      {
      case DXStatistic::SDT_UINT32:
        if (!counter->GetCounterData(counter_id, &type1))
        {
          problems = true;
          break;
        }
        else
        {
          fileCSV << dec << type1;
        }
        break;
      
      case DXStatistic::SDT_UINT64:
        if (!counter->GetCounterData(counter_id, &type2))
        {
          problems = true;
          break;
        }
        else
        {
          fileCSV << dec << type2;
        }
        break;
      
      case DXStatistic::SDT_FLOAT:
        if (!counter->GetCounterData(counter_id, &type3))
        {
          problems = true;
          break;
        }
        else
        {
          fileCSV << fixed << setprecision(6) << type3;
        }
        break;
      
      case DXStatistic::SDT_DOUBLE:
        if (!counter->GetCounterData(counter_id, &type4))
        {
          problems = true;
          break;
        }
        else
        {
          fileCSV << fixed << setprecision(6) << type4;
        }
        break;
      
      case DXStatistic::SDT_STRING:
        if (!counter->GetCounterData(counter_id, &type5))
        {
          problems = true;
          break;
        }
        else
        {
          ExportaStatisicsToCSVWriteString(fileCSV, type5);
        }
        break;
      }
    }

    if (problems)
    {
      break;
    }

    fileCSV << endl;
  }

  //////////////////////////////////////////////////////////////////////////////

  fileCSV.close();

  NotifyEnded();
  
  return !problems;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::ExportaStatisicsToCSVWriteString(ostream& stream, const char* str) const
{
  string cadena(str);
  
  if (cadena.length() == 0)
  {
    stream << "\"\"";
  }

  stream << "\"";
  
  for (unsigned int i=0; i < cadena.length(); ++i)
  {
    if (cadena[i] == '\"')
    {
      stream << "\"\"";
    }
    else
    {
      stream << cadena[i];
    }
  }

  stream << "\"";
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::NotifyBegin(char* message) const
{
  if (m_notifier)
  {
    m_notifier(PT_BEGIN, message, 0);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::NotifyProgress(char* message, unsigned int percen) const
{
  if (m_notifier)
  {
    m_notifier(PT_VALUE, message, percen);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticsData::NotifyEnded() const
{
  if (m_notifier)
  {
    m_notifier(PT_ENDED, "", 0);
  }
}

////////////////////////////////////////////////////////////////////////////////

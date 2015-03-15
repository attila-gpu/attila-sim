////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "xgraph.h"
#include "FastDelegate.h"
#include "DXIntPluginManager.h"

////////////////////////////////////////////////////////////////////////////////

class DXPlayerStatisticsData
{
public:

  enum ProgressType
  {
    PT_BEGIN = 1,
    PT_VALUE = 2,
    PT_ENDED = 3,
  };
  
  typedef fastdelegate::FastDelegate3<ProgressType, char*, unsigned int> ProgressNotifierPtr;
  
  DXPlayerStatisticsData();
  virtual ~DXPlayerStatisticsData();

  void Clear();

  void SetNotifier(ProgressNotifierPtr function);
  CXGraph* GetGraph() const;
  void SetGraph(CXGraph* graph);
  
  void AbortLoading();
  bool LoadTraceStatistics(const std::string& traceFilename);
  bool LoadTraceStatisticsOnTheFly(const std::string& traceFilename, dxplugin::DXIntPluginManager& plugman);
  
  unsigned int GetCounterCount() const;
  unsigned int GetStatisticsCount() const;
  
  char* GetLegendName(unsigned int position) const;
  char* GetLegendDescription(unsigned int position) const;
  dxtraceman::DXStatistic::StatisticDataType GetLegendType(unsigned int position) const;
  char* GetLegendTypeText(unsigned int position) const;
  double GetPointMIN(unsigned int position) const;
  double GetPointMAX(unsigned int position) const;

  bool ExportStatisticsToCSV(const CString& fileName) const;
  void ExportaStatisicsToCSVWriteString(ostream& stream, const char* str) const;

private:

  typedef std::vector<dxtraceman::DXStatistic::DXStatisticLegendInformation*> LegendInfoVector;
  typedef std::vector<dxtraceman::DXStatisticPtr> CounterVector;
  typedef std::vector<TDataPoint*> PointBufferVector;
  typedef std::vector<TDataPoint> PointBufferMinMaxVector;
  
  dxtraceman::DXStatisticPtr m_legend;
  dxtraceman::DXStatisticPtr m_legendOnTheFly;
  LegendInfoVector m_legendInfo;
  CounterVector m_counters;
  CounterVector m_countersOnTheFly;
  PointBufferVector m_points;
  PointBufferMinMaxVector m_pointsMinMax;
  ProgressNotifierPtr m_notifier;
  bool m_abortLoading;
  CXGraph* m_graph;

  void ClearOnTheFly();
  bool AddStatistic(dxtraceman::DXStatisticPtr statistic);
  bool AddStatisticOnTheFly(dxtraceman::DXStatisticPtr statistic);
  bool GetPointBuffer(unsigned int position, TDataPoint** buffer);
  
  bool FillPointBuffers();
  bool CreatePointBuffers();
  void FreePointBuffers();
  bool SetLegend(dxtraceman::DXStatisticPtr statistic);
  bool SetLegendOnTheFly(dxtraceman::DXStatisticPtr statistic);
  
  void NotifyBegin(char* message) const;
  void NotifyProgress(char* message, unsigned int percen) const;
  void NotifyEnded() const;

};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "xgraph10.h"
#include "LayoutHelper.h"
#include "RollupCtrl.h"
#include "DXPlayerStatisticsData.h"
#include "DXPlayerStatisticViewerRollupDialogs.h"

////////////////////////////////////////////////////////////////////////////////

class DXPlayerStatisticViewerDlg : public CDialog
{
public:
  
  DXPlayerStatisticViewerDlg(const std::string& traceFilePath, CWnd* pParent = NULL);
  virtual ~DXPlayerStatisticViewerDlg();

protected:
  
  friend class CStatisticCurveOptionsDlg;
  friend class CStatisticGraphOptionsDlg;
  friend class CStatisticExportOptionsDlg;
  friend class CStatisticOnTheFlyOptionsDlg;
  
  HICON m_hIcon;
  std::string m_traceFilePath;
  CXGraph m_graph;
  CLayoutHelper* m_layoutHelper;
  CRollupCtrl	m_rollup;
  DXPlayerStatisticsData m_dataman;
  CString m_defaultTitle;

  CStatisticCurveOptionsDlg m_dlgCurveOptions;
  CStatisticGraphOptionsDlg m_dlgGraphOptions;
  CStatisticExportOptionsDlg m_dlgExportOptions;
  CStatisticOnTheFlyOptionsDlg m_dlgOnTheFlyOptions;

  HANDLE m_thread;
  bool m_loadingStatistics;
  bool m_killingThread;
  void ThreadStart();
  bool ThreadEnd();
  static unsigned __stdcall ThreadWork(LPVOID lpParam);

  void LoadTraceStatistics();
  bool LoadTraceStatisticsOnTheFly(dxplugin::DXIntPluginManager& plugman);
  void ProgressUpdate(DXPlayerStatisticsData::ProgressType type, char* message, unsigned int value);
  
  afx_msg LRESULT UpdateWindowObjects(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnZoomChange(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnPanChange(WPARAM wParam, LPARAM lParam);
  afx_msg LRESULT OnMeasureCreated(WPARAM wParam, LPARAM lParam);
  
  virtual void PostNcDestroy();
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnCancel();
  virtual BOOL OnInitDialog();
  virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
  
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

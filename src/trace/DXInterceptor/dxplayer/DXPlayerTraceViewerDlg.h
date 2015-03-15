////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "TreeListCtrlHeaders.h"

////////////////////////////////////////////////////////////////////////////////

class DXPlayerTraceViewerDlg : public CDialog
{
public:
  
  DXPlayerTraceViewerDlg(const CString& traceFilePath, CWnd* pParent = NULL);

private:
  
  CString m_traceFilePath;
  CString m_dumpFileName;
  CProgressCtrl m_progress;

  HANDLE m_thread;
  bool m_dumpingTrace;
  bool m_killingThread;
  void ThreadStart();
  bool ThreadEnd();
  static unsigned __stdcall ThreadWork(LPVOID lpParam);
  LRESULT ThreadEnded(WPARAM wParam, LPARAM lParam);
  
  bool GetSaveFileNameTXT(CString& fileName);
  bool DumpTrace();

  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  
  afx_msg void OnDumpTrace();
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

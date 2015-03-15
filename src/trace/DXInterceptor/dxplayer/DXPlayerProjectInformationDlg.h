////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

class DXPlayerProjectInformationDlg : public CDialog
{
public:

  DXPlayerProjectInformationDlg(dxpainter::DXPainter& painter, CWnd* pParent = NULL);
  virtual ~DXPlayerProjectInformationDlg();

private:

  dxpainter::DXPainter& m_painter;
  CString m_projectName;
  CString m_projectAnotations;
  
  bool LoadInformation();
  bool SaveInformation();
  
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);
  
  afx_msg void OnBtnSave();
  afx_msg void OnDestroy();
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

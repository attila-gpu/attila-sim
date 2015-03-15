////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "resource.h"

////////////////////////////////////////////////////////////////////////////////

class DXPlayerOptionsDlg : public CDialog
{
public:

  DXPlayerOptionsDlg(CString optionsFilename, CWnd* pParent = NULL);
  virtual ~DXPlayerOptionsDlg();

protected:

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);
  
  afx_msg void OnBtnBrowse();
  afx_msg void OnBtnSave();
  afx_msg void OnDestroy();
  
  DECLARE_MESSAGE_MAP()

private:

  CString m_optionsFilename;
  DXPlayerOptions m_options;
  
  void FillCombos();
  void ApplyOptions();
  void RetrieveOptions();
  bool LoadOptions();
  bool SaveOptions();

};

////////////////////////////////////////////////////////////////////////////////

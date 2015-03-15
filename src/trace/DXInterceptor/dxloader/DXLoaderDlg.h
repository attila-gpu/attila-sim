////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class XMLConfig;

class DXLoaderDlg : public CDialog
{
public:
  
  DXLoaderDlg(CWnd* pParent = NULL);
  virtual ~DXLoaderDlg();

protected:
  
  HICON m_hIcon;
  CString m_strPath;
  XMLConfig* m_config;

  virtual BOOL OnInitDialog();
  afx_msg void OnPaint();
  afx_msg void OnBtnBrowse();
  afx_msg void OnBtnOptions();
  afx_msg void OnBtnRun();
  afx_msg void DoDataExchange(CDataExchange* pDX);
  afx_msg void OnDestroy();
  
  DECLARE_MESSAGE_MAP()

private:

  std::vector<std::string> m_detectedDLLs;
  
  void DetectDLLs(std::vector<std::string>& v_dlls);
  void DetectDXInterceptorDLLs(std::vector<std::string>& v_dllsName);
  bool CheckDXInterceptorDLL(const std::string& filename);
  void GetDXInterceptorDLLVersion(const std::string& filename, std::string& version);
  void FillCombo();

  void LoadSettings();
  void SaveSettings();

  CString StripPath(CString& filename);
  CString GetPathExecutable();
  CString GetPathDll();
  
};

////////////////////////////////////////////////////////////////////////////////


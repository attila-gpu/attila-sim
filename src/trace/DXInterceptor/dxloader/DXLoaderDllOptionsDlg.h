////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "ColourPickerXP.h"
#include "TreeListCtrlHeaders.h"
#include "DXIntPluginManager.h"

////////////////////////////////////////////////////////////////////////////////

class DXLoaderDllOptionsDlg : public CDialog
{
public:
  
  DXLoaderDllOptionsDlg(CString pathDLL, CWnd* pParent = NULL);
  virtual ~DXLoaderDllOptionsDlg();

protected:
  
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX); // DDX/DDV support
  
  afx_msg void OnBtnBrowse();
  afx_msg void OnChkShowBanner();
  afx_msg void OnBtnSave();
  
  DECLARE_MESSAGE_MAP()

private:

  CString m_pathDLL;
  DXInterceptorOptions m_options;
  CColourPickerXP m_colorBox;
  CImageList m_statCountersImages;
  CTreeListCtrl m_statCounters;
  dxplugin::DXIntPluginManager m_plugman;
  
  CString StripPath(CString& filename);
  CString GetOptionsFilename();
  void SetTitle(CString title);
  void FillCombos();
  void InitStatisticsTree();
  void FillStatisticsTree();
  void ApplyOptions();
  void RetrieveOptions();
  void EnableBannerOptions(bool enable);

  bool LoadOptions();
  bool SaveOptions();

  bool LoadStatisticsTree();
  bool SaveStatisticsTree();

  COLORREF D3DCOLOR_To_COLORREF(D3DCOLOR color);
  D3DCOLOR COLORREF_To_D3DCOLOR(COLORREF color);

};

////////////////////////////////////////////////////////////////////////////////


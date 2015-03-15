////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "ColourPickerXP.h"
#include "PPTooltip.h"
#include "DXPainterHeaders.h"
#include "DXPlayerStatisticsData.h"
#include "DXIntPluginManager.h"

////////////////////////////////////////////////////////////////////////////////

class CStatisticCurveOptionsDlg : public CDialog
{
public:

  CStatisticCurveOptionsDlg(const DXPlayerStatisticsData& dataman, CWnd* pParent = NULL);

  void Reset();
  void Refresh();

protected:

  const DXPlayerStatisticsData& m_dataman;
  CPPToolTip m_tooltip;
  CComboBox	m_curveList;
  CColourPickerXP m_curveColor;
  int m_curveStyle;
  CSpinButtonCtrl	m_curveWidthSpin;
  int m_curveWidth;
  BOOL m_curveShowMarkers;
  int m_curveMarkerType;
  CComboBox	m_curveMarkerSymbolCombo;
  int m_curveMarkerSymbol;
  BOOL m_curveVisible;
  CString m_valueMinY;
  CString m_valueMaxY;
  
  void EnableControls();
  void GetCurveProperties(unsigned int curveID);
  void SetCurveProperties(unsigned int curveID);

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnOK()     {} // Rollup Control
  virtual void OnCancel() {} // Rollup Control
  virtual BOOL PreTranslateMessage(MSG* pMsg);

  afx_msg void OnHideAllCurvesExceptThis();
  afx_msg void OnHideAllCurves();
  afx_msg void OnShowAllCurves();
  afx_msg void OnChangedVisible();
  afx_msg void OnChangedRead();
  afx_msg void OnChangedWrite();
  afx_msg LRESULT OnColorChange(WPARAM lParam, LPARAM wParam);
  afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
  afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
  afx_msg void NotifyDisplayTooltip(NMHDR* pNMHDR, LRESULT* result);
  afx_msg void OnDestroy();
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

class CStatisticGraphOptionsDlg : public CDialog
{
public:
  
  CStatisticGraphOptionsDlg(const DXPlayerStatisticsData& dataman, CWnd* pParent = NULL);

  void Reset();
  void Refresh();

protected:
  
  const DXPlayerStatisticsData& m_dataman;
  BOOL m_allRangeAxisX;
  BOOL m_allRangeAxisY;
  CSliderCtrl m_slideAxisX;
  CSliderCtrl m_slideAxisY;
  UINT m_rangeMinAxisX;
  UINT m_rangeMaxAxisX;
  UINT m_rangeMinAxisY;
  UINT m_rangeMaxAxisY;
  
  void EnableControls();
  void UpdateSlides();
  
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnOK()     {} // Rollup Control
  virtual void OnCancel() {} // Rollup Control

  afx_msg void OnChangedRead();
  afx_msg void OnChangedWrite();
  afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

class CStatisticExportOptionsDlg : public CDialog
{
public:

  CStatisticExportOptionsDlg(const DXPlayerStatisticsData& dataman, CWnd* pParent = NULL);

  void Reset();
  void Refresh();

protected:

  const DXPlayerStatisticsData& m_dataman;

  bool GetSaveFileNameBMP(CString& fileName);
  bool GetSaveFileNameCSV(CString& fileName);

  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnOK()     {} // Rollup Control
  virtual void OnCancel() {} // Rollup Control

  afx_msg void OnSaveBitmap();
  afx_msg void OnExportCSV();
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

class CStatisticOnTheFlyOptionsDlg : public CDialog
{
public:

  CStatisticOnTheFlyOptionsDlg(const DXPlayerStatisticsData& dataman, CWnd* pParent = NULL);

  void Reset();
  void Refresh();

protected:

  const DXPlayerStatisticsData& m_dataman;
  dxplugin::DXIntPluginManager m_plugman;
  CCheckListBox m_pluginList;
  std::vector<pair<dxplugin::DXIntPluginLoaded*, DXINTCOUNTERID> > m_counters;

  void FillPluginList();
  
  virtual BOOL OnInitDialog();
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual void OnOK()     {} // Rollup Control
  virtual void OnCancel() {} // Rollup Control

  afx_msg void OnPluginCheck();
  afx_msg void OnDoCalculations();
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

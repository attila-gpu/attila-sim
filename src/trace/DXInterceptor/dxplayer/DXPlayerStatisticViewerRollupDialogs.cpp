////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "RollupCtrl.h"
#include "GfxUtils.h"
#include "ThemeHelperST.h"
#include "DXPlayerStatisticViewerRollupDialogs.h"
#include "DXPlayerStatisticViewerDlg.h"

////////////////////////////////////////////////////////////////////////////////

void EnableAllChildWindows(CWnd* parent, bool enable = true)
{
  CWnd* pChildWnd = parent->GetWindow(GW_CHILD);
  while (pChildWnd != NULL && ::IsWindow(pChildWnd->m_hWnd))
  {
    pChildWnd->EnableWindow(enable == TRUE);
    pChildWnd = pChildWnd->GetWindow(GW_HWNDNEXT);
  }
}

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStatisticCurveOptionsDlg, CDialog)
  ON_WM_DRAWITEM()
  ON_WM_MEASUREITEM()
  ON_WM_DESTROY()
  ON_CBN_SELCHANGE(IDC_CBO_CURVE_LIST, OnChangedRead)
  ON_MESSAGE(CPN_SELENDOK, OnColorChange)
  ON_CBN_SELCHANGE(IDC_CBO_CURVE_STYLE, OnChangedWrite)
  ON_EN_CHANGE(IDC_EDT_CURVE_WIDTH, OnChangedWrite)
  ON_BN_CLICKED(IDC_CHK_SHOW_MARKERS, OnChangedWrite)
  ON_BN_CLICKED(IDC_RAD_MARKERS_NUMBERED, OnChangedWrite)
  ON_BN_CLICKED(IDC_RAD_MARKERS_SYMBOL, OnChangedWrite)
  ON_CBN_SELCHANGE(IDC_CBO_MARKERS_SYMBOL, OnChangedWrite)
  ON_BN_CLICKED(IDC_CHK_CURVE_VISIBLE, OnChangedVisible)
  ON_NOTIFY(UDM_TOOLTIP_DISPLAY, NULL, NotifyDisplayTooltip)
  ON_BN_CLICKED(IDC_BTN_HIDE_ALL_EXCEPT_THIS, OnHideAllCurvesExceptThis)
  ON_BN_CLICKED(IDC_BTN_HIDE_ALL, OnHideAllCurves)
  ON_BN_CLICKED(IDC_BTN_SHOW_ALL, OnShowAllCurves)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CStatisticCurveOptionsDlg::CStatisticCurveOptionsDlg(const DXPlayerStatisticsData& dataman, CWnd* pParent) :
CDialog(IDD_STATISTICVIEWER_ROLLUP_CURVE_OPTIONS, pParent),
m_dataman(dataman)
{
  m_curveStyle = 0;
  m_curveWidth = 1;
  m_curveShowMarkers = FALSE;
  m_curveMarkerType = 0;
  m_curveMarkerSymbol = 0;
  m_curveVisible = FALSE;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CStatisticCurveOptionsDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();

  m_tooltip.Create(this);
  
  m_tooltip.AddTool(GetDlgItem(IDC_CBO_CURVE_LIST), "");
  m_tooltip.SetColorBk(RGB(255, 255, 148), RGB(255, 231, 132), RGB(255, 207, 0));
  m_tooltip.SetBorder(RGB(255, 207, 0));
  m_tooltip.SetEffectBk(CPPDrawManager::EFFECT_SOFTBUMP);
  m_tooltip.SetTooltipShadow(0, 0);
  m_tooltip.SetDirection(PPTOOLTIP_TOPEDGE_RIGHT);
  m_tooltip.SetNotify();
  m_tooltip.SetMaxTipWidth(300);
  
  m_curveColor.SetStyle(true);
  m_curveColor.SetTrackSelection(true);
  m_curveWidthSpin.SetRange(1, 5);

  EnableAllChildWindows(this, false);
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_CBO_CURVE_LIST, m_curveList);
  DDX_Control(pDX, IDC_CBO_CURVE_COLOR, m_curveColor);
  DDX_CBIndex(pDX, IDC_CBO_CURVE_STYLE, m_curveStyle);
  DDX_Control(pDX, IDC_SPN_CURVE_WIDTH, m_curveWidthSpin);
  DDX_Text(pDX, IDC_EDT_CURVE_WIDTH, m_curveWidth);
  DDX_Check(pDX, IDC_CHK_SHOW_MARKERS, m_curveShowMarkers);
  DDX_Radio(pDX, IDC_RAD_MARKERS_NUMBERED, m_curveMarkerType);
  DDX_Control(pDX, IDC_CBO_MARKERS_SYMBOL, m_curveMarkerSymbolCombo);
  DDX_CBIndex(pDX, IDC_CBO_MARKERS_SYMBOL, m_curveMarkerSymbol);
  DDX_Check(pDX, IDC_CHK_CURVE_VISIBLE, m_curveVisible);
  DDX_Text(pDX, IDC_LBL_MIN_Y_VALUE, m_valueMinY);
  DDX_Text(pDX, IDC_LBL_MAX_Y_VALUE, m_valueMaxY);

  DDV_MinMaxUInt(pDX, m_curveWidth, 1, 5);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::Reset()
{
  m_curveList.ResetContent();
  m_curveMarkerSymbolCombo.ResetContent();
  EnableAllChildWindows(this, false);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::Refresh()
{
  //////////////////////////////////////////////////////////////////////////////
  // Fill the curves combo
  
  if (m_dataman.GetCounterCount() == 0)
  {
    EnableAllChildWindows(this, false);
    return;
  }
  else
  {
    EnableAllChildWindows(this, true);
  }
  
  m_curveList.ResetContent();
  for (unsigned int i=0; i < m_dataman.GetCounterCount(); ++i)
  {
    m_curveList.AddString(m_dataman.GetLegendName(i));
  }

  //////////////////////////////////////////////////////////////////////////////
  // Fill curve style combo
  
  m_curveMarkerSymbolCombo.ResetContent();
  for (unsigned int i=0; i < MAX_MARKERS; ++i)
  {
    m_curveMarkerSymbolCombo.AddString("");
  }
  
  //////////////////////////////////////////////////////////////////////////////

  m_curveList.SetCurSel(0);
  OnChangedRead();

  //////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnChangedRead() 
{
  GetCurveProperties(m_curveList.GetCurSel());
  EnableControls();
  UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnChangedWrite() 
{
  if (!::IsWindow(m_hWnd) || !IsWindowVisible())
  {
    return;
  }

  UpdateData(TRUE);
  EnableControls();
  SetCurveProperties(m_curveList.GetCurSel());
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnChangedVisible()
{
  OnChangedWrite();
  m_dataman.GetGraph()->AutoscaleVisible(-1, 0);
  ((DXPlayerStatisticViewerDlg*) GetParent()->GetParent())->m_dlgGraphOptions.Refresh();
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::EnableControls()
{
  GetDlgItem(IDC_CBO_CURVE_COLOR)->EnableWindow(m_curveVisible);
  GetDlgItem(IDC_CBO_CURVE_STYLE)->EnableWindow(m_curveVisible && m_curveWidth == 1);
  GetDlgItem(IDC_EDT_CURVE_WIDTH)->EnableWindow(m_curveVisible);
  GetDlgItem(IDC_SPN_CURVE_WIDTH)->EnableWindow(m_curveVisible);
  GetDlgItem(IDC_CHK_SHOW_MARKERS)->EnableWindow(m_curveVisible);
  GetDlgItem(IDC_RAD_MARKERS_NUMBERED)->EnableWindow(m_curveShowMarkers && m_curveVisible);
  GetDlgItem(IDC_RAD_MARKERS_SYMBOL)->EnableWindow(m_curveShowMarkers && m_curveVisible);
  GetDlgItem(IDC_CBO_MARKERS_SYMBOL)->EnableWindow(m_curveMarkerType == 1 && m_curveShowMarkers && m_curveVisible);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::GetCurveProperties(unsigned int curveID)
{
  if (!m_dataman.GetGraph())
  {
    return;
  }

  if (curveID > (unsigned int) m_dataman.GetGraph()->GetCurveCount())
  {
    return;
  }

  CXGraphDataSerie& curve = m_dataman.GetGraph()->GetCurve(curveID);

  m_curveColor.SetColor(curve.GetColor());
  m_curveStyle = curve.GetLineStyle();
  m_curveWidth = curve.GetLineSize();
  m_curveShowMarkers = curve.GetShowMarker();
  m_curveMarkerType = curve.GetMarkerType();
  m_curveMarkerSymbol = curve.GetMarker();
  m_curveVisible = curve.GetVisible();
  
  CString str;
  
  switch (m_dataman.GetLegendType(curveID))
  {
  case dxtraceman::DXStatistic::SDT_UINT32:
    str.Format("%.0f", m_dataman.GetPointMIN(curveID));
    m_valueMinY = str;
    str.Format("%.0f", m_dataman.GetPointMAX(curveID));
    m_valueMaxY = str;
    break;
  case dxtraceman::DXStatistic::SDT_UINT64:
    str.Format("%.0f", m_dataman.GetPointMIN(curveID));
    m_valueMinY = str;
    str.Format("%.0f", m_dataman.GetPointMAX(curveID));
    m_valueMaxY = str;
    break;
  case dxtraceman::DXStatistic::SDT_FLOAT:
    str.Format("%.3f", m_dataman.GetPointMIN(curveID));
    m_valueMinY = str;
    str.Format("%.3f", m_dataman.GetPointMAX(curveID));
    m_valueMaxY = str;
    break;
  case dxtraceman::DXStatistic::SDT_DOUBLE:
    str.Format("%.3g", m_dataman.GetPointMIN(curveID));
    m_valueMinY = str;
    str.Format("%.3g", m_dataman.GetPointMAX(curveID));
    m_valueMaxY = str;
    break;
  default:
  case dxtraceman::DXStatistic::SDT_STRING:
    m_valueMinY.Empty();
    m_valueMaxY.Empty();
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::SetCurveProperties(unsigned int curveID)
{
  if (!m_dataman.GetGraph())
  {
    return;
  }

  if (curveID > (unsigned int) m_dataman.GetGraph()->GetCurveCount())
  {
    return;
  }

  CXGraphDataSerie& curve = m_dataman.GetGraph()->GetCurve(curveID);
  
  curve.SetColor(m_curveColor.GetColor());
  curve.SetLineStyle(m_curveStyle);
  curve.SetLineSize(m_curveWidth);
  curve.SetShowMarker(m_curveShowMarkers == TRUE);
  curve.SetMarkerType(m_curveMarkerType);
  curve.SetMarker(m_curveMarkerSymbol) ;
  curve.SetVisible(m_curveVisible == TRUE);

  m_dataman.GetGraph()->Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

LRESULT CStatisticCurveOptionsDlg::OnColorChange(WPARAM lParam, LPARAM wParam)
{
  m_curveList.Invalidate();
  OnChangedWrite();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
  CDCEx dc;

  switch (nIDCtl)
  {
  case IDC_CBO_CURVE_LIST:
    {
      if (!m_dataman.GetGraph() ||
          !GetDlgItem(nIDCtl)->IsWindowEnabled() ||
          (UINT) m_dataman.GetGraph()->GetCurveCount() <= lpDrawItemStruct->itemID)
      {
        return;
      }

      CRect	rectItem(lpDrawItemStruct->rcItem);
      CRect	rectBlock(rectItem);
      CBrush rectBrush;
      rectBrush.CreateStockObject(BLACK_BRUSH);
      
      dc.Attach(lpDrawItemStruct->hDC);
      int nSaveDC = ::SaveDC(lpDrawItemStruct->hDC);

      // Draw the background
      
      dc.FillSolidRect(&lpDrawItemStruct->rcItem, (lpDrawItemStruct->itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_WINDOW)));     

      // Draw the color block
      
      rectBlock.right = rectBlock.left + 20;
      rectBlock.DeflateRect(CSize(2, 2));
      dc.FillSolidRect(&rectBlock, m_dataman.GetGraph()->GetCurve(lpDrawItemStruct->itemID).GetColor());
      dc.FrameRect(&rectBlock, &rectBrush);
      
      // Draw the text
      
      rectItem.left = rectBlock.right + 4;
      rectItem.top += 2;

      if (lpDrawItemStruct->itemState & ODS_DISABLED)
      {
        dc.SetTextColor(::GetSysColor(COLOR_GRAYTEXT));
      }
      dc.SetBkMode(TRANSPARENT);
      dc.TabbedTextOut(rectItem.left, rectItem.top, (LPCTSTR) lpDrawItemStruct->itemData, (int) strlen((LPCTSTR) lpDrawItemStruct->itemData), 0, NULL, 0);
      
      ::RestoreDC (lpDrawItemStruct->hDC, nSaveDC);
      dc.Detach ();
    }
    break;
  
  case IDC_CBO_CURVE_STYLE:
    {
      if (!GetDlgItem(nIDCtl)->IsWindowEnabled())
      {
        return;
      }

      if (lpDrawItemStruct->itemID < 0 || lpDrawItemStruct->itemID >= 5)
      {
        return;
      }

      dc.Attach(lpDrawItemStruct->hDC);
      int nSaveDC = ::SaveDC(lpDrawItemStruct->hDC);

      dc.FillSolidRect(&lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_WINDOW));
      
      {
        CPenSelector ps(0L, 1, &dc, lpDrawItemStruct->itemID);
        dc.MoveTo(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top + 6);
        dc.LineTo(lpDrawItemStruct->rcItem.right, lpDrawItemStruct->rcItem.top + 6);
      }

      ::RestoreDC (lpDrawItemStruct->hDC, nSaveDC);
      dc.Detach ();
    }
    break;

  case IDC_CBO_MARKERS_SYMBOL:
    {
      if (!m_dataman.GetGraph() || !GetDlgItem(nIDCtl)->IsWindowEnabled())
      {
        return;
      }
      
      if (lpDrawItemStruct->itemID < 0 || lpDrawItemStruct->itemID >= MAX_MARKERS)
      {
        return;
      }

      dc.Attach(lpDrawItemStruct->hDC);
      int nSaveDC = ::SaveDC(lpDrawItemStruct->hDC);

      dc.FillSolidRect(&lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_WINDOW));

      CPoint point;
      point.x = lpDrawItemStruct->rcItem.left + 2;
      point.y = lpDrawItemStruct->rcItem.top + 2;

      m_dataman.GetGraph()->m_pDrawFn[lpDrawItemStruct->itemID](point, 10, lpDrawItemStruct->itemState & ODS_DISABLED ? ::GetSysColor(COLOR_INACTIVEBORDER) : 0L, false, &dc);

      ::RestoreDC (lpDrawItemStruct->hDC, nSaveDC);
      dc.Detach ();
    }
    break;

  default:
    CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
    break;
  }      
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
  switch (nIDCtl)
  {
  case IDC_CBO_CURVE_STYLE:
    lpMeasureItemStruct->itemHeight = 16;
    lpMeasureItemStruct->itemWidth  = 100;
    break;

  case IDC_CBO_MARKERS_SYMBOL:
    lpMeasureItemStruct->itemHeight = 16;
    lpMeasureItemStruct->itemWidth  = 16;
    break;

  default:
    CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
    break;
  }      
}

////////////////////////////////////////////////////////////////////////////////

BOOL CStatisticCurveOptionsDlg::PreTranslateMessage(MSG* pMsg) 
{
  m_tooltip.RelayEvent(pMsg);
  return CDialog::PreTranslateMessage(pMsg);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::NotifyDisplayTooltip(NMHDR* pNMHDR, LRESULT* result)
{
  *result = 0;
  NM_PPTOOLTIP_DISPLAY* pNotify = (NM_PPTOOLTIP_DISPLAY*)pNMHDR;

  if (pNotify->hwndTool == NULL)
  {
    // Order to update a tooltip for a current Tooltip Help
    // He has not a handle of the window
    // If you want change tooltip's parameter than make it's here
  }
  else
  {
    // Order to update a tooltip for a specified window as tooltip's tool

    // Gets a ID of the window if needed
    UINT nID = CWnd::FromHandle(pNotify->hwndTool)->GetDlgCtrlID();
    bool isEnabled = CWnd::FromHandle(pNotify->hwndTool)->IsWindowEnabled() == TRUE;

    // Change a tooltip's parameters for a current window (control)
    switch (nID)
    {
    case IDC_CBO_CURVE_LIST:
      {
        if (isEnabled && m_curveList.GetCurSel() >= 0)
        {
          int curveID = m_curveList.GetCurSel();
          CString curveInfo;
          curveInfo += "<h2>Statistic Information</h2><br>";
          curveInfo += "<hr color=black><br>";
          curveInfo += "<b>Name:&nbsp;</b>";
          curveInfo += m_dataman.GetLegendName(curveID);
          curveInfo += "<br><b>Description:&nbsp;</b>";
          curveInfo += m_dataman.GetLegendDescription(curveID);
          curveInfo += "<br><b>Data Type:&nbsp;</b>";
          curveInfo += m_dataman.GetLegendTypeText(curveID);
          pNotify->ti->sTooltip = curveInfo.GetString();
        }
      }
      break;
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnDestroy()
{
  m_tooltip.HideTooltip();
  CDialog::OnDestroy();
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnHideAllCurvesExceptThis()
{
  if (m_curveList.GetCurSel() < 0)
  {
    return;
  }
  
  for (unsigned int i=0; i < (unsigned int) m_dataman.GetGraph()->GetCurveCount(); ++i)
  {
    if (i != m_curveList.GetCurSel())
    {
      m_dataman.GetGraph()->GetCurve(i).SetVisible(false);
    }
    else
    {
      m_dataman.GetGraph()->GetCurve(i).SetVisible(true);
    }
  }

  m_dataman.GetGraph()->AutoscaleVisible(-1, 0);
  ((DXPlayerStatisticViewerDlg*) GetParent()->GetParent())->m_dlgGraphOptions.Refresh();
  
  OnChangedRead();
  m_dataman.GetGraph()->Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnHideAllCurves()
{
  if (m_curveList.GetCurSel() < 0)
  {
    return;
  }

  for (unsigned int i=0; i < (unsigned int) m_dataman.GetGraph()->GetCurveCount(); ++i)
  {
    m_dataman.GetGraph()->GetCurve(i).SetVisible(false);
  }
  
  ((DXPlayerStatisticViewerDlg*) GetParent()->GetParent())->m_dlgGraphOptions.Refresh();
  
  OnChangedRead();
  m_dataman.GetGraph()->Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticCurveOptionsDlg::OnShowAllCurves()
{
  if (m_curveList.GetCurSel() < 0)
  {
    return;
  }

  for (unsigned int i=0; i < (unsigned int) m_dataman.GetGraph()->GetCurveCount(); ++i)
  {
    m_dataman.GetGraph()->GetCurve(i).SetVisible(true);
  }

  m_dataman.GetGraph()->AutoscaleAll(-1, 0);
  ((DXPlayerStatisticViewerDlg*) GetParent()->GetParent())->m_dlgGraphOptions.Refresh();
  
  OnChangedRead();
  m_dataman.GetGraph()->Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStatisticGraphOptionsDlg, CDialog)
  ON_BN_CLICKED(IDC_CHK_ALLRANGE_AXIS_X, OnChangedWrite)
  ON_BN_CLICKED(IDC_CHK_ALLRANGE_AXIS_Y, OnChangedWrite)
  ON_EN_CHANGE(IDC_EDT_MIN_AXIS_X, OnChangedWrite)
  ON_EN_CHANGE(IDC_EDT_MAX_AXIS_X, OnChangedWrite)
  ON_EN_CHANGE(IDC_EDT_MIN_AXIS_Y, OnChangedWrite)
  ON_EN_CHANGE(IDC_EDT_MAX_AXIS_Y, OnChangedWrite)
  ON_WM_HSCROLL()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CStatisticGraphOptionsDlg::CStatisticGraphOptionsDlg(const DXPlayerStatisticsData& dataman, CWnd* pParent) :
CDialog(IDD_STATISTICVIEWER_ROLLUP_GRAPH_OPTIONS, pParent),
m_dataman(dataman)
{
  m_allRangeAxisX = TRUE;
  m_allRangeAxisY = TRUE;
  m_rangeMinAxisX = 0;
  m_rangeMaxAxisX = 0;
  m_rangeMinAxisY = 0;
  m_rangeMaxAxisY = 0;
}

////////////////////////////////////////////////////////////////////////////////

BOOL CStatisticGraphOptionsDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
  
  // Disable themes for the slidebars
  ThemeHelperST themeHelper;
  if (themeHelper.IsAppThemed())
  {
    themeHelper.DisableWindowTheme(m_slideAxisX.m_hWnd);
    themeHelper.DisableWindowTheme(m_slideAxisY.m_hWnd);
  }

  EnableAllChildWindows(this, false);
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticGraphOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Check(pDX, IDC_CHK_ALLRANGE_AXIS_X, m_allRangeAxisX);
  DDX_Check(pDX, IDC_CHK_ALLRANGE_AXIS_Y, m_allRangeAxisY);
  DDX_Control(pDX, IDC_SLD_AXIS_X, m_slideAxisX);
  DDX_Control(pDX, IDC_SLD_AXIS_Y, m_slideAxisY);
  DDX_Text(pDX, IDC_EDT_MIN_AXIS_X, m_rangeMinAxisX);
  DDX_Text(pDX, IDC_EDT_MAX_AXIS_X, m_rangeMaxAxisX);
  DDX_Text(pDX, IDC_EDT_MIN_AXIS_Y, m_rangeMinAxisY);
  DDX_Text(pDX, IDC_EDT_MAX_AXIS_Y, m_rangeMaxAxisY);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticGraphOptionsDlg::Reset()
{
  EnableAllChildWindows(this, false);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticGraphOptionsDlg::Refresh()
{
  if (m_dataman.GetCounterCount() == 0)
  {
    EnableAllChildWindows(this, false);
    return;
  }
  else
  {
    EnableAllChildWindows(this, true);
  }
  
  if (m_dataman.GetGraph()->GetOperation() == CXGraph::opZoom ||
      m_dataman.GetGraph()->GetOperation() == CXGraph::opPan)
  {
    m_allRangeAxisX = FALSE;
    m_allRangeAxisY = FALSE;
  }

  OnChangedRead();

  if (m_dataman.GetGraph()->GetOperation() == CXGraph::opZoom ||
    m_dataman.GetGraph()->GetOperation() == CXGraph::opPan)
  {
    UpdateSlides();
  }
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticGraphOptionsDlg::EnableControls()
{
  GetDlgItem(IDC_EDT_MIN_AXIS_X)->EnableWindow(!m_allRangeAxisX);
  GetDlgItem(IDC_EDT_MAX_AXIS_X)->EnableWindow(!m_allRangeAxisX);
  GetDlgItem(IDC_EDT_MIN_AXIS_Y)->EnableWindow(!m_allRangeAxisY);
  GetDlgItem(IDC_EDT_MAX_AXIS_Y)->EnableWindow(!m_allRangeAxisY);
  GetDlgItem(IDC_SLD_AXIS_X)->EnableWindow(!m_allRangeAxisX);
  GetDlgItem(IDC_SLD_AXIS_Y)->EnableWindow(!m_allRangeAxisY);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticGraphOptionsDlg::OnChangedRead()
{
  if (!m_dataman.GetGraph())
  {
    return;
  }

  if (m_dataman.GetGraph()->GetXAxisCount() != 0 && m_dataman.GetGraph()->GetYAxisCount() != 0)
  {
    double rangeMin;
    double rangeMax;
    double rangeStp;

    m_dataman.GetGraph()->GetXAxis(0).GetCurrentRange(rangeMin, rangeMax, rangeStp);
    m_rangeMinAxisX = (UINT) floor(rangeMin);
    m_rangeMaxAxisX = (UINT) ceil(rangeMax);

    m_dataman.GetGraph()->GetYAxis(0).GetCurrentRange(rangeMin, rangeMax, rangeStp);
    m_rangeMinAxisY = (UINT) floor(rangeMin);
    m_rangeMaxAxisY = (UINT) ceil(rangeMax);
  }
  else
  {
    m_allRangeAxisX = FALSE;
    m_allRangeAxisY = FALSE;

    m_rangeMinAxisX = 0;
    m_rangeMaxAxisX = 0;
    m_rangeMinAxisY = 0;
    m_rangeMaxAxisY = 0;
  }
  
  EnableControls();
  UpdateData(FALSE);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticGraphOptionsDlg::OnChangedWrite()
{
  if (!::IsWindow(m_hWnd) || !IsWindowVisible())
  {
    return;
  }

  UpdateData(TRUE);
  EnableControls();
  
  if (m_allRangeAxisX)
  {
    m_dataman.GetGraph()->GetXAxis(0).SetAutoScale(true);
    m_dataman.GetGraph()->AutoscaleVisible(0, -1);
  }
  else
  {
    m_dataman.GetGraph()->GetXAxis(0).SetCurrentRange(m_rangeMinAxisX, m_rangeMaxAxisX);
    m_dataman.GetGraph()->GetYAxis(0).SetAutoScale(true);
  }
  
  if (m_allRangeAxisY)
  {
    m_dataman.GetGraph()->GetYAxis(0).SetAutoScale(true);
    m_dataman.GetGraph()->AutoscaleVisible(-1, 0);
  }
  else
  {
    m_dataman.GetGraph()->GetYAxis(0).SetCurrentRange(m_rangeMinAxisY, m_rangeMaxAxisY);
    m_dataman.GetGraph()->GetYAxis(0).SetAutoScale(true);
  }

  UpdateSlides();

  m_dataman.GetGraph()->Invalidate();
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticGraphOptionsDlg::UpdateSlides()
{
  if (!m_allRangeAxisX)
  {
    double rangeMin;
    double rangeMax;

    m_dataman.GetGraph()->GetXAxis(0).GetRange(rangeMin, rangeMax);
    m_slideAxisX.SetRangeMin((short) floor(rangeMin));
    m_slideAxisX.SetRangeMax((short) ceil(rangeMax));
    m_slideAxisX.SetPos(m_rangeMinAxisX);
  }

  if (!m_allRangeAxisY)
  {
    double rangeMin;
    double rangeMax;

    m_dataman.GetGraph()->GetYAxis(0).GetRange(rangeMin, rangeMax);
    m_slideAxisY.SetRangeMin((short) floor(rangeMin));
    m_slideAxisY.SetRangeMax((short) ceil(rangeMax));
    m_slideAxisY.SetPos(m_rangeMinAxisY);
  }
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticGraphOptionsDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
  if (nSBCode == TB_ENDTRACK || nSBCode == TB_THUMBPOSITION)
  {
    return;
  }
  
  CSliderCtrl* scroll = (CSliderCtrl*) pScrollBar;
  
  if (scroll == &m_slideAxisX)
  {
    UINT difference = m_rangeMaxAxisX - m_rangeMinAxisX;
    m_rangeMinAxisX = scroll->GetPos();
    m_rangeMaxAxisX = scroll->GetPos() + difference;
    UpdateData(FALSE);
    OnChangedWrite();
  }
  else
  if (scroll == &m_slideAxisY)
  {
    UINT difference = m_rangeMaxAxisY - m_rangeMinAxisY;
    m_rangeMinAxisY = scroll->GetPos();
    m_rangeMaxAxisY = scroll->GetPos() + difference;
    UpdateData(FALSE);
    OnChangedWrite();
  }
}

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStatisticExportOptionsDlg, CDialog)
  ON_BN_CLICKED(IDC_BTN_SAVE_BITMAP, OnSaveBitmap)
  ON_BN_CLICKED(IDC_BTN_EXPORT_CSV, OnExportCSV)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CStatisticExportOptionsDlg::CStatisticExportOptionsDlg(const DXPlayerStatisticsData& dataman, CWnd* pParent) :
CDialog(IDD_STATISTICVIEWER_ROLLUP_EXPORT_OPTIONS, pParent),
m_dataman(dataman)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL CStatisticExportOptionsDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
  EnableAllChildWindows(this, false);
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticExportOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticExportOptionsDlg::Reset()
{
  EnableAllChildWindows(this, false);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticExportOptionsDlg::Refresh()
{
  GetDlgItem(IDC_BTN_SAVE_BITMAP)->EnableWindow(m_dataman.GetStatisticsCount() > 0);
  GetDlgItem(IDC_BTN_EXPORT_CSV)->EnableWindow(m_dataman.GetStatisticsCount() > 0);
}

////////////////////////////////////////////////////////////////////////////////

bool CStatisticExportOptionsDlg::GetSaveFileNameBMP(CString& fileName)
{
  TCHAR tsFile[MAX_PATH] = "";
  TCHAR tsFilters[]= TEXT
    (
    "Windows Bitmap (BMP)\0*.bmp\0"
    "\0\0"
    );

  if (fileName.GetLength())
  {
    strcpy(tsFile, fileName.GetString());
  }

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = this->m_hWnd;
  ofn.lpstrFilter = tsFilters;
  ofn.lpstrInitialDir = "";
  ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFilterIndex = 4;
  ofn.lpstrFile = tsFile;
  ofn.nMaxFile = sizeof(tsFile) / sizeof(TCHAR);

  if (::GetSaveFileName(&ofn))
  {
    CString fileExtension;

    switch (ofn.nFilterIndex)
    {
    default:
    case 1:
      fileExtension = "bmp";
      break;
    }

    fileName = CString(tsFile);

    ////////////////////////////////////////////////////////////////////////////
    // Repair the filename extension

    if (ofn.nFileExtension == 0)
    {
      fileName += "." + fileExtension;
    }
    else if (tsFile[ofn.nFileExtension] == '\0')
    {
      fileName += fileExtension;
    }
    else
    {
      if (fileName.Mid(ofn.nFileExtension, 3).MakeLower().Compare(fileExtension) != 0)
      {
        fileName += "." + fileExtension;
      }
    }

    ////////////////////////////////////////////////////////////////////////////
  }

  return (fileName.GetLength() > 0);
}

////////////////////////////////////////////////////////////////////////////////

bool CStatisticExportOptionsDlg::GetSaveFileNameCSV(CString& fileName)
{
  TCHAR tsFile[MAX_PATH] = "";
  TCHAR tsFilters[]= TEXT
    (
    "Comma Separated Values (CSV)\0*.csv\0"
    "\0\0"
    );

  if (fileName.GetLength())
  {
    strcpy(tsFile, fileName.GetString());
  }

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = this->m_hWnd;
  ofn.lpstrFilter = tsFilters;
  ofn.lpstrInitialDir = "";
  ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFilterIndex = 4;
  ofn.lpstrFile = tsFile;
  ofn.nMaxFile = sizeof(tsFile) / sizeof(TCHAR);

  if (::GetSaveFileName(&ofn))
  {
    CString fileExtension;

    switch (ofn.nFilterIndex)
    {
    default:
    case 1:
      fileExtension = "csv";
      break;
    }

    fileName = CString(tsFile);

    ////////////////////////////////////////////////////////////////////////////
    // Repair the filename extension

    if (ofn.nFileExtension == 0)
    {
      fileName += "." + fileExtension;
    }
    else if (tsFile[ofn.nFileExtension] == '\0')
    {
      fileName += fileExtension;
    }
    else
    {
      if (fileName.Mid(ofn.nFileExtension, 3).MakeLower().Compare(fileExtension) != 0)
      {
        fileName += "." + fileExtension;
      }
    }

    ////////////////////////////////////////////////////////////////////////////
  }

  return (fileName.GetLength() > 0);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticExportOptionsDlg::OnSaveBitmap()
{
  CString fileName;
  if (GetSaveFileNameBMP(fileName))
  {
    m_dataman.GetGraph()->Invalidate();
    if (!m_dataman.GetGraph()->SaveBitmap(fileName))
    {
      MessageBox("An error ocurred saving the graph.", "Error", MB_OK | MB_ICONWARNING);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticExportOptionsDlg::OnExportCSV()
{
  CString fileName;
  if (GetSaveFileNameCSV(fileName))
  {
    if (!m_dataman.ExportStatisticsToCSV(fileName))
    {
      MessageBox("An error ocurred exporting data.", "Error", MB_OK | MB_ICONWARNING);
    }
  }  
}

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CStatisticOnTheFlyOptionsDlg, CDialog)
  ON_CLBN_CHKCHANGE(IDC_LST_PLUGIN_COUNTERS, OnPluginCheck)
  ON_BN_CLICKED(IDC_BTN_ONTHEFLY_COUNTERS, OnDoCalculations)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

CStatisticOnTheFlyOptionsDlg::CStatisticOnTheFlyOptionsDlg(const DXPlayerStatisticsData& dataman, CWnd* pParent) :
CDialog(IDD_STATISTICVIEWER_ROLLUP_ONTHEFLY_OPTIONS, pParent),
m_dataman(dataman)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL CStatisticOnTheFlyOptionsDlg::OnInitDialog() 
{
  CDialog::OnInitDialog();
  
  m_pluginList.ResetContent();
  m_pluginList.SetCheckStyle(BS_AUTOCHECKBOX);
  
  EnableAllChildWindows(this, false);
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticOnTheFlyOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LST_PLUGIN_COUNTERS, m_pluginList);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticOnTheFlyOptionsDlg::Reset()
{
  m_plugman.Clear();
  m_counters.clear();
  m_pluginList.ResetContent();
  EnableAllChildWindows(this, false);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticOnTheFlyOptionsDlg::Refresh()
{
  EnableAllChildWindows(this, true);
  FillPluginList();
  OnPluginCheck();
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticOnTheFlyOptionsDlg::FillPluginList()
{
  using namespace dxplugin;
  
  m_pluginList.ResetContent();
  m_pluginList.SetCheckStyle(BS_AUTOCHECKBOX);
  
  m_plugman.Clear();
  m_counters.clear();
  if (!m_plugman.LoadAllPluginsFromDirectory())
  {
    return;
  }
  
  for (unsigned int i=0; i < m_plugman.GetPluginCount(); ++i)
  {
    DXIntPluginLoaded* plugin;
    if (!m_plugman.GetPlugin(i, &plugin))
    {
      continue;
    }

    for (unsigned int j=0; j < plugin->GetCounterCount(); ++j)
    {
      DXINTCOUNTERINFO counter;
      if (!plugin->GetCounter(j, &counter))
      {
        continue;
      }

      m_counters.push_back(pair<dxplugin::DXIntPluginLoaded*, DXINTCOUNTERID>(plugin, counter.ID));
      
      m_pluginList.AddString(counter.Name);
      m_pluginList.SetCheck(m_pluginList.GetCount()-1, 0);
      m_pluginList.Enable(m_pluginList.GetCount()-1, TRUE);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticOnTheFlyOptionsDlg::OnPluginCheck()
{
  unsigned int checkedPlugins = 0;
  
  for (int i=0; i < m_pluginList.GetCount(); ++i)
  {
    checkedPlugins += (m_pluginList.GetCheck(i) == 1 ? 1 : 0);
  }
  
  GetDlgItem(IDC_BTN_ONTHEFLY_COUNTERS)->EnableWindow(checkedPlugins > 0);
}

////////////////////////////////////////////////////////////////////////////////

void CStatisticOnTheFlyOptionsDlg::OnDoCalculations()
{
  for (int i=0; i < m_pluginList.GetCount(); ++i)
  {
    if (m_pluginList.GetCheck(i) != 1)
    {
      continue;
    }

    m_plugman.AddCounterToRecordList(m_counters[i].first, m_counters[i].second);
  }
  
  if (!((DXPlayerStatisticViewerDlg*) GetParent()->GetParent())->LoadTraceStatisticsOnTheFly(m_plugman))
  {
    MessageBox("An error ocurred calculating new data.", "Error", MB_OK | MB_ICONWARNING);
    ((DXPlayerStatisticViewerDlg*) GetParent()->GetParent())->EndDialog(IDCANCEL);
  }
}

////////////////////////////////////////////////////////////////////////////////

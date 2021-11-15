// AxisDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "XGraph.h"
#include "AxisDlg.h"
#include "ChartPage.h"
#include "Float.h"
#include "math.h"
#include "xgraph_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CAxisDlg 

#pragma warning (disable : 4244)
#pragma warning (disable : 4800)

CAxisDlg::CAxisDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_AXIS, pParent)
{
	//{{AFX_DATA_INIT(CAxisDlg)
	m_cName = _T("");
	m_bAutoScale = FALSE;
	m_fMin = 0.0;
	m_fMax = 0.0;
	m_bShowGrid = FALSE; 
	m_bTimeAxis = FALSE;
	m_bVisible = TRUE;
	m_nPrecision = 0;
	m_nArcSize = 0;
	m_nTickSize = 0;
	m_nMarkerSize = 0;
	m_fStep = 0.0;
	m_EndDate = 0.0;
	m_EndTime = 0.0;
	m_StartDate = 0.0;
	m_StartTime = 0.0;
	m_cDateTimeFormat = _T("");
	m_bShowMarker = FALSE;
	m_nPlacement = -1;
	m_nGridType = -1;
	m_nTimeStepType = -1;
	m_nTimeStep = 0;
	//}}AFX_DATA_INIT
}


void CAxisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAxisDlg)
	DDX_Control(pDX, IDC_SPINMARKERSIZE, m_SpinMarkerSize);
	DDX_Control(pDX, IDC_SPINARCSIZE, m_SpinArcSize);
	DDX_Control(pDX, IDC_SPINTICKSIZE, m_SpinTickSize);
	DDX_Control(pDX, IDC_GRIDCOLOR, m_GridColor);
	DDX_Control(pDX, IDC_COLOR, m_Color);
	DDX_Control(pDX, IDC_SPINPRECISION, m_SpinPrecision);
	DDX_Text(pDX, IDC_NAME, m_cName);
	DDX_Check(pDX, IDC_CBAUTOSCALE, m_bAutoScale);
	DDX_Text(pDX, IDC_MIN, m_fMin);
	DDV_MinMaxDouble(pDX, m_fMin, -DBL_MAX, DBL_MAX);
	DDX_Text(pDX, IDC_MAX, m_fMax);
	DDV_MinMaxDouble(pDX, m_fMax, -DBL_MAX, DBL_MAX);
	DDX_Check(pDX, IDC_CBGRID, m_bShowGrid);
	DDX_Check(pDX, IDC_CBVISIBLE, m_bVisible);
	DDX_Check(pDX, IDC_CBTIMEAXIS, m_bTimeAxis);
	DDX_Text(pDX, IDC_PRECISION, m_nPrecision);
	DDV_MinMaxUInt(pDX, m_nPrecision, 0, 6);
	DDX_Text(pDX, IDC_ARCSIZE, m_nArcSize);
	DDV_MinMaxUInt(pDX, m_nArcSize, 1, 10);
	DDX_Text(pDX, IDC_TICKSIZE, m_nTickSize);
	DDV_MinMaxUInt(pDX, m_nTickSize, 1, 10);
	DDX_Text(pDX, IDC_MARKERSIZE, m_nMarkerSize);
	DDV_MinMaxUInt(pDX, m_nMarkerSize, 1, 20);
	DDX_Text(pDX, IDC_STEP, m_fStep);
	DDX_DateTimeCtrl(pDX, IDC_ENDDATE, m_EndDate);
	DDX_DateTimeCtrl(pDX, IDC_ENDTIME, m_EndTime);
	DDX_DateTimeCtrl(pDX, IDC_STARTDATE, m_StartDate);
	DDX_DateTimeCtrl(pDX, IDC_STARTTIME, m_StartTime);
	DDX_Text(pDX, IDC_DATETIMEFORMAT, m_cDateTimeFormat);
	DDX_Check(pDX, IDC_CBSHOWMARKERS, m_bShowMarker);
	DDX_CBIndex(pDX, IDC_PLACEMENT, m_nPlacement);
	DDX_CBIndex(pDX, IDC_CBGRIDTYPE, m_nGridType);
	DDX_CBIndex(pDX, IDC_CBTIMESTEPTYPE, m_nTimeStepType);
	DDX_Text(pDX, IDC_TIMESTEP, m_nTimeStep);
	DDV_MinMaxUInt(pDX, m_nTimeStep, 1, 22118400); //365 days
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CAxisDlg, CDialog)
	//{{AFX_MSG_MAP(CAxisDlg)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	ON_MESSAGE(CPN_SELCHANGE, OnColorChanged)
	ON_BN_CLICKED(IDC_BUTTON1, OnSelectFont)
	ON_BN_CLICKED(IDC_BUTTON2, OnTitleFont)
	ON_BN_CLICKED(IDC_CBAUTOSCALE, OnCbautoscale)
	ON_BN_CLICKED(IDC_CBTIMEAXIS, OnCbtimeaxis)
	ON_BN_CLICKED(IDC_CBGRID, OnChanged)
	ON_BN_CLICKED(IDC_CBVISIBLE, OnChanged)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_ENDDATE, OnDateTimeChanged)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_ENDTIME, OnDateTimeChanged)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STARTDATE, OnDateTimeChanged)
	ON_NOTIFY(DTN_DATETIMECHANGE, IDC_STARTTIME, OnDateTimeChanged)
	ON_EN_CHANGE(IDC_ARCSIZE, OnChanged)
	ON_EN_CHANGE(IDC_DATETIMEFORMAT, OnChanged)
	ON_EN_CHANGE(IDC_MARKERSIZE, OnChanged)
	ON_EN_CHANGE(IDC_MAX, OnChanged)
	ON_EN_CHANGE(IDC_MIN, OnChanged)
	ON_EN_CHANGE(IDC_NAME, OnChanged)
	ON_EN_CHANGE(IDC_PRECISION, OnChanged)
	ON_EN_CHANGE(IDC_STEP, OnChanged)
	ON_EN_CHANGE(IDC_TICKSIZE, OnChanged)
	ON_BN_CLICKED(IDC_CBSHOWMARKERS, OnChanged)
	ON_CBN_SELCHANGE(IDC_PLACEMENT, OnChanged)
	ON_CBN_SELCHANGE(IDC_CBGRIDTYPE, OnChanged)
	ON_CBN_SELCHANGE(IDC_CBTIMESTEPTYPE, OnChanged)
	ON_EN_CHANGE(IDC_TIMESTEP, OnChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CAxisDlg 

void CAxisDlg::OnDateTimeChanged(NMHDR *,LRESULT *)
{
	OnChanged();
}

LRESULT CAxisDlg::OnColorChanged(WPARAM, LPARAM)
{
	GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
	return 0;
}

void CAxisDlg::PresetValues()
{
	m_cName = m_pAxis->GetLabel();
	m_pAxis->GetCurrentRange(m_fMin, m_fMax, m_fStep);
	m_bAutoScale = m_pAxis->GetAutoScale();
	m_bTimeAxis = m_pAxis->GetDateTime();
	m_bShowGrid = m_pAxis->GetShowGrid();
	m_bVisible = m_pAxis->GetVisible ();
	m_Color.SetColor (m_pAxis->GetColor());
	m_GridColor.SetColor (m_pAxis->GetGridColor());
	m_bShowMarker = m_pAxis->GetShowMarker();
	m_nPlacement = (int) m_pAxis->GetPlacement();
		
	m_SpinPrecision.SetRange(0, 6);
	m_SpinArcSize.SetRange(1, 10);
	m_SpinTickSize.SetRange(1, 10);
	m_SpinMarkerSize.SetRange(1, 20);
	m_nArcSize  = m_pAxis->GetArcSize();
	m_nTickSize = m_pAxis->GetTickSize();
	m_nMarkerSize = m_pAxis->GetMarkerSize();
	m_SpinMarkerSize.SetPos(m_nMarkerSize);
	m_SpinArcSize.SetPos(m_nArcSize);
	m_SpinTickSize.SetPos(m_nArcSize);
	m_nGridType = m_pAxis->GetGridStyle();

	GetDlgItem(IDC_MIN)->EnableWindow(!m_bAutoScale && !m_bTimeAxis);
	GetDlgItem(IDC_MAX)->EnableWindow(!m_bAutoScale && !m_bTimeAxis);
	GetDlgItem(IDC_STEP)->EnableWindow(!m_bAutoScale && !m_bTimeAxis);
	GetDlgItem(IDC_STARTDATE)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
	GetDlgItem(IDC_STARTTIME)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
	GetDlgItem(IDC_ENDDATE)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
	GetDlgItem(IDC_ENDTIME)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
	GetDlgItem(IDC_TIMESTEP)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
	GetDlgItem(IDC_CBTIMESTEPTYPE)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
	GetDlgItem(IDC_PRECISION)->EnableWindow(!m_bTimeAxis);
	GetDlgItem(IDC_SPINPRECISION)->EnableWindow(!m_bTimeAxis);
	GetDlgItem(IDC_DATETIMEFORMAT)->EnableWindow(m_bTimeAxis);
	GetDlgItem(IDC_PLACEMENT)->EnableWindow(!m_bTimeAxis);

	m_nTimeStep = 1;
	
	if (!m_pAxis->GetDateTime())
	{
     COleDateTime currentTime;
    if (currentTime.GetStatus() == COleDateTime::valid)
    {
      m_StartDate = m_StartTime = m_EndDate = m_EndTime = currentTime;
    }
    else
    {
      m_StartDate = COleDateTime::GetCurrentTime();
      m_StartTime = COleDateTime::GetCurrentTime();
      m_EndDate   = COleDateTime::GetCurrentTime();
      m_EndTime   = COleDateTime::GetCurrentTime();
    }
		
		int nPos = m_pAxis->GetDisplayFmt().Find (".",0);
		if (nPos >= 0)
		{
			CString cPrecision = m_pAxis->GetDisplayFmt().Mid (nPos+1);
			m_nPrecision = atoi(cPrecision);
			m_SpinPrecision.SetPos(m_nPrecision);
		}
	}
	else
	{

		m_cDateTimeFormat = m_pAxis->GetDisplayFmt();
		m_cDateTimeFormat.Replace ("\n", "\\n");
		m_nPrecision = 1;

		COleDateTimeSpan span(m_fStep);

		int nSeconds = span.GetTotalSeconds ();

		int nTimeStep;

		if (nSeconds > (24 * 60 * 60) &&!(nSeconds % (24 * 60 * 60)))
		{
			nTimeStep = nSeconds / (24 * 60 * 60);
			m_nTimeStepType = 3;
		}
		else
		if (nSeconds > (24 * 60) &&!(nSeconds % (24 * 60)))
		{
			nTimeStep = nSeconds / (24 * 60);
			m_nTimeStepType = 2;
		}
		else
		if (nSeconds > 60 &&!(nSeconds %  60))
		{
			nTimeStep = nSeconds / 60;
			m_nTimeStepType = 1;
		}
		else
		{
			nTimeStep = nSeconds;
			m_nTimeStepType = 0;
		}

		m_nTimeStep  = nTimeStep;
		m_StartDate  = m_fMin;
		m_StartTime  = m_fMin;
		m_EndDate    = m_fMax;
		m_EndTime    = m_fMax;
	}

	UpdateData(FALSE);
}

BOOL CAxisDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	PresetValues();
			
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CAxisDlg::OnApply() 
{
	UpdateData(TRUE);

	m_pAxis->SetGridStyle(m_nGridType);
	m_pAxis->SetLabel(m_cName);
	m_pAxis->SetAutoScale(m_bAutoScale);
	m_pAxis->SetVisible(m_bVisible);

	
	if (!m_bAutoScale)
	{
		if (m_bTimeAxis)
		{
			int nType = 1;
			if (m_nTimeStepType == 1)
				nType = 60;
			if (m_nTimeStepType == 2)
				nType = 60 * 60;
			if (m_nTimeStepType == 3)
				nType = 60 * 60 * 24;

			COleDateTime dtStart, dtEnd;
			dtStart.SetDateTime(m_StartDate.GetYear (), m_StartDate.GetMonth (), m_StartDate.GetDay (),
				                m_StartTime.GetHour (), m_StartTime.GetMinute (), m_StartTime.GetSecond ());
			dtEnd.SetDateTime(m_EndDate.GetYear (), m_EndDate.GetMonth (), m_EndDate.GetDay (),
				                m_EndTime.GetHour (), m_EndTime.GetMinute (), m_EndTime.GetSecond ());

			m_fMin = dtStart;
			m_fMax = dtEnd;
			m_pAxis->SetCurrentRange (m_fMin, m_fMax, (double) COleDateTimeSpan(0,0,0,m_nTimeStep * nType));
		}
		else 
			m_pAxis->SetCurrentRange (m_fMin, m_fMax, m_fStep);
	}
	m_pAxis->SetShowGrid(m_bShowGrid);
	m_pAxis->SetPlacement((CXGraphAxis::EAxisPlacement) m_nPlacement);

	if (!m_bTimeAxis)
	{
		CString cTmp;
		
		double fCurMin, fCurMax, fCurStep;

		m_pAxis->GetCurrentRange(fCurMin, fCurMax, fCurStep);

		if (fabs(fCurMin) > 1E6 || fabs(fCurMax) > 1E6 ||
			(fabs(fCurMin) < 1E-6 && fabs(fCurMin) != 0.0) || (fabs(fCurMax) < 1E-6 && fabs(fCurMax) != 0.0))
			cTmp.Format("%%5.%de", m_nPrecision);
		else
			cTmp.Format("%%5.%df", m_nPrecision);

		m_pAxis->SetDisplayFmt(cTmp);
	}
	else
	{
		m_cDateTimeFormat.Replace ("\\n", "\n");
		m_pAxis->SetDisplayFmt(m_cDateTimeFormat);
	}

	m_pAxis->SetColor(m_Color.GetColor ());
	m_pAxis->SetGridColor(m_GridColor.GetColor ());
	m_pAxis->SetArcSize(m_nArcSize);
	m_pAxis->SetTickSize(m_nTickSize);
	m_pAxis->SetMarkerSize(m_nMarkerSize);
	m_pAxis->SetDateTime(m_bTimeAxis);
	m_pAxis->SetShowMarker( m_bShowMarker);
		
	m_pAxis->GetGraph()->Invalidate ();

	GetDlgItem(IDAPPLY)->EnableWindow(FALSE);
	
	
}

void CAxisDlg::OnSelectFont() 
{
	CFontDialog dlg;

	if (dlg.DoModal() == IDOK)
	{
		LOGFONT logFont;
		dlg.GetCurrentFont (&logFont);
		m_pAxis->SetFont (&logFont);
		OnChanged();
	}
	
}

void CAxisDlg::OnTitleFont() 
{
	CFontDialog dlg;

	if (dlg.DoModal() == IDOK)
	{
		LOGFONT logFont;
		dlg.GetCurrentFont (&logFont);
		m_pAxis->SetTitleFont (&logFont);
		OnChanged();
	}
	
}

void CAxisDlg::OnCbautoscale() 
{
	OnChanged();
}

void CAxisDlg::OnCbtimeaxis() 
{
	OnChanged();
}

void CAxisDlg::OnOK() 
{
	OnApply();	
	
	((CChartPage*)GetParentOwner ())->OnOK();
}

void CAxisDlg::OnChanged() 
{
	if (::IsWindow(m_hWnd) && IsWindowVisible())
	{
		UpdateData(TRUE);

		GetDlgItem(IDC_MIN)->EnableWindow(!m_bAutoScale && !m_bTimeAxis);
		GetDlgItem(IDC_MAX)->EnableWindow(!m_bAutoScale && !m_bTimeAxis);
		GetDlgItem(IDC_STEP)->EnableWindow(!m_bAutoScale && !m_bTimeAxis);
		GetDlgItem(IDC_STARTDATE)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
		GetDlgItem(IDC_STARTTIME)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
		GetDlgItem(IDC_ENDDATE)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
		GetDlgItem(IDC_ENDTIME)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
		GetDlgItem(IDC_PRECISION)->EnableWindow(!m_bTimeAxis);
		GetDlgItem(IDC_SPINPRECISION)->EnableWindow(!m_bTimeAxis);
		GetDlgItem(IDC_DATETIMEFORMAT)->EnableWindow(m_bTimeAxis);	
		GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
		GetDlgItem(IDC_TIMESTEP)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
		GetDlgItem(IDC_CBTIMESTEPTYPE)->EnableWindow(m_bTimeAxis && !m_bAutoScale);
		GetDlgItem(IDC_PLACEMENT)->EnableWindow(!m_bTimeAxis);
	
	}
}

void CAxisDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	if (nIDCtl == IDC_CBGRIDTYPE)
	{
		if (lpDrawItemStruct->itemID < 0 || lpDrawItemStruct->itemID >= 5)
			return;

		CDCEx dc;

		dc.Attach (lpDrawItemStruct->hDC);
		
		int nSaveDC = ::SaveDC(lpDrawItemStruct->hDC);
		
		dc.FillSolidRect (&lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_WINDOW));
		
		{
			CPenSelector ps(0L, 1, &dc, lpDrawItemStruct->itemID);
			dc.MoveTo(lpDrawItemStruct->rcItem.left, lpDrawItemStruct->rcItem.top + 6);
			dc.LineTo(lpDrawItemStruct->rcItem.right, lpDrawItemStruct->rcItem.top + 6);
		}

		::RestoreDC (lpDrawItemStruct->hDC, nSaveDC);

		dc.Detach ();

	}
	else
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CAxisDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	if (nIDCtl == IDC_CBGRIDTYPE)
	{
		lpMeasureItemStruct->itemHeight = 16;
		lpMeasureItemStruct->itemWidth  = 16;
	}
	else
		CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);

}

#pragma warning (default : 4244)
#pragma warning (default : 4800)

void CAxisDlg::OnCancel() 
{
	((CChartPage*)GetParentOwner ())->OnCancel ();
}

BOOL CAxisDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	else
	return CDialog::PreTranslateMessage(pMsg);
		
}

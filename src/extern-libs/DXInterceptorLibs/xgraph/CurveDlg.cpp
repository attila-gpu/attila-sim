// CurveDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "XGraph.h"
#include "CurveDlg.h"
#include "MovingAvgDlg.h"
#include "TrendDlg.h"
#include "ChartPage.h"
#include "xgraph_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


#pragma warning (disable : 4244)
#pragma warning (disable : 4800)

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CCurveDlg 


CCurveDlg::CCurveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_CURVE, pParent)
{
	//{{AFX_DATA_INIT(CCurveDlg)
	m_nType = -1;
	m_nLineSize = 0;
	m_nStyle = -1;
	m_bShowMarker = FALSE;
	m_bVisible = FALSE;
	m_bFillBeneath = FALSE;
	m_nFillStyle = -1;
	m_bFillTransparent = FALSE;
	m_cLabel = _T("");
	m_nMarkerType = 0;
	m_nMarker = 0;
	m_nMarkerSize = 1;
	m_nFillCurve = -1;
	//}}AFX_DATA_INIT
}


void CCurveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCurveDlg)
	DDX_Control(pDX, IDC_CBFILLCURVE, m_cbFillCurve);
	DDX_Control(pDX, IDC_SPINLINESIZE, m_SpinLineSize);
	DDX_Control(pDX, IDC_SPINMARKERSIZE, m_SpinMarkerSize);
	DDX_Control(pDX, IDC_CBO_MARKER_SYMBOLS, m_cbMarker);
	DDX_Control(pDX, IDC_COLOR, m_Color);
	DDX_CBIndex(pDX, IDC_CBTYPE, m_nType);
	DDX_Text(pDX, IDC_LINESIZE, m_nLineSize);
	DDV_MinMaxUInt(pDX, m_nLineSize, 1, 5);
	DDX_CBIndex(pDX, IDC_CBLINESTYLE, m_nStyle);
	DDX_Check(pDX, IDC_CBMARKER, m_bShowMarker);
	DDX_Check(pDX, IDC_CBVISIBLE, m_bVisible);
	DDX_Check(pDX, IDC_CBFILL, m_bFillBeneath);
	DDX_Radio(pDX, IDC_BSSTYLE0, m_nFillStyle);
	DDX_Check(pDX, IDC_CBTRANSPARENT, m_bFillTransparent);
	DDX_Text(pDX, IDC_NAME, m_cLabel);
	DDX_Radio(pDX, IDC_MARKERTYPE_NUMBER, m_nMarkerType);
	DDX_CBIndex(pDX, IDC_CBO_MARKER_SYMBOLS, m_nMarker);
	DDX_Text(pDX, IDC_MARKERSIZE, m_nMarkerSize);
	DDV_MinMaxUInt(pDX, m_nMarkerSize, 1, 20);
	DDX_CBIndex(pDX, IDC_CBFILLCURVE, m_nFillCurve);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCurveDlg, CDialog)
	//{{AFX_MSG_MAP(CCurveDlg)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	ON_WM_LBUTTONDBLCLK()
	ON_CBN_SELCHANGE(IDC_CBTYPE, OnChanged)
	ON_MESSAGE(CPN_SELCHANGE, OnColorChanged)
	ON_WM_DRAWITEM()
	ON_WM_MEASUREITEM()
	ON_BN_CLICKED(IDC_MAVG, OnMavg)
	ON_BN_CLICKED(IDC_LINTREND, OnLintrend)
	ON_BN_CLICKED(IDC_CUBTREND, OnCubtrend)
	ON_BN_CLICKED(IDC_POLYTREND, OnPolytrend)
	ON_CBN_SELCHANGE(IDC_CBLINESTYLE, OnChanged)
	ON_EN_CHANGE(IDC_LINESIZE, OnChanged)
	ON_BN_CLICKED(IDC_COLOR, OnChanged)
	ON_BN_CLICKED(IDC_CBVISIBLE, OnChanged)
	ON_BN_CLICKED(IDC_CBMARKER, OnChanged)
	ON_BN_CLICKED(IDC_CBFILL, OnChanged)
	ON_BN_CLICKED(IDC_BSSTYLE0, OnChanged)
	ON_BN_CLICKED(IDC_BSSTYLE1, OnChanged)
	ON_BN_CLICKED(IDC_BSSTYLE2, OnChanged)
	ON_BN_CLICKED(IDC_BSSTYLE3, OnChanged)
	ON_BN_CLICKED(IDC_BSSTYLE4, OnChanged)
	ON_BN_CLICKED(IDC_BSSTYLE5, OnChanged)
	ON_BN_CLICKED(IDC_BSSTYLE6, OnChanged)
	ON_BN_CLICKED(IDC_CBTRANSPARENT, OnChanged)
	ON_EN_CHANGE(IDC_NAME, OnChanged)
	ON_CBN_SELCHANGE(IDC_CBO_MARKER_SYMBOLS, OnChanged)
	ON_BN_CLICKED(IDC_MARKERTYPE_NUMBER, OnChanged)
	ON_BN_CLICKED(IDC_MARKERTYPE_SYMBOL, OnChanged)
	ON_EN_CHANGE(IDC_MARKERSIZE, OnChanged)
	ON_CBN_CLOSEUP(IDC_CBO_MARKER_SYMBOLS, OnChanged)
	ON_CBN_SELCHANGE(IDC_CBFILLCURVE, OnChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CCurveDlg 

LRESULT CCurveDlg::OnColorChanged(WPARAM, LPARAM)
{
	GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
	return 0;
}

void CCurveDlg::PresetValues()
{
	if (m_pSerie)
	{
		m_cbFillCurve.Clear();

		while (m_cbFillCurve.GetCount())
			m_cbFillCurve.DeleteString(0);

		m_cbMarker.Clear ();

		m_cbMarker.AddString ("");
		m_cbMarker.AddString ("");
		m_cbMarker.AddString ("");
		m_cbMarker.AddString ("");
		m_cbMarker.AddString ("");
		m_cbMarker.AddString ("");
		m_cbMarker.AddString ("");
		m_cbMarker.AddString ("");

		for (int i = 0; i < m_pSerie->GetGraph ()->GetCurveCount (); i++)
		{
			if (&(m_pSerie->GetGraph ()->GetCurve(i)) != m_pSerie)
			{
				CString cItem;
				cItem.Format("%d %s", i + 1, m_pSerie->GetGraph ()->GetCurve(i).GetLabel());
				m_cbFillCurve.AddString (cItem);
			}
		}
		
		m_SpinMarkerSize.SetRange (1,20);
		m_SpinLineSize.SetRange (1,5);

		m_nType     = (int) m_pSerie->GetType();
		m_nLineSize = m_pSerie->GetLineSize();
		m_Color.SetColor (m_pSerie->GetColor());
		m_nStyle = m_pSerie->GetLineStyle();
		m_bVisible = m_pSerie->GetVisible();
		m_bShowMarker = m_pSerie->GetShowMarker();
		m_bFillBeneath = m_pSerie->GetFillBeneath();
		m_nFillStyle = m_pSerie->GetFillStyle();
		m_bFillTransparent = m_pSerie->GetFillTransparent();
		m_cLabel = m_pSerie->GetLabel();
		m_nMarkerType = m_pSerie->GetMarkerType();
		m_nMarkerSize = m_pSerie->GetMarkerSize();
		m_nMarker = m_pSerie->GetMarker();
		m_nFillCurve = m_pSerie->GetFillCurve();

		GetDlgItem(IDC_CBFILLCURVE)->EnableWindow(m_bFillBeneath);
    GetDlgItem(IDC_BSSTYLE0)->EnableWindow(m_bFillBeneath);
		GetDlgItem(IDC_BSSTYLE1)->EnableWindow(m_bFillBeneath);
		GetDlgItem(IDC_BSSTYLE2)->EnableWindow(m_bFillBeneath);
		GetDlgItem(IDC_BSSTYLE3)->EnableWindow(m_bFillBeneath);
		GetDlgItem(IDC_BSSTYLE4)->EnableWindow(m_bFillBeneath);
		GetDlgItem(IDC_BSSTYLE5)->EnableWindow(m_bFillBeneath);
		GetDlgItem(IDC_BSSTYLE6)->EnableWindow(m_bFillBeneath);
		GetDlgItem(IDC_CBO_MARKER_SYMBOLS)->EnableWindow(m_nMarkerType == 1);

		UpdateData(FALSE);
	}

}

BOOL CCurveDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();

	PresetValues();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CCurveDlg::OnApply() 
{
	UpdateData(TRUE);

	m_pSerie->SetType((CXGraphDataSerie::EGraphType) m_nType);
	m_pSerie->SetLineSize(m_nLineSize);
	m_pSerie->SetColor(m_Color.GetColor());
	m_pSerie->SetLineStyle(m_nStyle);
	m_pSerie->SetVisible(m_bVisible);
	m_pSerie->SetShowMarker(m_bShowMarker);
	m_pSerie->SetFillBeneath(m_bFillBeneath);
	m_pSerie->SetFillStyle(m_nFillStyle);
	m_pSerie->SetFillTransparent(m_bFillTransparent);
	m_pSerie->SetLabel(m_cLabel);
	m_pSerie->SetMarkerType(m_nMarkerType);
	m_pSerie->SetMarkerSize(m_nMarkerSize);
	m_pSerie->SetMarker(m_nMarker) ;
	m_pSerie->SetFillCurve(m_nFillCurve);

	m_pSerie->GetGraph ()->Invalidate();
	
	GetDlgItem(IDAPPLY)->EnableWindow(FALSE);
	
}

void CCurveDlg::OnLButtonDblClk(UINT nFlags, CPoint point) 
{
	// TODO: Code für die Behandlungsroutine für Nachrichten hier einfügen und/oder Standard aufrufen
	
	CDialog::OnLButtonDblClk(nFlags, point);
}

void CCurveDlg::OnChanged() 
{
	if (!::IsWindow(m_hWnd) || !IsWindowVisible())
		return;

	UpdateData(TRUE);

	GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
	GetDlgItem(IDC_CBTRANSPARENT)->EnableWindow(m_bFillBeneath);
  GetDlgItem(IDC_BSSTYLE0)->EnableWindow(m_bFillBeneath);
	GetDlgItem(IDC_BSSTYLE1)->EnableWindow(m_bFillBeneath);
	GetDlgItem(IDC_BSSTYLE2)->EnableWindow(m_bFillBeneath);
	GetDlgItem(IDC_BSSTYLE3)->EnableWindow(m_bFillBeneath);
	GetDlgItem(IDC_BSSTYLE4)->EnableWindow(m_bFillBeneath);
	GetDlgItem(IDC_BSSTYLE5)->EnableWindow(m_bFillBeneath);
	GetDlgItem(IDC_BSSTYLE6)->EnableWindow(m_bFillBeneath);
	GetDlgItem(IDC_MARKERTYPE_SYMBOL)->EnableWindow(m_bShowMarker);
	GetDlgItem(IDC_MARKERTYPE_NUMBER)->EnableWindow(m_bShowMarker);
	GetDlgItem(IDC_CBO_MARKER_SYMBOLS)->EnableWindow(m_nMarkerType == 1 && m_bShowMarker);
	GetDlgItem(IDC_CBFILLCURVE)->EnableWindow(m_bFillBeneath);

}

void CCurveDlg::OnOK() 
{
	OnApply();
	
	((CChartPage*)GetParentOwner ())->OnOK();
}

void CCurveDlg::OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct) 
{
	CDCEx dc;

	if (nIDCtl == IDC_CBLINESTYLE)
	{
		if (lpDrawItemStruct->itemID < 0 || lpDrawItemStruct->itemID >= 5)
			return;

		dc.Attach (lpDrawItemStruct->hDC);
		
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
	else
	if (nIDCtl == IDC_CBO_MARKER_SYMBOLS)
	{
		if (lpDrawItemStruct->itemID < 0 || lpDrawItemStruct->itemID >= MAX_MARKERS)
			return;

		
		dc.Attach (lpDrawItemStruct->hDC);
		
		int nSaveDC = ::SaveDC(lpDrawItemStruct->hDC);
		
		dc.FillSolidRect(&lpDrawItemStruct->rcItem, lpDrawItemStruct->itemState & ODS_SELECTED ? ::GetSysColor(COLOR_HIGHLIGHT) : ::GetSysColor(COLOR_WINDOW));

		CPoint point;

		point.x = lpDrawItemStruct->rcItem.left + 2;
		point.y = lpDrawItemStruct->rcItem.top + 2;
		
		m_pSerie->GetGraph()->m_pDrawFn[lpDrawItemStruct->itemID](point, 10, lpDrawItemStruct->itemState & ODS_DISABLED ? ::GetSysColor (COLOR_INACTIVEBORDER) : 0L, false, &dc );

		::RestoreDC (lpDrawItemStruct->hDC, nSaveDC);

		dc.Detach ();

	}
	else
		CDialog::OnDrawItem(nIDCtl, lpDrawItemStruct);
}

void CCurveDlg::OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct) 
{
	if (nIDCtl == IDC_CBLINESTYLE)
	{
		lpMeasureItemStruct->itemHeight = 16;
		lpMeasureItemStruct->itemWidth  = 100;
	}
	else
	if (nIDCtl == IDC_CBO_MARKER_SYMBOLS)
	{
		lpMeasureItemStruct->itemHeight = 16;
		lpMeasureItemStruct->itemWidth  = 16;
	}
	else
		CDialog::OnMeasureItem(nIDCtl, lpMeasureItemStruct);
}


void CCurveDlg::OnMavg() 
{
	CMovingAvgDlg dlg;

	dlg.m_nMaxSize = m_pSerie->GetCount() / 2;
	CString cType;

	if (dlg.DoModal () == IDOK)
	{
		TDataPoint* pTrend = NULL;
		
		switch (dlg.m_nType)
		{
			case 0 : cType = "simple"; pTrend = m_pSerie->GetSimpleMovingAverage (dlg.m_nSize); break;
			case 1 : cType = "linear"; pTrend = m_pSerie->GetLinearMovingAverage (dlg.m_nSize); break;
			case 2 : cType = "exponential"; pTrend = m_pSerie->GetExponentialMovingAverage (dlg.m_nSize); break;
			case 3 : cType = "triangular"; pTrend = m_pSerie->GetTriangularMovingAverage (dlg.m_nSize); break;
			case 4 : cType = "sine weighted"; pTrend = m_pSerie->GetSineWeightedMovingAverage (dlg.m_nSize); break;
		}

		if (!pTrend)
			return;

		int nOldIndex   = m_pSerie->GetIndex();
		CXGraph* pGraph = m_pSerie->GetGraph();
		int	nCurveCount = m_pSerie->GetGraph()->GetCurveCount ();
		CXGraphDataSerie &ret = m_pSerie->GetGraph()->SetData (pTrend, m_pSerie->GetCount(), nCurveCount, m_pSerie->GetXAxis(), m_pSerie->GetYAxis(), true);
		m_pSerie = &pGraph->GetCurve (nOldIndex);
		ret.SetLabel(m_pSerie->GetLabel () + " AVG, " + cType);
		delete pTrend;
	}
}


void CCurveDlg::OnPolytrend()
{
	CTrendDlg dlg;

	if (dlg.DoModal () != IDOK)
		return;

	int nOldIndex   = m_pSerie->GetIndex();
	CXGraph* pGraph = m_pSerie->GetGraph();
	
	TDataPoint* pTrend = NULL;

	pTrend = m_pSerie->GetPolynomialTrend (dlg.m_nDegree, dlg.m_nSize);

	if (NULL == pTrend)
	{
		AfxMessageBox("Could not create polynomial trend for the selected curve.");
		return;
	}

	CString cPoly = "f(x)=";

	for (int i = 0; i < m_pSerie->m_PS.m_nGlobalO; i++)
	{
		if (m_pSerie->m_PS.m_fC[i] == 0.0)
			continue;

		CString cCoeff;
		cCoeff.Format("%5.4e*X^%d", m_pSerie->m_PS.m_fC[i], i);
		if (i < (m_pSerie->m_PS.m_nGlobalO-1))
			cCoeff += "+\n";
		cPoly += cCoeff;
	}

	int	nCurveCount = m_pSerie->GetGraph()->GetCurveCount ();
	CXGraphDataSerie &ret = m_pSerie->GetGraph()->SetData (pTrend, m_pSerie->GetCount() + dlg.m_nSize, nCurveCount, m_pSerie->GetXAxis(), m_pSerie->GetYAxis(), true);
	delete pTrend;
	
	m_pSerie = &pGraph->GetCurve (nOldIndex);

	CDC *pDC = pGraph->GetDC();
	CRect rect(200,200,210,210);
	pDC->DrawText(cPoly, rect, DT_LEFT | DT_CALCRECT);
	ret.SetLabel(m_pSerie->GetLabel() + " Trend (poly.) ");
	pGraph->ReleaseDC (pDC);

	pGraph->InsertLabel (rect, cPoly);

}

void CCurveDlg::OnLintrend() 
{
	CTrendDlg dlg;

	if (dlg.DoModal () != IDOK)
		return;

	int nOldIndex   = m_pSerie->GetIndex();
	CXGraph* pGraph = m_pSerie->GetGraph();
	
	TDataPoint* pTrend = NULL;
	pTrend = m_pSerie->GetLinearTrend (dlg.m_nSize);
	int	nCurveCount = m_pSerie->GetGraph()->GetCurveCount ();
	CXGraphDataSerie &ret = m_pSerie->GetGraph()->SetData (pTrend, m_pSerie->GetCount() + dlg.m_nSize, nCurveCount, m_pSerie->GetXAxis(), m_pSerie->GetYAxis(), true);
	delete pTrend;
	m_pSerie = &pGraph->GetCurve (nOldIndex);

	ret.SetLabel(m_pSerie->GetLabel() + " Trend (lin.)");
}

void CCurveDlg::OnCubtrend() 
{
	CTrendDlg dlg;

	if (dlg.DoModal () != IDOK)
		return;

	int nOldIndex   = m_pSerie->GetIndex();
	CXGraph* pGraph = m_pSerie->GetGraph();

	TDataPoint* pTrend = NULL;
	pTrend = m_pSerie->GetCubicTrend (dlg.m_nSize);
	int	nCurveCount = m_pSerie->GetGraph()->GetCurveCount ();
	CXGraphDataSerie &ret = m_pSerie->GetGraph()->SetData (pTrend, m_pSerie->GetCount() + dlg.m_nSize, nCurveCount, m_pSerie->GetXAxis(), m_pSerie->GetYAxis(), true);
	delete pTrend;

	m_pSerie = &pGraph->GetCurve (nOldIndex);

	ret.SetLabel(m_pSerie->GetLabel() + " Trend (cub.)");
	
	
}

#pragma warning (default : 4244)
#pragma warning (default : 4800)

void CCurveDlg::OnCancel() 
{
	((CChartPage*)this->GetParentOwner ())->OnCancel ();
}

BOOL CCurveDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	else
	return CDialog::PreTranslateMessage(pMsg);

}

void CCurveDlg::OnSelchangeCbfillcurve() 
{
	// TODO: Code für die Behandlungsroutine der Steuerelement-Benachrichtigung hier einfügen
	
}

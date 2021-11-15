// ChartDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "XGraph.h"
#include "ChartDlg.h"
#include "ChartPage.h"
#include "xgraph_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CChartDlg 

#pragma warning (disable : 4244)
#pragma warning (disable : 4800)


CChartDlg::CChartDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_CHART, pParent)
{
	m_pGraph = NULL;

	//{{AFX_DATA_INIT(CChartDlg)
	m_bShowLegend = FALSE;
	m_bDoubleBuffer = FALSE;
	m_nXAxis = -1;
	m_nYAxis = -1;
	m_nCurve = -1;
	m_bSnapCursor = FALSE;
	m_nTolerance = 0;
	//}}AFX_DATA_INIT
}


void CChartDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChartDlg)
	DDX_Control(pDX, IDC_SPINTOL, m_SpinTol);
	DDX_Control(pDX, IDC_CBCURVE, m_cbCurve);
	DDX_Control(pDX, IDC_CBYAXIS, m_cbYAxis);
	DDX_Control(pDX, IDC_CBXAXIS, m_cbXAxis);
	DDX_Control(pDX, IDC_MARGINS, m_Alignment);
	DDX_Control(pDX, IDC_INNERCOLOR, m_InnerColor);
	DDX_Control(pDX, IDC_CHARTCOLOR, m_CharColor);
	DDX_Control(pDX, IDC_BKCOLOR, m_BkColor);
	DDX_Check(pDX, IDC_CBSHOWLEGEND, m_bShowLegend);
	DDX_Check(pDX, IDC_CBDOUBLEBUFFER, m_bDoubleBuffer);
	DDX_CBIndex(pDX, IDC_CBXAXIS, m_nXAxis);
	DDX_CBIndex(pDX, IDC_CBYAXIS, m_nYAxis);
	DDX_CBIndex(pDX, IDC_CBCURVE, m_nCurve);
	DDX_Check(pDX, IDC_CBSNAP, m_bSnapCursor);
	DDX_Text(pDX, IDC_TOLERANCE, m_nTolerance);
	DDV_MinMaxUInt(pDX, m_nTolerance, 0, 100);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChartDlg, CDialog)
	//{{AFX_MSG_MAP(CChartDlg)
	ON_BN_CLICKED(IDC_CBSHOWLEGEND, OnChanged)
	ON_MESSAGE(CPN_SELCHANGE, OnColorChanged)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	ON_CBN_SELCHANGE(IDC_CBXAXIS, OnSelchangeCbxaxis)
	ON_CBN_SELCHANGE(IDC_CBYAXIS, OnSelchangeCbyaxis)
	ON_CBN_SELCHANGE(IDC_CBCURVE, OnSelchangeCbcurve)
	ON_CBN_SELCHANGE(IDC_MARGINS, OnChanged)
	ON_BN_CLICKED(IDC_CBDOUBLEBUFFER, OnChanged)
	ON_BN_CLICKED(IDC_CBSNAP, OnChanged)
	ON_EN_CHANGE(IDC_TOLERANCE, OnChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CChartDlg 

LRESULT CChartDlg::OnColorChanged(WPARAM, LPARAM)
{
	GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
	return 0;
}

void CChartDlg::OnChanged() 
{
	if (!::IsWindow(m_hWnd) || !IsWindowVisible())
		return;

	if (!UpdateData(TRUE))
		return;

	GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
	GetDlgItem(IDC_MARGINS)->EnableWindow(m_bShowLegend);
}

BOOL CChartDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_BkColor.SetColor (m_pGraph->GetBackColor());
	m_CharColor.SetColor (m_pGraph->GetGraphColor());
	m_InnerColor.SetColor (m_pGraph->GetInnerColor());
	m_bShowLegend = m_pGraph->GetShowLegend();
	m_bDoubleBuffer = m_pGraph->GetDoubleBuffer();
	m_Alignment.SetCurSel ((int)m_pGraph->GetLegendAlignment());
	m_bSnapCursor = m_pGraph->GetSnapCursor();
	m_nTolerance = m_pGraph->GetSnapRange();

	m_SpinTol.SetRange (0, 100);

	int i;
	CString cItem;

	for (i = 0; i < m_pGraph->GetCurveCount (); i++)
	{
		if (&m_pGraph->GetCurve(i) == ((CChartPage*)GetParentOwner ())->m_pGraphDataSerie)
		{
			m_nCurve = i;
			UpdateData(FALSE);
		}

		cItem.Format("%d %s", i + 1, m_pGraph->GetCurve(i).GetLabel());
		m_cbCurve.AddString (cItem);
	}

	for (i = 0; i < m_pGraph->GetXAxisCount (); i++)
	{
		if (&m_pGraph->GetXAxis(i) == ((CChartPage*)GetParentOwner ())->m_pGraphAxis)
		{
			m_nXAxis = i;
			UpdateData(FALSE);
		}

		cItem.Format("%d %s", i + 1, m_pGraph->GetXAxis(i).GetLabel());
		m_cbXAxis.AddString (cItem);
	}


	for (i = 0; i < m_pGraph->GetYAxisCount (); i++)
	{
		if (&m_pGraph->GetYAxis(i) == ((CChartPage*)GetParentOwner ())->m_pGraphAxis)
		{
			m_nYAxis = i;
			UpdateData(FALSE);
		}

		cItem.Format("%d %s", i + 1, m_pGraph->GetYAxis(i).GetLabel());
		m_cbYAxis.AddString (cItem);
	}

	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CChartDlg::OnOK() 
{
	OnApply();
	
	((CChartPage*)GetParentOwner ())->OnOK();
}

void CChartDlg::OnApply() 
{
	UpdateData(TRUE);

	m_pGraph->SetBackColor(m_BkColor.GetColor ());
	m_pGraph->SetGraphColor(m_CharColor.GetColor ());
	m_pGraph->SetInnerColor(m_InnerColor.GetColor ());
	m_pGraph->SetShowLegend(m_bShowLegend);
	m_pGraph->SetDoubleBuffer(m_bDoubleBuffer);
	m_pGraph->SetLegendAlignment((CXGraph::EAlignment) m_Alignment.GetCurSel ());
	m_pGraph->SetSnapCursor(m_bSnapCursor);
	m_pGraph->SetSnapRange(m_nTolerance);

	m_pGraph->Invalidate ();

	GetDlgItem(IDAPPLY)->EnableWindow(FALSE);

}

#pragma warning (default : 4244)
#pragma warning (default : 4800)

void CChartDlg::OnCancel() 
{
	((CChartPage*)GetParentOwner ())->OnCancel ();
}

BOOL CChartDlg::PreTranslateMessage(MSG* pMsg) 
{
	if ((pMsg->message == WM_KEYDOWN || pMsg->message == WM_KEYUP) && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	else
	return CDialog::PreTranslateMessage(pMsg);
}

void CChartDlg::OnSelchangeCbxaxis() 
{
	UpdateData(TRUE);
	((CChartPage*)GetParentOwner ())->SelectXAxis(m_nXAxis);
	m_nYAxis = -1;
	UpdateData(FALSE);
	m_pGraph->SelectXAxis(m_nXAxis);
}

void CChartDlg::OnSelchangeCbyaxis() 
{
	UpdateData(TRUE);
	((CChartPage*)GetParentOwner ())->SelectYAxis(m_nYAxis);
	m_nXAxis = -1;
	UpdateData(FALSE);
	m_pGraph->SelectYAxis(m_nYAxis);
}

void CChartDlg::OnSelchangeCbcurve() 
{
	UpdateData(TRUE);
	((CChartPage*)GetParentOwner ())->SelectCurve(m_nCurve);
	m_pGraph->SelectCurve(m_nCurve);
}

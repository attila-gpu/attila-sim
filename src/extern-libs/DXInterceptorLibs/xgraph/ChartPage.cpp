// ChartPage.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "xgraph.h"
#include "ChartPage.h"
#include "ChartDlg.h"
#include "xgraph_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CChartPage 


CChartPage::CChartPage(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_CHARTPROPERTIES, pParent)
{
	//{{AFX_DATA_INIT(CChartPage)
		
	//}}AFX_DATA_INIT

	m_pGraph = NULL;
	m_pGraphAxis = NULL;
	m_pGraphDataSerie = NULL;

	m_pChart = new CChartDlg;
	m_pCurve = new CCurveDlg;
	m_pAxis  = new CAxisDlg;

}

CChartPage::~CChartPage()
{
	delete m_pChart;
	delete m_pCurve;
	delete m_pAxis;
}


void CChartPage::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CChartPage)
	DDX_Control(pDX, IDC_TAB, m_Tab);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CChartPage, CDialog)
	//{{AFX_MSG_MAP(CChartPage)
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB, OnSelchangeTab)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CChartPage 

BOOL CChartPage::OnInitDialog() 
{
	CDialog::OnInitDialog();

	TCITEM tabItem;

	
	int nPageSelect = 0;

	if (m_pGraphAxis == NULL)
		m_pGraphAxis = &m_pGraph->GetYAxis (0);
	else
		nPageSelect = 1;

	if (m_pGraphDataSerie == NULL)
		m_pGraphDataSerie = &m_pGraph->GetCurve (0);
	else
		nPageSelect = 2;

	m_pChart->m_pGraph = m_pGraph;
	m_pCurve->m_pSerie = m_pGraphDataSerie; 
	m_pAxis->m_pAxis  = m_pGraphAxis; 
	
	tabItem.mask = TCIF_PARAM | TCIF_TEXT;
    tabItem.lParam = (LPARAM)m_pChart;
    VERIFY(m_pChart->Create(IDD_CHART, &m_Tab));
    tabItem.pszText = "Chart";
    m_Tab.InsertItem(0,&tabItem);
	
	tabItem.lParam = (LPARAM)m_pAxis;
    VERIFY(m_pAxis->Create(IDD_AXIS, &m_Tab));
    tabItem.pszText = "Axis";
    m_Tab.InsertItem(1,&tabItem);
	
	tabItem.lParam = (LPARAM)m_pCurve;
    VERIFY(m_pCurve->Create(IDD_CURVE, &m_Tab));
    tabItem.pszText = "Curve";
    m_Tab.InsertItem(2,&tabItem);
		
	if (nPageSelect == 0)
	{
		m_pChart->SetWindowPos(NULL, 8, 25, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		m_pChart->ShowWindow(SW_SHOW);
		m_pChart->EnableWindow(TRUE);
		m_pChart->ModifyStyle(WS_CAPTION,0);
	}

	
	if (nPageSelect == 1)
	{
		m_pAxis->SetWindowPos(NULL, 8, 25, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		m_pAxis->ShowWindow(SW_SHOW);
		m_pAxis->EnableWindow(TRUE);
		m_pAxis->ModifyStyle(WS_CAPTION,0);
	}

	if (nPageSelect == 2)
	{
		m_pCurve->SetWindowPos(NULL, 8, 25, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		m_pCurve->ShowWindow(SW_SHOW);
		m_pCurve->EnableWindow(TRUE);
		m_pCurve->ModifyStyle(WS_CAPTION,0);
	}

	m_Tab.SetCurSel(nPageSelect);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

void CChartPage::OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult) 
{
	m_pChart->ShowWindow(SW_HIDE);
	m_pChart->EnableWindow(FALSE);
	m_pAxis->ShowWindow(SW_HIDE);
	m_pAxis->EnableWindow(FALSE);
	m_pCurve->ShowWindow(SW_HIDE);
	m_pCurve->EnableWindow(FALSE);
	
    switch (m_Tab.GetCurSel()) 
	{
	case 0:
		{
			m_pChart->SetWindowPos(&wndTop, 8, 25, 0, 0, SWP_NOSIZE );
		    m_pChart->ShowWindow(SW_SHOW);
			m_pChart->EnableWindow(TRUE);
			m_pChart->ModifyStyle(WS_CAPTION,0);
			break;
		}
	case 1:
		{
			m_pAxis->SetWindowPos(&wndTop, 8, 25, 0, 0, SWP_NOSIZE );
		    m_pAxis->ShowWindow(SW_SHOW);
			m_pAxis->EnableWindow(TRUE);
			m_pAxis->ModifyStyle(WS_CAPTION,0);
			break;
		}

	case 2:
		{
			m_pCurve->SetWindowPos(&wndTop, 8, 25, 0, 0, SWP_NOSIZE );
		    m_pCurve->ShowWindow(SW_SHOW);
			m_pCurve->EnableWindow(TRUE);
			m_pCurve->ModifyStyle(WS_CAPTION,0);
			break;
		}

	}
	
	*pResult = 0;
}

void CChartPage::SelectXAxis(int nItem)
{
	m_pAxis->m_pAxis = &m_pGraph->GetXAxis(nItem);
	m_pAxis->PresetValues ();
}

void CChartPage::SelectYAxis(int nItem)
{
	m_pAxis->m_pAxis = &m_pGraph->GetYAxis(nItem);
	m_pAxis->PresetValues ();
}

void CChartPage::SelectCurve(int nItem)
{
	m_pCurve->m_pSerie = &m_pGraph->GetCurve(nItem);
	m_pCurve->PresetValues ();
}


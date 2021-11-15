// MovingAvgDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "XGraph.h"
#include "MovingAvgDlg.h"
#include "xgraph_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CMovingAvgDlg 


CMovingAvgDlg::CMovingAvgDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_AVG, pParent)
{
	m_nMaxSize = 100;
	//{{AFX_DATA_INIT(CMovingAvgDlg)
	m_nType = 0;
	m_nSize = 10;
	//}}AFX_DATA_INIT
}


void CMovingAvgDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CMovingAvgDlg)
	DDX_Control(pDX, IDC_SPINWSIZE, m_SpinWSize);
	DDX_CBIndex(pDX, IDC_CBTYPE, m_nType);
	DDX_Text(pDX, IDC_WSIZE, m_nSize);
	DDV_MinMaxUInt(pDX, m_nSize, 1, m_nMaxSize);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CMovingAvgDlg, CDialog)
	//{{AFX_MSG_MAP(CMovingAvgDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CMovingAvgDlg 

BOOL CMovingAvgDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_SpinWSize.SetRange (1,m_nMaxSize);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

// TrendDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "XGraph.h"
#include "TrendDlg.h"
#include "xgraph_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTrendDlg 


CTrendDlg::CTrendDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_TRENDDLG, pParent)
{
	//{{AFX_DATA_INIT(CTrendDlg)
	m_nSize = 0;
	m_nDegree = 5;
	//}}AFX_DATA_INIT
}


void CTrendDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTrendDlg)
	DDX_Control(pDX, IDC_SPIN2, m_Degree);
	DDX_Control(pDX, IDC_SPIN1, m_Spin);
	DDX_Text(pDX, IDC_EDIT1, m_nSize);
	DDV_MinMaxUInt(pDX, m_nSize, 0, 100000000);
	DDX_Text(pDX, IDC_EDIT2, m_nDegree);
	DDV_MinMaxUInt(pDX, m_nDegree, 2, 20);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CTrendDlg, CDialog)
	//{{AFX_MSG_MAP(CTrendDlg)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CTrendDlg 

BOOL CTrendDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_Spin.SetRange32(0,100000000);
	m_Degree.SetRange(2,20);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX-Eigenschaftenseiten sollten FALSE zurückgeben
}

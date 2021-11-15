// LabelDlg.cpp: Implementierungsdatei
//

#include "stdafx.h"
#include "XGraph.h"
#include "LabelDlg.h"
#include "xgraph_resources.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CLabelDlg 

#pragma warning (disable : 4244)
#pragma warning (disable : 4800)


CLabelDlg::CLabelDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_LABELDLG, pParent)
{
	//{{AFX_DATA_INIT(CLabelDlg)
	m_bBorder = FALSE;
	m_nAlignment = -1;
	m_nCurve = -1;
	m_bTransparent = FALSE;
	//}}AFX_DATA_INIT
}


void CLabelDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CLabelDlg)
	DDX_Control(pDX, IDC_CURVE, m_Curve);
	DDX_Control(pDX, IDC_COLOR, m_Color);
	DDX_Check(pDX, IDC_BORDER, m_bBorder);
	DDX_CBIndex(pDX, IDC_CBALIGNMENT, m_nAlignment);
	DDX_CBIndex(pDX, IDC_CURVE, m_nCurve);
	DDX_Check(pDX, IDC_TRANSPARENT, m_bTransparent);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CLabelDlg, CDialog)
	//{{AFX_MSG_MAP(CLabelDlg)
	ON_BN_CLICKED(IDC_BORDER, OnChanged)
	ON_BN_CLICKED(IDAPPLY, OnApply)
	ON_BN_CLICKED(IDC_FONT, OnFont)
	ON_CBN_SELCHANGE(IDC_CBALIGNMENT, OnChanged)
	ON_CBN_SELCHANGE(IDC_CURVE, OnChanged)
	ON_BN_CLICKED(IDC_TRANSPARENT, OnChanged)
	ON_MESSAGE(CPN_SELCHANGE, OnColorChanged)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// Behandlungsroutinen für Nachrichten CLabelDlg 


LRESULT CLabelDlg::OnColorChanged(WPARAM, LPARAM)
{
	OnChanged();
	return 0;
}

void CLabelDlg::OnChanged() 
{
	GetDlgItem(IDAPPLY)->EnableWindow(TRUE);
}

BOOL CLabelDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	m_bBorder = m_pLabel->GetBorder();
	m_Color.SetColor(m_pLabel->GetColor());
	m_nAlignment = m_pLabel->GetAlignment();
	m_bTransparent = m_pLabel->GetTransparent();

	CString cCurve;

	for (int i = 0; i < m_pLabel->GetGraph()->GetCurveCount(); i++)
	{
		CXGraphDataSerie& serie = m_pLabel->GetGraph()->GetCurve (i);

		cCurve.Format("%02d %s", i+1, serie.GetLabel());
		m_Curve.AddString(cCurve);
	}
	m_nCurve = m_pLabel->GetCurve();
	
	UpdateData(FALSE);

	return TRUE;  
	              
}

void CLabelDlg::OnOK() 
{
	OnApply();
	
	CDialog::OnOK();
}

void CLabelDlg::OnApply() 
{
	UpdateData(TRUE);

	m_pLabel->SetBorder(m_bBorder);
	m_pLabel->SetColor(m_Color.GetColor());
	m_pLabel->SetAlignment(m_nAlignment);
	m_pLabel->SetCurve(m_nCurve);
	m_pLabel->SetTransparent(m_bTransparent);
	
	m_pLabel->GetGraph()->Invalidate ();

	GetDlgItem(IDAPPLY)->EnableWindow(FALSE);
}

void CLabelDlg::OnFont() 
{
	CFontDialog dlg;
	
	if (dlg.DoModal() == IDOK)
	{
		LOGFONT logFont;
		dlg.GetCurrentFont (&logFont);
		m_pLabel->SetFont (&logFont);
		m_pLabel->SetTextColor(dlg.GetColor ());
		OnChanged();
	}
	
}

#pragma warning (default : 4244)
#pragma warning (default : 4800)

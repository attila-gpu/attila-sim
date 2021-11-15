#if !defined(AFX_MOVINGAVGDLG_H__069695F4_B99B_4BA7_83F7_EEAA64B5460E__INCLUDED_)
#define AFX_MOVINGAVGDLG_H__069695F4_B99B_4BA7_83F7_EEAA64B5460E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// MovingAvgDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CMovingAvgDlg 

class CMovingAvgDlg : public CDialog
{
// Konstruktion
public:
	CMovingAvgDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CMovingAvgDlg)
	CSpinButtonCtrl	m_SpinWSize;
	int		m_nType;
	UINT	m_nSize;
	//}}AFX_DATA


	int m_nMaxSize;
// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CMovingAvgDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CMovingAvgDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_MOVINGAVGDLG_H__069695F4_B99B_4BA7_83F7_EEAA64B5460E__INCLUDED_

#if !defined(AFX_TRENDDLG_H__E26907DF_8686_47C8_84AA_CB34CA652FF3__INCLUDED_)
#define AFX_TRENDDLG_H__E26907DF_8686_47C8_84AA_CB34CA652FF3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// TrendDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CTrendDlg 

class CTrendDlg : public CDialog
{
// Konstruktion
public:
	CTrendDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CTrendDlg)
	CSpinButtonCtrl	m_Degree;
	CSpinButtonCtrl	m_Spin;
	UINT	m_nSize;
	UINT	m_nDegree;
	//}}AFX_DATA


// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CTrendDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CTrendDlg)
	virtual BOOL OnInitDialog();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_TRENDDLG_H__E26907DF_8686_47C8_84AA_CB34CA652FF3__INCLUDED_

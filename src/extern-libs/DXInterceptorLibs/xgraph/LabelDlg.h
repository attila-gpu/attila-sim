#if !defined(AFX_LABELDLG_H__9933FDF4_A574_4CF4_A66F_0CEC6FC3CEE1__INCLUDED_)
#define AFX_LABELDLG_H__9933FDF4_A574_4CF4_A66F_0CEC6FC3CEE1__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// LabelDlg.h : Header-Datei
//

#include "ColorButton.h"
#include "XGraphLabel.h"
#include "XGraph.h"


/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CLabelDlg 

class CLabelDlg : public CDialog
{
// Konstruktion
public:
	CLabelDlg(CWnd* pParent = NULL);   // Standardkonstruktor

// Dialogfelddaten
	//{{AFX_DATA(CLabelDlg)
	CComboBox	m_Curve;
	CColorButton	m_Color;
	BOOL	m_bBorder;
	int		m_nAlignment;
	int		m_nCurve;
	BOOL	m_bTransparent;
	//}}AFX_DATA

	CXGraphLabel* m_pLabel;

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CLabelDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CLabelDlg)
	afx_msg void OnChanged();
	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnColorChanged(WPARAM, LPARAM);
	virtual void OnOK();
	afx_msg void OnApply();
	afx_msg void OnFont();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_LABELDLG_H__9933FDF4_A574_4CF4_A66F_0CEC6FC3CEE1__INCLUDED_

#if !defined(AFX_CHARTPAGE_H__6A977A26_3AD8_45A7_B097_3E2CDC195D21__INCLUDED_)
#define AFX_CHARTPAGE_H__6A977A26_3AD8_45A7_B097_3E2CDC195D21__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ChartPage.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CChartPage 

#include "ChartDlg.h"
#include "AxisDlg.h"
#include "CurveDlg.h"

class CChartPage : public CDialog
{
	friend class CCurveDlg;
	friend class CAxisDlg;
	friend class CChartDlg;

// Konstruktion
public:
	CChartPage(CWnd* pParent = NULL);   // Standardkonstruktor
	virtual ~CChartPage();

	void SelectXAxis(int nItem);
	void SelectYAxis(int nItem);
	void SelectCurve(int nItem);

// Dialogfelddaten
	//{{AFX_DATA(CChartPage)
	CTabCtrl	m_Tab;
	//}}AFX_DATA

	CChartDlg* m_pChart;
	CCurveDlg* m_pCurve;
	CAxisDlg*  m_pAxis;


	CXGraph* m_pGraph;
	CXGraphAxis* m_pGraphAxis;
	CXGraphDataSerie* m_pGraphDataSerie;

// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CChartPage)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:


	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CChartPage)
	virtual BOOL OnInitDialog();
	afx_msg void OnSelchangeTab(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_CHARTPAGE_H__6A977A26_3AD8_45A7_B097_3E2CDC195D21__INCLUDED_

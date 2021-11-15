#if !defined(AFX_AXISDLG_H__1938BFDC_F7A8_4F97_8D26_C5D55063EC8B__INCLUDED_)
#define AFX_AXISDLG_H__1938BFDC_F7A8_4F97_8D26_C5D55063EC8B__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// AxisDlg.h : Header-Datei
//

/////////////////////////////////////////////////////////////////////////////
// Dialogfeld CAxisDlg 

#include "XGraphAxis.h"
#include "XGraph.h"
#include "ColorButton.h"

class CAxisDlg : public CDialog
{
// Konstruktion
public:
	CAxisDlg(CWnd* pParent = NULL);   // Standardkonstruktor

	void PresetValues();
// Dialogfelddaten
	//{{AFX_DATA(CAxisDlg)
	CSpinButtonCtrl	m_SpinMarkerSize;
	CSpinButtonCtrl	m_SpinArcSize;
	CSpinButtonCtrl	m_SpinTickSize;
	CColorButton	m_GridColor;
	CColorButton	m_Color;
	CSpinButtonCtrl	m_SpinPrecision;
	CString	m_cName;
	BOOL	m_bAutoScale;
	double	m_fMin;
	double	m_fMax;
	BOOL	m_bShowGrid;
	BOOL	m_bTimeAxis;
	BOOL    m_bVisible;
	UINT	m_nPrecision;
	UINT	m_nArcSize;
	UINT	m_nTickSize;
	UINT	m_nMarkerSize;
	double	m_fStep;
	COleDateTime	m_EndDate;
	COleDateTime	m_EndTime;
	COleDateTime	m_StartDate;
	COleDateTime	m_StartTime;
	CString	m_cDateTimeFormat;
	BOOL	m_bShowMarker;
	int		m_nPlacement;
	int		m_nGridType;
	int		m_nTimeStepType;
	UINT	m_nTimeStep;
	//}}AFX_DATA


	CXGraphAxis* m_pAxis;
// Überschreibungen
	// Vom Klassen-Assistenten generierte virtuelle Funktionsüberschreibungen
	//{{AFX_VIRTUAL(CAxisDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV-Unterstützung
	//}}AFX_VIRTUAL

// Implementierung
protected:

	// Generierte Nachrichtenzuordnungsfunktionen
	//{{AFX_MSG(CAxisDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnApply();
	afx_msg LRESULT OnColorChanged(WPARAM, LPARAM);
	afx_msg void OnSelectFont();
	afx_msg void OnTitleFont();
	afx_msg void OnCbautoscale();
	afx_msg void OnCbtimeaxis();
	virtual void OnOK();
	afx_msg void OnChanged();
	afx_msg void OnDrawItem(int nIDCtl, LPDRAWITEMSTRUCT lpDrawItemStruct);
	afx_msg void OnMeasureItem(int nIDCtl, LPMEASUREITEMSTRUCT lpMeasureItemStruct);
	afx_msg void OnDateTimeChanged(NMHDR *,LRESULT *);

	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ fügt unmittelbar vor der vorhergehenden Zeile zusätzliche Deklarationen ein.

#endif // AFX_AXISDLG_H__1938BFDC_F7A8_4F97_8D26_C5D55063EC8B__INCLUDED_

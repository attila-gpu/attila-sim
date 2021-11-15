// XGraphLabel.cpp: Implementierung der Klasse CXGraphLabel.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XGraph.h"
#include "XGraphLabel.h"
#ifndef _WIN32_WCE
#include "LabelDlg.h"
#endif
#include "GfxUtils.h"
#include "xgraph_resources.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL( CXGraphLabel, CXGraphObject, 1 )

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CXGraphLabel::CXGraphLabel()
{
	m_nCurve = -1;
	m_crColor = ::GetSysColor(COLOR_WINDOW);
	m_cText   = _T(" ");
	m_bBorder = true;
	m_nAlignment = DT_LEFT;
	m_bEditing = false;
	m_bTransparent = false;
	m_bCanMove = m_bCanResize = true;
	m_crTextColor = 0L;
	m_Font.CreateFont(14, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			   OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
			   DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));
}

CXGraphLabel::~CXGraphLabel()
{
	m_Font.DeleteObject ();

	if (::IsWindow (m_Edit.m_hWnd))
		m_Edit.DestroyWindow ();

}

void CXGraphLabel::Draw(CDCEx *pDC)
{	
	if (!m_bVisible)
		return;
	
	CFontSelector   fs(&m_Font, pDC);
	CBrushSelector* bs = NULL;
	CPenSelector*   ps = NULL;
	CGdiObject*     pOldObject = NULL;

	if (!m_bTransparent)
		bs = new CBrushSelector(pDC->m_bMono ? RGB(255,255,255) : m_crColor, pDC);
	else
		pOldObject = pDC->SelectStockObject (NULL_BRUSH);
		
	if (m_bBorder)
		ps = new CPenSelector (0, 1, pDC);

	if (m_nCurve != -1)
	{
		CXGraphDataSerie& serie = m_pGraph->GetCurve (m_nCurve);
		CXGraphAxis& xAxis      = m_pGraph->GetXAxis (serie.m_nXAxis);
		CXGraphAxis& yAxis      = m_pGraph->GetYAxis (serie.m_nYAxis);
		
		int		nXPos = xAxis.GetPointForValue (&serie.m_pData[serie.m_nCount / 2]).x;
		int		nYPos = 0;
		double	fVal  = xAxis.GetValueForPos (nXPos);
		
		nYPos = yAxis.GetPointForValue (&serie.m_pData[serie.m_nCount / 2]).y;

		pDC->MoveTo(nXPos, nYPos);
		pDC->LineTo(m_clRect.CenterPoint ().x, m_clRect.CenterPoint ().y);
	}
	
	pDC->SetBkMode(OPAQUE);

	if (!m_bTransparent)
		pDC->FillSolidRect(m_clRect, pDC->m_bMono ? RGB(255,255,255) : m_crColor);
	else
		pDC->SetBkMode(TRANSPARENT);

	
	if (m_bBorder)
		pDC->Rectangle(m_clRect);

	pDC->SetBkColor(pDC->m_bMono ? RGB(255,255,255) : m_crColor);
	
	COLORREF nOldColor = pDC->SetTextColor (pDC->m_bMono ? 0L : m_crTextColor);
	
	if (m_bSizing)
		m_Tracker.Draw (pDC);
		
	CRect rect = m_clRect;
	rect.DeflateRect (1,1,1,1);
	
	if (m_bEditing)
		m_Edit.ShowWindow (SW_SHOW);
	else
		pDC->DrawText(m_cText, rect, m_nAlignment);
	
	pDC->SetTextColor (nOldColor);
	
	if (pOldObject)
		pDC->SelectObject (pOldObject);
	
	if (bs)
		delete bs;

	if (ps)
		delete ps;
}

void CXGraphLabel::Edit()
{
	if (!m_bCanEdit)
		return;

	if (::IsWindow (m_Edit.m_hWnd))
		m_Edit.DestroyWindow ();

	m_bSizing = false;
	m_bEditing = true;

	CRect rect = m_clRect;
	rect.DeflateRect (1,1,1,1);
	m_Edit.Create (m_nAlignment | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_MULTILINE | ES_WANTRETURN | WS_CHILD | WS_VISIBLE, rect, m_pGraph, 0xFFFE);
	m_Edit.SetFont (&m_Font);
	m_Edit.SetWindowText(m_cText);
	m_Edit.ShowWindow (SW_SHOW);
	m_pGraph->Invalidate ();
	m_Edit.SetFocus ();
	m_Edit.SetSel (0,-1);
}

void CXGraphLabel::EndEdit()
{
	if (!m_bCanEdit)
		return;

	m_Edit.GetWindowText (m_cText);
	m_Edit.DestroyWindow ();

	m_bEditing = false;
}


void CXGraphLabel::InvokeProperties()
{
#ifndef _WIN32_WCE
	CPoint point;

	GetCursorPos(&point);

	CMenu menu;
	menu.CreatePopupMenu ();
	menu.AppendMenu( MF_STRING, IDM_LABELDOPROP, _T("Properties"));
	UINT nCmd = menu.TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
									  point.x, point.y ,
									  m_pGraph);

	if (nCmd == IDM_LABELDOPROP)
	{
		CLabelDlg dlg;
		dlg.m_pLabel = this;
		dlg.DoModal();
	}
#endif
}

void CXGraphLabel::SetFont(LOGFONT* pLogFont)
{
	if (m_Font.m_hObject != NULL)
		m_Font.DeleteObject ();
	
	m_Font.CreateFontIndirect (pLogFont);
}


void CXGraphLabel::Serialize( CArchive& archive )
{
	CXGraphObject::Serialize (archive);

    if( archive.IsStoring() )
    {
		archive << m_cText;
	
		LOGFONT logFont;
		m_Font.GetLogFont (&logFont);

		archive.Write (&logFont, sizeof(LOGFONT));
		
		archive << m_crColor;
		archive << m_crTextColor;
		archive << m_bBorder;
		archive << m_bTransparent;
		archive << m_nAlignment;
		archive << m_nCurve;
    }
	else
    {
		archive >> m_cText;

		LOGFONT logFont;
		archive.Read (&logFont, sizeof(LOGFONT));

		if (m_Font.m_hObject != NULL)
			m_Font.DeleteObject ();

		m_Font.CreateFontIndirect (&logFont);

		archive >> m_crColor;
		archive >> m_crTextColor;
		archive >> m_bBorder;
		archive >> m_bTransparent;
		archive >> m_nAlignment;

		archive >> m_nCurve;
    }

}
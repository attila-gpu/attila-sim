// XGraphDataNotation.cpp: Implementierung der Klasse XGraphDataNotation.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XGraphDataNotation.h"
#include "XGraphDataSerie.h"
#include "XGraph.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif 

IMPLEMENT_SERIAL( CXGraphDataNotation, CXGraphObject, 1 )

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CXGraphDataNotation::CXGraphDataNotation()
{
	m_Font.CreateFont(12, 0, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			   OUT_DEFAULT_PRECIS, CLIP_CHARACTER_PRECIS, ANTIALIASED_QUALITY,
			   DEFAULT_PITCH | FF_DONTCARE, _T("Arial"));

	m_bCanMove = m_bCanEdit = m_bCanResize = false;
	m_bPositioned = false;
}

CXGraphDataNotation::~CXGraphDataNotation()
{
	m_Font.DeleteObject();

}

void CXGraphDataNotation::Draw(CDCEx *pDC)
{
	TDataPoint point;

	if (!m_bVisible)
		return;
	
	if (m_bSizing)
		m_Tracker.Draw (pDC);
	
	CFontSelector   fs(&m_Font, pDC);
	
	point.fXVal = m_fXVal;
	point.fYVal = m_fYVal;

	int nXPos = m_pGraph->GetXAxis (m_pGraph->GetCurve (m_nCurve).GetXAxis ()).GetPointForValue (&point).x;
	int nYPos = m_pGraph->GetYAxis (m_pGraph->GetCurve (m_nCurve).GetYAxis ()).GetPointForValue (&point).y;
	
	if (!m_bPositioned)
	{
		m_clRect.SetRect (nXPos, nYPos - 20, nXPos + 1, nYPos);
		pDC->DrawText(m_cText, m_clRect,  DT_CENTER | DT_CALCRECT);
		m_bPositioned = true;
	}

	m_clRect.OffsetRect (nXPos - m_clRect.left, nYPos - m_clRect.top - 20);
	m_clRect.InflateRect (1,1,1,1);
	pDC->FillSolidRect (m_clRect, 0L);
	m_clRect.DeflateRect (1,1,1,1);
	pDC->FillSolidRect (m_clRect, RGB(255,255,255));
	pDC->DrawText(m_cText, m_clRect,  DT_CENTER);

	pDC->FillSolidRect (nXPos - 1, nYPos - 1, 3, 3, 0);
	pDC->MoveTo(nXPos, nYPos);
	pDC->LineTo(nXPos, m_clRect.bottom);
}


void CXGraphDataNotation::Serialize( CArchive& archive )
{
	CXGraphObject::Serialize (archive);

    if( archive.IsStoring() )
    {
		archive << m_fXVal;
		archive << m_fYVal;
		archive << m_nCurve;
		archive << m_nIndex;
		archive << m_cText;
    }
	else
    {
		archive >> m_fXVal;
		archive >> m_fYVal;
		archive >> m_nCurve;
		archive >> m_nIndex;
		archive >> m_cText;
    }

}

// XGraphBitmap.cpp: Implementierung der Klasse CXGraphBitmap.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XGraph.h"
#include "XGraphBitmap.h"
//#include "BitmapDlg.h"
#include "GfxUtils.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

IMPLEMENT_SERIAL( CXGraphBitmap, CXGraphObject, 1 )

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CXGraphBitmap::CXGraphBitmap() 
{
	m_pBitmap = NULL;
	m_bBorder = true;
	m_bEditing = false;
	m_bLocalCreated = false;
	m_bCanMove = m_bCanResize = true;
	
}

CXGraphBitmap::CXGraphBitmap(const CXGraphBitmap& copy)
{
	*this = copy;
}

CXGraphBitmap& CXGraphBitmap::operator=(const CXGraphBitmap& copy)
{
	m_clRect     = copy.m_clRect;
	m_bSelected  = copy.m_bSelected;
	m_bCanResize = copy.m_bCanResize;
	m_bCanMove	 = copy.m_bCanMove;
	m_bSizing	 = copy.m_bSizing;
	m_bEditing	 = copy.m_bEditing;
	m_crColor	 = copy.m_crColor;
	m_pGraph	 = copy.m_pGraph;
	m_Tracker	 = copy.m_Tracker;
	m_bVisible   = copy.m_bVisible;
	m_bCanEdit   = copy.m_bCanEdit;
	m_bBorder    = copy.m_bBorder;
	
	m_pBitmap = (CBitmapEx*) new CBitmapEx;
	BITMAP bitmap;
	copy.m_pBitmap->GetBitmap (&bitmap);
	m_pBitmap->CreateBitmapIndirect (&bitmap);
	m_bLocalCreated = true;

	return *this;
}

CXGraphBitmap::~CXGraphBitmap()
{
	if (m_bLocalCreated)
	{
		m_pBitmap->DeleteObject ();
		delete m_pBitmap;
	}

}

void CXGraphBitmap::Draw(CDCEx *pDC)
{	
	if (!m_bVisible)
		return;
	
	CDCEx  dc;
	BITMAP bmp;
	CRect  bmpRect = m_clRect;

	if (m_bBorder)
	{
		CPenSelector ps(0, 1, pDC);
		pDC->Rectangle(m_clRect);
		bmpRect.DeflateRect (1,1,1,1);
	}
	
	if (pDC->m_bPrinting)
		pDC->AdjustRatio (bmpRect);

	m_pBitmap->GetBitmap(&bmp);

	dc.CreateCompatibleDC (pDC);
	
	CGdiObject* pOldObject = dc.SelectObject (m_pBitmap);

	pDC->StretchBlt(bmpRect.left, bmpRect.top, bmpRect.Width(), bmpRect.Height(), &dc, 0, 0, bmp.bmWidth, bmp.bmHeight, SRCCOPY);

	dc.SelectObject (pOldObject);
		
	if (m_bSizing)
	{
		if (pDC->m_bPrinting)
		{
			CRect rect = m_Tracker.m_rect;
			pDC->AdjustRatio (m_Tracker.m_rect); 
			m_Tracker.Draw (pDC);
			m_Tracker.m_rect = rect;
		}
		else
			m_Tracker.Draw (pDC);
	}



}

void CXGraphBitmap::InvokeProperties()
{
	CPoint point;

	/*GetCursorPos(&point);

	CMenu menu;
	menu.CreatePopupMenu ();
	menu.AppendMenu( MF_STRING, IDM_BITMAPDOPROP, _T("Properties"));
	UINT nCmd = menu.TrackPopupMenu( TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY | TPM_RETURNCMD,
									  point.x, point.y ,
									  m_pGraph);

	if (nCmd == IDM_BITMAPDOPROP)
	{
		CBitmapDlg dlg;
		dlg.m_pBitmap = this;
		dlg.DoModal();
	}*/
}

void CXGraphBitmap::PrepareClipboard(CFBITMAP& bitmap)
{
	bitmap.bBorder = m_bBorder;
	memcpy(&bitmap.rect, (LPRECT)m_clRect, sizeof(RECT));
}


void CXGraphBitmap::Serialize( CArchive& archive )
{
	CXGraphObject::Serialize (archive);

    if( archive.IsStoring() )
    {
		archive << m_bBorder;
		((CBitmapEx*)m_pBitmap)->Serialize (archive);
    }
	else
    {
		archive >> m_bBorder;

		if (!m_pBitmap)
		{
			m_pBitmap = (CBitmapEx*) new CBitmapEx;
			m_bLocalCreated = true;
		}
	
		m_pBitmap->Serialize (archive);
    }

}
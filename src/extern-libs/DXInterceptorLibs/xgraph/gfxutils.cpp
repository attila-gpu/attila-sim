#include "stdafx.h"
#include "gfxutils.h"

#pragma warning (disable : 4244)

CSize GetTextExtentEx(CDCEx *pDC, CString cTxt)
{
	CSize size; 

	bool bMultiLine = false;

	for (int i = 0; i < cTxt.GetLength (); i++)
		if (cTxt[i] == '\n')
			bMultiLine = true;

	if (bMultiLine)
	{
		CRect rect;
		int nHeight = pDC->DrawText(cTxt, rect, DT_CENTER | DT_VCENTER | DT_CALCRECT);
		size.cx = rect.Width();
		size.cy = nHeight;
	}
	else
		return pDC->GetTextExtent (cTxt);

	return size;
}

COLORREF GetColorMono(COLORREF color)
{
	COLORREF clr = 0L;

	if ((GetRValue(color) + GetGValue(color)  + GetBValue(color) ) / 3 > 127)
		clr = RGB(255,255,255);

	return clr;
}

CDCEx::CDCEx ()
{

	m_bPrepared = false;
	m_bPrinting = false;
	m_bMono     = false;
	m_bPrintPreview = false;
	m_fScaleX = m_fScaleY = 1.0;
}

CDCEx::~CDCEx()
{
} 

void CDCEx::AdjustRatio(RECT& rect)
{
	if (!m_bPrepared)
		Prepare(m_hDC);

    rect.left *= m_fScaleX;
	rect.top  *= m_fScaleY;
	rect.right *= m_fScaleX;
	rect.bottom *= m_fScaleY;
}

void CDCEx::AdjustRatio(POINT& point)
{
	if (!m_bPrepared)
		Prepare(m_hDC);

    point.x *= m_fScaleX;
	point.y *= m_fScaleY;

}

void CDCEx::AdjustRatio(int& x, int& y)
{
	if (!m_bPrepared)
		Prepare(m_hDC);

    x *= m_fScaleX;
	y *= m_fScaleY;

}

void CDCEx::Prepare (HDC hdc)
{
	HDC hWinDC = GetDC(AfxGetMainWnd()->m_hWnd);
	int DevCaps;

	
    float fLogPelsX1 = (float) ::GetDeviceCaps(hWinDC, LOGPIXELSX); 
    float fLogPelsY1 = (float) ::GetDeviceCaps(hWinDC, LOGPIXELSY); 
	float fLogPelsX2;
	float fLogPelsY2;

	if (hdc == m_hDC)
	{
		fLogPelsX2 = (float) ::GetDeviceCaps(m_hDC, LOGPIXELSX); 
		fLogPelsY2 = (float) ::GetDeviceCaps(m_hDC, LOGPIXELSY); 
	}
	else
	{
		fLogPelsX2 = (float) ::GetDeviceCaps(hdc, LOGPIXELSX); 
		fLogPelsY2 = (float) ::GetDeviceCaps(hdc, LOGPIXELSY); 
	}
	
    if (fLogPelsX1 > fLogPelsX2) 
        m_fScaleX = (fLogPelsX1 / fLogPelsX2); 
    else m_fScaleX = (fLogPelsX2 / fLogPelsX1); 
 
    if (fLogPelsY1 > fLogPelsY2) 
        m_fScaleY = (fLogPelsY1 / fLogPelsY2); 
    else m_fScaleY = (fLogPelsY2 / fLogPelsY1); 
	
	if (hdc == m_hDC)
	{
		DevCaps = ::GetDeviceCaps (m_hDC, NUMCOLORS);
		m_bMono = ((DevCaps >= 0) && (DevCaps < 256)); 
	}
	else
	{
		DevCaps = ::GetDeviceCaps (hdc, NUMCOLORS);
		m_bMono = ((DevCaps >= 0) && (DevCaps < 256)); 
	}

	if (m_fScaleY < 1E-10)
		m_fScaleY = 1.0;

	if (m_fScaleX < 1E-10)
		m_fScaleX = 1.0;

	m_bPrepared = true;

	::ReleaseDC(AfxGetMainWnd()->m_hWnd, hWinDC);
}
    
CPoint CDCEx::MoveTo(int x, int y)
{
	if (m_bPrinting)
		AdjustRatio(x,y);
	
	return CDC::MoveTo(x,y);
}

CPoint CDCEx::MoveTo(POINT point)
{
	if (m_bPrinting)
		AdjustRatio(point);
	
	return CDC::MoveTo(point);
}

BOOL   CDCEx::LineTo(int x, int y)
{
	if (m_bPrinting)
		AdjustRatio(x,y);
	
	return CDC::LineTo(x,y);
}

BOOL   CDCEx::LineTo(POINT point)
{
	if (m_bPrinting)
		AdjustRatio(point);
	
	return CDC::LineTo(point);
}


void CDCEx::FillRect(LPCRECT lpRect, CBrush* pBrush)
{
	if (m_bPrinting)
	{
		CRect rect(lpRect);
		AdjustRatio(rect);
		CDC::FillRect (rect, pBrush);
	}
	else
		CDC::FillRect (lpRect, pBrush);
}

void CDCEx::DrawFocusRect(LPCRECT lpRect)
{
	if (m_bPrinting)
	{
		CRect rect(lpRect);
		AdjustRatio(rect);
		CDC::DrawFocusRect (rect);
	}
	else
		CDC::DrawFocusRect (lpRect);
}

BOOL CDCEx::Ellipse(int x1, int y1, int x2, int y2)
{
	if (m_bPrinting)
	{
		AdjustRatio(x1,y1);
		AdjustRatio(x2,y2);
	}

	return CDC::Ellipse (x1,y1,x2,y2);
}

BOOL CDCEx::Ellipse(LPCRECT lpRect)
{
	if (m_bPrinting)
	{
		CRect rect(lpRect);
		AdjustRatio(rect);
		return CDC::Ellipse (rect);
	}
	else
		return CDC::Ellipse (lpRect);

}
BOOL CDCEx::Polygon(LPPOINT lpPoints, int nCount)
{
	if (m_bPrinting)
	{
		LPPOINT points = new POINT[nCount];

		for (int i = 0; i < nCount; i++)
		{
			memcpy(&points[i], &lpPoints[i], sizeof(POINT));
			AdjustRatio(points[i]);
		}

		BOOL bRet = CDC::Polygon (points, nCount);
		delete points;
		return bRet;
	}
	else
		return CDC::Polygon (lpPoints, nCount);

}

BOOL CDCEx::Rectangle(int x1, int y1, int x2, int y2)
{
	if (m_bPrinting)
	{
		AdjustRatio(x1,y1);
		AdjustRatio(x2,y2);
	}

	return CDC::Rectangle (x1,y1,x2,y2);

}
BOOL CDCEx::Rectangle(LPCRECT lpRect)
{
	if (m_bPrinting)
	{
		CRect rect(lpRect);
		AdjustRatio(rect);
		return CDC::Rectangle (rect);
	}
	else
		return CDC::Rectangle (lpRect);

}
int CDCEx::DrawText(const CString& str, LPRECT lpRect, UINT nFormat)
{
	if (m_bPrinting)
	{
		if (nFormat & DT_CALCRECT)
		{
			AdjustRatio(*lpRect);
			return CDC::DrawText(str, lpRect, nFormat);
		}
		else
		{
			CRect rect(lpRect);
			AdjustRatio(rect);
			return CDC::DrawText(str, rect, nFormat);
		}
	}
	else
		return CDC::DrawText(str, lpRect, nFormat);

}
void CDCEx::FillSolidRect(LPCRECT lpRect, COLORREF clr)
{
	if (m_bPrinting)
	{
		CRect rect(lpRect);
		AdjustRatio(rect);
		CDC::FillSolidRect (rect, clr);
	}
	else
		CDC::FillSolidRect (lpRect, clr);

}
void CDCEx::FillSolidRect(int x, int y, int cx, int cy, COLORREF clr)
{
	if (m_bPrinting)
	{
		AdjustRatio(x,y);
		AdjustRatio(cx,cy);
	}

	CDC::FillSolidRect (x,y,cx,cy,clr);
}


COLORREF CDCEx::SetTextColor(COLORREF crColor)
{
	return CDC::SetTextColor (crColor);
}

COLORREF CDCEx::SetBkColor(COLORREF crColor)
{
	return CDC::SetBkColor (crColor);

}
	

void DrawRect(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC)
{
	CPenSelector   ps(0L, 1, pDC);
	CBrushSelector bs(pDC->m_bMono ? 0L : color, pDC);

	pDC->Rectangle(point.x, point.y, point.x + nSize, point.y + nSize);
}

void DrawCircle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC)
{
	CPenSelector   ps(0L, 1, pDC);
	CBrushSelector bs(pDC->m_bMono ? 0L : color, pDC);
	pDC->Ellipse (point.x , point.y, point.x + nSize, point.y + nSize);
}

void DrawLeftTriangle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC)
{
	CPenSelector   ps(0L, 1, pDC);
	CBrushSelector bs(pDC->m_bMono ? 0L : color, pDC);

	POINT points[4];

	points[0].x = point.x + nSize - 1;
	points[0].y = point.y;

	points[1].x = point.x ;
	points[1].y = point.y + nSize / 2;

	points[2].x = point.x + nSize - 1;
	points[2].y = point.y + nSize;

	pDC->Polygon (points, 3);
}

void DrawUpTriangle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC)
{
	CPenSelector   ps(0L, 1, pDC);
	CBrushSelector bs(pDC->m_bMono ? 0L : color, pDC);

	POINT points[4];

	points[0].x = point.x + nSize / 2;
	points[0].y = point.y;

	points[1].x = point.x + nSize;
	points[1].y = point.y + nSize;

	points[2].x = point.x ;
	points[2].y = point.y + nSize;

	pDC->Polygon (points, 3);
}

void DrawRightTriangle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC)
{
	CPenSelector   ps(0L, 1, pDC);
	CBrushSelector bs(pDC->m_bMono ? 0L : color, pDC);

	POINT points[4];

	points[0].x = point.x;
	points[0].y = point.y;

	points[1].x = point.x + nSize;
	points[1].y = point.y + nSize / 2;

	points[2].x = point.x ;
	points[2].y = point.y + nSize;

	pDC->Polygon (points, 3);
}

void DrawDownTriangle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC)
{
	CPenSelector   ps(0L, 1, pDC);
	CBrushSelector bs(pDC->m_bMono ? 0L : color, pDC);

	POINT points[4];

	points[0].x = point.x;
	points[0].y = point.y;

	points[1].x = point.x + nSize;
	points[1].y = point.y;

	points[2].x = point.x + nSize / 2 ;
	points[2].y = point.y + nSize;

	pDC->Polygon (points, 3);
}

void DrawCross(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC)
{
	CPenSelector   ps(pDC->m_bMono ? 0L : color, 2, pDC);

	pDC->MoveTo(point);
	pDC->LineTo(point.x + nSize, point.y + nSize);
	pDC->MoveTo(point.x + nSize, point.y);
	pDC->LineTo(point.x, point.y + nSize);

}

void DrawRhombus(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC)
{
	CPenSelector   ps(0L, 1, pDC);
	CBrushSelector bs(pDC->m_bMono ? 0L : color, pDC);

	POINT points[5];

	points[0].x = point.x + nSize / 2;
	points[0].y = point.y;

	points[1].x = point.x + nSize;
	points[1].y = point.y + nSize / 2;

	points[2].x = point.x + nSize / 2 ;
	points[2].y = point.y + nSize;

	points[3].x = point.x;
	points[3].y = point.y + nSize / 2;

	pDC->Polygon (points, 4);

}


void DrawEmptyRect(CDCEx* pDC, CRect rect, COLORREF crColor)
{
	CPenSelector ps(crColor, 1, pDC);

	pDC->MoveTo (rect.left, rect.top);
	pDC->LineTo (rect.right, rect.top);
	pDC->LineTo (rect.right, rect.bottom);
	pDC->LineTo (rect.left, rect.bottom);
	pDC->LineTo (rect.left, rect.top);

}

void DrawPatternRectangle(CDCEx* pDC, CRect rect, COLORREF crPattern, COLORREF crOutline, UINT nStyle)
{
	if (pDC->m_bPrinting)
	{
		crPattern = pDC->GetNearestColor (crPattern);
		crOutline = pDC->GetNearestColor (crOutline);
	}

	if (nStyle == HS_SOLID)
		pDC->FillSolidRect (rect, crPattern);
	
	DrawEmptyRect(pDC, rect, crOutline);
}




void DrawAlphaBlendRect(CDCEx* pDC, CRect& rect, COLORREF crColor, BYTE nAlpha)
{
	CRect clipRect;
	CDC   memDC;
	
	pDC->GetClipBox(&clipRect);
    	
	memDC.CreateCompatibleDC (pDC);
	
	CBitmap bmp;
	bmp.CreateCompatibleBitmap (&memDC, clipRect.Width(), clipRect.Height ());

	CBitmap* pOldBmp = memDC.SelectObject (&bmp);

	memDC.FillSolidRect (rect, crColor);
	
	BLENDFUNCTION bf; // structure for alpha blending
	
	bf.BlendOp = AC_SRC_OVER;
	bf.BlendFlags = 0;
	bf.SourceConstantAlpha = nAlpha; 
	bf.AlphaFormat = 0; 

	::AlphaBlend(pDC->m_hDC, rect.left, rect.top, rect.Width(), rect.Height(), memDC.m_hDC, rect.left, rect.top, rect.Width(), rect.Height(), bf);

	memDC.SelectObject (pOldBmp);


}


#pragma warning (default : 4244)
#ifndef GFX_UTILS_H
#define GFX_UTILS_H


#include "stdafx.h"

// Creates a pen within the constructor,
// selects the pen into the DC and
// restores everything when the destructor is called

#define HS_SOLID 6

#pragma warning (disable : 4244)

COLORREF GetColorMono(COLORREF color);

class CDCEx : public CDC
{
private:

	
public:

	CDCEx ();
	virtual ~CDCEx();

	void Prepare(HDC hdc);

	void AdjustRatio(RECT& rect);
	void AdjustRatio(POINT& point);
	void AdjustRatio(int& x, int& y);

	virtual COLORREF SetTextColor(COLORREF crColor);
	virtual COLORREF SetBkColor(COLORREF crColor);
	
	CPoint MoveTo(int x, int y);
	CPoint MoveTo(POINT point);
	BOOL   LineTo(int x, int y);
	BOOL   LineTo(POINT point);
	void   FillRect(LPCRECT lpRect, CBrush* pBrush);
	void   DrawFocusRect(LPCRECT lpRect);
	BOOL   Ellipse(int x1, int y1, int x2, int y2);
	BOOL   Ellipse(LPCRECT lpRect);
	BOOL   Polygon(LPPOINT lpPoints, int nCount);
	BOOL   Rectangle(int x1, int y1, int x2, int y2);
	BOOL   Rectangle(LPCRECT lpRect);
	int    DrawText(const CString& str, LPRECT lpRect, UINT nFormat);
	void   FillSolidRect(LPCRECT lpRect, COLORREF clr);
	void   FillSolidRect(int x, int y, int cx, int cy, COLORREF clr);
	
	bool   m_bPrepared;
	bool   m_bMono;
	bool   m_bPrinting;
	bool   m_bPrintPreview;
	float  m_fScaleX,
		   m_fScaleY;


};

typedef void  (*lpDrawFunc) (CPoint, int, COLORREF, bool, CDCEx*);

class CRgnEx : public CRgn
{
public:
	CRgnEx() {};
	virtual ~CRgnEx() { DeleteObject(); };

	CRgnEx(const CRgnEx& copy) { CreateRectRgn(0,0,0,0); CopyRgn((CRgn*)(&copy)); };
	void operator= (const CRgnEx& copy) { CreateRectRgn(0,0,0,0); CopyRgn((CRgn*) (&copy)); };
};

class CPenSelector
{
	private:
		CPen *m_pPen, *m_poPen;
		CDCEx  *m_pDC;
	public:

	CPenSelector(COLORREF color, int nSize, CDCEx *pDC, UINT nStyle = PS_SOLID)
	{
		m_pDC = pDC;
		if (!m_pDC->m_bPrepared)
			m_pDC->Prepare(m_pDC->m_hDC);

		m_pPen = (CPen*) new CPen(nStyle, pDC->m_bPrinting && nStyle == PS_SOLID ? pDC->m_fScaleX * nSize : nSize,  color);
		m_poPen = m_pDC->SelectObject(m_pPen);
	};

	virtual ~CPenSelector()
	{
		m_pDC->SelectObject(m_poPen);
		m_pPen->DeleteObject();
		delete m_pPen;
	};
};

class CBrushSelector
{
	private:
		CBrush *m_pBrush, *m_poBrush;
		CDCEx  *m_pDC;
	public:

#ifndef _WIN32_WCE
	CBrushSelector(COLORREF color, int nIndex, CDCEx *pDC)
	{
		m_pDC = pDC;
		m_pBrush = (CBrush*) new CBrush(nIndex, color );
		m_poBrush = m_pDC->SelectObject(m_pBrush);
	}
#endif

	CBrushSelector(COLORREF color, CDCEx *pDC)
	{
		m_pDC = pDC;
		m_pBrush = (CBrush*) new CBrush(color);
		m_poBrush = m_pDC->SelectObject(m_pBrush);
	};

	virtual ~CBrushSelector()
	{
		m_pDC->SelectObject(m_poBrush);
		m_pBrush->DeleteObject();
		delete m_pBrush;
	};
};

class CQuickFont : public CFont
{
public :
	CQuickFont(CString cFontName, int nHeight, int nWidth, bool bUnderLine = false)
	{
		CreateFont(nHeight,0,0,0, nWidth, 0,bUnderLine ? 1 : 0,0, DEFAULT_CHARSET,
			       OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, ANTIALIASED_QUALITY,
				    DEFAULT_PITCH | FF_DONTCARE, cFontName);
	}

	CQuickFont(LOGFONT* lpLogFont)
	{
		CreateFontIndirect(lpLogFont);
	}

	virtual ~CQuickFont()
	{
		DeleteObject();
	};
};

class CVerticalQuickFont : public CFont
{
public :
	CVerticalQuickFont(CString cFontName, int nHeight, int nWidth, bool bUnderLine = false)
	{
		CreateFont(nHeight,0,900,900, nWidth, 0,bUnderLine ? 1 : 0,0, DEFAULT_CHARSET,
			       OUT_DEFAULT_PRECIS, CLIP_LH_ANGLES, ANTIALIASED_QUALITY,
				    DEFAULT_PITCH | FF_DONTCARE, cFontName);
	}

	CVerticalQuickFont(LOGFONT* lpLogFont)
	{
		CreateFontIndirect(lpLogFont);
	}

	virtual ~CVerticalQuickFont()
	{
		DeleteObject();
	};
};


// Selects a font into the DC when the constructor is called and
// restores everything when the destructor is called

class CFontSelector
{
private:
	CFont  m_PrintFont;
	CFont *m_pFont;
	CDCEx   *m_pDC;
public:

	CFontSelector(CFont* pFont, CDCEx *pDC, bool bAdjust = true)
	{
			m_pDC = pDC;
		if (!m_pDC->m_bPrepared)
			m_pDC->Prepare(m_pDC->m_hDC);

		if (pDC->m_bPrinting && bAdjust)
		{
			LOGFONT logFont;
			pFont->GetLogFont (&logFont);
			logFont.lfHeight = (logFont.lfHeight-1) * pDC->m_fScaleY;
			logFont.lfWidth *= pDC->m_fScaleX;
			m_PrintFont.CreateFontIndirect(&logFont);
			m_pFont = m_pDC->SelectObject(&m_PrintFont);
		}
		else
			m_pFont = m_pDC->SelectObject(pFont);
	}

	virtual ~CFontSelector()
	{
		m_pDC->SelectObject(m_pFont);
		if (m_pDC->m_bPrinting)
			m_PrintFont.DeleteObject();
	}

};

void DrawRect(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC);
void DrawCircle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC);
void DrawCross(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC);
void DrawLeftTriangle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC);
void DrawUpTriangle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC);
void DrawRightTriangle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC);
void DrawDownTriangle(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC);
void DrawRhombus(CPoint point, int nSize, COLORREF color, bool bInvert, CDCEx* pDC);
void DrawPatternRectangle(CDCEx* pDC, CRect rect, COLORREF crPattern, COLORREF crOutline, UINT nStyle);
void DrawEmptyRect(CDCEx* pDC, CRect rect, COLORREF crColor);

CSize GetTextExtentEx(CDCEx *pDC, CString cTxt);

void DrawAlphaBlendRect(CDCEx* pDC, CRect& rect, COLORREF crColor, BYTE nAlpha);


#pragma warning (default : 4244)

#endif


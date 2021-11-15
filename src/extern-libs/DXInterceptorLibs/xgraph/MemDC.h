#if !defined(AFX_MEMDC_H__CA1D3541_7235_11D1_ABBA_00A0243D1382__INCLUDED_)
#define AFX_MEMDC_H__CA1D3541_7235_11D1_ABBA_00A0243D1382__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000
// MemDC.h : header file
//

//////////////////////////////////////////////////
// CMemDC - memory DC
//
// Author: Keith Rule
// Email:  keithr@europa.com
// Copyright 1996-1997, Keith Rule
//
// You may freely use or modify this code provided this
// Copyright is included in all derived versions.
//
// History - 10/3/97 Fixed scrolling bug.
//                   Added print support.
//           25 feb 98 - fixed minor assertion bug
//
// This class implements a memory Device Context


class CMemDC : public CDC
{
public:

	CMemDC(CDC* pDC, CRect wndRect) : CDC()
	{
		ASSERT(pDC != NULL);
		
		m_pDC = pDC;
		m_pOldBitmap = NULL;
        m_bMemDC = !pDC->IsPrinting();
		m_DstRect.SetRectEmpty();
		
        if (m_bMemDC)	// Create a Memory DC
		{
            m_rect = wndRect;
            CreateCompatibleDC(pDC);
            m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_pOldBitmap = SelectObject(&m_bitmap);
#ifndef _WIN32_WCE
            SetWindowOrg(m_rect.left, m_rect.top);
#endif
        }
		else		// Make a copy of the relevent parts of the current DC for printing
		{
#ifndef _WIN32_WCE
            m_bPrinting = pDC->m_bPrinting;
#endif
            m_hDC		= pDC->m_hDC;
            m_hAttribDC = pDC->m_hAttribDC;
        }

		FillSolidRect(m_rect, pDC->GetBkColor());
	};

	// constructor sets up the memory DC
	CMemDC(CDC* pDC, int nOffset = 0) : CDC()
    {
		ASSERT(pDC != NULL);
		
		m_pDC = pDC;
		m_pOldBitmap = NULL;
        m_bMemDC = !pDC->IsPrinting();
		m_DstRect.SetRectEmpty();
		
        if (m_bMemDC)	// Create a Memory DC
		{
            pDC->GetClipBox(&m_rect);
            CreateCompatibleDC(pDC);
            m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width() - nOffset, m_rect.Height());
			m_pOldBitmap = SelectObject(&m_bitmap);
#ifndef _WIN32_WCE
            SetWindowOrg(m_rect.left, m_rect.top);
#endif
        }
		else		// Make a copy of the relevent parts of the current DC for printing
		{
#ifndef _WIN32_WCE
            m_bPrinting = pDC->m_bPrinting;
#endif
            m_hDC		= pDC->m_hDC;
            m_hAttribDC = pDC->m_hAttribDC;
        }

		FillSolidRect(m_rect, pDC->GetBkColor());
	}
	
	CMemDC(CDC* pDC, CRect DstRect, CRect wndRect) : CDC()
    {
		ASSERT(pDC != NULL);
		
		m_pDC = pDC;
		m_pOldBitmap = NULL;
        m_bMemDC = !pDC->IsPrinting();
		m_rect = wndRect;
		m_DstRect = DstRect;
		if (m_bMemDC)	// Create a Memory DC
		{
            CreateCompatibleDC(pDC);
            m_bitmap.CreateCompatibleBitmap(pDC, m_rect.Width(), m_rect.Height());
			m_pOldBitmap = SelectObject(&m_bitmap);
#ifndef _WIN32_WCE
            SetWindowOrg(m_rect.left, m_rect.top);
#endif
        }
		else		// Make a copy of the relevent parts of the current DC for printing
		{
#ifndef _WIN32_WCE
            m_bPrinting = pDC->m_bPrinting;
#endif
            m_hDC		= pDC->m_hDC;
            m_hAttribDC = pDC->m_hAttribDC;
        } 

		FillSolidRect(m_rect, pDC->GetBkColor());
	}
	
	// Destructor copies the contents of the mem DC to the original DC
	~CMemDC()
    {
		if (m_bMemDC) {	
			if (!m_DstRect.IsRectEmpty())
				m_pDC->BitBlt(m_DstRect.left, m_DstRect.top, m_DstRect.Width(), m_DstRect.Height(),
				this, m_DstRect.left, m_DstRect.top, SRCCOPY);
			else
			// Copy the offscreen bitmap onto the screen.
			m_pDC->BitBlt(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height(),
				this, m_rect.left, m_rect.top, SRCCOPY);
			
			//Swap back the original bitmap.
            SelectObject(m_pOldBitmap);
			m_bitmap.DeleteObject();
		} else {
			// All we need to do is replace the DC with an illegal value,
			// this keeps us from accidently deleting the handles associated with
			// the CDC that was passed to the constructor.
            m_hDC = m_hAttribDC = NULL;
		}
	}
	
	// Allow usage as a pointer
    CMemDC* operator->() {return this;}
	
    // Allow usage as a pointer
    operator CMemDC*() {return this;}

private:
	CBitmap  m_bitmap;		// Offscreen bitmap
    CBitmap* m_pOldBitmap;	// bitmap originally found in CMemDC
    CDC*     m_pDC;			// Saves CDC passed in constructor
    CRect    m_rect;		// Rectangle of drawing area.
	CRect    m_DstRect;
    BOOL     m_bMemDC;		// TRUE if CDC really is a Memory DC.
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Developer Studio will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MEMDC_H__CA1D3541_7235_11D1_ABBA_00A0243D1382__INCLUDED_)

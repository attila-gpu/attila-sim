////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class CSize;
class CPoint;
class CRect;

////////////////////////////////////////////////////////////////////////////////
// CSize - Wrapper for Windows SIZE structure
////////////////////////////////////////////////////////////////////////////////

class CSize : public tagSIZE
{
public:
  
  // Constructors
	CSize();
	CSize(int initCX, int initCY);
	CSize(SIZE initSize);
	CSize(POINT initPt);
	CSize(DWORD dwSize);

  // Operations
	bool operator == (CSize size) const;
	bool operator != (CSize size) const;
	void operator += (CSize size);
	void operator -= (CSize size);
	void SetSize(int paramCX, int paramCY);

  // Operators returning CSize values
	CSize operator + (CSize size) const;
	CSize operator - (CSize size) const;
	CSize operator - () const;

  // Operators returning CPoint values
	CPoint operator + (CPoint point) const;
	CPoint operator - (CPoint point) const;

  // Operators returning CRect values
	CRect operator + (const RECT* lpRect) const;
	CRect operator - (const RECT* lpRect) const;

};

////////////////////////////////////////////////////////////////////////////////
// CPoint - Wrapper for Windows POINT structure
////////////////////////////////////////////////////////////////////////////////

class CPoint : public tagPOINT
{
public:
  
  // Constructors
	CPoint();
	CPoint(int initX, int initY);
	CPoint(const POINT &initPt);
	CPoint(const SIZE &initSize);
	CPoint(DWORD dwPoint);

  // Operations
	void Offset(int xOffset, int yOffset);
	void Offset(CPoint point);
	void Offset(CSize size);
	bool operator == (const CPoint &point) const;
	bool operator != (const CPoint &point) const;
	void operator += (CSize size);
	void operator -= (CSize size);
	void operator += (CPoint point);
	void operator -= (CPoint point);
	void SetPoint(int x, int y);

  // Operators returning CPoint values
	CPoint operator + (CSize size) const;
	CPoint operator - (CSize size) const;
	CPoint operator - () const;
	CPoint operator + (CPoint point) const;

  // Operators returning CSize values
	CSize operator - (CPoint point) const;

  // Operators returning CRect values
	CRect operator + (const RECT* lpRect) const;
	CRect operator - (const RECT* lpRect) const;

};

////////////////////////////////////////////////////////////////////////////////
// CRect - Wrapper for Windows RECT structure
////////////////////////////////////////////////////////////////////////////////

class CRect : public tagRECT
{
public:
  
  // Constructors
	CRect();
	CRect(int l, int t, int r, int b);
	CRect(const RECT& srcRect);
	CRect(LPCRECT lpSrcRect);
	CRect(POINT point, SIZE size);
	CRect(POINT topLeft, POINT bottomRight);

  // Attributes (in addition to RECT members)
	int Width() const;
	int Height() const;
	CSize Size() const;
	CPoint& LeftTop();
	CPoint& RightBottom();
	const CPoint& LeftTop() const;
	const CPoint& RightBottom() const;
	CPoint CenterPoint() const;

	// Convert between CRect and LPRECT/LPCRECT (no need for &)
	operator LPRECT();
	operator LPCRECT() const;

	bool IsRectEmpty() const;
	bool IsRectNull() const;
	bool PtInRect(POINT point) const;

  // Operations
	void SetRect(int x1, int y1, int x2, int y2);
	void SetRect(POINT topLeft, POINT bottomRight);
	void SetRectEmpty();
	void CopyRect(LPCRECT lpSrcRect);
	bool EqualRect(LPCRECT lpRect) const;

	void InflateRect(int x, int y);
	void InflateRect(SIZE size);
	void InflateRect(LPCRECT lpRect);
	void InflateRect(int l, int t, int r, int b);
	void DeflateRect(int x, int y);
	void DeflateRect(SIZE size);
	void DeflateRect(LPCRECT lpRect);
	void DeflateRect(int l, int t, int r, int b);

	void OffsetRect(int x, int y);
	void OffsetRect(SIZE size);
	void OffsetRect(POINT point);
	void NormalizeRect();

	// Absolute position of rectangle
	void MoveToX(int x);
	void MoveToY(int y);
	void MoveToXY(int x, int y);
	void MoveToXY(POINT point);

	// Operations that fill '*this' with result
	bool IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2);
	bool UnionRect(LPCRECT lpRect1, LPCRECT lpRect2);
	bool SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2);

  // Additional Operations
	void operator = (const RECT& srcRect);
	bool operator == (const RECT& rect) const;
	bool operator != (const RECT& rect) const;
	void operator += (POINT point);
	void operator += (SIZE size);
	void operator += (LPCRECT lpRect);
	void operator -= (POINT point);
	void operator -= (SIZE size);
	void operator -= (LPCRECT lpRect);
	void operator &= (const RECT& rect);
	void operator |= (const RECT& rect);

  // Operators returning CRect values
	CRect operator + (POINT point) const;
	CRect operator - (POINT point) const;
	CRect operator + (LPCRECT lpRect) const;
	CRect operator + (SIZE size) const;
	CRect operator - (SIZE size) const;
	CRect operator - (LPCRECT lpRect) const;
	CRect operator & (const RECT& rect2) const;
	CRect operator | (const RECT& rect2) const;
	CRect MulDiv(int nMultiplier, int nDivisor) const;

};

////////////////////////////////////////////////////////////////////////////////
// CSize Implementation
////////////////////////////////////////////////////////////////////////////////

inline CSize::CSize()
{
	/* random filled */
}

inline CSize::CSize(int initCX, int initCY)
{
	cx = initCX;
	cy = initCY;
}

inline CSize::CSize(SIZE initSize)
{
	*(SIZE*) this = initSize;
}

inline CSize::CSize(POINT initPt)
{
	*(POINT*)this = initPt;
}

inline CSize::CSize(DWORD dwSize)
{
	cx = (short) LOWORD(dwSize);
	cy = (short) HIWORD(dwSize);
}

inline bool CSize::operator == (CSize size) const
{
	return (cx == size.cx && cy == size.cy);
}

inline bool CSize::operator != (CSize size) const
{
	return (cx != size.cx || cy != size.cy);
}

inline void CSize::operator += (CSize size)
{
	cx += size.cx;
	cy += size.cy;
}

inline void CSize::operator -= (CSize size)
{
	cx -= size.cx;
	cy -= size.cy;
}

inline void CSize::SetSize(int paramCX, int paramCY)
{
	cx = paramCX;
	cy = paramCY;
}

inline CSize CSize::operator + (CSize size) const
{
	return CSize(cx + size.cx, cy + size.cy);
}

inline CSize CSize::operator - (CSize size) const
{
	return CSize(cx - size.cx, cy - size.cy);
}

inline CSize CSize::operator - () const
{
	return CSize(-cx, -cy);
}

inline CPoint CSize::operator + (CPoint point) const
{
	return CPoint(cx + point.x, cy + point.y);
}

inline CPoint CSize::operator - (CPoint point) const
{
	return CPoint(cx - point.x, cy - point.y);
}

inline CRect CSize::operator + (const RECT* lpRect) const
{
	return CRect(lpRect) + *this;
}

inline CRect CSize::operator - (const RECT* lpRect) const
{
	return CRect(lpRect) - *this;
}

////////////////////////////////////////////////////////////////////////////////
// CPoint Implementation
////////////////////////////////////////////////////////////////////////////////

inline CPoint::CPoint()
{
	/* random filled */
}

inline CPoint::CPoint(int initX, int initY)
{
	x = initX;
	y = initY;
}

inline CPoint::CPoint(const POINT &initPt)
{
	*(POINT*)this = initPt;
}

inline CPoint::CPoint(const SIZE &initSize)
{
	*(SIZE*)this = initSize;
}

inline CPoint::CPoint(DWORD dwPoint)
{
	x = (short)LOWORD(dwPoint);
	y = (short)HIWORD(dwPoint);
}

inline void CPoint::Offset(int xOffset, int yOffset)
{
	x += xOffset;
	y += yOffset;
}

inline void CPoint::Offset(CPoint point)
{
	x += point.x;
	y += point.y;
}

inline void CPoint::Offset(CSize size)
{
	x += size.cx;
	y += size.cy;
}

inline bool CPoint::operator == (const CPoint &point) const
{
	return (x == point.x && y == point.y);
}

inline bool CPoint::operator != (const CPoint &point) const
{
	return (x != point.x || y != point.y);
}

inline void CPoint::operator += (CSize size)
{
	x += size.cx;
	y += size.cy;
}

inline void CPoint::operator -= (CSize size)
{
	x -= size.cx;
	y -= size.cy;
}

inline void CPoint::operator += (CPoint point)
{
	x += point.x;
	y += point.y;
}

inline void CPoint::operator -= (CPoint point)
{
	x -= point.x;
	y -= point.y;
}

inline void CPoint::SetPoint(int paramX, int paramY)
{
	x = paramX;
	y = paramY;
}

inline CPoint CPoint::operator + (CSize size) const
{
	return CPoint(x + size.cx, y + size.cy);
}

inline CPoint CPoint::operator - (CSize size) const
{
	return CPoint(x - size.cx, y - size.cy);
}

inline CPoint CPoint::operator - () const
{
	return CPoint(-x, -y);
}

inline CPoint CPoint::operator + (CPoint point) const
{
	return CPoint(x + point.x, y + point.y);
}

inline CSize CPoint::operator - (CPoint point) const
{
	return CSize(x - point.x, y - point.y);
}

inline CRect CPoint::operator + (const RECT* lpRect) const
{
	return CRect(lpRect) + *this;
}

inline CRect CPoint::operator - (const RECT* lpRect) const
{
	return CRect(lpRect) - *this;
}

////////////////////////////////////////////////////////////////////////////////
// CRect Implementation
////////////////////////////////////////////////////////////////////////////////

inline CRect::CRect()
{
	/* random filled */
}

inline CRect::CRect(int l, int t, int r, int b)
{
	left = l;
	top = t;
	right = r;
	bottom = b;
}

inline CRect::CRect(const RECT& srcRect)
{
	::CopyRect(this, &srcRect);
}

inline CRect::CRect(LPCRECT lpSrcRect)
{
	::CopyRect(this, lpSrcRect);
}

inline CRect::CRect(POINT point, SIZE size)
{
	right = (left = point.x) + size.cx;
	bottom = (top = point.y) + size.cy;
}

inline CRect::CRect(POINT topLeft, POINT bottomRight)
{
	left = topLeft.x;
	top = topLeft.y;
	right = bottomRight.x;
	bottom = bottomRight.y;
}

inline int CRect::Width() const
{
	return right - left;
}

inline int CRect::Height() const
{
	return bottom - top;
}

inline CSize CRect::Size() const
{
	return CSize(right - left, bottom - top);
}

inline CPoint& CRect::LeftTop()
{
	return *((CPoint*)this);
}

inline CPoint& CRect::RightBottom()
{
	return *((CPoint*)this + 1);
}

inline const CPoint& CRect::LeftTop() const
{
	return *((CPoint*)this);
}

inline const CPoint& CRect::RightBottom() const
{
	return *((CPoint*)this + 1);
}

inline CPoint CRect::CenterPoint() const
{
	return CPoint((left + right) >> 1, (top + bottom) >> 1);
}

inline CRect::operator LPRECT()
{
	return this;
}

inline CRect::operator LPCRECT() const
{
	return this;
}

inline bool CRect::IsRectEmpty() const
{
	return (::IsRectEmpty(this)==TRUE);
}

inline bool CRect::IsRectNull() const
{
	return (left == 0 && right == 0 && top == 0 && bottom == 0);
}

inline bool CRect::PtInRect(POINT point) const
{
	return (::PtInRect(this, point)==TRUE);
}

inline void CRect::SetRect(int x1, int y1, int x2, int y2)
{
	::SetRect(this, x1, y1, x2, y2);
}

inline void CRect::SetRect(POINT topLeft, POINT bottomRight)
{
	::SetRect(this, topLeft.x, topLeft.y, bottomRight.x, bottomRight.y);
}

inline void CRect::SetRectEmpty()
{
	::SetRectEmpty(this);
}

inline void CRect::CopyRect(LPCRECT lpSrcRect)
{
	::CopyRect(this, lpSrcRect);
}

inline bool CRect::EqualRect(LPCRECT lpRect) const
{
	return (::EqualRect(this, lpRect)==TRUE);
}

inline void CRect::InflateRect(int x, int y)
{
	::InflateRect(this, x, y);
}

inline void CRect::InflateRect(SIZE size)
{
	::InflateRect(this, size.cx, size.cy);
}

inline void CRect::DeflateRect(int x, int y)
{
	::InflateRect(this, -x, -y);
}

inline void CRect::DeflateRect(SIZE size)
{
	::InflateRect(this, -size.cx, -size.cy);
}

inline void CRect::OffsetRect(int x, int y)
{
	::OffsetRect(this, x, y);
}

inline void CRect::OffsetRect(POINT point)
{
	::OffsetRect(this, point.x, point.y);
}

inline void CRect::OffsetRect(SIZE size)
{
	::OffsetRect(this, size.cx, size.cy);
}

inline bool CRect::IntersectRect(LPCRECT lpRect1, LPCRECT lpRect2)
{
	return (::IntersectRect(this, lpRect1, lpRect2)==TRUE);
}

inline bool CRect::UnionRect(LPCRECT lpRect1, LPCRECT lpRect2)
{
	return (::UnionRect(this, lpRect1, lpRect2)==TRUE);
}

inline void CRect::operator = (const RECT& srcRect)
{
	::CopyRect(this, &srcRect);
}

inline bool CRect::operator == (const RECT& rect) const
{
	return (::EqualRect(this, &rect)==TRUE);
}

inline bool CRect::operator != (const RECT& rect) const
{
	return (::EqualRect(this, &rect)==FALSE);
}

inline void CRect::operator += (POINT point)
{
	::OffsetRect(this, point.x, point.y);
}

inline void CRect::operator += (SIZE size)
{
	::OffsetRect(this, size.cx, size.cy);
}

inline void CRect::operator += (LPCRECT lpRect)
{
	InflateRect(lpRect);
}

inline void CRect::operator -= (POINT point)
{
	::OffsetRect(this, -point.x, -point.y);
}

inline void CRect::operator -= (SIZE size)
{
	::OffsetRect(this, -size.cx, -size.cy);
}

inline void CRect::operator -= (LPCRECT lpRect)
{
	DeflateRect(lpRect);
}

inline void CRect::operator &= (const RECT& rect)
{
	::IntersectRect(this, this, &rect);
}

inline void CRect::operator |= (const RECT& rect)
{
	::UnionRect(this, this, &rect);
}

inline CRect CRect::operator + (POINT pt) const
{
	CRect rect(*this);
	::OffsetRect(&rect, pt.x, pt.y);
	return rect;
}

inline CRect CRect::operator - (POINT pt) const
{
	CRect rect(*this);
	::OffsetRect(&rect, -pt.x, -pt.y);
	return rect;
}

inline CRect CRect::operator + (SIZE size) const
{
	CRect rect(*this);
	::OffsetRect(&rect, size.cx, size.cy);
	return rect;
}

inline CRect CRect::operator - (SIZE size) const
{
	CRect rect(*this);
	::OffsetRect(&rect, -size.cx, -size.cy);
	return rect;
}

inline CRect CRect::operator + (LPCRECT lpRect) const
{
	CRect rect(this);
	rect.InflateRect(lpRect);
	return rect;
}

inline CRect CRect::operator - (LPCRECT lpRect) const
{
	CRect rect(this);
	rect.DeflateRect(lpRect);
	return rect;
}

inline CRect CRect::operator & (const RECT& rect2) const
{
	CRect rect;
	::IntersectRect(&rect, this, &rect2);
	return rect;
}

inline CRect CRect::operator | (const RECT& rect2) const
{
	CRect rect;
	::UnionRect(&rect, this, &rect2);
	return rect;
}

inline bool CRect::SubtractRect(LPCRECT lpRectSrc1, LPCRECT lpRectSrc2)
{
	return (::SubtractRect(this, lpRectSrc1, lpRectSrc2)==TRUE);
}

inline void CRect::NormalizeRect()
{
	int nTemp;
	if (left > right)
	{
		nTemp = left;
		left = right;
		right = nTemp;
	}
	if (top > bottom)
	{
		nTemp = top;
		top = bottom;
		bottom = nTemp;
	}
}

inline void CRect::MoveToX(int x)
{
	right = Width() + x;
	left = x;
}

inline void CRect::MoveToY(int y)
{
	bottom = Height() + y;
	top = y;
}

inline void CRect::MoveToXY(int x, int y)
{
	MoveToX(x);
	MoveToY(y);
}

inline void CRect::MoveToXY(POINT pt)
{
	MoveToX(pt.x);
	MoveToY(pt.y);
}

inline void CRect::InflateRect(LPCRECT lpRect)
{
	left -= lpRect->left;
	top -= lpRect->top;
	right += lpRect->right;
	bottom += lpRect->bottom;
}

inline void CRect::InflateRect(int l, int t, int r, int b)
{
	left -= l;
	top -= t;
	right += r;
	bottom += b;
}

inline void CRect::DeflateRect(LPCRECT lpRect)
{
	left += lpRect->left;
	top += lpRect->top;
	right -= lpRect->right;
	bottom -= lpRect->bottom;
}

inline void CRect::DeflateRect(int l, int t, int r, int b)
{
	left += l;
	top += t;
	right -= r;
	bottom -= b;
}

inline CRect CRect::MulDiv(int nMultiplier, int nDivisor) const
{
	return CRect
		(
		::MulDiv(left, nMultiplier, nDivisor),
		::MulDiv(top, nMultiplier, nDivisor),
		::MulDiv(right, nMultiplier, nDivisor),
		::MulDiv(bottom, nMultiplier, nDivisor)
		);
}

////////////////////////////////////////////////////////////////////////////////

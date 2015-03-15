////////////////////////////////////////////////////////////////////////////////
#pragma once

////////////////////////////////////////////////////////////////////////////////

class CTreeListDC : public CDC
{
public:
  CTreeListDC( CDC* pDC );
  virtual ~CTreeListDC();
  
  CDC* operator->()  {  return ( CDC* )this;  }
  operator CDC*()    {  return ( CDC* )this;  }

private:
    CBitmap  m_Bitmap;      // offscreen
    CBitmap* m_pOldBitmap;    // onscreen
    CDC*     m_pDC;        // render device
    CRect    m_rcRect;      // render rect
    BOOL     m_bPrint;
};

////////////////////////////////////////////////////////////////////////////////

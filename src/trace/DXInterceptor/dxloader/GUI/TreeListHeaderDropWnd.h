////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define TLHDROPWND_CLASSNAME _T("TreeListCtrl.TreeListHeaderDropWnd")

////////////////////////////////////////////////////////////////////////////////

class CTreeListHeaderCtrl;
class CTreeListCtrl;

class CTreeListHeaderDropWnd : public CWnd
{
public:
  
  CTreeListHeaderDropWnd();
  virtual ~CTreeListHeaderDropWnd();
  
  BOOL Create( CTreeListHeaderCtrl* pTreeListHeaderCtrl );

  BOOL Show( CPoint pt );
  BOOL Hide();

protected:

  CTreeListHeaderCtrl* m_pTreeListHeaderCtrl;
  int m_iCol;
  
  BOOL DrawWnd( CDC* pDC );
  
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();

  DECLARE_MESSAGE_MAP()

private:

  CRgn      m_rgn;
  COLORREF  m_cr3DFace;
  COLORREF  m_cr3DLight;
  COLORREF  m_cr3DShadow;
  COLORREF  m_crText;
  BOOL      m_bLayeredWindows; // has a layered window

};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define TLCDROPWND_CLASSNAME _T("TreeListCtrl.TreeListColumnDropWnd")

////////////////////////////////////////////////////////////////////////////////

class CTreeListCtrl;

class CTreeListColumnDropWnd : public CWnd
{
public:
  
  CTreeListColumnDropWnd();
  virtual ~CTreeListColumnDropWnd();
  
  BOOL Create( CTreeListCtrl* pTreeListCtrl );

  BOOL Show( CPoint pt );
  BOOL Hide();

protected:

  CTreeListCtrl* m_pTreeListCtrl;
  
  BOOL  DrawWnd( CDC* pDC );
  
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();

  DECLARE_MESSAGE_MAP()

private:

  CRgn m_rgn;
  BOOL m_bLayeredWindows; // has a layered window

};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define TLHDRAGWND_CLASSNAME _T("TreeListCtrl.TreeListHeaderDragWnd")

////////////////////////////////////////////////////////////////////////////////

class CTreeListHeaderCtrl;
class CTreeListCtrl;

class CTreeListHeaderDragWnd : public CWnd
{
public:
  
  CTreeListHeaderDragWnd();
  virtual ~CTreeListHeaderDragWnd();
  
  BOOL Create(CRect rcHeader, CTreeListHeaderCtrl* pTreeListHeaderCtrl, int iCol );

  BOOL Show();
  BOOL Hide();

protected:
  
  CTreeListHeaderCtrl* m_pTreeListHeaderCtrl;
  int m_iCol;

  BOOL DrawWnd( CDC* pDC );

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();

  DECLARE_MESSAGE_MAP()

private:

  COLORREF  m_cr3DFace;
  COLORREF  m_cr3DLight;
  COLORREF  m_cr3DShadow;
  COLORREF  m_crText;
  BOOL      m_bLayeredWindows; // has a layered window

};

////////////////////////////////////////////////////////////////////////////////

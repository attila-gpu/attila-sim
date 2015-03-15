////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define TLCDRAGWND_CLASSNAME _T("TreeListCtrl.TreeListColumnDragWnd")

////////////////////////////////////////////////////////////////////////////////

class CTreeListCtrl;

class CTreeListColumnDragWnd : public CWnd
{
public:

  CTreeListColumnDragWnd();
  virtual ~CTreeListColumnDragWnd();
  
  BOOL Create( CRect rcWindow, CTreeListCtrl* pTreeListCtrl );
  
  BOOL Show();
  BOOL Hide();

  BOOL HasLayeredWindow() {return m_bLayeredWindows;}

protected:

  CTreeListCtrl* m_pTreeListCtrl;
  
  void  OnPaintBMP();
  BOOL  DrawWnd( CDC* pDC );
  BOOL  DrawDragWnd( CDC* pDC );
  void  DrawItem( CDC* pDC, CRect rcItem, CTreeListItem* pItem );
  void  DrawGrid( CDC* pDC, CRect rcItem );
  void  DrawItemBkgnd( CDC* pDC, CRect rcItem, CTreeListItem* pItem );
  void  DrawItemExclude( CDC* pDC, CRect rcItem, CTreeListItem* pItem );
  void  DrawItemTree( CDC* pDC, CRect rcColumn, CTreeListItem* pItem, int iCol );
  void  DrawItemMain( CDC* pDC, CRect rcColumn, CTreeListItem* pItem, int iCol );
  void  DrawItemList( CDC* pDC, CRect rcColumn, CTreeListItem* pItem, int iCol );
  void  DrawItemTreeText( CDC* pDC, CRect rcBkgnd, CTreeListItem* pItem, int iCol );
  void  DrawItemListText( CDC* pDC, CRect rcBkgnd, CTreeListItem* pItem, int iCol );
  void  DrawItemText( CDC* pDC, CRect rcBkgnd, CRect rcText, CTreeListItem* pItem, int iCol );
  
  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
  
  DECLARE_MESSAGE_MAP()

private:

  CDC       m_dcMem;
  COLORREF* m_pDIB;
  CBitmap*  m_pBitmap;
  CBitmap*  m_pBitmap2;
  BOOL      m_bLayeredWindows; // has a layered window

};

////////////////////////////////////////////////////////////////////////////////

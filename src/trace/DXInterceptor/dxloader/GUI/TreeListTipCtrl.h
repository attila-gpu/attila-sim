////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define TREELISTTIPCTRL_CLASSNAME _T("TreeListCtrl.TreeListTipCtrl")
#define WS_EX_LAYERED 0x00080000
#define ULW_COLORKEY  0x00000001
#define ULW_ALPHA     0x00000002

typedef BOOL (WINAPI *lpfnUpdateLayeredWindow)(  HWND hWnd, HDC hdcDst, POINT *pptDst, 
                        SIZE *psize,HDC hdcSrc, POINT *pptSrc, 
                        COLORREF crKey, BLENDFUNCTION *pblend, DWORD dwFlags );
typedef BOOL (WINAPI *lpfnSetLayeredWindowAttributes)( HWND hwnd, COLORREF crKey, BYTE xAlpha, DWORD dwFlags );


BOOL InitLayeredWindows();

class CTreeListCtrl;

class CTreeListTipCtrl : public CWnd
{
// Construction
public:
  CTreeListTipCtrl();
  virtual ~CTreeListTipCtrl();
  BOOL Create( CTreeListCtrl* pTreeListCtrl );

// Members
protected:
  DWORD      m_dwFadeStep;      // default   5 ( 0.05s )
  DWORD      m_dwFadeInTime;      // default 500 ( 0.50s )
  DWORD      m_dwFadeOutTime;    // default 200 ( 0.20s )

  BOOL      m_bLayeredWindows;    // has a layered window
  int        m_nAlpha;        // alpha

  CRect      m_rcItem;        // rect of item
  CRect      m_rcTip;        // rect of tip

  CString      m_strText;        // text of tip

  CFont      m_Font;          // font of tip
  UINT      m_uFormat;        // draw format

// Attributes
public:
  CTreeListCtrl*    m_pTreeListCtrl;  // parent CTreeListCtrl

// Operations
public:
  BOOL  Show( CRect rcText, LPCTSTR lpszText, UINT uFormat );
  void  Hide();
  void  SetFont();
  CFont*  GetFont();

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTreeListTipCtrl)
  public:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

// Implementation
public:

  // Generated message map functions
protected:
  //{{AFX_MSG(CTreeListTipCtrl)
  afx_msg void OnPaint();
  afx_msg void OnTimer(UINT nIDEvent);
  afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
  //}}AFX_MSG
  DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////////

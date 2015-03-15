////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class CTreeListCtrl;

class CTreeListStaticCtrl : public CStatic
{
// Construction
public:
  CTreeListStaticCtrl();

// Members
  CTreeListCtrl*  m_pTreeListCtrl;

// Attributes
public:

// Operations
public:

private:
  BOOL PreTranslateKeyDownMessage( MSG* pMsg );

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTreeListStaticCtrl)
  public:
  virtual BOOL PreTranslateMessage(MSG* pMsg);
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CTreeListStaticCtrl();

  // Generated message map functions
protected:
  //{{AFX_MSG(CTreeListStaticCtrl)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg void OnPaint();
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////////

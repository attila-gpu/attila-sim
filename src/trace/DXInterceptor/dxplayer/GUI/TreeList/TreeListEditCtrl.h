////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class CTreeListItem;

class CTreeListEditCtrl : public CEdit
{
// Construction
public:
  CTreeListEditCtrl();

// Members
  CTreeListCtrl*  m_pTreeListCtrl;

// Attributes
public:

// Operations
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTreeListEditCtrl)
  public:
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CTreeListEditCtrl();

  // Generated message map functions
protected:
  //{{AFX_MSG(CTreeListEditCtrl)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class CTreeListComboCtrl : public CComboBox
{
// Construction
public:
  CTreeListComboCtrl();

// Members
  CTreeListCtrl*  m_pTreeListCtrl;

// Attributes
public:

// Operations
public:

// Overrides
  // ClassWizard generated virtual function overrides
  //{{AFX_VIRTUAL(CTreeListComboCtrl)
  //}}AFX_VIRTUAL

// Implementation
public:
  virtual ~CTreeListComboCtrl();

  // Generated message map functions
protected:
  //{{AFX_MSG(CTreeListComboCtrl)
  afx_msg void OnKillFocus(CWnd* pNewWnd);
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  //}}AFX_MSG

  DECLARE_MESSAGE_MAP()
};

////////////////////////////////////////////////////////////////////////////////

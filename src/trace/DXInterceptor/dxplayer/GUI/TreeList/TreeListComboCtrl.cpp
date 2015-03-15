////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "TreeListDC.h"
#include "TreeListHeaderCtrl.h"
#include "TreeListTipCtrl.h"
#include "TreeListStaticCtrl.h"
#include "TreeListEditCtrl.h"
#include "TreeListComboCtrl.h"
#include "TreeListCtrl.h"

////////////////////////////////////////////////////////////////////////////////

CTreeListComboCtrl::CTreeListComboCtrl() :
m_pTreeListCtrl( NULL )
{
}

////////////////////////////////////////////////////////////////////////////////

CTreeListComboCtrl::~CTreeListComboCtrl()
{
}

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(CTreeListComboCtrl, CComboBox)
  //{{AFX_MSG_MAP(CTreeListComboCtrl)
  ON_WM_KILLFOCUS()
  ON_WM_CREATE()
  //}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////

int CTreeListComboCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct) 
{
  if (CComboBox::OnCreate(lpCreateStruct) == -1)
    return -1;

  m_pTreeListCtrl = (CTreeListCtrl*)GetParent();

  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void CTreeListComboCtrl::OnKillFocus(CWnd* pNewWnd) 
{
  CComboBox::OnKillFocus(pNewWnd);

  m_pTreeListCtrl->SetCtrlFocus( pNewWnd, FALSE );
}

////////////////////////////////////////////////////////////////////////////////

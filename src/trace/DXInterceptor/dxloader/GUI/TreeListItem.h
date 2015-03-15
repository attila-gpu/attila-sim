////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#define TLIS_SELECTED       0x00000001    // selected
#define TLIS_EXPANDED       0x00000002    // expanded
#define TLIS_EXPANDEDONCE   0x00000004    // expanded once

#define TLIS_SHOWCHECKBOX   0x00000010    // show check box
#define TLIS_SHOWLOCKBOX    0x00000020    // show lock box
#define TLIS_SHOWTREEIMAGE  0x00000040    // show image

#define TLIS_SHOWBKCOLOR    0x00000100    // owner background color
#define TLIS_SHOWTEXTCOLOR  0x00000200    // owner text color

#define TLIS_CHECKED        0x00010000    // [V]
#define TLIS_CHECKED_NULL   0x00020000    // [ ]

#define TLIS_CHECKEDPART    0x00030000    // [?]
#define TLIS_CHECKEDMASK    0x00030000    // TLI_CHECKED | TLI_CHECKED_NULL

#define TLIS_LOCKED         0x01000000    // [L]
#define TLIS_LOCKED_NULL    0x02000000    // [ ]

#define TLIS_LOCKEDPART     0x03000000    // [?]
#define TLIS_LOCKEDMASK     0x03000000    // TLI_LOCKED | TLI_LOCKED_NULL

#define TLIS_ALLSTATE       0xFFFFFFFF

class CTreeListItem
{
public:
  CTreeListItem( int nSubItem );
  virtual ~CTreeListItem();

// Members
public:
  CTreeListItem* m_pParent;
  CTreeListItem* m_pChild;
  CTreeListItem* m_pPrev;
  CTreeListItem* m_pNext;
  int            m_nLevel;
  int            m_nChild;
  int            m_nVisibleChild;

  DWORD          m_dwState;
  DWORD          m_dwData;

  int            m_nImage;
  int            m_nSelectedImage;
  int            m_nExpandImage;
  int            m_nExpandSelectedImage;

//  CImageList*    m_pImageList;

// MODIF_KOCh 060120
  CStringArray   m_arSubItems;
//  CPtrArray      m_arSubItems;
// MODIF_KOCh 060120

// Oprations
public:
  void    Expand();
  void    Collapse();

  DWORD   GetState( DWORD dwStateMask = TLIS_ALLSTATE );
  void    SetState( DWORD dwAddStatus, DWORD dwRemoveStatus = 0 );

  void    GetImage( int& nImage, int& nExpandImage, int& nSelectedImage, int& nExpandSelectedImage );
  void    SetImage( int nImage, int nExpandImage, int nSelectedImage, int nExpandSelectedImage );

  LPCTSTR GetText( int nIndex = 0 );
  BOOL    SetText( LPCTSTR lpszItem, int nIndex = 0 );

  DWORD   GetData();
  void    SetData( DWORD dwData );

  BOOL    IsShowTreeImage();
  void    ShowTreeImage( BOOL bShow );

  BOOL    IsShowCheckBox();
  void    ShowCheckBox( BOOL bShow );

  BOOL    IsShowLockBox();
  void    ShowLockBox( BOOL bShow );

  int     GetSelected();
  void    SetSelected( BOOL bSelected = TRUE );

  int     GetCheck();
  void    SetCheck( BOOL bCheck = TRUE );

  int     GetLock();
  void    SetLock( BOOL bLock = TRUE );

};

////////////////////////////////////////////////////////////////////////////////

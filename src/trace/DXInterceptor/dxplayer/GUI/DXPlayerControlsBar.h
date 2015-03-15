////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "CLabel.h"
#include "ThemeHelperST.h"

////////////////////////////////////////////////////////////////////////////////

#define ID_BASE    1000
#define ID_BASEMAX ID_BASE+20

#define ID_PLAYPAUSE  ID_BASE+0
#define ID_STOP       ID_BASE+1
#define ID_PLAYSTEP   ID_BASE+2
#define ID_SCREENSHOT ID_BASE+3

////////////////////////////////////////////////////////////////////////////////

class DXPlayerControlsBar : public CControlBar
{
public:
  
  DXPlayerControlsBar();
  virtual ~DXPlayerControlsBar();

  void SetFrameLabelCaption(const CString& cadena);
  
  virtual BOOL Create(CWnd* pParentWnd, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, UINT nID = AFX_IDW_STATUS_BAR);
  virtual BOOL CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle = 0, DWORD dwStyle = WS_CHILD | WS_VISIBLE | CBRS_BOTTOM, UINT nID = AFX_IDW_STATUS_BAR);
  
  virtual CSize CalcFixedLayout(BOOL bStretch, BOOL bHorz);
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void CalcInsideRect(CRect& rect, BOOL bHorz) const;
  virtual void OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler);

protected:

  CObArray m_buttons;
  ThemeHelperST m_theme;
  CFont* m_font;

  Gdiplus::Bitmap* m_bitmapControls;
  Gdiplus::Bitmap* m_bitmapPlay;
  Gdiplus::Bitmap* m_bitmapPause;
  Gdiplus::Bitmap* m_bitmapStop;
  Gdiplus::Bitmap* m_bitmapPlayStep;
  Gdiplus::Bitmap* m_bitmapScreenshot;
  CLabel* m_lblFrames;
  
  void CreateButtons();
  void AddButton(UINT nID, const CRect& rectButton, HICON icon, LPCTSTR tooltipText);
  void AddFrameLabel(const CRect& rectLabel);
  void CreateButtonIcons();
  void CreateButtonIconTransparent(Gdiplus::Bitmap* bitmap);
  
  DECLARE_DYNAMIC(DXPlayerControlsBar)
  afx_msg LRESULT OnNcHitTest(CPoint);
  afx_msg void OnNcCalcSize(BOOL, NCCALCSIZE_PARAMS*);
  afx_msg void OnNcPaint();
  afx_msg void OnPaint();
  afx_msg void OnSize(UINT nType, int cx, int cy);
  afx_msg void OnWindowPosChanging(LPWINDOWPOS);
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

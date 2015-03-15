////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXPlayer.h"
#include "DXPlayerControlsBar.h"
#include "XPStyleButtonST.h"

using namespace Gdiplus;

////////////////////////////////////////////////////////////////////////////////

#define SAFE_DELETE(elem) if (elem) {delete elem; elem=NULL;}

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNAMIC(DXPlayerControlsBar, CControlBar)

BEGIN_MESSAGE_MAP(DXPlayerControlsBar, CControlBar)
  ON_WM_NCHITTEST()
  ON_WM_NCCALCSIZE()
  ON_WM_NCPAINT()
  ON_WM_PAINT()
  ON_WM_SIZE()
  ON_WM_WINDOWPOSCHANGING()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayerControlsBar::DXPlayerControlsBar()
{
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerControlsBar::~DXPlayerControlsBar()
{
  for (int i = 0; i < m_buttons.GetSize(); i++)
  {
    CXPStyleButtonST *pButton = (CXPStyleButtonST*) m_buttons.GetAt(i);
    if (!pButton)
    {
      continue;
    }
    delete pButton;
  }
  m_buttons.RemoveAll();

  SAFE_DELETE(m_bitmapControls);
  SAFE_DELETE(m_bitmapPlay);
  SAFE_DELETE(m_bitmapPause);
  SAFE_DELETE(m_bitmapStop);
  SAFE_DELETE(m_bitmapPlayStep);
  SAFE_DELETE(m_bitmapScreenshot);
  SAFE_DELETE(m_lblFrames);
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerControlsBar::Create(CWnd* pParentWnd, DWORD dwStyle, UINT nID)
{
  if (CreateEx(pParentWnd, 0, dwStyle, nID))
  {
    CreateButtonIcons();
    CreateButtons();
    return TRUE;
  }
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerControlsBar::CreateEx(CWnd* pParentWnd, DWORD dwCtrlStyle, DWORD dwStyle, UINT nID)
{
  ASSERT_VALID(pParentWnd);

  m_dwStyle = (dwStyle & CBRS_ALL);

  dwStyle &= ~CBRS_ALL;
  dwStyle |= CCS_NOPARENTALIGN|CCS_NOMOVEY|CCS_NODIVIDER|CCS_NORESIZE;
  if (pParentWnd->GetStyle() & WS_THICKFRAME)
  {
    dwStyle |= SBARS_SIZEGRIP;
  }
  dwStyle |= dwCtrlStyle;

  CRect rect;
  rect.SetRectEmpty();
  return CWnd::Create(STATUSCLASSNAME, NULL, dwStyle, rect, pParentWnd, nID);
}

////////////////////////////////////////////////////////////////////////////////

CSize DXPlayerControlsBar::CalcFixedLayout(BOOL, BOOL bHorz)
{
  return CSize(32767, 40);
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerControlsBar::PreCreateWindow(CREATESTRUCT& cs)
{
  if ((m_dwStyle & (CBRS_ALIGN_ANY|CBRS_BORDER_ANY)) == CBRS_BOTTOM)
  {
    m_dwStyle &= ~(CBRS_BORDER_ANY|CBRS_BORDER_3D);
  }
  return CControlBar::PreCreateWindow(cs);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::CalcInsideRect(CRect& rect, BOOL bHorz) const
{
  CControlBar::CalcInsideRect(rect, bHorz);

  if ((GetStyle() & SBARS_SIZEGRIP) && !::IsZoomed(::GetParent(m_hWnd)))
  {
    int rgBorders[3];
    DXPlayerControlsBar* pBar = (DXPlayerControlsBar*) this;
    pBar->DefWindowProc(SB_GETBORDERS, 0, (LPARAM) &rgBorders);
    rect.right -= rgBorders[0] + ::GetSystemMetrics(SM_CXVSCROLL) + ::GetSystemMetrics(SM_CXBORDER) * 2;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::OnUpdateCmdUI(CFrameWnd* pTarget, BOOL bDisableIfNoHndler)
{
}

////////////////////////////////////////////////////////////////////////////////

LRESULT DXPlayerControlsBar::OnNcHitTest(CPoint)
{
  UINT nResult = (UINT) Default();
  if (nResult == HTBOTTOMRIGHT)
    return HTBOTTOMRIGHT;
  else
    return HTCLIENT;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
  CRect rect;
  rect.SetRectEmpty();
  CControlBar::CalcInsideRect(rect, TRUE);
  lpncsp->rgrc[0].top -= 2;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::OnNcPaint()
{
  EraseNonClient();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::OnPaint()
{
  Default();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::OnSize(UINT nType, int cx, int cy)
{
  CControlBar::OnSize(nType, cx, cy);

  if (cx > 0)
  {
    CRect rect;
    rect.SetRectEmpty();
    m_lblFrames->GetWindowRect(&rect);
    rect.MoveToXY(cx - rect.Width() - 8, 18);
    m_lblFrames->MoveWindow(&rect);
    if (!m_lblFrames->IsWindowVisible())
    {
      m_lblFrames->ShowWindow(SW_SHOW);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::OnWindowPosChanging(LPWINDOWPOS lpWndPos)
{
  DWORD dwStyle = m_dwStyle;
  m_dwStyle &= ~(CBRS_BORDER_ANY);
  CControlBar::OnWindowPosChanging(lpWndPos);
  m_dwStyle = dwStyle;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::CreateButtonIcons()
{
  m_bitmapControls = new Bitmap(theApp.m_hInstance, (CONST WCHAR*) MAKEINTRESOURCE(IDB_CONTROLS));
  
  m_bitmapPlay       = m_bitmapControls->Clone(0, 0, 16, 16, PixelFormat32bppARGB);
  m_bitmapPause      = m_bitmapControls->Clone(16, 0, 16, 16, PixelFormat32bppARGB);
  m_bitmapStop       = m_bitmapControls->Clone(32, 0, 16, 16, PixelFormat32bppARGB);
  m_bitmapPlayStep   = m_bitmapControls->Clone(48, 0, 16, 16, PixelFormat32bppARGB);
  m_bitmapScreenshot = m_bitmapControls->Clone(64, 0, 16, 16, PixelFormat32bppARGB);
  
  CreateButtonIconTransparent(m_bitmapPlay);
  CreateButtonIconTransparent(m_bitmapPause);
  CreateButtonIconTransparent(m_bitmapStop);
  CreateButtonIconTransparent(m_bitmapPlayStep);
  CreateButtonIconTransparent(m_bitmapScreenshot);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::CreateButtonIconTransparent(Bitmap* bitmap)
{
  BitmapData bitmapData;
  Rect rect(0, 0, 16, 16);
  
  bitmap->LockBits(&rect, ImageLockModeWrite, PixelFormat32bppARGB, &bitmapData);

  UINT* pixels = (UINT*) bitmapData.Scan0;
  for (UINT i=0; i < 16*16; ++i)
  {
    if (*pixels == 0xFFFF00FF)
    {
      *pixels &= 0x00FFFFFF;
    }
    pixels++;
  }

  bitmap->UnlockBits(&bitmapData);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::CreateButtons()
{
  HICON icon;
  CRect rect;

  rect.SetRect(0, 0, 32, 32);
  rect.MoveToXY(4+(32+4)*0, 6);
  m_bitmapPlay->GetHICON(&icon);
  AddButton(ID_PLAYPAUSE, rect, icon, "Play/Pause");

  rect.SetRect(0, 0, 32, 32);
  rect.MoveToXY(4+(32+4)*1, 6);
  m_bitmapStop->GetHICON(&icon);
  AddButton(ID_STOP, rect, icon, "Stop");

  rect.SetRect(0, 0, 32, 32);
  rect.MoveToXY(4+(32+4)*2, 6);
  m_bitmapPlayStep->GetHICON(&icon);
  AddButton(ID_PLAYSTEP, rect, icon, "Step");

  rect.SetRect(0, 0, 32, 32);
  rect.MoveToXY(4+(32+4)*3, 6);
  m_bitmapScreenshot->GetHICON(&icon);
  AddButton(ID_SCREENSHOT, rect, icon, "Take a screenshot");

  rect.SetRect(0, 0, 150, 16);
  AddFrameLabel(rect);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::AddButton(UINT nID, const CRect& rectButton, HICON icon, LPCTSTR tooltipText)
{
  CXPStyleButtonST* pButton = new CXPStyleButtonST;

  VERIFY(pButton->Create("", WS_CHILD | WS_VISIBLE | WS_GROUP | WS_TABSTOP, rectButton, this, nID));

  pButton->SetIcon(icon);
  pButton->SetThemeHelper(&m_theme);
  pButton->SetTooltipText(tooltipText);

  m_buttons.Add(pButton);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::AddFrameLabel(const CRect& rectLabel)
{
  m_lblFrames = new CLabel;
  m_lblFrames->Create("", WS_CHILD | SS_RIGHT, rectLabel, this);
  m_lblFrames->SetFontSize(12);
  m_lblFrames->SetFontName("Ms Sans Serif");
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerControlsBar::SetFrameLabelCaption(const CString& cadena)
{
  m_lblFrames->SetText(cadena);
}

////////////////////////////////////////////////////////////////////////////////
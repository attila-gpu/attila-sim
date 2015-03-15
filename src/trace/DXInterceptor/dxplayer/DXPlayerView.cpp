////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXPlayer.h"
#include "DXPlayerView.h"

using namespace Gdiplus;

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(DXPlayerView, CView)

BEGIN_MESSAGE_MAP(DXPlayerView, CView)
  ON_WM_ERASEBKGND()
  ON_WM_PAINT()
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayerView::DXPlayerView() :
m_hwnd(NULL),
m_enableOwnerPaint(true)
{
  m_logo.Load(IDB_LOGO, _T("PNG"));
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerView::~DXPlayerView()
{
}

////////////////////////////////////////////////////////////////////////////////

#ifdef _DEBUG
DXPlayerDocument* DXPlayerView::GetDocument() const
{
  ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(DXPlayerDocument)));
  return (DXPlayerDocument*) m_pDocument;
}
#endif // ifdef _DEBUG

////////////////////////////////////////////////////////////////////////////////

HWND DXPlayerView::GetHWND() const
{
  return m_hwnd;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerView::GetRenderRect(CRect* rect)
{
  rect->SetRectEmpty();
  GetClientRect(rect);
  ClientToScreen(rect);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerView::EnableOwnerPaint(bool enable)
{
  m_enableOwnerPaint = enable;
  if (m_enableOwnerPaint)
  {
    Invalidate();
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerView::PreCreateWindow(CREATESTRUCT& cs) 
{
  cs.style |= WS_VISIBLE;
  cs.style &= ~WS_BORDER;
  cs.dwExStyle |= WS_EX_TOPMOST;

  if (!CView::PreCreateWindow(cs))
  {
    return FALSE;
  }

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerView::OnInitialUpdate()
{
  CView::OnInitialUpdate();
  
  m_hwnd = GetSafeHwnd();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerView::OnDraw(CDC* pDC)
{
  UNUSED_ALWAYS(pDC);
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerView::OnEraseBkgnd(CDC* pDC)
{
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerView::OnPaint()
{
  if (m_enableOwnerPaint)
  {
    CPaintDC dc(this);
    Graphics graphics(dc.m_hDC);
    
    CRect rectClient;
    GetClientRect(&rectClient);
    Rect rect(0, 0, rectClient.right, rectClient.bottom);

    SolidBrush white(Color::White);
    graphics.FillRectangle(&white, rect);
    
    Rect rectDraw;
    if (rect.Width < (INT) m_logo->GetWidth() || rect.Height < (INT) m_logo->GetHeight())
    {
      rectDraw.X = 0;
      rectDraw.Y = 0;
      rectDraw.Width = rect.Width;
      rectDraw.Height = rect.Height;
    }
    else
    {
      rectDraw.X = (rect.Width - m_logo->GetWidth()) >> 1;
      rectDraw.Y = (rect.Height - m_logo->GetHeight()) >> 1;
      rectDraw.Width = m_logo->GetWidth();
      rectDraw.Height = m_logo->GetHeight();
    }
    
    graphics.DrawImage(m_logo, rectDraw.X, rectDraw.Y, rectDraw.Width, rectDraw.Height);
  }
  else
  {
    CPaintDC dc(this);
    Graphics graphics(dc.m_hDC);
  }
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXPlayer.h"
#include "DXPlayerMainFrame.h"

////////////////////////////////////////////////////////////////////////////////

IMPLEMENT_DYNCREATE(DXPlayerMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(DXPlayerMainFrame, CFrameWnd)
  ON_WM_CREATE()
  ON_MESSAGE(WM_SETMESSAGESTRING, OnSetMessageString)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayerMainFrame::DXPlayerMainFrame()
{
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerMainFrame::~DXPlayerMainFrame()
{
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerMainFrame::SetDimensions(UINT width, UINT height)
{
  CRect rectDesktop;
  ::GetWindowRect(::GetDesktopWindow(), &rectDesktop);
  
  if (rectDesktop.Width() <= (int) width || rectDesktop.Height() <= (int) height)
  {
    return false;
  }
  
  if (GetActiveView())
  {
    CRect rectCurrentView;
    GetActiveView()->GetWindowRect(rectCurrentView);

    CRect rectFrame;
    GetClientRect(rectFrame);
    rectFrame.bottom += GetSystemMetrics(SM_CYMENU);

    CRect rectView;
    rectView.left = 0;
    rectView.top = 0;
    rectView.right = width + rectFrame.Width() - rectCurrentView.Width();
    rectView.bottom = height + rectFrame.Height() - rectCurrentView.Height();
    rectView.MoveToXY((rectDesktop.Width() - width) >> 1, (rectDesktop.Height() - height) >> 1);

    CalcWindowRect(&rectView, CWnd::adjustBorder);

    ShowWindow(SW_HIDE);
    MoveWindow(&rectView);
    ShowWindow(SW_SHOW);
  }
  else
  {
    CRect rectView;
    rectView.left = 0;
    rectView.top = 0;
    rectView.right = width;
    rectView.bottom = height + GetSystemMetrics(SM_CYMENU) + 40;
    rectView.MoveToXY((rectDesktop.Width() - width) >> 1, (rectDesktop.Height() - height) >> 1);

    CalcWindowRect(&rectView, CWnd::adjustBorder);

    ShowWindow(SW_HIDE);
    MoveWindow(&rectView);
    ShowWindow(SW_SHOW);
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerMainFrame::SetFrameNumber(UINT currentFrame, UINT totalFrames)
{
  CString cadena;
  cadena.Format("Frame %u/%u", currentFrame, totalFrames);
  m_wndPlayerBar.SetFrameLabelCaption(cadena);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerMainFrame::SetLabelText(const CString& cadena)
{
  m_wndPlayerBar.SetFrameLabelCaption(cadena);
}

////////////////////////////////////////////////////////////////////////////////

int DXPlayerMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
  {
		return -1;
  }

  SetDimensions(640, 480);
  
  if (!m_wndPlayerBar.Create(this))
	{
	  return -1;
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
  cs.style &= ~WS_MAXIMIZEBOX;
  cs.style &= ~WS_THICKFRAME;
  
  if (!CFrameWnd::PreCreateWindow(cs))
  {
		return FALSE;
  }

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext)
{
  if (!CFrameWnd::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))
  {
    return false;
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

LRESULT DXPlayerMainFrame::OnSetMessageString(WPARAM wParam, LPARAM lParam)
{
  UINT nIDLast = m_nIDLastMessage;
  m_nIDLastMessage = (UINT) wParam;
  m_nIDTracking = (UINT) wParam;
  return nIDLast;
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "GdiPlusBitmapResource.h"
#include "DXPlayerDocument.h"

////////////////////////////////////////////////////////////////////////////////

class DXPlayerView : public CView
{
protected:
  
  DXPlayerView();
  DECLARE_DYNCREATE(DXPlayerView)

public:
	
  virtual ~DXPlayerView();

  DXPlayerDocument* GetDocument() const;
  HWND GetHWND() const;
  void GetRenderRect(CRect* rect);
  void EnableOwnerPaint(bool enable);
  
protected:
	
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual void OnInitialUpdate();
  virtual void OnDraw(CDC* pDC);

  afx_msg BOOL OnEraseBkgnd(CDC* pDC);
  afx_msg void OnPaint();
  DECLARE_MESSAGE_MAP()

private:

  HWND m_hwnd;
  GdiPlusBitmapResource m_logo;
  bool m_enableOwnerPaint;
 
};

////////////////////////////////////////////////////////////////////////////////

#ifndef _DEBUG
inline DXPlayerDocument* DXPlayerView::GetDocument() const
{
  return reinterpret_cast<DXPlayerDocument*>(m_pDocument);
}
#endif

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

#include "DXPlayerControlsBar.h"

////////////////////////////////////////////////////////////////////////////////

class DXPlayerMainFrame : public CFrameWnd
{
public:
	
  DXPlayerMainFrame();
  virtual ~DXPlayerMainFrame();
  
  bool SetDimensions(UINT width, UINT height);
  void SetFrameNumber(UINT currentFrame, UINT totalFrames);
  void SetLabelText(const CString& cadena);
  
  virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
  virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);

protected: 
	
  DXPlayerControlsBar m_wndPlayerBar;

  DECLARE_DYNCREATE(DXPlayerMainFrame)
  afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
  afx_msg LRESULT OnSetMessageString(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()
  
};

////////////////////////////////////////////////////////////////////////////////

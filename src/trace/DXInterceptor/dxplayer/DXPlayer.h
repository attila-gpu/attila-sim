////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

//#include <vld.h>
#include "resource.h"
#include "DXPlayerOptions.h"
#include "DXPlayerMainFrame.h"
#include "DXPlayerView.h"
#include "DXPlayerDocument.h"
#include "DXPainterHeaders.h"

////////////////////////////////////////////////////////////////////////////////

class dxpainter::DXPainter;

class DXPlayer : public CWinApp
{
public:

  DXPlayer();

  HWND GetViewportHWND();
  void SetTitle(const std::string& title);
  void ShowBackgroundLogo(bool show);
  void SetViewportDimensions(UINT width, UINT height);
  void UpdatePlayStatus();

protected:

  static const CString OPTIONS_FILENAME;
  
  friend class DXPlayerTraceViewerDlg;
  friend class DXPlayerTextureViewerDlg;
  
  DXPlayerOptions m_options;
  DXPlayerMainFrame* m_frame;
  DXPlayerView* m_view;
  DXPlayerDocument* m_document;
  dxpainter::DXPainter* m_painter;
  bool m_painterActive;
  ULONG_PTR m_gdiPlusToken;
  
  CString OpenFileName(const CString& initialDirectory = "");
  void InitGdiPlus();
  void CloseGdiPlus();
  
  virtual BOOL InitInstance();
  virtual int  ExitInstance();
  virtual BOOL OnIdle(LONG lCount);

  afx_msg void OnAppExit();
  afx_msg void OnHelpAbout();
  afx_msg void OnFileOpen();
  afx_msg void OnFileProjectInformation();
  afx_msg void OnFileOptions();
  afx_msg void OnDataStatisticViewer();
  afx_msg void OnDataTraceViewer();
  afx_msg void OnDataTextureViewer();
  afx_msg void OnPlayPause();
  afx_msg void OnStop();
  afx_msg void OnStep();
  afx_msg void OnTakeScreenshot();
  DECLARE_MESSAGE_MAP()

};

////////////////////////////////////////////////////////////////////////////////

extern DXPlayer theApp;

////////////////////////////////////////////////////////////////////////////////

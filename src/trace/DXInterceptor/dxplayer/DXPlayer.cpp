////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DXPlayerMainFrame.h"
#include "DXPlayerView.h"
#include "DXPlayerDocument.h"
#include "DXPlayerControlsBar.h"
#include "DXPlayerAboutDlg.h"
#include "DXPlayerOptions.h"
#include "DXPlayerOptionsDlg.h"
#include "DXPainterHeaders.h"
#include "DXPlayerProjectInformationDlg.h"
#include "DXPlayerStatisticViewerDlg.h"
#include "DXPlayerTraceViewerDlg.h"
#include "DXPlayerTextureViewerDlg.h"
#include "DXPlayer.h"

using namespace std;
using namespace Gdiplus;
using namespace dxpainter;

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXPlayer, CWinApp)
  ON_COMMAND(ID_APP_EXIT, OnAppExit)
  ON_COMMAND(ID_APP_ABOUT, OnHelpAbout)
  ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
  ON_COMMAND(ID_FILE_PROJECTINFORMATION, OnFileProjectInformation)
  ON_COMMAND(ID_FILE_OPTIONS, OnFileOptions)
  ON_COMMAND(ID_DATA_STATISTICVIEWER, OnDataStatisticViewer)
  ON_COMMAND(ID_DATA_TRACEVIEWER, OnDataTraceViewer)
  ON_COMMAND(ID_DATA_TEXTUREVIEWER, OnDataTextureViewer)
  ON_COMMAND(ID_PLAYPAUSE, OnPlayPause)
  ON_COMMAND(ID_STOP, OnStop)
  ON_COMMAND(ID_PLAYSTEP, OnStep)
  ON_COMMAND(ID_SCREENSHOT, OnTakeScreenshot)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayer theApp;
const CString DXPlayer::OPTIONS_FILENAME = "dxplayer.config";

////////////////////////////////////////////////////////////////////////////////

DXPlayer::DXPlayer() :
m_frame(NULL),
m_view(NULL),
m_document(NULL),
m_painter(NULL),
m_painterActive(false)
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayer::InitInstance()
{
  InitGdiPlus();
  InitCommonControls();
  CWinApp::InitInstance();
  
  CSingleDocTemplate* pDocTemplate = new CSingleDocTemplate(IDR_MAINFRAME, RUNTIME_CLASS(DXPlayerDocument), RUNTIME_CLASS(DXPlayerMainFrame), RUNTIME_CLASS(DXPlayerView));
  if (pDocTemplate)
  {
    AddDocTemplate(pDocTemplate);
  }
  else
  {
    return FALSE;
  }
  
  CCommandLineInfo cmdInfo;
  ParseCommandLine(cmdInfo);
  if (!ProcessShellCommand(cmdInfo))
  {
    return FALSE;
  }
  
  m_frame = (DXPlayerMainFrame*) m_pMainWnd;
  m_view = (DXPlayerView*) m_pMainWnd->GetWindow(GW_CHILD);
  m_document = m_view->GetDocument();
  
  if (!(m_painter = DXPainterCreate()))
  {
    AfxMessageBox("ERROR: could'nt create a DXPainter instance");
    return FALSE;
  }
  
  m_options.LoadXML(OPTIONS_FILENAME.GetString());
  
  SetTitle(_T("(nothing opened)"));
  UpdatePlayStatus();

  m_pMainWnd->ShowWindow(SW_SHOW);
  m_pMainWnd->UpdateWindow();
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

int DXPlayer::ExitInstance()
{
  if (m_painter)
  {
    delete m_painter;
    m_painter = NULL;
    m_painterActive = false;
  }

  CloseAllDocuments(FALSE);  
  CloseGdiPlus();
  return CWinApp::ExitInstance();
}

////////////////////////////////////////////////////////////////////////////////

HWND DXPlayer::GetViewportHWND()
{
  return m_view->GetHWND();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::SetTitle(const string& title)
{
  m_document->SetTitle(title.c_str());
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::ShowBackgroundLogo(bool show)
{
  m_view->EnableOwnerPaint(show);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::SetViewportDimensions(UINT width, UINT height)
{
  m_frame->SetDimensions(width, height);
  ShowBackgroundLogo(false);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::UpdatePlayStatus()
{
  m_frame->SetFrameNumber(m_painter->GetCurrentFrameCount(), m_painter->GetTotalFrameCount());
}

////////////////////////////////////////////////////////////////////////////////

CString DXPlayer::OpenFileName(const CString& initialDirectory)
{
  TCHAR tsFilters[]= TEXT("DXInterceptor Run Files (*.DXIntRun)\0*.DXIntRun\0\0");
  TCHAR tsFile[MAX_PATH] = "";
  
  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = m_frame->m_hWnd;
  ofn.lpstrFilter = tsFilters;
  ofn.lpstrInitialDir = initialDirectory;
  ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = tsFile;
  ofn.nMaxFile = sizeof(tsFile) / sizeof(TCHAR);

  if (GetOpenFileName(&ofn))
  {
    return CString(tsFile);
  }

  return "";
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::InitGdiPlus()
{
  GdiplusStartupInput gdiPlusStartupInput;
  GdiplusStartup(&m_gdiPlusToken, &gdiPlusStartupInput, NULL);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::CloseGdiPlus()
{
  GdiplusShutdown(m_gdiPlusToken);
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayer::OnIdle(LONG lCount)
{
  CWinApp::OnIdle(lCount);

  if (m_painterActive)
  {
    switch (m_painter->Paint())
    {
    case DXPainter::PR_ERROR_DIRECT3D:
    case DXPainter::PR_ERROR_PAINTER:
    case DXPainter::PR_ERROR_TRACEMANAGER:
      ShowBackgroundLogo(true);
      m_painterActive = false;
      return FALSE;
      break;

    case DXPainter::PR_OK_NOMOREFRAMES:
      UpdatePlayStatus();
      ShowBackgroundLogo(true);
      m_painterActive = false;
      return FALSE;
      break;
    
    case DXPainter::PR_OK_PAUSE:
    case DXPainter::PR_OK_STOP:
    case DXPainter::PR_OK_PLAYSTEPPERFRAME:
      UpdatePlayStatus();
      m_painterActive = false;
      return FALSE;
      break;
    
    case DXPainter::PR_OK_PLAY:
      UpdatePlayStatus();
      return TRUE;
      break;
    }
  }

  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnAppExit()
{
  ::PostQuitMessage(0);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnHelpAbout()
{
  DXPlayerAboutDlg aboutDlg;
  aboutDlg.DoModal();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnDataStatisticViewer()
{
  if (m_painter && !m_painter->GetProjectFilePath().empty())
  {
    DXPlayerStatisticViewerDlg statisticDlg(m_painter->GetProjectFilePath());
    statisticDlg.DoModal();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnDataTraceViewer()
{
  if (m_painter && !m_painter->GetProjectFilePath().empty())
  {
    DXPlayerTraceViewerDlg traceDlg(m_painter->GetProjectFilePath().c_str());
    traceDlg.DoModal();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnDataTextureViewer()
{
  if (m_painter && !m_painter->GetProjectFilePath().empty())
  {
    DXPlayerTextureViewerDlg textureDlg(m_painter->GetProjectFilePath());
    textureDlg.DoModal();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnFileProjectInformation()
{
  if (m_painter && !m_painter->GetProjectFilePath().empty())
  {
    DXPlayerProjectInformationDlg projectInfoDlg(*m_painter);
    if (projectInfoDlg.DoModal() == IDOK)
    {
      SetTitle(m_painter->GetProjectGameName());
      UpdatePlayStatus();
      ShowBackgroundLogo(true);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnFileOptions()
{
  DXPlayerOptionsDlg optionsDlg(OPTIONS_FILENAME, m_frame);
  optionsDlg.DoModal();
  m_options.LoadXML(OPTIONS_FILENAME.GetString());
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnFileOpen()
{
  string projectName = OpenFileName();
  
  if (projectName.empty())
  {
    return;
  }
  
  DXPainter::InitParameters params;
  params.StartWindowed = true;
  params.ViewportWindowed = GetViewportHWND();
  params.ViewportFullScreen = NULL;
  params.EnableVSync = false;
  params.TraceFilename = projectName;
  
  if (m_painter->Init(&params))
  {
    SetTitle(m_painter->GetProjectGameName());
    m_painterActive = true;
  }
  else
  {
    m_painterActive = false;
  }

  UpdatePlayStatus();
  ShowBackgroundLogo(true);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnPlayPause()
{
  m_painterActive = true;
  m_painter->Play();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnStop()
{
  m_painter->Stop();
  UpdatePlayStatus();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnStep()
{
  m_painterActive = true;
  m_painter->PlayStepPerFrame();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayer::OnTakeScreenshot()
{
  string basePath = m_options.GetDestinationPath();
  
  if (basePath.empty() || !::PathFileExists(basePath.c_str()))
  {
    TCHAR currentDir[MAX_PATH];
    ::GetCurrentDirectory(MAX_PATH, currentDir);
    ::PathAddBackslash(currentDir);
    basePath = currentDir;
  }  
  
  m_painter->TakeScreenshot(basePath, (DXPainter::ScreenshotFormat) m_options.GetScreenshotFormat());
}

////////////////////////////////////////////////////////////////////////////////

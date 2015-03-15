////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DXPainterHeaders.h"
#include "DXPlayerStatisticViewerDlg.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

#define SVDLG_THREAD_ENDS WM_USER + 20 // Thread ends

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXPlayerStatisticViewerDlg, CDialog)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_SIZE()
  ON_WM_GETMINMAXINFO()
  ON_MESSAGE(SVDLG_THREAD_ENDS, UpdateWindowObjects)
  ON_MESSAGE(XG_ZOOMCHANGE, OnZoomChange)
  ON_MESSAGE(XG_PANCHANGE, OnPanChange)
  ON_MESSAGE(XG_MEASURECREATED, OnMeasureCreated)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayerStatisticViewerDlg::DXPlayerStatisticViewerDlg(const string& traceFilePath, CWnd* pParent /*=NULL*/) :
CDialog(IDD_STATISTICVIEWER, pParent),
m_traceFilePath(traceFilePath),
m_dlgCurveOptions(m_dataman),
m_dlgGraphOptions(m_dataman),
m_dlgExportOptions(m_dataman),
m_dlgOnTheFlyOptions(m_dataman),
m_loadingStatistics(false),
m_killingThread(false),
m_thread(NULL)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

  // Create the layout helper and attach it to this dialog.
  m_layoutHelper = new CLayoutHelper;
  m_layoutHelper->SetLayoutStyle(CLayoutHelper::DEFAULT_LAYOUT);
  m_layoutHelper->AttachWnd(this);
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerStatisticViewerDlg::~DXPlayerStatisticViewerDlg()
{
  m_graph.ResetAll();
  m_dataman.Clear();
  delete m_layoutHelper;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticViewerDlg::PostNcDestroy()
{
  m_layoutHelper->DetachWnd();
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticViewerDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerStatisticViewerDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  SetIcon(m_hIcon, TRUE);
  SetIcon(m_hIcon, FALSE);

  CRect rect, clientRect;
  GetClientRect(&clientRect);
  
  //////////////////////////////////////////////////////////////////////////////
  // Create the rollup control and insert pages

  rect.SetRect(clientRect.Width()-214, 4, clientRect.Width()-4, clientRect.Height()-4);
  
  m_rollup.Create(WS_CHILD | WS_VISIBLE, rect, this, 0);

  m_dlgCurveOptions.Create(MAKEINTRESOURCE(IDD_STATISTICVIEWER_ROLLUP_CURVE_OPTIONS), &m_rollup);
  m_rollup.InsertPage("Curve Options", &m_dlgCurveOptions, FALSE, 0);

  m_dlgGraphOptions.Create(MAKEINTRESOURCE(IDD_STATISTICVIEWER_ROLLUP_GRAPH_OPTIONS), &m_rollup);
  m_rollup.InsertPage("Graph Options", &m_dlgGraphOptions, FALSE, 1);

  m_dlgExportOptions.Create(MAKEINTRESOURCE(IDD_STATISTICVIEWER_ROLLUP_EXPORT_OPTIONS), &m_rollup);
  m_rollup.InsertPage("Export Options", &m_dlgExportOptions, FALSE, 2);

  m_dlgOnTheFlyOptions.Create(MAKEINTRESOURCE(IDD_STATISTICVIEWER_ROLLUP_ONTHEFLY_OPTIONS), &m_rollup);
  m_rollup.InsertPage("On the Fly Options", &m_dlgOnTheFlyOptions, FALSE, 3);
  
  //////////////////////////////////////////////////////////////////////////////
  // Create the graph control  
  
  rect.SetRect(10, 10, clientRect.Width()-224, clientRect.Height()-10);
  
  if (!::IsWindow(m_graph.m_hWnd))
  {
    m_graph.Create(_T("XGraph"), _T(""), WS_CHILD  | WS_VISIBLE | WS_BORDER, rect, this, 0);
    m_graph.SetData(NULL, 0, 0, 0, false);
    m_graph.SetCursorFlags(XGC_LEGEND | XGC_VERT | XGC_ADJUSTSMOOTH);
    m_graph.ForceSnap(0);
    m_graph.SetInteraction(true);
    m_graph.EnableWindow(true);
    m_graph.SetShowLegend(false);
    m_graph.SetInteraction(false);
    m_graph.SetInnerColor(RGB(255,255,255));
    m_graph.SetPanMode(CXGraph::BothAxis);
  }
  
  //////////////////////////////////////////////////////////////////////////////
  // Update constraints for layout
  
  CLayoutInfo layoutInfo;

  layoutInfo.Reset();
  layoutInfo.AddOption(CLayoutInfo::OT_LEFT_OFFSET,   0);
  layoutInfo.AddOption(CLayoutInfo::OT_LEFT_ANCHOR,   clientRect.Width()-204);
  layoutInfo.AddOption(CLayoutInfo::OT_TOP_OFFSET,    4);
  layoutInfo.AddOption(CLayoutInfo::OT_RIGHT_OFFSET,  4);
  layoutInfo.AddOption(CLayoutInfo::OT_BOTTOM_OFFSET, 4);
  m_layoutHelper->AddControl(&m_rollup, layoutInfo);
  
  layoutInfo.Reset();
  layoutInfo.AddOption(CLayoutInfo::OT_MIN_LEFT,      10);
  layoutInfo.AddOption(CLayoutInfo::OT_MAX_LEFT,      10);
  layoutInfo.AddOption(CLayoutInfo::OT_MIN_TOP,       10);
  layoutInfo.AddOption(CLayoutInfo::OT_MAX_TOP,       10);
  layoutInfo.AddOption(CLayoutInfo::OT_LEFT_OFFSET,   10);
  layoutInfo.AddOption(CLayoutInfo::OT_TOP_OFFSET,    10);
  layoutInfo.AddOption(CLayoutInfo::OT_RIGHT_OFFSET,  214);
  layoutInfo.AddOption(CLayoutInfo::OT_BOTTOM_OFFSET, 10);
  m_layoutHelper->AddControl(&m_graph, layoutInfo);
  
  m_layoutHelper->LayoutControls();
  
  //////////////////////////////////////////////////////////////////////////////

  this->GetWindowText(m_defaultTitle);
  m_dataman.SetNotifier(fastdelegate::MakeDelegate(this, &DXPlayerStatisticViewerDlg::ProgressUpdate));
  m_dataman.SetGraph(&m_graph);
  
  //////////////////////////////////////////////////////////////////////////////
  // Load trace statistics data in background
  
  ThreadStart();
   
  //////////////////////////////////////////////////////////////////////////////
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////
// If you add a minimize button to your dialog, you will need the code below to
// draw the icon. For MFC applications using the document/view model, this is
// automatically done for you by the framework.

void DXPlayerStatisticViewerDlg::OnPaint() 
{
  if (IsIconic())
  {
    CPaintDC dc(this);

    SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

    // Center icon in client rectangle
    int cxIcon = GetSystemMetrics(SM_CXICON);
    int cyIcon = GetSystemMetrics(SM_CYICON);
    CRect rect;
    GetClientRect(&rect);
    int x = (rect.Width() - cxIcon + 1) / 2;
    int y = (rect.Height() - cyIcon + 1) / 2;

    dc.DrawIcon(x, y, m_hIcon);
  }
  else
  {
    CDialog::OnPaint();
  }
}

////////////////////////////////////////////////////////////////////////////////

HCURSOR DXPlayerStatisticViewerDlg::OnQueryDragIcon()
{
  return (HCURSOR) m_hIcon;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticViewerDlg::OnSize(UINT nType, int cx, int cy)
{
  CDialog::OnSize(nType, cx, cy);
  
  CRect rect;
  GetClientRect(&rect);
  m_layoutHelper->SetReferenceSize(rect.Width(), rect.Height());
  
  CLayoutInfo layoutInfo;
  if (m_layoutHelper->GetLayoutInfo(&m_rollup, layoutInfo))
  {
    layoutInfo.AddOption(CLayoutInfo::OT_LEFT_ANCHOR, rect.Width()-204);
    m_layoutHelper->AddControl(&m_rollup, layoutInfo);
  }
  
  m_layoutHelper->OnSize(nType, cx, cy);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticViewerDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
  lpMMI->ptMinTrackSize.x = 500;
  lpMMI->ptMinTrackSize.y = 400;

  CDialog::OnGetMinMaxInfo(lpMMI);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticViewerDlg::OnCancel()
{
  if (ThreadEnd())
  {
    CDialog::OnCancel();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticViewerDlg::ThreadStart() 
{
  ThreadEnd();
  m_killingThread = false;
  m_loadingStatistics = true;
  m_thread = (HANDLE) _beginthreadex(NULL, 0, ThreadWork, (LPVOID) this, 0, NULL); 
}


////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticViewerDlg::ThreadEnd()
{
  if (!m_loadingStatistics)
  {
    ::CloseHandle(m_thread);
    return true;
  }

  m_killingThread = true;
  m_dataman.AbortLoading();

  for (;;)
  {
    if (::WaitForSingleObject(m_thread, 0) == WAIT_OBJECT_0)
    {
      break;
    }

    MSG msg;
    while (::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE)) 
    { 
      if (!AfxGetApp()->PumpMessage())
      {
        break;
      }
    } 
  }

  ::CloseHandle(m_thread);
  return true;
}

////////////////////////////////////////////////////////////////////////////////

unsigned __stdcall DXPlayerStatisticViewerDlg::ThreadWork(LPVOID lpParam)
{
  DXPlayerStatisticViewerDlg* dialog = (DXPlayerStatisticViewerDlg*) lpParam;

  dialog->LoadTraceStatistics();

  if (!dialog->m_killingThread)
  {
    dialog->SendMessage(SVDLG_THREAD_ENDS, ::GetCurrentThreadId(), 0);
  }

  dialog->m_loadingStatistics = false;
  dialog->m_killingThread = false;
  _endthreadex(0);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticViewerDlg::LoadTraceStatistics()
{
  m_dataman.LoadTraceStatistics(m_traceFilePath);
}

////////////////////////////////////////////////////////////////////////////////

LRESULT DXPlayerStatisticViewerDlg::UpdateWindowObjects(WPARAM wParam, LPARAM lParam)
{
  m_dlgCurveOptions.Refresh();
  m_dlgGraphOptions.Refresh();
  m_dlgExportOptions.Refresh();
  m_dlgOnTheFlyOptions.Refresh();
  
  if (m_dataman.GetStatisticsCount() > 0)
  {
    m_rollup.ExpandAllPages();
  }
  else
  {
    m_rollup.ExpandAllPages(FALSE);
    m_rollup.ExpandPage(3);
  }

  m_graph.Invalidate();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerStatisticViewerDlg::LoadTraceStatisticsOnTheFly(dxplugin::DXIntPluginManager& plugman)
{
  m_graph.ResetAll();
  m_graph.Invalidate();
  m_rollup.ExpandAllPages(FALSE);
    
  if (m_dataman.LoadTraceStatisticsOnTheFly(m_traceFilePath, plugman))
  {
    m_dlgCurveOptions.Refresh();
    m_dlgGraphOptions.Refresh();
    m_dlgExportOptions.Refresh();
    m_dlgOnTheFlyOptions.Refresh();
    m_rollup.ExpandAllPages();
    m_graph.Invalidate();
    return true;
  }
  else
  {
    m_dlgCurveOptions.Reset();
    m_dlgGraphOptions.Reset();
    m_dlgExportOptions.Reset();
    m_dlgOnTheFlyOptions.Reset();
    m_graph.ResetAll();
    m_graph.Invalidate();
    m_rollup.ExpandAllPages(FALSE);
    return false;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerStatisticViewerDlg::ProgressUpdate(DXPlayerStatisticsData::ProgressType type, char* message, unsigned int value)
{
  static unsigned int lastValue = 0;
  
  switch (type)
  {
  case DXPlayerStatisticsData::PT_BEGIN:
    this->SetWindowText(m_defaultTitle+" - "+message+"...");
    lastValue = (unsigned int) -1;
    break;
  
  case DXPlayerStatisticsData::PT_VALUE:
    if (value != lastValue)
    {
      CString progressTitle;
      progressTitle.Format(" - %s %u%%", message, value);
      this->SetWindowText(m_defaultTitle+progressTitle);
      lastValue = value;
    }
    break;
  
  case DXPlayerStatisticsData::PT_ENDED:
    this->SetWindowText(m_defaultTitle);
    lastValue = (unsigned int) -1;
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

#define IDM_ZOOM            40002
#define IDM_PAN             40004
#define IDM_ZOOM_BACK       40013
#define IDM_MEASURE         40015
#define IDM_DELETE_MEASURES 40016

LRESULT DXPlayerStatisticViewerDlg::WindowProc(UINT message, WPARAM wParam, LPARAM lParam) 
{	
  switch (message)
  {
  case XG_RBUTTONUP:
    {
      if (m_loadingStatistics || m_graph.GetOperation() == CXGraph::EOperation::opPan)
      {
        m_graph.NoOp();
        return CDialog::WindowProc(message, wParam, lParam);
      }

      m_graph.NoOp();

      CPoint point;
      GetCursorPos(&point);
      ScreenToClient(&point);
      CMenu menu;
      menu.CreatePopupMenu (); 
      menu.AppendMenu(MF_STRING, IDM_MEASURE, _T("Measure"));
      menu.AppendMenu(MF_STRING, IDM_PAN,     _T("Pan"));
      menu.AppendMenu(MF_STRING, IDM_ZOOM,    _T("Zoom"));
      if (m_graph.GetZoomDepth() > 0 || m_graph.GetMeasureCount() > 0)
      {
        menu.AppendMenu(MF_SEPARATOR);

        if (m_graph.GetZoomDepth() > 0)
        {
          menu.AppendMenu(MF_STRING, IDM_ZOOM_BACK, _T("Zoom Out"));
        }

        if (m_graph.GetMeasureCount() > 0)
        {
          menu.AppendMenu(MF_STRING, IDM_DELETE_MEASURES, _T("Delete Measure"));
        }
      }

      ClientToScreen(&point);
      UINT nCmd = menu.TrackPopupMenu(TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_NONOTIFY, point.x, point.y, (CXGraph*) lParam);
    }
    break;
  }

  return CDialog::WindowProc(message, wParam, lParam);
}

////////////////////////////////////////////////////////////////////////////////

LRESULT DXPlayerStatisticViewerDlg::OnZoomChange(WPARAM wParam, LPARAM lParam)
{
  m_dlgGraphOptions.Refresh();
  m_graph.NoOp();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

LRESULT DXPlayerStatisticViewerDlg::OnPanChange(WPARAM wParam, LPARAM lParam)
{
  m_dlgGraphOptions.Refresh();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

LRESULT DXPlayerStatisticViewerDlg::OnMeasureCreated(WPARAM wParam, LPARAM lParam)
{
  m_graph.NoOp();
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "DXPainterHeaders.h"
#include "DXPlayerTraceViewerDlg.h"

using namespace std;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

#define THN_THREAD_ENDED WM_USER + 1 // Thread Ends

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXPlayerTraceViewerDlg, CDialog)
  ON_BN_CLICKED(IDC_BTN_DUMP_TRACE, OnDumpTrace)
  ON_MESSAGE(THN_THREAD_ENDED, ThreadEnded)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayerTraceViewerDlg::DXPlayerTraceViewerDlg(const CString& traceFilePath, CWnd* pParent) :
CDialog(IDD_TRACEVIEWER, pParent),
m_traceFilePath(traceFilePath),
m_dumpingTrace(false),
m_killingThread(false),
m_thread(NULL)
{
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTraceViewerDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_PGR_DUMP_TRACE, m_progress);
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerTraceViewerDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  
  m_progress.SetRange(0, 100);
  m_progress.SetStep(1);
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTraceViewerDlg::OnCancel()
{
  if (ThreadEnd())
  {
    CDialog::OnCancel();
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerTraceViewerDlg::GetSaveFileNameTXT(CString& fileName)
{
  TCHAR tsFile[MAX_PATH] = "";
  TCHAR tsFilters[]= TEXT
    (
    "Text file (TXT)\0*.txt\0"
    "\0\0"
    );

  if (fileName.GetLength())
  {
    strcpy(tsFile, fileName.GetString());
  }

  OPENFILENAME ofn;
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = this->m_hWnd;
  ofn.lpstrFilter = tsFilters;
  ofn.lpstrInitialDir = "";
  ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFilterIndex = 4;
  ofn.lpstrFile = tsFile;
  ofn.nMaxFile = sizeof(tsFile) / sizeof(TCHAR);

  if (::GetSaveFileName(&ofn))
  {
    CString fileExtension;

    switch (ofn.nFilterIndex)
    {
    default:
    case 1:
      fileExtension = "txt";
      break;
    }

    fileName = CString(tsFile);

    ////////////////////////////////////////////////////////////////////////////
    // Repair the filename extension

    if (ofn.nFileExtension == 0)
    {
      fileName += "." + fileExtension;
    }
    else if (tsFile[ofn.nFileExtension] == '\0')
    {
      fileName += fileExtension;
    }
    else
    {
      if (fileName.Mid(ofn.nFileExtension, 3).MakeLower().Compare(fileExtension) != 0)
      {
        fileName += "." + fileExtension;
      }
    }

    ////////////////////////////////////////////////////////////////////////////
  }

  return (fileName.GetLength() > 0);
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerTraceViewerDlg::DumpTrace()
{
  CString title;
  CString titleOriginal;
  this->GetWindowText(titleOriginal);
  
  DXTraceManager traceman;

  this->SetWindowText(titleOriginal+" - Dumped 0Mb");
  
  if (!traceman.OpenRead(m_traceFilePath.GetString()))
  {
    CString str;
    str.Format("Could'nt open '%s'.", m_traceFilePath);
    MessageBox(str, "Error", MB_OK | MB_ICONWARNING);
    return false;
  }

  this->SetWindowText(titleOriginal+" - Dumped 0Mb");
  
  ofstream fileTrace(m_dumpFileName, ios::out | ios::trunc);
  if (!fileTrace.is_open())
  {
    CString str;
    str.Format("Could'nt open '%s'.", m_dumpFileName);
    MessageBox(str, "Error", MB_OK | MB_ICONWARNING);
    return false;
  }
  
  DXMethodCallPtr call;
  unsigned int k = traceman.GetMethodCallCount();
  
  for (unsigned int i=0, j=0; i < k && !m_killingThread; ++i)
  {
    m_progress.SetPos((unsigned int) ((i+1)*100 / k));
    
    if (!(i % 10000))
    {
      title.Format(" - Dumped %.2fMb", fileTrace.tellp() / (1024.0*1024.0));
      this->SetWindowText(titleOriginal+title);
    }

    traceman.ReadMethodCall(&call, i);
    fileTrace << "[" << i+1 << "] ";
    call->SerializeToString(fileTrace);
    fileTrace << endl;
    if (call->GetToken() == DXMethodCallHelper::TOK_IDirect3DDevice9_Present)
    {
      fileTrace << "- end frame " << setw(5) << setfill(' ') << ++j << " --------------------------------------------------------------" << endl;
    }
  }

  this->SetWindowText(titleOriginal);
  
  fileTrace.close();
  traceman.Close();
  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTraceViewerDlg::OnDumpTrace() 
{
  if (GetSaveFileNameTXT(m_dumpFileName))
  {
    GetDlgItem(IDC_BTN_DUMP_TRACE)->EnableWindow(FALSE);
    ThreadStart();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTraceViewerDlg::ThreadStart() 
{
  ThreadEnd();
  m_dumpingTrace = true;
  m_thread = (HANDLE) _beginthreadex(NULL, 0, ThreadWork, (LPVOID) this, 0, NULL); 
}


////////////////////////////////////////////////////////////////////////////////

bool DXPlayerTraceViewerDlg::ThreadEnd()
{
  if (!m_dumpingTrace)
  {
    ::CloseHandle(m_thread);
    return true;
  }

  m_killingThread = true;

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

unsigned __stdcall DXPlayerTraceViewerDlg::ThreadWork(LPVOID lpParam)
{
  DXPlayerTraceViewerDlg* dialog = (DXPlayerTraceViewerDlg*) lpParam;

  dialog->DumpTrace();

  dialog->m_dumpingTrace = false;
  dialog->m_killingThread = false;

  dialog->SendMessage(THN_THREAD_ENDED, ::GetCurrentThreadId(), 0);
  
  _endthreadex(0);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

LRESULT DXPlayerTraceViewerDlg::ThreadEnded(WPARAM wParam, LPARAM lParam)
{
  EndDialog(IDOK);
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

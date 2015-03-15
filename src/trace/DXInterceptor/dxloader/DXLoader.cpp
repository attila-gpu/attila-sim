////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXLoader.h"
#include "DXLoaderDlg.h"

////////////////////////////////////////////////////////////////////////////////

#define arrayof(x) (sizeof(x)/sizeof(x[0]))

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXLoader, CWinApp)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXLoader gApp;

////////////////////////////////////////////////////////////////////////////////

DXLoader::DXLoader()
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXLoader::InitInstance()
{
	InitCommonControls();
	CWinApp::InitInstance();  
  OleInitialize(NULL);
  
  DXLoaderDlg dlg;
  m_pMainWnd = &dlg;
  INT_PTR nResponse = dlg.DoModal();
  if (nResponse == IDOK)
  {
    LoadProgramWithDll();
  }
  
  return FALSE;
}

////////////////////////////////////////////////////////////////////////////////

DWORD DXLoader::LoadProgramWithDll()
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;
  
  ZeroMemory(&si, sizeof(si));
  ZeroMemory(&pi, sizeof(pi));
  si.cb = sizeof(si);

  TCHAR szExePath[MAX_PATH];
  _tcsncpy(szExePath, m_runInfo.m_pathProgramToLoad.c_str(), MAX_PATH);

  TCHAR szExeCommand[MAX_PATH];
  _tcsncpy(szExeCommand, m_runInfo.m_pathProgramToLoad.c_str(), MAX_PATH);
  ::PathQuoteSpaces(szExeCommand);
  
  CHAR szExeCurrentDir[MAX_PATH];
  _tcsncpy(szExeCurrentDir, m_runInfo.m_pathProgramToLoad.c_str(), MAX_PATH);
  ::PathRemoveFileSpec(szExeCurrentDir);
  ::PathAddBackslash(szExeCurrentDir);
  
  CHAR szDllPath[MAX_PATH];
  _tcsncpy(szDllPath, m_runInfo.m_pathDllToLoad.c_str(), MAX_PATH);  
  
  SetLastError(0);
  if (!DetourCreateProcessWithDll(szExePath, szExeCommand, NULL, NULL, TRUE, CREATE_DEFAULT_ERROR_MODE | CREATE_SUSPENDED, NULL, szExeCurrentDir, &si, &pi, NULL, szDllPath, NULL))
  {
    CString strError;
    strError.Format("Failed 'DetourCreateProcessWithDll'\nError code: %d", GetLastError());
    ::MessageBox(NULL, strError.GetString(), "Error", MB_DEFAULT_DESKTOP_ONLY | MB_ICONERROR | MB_OK);
    ExitProcess(2);
  }

  ::ResumeThread(pi.hThread);
  WaitForSingleObject(pi.hProcess, INFINITE);

  DWORD dwResult = 0;
  if (!GetExitCodeProcess(pi.hProcess, &dwResult))
  {
    CString strError;
    strError.Format("Failed 'GetExitCodeProcess'\nError code: %d", GetLastError());
    ::MessageBox(NULL, strError.GetString(), "Error", MB_DEFAULT_DESKTOP_ONLY | MB_ICONERROR | MB_OK);
    ExitProcess(3);
  }

  return dwResult;
}

////////////////////////////////////////////////////////////////////////////////

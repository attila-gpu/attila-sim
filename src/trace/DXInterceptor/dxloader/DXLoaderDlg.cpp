////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "XMLConfig.h"
#include "DXInterceptorOptions.h"
#include "DXLoader.h"
#include "DXLoaderDlg.h"
#include "DXTraceManagerHeaders.h"
#include "DXLoaderDllOptionsDlg.h"

////////////////////////////////////////////////////////////////////////////////

using namespace std;

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXLoaderDlg, CDialog)
  ON_WM_DESTROY()
  ON_WM_PAINT()
  ON_BN_CLICKED(IDC_BTN_BROWSE, OnBtnBrowse)
  ON_BN_CLICKED(IDC_BTN_OPTIONS, OnBtnOptions)
  ON_BN_CLICKED(IDOK, OnBtnRun)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXLoaderDlg::DXLoaderDlg(CWnd* pParent) :
CDialog(IDD_DXLOADER_DIALOG, pParent)
{
  m_hIcon = AfxGetApp()->LoadIcon(IDI_DXLOADER);
  m_config = new XMLConfig("dxloader.config", true);
}

////////////////////////////////////////////////////////////////////////////////

DXLoaderDlg::~DXLoaderDlg()
{
  if (m_config)
  {
    delete m_config;
    m_config = NULL;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::DetectDLLs(vector<string>& v_dlls)
{
  HANDLE hFind;
  WIN32_FIND_DATA wfd;
  TCHAR	szPath[MAX_PATH];

  _tcsncpy(szPath, m_strPath, MAX_PATH);
  ::PathAppend(szPath, _T("*"));

  hFind = ::FindFirstFile(szPath, &wfd);
  if (hFind != INVALID_HANDLE_VALUE)
  {
    do {
    	if ((wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
      {
        if (::PathMatchSpec(wfd.cFileName, "*.dll"))
        {
          TCHAR szNextFile[MAX_PATH];
          _tcsncpy(szNextFile, m_strPath, MAX_PATH);
          ::PathAppend(szNextFile, wfd.cFileName);
          v_dlls.push_back(szNextFile);
        }
      }
    } while(::FindNextFile(hFind, &wfd));
    ::FindClose(hFind);
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXLoaderDlg::CheckDXInterceptorDLL(const string& filename)
{
  HMODULE hModule = ::LoadLibrary(filename.c_str());
  if (hModule)
  {
    FARPROC lpProc = ::GetProcAddress(hModule, "Direct3DCreate9");
    ::FreeLibrary(hModule);
    return (lpProc != NULL);
  }
  return false;
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::GetDXInterceptorDLLVersion(const string& filename, string& version)
{
  DWORD dummy = 0;
  DWORD infoSize = ::GetFileVersionInfoSize(filename.c_str(), &dummy);
  char* buffer = new char[infoSize];
  if (::GetFileVersionInfo(filename.c_str(), 0, infoSize, buffer))
  {
    struct LANGANDCODEPAGE
    {
      WORD wLanguage;
      WORD wCodePage;
    } *lpTranslate;
   
    UINT cbTranslate;

    VerQueryValue(buffer, TEXT("\\VarFileInfo\\Translation"), (LPVOID*) &lpTranslate, &cbTranslate);

    for(unsigned int i=0; i < (cbTranslate / sizeof(LANGANDCODEPAGE)); i++)
    {
      char blockField[1024];
      sprintf(blockField, TEXT("\\StringFileInfo\\%04x%04x\\FileVersion"), lpTranslate[i].wLanguage, lpTranslate[i].wCodePage);
      
      LPVOID lpBuffer;
      UINT cbBuffer;
      VerQueryValue(buffer, blockField, &lpBuffer, &cbBuffer);

      char strVersion[1024];
      strncpy(strVersion, (const char*) lpBuffer, cbBuffer);
      strVersion[cbBuffer] = '\0';

      version = strVersion;
    }
  }
  delete[] buffer;
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::DetectDXInterceptorDLLs(vector<string>& v_dllsName)
{
  vector<string> v_dlls;
  DetectDLLs(v_dlls);

  m_detectedDLLs.clear();
  v_dllsName.clear();
  vector<string>::iterator it;
  for (it = v_dlls.begin(); it != v_dlls.end(); it++)
  {
    if (CheckDXInterceptorDLL(*it))
    {
      TCHAR szFilename[MAX_PATH];
      _tcsncpy(szFilename, (*it).c_str(), MAX_PATH);
      ::PathStripPath(szFilename);

      m_detectedDLLs.push_back(*it);
      
      string version;
      GetDXInterceptorDLLVersion((*it).c_str(), version);

      string strDLL = szFilename;
      if (!version.empty())
      {
        strDLL += " (" + version + ")";
      }

      v_dllsName.push_back(strDLL);    
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::FillCombo()
{
  vector<string> v_dlls;
  DetectDXInterceptorDLLs(v_dlls);
  
  CComboBox* combo = (CComboBox*) GetDlgItem(IDC_CBO_DLL);
  combo->ResetContent();

  if (v_dlls.size() > 0)
  {
    int i = 0;
    vector<string>::iterator it;
    for (it = v_dlls.begin(); it != v_dlls.end(); it++)
    {
      combo->InsertString(i++, (*it).c_str());
    }
    combo->SetCurSel(0);
  }

  if (combo->GetCount() <= 0)
  {
    combo->EnableWindow(false);
  }
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXLoaderDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  
  SetIcon(m_hIcon, TRUE); // Set big icon
  SetIcon(m_hIcon, FALSE); // Set small icon

  TCHAR curDir[MAX_PATH];
  ::GetCurrentDirectory(MAX_PATH, curDir);
  ::PathAddBackslash(curDir);
  m_strPath = curDir;

  FillCombo();
  LoadSettings();
  
  CComboBox* combo = (CComboBox*) GetDlgItem(IDC_CBO_DLL);
  if (!combo->IsWindowEnabled())
  {
    CButton* button;
    button= (CButton*) GetDlgItem(IDC_BTN_OPTIONS);
    button->EnableWindow(false);
    button= (CButton*) GetDlgItem(IDOK);
    button->EnableWindow(false);
  }
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::OnPaint() 
{
  if (IsIconic())
  {
    CPaintDC dc(this);

    SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

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

void DXLoaderDlg::OnBtnBrowse()
{
  TCHAR tsFilters[]= TEXT("Executables (*.exe)\0*.exe\0\0");
  TCHAR tsFile[MAX_PATH] = "";
  OPENFILENAME ofn;

  CEdit* edt = (CEdit*) GetDlgItem(IDC_TXT_BROWSE);
  CString pathProgramLoad;
  CString pathInitial;
  edt->GetWindowText(pathProgramLoad);
  
  pathInitial = m_strPath;
  if (!pathProgramLoad.IsEmpty())
  {
    TCHAR szInitialDir[MAX_PATH];
    _tcsncpy(szInitialDir, pathProgramLoad.GetString(), MAX_PATH);
    ::PathRemoveFileSpec(szInitialDir);
    ::PathAddBackslash(szInitialDir);
    if (::PathFileExists(szInitialDir))
    {
      pathInitial = szInitialDir;
    }
  }
  
  ZeroMemory(&ofn, sizeof(ofn));

  ofn.lStructSize = sizeof(ofn);
  ofn.hwndOwner = this->m_hWnd;
  ofn.lpstrFilter = tsFilters;
  ofn.lpstrInitialDir = pathInitial;
  ofn.Flags = OFN_EXPLORER | OFN_HIDEREADONLY | OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile = tsFile;
  ofn.nMaxFile = sizeof(tsFile) / sizeof(TCHAR);

  if (GetOpenFileName(&ofn))
  {
    CEdit* edt = (CEdit*) GetDlgItem(IDC_TXT_BROWSE);
    edt->SetWindowText(tsFile);
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::OnBtnOptions()
{
  DXLoaderDllOptionsDlg dlgOptions(GetPathDll(), this);
  dlgOptions.DoModal();
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::OnBtnRun()
{
  CString pathProgramLoad = GetPathExecutable();
  CString pathDllLoad = GetPathDll();
  
  if (!pathProgramLoad.IsEmpty() && !pathDllLoad.IsEmpty() && ::PathFileExists(pathProgramLoad) && ::PathFileExists(pathDllLoad))
  {
    gApp.m_runInfo.m_pathProgramToLoad = pathProgramLoad;
    gApp.m_runInfo.m_pathDllToLoad = pathDllLoad;
    OnOK();
  }
  else
  {
    if (pathProgramLoad.IsEmpty())
    {
      MessageBox("No program to be loaded has been indicated.", "Error", MB_OK | MB_ICONSTOP);
    }
    else
    if (pathDllLoad.IsEmpty())
    {
      MessageBox("No valid DLL found in the current directory.", "Error", MB_OK | MB_ICONSTOP);
    }
    else
    if (!PathFileExists(pathProgramLoad))
    {
      MessageBox("Could'nt find the program to load.", "Error", MB_OK | MB_ICONSTOP);
    }
    else
    if (!PathFileExists(pathDllLoad))
    {
      MessageBox("Could'nt find the DLL to load.", "Error", MB_OK | MB_ICONSTOP);
      FillCombo();
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::OnDestroy()
{
  SaveSettings();
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::LoadSettings()
{
  string cadena;  
  
  if (m_config->GetSectionText("configuration/lastexecutable", cadena))
  {
    CEdit* edt = (CEdit*) GetDlgItem(IDC_TXT_BROWSE);
    edt->SetWindowText(cadena.c_str());
  }
  
  if (m_config->GetSectionText("configuration/lastdll", cadena))
  {
    CComboBox* combo = (CComboBox*) GetDlgItem(IDC_CBO_DLL);
    if (combo->IsWindowEnabled())
    {
      CString configPathDll = cadena.c_str();
      configPathDll = StripPath(configPathDll).MakeLower();
      int position = 0;
      for (vector<string>::iterator it=m_detectedDLLs.begin(); it != m_detectedDLLs.end(); it++, position++)
      {
        CString detectedPathDll = (*it).c_str();
        detectedPathDll = StripPath(detectedPathDll).MakeLower();
        if (configPathDll == detectedPathDll)
        {
          combo->SetCurSel(position);
          break;
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDlg::SaveSettings()
{
  CString pathExecutable = GetPathExecutable();
  if (!pathExecutable.IsEmpty())
  {
    m_config->AddSectionText("configuration/lastexecutable", pathExecutable.GetString());
  }

  CString pathDll = StripPath(GetPathDll());
  if (!pathDll.IsEmpty())
  {
    m_config->AddSectionText("configuration/lastdll", pathDll.GetString());
  }

  m_config->Save();
}

////////////////////////////////////////////////////////////////////////////////

CString DXLoaderDlg::StripPath(CString& filename)
{
  TCHAR szFilename[MAX_PATH];
  _tcsncpy(szFilename, filename.GetString(), MAX_PATH);
  ::PathStripPath(szFilename);
  return CString(szFilename);
}

////////////////////////////////////////////////////////////////////////////////

CString DXLoaderDlg::GetPathExecutable()
{
  CEdit* edt = (CEdit*) GetDlgItem(IDC_TXT_BROWSE);
  CString pathExecutable;
  edt->GetWindowText(pathExecutable);
  return pathExecutable;
}

////////////////////////////////////////////////////////////////////////////////

CString DXLoaderDlg::GetPathDll()
{
  CComboBox* combo = (CComboBox*) GetDlgItem(IDC_CBO_DLL);
  CString pathDll;
  if (combo->IsWindowEnabled())
  {
    pathDll = m_detectedDLLs[combo->GetCurSel()].c_str();
  }
  return pathDll;
}

////////////////////////////////////////////////////////////////////////////////

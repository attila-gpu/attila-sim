////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXPlayerOptions.h"
#include "DXPlayerOptionsDlg.h"

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXPlayerOptionsDlg, CDialog)
  ON_BN_CLICKED(IDC_BTN_BROWSE_DESTPATH, OnBtnBrowse)
  ON_BN_CLICKED(IDOK, OnBtnSave)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayerOptionsDlg::DXPlayerOptionsDlg(CString optionsFilename, CWnd* pParent) :
m_optionsFilename(optionsFilename),
CDialog(IDD_OPTIONS, pParent)
{
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerOptionsDlg::~DXPlayerOptionsDlg()
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerOptionsDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  FillCombos();
  LoadOptions();

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptionsDlg::FillCombos()
{
  CComboBox* combo = (CComboBox*) GetDlgItem(IDC_CBO_SCREENSHOTFORMATS);
  combo->ResetContent();
  combo->InsertString(0, "Windows Bitmap (BMP)");
  combo->InsertString(1, "Joint Photographics Experts Group (JPG)");
  combo->InsertString(2, "Truevision Targa (TGA)");
  combo->InsertString(3, "Portable Network Graphics (PNG)");
  combo->InsertString(4, "DirectDraw Surface (DDS)");
  combo->InsertString(5, "Portable Pixmap (PPM)");
  combo->InsertString(6, "Windows Device-Independent Bitmap (DIB)");
  combo->InsertString(7, "High Dynamic Range (HDR)");
  combo->InsertString(8, "Portable Float Map (PFM)");
  combo->SetCurSel(0);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptionsDlg::ApplyOptions()
{
  CEdit* textbox;
  CComboBox* combo;

  textbox = (CEdit*) GetDlgItem(IDC_TXT_BROWSE_DESTPATH);
  textbox->SetWindowText(m_options.GetDestinationPath().c_str());

  combo = (CComboBox*) GetDlgItem(IDC_CBO_SCREENSHOTFORMATS);
  switch (m_options.GetScreenshotFormat())
  {
  case DXPlayerOptions::SSF_BMP:
    combo->SetCurSel(0);
    break;
  case DXPlayerOptions::SSF_JPG:
    combo->SetCurSel(1);
    break;
  case DXPlayerOptions::SSF_TGA:
    combo->SetCurSel(2);
    break;
  case DXPlayerOptions::SSF_PNG:
    combo->SetCurSel(3);
    break;
  case DXPlayerOptions::SSF_DDS:
    combo->SetCurSel(4);
    break;
  case DXPlayerOptions::SSF_PPM:
    combo->SetCurSel(5);
    break;
  case DXPlayerOptions::SSF_DIB:
    combo->SetCurSel(6);
    break;
  case DXPlayerOptions::SSF_HDR:
    combo->SetCurSel(7);
    break;
  case DXPlayerOptions::SSF_PFM:
    combo->SetCurSel(8);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptionsDlg::RetrieveOptions()
{
  CComboBox* combo;
  CEdit* textbox;
  CString cadena;

  textbox = (CEdit*) GetDlgItem(IDC_TXT_BROWSE_DESTPATH);
  textbox->GetWindowText(cadena);
  m_options.SetDestinationPath(cadena.GetString());

  combo = (CComboBox*) GetDlgItem(IDC_CBO_SCREENSHOTFORMATS);
  switch (combo->GetCurSel())
  {
  case 8:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_PFM);
    break;
  case 7:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_HDR);
    break;
  case 6:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_DIB);
    break;
  case 5:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_PPM);
    break;
  case 4:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_DDS);
    break;
  case 3:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_PNG);
    break;
  case 2:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_TGA);
    break;
  case 1:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_JPG);
    break;
  case 0:
  default:
    m_options.SetScreenshotFormat(DXPlayerOptions::SSF_BMP);
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerOptionsDlg::LoadOptions()
{
  bool loaded = m_options.LoadXML(m_optionsFilename.GetString());  
  ApplyOptions();
  return loaded;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerOptionsDlg::SaveOptions()
{
  RetrieveOptions();
  return m_options.SaveXML(m_optionsFilename.GetString());
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptionsDlg::OnBtnBrowse()
{
  BROWSEINFO browseInfo = {0};

  browseInfo.hwndOwner = this->m_hWnd;
  browseInfo.pidlRoot = NULL;
  browseInfo.lpszTitle = "Pick a directory:";
  browseInfo.ulFlags = BIF_NEWDIALOGSTYLE | BIF_RETURNONLYFSDIRS | BIF_SHAREABLE;
  browseInfo.lpfn = NULL;

  LPITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
  if (pidl != 0)
  {
    TCHAR selectedPath[MAX_PATH];
    if (SHGetPathFromIDList(pidl, selectedPath))
    {
      if (strlen(selectedPath))
      {
        ::PathAddBackslash(selectedPath);
        CEdit* edt = (CEdit*) GetDlgItem(IDC_TXT_BROWSE_DESTPATH);
        edt->SetWindowText(selectedPath);
      }
    }

    IMalloc* imalloc = NULL;
    if (SUCCEEDED(SHGetMalloc(&imalloc)))
    {
      imalloc->Free(pidl);
      imalloc->Release();
    }    
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerOptionsDlg::OnBtnSave()
{
  if (MessageBox("Do you want to save this options?", "Save", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
  {
    SaveOptions();
    EndDialog(0);
  }
}

////////////////////////////////////////////////////////////////////////////////

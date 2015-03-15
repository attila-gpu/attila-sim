////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXPainterHeaders.h"
#include "DXPlayerProjectInformationDlg.h"

using namespace dxpainter;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXPlayerProjectInformationDlg, CDialog)
  ON_BN_CLICKED(IDOK, OnBtnSave)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayerProjectInformationDlg::DXPlayerProjectInformationDlg(DXPainter& painter, CWnd* pParent) :
CDialog(IDD_PROJECT_INFORMATION, pParent),
m_painter(painter)
{
}

////////////////////////////////////////////////////////////////////////////////

DXPlayerProjectInformationDlg::~DXPlayerProjectInformationDlg()
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerProjectInformationDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  LoadInformation();

  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerProjectInformationDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_TXT_PROJECT_NAME, m_projectName);
  DDX_Text(pDX, IDC_TXT_PROJECT_ANOTATIONS, m_projectAnotations);

  DDV_MaxChars(pDX, m_projectAnotations, 128);
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerProjectInformationDlg::LoadInformation()
{
  CString str;
  
  const DXTraceManager& traceman = m_painter.GetTraceManager();
  
  //////////////////////////////////////////////////////////////////////////////
  
  struct tm* today;
  time_t created = traceman.GetCreationDate();
  today = localtime(&created);
  strftime(str.GetBuffer(256), 256, "%d/%m/%Y %H:%M:%S", today);	
  str.ReleaseBuffer();
  GetDlgItem(IDC_LBL_CREATION_DATE)->SetWindowText(str);
  
  //////////////////////////////////////////////////////////////////////////////
  
  str.Format("%u", traceman.GetMethodCallCount());
  GetDlgItem(IDC_LBL_NUM_METHOD_CALLS)->SetWindowText(str);

  str.Format("%u", traceman.GetBufferCount());
  GetDlgItem(IDC_LBL_NUM_BUFFERS)->SetWindowText(str);

  str.Format("%u", traceman.GetTextureCount());
  GetDlgItem(IDC_LBL_NUM_TEXTURES)->SetWindowText(str);

  str.Format("%u", traceman.GetStatisticCount());
  GetDlgItem(IDC_LBL_NUM_STATISTICS)->SetWindowText(str);
  
  //////////////////////////////////////////////////////////////////////////////
  
  if (traceman.GetFileSize() >= 1024*1024*1024)
    str.Format("%.2f Gb", traceman.GetFileSize() / (1024.0*1024.0*1024.0));
  else
    str.Format("%.2f Mb", traceman.GetFileSize() / (1024.0*1024.0));
  GetDlgItem(IDC_LBL_DATA_SIZE)->SetWindowText(str);

  if (traceman.GetFileSizeInDisk() >= 1024*1024*1024)
    str.Format("%.2f Gb", traceman.GetFileSizeInDisk() / (1024.0*1024.0*1024.0));
  else
    str.Format("%.2f Mb", traceman.GetFileSizeInDisk() / (1024.0*1024.0));
  GetDlgItem(IDC_LBL_DISK_SIZE)->SetWindowText(str);
  
  if (traceman.IsCompressed())
    str.Format("yes (%.0f%%)", (traceman.GetFileSize() ? traceman.GetFileSizeInDisk() * 100.0 / traceman.GetFileSize() : 0.0));
  else
    str = "no";
  GetDlgItem(IDC_LBL_COMPRESSED_INFO)->SetWindowText(str);

  str.Format("%s", (traceman.IsOptimized() ? "yes" : "no"));
  GetDlgItem(IDC_LBL_OPTIMIZED_INFO)->SetWindowText(str);

  //////////////////////////////////////////////////////////////////////////////
  
  m_projectName = traceman.GetProjectGameName().c_str();
  m_projectAnotations = traceman.GetProjectAnotations().c_str();

  //////////////////////////////////////////////////////////////////////////////
  
  UpdateData(FALSE); 
  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerProjectInformationDlg::SaveInformation()
{
  UpdateData(TRUE);
  return m_painter.UpdateProjectInformation(m_projectName.GetString(), m_projectAnotations.GetString());
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerProjectInformationDlg::OnBtnSave()
{
  if (MessageBox("Do you want to save this information?", "Save", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
  {
    if (!SaveInformation())
    {
      MessageBox("An error ocurred updating the project information.", "Error", MB_OK | MB_ICONWARNING);
      EndDialog(IDCANCEL);
    }
    else
    {
      EndDialog(IDOK);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

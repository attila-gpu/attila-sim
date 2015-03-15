////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DXLoader.h"
#include "ColourPickerXP.h"
#include "DXInterceptorOptions.h"
#include "DXTraceManagerHeaders.h"
#include "DXLoaderDllOptionsDlg.h"

////////////////////////////////////////////////////////////////////////////////

using namespace std;
using namespace dxplugin;

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXLoaderDllOptionsDlg, CDialog)
  ON_BN_CLICKED(IDC_BTN_BROWSE_DESTPATH, OnBtnBrowse)
  ON_BN_CLICKED(IDC_CHK_SHOWPROGRESSBANNER, OnChkShowBanner)
  ON_BN_CLICKED(IDOK, OnBtnSave)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXLoaderDllOptionsDlg::DXLoaderDllOptionsDlg(CString pathDLL, CWnd* pParent) :
CDialog(IDD_DXOPTIONS_DIALOG, pParent),
m_pathDLL(pathDLL)
{
}

////////////////////////////////////////////////////////////////////////////////

DXLoaderDllOptionsDlg::~DXLoaderDllOptionsDlg()
{
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXLoaderDllOptionsDlg::OnInitDialog()
{
  CDialog::OnInitDialog();
  
  SetTitle(StripPath(m_pathDLL)+" Options");
  FillCombos();
  InitStatisticsTree();
  FillStatisticsTree();
  LoadOptions();
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_BTN_PROGRESSBANNERTEXTCOLOR, m_colorBox);
  DDX_Control(pDX, IDC_STATISTICS_COUNTERS, m_statCounters);
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::OnBtnBrowse()
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

void DXLoaderDllOptionsDlg::OnChkShowBanner()
{
  CButton* button;
  button = (CButton*) GetDlgItem(IDC_CHK_SHOWPROGRESSBANNER);
  EnableBannerOptions(button->GetCheck() == BST_CHECKED);
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::OnBtnSave()
{
  if (MessageBox("Do you want to save this options?", "Save", MB_YESNO | MB_ICONQUESTION | MB_DEFBUTTON2) == IDYES)
  {
    SaveOptions();
    EndDialog(0);
  }
}

////////////////////////////////////////////////////////////////////////////////

CString DXLoaderDllOptionsDlg::StripPath(CString& filename)
{
  TCHAR szFilename[MAX_PATH];
  _tcsncpy(szFilename, filename.GetString(), MAX_PATH);
  ::PathStripPath(szFilename);
  return CString(szFilename);
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::SetTitle(CString title)
{
  SetWindowText(title);
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::FillCombos()
{
  CComboBox* combo;
  
  combo = (CComboBox*) GetDlgItem(IDC_CBO_PROGRESSBANNERPOSITION);
  combo->ResetContent();
  combo->InsertString(0, "Top-left");
  combo->InsertString(1, "Top-right");
  combo->InsertString(2, "Bottom-left");
  combo->InsertString(3, "Bottom-right");
  combo->SetCurSel(0);

  m_colorBox.SetStyle(true);
  m_colorBox.SetTrackSelection(true);
  m_colorBox.SetDefaultText(_T(""));
  m_colorBox.SetColor(RGB(0xFF, 0xFF, 0xFF));
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::InitStatisticsTree()
{
  m_statCountersImages.Create(IDB_TREELIST_ICONS, 16, 4, 0xFF00FF);

  m_statCounters.SetStyle
    ( 0
    | TLC_TREELIST      // TreeList or List
    | TLC_SHOWSELALWAYS // show selected item always
    | TLC_READONLY      // read only
    | TLC_TREELINE      // show tree line
    | TLC_ROOTLINE      // show root line
    | TLC_BUTTON        // show expand/collapse button [+]
    | TLC_CHECKBOX      // show check box
    );

  m_statCounters.SetImageList(&m_statCountersImages);

  m_statCounters.InsertColumn(_T("Plugins / Counters"), TLF_DEFAULT_LEFT | TLF_CAPTION_TEXT);
  m_statCounters.SetColumnModify(0, TLM_EDIT);
  m_statCounters.SetColumnWidth(0, 310);
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::FillStatisticsTree()
{
  if (m_plugman.LoadAllPluginsFromDirectory())
  {
    CString cadena;
    CTreeListItem* treeItem;
    CTreeListItem* treeItemChild;

    for (unsigned int i=0; i < m_plugman.GetPluginCount(); ++i)
    {
      DXIntPluginLoaded* plugin;
      m_plugman.GetPlugin(i, &plugin);
      
      cadena.Format("%s (%s)", plugin->GetName().c_str(), plugin->GetFileName().c_str());
      treeItem = m_statCounters.InsertItem(cadena);
      treeItem->SetData(i);
      treeItem->Expand();

      for (unsigned int j=0; j < plugin->GetCounterCount(); ++j)
      {
        DXINTCOUNTERINFO counterInfo;
        plugin->GetCounter(j, &counterInfo);
        
        cadena.Format("%s: %s", counterInfo.Name, counterInfo.Description);
        treeItemChild = m_statCounters.InsertItem(cadena, treeItem);
        treeItemChild->SetData(j);
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::ApplyOptions()
{
  CComboBox* combo;
  CEdit* textbox;
  CButton* button;
  
  textbox = (CEdit*) GetDlgItem(IDC_TXT_BROWSE_DESTPATH);
  textbox->SetWindowText(m_options.GetDestinationPath().c_str());
  
  button = (CButton*) GetDlgItem(IDC_CHK_ENABLECOMPRESSION);
  button->SetCheck(m_options.GetCompression() ? BST_CHECKED : BST_UNCHECKED);
  button = (CButton*) GetDlgItem(IDC_CHK_SHOWPROGRESSBANNER);
  button->SetCheck(m_options.GetBannerShow() ? BST_CHECKED : BST_UNCHECKED);

  EnableBannerOptions(m_options.GetBannerShow());
  
  combo = (CComboBox*) GetDlgItem(IDC_CBO_PROGRESSBANNERPOSITION);
  switch (m_options.GetBannerPosition())
  {
  case DXInterceptorOptions::BP_TopLeft:
    combo->SetCurSel(0);
    break;
  case DXInterceptorOptions::BP_TopRight:
    combo->SetCurSel(1);
    break;
  case DXInterceptorOptions::BP_BottomLeft:
    combo->SetCurSel(2);
    break;
  case DXInterceptorOptions::BP_BottomRight:
    combo->SetCurSel(3);
    break;
  }

  m_colorBox.SetColor(D3DCOLOR_To_COLORREF(m_options.GetBannerTextColor()));

  LoadStatisticsTree();
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::RetrieveOptions()
{
  CComboBox* combo;
  CEdit* textbox;
  CButton* button;
  CString cadena;

  textbox = (CEdit*) GetDlgItem(IDC_TXT_BROWSE_DESTPATH);
  textbox->GetWindowText(cadena);
  m_options.SetDestinationPath(cadena.GetString());

  button = (CButton*) GetDlgItem(IDC_CHK_ENABLECOMPRESSION);
  m_options.SetCompression(button->GetCheck() == BST_CHECKED);
  button = (CButton*) GetDlgItem(IDC_CHK_SHOWPROGRESSBANNER);
  m_options.SetBannerShow(button->GetCheck() == BST_CHECKED);

  combo = (CComboBox*) GetDlgItem(IDC_CBO_PROGRESSBANNERPOSITION);
  switch (combo->GetCurSel())
  {
  case 3:
    m_options.SetBannerPosition(DXInterceptorOptions::BP_BottomRight);
    break;
  case 2:
    m_options.SetBannerPosition(DXInterceptorOptions::BP_BottomLeft);
    break;
  case 1:
    m_options.SetBannerPosition(DXInterceptorOptions::BP_TopRight);
    break;
  case 0:
  default:
    m_options.SetBannerPosition(DXInterceptorOptions::BP_TopLeft);
    break;
  }

  m_options.SetBannerTextColor(COLORREF_To_D3DCOLOR(m_colorBox.GetColor()));

  SaveStatisticsTree();
}

////////////////////////////////////////////////////////////////////////////////

CString DXLoaderDllOptionsDlg::GetOptionsFilename()
{
  TCHAR configFilename[MAX_PATH];
  _tcsncpy(configFilename, m_pathDLL.GetString(), MAX_PATH);
  ::PathRenameExtension(configFilename, TEXT(".config"));
  return CString(configFilename);
}

////////////////////////////////////////////////////////////////////////////////

bool DXLoaderDllOptionsDlg::LoadOptions()
{
  bool loaded = m_options.LoadXML(GetOptionsFilename().GetString());  
  ApplyOptions();
  return loaded;
}

////////////////////////////////////////////////////////////////////////////////

bool DXLoaderDllOptionsDlg::SaveOptions()
{
  RetrieveOptions();
  return m_options.SaveXML(GetOptionsFilename().GetString());
}

////////////////////////////////////////////////////////////////////////////////

bool DXLoaderDllOptionsDlg::LoadStatisticsTree()
{
  CTreeListItem* treeItem;
  CTreeListItem* treeItemChild;

  treeItem = m_statCounters.GetRootItem();
  if (treeItem)
  {
    do {
      
      DXIntPluginLoaded* plugin;
      if (!m_plugman.GetPlugin(treeItem->GetData(), &plugin))
      {
        continue;
      }

      bool found = false;
      DXInterceptorOptions::StatisticsPlugin optionsPlugin;
      for (unsigned int i=0; i < m_options.GetPluginCount(); ++i)
      {
        if (m_options.GetPlugin(i, optionsPlugin))
        {
          if (optionsPlugin.PluginFileName == plugin->GetFileName())
          {
            found = true;
            break;
          }
        }
        else
        {
          break;
        }
      }

      if (!found)
      {
        continue;
      }

      if (m_statCounters.ItemHasChildren(treeItem))
      {
        treeItemChild = m_statCounters.GetChildItem(treeItem);
        if (treeItemChild)
        {
          do {
            DXINTCOUNTERINFO counterInfo;
            if (!plugin->GetCounter(treeItemChild->GetData(), &counterInfo))
            {
              continue;
            }

            found = false;
            for (unsigned int i=0; i < (unsigned int) optionsPlugin.Counters.size(); ++i)
            {
              if (optionsPlugin.Counters[i] == counterInfo.ID)
              {
                found = true;
                break;
              }
            }
            
            if (found)
            {
              treeItemChild->SetCheck(TRUE);
            }

          } while (treeItemChild = m_statCounters.GetNextSiblingItem(treeItemChild));
        }
      }

      if (optionsPlugin.Counters.size() > 0)
      {
        m_options.AddPlugin(optionsPlugin);
      }
    } while (treeItem = m_statCounters.GetNextSiblingItem(treeItem));
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

bool DXLoaderDllOptionsDlg::SaveStatisticsTree()
{
  CTreeListItem* treeItem;
  CTreeListItem* treeItemChild;
  
  m_options.ClearPlugins();
  
  treeItem = m_statCounters.GetRootItem();
  if (treeItem)
  {
    do {
      DXInterceptorOptions::StatisticsPlugin optionsPlugin;
      DXIntPluginLoaded* plugin;
      if (!m_plugman.GetPlugin(treeItem->GetData(), &plugin))
      {
        continue;
      }
      
      optionsPlugin.PluginFileName = plugin->GetFileName();
      
      if (m_statCounters.ItemHasChildren(treeItem))
      {
        treeItemChild = m_statCounters.GetChildItem(treeItem);
        if (treeItemChild)
        {
          do {
            if (!treeItemChild->GetCheck())
            {
              continue;
            }
            
            DXINTCOUNTERINFO counterInfo;
            if (!plugin->GetCounter(treeItemChild->GetData(), &counterInfo))
            {
              continue;
            }
            
            optionsPlugin.Counters.push_back(counterInfo.ID);

          } while (treeItemChild = m_statCounters.GetNextSiblingItem(treeItemChild));
        }
      }

      if (optionsPlugin.Counters.size() > 0)
      {
        m_options.AddPlugin(optionsPlugin);
      }
    } while (treeItem = m_statCounters.GetNextSiblingItem(treeItem));
  }

  return true;
}

////////////////////////////////////////////////////////////////////////////////

void DXLoaderDllOptionsDlg::EnableBannerOptions(bool enable)
{
  CComboBox* combo;
  combo = (CComboBox*) GetDlgItem(IDC_CBO_PROGRESSBANNERPOSITION);
  combo->EnableWindow(enable);
  m_colorBox.EnableWindow(enable);
}

////////////////////////////////////////////////////////////////////////////////

COLORREF DXLoaderDllOptionsDlg::D3DCOLOR_To_COLORREF(D3DCOLOR color)
{
  return RGB((color >> 16) & 0xFF, (color >> 8) & 0xFF, (color >> 0) & 0xFF);
}

////////////////////////////////////////////////////////////////////////////////

D3DCOLOR DXLoaderDllOptionsDlg::COLORREF_To_D3DCOLOR(COLORREF color)
{
  return D3DCOLOR_RGBA((color >> 0) & 0xFF, (color >> 8) & 0xFF, (color >> 16) & 0xFF, 0xFF);
}

////////////////////////////////////////////////////////////////////////////////

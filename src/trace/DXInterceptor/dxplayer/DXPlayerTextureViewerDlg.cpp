////////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "resource.h"
#include "GdiPlusHelper.h"
#include "DXPlayer.h"
#include "DXPainterHeaders.h"
#include "DXPlayerTextureViewerDlg.h"

using namespace std;
using namespace Gdiplus;
using namespace dxtraceman;

////////////////////////////////////////////////////////////////////////////////

#define THN_THREAD_UPDATE_SCREEN WM_USER + 1 // Thread needs update screen

////////////////////////////////////////////////////////////////////////////////

BEGIN_MESSAGE_MAP(DXPlayerTextureViewerDlg, CDialog)
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_NOTIFY(LVN_ITEMCHANGED, IDC_LST_TEXTURELIST, OnItemChanged)
  ON_BN_CLICKED(IDC_BTN_SAVE, OnSaveTexture)
  ON_MESSAGE(THN_THREAD_UPDATE_SCREEN, UpdateScreen)
END_MESSAGE_MAP()

////////////////////////////////////////////////////////////////////////////////

DXPlayerTextureViewerDlg::DXPlayerTextureViewerDlg(const string& traceFilePath, CWnd* pParent /*=NULL*/) :
CDialog(IDD_TEXTUREVIEWER, pParent),
m_loadingThumbnails(false),
m_killingThread(false),
m_thread(NULL),
m_currentTexturePreview(-1),
m_traceFilePath(traceFilePath)
{
  m_strNumTextures = _T("0 textures");
  m_strPreview = _T("");
  m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Control(pDX, IDC_LST_TEXTURELIST, m_listThumbs);
  DDX_Control(pDX, IDC_IMG_PREVIEW, m_preview);
  DDX_Text(pDX, IDC_LBL_TEXTURELIST, m_strNumTextures);
  DDX_Text(pDX, IDC_LBL_PREVIEW, m_strPreview);
}

////////////////////////////////////////////////////////////////////////////////

BOOL DXPlayerTextureViewerDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  // Set the icon for this dialog.
  SetIcon(m_hIcon, TRUE);  // Set big icon
  SetIcon(m_hIcon, FALSE); // Set small icon

  // Initialize ImageList and attach it to ListCtrl
  m_imageListThumb.Create(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, ILC_COLOR32, 0, 1);
  m_listThumbs.SetImageList(&m_imageListThumb, LVSIL_NORMAL);
  
  //////////////////////////////////////////////////////////////////////////////
  // Load textures thumbnails in background
  
  ThreadStart();

  //////////////////////////////////////////////////////////////////////////////
  
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::OnPaint() 
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

HCURSOR DXPlayerTextureViewerDlg::OnQueryDragIcon()
{
  return (HCURSOR) m_hIcon;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::OnCancel()
{
  if (ThreadEnd())
  {
    CDialog::OnCancel();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult) 
{
  LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
  PreviewTexture(pNMLV->iItem);
  *pResult = 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::ThreadStart() 
{
  ThreadEnd();
  m_killingThread = false;
  m_loadingThumbnails = true;
  m_thread = (HANDLE) _beginthreadex(NULL, 0, ThreadWork, (LPVOID) this, 0, NULL); 
}


////////////////////////////////////////////////////////////////////////////////

bool DXPlayerTextureViewerDlg::ThreadEnd()
{
  if (!m_loadingThumbnails)
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

unsigned __stdcall DXPlayerTextureViewerDlg::ThreadWork(LPVOID lpParam)
{
  DXPlayerTextureViewerDlg* dialog = (DXPlayerTextureViewerDlg*) lpParam;
  
  dialog->LoadThumbnails();
  
  dialog->m_loadingThumbnails = false;
  dialog->m_killingThread = false;
  
  _endthreadex(0);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::RefreshScreenThreadSafe()
{
  SendMessage(THN_THREAD_UPDATE_SCREEN, ::GetCurrentThreadId(), 0);
}

////////////////////////////////////////////////////////////////////////////////

LRESULT DXPlayerTextureViewerDlg::UpdateScreen(WPARAM wParam, LPARAM lParam)
{
  UpdateData(FALSE);
  return TRUE;
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::LoadThumbnails()
{
  // clean the list data [
  m_strNumTextures.Format("Cleaning list...");
  RefreshScreenThreadSafe();
  
  PreviewTexture(-1);
  m_vectTextureIds.clear();
  m_listThumbs.SetRedraw(FALSE);
  m_imageListThumb.SetImageCount(0);
  m_listThumbs.DeleteAllItems();
  m_listThumbs.SetRedraw(TRUE);
  m_listThumbs.Invalidate();
  // ]
  
  m_strNumTextures.Format("Filling list...");
  RefreshScreenThreadSafe();
  
  if (!m_traceman.IsOpened())
  {
    m_traceman.OpenRead(m_traceFilePath);
  }
  
  if (m_traceman.IsOpened())
  {
    // set redraw to FALSE to avoid flickering during adding new items
    m_listThumbs.SetRedraw(FALSE);
    
    // set the description of the textures and associate with the thumbnail
    unsigned int i, j;
    for (i=0, j=0; i < m_traceman.GetTextureCount() && !m_killingThread; ++i)
    {
      DXTexturePtr textura;
      if (!m_traceman.ReadTexture(&textura, i) || textura->GetWidth() < 5 || textura->GetHeight() < 5 || textura->GetLevel() > 0)
      {
        continue;
      }
      
      ostringstream description;
      D3DFORMAT format = textura->GetFormat();
      description << textura->GetWidth() << "x" << textura->GetHeight() << endl;
      DXTypeHelper::ToString(description, &format, DXTypeHelper::TT_D3DFORMAT);
      
      m_vectTextureIds.push_back(i);
      m_listThumbs.InsertItem(j, description.str().c_str(), j++);

      m_strNumTextures.Format("%d (filling list...)", j);
      RefreshScreenThreadSafe();
    }

    // set number of thumbnails and textures processed
    m_imageListThumb.SetImageCount(j);
    m_strNumTextures.Format("%d/%d textures (loading...)", 0, m_imageListThumb.GetImageCount());
    RefreshScreenThreadSafe();

    m_listThumbs.SetRedraw(TRUE);
    m_listThumbs.Invalidate();

    unsigned int position = 0;
    for (vector<unsigned int>::iterator it=m_vectTextureIds.begin(); it != m_vectTextureIds.end() && !m_killingThread; it++, position++)
    {
      DXTexturePtr textura;
      m_traceman.ReadTexture(&textura, *it);
      GenerateThumbnailFromDXTexture(textura, position);

      m_strNumTextures.Format("%d/%d textures (loading...)", position+1, m_imageListThumb.GetImageCount());
      RefreshScreenThreadSafe();
    }

    m_listThumbs.Invalidate();

    m_strNumTextures.Format("%d textures", m_imageListThumb.GetImageCount());
    RefreshScreenThreadSafe();
  }
  else
  {
    m_strNumTextures.Format("%d textures", 0);
    RefreshScreenThreadSafe();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::GenerateThumbnailFromDXTexture(DXTexturePtr texture, unsigned int position)
{
  IStream* pIStream = NULL;
  if (theApp.m_painter->SaveDXTextureToStream(texture, &pIStream) && pIStream)
  {
    Bitmap* textureMEM = Bitmap::FromStream(pIStream);
    
    if (textureMEM->GetLastStatus() == Ok)
    {
      const float ratioORI = (float) THUMBNAIL_HEIGHT / THUMBNAIL_WIDTH;
      const float ratioIMG = (float) textureMEM->GetHeight() / textureMEM->GetWidth();
      
      Rect rectIMG;
      if (ratioIMG > ratioORI)
      {
        rectIMG.Width = (int) (THUMBNAIL_HEIGHT / ratioIMG);
        rectIMG.Height = THUMBNAIL_HEIGHT;
        rectIMG.X = (THUMBNAIL_WIDTH - rectIMG.Width) >> 1;
        rectIMG.Y = 0;
      }
      else
      {
        rectIMG.Width = THUMBNAIL_WIDTH;
        rectIMG.Height = (int) (THUMBNAIL_WIDTH*ratioIMG);
        rectIMG.X = 0;
        rectIMG.Y = (THUMBNAIL_HEIGHT - rectIMG.Height) >> 1;
      }
      rectIMG.Width--;
      rectIMG.Height--;
      
      Bitmap thumbnail(THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT, PixelFormat32bppARGB);
      Graphics graphics(&thumbnail);

      SolidBrush white(Color::White);
      graphics.FillRectangle(&white, 0, 0, THUMBNAIL_WIDTH, THUMBNAIL_HEIGHT);
      
      graphics.DrawImage(textureMEM, rectIMG);

      Pen negre(Color::Black);
      graphics.DrawRectangle(&negre, rectIMG);

      HBITMAP hbm;
      thumbnail.GetHBITMAP(Color::White, &hbm);
      CBitmap bitmap;
      bitmap.Attach(hbm);

      m_imageListThumb.Replace(position, &bitmap, NULL);
      m_listThumbs.RedrawItems(position, position);
    }

    if (textureMEM)
    {
      delete textureMEM;
      textureMEM = NULL;
    }
    
    pIStream->Release();
  }
}

////////////////////////////////////////////////////////////////////////////////

void DXPlayerTextureViewerDlg::PreviewTexture(int number)
{
  if (number < 0)
  {
    CRect clientRect;
    m_preview.GetClientRect(&clientRect);
    Rect rectWIN(clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);
    
    GdiPlusBitmapResource logo;
    logo.Load(IDB_LOGO, _T("PNG"));
    
    Bitmap preview(rectWIN.Width, rectWIN.Height, PixelFormat32bppARGB);
    Graphics graphics(&preview);

    graphics.DrawImage(logo, (int) (rectWIN.Width - logo->GetWidth()) >> 1, (int) (rectWIN.Height - logo->GetHeight()) >> 1, logo->GetWidth(), logo->GetHeight());

    HBITMAP hbm;
    preview.GetHBITMAP(Color::Transparent, &hbm);
    m_preview.SetBitmap(hbm);

    m_currentTexturePreview = -1;
    m_strPreview.Format("");
    RefreshScreenThreadSafe();
  }
  else
  {
    if (m_currentTexturePreview == number || m_loadingThumbnails)
    {
      return;
    }
    
    m_strPreview.Format("Loading...");
    RefreshScreenThreadSafe();
    
    if (!m_traceman.IsOpened())
    {
      m_traceman.OpenRead(m_traceFilePath);
    }

    if (m_traceman.IsOpened())
    {
      DXTexturePtr texture;
      if (m_traceman.ReadTexture(&texture, m_vectTextureIds[number]))
      {
        IStream* pIStream = NULL;
        if (theApp.m_painter->SaveDXTextureToStream(texture, &pIStream) && pIStream)
        {
          Bitmap* textureMEM = Bitmap::FromStream(pIStream);

          if (textureMEM->GetLastStatus() == Ok)
          {
            CRect clientRect;
            m_preview.GetClientRect(&clientRect);
            Rect rectWIN(clientRect.left, clientRect.top, clientRect.right, clientRect.bottom);

            const float ratioORI = (float) rectWIN.Height / rectWIN.Width;
            const float ratioIMG = (float) textureMEM->GetHeight() / textureMEM->GetWidth();

            Rect rectIMG;
            if (ratioIMG > ratioORI)
            {
              rectIMG.Width = (int) (rectWIN.Height / ratioIMG);
              rectIMG.Height = rectWIN.Height;
              rectIMG.X = (rectWIN.Width - rectIMG.Width) >> 1;
              rectIMG.Y = 0;
            }
            else
            {
              rectIMG.Width = rectWIN.Width;
              rectIMG.Height = (int) (rectWIN.Width*ratioIMG);
              rectIMG.X = 0;
              rectIMG.Y = (rectWIN.Height - rectIMG.Height) >> 1;
            }
            rectIMG.Width--;
            rectIMG.Height--;

            Bitmap preview(rectWIN.Width, rectWIN.Height, PixelFormat32bppARGB);
            Graphics graphics(&preview);

            graphics.DrawImage(textureMEM, rectIMG);

            HBITMAP hbm;
            preview.GetHBITMAP(Color::Transparent, &hbm);
            m_preview.SetBitmap(hbm);

            
            m_currentTexturePreview = number;
            m_strPreview.Format("Texture %d", m_vectTextureIds[number]);
            RefreshScreenThreadSafe();
          }

          if (textureMEM)
          {
            delete textureMEM;
            textureMEM = NULL;
          }

          pIStream->Release();
        }
      }
    }
  }
}

////////////////////////////////////////////////////////////////////////////////

bool DXPlayerTextureViewerDlg::GetSaveFileNameIMG(CString& fileName, dxpainter::DXPainter::ScreenshotFormat& fileFormat)
{
  TCHAR tsFile[MAX_PATH] = "";
  TCHAR tsFilters[]= TEXT
    (
    "Windows Bitmap (BMP)\0*.bmp\0"
    "Joint Photographics Experts Group (JPG)\0*.jpg\0"
    "Truevision Targa (TGA)\0*.tga\0"
    "Portable Network Graphics (PNG)\0*.png\0"
    "DirectDraw Surface (DDS)\0*.dds\0"
    "Portable Pixmap (PPM)\0*.ppm\0"
    "Windows Device-Independent Bitmap (DIB)\0*.dib\0"
    "High Dynamic Range (HDR)\0*.hdr\0"
    "Portable Float Map (PFM)\0*.pfm\0"
    "\0\0"
    );

  fileName.Format("texture_%u", m_vectTextureIds[m_currentTexturePreview]);
  strcpy(tsFile, fileName.GetString());

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
    case 1:
      fileExtension = "bmp";
      fileFormat = dxpainter::DXPainter::SSF_BMP;
      break;
    case 2:
      fileExtension = "jpg";
      fileFormat = dxpainter::DXPainter::SSF_JPG;
      break;
    case 3:
      fileExtension = "tga";
      fileFormat = dxpainter::DXPainter::SSF_TGA;
      break;
    default:
    case 4:
      fileExtension = "png";
      fileFormat = dxpainter::DXPainter::SSF_PNG;
      break;
    case 5:
      fileExtension = "dds";
      fileFormat = dxpainter::DXPainter::SSF_DDS;
      break;
    case 6:
      fileExtension = "ppm";
      fileFormat = dxpainter::DXPainter::SSF_PPM;
      break;
    case 7:
      fileExtension = "dib";
      fileFormat = dxpainter::DXPainter::SSF_DIB;
      break;
    case 8:
      fileExtension = "hdr";
      fileFormat = dxpainter::DXPainter::SSF_HDR;
      break;
    case 9:
      fileExtension = "pfm";
      fileFormat = dxpainter::DXPainter::SSF_PFM;
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

void DXPlayerTextureViewerDlg::OnSaveTexture()
{
  if (m_currentTexturePreview < 0 || m_loadingThumbnails)
  {
    return;
  }
  
  if (!m_traceman.IsOpened())
  {
    m_traceman.OpenRead(m_traceFilePath);
  }

  if (m_traceman.IsOpened())
  {
    DXTexturePtr texture;
    if (m_traceman.ReadTexture(&texture, m_vectTextureIds[m_currentTexturePreview]))
    {
      CString fileName;
      dxpainter::DXPainter::ScreenshotFormat fileFormat = dxpainter::DXPainter::SSF_PNG;
      if (GetSaveFileNameIMG(fileName, fileFormat))
      {
        if (!theApp.m_painter->SaveDXTextureToFile(texture, fileName.GetString(), fileFormat))
        {
          MessageBox("An error ocurred saving the texture.", "Error", MB_OK | MB_ICONWARNING);
        }
      }
    }
  }  
}

////////////////////////////////////////////////////////////////////////////////

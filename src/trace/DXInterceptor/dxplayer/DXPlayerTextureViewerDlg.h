////////////////////////////////////////////////////////////////////////////////

#pragma once
#include "afxwin.h"

////////////////////////////////////////////////////////////////////////////////

#define	THUMBNAIL_WIDTH		90
#define	THUMBNAIL_HEIGHT	70

////////////////////////////////////////////////////////////////////////////////

class DXPlayerTextureViewerDlg : public CDialog
{
public:
  
  DXPlayerTextureViewerDlg(const std::string& traceFilePath, CWnd* pParent = NULL);

protected:
  
  HICON m_hIcon;
  CListCtrl m_listThumbs;
  CStatic m_preview;
  CString m_strNumTextures;
  CString m_strPreview;
  int m_currentTexturePreview;
  CImageList m_imageListThumb;
  std::string m_traceFilePath;
  std::vector<unsigned int> m_vectTextureIds;
  dxtraceman::DXTraceManager m_traceman;
  
  HANDLE m_thread;
  bool m_loadingThumbnails;
  bool m_killingThread;
  void ThreadStart();
  bool ThreadEnd();
  static unsigned __stdcall ThreadWork(LPVOID lpParam);
  void RefreshScreenThreadSafe();
  LRESULT UpdateScreen(WPARAM wParam, LPARAM lParam);

  void LoadThumbnails();
  void GenerateThumbnailFromDXTexture(dxtraceman::DXTexturePtr texture, unsigned int position);
  void PreviewTexture(int number);
  bool GetSaveFileNameIMG(CString& fileName, dxpainter::DXPainter::ScreenshotFormat& fileFormat);
  
  virtual void DoDataExchange(CDataExchange* pDX);
  virtual BOOL OnInitDialog();
  virtual void OnCancel();
  
  afx_msg void OnItemChanged(NMHDR* pNMHDR, LRESULT* pResult);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  afx_msg void OnSaveTexture();
  DECLARE_MESSAGE_MAP()
  
};

////////////////////////////////////////////////////////////////////////////////

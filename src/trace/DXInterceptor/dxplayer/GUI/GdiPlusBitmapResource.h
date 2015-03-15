////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

class GdiPlusBitmapResource
{
public:
  
  GdiPlusBitmapResource(void);  
  GdiPlusBitmapResource(LPCTSTR pName, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
  GdiPlusBitmapResource(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
  GdiPlusBitmapResource(UINT id, UINT type, HMODULE hInst = NULL);
  virtual ~GdiPlusBitmapResource(void);

  void Clear();

  bool Load(LPCTSTR pName, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
  bool Load(UINT id, LPCTSTR pType = RT_RCDATA, HMODULE hInst = NULL);
  bool Load(UINT id, UINT type, HMODULE hInst = NULL);

  operator Gdiplus::Bitmap * () const;
  Gdiplus::Bitmap* operator -> () const;

protected:

  Gdiplus::Bitmap* m_pBitmap;
  HGLOBAL m_hBuffer;

};

////////////////////////////////////////////////////////////////////////////////
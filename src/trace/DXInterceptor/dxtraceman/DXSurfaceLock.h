////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXSurfaceLock
  {
  public:

    DXSurfaceLock();
    virtual ~DXSurfaceLock();

    D3DLOCKED_RECT GetLockedRect() const;
    void SetLockedRect(D3DLOCKED_RECT* lockedRect);

    void SetLock(IDirect3DSurface9* surface, UINT level, CONST RECT* rect, D3DFORMAT format);
    void SetLock(IDirect3DTexture9* texture, UINT level, CONST RECT* rect, D3DFORMAT format);
    void SetLock(IDirect3DCubeTexture9* texture, UINT level, CONST RECT* rect, D3DFORMAT format);

    bool ReadLock(DXTexturePtr texture);
    bool WriteLock(DXTexturePtr texture);

    static DWORD GetBitsPerPixelForFormat(D3DFORMAT format);

  protected:

    D3DLOCKED_RECT m_lockedRect;
    UINT m_bytesPerLine;
    UINT m_numLines;

    UINT m_width;
    UINT m_height;
    D3DFORMAT m_format;
    UINT m_level;

    void SetLockBase(CONST RECT* rect, D3DSURFACE_DESC* description, D3DFORMAT format);

  };

  //////////////////////////////////////////////////////////////////////////////

  typedef DXSurfaceLock DXCubeMapLock[6];

  //////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////

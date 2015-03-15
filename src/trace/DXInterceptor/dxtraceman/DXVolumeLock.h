////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////

namespace dxtraceman
{
  class DXVolumeLock
  {
  public:

    DXVolumeLock();
    virtual ~DXVolumeLock();

    D3DLOCKED_BOX GetLockedBox() const;
    void SetLockedBox(D3DLOCKED_BOX* lockedBox);

    void SetLock(IDirect3DVolume9* volume, UINT level, CONST D3DBOX* box, D3DFORMAT format);
    void SetLock(IDirect3DVolumeTexture9* volumeTexture, UINT level, CONST D3DBOX* box, D3DFORMAT format);

    bool ReadLock(DXTexturePtr volumeTexture);
    bool WriteLock(DXTexturePtr volumeTexture);

    static DWORD GetBitsPerPixelForFormat(D3DFORMAT format);

  protected:

    D3DLOCKED_BOX m_lockedBox;
    UINT m_bytesPerLine;
    UINT m_numLines;
    UINT m_numSurfaces;

    UINT m_width;
    UINT m_height;
    UINT m_depth;
    D3DFORMAT m_format;
    UINT m_level;

    void SetLockBase(CONST D3DBOX* box, D3DVOLUME_DESC* description, D3DFORMAT format);

  };

  //////////////////////////////////////////////////////////////////////////////
}

////////////////////////////////////////////////////////////////////////////////

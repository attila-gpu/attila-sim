/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

#ifndef AIVOLUMEIMP_9_H_INCLUDED
#define AIVOLUMEIMP_9_H_INCLUDED

class AIVolumeTextureImp9;

class AIVolumeImp9 : public IDirect3DVolume9{
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static AIVolumeImp9 &getInstance();

   AIVolumeImp9(/*StateDataNode* s_parent,*/ AIDeviceImp9* i_parent, UINT Width, UINT Height, UINT Depth,
        DWORD Usage , D3DFORMAT Format, D3DPOOL Pool);

   void setVolumeTexture(AIVolumeTextureImp9* tex);
   void setMipMapLevel(UINT m);

private:
    /// Singleton constructor method

    AIVolumeImp9();

    AIDeviceImp9* i_parent;

    UINT Width;
    UINT Height;
    UINT Depth;
    UINT Levels;
    DWORD Usage;
    D3DFORMAT Format;
    D3DPOOL Pool;

    LONG leftLock;
    LONG topLock;
    LONG rightLock;
    LONG bottomLock;
    LONG frontLock;
    LONG backLock;

    unsigned char* lockedData;
    UINT lockedPitch;
    UINT slicePitch;

    ULONG ref_count;

    AIVolumeTextureImp9* volumeTexture;
    UINT mipmap;

public:
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags );
    HRESULT D3D_CALL GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData );
    HRESULT D3D_CALL FreePrivateData (  REFGUID refguid );
    HRESULT D3D_CALL GetContainer (  REFIID riid , void** ppContainer );
    HRESULT D3D_CALL GetDesc (  D3DVOLUME_DESC * pDesc );
    HRESULT D3D_CALL LockBox (  D3DLOCKED_BOX * pLockedVolume , CONST D3DBOX* pBox , DWORD Flags );
    HRESULT D3D_CALL UnlockBox ( );

    virtual DWORD D3D_CALL SetPriority(DWORD);
    virtual DWORD D3D_CALL GetPriority();
    virtual void D3D_CALL PreLoad();
    virtual D3DRESOURCETYPE D3D_CALL GetType();

};


#endif


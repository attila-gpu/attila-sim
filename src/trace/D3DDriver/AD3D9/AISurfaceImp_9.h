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

#ifndef AISURFACEIMP_9_H_INCLUDED
#define AISURFACEIMP_9_H_INCLUDED

class AIDeviceImp9;

class AISurfaceImp9 : public IDirect3DSurface9 {
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static AISurfaceImp9 &getInstance();

    AISurfaceImp9(AIDeviceImp9* i_parent, UINT Width, UINT Height,
    DWORD Usage, D3DFORMAT Format, D3DPOOL Pool);

    void setAsTexture2D(AITextureImp9* tex);
    void setAsCubeTexture(AICubeTextureImp9* tex);
    void setAsRenderTarget(acdlib::ACDRenderTarget* rt);

    void setMipMapLevel(UINT m);
    void setFace(D3DCUBEMAP_FACES f);

    TextureType getAD3D9Type();

    acdlib::ACDTexture2D* getACDTexture2D();
    acdlib::ACDRenderTarget* getACDRenderTarget();

    UINT getMipMapLevel();
    

private:
    /// Singleton constructor method
    AISurfaceImp9();

    AIDeviceImp9* i_parent;

    UINT Width;
    UINT Height;
    DWORD Usage;
    D3DFORMAT Format;
    D3DPOOL Pool;

    //unsigned char* data;

    LONG leftLock;
    LONG topLock;
    LONG rightLock;
    LONG bottomLock;

    unsigned char* lockedData;
    UINT lockedPitch;

    ULONG ref_count;

    //UINT numModif;

    TextureType surfaceType;

    AITextureImp9* texture2d;
    AICubeTextureImp9* cubeTexture;
    acdlib::ACDRenderTarget* renderTarget;
    UINT mipmap;
    D3DCUBEMAP_FACES face;

public:
    HRESULT D3D_CALL QueryInterface(REFIID riid, void** ppvObj);
    ULONG D3D_CALL AddRef();
    ULONG D3D_CALL Release();
    HRESULT D3D_CALL GetDevice(IDirect3DDevice9** ppDevice);
    HRESULT D3D_CALL SetPrivateData(REFGUID refguid, CONST void* pData, DWORD SizeOfData, DWORD Flags);
    HRESULT D3D_CALL GetPrivateData(REFGUID refguid, void* pData, DWORD* pSizeOfData);
    HRESULT D3D_CALL FreePrivateData(REFGUID refguid);
    DWORD D3D_CALL SetPriority(DWORD PriorityNew);
    DWORD D3D_CALL GetPriority();
    void D3D_CALL PreLoad();
    D3DRESOURCETYPE D3D_CALL GetType();
    HRESULT D3D_CALL GetContainer(REFIID riid, void** ppContainer);
    HRESULT D3D_CALL GetDesc(D3DSURFACE_DESC * pDesc);
    HRESULT D3D_CALL LockRect(D3DLOCKED_RECT* pLockedRect, CONST RECT* pRect, DWORD Flags);
    HRESULT D3D_CALL UnlockRect();
    HRESULT D3D_CALL GetDC(HDC * phdc);
    HRESULT D3D_CALL ReleaseDC(HDC hdc);

};

#endif


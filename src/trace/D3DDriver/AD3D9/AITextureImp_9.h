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

#ifndef AITEXTUREIMP_9_H
#define AITEXTUERIMP_9_H

class AITextureImp9 : public IDirect3DTexture9 {
public:
    /// Singleton method, maintained to allow unimplemented methods to return valid interface addresses.
    static AITextureImp9 &getInstance();

    AITextureImp9(AIDeviceImp9* _i_parent, UINT Width, UINT Height,
    UINT Levels, DWORD Usage , D3DFORMAT Format, D3DPOOL Pool);
    ~AITextureImp9();

    acdlib::ACDTexture2D* getACDTexture2D();

    BOOL usePCF();

private:
    /// Singleton constructor method
    AITextureImp9();

    AIDeviceImp9* i_parent;

    UINT Width;
    UINT Height;
    UINT Levels;
    DWORD Usage;
    D3DFORMAT Format;
    D3DPOOL Pool;

    UINT numLevels;

    AISurfaceImp9 **mipLevels;

    ULONG ref_count;

    acdlib::ACDTexture2D* texture2d;

public:
    HRESULT D3D_CALL QueryInterface (  REFIID riid , void** ppvObj );
    ULONG D3D_CALL AddRef ( );
    ULONG D3D_CALL Release ( );
    HRESULT D3D_CALL GetDevice (  IDirect3DDevice9** ppDevice );
    HRESULT D3D_CALL SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags );
    HRESULT D3D_CALL GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData );
    HRESULT D3D_CALL FreePrivateData (  REFGUID refguid );
    DWORD D3D_CALL SetPriority (  DWORD PriorityNew );
    DWORD D3D_CALL GetPriority ( );
    void D3D_CALL PreLoad ( );
    D3DRESOURCETYPE D3D_CALL GetType ( );
    DWORD D3D_CALL SetLOD (  DWORD LODNew );
    DWORD D3D_CALL GetLOD ( );
    DWORD D3D_CALL GetLevelCount ( );
    HRESULT D3D_CALL SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType );
    D3DTEXTUREFILTERTYPE D3D_CALL GetAutoGenFilterType ( );
    void D3D_CALL GenerateMipSubLevels ( );
    HRESULT D3D_CALL GetLevelDesc (  UINT Level , D3DSURFACE_DESC * pDesc );
    HRESULT D3D_CALL GetSurfaceLevel (  UINT Level , IDirect3DSurface9** ppSurfaceLevel );
    HRESULT D3D_CALL LockRect (  UINT Level , D3DLOCKED_RECT* pLockedRect , CONST RECT* pRect , DWORD Flags );
    HRESULT D3D_CALL UnlockRect (  UINT Level );
    HRESULT D3D_CALL AddDirtyRect (  CONST RECT* pDirtyRect );
};

#endif

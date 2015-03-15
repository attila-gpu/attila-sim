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

#include "Common.h"
#include "AD3D9State.h"
#include "AIDeviceImp_9.h"
#include "AISurfaceImp_9.h"
#include "AITextureImp_9.h"
#include "Utils.h"


AITextureImp9::AITextureImp9(AIDeviceImp9* _i_parent, UINT _Width, UINT _Height,
                             UINT _Levels, DWORD _Usage , D3DFORMAT _Format, D3DPOOL _Pool) : 

i_parent(_i_parent), Width(_Width), Height(_Height),
Levels(_Levels), Usage(_Usage), Format(_Format), Pool(_Pool)
    
{
    // Create a new texture 2D
    //AD3D9State::instance().addTexture2D(this, Usage);


    // Create a new ACD Texture 2D
    texture2d = AD3D9State::instance().createTexture2D();

    // Add the texture 2D to the known textures 2D
    //textures2D[tex] = acdTexture;

    //  Set layout to render buffer.
    if ((Usage & D3DUSAGE_DEPTHSTENCIL) || (Usage & D3DUSAGE_RENDERTARGET))
    {
        texture2d->setMemoryLayout(acdlib::ACD_LAYOUT_RENDERBUFFER);

        if (is_compressed(Format))
            Format = D3DFMT_A8R8G8B8;

    }
    
    //textureTextureType[(AIBaseTextureImp9*)tex] = AD3D9_TEXTURE2D;
    AD3D9State::instance().addTypeTexture((AIBaseTextureImp9*)this, AD3D9_TEXTURE2D);



    numLevels = level_count(Levels, Width, Height);

    mipLevels = new AISurfaceImp9*[numLevels];
    
    UINT mip_width = Width;
    UINT mip_height = Height;

    for (UINT i = 0; i < numLevels; i++)
    {
        AISurfaceImp9* i_mipmap;
        i_mipmap = new AISurfaceImp9(i_parent, mip_width, mip_height, Usage, Format, Pool);

        mipLevels[i] = i_mipmap;

        //AD3D9State::instance().addMipLevel(this, i_mipmap, i, mip_width, mip_height, Format);


        // Reference to the mip level texture
        //mipLevelsTextures2D[mip] = tex;

        // Reference to the mip level number
        //mipLevelsLevels[mip] = mipLevel;

        //mipLevelsTextureType[mip] = AD3D9_TEXTURE2D;

        // Get mip level size
        UINT size = getSurfaceSize(mip_width, mip_height, Format);

        //BYTE* data = new BYTE[size];

        // If isn't a compressed texture, the size passed to ACD must be 0
        if (!is_compressed(Format))
            size = 0;

        // Set a void buffer to create the new mipmap in ACD
        //textures2D[tex]->setData(mipLevel, width, height, getACDFormat(format), /*getSurfacePitch(width, format)*/ 0, (acdlib::acd_ubyte*)data, size);
        texture2d->setData(i, mip_width, mip_height, AD3D9State::instance().getACDFormat(Format), 0, NULL, size);

        //delete[] data;

        i_mipmap->setAsTexture2D(this);

        i_mipmap->setMipMapLevel(i);



        if (mip_width == 1)
            mip_width = 1;
        else
            mip_width = mip_width / 2;

        if (mip_height == 1)
            mip_height = 1;
        else
            mip_height = mip_height / 2;

    }

    ref_count = 0;

}

AITextureImp9::~AITextureImp9()
{
    // Delete surface childs
    for(UINT i = 0; i < numLevels; i++)
        delete mipLevels[i];
        
    delete[] mipLevels;
}

AITextureImp9::AITextureImp9() {}

AITextureImp9& AITextureImp9::getInstance() 
{
    static AITextureImp9 instance;
    return instance;
}

acdlib::ACDTexture2D* AITextureImp9::getACDTexture2D() 
{
    return texture2d;
}

BOOL AITextureImp9::usePCF() 
{
    if (Format == D3DFMT_D24S8 ||
        Format == D3DFMT_D24X8) 
    {
        return true;
    }
    else 
    {
        return false;
    }
}

HRESULT D3D_CALL AITextureImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AITextureImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AITextureImp9 :: AddRef ( ) 
{
    D3D9_CALL(true, "AITextureImp9::AddRef")
    if(i_parent != 0) {
        ref_count++;
        return ref_count;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }
}

ULONG D3D_CALL AITextureImp9 :: Release ( )
{
    D3D9_CALL(true, "AITextureImp9::Release")
    
    if(i_parent != 0)
    {
        ref_count --;
        /*if(ref_count == 0) {
            // Release mipmaps
            set<ISurfaceImp9*>::iterator it_s;
            for(it_s = i_surface_childs.begin(); it_s != i_surface_childs.end(); it_s ++) {
                (*it_s)->Release();
            }
            // Remove state
            StateDataNode* parent = state->get_parent();
            parent->remove_child(state);
            delete state;
            state = 0;
        }*/
        return ref_count;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }
}

HRESULT D3D_CALL AITextureImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) 
{
    D3D9_CALL(true, "AITextureImp9::GetDevice")
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL AITextureImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) 
{
    D3D9_CALL(false, "AITextureImp9::SetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AITextureImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) 
{
    D3D9_CALL(false, "AITextureImp9::GetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AITextureImp9 :: FreePrivateData (  REFGUID refguid ) 
{
    D3D9_CALL(false, "AITextureImp9::FreePrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL AITextureImp9 :: SetPriority (  DWORD PriorityNew ) 
{
    D3D9_CALL(false, "AITextureImp9::SetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AITextureImp9 :: GetPriority ( ) 
{
    D3D9_CALL(false, "AITextureImp9::GetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL AITextureImp9 :: PreLoad ( ) 
{
    D3D9_CALL(false, "AITextureImp9::PreLoad")
}

D3DRESOURCETYPE D3D_CALL AITextureImp9 :: GetType ( ) 
{
    D3D9_CALL(false, "AITextureImp9::GetType")
    return D3DRTYPE_TEXTURE;
}

DWORD D3D_CALL AITextureImp9 :: SetLOD (  DWORD LODNew ) 
{

    D3D9_CALL(true, "AITextureImp9::SetLOD")

    AD3D9State::instance().setLOD((AIBaseTextureImp9*)this, LODNew);

    return D3D_OK;

}

DWORD D3D_CALL AITextureImp9 :: GetLOD ( ) 
{
    D3D9_CALL(false, "AITextureImp9::GetLOD")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AITextureImp9 :: GetLevelCount ( ) 
{
    D3D9_CALL(false, "AITextureImp9::GetLevelCount")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL AITextureImp9 :: SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType ) 
{
    D3D9_CALL(false, "AITextureImp9::SetAutoGenFilterType")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DTEXTUREFILTERTYPE D3D_CALL AITextureImp9 :: GetAutoGenFilterType ( ) 
{
    D3D9_CALL(false, "AITextureImp9::GetAutoGenFilterType")
    D3DTEXTUREFILTERTYPE ret = static_cast< D3DTEXTUREFILTERTYPE >(0);
    return ret;
}

void D3D_CALL AITextureImp9 :: GenerateMipSubLevels ( ) 
{
    D3D9_CALL(false, "AITextureImp9::GenerateMipSubLevels")
}

HRESULT D3D_CALL AITextureImp9 :: GetLevelDesc (  UINT Level , D3DSURFACE_DESC * pDesc ) 
{
    D3D9_CALL(false, "AITextureImp9::GetLevelDesc")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AITextureImp9::GetSurfaceLevel(UINT Level, IDirect3DSurface9** ppSurfaceLevel) 
{

    D3D9_CALL(true, "AITextureImp9::GetSurfaceLevel")

    *ppSurfaceLevel = mipLevels[Level];
    (*ppSurfaceLevel)->AddRef();

    return D3D_OK;

}

HRESULT D3D_CALL AITextureImp9 :: LockRect (  UINT Level , D3DLOCKED_RECT* pLockedRect , CONST RECT* pRect , DWORD Flags ) 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: LockRect  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/


    D3D9_CALL(true, "AITextureImp9::LockRect")

    mipLevels[Level]->LockRect(pLockedRect, pRect, Flags);

    return D3D_OK;

}

HRESULT D3D_CALL AITextureImp9 :: UnlockRect (  UINT Level ) 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: UnlockRect  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/


    D3D9_CALL(false, "AITextureImp9::UnlockRect")

    mipLevels[Level]->UnlockRect();

    return D3D_OK;

}

HRESULT D3D_CALL AITextureImp9 :: AddDirtyRect (  CONST RECT* pDirtyRect ) 
{
    D3D9_CALL(false, "AITextureImp9::AddDirtyRect")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

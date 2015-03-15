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
#include "AIVolumeTextureImp_9.h"
#include "Utils.h"

#include "AD3D9State.h"

AIVolumeTextureImp9::AIVolumeTextureImp9(AIDeviceImp9* _i_parent, UINT _Width, UINT _Height, UINT _Depth,
                                         UINT _Levels, DWORD _Usage , D3DFORMAT _Format, D3DPOOL _Pool) :
                                         
    i_parent(_i_parent), Width(_Width), Height(_Height), Depth(_Depth),
    Levels(_Levels), Usage(_Usage), Format(_Format), Pool(_Pool)
    
{
    // Create a new texture 3D
    //AD3D9State::instance().addVolumeTexture(this);

    
    // Create a new ACD Volume Texture
    acdVolumeTexture = AD3D9State::instance().createVolumeTexture();

    AD3D9State::instance().addTypeTexture((AIBaseTextureImp9*)this, AD3D9_VOLUMETEXTURE);



    numLevels = level_count(Levels, Width, Height, Depth);
    
    mipLevels = new AIVolumeImp9*[numLevels];
    
    UINT mip_width = Width;
    UINT mip_height = Height;
    UINT mip_depth = Depth;

    for (UINT i = 0; i < numLevels; i++)
    {
        AIVolumeImp9* i_mipmap;
        i_mipmap = new AIVolumeImp9(i_parent, mip_width, mip_height, mip_depth, Usage, Format, Pool);

        mipLevels[i] = i_mipmap;

        //AD3D9State::instance().addVolumeLevel(this, i_mipmap, i, mip_width, mip_height, mip_depth, Format);



    /*mipVolumesVolumeTextures[vol] = tex;

    mipVolumesLevels[vol] = mipLevel;*/

        UINT size = getVolumeSize(mip_width, mip_height, mip_depth, Format);

        //BYTE* data = new BYTE[size];

        // If isn't a compressed texture, the size passed to ACD must be 0
        if (!is_compressed(Format)) size = 0;

        acdVolumeTexture->setData(i, mip_width, mip_height, mip_depth, AD3D9State::instance().getACDFormat(Format), 0, /*(acdlib::acd_ubyte*)data*/NULL, size);

        //delete[] data;

        i_mipmap->setVolumeTexture(this);

        i_mipmap->setMipMapLevel(i);



        if (mip_width == 1)
            mip_width = 1;
        else
            mip_width = mip_width / 2;

        if (mip_height == 1)
            mip_height = 1;
        else
            mip_height = mip_height / 2;

        if (mip_depth == 1)
            mip_depth = 1;
        else
            mip_depth = mip_depth / 2;
    }

    ref_count = 0;

}

AIVolumeTextureImp9 ::~AIVolumeTextureImp9()
{
    //  Delete volume childs.
    for(UINT i = 0; i < numLevels; i++)
        delete mipLevels[i];
        
    delete[] mipLevels;

}

AIVolumeTextureImp9::AIVolumeTextureImp9() {}

AIVolumeTextureImp9 & AIVolumeTextureImp9::getInstance() 
{
    static AIVolumeTextureImp9 instance;
    return instance;
}

acdlib::ACDTexture3D* AIVolumeTextureImp9::getACDVolumeTexture() 
{
    return acdVolumeTexture;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AIVolumeTextureImp9 :: AddRef ( ) 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;*/
    
    D3D9_CALL(true, "AIVolumeTextureImp9::AddRef")
    if(i_parent != 0) 
    {
        ref_count++;
        return ref_count;
    }
    else 
    {
        // Object is used as singleton "cover"
        return 0;
    }
}

ULONG D3D_CALL AIVolumeTextureImp9 :: Release ( ) 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;*/
    
    D3D9_CALL(true, "AIVolumeTextureImp9::Release")
    
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

HRESULT D3D_CALL AIVolumeTextureImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GetDevice")
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::SetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: FreePrivateData (  REFGUID refguid ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::FreePrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL AIVolumeTextureImp9 :: SetPriority (  DWORD PriorityNew ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::SetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AIVolumeTextureImp9 :: GetPriority ( ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL AIVolumeTextureImp9 :: PreLoad ( ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::PreLoad")
}

D3DRESOURCETYPE D3D_CALL AIVolumeTextureImp9 :: GetType ( ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GetType")
    return D3DRTYPE_VOLUMETEXTURE;

}

DWORD D3D_CALL AIVolumeTextureImp9 :: SetLOD (  DWORD LODNew ) 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: SetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;*/

    D3D9_CALL(true, "AIVolumeTextureImp9::SetLOD")

    AD3D9State::instance().setLOD((AIBaseTextureImp9*)this, LODNew);

    return D3D_OK;

}

DWORD D3D_CALL AIVolumeTextureImp9 :: GetLOD ( ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GetLOD")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AIVolumeTextureImp9 :: GetLevelCount ( ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GetLevelCount")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::SetAutoGenFilterType")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DTEXTUREFILTERTYPE D3D_CALL AIVolumeTextureImp9 :: GetAutoGenFilterType ( ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GetAutoGenFilterType")
    D3DTEXTUREFILTERTYPE ret = static_cast< D3DTEXTUREFILTERTYPE >(0);
    return ret;
}

void D3D_CALL AIVolumeTextureImp9 :: GenerateMipSubLevels ( ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GenerateMipSubLevels")
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: GetLevelDesc (  UINT Level , D3DVOLUME_DESC * pDesc ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::GetLevelDesc")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: GetVolumeLevel (  UINT Level , IDirect3DVolume9** ppVolumeLevel )
{
    D3D9_CALL(true, "AIVolumeTextureImp9::GetVolumeLevel")

    (*ppVolumeLevel) = mipLevels[Level];
    (*ppVolumeLevel)->AddRef();

    return D3D_OK;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: LockBox (  UINT Level , D3DLOCKED_BOX* pLockedVolume , CONST D3DBOX* pBox , DWORD Flags ) 
{
    /*pLockedVolume ->RowPitch = 1;
    pLockedVolume ->SlicePitch = 1;
    pLockedVolume ->pBits = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: LockBox  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/

    D3D9_CALL(true, "AIVolumeTextureImp9::LockBox")

    mipLevels[Level]->LockBox(pLockedVolume, pBox, Flags);

    return D3D_OK;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: UnlockBox (  UINT Level ) 
{
    /*D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: UnlockBox  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;*/

    D3D9_CALL(true, "AIVolumeTextureImp9::UnlockBox")

    mipLevels[Level]->UnlockBox();

    return D3D_OK;
}

HRESULT D3D_CALL AIVolumeTextureImp9 :: AddDirtyBox (  CONST D3DBOX* pDirtyBox ) 
{
    D3D9_CALL(false, "AIVolumeTextureImp9::AddDirtyBox")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

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
#include "AICubeTextureImp_9.h"
#include "../D3DControllers/Utils.h"


AICubeTextureImp9::AICubeTextureImp9(AIDeviceImp9* _i_parent, UINT _EdgeLength,
    UINT _Levels, DWORD _Usage , D3DFORMAT _Format, D3DPOOL _Pool): i_parent(_i_parent), EdgeLength(_EdgeLength),
    Levels(_Levels), Usage(_Usage), Format(_Format), Pool(_Pool) {

    // Create state
    /*state = D3DState::create_cubetexture_state_9(this);
    // Initialize state
    state->get_child(StateId("LENGTH"))->write_data(&EdgeLength);
    state->get_child(StateId("LEVELS"))->write_data(&Levels);
    state->get_child(StateId("FORMAT"))->write_data(&Format);
    state->get_child(StateId("USAGE"))->write_data(&Usage);
    state->get_child(StateId("POOL"))->write_data(&Pool);

    // Add state to state data tree
    s_parent->add_child(state);*/
    /**@note D3DControllers layer is responsible of creating
            mipmap levels state nodes. Now LEVELS contains how many
            levels have been created, and its state is accessible through
            the nodes POSITIVE_X, NEGATIVE_X, etc. This nodes have
            childs named MIPMAP indexed by level.
            ITEXTURE9 is now responsible of creating interfaces for
            the mipmap levels surfaces, and write its addresses in MIMPAP nodes
            data */
    /*D3D_DEBUG( cout << "ICUBETEXTURE9: Creating interfaces for mipmap levels." << endl; )
    UINT created_levels;
    state->get_child(StateId("LEVEL_COUNT"))->read_data(&created_levels);*/

    /* For each cubemap face gather created mipmap levels lenght and
        create corresponding interfaces. */
    /*string names[] = { "FACE_POSITIVE_X", "FACE_NEGATIVE_X",
        "FACE_POSITIVE_Y", "FACE_NEGATIVE_Y",
        "FACE_POSITIVE_Z", "FACE_NEGATIVE_Z" };
    for(UINT i = 0; i < 6; i++) {
        StateDataNode* s_face = state->get_child(StateId(names[i]));
        for(UINT j = 0; j < created_levels; j ++) {
            StateDataNode* s_mipmap = s_face->get_childs("MIPMAP")[j];
            UINT mip_length;
            s_mipmap->get_child(StateId("LENGTH"))->read_data(&mip_length);
            // Create surface for mipmap
            ISurfaceImp9* i_mipmap;
            i_mipmap = new ISurfaceImp9(state->get_parent(), i_parent, mip_length, mip_length,
                Usage, Format, Pool);
            i_mipmap->AddRef();
            // Set mipmap node data pointing to created interface
            s_mipmap->write_data(&i_mipmap);
            i_surface_childs.insert(i_mipmap);
        }
    }
    D3D_DEBUG( cout << "ICUBETEXTURE9: Created " << (int)created_levels << " levels." << endl; )*/

    //AD3D9State::instance().addCubeTexture(this);



    // Create a new ACD Texture Cube Map
    cubeTexture = AD3D9State::instance().createCubeMap();

    // Add the cube texture to the known cube textures
    //cubeTextures[tex] = acdTexture;

    //textureTextureType[(AIBaseTextureImp9*)tex] = AD3D9_CUBETEXTURE;
    AD3D9State::instance().addTypeTexture((AIBaseTextureImp9*)this, AD3D9_CUBETEXTURE);

    //  Set layout to render buffer.
    if ((Usage & D3DUSAGE_DEPTHSTENCIL) || (Usage & D3DUSAGE_RENDERTARGET))
    {
        cubeTexture->setMemoryLayout(acdlib::ACD_LAYOUT_RENDERBUFFER);
    }


    numLevels = level_count(Levels, EdgeLength, EdgeLength);

    mipLevels = new AISurfaceImp9**[6];
    
    /*UINT mip_width = EdgeLength;
    UINT mip_height = EdgeLength;*/

    for (UINT i = 0; i < 6; i++) {

        UINT mip_width = EdgeLength;
        UINT mip_height = EdgeLength;

        mipLevels[i] = new AISurfaceImp9*[numLevels];
        
        for (UINT j = 0; j < numLevels; j++) {

            AISurfaceImp9* i_mipmap;
            i_mipmap = new AISurfaceImp9(i_parent, mip_width, mip_height, Usage, Format, Pool);

            mipLevels[(D3DCUBEMAP_FACES)i][j] = i_mipmap;

            //AD3D9State::instance().addCubeTextureMipLevel(this, i_mipmap, (D3DCUBEMAP_FACES)i, j, mip_width, Format);


            // Reference to the mip level texture
            //mipLevelsCubeTextures[mip] = tex;

            // Reference to the mip level number
            //mipLevelsLevels[mip] = mipLevel;

            // Reference to the face
            //mipLevelsFaces[mip] = getACDCubeMapFace(face);
            
            //mipLevelsTextureType[mip] = AD3D9_CUBETEXTURE;

            // Get mip level size
            UINT size = getSurfaceSize(mip_width, mip_width, Format);

            //BYTE* data = new BYTE[size];

            // If isn't a compressed texture, the size passed to ACD must be 0
            if (!is_compressed(Format)) size = 0;

            // Set a void buffer to create the new mipmap in ACD
            //cubeTextures[tex]->setData(getACDCubeMapFace(face), mipLevel, width, width, getACDFormat(format), /*getSurfacePitch(width, format)*/ 0, (acdlib::acd_ubyte*)data, size);
            cubeTexture->setData(AD3D9State::instance().getACDCubeMapFace((D3DCUBEMAP_FACES)i), j, mip_width, mip_width, AD3D9State::instance().getACDFormat(Format), 0, NULL, size);

            //delete[] data;
            i_mipmap->setAsCubeTexture(this);

            i_mipmap->setMipMapLevel(j);

            i_mipmap->setFace((D3DCUBEMAP_FACES)i);



            mip_width = mip_width / 2;
            mip_height = mip_height / 2;

        }

    }

    ref_count = 0;
}

AICubeTextureImp9 :: ~AICubeTextureImp9()
{
    // Delete surface childs
    for(UINT f = 0; f < 6; f++)
        for(UINT i = 0; i < numLevels; i++)
            delete mipLevels[f][i];

    delete[] mipLevels;
}

AICubeTextureImp9 :: AICubeTextureImp9() {}

AICubeTextureImp9 & AICubeTextureImp9 :: getInstance() 
{
    static AICubeTextureImp9 instance;
    return instance;
}

acdlib::ACDTextureCubeMap* AICubeTextureImp9::getACDCubeTexture() 
{
    return cubeTexture;
}

HRESULT D3D_CALL AICubeTextureImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::QueryInterface")
    * ppvObj = cover_buffer_9;
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL AICubeTextureImp9 :: AddRef ( ) 
{
    D3D9_CALL(true, "AICubeTextureImp9::AddRef")
    
    if(i_parent != 0) 
    {
        ref_count ++;
        return ref_count;
    }
    else 
    {
        // Object is used as singleton "cover"
        return 0;
    }
}

ULONG D3D_CALL AICubeTextureImp9 :: Release ( )
{
    D3D9_CALL(true, "AICubeTextureImp9::Release")

    if(i_parent != 0)
    {
        ref_count --;
        if(ref_count == 0)
        {
            // Release mipmaps.
            for(UINT f = 0; f < 6; f++)
                for(UINT i = 0; i < numLevels; i++)
                    delete mipLevels[f][i];
            
            delete[] mipLevels;            
        }
        
        return ref_count;
    }
    else
    {
        // Object is used as singleton "cover"
        return 0;
    }
}

HRESULT D3D_CALL AICubeTextureImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GetDevice")
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL AICubeTextureImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::SetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AICubeTextureImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GetPrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AICubeTextureImp9 :: FreePrivateData (  REFGUID refguid ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::FreePrivateData")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL AICubeTextureImp9 :: SetPriority (  DWORD PriorityNew ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::SetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AICubeTextureImp9 :: GetPriority ( ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GetPriority")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL AICubeTextureImp9 :: PreLoad ( ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::PreLoad")
}

D3DRESOURCETYPE D3D_CALL AICubeTextureImp9 :: GetType ( ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GetType")
    return D3DRTYPE_CUBETEXTURE;
}

DWORD D3D_CALL AICubeTextureImp9 :: SetLOD (  DWORD LODNew ) 
{

    D3D9_CALL(true, "AICubeTextureImp9::SetLOD")

    AD3D9State::instance().setLOD((AIBaseTextureImp9*)this, LODNew);

    return D3D_OK;

}

DWORD D3D_CALL AICubeTextureImp9 :: GetLOD ( ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GetLOD")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL AICubeTextureImp9 :: GetLevelCount ( ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GetLevelCount")
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL AICubeTextureImp9 :: SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::SetAutoGenFilterType")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DTEXTUREFILTERTYPE D3D_CALL AICubeTextureImp9 :: GetAutoGenFilterType ( ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GetAutoGenFilterType")
    D3DTEXTUREFILTERTYPE ret = static_cast< D3DTEXTUREFILTERTYPE >(0);
    return ret;
}

void D3D_CALL AICubeTextureImp9 :: GenerateMipSubLevels ( ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GenerateMipSubLevels")
}

HRESULT D3D_CALL AICubeTextureImp9 :: GetLevelDesc (  UINT Level , D3DSURFACE_DESC * pDesc ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::GetLevelDesc")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL AICubeTextureImp9 :: GetCubeMapSurface (  D3DCUBEMAP_FACES FaceType , UINT Level , IDirect3DSurface9** ppCubeMapSurface ) 
{
    D3D9_CALL(true, "AICubeTextureImp9::GetCubeMapSurface")
    /*StateDataNode* s_face;
    switch (FaceType) {
        case D3DCUBEMAP_FACE_POSITIVE_X:
            s_face = state->get_child(StateId("FACE_POSITIVE_X"));
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_X:
            s_face = state->get_child(StateId("FACE_NEGATIVE_X"));
            break;
        case D3DCUBEMAP_FACE_POSITIVE_Y:
            s_face = state->get_child(StateId("FACE_POSITIVE_Y"));
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_Y:
            s_face = state->get_child(StateId("FACE_NEGATIVE_Y"));
            break;
        case D3DCUBEMAP_FACE_POSITIVE_Z:
            s_face = state->get_child(StateId("FACE_POSITIVE_Z"));
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_Z:
            s_face = state->get_child(StateId("FACE_NEGATIVE_Z"));
            break;
    }
    s_face->get_child(StateId("MIPMAP", StateIndex(Level)))->read_data(ppCubeMapSurface);
    (*ppCubeMapSurface)->AddRef();*/
    

    *ppCubeMapSurface = mipLevels[FaceType][Level];

    (*ppCubeMapSurface)->AddRef();

    return D3D_OK;
}

HRESULT D3D_CALL AICubeTextureImp9 :: LockRect (  D3DCUBEMAP_FACES FaceType , UINT Level , D3DLOCKED_RECT* pLockedRect , CONST RECT* pRect , DWORD Flags ) 
{
    //D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: LockRect  NOT IMPLEMENTED" << endl; )
    //HRESULT ret = static_cast< HRESULT >(0);
    //return ret;
    
    D3D9_CALL(true, "AICubeTextureImp9::LockRect")

    mipLevels[FaceType][Level]->LockRect(pLockedRect, pRect, Flags);;

    return D3D_OK;    
}

HRESULT D3D_CALL AICubeTextureImp9 :: UnlockRect (  D3DCUBEMAP_FACES FaceType , UINT Level ) 
{
    //D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: UnlockRect  NOT IMPLEMENTED" << endl; )
    //HRESULT ret = static_cast< HRESULT >(0);
    //return ret;

    D3D9_CALL(true, "AICubeTextureImp9::UnlockRect")

    mipLevels[FaceType][Level]->UnlockRect();

    return D3D_OK;
}

HRESULT D3D_CALL AICubeTextureImp9 :: AddDirtyRect (  D3DCUBEMAP_FACES FaceType , CONST RECT* pDirtyRect ) 
{
    D3D9_CALL(false, "AICubeTextureImp9::AddDirtyRect")
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}


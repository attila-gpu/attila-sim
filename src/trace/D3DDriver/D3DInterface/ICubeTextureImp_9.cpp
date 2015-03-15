#include "Common.h"
#include "IDeviceImp_9.h"
#include "ISurfaceImp_9.h"
#include "ICubeTextureImp_9.h"

ICubeTextureImp9::ICubeTextureImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, UINT EdgeLength,
    UINT Levels, DWORD Usage , D3DFORMAT Format, D3DPOOL Pool): i_parent(_i_parent) {

    // Create state
    state = D3DState::create_cubetexture_state_9(this);
    // Initialize state
    state->get_child(StateId(NAME_LENGTH))->write_data(&EdgeLength);
    state->get_child(StateId(NAME_LEVELS))->write_data(&Levels);
    state->get_child(StateId(NAME_FORMAT))->write_data(&Format);
    state->get_child(StateId(NAME_USAGE))->write_data(&Usage);
    state->get_child(StateId(NAME_POOL))->write_data(&Pool);

    // Add state to state data tree
    s_parent->add_child(state);
    /**@note D3DControllers layer is responsible of creating
            mipmap levels state nodes. Now LEVELS contains how many
            levels have been created, and its state is accessible through
            the nodes POSITIVE_X, NEGATIVE_X, etc. This nodes have
            childs named MIPMAP indexed by level.
            ITEXTURE9 is now responsible of creating interfaces for
            the mipmap levels surfaces, and write its addresses in MIMPAP nodes
            data */
    D3D_DEBUG( cout << "ICUBETEXTURE9: Creating interfaces for mipmap levels." << endl; )
    UINT created_levels;
    state->get_child(StateId(NAME_LEVEL_COUNT))->read_data(&created_levels);

    /* For each cubemap face gather created mipmap levels lenght and
        create corresponding interfaces. */
    StateName names[] = { NAME_FACE_POSITIVE_X, NAME_FACE_NEGATIVE_X,
                          NAME_FACE_POSITIVE_Y, NAME_FACE_NEGATIVE_Y,
                          NAME_FACE_POSITIVE_Z, NAME_FACE_NEGATIVE_Z };
    for(UINT i = 0; i < 6; i++) {
        StateDataNode* s_face = state->get_child(StateId(names[i]));
        for(UINT j = 0; j < created_levels; j ++) {
            StateDataNode* s_mipmap = s_face->get_childs(NAME_MIPMAP)[j];
            UINT mip_length;
            s_mipmap->get_child(StateId(NAME_LENGTH))->read_data(&mip_length);
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
    D3D_DEBUG( cout << "ICUBETEXTURE9: Created " << (int)created_levels << " levels." << endl; )

    ref_count = 0;
}

ICubeTextureImp9 :: ~ICubeTextureImp9() {
    // Delete surface childs
    set<ISurfaceImp9*>::iterator it_s;
    for(it_s = i_surface_childs.begin(); it_s != i_surface_childs.end(); it_s ++) {
        delete(*it_s);
    }
}

ICubeTextureImp9 :: ICubeTextureImp9() {}

ICubeTextureImp9 & ICubeTextureImp9 :: getInstance() {
    static ICubeTextureImp9 instance;
    return instance;
}

HRESULT D3D_CALL ICubeTextureImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL ICubeTextureImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"ICUBETEXTURE9: AddRef" << endl; )
    if(i_parent != 0) {
        ref_count ++;
        return ref_count;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }
}

ULONG D3D_CALL ICubeTextureImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"ICUBETEXTURE9: Release" << endl; )
    if(i_parent != 0) {
        ref_count --;
        if(ref_count == 0) {
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
        }

        return ref_count;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }
}

HRESULT D3D_CALL ICubeTextureImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    D3D_DEBUG( cout <<"ICUBETEXTURE9: GetDevice" << endl; )
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL ICubeTextureImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: SetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ICubeTextureImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: GetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ICubeTextureImp9 :: FreePrivateData (  REFGUID refguid ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: FreePrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL ICubeTextureImp9 :: SetPriority (  DWORD PriorityNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: SetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL ICubeTextureImp9 :: GetPriority ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: GetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL ICubeTextureImp9 :: PreLoad ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: PreLoad  NOT IMPLEMENTED" << endl; )
}

D3DRESOURCETYPE D3D_CALL ICubeTextureImp9 :: GetType ( ) {
    D3D_DEBUG( cout <<"ICUBETEXTURE9: GetType" << endl; )
    return D3DRTYPE_CUBETEXTURE;
}

DWORD D3D_CALL ICubeTextureImp9 :: SetLOD (  DWORD LODNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: SetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL ICubeTextureImp9 :: GetLOD ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: GetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL ICubeTextureImp9 :: GetLevelCount ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: GetLevelCount  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL ICubeTextureImp9 :: SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: SetAutoGenFilterType  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DTEXTUREFILTERTYPE D3D_CALL ICubeTextureImp9 :: GetAutoGenFilterType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: GetAutoGenFilterType  NOT IMPLEMENTED" << endl; )
    D3DTEXTUREFILTERTYPE ret = static_cast< D3DTEXTUREFILTERTYPE >(0);
    return ret;
}

void D3D_CALL ICubeTextureImp9 :: GenerateMipSubLevels ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: GenerateMipSubLevels  NOT IMPLEMENTED" << endl; )
}

HRESULT D3D_CALL ICubeTextureImp9 :: GetLevelDesc (  UINT Level , D3DSURFACE_DESC * pDesc ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: GetLevelDesc  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ICubeTextureImp9 :: GetCubeMapSurface (  D3DCUBEMAP_FACES FaceType , UINT Level , IDirect3DSurface9** ppCubeMapSurface ) {
    D3D_DEBUG( cout <<"ICUBETEXTURE9: GetCubeMapSurface" << endl; )
    StateDataNode* s_face;
    switch (FaceType) {
        case D3DCUBEMAP_FACE_POSITIVE_X:
            s_face = state->get_child(StateId(NAME_FACE_POSITIVE_X));
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_X:
            s_face = state->get_child(StateId(NAME_FACE_NEGATIVE_X));
            break;
        case D3DCUBEMAP_FACE_POSITIVE_Y:
            s_face = state->get_child(StateId(NAME_FACE_POSITIVE_Y));
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_Y:
            s_face = state->get_child(StateId(NAME_FACE_NEGATIVE_Y));
            break;
        case D3DCUBEMAP_FACE_POSITIVE_Z:
            s_face = state->get_child(StateId(NAME_FACE_POSITIVE_Z));
            break;
        case D3DCUBEMAP_FACE_NEGATIVE_Z:
            s_face = state->get_child(StateId(NAME_FACE_NEGATIVE_Z));
            break;
    }
    s_face->get_child(StateId(NAME_MIPMAP, StateIndex(Level)))->read_data(ppCubeMapSurface);
    (*ppCubeMapSurface)->AddRef();
    return D3D_OK;
}

HRESULT D3D_CALL ICubeTextureImp9 :: LockRect (  D3DCUBEMAP_FACES FaceType , UINT Level , D3DLOCKED_RECT* pLockedRect , CONST RECT* pRect , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: LockRect  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ICubeTextureImp9 :: UnlockRect (  D3DCUBEMAP_FACES FaceType , UINT Level ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: UnlockRect  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ICubeTextureImp9 :: AddDirtyRect (  D3DCUBEMAP_FACES FaceType , CONST RECT* pDirtyRect ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DCubeTexture9 :: AddDirtyRect  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}


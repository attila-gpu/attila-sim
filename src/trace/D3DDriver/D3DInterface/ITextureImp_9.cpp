#include "Common.h"
#include "IDeviceImp_9.h"
#include "ISurfaceImp_9.h"
#include "ITextureImp_9.h"

ITextureImp9::ITextureImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, UINT Width, UINT Height,
    UINT Levels, DWORD Usage , D3DFORMAT Format, D3DPOOL Pool): i_parent(_i_parent) {
    // Create state
    state = D3DState::create_texture_state_9(this);
    // Initialize state
    state->get_child(StateId(NAME_WIDTH))->write_data(&Width);
    state->get_child(StateId(NAME_HEIGHT))->write_data(&Height);
    state->get_child(StateId(NAME_LEVELS))->write_data(&Levels);
    state->get_child(StateId(NAME_FORMAT))->write_data(&Format);
    state->get_child(StateId(NAME_USAGE))->write_data(&Usage);
    state->get_child(StateId(NAME_POOL))->write_data(&Pool);
    // Add state to state data tree
    s_parent->add_child(state);
    /**@note D3DControllers layer is responsible of creating
            mipmap levels state nodes. Now LEVELS contains how many
            levels have been created, and its state is
            in childs named MIPMAP indexed by level.
            ITEXTURE9 is then responsible of creating interfaces for
            the mipmap levels surfaces, and write its addresses in MIMPAP nodes
            data */
    D3D_DEBUG( cout << "ITEXTURE9: Creating interfaces for mipmap levels." << endl; )
    UINT created_levels;
    state->get_child(StateId(NAME_LEVEL_COUNT))->read_data(&created_levels);
    /* Gather created mipmap levels width and height and
        create corresponding interfaces. */
    for(UINT i = 0; i < created_levels; i ++) {
        StateDataNode* s_mipmap = state->get_childs(NAME_MIPMAP)[i];
        UINT mip_width;
        UINT mip_height;
        s_mipmap->get_child(StateId(NAME_WIDTH))->read_data(&mip_width);
        s_mipmap->get_child(StateId(NAME_HEIGHT))->read_data(&mip_height);
        // Create surface for mipmap
        ISurfaceImp9* i_mipmap;
        i_mipmap = new ISurfaceImp9(state->get_parent(), i_parent, mip_width, mip_height,
            Usage, Format, Pool);
        i_mipmap->AddRef();
        // Set mipmap node data pointing to created interface
        s_mipmap->write_data(&i_mipmap);
        i_surface_childs.insert(i_mipmap);
    }
    D3D_DEBUG( cout << "ITEXTURE9: Created " << (int)created_levels << " levels." << endl; )

    ref_count = 0;
}

ITextureImp9 :: ~ITextureImp9() {
    // Delete surface childs
    set<ISurfaceImp9*>::iterator it_s;
    for(it_s = i_surface_childs.begin(); it_s != i_surface_childs.end(); it_s ++) {
        delete(*it_s);
    }
}

ITextureImp9 :: ITextureImp9() {}

ITextureImp9 & ITextureImp9 :: getInstance() {
    static ITextureImp9 instance;
    return instance;
}

HRESULT D3D_CALL ITextureImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL ITextureImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"ITEXTURE9: AddRef" << endl; )
    if(i_parent != 0) {
        ref_count++;
        return ref_count;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }
}

ULONG D3D_CALL ITextureImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"ITEXTURE9: Release" << endl; )
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

HRESULT D3D_CALL ITextureImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    D3D_DEBUG( cout <<"ITEXTURE9: GetDevice" << endl; )
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL ITextureImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: SetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ITextureImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: GetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ITextureImp9 :: FreePrivateData (  REFGUID refguid ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: FreePrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL ITextureImp9 :: SetPriority (  DWORD PriorityNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: SetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL ITextureImp9 :: GetPriority ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: GetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL ITextureImp9 :: PreLoad ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: PreLoad  NOT IMPLEMENTED" << endl; )
}

D3DRESOURCETYPE D3D_CALL ITextureImp9 :: GetType ( ) {
    D3D_DEBUG( cout <<"ITEXTURE9 :: GetType" << endl; )
    return D3DRTYPE_TEXTURE;
}

DWORD D3D_CALL ITextureImp9 :: SetLOD (  DWORD LODNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: SetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL ITextureImp9 :: GetLOD ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: GetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL ITextureImp9 :: GetLevelCount ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: GetLevelCount  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL ITextureImp9 :: SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: SetAutoGenFilterType  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DTEXTUREFILTERTYPE D3D_CALL ITextureImp9 :: GetAutoGenFilterType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: GetAutoGenFilterType  NOT IMPLEMENTED" << endl; )
    D3DTEXTUREFILTERTYPE ret = static_cast< D3DTEXTUREFILTERTYPE >(0);
   return ret;
}

void D3D_CALL ITextureImp9 :: GenerateMipSubLevels ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: GenerateMipSubLevels  NOT IMPLEMENTED" << endl; )
}

HRESULT D3D_CALL ITextureImp9 :: GetLevelDesc (  UINT Level , D3DSURFACE_DESC * pDesc ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: GetLevelDesc  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ITextureImp9 :: GetSurfaceLevel (  UINT Level , IDirect3DSurface9** ppSurfaceLevel ) {
    D3D_DEBUG( cout <<"ITEXTURE9: GetSurfaceLevel" << endl; )
    state->get_child(StateId(NAME_MIPMAP, StateIndex(Level)))->read_data(ppSurfaceLevel);
    (*ppSurfaceLevel)->AddRef();
    return D3D_OK;
}

HRESULT D3D_CALL ITextureImp9 :: LockRect (  UINT Level , D3DLOCKED_RECT* pLockedRect , CONST RECT* pRect , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: LockRect  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ITextureImp9 :: UnlockRect (  UINT Level ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: UnlockRect  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ITextureImp9 :: AddDirtyRect (  CONST RECT* pDirtyRect ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DTexture9 :: AddDirtyRect  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

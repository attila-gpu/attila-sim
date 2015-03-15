#include "Common.h"
#include "IDeviceImp_9.h"
#include "IVolumeImp_9.h"
#include "IVolumeTextureImp_9.h"

IVolumeTextureImp9::IVolumeTextureImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, UINT Width, UINT Height, UINT Depth,
    UINT Levels, DWORD Usage , D3DFORMAT Format, D3DPOOL Pool) {

    i_parent = _i_parent;
    // Create state
    state = D3DState::create_volumetexture_state_9(this);
    // Initialize state
    state->get_child(StateId(NAME_WIDTH))->write_data(&Width);
    state->get_child(StateId(NAME_HEIGHT))->write_data(&Height);
    state->get_child(StateId(NAME_DEPTH))->write_data(&Depth);
    state->get_child(StateId(NAME_LEVELS))->write_data(&Levels);
    state->get_child(StateId(NAME_FORMAT))->write_data(&Format);
    state->get_child(StateId(NAME_USAGE))->write_data(&Usage);
    state->get_child(StateId(NAME_POOL))->write_data(&Pool);
    // Add state to state data tree
    s_parent->add_child(state);
    /**@note See explanation of the protocol between interface
             and controllers at ITextureImp_9 constructor */
    D3D_DEBUG( cout << "IVOLUMETEXTURE9: Creating interfaces for mipmap levels." << endl; )
    UINT created_levels;
    state->get_child(StateId(NAME_LEVEL_COUNT))->read_data(&created_levels);
    /* Gather created mipmap levels width, height and depth and
        create corresponding interfaces. */
    for(UINT i = 0; i < created_levels; i ++) {
        StateDataNode* s_mipmap = state->get_childs(NAME_MIPMAP)[i];
        UINT mip_width;
        UINT mip_height;
        UINT mip_depth;
        s_mipmap->get_child(StateId(NAME_WIDTH))->read_data(&mip_width);
        s_mipmap->get_child(StateId(NAME_HEIGHT))->read_data(&mip_height);
        s_mipmap->get_child(StateId(NAME_DEPTH))->read_data(&mip_depth);
        // Create surface for mipmap
        IVolumeImp9* i_mipmap;
        i_mipmap = new IVolumeImp9(state->get_parent(), i_parent, mip_width, mip_height,
            mip_depth, Usage, Format, Pool);
        // Set mipmap node data pointing to created interface
        s_mipmap->write_data(&i_mipmap);
        i_volume_childs.insert(i_mipmap);
    }
    D3D_DEBUG( cout << "IVOLUMETEXTURE9: Created " << (int)created_levels << " levels." << endl; )
}

IVolumeTextureImp9 ::~IVolumeTextureImp9() {
    // Delete volume childs
    set<IVolumeImp9*>::iterator it_v;
    for(it_v = i_volume_childs.begin(); it_v != i_volume_childs.end(); it_v ++) {
        delete(*it_v);
    }
}

IVolumeTextureImp9 :: IVolumeTextureImp9() {}

IVolumeTextureImp9 & IVolumeTextureImp9 :: getInstance() {
    static IVolumeTextureImp9 instance;
    return instance;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IVolumeTextureImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL IVolumeTextureImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    D3D_DEBUG( cout <<"IVOLUMETEXTURE9: GetDevice" << endl; )
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: SetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: GetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: FreePrivateData (  REFGUID refguid ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: FreePrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL IVolumeTextureImp9 :: SetPriority (  DWORD PriorityNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: SetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL IVolumeTextureImp9 :: GetPriority ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: GetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL IVolumeTextureImp9 :: PreLoad ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: PreLoad  NOT IMPLEMENTED" << endl; )
}

D3DRESOURCETYPE D3D_CALL IVolumeTextureImp9 :: GetType ( ) {
    D3D_DEBUG( cout <<"IVOLUMETEXTURE9: GetType" << endl; )
    return D3DRTYPE_VOLUMETEXTURE;

}

DWORD D3D_CALL IVolumeTextureImp9 :: SetLOD (  DWORD LODNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: SetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL IVolumeTextureImp9 :: GetLOD ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: GetLOD  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL IVolumeTextureImp9 :: GetLevelCount ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: GetLevelCount  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: SetAutoGenFilterType (  D3DTEXTUREFILTERTYPE FilterType ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: SetAutoGenFilterType  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

D3DTEXTUREFILTERTYPE D3D_CALL IVolumeTextureImp9 :: GetAutoGenFilterType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: GetAutoGenFilterType  NOT IMPLEMENTED" << endl; )
    D3DTEXTUREFILTERTYPE ret = static_cast< D3DTEXTUREFILTERTYPE >(0);
    return ret;
}

void D3D_CALL IVolumeTextureImp9 :: GenerateMipSubLevels ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: GenerateMipSubLevels  NOT IMPLEMENTED" << endl; )
}

HRESULT D3D_CALL IVolumeTextureImp9 :: GetLevelDesc (  UINT Level , D3DVOLUME_DESC * pDesc ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: GetLevelDesc  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: GetVolumeLevel (  UINT Level , IDirect3DVolume9** ppVolumeLevel ) {
    D3D_DEBUG( cout <<"IVOLUMETEXTURE9: GetVolumeLevel" << endl; )
    state->get_child(StateId(NAME_MIPMAP, StateIndex(Level)))->read_data(ppVolumeLevel);
    return D3D_OK;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: LockBox (  UINT Level , D3DLOCKED_BOX* pLockedVolume , CONST D3DBOX* pBox , DWORD Flags ) {
    pLockedVolume ->RowPitch = 1;
    pLockedVolume ->SlicePitch = 1;
    pLockedVolume ->pBits = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: LockBox  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: UnlockBox (  UINT Level ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: UnlockBox  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeTextureImp9 :: AddDirtyBox (  CONST D3DBOX* pDirtyBox ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolumeTexture9 :: AddDirtyBox  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

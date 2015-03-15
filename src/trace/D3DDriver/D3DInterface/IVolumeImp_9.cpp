#include "Common.h"
#include "IDeviceImp_9.h"
#include "IVolumeImp_9.h"


IVolumeImp9::IVolumeImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, UINT Width, UINT Height, UINT Depth,
    DWORD Usage , D3DFORMAT Format, D3DPOOL Pool) {
    i_parent = _i_parent;
    // Create state
    state = D3DState::create_volume_state_9(this);
    // Fill state with parameters
    D3DRESOURCETYPE type = D3DRTYPE_VOLUME;
    state->get_child(StateId(NAME_TYPE))->write_data(&type);
    state->get_child(StateId(NAME_WIDTH))->write_data(&Width);
    state->get_child(StateId(NAME_HEIGHT))->write_data(&Height);
    state->get_child(StateId(NAME_DEPTH))->write_data(&Depth);
    state->get_child(StateId(NAME_FORMAT))->write_data(&Format);
    state->get_child(StateId(NAME_USAGE))->write_data(&Usage);
    state->get_child(StateId(NAME_POOL))->write_data(&Pool);

    // Add state
    s_parent->add_child(state);
}

IVolumeImp9 :: IVolumeImp9() {
    ///@note used to differentiate when creating singleton
    i_parent = 0;
}

IVolumeImp9 & IVolumeImp9 :: getInstance() {
    static IVolumeImp9 instance;
    return instance;
}

HRESULT D3D_CALL IVolumeImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IVolumeImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL IVolumeImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    ///@note Depending on the constructor used, parent can be null.
    D3D_DEBUG( cout <<"IVOLUME9 :: GetDevice" << endl; )
    *ppDevice = (i_parent != 0)? i_parent : &IDeviceImp9::getInstance();
    return D3D_OK;
}

HRESULT D3D_CALL IVolumeImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: SetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: GetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeImp9 :: FreePrivateData (  REFGUID refguid ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: FreePrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeImp9 :: GetContainer (  REFIID riid , void** ppContainer ) {
    * ppContainer = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVolume9 :: GetContainer  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IVolumeImp9 :: GetDesc (  D3DVOLUME_DESC * pDesc ) {
    // Surface is being used as a cover, so do nothing
    if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout <<"IVOLUME: GetDesc" << endl; )
    state->get_child(StateId(NAME_FORMAT))->read_data(&pDesc->Format);
    state->get_child(StateId(NAME_TYPE))->read_data(&pDesc->Type);
    state->get_child(StateId(NAME_USAGE))->read_data(&pDesc->Usage);
    state->get_child(StateId(NAME_POOL))->read_data(&pDesc->Pool);
    state->get_child(StateId(NAME_WIDTH))->read_data(&pDesc->Width);
    state->get_child(StateId(NAME_HEIGHT))->read_data(&pDesc->Height);
    state->get_child(StateId(NAME_DEPTH))->read_data(&pDesc->Depth);

    return D3D_OK;
}

HRESULT D3D_CALL IVolumeImp9 :: LockBox (  D3DLOCKED_BOX * pLockedVolume , CONST D3DBOX* pBox , DWORD Flags ) {
    // Surface is being used as a cover, so do nothing
    if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout << "IVOLUME9<" << this << ">: LockVolume" << endl; )

    ///@todo Study possible problems of discard or readonly flags

    if(Flags & D3DLOCK_DISCARD) {
        D3D_DEBUG( cout << "WARNING: D3DLOCK_DISCARD NOT IMPLEMENTED" << endl; )
    }

    UINT lock_width;
    UINT lock_height;
    UINT lock_depth;
    UINT lock_offset_x;
    UINT lock_offset_y;
    UINT lock_offset_z;


    if (pBox == 0) {
        // Remember a pBox of 0 means lock entire volume.
        state->get_child(StateId(NAME_WIDTH))->read_data(&lock_width);
        state->get_child(StateId(NAME_HEIGHT))->read_data(&lock_height);
        state->get_child(StateId(NAME_DEPTH))->read_data(&lock_depth);
        lock_offset_x = 0;
        lock_offset_y = 0;
        lock_offset_z = 0;
    }
    else {
        lock_width = pBox->Right - pBox->Left;
        lock_height = pBox->Bottom - pBox->Top;
        lock_depth = pBox->Back - pBox->Front;
        lock_offset_x = pBox->Left;
        lock_offset_y = pBox->Top;
        lock_offset_z = pBox->Front;
    }

    // Update lock state
    StateDataNode* lock_state = D3DState::create_lockbox_state_9();
    lock_state->get_child(StateId(NAME_LEFT))->write_data(&lock_offset_x);
    lock_state->get_child(StateId(NAME_TOP))->write_data(&lock_offset_y);
    lock_state->get_child(StateId(NAME_FRONT))->write_data(&lock_offset_z);
    LONG lock_right = lock_offset_x + lock_width;
    lock_state->get_child(StateId(NAME_RIGHT))->write_data(&lock_right);
    LONG lock_bottom = lock_offset_y + lock_height;
    lock_state->get_child(StateId(NAME_BOTTOM))->write_data(&lock_bottom);
    LONG lock_back = lock_offset_z + lock_depth;
    lock_state->get_child(StateId(NAME_BACK))->write_data(&lock_back);
    lock_state->get_child(StateId(NAME_FLAGS))->write_data(&Flags);

    // add lock state
    state->add_child(lock_state);

    /* Now controllers layer has reserved memory in DATA node
       and has written ROW_PITCH and SLICE_PITCH. */

    lock_state->get_child(StateId(NAME_ROW_PITCH))->read_data(&pLockedVolume->RowPitch);
    lock_state->get_child(StateId(NAME_SLICE_PITCH))->read_data(&pLockedVolume->SlicePitch);
    // Map data node to allow user application to modify the contents
    StateDataNode* data_node = lock_state->get_child(StateId(NAME_DATA));
    if(Flags & D3DLOCK_READONLY) {
        pLockedVolume->pBits = data_node->map_data(READ_ACCESS);
    }
    else {
        pLockedVolume->pBits = data_node->map_data(WRITE_ACCESS);
    }

    return D3D_OK;
}

HRESULT D3D_CALL IVolumeImp9 :: UnlockBox ( ) {
     // Surface is being used as a cover, so do nothing
    if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout << "IVOLUME9<" << this << ">: UnlockBox" << endl; )

    // Application has finished accessing data.
    StateDataNode* lock_state = state->get_child(StateId(NAME_LOCKBOX));
    lock_state->get_child(StateId(NAME_DATA))->unmap_data();

    // Remove lock state
    state->remove_child(lock_state);

    /** @todo Lock State is not deleted! At this point of
        development deletion policy is not decided. Revisit this code then. */

    return D3D_OK;
}


DWORD IVolumeImp9 :: SetPriority(DWORD) {
    D3D_DEBUG( cout << "WARNING: IDirect3DVolume9 :: SetPriority NOT IMPLEMENTED" << endl; )
    return static_cast<DWORD>(0);
}

DWORD IVolumeImp9 :: GetPriority() {
    D3D_DEBUG( cout << "WARNING: IDirect3DVolume9 :: GetPriority NOT IMPLEMENTED" << endl; )
    return static_cast<DWORD>(0);
}

void IVolumeImp9 :: PreLoad() {
    D3D_DEBUG( cout << "WARNING: IDirect3DVolume9 :: PreLoad NOT IMPLEMENTED" << endl; )
}

D3DRESOURCETYPE IVolumeImp9 :: GetType() {
    D3D_DEBUG( cout << "WARNING: IDirect3DVolume9 :: GetType NOT IMPLEMENTED" << endl; )
    return static_cast<D3DRESOURCETYPE>(0);
}


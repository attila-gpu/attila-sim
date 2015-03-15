#include "Common.h"
#include "IDeviceImp_9.h"
#include "ISurfaceImp_9.h"

ISurfaceImp9::ISurfaceImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, UINT Width, UINT Height,
    DWORD Usage , D3DFORMAT Format, D3DPOOL Pool) {
    i_parent = _i_parent;
    // Create state
    state = D3DState::create_surface_state_9(this);
    // Fill state with parameters
    D3DRESOURCETYPE type = D3DRTYPE_SURFACE;
    state->get_child(StateId(NAME_TYPE))->write_data(&type);
    state->get_child(StateId(NAME_WIDTH))->write_data(&Width);
    state->get_child(StateId(NAME_HEIGHT))->write_data(&Height);
    state->get_child(StateId(NAME_FORMAT))->write_data(&Format);
    state->get_child(StateId(NAME_USAGE))->write_data(&Usage);
    state->get_child(StateId(NAME_POOL))->write_data(&Pool);

    // Add state
    s_parent->add_child(state);

    ref_count = 0;
}

ISurfaceImp9 :: ISurfaceImp9() {
    ///@note used to differentiate when creating singleton
    i_parent = 0;
}

ISurfaceImp9 & ISurfaceImp9 :: getInstance() {
    static ISurfaceImp9 instance;
    return instance;
}

HRESULT D3D_CALL ISurfaceImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL ISurfaceImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"ISURFACE9: AddRef" << endl; )
    if(i_parent != 0) {
        ref_count ++;
        return ref_count;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }
}

ULONG D3D_CALL ISurfaceImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"ISURFACE9: Release" << endl; )
    if(i_parent != 0) {
        ref_count--;
        if(ref_count == 0) {
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

HRESULT D3D_CALL ISurfaceImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    ///@note Depending on the constructor used, parent can be null.
    D3D_DEBUG( cout <<"ISURFACE9 :: GetDevice" << endl; )
    *ppDevice = (i_parent != 0)? i_parent : &IDeviceImp9::getInstance();
    return D3D_OK;
}

HRESULT D3D_CALL ISurfaceImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: SetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISurfaceImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: GetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISurfaceImp9 :: FreePrivateData (  REFGUID refguid ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: FreePrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL ISurfaceImp9 :: SetPriority (  DWORD PriorityNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: SetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL ISurfaceImp9 :: GetPriority ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: GetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL ISurfaceImp9 :: PreLoad ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: PreLoad  NOT IMPLEMENTED" << endl; )
}

D3DRESOURCETYPE D3D_CALL ISurfaceImp9 :: GetType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: GetType  NOT IMPLEMENTED" << endl; )
    D3DRESOURCETYPE ret = static_cast< D3DRESOURCETYPE >(0);
    return ret;
}

HRESULT D3D_CALL ISurfaceImp9 :: GetContainer (  REFIID riid , void** ppContainer ) {
    * ppContainer = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: GetContainer  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISurfaceImp9 :: GetDesc (  D3DSURFACE_DESC * pDesc ) {
    // Surface is being used as a cover, so do nothing
    if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout <<"ISURFACE9: GetDesc" << endl; )
    state->get_child(StateId(NAME_FORMAT))->read_data(&pDesc->Format);
    state->get_child(StateId(NAME_TYPE))->read_data(&pDesc->Type);
    state->get_child(StateId(NAME_USAGE))->read_data(&pDesc->Usage);
    state->get_child(StateId(NAME_POOL))->read_data(&pDesc->Pool);
    /// @note  Multisample info is not available
    state->get_child(StateId(NAME_WIDTH))->read_data(&pDesc->Width);
    state->get_child(StateId(NAME_HEIGHT))->read_data(&pDesc->Height);

    return D3D_OK;
}

HRESULT D3D_CALL ISurfaceImp9 :: LockRect (  D3DLOCKED_RECT* pLockedRect , CONST RECT* pRect , DWORD Flags ) {
    // Surface is being used as a cover, so do nothing
    if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: LockRect" << endl; )

    ///@todo Study possible problems of discard or readonly flags

    if(Flags & D3DLOCK_DISCARD) {
        D3D_DEBUG( cout << "WARNING: D3DLOCK_DISCARD NOT IMPLEMENTED" << endl; )
    }

    LONG lock_width;
    LONG lock_height;
    LONG lock_offset_x;
    LONG lock_offset_y;


    if (pRect == 0) {
        // Remember a pRect of 0 means lock entire surface.
        state->get_child(StateId(NAME_WIDTH))->read_data(&lock_width);
        state->get_child(StateId(NAME_HEIGHT))->read_data(&lock_height);
        lock_offset_x = 0;
        lock_offset_y = 0;
    }
    else {
        lock_width = pRect->right - pRect->left;
        lock_height = pRect->bottom - pRect->top;
        lock_offset_x = pRect->left;
        lock_offset_y = pRect->top;
    }

    // Update lock state
    StateDataNode* lock_state = D3DState::create_lockrect_state_9();
    lock_state->get_child(StateId(NAME_LEFT))->write_data(&lock_offset_x);
    lock_state->get_child(StateId(NAME_TOP))->write_data(&lock_offset_y);
    LONG lock_right = lock_offset_x + lock_width;
    lock_state->get_child(StateId(NAME_RIGHT))->write_data(&lock_right);
    LONG lock_bottom = lock_offset_y + lock_height;
    lock_state->get_child(StateId(NAME_BOTTOM))->write_data(&lock_bottom);
    lock_state->get_child(StateId(NAME_FLAGS))->write_data(&Flags);

    // add lock state
    state->add_child(lock_state);

    /* Now controllers layer has reserved memory in DATA node
       and has written PITCH. */

    lock_state->get_child(StateId(NAME_PITCH))->read_data(&pLockedRect->Pitch);
    // Map data node to allow user application to modify the contents
    StateDataNode* data_node = lock_state->get_child(StateId(NAME_DATA));
    if(Flags & D3DLOCK_READONLY) {
        pLockedRect->pBits = data_node->map_data(READ_ACCESS);
    }
    else {
        pLockedRect->pBits = data_node->map_data(WRITE_ACCESS);
    }

    return D3D_OK;
}

HRESULT D3D_CALL ISurfaceImp9 :: UnlockRect ( ) {
     // Surface is being used as a cover, so do nothing
    if(i_parent == 0) return D3D_OK;

    D3D_DEBUG( cout << "ISURFACE9<" << this << ">: UnlockRect" << endl; )

    // Application has finished accessing data.
    StateDataNode* lock_state = state->get_child(StateId(NAME_LOCKRECT));
    lock_state->get_child(StateId(NAME_DATA))->unmap_data();

    // Remove lock state
    state->remove_child(lock_state);
    delete lock_state;

    return D3D_OK;
}

HRESULT D3D_CALL ISurfaceImp9 :: GetDC (  HDC * phdc ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: GetDC  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL ISurfaceImp9 :: ReleaseDC (  HDC hdc ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DSurface9 :: ReleaseDC  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}


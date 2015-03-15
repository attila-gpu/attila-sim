#include "Common.h"
#include "IDeviceImp_9.h"
#include "IIndexBufferImp_9.h"


IIndexBufferImp9::IIndexBufferImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, UINT Length , DWORD Usage , DWORD Format, D3DPOOL Pool):
i_parent(_i_parent) {

    // Create state
    state = D3DState::create_index_buffer_state_9(this);

    // Initialize values
    D3DRESOURCETYPE t = D3DRTYPE_INDEXBUFFER;
    state->get_child(StateId(NAME_TYPE))->write_data(&t);

    state->get_child(StateId(NAME_LENGTH))->write_data(&Length);
    state->get_child(StateId(NAME_USAGE))->write_data(&Usage);
    state->get_child(StateId(NAME_FORMAT))->write_data(&Format);
    state->get_child(StateId(NAME_POOL))->write_data(&Pool);

    // Add to state data tree
    s_parent->add_child(state);

    refs = 0;

}

IIndexBufferImp9 :: IIndexBufferImp9() {
	// Used to diferentiate when creating singleton cover object
	i_parent = 0;
}

IIndexBufferImp9 & IIndexBufferImp9 :: getInstance() {
    static IIndexBufferImp9 instance;
    return instance;
}

HRESULT D3D_CALL IIndexBufferImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DIndexBuffer9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IIndexBufferImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"IINDEXBUFFER9: AddRef" << endl; )
	if(i_parent != 0) {
		refs ++;
    	return refs;
	}
	else return 0;
}

ULONG D3D_CALL IIndexBufferImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"IINDEXBUFFER9: Release" << endl; )
    if(i_parent != 0) {
        refs--;
        if(refs == 0) {
            // Remove state
            StateDataNode* parent = state->get_parent();
            parent->remove_child(state);
            delete state;
            state = 0;
        }
        return refs;
    }
    else {
        // Object is used as singleton "cover"
        return 0;
    }

}

HRESULT D3D_CALL IIndexBufferImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    D3D_DEBUG( cout <<"IINDEXBUFFER9: GetDevice" << endl; )
    * ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL IIndexBufferImp9 :: SetPrivateData (  REFGUID refguid , CONST void* pData , DWORD SizeOfData , DWORD Flags ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DIndexBuffer9 :: SetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IIndexBufferImp9 :: GetPrivateData (  REFGUID refguid , void* pData , DWORD* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DIndexBuffer9 :: GetPrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

HRESULT D3D_CALL IIndexBufferImp9 :: FreePrivateData (  REFGUID refguid ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DIndexBuffer9 :: FreePrivateData  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

DWORD D3D_CALL IIndexBufferImp9 :: SetPriority (  DWORD PriorityNew ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DIndexBuffer9 :: SetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

DWORD D3D_CALL IIndexBufferImp9 :: GetPriority ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DIndexBuffer9 :: GetPriority  NOT IMPLEMENTED" << endl; )
    DWORD ret = static_cast< DWORD >(0);
    return ret;
}

void D3D_CALL IIndexBufferImp9 :: PreLoad ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DIndexBuffer9 :: PreLoad  NOT IMPLEMENTED" << endl; )
}

D3DRESOURCETYPE D3D_CALL IIndexBufferImp9 :: GetType ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DIndexBuffer9 :: GetType  NOT IMPLEMENTED" << endl; )
    D3DRESOURCETYPE ret = static_cast< D3DRESOURCETYPE >(0);
    return ret;
}

HRESULT D3D_CALL IIndexBufferImp9 :: Lock (  UINT OffsetToLock , UINT SizeToLock , void** ppbData , DWORD Flags ) {
    D3D_DEBUG( cout << "IINDEXBUFFER9<" << this << ">: Lock " << SizeToLock << " bytes at " << OffsetToLock << endl; )

    ///@todo Study possible problems of discard or readonly flags
    if(Flags & D3DLOCK_DISCARD) {
        D3D_DEBUG( cout << "WARNING: D3DLOCK_DISCARD NOT IMPLEMENTED" << endl; )
    }

    // Multiple locks can be done, so lock count is used to identify each one.
    UINT count;
    state->get_child(StateId(NAME_LOCK_COUNT))->read_data(&count);

    UINT actual_lock_size;
    if (SizeToLock == 0) {
        // Remember a SizeToLock of 0 means lock entire vertex buffer.
        state->get_child(StateId(NAME_LENGTH))->read_data(&actual_lock_size);
    }
    else {
        actual_lock_size = SizeToLock;
    }

    // Update lock state
    StateDataNode* lock_state = D3DState::create_lock_state_9(count, actual_lock_size);
    lock_state->get_child(StateId(NAME_OFFSET))->write_data(&OffsetToLock);
    lock_state->get_child(StateId(NAME_SIZE))->write_data(&actual_lock_size);
    lock_state->get_child(StateId(NAME_FLAGS))->write_data(&Flags);

    StateDataNode* data_node = lock_state->get_child(StateId(NAME_DATA));
    if(Flags & D3DLOCK_READONLY) {
        * ppbData = data_node->map_data(READ_ACCESS);
    }
    else {
        * ppbData = data_node->map_data(WRITE_ACCESS);
    }

    // Update lock count
    count ++;
    state->get_child(StateId(NAME_LOCK_COUNT))->write_data(&count);

    // Finally add state
    state->add_child(lock_state);

    return D3D_OK;
}

HRESULT D3D_CALL IIndexBufferImp9 :: Unlock ( ) {
    D3D_DEBUG( cout << "IINDEXBUFFER9<" << this << ">: Unlock" << endl; )

    // Multiple locks can be done, so lock count is used to identify each one.
    UINT count;
    state->get_child(StateId(NAME_LOCK_COUNT))->read_data(&count);

    // Locks are accessed like a stack, so "pop" the last one.
    StateDataNode* lock_state = state->get_child(StateId(NAME_LOCK, StateIndex(count - 1)));

    // Application has finished accessing data.
    lock_state->get_child(StateId(NAME_DATA))->unmap_data();

    // Remove lock state, updating count
    count--;
    state->get_child(StateId(NAME_LOCK_COUNT))->write_data(&count);


    D3D_DEBUG( cout << "IINDEXBUFFER9<" << this << ">: Removing SLOCK" << endl; )
    state->remove_child(lock_state);

    /** @todo Lock State is not deleted! At this point of
        development deletion policy is not decided. Revisit this code then. */

    return D3D_OK;
}

HRESULT D3D_CALL IIndexBufferImp9 :: GetDesc (  D3DINDEXBUFFER_DESC * pDesc ) {
    /** @note This "Get" method is necessary for the d3d player, because it doesn't keep
        track of indexbuffers size. */
    D3D_DEBUG( cout <<"IINDEXBUFFER9: GetDesc" << endl; )
    state->get_child(StateId(NAME_LENGTH))->read_data(&pDesc->Size);
    D3D_DEBUG( cout << "IINDEXBUFFER9: Size is " << pDesc->Size << endl; )
    return D3D_OK;
}


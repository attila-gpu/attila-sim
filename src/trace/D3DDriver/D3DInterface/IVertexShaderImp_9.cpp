#include "Common.h"
#include "IDeviceImp_9.h"
#include "IVertexShaderImp_9.h"

IVertexShaderImp9 :: IVertexShaderImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, CONST DWORD* pFunction):
i_parent(_i_parent) {


    // Get lenght of shader
    /// @todo this is not a task for the interface, find an alternative place for this code
    int i = 0;
    while((pFunction[i] & D3DSI_OPCODE_MASK) != D3DSIO_END) {
        i++;
    }
    UINT token_count = i + 1;
    UINT size = sizeof(DWORD) * token_count;

    // Create state
    state = D3DState::create_vertex_shader_state_9(this, size);
    //Update state
    void* data = state->get_child(StateId(NAME_CODE))->map_data();
    DWORD* tokens = static_cast<DWORD*>(data);
    for(UINT i = 0; i < token_count; i++) {
        tokens[i] = pFunction[i];
    }
    state->get_child(StateId(NAME_CODE))->unmap_data();
    // Add to state data tree
    s_parent->add_child(state);

    refs = 0;
}


IVertexShaderImp9 :: IVertexShaderImp9() {
	///@note Used to differentiate singleton cover
	i_parent = 0;
}

IVertexShaderImp9 & IVertexShaderImp9 :: getInstance() {
    static IVertexShaderImp9 instance;
    return instance;
}


HRESULT D3D_CALL IVertexShaderImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVertexShader9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IVertexShaderImp9 :: AddRef ( ) {
	D3D_DEBUG( cout << "IVERTEXSHADER9: AddRef" << endl; )
	if(i_parent != 0) {
		refs ++;
    	return refs;
	}
	else return 0;
}

ULONG D3D_CALL IVertexShaderImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"IVERTEXSHADER9: Release" << endl; )
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

HRESULT D3D_CALL IVertexShaderImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    D3D_DEBUG( cout <<"IVERTEXSHADER:  GetDevice" << endl; )
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL IVertexShaderImp9 :: GetFunction (  void* pData , UINT* pSizeOfData ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVertexShader9 :: GetFunction  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}



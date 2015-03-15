#include "Common.h"
#include "IDeviceImp_9.h"
#include "IVertexDeclarationImp_9.h"

IVertexDeclarationImp9::IVertexDeclarationImp9(StateDataNode* s_parent, IDeviceImp9* _i_parent, CONST D3DVERTEXELEMENT9* elements):
i_parent(_i_parent) {

    StateDataNode* state = D3DState::create_vertex_declaration_state_9(this);

    // add elements
    D3DVERTEXELEMENT9 end = D3DDECL_END();
    UINT count = 0;
    while(elements[count].Stream != end.Stream) {
        StateDataNode* velem_state = D3DState::create_vertex_element_state_9(count);
        velem_state->get_child(StateId(NAME_STREAM))->write_data(&elements[count].Stream);
        velem_state->get_child(StateId(NAME_OFFSET))->write_data(&elements[count].Offset);
        velem_state->get_child(StateId(NAME_TYPE))->write_data(&elements[count].Type);
        velem_state->get_child(StateId(NAME_METHOD))->write_data(&elements[count].Method);
        velem_state->get_child(StateId(NAME_USAGE))->write_data(&elements[count].Usage);
        velem_state->get_child(StateId(NAME_USAGE_INDEX))->write_data(&elements[count].UsageIndex);
        state->add_child(velem_state);
        count ++;
    }

    state->get_child(StateId(NAME_ELEMENT_COUNT))->write_data(&count);

    s_parent->add_child(state);

}

IVertexDeclarationImp9 :: IVertexDeclarationImp9() {}

IVertexDeclarationImp9 & IVertexDeclarationImp9 :: getInstance() {
    static IVertexDeclarationImp9 instance;
    return instance;
}

HRESULT D3D_CALL IVertexDeclarationImp9 :: QueryInterface (  REFIID riid , void** ppvObj ) {
    * ppvObj = cover_buffer_9;
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVertexDeclaration9 :: QueryInterface  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

ULONG D3D_CALL IVertexDeclarationImp9 :: AddRef ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVertexDeclaration9 :: AddRef  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

ULONG D3D_CALL IVertexDeclarationImp9 :: Release ( ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVertexDeclaration9 :: Release  NOT IMPLEMENTED" << endl; )
    ULONG ret = static_cast< ULONG >(0);
    return ret;
}

HRESULT D3D_CALL IVertexDeclarationImp9 :: GetDevice (  IDirect3DDevice9** ppDevice ) {
    D3D_DEBUG( cout <<"IVERTEXDECLARATION9: GetDevice" << endl; )
    *ppDevice = i_parent;
    return D3D_OK;
}

HRESULT D3D_CALL IVertexDeclarationImp9 :: GetDeclaration (  D3DVERTEXELEMENT9* pElement , UINT* pNumElements ) {
    D3D_DEBUG( cout <<"WARNING:  IDirect3DVertexDeclaration9 :: GetDeclaration  NOT IMPLEMENTED" << endl; )
    HRESULT ret = static_cast< HRESULT >(0);
    return ret;
}

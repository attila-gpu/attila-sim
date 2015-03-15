#include "Common.h"
#include "D3DState.h"

void D3DState::initialize() {
}

void D3DState::finalize() {
}

StateDataNode* D3DState::get_root() {
    StateId rootID(NAME_ROOT);
    static StateDataNode root(rootID);
    return &root;
}

StateDataNode* D3DState::create_direct3d_state_9(IDirect3D9* iface) {
    return new StateDataNode(StateId(NAME_DIRECT3D_9, StateIndex(iface)));
}

StateDataNode* D3DState::create_stream_state_9(UINT index) {
        StateDataNode *stream = new StateDataNode(StateId(NAME_STREAM, index));

        stream->add_child(new StateDataNode(StateId(NAME_SOURCE), sizeof(IDirect3DVertexBuffer9 *)));
        stream->add_child(new StateDataNode(StateId(NAME_OFFSET), sizeof(UINT)));
        stream->add_child(new StateDataNode(StateId(NAME_STRIDE), sizeof(UINT)));

        return stream;
}

StateDataNode* D3DState::create_sampler_state_9(UINT index) {
        StateDataNode *sampler = new StateDataNode(StateId(NAME_SAMPLER, index));

        sampler->add_child(new StateDataNode(StateId(NAME_TEXTURE), sizeof(IDirect3DBaseTexture9 *)));
        sampler->add_child(new StateDataNode(StateId(NAME_ADDRESSU), sizeof(DWORD)));
        sampler->add_child(new StateDataNode(StateId(NAME_ADDRESSV), sizeof(DWORD)));
        sampler->add_child(new StateDataNode(StateId(NAME_ADDRESSW), sizeof(DWORD)));

        sampler->add_child(new StateDataNode(StateId(NAME_MAG_FILTER), sizeof(D3DTEXTUREFILTERTYPE)));
        sampler->add_child(new StateDataNode(StateId(NAME_MIN_FILTER), sizeof(D3DTEXTUREFILTERTYPE)));
        sampler->add_child(new StateDataNode(StateId(NAME_MIP_FILTER), sizeof(D3DTEXTUREFILTERTYPE)));
        sampler->add_child(new StateDataNode(StateId(NAME_MAX_ANISOTROPY), sizeof(DWORD)));

        return sampler;
}


StateDataNode* D3DState::create_device_state_9(IDirect3DDevice9* iface) {

    StateDataNode *device = new StateDataNode(StateId(NAME_DEVICE_9, StateIndex(iface)));

    device->add_child(new StateDataNode(StateId(NAME_STREAM_COUNT), sizeof(UINT)));

    device->add_child(new StateDataNode(StateId(NAME_SAMPLER_COUNT), sizeof(UINT)));

    device->add_child(new StateDataNode(StateId(NAME_FVF), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_CURRENT_VERTEX_DECLARATION), sizeof(IDirect3DVertexDeclaration9*)));

    device->add_child(new StateDataNode(StateId(NAME_CURRENT_INDEX_BUFFER), sizeof(IDirect3DIndexBuffer9*)));

    device->add_child(new StateDataNode(StateId(NAME_CURRENT_VERTEX_SHADER), sizeof(IDirect3DVertexShader9*)));
    device->add_child(new StateDataNode(StateId(NAME_CURRENT_PIXEL_SHADER), sizeof(IDirect3DPixelShader9*)));

    /**@todo move constants intialization to the interface layer,
       like samplers and streams */
    for(UINT i = 0; i < MAX_D3D_VS_CONSTANTS; i++) {
        device->add_child(new StateDataNode(StateId(NAME_VS_CONSTANT, StateIndex(i)), sizeof(float) * 4));
    }
    for(UINT i = 0; i < MAX_D3D_PS_CONSTANTS; i++) {
        device->add_child(new StateDataNode(StateId(NAME_PS_CONSTANT, StateIndex(i)), sizeof(float) * 4));
    }

    device->add_child(new StateDataNode(StateId(NAME_IMPLICIT_RENDER_TARGET), sizeof(IDirect3DSurface9*)));
    device->add_child(new StateDataNode(StateId(NAME_IMPLICIT_DEPTHSTENCIL), sizeof(IDirect3DSurface9*)));

    device->add_child(new StateDataNode(StateId(NAME_CURRENT_RENDER_TARGET), sizeof(IDirect3DSurface9*)));
    device->add_child(new StateDataNode(StateId(NAME_CURRENT_DEPTHSTENCIL), sizeof(IDirect3DSurface9*)));

    device->add_child(new StateDataNode(StateId(NAME_BACKBUFFER_WIDTH), sizeof(UINT)));
    device->add_child(new StateDataNode(StateId(NAME_BACKBUFFER_HEIGHT), sizeof(UINT)));
    device->add_child(new StateDataNode(StateId(NAME_BACKBUFFER_FORMAT), sizeof(D3DFORMAT)));
    device->add_child(new StateDataNode(StateId(NAME_DEPTHSTENCIL_FORMAT), sizeof(D3DFORMAT)));

    device->add_child(new StateDataNode(StateId(NAME_CULL_MODE), sizeof(D3DCULL)));

    device->add_child(new StateDataNode(StateId(NAME_ALPHA_TEST_ENABLED), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_ALPHA_FUNC), sizeof(D3DCMPFUNC)));
    device->add_child(new StateDataNode(StateId(NAME_ALPHA_REF), sizeof(DWORD)));

    device->add_child(new StateDataNode(StateId(NAME_Z_ENABLED), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_Z_FUNCTION), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_Z_WRITE_ENABLE), sizeof(DWORD)));

    device->add_child(new StateDataNode(StateId(NAME_ALPHA_BLEND_ENABLED), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_BLEND_FACTOR), sizeof(D3DCOLOR)));
    device->add_child(new StateDataNode(StateId(NAME_SRC_BLEND), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_DST_BLEND), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_BLEND_OP), sizeof(D3DBLENDOP)));
    device->add_child(new StateDataNode(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_SRC_BLEND_ALPHA), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_DST_BLEND_ALPHA), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_BLEND_OP_ALPHA), sizeof(D3DBLENDOP)));

    device->add_child(new StateDataNode(StateId(NAME_STENCIL_ENABLED), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_STENCIL_REF), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_STENCIL_FUNC), sizeof(D3DCMPFUNC)));
    device->add_child(new StateDataNode(StateId(NAME_STENCIL_FAIL), sizeof(D3DSTENCILOP)));
    device->add_child(new StateDataNode(StateId(NAME_STENCIL_ZFAIL), sizeof(D3DSTENCILOP)));
    device->add_child(new StateDataNode(StateId(NAME_STENCIL_PASS), sizeof(D3DSTENCILOP)));
    device->add_child(new StateDataNode(StateId(NAME_STENCIL_MASK), sizeof(DWORD)));
    device->add_child(new StateDataNode(StateId(NAME_STENCIL_WRITE_MASK), sizeof(DWORD)));

    device->add_child(new StateDataNode(StateId(NAME_COMMANDS)));

    return device;
}

StateDataNode* D3DState::create_vertex_buffer_state_9(IDirect3DVertexBuffer9* iface) {

    StateDataNode* vb = new StateDataNode(StateId(NAME_VERTEXBUFFER_9, StateIndex(iface)));

    vb->add_child(new StateDataNode(StateId(NAME_TYPE), sizeof(D3DRESOURCETYPE)));
    vb->add_child(new StateDataNode(StateId(NAME_LENGTH), sizeof(UINT)));
    vb->add_child(new StateDataNode(StateId(NAME_USAGE), sizeof(DWORD)));
    vb->add_child(new StateDataNode(StateId(NAME_FVF),   sizeof(DWORD)));
    vb->add_child(new StateDataNode(StateId(NAME_POOL),  sizeof(D3DPOOL)));

    vb->add_child(new StateDataNode(StateId(NAME_DATA)));

    vb->add_child(new StateDataNode(StateId(NAME_LOCK_COUNT), sizeof(UINT)));

    return vb;
}



StateDataNode* D3DState::create_index_buffer_state_9(IDirect3DIndexBuffer9* iface) {

    StateDataNode* ib = new StateDataNode(StateId(NAME_INDEXBUFFER_9, StateIndex(iface)));

    ib->add_child(new StateDataNode(StateId(NAME_TYPE), sizeof(D3DRESOURCETYPE)));

    ib->add_child(new StateDataNode(StateId(NAME_LENGTH), sizeof(UINT)));
    ib->add_child(new StateDataNode(StateId(NAME_USAGE),  sizeof(DWORD)));
    ib->add_child(new StateDataNode(StateId(NAME_FORMAT), sizeof(D3DFORMAT)));
    ib->add_child(new StateDataNode(StateId(NAME_POOL),   sizeof(D3DPOOL)));

    ib->add_child(new StateDataNode(StateId(NAME_DATA)));

    ib->add_child(new StateDataNode(StateId(NAME_LOCK_COUNT), sizeof(UINT)));

    return ib;
}

StateDataNode* D3DState::create_texture_state_9(IDirect3DTexture9* iface) {
    StateDataNode* tx = new StateDataNode(StateId(NAME_TEXTURE_9, StateIndex(iface)));
    tx->add_child(new StateDataNode(StateId(NAME_WIDTH), sizeof(UINT)));
    tx->add_child(new StateDataNode(StateId(NAME_HEIGHT), sizeof(UINT)));
    tx->add_child(new StateDataNode(StateId(NAME_FORMAT), sizeof(D3DFORMAT)));
    tx->add_child(new StateDataNode(StateId(NAME_USAGE), sizeof(DWORD)));
    tx->add_child(new StateDataNode(StateId(NAME_POOL), sizeof(D3DPOOL)));
   /**@note LEVELS is the parameter passed to CreateTexture
             LEVEL_COUNT is the actual number of levels created */
    tx->add_child(new StateDataNode(StateId(NAME_LEVELS), sizeof(UINT)));
    tx->add_child(new StateDataNode(StateId(NAME_LEVEL_COUNT), sizeof(UINT)));
    return tx;
}

StateDataNode* D3DState::create_texture_mipmap_state_9(UINT index) {
    StateDataNode* s_mipmap = new StateDataNode(StateId(NAME_MIPMAP, StateIndex(index)), sizeof(IDirect3DSurface9*));
    s_mipmap->add_child(new StateDataNode(StateId(NAME_WIDTH), sizeof(UINT)));
    s_mipmap->add_child(new StateDataNode(StateId(NAME_HEIGHT), sizeof(UINT)));
    return s_mipmap;
}

StateDataNode* D3DState::create_cubetexture_state_9(IDirect3DCubeTexture9* iface) {
    StateDataNode* s_cube = new StateDataNode(StateId(NAME_CUBETEXTURE_9, StateIndex(iface)));
    s_cube->add_child(new StateDataNode(StateId(NAME_LENGTH), sizeof(UINT)));
    s_cube->add_child(new StateDataNode(StateId(NAME_FORMAT), sizeof(D3DFORMAT)));
    s_cube->add_child(new StateDataNode(StateId(NAME_USAGE), sizeof(DWORD)));
    s_cube->add_child(new StateDataNode(StateId(NAME_POOL), sizeof(D3DPOOL)));
   /**@note LEVELS is the parameter passed to CreateCubeTexture
             LEVEL_COUNT is the actual number of levels created */
    s_cube->add_child(new StateDataNode(StateId(NAME_LEVELS), sizeof(UINT)));
    s_cube->add_child(new StateDataNode(StateId(NAME_LEVEL_COUNT), sizeof(UINT)));
    s_cube->add_child(new StateDataNode(StateId(NAME_FACE_POSITIVE_X)));
    s_cube->add_child(new StateDataNode(StateId(NAME_FACE_NEGATIVE_X)));
    s_cube->add_child(new StateDataNode(StateId(NAME_FACE_POSITIVE_Y)));
    s_cube->add_child(new StateDataNode(StateId(NAME_FACE_NEGATIVE_Y)));
    s_cube->add_child(new StateDataNode(StateId(NAME_FACE_POSITIVE_Z)));
    s_cube->add_child(new StateDataNode(StateId(NAME_FACE_NEGATIVE_Z)));
    return s_cube;
}

StateDataNode* D3DState::create_cubetexture_mipmap_state_9(UINT index) {
    StateDataNode* s_mipmap = new StateDataNode(StateId(NAME_MIPMAP, StateIndex(index)), sizeof(IDirect3DSurface9*));
    s_mipmap->add_child(new StateDataNode(StateId(NAME_LENGTH), sizeof(UINT)));
    return s_mipmap;
}


StateDataNode* D3DState::create_surface_state_9(IDirect3DSurface9* iface) {
    StateDataNode* surf = new StateDataNode(StateId(NAME_SURFACE_9, StateIndex(iface)));

    surf->add_child(new StateDataNode(StateId(NAME_TYPE), sizeof(D3DRESOURCETYPE)));
    surf->add_child(new StateDataNode(StateId(NAME_WIDTH), sizeof(UINT)));
    surf->add_child(new StateDataNode(StateId(NAME_HEIGHT), sizeof(UINT)));
    surf->add_child(new StateDataNode(StateId(NAME_FORMAT), sizeof(D3DFORMAT)));
    surf->add_child(new StateDataNode(StateId(NAME_USAGE), sizeof(DWORD)));
    surf->add_child(new StateDataNode(StateId(NAME_POOL), sizeof(D3DPOOL)));
    surf->add_child(new StateDataNode(StateId(NAME_DATA)));

    return surf;
}

StateDataNode* D3DState::create_volumetexture_state_9(IDirect3DVolumeTexture9* iface) {
    StateDataNode* voltx = new StateDataNode(StateId(NAME_VOLUMETEXTURE_9, StateIndex(iface)));
    voltx->add_child(new StateDataNode(StateId(NAME_WIDTH), sizeof(UINT)));
    voltx->add_child(new StateDataNode(StateId(NAME_HEIGHT), sizeof(UINT)));
    voltx->add_child(new StateDataNode(StateId(NAME_DEPTH), sizeof(UINT)));
    voltx->add_child(new StateDataNode(StateId(NAME_FORMAT), sizeof(D3DFORMAT)));
    voltx->add_child(new StateDataNode(StateId(NAME_USAGE), sizeof(DWORD)));
    voltx->add_child(new StateDataNode(StateId(NAME_POOL), sizeof(D3DPOOL)));
   /**@note LEVELS is the parameter passed to CreateVolumeTexture
             LEVEL_COUNT is the actual number of levels created */
    voltx->add_child(new StateDataNode(StateId(NAME_LEVELS), sizeof(UINT)));
    voltx->add_child(new StateDataNode(StateId(NAME_LEVEL_COUNT), sizeof(UINT)));
    return voltx;
}

StateDataNode* D3DState::create_volume_state_9(IDirect3DVolume9* iface) {
    StateDataNode* vol = new StateDataNode(StateId(NAME_VOLUME_9, StateIndex(iface)));

    vol->add_child(new StateDataNode(StateId(NAME_TYPE), sizeof(D3DRESOURCETYPE)));
    vol->add_child(new StateDataNode(StateId(NAME_WIDTH), sizeof(UINT)));
    vol->add_child(new StateDataNode(StateId(NAME_HEIGHT), sizeof(UINT)));
    vol->add_child(new StateDataNode(StateId(NAME_DEPTH), sizeof(UINT)));
    vol->add_child(new StateDataNode(StateId(NAME_FORMAT), sizeof(D3DFORMAT)));
    vol->add_child(new StateDataNode(StateId(NAME_USAGE), sizeof(DWORD)));
    vol->add_child(new StateDataNode(StateId(NAME_POOL), sizeof(D3DPOOL)));
    vol->add_child(new StateDataNode(StateId(NAME_DATA)));

    return vol;
}

StateDataNode* D3DState::create_volumetexture_mipmap_state_9(UINT index) {
    StateDataNode* s_mipmap = new StateDataNode(StateId(NAME_MIPMAP, StateIndex(index)), sizeof(IDirect3DVolume9*));
    s_mipmap->add_child(new StateDataNode(StateId(NAME_WIDTH), sizeof(UINT)));
    s_mipmap->add_child(new StateDataNode(StateId(NAME_HEIGHT), sizeof(UINT)));
    s_mipmap->add_child(new StateDataNode(StateId(NAME_DEPTH), sizeof(UINT)));
    return s_mipmap;
}



StateDataNode* D3DState::create_lock_state_9(UINT index, UINT size) {

    StateDataNode* lock = new StateDataNode(StateId(NAME_LOCK, index));

    lock->add_child(new StateDataNode(StateId(NAME_OFFSET), sizeof(UINT)));
    lock->add_child(new StateDataNode(StateId(NAME_SIZE), sizeof(UINT)));
    lock->add_child(new StateDataNode(StateId(NAME_DATA), size));
    lock->add_child(new StateDataNode(StateId(NAME_FLAGS), sizeof (DWORD)));


    return lock;
}

StateDataNode* D3DState::create_lockrect_state_9() {

    StateDataNode* lock = new StateDataNode(StateId(NAME_LOCKRECT));

    lock->add_child(new StateDataNode(StateId(NAME_LEFT), sizeof(LONG)));
    lock->add_child(new StateDataNode(StateId(NAME_TOP), sizeof(LONG)));
    lock->add_child(new StateDataNode(StateId(NAME_RIGHT), sizeof(LONG)));
    lock->add_child(new StateDataNode(StateId(NAME_BOTTOM), sizeof(LONG)));
    lock->add_child(new StateDataNode(StateId(NAME_DATA)));
    lock->add_child(new StateDataNode(StateId(NAME_PITCH), sizeof(INT)));
    lock->add_child(new StateDataNode(StateId(NAME_FLAGS), sizeof (DWORD)));

    return lock;
}

StateDataNode* D3DState::create_lockbox_state_9() {

    StateDataNode* lockbox = new StateDataNode(StateId(NAME_LOCKBOX));

    lockbox->add_child(new StateDataNode(StateId(NAME_LEFT), sizeof(UINT)));
    lockbox->add_child(new StateDataNode(StateId(NAME_TOP), sizeof(UINT)));
    lockbox->add_child(new StateDataNode(StateId(NAME_RIGHT), sizeof(UINT)));
    lockbox->add_child(new StateDataNode(StateId(NAME_BOTTOM), sizeof(UINT)));
    lockbox->add_child(new StateDataNode(StateId(NAME_FRONT), sizeof(UINT)));
    lockbox->add_child(new StateDataNode(StateId(NAME_BACK), sizeof(UINT)));
    lockbox->add_child(new StateDataNode(StateId(NAME_DATA)));
    lockbox->add_child(new StateDataNode(StateId(NAME_ROW_PITCH), sizeof(INT)));
    lockbox->add_child(new StateDataNode(StateId(NAME_SLICE_PITCH), sizeof(INT)));
    lockbox->add_child(new StateDataNode(StateId(NAME_FLAGS), sizeof (DWORD)));

    return lockbox;
}

StateDataNode* D3DState::create_draw_primitive_state_9() {

    StateDataNode* draw = new StateDataNode(StateId(NAME_DRAW_PRIMITIVE));

    draw->add_child(new StateDataNode(StateId(NAME_PRIMITIVE_MODE), sizeof(D3DPRIMITIVETYPE)));
    draw->add_child(new StateDataNode(StateId(NAME_START_VERTEX), sizeof(UINT)));
    draw->add_child(new StateDataNode(StateId(NAME_PRIMITIVE_COUNT), sizeof(UINT)));

    return draw;
}

StateDataNode* D3DState::create_vertex_declaration_state_9(IDirect3DVertexDeclaration9* iface) {

    StateDataNode* vdec = new StateDataNode(StateId(NAME_VERTEX_DECLARATION_9, StateIndex(iface)));
    vdec->add_child(new StateDataNode(StateId(NAME_ELEMENT_COUNT), sizeof(UINT)));
    return vdec;
}

StateDataNode* D3DState::create_vertex_element_state_9(UINT index) {

    StateDataNode* element;
    element = new StateDataNode(StateId(NAME_VERTEX_ELEMENT, index));

    element->add_child(new StateDataNode(StateId(NAME_STREAM), sizeof(WORD)));
    element->add_child(new StateDataNode(StateId(NAME_OFFSET), sizeof(WORD)));
    element->add_child(new StateDataNode(StateId(NAME_TYPE), sizeof(::BYTE)));
    element->add_child(new StateDataNode(StateId(NAME_METHOD), sizeof(::BYTE)));
    element->add_child(new StateDataNode(StateId(NAME_USAGE), sizeof(::BYTE)));
    element->add_child(new StateDataNode(StateId(NAME_USAGE_INDEX), sizeof(::BYTE)));

    return element;
}




StateDataNode* D3DState::create_draw_indexed_primitive_state_9() {

    StateDataNode* drawi = new StateDataNode(StateId(NAME_DRAW_INDEXED_PRIMITIVE));

    drawi->add_child(new StateDataNode(StateId(NAME_PRIMITIVE_MODE), sizeof(D3DPRIMITIVETYPE)));
    drawi->add_child(new StateDataNode(StateId(NAME_START_INDEX), sizeof(UINT)));
    drawi->add_child(new StateDataNode(StateId(NAME_PRIMITIVE_COUNT), sizeof(UINT)));

    return drawi;
}

StateDataNode* D3DState::create_present_state_9() {
    StateDataNode* present = new StateDataNode(StateId(NAME_PRESENT));
    return present;
}

StateDataNode* D3DState::create_clear_state_9() {
    StateDataNode* clear = new StateDataNode(StateId(NAME_CLEAR));
    clear->add_child(new StateDataNode(StateId(NAME_COLOR), sizeof(D3DCOLOR)));
    clear->add_child(new StateDataNode(StateId(NAME_Z), sizeof(float)));
    clear->add_child(new StateDataNode(StateId(NAME_STENCIL), sizeof(DWORD)));
    clear->add_child(new StateDataNode(StateId(NAME_FLAGS), sizeof(DWORD)));
    return clear;
}

StateDataNode* D3DState::create_vertex_shader_state_9(IDirect3DVertexShader9* iface, UINT size) {
    StateDataNode* vsh = new StateDataNode(StateId(NAME_VERTEX_SHADER_9, StateIndex(iface)));
    vsh->add_child(new StateDataNode(StateId(NAME_CODE), size));
    return vsh;
}

StateDataNode* D3DState::create_pixel_shader_state_9(IDirect3DPixelShader9* iface, UINT size) {
    StateDataNode* psh = new StateDataNode(StateId(NAME_PIXEL_SHADER_9, StateIndex(iface)));
    psh->add_child(new StateDataNode(StateId(NAME_CODE), size));
    return psh;
}



D3DState::D3DState() {}

void D3DState::dump_state(string filename) {
	ofstream f(filename.c_str());
	dump_state_recursive(get_root(), 0, f);
}

void D3DState::dump_state_recursive(StateDataNode* node, UINT level, ostream &out) {
    for(UINT i = 0; i < level; i ++)
        out << "    ";

	out << node->get_id().name <<
	"<0x" << hex << node->get_id().index << dec << ">" << endl;

	size_t size = node->get_data_size();
	if(size != 0) {
		UINT count_dw = ((size % 4) == 0) ? (size / 4) : (size / 4 + 1);
		DWORD* data_dw = (DWORD*)malloc(count_dw * 4);
		node->read_data(data_dw);
		UINT limit = (count_dw < 64) ? count_dw : 64;
		for(UINT i = 0; i < limit; i ++) {
			for(UINT j = 0; j < level; j ++)
				out << "    ";
			out << hex;
			out << "0x" << data_dw[i] << "|";
			out << dec;
			out << *(float*)&data_dw[i] << "f|";
			out << *(unsigned int*)&data_dw[i] << endl;
			out << dec;
		}
		free(data_dw);
	}
    set<StateDataNode*> childs = node->get_childs();
    set<StateDataNode*>::iterator it_c;
    for(it_c = childs.begin(); it_c != childs.end(); it_c ++) {
        dump_state_recursive(*it_c, level + 1, out);
    }
}

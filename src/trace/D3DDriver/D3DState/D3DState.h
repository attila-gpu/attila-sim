#ifndef STATE_H_INCLUDED
#define STATE_H_INCLUDED

#include "StateDataTree.h"

class D3DState {
public:
    static void initialize();
    static void finalize();

    static StateDataNode* get_root();

    static void dump_state(string filename);

    static StateDataNode* create_direct3d_state_9(IDirect3D9* iface);
    static StateDataNode* create_device_state_9(IDirect3DDevice9* iface);

    static StateDataNode* create_stream_state_9(UINT index);

    static StateDataNode* create_sampler_state_9(UINT index);

    static StateDataNode* create_vertex_declaration_state_9(IDirect3DVertexDeclaration9* iface);
    static StateDataNode* create_vertex_element_state_9(UINT index);


    static StateDataNode* create_vertex_buffer_state_9(IDirect3DVertexBuffer9* iface);
    static StateDataNode* create_index_buffer_state_9(IDirect3DIndexBuffer9* iface);

    static StateDataNode* create_texture_state_9(IDirect3DTexture9* iface);
    static StateDataNode* create_texture_mipmap_state_9(UINT index);
    static StateDataNode* create_cubetexture_state_9(IDirect3DCubeTexture9* iface);
    static StateDataNode* create_cubetexture_mipmap_state_9(UINT index);
    static StateDataNode* create_volumetexture_state_9(IDirect3DVolumeTexture9* iface);
    static StateDataNode* create_volumetexture_mipmap_state_9(UINT index);

    static StateDataNode* create_surface_state_9(IDirect3DSurface9* iface);
    static StateDataNode* create_volume_state_9(IDirect3DVolume9* iface);


    static StateDataNode* create_vertex_shader_state_9(IDirect3DVertexShader9* iface, UINT size);
    static StateDataNode* create_pixel_shader_state_9(IDirect3DPixelShader9* iface, UINT size);


    static StateDataNode* create_lock_state_9(UINT index, UINT size);
    static StateDataNode* create_lockrect_state_9();
    static StateDataNode* create_lockbox_state_9();

    static StateDataNode* create_draw_primitive_state_9();
    static StateDataNode* create_draw_indexed_primitive_state_9();
    static StateDataNode* create_present_state_9();
    static StateDataNode* create_clear_state_9();
private:
    D3DState();

    static void dump_state_recursive(StateDataNode* node, UINT level, ostream &out);
};

#endif


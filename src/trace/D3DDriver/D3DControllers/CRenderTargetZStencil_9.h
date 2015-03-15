#ifndef CRENDERTARGETZSTENCIL_9_H_INCLUDED
#define CRENDERTARGETZSTENCIL_9_H_INCLUDED

/**
    Manages:
    RenderTarget / ZStencil change
    DRAW CLEAR and PRESENT commands
    Add it DEVICE node.
    IMPORTANT:
    It's necessary to add it AFTER all other controllers that configure
    gpu for the batch are added, because this controller sends
    GPU_DRAW/GPU_SWAPBUFFERS commands to gpu.
*/

class CRenderTargetZStencil9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    CRenderTargetZStencil9();
private:
    u32bit implicitFrontBufferAddr;     //<  Stores the memory descriptor (GPU memory) of the front color buffer created on device creation (default or implicit).
    u32bit implicitBackBufferAddr;      //<  Stores the memory descriptor (GPU memory) of the back color buffer created on device creation (default or implicit).
    u32bit implicitZStencilBufferAddr;  //<  Stores the memory descriptor (GPU memory) of the z and stencil buffer created on device creation (default or implicit).
    IDirect3DSurface9* currentRenderTarget;     //<  Stores the pointer to the active render target.
    IDirect3DSurface9* currentZStencilBuffer;   //<  Stores the pointer to the active z and stencil buffer.
    bool swapped_front_back;

    u32bit colorBlockStateBufferMD;     //< Stores the memory descriptor (GPU memory) of the color block state buffer where block state data is saved.
    
    u32bit frame_count;

    void clear_using_rectangle(DWORD flags, D3DCOLOR color, float z, DWORD stencil);

    GPURegData stored_stream_elements_0;
    GPURegData stored_stream_data_0;
    u32bit stored_stream_address_0;
    u32bit stored_stream_address_offset_0;
    GPURegData stored_stream_stride_0;
    GPURegData stored_stream_start;
    GPURegData stored_stream_count;
    GPURegData stored_index_mode;
    GPURegData stored_primitive;
    void setup_clear_vertices();
    void restore_clear_vertices();
    u32bit clear_vb;


    GPURegData stored_culling;
    GPURegData stored_depth_mask;
    GPURegData stored_depth_test;
    GPURegData stored_depth_function;
    GPURegData stored_earlyz;
    GPURegData stored_hierarchicalz;

    GPURegData stored_update_mask;
    GPURegData stored_stencil_test;
    GPURegData stored_stencil_function;
    GPURegData stored_stencil_reference;

    GPURegData stored_color_blend;
    GPURegData stored_color_mask_r;
    GPURegData stored_color_mask_g;
    GPURegData stored_color_mask_b;
    GPURegData stored_color_mask_a;
    void setup_clear_pipeline(DWORD flags, DWORD stencil);
    void restore_clear_pipeline();


    GPURegData stored_vertex_attribute_map_0;
    GPURegData stored_vertex_output_attribute_0;
    GPURegData stored_vertex_output_attribute_1;
    u32bit stored_vertex_program;
    u32bit stored_vertex_program_offset;
    GPURegData stored_vertex_program_pc;
    GPURegData stored_vertex_program_size;
    GPURegData stored_vertex_constant_0;
    GPURegData stored_vertex_constant_1;
    GPURegData stored_fragment_input_attributes_1;
    GPURegData stored_modify_fragment_depth;
    u32bit stored_fragment_program;
    u32bit stored_fragment_program_offset;
    GPURegData stored_fragment_program_pc;
    GPURegData stored_fragment_program_size;
    void setup_clear_shaders(D3DCOLOR color, float z);
    void restore_clear_shaders();
    u32bit clear_psh_md;
    u32bit clear_psh_size;
    u32bit clear_vsh_md;
    u32bit clear_vsh_size;

};

#endif

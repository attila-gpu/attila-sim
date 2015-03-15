#ifndef CDEVICE_9_H_INCLUDED
#define CDEVICE_9_H_INCLUDED

class CIndexVertexBuffers9;
class CStreams9;
class CIndexStream9;
class CVSInput9;
class CRasterization9;
class CAlphaBlending9;
class CZStencilTest9;
class CDebugGeometry9;
class CShaders9;
class CConstants9;
class CTextures9;
class CSurfaces9;
class CVolumes9;
class CSamplers9;
class CRenderTargetZStencil9;

/**
    Reset GPU state when a SDEVICE is added.
    Manage SDEVICE controllers.
*/
class CDevice9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    CDevice9();
    ~CDevice9();

private:
    void initialize_gpu(StateDataNode* s_device);

    /// Each index/vertex buffer has assigned gpu memory
    ResourceAssignationTable *a_iv_buffer_memory;
    /// Each surface has assigned gpu memory
    ResourceAssignationTable *a_surface_memory;
    /// Each volume has assigned gpu memory
    ResourceAssignationTable *a_volume_memory;
    /// Each vertex element is assigned to a gpu stream.
    ResourceAssignationTable* a_element_stream;
    /** Many elements can be read from a d3d stream,
       but only one element can be read from a gpu stream.
       So each d3d stream has assigned many gpu streams, one
       for each element. This map stores the list of gpu streams
       in use for each d3d stream.
        */
    ResourceAssignationTable *a_streams;

    /// Input registers of vertex shader assigned to an usage
    ResourceAssignationTable* a_vs_input;
    /// Constants registers of vertex shader
    ResourceAssignationTable* a_vs_constants;
    /// Output registers of vertex shader assigned to an usage
    ResourceAssignationTable* a_vs_output;
    /// Input registers of pixel shader assigned to an usage
    ResourceAssignationTable* a_ps_input;
    /// Constants registers of pixel shader
    ResourceAssignationTable* a_ps_constants;
    /// Texture registers of pixel shader assigned to a sampler register
    ResourceAssignationTable* a_ps_textures;


    CIndexVertexBuffers9 *c_index_vertex_buffers;
    CStreams9       *c_streams;
    CIndexStream9     *c_index_stream;
    CVSInput9         *c_vs_input;
    CConstants9       *c_constants;
    CRasterization9   *c_rasterization;
    CAlphaBlending9 *c_alpha_blending;
    CZStencilTest9         *c_zstencil_test;
    CRenderTargetZStencil9     *c_render_target_zstencil;
    CDebugGeometry9 *c_debug_geometry;
    CShaders9* c_shaders;
    CTextures9* c_textures;
    CSurfaces9* c_surfaces;
    CVolumes9* c_volumes;
    CSamplers9* c_samplers;
};

#endif



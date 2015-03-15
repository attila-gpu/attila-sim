#ifndef CDEBUGGEOMETRYRENDERER_9_H_INCLUDED
#define CDEBUGGEOMETRYRENDERER_9_H_INCLUDED

/**
    Renders geometry data in a normalized way, scaling all vertexes to the
    normalized cube, and uses a color code to show vertex atributes.
*/
class CDebugGeometry9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);
private:
    void load_vertex_shader();
    void load_pixel_shader();
    void unload_pixel_shader();
    void unload_vertex_shader();

    void set_constants(StateDataNode*);

    /// Locate the position attribute using current vertex declaration
    bool find_position(StateDataNode* s_device, WORD* position_stream,
        WORD* position_offset, ::BYTE* position_type);

    void get_bounding_box(D3DVECTOR* bb_min, D3DVECTOR* bb_max,
        void* vertex_data, void* index_data,
        UINT stream_offset, UINT position_offset, UINT stride,
        UINT start_index, UINT total_indices);

    void get_matrix(D3DMATRIX* matrix, D3DVECTOR* bounding_min, D3DVECTOR* bounding_max);

    u32bit assigned_ps_memory;
    u32bit assigned_vs_memory;
};

#endif


#ifndef CSTREAMS_9_H_INCLUDED
#define CSTREAMS_9_H_INCLUDED

/**
    Updates GPU streaming state to match D3D streaming state:
        Setted D3D Vertex buffers.
        Current FVF/VertexDeclaration.
*/
class CStreams9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    CStreams9();
    ~CStreams9();
private:
    /* Keep track of vertex elements assigned */
    set< UsageId > elements_assigned;
    set< UsageId > streams_assigned;
    /* Maps gpu stream with offset of the vertex element
       assigned to it */
    map< u32bit, UINT > gpu_element_offset;
    // Put everything in its initial state
    void reset_all();

    /*  Updates data addresses for all gpu streams
    assigned to a d3d stream */
    void update_gpu_stream_addresses(StateDataNode* s_stream);
    /*  Updates stride for all gpu streams
        assigned to a d3d stream */
    void update_gpu_stream_stride(StateDataNode* s_stream);
    /*  Sets to default values all registers of a gpu stream */
    void reset_gpu_stream(u32bit gpu_stream);


    /* Each d3d element type is mapped to a gpu element type */
    struct GPUElementType {
        StreamData data;
        u32bit count;
        GPUElementType(): data(SD_FLOAT), count(0) {}
        GPUElementType(StreamData _data, u32bit _count):
            data(_data), count(_count) {}
    };
    map< D3DDECLTYPE, GPUElementType > gpu_element_type;

    std::map<D3DPRIMITIVETYPE, PrimitiveMode> gpu_primitive_mode;

};

#endif

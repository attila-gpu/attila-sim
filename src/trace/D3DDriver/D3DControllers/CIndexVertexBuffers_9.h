#ifndef CVERTEXINDEXBUFFERS9_H_INCLUDED
#define CVERTEXINDEXBUFFERS9_H_INCLUDED


class CLock9;

/**
    Manages Vertex/Index buffers memory allocation/deallocation and
    lock/unlock operations.

    Must be added to the DEVICE_9 state node.
*/

class CIndexVertexBuffers9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode* node);
    void on_removed_controller(StateDataNode* node);

    CIndexVertexBuffers9();
    ~CIndexVertexBuffers9();

private:
    CLock9* c_lock;
};

#endif


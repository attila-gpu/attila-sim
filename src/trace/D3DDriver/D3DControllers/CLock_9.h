#ifndef CLOCK9_H_INCLUDED
#define CLOCK9_H_INCLUDED

class CLock;

/**
    Updates GPU memory when an Index/Vertex buffer memory is
    updated using a Lock/Unlock operation.

    Must be added to the INDEXBUFFER_9 / VERTEXBUFFER_9 state node.
*/

class CLock9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode* node);
    void on_removed_controller(StateDataNode* node);
};

#endif

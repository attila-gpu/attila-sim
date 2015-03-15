#ifndef CTEXTURES_9_H_INCLUDED
#define CTEXTURES_9_H_INCLUDED

/**
    Manages texture mipmap levels creation.
    Must be added to the DEVICE_9 state node.
*/


class CTextures9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode* node);
    void on_removed_controller(StateDataNode* node);

    CTextures9();
    ~CTextures9();
};


#endif // CTEXTURES_9_H_INCLUDED

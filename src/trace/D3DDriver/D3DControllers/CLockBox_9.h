#ifndef CLOCKBOX_9_H_INCLUDED
#define CLOCKBOX_9_H_INCLUDED

/**
    Updates GPU memory when a Volume memory is
    updated using a LockBox/UnlockBox operation.

    Must be added to the VOLUME_9 state node.
*/

class CLockBox9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode* node);
    void on_removed_controller(StateDataNode* node);
};


#endif // CLOCKBOX_9_H_INCLUDED

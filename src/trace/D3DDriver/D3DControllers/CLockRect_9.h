#ifndef CLOCKRECT9_H_INCLUDED
#define CLOCKRECT9_H_INCLUDED

/**
    Updates GPU memory when an Surface memory is
    updated using a LockRect/UnlockRect operation.

    Must be added to the SURFACE_9 state node.
*/

class CLockRect9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode* node);
    void on_removed_controller(StateDataNode* node);
};



#endif // CLOCKRECT9_H_INCLUDED

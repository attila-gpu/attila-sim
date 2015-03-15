#ifndef CDEBUGSTATE_H_INCLUDED
#define CDEBUGSTATE_H_INCLUDED

/**
    Prints changes in state data tree for debugging purposes.

    Add to any state data node.
*/
class CDebugState : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode* node);
    void on_removed_controller(StateDataNode* node);

private:
    void print_recursive(StateDataNode* node, unsigned int level, ostream& out);
    void print(StateDataNode* node, ostream& out);
    void print_data(void* data, size_t size, ostream& out);
    void print_id(StateId id, ostream& out);
    void add_controller_recursive(StateDataNode* node);
    void remove_controller_recursive(StateDataNode* node);
};

#endif

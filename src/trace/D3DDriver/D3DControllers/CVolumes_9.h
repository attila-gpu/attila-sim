#ifndef CVOLUMES_9_H_INCLUDED
#define CVOLUMES_9_H_INCLUDED

class CLockBox9;

/**
    Manages gpu memory for volumes.
    Add to the DEVICE node.
*/
class CVolumes9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    CVolumes9();
    ~CVolumes9();
private:
    CLockBox9* c_lockbox;
};

#endif // CVOLUMES_9_H_INCLUDED

#ifndef CROOT_H_INCLUDED
#define CROOT_H_INCLUDED

class CDevice9;
class CDebugState;

/**
    Manages SDEVICE_9 controllers.
*/

class CRoot : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);

    CRoot();
    ~CRoot();
private:
    CDevice9 *c_device_9;
    CDebugState* c_debug_state;
};

#endif

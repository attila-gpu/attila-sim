#include "Common.h"
#include "CDebugState.h"
#include "CDevice_9.h"
#include "CRoot.h"

void CRoot::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(parent->get_id().name == NAME_ROOT) {
        if(child->get_id().name == NAME_DIRECT3D_9) {
            child->add_controller(c_device_9);
        }
    }
}

void CRoot::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    if(parent->get_id().name == NAME_ROOT) {
        if(child->get_id().name == NAME_DIRECT3D_9) {
            child->remove_controller(c_device_9);
        }
    }
}

void CRoot::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
}

void CRoot::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_ROOT) {
        D3D_DEBUG( node->add_controller(c_debug_state); )
    }
}

void CRoot::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_ROOT) {
        D3D_DEBUG( node->remove_controller(c_debug_state); )
    }
}

CRoot::CRoot() {
    c_device_9 = new CDevice9();
    c_debug_state = new CDebugState();
}

CRoot::~CRoot() {
    delete c_debug_state;
    delete c_device_9;
}

#include "Common.h"
#include "CLock_9.h"
#include "CIndexVertexBuffers_9.h"

CIndexVertexBuffers9::CIndexVertexBuffers9() {
    c_lock = new CLock9();
}

CIndexVertexBuffers9::~CIndexVertexBuffers9() {
    delete c_lock;
}

void CIndexVertexBuffers9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(parent->get_id().name == NAME_DEVICE_9) {
        if((child->get_id().name == NAME_INDEXBUFFER_9)||
            (child->get_id().name == NAME_VERTEXBUFFER_9)) {
            // Add controllers
            child->add_controller(this);
            child->add_controller(c_lock);
        }
    }
}

void CIndexVertexBuffers9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    if(parent->get_id().name == NAME_DEVICE_9) {
        if((child->get_id().name == NAME_INDEXBUFFER_9)||
            (child->get_id().name == NAME_VERTEXBUFFER_9)) {
            // Remove controllers
            child->remove_controller(c_lock);
            child->remove_controller(this);
        }
    }
}

void CIndexVertexBuffers9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
}

void CIndexVertexBuffers9::on_added_controller(StateDataNode* node) {
    if((node->get_id().name == NAME_INDEXBUFFER_9) ||
       (node->get_id().name == NAME_VERTEXBUFFER_9)) {

        UINT lenght;
        node->get_child(StateId(NAME_LENGTH))->read_data(&lenght);
        // Reserve storage in state
        node->get_child(StateId(NAME_DATA))->set_data_size(lenght);
        // Obtain GPU memory
        u32bit md;
        md = GPUProxy::get_instance()->obtainMemory(size_t(lenght));

        /* GPU memory is assigned to resource, so add the assignation
           to the assignation table */
        ResourceAssignationTable* a_resource_memory;
        a_resource_memory = AssignationsRegistry::get_table(NAME_IVBUFFER_MEMORY);
        a_resource_memory->add_resource(ResourceId(NAME_MEMORY, ResourceIndex(md)));
        a_resource_memory->assign( UsageId(NAME_IVBUFFER, node->get_id().index),
            ResourceId(NAME_MEMORY, ResourceIndex(md)));
        D3D_DEBUG( cout << "CINDEXVERTEXBUFFERS9: Memory descriptor is " << md << " size is " << lenght << endl; )
    }
}

void CIndexVertexBuffers9::on_removed_controller(StateDataNode* node) {
    if((node->get_id().name == NAME_INDEXBUFFER_9) ||
       (node->get_id().name == NAME_VERTEXBUFFER_9)) {

        /* Get GPU memory assigned to resource from the
           assignation table and release it */
        ResourceAssignationTable* a_resource_memory;
        a_resource_memory = AssignationsRegistry::get_table(NAME_IVBUFFER_MEMORY);
        ResourceId r = a_resource_memory->get_assigned_to(UsageId(NAME_IVBUFFER, node->get_id().index));
        u32bit md;
        md = u32bit(r.index);
        GPUProxy::get_instance()->releaseMemory(md);
        /* Remove the assignation from the assignation table */
        a_resource_memory->unassign( UsageId(NAME_IVBUFFER, node->get_id().index));
        a_resource_memory->remove_resource(ResourceId(NAME_MEMORY, ResourceIndex(md)));

        D3D_DEBUG( cout << "CINDEXVERTEXBUFFERS9: Memory descriptor deleted is " << md << endl; )
    }

}

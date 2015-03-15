#include "Common.h"
#include "CLockBox_9.h"
#include "CVolumes_9.h"

void CVolumes9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_VOLUME_9) {
        // Get parameters of surface
        UINT width;
        child->get_child(StateId(NAME_WIDTH))->read_data(&width);
        UINT height;
        child->get_child(StateId(NAME_HEIGHT))->read_data(&height);
        UINT depth;
        child->get_child(StateId(NAME_DEPTH))->read_data(&depth);
        D3DFORMAT format;
        child->get_child(StateId(NAME_FORMAT))->read_data(&format);
        DWORD surf_usage;
        child->get_child(StateId(NAME_USAGE))->read_data(&surf_usage);
        // Calculate size
        u32bit volume_size = get_volume_size(width, height, depth, format);
        // Reserve storage in state data tree
        D3D_DEBUG( cout << "CVOLUMES9: Reserving memory for a " << width << "x" << height << "x" << depth << " volume"; )
        D3D_DEBUG( cout << " (" << (int)volume_size << " bytes)" << endl; )
        child->get_child(StateId(NAME_DATA))->set_data_size(volume_size);
        // Reserve gpu memory and publish assignation to the surface
        u32bit md = GPUProxy::get_instance()->obtainMemory(volume_size);
        // Initialize memory
        /**@note this is necessary because during simulation accessing
           non initialized areas of the texture raises an error. */
        u8bit* initial_data = new u8bit[volume_size];
        for(u32bit i = 0; i < volume_size; i ++) initial_data[i] = rand()%256;
        GPUProxy::get_instance()->writeMemory(md, initial_data, volume_size);
        delete [] initial_data;
        ResourceAssignationTable* a_volume_memory = AssignationsRegistry::get_table(NAME_VOLUME_MEMORY);
        ResourceId resource(NAME_MEMORY, ResourceIndex(md));
        UsageId usage(NAME_VOLUME, UsageIndex(child->get_id().index));
        a_volume_memory->add_resource(resource);
        a_volume_memory->assign(usage, resource);
        D3D_DEBUG( cout << "CVOLUMES9: Memory descriptor is " << (int)md << endl; )

        // Add lockbox controller
        child->add_controller(c_lockbox);
    }

}

void CVolumes9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_VOLUME_9) {

        // Remove lockbox controller
        child->remove_controller(c_lockbox);

        // Release memory
        D3D_DEBUG( cout << "CVOLUMES9: Releasing memory for volume." << endl; )
        // Find memory assigned
        ResourceAssignationTable* a_volume_memory = AssignationsRegistry::get_table(NAME_VOLUME_MEMORY);
        UsageId usage(NAME_VOLUME, UsageIndex(child->get_id().index));
        ResourceId res = a_volume_memory->get_assigned_to(usage);
        u32bit md = u32bit(res.index);
        D3D_DEBUG( cout << "CVOLUMES9: Memory descriptor is " << (int)md << endl; )
        // Unassign and free memory
        a_volume_memory->unassign(usage);
        a_volume_memory->remove_resource(res);
        GPUProxy::get_instance()->releaseMemory(md);
    }
}

void CVolumes9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    // Nothing to do
}


void CVolumes9::on_added_controller(StateDataNode* node) {
    // Nothing to do
}

void CVolumes9::on_removed_controller(StateDataNode* node) {
    // Nothing to do
}

CVolumes9::CVolumes9() {
    c_lockbox = new CLockBox9();
}

CVolumes9::~CVolumes9() {
    delete c_lockbox;
}

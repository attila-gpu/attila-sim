#include "Common.h"
#include "CLockRect_9.h"
#include "CSurfaces_9.h"

void CSurfaces9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_SURFACE_9) {
        // Get parameters of surface
        UINT width;
        child->get_child(StateId(NAME_WIDTH))->read_data(&width);
        UINT height;
        child->get_child(StateId(NAME_HEIGHT))->read_data(&height);
        D3DFORMAT format;
        child->get_child(StateId(NAME_FORMAT))->read_data(&format);
        DWORD surf_usage;
        child->get_child(StateId(NAME_USAGE))->read_data(&surf_usage);
        // Calculate size
        u32bit surface_size = get_surface_size(width, height, format, surf_usage);
        // Reserve storage in state data tree
        D3D_DEBUG( cout << "CSURFACES9: Reserving memory for a " << width << "x" << height << " surface"; )
        D3D_DEBUG( cout << " (" << (int)surface_size << " bytes)" << endl; )
        child->get_child(StateId(NAME_DATA))->set_data_size(surface_size);
        // Reserve gpu memory and publish assignation to the surface
        u32bit md = GPUProxy::get_instance()->obtainMemory(surface_size);
        // Initialize memory
        /**@note this is necessary because during simulation accessing
           non initialized areas of the texture raises an error. */
        u8bit* initial_data = new u8bit[surface_size];
        for(u32bit i = 0; i < surface_size; i ++) initial_data[i] = rand()%256;
        GPUProxy::get_instance()->writeMemory(md, initial_data, surface_size);
        delete [] initial_data;
        ResourceAssignationTable* a_surface_memory = AssignationsRegistry::get_table(NAME_SURFACE_MEMORY);
        ResourceId resource(NAME_MEMORY, ResourceIndex(md));
        UsageId usage(NAME_SURFACE, UsageIndex(child->get_id().index));
        a_surface_memory->add_resource(resource);
        a_surface_memory->assign(usage, resource);
        D3D_DEBUG( cout << "CSURFACES9: Memory descriptor is " << (int)md << endl; )

        // Add lockrect controller
        child->add_controller(c_lockrect);
    }
}

void CSurfaces9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_SURFACE_9) {
        // Remove lockrect controller
        child->remove_controller(c_lockrect);

        // Release memory
        D3D_DEBUG( cout << "CSURFACES9: Releasing memory for surface." << endl; )
        // Find memory assigned
        ResourceAssignationTable* a_surface_memory = AssignationsRegistry::get_table(NAME_SURFACE_MEMORY);
        UsageId usage(NAME_SURFACE, UsageIndex(child->get_id().index));
        ResourceId res = a_surface_memory->get_assigned_to(usage);
        u32bit md = u32bit(res.index);
        D3D_DEBUG( cout << "CSURFACES9: Memory descriptor is " << (int)md << endl; )
        // Unassign and free memory
        a_surface_memory->unassign(usage);
        a_surface_memory->remove_resource(res);
        GPUProxy::get_instance()->releaseMemory(md);
    }
}

void CSurfaces9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    // Nothing to do
}


void CSurfaces9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // Get needed configuration parameters from GPU
        u32bit gpuMemSz, sysMemSz, blocksz, sblocksz, fetchRate;
        bool doubleBuffer;
        GPUProxy::get_instance()->getGPUParameters(gpuMemSz, sysMemSz, blocksz, sblocksz,
            scan_width, scan_height, overscan_width, overscan_height,
            doubleBuffer, fetchRate);
    }
}

void CSurfaces9::on_removed_controller(StateDataNode* node) {
    // Nothing to do
}

CSurfaces9::CSurfaces9() {
    c_lockrect = new CLockRect9();
}

CSurfaces9::~CSurfaces9() {
    delete c_lockrect;
}

UINT CSurfaces9::get_surface_size(UINT width, UINT height, D3DFORMAT format, DWORD usage) {

    UINT size;
    if(usage & D3DUSAGE_DEPTHSTENCIL) {
        size = u32bit(ceil(f32bit(height) / f32bit(scan_height * overscan_height))) *
        u32bit(ceil(f32bit(width) / f32bit(scan_width * overscan_width))) *
        scan_width * overscan_width * scan_height * overscan_height * texel_size(format);
    }
    else if(usage & D3DUSAGE_RENDERTARGET) {
        size = u32bit(ceil(f32bit(height) / f32bit(scan_height * overscan_height))) *
        u32bit(ceil(f32bit(width) / f32bit(scan_width * overscan_width))) *
        scan_width * overscan_width * scan_height * overscan_height * texel_size(format);
    }
    else {

    	// Common texture
		// TODO: Get blocking parameters from configuration
		u32bit block_dim = 3;
		u32bit s_block_dim = 3;
        if(is_compressed(format)) {
            // Calculate how many 4x4 blocks needed
            width = width / 4 + (((width % 4) == 0) ? 0 : 1);
            height = height / 4 + (((height % 4) == 0) ? 0 : 1);
            block_dim -= 2;
        }

		u32bit width2 = ceiledLog2(width);
        u32bit last = texel2MortonAddress(width, height, block_dim, s_block_dim, width2);
        size = last * texel_size(format);
    }
    return size;
}

#include "Common.h"
#include "CLockRect_9.h"

void CLockRect9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_LOCKRECT) {
        /** @todo Perhaps it's necessary to copy surface DATA to lock DATA, depending on the flags.
            Without this, if application only modifies a region of lock DATA
            the contents of the surface DATA corresponding to a non modified regions
            of lock DATA are overwritten with zeroes.
        */

        // Calculate size of DATA node and resize it
        D3DFORMAT format;
        child->get_parent()->get_child(StateId(NAME_FORMAT))->read_data(&format);
        LONG left, right, top, bottom;
        child->get_child(StateId(NAME_LEFT))->read_data(&left);
        child->get_child(StateId(NAME_RIGHT))->read_data(&right);
        child->get_child(StateId(NAME_TOP))->read_data(&top);
        child->get_child(StateId(NAME_BOTTOM))->read_data(&bottom);
        UINT width = right - left;
        UINT height = bottom - top;
        // Change units for compressed formats
        if(is_compressed(format)) {
            width = width / 4 + (((width % 4) == 0) ? 0 : 1);
            height = height / 4 + (((height % 4) == 0) ? 0 : 1);
        }
        UINT size = width * height * texel_size(format);
        child->get_child(StateId(NAME_DATA))->set_data_size(size);
        D3D_DEBUG( cout << "CLOCKRECT9: Resizing SLOCKRECT DATA to " << (int)size << endl; )
        // Write pitch
        INT pitch;
        pitch = texel_size(format) * width;
        child->get_child(StateId(NAME_PITCH))->write_data(&pitch);

        // Watch node
        child->add_controller(this);
    }
}

void CLockRect9::on_remove_node_child(StateDataNode* parent, StateDataNode* child)  {
    if(child->get_id().name == NAME_LOCKRECT) {
        // Unwatch node
        child->remove_controller(this);
    }
}

void CLockRect9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset)  {
    // Note state nodes SURFACE and LOCKRECT have a child named DATA.
    if(node->get_id().name == NAME_DATA) {
        if(node->get_parent()->get_id().name == NAME_SURFACE_9) {

            /* When surface buffer data is modified,
               it has to be copied to the assigned GPU memory*/

            // Get the assignation table
            ResourceAssignationTable* assignation;
            assignation = AssignationsRegistry::get_table(NAME_SURFACE_MEMORY);
            // Locate assignated resource, note usageid is taken from the resource stateid
            StateId s = node->get_parent()->get_id();
            ResourceId r = assignation->get_assigned_to(UsageId(NAME_SURFACE, s.index));
            // Get assigned GPU memory descriptor,
            u32bit md = u32bit(r.index);

            // Copy modified data to GPU memory
            D3D_DEBUG( cout << "CLOCKRECT9: Writing " << size << " bytes at " << offset << " in md " << md << endl; )
            void* data = node->map_data(READ_ACCESS, size, offset);

            GPUProxy::get_instance()->writeMemory(md, offset, (u8bit*)(data), size);
            node->unmap_data();


        }
        else if(node->get_parent()->get_id().name == NAME_LOCKRECT) {

            /** When lock DATA is modified, it has to be copied to the DATA node of the
               surface **/

            // Get attributes of surface
            StateDataNode* s_surf = node->get_parent()->get_parent();
            UINT surf_width;
            s_surf->get_child(StateId(NAME_WIDTH))->read_data(&surf_width);
            UINT surf_height;
            s_surf->get_child(StateId(NAME_HEIGHT))->read_data(&surf_height);
            D3DFORMAT surf_format;
            s_surf->get_child(StateId(NAME_FORMAT))->read_data(&surf_format);

            // Get surface data node
            StateDataNode* surf_data_node = s_surf->get_child(StateId(NAME_DATA));

            // Get attributes of locked rect
            LONG lock_left;
            node->get_parent()->get_child(StateId(NAME_LEFT))->read_data(&lock_left);
            LONG lock_right;
            node->get_parent()->get_child(StateId(NAME_RIGHT))->read_data(&lock_right);
            LONG lock_top;
            node->get_parent()->get_child(StateId(NAME_TOP))->read_data(&lock_top);
            LONG lock_bottom;
            node->get_parent()->get_child(StateId(NAME_BOTTOM))->read_data(&lock_bottom);

            // Calculate derived variables
            UINT lock_width = lock_right - lock_left;
            UINT lock_height = lock_bottom - lock_top;
            UINT pixel_size = texel_size(surf_format);

 			// TODO: Get blocking parameters from configuration
            u32bit block_dim = 3;
            u32bit s_block_dim = 3;

            // Consider blocks instead of pixels for compressed formats
            if(is_compressed(surf_format)) {
                lock_left /= 4;
                lock_right/= 4;
                lock_top/= 4;
                lock_bottom/= 4;
                lock_width /= 4;
                lock_height /= 4;

                surf_width /= 4;
                surf_height /= 4;

                block_dim -= 2; // Memory blocks considers 4x4 texels as a unit.
            }

            // Read lock data node
            // Note using write access will fire again the event we are controlling !
            ::BYTE* lock_data = (::BYTE*)node->map_data(READ_ACCESS);

            ///@note Copy data in morton order
            ::BYTE* surf_data = (::BYTE*)surf_data_node->map_data(WRITE_ACCESS);
            u32bit width2 = ceiledLog2(surf_width);

            /*for(UINT i = 0; i < lock_height; i ++) {
                for(UINT j = 0; j < lock_width; j ++) {
                    u32bit m = texel2MortonAddress(lock_left + j ,lock_top + i , block_dim, s_block_dim, width2);
                    memcpy(&surf_data[m * pixel_size], &lock_data[(i * lock_width + j) * pixel_size], pixel_size);
                        }
                    }*/
                    
            switch(pixel_size)
            {
                case 1:
                    for(UINT i = 0; i < lock_height; i ++)
                    {
                        for(UINT j = 0; j < lock_width; j ++)
                        {
                            u32bit m = texel2MortonAddress(lock_left + j ,lock_top + i , block_dim, s_block_dim, width2);
                            ((u8bit *) surf_data)[m] = ((u8bit *) lock_data)[(i * lock_width + j)];
                        }
                    }
                    break;
                    
                case 2:
                    for(UINT i = 0; i < lock_height; i ++)
                    {
                        for(UINT j = 0; j < lock_width; j ++)
                        {
                            u32bit m = texel2MortonAddress(lock_left + j ,lock_top + i , block_dim, s_block_dim, width2);
                            ((u16bit *) surf_data)[m] = ((u16bit *) lock_data)[(i * lock_width + j)];
                        }
                    }
                    break;
                                        
                case 4:
                    for(UINT i = 0; i < lock_height; i ++)
                    {
                        for(UINT j = 0; j < lock_width; j ++)
                        {
                            u32bit m = texel2MortonAddress(lock_left + j ,lock_top + i , block_dim, s_block_dim, width2);
                            ((u32bit *) surf_data)[m] = ((u32bit *) lock_data)[(i * lock_width + j)];
                        }
                    }
                    break;
                    
                case 8:
                    for(UINT i = 0; i < lock_height; i ++)
                    {
                        for(UINT j = 0; j < lock_width; j ++)
                        {
                            u32bit m = texel2MortonAddress(lock_left + j ,lock_top + i , block_dim, s_block_dim, width2);
                            ((u64bit *) surf_data)[m] = ((u64bit *) lock_data)[(i * lock_width + j)];
                        }
                    }
                    break;

                case 16:
                    for(UINT i = 0; i < lock_height; i ++)
                    {
                        for(UINT j = 0; j < lock_width; j ++)
                        {
                            u32bit m = texel2MortonAddress(lock_left + j ,lock_top + i , block_dim, s_block_dim, width2);
                            ((u64bit *) surf_data)[m * 2] = ((u64bit *) lock_data)[(i * lock_width + j) * 2];
                            ((u64bit *) surf_data)[m * 2 + 1] = ((u64bit *) lock_data)[(i * lock_width + j) * 2 + 1];
                        }
                    }
                    break;
                    
                default:
                    for(UINT i = 0; i < lock_height; i ++)
                    {
                        for(UINT j = 0; j < lock_width; j ++)
                        {
                            u32bit m = texel2MortonAddress(lock_left + j ,lock_top + i , block_dim, s_block_dim, width2);
                            for(u32bit b = 0; b < pixel_size; b++)
                                ((u8bit *) surf_data)[m * pixel_size + b] = ((u8bit *) lock_data)[(i * lock_width + j) * pixel_size + b];
                         }
                    }
                    break;
            }
            
            // Finish acesses
            surf_data_node->unmap_data();
            node->unmap_data();

        }
    }
}

void CLockRect9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_SURFACE_9) {
        // Watch surface DATA for changes
        node->get_child(StateId(NAME_DATA))->add_controller(this);
    }
    else if(node->get_id().name == NAME_LOCKRECT) {
        // Watch lock DATA for changes
        node->get_child(StateId(NAME_DATA))->add_controller(this);
    }
}

void CLockRect9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_SURFACE_9) {
        // Unwatch surface DATA
        node->get_child(StateId(NAME_DATA))->remove_controller(this);
    }
    else if(node->get_id().name == NAME_LOCKRECT) {
        // Unwatch lock DATA
        node->get_child(StateId(NAME_DATA))->remove_controller(this);
    }
}

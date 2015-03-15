#include "Common.h"
#include "CLockBox_9.h"

void CLockBox9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_LOCKBOX) {
        /** @todo Perhaps it's necessary to copy volume DATA to lock DATA, depending on the flags.
            Without this, if application only modifies a region of lock DATA
            the contents of the surface DATA corresponding to a non modified regions
            of lock DATA are overwritten with zeroes.
        */

        // Calculate size of DATA node and resize it
        D3DFORMAT format;
        child->get_parent()->get_child(StateId(NAME_FORMAT))->read_data(&format);
        UINT left, right, top, bottom, front, back;
        child->get_child(StateId(NAME_LEFT))->read_data(&left);
        child->get_child(StateId(NAME_RIGHT))->read_data(&right);
        child->get_child(StateId(NAME_TOP))->read_data(&top);
        child->get_child(StateId(NAME_BOTTOM))->read_data(&bottom);
        child->get_child(StateId(NAME_TOP))->read_data(&front);
        child->get_child(StateId(NAME_BOTTOM))->read_data(&back);
        UINT width = right - left;
        UINT height = bottom - top;
        UINT depth = back - front;
        // Change units for compressed formats
        /// @note Depth is not affected
        if(is_compressed(format)) {
            width = width / 4 + (((width % 4) == 0) ? 0 : 1);
            height = height / 4 + (((height % 4) == 0) ? 0 : 1);
        }
        UINT size = depth * width * height * texel_size(format);
        child->get_child(StateId(NAME_DATA))->set_data_size(size);
        D3D_DEBUG( cout << "CLOCKBOX9: Resizing SLOCKBOX DATA to " << (int)size << endl; )
        // Write pitches
        INT row_pitch;
        row_pitch = texel_size(format) * width;
        child->get_child(StateId(NAME_ROW_PITCH))->write_data(&row_pitch);
        INT slice_pitch;
        slice_pitch = row_pitch * height;
        child->get_child(StateId(NAME_SLICE_PITCH))->write_data(&slice_pitch);

        // Watch node
        child->add_controller(this);
    }
}

void CLockBox9::on_remove_node_child(StateDataNode* parent, StateDataNode* child)  {
    if(child->get_id().name == NAME_LOCKBOX) {
        // Unwatch node
        child->remove_controller(this);
    }
}

void CLockBox9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset)  {
    // Note state nodes VOLUME and LOCKBOX have a child named DATA.
    if(node->get_id().name == NAME_DATA) {
        if(node->get_parent()->get_id().name == NAME_VOLUME_9) {

            /* When volume buffer data is modified,
               it has to be copied to the assigned GPU memory*/

            // Get the assignation table
            ResourceAssignationTable* assignation;
            assignation = AssignationsRegistry::get_table(NAME_VOLUME_MEMORY);
            // Locate assignated resource, note usageid is taken from the resource stateid
            StateId s = node->get_parent()->get_id();
            ResourceId r = assignation->get_assigned_to(UsageId(NAME_VOLUME, s.index));
            // Get assigned GPU memory descriptor,
            u32bit md = u32bit(r.index);

            // Copy modified data to GPU memory
            D3D_DEBUG( cout << "CLOCKBOX9: Writing " << size << " bytes at " << offset << " in md " << md << endl; )
            void* data = node->map_data(READ_ACCESS, size, offset);

            GPUProxy::get_instance()->writeMemory(md, offset, (u8bit*)(data), size);
            node->unmap_data();
        }
        else if(node->get_parent()->get_id().name == NAME_LOCKBOX) {

            /** When lock DATA is modified, it has to be copied to the DATA node of the
               volume **/

            // Get attributes of surface
            StateDataNode* s_vol = node->get_parent()->get_parent();
            UINT vol_width;
            s_vol->get_child(StateId(NAME_WIDTH))->read_data(&vol_width);
            UINT vol_height;
            s_vol->get_child(StateId(NAME_HEIGHT))->read_data(&vol_height);
            UINT vol_depth;
            s_vol->get_child(StateId(NAME_DEPTH))->read_data(&vol_depth);
            D3DFORMAT vol_format;
            s_vol->get_child(StateId(NAME_FORMAT))->read_data(&vol_format);

            // Get surface data node
            StateDataNode* vol_data_node = s_vol->get_child(StateId(NAME_DATA));

            // Get attributes of locked rect
            UINT lock_left;
            node->get_parent()->get_child(StateId(NAME_LEFT))->read_data(&lock_left);
            UINT lock_right;
            node->get_parent()->get_child(StateId(NAME_RIGHT))->read_data(&lock_right);
            UINT lock_top;
            node->get_parent()->get_child(StateId(NAME_TOP))->read_data(&lock_top);
            UINT lock_bottom;
            node->get_parent()->get_child(StateId(NAME_BOTTOM))->read_data(&lock_bottom);
            UINT lock_front;
            node->get_parent()->get_child(StateId(NAME_FRONT))->read_data(&lock_front);
            UINT lock_back;
            node->get_parent()->get_child(StateId(NAME_BACK))->read_data(&lock_back);

            // Calculate derived variables
            UINT lock_width = lock_right - lock_left;
            UINT lock_height = lock_bottom - lock_top;
            UINT lock_depth = lock_back - lock_front;
            UINT pixel_size = texel_size(vol_format);

            // Get blocking parameters
            u32bit block_dim = 3;
            u32bit s_block_dim = 3;

            // Consider blocks instead of pixels for compressed formats
            /// @note Depth is not affected by compression
            if(is_compressed(vol_format)) {
                lock_left /= 4;
                lock_right/= 4;
                lock_top/= 4;
                lock_bottom/= 4;
                lock_width /= 4;
                lock_height /= 4;

                vol_width /= 4;
                vol_height /= 4;

                block_dim -= 2; // Memory blocks considers 4x4 texels as a unit.
            }

            // Read lock data node
            // Note using write access will fire again the event we are controlling !
            ::BYTE* lock_data = (::BYTE*)node->map_data(READ_ACCESS);

            ///@note Copy data in morton order
            ::BYTE* vol_data = (::BYTE*)vol_data_node->map_data(WRITE_ACCESS);
            UINT largest = (vol_width > vol_height) ? vol_width : vol_height;
            u32bit largest2 = ceiledLog2(largest);
            for(UINT k = 0; k < lock_depth; k ++) {
                UINT slice_offset = k * largest * largest * pixel_size;
                for(UINT i = 0; i < lock_height; i ++) {
                    for(UINT j = 0; j < lock_width; j ++) {
                        u32bit m = texel2MortonAddress(lock_left + j ,lock_top + i , block_dim, s_block_dim, largest2);
                        memcpy(&vol_data[slice_offset + m * pixel_size], &lock_data[(k * lock_width * lock_height + i * lock_width + j) * pixel_size], pixel_size);
                    }
                }
            }
            // Finish acesses
            vol_data_node->unmap_data();
            node->unmap_data();

        }
    }
}

void CLockBox9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_VOLUME_9) {
        // Watch surface DATA for changes
        node->get_child(StateId(NAME_DATA))->add_controller(this);
    }
    else if(node->get_id().name == NAME_LOCKBOX) {
        // Watch lock DATA for changes
        node->get_child(StateId(NAME_DATA))->add_controller(this);
    }
}

void CLockBox9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_VOLUME_9) {
        // Unwatch surface DATA
        node->get_child(StateId(NAME_DATA))->remove_controller(this);
    }
    else if(node->get_id().name == NAME_LOCKBOX) {
        // Unwatch lock DATA
        node->get_child(StateId(NAME_DATA))->remove_controller(this);
    }
}

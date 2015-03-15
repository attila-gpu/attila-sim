#include "Common.h"
#include "CLock_9.h"

void CLock9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_LOCK) {
        /** @todo Perhaps it's necessary to copy v/i buffer DATA to lock DATA, depending on the flags.
            Without this, if application only modifies a region of lock DATA
            the contents of the buffer are overwritten with zeroes. */
        child->add_controller(this);
    }
}

void CLock9::on_remove_node_child(StateDataNode* parent, StateDataNode* child)  {
    if(child->get_id().name == NAME_LOCK) {
        child->remove_controller(this);
    }
}

void CLock9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset)  {
    // Note state nodes INDEXBUFFER_9 and VERTEXBUFFER_9 and LOCK have a child named DATA.
    if(node->get_id().name == NAME_DATA) {
        if((node->get_parent()->get_id().name == NAME_INDEXBUFFER_9) ||
           (node->get_parent()->get_id().name == NAME_VERTEXBUFFER_9)){

            /* When vertex/index buffer data is modified,
               it has to be copied to the assigned GPU memory*/

            // Get the assignation table
            ResourceAssignationTable* assignation;
            assignation = AssignationsRegistry::get_table(NAME_IVBUFFER_MEMORY);
            // Locate assignated resource, note usageid is taken from the resource stateid
            StateId s = node->get_parent()->get_id();

            ResourceId r = assignation->get_assigned_to(UsageId(NAME_IVBUFFER, s.index));
            // Get assigned GPU memory descriptor,
            u32bit md = u32bit(r.index);

            // Copy modified data to GPU memory
            D3D_DEBUG( D3D_DEBUG( cout << "CLOCK9: Writing " << size << " bytes at " << offset << " in md " << md << endl; ) )

            u32bit alignedOffset = offset - GPU_MOD(offset, 64);
            u32bit alignedSize = size + GPU_MOD(offset, 64);
            void* data = node->map_data(READ_ACCESS, alignedSize, alignedOffset);
            GPUProxy::get_instance()->writeMemory(md, alignedOffset, (u8bit*)(data), alignedSize);
            node->unmap_data();
        }
        else if(node->get_parent()->get_id().name == NAME_LOCK) {

            /* When lock data is modified, it has to be copied to the data node of the
               vertex/index buffer resource */

            // Get destination area in the index/vertex buffer data.
            UINT lock_offset;
            node->get_parent()->get_child(StateId(NAME_OFFSET))->read_data(&lock_offset);
            UINT lock_size;
            node->get_parent()->get_child(StateId(NAME_SIZE))->read_data(&lock_size);

            // Read lock data node
            // Note using write access will fire again the event we are controlling,
            void* lock_data = node->map_data(READ_ACCESS);

            // Navigate to the destination index/vertex buffer data node
            StateDataNode* buffer_data_node = node->get_parent()->get_parent()->get_child(StateId(NAME_DATA));
            void* buffer_data = buffer_data_node->map_data(WRITE_ACCESS, lock_size, lock_offset);

            D3D_DEBUG( D3D_DEBUG( cout << "CLOCK9: Copying " << lock_size << " bytes at " << lock_offset << endl; ) )
            //Update index/vertex buffer data with lock data
            memcpy((::BYTE*)buffer_data, lock_data, lock_size);

            // Finish acesses
            buffer_data_node->unmap_data();
            node->unmap_data();
        }
    }
}

void CLock9::on_added_controller(StateDataNode* node) {
    if((node->get_id().name == NAME_VERTEXBUFFER_9) ||
    (node->get_id().name == NAME_INDEXBUFFER_9)) {
        // Watch vertex/index buffer data for changes
        node->get_child(StateId(NAME_DATA))->add_controller(this);
    }
    else if(node->get_id().name == NAME_LOCK) {
        // Watch lock data for changes
        node->get_child(StateId(NAME_DATA))->add_controller(this);
    }
}

void CLock9::on_removed_controller(StateDataNode* node) {
    if((node->get_id().name == NAME_VERTEXBUFFER_9) ||
    (node->get_id().name == NAME_INDEXBUFFER_9)) {
        // Unwatch vertex/index buffer data
        node->get_child(StateId(NAME_DATA))->remove_controller(this);
    }
    else if(node->get_id().name == NAME_LOCK) {
        // Unwatch lock data
        node->get_child(StateId(NAME_DATA))->remove_controller(this);
    }
}

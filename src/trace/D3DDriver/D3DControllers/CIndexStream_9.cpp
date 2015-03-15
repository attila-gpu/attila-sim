#include "Common.h"
#include "CIndexStream_9.h"

CIndexStream9::CIndexStream9(): assigned_index_stream(false) {
    gpu_format[D3DFMT_INDEX16] = SD_U16BIT;
    gpu_format[D3DFMT_INDEX32] = SD_U32BIT;

    format_size[D3DFMT_INDEX16] = 2 * sizeof(BYTE);
    format_size[D3DFMT_INDEX32] = 4 * sizeof(BYTE);
}

CIndexStream9::~CIndexStream9() {
}

void CIndexStream9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_DRAW_INDEXED_PRIMITIVE) {
        // Update index stream address

        // Get d3d source index buffer
        StateDataNode* s_device;
        s_device = child->get_parent()->get_parent();
        IDirect3DIndexBuffer9* i_ib;
        s_device->get_child(StateId(NAME_CURRENT_INDEX_BUFFER))->read_data(&i_ib);
        StateDataNode* s_ib;
        s_ib = s_device->get_child(StateId(NAME_INDEXBUFFER_9, StateIndex(i_ib)));

        // Get gpu stream assigned to it
        ResourceAssignationTable* a_streams;
        a_streams = AssignationsRegistry::get_table(NAME_STREAMS);
        ResourceId r;
        r = a_streams->get_assigned_to(UsageId(NAME_INDEX_STREAM));
        u32bit gpu_stream  = r.index;

        // Get gpu memory assigned to it
        ResourceAssignationTable* a_resource_memory;
        a_resource_memory = AssignationsRegistry::get_table(NAME_IVBUFFER_MEMORY);
        r = a_resource_memory->get_assigned_to(UsageId(NAME_IVBUFFER, UsageIndex(i_ib)));
        u32bit gpu_memory = r.index;

        // Get size of an index
        D3DFORMAT format;
        s_ib->get_child(StateId(NAME_FORMAT))->read_data(&format);
        u32bit index_size = format_size[format];

        // Get start index
        UINT start_index;
        child->get_child(StateId(NAME_START_INDEX))->read_data(&start_index);

        u32bit offset;
        offset = start_index * index_size;

        // Update gpu stream address
        GPUProxy::get_instance()->writeGPUAddrRegister(GPU_STREAM_ADDRESS, gpu_stream, gpu_memory, offset);
        D3D_DEBUG( cout << "CINDEXSTREAM9: Updated address to " << gpu_memory << "[" << offset << "] "; )
        D3D_DEBUG( cout << "for gpu stream " << gpu_stream << endl; )
    }
}
void CIndexStream9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {}

void CIndexStream9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    if(node->get_id().name == NAME_CURRENT_INDEX_BUFFER) {

        // Release previously assigned index stream
        ResourceAssignationTable* a_streams;
        a_streams = AssignationsRegistry::get_table(NAME_STREAMS);
        if(assigned_index_stream) {

                // Reset gpu stream
                ResourceId r = a_streams->get_assigned_to(UsageId(NAME_INDEX_STREAM));
                u32bit gpu_stream = r.index;
                GPURegData zero_value;
                zero_value.uintVal = 0;
                GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_STRIDE, gpu_stream, zero_value);
                GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_ADDRESS, gpu_stream, zero_value);
                GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_ELEMENTS, gpu_stream, zero_value);
                GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_DATA, gpu_stream, zero_value);
                D3D_DEBUG( cout << "CINDEXSTREAM9: Reseted gpu stream " << gpu_stream << endl; )

                // Unassign
                a_streams->unassign(UsageId(NAME_INDEX_STREAM));
        }
        assigned_index_stream = false;

        // Get d3d source index buffer
        IDirect3DIndexBuffer9* i_ib;
        node->read_data(&i_ib);

        u32bit gpu_memory;
        if(i_ib != 0) {

            // Get gpu memory assigned to it
            ResourceAssignationTable* a_resource_memory;
            a_resource_memory = AssignationsRegistry::get_table(NAME_IVBUFFER_MEMORY);
            ResourceId r = a_resource_memory->get_assigned_to(UsageId(NAME_IVBUFFER, UsageIndex(i_ib)));
            gpu_memory = r.index;

            // Assign a gpu stream
            ResourceAssignationTable* a_streams;
            a_streams = AssignationsRegistry::get_table(NAME_STREAMS);
            r = a_streams->assign(UsageId(NAME_INDEX_STREAM), NAME_STREAM);
            u32bit gpu_stream  = r.index;

            // Remember it's assigned
            assigned_index_stream = true;

            D3D_DEBUG( cout << "CINDEXSTREAM9: Using gpu stream " << gpu_stream << " for indices" << endl; )

            // Configure the gpu to use this stream
            GPURegData data;
            data.uintVal = gpu_stream;
            GPUProxy::get_instance()->writeGPURegister(GPU_INDEX_STREAM, data);

            // Update gpu stream type using the index buffer format
            D3DFORMAT format;
            StateDataNode* s_index_buffer = node->get_parent()->get_child(StateId(NAME_INDEXBUFFER_9, StateIndex(i_ib)));
            s_index_buffer->get_child(StateId(NAME_FORMAT))->read_data(&format);
            data.streamData = gpu_format[format];
            GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_DATA, gpu_stream, data);

            // Update gpu stream stride
            data.uintVal = format_size[format];
            GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_STRIDE, gpu_stream, data);
            D3D_DEBUG( cout << "CINDEXSTREAM9: Setted stride to " << (int)data.uintVal; )
            D3D_DEBUG( cout << "for gpu stream " << gpu_stream << endl; )

            // Update gpu stream elements
            data.uintVal = 1;
            GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_ELEMENTS, gpu_stream, data);

        }
    }
}

void CIndexStream9::on_added_controller(StateDataNode*node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        node->get_child(StateId(NAME_CURRENT_INDEX_BUFFER))->add_controller(this);
        node->get_child(StateId(NAME_COMMANDS))->add_controller(this);
        assigned_index_stream = false;
    }
}

void CIndexStream9::on_removed_controller(StateDataNode*node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        node->get_child(StateId(NAME_COMMANDS))->remove_controller(this);
        node->get_child(StateId(NAME_CURRENT_INDEX_BUFFER))->remove_controller(this);
    }
}

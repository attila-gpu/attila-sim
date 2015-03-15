#include "Common.h"
#include "CStreams_9.h"

CStreams9::CStreams9() {

    // Initialize element type map
    gpu_element_type[D3DDECLTYPE_FLOAT2] = GPUElementType(SD_FLOAT, 2);
    gpu_element_type[D3DDECLTYPE_FLOAT3] = GPUElementType(SD_FLOAT, 3);
    gpu_element_type[D3DDECLTYPE_FLOAT4] = GPUElementType(SD_FLOAT, 4);
    gpu_element_type[D3DDECLTYPE_D3DCOLOR] = GPUElementType(SD_U8BIT, 4);
    gpu_element_type[D3DDECLTYPE_UBYTE4] = GPUElementType(SD_U8BIT, 4);
      /// @todo Complete the element type map
    // gpu_element_type[D3DDECLTYPE_SHORT2] =
    // gpu_element_type[D3DDECLTYPE_SHORT4] =
    // gpu_element_type[D3DDECLTYPE_UBYTE4N] =
    // gpu_element_type[D3DDECLTYPE_SHORT2N] =
    // gpu_element_type[D3DDECLTYPE_SHORT4N] =
    // gpu_element_type[D3DDECLTYPE_USHORT2N] =
    // gpu_element_type[D3DDECLTYPE_USHORT4N] =
    // gpu_element_type[D3DDECLTYPE_UDEC3] =
    // gpu_element_type[D3DDECLTYPE_DEC3N] =
    // gpu_element_type[D3DDECLTYPE_FLOAT16_2] =
    // gpu_element_type[D3DDECLTYPE_FLOAT16_4] =
    // gpu_element_type[D3DDECLTYPE_UNUSED] =

    /// @todo complete this mapping
    gpu_primitive_mode[D3DPT_TRIANGLELIST] = TRIANGLE;
    gpu_primitive_mode[D3DPT_TRIANGLESTRIP] = TRIANGLE_STRIP;
    gpu_primitive_mode[D3DPT_TRIANGLEFAN] = TRIANGLE_FAN;

}

CStreams9::~CStreams9() {

}


void CStreams9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_DRAW_PRIMITIVE) {
         D3D_DEBUG( cout << "CSTREAMS9: Configuring non indexed batch" << endl; )

         // Read state data
         UINT start_vertex;
         child->get_child(StateId(NAME_START_VERTEX))->read_data(&start_vertex);
         UINT prim_count;
         child->get_child(StateId(NAME_PRIMITIVE_COUNT))->read_data(&prim_count);
         D3DPRIMITIVETYPE prim_mode;
         child->get_child(StateId(NAME_PRIMITIVE_MODE))->read_data(&prim_mode);

         D3D_DEBUG( cout << "CSTREAMS9: Vertex count is " << get_vertex_count(prim_mode, prim_count) << endl; )

         // Configure the gpu for the batch
         GPUProxy* gpu = GPUProxy::get_instance();
         GPURegData data;
         data.uintVal = start_vertex;
         gpu->writeGPURegister(GPU_STREAM_START, data);
         data.uintVal = get_vertex_count(prim_mode, prim_count);
         gpu->writeGPURegister(GPU_STREAM_COUNT, data);
         data.booleanVal = false;
         gpu->writeGPURegister(GPU_INDEX_MODE, data);

         map< D3DPRIMITIVETYPE, PrimitiveMode > :: iterator it_pm;
         it_pm = gpu_primitive_mode.find(prim_mode);
         /// @todo error if primitive mode not supported
         if(it_pm != gpu_primitive_mode.end())
            data.primitive = (*it_pm).second;
         gpu->writeGPURegister(GPU_PRIMITIVE, data);
    }
    else if(child->get_id().name == NAME_DRAW_INDEXED_PRIMITIVE) {

         D3D_DEBUG( cout << "CSTREAMS9: Configuring indexed batch" << endl; )

         // Read state data
         UINT start_index;
         child->get_child(StateId(NAME_START_INDEX))->read_data(&start_index);
         UINT prim_count;
         child->get_child(StateId(NAME_PRIMITIVE_COUNT))->read_data(&prim_count);
         D3DPRIMITIVETYPE prim_mode;
         child->get_child(StateId(NAME_PRIMITIVE_MODE))->read_data(&prim_mode);

         D3D_DEBUG( cout << "CSTREAMS9: Vertex count is " << get_vertex_count(prim_mode, prim_count) << endl; )

         // Configure the gpu for the batch
         GPUProxy* gpu = GPUProxy::get_instance();
         GPURegData data;
         data.uintVal = 0;
         gpu->writeGPURegister(GPU_STREAM_START, data);
         data.uintVal = get_vertex_count(prim_mode, prim_count);
         gpu->writeGPURegister(GPU_STREAM_COUNT, data);
         data.booleanVal = true;
         gpu->writeGPURegister(GPU_INDEX_MODE, data);

         map< D3DPRIMITIVETYPE, PrimitiveMode > :: iterator it_pm;
         it_pm = gpu_primitive_mode.find(prim_mode);
         /// @todo error if primitive mode not supported
         if(it_pm != gpu_primitive_mode.end())
            data.primitive = (*it_pm).second;
         gpu->writeGPURegister(GPU_PRIMITIVE, data);
    }


}

void CStreams9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
}

void CStreams9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    if(node->get_id().name == NAME_CURRENT_VERTEX_DECLARATION) {

        // Unassign elements
        ResourceAssignationTable* a_element_stream;
        a_element_stream = AssignationsRegistry::get_table(NAME_ELEMENT_STREAM);
        set< UsageId >::iterator it_assigned;
        for(it_assigned = elements_assigned.begin(); it_assigned != elements_assigned.end(); it_assigned ++) {
            a_element_stream->unassign(*it_assigned);
        }
        elements_assigned.clear();

        // Unassign gpu streams
        ResourceAssignationTable* a_streams;
        a_streams = AssignationsRegistry::get_table(NAME_STREAMS);

        // Iterate over d3d streams
        set< UsageId >::iterator it_d3d_stream;
         for(it_d3d_stream = streams_assigned.begin(); it_d3d_stream != streams_assigned.end(); it_d3d_stream ++) {
             // Get assigned gpu streams
            list< ResourceId > gpu_streams = a_streams->get_all_assigned_to(*it_d3d_stream);
            list< ResourceId >::iterator it_gpu_stream;
            // Reset them
            for(it_gpu_stream = gpu_streams.begin(); it_gpu_stream != gpu_streams.end(); it_gpu_stream ++) {
                u32bit gpu_stream = u32bit((*it_gpu_stream).index);
                reset_gpu_stream(gpu_stream);
            }
            // Finally unassign

            a_streams->unassign(*it_d3d_stream);
         }
         streams_assigned.clear();

         // Clear element offsets
         gpu_element_offset.clear();


        // Get vertex declaration state from device state
        IDirect3DVertexDeclaration9* i_vdec;
        node->read_data(&i_vdec);
        if(i_vdec != 0) {

            set<UINT> d3d_streams_to_update;

            StateDataNode* s_vdec;
            s_vdec = node->get_parent()->get_child(StateId(NAME_VERTEX_DECLARATION_9, StateIndex(i_vdec)));

            UINT element_count;
            s_vdec->get_child(StateId(NAME_ELEMENT_COUNT))->read_data(&element_count);
            for(UINT i = 0; i < element_count; i ++) {

                // Get element state
                StateDataNode* s_velem = s_vdec->get_child(StateId(NAME_VERTEX_ELEMENT, StateIndex(i)));

                // Get d3d element type
                ::BYTE type;
                s_velem->get_child(StateId(NAME_TYPE))->read_data(&type);

                // Get gpu type for this d3d element type
                map< D3DDECLTYPE, GPUElementType > :: iterator it_t;
                it_t = gpu_element_type.find(D3DDECLTYPE(type));

                if(it_t == gpu_element_type.end())
                    // Element type is not supported
                    D3D_DEBUG( cout << "WARNING: Unsupported element type " << type << endl; )
                else {
                    // Element type is supported, so configure gpu stream

                    // Assign one gpu stream with d3d stream
                    WORD stream;
                    s_velem->get_child(StateId(NAME_STREAM))->read_data(&stream);
                    UsageId stream_usage(NAME_VERTEX_STREAM, stream);
                    ResourceId r;
                    r = a_streams->assign(stream_usage, NAME_STREAM);
                    u32bit gpu_stream = u32bit(r.index);

                    // Remember this assignation
                    streams_assigned.insert(stream_usage);

                    D3D_DEBUG( cout << "CSTREAMS9: Assigned gpu stream " << gpu_stream; )
                    D3D_DEBUG( cout << " to d3d stream " << stream << endl; )

                    // Assign gpu stream to this element
                    ::BYTE usage;
                    s_velem->get_child(StateId(NAME_USAGE))->read_data(&usage);
                    ::BYTE usage_index;
                    s_velem->get_child(StateId(NAME_USAGE_INDEX))->read_data(&usage_index);
                    Names usage_name = usage2name(usage);
                    UsageId element_usage(usage_name, UsageIndex(usage_index));
                    a_element_stream->assign(element_usage, ResourceId(NAME_STREAM, gpu_stream));

                    D3D_DEBUG( cout << "CSTREAMS9: Assigned gpu stream " << gpu_stream; )
                    //D3D_DEBUG( cout << " to element " << usage2str(usage) << "[" << (int)usage_index << "]" << endl; )

                    // Remember this assignation
                    elements_assigned.insert(element_usage);

                    // Update gpu stream with gpu element type
                    GPUElementType gpu_type = (*it_t).second;
                    GPURegData gpu_stream_elements; // In gpu "elements" means how many components has the type
                    gpu_stream_elements.uintVal = gpu_type.count;
                    GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_ELEMENTS, gpu_stream, gpu_stream_elements);
                    GPURegData gpu_stream_data;
                    gpu_stream_data.streamData = gpu_type.data;
                    GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_DATA, gpu_stream, gpu_stream_data);
                    //Set D3D9 color components order if needed
                    if(usage == D3DDECLUSAGE_COLOR) {
                        D3D_DEBUG( cout << "CSTREAMS9: Enabling D3D9 color order in gpu stream " << (int)gpu_stream << endl; )
                        GPURegData d3d9color;
                        d3d9color.booleanVal = true;
                        GPUProxy::get_instance()->writeGPURegister(GPU_D3D9_COLOR_STREAM, gpu_stream, d3d9color); // Oe!
                    }


                    // Store offset to update this gpu stream
                    WORD offset;
                    s_velem->get_child(StateId(NAME_OFFSET))->read_data(&offset);
                    gpu_element_offset[gpu_stream] = offset;

                    // Mark d3d stream for update later
                    d3d_streams_to_update.insert(stream);
                }
            }

            /* Update gpu streams assigned to d3d streams marked to update*/
            set< UINT >::iterator it_s;
            for(it_s = d3d_streams_to_update.begin(); it_s != d3d_streams_to_update.end(); it_s ++) {
                UINT d3d_stream_index = (*it_s);
                // Get state node for this stream
                StateDataNode* s_stream = node->get_parent()->get_child(StateId(NAME_STREAM, StateIndex(d3d_stream_index)));
                update_gpu_stream_addresses(s_stream);
                update_gpu_stream_stride(s_stream);
            }
        }

    }
    else if(node->get_id().name == NAME_STRIDE) {
        StateDataNode* s_stream = node->get_parent();
        update_gpu_stream_stride(s_stream);
    }
    else if(node->get_id().name == NAME_OFFSET) {
        StateDataNode* s_stream = node->get_parent();
        update_gpu_stream_addresses(s_stream);
    }

    else if(node->get_id().name == NAME_SOURCE) {
    	// Nothing to do
    	///@see The note in IDeviceImpl9::SetStreamSource
    }

}

void CStreams9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // Watch state
        node->get_child(StateId(NAME_CURRENT_VERTEX_DECLARATION))->add_controller(this);

        UINT stream_count;
        node->get_child(StateId(NAME_STREAM_COUNT))->read_data(&stream_count);
        for(UINT i = 0; i < stream_count; i ++) {
            StateDataNode* s_stream = node->get_child(StateId(NAME_STREAM, i));
            s_stream->get_child(StateId(NAME_STRIDE))->add_controller(this);
            s_stream->get_child(StateId(NAME_OFFSET))->add_controller(this);
            s_stream->get_child(StateId(NAME_SOURCE))->add_controller(this);
        }

        node->get_child(StateId(NAME_COMMANDS))->add_controller(this);

    }
}

void CStreams9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {

        node->get_child(StateId(NAME_COMMANDS))->remove_controller(this);

        // Unwatch state
        UINT stream_count;
        node->get_child(StateId(NAME_STREAM_COUNT))->read_data(&stream_count);
        for(UINT i = 0; i < stream_count; i ++) {
            StateDataNode* s_stream = node->get_child(StateId(NAME_STREAM, i));
            s_stream->get_child(StateId(NAME_SOURCE))->remove_controller(this);
            s_stream->get_child(StateId(NAME_OFFSET))->remove_controller(this);
            s_stream->get_child(StateId(NAME_STRIDE))->remove_controller(this);
        }

        node->get_child(StateId(NAME_CURRENT_VERTEX_DECLARATION))->remove_controller(this);
    }
}


void CStreams9::update_gpu_stream_addresses(StateDataNode* s_stream) {

    // Check if the d3d stream has gpu streams assigned
    UINT d3d_stream = s_stream->get_id().index;
    ResourceAssignationTable* a_streams;
    a_streams = AssignationsRegistry::get_table(NAME_STREAMS);
    if(a_streams->has_been_assigned(UsageId(NAME_VERTEX_STREAM, d3d_stream))) {

        // Get d3d source vertex buffer
        IDirect3DVertexBuffer9* i_vb;
        s_stream->get_child(StateId(NAME_SOURCE))->read_data(&i_vb);

        u32bit gpu_memory;
        if(i_vb != 0) {
            // Get gpu memory assigned to it
            ResourceAssignationTable* a_resource_memory;
            a_resource_memory = AssignationsRegistry::get_table(NAME_IVBUFFER_MEMORY);
            ResourceId r = a_resource_memory->get_assigned_to(UsageId(NAME_IVBUFFER, UsageIndex(i_vb)));
            gpu_memory = r.index;

            // Get d3d stream offset
            UINT d3d_stream_offset;
            s_stream->get_child(StateId(NAME_OFFSET))->read_data(&d3d_stream_offset);

            // Get gpu streams that are assigned to this d3d stream
            list< ResourceId > gpu_streams;
            gpu_streams = a_streams->get_all_assigned_to(UsageId(NAME_VERTEX_STREAM, d3d_stream));

            // Update gpu stream address
            list< ResourceId >::iterator it_s;
            for(it_s = gpu_streams.begin(); it_s != gpu_streams.end(); it_s ++) {
                u32bit gpu_stream = (*it_s).index;
                // Calculate gpu stream offset and write address at this offset
                u32bit gpu_stream_offset;
                gpu_stream_offset = d3d_stream_offset + gpu_element_offset[gpu_stream];
                GPUProxy::get_instance()->writeGPUAddrRegister(GPU_STREAM_ADDRESS, gpu_stream, gpu_memory, gpu_stream_offset);

                D3D_DEBUG( cout << "CSTREAMS9: Updated address to " << gpu_memory << "[" << gpu_stream_offset << "]"; )
                D3D_DEBUG( cout << "for gpu stream " << gpu_stream << endl; )
            }
        }
        else {
            // Get gpu streams that are assigned to this d3d stream
            list< ResourceId > gpu_streams;
            gpu_streams = a_streams->get_all_assigned_to(UsageId(NAME_VERTEX_STREAM, d3d_stream));

            // Set gpu stream addresses to 0
            list< ResourceId >::iterator it_s;
            for(it_s = gpu_streams.begin(); it_s != gpu_streams.end(); it_s ++) {
                u32bit gpu_stream = (*it_s).index;
                GPURegData zero_value;
                zero_value.uintVal = 0;
                GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_ADDRESS, gpu_stream, zero_value);

                D3D_DEBUG( cout << "CSTREAMS9: Updated address to 0 for gpu stream " << gpu_stream << endl; )
            }
        }
    }
}

void CStreams9::update_gpu_stream_stride(StateDataNode* s_stream) {

    UINT d3d_stream = s_stream->get_id().index;

    ResourceAssignationTable* a_streams;
    a_streams = AssignationsRegistry::get_table(NAME_STREAMS);

    if(a_streams->has_been_assigned(UsageId(NAME_VERTEX_STREAM, d3d_stream))) {

        // Get gpu streams that are assigned to this d3d stream
        list< ResourceId > gpu_streams;
        gpu_streams = a_streams->get_all_assigned_to(UsageId(NAME_VERTEX_STREAM, d3d_stream));

        // Propagate d3d stream stride to all assignated gpu streams
        list< ResourceId >::iterator it_s;
        for(it_s = gpu_streams.begin(); it_s != gpu_streams.end(); it_s ++) {
            u32bit gpu_stream = (*it_s).index;
            UINT stride;
            s_stream->get_child(StateId(NAME_STRIDE))->read_data(&stride);
            GPURegData gpu_stride;
            gpu_stride.uintVal = stride;
            GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_STRIDE, gpu_stream, gpu_stride);

            D3D_DEBUG( cout << "CSTREAMS9: Updated stride to " << stride << "for gpu stream " << gpu_stream << endl; )
        }
    }
}

void CStreams9::reset_gpu_stream(u32bit gpu_stream) {
    GPURegData zero_value;
    zero_value.uintVal = 0;
    GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_STRIDE, gpu_stream, zero_value);
    GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_ADDRESS, gpu_stream, zero_value);
    GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_ELEMENTS, gpu_stream, zero_value);
    GPUProxy::get_instance()->writeGPURegister(GPU_STREAM_DATA, gpu_stream, zero_value);
    GPURegData d3d9color;
    d3d9color.booleanVal = false;
    GPUProxy::get_instance()->writeGPURegister(GPU_D3D9_COLOR_STREAM, gpu_stream, d3d9color);


    D3D_DEBUG( cout << "CSTREAMS9: Reseted gpu stream " << gpu_stream << endl; )
}

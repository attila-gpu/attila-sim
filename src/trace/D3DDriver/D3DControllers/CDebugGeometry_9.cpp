#include "Common.h"
#include "CDebugGeometry_9.h"
#include <limits>


void CDebugGeometry9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_DRAW_INDEXED_PRIMITIVE) {
        set_constants(child);
        GPUProxy::get_instance()->sendCommand(GPU_CLEARCOLORBUFFER);
        GPUProxy::get_instance()->sendCommand(GPU_CLEARZSTENCILBUFFER);
    }
}

void CDebugGeometry9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    /// @todo
}

void CDebugGeometry9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    /// @todo
}

void CDebugGeometry9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {

        // Add to commands
        node->get_child(StateId(NAME_COMMANDS))->add_controller(this);

        // Assign vs input/output registers and load vertex shader
        ResourceAssignationTable* a_vs_input;
        a_vs_input = AssignationsRegistry::get_table(NAME_VS_INPUT);
        a_vs_input->assign(UsageId(NAME_POSITION), ResourceId(NAME_REGISTER, 0));
        a_vs_input->assign(UsageId(NAME_TEXCOORD), ResourceId(NAME_REGISTER, 8));
        ResourceAssignationTable* a_vs_output;
        a_vs_output = AssignationsRegistry::get_table(NAME_VS_OUTPUT);
        a_vs_output->assign(UsageId(NAME_POSITION), ResourceId(NAME_REGISTER, 0));
        a_vs_output->assign(UsageId(NAME_TEXCOORD), ResourceId(NAME_REGISTER, 8));
        load_vertex_shader();

        // Assign ps input and load fragment shader
        ResourceAssignationTable* a_ps_input;
        a_ps_input = AssignationsRegistry::get_table(NAME_PS_INPUT);
        a_ps_input->assign(UsageId(NAME_POSITION), ResourceId(NAME_REGISTER, 0));
        a_ps_input->assign(UsageId(NAME_TEXCOORD), ResourceId(NAME_REGISTER, 8));
        load_pixel_shader();


    }
}

void CDebugGeometry9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {

        // Unassign ps input
        ResourceAssignationTable* a_ps_input;
        a_ps_input = AssignationsRegistry::get_table(NAME_PS_INPUT);
        a_ps_input->unassign(UsageId(NAME_POSITION));
        a_ps_input->unassign(UsageId(NAME_TEXCOORD));
        // Unassign vs output
        ResourceAssignationTable* a_vs_output;
        a_vs_output = AssignationsRegistry::get_table(NAME_VS_OUTPUT);
        a_vs_output->unassign(UsageId(NAME_POSITION));
        a_vs_output->unassign(UsageId(NAME_TEXCOORD));
        // Unassign vs input registers
        ResourceAssignationTable* a_vs_input;
        a_vs_input = AssignationsRegistry::get_table(NAME_VS_INPUT);
        a_vs_input->unassign(UsageId(NAME_POSITION));
        a_vs_input->unassign(UsageId(NAME_TEXCOORD));
        // Remove shaders
        unload_pixel_shader();
        unload_vertex_shader();

        // Remove from commands
        node->get_child(StateId(NAME_COMMANDS))->remove_controller(this);

    }
}

void CDebugGeometry9::load_vertex_shader() {


    D3D_DEBUG( cout << "CDEBUGGEOMETRY9: Loading vertex shader" << endl; )

    list<ShaderInstruction*> instructions;

    ShaderInstructionBuilder builder;
    Operand op0;
    Operand op1;
    Result res;

    // Compose the vertex shader
    instructions.push_back(generate_mov(GPURegisterId(0, TEMP), GPURegisterId(8, IN)));

    builder.setOpcode(gpu3d::MAX);
    op0.negate = true;
    op0.registerId = GPURegisterId(0, TEMP);
    builder.setOperand(0, op0);
    op1.registerId = GPURegisterId(0, TEMP);
    builder.setOperand(1, op1);
    res.registerId = GPURegisterId(8, OUT);
    builder.setResult(res);
    instructions.push_back(builder.buildInstruction());

    builder.resetParameters(); op0 = op1 = Operand(); res = Result();
    builder.setOpcode(gpu3d::DP4);
    op0.registerId = GPURegisterId(0, IN);
    builder.setOperand(0, op0);
    op1.registerId = GPURegisterId(0, PARAM);
    builder.setOperand(1, op1);
    res.registerId = GPURegisterId(0, OUT);
    res.maskMode = XNNN;
    builder.setResult(res);
    instructions.push_back(builder.buildInstruction());

    builder.resetParameters(); op0 = op1 = Operand(); res = Result();
    builder.setOpcode(gpu3d::DP4);
    op0.registerId = GPURegisterId(0, IN);
    builder.setOperand(0, op0);
    op1.registerId = GPURegisterId(1, PARAM);
    builder.setOperand(1, op1);
    res.registerId = GPURegisterId(0, OUT);
    res.maskMode = NYNN;
    builder.setResult(res);
    instructions.push_back(builder.buildInstruction());

    builder.resetParameters(); op0 = op1 = Operand(); res = Result();
    builder.setOpcode(gpu3d::DP4);
    op0.registerId = GPURegisterId(0, IN);
    builder.setOperand(0, op0);
    op1.registerId = GPURegisterId(2, PARAM);
    builder.setOperand(1, op1);
    res.registerId = GPURegisterId(0, OUT);
    res.maskMode = NNZN;
    builder.setResult(res);
    instructions.push_back(builder.buildInstruction());

    builder.resetParameters(); op0 = op1 = Operand(); res = Result();
    builder.setOpcode(gpu3d::DP4);
    op0.registerId = GPURegisterId(0, IN);
    builder.setOperand(0, op0);
    op1.registerId = GPURegisterId(3, PARAM);
    builder.setOperand(1, op1);
    res.registerId = GPURegisterId(0, OUT);
    res.maskMode = NNNW;
    builder.setResult(res);
    instructions.push_back(builder.buildInstruction());

    builder.resetParameters(); op0 = op1 = Operand(); res = Result();
    builder.setOpcode(gpu3d::END);
    instructions.push_back(builder.buildInstruction());

    list<ShaderInstruction*> ending = generate_ending_nops();
    instructions.insert(instructions.end(), ending.begin(), ending.end());

    // Create a buffer for shader
    u32bit program_size = instructions.size() * 16;
    u8bit *memory = new u8bit[program_size];

    // Copy binary to buffer and print assembly
    char line[80];
    list<ShaderInstruction *>::iterator it_i;
    u32bit i = 0;
    for(it_i = instructions.begin(); it_i != instructions.end(); it_i ++) {
        (*it_i)->getCode(&memory[i * 16]);
        (*it_i)->disassemble(line);
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: " << line << endl; )
        delete (*it_i);
        i ++;
    }

    // Copy buffer to gpu memory
    GPUProxy* gpu = GPUProxy::get_instance();
    assigned_vs_memory = gpu->obtainMemory(program_size);
    gpu->writeMemory(assigned_vs_memory , memory, program_size);
    delete [] memory;

    // Setup vertex shader on gpu and load it
    gpu->writeGPUAddrRegister(GPU_VERTEX_PROGRAM, 0, assigned_vs_memory );

    GPURegData data;
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_PC, data);

    data.uintVal = program_size;
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_SIZE, data);

    gpu->sendCommand(GPU_LOAD_VERTEX_PROGRAM);
}

void CDebugGeometry9::load_pixel_shader() {

    D3D_DEBUG( cout << "CDEBUGGEOMETRY9: Loading pixel shader" << endl; )

    // Create pixel shader instructions
    list<ShaderInstruction*> instructions;
    instructions.push_back(generate_mov(GPURegisterId(0, OUT), GPURegisterId(0, IN)));
    instructions.push_back(generate_mov(GPURegisterId(1, OUT), GPURegisterId(8, IN)));

    ShaderInstructionBuilder builder;

    builder.resetParameters();
    builder.setOpcode(gpu3d::END);
    instructions.push_back(builder.buildInstruction());
    list<ShaderInstruction*> ending = generate_ending_nops();
    instructions.insert(instructions.end(), ending.begin(), ending.end());

    // Create a buffer
    u32bit program_size = instructions.size() * 16;
    u8bit *memory = new u8bit[program_size];

    // Store shader in buffer and and print assembly code
    char line[80];
    list<ShaderInstruction *>::iterator it_i;
    u32bit i = 0;
    for(it_i = instructions.begin(); it_i != instructions.end(); it_i ++) {
        (*it_i)->getCode(&memory[i * 16]);
        (*it_i)->disassemble(line);
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: " << line << endl; )
        delete (*it_i);
        i ++;
    }


    // Copy buffer to gpu memory
    GPUProxy* gpu = GPUProxy::get_instance();
    assigned_ps_memory  = gpu->obtainMemory(program_size);
    gpu->writeMemory(assigned_ps_memory  , memory, program_size);
    delete [] memory;

    // Setup and load shader on gpu
    gpu->writeGPUAddrRegister(GPU_FRAGMENT_PROGRAM, 0, assigned_ps_memory);
    GPURegData data;
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_PC, data);
    data.uintVal = program_size;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_SIZE, data);
    gpu->sendCommand(GPU_LOAD_FRAGMENT_PROGRAM);

}

void CDebugGeometry9::unload_pixel_shader() {

    D3D_DEBUG( cout << "CDEBUGGEOMETRY9: Resetting pixel shader" << endl; )

    GPUProxy* gpu = GPUProxy::get_instance();
    gpu->releaseMemory(assigned_ps_memory);
    assigned_ps_memory = 0;
    GPURegData data;
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM, data);
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_SIZE, data);
}

void CDebugGeometry9::unload_vertex_shader() {

    D3D_DEBUG( cout << "CDEBUGGEOMETRY9: Resetting vertex shader" << endl; )


    GPUProxy* gpu = GPUProxy::get_instance();

    gpu->releaseMemory(assigned_vs_memory);
    assigned_vs_memory = 0;

    GPURegData data;
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM, data);
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_SIZE, data);
}

void CDebugGeometry9::set_constants(StateDataNode* s_draw_indexed) {

    // Get position element data
    WORD position_stream;
    WORD position_offset;
    ::BYTE position_type;
    StateDataNode* s_device = s_draw_indexed->get_parent()->get_parent();
    bool position_found;
    position_found = find_position(s_device, &position_stream, &position_offset, &position_type);
    if(position_found) {
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: Position found at stream " << position_stream; )
        D3D_DEBUG( cout << " offset " << position_offset << endl; )
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: Type is " << (int)(position_type) << endl; )
    }
    else {
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: WARNING: Position element not found" << endl; )
        return;
    }

    if(position_type != D3DDECLTYPE_FLOAT3) {
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: WARNING: Vertex position format not supported" << endl; )
        return;
    }


    // Get stream info for position element's stream
    UINT stream_offset;
    UINT stream_stride;
    StateDataNode* s_stream;
    s_stream = s_device->get_child(StateId(NAME_STREAM, StateIndex(position_stream)));
    IDirect3DVertexBuffer9* i_stream_source;
    s_stream->get_child(StateId(NAME_OFFSET))->read_data(&stream_offset);
    s_stream->get_child(StateId(NAME_STRIDE))->read_data(&stream_stride);
    s_stream->get_child(StateId(NAME_SOURCE))->read_data(&i_stream_source);

    // Get index buffer info
    UINT ib_length;
    void *ib_data;
    D3DFORMAT ib_format;
    IDirect3DIndexBuffer9* i_ib;
    s_device->get_child(StateId(NAME_CURRENT_INDEX_BUFFER))->read_data(&i_ib);
    if(i_ib == 0) {
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: WARNING: No indices attached to device" << endl; )
        return;
    }
    StateDataNode* s_ib;
    s_ib = s_device->get_child(StateId(NAME_INDEXBUFFER_9, StateIndex(i_ib)));
    s_ib->get_child(StateId(NAME_LENGTH))->read_data(&ib_length);
    s_ib->get_child(StateId(NAME_FORMAT))->read_data(&ib_format);
    if(ib_format != D3DFMT_INDEX16) {
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: WARNING: Index format not supported" << endl; )
        return;
    }
    ib_data = malloc(ib_length);
    s_ib->get_child(StateId(NAME_DATA))->read_data(ib_data);

    // Get vertex buffer info
    UINT vb_length;
    void *vb_data;
    if(i_stream_source == 0) {
        D3D_DEBUG( cout << "CDEBUGGEOMETRY9: WARNING: No vertex buffer attached to stream" << endl; )
        return;
    }
    StateDataNode* s_vb;
    s_vb = s_device->get_child(StateId(NAME_VERTEXBUFFER_9, StateIndex(i_stream_source)));
    s_vb->get_child(StateId(NAME_LENGTH))->read_data(&vb_length);
    vb_data = malloc(vb_length);
    s_vb->get_child(StateId(NAME_DATA))->read_data(vb_data);

    // Get draw indexed command arguments
    UINT start_index;
    D3DPRIMITIVETYPE prim_type;
    UINT primitive_count;
    s_draw_indexed->get_child(StateId(NAME_START_INDEX))->read_data(&start_index);
    s_draw_indexed->get_child(StateId(NAME_PRIMITIVE_COUNT))->read_data(&primitive_count);
    s_draw_indexed->get_child(StateId(NAME_PRIMITIVE_MODE))->read_data(&prim_type);
    UINT total_indices = get_vertex_count(prim_type, primitive_count);

    // Calculate the bounding box
    D3DVECTOR bounding_min;
    D3DVECTOR bounding_max;
    get_bounding_box(&bounding_min, &bounding_max,
        vb_data, ib_data, stream_offset,
        position_offset, stream_stride,
        start_index, total_indices);

    // Free data
    free(vb_data);
    free(ib_data);

    // Build a matrix that fits everything inside a normalized centered box
    D3DMATRIX matrix;
    get_matrix(&matrix, &bounding_min, &bounding_max);

    // Set matrix into vertex shader constants
    // note that is stored by rows
    GPURegData data;
    for(int i = 0; i < 4; i ++) {
        for(int j = 0; j < 4; j ++) {
            data.qfVal[j] = matrix.m[i][j];
        }
        GPUProxy::get_instance()->writeGPURegister(GPU_VERTEX_CONSTANT, i, data);
    }
}

void CDebugGeometry9::get_matrix(D3DMATRIX* matrix, D3DVECTOR* bounding_min, D3DVECTOR* bounding_max) {
    D3DVECTOR offset;
    D3DMATRIX translate;
    offset.x = -bounding_min->x - (bounding_max->x - bounding_min->x) / 2.0f;
    offset.y = -bounding_min->y - (bounding_max->y - bounding_min->y) / 2.0f;
    offset.z = -bounding_min->z;
    d3d_translate_matrix(&translate, &offset);

    D3DVECTOR factor;
    D3DVECTOR range;
    D3DMATRIX scale;
    range.x = bounding_max->x - bounding_min->x;
    range.y = bounding_max->y - bounding_min->y;
    range.z = bounding_max->z - bounding_min->z;
    factor.x = (range.x != 0) ? 1.0f / range.x : 0.0f;
    factor.y = (range.y != 0) ? 1.0f / range.y : 0.0f;
    factor.z = (range.z != 0) ? 1.0f / range.z : 0.0f;
    d3d_scale_matrix(&scale, &factor);

    d3d_multiply_matrix(matrix, &scale, &translate);
}

void CDebugGeometry9::get_bounding_box(D3DVECTOR* bounding_min, D3DVECTOR* bounding_max, void* vertex_data, void* index_data, UINT stream_offset, UINT position_offset, UINT stride, UINT start_index, UINT total_indices) {
    /** @note Usage of -max() instead of min(). Actually min returns a positive value!
              http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2005/n1880.htm
    */

    #undef max

    bounding_min->x = bounding_min->y = bounding_min->z = std::numeric_limits<FLOAT>::max();
    bounding_max->x = bounding_max->y = bounding_max->z = -std::numeric_limits<FLOAT>::max();

    for(UINT i = 0; i < total_indices; i ++) {
        u16bit index = ((u16bit*)(index_data))[start_index + i];
        FLOAT* position_data = (FLOAT*)&(((::BYTE*)(vertex_data))[stream_offset + position_offset + stride * index]);
        bounding_min->x = (position_data[0] < bounding_min->x) ? position_data[0] : bounding_min->x;
        bounding_min->y = (position_data[1] < bounding_min->y) ? position_data[1] : bounding_min->y;
        bounding_min->z = (position_data[2] < bounding_min->z) ? position_data[2] : bounding_min->z;
        bounding_max->x = (position_data[0] > bounding_max->x) ? position_data[0] : bounding_max->x;
        bounding_max->y = (position_data[1] > bounding_max->y) ? position_data[1] : bounding_max->y;
        bounding_max->z = (position_data[2] > bounding_max->z) ? position_data[2] : bounding_max->z;
    }
}



bool CDebugGeometry9::find_position(StateDataNode* s_device, WORD* position_stream,
        WORD* position_offset, ::BYTE* position_type) {

    // Locate vertex declaration
    IDirect3DVertexDeclaration9* i_vdec;
    s_device->get_child(StateId(NAME_CURRENT_VERTEX_DECLARATION))->read_data(&i_vdec);

    /// @todo error if no vertex declaration
    /// @todo support fvf also

    StateDataNode* s_vdec;
    s_vdec = s_device->get_child(StateId(NAME_VERTEX_DECLARATION_9, StateIndex(i_vdec)));

    // Iterate over elements until one with position usage is found
    UINT element_count;
    s_vdec->get_child(StateId(NAME_ELEMENT_COUNT))->read_data(&element_count);

    bool found_position = false;
    for(UINT i = 0; (i < element_count) & !found_position; i++) {
        StateDataNode* s_velem;
        s_velem = s_vdec->get_child(StateId(NAME_VERTEX_ELEMENT, StateIndex(i)));

        ::BYTE elem_usage;
        s_velem->get_child(StateId(NAME_USAGE))->read_data(&elem_usage);

        if((elem_usage == D3DDECLUSAGE_POSITION)) {
            found_position = true;
            s_velem->get_child(StateId(NAME_STREAM))->read_data(position_stream);
            s_velem->get_child(StateId(NAME_OFFSET))->read_data(position_offset);
            s_velem->get_child(StateId(NAME_TYPE))->read_data(position_type);
        }
    }

    return found_position;
}

#include "Common.h"
#include "CIndexVertexBuffers_9.h"
#include "CStreams_9.h"
#include "CIndexStream_9.h"
#include "CVSInput_9.h"
#include "CConstants_9.h"
#include "CRasterization_9.h"
#include "CAlphaBlending_9.h"
#include "CZStencilTest_9.h"
#include "CRenderTargetZStencil_9.h"
#include "CDebugGeometry_9.h"
#include "CShaders_9.h"
#include "CTextures_9.h"
#include "CSurfaces_9.h"
#include "CVolumes_9.h"
#include "CSamplers_9.h"
#include "CDevice_9.h"

void CDevice9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(parent->get_id().name == NAME_DIRECT3D_9) {
        if(child->get_id().name == NAME_DEVICE_9) {
            child->add_controller(this);
        }
    }
}

void CDevice9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    if(parent->get_id().name == NAME_DIRECT3D_9) {
        if(child->get_id().name == NAME_DEVICE_9) {
            child->remove_controller(this);
        }
    }
}

void CDevice9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
}

void CDevice9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DIRECT3D_9) {
    }
    else if(node->get_id().name == NAME_DEVICE_9) {

            initialize_gpu(node);

             //Add controllers to sdevice9
            /** @note Order is important here,
                because the controllers are notified in the
                same order than added. That's why
                controllers that send commands to gpu
                are on the last positions.
            **/
            node->add_controller(c_index_vertex_buffers);
            node->add_controller(c_index_stream);
            node->add_controller(c_streams);
            node->add_controller(c_vs_input);
            node->add_controller(c_constants);
            node->add_controller(c_rasterization);
            node->add_controller(c_alpha_blending);
            /** @note If CDebugGeometry used, disable CShaders and
                CConstants, they are incompatible **/
//            node->add_controller(c_debug_geometry);
            node->add_controller(c_shaders);
            node->add_controller(c_textures);
            node->add_controller(c_surfaces);
            node->add_controller(c_volumes);
            node->add_controller(c_samplers);
            node->add_controller(c_zstencil_test);
            node->add_controller(c_render_target_zstencil);
    }
}

void CDevice9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DIRECT3D_9) {
    }
    else if(node->get_id().name == NAME_DEVICE_9) {
            //Remove controllers to sdevice9 in reverse order
            node->remove_controller(c_render_target_zstencil);
            node->remove_controller(c_zstencil_test);
            node->remove_controller(c_samplers);
            node->remove_controller(c_volumes);
            node->remove_controller(c_surfaces);
            node->remove_controller(c_textures);
             node->remove_controller(c_shaders);
            /** @note If CDebugGeometry used, disable CShaders and
                CConstants, they are incompatible **/
            // node->remove_controller(c_debug_geometry);
            node->remove_controller(c_alpha_blending);
            node->remove_controller(c_rasterization);
            node->remove_controller(c_constants);
            node->remove_controller(c_vs_input);
            node->remove_controller(c_streams);
            node->remove_controller(c_index_stream);
            node->remove_controller(c_index_vertex_buffers);

            /// @todo Cleanup gpu state
    }
}


CDevice9::CDevice9() {
    // Create assignations, add resources if needed
    a_iv_buffer_memory = new ResourceAssignationTable(NAME_IVBUFFER_MEMORY);

    a_surface_memory = new ResourceAssignationTable(NAME_SURFACE_MEMORY);

    a_volume_memory = new ResourceAssignationTable(NAME_VOLUME_MEMORY);

    a_element_stream = new ResourceAssignationTable(NAME_ELEMENT_STREAM);
    for(u32bit i = 0; i < MAX_STREAM_BUFFERS; i ++)
        a_element_stream->add_resource(ResourceId(NAME_STREAM, i));

    a_streams = new ResourceAssignationTable(NAME_STREAMS);
    for(u32bit i = 0; i < MAX_STREAM_BUFFERS; i ++)
        a_streams->add_resource(ResourceId(NAME_STREAM, i));


    a_vs_input  = new ResourceAssignationTable(NAME_VS_INPUT);
    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i ++)
        a_vs_input->add_resource(ResourceId(NAME_REGISTER, i));

    a_vs_constants = new ResourceAssignationTable(NAME_VS_CONSTANTS);
    for(u32bit i = 0; i < MAX_VERTEX_CONSTANTS; i ++)
        a_vs_constants->add_resource(ResourceId(NAME_REGISTER, i));

    a_vs_output = new ResourceAssignationTable(NAME_VS_OUTPUT);
    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i ++)
        a_vs_output->add_resource(ResourceId(NAME_REGISTER, i));

    a_ps_input  = new ResourceAssignationTable(NAME_PS_INPUT);
    for(u32bit i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i ++)
        a_ps_input->add_resource(ResourceId(NAME_REGISTER, i));

    a_ps_constants = new ResourceAssignationTable(NAME_PS_CONSTANTS);
    for(u32bit i = 0; i < MAX_FRAGMENT_CONSTANTS; i ++)
        a_ps_constants->add_resource(ResourceId(NAME_REGISTER, i));

    a_ps_textures = new ResourceAssignationTable(NAME_PS_TEXTURE);
    for(u32bit i = 0; i < MAX_TEXTURES; i ++)
        a_ps_textures->add_resource(ResourceId(NAME_REGISTER, i));


    // Register assignations
    AssignationsRegistry::add_table(a_iv_buffer_memory);
    AssignationsRegistry::add_table(a_surface_memory);
    AssignationsRegistry::add_table(a_volume_memory);
    AssignationsRegistry::add_table(a_element_stream);
    AssignationsRegistry::add_table(a_streams);
    AssignationsRegistry::add_table(a_vs_input);
    AssignationsRegistry::add_table(a_vs_constants);
    AssignationsRegistry::add_table(a_vs_output);
    AssignationsRegistry::add_table(a_ps_input);
    AssignationsRegistry::add_table(a_ps_constants);
    AssignationsRegistry::add_table(a_ps_textures);

    // Create controllers
    c_index_vertex_buffers = new CIndexVertexBuffers9();
    c_streams = new CStreams9();
    c_index_stream = new CIndexStream9();
    c_vs_input = new CVSInput9();
    c_constants = new CConstants9();
    c_rasterization = new CRasterization9();
    c_alpha_blending = new CAlphaBlending9();
    c_zstencil_test = new CZStencilTest9();
    c_render_target_zstencil = new CRenderTargetZStencil9();
    c_debug_geometry = new CDebugGeometry9();
    c_shaders = new CShaders9();
    c_textures = new CTextures9();
    c_surfaces = new CSurfaces9();
    c_volumes = new CVolumes9();
    c_samplers = new CSamplers9();
}

CDevice9::~CDevice9() {

    // Delete controllers
    delete c_samplers;
    delete c_volumes;
    delete c_surfaces;
    delete c_textures;
    delete c_shaders;
    delete c_debug_geometry;
    delete c_render_target_zstencil;
    delete c_zstencil_test;
    delete c_alpha_blending;
    delete c_rasterization;
    delete c_constants;
    delete c_vs_input;
    delete c_streams;
    delete c_index_stream;
    delete c_index_vertex_buffers;

    // Unregister assignations
    AssignationsRegistry::remove_table(a_ps_textures);
    AssignationsRegistry::remove_table(a_ps_constants);
    AssignationsRegistry::remove_table(a_ps_input);
    AssignationsRegistry::remove_table(a_vs_output);
    AssignationsRegistry::remove_table(a_vs_constants);
    AssignationsRegistry::remove_table(a_vs_input);
    AssignationsRegistry::remove_table(a_streams);
    AssignationsRegistry::remove_table(a_element_stream);
    AssignationsRegistry::remove_table(a_volume_memory);
    AssignationsRegistry::remove_table(a_surface_memory);
    AssignationsRegistry::remove_table(a_iv_buffer_memory);

    // Delete assignations
    delete a_ps_textures;
    delete a_ps_constants;
    delete a_ps_input;
    delete a_vs_output;
    delete a_vs_constants;
    delete a_vs_input;
    delete a_streams;
    delete a_element_stream;
    delete a_volume_memory;
    delete a_surface_memory;
    delete a_iv_buffer_memory;
}

void CDevice9::initialize_gpu(StateDataNode* s_device) {
    GPUProxy* gpu = GPUProxy::get_instance();
    GPURegData data;

    // Streaming
    for(u32bit i = 0; i < MAX_STREAM_BUFFERS; i ++) {
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_STREAM_STRIDE, i, data);
        gpu->writeGPURegister(GPU_STREAM_ADDRESS, i, data);
        gpu->writeGPURegister(GPU_STREAM_ELEMENTS, i, data);
        gpu->writeGPURegister(GPU_STREAM_DATA, i, data);
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_D3D9_COLOR_STREAM, i, data);

    }
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_STREAM_START, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_STREAM_COUNT, data);
    data.primitive = TRIANGLE;
    gpu->writeGPURegister(GPU_PRIMITIVE, data);
    data.booleanVal = false;
    gpu->writeGPURegister(GPU_INDEX_MODE, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_INDEX_STREAM, data);


    // Shading
    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i ++) {
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, i, data);
        gpu->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, i, data);
    }

    ///@note Only 10 constants initialized to prevent overflow in the AGPTransaction buffer */
    for(u32bit i = 0; i < 10; i++) {
        for(u32bit j = 0; j < 4; j++)
            data.qfVal[0] = 0.0f;
        gpu->writeGPURegister(GPU_VERTEX_CONSTANT, i, data);
    }

    data.uintVal = 0;
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_PC, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_SIZE, data);


    for(u32bit i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i ++) {
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, i, data);
    }

    data.booleanVal = false;
    gpu->writeGPURegister(GPU_MODIFY_FRAGMENT_DEPTH, data);


    ///@note Only 10 constants initialized to prevent overflow in the AGPTransaction buffer */
    for(u32bit i = 0; i < 10; i++) {
        for(u32bit j = 0; j < 4; j++)
            data.qfVal[0] = 0.0f;
        gpu->writeGPURegister(GPU_FRAGMENT_CONSTANT, i, data);
    }

    data.uintVal = 0;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_PC, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_SIZE, data);

    // Rasterization and interpolation
    UINT w,h;
    s_device->get_child(StateId(NAME_BACKBUFFER_WIDTH))->read_data(&w);
    s_device->get_child(StateId(NAME_BACKBUFFER_HEIGHT))->read_data(&h);

    data.uintVal = w;
    gpu->writeGPURegister(GPU_VIEWPORT_WIDTH, data);
    data.uintVal = h;
    gpu->writeGPURegister(GPU_VIEWPORT_HEIGHT, data);
    data.intVal = 0;
    gpu->writeGPURegister(GPU_VIEWPORT_INI_X, data);
    data.intVal = 0;
    gpu->writeGPURegister(GPU_VIEWPORT_INI_Y, data);
    data.booleanVal = false;
     gpu->writeGPURegister(GPU_TWOSIDED_LIGHTING, data);
    data.culling = gpu3d::BACK;
    gpu->writeGPURegister(GPU_CULLING, data);
    data.faceMode = GPU_CW;
    gpu->writeGPURegister(GPU_FACEMODE, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_FRUSTUM_CLIPPING, data);

    // Color blending
    data.booleanVal = false;
    gpu->writeGPURegister(GPU_COLOR_BLEND, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_COLOR_MASK_A, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_COLOR_MASK_R, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_COLOR_MASK_G, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_COLOR_MASK_B, data);

    data.booleanVal = false;
    gpu->writeGPURegister(GPU_LOGICAL_OPERATION, data);

    // Depth test
    data.booleanVal = false;
    gpu->writeGPURegister(GPU_MODIFY_FRAGMENT_DEPTH, data);

    data.booleanVal = true;
    gpu->writeGPURegister(GPU_HIERARCHICALZ, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_EARLYZ, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_DEPTH_TEST, data);
    data.uintVal = 0x00FFFFFF;
    gpu->writeGPURegister(GPU_Z_BUFFER_CLEAR, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_DEPTH_MASK, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_ZSTENCIL_COMPRESSION, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_COLOR_COMPRESSION, data);
    data.compare = GPU_LEQUAL;
    gpu->writeGPURegister(GPU_DEPTH_FUNCTION, data);
    data.f32Val = 0.0f;
    gpu->writeGPURegister(GPU_DEPTH_RANGE_NEAR, data);
    data.f32Val = 1.0f;
    gpu->writeGPURegister(GPU_DEPTH_RANGE_FAR, data);

    data.f32Val = 0;
    gpu->writeGPURegister(GPU_DEPTH_SLOPE_FACTOR, data);
    data.f32Val = 0;
    gpu->writeGPURegister(GPU_DEPTH_UNIT_OFFSET, data);

    data.uintVal = 24;
    gpu->writeGPURegister(GPU_Z_BUFFER_BIT_PRECISSION, data);

    data.stencilUpdate = STENCIL_KEEP;
    gpu->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);
    data.stencilUpdate = STENCIL_KEEP;
    gpu->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);


    // Stencil test
    data.booleanVal = false;
    gpu->writeGPURegister(GPU_STENCIL_TEST, data);
    data.uintVal = 0xFF;
    gpu->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
    data.uintVal = 0xFF;
    gpu->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data);
    data.uintVal = 0x000000FF;
    gpu->writeGPURegister(GPU_STENCIL_BUFFER_CLEAR, data);
    data.uintVal = 0x00;
    gpu->writeGPURegister(GPU_STENCIL_REFERENCE, data);

    //Alpha blending
    data.booleanVal = false;
    gpu->writeGPURegister(GPU_COLOR_BLEND, data);
    data.blendEquation = BLEND_FUNC_ADD;
    gpu->writeGPURegister(GPU_BLEND_EQUATION, data);
    data.blendFunction = BLEND_ONE;
    gpu->writeGPURegister(GPU_BLEND_SRC_RGB, data);
    data.blendFunction = BLEND_ZERO;
    gpu->writeGPURegister(GPU_BLEND_DST_RGB, data);
    data.blendFunction = BLEND_ONE;
    gpu->writeGPURegister(GPU_BLEND_SRC_ALPHA, data);
    data.blendFunction = BLEND_ZERO;
    gpu->writeGPURegister(GPU_BLEND_DST_ALPHA, data);
    for(u32bit i = 0; i < 4; i ++)
        data.qfVal[i] = 1.0f;
    gpu->writeGPURegister(GPU_BLEND_COLOR, data);

    data.booleanVal = false;
    gpu->writeGPURegister(GPU_STENCIL_TEST, data);
    data.compare = GPU_ALWAYS;
    gpu->writeGPURegister(GPU_STENCIL_FUNCTION, data);
    /**@note In D3D stencil reference seems to be a signed integer,
        I'm not sure about the value to assign. */
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_STENCIL_REFERENCE, data);
    data.uintVal = 0xFFFFFFFF;
    gpu->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
    data.uintVal = 0xFFFFFFFF;
    gpu->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data);
    data.stencilUpdate = STENCIL_KEEP;
    gpu->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);
    data.stencilUpdate = STENCIL_KEEP;
    gpu->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);
    data.stencilUpdate = STENCIL_KEEP;
    gpu->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);

    // Rasterization
    // VS position register output will be always enabled
    data.booleanVal = true;
    GPUProxy::get_instance()->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, 0, data);



   // Texture units
    for(u32bit i = 0; i < MAX_TEXTURES; i ++) {
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_TEXTURE_ENABLE, i, data);
        data.txMode = GPU_TEXTURE2D;
        gpu->writeGPURegister(GPU_TEXTURE_MODE, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_ADDRESS, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_WIDTH, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_HEIGHT, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_DEPTH, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_WIDTH2, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_HEIGHT2, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_DEPTH2, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_BORDER, i, data);
        data.txFormat = GPU_RGBA8888;
        gpu->writeGPURegister(GPU_TEXTURE_FORMAT, i, data);
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_TEXTURE_REVERSE, i, data);
        data.txCompression = GPU_NO_TEXTURE_COMPRESSION;
        gpu->writeGPURegister(GPU_TEXTURE_COMPRESSION, i, data);
        data.txBlocking = GPU_TXBLOCK_TEXTURE;
        gpu->writeGPURegister(GPU_TEXTURE_BLOCKING, i, data);
        for(u32bit j = 0; j < 4; j ++) data.qfVal[j] = 0.0f;
        gpu->writeGPURegister(GPU_TEXTURE_BORDER_COLOR, i, data);
        data.txClamp = GPU_TEXT_REPEAT;
        gpu->writeGPURegister(GPU_TEXTURE_WRAP_S, i, data);
        data.txClamp = GPU_TEXT_REPEAT;
        gpu->writeGPURegister(GPU_TEXTURE_WRAP_T, i, data);
        data.txClamp = GPU_TEXT_REPEAT;
        gpu->writeGPURegister(GPU_TEXTURE_WRAP_R, i, data);
        data.txFilter = GPU_NEAREST;
        gpu->writeGPURegister(GPU_TEXTURE_MIN_FILTER, i, data);
        data.txFilter = GPU_NEAREST;
        gpu->writeGPURegister(GPU_TEXTURE_MAG_FILTER, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_MIN_LEVEL, i, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_MAX_LEVEL, i, data);
        data.uintVal = 1;
        gpu->writeGPURegister(GPU_TEXTURE_MAX_ANISOTROPY, i, data);
        data.booleanVal = true;
        gpu->writeGPURegister(GPU_TEXTURE_D3D9_COLOR_CONV, i, data); // Oe!
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_TEXTURE_D3D9_V_INV, i, data); // Oe!
    }

}


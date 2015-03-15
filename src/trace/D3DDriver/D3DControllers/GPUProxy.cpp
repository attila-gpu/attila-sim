#include "Common.h"
#include "GPUProxy.h"



bool GPUProxyRegId::operator()(const GPUProxyRegId &a, const GPUProxyRegId &b) const {
    if((unsigned int)(a.gpu_reg) < (unsigned int)(b.gpu_reg))
        return true;
    else if((unsigned int)(a.gpu_reg) > (unsigned int)(b.gpu_reg))
        return false;
    else {
        return (a.index < b.index);
    }
}

GPUProxyRegId::GPUProxyRegId() {
    index = 0;
    gpu_reg = (gpu3d::GPURegister)(0);
}

GPUProxyRegId::GPUProxyRegId(gpu3d::GPURegister _gpu_reg, u32bit _index) {
    gpu_reg = _gpu_reg;
    index = _index;
}

GPUProxyAddress::GPUProxyAddress() {
    md = 0;
    offset = 0;
}

GPUProxyAddress::GPUProxyAddress(u32bit _md, u32bit _offset) {
    md = _md;
    offset = _offset;
};

GPUProxy* GPUProxy::get_instance() {
    static GPUProxy proxy;
    return &proxy;
}


GPUProxy::GPUProxy()
{
    // Fill tables, note how both tables are being filled with the same macro

    #define ASSIGN(register,type) { register_types[GPUProxyRegId(register)] = type; register_names[register] = #register; }
    #define ASSIGN_I(register,index,type) { register_types[GPUProxyRegId(register, index)] = type;register_names[register] = #register; }


    /*  GPU state registers.  */
    ASSIGN(GPU_STATUS,RT_GPU_STATUS)
    // GPU_MEMORY

    /*  GPU display registers.  */
    ASSIGN(GPU_DISPLAY_X_RES,RT_U32BIT)
    ASSIGN(GPU_DISPLAY_Y_RES,RT_U32BIT)

    /*  GPU memory registers.  */
    ASSIGN(GPU_FRONTBUFFER_ADDR,RT_ADDRESS)
    ASSIGN(GPU_BACKBUFFER_ADDR,RT_ADDRESS)
    ASSIGN(GPU_ZSTENCILBUFFER_ADDR,RT_ADDRESS)
    ASSIGN(GPU_TEXTURE_MEM_ADDR,RT_ADDRESS)
    ASSIGN(GPU_PROGRAM_MEM_ADDR,RT_ADDRESS)

    /*  GPU vertex shader.  */
    ASSIGN(GPU_VERTEX_PROGRAM,RT_ADDRESS)
    ASSIGN(GPU_VERTEX_PROGRAM_PC,RT_U32BIT)
    ASSIGN(GPU_VERTEX_PROGRAM_SIZE,RT_U32BIT)
    ASSIGN(GPU_VERTEX_THREAD_RESOURCES,RT_U32BIT)
    for(u32bit i = 0; i < MAX_VERTEX_CONSTANTS; i ++) {
        ASSIGN_I(GPU_VERTEX_CONSTANT, i, RT_QUADFLOAT)
    }
    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i ++) {
        ASSIGN_I(GPU_VERTEX_OUTPUT_ATTRIBUTE, i, RT_BOOL)
    }

    /*  GPU vertex stream buffer registers.  */
    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i ++) {
        ASSIGN_I(GPU_VERTEX_ATTRIBUTE_MAP, i, RT_U32BIT)
        ASSIGN_I(GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, i, RT_QUADFLOAT)
    }
    for(u32bit i = 0; i < MAX_STREAM_BUFFERS; i ++) {
        ASSIGN_I(GPU_STREAM_ADDRESS, i, RT_ADDRESS)
        ASSIGN_I(GPU_STREAM_STRIDE, i, RT_U32BIT)
        ASSIGN_I(GPU_STREAM_DATA, i, RT_STREAM_DATA)
        ASSIGN_I(GPU_STREAM_ELEMENTS, i, RT_U32BIT)
    }
    ASSIGN(GPU_STREAM_START, RT_U32BIT)
    ASSIGN(GPU_STREAM_COUNT, RT_U32BIT)
    ASSIGN(GPU_INDEX_MODE, RT_BOOL)
    ASSIGN(GPU_INDEX_STREAM, RT_U32BIT)
    for(u32bit i = 0; i < MAX_STREAM_BUFFERS; i ++) {
        ASSIGN_I(GPU_D3D9_COLOR_STREAM, i, RT_BOOL)
    }

    /*  GPU primitive assembly registers.  */
    ASSIGN(GPU_PRIMITIVE, RT_PRIMITIVE_MODE)

    /*  GPU clipping and culling registers.  */
    ASSIGN(GPU_FRUSTUM_CLIPPING, RT_BOOL)
    for(u32bit i = 0; i < MAX_USER_CLIP_PLANES; i++) {
        ASSIGN_I(GPU_USER_CLIP, i, RT_QUADFLOAT)
    }
    ASSIGN(GPU_USER_CLIP_PLANE, RT_BOOL)

    ASSIGN(GPU_FACEMODE, RT_FACE_MODE)
    ASSIGN(GPU_CULLING, RT_CULLING_MODE)

    /*  GPU hierarchical Z registers.  */
    ASSIGN(GPU_HIERARCHICALZ, RT_BOOL)

    /*  GPU early Z registers.  */
    ASSIGN(GPU_EARLYZ, RT_BOOL)

    /*  GPU rasterization registers.  */
    ASSIGN(GPU_VIEWPORT_INI_X, RT_S32BIT)
    ASSIGN(GPU_VIEWPORT_INI_Y, RT_S32BIT)
    ASSIGN(GPU_VIEWPORT_WIDTH, RT_U32BIT)
    ASSIGN(GPU_VIEWPORT_HEIGHT, RT_U32BIT)
    ASSIGN(GPU_SCISSOR_TEST, RT_BOOL)
    ASSIGN(GPU_SCISSOR_INI_X, RT_S32BIT)
    ASSIGN(GPU_SCISSOR_INI_Y, RT_S32BIT)
    ASSIGN(GPU_SCISSOR_WIDTH, RT_U32BIT)
    ASSIGN(GPU_SCISSOR_HEIGHT, RT_U32BIT)
    ASSIGN(GPU_DEPTH_RANGE_NEAR, RT_F32BIT)
    ASSIGN(GPU_DEPTH_RANGE_FAR, RT_F32BIT)
    ASSIGN(GPU_DEPTH_SLOPE_FACTOR, RT_F32BIT)
    ASSIGN(GPU_DEPTH_UNIT_OFFSET, RT_F32BIT)
    ASSIGN(GPU_Z_BUFFER_BIT_PRECISSION, RT_U32BIT)
    ASSIGN(GPU_TWOSIDED_LIGHTING, RT_BOOL)
    ASSIGN(GPU_MULTISAMPLING, RT_BOOL)
    ASSIGN(GPU_MSAA_SAMPLES, RT_U32BIT)
    for(u32bit i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i ++) {
        ASSIGN_I(GPU_INTERPOLATION, i, RT_BOOL)
        ASSIGN_I(GPU_FRAGMENT_INPUT_ATTRIBUTES, i, RT_BOOL)
    }

    /*  GPU fragment registers.  */
    ASSIGN(GPU_FRAGMENT_PROGRAM, RT_ADDRESS)
    ASSIGN(GPU_FRAGMENT_PROGRAM_PC, RT_U32BIT)
    ASSIGN(GPU_FRAGMENT_PROGRAM_SIZE, RT_U32BIT)
    ASSIGN(GPU_FRAGMENT_THREAD_RESOURCES, RT_U32BIT)
    for(u32bit i = 0; i < MAX_FRAGMENT_CONSTANTS; i ++) {
        ASSIGN_I(GPU_FRAGMENT_CONSTANT, i, RT_QUADFLOAT)
    }
    ASSIGN(GPU_MODIFY_FRAGMENT_DEPTH, RT_BOOL);

    /*  GPU Texture Unit registers.  */
    for(u32bit i = 0; i < MAX_TEXTURES; i ++) {
        ASSIGN_I(GPU_TEXTURE_ENABLE, i, RT_BOOL)
        ASSIGN_I(GPU_TEXTURE_MODE, i, RT_TEXTURE_MODE)
        for(u32bit j = 0; j < MAX_TEXTURE_SIZE; j ++)
            for(u32bit k = 0; k < CUBEMAP_IMAGES; k++)
                ASSIGN_I(GPU_TEXTURE_ADDRESS, i * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + j * CUBEMAP_IMAGES + k, RT_ADDRESS)
        ASSIGN_I(GPU_TEXTURE_WIDTH, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXTURE_HEIGHT, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXTURE_DEPTH, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXTURE_WIDTH2, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXTURE_HEIGHT2, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXTURE_DEPTH2, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXTURE_BORDER, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXTURE_FORMAT, i, RT_TEXTURE_FORMAT)
        ASSIGN_I(GPU_TEXTURE_REVERSE, i, RT_BOOL)
        ASSIGN_I(GPU_TEXTURE_D3D9_COLOR_CONV, i, RT_BOOL)
        ASSIGN_I(GPU_TEXTURE_COMPRESSION, i, RT_TEXTURE_COMPRESSION)
        ASSIGN_I(GPU_TEXTURE_BLOCKING, i, RT_TEXTURE_BLOCKING)
        ASSIGN_I(GPU_TEXTURE_BORDER_COLOR, i, RT_QUADFLOAT)
        ASSIGN_I(GPU_TEXTURE_WRAP_S, i, RT_CLAMP_MODE)
        ASSIGN_I(GPU_TEXTURE_WRAP_T, i, RT_CLAMP_MODE)
        ASSIGN_I(GPU_TEXTURE_WRAP_R, i, RT_CLAMP_MODE)
        ASSIGN_I(GPU_TEXTURE_MIN_FILTER, i, RT_FILTER_MODE)
        ASSIGN_I(GPU_TEXTURE_MAG_FILTER, i, RT_FILTER_MODE)
        ASSIGN_I(GPU_TEXTURE_MIN_LOD, i, RT_F32BIT)
        ASSIGN_I(GPU_TEXTURE_MAX_LOD, i, RT_F32BIT)
        ASSIGN_I(GPU_TEXTURE_LOD_BIAS, i, RT_F32BIT)
        ASSIGN_I(GPU_TEXTURE_MIN_LEVEL, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXTURE_MAX_LEVEL, i, RT_U32BIT)
        ASSIGN_I(GPU_TEXT_UNIT_LOD_BIAS, i, RT_F32BIT)
        ASSIGN_I(GPU_TEXTURE_MAX_ANISOTROPY, i, RT_U32BIT)
    }

    /*  GPU depth and stencil clear values.  */
    ASSIGN(GPU_Z_BUFFER_CLEAR, RT_U32BIT)
    ASSIGN(GPU_STENCIL_BUFFER_CLEAR, RT_U32BIT)

    /*  GPU Stencil test registers.  */
    ASSIGN(GPU_STENCIL_TEST, RT_BOOL)
    ASSIGN(GPU_STENCIL_FUNCTION, RT_COMPARE_MODE)
    ASSIGN(GPU_STENCIL_REFERENCE, RT_U8BIT)
    ASSIGN(GPU_STENCIL_COMPARE_MASK, RT_U8BIT)
    ASSIGN(GPU_STENCIL_UPDATE_MASK, RT_U8BIT)
    ASSIGN(GPU_STENCIL_FAIL_UPDATE, RT_STENCIL_UPDATE_FUNCTION)
    ASSIGN(GPU_DEPTH_FAIL_UPDATE, RT_STENCIL_UPDATE_FUNCTION)
    ASSIGN(GPU_DEPTH_PASS_UPDATE, RT_STENCIL_UPDATE_FUNCTION)

    /*  GPU Depth test registers.  */
    ASSIGN(GPU_DEPTH_TEST, RT_BOOL)
    ASSIGN(GPU_DEPTH_FUNCTION, RT_COMPARE_MODE)
    ASSIGN(GPU_DEPTH_MASK, RT_BOOL)

    ASSIGN(GPU_ZSTENCIL_COMPRESSION, RT_BOOL)

    //  Render Target and Color Buffer registers.
    ASSIGN(GPU_COLOR_BUFFER_FORMAT, RT_TEXTURE_FORMAT)
    ASSIGN(GPU_COLOR_COMPRESSION, RT_BOOL)
    for(u32bit i = 0; i < MAX_RENDER_TARGETS; i ++) {
        ASSIGN_I(GPU_RENDER_TARGET_ENABLE, i, RT_BOOL)
        ASSIGN_I(GPU_RENDER_TARGET_FORMAT, i, RT_TEXTURE_FORMAT)
        ASSIGN_I(GPU_RENDER_TARGET_ADDRESS, i, RT_ADDRESS)
        
        /*  GPU Color Blend registers.  */
        ASSIGN_I(GPU_COLOR_BLEND, i, RT_BOOL)
        ASSIGN_I(GPU_BLEND_EQUATION, i, RT_BLEND_EQUATION)
        ASSIGN_I(GPU_BLEND_SRC_RGB, i, RT_BLEND_FUNCTION)
        ASSIGN_I(GPU_BLEND_DST_RGB, i, RT_BLEND_FUNCTION)
        ASSIGN_I(GPU_BLEND_SRC_ALPHA, i, RT_BLEND_FUNCTION)
        ASSIGN_I(GPU_BLEND_DST_ALPHA, i, RT_BLEND_FUNCTION)
        ASSIGN_I(GPU_BLEND_COLOR, i, RT_QUADFLOAT)
        ASSIGN_I(GPU_COLOR_MASK_R, i, RT_BOOL)
        ASSIGN_I(GPU_COLOR_MASK_G, i, RT_BOOL)
        ASSIGN_I(GPU_COLOR_MASK_B, i, RT_BOOL)
        ASSIGN_I(GPU_COLOR_MASK_A, i, RT_BOOL)
    }

    /*  GPU color clear value.  */
    ASSIGN(GPU_COLOR_BUFFER_CLEAR, RT_BOOL)


    /*  GPU Color Logical Operation registers.  */
    ASSIGN(GPU_LOGICAL_OPERATION, RT_BOOL)
    ASSIGN(GPU_LOGICOP_FUNCTION, RT_LOGIC_OP_MODE)

    /* Register indicating the start address of the second interleaving.
       Must be set to 0 to use a single (CHANNEL/BANK) memory interleaving */
    ASSIGN(GPU_MCV2_2ND_INTERLEAVING_START_ADDR, RT_ADDRESS)

    /* Registers for Framebuffer bit blit operation */
    ASSIGN(GPU_BLIT_INI_X, RT_U32BIT)
    ASSIGN(GPU_BLIT_INI_Y, RT_U32BIT)
    ASSIGN(GPU_BLIT_X_OFFSET, RT_U32BIT)
    ASSIGN(GPU_BLIT_Y_OFFSET, RT_U32BIT)
    ASSIGN(GPU_BLIT_WIDTH, RT_U32BIT)
    ASSIGN(GPU_BLIT_HEIGHT, RT_U32BIT)
    ASSIGN(GPU_BLIT_DST_ADDRESS, RT_ADDRESS)
    ASSIGN(GPU_BLIT_DST_TX_WIDTH2, RT_U32BIT)
    ASSIGN(GPU_BLIT_DST_TX_FORMAT, RT_TEXTURE_FORMAT)
    ASSIGN(GPU_BLIT_DST_TX_BLOCK, RT_TEXTURE_BLOCKING)

    #undef ASSIGN
    #undef ASSIGN_I


}

GPUProxy::~GPUProxy()
{
    //dtor
}


void GPUProxy::writeGPURegister( gpu3d::GPURegister regId, gpu3d::GPURegData data) {
    writeGPURegister( regId, 0, data );
}

void GPUProxy::writeGPURegister( gpu3d::GPURegister regId, u32bit index, gpu3d::GPURegData data) {
    GPUProxyRegId id(regId, index);
    GPUProxyAddress addr(0, 0);
    values[id] = data;
    addresses[id] = addr;
    GPUDriver::getGPUDriver()->writeGPURegister(regId, index, data);
}

void GPUProxy::writeGPUAddrRegister( gpu3d::GPURegister regId, u32bit index, u32bit md, u32bit offset) {
    GPUProxyRegId id(regId, index);
    GPUProxyAddress addr(md, offset);
    addresses[id] = addr;
    GPUDriver::getGPUDriver()->writeGPUAddrRegister(regId, index, md, offset);
}

void GPUProxy::readGPURegister( gpu3d::GPURegister regId, gpu3d::GPURegData* data) {
    readGPURegister(regId, 0, data);
}

void GPUProxy::readGPURegister( gpu3d::GPURegister regId, u32bit index, gpu3d::GPURegData* data) {
    GPUProxyRegId id(regId, index);
    map<GPUProxyRegId, GPURegData, GPUProxyRegId>::iterator it;
    it = values.find(id);
    if(it == values.end()) {
        throw "GPUProxy: Register not written";
    }
    *data = (*it).second;
}

void GPUProxy::readGPUAddrRegister( gpu3d::GPURegister regId, u32bit index, u32bit* md, u32bit* offset) {
    GPUProxyRegId id(regId, index);
    map<GPUProxyRegId, GPUProxyAddress, GPUProxyRegId>::iterator it;
    it = addresses.find(id);
    if(it == addresses.end()) {
        /** If register address value is 0 it's value will have been written
            through writeGPURegister */
        map<GPUProxyRegId, GPURegData, GPUProxyRegId>::iterator it_v;
        it_v = values.find(id);
        if(it_v == values.end()) {
            throw "GPUProxy: Register not written";
        }
        else {
            *md = (*it_v).second.uintVal;
            *offset = 0;
        }
    }
    else {
        *md = (*it).second.md;
        *offset = (*it).second.offset;
    }
}


void GPUProxy::sendCommand(gpu3d::GPUCommand com) {
    GPUDriver::getGPUDriver()->sendCommand(com);

    if(com == GPU_SWAPBUFFERS) {
        // Addresses of front and back buffers are swapped,
        GPUProxyAddress addr;
        addr = addresses[GPUProxyRegId(GPU_BACKBUFFER_ADDR)];
        addresses[GPUProxyRegId(GPU_BACKBUFFER_ADDR)] =
            addresses[GPUProxyRegId(GPU_FRONTBUFFER_ADDR)];
        addresses[GPUProxyRegId(GPU_FRONTBUFFER_ADDR)] = addr;

    }
}

void GPUProxy::initBuffers(u32bit* _mdCWFront, u32bit* _mdCWBack, u32bit* _mdZS) {
   u32bit mdCWFront;
   u32bit mdCWBack;
   u32bit mdZS;
   GPUDriver::getGPUDriver()->initBuffers(&mdCWFront, &mdCWBack, &mdZS);

   /* GPUDriver initBuffers assigns privately some registers.
      Assigning them in GPUProxy ensures the values are cached for
      further reading. */
    GPUProxyRegId id;
    GPUProxyAddress addr;
    id.gpu_reg = GPU_FRONTBUFFER_ADDR;
    id.index = 0;
    addr.md = mdCWFront;
    addr.offset = 0;
    addresses[id] = addr;

    id.gpu_reg = GPU_BACKBUFFER_ADDR;
    id.index = 0;
    addr.md = mdCWBack;
    addr.offset = 0;
    addresses[id] = addr;

    id.gpu_reg = GPU_ZSTENCILBUFFER_ADDR;
    id.index = 0;
    addr.md = mdZS;
    addr.offset = 0;
    addresses[id] = addr;

    // Return addresses
    if(_mdCWFront != 0) *_mdCWFront = mdCWFront;
    if(_mdCWBack != 0) *_mdCWBack = mdCWBack;
    if(_mdZS != 0) *_mdZS = mdZS;

}

u32bit GPUProxy::obtainMemory( u32bit sizeBytes) {
    u32bit md = GPUDriver::getGPUDriver()->obtainMemory(sizeBytes);
    D3D_DEBUG( cout << "GPUPROXY: Allocating " << (int)sizeBytes << " for md " << (int)md << endl; )
    D3D_DEBUG( GPUDriver::getGPUDriver()->printMemoryUsage(); )
    return md;
}

void GPUProxy::releaseMemory( u32bit md ) {
    GPUDriver::getGPUDriver()->releaseMemory(md);
    /* Set every reference to this md to zero in addresses and values map
       This prevents reading a register with a value that corresponds to a
       released memory descriptor. */
    set<GPUProxyRegId> references;
    map<GPUProxyRegId, GPUProxyAddress, GPUProxyRegId>::iterator it_a;
    for(it_a = addresses.begin(); it_a != addresses.end(); it_a ++) {
    	if((*it_a).second.md == md) {
    		references.insert((*it_a).first);
    	}
    }
    set<GPUProxyRegId>::iterator it_r;
    for(it_r = references.begin(); it_r !=  references.end(); it_r ++) {
    	addresses[*it_r] = GPUProxyAddress(0, 0);
    	GPURegData data;
    	data.uintVal = 0;
    	values[*it_r] = data;
    }

    D3D_DEBUG( cout << "GPUPROXY: Releasing md " << (int)md << endl; )
    D3D_DEBUG( GPUDriver::getGPUDriver()->printMemoryUsage(); )
}

bool GPUProxy::writeMemory( u32bit md, const u8bit* data, u32bit dataSize, bool isLocked) {
    return GPUDriver::getGPUDriver()->writeMemory(md, data, dataSize, isLocked);
}

bool GPUProxy::writeMemory( u32bit md, u32bit offset, const u8bit* data, u32bit dataSize, bool isLocked) {
    return GPUDriver::getGPUDriver()->writeMemory(md, offset, data, dataSize, isLocked);
}

void GPUProxy::setResolution(u32bit width, u32bit height) {
     GPUDriver::getGPUDriver()->setResolution(width, height);
}

bool GPUProxy::commitVertexProgram( u32bit memDesc, u32bit programSize, u32bit startPC ) {
     return GPUDriver::getGPUDriver()->commitVertexProgram(memDesc, programSize, startPC);
}

bool GPUProxy::commitFragmentProgram( u32bit memDesc, u32bit programSize, u32bit startPC ) {
     return GPUDriver::getGPUDriver()->commitFragmentProgram(memDesc, programSize, startPC);
}

void GPUProxy::debug_print_registers() {

    /*  GPU state registers.  */
    debug_print_register(GPU_STATUS);
    // GPU_MEMORY

    /*  GPU display registers.  */
    debug_print_register(GPU_DISPLAY_X_RES);
    debug_print_register(GPU_DISPLAY_Y_RES);

    /*  GPU memory registers.  */
    debug_print_register(GPU_FRONTBUFFER_ADDR);
    debug_print_register(GPU_BACKBUFFER_ADDR);
    debug_print_register(GPU_ZSTENCILBUFFER_ADDR);
    debug_print_register(GPU_TEXTURE_MEM_ADDR);
    debug_print_register(GPU_PROGRAM_MEM_ADDR);

    /*  GPU vertex shader.  */
    debug_print_register(GPU_VERTEX_PROGRAM);
    debug_print_register(GPU_VERTEX_PROGRAM_PC);
    debug_print_register(GPU_VERTEX_PROGRAM_SIZE);
    debug_print_register(GPU_VERTEX_THREAD_RESOURCES);
    for(u32bit i = 0; i < MAX_VERTEX_CONSTANTS; i ++) {
        debug_print_register(GPU_VERTEX_CONSTANT, i);
    }
    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i ++) {
        debug_print_register(GPU_VERTEX_OUTPUT_ATTRIBUTE, i);
    }

    /*  GPU vertex stream buffer registers.  */
    for(u32bit i = 0; i < MAX_VERTEX_ATTRIBUTES; i ++) {
        debug_print_register(GPU_VERTEX_ATTRIBUTE_MAP, i);
        debug_print_register(GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, i);
    }
    for(u32bit i = 0; i < MAX_STREAM_BUFFERS; i ++) {
        debug_print_register(GPU_STREAM_ADDRESS, i);
        debug_print_register(GPU_STREAM_STRIDE, i);
        debug_print_register(GPU_STREAM_DATA, i);
        debug_print_register(GPU_STREAM_ELEMENTS, i);
    }
    debug_print_register(GPU_STREAM_START);
    debug_print_register(GPU_STREAM_COUNT);
    debug_print_register(GPU_INDEX_MODE);
    debug_print_register(GPU_INDEX_STREAM);
    for(u32bit i = 0; i < MAX_STREAM_BUFFERS; i ++) {
        debug_print_register(GPU_D3D9_COLOR_STREAM, i);
    }

    /*  GPU primitive assembly registers.  */
    debug_print_register(GPU_PRIMITIVE);

    /*  GPU clipping and culling registers.  */
    debug_print_register(GPU_FRUSTUM_CLIPPING);
    for(u32bit i = 0; i < MAX_USER_CLIP_PLANES; i++) {
        debug_print_register(GPU_USER_CLIP, i);
    }
    debug_print_register(GPU_USER_CLIP_PLANE);

    debug_print_register(GPU_FACEMODE);
    debug_print_register(GPU_CULLING);

    /*  GPU hierarchical Z registers.  */
    debug_print_register(GPU_HIERARCHICALZ);

    /*  GPU early Z registers.  */
    debug_print_register(GPU_EARLYZ);

    /*  GPU rasterization registers.  */
    debug_print_register(GPU_VIEWPORT_INI_X);
    debug_print_register(GPU_VIEWPORT_INI_Y);
    debug_print_register(GPU_VIEWPORT_WIDTH);
    debug_print_register(GPU_VIEWPORT_HEIGHT);
    debug_print_register(GPU_SCISSOR_TEST);
    debug_print_register(GPU_SCISSOR_INI_X);
    debug_print_register(GPU_SCISSOR_INI_Y);
    debug_print_register(GPU_SCISSOR_WIDTH);
    debug_print_register(GPU_SCISSOR_HEIGHT);
    debug_print_register(GPU_DEPTH_RANGE_NEAR);
    debug_print_register(GPU_DEPTH_RANGE_FAR);
    debug_print_register(GPU_DEPTH_SLOPE_FACTOR);
    debug_print_register(GPU_DEPTH_UNIT_OFFSET);
    debug_print_register(GPU_Z_BUFFER_BIT_PRECISSION);
    debug_print_register(GPU_TWOSIDED_LIGHTING);
    debug_print_register(GPU_MULTISAMPLING);
    debug_print_register(GPU_MSAA_SAMPLES);
    for(u32bit i = 0; i < MAX_FRAGMENT_ATTRIBUTES; i ++) {
        debug_print_register(GPU_INTERPOLATION, i);
        debug_print_register(GPU_FRAGMENT_INPUT_ATTRIBUTES, i);
    }

    /*  GPU fragment registers.  */
    debug_print_register(GPU_FRAGMENT_PROGRAM);
    debug_print_register(GPU_FRAGMENT_PROGRAM_PC);
    debug_print_register(GPU_FRAGMENT_PROGRAM_SIZE);
    debug_print_register(GPU_FRAGMENT_THREAD_RESOURCES);
    for(u32bit i = 0; i < MAX_FRAGMENT_CONSTANTS; i ++) {
        debug_print_register(GPU_FRAGMENT_CONSTANT, i);
    }
    debug_print_register(GPU_MODIFY_FRAGMENT_DEPTH);

    /*  GPU Texture Unit registers.  */
    for(u32bit i = 0; i < MAX_TEXTURES; i ++) {
        debug_print_register(GPU_TEXTURE_ENABLE, i);
        debug_print_register(GPU_TEXTURE_MODE, i);
        for(u32bit j = 0; j < MAX_TEXTURE_SIZE; j ++)
            for(u32bit k = 0; k < CUBEMAP_IMAGES; k++)
                debug_print_register(GPU_TEXTURE_ADDRESS, i * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + j * CUBEMAP_IMAGES + k);
        debug_print_register(GPU_TEXTURE_WIDTH, i);
        debug_print_register(GPU_TEXTURE_HEIGHT, i);
        debug_print_register(GPU_TEXTURE_DEPTH, i);
        debug_print_register(GPU_TEXTURE_WIDTH2, i);
        debug_print_register(GPU_TEXTURE_HEIGHT2, i);
        debug_print_register(GPU_TEXTURE_DEPTH2, i);
        debug_print_register(GPU_TEXTURE_BORDER, i);
        debug_print_register(GPU_TEXTURE_FORMAT, i);
        debug_print_register(GPU_TEXTURE_REVERSE, i);
        debug_print_register(GPU_TEXTURE_D3D9_COLOR_CONV, i);
        debug_print_register(GPU_TEXTURE_COMPRESSION, i);
        debug_print_register(GPU_TEXTURE_BLOCKING, i);
        debug_print_register(GPU_TEXTURE_BORDER_COLOR, i);
        debug_print_register(GPU_TEXTURE_WRAP_S, i);
        debug_print_register(GPU_TEXTURE_WRAP_T, i);
        debug_print_register(GPU_TEXTURE_WRAP_R, i);
        debug_print_register(GPU_TEXTURE_MIN_FILTER, i);
        debug_print_register(GPU_TEXTURE_MAG_FILTER, i);
        debug_print_register(GPU_TEXTURE_MIN_LOD, i);
        debug_print_register(GPU_TEXTURE_MAX_LOD, i);
        debug_print_register(GPU_TEXTURE_LOD_BIAS, i);
        debug_print_register(GPU_TEXTURE_MIN_LEVEL, i);
        debug_print_register(GPU_TEXTURE_MAX_LEVEL, i);
        debug_print_register(GPU_TEXT_UNIT_LOD_BIAS, i);
        debug_print_register(GPU_TEXTURE_MAX_ANISOTROPY, i);
    }

    /*  GPU depth and stencil clear values.  */
    debug_print_register(GPU_Z_BUFFER_CLEAR);
    debug_print_register(GPU_STENCIL_BUFFER_CLEAR);

    /*  GPU Stencil test registers.  */
    debug_print_register(GPU_STENCIL_TEST);
    debug_print_register(GPU_STENCIL_FUNCTION);
    debug_print_register(GPU_STENCIL_REFERENCE);
    debug_print_register(GPU_STENCIL_COMPARE_MASK);
    debug_print_register(GPU_STENCIL_UPDATE_MASK);
    debug_print_register(GPU_STENCIL_FAIL_UPDATE);
    debug_print_register(GPU_DEPTH_FAIL_UPDATE);
    debug_print_register(GPU_DEPTH_PASS_UPDATE);

    /*  GPU Depth test registers.  */
    debug_print_register(GPU_DEPTH_TEST);
    debug_print_register(GPU_DEPTH_FUNCTION);
    debug_print_register(GPU_DEPTH_MASK);

    debug_print_register(GPU_ZSTENCIL_COMPRESSION);

    //  Render Target and Color Buffer registers.
    debug_print_register(GPU_COLOR_BUFFER_FORMAT);
    debug_print_register(GPU_COLOR_COMPRESSION);
    for(u32bit i = 0; i < MAX_RENDER_TARGETS; i ++) {
        debug_print_register(GPU_RENDER_TARGET_ENABLE, i);
        debug_print_register(GPU_RENDER_TARGET_FORMAT, i);
        debug_print_register(GPU_RENDER_TARGET_ADDRESS, i);
        
        /*  GPU Color Blend registers.  */
        debug_print_register(GPU_COLOR_BLEND, i);
        debug_print_register(GPU_BLEND_EQUATION, i);
        debug_print_register(GPU_BLEND_SRC_RGB, i);
        debug_print_register(GPU_BLEND_DST_RGB, i);
        debug_print_register(GPU_BLEND_SRC_ALPHA, i);
        debug_print_register(GPU_BLEND_DST_ALPHA, i);
        debug_print_register(GPU_BLEND_COLOR, i);
        debug_print_register(GPU_COLOR_MASK_R, i);
        debug_print_register(GPU_COLOR_MASK_G, i);
        debug_print_register(GPU_COLOR_MASK_B, i);
        debug_print_register(GPU_COLOR_MASK_A, i);
    }

    /*  GPU color clear value.  */
    debug_print_register(GPU_COLOR_BUFFER_CLEAR);


    /*  GPU Color Logical Operation registers.  */
    debug_print_register(GPU_LOGICAL_OPERATION);
    debug_print_register(GPU_LOGICOP_FUNCTION);

    /* Register indicating the start address of the second interleaving.
       Must be set to 0 to use a single (CHANNEL/BANK) memory interleaving */
    debug_print_register(GPU_MCV2_2ND_INTERLEAVING_START_ADDR);

    /* Registers for Framebuffer bit blit operation */
    debug_print_register(GPU_BLIT_INI_X);
    debug_print_register(GPU_BLIT_INI_Y);
    debug_print_register(GPU_BLIT_X_OFFSET);
    debug_print_register(GPU_BLIT_Y_OFFSET);
    debug_print_register(GPU_BLIT_WIDTH);
    debug_print_register(GPU_BLIT_HEIGHT);
    debug_print_register(GPU_BLIT_DST_ADDRESS);
    debug_print_register(GPU_BLIT_DST_TX_WIDTH2);
    debug_print_register(GPU_BLIT_DST_TX_FORMAT);
    debug_print_register(GPU_BLIT_DST_TX_BLOCK);
}

void GPUProxy::debug_print_register(gpu3d::GPURegister reg, u32bit index) {
    GPUProxyRegId id(reg, index);
    map<GPUProxyRegId, GPUProxyRegisterType, GPUProxyRegId>::iterator it_t;
    it_t = register_types.find(id);
    if(it_t == register_types.end())
        D3D_DEBUG( cout << "GPUPROXY: WARNING Register type not found." << endl; )
    map<GPURegister, string>::iterator it_n;
    GPUProxyRegisterType type = (*it_t).second;
    it_n = register_names.find(reg);
    if(it_n == register_names.end())
        D3D_DEBUG( cout << "GPUPROXY: WARNING Register name not found." << endl; )
    string name = (*it_n).second;

    if(type != RT_ADDRESS) {
        map<GPUProxyRegId, GPURegData, GPUProxyRegId>::iterator it_v;
        it_v = values.find(GPUProxyRegId(reg, index));
        if(it_v == values.end()) {
            ///@note uncomment to see not written registers
            // D3D_DEBUG( cout << "GPUPROXY: " << name << ": NOT WRITTEN" << endl; )
        }
        else {
            D3D_DEBUG( cout << "GPUPROXY: " << name << " " << (int)index << ": "; )

            ///@todo Put the enums string translation inside auxiliar functions
            GPURegData data = (*it_v).second;
            switch(type) {
                case RT_U32BIT:
                    D3D_DEBUG( cout << std::hex << "0x" << data.uintVal << endl; )
                    break;
                case RT_S32BIT:
                    D3D_DEBUG( cout << std::dec << data.intVal << endl; )
                    break;
                case RT_F32BIT:
                    D3D_DEBUG( cout << data.f32Val << endl; )
                    break;
                case RT_U8BIT:
                    D3D_DEBUG( cout << std::hex << "0x" << data.uintVal << endl; )
                    break;
                case RT_BOOL:
                    D3D_DEBUG( cout << std::boolalpha << data.booleanVal << endl; )
                    break;
                case RT_QUADFLOAT: {
                        D3D_DEBUG( cout << "("; )
                        for(u32bit i = 0; i < 4; i ++) {
                            D3D_DEBUG( cout << data.qfVal[i]; )
                            if(i != 3) D3D_DEBUG( cout << ", "; )
                        }
                        D3D_DEBUG( cout << ")" << endl; )
                    } break;
                case RT_GPU_STATUS: {
                    switch(data.status) {
                        case GPU_ST_RESET: D3D_DEBUG( cout << "GPU_ST_RESET" << endl; ) break;
                        case GPU_READY: D3D_DEBUG( cout << "GPU_READY" << endl; ) break;
                        case GPU_DRAWING: D3D_DEBUG( cout << "GPU_DRAWING" << endl; ) break;
                        case GPU_END_GEOMETRY: D3D_DEBUG( cout << "GPU_END_GEOMETRY" << endl; ) break;
                        case GPU_END_FRAGMENT: D3D_DEBUG( cout << "GPU_END_FRAGMENT" << endl; ) break;
                        case GPU_MEMORY_READ: D3D_DEBUG( cout << "GPU_MEMORY_READ" << endl; ) break;
                        case GPU_MEMORY_WRITE: D3D_DEBUG( cout << "GPU_MEMORY_WRITE" << endl; ) break;
                        case GPU_MEMORY_PRELOAD: D3D_DEBUG( cout << "GPU_MEMORY_PRELOAD" << endl; ) break;
                        case GPU_SWAP: D3D_DEBUG( cout << "GPU_SWAP" << endl; ) break;
                        case GPU_BLITTING: D3D_DEBUG( cout << "GPU_BLITTING" << endl; ) break;
                        case GPU_CLEAR_COLOR: D3D_DEBUG( cout << "GPU_CLEAR_COLOR" << endl; ) break;
                        case GPU_CLEAR_Z: D3D_DEBUG( cout << "GPU_CLEAR_Z" << endl; ) break;
                        case GPU_ERROR: D3D_DEBUG( cout << "GPU_ERROR" << endl; ) break;
                        case GPU_FLUSH_Z: D3D_DEBUG( cout << "GPU_FLUSH_Z" << endl; ) break;
                        case GPU_FLUSH_COLOR: D3D_DEBUG( cout << "GPU_FLUSH_COLOR" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.status << endl; ) break;
                    }
                }; break;
                case RT_FACE_MODE: {
                    switch(data.faceMode) {
                        case GPU_CW: D3D_DEBUG( cout << "GPU_CW" << endl; ) break;
                        case GPU_CCW: D3D_DEBUG( cout << "GPU_CCW" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.faceMode << endl; ) break;
                    }
                }; break;
                case RT_CULLING_MODE: {
                    switch(data.culling) {
                        case FRONT: D3D_DEBUG( cout << "FRONT" << endl; ) break;
                        case BACK: D3D_DEBUG( cout << "BACK" << endl; ) break;
                        case FRONT_AND_BACK: D3D_DEBUG( cout << "FRONT_AND_BACK" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.culling << endl; ) break;
                    }
                }; break;
                case RT_PRIMITIVE_MODE: {
                    switch(data.primitive) {
                        case TRIANGLE: D3D_DEBUG( cout << "TRIANGLE" << endl; ) break;
                        case TRIANGLE_STRIP: D3D_DEBUG( cout << "TRIANGLE_STRIP" << endl; ) break;
                        case TRIANGLE_FAN: D3D_DEBUG( cout << "TRIANGLE_FAN" << endl; ) break;
                        case QUAD: D3D_DEBUG( cout << "QUAD" << endl; ) break;
                        case QUAD_STRIP: D3D_DEBUG( cout << "QUAD_STRIP" << endl; ) break;
                        case LINE: D3D_DEBUG( cout << "LINE" << endl; ) break;
                        case LINE_STRIP: D3D_DEBUG( cout << "LINE_STRIP" << endl; ) break;
                        case LINE_FAN: D3D_DEBUG( cout << "LINE_FAN" << endl; ) break;
                        case gpu3d::POINT: D3D_DEBUG( cout << "POINT" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.primitive << endl; ) break;
                    }
                }; break;
                case RT_STREAM_DATA: {
                    switch(data.streamData) {
                        case SD_U8BIT: D3D_DEBUG( cout << "SD_U8BIT" << endl; ) break;
                        case SD_U16BIT: D3D_DEBUG( cout << "SD_U16BIT" << endl; ) break;
                        case SD_U32BIT: D3D_DEBUG( cout << "SD_U32BIT" << endl; ) break;
                        case SD_FLOAT: D3D_DEBUG( cout << "SD_FLOAT" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.streamData << endl; ) break;
                    }
                }; break;
                case RT_COMPARE_MODE: {
                    switch(data.compare) {
                        case GPU_NEVER: D3D_DEBUG( cout << "GPU_NEVER" << endl; ) break;
                        case GPU_ALWAYS: D3D_DEBUG( cout << "GPU_ALWAYS" << endl; ) break;
                        case GPU_LESS: D3D_DEBUG( cout << "GPU_LESS" << endl; ) break;
                        case GPU_LEQUAL: D3D_DEBUG( cout << "GPU_LEQUAL" << endl; ) break;
                        case GPU_EQUAL: D3D_DEBUG( cout << "GPU_EQUAL" << endl; ) break;
                        case GPU_GEQUAL: D3D_DEBUG( cout << "GPU_GEQUAL" << endl; ) break;
                        case GPU_GREATER: D3D_DEBUG( cout << "GPU_GREATER" << endl; ) break;
                        case GPU_NOTEQUAL: D3D_DEBUG( cout << "GPU_NOTEQUAL" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.compare << endl; ) break;
                    }
                }; break;
                case RT_STENCIL_UPDATE_FUNCTION: {
                    switch(data.stencilUpdate) {
                        case STENCIL_KEEP: D3D_DEBUG( cout << "STENCIL_KEEP" << endl; ) break;
                        case STENCIL_ZERO: D3D_DEBUG( cout << "STENCIL_ZERO" << endl; ) break;
                        case STENCIL_REPLACE: D3D_DEBUG( cout << "STENCIL_REPLACE" << endl; ) break;
                        case STENCIL_INCR: D3D_DEBUG( cout << "STENCIL_INCR" << endl; ) break;
                        case STENCIL_DECR: D3D_DEBUG( cout << "STENCIL_DECR" << endl; ) break;
                        case STENCIL_INVERT: D3D_DEBUG( cout << "STENCIL_INVERT" << endl; ) break;
                        case STENCIL_INCR_WRAP: D3D_DEBUG( cout << "STENCIL_INCR_WRAP" << endl; ) break;
                        case STENCIL_DECR_WRAP: D3D_DEBUG( cout << "STENCIL_DECR_WRAP" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.stencilUpdate << endl; ) break;
                    }
                }; break;
                case RT_BLEND_EQUATION: {
                    switch(data.blendEquation) {
                        case BLEND_FUNC_ADD: D3D_DEBUG( cout << "BLEND_FUNC_ADD" << endl; ) break;
                        case BLEND_FUNC_SUBTRACT: D3D_DEBUG( cout << "BLEND_FUNC_SUBTRACT" << endl; ) break;
                        case BLEND_FUNC_REVERSE_SUBTRACT: D3D_DEBUG( cout << "BLEND_FUNC_REVERSE_SUBTRACT" << endl; ) break;
                        case BLEND_MIN: D3D_DEBUG( cout << "BLEND_MIN" << endl; ) break;
                        case BLEND_MAX: D3D_DEBUG( cout << "BLEND_MAX" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.blendEquation << endl; ) break;
                    }
                }; break;
                case RT_BLEND_FUNCTION: {
                    switch(data.blendFunction) {
                        case BLEND_ZERO: D3D_DEBUG( cout << "BLEND_ZERO" << endl; ) break;
                        case BLEND_ONE: D3D_DEBUG( cout << "BLEND_ONE" << endl; ) break;
                        case BLEND_SRC_COLOR: D3D_DEBUG( cout << "BLEND_SRC_COLOR" << endl; ) break;
                        case BLEND_ONE_MINUS_SRC_COLOR: D3D_DEBUG( cout << "BLEND_ONE_MINUS_SRC_COLOR" << endl; ) break;
                        case BLEND_DST_COLOR: D3D_DEBUG( cout << "BLEND_DST_COLOR" << endl; ) break;
                        case BLEND_ONE_MINUS_DST_COLOR: D3D_DEBUG( cout << "BLEND_ONE_MINUS_DST_COLOR" << endl; ) break;
                        case BLEND_SRC_ALPHA: D3D_DEBUG( cout << "BLEND_SRC_ALPHA" << endl; ) break;
                        case BLEND_ONE_MINUS_SRC_ALPHA: D3D_DEBUG( cout << "BLEND_ONE_MINUS_SRC_ALPHA" << endl; ) break;
                        case BLEND_DST_ALPHA: D3D_DEBUG( cout << "BLEND_DST_ALPHA" << endl; ) break;
                        case BLEND_ONE_MINUS_DST_ALPHA: D3D_DEBUG( cout << "BLEND_ONE_MINUS_DST_ALPHA" << endl; ) break;
                        case BLEND_CONSTANT_COLOR: D3D_DEBUG( cout << "BLEND_CONSTANT_COLOR" << endl; ) break;
                        case BLEND_ONE_MINUS_CONSTANT_COLOR: D3D_DEBUG( cout << "BLEND_ONE_MINUS_CONSTANT_COLOR" << endl; ) break;
                        case BLEND_CONSTANT_ALPHA: D3D_DEBUG( cout << "BLEND_CONSTANT_ALPHA" << endl; ) break;
                        case BLEND_ONE_MINUS_CONSTANT_ALPHA: D3D_DEBUG( cout << "BLEND_ONE_MINUS_CONSTANT_ALPHA" << endl; ) break;
                        case BLEND_SRC_ALPHA_SATURATE: D3D_DEBUG( cout << "BLEND_SRC_ALPHA_SATURATE" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.blendFunction << endl; ) break;
                    }
                }; break;
                case RT_LOGIC_OP_MODE: {
                    switch(data.logicOp) {
                        case LOGICOP_CLEAR: D3D_DEBUG( cout << "LOGICOP_CLEAR" << endl; ) break;
                        case LOGICOP_AND: D3D_DEBUG( cout << "LOGICOP_AND" << endl; ) break;
                        case LOGICOP_AND_REVERSE: D3D_DEBUG( cout << "LOGICOP_AND_REVERSE" << endl; ) break;
                        case LOGICOP_COPY: D3D_DEBUG( cout << "LOGICOP_COPY" << endl; ) break;
                        case LOGICOP_AND_INVERTED: D3D_DEBUG( cout << "LOGICOP_AND_INVERTED" << endl; ) break;
                        case LOGICOP_NOOP: D3D_DEBUG( cout << "LOGICOP_NOOP" << endl; ) break;
                        case LOGICOP_XOR: D3D_DEBUG( cout << "LOGICOP_XOR" << endl; ) break;
                        case LOGICOP_OR: D3D_DEBUG( cout << "LOGICOP_OR" << endl; ) break;
                        case LOGICOP_NOR: D3D_DEBUG( cout << "LOGICOP_NOR" << endl; ) break;
                        case LOGICOP_EQUIV: D3D_DEBUG( cout << "LOGICOP_EQUIV" << endl; ) break;
                        case LOGICOP_INVERT: D3D_DEBUG( cout << "LOGICOP_INVERT" << endl; ) break;
                        case LOGICOP_OR_REVERSE: D3D_DEBUG( cout << "LOGICOP_OR_REVERSE" << endl; ) break;
                        case LOGICOP_COPY_INVERTED: D3D_DEBUG( cout << "LOGICOP_COPY_INVERTED" << endl; ) break;
                        case LOGICOP_OR_INVERTED: D3D_DEBUG( cout << "LOGICOP_OR_INVERTED" << endl; ) break;
                        case LOGICOP_NAND: D3D_DEBUG( cout << "LOGICOP_NAND" << endl; ) break;
                        case LOGICOP_SET: D3D_DEBUG( cout << "LOGICOP_SET" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.logicOp << endl; ) break;
                    }
                }; break;
                case RT_TEXTURE_MODE: {
                    switch(data.txMode) {
                        case GPU_TEXTURE1D: D3D_DEBUG( cout << "GPU_TEXTURE1D" << endl; ) break;
                        case GPU_TEXTURE2D: D3D_DEBUG( cout << "GPU_TEXTURE2D" << endl; ) break;
                        case GPU_TEXTURE3D: D3D_DEBUG( cout << "GPU_TEXTURE3D" << endl; ) break;
                        case GPU_TEXTURECUBEMAP: D3D_DEBUG( cout << "GPU_TEXTURECUBEMAP" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.txMode << endl; ) break;
                    }
                }; break;
                case RT_TEXTURE_FORMAT:  {
                    switch(data.txFormat) {
                        case GPU_ALPHA8: D3D_DEBUG( cout << "GPU_ALPHA8" << endl; ) break;
                        case GPU_ALPHA12: D3D_DEBUG( cout << "GPU_ALPHA12" << endl; ) break;
                        case GPU_ALPHA16: D3D_DEBUG( cout << "GPU_ALPHA16" << endl; ) break;
                        case GPU_DEPTH_COMPONENT16: D3D_DEBUG( cout << "GPU_DEPTH_COMPONENT16" << endl; ) break;
                        case GPU_DEPTH_COMPONENT24: D3D_DEBUG( cout << "GPU_DEPTH_COMPONENT24" << endl; ) break;
                        case GPU_DEPTH_COMPONENT32: D3D_DEBUG( cout << "GPU_DEPTH_COMPONENT32" << endl; ) break;
                        case GPU_LUMINANCE8: D3D_DEBUG( cout << "GPU_LUMINANCE8" << endl; ) break;
                        case GPU_LUMINANCE12: D3D_DEBUG( cout << "GPU_LUMINANCE12" << endl; ) break;
                        case GPU_LUMINANCE16: D3D_DEBUG( cout << "GPU_LUMINANCE16" << endl; ) break;
                        case GPU_LUMINANCE4_ALPHA4: D3D_DEBUG( cout << "GPU_LUMINANCE4_ALPHA4" << endl; ) break;
                        case GPU_LUMINANCE6_ALPHA2: D3D_DEBUG( cout << "GPU_LUMINANCE6_ALPHA2" << endl; ) break;
                        case GPU_LUMINANCE8_ALPHA8: D3D_DEBUG( cout << "GPU_LUMINANCE8_ALPHA8" << endl; ) break;
                        case GPU_LUMINANCE12_ALPHA4: D3D_DEBUG( cout << "GPU_LUMINANCE12_ALPHA4" << endl; ) break;
                        case GPU_LUMINANCE12_ALPHA12: D3D_DEBUG( cout << "GPU_LUMINANCE12_ALPHA12" << endl; ) break;
                        case GPU_LUMINANCE16_ALPHA16: D3D_DEBUG( cout << "GPU_LUMINANCE16_ALPHA16" << endl; ) break;
                        case GPU_INTENSITY8: D3D_DEBUG( cout << "GPU_INTENSITY8" << endl; ) break;
                        case GPU_INTENSITY12: D3D_DEBUG( cout << "GPU_INTENSITY12" << endl; ) break;
                        case GPU_INTENSITY16: D3D_DEBUG( cout << "GPU_INTENSITY16" << endl; ) break;
                        case GPU_RGB332: D3D_DEBUG( cout << "GPU_RGB332" << endl; ) break;
                        case GPU_RGB444: D3D_DEBUG( cout << "GPU_RGB444" << endl; ) break;
                        case GPU_RGB555: D3D_DEBUG( cout << "GPU_RGB555" << endl; ) break;
                        case GPU_RGB888: D3D_DEBUG( cout << "GPU_RGB888" << endl; ) break;
                        case GPU_RGB101010: D3D_DEBUG( cout << "GPU_RGB101010" << endl; ) break;
                        case GPU_RGB121212: D3D_DEBUG( cout << "GPU_RGB121212" << endl; ) break;
                        case GPU_RGBA2222: D3D_DEBUG( cout << "GPU_RGBA2222" << endl; ) break;
                        case GPU_RGBA4444: D3D_DEBUG( cout << "GPU_RGBA4444" << endl; ) break;
                        case GPU_RGBA5551: D3D_DEBUG( cout << "GPU_RGBA5551" << endl; ) break;
                        case GPU_RGBA8888: D3D_DEBUG( cout << "GPU_RGBA8888" << endl; ) break;
                        case GPU_RGBA1010102: D3D_DEBUG( cout << "GPU_RGBA1010102" << endl; ) break;
                        case GPU_R16: D3D_DEBUG( cout << "GPU_R16" << endl; ) break;
                        case GPU_RG16: D3D_DEBUG( cout << "GPU_RG16" << endl; ) break;
                        case GPU_RGBA16: D3D_DEBUG( cout << "GPU_RGBA16" << endl; ) break;
                        case GPU_R16F: D3D_DEBUG( cout << "GPU_R16F" << endl; ) break;
                        case GPU_RG16F: D3D_DEBUG( cout << "GPU_RG16F" << endl; ) break;
                        case GPU_RGBA16F: D3D_DEBUG( cout << "GPU_RGBA16F" << endl; ) break;
                        case GPU_R32F: D3D_DEBUG( cout << "GPU_R32F" << endl; ) break;
                        case GPU_RG32F: D3D_DEBUG( cout << "GPU_RG32F" << endl; ) break;
                        case GPU_RGBA32F: D3D_DEBUG( cout << "GPU_RGBA32F" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.txFormat << endl; ) break;
                    }
                }; break;
                case RT_TEXTURE_COMPRESSION: {
                    switch(data.txCompression) {
                        case GPU_NO_TEXTURE_COMPRESSION: D3D_DEBUG( cout << "GPU_NO_TEXTURE_COMPRESSION" << endl; ) break;
                        case GPU_S3TC_DXT1_RGB: D3D_DEBUG( cout << "GPU_S3TC_DXT1_RGB" << endl; ) break;
                        case GPU_S3TC_DXT1_RGBA: D3D_DEBUG( cout << "GPU_S3TC_DXT1_RGBA" << endl; ) break;
                        case GPU_S3TC_DXT3_RGBA: D3D_DEBUG( cout << "GPU_S3TC_DXT3_RGBA" << endl; ) break;
                        case GPU_S3TC_DXT5_RGBA: D3D_DEBUG( cout << "GPU_S3TC_DXT5_RGBA" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.txCompression << endl; ) break;
                    }
                }; break;
                case RT_TEXTURE_BLOCKING: {
                    switch(data.txBlocking) {
                        case GPU_TXBLOCK_TEXTURE: D3D_DEBUG( cout << "GPU_TXBLOCK_TEXTURE" << endl; ) break;
                        case GPU_TXBLOCK_FRAMEBUFFER: D3D_DEBUG( cout << "GPU_TXBLOCK_FRAMEBUFFER" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.txBlocking << endl; ) break;
                    }
                }; break;
                case RT_CLAMP_MODE: {
                    switch(data.txClamp) {
                        case GPU_TEXT_CLAMP: D3D_DEBUG( cout << "GPU_TEXT_CLAMP" << endl; ) break;
                        case GPU_TEXT_CLAMP_TO_EDGE: D3D_DEBUG( cout << "GPU_TEXT_CLAMP_TO_EDGE" << endl; ) break;
                        case GPU_TEXT_CLAMP_TO_BORDER: D3D_DEBUG( cout << "GPU_TEXT_CLAMP_TO_BORDER" << endl; ) break;
                        case GPU_TEXT_MIRRORED_REPEAT: D3D_DEBUG( cout << "GPU_TEXT_MIRRORED_REPEAT" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.txClamp << endl; ) break;
                    }
                }; break;
                case RT_FILTER_MODE: {
                    switch(data.txFilter) {
                        case GPU_NEAREST: D3D_DEBUG( cout << "GPU_NEAREST" << endl; ) break;
                        case GPU_LINEAR: D3D_DEBUG( cout << "GPU_LINEAR" << endl; ) break;
                        case GPU_NEAREST_MIPMAP_NEAREST: D3D_DEBUG( cout << "GPU_NEAREST_MIPMAP_NEAREST" << endl; ) break;
                        case GPU_NEAREST_MIPMAP_LINEAR: D3D_DEBUG( cout << "GPU_NEAREST_MIPMAP_LINEAR" << endl; ) break;
                        case GPU_LINEAR_MIPMAP_LINEAR: D3D_DEBUG( cout << "GPU_LINEAR_MIPMAP_LINEAR" << endl; ) break;
                        default: D3D_DEBUG( cout << (int)data.txFilter << endl; ) break;
                    }
                }; break;
            }
            D3D_DEBUG( cout << std::dec; noboolalpha(cout); )
        }
    }
    else { // Addresses
        map<GPUProxyRegId, GPUProxyAddress, GPUProxyRegId>::iterator it_ad;
        it_ad = addresses.find(GPUProxyRegId(reg, index));
        if(it_ad == addresses.end()) {
            ///@note uncomment to see not written registers
            // D3D_DEBUG( cout << "GPUPROXY: " << name << ": NOT WRITTEN" << endl; )
        }
        else {
            D3D_DEBUG( cout << "GPUPROXY: " << name << " " << (int)index << ": "; )
            GPUProxyAddress addr = (*it_ad).second;
            D3D_DEBUG( cout << "md " << std::hex << "0x" << addr.md << " at 0x" << addr.offset << endl; )
            D3D_DEBUG( cout << std::dec; )
        }
    }

}

void GPUProxy::getGPUParameters(u32bit& gpuMemSz, u32bit& sysMemSz, u32bit& blocksz, u32bit& sblocksz,
u32bit& scanWidth, u32bit& scanHeight, u32bit& overScanWidth, u32bit& overScanHeight,
bool& doubleBuffer, u32bit& fetchRate) const {
    GPUDriver::getGPUDriver()->getGPUParameters(gpuMemSz, sysMemSz, blocksz, sblocksz,
        scanWidth, scanHeight, overScanWidth, overScanHeight,
        doubleBuffer, fetchRate);
}

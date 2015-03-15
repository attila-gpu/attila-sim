#include "Common.h"
#include "CSamplers_9.h"


void CSamplers9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    // nothing to do
}
void CSamplers9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    // nothing to do
}
void CSamplers9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    // Update gpu texture unit
    if(node->get_id() == NAME_TEXTURE) {
        // Obtain texture interface address
        IDirect3DBaseTexture9* i_tex;
        node->read_data(&i_tex);
        UINT sampler = UINT(node->get_parent()->get_id().index);
        if(i_tex != 0) {
            // Navigate to get texture state
            StateDataNode* s_device = node->get_parent()->get_parent();
            ///@todo Doing this by polling is not very elegant, consider alternatives
            StateDataNode* s_texture;
            s_texture = s_device->find_child(StateId(NAME_TEXTURE_9, StateIndex(i_tex)));
            if(s_texture == 0)
                s_texture = s_device->find_child(StateId(NAME_CUBETEXTURE_9, StateIndex(i_tex)));
            if(s_texture == 0)
                s_texture = s_device->find_child(StateId(NAME_VOLUMETEXTURE_9, StateIndex(i_tex)));

            if(is_sampler_assigned(sampler)) {
                u32bit texture_unit = get_assigned_texture_unit(sampler);
                update_texture(s_texture, texture_unit);
            }
        }
        else {
            reset_texture(sampler);
        }
    }
    else if(node->get_id() == NAME_ADDRESSU) {
         UINT sampler = UINT(node->get_parent()->get_id().index);
         DWORD address;
         node->read_data(&address);
         if(is_sampler_assigned(sampler)) {
             u32bit texture_unit = get_assigned_texture_unit(sampler);
             GPURegData data;
             data.txClamp = get_clamp(address);
             GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_WRAP_S,
                texture_unit, data);
         }
    }
    else if(node->get_id() == NAME_ADDRESSV) {
         UINT sampler = UINT(node->get_parent()->get_id().index);
         DWORD address;
         node->read_data(&address);
         if(is_sampler_assigned(sampler)) {
             u32bit texture_unit = get_assigned_texture_unit(sampler);
             GPURegData data;
             data.txClamp = get_clamp(address);
             GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_WRAP_T,
                texture_unit, data);
         }
    }
    else if(node->get_id() == NAME_ADDRESSW) {
         UINT sampler = UINT(node->get_parent()->get_id().index);
         DWORD address;
         node->read_data(&address);
         if(is_sampler_assigned(sampler)) {
             u32bit texture_unit = get_assigned_texture_unit(sampler);
             GPURegData data;
             data.txClamp = get_clamp(address);
             GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_WRAP_R,
                texture_unit, data);
         }
    }
    else if(node->get_id() == NAME_MIN_FILTER) {
        UINT sampler = UINT(node->get_parent()->get_id().index);
         if(is_sampler_assigned(sampler)) {
            u32bit texture_unit = get_assigned_texture_unit(sampler);
            update_filtering(node->get_parent(), texture_unit);
         }
    }
    else if(node->get_id() == NAME_MIP_FILTER) {
        UINT sampler = UINT(node->get_parent()->get_id().index);
         if(is_sampler_assigned(sampler)) {
            u32bit texture_unit = get_assigned_texture_unit(sampler);
            update_filtering(node->get_parent(), texture_unit);
         }
    }
    else if(node->get_id() == NAME_MAG_FILTER) {
         UINT sampler = UINT(node->get_parent()->get_id().index);
         if(is_sampler_assigned(sampler)) {
             u32bit texture_unit = get_assigned_texture_unit(sampler);
             update_filtering(node->get_parent(), texture_unit);
         }
    }
}


void CSamplers9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // watch sampler state
        for(UINT i = 0; i < MAX_D3D_SAMPLERS; i ++) {
            StateDataNode* s_sampler = node->get_child(StateId(NAME_SAMPLER, StateIndex(i)));
            s_sampler->get_child(StateId(NAME_TEXTURE))->add_controller(this);
            s_sampler->get_child(StateId(NAME_ADDRESSU))->add_controller(this);
            s_sampler->get_child(StateId(NAME_ADDRESSV))->add_controller(this);
            s_sampler->get_child(StateId(NAME_ADDRESSW))->add_controller(this);
            s_sampler->get_child(StateId(NAME_MIN_FILTER))->add_controller(this);
            s_sampler->get_child(StateId(NAME_MIP_FILTER))->add_controller(this);
            s_sampler->get_child(StateId(NAME_MAG_FILTER))->add_controller(this);
            s_sampler->get_child(StateId(NAME_MAX_ANISOTROPY))->add_controller(this);
        }
        // watch texture registers assignation
        ResourceAssignationTable* a_ps_textures;
        a_ps_textures = AssignationsRegistry::get_table(NAME_PS_TEXTURE);
        a_ps_textures->add_observer(this);

        // Create a default texture
        u32bit default_texels[] = {0xFF808080, 0xFFFFFFFF, 0xFFFFFFFF, 0xFF808080};
        default_texture = GPUProxy::get_instance()->obtainMemory(sizeof(default_texels));
        GPUProxy::get_instance()->writeMemory(default_texture, (u8bit*)&default_texels,
            sizeof(default_texels));

        // Attach to device
        s_device = node;
    }
}

void CSamplers9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_S_DEVICE) {
        // Unattach from device
        s_device = node;
        // Release default texture
        GPUProxy::get_instance()->releaseMemory(default_texture);
        // unwatch texture registers assignation
        ResourceAssignationTable* a_ps_textures;
        a_ps_textures = AssignationsRegistry::get_table(NAME_PS_TEXTURES);
        a_ps_textures->remove_observer(this);
        // unwatch sampler state
        for(UINT i = 0; i < MAX_D3D_SAMPLERS; i ++) {
            StateDataNode* s_sampler = node->get_child(StateId(NAME_SAMPLER, StateIndex(i)));
            s_sampler->get_child(StateId(NAME_TEXTURE))->remove_controller(this);
            s_sampler->get_child(StateId(NAME_ADDRESSU))->remove_controller(this);
            s_sampler->get_child(StateId(NAME_ADDRESSV))->remove_controller(this);
            s_sampler->get_child(StateId(NAME_ADDRESSW))->remove_controller(this);
            s_sampler->get_child(StateId(NAME_MIN_FILTER))->remove_controller(this);
            s_sampler->get_child(StateId(NAME_MIP_FILTER))->remove_controller(this);
            s_sampler->get_child(StateId(NAME_MAG_FILTER))->remove_controller(this);
            s_sampler->get_child(StateId(NAME_MAX_ANISOTROPY))->remove_controller(this);
        }
    }
}

void CSamplers9::on_assigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) {
    if(table->get_name() == NAME_PS_TEXTURE) {
        // Get texture from this sampler
        StateDataNode* s_sampler;
        s_sampler = s_device->get_child(StateId(NAME_SAMPLER, StateIndex(usage.index)));
        IDirect3DBaseTexture9* i_tex;
        s_sampler->get_child(NAME_TEXTURE)->read_data(&i_tex);
        /// @todo Doing this guessing is not very elegant, find an alternative
        StateDataNode* s_texture = 0;
        if(i_tex != 0) {
            s_texture = s_device->find_child(StateId(NAME_TEXTURE_9, StateIndex(i_tex)));
            if(s_texture == 0)
                s_texture = s_device->find_child(StateId(NAME_CUBETEXTURE_9, StateIndex(i_tex)));
            if(s_texture == 0)
                s_texture = s_device->find_child(StateId(NAME_VOLUMETEXTURE_9, StateIndex(i_tex)));
        }
        // Update or reset texture unit
        if(s_texture == 0)
            reset_texture(resource.index);
        else {
            update_texture(s_texture, resource.index);
            update_addressing(s_sampler, resource.index);
            update_filtering(s_sampler, resource.index);
        }
    }
}

void CSamplers9::on_unassigned(ResourceAssignationTable* table, UsageId usage, ResourceId resource) {
    if(table->get_name() == NAME_PS_TEXTURE) {
        reset_texture(resource.index);
    }
}

void CSamplers9::reset_texture(u32bit texture_unit) {
    D3D_DEBUG( cout << "CSAMPLERS9: Resetting texture unit " << (int)texture_unit << endl; )

    GPUProxy* gpu = GPUProxy::get_instance();
    GPURegData data;
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_TEXTURE_MAX_LEVEL, texture_unit, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_TEXTURE_MIN_LEVEL, texture_unit, data);
    data.uintVal = 2;
    gpu->writeGPURegister(GPU_TEXTURE_WIDTH, texture_unit, data);
    data.uintVal = 2;
    gpu->writeGPURegister(GPU_TEXTURE_HEIGHT, texture_unit, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_TEXTURE_DEPTH, texture_unit, data);
    data.uintVal = 1;
    gpu->writeGPURegister(GPU_TEXTURE_WIDTH2, texture_unit, data);
    data.uintVal = 1;
    gpu->writeGPURegister(GPU_TEXTURE_HEIGHT2, texture_unit, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_TEXTURE_DEPTH2, texture_unit, data);
    data.txFormat = GPU_RGBA8888;
    gpu->writeGPURegister(GPU_TEXTURE_FORMAT, texture_unit, data);
    data.txCompression = GPU_NO_TEXTURE_COMPRESSION;
    gpu->writeGPURegister(GPU_TEXTURE_COMPRESSION, texture_unit, data);
    data.txBlocking = GPU_TXBLOCK_TEXTURE;
    gpu->writeGPURegister(GPU_TEXTURE_BLOCKING, texture_unit, data);
    data.booleanVal = false;
    gpu->writeGPURegister(GPU_TEXTURE_REVERSE, texture_unit, data);
    data.txClamp = GPU_TEXT_REPEAT;
    gpu->writeGPURegister(GPU_TEXTURE_WRAP_S, texture_unit, data);
    data.txClamp = GPU_TEXT_REPEAT;
    gpu->writeGPURegister(GPU_TEXTURE_WRAP_T, texture_unit, data);
    data.txClamp = GPU_TEXT_REPEAT;
    gpu->writeGPURegister(GPU_TEXTURE_WRAP_R, texture_unit, data);
	data.booleanVal = false;
	gpu->writeGPURegister(GPU_TEXTURE_D3D9_V_INV, texture_unit, data); // Oe!
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_TEXTURE_ENABLE, texture_unit, data);
    // This texture could be used as tilemap, so set default texture to all faces
    for(u32bit face = 0; face < CUBEMAP_IMAGES; face ++) {
        u32bit offset = texture_unit * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + face;
        gpu->writeGPUAddrRegister(GPU_TEXTURE_ADDRESS, offset, default_texture);
    }
}

void CSamplers9::update_texture(StateDataNode* s_texture, u32bit texture_unit) {

      GPURegData data;
    GPUProxy* gpu = GPUProxy::get_instance();

    D3D_DEBUG( cout << "CSAMPLERS9: Updating GPU texture unit " << (int)texture_unit << endl; )

    if(s_texture->get_id().name == NAME_TEXTURE_9) {
        data.txMode = GPU_TEXTURE2D;
        gpu->writeGPURegister(GPU_TEXTURE_MODE, texture_unit, data);

        UINT width;
        UINT height;
        s_texture->get_child(StateId(NAME_WIDTH))->read_data(&width);
        s_texture->get_child(StateId(NAME_HEIGHT))->read_data(&height);
        data.uintVal = width;
        gpu->writeGPURegister(GPU_TEXTURE_WIDTH, texture_unit, data);
        data.uintVal = height;
        gpu->writeGPURegister(GPU_TEXTURE_HEIGHT, texture_unit, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_DEPTH, texture_unit, data);
        data.uintVal = ceiledLog2(width);
        gpu->writeGPURegister(GPU_TEXTURE_WIDTH2, texture_unit, data);
        data.uintVal = ceiledLog2(height);
        gpu->writeGPURegister(GPU_TEXTURE_HEIGHT2, texture_unit, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_DEPTH2, texture_unit, data);
    }
    else if(s_texture->get_id().name == NAME_CUBETEXTURE_9) {
        data.txMode = GPU_TEXTURECUBEMAP;
        gpu->writeGPURegister(GPU_TEXTURE_MODE, texture_unit, data);

        UINT length;
        s_texture->get_child(StateId(NAME_LENGTH))->read_data(&length);
        data.uintVal = length;
        gpu->writeGPURegister(GPU_TEXTURE_WIDTH, texture_unit, data);
        data.uintVal = length;
        gpu->writeGPURegister(GPU_TEXTURE_HEIGHT, texture_unit, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_DEPTH, texture_unit, data);
        data.uintVal = ceiledLog2(length);
        gpu->writeGPURegister(GPU_TEXTURE_WIDTH2, texture_unit, data);
        data.uintVal = ceiledLog2(length);
        gpu->writeGPURegister(GPU_TEXTURE_HEIGHT2, texture_unit, data);
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_TEXTURE_DEPTH2, texture_unit, data);
    }
    else if(s_texture->get_id().name == NAME_VOLUMETEXTURE_9) {
        data.txMode = GPU_TEXTURE3D;
        gpu->writeGPURegister(GPU_TEXTURE_MODE, texture_unit, data);

        UINT width;
        UINT height;
        UINT depth;
        s_texture->get_child(StateId(NAME_WIDTH))->read_data(&width);
        s_texture->get_child(StateId(NAME_HEIGHT))->read_data(&height);
        s_texture->get_child(StateId(NAME_DEPTH))->read_data(&depth);
        data.uintVal = width;
        gpu->writeGPURegister(GPU_TEXTURE_WIDTH, texture_unit, data);
        data.uintVal = height;
        gpu->writeGPURegister(GPU_TEXTURE_HEIGHT, texture_unit, data);
        data.uintVal = depth;
        gpu->writeGPURegister(GPU_TEXTURE_DEPTH, texture_unit, data);
        data.uintVal = ceiledLog2(width);
        gpu->writeGPURegister(GPU_TEXTURE_WIDTH2, texture_unit, data);
        data.uintVal = ceiledLog2(height);
        gpu->writeGPURegister(GPU_TEXTURE_HEIGHT2, texture_unit, data);
        data.uintVal = ceiledLog2(depth);
        gpu->writeGPURegister(GPU_TEXTURE_DEPTH2, texture_unit, data);
    }

    D3DFORMAT format;
    DWORD usage;
    s_texture->get_child(StateId(NAME_FORMAT))->read_data(&format);
    s_texture->get_child(StateId(NAME_USAGE))->read_data(&usage);
      if((format != D3DFMT_A8R8G8B8) &
         (format != D3DFMT_X8R8G8B8 ) &
         (format != D3DFMT_DXT1) &
         (format != D3DFMT_DXT5) &
         (format != D3DFMT_Q8W8V8U8) &
         (format != D3DFMT_A16B16G16R16)) {
          D3D_DEBUG( cout << "CSAMPLERS9: WARNING: Not supported texture format " << (int)format << endl; )
          reset_texture(texture_unit);
          return;
      }

    data.txFormat = get_texture_format(format);
    gpu->writeGPURegister(GPU_TEXTURE_FORMAT, texture_unit, data);
    if(format == D3DFMT_DXT1)
        data.txCompression = GPU_S3TC_DXT1_RGBA;
    else if(format == D3DFMT_DXT5)
        data.txCompression = GPU_S3TC_DXT5_RGBA;
    else
        data.txCompression = GPU_NO_TEXTURE_COMPRESSION;
    gpu->writeGPURegister(GPU_TEXTURE_COMPRESSION, texture_unit, data);

	data.txBlocking = (usage & D3DUSAGE_RENDERTARGET) ? GPU_TXBLOCK_FRAMEBUFFER : GPU_TXBLOCK_TEXTURE;
    gpu->writeGPURegister(GPU_TEXTURE_BLOCKING, texture_unit, data);

	data.booleanVal = (usage & D3DUSAGE_RENDERTARGET) ? true : false;
	gpu->writeGPURegister(GPU_TEXTURE_D3D9_V_INV, texture_unit, data); // Oe!


    // Texel's components must be swapped with some texture types
    /* NOTE Render targets are written by gpu, so the order of components hasn't to
            be swapped */
    ///@todo check if Q8W8V8U8 must be swapped too
    if(((format == D3DFMT_A8R8G8B8)|
    (format == D3DFMT_X8R8G8B8)|
	(format == D3DFMT_Q8W8V8U8)) &&
	!(usage & D3DUSAGE_RENDERTARGET)) {
    	data.booleanVal = true;
	}
    else data.booleanVal = false;
    gpu->writeGPURegister(GPU_TEXTURE_D3D9_COLOR_CONV, texture_unit, data); // Oe!
	data.booleanVal = false;
    gpu->writeGPURegister(GPU_TEXTURE_REVERSE, texture_unit, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_TEXTURE_ENABLE, texture_unit, data);

    UINT level_count;
    s_texture->get_child(StateId(NAME_LEVEL_COUNT))->read_data(&level_count);
    data.uintVal = level_count - 1;
    gpu->writeGPURegister(GPU_TEXTURE_MAX_LEVEL, texture_unit, data);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_TEXTURE_MIN_LEVEL, texture_unit, data);

    for(UINT level = 0; level < level_count; level ++) {
        if(s_texture->get_id().name == NAME_TEXTURE_9) {
            // Locate the gpu memory assigned to each level
            ResourceAssignationTable* a_surface_memory;
            a_surface_memory = AssignationsRegistry::get_table(NAME_SURFACE_MEMORY);
            // Locate surface
            IDirect3DSurface9* i_surface;
            s_texture->get_child(StateId(NAME_MIPMAP, StateIndex(level)))->read_data(&i_surface);
            // Get memory associated to it
            UsageId usage(NAME_SURFACE, UsageIndex(i_surface));
            ResourceId res;
            res = a_surface_memory->get_assigned_to(usage);
            u32bit md = (u32bit)res.index;
            // Write address
            gpu->writeGPUAddrRegister(GPU_TEXTURE_ADDRESS, texture_unit * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + level * CUBEMAP_IMAGES, md);
            D3D_DEBUG( cout << "CSAMPLERS9: Setting mipmap " << (int)level << ", address is md " << (int)md << " at offset "; )
            D3D_DEBUG( cout << (int)(texture_unit * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + level * CUBEMAP_IMAGES) << endl; )
        }
        else if(s_texture->get_id().name == NAME_CUBETEXTURE_9) {
            // Locate the gpu memory assigned to each level
            ResourceAssignationTable* a_surface_memory;
            a_surface_memory = AssignationsRegistry::get_table(NAME_SURFACE_MEMORY);
            // Basically the same but for each face
            Names names[] = { NAME_FACE_POSITIVE_X, NAME_FACE_NEGATIVE_X,
                              NAME_FACE_POSITIVE_Y, NAME_FACE_NEGATIVE_Y,
                              NAME_FACE_POSITIVE_Z, NAME_FACE_NEGATIVE_Z };
            for(UINT face = 0; face < 6; face++) {
                StateDataNode* s_face = s_texture->get_child(StateId(names[face]));
                // Locate surface
                IDirect3DSurface9* i_surface;
                s_face->get_child(StateId(NAME_MIPMAP, StateIndex(level)))->read_data(&i_surface);
                // Get memory associated to it
                UsageId usage(NAME_SURFACE, UsageIndex(i_surface));
                ResourceId res;
                res = a_surface_memory->get_assigned_to(usage);
                u32bit md = (u32bit)res.index;
                // Write address
                u32bit offset = texture_unit * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + level * CUBEMAP_IMAGES + face;
                gpu->writeGPUAddrRegister(GPU_TEXTURE_ADDRESS, offset, md);
                D3D_DEBUG( cout << "CSAMPLERS9: Setting face " << names[face] << " mipmap " << (int)level << ", address is md " << (int)md << " at offset "; )
                D3D_DEBUG( cout << (int)(offset) << endl; )
            }
        }
        else if(s_texture->get_id().name == NAME_VOLUMETEXTURE_9) {
            // Locate the gpu memory assigned to each level
            ResourceAssignationTable* a_volume_memory;
            a_volume_memory = AssignationsRegistry::get_table(NAME_VOLUME_MEMORY);
            // Locate volume
            IDirect3DVolume9* i_volume;
            s_texture->get_child(StateId(NAME_MIPMAP, StateIndex(level)))->read_data(&i_volume);
            // Get memory associated to it
            UsageId usage(NAME_VOLUME, UsageIndex(i_volume));
            ResourceId res;
            res = a_volume_memory->get_assigned_to(usage);
            u32bit md = (u32bit)res.index;
            // Write address
            gpu->writeGPUAddrRegister(GPU_TEXTURE_ADDRESS, texture_unit * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + level * CUBEMAP_IMAGES, md);
            D3D_DEBUG( cout << "CSAMPLERS9: Setting mipmap " << (int)level << ", address is md " << (int)md << " at offset "; )
            D3D_DEBUG( cout << (int)(texture_unit * MAX_TEXTURE_SIZE * CUBEMAP_IMAGES + level * CUBEMAP_IMAGES) << endl; )
        }
    }
}

void CSamplers9::update_addressing(StateDataNode* s_sampler, u32bit texture_unit) {
     DWORD address;
     s_sampler->get_child(NAME_ADDRESSU)->read_data(&address);
     GPURegData data;
     data.txClamp = get_clamp(address);
     GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_WRAP_S,
            texture_unit, data);

     s_sampler->get_child(NAME_ADDRESSV)->read_data(&address);
     data.txClamp = get_clamp(address);
     GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_WRAP_T,
            texture_unit, data);

     s_sampler->get_child(NAME_ADDRESSW)->read_data(&address);
     data.txClamp = get_clamp(address);
     GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_WRAP_R,
            texture_unit, data);

}

void CSamplers9::update_filtering(StateDataNode* s_sampler, u32bit texture_unit) {
     D3DTEXTUREFILTERTYPE min_filter;
     D3DTEXTUREFILTERTYPE mip_filter;
     s_sampler->get_child(StateId(NAME_MIN_FILTER))->read_data(&min_filter);
     s_sampler->get_child(StateId(NAME_MIP_FILTER))->read_data(&mip_filter);

     GPURegData data;
     if(min_filter == D3DTEXF_ANISOTROPIC) {
        DWORD max_anisotropy;
        s_sampler->get_child(StateId(NAME_MAX_ANISOTROPY))->read_data(&max_anisotropy);
        data.uintVal = max_anisotropy;
         GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_MAX_ANISOTROPY,
            texture_unit, data);
     }

     ///@note Not sure about mip_filter == D3DTEXF_NONE semantics
     if(((mip_filter == D3DTEXF_POINT) | (mip_filter == D3DTEXF_NONE)) &&
        (min_filter == D3DTEXF_POINT)) {
        data.txFilter = GPU_NEAREST_MIPMAP_NEAREST;
     }
     else if(((mip_filter == D3DTEXF_POINT) | (mip_filter == D3DTEXF_NONE)) &&
            ((min_filter == D3DTEXF_LINEAR) | (min_filter == D3DTEXF_ANISOTROPIC))) {
        data.txFilter = GPU_LINEAR_MIPMAP_NEAREST;
     }
     else if((mip_filter == D3DTEXF_LINEAR) && (min_filter == D3DTEXF_POINT)) {
        data.txFilter = GPU_NEAREST_MIPMAP_LINEAR;
     }
     else if((mip_filter == D3DTEXF_LINEAR) &&
            ((min_filter == D3DTEXF_LINEAR) | (min_filter == D3DTEXF_ANISOTROPIC))) {
        data.txFilter = GPU_LINEAR_MIPMAP_LINEAR;
     }
     else {
        data.txFilter = GPU_LINEAR_MIPMAP_LINEAR;
        D3D_DEBUG( cout << "CSAMPLERS9: WARNING: Not supported MIN filtering, using LINEAR mipmap LINEAR instead." << endl; )
     }

     GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_MIN_FILTER,
        texture_unit, data);

     D3DTEXTUREFILTERTYPE mag_filter;
     s_sampler->get_child(StateId(NAME_MAG_FILTER))->read_data(&mag_filter);
     switch (mag_filter) {
        case D3DTEXF_POINT:
            data.txFilter = GPU_NEAREST;
            break;
        case D3DTEXF_LINEAR:
            data.txFilter = GPU_LINEAR;
            break;
        default:
            data.txFilter = GPU_LINEAR;
            D3D_DEBUG( cout << "CSAMPLERS9: WARNING: Not supported MAG filter, using LINEAR instead." << endl; )
            break;
     };
     GPUProxy::get_instance()->writeGPURegister(GPU_TEXTURE_MAG_FILTER,
        texture_unit, data);
}


TextureFormat CSamplers9::get_texture_format(D3DFORMAT format) {
    TextureFormat txFormat;
    switch(format) {
        case D3DFMT_A8R8G8B8:
            txFormat = GPU_RGBA8888;
            break;
        case D3DFMT_X8R8G8B8:
            txFormat = GPU_RGB888;
            break;
        case D3DFMT_DXT1:
            txFormat = GPU_RGBA8888;
            break;
        case D3DFMT_DXT5:
            txFormat = GPU_RGBA8888;
            break;
        ///@todo check if this is correct
        case D3DFMT_Q8W8V8U8:
            txFormat = GPU_RGBA8888;
            break;
        case D3DFMT_A16B16G16R16:
            txFormat = GPU_RGBA16;
            break;

    }
    return txFormat;
}

ClampMode CSamplers9::get_clamp(DWORD addressing) {
    ClampMode txClamp;
    switch(addressing) {
        case D3DTADDRESS_CLAMP:
            txClamp = GPU_TEXT_CLAMP;
            break;
        case D3DTADDRESS_BORDER:
            txClamp = GPU_TEXT_CLAMP_TO_BORDER;
            break;
        case D3DTADDRESS_WRAP:
            txClamp = GPU_TEXT_REPEAT;
            break;
        case D3DTADDRESS_MIRROR:
            txClamp = GPU_TEXT_MIRRORED_REPEAT;
            break;
        default:
            D3D_DEBUG( cout << "CSAMPLERS9: WARNING: Not supported texture addressing mode" << endl; )
            txClamp = GPU_TEXT_MIRRORED_REPEAT;
            break;
    }
    return txClamp;

}

bool CSamplers9::is_sampler_assigned(UINT sampler) {
    ///@todo Find a way to avoid polling for sampler type
    ResourceAssignationTable* a_ps_texture = AssignationsRegistry::get_table(NAME_PS_TEXTURE);
    return (a_ps_texture->has_been_assigned(UsageId(NAME_SAMPLER, sampler)) |
        a_ps_texture->has_been_assigned(UsageId(NAME_SAMPLER_2D, sampler)) |
        a_ps_texture->has_been_assigned(UsageId(NAME_SAMPLER_VOLUME, sampler)) |
        a_ps_texture->has_been_assigned(UsageId(NAME_SAMPLER_CUBE, sampler)));
}

u32bit CSamplers9::get_assigned_texture_unit(UINT sampler) {
    ///@todo Find a way to avoid polling for sampler type
    ResourceAssignationTable* a_ps_texture = AssignationsRegistry::get_table(NAME_PS_TEXTURE);
    ResourceId r;
    if(a_ps_texture->has_been_assigned(UsageId(NAME_SAMPLER, sampler)))
        r = a_ps_texture->get_assigned_to(UsageId(NAME_SAMPLER, sampler));
    else if(a_ps_texture->has_been_assigned(UsageId(NAME_SAMPLER_2D, sampler)))
        r = a_ps_texture->get_assigned_to(UsageId(NAME_SAMPLER_2D, sampler));
    else if(a_ps_texture->has_been_assigned(UsageId(NAME_SAMPLER_CUBE, sampler)))
        r = a_ps_texture->get_assigned_to(UsageId(NAME_SAMPLER_CUBE, sampler));
    else if(a_ps_texture->has_been_assigned(UsageId(NAME_SAMPLER_VOLUME, sampler)))
        r = a_ps_texture->get_assigned_to(UsageId(NAME_SAMPLER_VOLUME, sampler));
    else throw "CSAMPLERS9: Texture unit not assigned";
    return r.index;
}

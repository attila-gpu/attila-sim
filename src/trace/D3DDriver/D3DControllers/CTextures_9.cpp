#include "Common.h"
#include "CTextures_9.h"


void CTextures9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_TEXTURE_9) {
        // Create mipmap levels
        UINT width;
        child->get_child(StateId(NAME_WIDTH))->read_data(&width);
        UINT height;
        child->get_child(StateId(NAME_HEIGHT))->read_data(&height);
        DWORD usage;
        child->get_child(StateId(NAME_USAGE))->read_data(&usage);
        D3DFORMAT format;
        child->get_child(StateId(NAME_FORMAT))->read_data(&format);
        UINT max_level;
        child->get_child(StateId(NAME_LEVELS))->read_data(&max_level);
        UINT created_levels = level_count(max_level, width, height);
        child->get_child(StateId(NAME_LEVEL_COUNT))->write_data(&created_levels);
        for(UINT i = 0; i < created_levels; i ++) {
            StateDataNode* s_mipmap = D3DState::create_texture_mipmap_state_9(i);
            // Calculate width, height, etc. for mipmaps and write that to mipmap state
            if((usage & D3DUSAGE_RENDERTARGET) | (usage & D3DUSAGE_DEPTHSTENCIL)) {
                if(created_levels > 1)
                    D3D_DEBUG( cout << "CTEXTURES9: WARNING: Creating a render target texture with more than one mipmap level" << endl; )
                s_mipmap->get_child(StateId(NAME_WIDTH))->write_data(&width);
                s_mipmap->get_child(StateId(NAME_HEIGHT))->write_data(&height);
            }
            else {
                /**@todo panic if width or height is not power 2 and more than one
                   mipmap level has to be created **/
                UINT mip_width = get1DMipLength(width, i, max_level, is_compressed(format));
                UINT mip_height = get1DMipLength(height, i, max_level, is_compressed(format));
                s_mipmap->get_child(StateId(NAME_WIDTH))->write_data(&mip_width);
                s_mipmap->get_child(StateId(NAME_HEIGHT))->write_data(&mip_height);
            }
            child->add_child(s_mipmap);
        }
    }
    else if(child->get_id().name == NAME_CUBETEXTURE_9) {
        // Create mipmap levels
        UINT length;
        child->get_child(StateId(NAME_LENGTH))->read_data(&length);
        DWORD usage;
        child->get_child(StateId(NAME_USAGE))->read_data(&usage);
        D3DFORMAT format;
        child->get_child(StateId(NAME_FORMAT))->read_data(&format);
        UINT max_level;
        child->get_child(StateId(NAME_LEVELS))->read_data(&max_level);
        UINT created_levels = level_count(max_level, length, length);
        child->get_child(StateId(NAME_LEVEL_COUNT))->write_data(&created_levels);

        Names names[] = { NAME_FACE_POSITIVE_X, NAME_FACE_NEGATIVE_X,
                          NAME_FACE_POSITIVE_Y, NAME_FACE_NEGATIVE_Y,
                          NAME_FACE_POSITIVE_Z, NAME_FACE_NEGATIVE_Z };
        for(UINT i = 0; i < 6; i++) {
            StateDataNode* s_face = child->get_child(StateId(names[i]));
            for(UINT j = 0; j < created_levels; j ++) {
                StateDataNode* s_mipmap = D3DState::create_cubetexture_mipmap_state_9(j);
                // Calculate width, height, etc. for mipmaps and write that to mipmap state
                if((usage & D3DUSAGE_RENDERTARGET) | (usage & D3DUSAGE_DEPTHSTENCIL)) {
                    ///@todo panic if more than 1 level has to be created
                    s_mipmap->get_child(StateId(NAME_LENGTH))->write_data(&length);
                }
                else {
                    /**@todo panic if width or height is not power 2 and more than one
                       mipmap level has to be created **/
                    UINT mip_length = get1DMipLength(length, j, max_level, is_compressed(format));
                    s_mipmap->get_child(StateId(NAME_LENGTH))->write_data(&mip_length);
                }
                s_face->add_child(s_mipmap);
            }
        }
    }
    else if(child->get_id().name == NAME_VOLUMETEXTURE_9) {
        // Create mipmap levels
        UINT width;
        child->get_child(StateId(NAME_WIDTH))->read_data(&width);
        UINT height;
        child->get_child(StateId(NAME_HEIGHT))->read_data(&height);
        UINT depth;
        child->get_child(StateId(NAME_DEPTH))->read_data(&depth);
        DWORD usage;
        child->get_child(StateId(NAME_USAGE))->read_data(&usage);
        D3DFORMAT format;
        child->get_child(StateId(NAME_FORMAT))->read_data(&format);
        UINT max_level;
        child->get_child(StateId(NAME_LEVELS))->read_data(&max_level);
        UINT created_levels = level_count(max_level, width, height, depth);
        child->get_child(StateId(NAME_LEVEL_COUNT))->write_data(&created_levels);
        for(UINT i = 0; i < created_levels; i ++) {
            StateDataNode* s_mipmap = D3DState::create_volumetexture_mipmap_state_9(i);
            /**@todo panic if width, height or depth is not power 2 and more than one
               mipmap level has to be created **/
            UINT mip_width = get1DMipLength(width, i, max_level, is_compressed(format));
            UINT mip_height = get1DMipLength(height, i, max_level, is_compressed(format));
            UINT mip_depth = get1DMipLength(depth, i, max_level, is_compressed(format));
            s_mipmap->get_child(StateId(NAME_WIDTH))->write_data(&mip_width);
            s_mipmap->get_child(StateId(NAME_HEIGHT))->write_data(&mip_height);
            s_mipmap->get_child(StateId(NAME_DEPTH))->write_data(&mip_depth);
            child->add_child(s_mipmap);
        }
    }

}

void CTextures9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
}

void CTextures9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
}

void CTextures9::on_added_controller(StateDataNode* node) {
}

void CTextures9::on_removed_controller(StateDataNode* node) {
}

CTextures9::CTextures9() {
}

CTextures9::~CTextures9() {
}



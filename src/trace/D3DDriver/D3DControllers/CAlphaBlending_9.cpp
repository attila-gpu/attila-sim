#include "Common.h"
#include "CAlphaBlending_9.h"

void CAlphaBlending9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    /// @todo
}

void CAlphaBlending9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    /// @todo
}

void CAlphaBlending9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    if(node->get_id().name == NAME_ALPHA_BLEND_ENABLED) {
        DWORD value;
        node->read_data(&value);
        GPURegData data;
        data.booleanVal = (value == TRUE);
        GPUProxy::get_instance()->writeGPURegister(GPU_COLOR_BLEND, data);
    }
    else if(node->get_id().name == NAME_SEPARATE_ALPHA_BLEND_ENABLED) {
        DWORD value;
        node->read_data(&value);
        if(value == FALSE) {
            // Propagate color functions  to alpha functions
            DWORD color;
            node->get_parent()->get_child(StateId(NAME_SRC_BLEND))->read_data(&color);
            update_blend_functions(D3DRS_SRCBLENDALPHA, color, false);
            node->get_parent()->get_child(StateId(NAME_DST_BLEND))->read_data(&color);
            update_blend_functions(D3DRS_DESTBLENDALPHA, color, false);
        }
    }
    else if(node->get_id().name == NAME_BLEND_OP) {
        DWORD value;
        node->read_data(&value);
        DWORD separate;
        node->get_parent()->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->read_data(&separate);
        bool sep = (separate == 0 ? false : true);
        update_blend_op(D3DRS_BLENDOP, value, sep);
    }
    else if(node->get_id().name == NAME_BLEND_OP_ALPHA) {
        DWORD value;
        node->read_data(&value);
        DWORD separate;
        node->get_parent()->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->read_data(&separate);
        bool sep = (separate == 0 ? false : true);
        update_blend_op(D3DRS_BLENDOPALPHA, value, sep);
    }
    else if(node->get_id().name == NAME_SRC_BLEND) {
        DWORD value;
        node->read_data(&value);
        DWORD separate;
        node->get_parent()->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->read_data(&separate);
        bool sep = (separate == 0 ? false : true);
        update_blend_functions(D3DRS_SRCBLEND, value, sep);
    }
    else if(node->get_id().name == NAME_SRC_BLEND_ALPHA) {
        DWORD value;
        node->read_data(&value);
        DWORD separate;
        node->get_parent()->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->read_data(&separate);
        bool sep = (separate == 0 ? false : true);
        update_blend_functions(D3DRS_SRCBLENDALPHA, value, sep);
    }
    else if(node->get_id().name == NAME_DST_BLEND) {
        DWORD value;
        node->read_data(&value);
        DWORD separate;
        node->get_parent()->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->read_data(&separate);
        bool sep = (separate == 0 ? false : true);
        update_blend_functions(D3DRS_DESTBLEND, value, sep);
    }
    else if(node->get_id().name == NAME_DST_BLEND_ALPHA) {
        DWORD value;
        node->read_data(&value);
        DWORD separate;
        node->get_parent()->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->read_data(&separate);
        bool sep = (separate == 0 ? false : true);
        update_blend_functions(D3DRS_DESTBLENDALPHA, value, sep);
    }
    else if(node->get_id().name == NAME_BLEND_FACTOR) {
        DWORD value;
        node->read_data(&value);
        QuadFloat qf_value;
        d3dcolor2quadfloat(value, &qf_value);
        GPURegData data;
        for(u32bit i = 0; i < 4; i++)
            data.qfVal[i] = qf_value[i];
        GPUProxy::get_instance()->writeGPURegister(GPU_BLEND_COLOR, data);
    }
}

void CAlphaBlending9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // Watch Alpha Blend state
        node->get_child(StateId(NAME_ALPHA_BLEND_ENABLED))->add_controller(this);
        node->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->add_controller(this);
        node->get_child(StateId(NAME_BLEND_FACTOR))->add_controller(this);
        node->get_child(StateId(NAME_SRC_BLEND))->add_controller(this);
        node->get_child(StateId(NAME_DST_BLEND))->add_controller(this);
        node->get_child(StateId(NAME_SRC_BLEND_ALPHA))->add_controller(this);
        node->get_child(StateId(NAME_DST_BLEND_ALPHA))->add_controller(this);
        node->get_child(StateId(NAME_BLEND_OP))->add_controller(this);
        node->get_child(StateId(NAME_BLEND_OP_ALPHA))->add_controller(this);
    }
}

void CAlphaBlending9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // Unwatch Alpha Blend state
        node->get_child(StateId(NAME_ALPHA_BLEND_ENABLED))->remove_controller(this);
        node->get_child(StateId(NAME_SEPARATE_ALPHA_BLEND_ENABLED))->remove_controller(this);
        node->get_child(StateId(NAME_BLEND_FACTOR))->remove_controller(this);
        node->get_child(StateId(NAME_SRC_BLEND))->remove_controller(this);
        node->get_child(StateId(NAME_DST_BLEND))->remove_controller(this);
        node->get_child(StateId(NAME_SRC_BLEND_ALPHA))->remove_controller(this);
        node->get_child(StateId(NAME_DST_BLEND_ALPHA))->remove_controller(this);
        node->get_child(StateId(NAME_BLEND_OP))->remove_controller(this);
        node->get_child(StateId(NAME_BLEND_OP_ALPHA))->remove_controller(this);
    }
}

CompareMode CAlphaBlending9::get_compare(DWORD func) {
    CompareMode compare;
    switch(func) {
        case D3DCMP_NEVER:
            compare = GPU_NEVER;
            break;
        case D3DCMP_LESS:
            compare = GPU_LESS;
            break;
        case D3DCMP_EQUAL:
            compare = GPU_EQUAL;
            break;
        case D3DCMP_LESSEQUAL:
            compare = GPU_LEQUAL;
            break;
        case D3DCMP_GREATER:
            compare = GPU_GREATER;
            break;
        case D3DCMP_NOTEQUAL:
            compare = GPU_NOTEQUAL;
            break;
        case D3DCMP_GREATEREQUAL:
            compare = GPU_GEQUAL;
            break;
        case D3DCMP_ALWAYS:
            compare = GPU_ALWAYS;
            break;
        default:
            compare = GPU_ALWAYS;
            break;
    }
    return compare;
}

void CAlphaBlending9::update_blend_functions(DWORD render_state, DWORD value, bool separate) {
    GPUProxy* gpu = GPUProxy::get_instance();
    GPURegData data;

    if((render_state == D3DRS_SRCBLEND) |
       (render_state == D3DRS_SRCBLENDALPHA) |
       (render_state == D3DRS_DESTBLEND) |
       (render_state == D3DRS_DESTBLENDALPHA)) {

        if (value == D3DBLEND_BOTHSRCALPHA) {
            ///@see SDK docs
            update_blend_functions(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA, separate);
            update_blend_functions(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA, separate);
            return;
        }
        else if (value == D3DBLEND_BOTHINVSRCALPHA) {
            ///@see SDK docs
            update_blend_functions(D3DRS_SRCBLEND, D3DBLEND_INVSRCALPHA, separate);
            update_blend_functions(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA, separate);
            return;
        }

        BlendFunction color_function;
        BlendFunction alpha_function;
        get_blend_functions(value, &color_function, &alpha_function);

        if(render_state == D3DRS_SRCBLEND) {
            data.blendFunction = color_function;
            gpu->writeGPURegister(GPU_BLEND_SRC_RGB, data);
            if(!separate) {
                data.blendFunction = alpha_function;
                gpu->writeGPURegister(GPU_BLEND_SRC_ALPHA, data);
            }
        }
        else if(render_state == D3DRS_SRCBLENDALPHA) {
            if(separate) {
                data.blendFunction = alpha_function;
                gpu->writeGPURegister(GPU_BLEND_SRC_ALPHA, data);
            }
        }
        else if(render_state == D3DRS_DESTBLEND) {
            data.blendFunction = color_function;
            gpu->writeGPURegister(GPU_BLEND_DST_RGB, data);
            if(!separate)
                gpu->writeGPURegister(GPU_BLEND_DST_ALPHA, data);
        }
        else if(render_state == D3DRS_DESTBLENDALPHA) {
            if(separate) {
                data.blendFunction = alpha_function;
                gpu->writeGPURegister(GPU_BLEND_DST_ALPHA, data);
            }
        }
    }
}

void CAlphaBlending9::update_blend_op(DWORD state, DWORD value, bool separate) {
    if(separate) {
        D3D_DEBUG( cout << "CRASTEROPERATIONS9: WARNING: Separate blending equations are not supported." << endl; )
        return;
    }
    GPURegData data;
    data.blendEquation = get_blend_equation(value);
    GPUProxy::get_instance()->writeGPURegister(GPU_BLEND_EQUATION, data);
}

BlendEquation CAlphaBlending9::get_blend_equation(DWORD op) {
    switch (op) {
        case D3DBLENDOP_ADD:
            return BLEND_FUNC_ADD;
        case D3DBLENDOP_SUBTRACT:
            return BLEND_FUNC_SUBTRACT;
        case D3DBLENDOP_REVSUBTRACT:
            return BLEND_FUNC_REVERSE_SUBTRACT;
        case D3DBLENDOP_MAX:
            return BLEND_MIN;
        case D3DBLENDOP_MIN:
            return BLEND_MIN;
        default:
            panic("CAlphaBlending9", "get_blend_equation", "Unsupported blend equation");
    }
    // to avoid stupid warnings...
    return BLEND_MIN;
}

void CAlphaBlending9::get_blend_functions(DWORD function, BlendFunction* color_f, BlendFunction* alpha_f) {
    switch(function) {
        case D3DBLEND_ZERO: // (0, 0, 0, 0)
            *color_f = BLEND_ZERO;
            *alpha_f = BLEND_ZERO;
            break;
        case D3DBLEND_ONE: // (1, 1, 1, 1)
            *color_f = BLEND_ONE;
            *alpha_f = BLEND_ONE;
            break;
        case D3DBLEND_SRCCOLOR: // (Rs, Gs, Bs, As)
            *color_f = BLEND_SRC_COLOR;
            *alpha_f = BLEND_SRC_ALPHA;
            break;
        case D3DBLEND_INVSRCCOLOR: // (1-Rs, 1-Gs, 1-Bs, 1-As):
            *color_f = BLEND_ONE_MINUS_SRC_COLOR;
            *alpha_f = BLEND_ONE_MINUS_SRC_ALPHA;
            break;
        case D3DBLEND_SRCALPHA: // (As, As, As, As)
            *color_f = BLEND_SRC_ALPHA;
            *alpha_f = BLEND_SRC_ALPHA;
            break;
        case D3DBLEND_INVSRCALPHA: // (1-As, 1-As, 1-As, 1-As)
            *color_f = BLEND_ONE_MINUS_SRC_ALPHA;
           *alpha_f = BLEND_ONE_MINUS_SRC_ALPHA;
            break;
        case D3DBLEND_DESTCOLOR: // (Rd, Gd, Bd, Ad)
            *color_f = BLEND_DST_COLOR;
            *alpha_f = BLEND_DST_ALPHA;
            break;
        case D3DBLEND_INVDESTCOLOR: // (1-Rd, 1-Gd, 1-Bd, 1-Ad):
            *color_f = BLEND_ONE_MINUS_DST_COLOR;
            *alpha_f = BLEND_ONE_MINUS_DST_ALPHA;
            break;
        case D3DBLEND_DESTALPHA: // (Ad, Ad, Ad, Ad)
            *color_f = BLEND_DST_ALPHA;
            *alpha_f = BLEND_DST_ALPHA;
            break;
        case D3DBLEND_INVDESTALPHA: // (1-Ad, 1-Ad, 1-Ad, 1-Ad)
            *color_f = BLEND_ONE_MINUS_DST_ALPHA;
            *alpha_f = BLEND_ONE_MINUS_DST_ALPHA;
            break;
        case D3DBLEND_SRCALPHASAT: //(f, f, f, 1); f = min(As, 1-Ad)
            D3D_DEBUG( cout << "CRASTEROPERATIONS9: WARNING: Approximating D3DBLEND_SRCALPHASAT." << endl; )
            *color_f = BLEND_SRC_ALPHA_SATURATE;
            *alpha_f = BLEND_ONE;
            break;
        case D3DBLEND_BLENDFACTOR: // Use constant color
            *color_f = BLEND_CONSTANT_COLOR;
            *alpha_f = BLEND_CONSTANT_ALPHA;
            break;
        case D3DBLEND_INVBLENDFACTOR: // Use constant color
            *color_f = BLEND_ONE_MINUS_CONSTANT_COLOR;
            *alpha_f = BLEND_ONE_MINUS_CONSTANT_ALPHA;
            break;
    }
}

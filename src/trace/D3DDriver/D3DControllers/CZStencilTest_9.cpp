#include "Common.h"
#include "CZStencilTest_9.h"

void CZStencilTest9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
}

void CZStencilTest9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
}

void CZStencilTest9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    if (node->get_id().name == NAME_CURRENT_DEPTHSTENCIL)
    {
        GPURegData data;
        StateDataNode* s_device = node->get_parent();
        IDirect3DSurface9* currentDepthStencil;
        
        //  Get the pointer to the new surface that will be used as depth/stencil buffer.
        node->read_data(&currentDepthStencil);
        
        //  Check for a null pointer.
        if (currentDepthStencil == NULL)
        {
            //  If the depth stencil buffer has been unset (NULL pointer) then disable z and stencil tests.
            data.booleanVal = false;
            GPUProxy::get_instance()->writeGPURegister(GPU_DEPTH_TEST, data);
            GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_TEST, data);
        }
        else
        {
            //  If the current depth stencil buffer is enabled, use the stored state to re enable the depth and stencil test
            DWORD depthEnabled;
            DWORD stencilEnabled;
            
            s_device->get_child(StateId(NAME_Z_ENABLED))->read_data(&depthEnabled);
            s_device->get_child(StateId(NAME_STENCIL_ENABLED))->read_data(&stencilEnabled);
                        
            data.booleanVal = (depthEnabled == D3DZB_TRUE);
            GPUProxy::get_instance()->writeGPURegister(GPU_DEPTH_TEST, data);
            
            data.booleanVal = (stencilEnabled == TRUE);
            GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_TEST, data);
        }
    }
    else if(node->get_id().name == NAME_Z_ENABLED)
    {
        DWORD z_enable;
        node->read_data(&z_enable);

        StateDataNode* s_device = node->get_parent();

        // First check if a depth stencil buffer is associated (not NULL pointer)
        IDirect3DSurface9* currentDepthStencil;
        s_device->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->read_data(&currentDepthStencil);
        
        if (currentDepthStencil != NULL)
        {
        
            if(z_enable == D3DZB_USEW)
            {
                D3D_DEBUG( cout << "CZSTENCILTEST9: WARNING: W buffering not supported." << endl; )
            }
            
            if(z_enable == D3DZB_TRUE)
                D3D_DEBUG( cout << "CZSTENCILTEST9: Enabling Z Test." << endl; )
            else
                D3D_DEBUG( cout << "CZSTENCILTEST9: Disabling Z Test." << endl; )                
                        
            GPURegData data;
            data.booleanVal = (z_enable == D3DZB_TRUE);
            GPUProxy::get_instance()->writeGPURegister(GPU_DEPTH_TEST, data);
        }
    }
    else if(node->get_id().name == NAME_Z_FUNCTION)
    {
        D3D_DEBUG( cout << "CZSTENCILTEST9: Updating Z Function." << endl; )
        D3DCMPFUNC z_func;
        node->read_data(&z_func);
        GPURegData data;
        data.compare = get_compare(z_func);
        GPUProxy::get_instance()->writeGPURegister(GPU_DEPTH_FUNCTION, data);
        
        //  Check if depth comparison function is different than LEQUAL.
        if (z_func != D3DCMP_LESSEQUAL)
        {
            //  Disable Hierarchical Z.
            data.booleanVal = false;
            GPUProxy::get_instance()->writeGPURegister(GPU_HIERARCHICALZ, data);
        }
    }
    else if(node->get_id().name == NAME_Z_WRITE_ENABLE) {
        D3D_DEBUG( cout << "CZSTENCILTEST9: Updating Z Write Mask." << endl; )
        DWORD z_write_enable;
        node->read_data(&z_write_enable);
        GPURegData data;
        data.booleanVal = (z_write_enable == TRUE);
        GPUProxy::get_instance()->writeGPURegister(GPU_DEPTH_MASK, data);
    }
    else if(node->get_id().name == NAME_STENCIL_ENABLED)
    {
        DWORD value;
        node->read_data(&value);

        StateDataNode* s_device = node->get_parent();

        // First check if a depth stencil buffer is associated (not NULL pointer)
        IDirect3DSurface9* currentDepthStencil;
        s_device->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->read_data(&currentDepthStencil);
        
        if (currentDepthStencil != NULL)
        {
        
            GPURegData data;
            data.booleanVal = (value == TRUE);
            GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_TEST, data);
        }
    }
    else if(node->get_id().name == NAME_STENCIL_REF) {
        DWORD value;
        node->read_data(&value);
        GPURegData data;
        data.uintVal = value;
        GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_REFERENCE, data);
    }
    else if(node->get_id().name == NAME_STENCIL_FUNC) {
        D3DCMPFUNC value;
        node->read_data(&value);
        GPURegData data;
        data.compare = get_compare(value);
        GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_FUNCTION, data);
    }
    else if(node->get_id().name == NAME_STENCIL_FAIL) {
        D3DSTENCILOP value;
        node->read_data(&value);
        GPURegData data;
        data.stencilUpdate = get_update_func(value);
        GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);
    }
    else if(node->get_id().name == NAME_STENCIL_ZFAIL) {
        D3DSTENCILOP value;
        node->read_data(&value);
        GPURegData data;
        data.stencilUpdate = get_update_func(value);
        GPUProxy::get_instance()->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);
    }
    else if(node->get_id().name == NAME_STENCIL_PASS) {
        D3DSTENCILOP value;
        node->read_data(&value);
        GPURegData data;
        data.stencilUpdate = get_update_func(value);
        GPUProxy::get_instance()->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);
    }
    else if(node->get_id().name == NAME_STENCIL_MASK) {
        DWORD value;
        node->read_data(&value);
        GPURegData data;
        data.uintVal = value;
        GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);
    }
    else if(node->get_id().name == NAME_STENCIL_WRITE_MASK) {
        DWORD value;
        node->read_data(&value);
        GPURegData data;
        data.uintVal = value;
        GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data);
    }
    else if ((node->get_id().name == NAME_ALPHA_TEST_ENABLE) || (node->get_id().name == NAME_ALPHA_FUNC))
    {
		DWORD alphaTestEnabled;
		D3DCMPFUNC alphaTestFunction;
		
        StateDataNode* s_device = node->get_parent();
        
	    s_device->get_child(StateId(NAME_ALPHA_FUNC))->read_data(&alphaTestFunction);
	    s_device->get_child(StateId(NAME_ALPHA_TEST_ENABLED))->read_data(&alphaTestEnabled);
		    
        GPURegData data;

		//  Check if alpha test has been enabled
		if (alphaTestEnabled && (alphaTestFunction != D3DCMP_ALWAYS))
		{
	        //  Check if early z is disabled.
            GPUProxy::get_instance()->readGPURegister(GPU_EARLYZ, &data);
            if (data.booleanVal)
            {
                //  Disable early z.
                data.booleanVal = false;
                GPUProxy::get_instance()->writeGPURegister(GPU_EARLYZ, data);
            }
	    }
	    else
	    {
	        //  Check if early z is enabled.
            GPUProxy::get_instance()->readGPURegister(GPU_EARLYZ, &data);
            if (!data.booleanVal)
            {
                //  Enable early z.
                data.booleanVal = true;
                GPUProxy::get_instance()->writeGPURegister(GPU_EARLYZ, data);
            }
        }
    }
}

void CZStencilTest9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // Watch Z test state
        node->get_child(StateId(NAME_Z_ENABLED))->add_controller(this);
        node->get_child(StateId(NAME_Z_FUNCTION))->add_controller(this);
        node->get_child(StateId(NAME_Z_WRITE_ENABLE))->add_controller(this);

        // Watch stencil state
        node->get_child(StateId(NAME_STENCIL_ENABLED))->add_controller(this);
        node->get_child(StateId(NAME_STENCIL_REF))->add_controller(this);
        node->get_child(StateId(NAME_STENCIL_FUNC))->add_controller(this);
        node->get_child(StateId(NAME_STENCIL_FAIL))->add_controller(this);
        node->get_child(StateId(NAME_STENCIL_ZFAIL))->add_controller(this);
        node->get_child(StateId(NAME_STENCIL_PASS))->add_controller(this);
        node->get_child(StateId(NAME_STENCIL_MASK))->add_controller(this);
        node->get_child(StateId(NAME_STENCIL_WRITE_MASK))->add_controller(this);

        //  Watch depth stencil buffer
        node->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->add_controller(this);
        
        //  Watch alpha test state
        node->get_child(StateId(NAME_ALPHA_TEST_ENABLED))->add_controller(this);
        node->get_child(StateId(NAME_ALPHA_FUNC))->add_controller(this);
    }
}

void CZStencilTest9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        // Unwatch Z test state
        node->get_child(StateId(NAME_Z_ENABLED))->remove_controller(this);
        node->get_child(StateId(NAME_Z_FUNCTION))->remove_controller(this);
        node->get_child(StateId(NAME_Z_WRITE_ENABLE))->remove_controller(this);

        // Unwatch stencil test state
        node->get_child(StateId(NAME_STENCIL_ENABLED))->remove_controller(this);
        node->get_child(StateId(NAME_STENCIL_REF))->remove_controller(this);
        node->get_child(StateId(NAME_STENCIL_FUNC))->remove_controller(this);
        node->get_child(StateId(NAME_STENCIL_FAIL))->remove_controller(this);
        node->get_child(StateId(NAME_STENCIL_ZFAIL))->remove_controller(this);
        node->get_child(StateId(NAME_STENCIL_PASS))->remove_controller(this);
        node->get_child(StateId(NAME_STENCIL_MASK))->remove_controller(this);
        node->get_child(StateId(NAME_STENCIL_WRITE_MASK))->remove_controller(this);
        
        //  Unwatch depth stencil buffer
        node->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->add_controller(this);

        //  Unwatch alpha test state
        node->get_child(StateId(NAME_ALPHA_FUNC))->remove_controller(this);
        node->get_child(StateId(NAME_ALPHA_TEST_ENABLED))->remove_controller(this);
    }
}

StencilUpdateFunction CZStencilTest9::get_update_func(D3DSTENCILOP op) {
    StencilUpdateFunction func;
    switch (op) {
        case D3DSTENCILOP_KEEP:
            func = STENCIL_KEEP;
            break;
        case D3DSTENCILOP_ZERO:
            func = STENCIL_ZERO;
            break;
        case D3DSTENCILOP_REPLACE:
            func = STENCIL_REPLACE;
            break;
        case D3DSTENCILOP_INCRSAT:
            func = STENCIL_INCR;
            break;
        case D3DSTENCILOP_DECRSAT:
            func = STENCIL_DECR;
            break;
        case D3DSTENCILOP_INVERT:
            func = STENCIL_INVERT;
            break;
        case D3DSTENCILOP_INCR:
            func = STENCIL_INCR_WRAP;
            break;
        case D3DSTENCILOP_DECR:
            func = STENCIL_DECR_WRAP;
            break;
    }
    return func;
}

CompareMode CZStencilTest9::get_compare(D3DCMPFUNC func) {
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

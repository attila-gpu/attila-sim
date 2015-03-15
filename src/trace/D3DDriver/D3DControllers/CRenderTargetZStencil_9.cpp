#include "Common.h"
#include "CRenderTargetZStencil_9.h"

CRenderTargetZStencil9::CRenderTargetZStencil9() {
	frame_count = 0;
}

void CRenderTargetZStencil9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_DRAW_PRIMITIVE) {
         D3D_DEBUG( cout << "CRENDERTARGET9: Drawing non indexed batch" << endl; )
         GPUProxy::get_instance()->sendCommand(GPU_DRAW);
         /** @note Enable following for debug batches. */
         // GPUProxy::get_instance()->sendCommand(GPU_SWAPBUFFERS);
//         D3D_DEBUG(
//         	char filename[20];
//         	sprintf(filename, "state_%04d.txt", frame_count);
//         	D3DState::dump_state(filename);
//		 )
         frame_count ++;


    }
    else if(child->get_id().name == NAME_DRAW_INDEXED_PRIMITIVE) {

         D3D_DEBUG( cout << "CRENDERTARGET9: Drawing indexed batch" << endl; )
         GPUProxy::get_instance()->sendCommand(GPU_DRAW);
         /** @note Enable following for debug batches. */
         // GPUProxy::get_instance()->sendCommand(GPU_SWAPBUFFERS);
//         D3D_DEBUG(
//         	char filename[20];
//         	sprintf(filename, "state_%04d.txt", frame_count);
//         	D3DState::dump_state(filename);
//		 )
         frame_count ++;
    }
    else if(child->get_id().name == NAME_CLEAR) {
        DWORD flags;
        child->get_child(StateId(NAME_FLAGS))->read_data(&flags);
        D3DCOLOR color;
        child->get_child(StateId(NAME_COLOR))->read_data(&color);
        float d3d_z;
        child->get_child(NAME_Z)->read_data(&d3d_z);
        DWORD d3d_stencil;
        child->get_child(NAME_STENCIL)->read_data(&d3d_stencil);

        /* Configure gpu registers, when the clear is emulated using
           a rectangle these register writes are not necessary. I leave
           it this way for clarity. */
        if (flags & D3DCLEAR_TARGET) {
            GPURegData gpu_color;
            QuadFloat qf_value;
            d3dcolor2quadfloat(color, &qf_value);
            for(u32bit i = 0; i < 4; i++)
                gpu_color.qfVal[i] = qf_value[i];
            GPUProxy::get_instance()->writeGPURegister(GPU_COLOR_BUFFER_CLEAR, gpu_color);
        }
        if(flags & D3DCLEAR_STENCIL) {
            ///@note 8 bits for Stencil, the only format supported by gpu
            GPURegData gpu_stencil;
            gpu_stencil.uintVal = d3d_stencil;
            GPUProxy::get_instance()->writeGPURegister(GPU_STENCIL_BUFFER_CLEAR, gpu_stencil);
        }
        if(flags & D3DCLEAR_ZBUFFER) {
            // Set z clear on gpu
            ///@note 24 bits for Z, the only format supported by gpu
            GPURegData gpu_z;
            gpu_z.uintVal = u32bit(d3d_z / 1.0f * 0x00FFFFFF);
            GPUProxy::get_instance()->writeGPURegister(GPU_Z_BUFFER_CLEAR, gpu_z);
        }

        /* Choose the clear method: Emulated drawing a rectangle or
           fast clear using a gpu command*/

        // Is a implicit render target in use?
        IDirect3DSurface9* current_rt;
        IDirect3DSurface9* implicit_rt;
        StateDataNode* s_device = child->get_parent()->get_parent();
        s_device->get_child(StateId(NAME_CURRENT_RENDER_TARGET))->read_data(&current_rt);
        s_device->get_child(StateId(NAME_IMPLICIT_RENDER_TARGET))->read_data(&implicit_rt);

        bool partialColorClear = false;
        bool partialDepthClear = false;
        bool partialStencilClear = false;

        //  Check if the color buffer has to be cleared.        
        if (flags & D3DCLEAR_TARGET)
        {
            //  Clear color commands can't be used with a non implicit render target.
            //  Compression and color clear is not supported for render to texture.
            if (current_rt != implicit_rt)
            {
                D3D_DEBUG( cout << "CRENDERTARGETZSTENCIL9: Not implicit render target, clearing using rectangle." << endl; )

//cout << "CRenderTargetZStencil_9::on_add_node_child => WARNING.  Explicit render target color clear not enabled." << endl;
                //  Enable partial color clear.
                partialColorClear = true;
            }
            else
            {
                GPUProxy::get_instance()->sendCommand(GPU_CLEARCOLORBUFFER);
            }
        }
            
        // Is a implicit depth stencil buffer in use?
        IDirect3DSurface9* currentDepthStencil;
        IDirect3DSurface9* implicitDepthStencil;
        s_device->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->read_data(&currentDepthStencil);
        s_device->get_child(StateId(NAME_IMPLICIT_DEPTHSTENCIL))->read_data(&implicitDepthStencil);

        //  Check if both depth and stencil buffer have to be cleared.
        if ((flags & D3DCLEAR_STENCIL) && (flags & D3DCLEAR_ZBUFFER))
        {
            //  If the depth stencil buffer is not valid don't do the clear.
            if (currentDepthStencil != NULL)
            {
                //  Check if the active depth stencil buffer is the implicit one.
                if (currentDepthStencil == implicitDepthStencil)
                {
                    GPUProxy::get_instance()->sendCommand(GPU_CLEARZSTENCILBUFFER);
                }
                else
                {
cout << "CRenderTargetZStencil9::on_add_node_child => NOT IMPLEMENTED.  Clear of explicit depth stencil buffer not implemented yet" << endl;                
                }
            }
        }

        //  Check if only stencil buffer has to be cleared.        
        if ((flags & D3DCLEAR_STENCIL) && !(flags & D3DCLEAR_ZBUFFER))
        {
            //  NOTE:  This case is for now handled as a clear of both z and stencil buffer.
            //  A warning is issued.
            //  If the depth stencil buffer is not valid don't do the clear.
            if (currentDepthStencil != NULL)
            {
cout << "CRenderTargetZStencil9::on_add_node_child => NOT IMPLEMENTED.  Clear of stencil only not implemented yet." << endl;
                //  Enable partial stencil clear.
                //partialStencilClear = true;
    		    
                //  Check if the active depth stencil buffer is the implicit one.
            //    if (currentDepthStencil == implicitDepthStencil)
            //    {
            //       GPUProxy::get_instance()->sendCommand(GPU_CLEARZSTENCILBUFFER);
            //    }
            //    else
            //    {
//cout << "CRenderTargetZStencil9::on_add_node_child => NOT IMPLEMENTED.  Clear of explicit depth stencil buffer not implemented yet" << endl;                
            //    }
            }
        }
        
        //  Check if only depth buffer has to be cleared.
        if (!(flags & D3DCLEAR_STENCIL) && (flags & D3DCLEAR_ZBUFFER))
        {
            //  NOTE:  This case is for now handled as a clear of both z and stencil buffer.
            //  A warning is issued.
cout << "CRenderTargetZStencil9::on_add_node_child => WARNING.  Clear of depth only implemented as clear depth and stencil." << endl;                
            
            //  Enable partial depth clear.
            //partialDepthClear = true;
		    
            //  If the depth stencil buffer is not valid don't do the clear.
            if (currentDepthStencil != NULL)
            {
                //  Check if the active depth stencil buffer is the implicit one.
                if (currentDepthStencil == implicitDepthStencil)
                {
                    GPUProxy::get_instance()->sendCommand(GPU_CLEARZSTENCILBUFFER);
                }
                else
                {
cout << "CRenderTargetZStencil9::on_add_node_child => NOT IMPLEMENTED.  Clear of explicit depth stencil buffer not implemented yet" << endl;                
                }
            }
        }
        
        //  Check partial clears pending and build flags.
        DWORD partialClearFlags = 0;
        
        if (partialColorClear)
            partialClearFlags |= D3DCLEAR_TARGET;
        if (partialStencilClear)
            partialClearFlags |= D3DCLEAR_STENCIL;
        if (partialDepthClear)
            partialClearFlags |= D3DCLEAR_ZBUFFER;
        
        if (partialClearFlags != 0)
        {
            clear_using_rectangle(partialClearFlags, color, d3d_z, d3d_stencil);
        }        
    }
    else if(child->get_id().name == NAME_PRESENT) {

        IDirect3DSurface9* current_rt;
        IDirect3DSurface9* implicit_rt;
        StateDataNode* s_device = child->get_parent()->get_parent();
        s_device->get_child(StateId(NAME_CURRENT_RENDER_TARGET))->read_data(&current_rt);
        s_device->get_child(StateId(NAME_IMPLICIT_RENDER_TARGET))->read_data(&implicit_rt);
        if(current_rt == implicit_rt) {
			///@note Disable to debug batches
            GPUProxy::get_instance()->sendCommand(GPU_SWAPBUFFERS);
            swapped_front_back = !swapped_front_back;
        }
        else {
                /// @todo not sure what to do if render target is not the implicit.
                D3D_DEBUG( cout << "CRENDERTARGETZSTENCIL9: WARNING: Present operation with non implicit render target." << endl; )
        }
    }
}

void CRenderTargetZStencil9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
}

void CRenderTargetZStencil9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    if(node->get_id().name == NAME_CURRENT_RENDER_TARGET)
    {    
        GPUProxy* gpu = GPUProxy::get_instance();
        GPURegData data;
        StateDataNode* s_device = node->get_parent();

        // First check if the new render target is the active one.  Check for false new render target activation.
        IDirect3DSurface9* newActiveRT;
        node->read_data(&newActiveRT);
        
        if (newActiveRT != currentRenderTarget)
        {
            //  Check if the new render target is the implicit (default) render target.
            IDirect3DSurface9* implicitRT;
            s_device->get_child(StateId(NAME_IMPLICIT_RENDER_TARGET))->read_data(&implicitRT);
            
            if(newActiveRT == implicitRT)
            {
                // Ensures any pending drawing is done on previous RT
                gpu->sendCommand(GPU_FLUSHCOLOR);
                
                //  Restore the color block state data from the save area.
                gpu->sendCommand(GPU_RESTORE_COLOR_STATE);

                // Restore the implicit front and back buffers
                if(!swapped_front_back)
                {
                    gpu->writeGPUAddrRegister(GPU_FRONTBUFFER_ADDR, 0, implicitFrontBufferAddr);
                    gpu->writeGPUAddrRegister(GPU_BACKBUFFER_ADDR, 0, implicitBackBufferAddr);
                }
                else
                {
                    gpu->writeGPUAddrRegister(GPU_FRONTBUFFER_ADDR, 0, implicitBackBufferAddr);
                    gpu->writeGPUAddrRegister(GPU_BACKBUFFER_ADDR, 0, implicitFrontBufferAddr);
                }
                
                
                //  Enable compression for the implicit render target.
                data.booleanVal = true;
                gpu->writeGPURegister(GPU_COLOR_COMPRESSION, data);
            }
            else
            {
                // Ensures any pending drawing is done on previous RT
                gpu->sendCommand(GPU_FLUSHCOLOR);
                
                //  NOTE: Saving the color block state data is only allowed/required for the implicit render target
                //  as compression is disabled for all other render targets in the current implementation.
                
                if (currentRenderTarget == implicitRT)
                {
                    //  Save the color block state date to the save area.
                    gpu->sendCommand(GPU_SAVE_COLOR_STATE);
                }

                // Get memory associated with the render target
                u32bit md;
                ResourceAssignationTable* surf_mem = AssignationsRegistry::get_table(NAME_SURFACE_MEMORY);
                ResourceId res = surf_mem->get_assigned_to(UsageId(NAME_SURFACE, UsageIndex(newActiveRT)));
                md = res.index;
                
                // Set as backbuffer
                gpu->writeGPUAddrRegister(GPU_BACKBUFFER_ADDR, 0, md);
                
                //  Disable compression for render targets that are not the implicit one.
                data.booleanVal = false;
                gpu->writeGPURegister(GPU_COLOR_COMPRESSION, data);
            }
            
            //  Set new RT as current RT.
            currentRenderTarget = newActiveRT;
            
            // Get dimensions of rendertarget
            StateDataNode* s_rt = s_device->get_child(StateId(NAME_SURFACE_9, StateIndex(newActiveRT)));
            UINT rt_width;
            UINT rt_height;
            s_rt->get_child(NAME_WIDTH)->read_data(&rt_width);
            s_rt->get_child(NAME_HEIGHT)->read_data(&rt_height);

            // Set new resolution
            data.uintVal = rt_width;
            gpu->writeGPURegister(GPU_DISPLAY_X_RES, data);
            data.uintVal = rt_height;
            gpu->writeGPURegister(GPU_DISPLAY_Y_RES, data);

            // Set new viewport
            ///@todo Consider moving this to CRasterization
            data.uintVal = rt_width;
            gpu->writeGPURegister(GPU_VIEWPORT_WIDTH, data);
            data.uintVal = rt_height;
            gpu->writeGPURegister(GPU_VIEWPORT_HEIGHT, data);
            data.intVal = 0;
            gpu->writeGPURegister(GPU_VIEWPORT_INI_X, data);
            data.intVal = 0;
            gpu->writeGPURegister(GPU_VIEWPORT_INI_Y, data);
            
            if (newActiveRT != implicitRT)
            {
                // This cleans block's state bits in color caches
                gpu->sendCommand(GPU_CLEARCOLORBUFFER);
            }
        }
    }
    else if (node->get_id().name == NAME_CURRENT_DEPTHSTENCIL)                                     
    {
        GPUProxy* gpu = GPUProxy::get_instance();
        GPURegData data;
        StateDataNode* s_device = node->get_parent();

        // First check if the new ZStencil buffer is the current one.
        IDirect3DSurface9* newZStencil;
        node->read_data(&newZStencil);
        
        if (newZStencil != currentZStencilBuffer)
        {
            //  Check if the new ZStencil buffer is the implicit (default) z and stencil buffer.
            IDirect3DSurface9* implicitZStencil;
            s_device->get_child(StateId(NAME_IMPLICIT_DEPTHSTENCIL))->read_data(&implicitZStencil);

            if(newZStencil == implicitZStencil)
            {
                // Ensures any pending drawing is done on previous RT
                gpu->sendCommand(GPU_FLUSHZSTENCIL);

                // Restore the implicit z stencil buffer
                gpu->writeGPUAddrRegister(GPU_ZSTENCILBUFFER_ADDR, 0, implicitZStencilBufferAddr);
                
                //data.booleanVal = true;
                //gpu->writeGPURegister(GPU_ZSTENCIL_COMPRESSION, data);
                
            }
            else
            {
                //  Check if the new zstencil buffer is NULL (disable z and stencill buffer).
                if (newZStencil != NULL)
                {
                    // Ensures any pending drawing is done on previous RT
                    gpu->sendCommand(GPU_FLUSHZSTENCIL);
                    
                    // Get memory associated with rendertarget
                    u32bit md;
                    ResourceAssignationTable* surf_mem = AssignationsRegistry::get_table(NAME_SURFACE_MEMORY);
                    ResourceId res = surf_mem->get_assigned_to(UsageId(NAME_SURFACE, UsageIndex(newZStencil)));
                    md = res.index;
                    
                    // Set as backbuffer
                    gpu->writeGPUAddrRegister(GPU_ZSTENCILBUFFER_ADDR, 0, md);
                    
                    //  Disable compression
                    //data.booleanVal = false;
                    //gpu->writeGPURegister(GPU_ZSTENCIL_COMPRESSION, data);
                }
            }
            
            //  Set new z and stencil buffer as current.
            currentZStencilBuffer = newZStencil;
        }
    }
}

void CRenderTargetZStencil9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9)
    {
        node->get_child(StateId(NAME_COMMANDS))->add_controller(this);
        // NOTE disable to put all batches on default render target
        node->get_child(StateId(NAME_CURRENT_RENDER_TARGET))->add_controller(this);
        node->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->add_controller(this);

        // Init color/z/depth buffers
        GPUProxy* gpu = GPUProxy::get_instance();
        UINT w,h;
        node->get_child(StateId(NAME_BACKBUFFER_WIDTH))->read_data(&w);
        node->get_child(StateId(NAME_BACKBUFFER_HEIGHT))->read_data(&h);
        gpu->setResolution(w,h);
        D3D_DEBUG( cout << "CRENDERTARGETZSTENCIL9: Resolution is " << w << "x" << h << endl; )
        gpu->initBuffers(&implicitFrontBufferAddr, &implicitBackBufferAddr, &implicitZStencilBufferAddr);
        
        //  Store pointers to the current frame buffers.
        node->get_child(StateId(NAME_IMPLICIT_RENDER_TARGET))->read_data(&currentRenderTarget);
        node->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->read_data(&currentZStencilBuffer);
        
        swapped_front_back = false;
        
        //  Create a buffer for saving the color block state data on render target switch.
        colorBlockStateBufferMD = gpu->obtainMemory(512 * 1024);
        
        //  Set the gpu register that points to the color state buffer save area.
        gpu->writeGPUAddrRegister(GPU_COLOR_STATE_BUFFER_MEM_ADDR, 0, colorBlockStateBufferMD);
    }
}

void CRenderTargetZStencil9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9)
    {
        // NOTE disable to put all batches on default render target
        node->get_child(StateId(NAME_CURRENT_DEPTHSTENCIL))->remove_controller(this);
        node->get_child(StateId(NAME_CURRENT_RENDER_TARGET))->remove_controller(this);
        node->get_child(StateId(NAME_COMMANDS))->remove_controller(this);
    }
}

void CRenderTargetZStencil9::clear_using_rectangle(DWORD flags, D3DCOLOR color, float z, DWORD stencil) {

    /**@todo Create vertex buffer */
    /**@todo Store configuration of stream to be used */
    /**@todo Configure stream */
    setup_clear_vertices();

    /**@todo Store vsh and psh */
    /**@todo Create vsh and psh */
    /**@todo Store assigned input/output/constants registers that will be used */
    /**@todo load vsh and psh */
    setup_clear_shaders(color, (flags & D3DCLEAR_ZBUFFER) ? z : 0.0f);

    /**@todo Store culling state. */
    /**@todo Disable culling. */
    /**@todo Store ztest, stencil test, alpha blending, color mask depth mask */
    /**@todo Disable ztest, stencil test, alpha blending, enable color mask, disable depth mask */
    setup_clear_pipeline(flags, stencil);

    /**@todo Send draw command */

    GPUProxy::get_instance()->sendCommand(GPU_DRAW);

    /**@todo Restore culling state. */
    /**@todo Restore ztest, stencil test, alpha blending, color mask depth mask */
    restore_clear_pipeline();

    /**@todo Unassign input/output/constants used */
    /**@todo Restore assigned input/output/constants registers that will be used */
    /**@todo Restore vsh and psh */
    restore_clear_shaders();

    /**@todo Delete vertex buffer */
    /**@todo Restore configuration of stream used */
    restore_clear_vertices();

}

void CRenderTargetZStencil9::setup_clear_shaders(DWORD color, float z) {

    GPUProxy* gpu = GPUProxy::get_instance();
    GPURegData data;

    // Save values of registers to be modified
    gpu->readGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, 0, &stored_vertex_attribute_map_0);
    gpu->readGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, 0, &stored_vertex_output_attribute_0);
    gpu->readGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, 1, &stored_vertex_output_attribute_1);

    // Configure vs input/output
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, 0, data);
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, 0, data);
    gpu->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, 1, data);

    D3D_DEBUG( cout << "CRENDERTARGET9: Creating vertex shader to clear using a quad. " << endl; )

    //  Vertex shader for clear:
    //
    //    mov o1, c0;       copy clear color to output color
    //    mov r0, i0;       copy input position to temporal
    //    mov r0.z, c1;     assign clear depth to temporal
    //    mov o0, r0;       assign temporal to output position
    //    end
    //
    
    // Create vertex shader
    ShaderInstructionBuilder builder;
    Operand op;
    Result r;
    list<ShaderInstruction*> instructions;
    // Move color constant to output
    instructions.push_back(generate_mov(GPURegisterId(1, OUT), GPURegisterId(0, PARAM)));
    // Move position to a temp
    instructions.push_back(generate_mov(GPURegisterId(0, TEMP), GPURegisterId(0, IN)));
    // Change z taking it from constant
    builder.resetParameters();
    op.registerId = GPURegisterId(1, PARAM);
    r.registerId = GPURegisterId(0, TEMP);
    r.maskMode = NNZN;
    builder.setOperand(0, op);
    builder.setResult(r);
    builder.setOpcode(MOV);
    instructions.push_back(builder.buildInstruction());
    // Move final position to output
    instructions.push_back(generate_mov(GPURegisterId(0, OUT), GPURegisterId(0, TEMP)));
    builder.resetParameters();
    builder.setOpcode(gpu3d::END);
    instructions.push_back(builder.buildInstruction());
    list<ShaderInstruction*> ending = generate_ending_nops();
    instructions.insert(instructions.end(), ending.begin(), ending.end());

    // Create a buffer
    clear_vsh_size = instructions.size() * 16;
    u8bit *memory = new u8bit[clear_vsh_size];


    // Copy binary to buffer and print assembly
    char line[80];
    list<ShaderInstruction *>::iterator it_i;
    u32bit i = 0;
    for(it_i = instructions.begin(); it_i != instructions.end(); it_i ++) {
        (*it_i)->getCode(&memory[i * 16]);
        (*it_i)->disassemble(line);
        D3D_DEBUG( cout << "CRENDERTARGET9: " << line << endl; )
        delete (*it_i);
        i ++;
    }

    // Copy buffer to gpu memory
    clear_vsh_md = gpu->obtainMemory(clear_vsh_size);
    gpu->writeMemory(clear_vsh_md, memory, clear_vsh_size );
    delete [] memory;

    // Save registers values to be modified
    gpu->readGPUAddrRegister(GPU_VERTEX_PROGRAM, 0, &stored_vertex_program, &stored_vertex_program_offset);
    gpu->readGPURegister(GPU_VERTEX_PROGRAM_PC, &stored_vertex_program_pc);
    gpu->readGPURegister(GPU_VERTEX_PROGRAM_SIZE, &stored_vertex_program_size);

    // Setup vertex shader on gpu and load it
    gpu->writeGPUAddrRegister(GPU_VERTEX_PROGRAM, 0, clear_vsh_md);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_PC, data);
    data.uintVal = clear_vsh_size;
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_SIZE, data);
    gpu->sendCommand(GPU_LOAD_VERTEX_PROGRAM);

    // Save registers values to be modified
    gpu->readGPURegister(GPU_VERTEX_CONSTANT, 0, &stored_vertex_constant_0);
    gpu->readGPURegister(GPU_VERTEX_CONSTANT, 1, &stored_vertex_constant_1);

    // Set the constant register used value to clear color
    QuadFloat qf_value;
    d3dcolor2quadfloat(color, &qf_value);
    for(u32bit i = 0; i < 4; i++)
        data.qfVal[i] = qf_value[i];
    gpu->writeGPURegister(GPU_VERTEX_CONSTANT, 0, data);
    for(u32bit i = 0; i < 4; i++)
        data.qfVal[i] = 0.0f;
    data.qfVal[2] = z;
    gpu->writeGPURegister(GPU_VERTEX_CONSTANT, 1, data);

    // Save registers values to be modified
    gpu->readGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, 1, &stored_fragment_input_attributes_1);
    gpu->readGPURegister(GPU_MODIFY_FRAGMENT_DEPTH, &stored_modify_fragment_depth);

    // Configure ps input/output
    data.booleanVal = true;
    gpu->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, 1, data);
    data.booleanVal = false;
    gpu->writeGPURegister(GPU_MODIFY_FRAGMENT_DEPTH, data);



    D3D_DEBUG( cout << "CRENDERTARGET9: Creating pixel shader to clear using a quad. " << endl; )


    //  Pixel shader for clear
    //
    //     mov o1, i1;      copy input color to output color
    //     end
    //
    //
    
    instructions.clear();
    ending.clear();
    instructions.push_back(generate_mov(GPURegisterId(1, OUT), GPURegisterId(1, IN)));
    builder.resetParameters();
    builder.setOpcode(gpu3d::END);
    instructions.push_back(builder.buildInstruction());
    ending = generate_ending_nops();
    instructions.insert(instructions.end(), ending.begin(), ending.end());

    // Create a buffer
    clear_psh_size = instructions.size() * 16;
    memory = new u8bit[clear_psh_size];

    // Copy binary to buffer and print assembly
    i = 0;
    for(it_i = instructions.begin(); it_i != instructions.end(); it_i ++) {
        (*it_i)->getCode(&memory[i * 16]);
        (*it_i)->disassemble(line);
        D3D_DEBUG( cout << "CRENDERTARGET9: " << line << endl; )
        delete (*it_i);
        i ++;
    }


    // Copy buffer to gpu memory
    gpu = GPUProxy::get_instance();
    clear_psh_md = gpu->obtainMemory(clear_psh_size );
    gpu->writeMemory(clear_psh_md, memory, clear_psh_size );
    delete [] memory;

    // Save registers values to be modified
    gpu->readGPUAddrRegister(GPU_FRAGMENT_PROGRAM, 0, &stored_fragment_program, &stored_fragment_program_offset);
    gpu->readGPURegister(GPU_FRAGMENT_PROGRAM_PC, &stored_fragment_program_pc);
    gpu->readGPURegister(GPU_FRAGMENT_PROGRAM_SIZE, &stored_fragment_program_size);

    // Setup pixel shader on gpu and load it
    gpu->writeGPUAddrRegister(GPU_FRAGMENT_PROGRAM, 0, clear_psh_md);
    data.uintVal = 0;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_PC, data);
    data.uintVal = clear_psh_size;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_SIZE, data);
    gpu->sendCommand(GPU_LOAD_FRAGMENT_PROGRAM);
}

void CRenderTargetZStencil9::restore_clear_shaders() {
    GPUProxy* gpu = GPUProxy::get_instance();

    gpu->releaseMemory(clear_vsh_md);
    gpu->releaseMemory(clear_psh_md);

    gpu->writeGPURegister(GPU_VERTEX_ATTRIBUTE_MAP, 0, stored_vertex_attribute_map_0);
    gpu->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, 0, stored_vertex_output_attribute_0);
    gpu->writeGPURegister(GPU_VERTEX_OUTPUT_ATTRIBUTE, 1, stored_vertex_output_attribute_1);

    if(stored_vertex_program != 0) {
        gpu->writeGPUAddrRegister(GPU_VERTEX_PROGRAM, 0, stored_vertex_program, stored_vertex_program_offset);
    }
    else {
        GPURegData data;
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_VERTEX_PROGRAM, data);
    }

    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_PC, stored_vertex_program_pc);
    gpu->writeGPURegister(GPU_VERTEX_PROGRAM_SIZE, stored_vertex_program_size);


    gpu->writeGPURegister(GPU_VERTEX_CONSTANT, 0, stored_vertex_constant_0);
    gpu->writeGPURegister(GPU_VERTEX_CONSTANT, 1, stored_vertex_constant_1);

    gpu->writeGPURegister(GPU_FRAGMENT_INPUT_ATTRIBUTES, 1, stored_fragment_input_attributes_1);
    gpu->writeGPURegister(GPU_MODIFY_FRAGMENT_DEPTH, stored_modify_fragment_depth);

    if(stored_fragment_program != 0) {
        gpu->writeGPUAddrRegister(GPU_FRAGMENT_PROGRAM, 0, stored_fragment_program, stored_fragment_program_offset);
    }
    else {
        GPURegData data;
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM, data);
    }

    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_PC, stored_fragment_program_pc);
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_SIZE, stored_fragment_program_size);

    if(stored_vertex_program != 0) {
        gpu->sendCommand(GPU_LOAD_VERTEX_PROGRAM);
    }

    if(stored_fragment_program != 0) {
        gpu->sendCommand(GPU_LOAD_FRAGMENT_PROGRAM);
    }

}

void CRenderTargetZStencil9::setup_clear_vertices() {
    GPUProxy* gpu = GPUProxy::get_instance();

    /**@todo Create vertex buffer */
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 0.0f,
        1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f,
        -1.0f, 1.0f, 0.0f
        };
    clear_vb = gpu->obtainMemory(sizeof(float) * 18);
    gpu->writeMemory(clear_vb, (u8bit*)(vertices), sizeof(float) * 18);

    // Store register values to be modified
    gpu->readGPURegister(GPU_STREAM_ELEMENTS, 0, &stored_stream_elements_0);
    gpu->readGPURegister(GPU_STREAM_DATA, 0, &stored_stream_data_0);
    gpu->readGPUAddrRegister(GPU_STREAM_ADDRESS, 0, &stored_stream_address_0, &stored_stream_address_offset_0);
    gpu->readGPURegister(GPU_STREAM_STRIDE, 0, &stored_stream_stride_0);
    gpu->readGPURegister(GPU_STREAM_START, &stored_stream_start);
    gpu->readGPURegister(GPU_STREAM_COUNT, &stored_stream_count);
    gpu->readGPURegister(GPU_INDEX_MODE, &stored_index_mode);
    gpu->readGPURegister(GPU_PRIMITIVE, &stored_primitive);

    /**@todo Configure streams */
    GPURegData data;
    data.uintVal = 3;
    gpu->writeGPURegister(GPU_STREAM_ELEMENTS, 0, data);
    data.streamData = SD_FLOAT;
    gpu->writeGPURegister(GPU_STREAM_DATA, 0, data);
    gpu->writeGPUAddrRegister(GPU_STREAM_ADDRESS, 0, clear_vb, 0);
    data.uintVal = sizeof(float) * 3;
    gpu->writeGPURegister(GPU_STREAM_STRIDE, 0, data);

    data.uintVal = 0;
    gpu->writeGPURegister(GPU_STREAM_START, data);
    data.uintVal = 6;
    gpu->writeGPURegister(GPU_STREAM_COUNT, data);
    data.booleanVal = false;
    gpu->writeGPURegister(GPU_INDEX_MODE, data);
    data.primitive = TRIANGLE;
    gpu->writeGPURegister(GPU_PRIMITIVE, data);


}

void CRenderTargetZStencil9::restore_clear_vertices() {
    GPUProxy* gpu = GPUProxy::get_instance();

    gpu->releaseMemory(clear_vb);
    gpu->writeGPURegister(GPU_STREAM_ELEMENTS, 0, stored_stream_elements_0);
    gpu->writeGPURegister(GPU_STREAM_DATA, 0, stored_stream_data_0);

    if(stored_stream_address_0 != 0) {
        gpu->writeGPUAddrRegister(GPU_STREAM_ADDRESS, 0, stored_stream_address_0, stored_stream_address_offset_0);
    }
    else {
        GPURegData data;
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_STREAM_ADDRESS, 0, data);
    }

    gpu->writeGPURegister(GPU_STREAM_STRIDE, 0, stored_stream_stride_0);
    gpu->writeGPURegister(GPU_STREAM_START, stored_stream_start);
    gpu->writeGPURegister(GPU_STREAM_COUNT, stored_stream_count);
    gpu->writeGPURegister(GPU_INDEX_MODE, stored_index_mode);
    gpu->writeGPURegister(GPU_PRIMITIVE, stored_primitive);
}


 void CRenderTargetZStencil9::setup_clear_pipeline(DWORD flags, DWORD stencil) {
    GPUProxy* gpu = GPUProxy::get_instance();

    // Store register values to be modified
    gpu->readGPURegister(GPU_CULLING, &stored_culling);
    gpu->readGPURegister(GPU_DEPTH_MASK, &stored_depth_mask);
    gpu->readGPURegister(GPU_DEPTH_TEST, &stored_depth_test);
    gpu->readGPURegister(GPU_DEPTH_FUNCTION, &stored_depth_function);
    gpu->readGPURegister(GPU_EARLYZ, &stored_earlyz);
    gpu->readGPURegister(GPU_HIERARCHICALZ, &stored_hierarchicalz);

    GPURegData data;
    data.culling = gpu3d::NONE;
    gpu->writeGPURegister(GPU_CULLING, data);

    if(flags & D3DCLEAR_ZBUFFER) {
        data.booleanVal = true;
        gpu->writeGPURegister(GPU_DEPTH_MASK, data);
        data.booleanVal = true;
        gpu->writeGPURegister(GPU_DEPTH_TEST, data);

        data.compare = GPU_ALWAYS;
        gpu->writeGPURegister(GPU_DEPTH_FUNCTION, data);
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_EARLYZ, data);
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_HIERARCHICALZ, data);
    }
    else {
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_DEPTH_MASK, data);
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_DEPTH_TEST, data);
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_EARLYZ, data);
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_HIERARCHICALZ, data);
    }

    // Store register values to be modified
    gpu->readGPURegister(GPU_STENCIL_UPDATE_MASK, &stored_update_mask);
    gpu->readGPURegister(GPU_STENCIL_TEST, &stored_stencil_test);
    gpu->readGPURegister(GPU_STENCIL_FUNCTION, &stored_stencil_function);
    gpu->readGPURegister(GPU_STENCIL_REFERENCE, &stored_stencil_reference);


    if(flags & D3DCLEAR_STENCIL) {
        data.booleanVal = true;
        gpu->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data); // Save
        data.booleanVal = true;
        gpu->writeGPURegister(GPU_STENCIL_TEST, data); // Save
        data.compare = GPU_ALWAYS;
        gpu->writeGPURegister(GPU_STENCIL_FUNCTION, data); // Save
        data.uintVal = stencil;
        gpu->writeGPURegister(GPU_STENCIL_REFERENCE, data); // Save
    }
    else {
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data); // Save
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_STENCIL_TEST, data); // Save
        data.compare = GPU_NEVER;
        gpu->writeGPURegister(GPU_STENCIL_FUNCTION, data); // Save
        data.uintVal = 0;
        gpu->writeGPURegister(GPU_STENCIL_REFERENCE, data); // Save
    }

    // Store register values to be modified
    gpu->readGPURegister(GPU_COLOR_BLEND, &stored_color_blend);
    gpu->readGPURegister(GPU_COLOR_MASK_R, &stored_color_mask_r);
    gpu->readGPURegister(GPU_COLOR_MASK_G, &stored_color_mask_g);
    gpu->readGPURegister(GPU_COLOR_MASK_B, &stored_color_mask_b);
    gpu->readGPURegister(GPU_COLOR_MASK_A, &stored_color_mask_a);

    data.booleanVal = false;
    gpu->writeGPURegister(GPU_COLOR_BLEND, data);

    if(flags & D3DCLEAR_TARGET) {
        data.booleanVal = true;
        gpu->writeGPURegister(GPU_COLOR_MASK_R, data);
        gpu->writeGPURegister(GPU_COLOR_MASK_G, data);
        gpu->writeGPURegister(GPU_COLOR_MASK_B, data);
        gpu->writeGPURegister(GPU_COLOR_MASK_A, data);
    }
    else {
        data.booleanVal = false;
        gpu->writeGPURegister(GPU_COLOR_MASK_R, data);
        gpu->writeGPURegister(GPU_COLOR_MASK_G, data);
        gpu->writeGPURegister(GPU_COLOR_MASK_B, data);
        gpu->writeGPURegister(GPU_COLOR_MASK_A, data);
    }

 }

 void CRenderTargetZStencil9::restore_clear_pipeline() {
    GPUProxy* gpu = GPUProxy::get_instance();

    gpu->writeGPURegister(GPU_CULLING, stored_culling);
    gpu->writeGPURegister(GPU_DEPTH_MASK, stored_depth_mask);
    gpu->writeGPURegister(GPU_DEPTH_TEST, stored_depth_test);
    gpu->writeGPURegister(GPU_DEPTH_FUNCTION, stored_depth_function);
    gpu->writeGPURegister(GPU_EARLYZ, stored_earlyz);
    gpu->writeGPURegister(GPU_HIERARCHICALZ, stored_hierarchicalz);

    gpu->writeGPURegister(GPU_STENCIL_UPDATE_MASK, stored_update_mask);
    gpu->writeGPURegister(GPU_STENCIL_TEST, stored_stencil_test);
    gpu->writeGPURegister(GPU_STENCIL_FUNCTION, stored_stencil_function);
    gpu->writeGPURegister(GPU_STENCIL_REFERENCE, stored_stencil_reference);

    gpu->writeGPURegister(GPU_COLOR_BLEND, stored_color_blend);
    gpu->writeGPURegister(GPU_COLOR_MASK_R, stored_color_mask_r);
    gpu->writeGPURegister(GPU_COLOR_MASK_G, stored_color_mask_g);
    gpu->writeGPURegister(GPU_COLOR_MASK_B, stored_color_mask_b);
    gpu->writeGPURegister(GPU_COLOR_MASK_A, stored_color_mask_a);
 }


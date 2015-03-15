
#include "Common.h"
#include "ShaderTranslator/ShaderTranslator.h"
#include "CShaders_9.h"

ShaderSetupData::ShaderSetupData() {}
ShaderSetupData::ShaderSetupData(NativeShader* _translated, u32bit _gpu_memory):
    translated(_translated), gpu_memory(_gpu_memory) {}

void CShaders9::on_add_node_child(StateDataNode* parent, StateDataNode* child) {
	/// Vertex shaders are translated inmediatly
    if(child->get_id().name == NAME_VERTEX_SHADER_9) {
        // Translate vertex shader
        D3D_DEBUG( cout << "CSHADERS9: Translating vertex shader:" << endl; )
        StateDataNode* s_code = child->get_child(StateId(NAME_CODE));
        void* code = s_code->map_data(READ_ACCESS);
        NativeShader*translated = ShaderTranslator::get_instance().translate((DWORD*)code);
        s_code->unmap_data();
        D3D_DEBUG( cout << translated->debug_assembler << endl; )
        // Reserve memory for vertex shader & copy translated code
        D3D_DEBUG( cout << "CSHADERS9: Reserving gpu memory" << endl; )
        GPUProxy* gpu = GPUProxy::get_instance();
        u32bit md = gpu->obtainMemory(translated->lenght);
        gpu->writeMemory(md, translated->bytecode, translated->lenght);
        // Register data for shader setup
        ShaderSetupData setup_data(translated, md);
        IDirect3DVertexShader9* i_vsh = reinterpret_cast<IDirect3DVertexShader9*>(child->get_id().index);
        vs_setup_data[i_vsh] = setup_data;
    }
}

void CShaders9::on_remove_node_child(StateDataNode* parent, StateDataNode* child) {
    if(child->get_id().name == NAME_VERTEX_SHADER_9) {
        D3D_DEBUG( cout << "CSHADERS9: Releasing gpu memory" << endl; )
        // Obtain setup data
        IDirect3DVertexShader9* i_vsh = reinterpret_cast<IDirect3DVertexShader9*>(child->get_id().index);
        // Delete memory for vertex shader and translated shader, remove setup data
        GPUProxy::get_instance()->releaseMemory(vs_setup_data[i_vsh].gpu_memory);
        delete vs_setup_data[i_vsh].translated;
        vs_setup_data.erase(i_vsh);
    }
    else if(child->get_id().name == NAME_PIXEL_SHADER_9) {
        D3D_DEBUG( cout << "CSHADERS9: Releasing gpu memory for pixel shader" << endl; )

        // Iterate through all setup data generated for this shader and release gpu memory
        IDirect3DPixelShader9* i_psh = reinterpret_cast<IDirect3DPixelShader9*>(child->get_id().index);
        // map< PixelShaderSetupId , ShaderSetupData, PixelShaderSetupId > :: iterator it_setup;
        map< PixelShaderSetupId , ShaderSetupData > :: iterator it_setup;


        list< PixelShaderSetupId > setups_to_delete;
        for(it_setup = ps_setup_data.begin(); it_setup != ps_setup_data.end(); it_setup ++) {
        	if((*it_setup).first.d3d_shader == i_psh) {
        		GPUProxy::get_instance()->releaseMemory((*it_setup).second.gpu_memory);
        		setups_to_delete.push_back((*it_setup).first);
        	}
        }
        // Delete setup data itself
        list< PixelShaderSetupId >::iterator it_delete;
        for(it_delete = setups_to_delete.begin(); it_delete != setups_to_delete.end(); it_delete ++) {
        	ps_setup_data.erase(*it_delete);
        }
    }
}

void CShaders9::on_write_node_data(StateDataNode* node, size_t size, unsigned int offset) {
    if(node->get_id().name == NAME_CURRENT_VERTEX_SHADER) {
        // Unassign input registers from previous shader
        ResourceAssignationTable* a_input;
        a_input = AssignationsRegistry::get_table(NAME_VS_INPUT);
        set<UsageId>::iterator it_i;
        for(it_i = assigned_vs_input.begin(); it_i != assigned_vs_input.end(); it_i ++) {
            a_input->unassign(*it_i);
        }
        assigned_vs_input.clear();

        // Unassign constants registers from previous shader
        ResourceAssignationTable* a_constants;
        a_constants = AssignationsRegistry::get_table(NAME_VS_CONSTANTS);
        set<UsageId>::iterator it_c;
        for(it_c = assigned_vs_constants.begin(); it_c != assigned_vs_constants.end(); it_c ++) {
            a_constants->unassign(*it_c);
        }
        assigned_vs_constants.clear();

        // Unassign output
        ResourceAssignationTable* a_output;
        a_output = AssignationsRegistry::get_table(NAME_VS_OUTPUT);
        set<UsageId>::iterator it_o;
        for(it_o = assigned_vs_output.begin(); it_o != assigned_vs_output.end(); it_o ++) {
            a_output->unassign(*it_o);
        }
        assigned_vs_output.clear();
        // Setup vertex shader if needed
        IDirect3DVertexShader9* i_vsh;
        node->read_data(&i_vsh);
        if(i_vsh != 0) {
            ShaderSetupData setup_data = vs_setup_data[i_vsh];
            // Assign gpu input registers
            list<InputRegisterDeclaration> &in_dec = setup_data.translated->declaration.input_registers;
            list<InputRegisterDeclaration>::iterator it_i;
            for(it_i = in_dec.begin(); it_i != in_dec.end(); it_i ++) {
                InputRegisterDeclaration &in = *it_i;
                UsageId usage(usage2name(in.usage), UsageIndex(in.usage_index));
                ResourceId resource(NAME_REGISTER, ResourceIndex(in.native_register));
                D3D_DEBUG( cout << "CSHADERS9: Assigning vs input register " << (int)in.native_register; )
                //D3D_DEBUG( cout << " to usage " << usage2str(in.usage) << " " << (int)in.usage_index << endl; )
                a_input->assign(usage, resource);
                assigned_vs_input.insert(usage);
            }
            // Assign gpu constants registers
            list<ConstRegisterDeclaration> &const_dec = setup_data.translated->declaration.constant_registers;
            list<ConstRegisterDeclaration>::iterator it_c;
            for(it_c = const_dec.begin(); it_c != const_dec.end(); it_c ++) {
                ConstRegisterDeclaration &con = *it_c;
                UsageId usage(NAME_VS_CONSTANT, UsageIndex(con.d3d_register));
                ResourceId resource(NAME_REGISTER, ResourceIndex(con.native_register));
                D3D_DEBUG( cout << "CSHADERS9: Assigning vs constant register " << (int)con.native_register; )
                D3D_DEBUG( cout << " to usage " << "VS_CONSTANT " << (int)con.d3d_register << endl; )
                a_constants->assign(usage, resource);
                assigned_vs_constants.insert(usage);
                // update value if constant is defined
                if(con.defined) {
                    D3D_DEBUG( cout << "CSHADERS9: Initializing to"; )
                    GPURegData data;
                    ConstValue value = con.value;
                    data.qfVal[0] = value.value.floatValue.x;
                    data.qfVal[1] = value.value.floatValue.y;
                    data.qfVal[2] = value.value.floatValue.z;
                    data.qfVal[3] = value.value.floatValue.w;
                    D3D_DEBUG( cout << "(" << value.value.floatValue.x << ", " << value.value.floatValue.y; )
                    D3D_DEBUG( cout << ", " << value.value.floatValue.z << ", " << value.value.floatValue.w << ")" << endl; )
                    GPUProxy* gpu = GPUProxy::get_instance();
                    gpu->writeGPURegister(GPU_VERTEX_CONSTANT, u32bit(resource.index), data);
                }
            }
            // Assign gpu output registers
            list<OutputRegisterDeclaration> &out_dec = setup_data.translated->declaration.output_registers;
            list<OutputRegisterDeclaration>::iterator it_o;
            for(it_o = out_dec.begin(); it_o != out_dec.end(); it_o ++) {
                OutputRegisterDeclaration &out = *it_o;
                UsageId usage(usage2name(out.usage), UsageIndex(out.usage_index));
                ResourceId resource(NAME_REGISTER, UsageIndex(out.native_register));
                D3D_DEBUG( cout << "CSHADERS9: Assigning vs output register " << (int)out.native_register; )
                //D3D_DEBUG( cout << " to usage " << usage2str(out.usage) << " " << (int)out.usage_index << endl; )
                a_output->assign(usage, resource);
                assigned_vs_output.insert(usage);
            }
            // Load vertex shader
            D3D_DEBUG( cout << "CSHADERS9: Loading vertex shader " << i_vsh << endl; )
            GPUProxy* gpu = GPUProxy::get_instance();
            gpu->writeGPUAddrRegister(GPU_VERTEX_PROGRAM, 0, setup_data.gpu_memory);
            GPURegData data;
            data.uintVal = 0;
            gpu->writeGPURegister(GPU_VERTEX_PROGRAM_PC, data);
            data.uintVal = setup_data.translated->lenght;
            gpu->writeGPURegister(GPU_VERTEX_PROGRAM_SIZE, data);
            gpu->sendCommand(GPU_LOAD_VERTEX_PROGRAM);
        }

    }
    else if((node->get_id().name == NAME_CURRENT_PIXEL_SHADER) ||
    (node->get_id().name == NAME_ALPHA_TEST_ENABLED) ||
    (node->get_id().name == NAME_ALPHA_FUNC)) {
    	update_pixel_shader(node->get_parent());
    }
}

void CShaders9::on_added_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        node->get_child(StateId(NAME_CURRENT_VERTEX_SHADER))->add_controller(this);
        node->get_child(StateId(NAME_CURRENT_PIXEL_SHADER))->add_controller(this);
        node->get_child(StateId(NAME_ALPHA_TEST_ENABLED))->add_controller(this);
        node->get_child(StateId(NAME_ALPHA_FUNC))->add_controller(this);

        /// @note uncomment for debugging
        // load_default_pixel_shader();
    }
}

void CShaders9::on_removed_controller(StateDataNode* node) {
    if(node->get_id().name == NAME_DEVICE_9) {
        /// @note uncomment for debugging
        // unload_default_pixel_shader();
        GPUProxy::get_instance()->releaseMemory(assigned_ps_memory);
        /// @todo unwatch pixel shader
        node->get_child(StateId(NAME_ALPHA_FUNC))->remove_controller(this);
        node->get_child(StateId(NAME_ALPHA_TEST_ENABLED))->remove_controller(this);
        node->get_child(StateId(NAME_CURRENT_PIXEL_SHADER))->remove_controller(this);
        node->get_child(StateId(NAME_CURRENT_VERTEX_SHADER))->remove_controller(this);
    }
}

void CShaders9::load_default_pixel_shader() {
   D3D_DEBUG( cout << "CSHADERS9: Load default pixel shader" << endl; )

    // Assign input registers
    ResourceAssignationTable* a_input;
    a_input = AssignationsRegistry::get_table(NAME_PS_INPUT);
    UsageId usage(usage2name(D3DDECLUSAGE_POSITION), UsageIndex(0));
    ResourceId resource(NAME_REGISTER, ResourceIndex(0));
    D3D_DEBUG( cout << "CSHADERS9: Assigning ps input register 0"; )
    //D3D_DEBUG( cout << " to usage " << usage2str(D3DDECLUSAGE_POSITION) << endl; )
    a_input->assign(usage, resource);
    usage = UsageId(usage2name(D3DDECLUSAGE_TEXCOORD), UsageIndex(0));
    resource = ResourceId(NAME_REGISTER, ResourceIndex(5));
    D3D_DEBUG( cout << "CSHADERS9: Assigning ps input register 5"; )
    //D3D_DEBUG( cout << " to usage " << usage2str(D3DDECLUSAGE_COLOR) << " 0" << endl; )
    a_input->assign(usage, resource);

    ///@todo Assign texture registers

    // Create pixel shader instructions
    list<ShaderInstruction*> instructions;
    ShaderInstructionBuilder builder;

    builder.resetParameters();
    builder.setOpcode(gpu3d::TEX);
    Result r;
    r.registerId = GPURegisterId(1, OUT);
    builder.setResult(r);
    Operand op0;
    op0.registerId = GPURegisterId(5, IN);
    builder.setOperand(0, op0);
    Operand op1;
    /**@note TEXT member of Bank enum does
       not correspond to gpu texture register.*/
    op1.registerId = GPURegisterId(0, static_cast<Bank>(0x06));
    builder.setOperand(1, op1);
    instructions.push_back(builder.buildInstruction());

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
        D3D_DEBUG( cout << "CSHADERS9: " << line << endl; )
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
    data.uintVal = 0x200;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_PC, data);
    data.uintVal = program_size;
    gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_SIZE, data);
    gpu->sendCommand(GPU_LOAD_FRAGMENT_PROGRAM);


}

void CShaders9::unload_default_pixel_shader() {
    // Release pixel shader memory
    GPUProxy::get_instance()->releaseMemory(assigned_ps_memory);

    // Unassign input registers
    ResourceAssignationTable* a_input;
    a_input = AssignationsRegistry::get_table(NAME_PS_INPUT);
    UsageId usage(usage2name(D3DDECLUSAGE_POSITION), UsageIndex(0));
    D3D_DEBUG( cout << "CSHADERS9: Unassigning ps input register 0"; )
    //D3D_DEBUG( cout << " from usage " << usage2str(D3DDECLUSAGE_POSITION) << endl; )
    a_input->unassign(usage);
    usage = UsageId(usage2name(D3DDECLUSAGE_TEXCOORD), UsageIndex(0));
    D3D_DEBUG( cout << "CSHADERS9: Unassigning ps input register 5"; )
    //D3D_DEBUG( cout << " from usage " << usage2str(D3DDECLUSAGE_TEXCOORD) << " 0" << endl; )
    a_input->unassign(usage);

}

void CShaders9::update_pixel_shader(StateDataNode* s_device) {
	// Unassign input registers from previous shader
	ResourceAssignationTable* a_input;
	a_input = AssignationsRegistry::get_table(NAME_PS_INPUT);
	set<UsageId>::iterator it_i;
	for(it_i = assigned_ps_input.begin(); it_i != assigned_ps_input.end(); it_i ++) {
		a_input->unassign(*it_i);
	}
	assigned_ps_input.clear();

	// Unassign constants registers from previous shader
	ResourceAssignationTable* a_constants;
	a_constants = AssignationsRegistry::get_table(NAME_PS_CONSTANTS);
	set<UsageId>::iterator it_c;
	for(it_c = assigned_ps_constants.begin(); it_c != assigned_ps_constants.end(); it_c ++) {
		a_constants->unassign(*it_c);
	}
	assigned_ps_constants.clear();

	// Unassign texture registers from previous shader
	ResourceAssignationTable* a_textures;
	a_textures = AssignationsRegistry::get_table(NAME_PS_TEXTURE);
	set<UsageId>::iterator it_s;
	for(it_s = assigned_ps_textures.begin(); it_s != assigned_ps_textures.end(); it_s ++) {
		a_textures->unassign(*it_s);
	}
	assigned_ps_textures.clear();

	// Setup pixel shader if needed
	IDirect3DPixelShader9* i_psh;
	s_device->get_child(StateId(NAME_CURRENT_PIXEL_SHADER))->read_data(&i_psh);
	if(i_psh != 0) {
		// Get alpha test parameters
		DWORD alpha_test_enabled;
		s_device->get_child(StateId(NAME_ALPHA_TEST_ENABLED))->read_data(&alpha_test_enabled);
		D3DCMPFUNC alpha_func;
		s_device->get_child(StateId(NAME_ALPHA_FUNC))->read_data(&alpha_func);

		// The shader setup is identified by ID3DPixelShader* and alpha test function
		if(!alpha_test_enabled) alpha_func = D3DCMP_ALWAYS;
		PixelShaderSetupId setup_data_id(i_psh, alpha_func);

		// Find shader setup data, generate it if not found
		// map< PixelShaderSetupId , ShaderSetupData, PixelShaderSetupId > :: iterator it_setup;
         map< PixelShaderSetupId , ShaderSetupData > :: iterator it_setup;
		it_setup = ps_setup_data.find(setup_data_id);

		if(it_setup == ps_setup_data.end()) {
			add_pixel_shader_setup_data(s_device, i_psh, alpha_func);
		}
		ShaderSetupData setup_data = ps_setup_data[setup_data_id];

		// Assign gpu input registers
		list<InputRegisterDeclaration> &in_dec = setup_data.translated->declaration.input_registers;
		list<InputRegisterDeclaration>::iterator it_i;
		for(it_i = in_dec.begin(); it_i != in_dec.end(); it_i ++) {
			InputRegisterDeclaration &in = *it_i;
			UsageId usage(usage2name(in.usage), UsageIndex(in.usage_index));
			ResourceId resource(NAME_REGISTER, ResourceIndex(in.native_register));
			D3D_DEBUG( cout << "CSHADERS9: Assigning ps input register " << (int)in.native_register; )
			//D3D_DEBUG( cout << " to usage " << usage2str(in.usage) << " " << (int)in.usage_index << endl; )
			a_input->assign(usage, resource);
			assigned_ps_input.insert(usage);
		}
		// Assign gpu constants registers
		list<ConstRegisterDeclaration> &const_dec = setup_data.translated->declaration.constant_registers;
		list<ConstRegisterDeclaration>::iterator it_c;
		for(it_c = const_dec.begin(); it_c != const_dec.end(); it_c ++) {
			ConstRegisterDeclaration &con = *it_c;
			UsageId usage(NAME_PS_CONSTANT, UsageIndex(con.d3d_register));
			ResourceId resource(NAME_REGISTER, ResourceIndex(con.native_register));
			D3D_DEBUG( cout << "CSHADERS9: Assigning ps constant register " << (int)con.native_register; )
			D3D_DEBUG( cout << " to usage " << "PS_CONSTANT " << (int)con.d3d_register << endl; )
			a_constants->assign(usage, resource);
			assigned_ps_constants.insert(usage);
			// update value if constant is defined
			if(con.defined) {
				D3D_DEBUG( cout << "CSHADERS9: Initializing to"; )
				GPURegData data;
				ConstValue value = con.value;
				data.qfVal[0] = value.value.floatValue.x;
				data.qfVal[1] = value.value.floatValue.y;
				data.qfVal[2] = value.value.floatValue.z;
				data.qfVal[3] = value.value.floatValue.w;
				D3D_DEBUG( cout << "(" << value.value.floatValue.x << ", " << value.value.floatValue.y; )
				D3D_DEBUG( cout << ", " << value.value.floatValue.z << ", " << value.value.floatValue.w << ")" << endl; )
				GPUProxy* gpu = GPUProxy::get_instance();
				gpu->writeGPURegister(GPU_FRAGMENT_CONSTANT, u32bit(resource.index), data);
			}
		}
		// Assign gpu texture registers
		list<SamplerDeclaration> &sampler_dec = setup_data.translated->declaration.sampler_registers;
		list<SamplerDeclaration>::iterator it_s;
		for(it_s = sampler_dec.begin(); it_s != sampler_dec.end(); it_s ++) {
			SamplerDeclaration &samp = *it_s;
			UsageId usage;
			usage.index = UsageIndex(samp.d3d_sampler);
			switch (samp.type) {
				case D3DSTT_UNKNOWN:
					usage.name = NAME_SAMPLER;
					break;
				case D3DSTT_2D:
					usage.name = NAME_SAMPLER_2D;
					break;
				case D3DSTT_CUBE:
					usage.name = NAME_SAMPLER_CUBE;
					break;
				case D3DSTT_VOLUME:
					usage.name = NAME_SAMPLER_VOLUME;
					break;
				default:
					break;
			}
			ResourceId resource(NAME_REGISTER, ResourceIndex(samp.native_sampler));
			D3D_DEBUG( cout << "CSHADERS9: Assigning ps texture register " << (int)samp.native_sampler; )
			D3D_DEBUG( cout << " to D3D sampler " << (int)samp.d3d_sampler << endl; )
			a_textures->assign(usage, resource);
			assigned_ps_textures.insert(usage);
		}

		// Assign constant used for alpha test
		AlphaTestDeclaration alpha_dec = setup_data.translated->declaration.alpha_test;
		if(alpha_dec.comparison != D3DCMP_ALWAYS) {
			D3D_DEBUG( cout << "CSHADERS9: Assigning ps constants " <<
			(int)alpha_dec.alpha_const_ref << " and " <<
			(int)alpha_dec.alpha_const_minus_one << " for alpha test" << endl;)
			UsageId usage(NAME_ALPHATEST_MINUS_ONE);
			ResourceId resource(NAME_REGISTER, ResourceIndex(alpha_dec.alpha_const_minus_one));
			a_constants->assign(usage, resource);
			assigned_ps_constants.insert(usage);

			usage = UsageId(NAME_ALPHATEST_REF);
			resource = ResourceId(NAME_REGISTER, ResourceIndex(alpha_dec.alpha_const_ref));
			a_constants->assign(usage, resource);
			assigned_ps_constants.insert(usage);
		}

		// Load pixel shader
		D3D_DEBUG( cout << "CSHADERS9: Loading pixel shader " << i_psh << endl; )
		GPUProxy* gpu = GPUProxy::get_instance();
		gpu->writeGPUAddrRegister(GPU_FRAGMENT_PROGRAM, 0, setup_data.gpu_memory);
		GPURegData data;
		data.uintVal = 0x200;
		gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_PC, data);
		data.uintVal = setup_data.translated->lenght;
		gpu->writeGPURegister(GPU_FRAGMENT_PROGRAM_SIZE, data);
		gpu->sendCommand(GPU_LOAD_FRAGMENT_PROGRAM);
	}
}

ShaderSetupData CShaders9::add_pixel_shader_setup_data(StateDataNode* s_device, IDirect3DPixelShader9* i_pixel_shader, D3DCMPFUNC alpha_func) {
	StateDataNode* s_psh = s_device->get_child(StateId(NAME_PIXEL_SHADER_9, StateIndex(i_pixel_shader)));
	// Translate vertex shader
	D3D_DEBUG( cout << "CSHADERS9: Translating pixel shader:" << endl; )
	StateDataNode* s_code = s_psh->get_child(StateId(NAME_CODE));
	void* code = s_code->map_data(READ_ACCESS);
	NativeShader*translated = ShaderTranslator::get_instance().translate((DWORD*)code, alpha_func);
	s_code->unmap_data();
	D3D_DEBUG( cout << translated->debug_assembler << endl; )

	// Reserve memory for vertex shader & copy translated code
	D3D_DEBUG( cout << "CSHADERS9: Reserving gpu memory" << endl; )
	GPUProxy* gpu = GPUProxy::get_instance();
	u32bit md = gpu->obtainMemory(translated->lenght);
	gpu->writeMemory(md, translated->bytecode, translated->lenght);
	// Register data for shader setup
	ShaderSetupData setup_data(translated, md);
	PixelShaderSetupId setup_data_id(i_pixel_shader, alpha_func);
	ps_setup_data[setup_data_id] = setup_data;
	return setup_data;
}

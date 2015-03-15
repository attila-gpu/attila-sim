#ifndef CSHADERS_9_H_INCLUDED
#define CSHADERS_9_H_INCLUDED

struct NativeShader;
struct PixelShaderSetupId {
	IDirect3DPixelShader9* d3d_shader;
	D3DCMPFUNC alpha_func;
	PixelShaderSetupId() {
		d3d_shader = 0;
		alpha_func = D3DCMP_ALWAYS;
	}
	PixelShaderSetupId(IDirect3DPixelShader9* _d3d_shader, D3DCMPFUNC _alpha_func) {
		d3d_shader = _d3d_shader;
		alpha_func = _alpha_func;
	}
	bool operator<(const PixelShaderSetupId &b) const {
		if(d3d_shader < b.d3d_shader)
			return true;
		else if(d3d_shader > b.d3d_shader)
			return false;
		else
			return (unsigned int)(alpha_func) < (unsigned int)(b.alpha_func);
	}
	bool operator==(const PixelShaderSetupId &b) const {
		return (d3d_shader == b.d3d_shader) && (alpha_func == b.alpha_func);
	}
};


/** Holds necessary info to load a shader */
struct ShaderSetupData {
    NativeShader* translated;
    u32bit gpu_memory;
    ShaderSetupData();
    ShaderSetupData(NativeShader* _translated, u32bit _gpu_memory);
};


/**
    Reserves gpu memory for vertex/pixel shaders and
    writes the translated shader to it.
    Pixel shaders have extra code to implement Alpha test.
    Keeps GPUState up to date with gpu buffer state.
    Add to the DEVICE node.
*/
class CShaders9 : public StateDataNodeController {
public:
    void on_add_node_child(StateDataNode* parent, StateDataNode* child);
    void on_remove_node_child(StateDataNode* parent, StateDataNode* child);
    void on_write_node_data(StateDataNode* node, size_t size, unsigned int offset);

    void on_added_controller(StateDataNode*);
    void on_removed_controller(StateDataNode*);
private:
    map< IDirect3DVertexShader9* , ShaderSetupData > vs_setup_data;
    map< PixelShaderSetupId , ShaderSetupData > ps_setup_data;

    set< UsageId > assigned_vs_input;
    set< UsageId > assigned_vs_constants;
    set< UsageId > assigned_vs_output;

    set< UsageId > assigned_ps_input;
    set< UsageId > assigned_ps_constants;
    set< UsageId > assigned_ps_textures;

	void update_pixel_shader(StateDataNode* s_device);
	ShaderSetupData add_pixel_shader_setup_data(StateDataNode* s_device, IDirect3DPixelShader9* i_pixel_shader, D3DCMPFUNC alpha_func);


    u32bit assigned_ps_memory;
    void load_default_pixel_shader();
    void unload_default_pixel_shader();

};

#endif

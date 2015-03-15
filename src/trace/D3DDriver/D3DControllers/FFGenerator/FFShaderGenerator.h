#ifndef ____FF_SHADER_GENERATOR
#define ____FF_SHADER_GENERATOR

#include <d3d9.h>
#include <set>
#include <list>
#include <map>

bool fvf_has_texcoord(BYTE index, DWORD fvf);
BYTE fvf_texture_format(BYTE index, DWORD fvf);


struct FFState {
	DWORD fvf;

	D3DMATRIX world;
	D3DMATRIX view;
	D3DMATRIX projection;

    BOOL lighting_enabled;
	D3DMATERIAL9 material;
	D3DCOLORVALUE ambient;
	D3DLIGHT9 lights[8];
	BOOL lights_enabled[8];
	
	D3DCOLORVALUE fog_color;
	D3DFOGMODE fog_mode;
	FLOAT fog_start;
	FLOAT fog_end;
	FLOAT fog_density;
	BOOL  fog_range;

	bool setted_texture;
	D3DTEXTUREOP color_op;

	FFState();
};

enum FFUsage {
	FF_NONE,
	FF_WORLDVIEWPROJ,
	FF_WORLDVIEW,
	FF_WORLDVIEW_IT,
	FF_VIEW_IT,
	FF_WORLD,
	FF_MATERIAL_EMISSIVE,
	FF_MATERIAL_SPECULAR,
	FF_MATERIAL_DIFFUSE,
	FF_MATERIAL_AMBIENT,
	FF_MATERIAL_POWER,
	FF_AMBIENT,
	FF_LIGHT_POSITION,
	FF_LIGHT_DIRECTION,
	FF_LIGHT_AMBIENT,
	FF_LIGHT_DIFFUSE,
	FF_LIGHT_SPECULAR,
	FF_LIGHT_RANGE,
	FF_LIGHT_ATTENUATION,
	FF_LIGHT_SPOT
};

struct FFUsageId {
	FFUsage usage;
	BYTE index;
	bool operator<(const FFUsageId &b) const {
		if(unsigned int(usage) < unsigned int(b.usage))
			return true;
		else if(unsigned int(usage) > unsigned int(b.usage))
			return false;
		else {
			return (index < b.index);
		}
	}

	bool operator==(const FFUsageId &b) const {
		return (usage == b.usage) && (index == b.index);
	}
	FFUsageId(): usage(FF_NONE), index(0) {}
	FFUsageId(FFUsage _usage, BYTE _index = 0): usage(_usage), index(_index) {}
};

struct D3DRegisterId {
	DWORD num;
	D3DSHADER_PARAM_REGISTER_TYPE type;
	bool operator<(const D3DRegisterId &b) const {
		if(unsigned int(type) < unsigned int(b.type))
			return true;
		else if(unsigned int(type) > unsigned int(b.type))
			return false;
		else {
			return (num < b.num);
		}
	}
	bool operator==(const D3DRegisterId &b) const {
		return (num == b.num) && (type == b.type);
	}
	D3DRegisterId(): num(0), type(D3DSPR_TEMP) {}
	D3DRegisterId(DWORD _num, D3DSHADER_PARAM_REGISTER_TYPE _type): num(_num), type(_type) {}
};

struct D3DUsageId {
	DWORD usage;
	BYTE index;
	bool operator<(const D3DUsageId &b) const {
		if(unsigned int(usage) < unsigned int(b.usage))
			return true;
		else if(unsigned int(usage) > unsigned int(b.usage))
			return false;
		else {
			return (index < b.index);
		}
	}
	bool operator==(const D3DUsageId &b) const {
		return (usage == b.usage) && (index == b.index);
	}
	D3DUsageId(): usage(0), index(0) {}
	D3DUsageId(DWORD _usage, BYTE _index = 0): usage(_usage), index(_index) {}
};

template<typename REGISTERID, typename USAGEID = REGISTERID>
class RegisterBank {
private:
	std::set<REGISTERID> available;
	std::map<USAGEID, REGISTERID> reserved_usage;
	std::set<REGISTERID> reserved;
public:
	void insert(REGISTERID reg) { available.insert(reg); }

	void clear() { available.clear(); reserved.clear(); }

	REGISTERID reserve() {
		REGISTERID reserved;
		std::set<REGISTERID>::iterator it_a;
		it_a = available.begin();
		if(it_a == available.end()) 
			throw "no available register";
		else {
			reserved = *it_a;
			available.erase(reserved);
		}
		return reserved;
	}

	REGISTERID reserve_usage(USAGEID usage) {
		// TODO panic if usage already reserved
		REGISTERID reserved = reserve();
		reserved_usage[usage] = reserved;
		return reserved;
	}

	bool is_reserved_usage(USAGEID usage) {
		return (reserved_usage.find(usage) != reserved_usage.end());
	}

	bool is_reserved(REGISTERID reg) {
		return (reserved.find(reg) != reserved.end());
	}


	REGISTERID find_usage(USAGEID usage, bool reserve = false) {
		REGISTERID found_register;
		std::map<USAGEID, REGISTERID>::iterator it_u;
		it_u = reserved_usage.find(usage);
		if(it_u != reserved_usage.end())
			found_register = (*it_u).second;
		else
			if(reserve)
				found_register = reserve_usage(usage);
			else
				throw "usage not found";
		return found_register;
	}

	void release(REGISTERID reg) {
		available.insert(reg);
		reserved.erase(reg);
	}
	void release_usage(USAGEID usage) {
		std::map<USAGEID, REGISTERID>::iterator it_u;
		it_u = reserved_usage.find(usage);
		if(it_u != reserved_usage.end()) {
			REGISTERID released;
			released = (*it_u).second;
			reserved_usage.erase(usage);
			reserved.erase(released);
		}
		else
			panic("RegisterBank", "release_usage", "usage not reserved");
	}

};


void identity_matrix(D3DMATRIX *dest);
void copy_matrix(D3DMATRIX *dest, const D3DMATRIX* source);
void multiply_matrix(D3DMATRIX *dest, const D3DMATRIX* source_a, const D3DMATRIX* source_b);
void transpose_matrix(D3DMATRIX *dest, const D3DMATRIX *source);
void invert_matrix(D3DMATRIX *dest, const D3DMATRIX *source);

DWORD ver_ps_tk(UINT _Major,UINT _Minor);
DWORD ver_vs_tk(UINT _Major,UINT _Minor);
DWORD end_tk();
DWORD ins_tk(D3DSHADER_INSTRUCTION_OPCODE_TYPE opcode, BYTE length = 0);
DWORD src_tk(D3DRegisterId reg, 
			BYTE swz_x = 0,
			BYTE swz_y = 1, BYTE swz_z = 2, BYTE swz_w = 3,
			D3DSHADER_PARAM_SRCMOD_TYPE modifier = D3DSPSM_NONE );
DWORD dst_tk(D3DRegisterId reg,  BYTE wmx = 1, BYTE wmy = 1, BYTE wmz = 1, BYTE wmw = 1,
			DWORD modifier = 0, DWORD shift = 0);
DWORD sem_tk(UINT usage, UINT usage_index = 0);
DWORD flt_tk(FLOAT value);
DWORD sam_tk(D3DSAMPLER_TEXTURE_TYPE type);
DWORD com_tk(DWORD size);


struct FFConstRegisterDeclaration {
	D3DRegisterId constant;
	FFUsageId usage;
	FFConstRegisterDeclaration(D3DRegisterId _constant, FFUsageId _usage):
	constant(_constant), usage(_usage) {}
	FFConstRegisterDeclaration() {}
};

struct FFGeneratedShader {
	std::list<FFConstRegisterDeclaration> const_declaration;
	DWORD *code;
	FFGeneratedShader(std::list<FFConstRegisterDeclaration> _const_declaration,
		DWORD *_code): const_declaration(_const_declaration), code(_code) {}
	FFGeneratedShader(): code(0) {}
	~FFGeneratedShader() { if(code !=0) delete code; }
};

class FFShaderGenerator {
public:
	FFGeneratedShader *generate_vertex_shader(FFState _ff_state);
	FFGeneratedShader *generate_pixel_shader(FFState _ff_state);
private:
	FFState ff_state;

	std::list<DWORD> def;
	std::list<DWORD> dec;
	std::list<DWORD> cod;

	D3DRegisterId literals;
	RegisterBank<D3DRegisterId, D3DUsageId> input;
	RegisterBank<D3DRegisterId, D3DUsageId> output;
	RegisterBank<D3DRegisterId, D3DUsageId> samplers;
	RegisterBank<D3DRegisterId> temp;
	RegisterBank<D3DRegisterId, FFUsageId> constant;
	void vs_initialize_register_banks();
	void ps_initialize_register_banks();

	std::list<FFConstRegisterDeclaration> const_declaration;

	void vs_transform(D3DRegisterId dst_N, D3DRegisterId dst_V, D3DRegisterId dst_P);
	void vs_lighting(D3DRegisterId src_N, D3DRegisterId src_V);
	void vs_input_declaration();

    void comment(char *text);
	void negate(D3DRegisterId dst, D3DRegisterId src);
	void mov(D3DRegisterId dst, D3DRegisterId src);
	void normalize(D3DRegisterId res, D3DRegisterId vec);
	void mul_mat3_vec3(D3DRegisterId dst, D3DRegisterId src_vec, D3DRegisterId src_mat0,
		D3DRegisterId src_mat1, D3DRegisterId src_mat2);
	void mul_mat4_vec4(D3DRegisterId dst, D3DRegisterId src_vec, D3DRegisterId src_mat0,
		D3DRegisterId src_mat1, D3DRegisterId src_mat2, D3DRegisterId src_mat3);
	void mul_mat4_vec3(D3DRegisterId dst, D3DRegisterId src_vec, D3DRegisterId src_mat0,
		D3DRegisterId src_mat1, D3DRegisterId src_mat2, D3DRegisterId src_mat3);

};

#endif

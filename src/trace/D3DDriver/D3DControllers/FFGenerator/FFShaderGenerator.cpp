#include "FFShaderGenerator.h"
using namespace std;

bool fvf_has_texcoord(BYTE index, DWORD fvf) {

	BYTE tex_count = (fvf & D3DFVF_TEXCOUNT_MASK) >> D3DFVF_TEXCOUNT_SHIFT;
	return index < tex_count;
}

BYTE fvf_texture_format(BYTE index, DWORD fvf) {
	return 0xfc & (fvf >> (index * 2 + 16));
}

FFState::FFState()
{
	fvf = D3DFVF_XYZ;

	// TRANSFORM
	identity_matrix(&world);
	identity_matrix(&view);
	identity_matrix(&projection);

	// LIGHTING
	BOOL lighting_enabled = false;
	material.Ambient.r = material.Ambient.b = 
	material.Ambient.g = material.Ambient.a = 0;
	material.Diffuse.r = material.Diffuse.b = 
	material.Diffuse.g = material.Diffuse.a = 0;
	material.Specular.r = material.Specular.b = 
	material.Specular.g = material.Specular.a = 0;
	material.Emissive.r = material.Emissive.b = 
	material.Emissive.g = material.Emissive.a = 0;
	material.Power = 0.0f;
	ambient.r = ambient.g =
	ambient.b = ambient.a = 0;
	for(DWORD i = 0; i < 8; i ++) {
		lights_enabled[i] = FALSE;
		D3DLIGHT9 &l = lights[i];
		l.Type = D3DLIGHT_POINT;
		l.Ambient.r = l.Ambient.g = 
		l.Ambient.b = l.Ambient.a = 0;
		l.Diffuse.r = l.Diffuse.g = 
		l.Diffuse.b = l.Diffuse.a = 0;
		l.Specular.r = l.Specular.g = 
		l.Specular.b = l.Specular.a = 0;
		l.Direction.x = l.Direction.y = l.Direction.z;
		l.Attenuation0 = l.Attenuation1 = l.Attenuation2 = 0;
		l.Falloff = 0;
		l.Theta = 0;
		l.Phi = 0;
	}
	
	// FOG
	fog_color.r = fog_color.g = 
	fog_color.b = fog_color.a = 0;
	fog_mode = D3DFOG_NONE;
	fog_density = 0;
	fog_start = 0;
	fog_end = 0;
	fog_range = FALSE;

	setted_texture = false;
	color_op = D3DTOP_MODULATE;
}

void identity_matrix(D3DMATRIX *dest) {
	for(UINT i = 0; i < 4; i ++)
	for(UINT j = 0; j < 4; j ++)
		if(i == j ) dest->m[i][j] = 1.0f;
		else dest->m[i][j] = 0.0f;
}

void copy_matrix(D3DMATRIX *dest, const D3DMATRIX* source) {
	for(UINT i = 0; i < 4; i ++)
	for(UINT j = 0; j < 4; j ++)
		dest->m[i][j] = source->m[i][j];
}

void multiply_matrix(D3DMATRIX *dest, const D3DMATRIX* source_a, const D3DMATRIX* source_b) {
	D3DMATRIX temp;
	for(UINT i = 0; i < 4; i ++)
		for(UINT j = 0; j < 4; j ++) {
			float dot = 0;
			for(UINT k = 0; k < 4; k ++)
				dot += source_a->m[i][k] * source_b->m[k][j];
			temp.m[i][j] = dot;
		}
	copy_matrix(dest, &temp);
}

void transpose_matrix(D3DMATRIX *dest, const D3DMATRIX *source) {
	D3DMATRIX temp;
	for(UINT i = 0; i < 4; i ++)
		for(UINT j = 0; j < 4; j ++)
			temp.m[j][i] = source->m[i][j];
	copy_matrix(dest, &temp);
}

void invert_matrix(D3DMATRIX *dest, const D3DMATRIX *source) {
  // Thanks to gmt@aviator.cis.ufl.edu
  D3DMATRIX temp;

  float Tx, Ty, Tz;
  temp.m[0][0] = source->m[0][0];
  temp.m[1][0] = source->m[0][1];
  temp.m[2][0] = source->m[0][2];

  temp.m[0][1] = source->m[1][0];
  temp.m[1][1] = source->m[1][1];
  temp.m[2][1] = source->m[1][2];

  temp.m[0][2] = source->m[2][0];
  temp.m[1][2] = source->m[2][1];
  temp.m[2][2] = source->m[2][2];

  temp.m[0][3] = temp.m[1][3] = temp.m[2][3] = 0;
  temp.m[3][3] = 1;

  Tx = source->m[3][0];
  Ty = source->m[3][1];
  Tz = source->m[3][2];

  temp.m[3][0] = -( source->m[0][0] * Tx + source->m[0][1] * Ty + source->m[0][2] * Tz );
  temp.m[3][1] = -( source->m[1][0] * Tx + source->m[1][1] * Ty + source->m[1][2] * Tz );
  temp.m[3][2] = -( source->m[2][0] * Tx + source->m[2][1] * Ty + source->m[2][2] * Tz );

  copy_matrix(dest, &temp);
}


DWORD  ver_ps_tk(UINT _Major,UINT _Minor) { return (0xFFFF0000|((_Major)<<8)|(_Minor)); }
DWORD  ver_vs_tk(UINT _Major,UINT _Minor) { return (0xFFFE0000|((_Major)<<8)|(_Minor)); }
DWORD  end_tk() { return 0x0000FFFF; }
DWORD  ins_tk(D3DSHADER_INSTRUCTION_OPCODE_TYPE opcode, BYTE length) {
	return ( 0x00000000 | opcode | ( D3DSI_INSTLENGTH_MASK & (length << D3DSI_INSTLENGTH_SHIFT) ) );
}
DWORD  src_tk(D3DRegisterId reg, 
			 BYTE swz_x, BYTE swz_y, BYTE swz_z, BYTE swz_w,
			 D3DSHADER_PARAM_SRCMOD_TYPE modifier) {
				 return( 0x80000000 |
					 (reg.num & D3DSP_REGNUM_MASK) |
					 ((reg.type << D3DSP_REGTYPE_SHIFT)  & D3DSP_REGTYPE_MASK) |
					 ((reg.type << D3DSP_REGTYPE_SHIFT2) & D3DSP_REGTYPE_MASK2) |
					 (swz_x << D3DSP_SWIZZLE_SHIFT) |
					 (swz_y << (D3DSP_SWIZZLE_SHIFT + 2)) |
					 (swz_z << (D3DSP_SWIZZLE_SHIFT + 4)) |
					 (swz_w << (D3DSP_SWIZZLE_SHIFT + 6)) |
					 (modifier  & D3DSP_SRCMOD_MASK));
			 }

DWORD  dst_tk(D3DRegisterId reg,	 BYTE wmx, BYTE wmy, BYTE wmz, BYTE wmw,
			 DWORD modifier, DWORD shift) {
				return( 0x80000000 |
					(reg.num & D3DSP_REGNUM_MASK) |
					((reg.type << D3DSP_REGTYPE_SHIFT)  & D3DSP_REGTYPE_MASK) |
					((reg.type << D3DSP_REGTYPE_SHIFT2) & D3DSP_REGTYPE_MASK2) |
					 (wmx << 16) |
					 (wmy << (16 + 1)) |
					 (wmz << (16 + 2)) |
					 (wmw << (16 + 3)) |
					 (modifier  & D3DSP_DSTMOD_MASK));
;
			 }

DWORD  sem_tk(UINT usage, UINT usage_index) {
	return ( 0x80000000 | 
		(usage << D3DSP_DCL_USAGE_SHIFT) |
		(usage_index <<  D3DSP_DCL_USAGEINDEX_SHIFT));
}

DWORD  flt_tk(FLOAT value) {
	return (*reinterpret_cast<DWORD *>(&value));
}

DWORD  sam_tk(D3DSAMPLER_TEXTURE_TYPE type) {
	return ( 0x80000000 | type );
}

DWORD  com_tk(DWORD size) {
	return( 0x0000FFFE | ((size << D3DSI_COMMENTSIZE_SHIFT) & D3DSI_COMMENTSIZE_MASK) );
}


FFGeneratedShader *FFShaderGenerator :: generate_vertex_shader(FFState _ff_state) {

	ff_state = _ff_state;

	def.clear();
	dec.clear();
	cod.clear();

	const_declaration.clear();

	vs_initialize_register_banks();

	dec.push_back(ver_vs_tk(3,0));
	vs_input_declaration();

	// Define constants to use as literals
	literals = constant.reserve();
	def.push_back(ins_tk(D3DSIO_DEF, 5));
	def.push_back(dst_tk(literals));
	def.push_back(flt_tk(0.0f));
	def.push_back(flt_tk(1.0f));
	def.push_back(flt_tk(0.0f));
	def.push_back(flt_tk(0.0f));

	D3DRegisterId P = temp.reserve();

	D3DRegisterId N = temp.reserve();
	D3DRegisterId V = temp.reserve();

	vs_transform(N, V, P);

	

	vs_lighting(N, V);

	// Propagate texcoords
	for(DWORD i = 0; i < 8; i ++) {
		if(fvf_has_texcoord(i, ff_state.fvf)) {
			cod.push_back(ins_tk(D3DSIO_MOV, 2));
			cod.push_back(dst_tk(output.find_usage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i))));
			cod.push_back(src_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i))));
		}
	}

	




	cod.push_back(end_tk());

	list<DWORD>::iterator it_tk;
	DWORD *code = new DWORD[dec.size() + def.size() + cod.size()];
	DWORD i = 0;
	for(it_tk = dec.begin(); it_tk != dec.end(); it_tk ++) {
		code[i] = *it_tk;
		i ++;
	}
	for(it_tk = def.begin(); it_tk != def.end(); it_tk ++) {
		code[i] = *it_tk;
		i ++;
	}
	for(it_tk = cod.begin(); it_tk != cod.end(); it_tk ++) {
		code[i] = *it_tk;
		i ++;
	}

	return new FFGeneratedShader(const_declaration, code);


}

FFGeneratedShader *FFShaderGenerator :: generate_pixel_shader(FFState _ff_state) {

	ff_state = _ff_state;

	ps_initialize_register_banks();

	const_declaration.clear();


	D3DRegisterId reg;


	cod.clear();

	cod.push_back(ver_ps_tk(3, 0));

	// TODO provisional, maintain separate lists for code and declarations
	D3DRegisterId sampler;
	if(ff_state.setted_texture & (ff_state.color_op != D3DTOP_SELECTARG1)) {
		sampler = samplers.reserve();
		cod.push_back(ins_tk(D3DSIO_DCL, 2));
		cod.push_back(sam_tk(D3DSTT_2D));
		cod.push_back(dst_tk(sampler));
	}

	// TODO Declare input depending on fvf code

	reg = input.reserve_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 0));
	cod.push_back(ins_tk(D3DSIO_DCL, 2));
	cod.push_back(sem_tk(D3DDECLUSAGE_COLOR, 0));
	cod.push_back(dst_tk(reg));

	reg = input.reserve_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 1));
	cod.push_back(ins_tk(D3DSIO_DCL, 2));
	cod.push_back(sem_tk(D3DDECLUSAGE_COLOR, 1));
	cod.push_back(dst_tk(reg));

	for(DWORD i = 0; i < 8; i ++) {
		if(fvf_has_texcoord(i, ff_state.fvf)) {
			reg = input.reserve_usage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i));
			cod.push_back(ins_tk(D3DSIO_DCL, 2));
			cod.push_back(sem_tk(D3DDECLUSAGE_TEXCOORD, i));
			cod.push_back(dst_tk(reg));
		}
	}

	D3DRegisterId r0 = temp.reserve();

	cod.push_back(ins_tk(D3DSIO_MOV, 2));
	cod.push_back(dst_tk(r0));
	cod.push_back(src_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));

	// Texture op
	if((ff_state.color_op != D3DTOP_SELECTARG1) & ff_state.setted_texture) {
		D3DRegisterId r1 = temp.reserve();
		cod.push_back(ins_tk(D3DSIO_TEX, 3));
		cod.push_back(dst_tk(r1));
		cod.push_back(src_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, 0))));
		cod.push_back(src_tk(sampler));

		if(ff_state.color_op == D3DTOP_MODULATE) {
			cod.push_back(ins_tk(D3DSIO_MUL, 3));
			cod.push_back(dst_tk(r0));
			cod.push_back(src_tk(r1));
			cod.push_back(src_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));
		}
		else if(ff_state.color_op == D3DTOP_ADD) {
			cod.push_back(ins_tk(D3DSIO_ADD, 3));
			cod.push_back(dst_tk(r0));
			cod.push_back(src_tk(r1));
			cod.push_back(src_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));
		}

		temp.release(r1);
	}
	
	cod.push_back(ins_tk(D3DSIO_ADD, 3));
	cod.push_back(dst_tk(D3DRegisterId(0, D3DSPR_COLOROUT)));
	cod.push_back(src_tk(r0));
	cod.push_back(src_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 1))));

	cod.push_back(end_tk());

	temp.release(r0);


	list<DWORD>::iterator it_tk;
	DWORD *code = new DWORD[cod.size()];
	DWORD i = 0;
	for(it_tk = cod.begin(); it_tk != cod.end(); it_tk ++) {
			code[i] = *it_tk;
		i ++;
	}

	return new FFGeneratedShader(const_declaration, code);
}

void FFShaderGenerator :: vs_initialize_register_banks() {
	input.clear();
	output.clear();
	temp.clear();
	constant.clear();
	for(DWORD i = 0; i < 16; i++) input.insert(D3DRegisterId(i, D3DSPR_INPUT));
	for(DWORD i = 0; i < 12; i++) output.insert(D3DRegisterId(i, D3DSPR_OUTPUT));
	for(DWORD i = 0; i < 32; i++) temp.insert(D3DRegisterId(i, D3DSPR_TEMP));
	for(DWORD i = 0; i < 256; i++) constant.insert(D3DRegisterId(i, D3DSPR_CONST));
}

void FFShaderGenerator :: ps_initialize_register_banks() {
	input.clear();
	temp.clear();
	constant.clear();
	for(DWORD i = 0; i < 10; i++) input.insert(D3DRegisterId(i, D3DSPR_INPUT));
	for(DWORD i = 0; i < 32; i++) temp.insert(D3DRegisterId(i, D3DSPR_TEMP));
	for(DWORD i = 0; i < 224; i++) constant.insert(D3DRegisterId(i, D3DSPR_CONST));
	for(DWORD i = 0; i < 8; i++) samplers.insert(D3DRegisterId(i, D3DSPR_SAMPLER));
}



void FFShaderGenerator :: vs_transform(D3DRegisterId dst_N, D3DRegisterId dst_V, D3DRegisterId dst_P) {

	// Declare output
	dec.push_back(ins_tk(D3DSIO_DCL, 2));
	dec.push_back(sem_tk(D3DDECLUSAGE_POSITION, 0));
	dec.push_back(dst_tk(output.reserve_usage(D3DUsageId(D3DDECLUSAGE_POSITION, 0))));

	// Declare constants needed
	FFUsageId const_usage;

	for(DWORD i = 0; i < 4; i ++) {
		const_usage = FFUsageId(FF_WORLDVIEWPROJ, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
			constant.reserve_usage(const_usage), const_usage));
	}

	for(DWORD i = 0; i < 4; i ++)  {
		const_usage = FFUsageId(FF_WORLDVIEW, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
			constant.reserve_usage(const_usage), const_usage));
	}

	for(DWORD i = 0; i < 4; i ++)  {
		const_usage = FFUsageId(FF_WORLDVIEW_IT, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
			constant.reserve_usage(const_usage), const_usage));
	}

	mul_mat4_vec4(output.find_usage(D3DUsageId(D3DDECLUSAGE_POSITION)),
		input.find_usage(D3DUsageId(D3DDECLUSAGE_POSITION)),
		constant.find_usage(FFUsageId(FF_WORLDVIEWPROJ, 0)),
		constant.find_usage(FFUsageId(FF_WORLDVIEWPROJ, 1)),
		constant.find_usage(FFUsageId(FF_WORLDVIEWPROJ, 2)),
		constant.find_usage(FFUsageId(FF_WORLDVIEWPROJ, 3)));

	mul_mat4_vec3(dst_P,
		input.find_usage(D3DUsageId(D3DDECLUSAGE_POSITION)),
		constant.find_usage(FFUsageId(FF_WORLDVIEW, 0)),
		constant.find_usage(FFUsageId(FF_WORLDVIEW, 1)),
		constant.find_usage(FFUsageId(FF_WORLDVIEW, 2)),
		constant.find_usage(FFUsageId(FF_WORLDVIEW, 3)));

	mul_mat3_vec3(dst_N,
		input.find_usage(D3DUsageId(D3DDECLUSAGE_NORMAL)),
		constant.find_usage(FFUsageId(FF_WORLDVIEW_IT, 0)),
		constant.find_usage(FFUsageId(FF_WORLDVIEW_IT, 1)),
		constant.find_usage(FFUsageId(FF_WORLDVIEW_IT, 2)));

	normalize(dst_V, dst_P);
	cod.push_back(ins_tk(D3DSIO_MOV, 2));
	cod.push_back(dst_tk(dst_V, 1, 1, 1, 0));
	cod.push_back(src_tk(dst_V, 0, 1, 2, 3, D3DSPSM_NEG));
}

void FFShaderGenerator :: vs_lighting(D3DRegisterId src_N, D3DRegisterId src_V) {

	// Declare output registers

	dec.push_back(ins_tk(D3DSIO_DCL, 2));
	dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 0));
	dec.push_back(dst_tk(output.reserve_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));

	dec.push_back(ins_tk(D3DSIO_DCL, 2));
	dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 1));
	dec.push_back(dst_tk(output.reserve_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 1))));

	// Declare constants required

	FFUsageId const_usage;

	for(DWORD i = 0; i < 4; i ++) {
		const_usage = FFUsageId(FF_WORLD, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
			constant.reserve_usage(const_usage), const_usage));
	}


	for(DWORD i = 0; i < 4; i ++) {
		const_usage = FFUsageId(FF_VIEW_IT, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
			constant.reserve_usage(const_usage), const_usage));
	}

	const_usage = FFUsageId(FF_MATERIAL_EMISSIVE);
	const_declaration.push_back( FFConstRegisterDeclaration(
	constant.reserve_usage(const_usage), const_usage));
	const_usage = FFUsageId(FF_MATERIAL_SPECULAR);
	const_declaration.push_back( FFConstRegisterDeclaration(
	constant.reserve_usage(const_usage), const_usage));
	const_usage = FFUsageId(FF_MATERIAL_DIFFUSE);
	const_declaration.push_back( FFConstRegisterDeclaration(
	constant.reserve_usage(const_usage), const_usage));
	const_usage = FFUsageId(FF_MATERIAL_AMBIENT);
	const_declaration.push_back( FFConstRegisterDeclaration(
	constant.reserve_usage(const_usage), const_usage));
	const_usage = FFUsageId(FF_MATERIAL_POWER);
	const_declaration.push_back( FFConstRegisterDeclaration(
	constant.reserve_usage(const_usage), const_usage));

	const_usage = FFUsageId(FF_AMBIENT);
	const_declaration.push_back( FFConstRegisterDeclaration(
	constant.reserve_usage(const_usage), const_usage));

	for(DWORD i = 0; i < 8; i ++) {
		const_usage = FFUsageId(FF_LIGHT_AMBIENT, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
		constant.reserve_usage(const_usage), const_usage));
		const_usage = FFUsageId(FF_LIGHT_DIFFUSE, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
		constant.reserve_usage(const_usage), const_usage));
		const_usage = FFUsageId(FF_LIGHT_SPECULAR, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
		constant.reserve_usage(const_usage), const_usage));
		const_usage = FFUsageId(FF_LIGHT_DIRECTION, i);
		const_declaration.push_back( FFConstRegisterDeclaration(
		constant.reserve_usage(const_usage), const_usage));
	}

	// Instructions

	D3DRegisterId Color = temp.reserve();
	D3DRegisterId ColorSpecular = temp.reserve();
		
	cod.push_back(ins_tk(D3DSIO_MOV, 2));
	cod.push_back(dst_tk(Color));
	cod.push_back(src_tk(literals, 0, 0, 0, 1));

	cod.push_back(ins_tk(D3DSIO_MOV, 2));
	cod.push_back(dst_tk(ColorSpecular));
	cod.push_back(src_tk(literals, 0, 0, 0, 1));

	D3DRegisterId r0 = temp.reserve();

	cod.push_back(ins_tk(D3DSIO_MOV, 2));
	cod.push_back(dst_tk(r0));
	cod.push_back(src_tk(constant.find_usage(FFUsageId(FF_AMBIENT))));

	cod.push_back(ins_tk(D3DSIO_MUL, 3));
	cod.push_back(dst_tk(r0));
	cod.push_back(src_tk(r0));
	cod.push_back(src_tk(constant.find_usage(FFUsageId(FF_MATERIAL_AMBIENT))));

	cod.push_back(ins_tk(D3DSIO_ADD, 3));
	cod.push_back(dst_tk(Color));
	cod.push_back(src_tk(r0));
	cod.push_back(src_tk(constant.find_usage(FFUsageId(FF_MATERIAL_EMISSIVE))));

	temp.release(r0);

	for(DWORD i = 0; i < 8; i ++) {
		if(ff_state.lights_enabled[i]) {

			D3DRegisterId L;
			D3DRegisterId r0, r1, r2, r3, r4, r5;


			L = temp.reserve();
			r0 = temp.reserve();
			r1 = temp.reserve();
			r2 = temp.reserve();

			
			mov(r2, constant.find_usage(FFUsageId(FF_LIGHT_DIRECTION, i)));
			normalize(r0, r2);
			negate(r1, r0);
			mul_mat3_vec3(L, r1, constant.find_usage(FFUsageId(FF_VIEW_IT, 0)),
				constant.find_usage(FFUsageId(FF_VIEW_IT, 1)),
				constant.find_usage(FFUsageId(FF_VIEW_IT, 2)));

			temp.release(r0);
			temp.release(r1);
			temp.release(r2);

			D3DRegisterId NdotL = temp.reserve();
			cod.push_back(ins_tk(D3DSIO_DP3, 3));
			cod.push_back(dst_tk(NdotL, 0, 0, 0, 1));
			cod.push_back(src_tk(src_N));
			cod.push_back(src_tk(L));

			// if(NdotL < 0.0f)
			D3DRegisterId NdotL_positive = temp.reserve();
			cod.push_back(ins_tk(D3DSIO_SGE, 3));
			cod.push_back(dst_tk(NdotL_positive));
			cod.push_back(src_tk(NdotL, 3, 3, 3, 3));
			cod.push_back(src_tk(literals, 0, 0, 0, 0));

			r0 = temp.reserve();
			r1 = temp.reserve();
			cod.push_back(ins_tk(D3DSIO_MUL, 3));
			cod.push_back(dst_tk(r0));
			cod.push_back(src_tk(NdotL, 3, 3, 3, 3));
			cod.push_back(src_tk(constant.find_usage(FFUsageId(FF_LIGHT_DIFFUSE, i))));


			cod.push_back(ins_tk(D3DSIO_MUL, 3));
			cod.push_back(dst_tk(r1));
			cod.push_back(src_tk(r0));
			cod.push_back(src_tk(constant.find_usage(FFUsageId(FF_MATERIAL_DIFFUSE))));

			cod.push_back(ins_tk(D3DSIO_MUL, 3));
			cod.push_back(dst_tk(r1));
			cod.push_back(src_tk(r1));
			cod.push_back(src_tk(NdotL_positive));


			cod.push_back(ins_tk(D3DSIO_ADD, 3));
			cod.push_back(dst_tk(Color));
			cod.push_back(src_tk(Color));
			cod.push_back(src_tk(r1));

			temp.release(r0);
			temp.release(r1);

			D3DRegisterId H = temp.reserve();
			
			r0 = temp.reserve();

			cod.push_back(ins_tk(D3DSIO_ADD, 3));
			cod.push_back(dst_tk(r0, 1, 1, 1, 0));
			cod.push_back(src_tk(src_V));
			cod.push_back(src_tk(L));
			normalize(H, r0);

			temp.release(r0);

			r0 = temp.reserve();
			r1 = temp.reserve();
			r2 = temp.reserve();
			r3 = temp.reserve();
			r4 = temp.reserve();
			r5 = temp.reserve();

			cod.push_back(ins_tk(D3DSIO_DP3, 3));
			cod.push_back(dst_tk(r0, 0, 0, 0, 1));
			cod.push_back(src_tk(H));
			cod.push_back(src_tk(src_N));

			cod.push_back(ins_tk(D3DSIO_MAX, 3));
			cod.push_back(dst_tk(r1, 0, 0, 0, 1));
			cod.push_back(src_tk(literals, 0, 0, 0, 0));
			cod.push_back(src_tk(r0, 3, 3, 3, 3));

			cod.push_back(ins_tk(D3DSIO_LOG, 2));
			cod.push_back(dst_tk(r2, 0, 0, 0, 1));
			cod.push_back(src_tk(r1, 3, 3, 3, 3));

			cod.push_back(ins_tk(D3DSIO_MUL, 3));
			cod.push_back(dst_tk(r3, 0, 0, 0, 1));
			cod.push_back(src_tk(r2, 3, 3, 3, 3));
			cod.push_back(src_tk(constant.find_usage(FFUsageId(FF_MATERIAL_POWER)), 0, 0, 0, 0));

			cod.push_back(ins_tk(D3DSIO_EXP, 2));
			cod.push_back(dst_tk(r4, 0, 0, 0, 1));
			cod.push_back(src_tk(r3, 3, 3, 3, 3));


			cod.push_back(ins_tk(D3DSIO_MUL, 3));
			cod.push_back(dst_tk(r4));
			cod.push_back(src_tk(r4, 3, 3, 3, 3));
			cod.push_back(src_tk(constant.find_usage(FFUsageId(FF_LIGHT_SPECULAR, i))));

			cod.push_back(ins_tk(D3DSIO_MUL, 3));
			cod.push_back(dst_tk(r5));
			cod.push_back(src_tk(r4));
			cod.push_back(src_tk(constant.find_usage(FFUsageId(FF_MATERIAL_SPECULAR))));

			cod.push_back(ins_tk(D3DSIO_MUL, 3));
			cod.push_back(dst_tk(r5));
			cod.push_back(src_tk(r5));
			cod.push_back(src_tk(NdotL_positive));

			cod.push_back(ins_tk(D3DSIO_ADD, 3));
			cod.push_back(dst_tk(ColorSpecular));
			cod.push_back(src_tk(ColorSpecular));
			cod.push_back(src_tk(r5));
			


			temp.release(NdotL_positive);
			temp.release(NdotL);

		}
	}

	cod.push_back(ins_tk(D3DSIO_MOV, 2));
	cod.push_back(dst_tk(output.find_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));
	cod.push_back(src_tk(Color));	

	cod.push_back(ins_tk(D3DSIO_MOV, 2));
	cod.push_back(dst_tk(output.find_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 1))));
	cod.push_back(src_tk(ColorSpecular));

	temp.release(Color);
	temp.release(ColorSpecular);


}

void FFShaderGenerator :: vs_input_declaration() {


	// TODO Make this depend on FVF
	input.reserve_usage(D3DUsageId(D3DDECLUSAGE_POSITION, 0));
	input.reserve_usage(D3DUsageId(D3DDECLUSAGE_NORMAL, 0));
	input.reserve_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 0));

	dec.push_back(ins_tk(D3DSIO_DCL, 2));
	dec.push_back(sem_tk(D3DDECLUSAGE_POSITION, 0));
	dec.push_back(dst_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_POSITION, 0))));

	dec.push_back(ins_tk(D3DSIO_DCL, 2));
	dec.push_back(sem_tk(D3DDECLUSAGE_NORMAL, 0));
	dec.push_back(dst_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_NORMAL, 0))));

	dec.push_back(ins_tk(D3DSIO_DCL, 2));
	dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 0));
	dec.push_back(dst_tk(input.find_usage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));


	D3DRegisterId reg;
	for(DWORD i = 0; i < 8; i ++) {
		if(fvf_has_texcoord(i, ff_state.fvf)) {
			reg = input.reserve_usage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i));
			dec.push_back(ins_tk(D3DSIO_DCL, 2));
			dec.push_back(sem_tk(D3DDECLUSAGE_TEXCOORD, i));
			dec.push_back(dst_tk(reg));
			reg = output.reserve_usage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i));
			dec.push_back(ins_tk(D3DSIO_DCL, 2));
			dec.push_back(sem_tk(D3DDECLUSAGE_TEXCOORD, i));
			dec.push_back(dst_tk(reg));
		}
	}
}

void FFShaderGenerator::  comment(char *text) {
	UINT count_b = strlen(text) + 1;
	DWORD count_tk = count_b / sizeof(DWORD);
	if((count_b % sizeof(DWORD)) != 0)
		count_tk ++;
	DWORD com = com_tk(count_tk);
	cod.push_back(com);
	DWORD data = 0x00000000;
	for(UINT i = 0; i < count_b; i ++)  {
		data |= static_cast<DWORD>(text[i]) <<
			(((sizeof(DWORD) - 1) - (i % sizeof(DWORD)))* 8);
		if(((i > 0) && (((i + 1) % sizeof(DWORD)) == 0)) || (text[i] == 0x00)) {
			cod.push_back(data);
			data = 0x00000000;
		}
	}
}

void FFShaderGenerator::  mov( D3DRegisterId res, D3DRegisterId src) {
	cod.push_back(ins_tk(D3DSIO_MOV, 2));
	cod.push_back(dst_tk(res));
	cod.push_back(src_tk(src));
}

void FFShaderGenerator::  normalize(D3DRegisterId res, D3DRegisterId vec) {
	D3DRegisterId r0 = temp.reserve(),
		r1 = temp.reserve();

	cod.push_back(ins_tk(D3DSIO_DP3, 3));
	cod.push_back(dst_tk(r0, 0, 0, 0, 1));
	cod.push_back(src_tk(vec));
	cod.push_back(src_tk(vec));

	cod.push_back(ins_tk(D3DSIO_RSQ, 2));
	cod.push_back(dst_tk(r1, 0, 0, 0, 1));
	cod.push_back(src_tk(r0, 3, 3, 3, 3));

	cod.push_back(ins_tk(D3DSIO_MUL, 3));
	cod.push_back(dst_tk(res, 1, 1, 1, 0));
	cod.push_back(src_tk(r1, 3, 3, 3, 3));
	cod.push_back(src_tk(vec));

	temp.release(r0);
	temp.release(r1);
}

void FFShaderGenerator::  negate(D3DRegisterId res, D3DRegisterId src) {
		cod.push_back(ins_tk(D3DSIO_MOV, 2));
		cod.push_back(dst_tk(res));
		cod.push_back(src_tk(src, 0, 1, 2, 3, D3DSPSM_NEG));
}


void FFShaderGenerator::  mul_mat4_vec3(D3DRegisterId res, D3DRegisterId vec, D3DRegisterId mat0,
									 D3DRegisterId mat1, D3DRegisterId mat2, D3DRegisterId mat3) {
		// Reserve temporals
		D3DRegisterId r0 = temp.reserve(),
			r1 = temp.reserve(),
			r2 = temp.reserve(),
			r3 = temp.reserve(),
			r4 = temp.reserve(),
			r5 = temp.reserve(),
			r6 = temp.reserve();

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r1, 1, 1, 1, 0));
		cod.push_back(src_tk(mat0));
		cod.push_back(src_tk(vec, 0, 0, 0, 0));

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r2, 1, 1, 1, 0));
		cod.push_back(src_tk(mat1));
		cod.push_back(src_tk(vec, 1, 1, 1, 1));

		cod.push_back(ins_tk(D3DSIO_ADD, 3));
		cod.push_back(dst_tk(r0, 1, 1, 1, 0));
		cod.push_back(src_tk(r1));
		cod.push_back(src_tk(r2));

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r3, 1, 1, 1, 0));
		cod.push_back(src_tk(mat2));
		cod.push_back(src_tk(vec, 2, 2, 2, 2));

		cod.push_back(ins_tk(D3DSIO_ADD, 3));
		cod.push_back(dst_tk(r4, 1, 1, 1, 0));
		cod.push_back(src_tk(r0));
		cod.push_back(src_tk(r3));

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r5, 1, 1, 1, 0));
		cod.push_back(src_tk(mat3));
		cod.push_back(src_tk(vec, 3, 3, 3, 3));

		cod.push_back(ins_tk(D3DSIO_ADD, 3));
		cod.push_back(dst_tk(r6, 1, 1, 1, 0));
		cod.push_back(src_tk(r4));
		cod.push_back(src_tk(r5));

		cod.push_back(ins_tk(D3DSIO_MOV, 2));
		cod.push_back(dst_tk(res, 1, 1, 1, 0));
		cod.push_back(src_tk(r6));

		// Release constants

		temp.release(r0);
		temp.release(r1);
		temp.release(r2);
		temp.release(r3);
		temp.release(r4);
		temp.release(r5);
		temp.release(r6);
}

void FFShaderGenerator::  mul_mat3_vec3(D3DRegisterId res, D3DRegisterId vec, D3DRegisterId mat0,
	   D3DRegisterId mat1, D3DRegisterId mat2) {

		D3DRegisterId r0 = temp.reserve(),
			r1 = temp.reserve(),
			r2 = temp.reserve(),
			r3 = temp.reserve();

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r0, 1, 1, 1, 0));
		cod.push_back(src_tk(vec, 0, 0, 0, 0));
		cod.push_back(src_tk(mat0));
		
		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r1, 1, 1, 1, 0));
		cod.push_back(src_tk(vec, 1, 1, 1, 1));
		cod.push_back(src_tk(mat1));

		cod.push_back(ins_tk(D3DSIO_ADD, 3));
		cod.push_back(dst_tk(r2, 1, 1, 1, 0));
		cod.push_back(src_tk(r0));
		cod.push_back(src_tk(r1));

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r3, 1, 1, 1, 0));
		cod.push_back(src_tk(vec, 2, 2, 2, 2));
		cod.push_back(src_tk(mat2));

		cod.push_back(ins_tk(D3DSIO_ADD, 3));
		cod.push_back(dst_tk(res, 1, 1, 1, 0));
		cod.push_back(src_tk(r2));
		cod.push_back(src_tk(r3));

		temp.release(r0);
		temp.release(r1);
		temp.release(r2);
		temp.release(r3);

}


void FFShaderGenerator::  mul_mat4_vec4(D3DRegisterId res, D3DRegisterId vec,
	D3DRegisterId mat0, D3DRegisterId mat1, D3DRegisterId mat2, D3DRegisterId mat3) {
		// Reserve temporals
		D3DRegisterId r0 = temp.reserve(),
			r1 = temp.reserve(),
			r2 = temp.reserve(),
			r3 = temp.reserve(),
			r4 = temp.reserve(),
			r5 = temp.reserve(),
			r6 = temp.reserve();

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r1));
		cod.push_back(src_tk(mat0));
		cod.push_back(src_tk(vec, 0, 0, 0, 0));

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r2));
		cod.push_back(src_tk(mat1));
		cod.push_back(src_tk(vec, 1, 1, 1, 1));

		cod.push_back(ins_tk(D3DSIO_ADD, 3));
		cod.push_back(dst_tk(r0));
		cod.push_back(src_tk(r1));
		cod.push_back(src_tk(r2));

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r3));
		cod.push_back(src_tk(mat2));
		cod.push_back(src_tk(vec, 2, 2, 2, 2));

		cod.push_back(ins_tk(D3DSIO_ADD, 3));
		cod.push_back(dst_tk(r4));
		cod.push_back(src_tk(r0));
		cod.push_back(src_tk(r3));

		cod.push_back(ins_tk(D3DSIO_MUL, 3));
		cod.push_back(dst_tk(r5));
		cod.push_back(src_tk(mat3));
		cod.push_back(src_tk(vec, 3, 3, 3, 3));

		cod.push_back(ins_tk(D3DSIO_ADD, 3));
		cod.push_back(dst_tk(r6));
		cod.push_back(src_tk(r4));
		cod.push_back(src_tk(r5));

		cod.push_back(ins_tk(D3DSIO_MOV, 2));
		cod.push_back(dst_tk(res));
		cod.push_back(src_tk(r6));

		// Release constants

		temp.release(r0);
		temp.release(r1);
		temp.release(r2);
		temp.release(r3);
		temp.release(r4);
		temp.release(r5);
		temp.release(r6);

}


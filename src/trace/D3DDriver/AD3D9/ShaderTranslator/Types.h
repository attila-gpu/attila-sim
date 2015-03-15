/**************************************************************************
 *
 * Copyright (c) 2002 - 2011 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 */

#ifndef SHADER_TRANSLATOR_TYPES_H_INCLUDED
#define SHADER_TRANSLATOR_TYPES_H_INCLUDED

#include "ShaderInstruction.h"
#include "d3d9_port.h"
#include "d3d9types.h"
#include <list>
#include <string>

struct ConstValue
{
    union
    {
        u8bit data[4 * 4];
        struct 
        {
            f32bit x;
            f32bit y;
            f32bit z;
            f32bit w;            
        } floatValue;
        struct
        {
            s32bit x;
            s32bit y;
            s32bit z;
            s32bit w;
        } intValue;
        bool boolValue;
    } value;
    
    ConstValue(f32bit *v)
    {
        value.floatValue.x = v[0];
        value.floatValue.y = v[1];
        value.floatValue.z = v[2];
        value.floatValue.w = v[3];
    }
    
    ConstValue(s32bit *v)
    {
        value.intValue.x = v[0];
        value.intValue.y = v[1];
        value.intValue.z = v[2];
        value.intValue.w = v[3];
    }
    
    ConstValue(bool v)
    {
        value.boolValue = v;
    }
    
    ConstValue()
    {
        for(u32bit b = 0; b < (4*4); b++)
            value.data[b] = 0;
    }
    
};

struct D3DUsageId {
    DWORD usage;
    BYTE index;
    bool operator<(const D3DUsageId &b) const {
        if((unsigned int)(usage) < (unsigned int)(b.usage))
            return true;
        else if((unsigned int)(usage) > (unsigned int)(b.usage))
            return false;
        else {
            return (index < b.index);
        }
    }
    bool operator==(const D3DUsageId &b) const {
        return (usage == b.usage) & (index == b.index);
    }
    D3DUsageId(): usage(0), index(0) {}
    D3DUsageId(DWORD _usage, BYTE _index = 0): usage(_usage), index(_index) {}
};

struct D3DRegisterId {
    DWORD num;
    D3DSHADER_PARAM_REGISTER_TYPE type;
    bool operator<(const D3DRegisterId &b) const {
        if((unsigned int)(type) < (unsigned int)(b.type))
            return true;
        else if((unsigned int)(type) > (unsigned int)(b.type))
            return false;
        else {
            return (num < b.num);
        }
    }
    bool operator==(const D3DRegisterId &b) const {
        return (num == b.num) & (type == b.type);
    }
    D3DRegisterId(): num(0), type(D3DSPR_TEMP) {}
    D3DRegisterId(DWORD _num, D3DSHADER_PARAM_REGISTER_TYPE _type): num(_num), type(_type) {}
};

/// Identifies a GPURegister, not surprisingly uh?
struct GPURegisterId {
    u32bit num;
    gpu3d::Bank bank;
    bool operator<(const GPURegisterId &b) const {
        if((unsigned int)(bank) < (unsigned int)(b.bank))
            return true;
        else if((unsigned int)(bank) > (unsigned int)(b.bank))
            return false;
        else {
            return (num < b.num);
        }
    }
    bool operator==(const GPURegisterId &b) const {
        return (num == b.num) & (bank == b.bank);
    }

    GPURegisterId(): num(0), bank(gpu3d::INVALID) {}
    GPURegisterId(u32bit _num, gpu3d::Bank _bank): num(_num), bank(_bank) {}
};

struct SamplerDeclaration {
    D3DSAMPLER_TEXTURE_TYPE type;
    DWORD d3d_sampler;
    u32bit native_sampler;
    SamplerDeclaration() : d3d_sampler(0), native_sampler(0), type(D3DSTT_2D) {}
    SamplerDeclaration(DWORD _d3d_sampler, u32bit _native_sampler, D3DSAMPLER_TEXTURE_TYPE _type) :
        d3d_sampler(_d3d_sampler), native_sampler(_native_sampler), type(_type) {}
};

struct InputRegisterDeclaration {
    BYTE usage;
    BYTE  usage_index;
    u32bit native_register;
    InputRegisterDeclaration(): usage(0), usage_index(0), native_register(0) {}
    InputRegisterDeclaration(BYTE _usage, BYTE _usage_index, u32bit _native_register):
    usage(_usage), usage_index(_usage_index), native_register(_native_register) {}
};

struct OutputRegisterDeclaration {
    BYTE usage;
    BYTE  usage_index;
    u32bit native_register;
    OutputRegisterDeclaration(): usage(0), usage_index(0), native_register(0) {}
    OutputRegisterDeclaration(BYTE _usage, BYTE _usage_index, u32bit _native_register):
    usage(_usage), usage_index(_usage_index), native_register(_native_register) {}
};

struct ConstRegisterDeclaration {
    DWORD d3d_register;
    u32bit native_register;
    bool defined;
    ConstValue value;
    ConstRegisterDeclaration(): native_register(0), d3d_register(0), defined(false) {}
    ConstRegisterDeclaration(u32bit _native_register, DWORD _d3d_register,
        bool _defined, ConstValue _value):
    native_register(_native_register), value(_value),
        defined(_defined), d3d_register(_d3d_register) {}
};

struct AlphaTestDeclaration {
	D3DCMPFUNC comparison;
	u32bit alpha_const_minus_one;
	u32bit alpha_const_ref;
	AlphaTestDeclaration() {
		comparison = D3DCMP_ALWAYS;
		alpha_const_ref = 0;
		alpha_const_minus_one = 0;
	}
	AlphaTestDeclaration(D3DCMPFUNC _comparison, u32bit _alpha_const_ref,
		u32bit _alpha_const_minus_one) {
		comparison = _comparison;
		alpha_const_minus_one = _alpha_const_minus_one;
		alpha_const_ref = _alpha_const_ref;
	}
};

struct FogDeclaration {
	bool fog_enabled;
	u32bit fog_const_color;
	FogDeclaration() {
		fog_enabled = false;
		fog_const_color = 0;
	}
	
	FogDeclaration(bool _fog_enabled, u32bit _fog_const_color)
	{
		fog_enabled = _fog_enabled;
		fog_const_color = _fog_const_color;
	}
};


struct NativeShaderDeclaration {
    std::list<ConstRegisterDeclaration> constant_registers;
    std::list<InputRegisterDeclaration> input_registers;
    std::list<OutputRegisterDeclaration> output_registers;
    std::list<SamplerDeclaration> sampler_registers;
	AlphaTestDeclaration alpha_test;
	FogDeclaration fog;
    NativeShaderDeclaration() {}
    NativeShaderDeclaration(    std::list<ConstRegisterDeclaration> _constant_registers,
        std::list<InputRegisterDeclaration> _input_registers,
        std::list<OutputRegisterDeclaration> _output_registers,
        std::list<SamplerDeclaration> _sampler_registers,
        AlphaTestDeclaration _alpha_test,
        FogDeclaration _fog ):
        constant_registers(_constant_registers), input_registers(_input_registers),
        output_registers(_output_registers), sampler_registers(_sampler_registers),
        alpha_test(_alpha_test), fog(_fog) {}
};

struct NativeShader {
    NativeShaderDeclaration declaration;
    u8bit *bytecode;
    u32bit lenght;
    
    //  D3D9 shader tokens and disassembled code.
    std::string debug_ir;
    std::string debug_disassembler;

    //  ATTILA binary and assembly code.
    std::string debug_binary;
    std::string debug_assembler;

    bool untranslatedControlFlow;
    u32bit untranslatedInstructions;

    NativeShader(): bytecode(0), lenght(0) {}
    
    NativeShader(NativeShaderDeclaration _declaration,
        u8bit *_bytecode, u32bit _lenght,
        std::string _debug_binary = "", std::string _debug_assembler = "", std::string _debug_ir = "", std::string _debug_disassembler = ""):
    declaration(_declaration), bytecode(_bytecode), lenght(_lenght), untranslatedControlFlow(false), untranslatedInstructions(0),
    debug_binary(_debug_binary), debug_assembler(_debug_assembler), debug_ir(_debug_ir), debug_disassembler(_debug_disassembler) {}
    
    ~NativeShader() { if(bytecode != 0) delete bytecode; }
};


#endif // SHADER_TRANSLATOR_TYPES_H_INCLUDED

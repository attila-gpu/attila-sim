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

#ifndef ____FF_SHADER_GENERATOR
#define ____FF_SHADER_GENERATOR

#include <set>
#include <list>
#include <map>
#include <vector>

#include "Types.h"

/**
 *
 *  Defines the state associated with fixed function D3D9 Texture Stage.
 *
 */
struct TextureStageState
{
    D3DTEXTUREOP colorOp;           /**<  Stage operation for the color components RGB.  */
    u32bit colorArg0;               /**<  Third argument for the color operation (triadic operations MAD and LERP).  */
    u32bit colorArg1;               /**<  First argument for the color operation.  */
    u32bit colorArg2;               /**<  Second argument for the color operation.  */
    D3DTEXTUREOP alphaOp;           /**<  Stage operation for the alpha component.  */
    u32bit alphaArg0;               /**<  Third argument for the alpha operation (triadic operations MAD and LERP).  */
    u32bit alphaArg1;               /**<  First argument for the alpha operation.  */
    u32bit alphaArg2;               /**<  Second argument for the alpha operation.  */
    f32bit bumpEnvMatrix[2][2];     /**<  Bump-mapping matrix.  */
    f32bit bumpEnvLScale;           /**<  Bump-mapping luminance scale.  */
    f32bit bumpEnvLOffset;          /**<  Bump-mapping luminance offset.  */
    u32bit index;                   /**<  Index to the texture coordinate to use for the stage.  */
    D3DTEXTURETRANSFORMFLAGS transformFlags;        /**<  Transformation for the texture coordinates.  */
    u32bit resultArg;               /**<  Destination register for the result (current or temp).  */
    D3DCOLORVALUE constant;         /**<  Constant color value.  */

    /**
     *
     *  Constructor.  Used to initialize the TextureStageState fields to default values.
     *
     */
     
    TextureStageState();
};

/**
 *
 *  Defines the fixed function state for D3D9 shader generation.
 *
 */
struct FFState
{

    //
    //  Vertex related state.
    //
    
    //
    //  Vertex input declaration.
    //
    DWORD fvf;      /**<  Fixed function vertex declaration.  */    
    std::vector<D3DVERTEXELEMENT9> vertexDeclaration;       /**<  Vertex declaration.  */

    //
    //  Vertex position transformation and blending.
    //
    D3DMATRIX world;            /**<  World matrix.  */
    D3DMATRIX view;             /**<  View matrix.  */
    D3DMATRIX projection;       /**<  Projection matrix.  */
    DWORD vertexBlend;          /**<  Defines if vertex blending is enabled and the mode.  */
    bool indexedVertexBlend;    /**<  Defines if indexed vertex blending is enabled.  */
    f32bit tweenFactor;         /**<  Tween factor for vertex blending.  */

    //  Texture coordinate transformation matrices.    
    D3DMATRIX texture[8];                   /**<  Texture coordinate transform matrix.  */

    //
    //  Vertex color and ligthing.
    //
    
    bool lightingEnabled;       /**<  Defines if vertex lighting is enabled.  */
    bool vertexColor;           /**<  Defines if vertex color is enabled.  */
    bool specularEnable;        /**<  Defines if specular highlights are enabled (adds a second color output, applied in pixel).  */
    bool localViewer;           /**<  Defines if camera-relative specular highlights (true) or orthogonal specular highlights (false) are used.  */
    bool normalizeNormals;      /**<  Defines if vertex normals are normalized.  */
    D3DMATERIALCOLORSOURCE diffuseMaterialSource;   /**<  Defines the diffuse material source for vertex lighting.  */
    D3DMATERIALCOLORSOURCE specularMaterialSource;  /**<  Defines the specular material source for vertex lighting.  */
    D3DMATERIALCOLORSOURCE ambientMaterialSource;   /**<  Defines the ambient material source for vertex lighting.  */
    D3DMATERIALCOLORSOURCE emissiveMaterialSource;  /**<  Defines the emissive material source for vertex lighting.  */
    D3DMATERIAL9 material;                          /**<  Defines the material for vertex lighting.  */
    D3DCOLORVALUE ambient;                          /**<  Defines the ambient color for vertex lighting.  */
    D3DLIGHT9 lights[8];                            /**<  Defines the lights for vertex lighiting.  */
    bool lightsEnabled[8];                          /**<  Defines if lights are enabled for vertex lighting.  */

    //
    //  Pixel related state.
    //
    
    bool settedTexture[8];                      /**<  Defines if a texture stage has a defined texture.  */
    D3DSAMPLER_TEXTURE_TYPE textureType[8];     /**<  The texture type for the defined texture.  */
    TextureStageState textureStage[8];          /**<  Texture stage state.  */
    D3DCOLORVALUE textureFactor;                /**<  Texture factor from render state.  */

    //
    //  Fog related state (shared between vertex and pixel).
    //
    
    bool fogEnable;                 /**<  Defines if fog is enabled.  */
    D3DCOLORVALUE fogColor;         /**<  Defines the fog color.  */
    D3DFOGMODE fogPixelMode;        /**<  Defines the pixel fog mode.  */
    D3DFOGMODE fogVertexMode;       /**<  Defines the vertex fog mode.  */
    FLOAT fogStart;                 /**<  Defines the fog start.  */
    FLOAT fogEnd;                   /**<  Defines the fog end.  */
    FLOAT fogDensity;               /**<  Defines the fog density.  */
    bool  fogRange;                 /**<  Defines the fog range.  */

    /**
     *
     *  Constructor.  Initializes the structure with default values.
     *
     */
     
    FFState();
};

/**
 *
 *  Defines the usages for constants in the fixed function shaders.
 *
 */
enum FFUsage
{
    FF_NONE,
    FF_VIEWPORT,                    /**<  Constant stores viewport/resolution information -> (2/width, 2/height, 1, ) */
    FF_WORLDVIEWPROJ,               /**<  Constants store the world x view x projection matrix.  */
    FF_WORLDVIEW,                   /**<  Constants store the world x view matrix.  */
    FF_WORLDVIEW_IT,                /**<  Constants store the world x view ? matrix.  */
    FF_VIEW_IT,                     /**<  Constants store the view matrix ?.  */
    FF_WORLD,                       /**<  Constants store the world matrix.  */
    FF_MATERIAL_EMISSIVE,           /**<  Constant stores the material emissive color.  */
    FF_MATERIAL_SPECULAR,           /**<  Constant stores the material specular color.  */
    FF_MATERIAL_DIFFUSE,            /**<  Constant stores the material diffuse color.  */
    FF_MATERIAL_AMBIENT,            /**<  Constant stores the material ambient color.  */
    FF_MATERIAL_POWER,              /**<  Constant stores the material specular power ?.  */
    FF_AMBIENT,                     /**<  Constant stores the ambient color.  */
    FF_LIGHT_POSITION,              /**<  Constant stores the light position.  */
    FF_LIGHT_DIRECTION,             /**<  Constant stores the light direction.  */
    FF_LIGHT_AMBIENT,               /**<  Constant stores the light ambient color.  */
    FF_LIGHT_DIFFUSE,               /**<  Constant stores the light diffuse color.  */
    FF_LIGHT_SPECULAR,              /**<  Constant stores the light specular color.  */
    FF_LIGHT_RANGE,                 /**<  Constant stores the light range.  */
    FF_LIGHT_ATTENUATION,           /**<  Constant stores the light attenuation.  */
    FF_LIGHT_SPOT                   /**<  Constant stores the light spot ?.  */
};

/**
 *
 *  Defines the fixed function usage for a constant in a generated shader.
 */
struct FFUsageId
{
    FFUsage usage;      /**<  Defines the usage of the constant.  */
    BYTE index;         /**<  Defines the index for the usage.  */
    
    /**
     *
     *  Less-than comparison operator.
     *
     *  @param b Reference to a FFUsageId object to be compared with this object.
     *
     *  @return If this object usage is smaller than the parameter object usage, or if equal if this object usage index
        is smaller than the parameter object usage index.
     *
     */
    bool operator<(const FFUsageId &b) const
    {
        if((unsigned int)(usage) < (unsigned int)(b.usage))
            return true;
        else if((unsigned int)(usage) > (unsigned int)(b.usage))
            return false;
        else
            return (index < b.index);
    }

    /**
     *
     *  Equal comparison operator.
     *
     *  @param b Reference to a FFUsageId object to be compared with this object.
     *
     *  @return If this object usage and usage index is equal to the parameter object usage and usage index.
     *
     */     
    bool operator==(const FFUsageId &b) const
    {
        return (usage == b.usage) && (index == b.index);
    }
    
    /**
     *
     *  Constructor.  Sets default values.
     *
     */     
    FFUsageId(): usage(FF_NONE), index(0) {}
    
    /**
     *
     *  Constructor.  Sets defined values.
     *
     *  @param _usage The usage defined for the new object.
     *  @param _index The usage index defined for the new object, default is 0.
     *
     **/
    FFUsageId(FFUsage _usage, BYTE _index = 0): usage(_usage), index(_index) {}
};

/**
 *
 *  Template class that stores information about the different registers banks for the
 *  generated shaders.
 *
 *  Class parameters: 
 *
 *    REGISTERID  The type to be used the register identifiers.
 *    USAGEID The type to be used for the register usages.
 *
 */
template<typename REGISTERID, typename USAGEID = REGISTERID>
class RegisterBank
{
private:
    std::set<REGISTERID> available;                     /**<  Set storing the available registers.  */
    std::map<USAGEID, REGISTERID> reservedUsage;       /**<  Map that maps registers to usages.  */
    std::set<REGISTERID> reserved;                      /**<  Set storing the reserved registers.  */
public:

    /**
     *
     *  Adds a register to the bank.
     *
     *  @param reg The register identifier for the new register to be added.
     *
     */
    void insert(REGISTERID reg)
    {
        available.insert(reg);
    }

    /**
     *
     *  Clears the register bank from all registers.
     *
     */
    void clear()
    {
        available.clear();
        reserved.clear();
        reservedUsage.clear();
    }

    /**
     *
     *  Reserves a register.
     *
     *  @return The identifier of the reserved register.
     *
     */
    REGISTERID reserve()
    {
        REGISTERID reserved;
        typename std::set<REGISTERID>::iterator it_a;
        
        //  Get the first available register.
        it_a = available.begin();
        
        //  Check if it actually got a register.
        if (it_a == available.end())
        {
            panic("RegisterBank", "reserve", "There are no available registers.");
        }
        else
        {
            //  Set the register as reserved.
            reserved = *it_a;
            available.erase(reserved);
        }
        
        return reserved;
    }

    /**
     *
     *  Checks if the usage is used in a reserved register.
     *
     *  @param usage The usage to check for a reserved register.
     *
     *  @return If the usage is used by a reserved register.
     *
     */
    bool isReservedUsage(USAGEID usage)
    {
        return (reservedUsage.find(usage) != reservedUsage.end());
    }


    /**
     *
     *  Checks if a register is reserved.
     *
     *  @param reg The identifier of the register to check if it is reserved.
     *
     *  @return If the register is reserved.
     *
     */
    bool isReserved(REGISTERID reg)
    {
        return (reserved.find(reg) != reserved.end());
    }


    /**
     *
     *  Reserves a register and associates an usage.
     *
     *  @param usage The usage defined for the usage.
     *
     *  @return The reserved register identifier.
     *
     */
     
    REGISTERID reserveUsage(USAGEID usage)
    {
        // Reserve a register.
        REGISTERID reserved = reserve();
                
        typename std::map<USAGEID, REGISTERID>::iterator it;
        
        //  Check if the usage is already defined to reserved register.
        if (isReservedUsage(usage))
            panic("RegisterBank", "reserveUsage", "Usage was already reserved for a reserved register.");
        
        //  Map the reserved register with the usage.
        reservedUsage[usage] = reserved;
        
        return reserved;
    }

    /**
     *
     *  Search for the register associated with the defined usage.  If the usage has no register reserved and
     *  the function is asked to a new register will be reserved and returned.
     *
     *  @param usage The usage for which to search the associated reserved register.
     *  @param reserve Defines if when the usage has not register reserved reserve a register on the spot and return it.
     *
     *  @return The register identifier for the reserved register associated with the defined usage.
     *
     */
    REGISTERID findUsage(USAGEID usage, bool reserve = false)
    {
        REGISTERID found_register;
        typename std::map<USAGEID, REGISTERID>::iterator it_u;
        
        //  Search for the usage.
        it_u = reservedUsage.find(usage);
        if(it_u != reservedUsage.end())
        {
            //  Return the usage found.
            found_register = (*it_u).second;
        }
        else
        {
            // Check if requested to reserve a register if not found.
            if(reserve)
            {
                //  Reserve a new register for the usage.
                found_register = reserveUsage(usage);
            }
            else
                panic("RegisterBank", "findUsage", "Could not reserve register for usage.");
        }
        
        return found_register;
    }

    /**
     *
     *  Releases as register.
     *
     *  @param reg The identifier of the register to release.
     *
     */

    void release(REGISTERID reg)
    {
        available.insert(reg);
        reserved.erase(reg);
    }
    
    /**
     *
     *  Releases an usage and it's associated register.
     *
     *  @param usage The usage to release.
     *
     */
    void releaseUsage(USAGEID usage)
    {
        typename std::map<USAGEID, REGISTERID>::iterator it_u;
        
        //  Search for the usage.
        it_u = reservedUsage.find(usage);
        
        //  Check if the usage was found.
        if(it_u != reservedUsage.end())
        {
            REGISTERID released;
            released = (*it_u).second;
            reservedUsage.erase(usage);
            reserved.erase(released);
        }
        else
            panic("RegisterBank", "releaseUsage", "Usage not reserved.");
    }

};

//
//
//  Helper functions used to operate with matrices.
//

/**
 *
 *  Creates an identity matrix.
 *
 *  @param dest Pointer to a matrix where to store the matrix.
 *
 */
void identityMatrix(D3DMATRIX *dest);

/**
 *
 *  Copy a matrix.
 *  
 *  @param dest Pointer to a matrix where to store the copy.
 *  @param source Pointer to the matrix to copy.
 *
 */
 
void copyMatrix(D3DMATRIX *dest, const D3DMATRIX* source);

/**
 *
 *  Multiply two matrices.
 *
 *  @param dest Pointer to a matrix where to store the multiplication result matrix.
 *  @param source_a Pointer to the first matrix to multiply.
 *  @param source_b Pointer to the second matrix to multiply.
 *
 */
 
void multiplyMatrix(D3DMATRIX *dest, const D3DMATRIX* source_a, const D3DMATRIX* source_b);

/**
 *
 *  Transpose a matrix.
 *
 *  @param dest Pointer to the matrix where to store the transposition result matrix.
 *  @param source Pointer to the matrix to transpose.
 *
 */
 
void transpose_matrix(D3DMATRIX *dest, const D3DMATRIX *source);


/**
 *
 *  Invert a matrix.
 *
 *  @param dest Pointer to the matrix where to store the inverted result matrix.
 *  @param source Pointer to the matrix to invert.
 *
 */
 
void invertMatrix(D3DMATRIX *dest, const D3DMATRIX *source);


//
//  Helper functions used to assemble D3D9 shader bytecode tokens.
//

/**
 *
 *  Assemble a D3D9 shader bytecode pixel shader version token.
 *
 *  @param _Major The shader version major number.
 *  @param _Minor The shader version minor number.
 *
 *  @return The assembled D3D9 shader bytecode pixel shader version token.
 *
 */
 
DWORD ver_ps_tk(UINT _Major,UINT _Minor);

/**
 *
 *  Assemble a D3D9 shader bytecode vertex shader version token.
 *
 *  @param _Major The shader version major number.
 *  @param _Minor The shader version minor number.
 *
 *  @return The assembled D3D9 shader bytecode vertex shader version token.
 *
 */

DWORD ver_vs_tk(UINT _Major,UINT _Minor);

/**
 *
 *  Assemble a D3D9 shader bytecode end token.
 *
 *  @return The assembled a D3D9 shader bytecode end token.
 *
 */

DWORD end_tk();

/**
 *
 *  Assemble a D3D9 shader bytecode instruction token.
 *
 *  @param opcode The instruction opcode.
 *  @param length The number of tokens associated with the instruction (arguments, results, ...), default 0
 *
 *  @return The assembled a D3D9 shader bytecode instruction token.
 *
 */

DWORD ins_tk(D3DSHADER_INSTRUCTION_OPCODE_TYPE opcode, BYTE length = 0);

/**
 *
 *  Assemble a D3D9 shader bytecode source parameter token.
 *
 *  @param reg Identifier of a D3D9 shader register.
 *  @param swz_x Swizzle for the the parameter x component, default x (0 -> x , 1 -> y , 2 -> z , 3 -> w).
 *  @param swz_y Swizzle for the the parameter y component, default y (0 -> x , 1 -> y , 2 -> z , 3 -> w).
 *  @param swz_z Swizzle for the the parameter z component, default z (0 -> x , 1 -> y , 2 -> z , 3 -> w).
 *  @param swz_w Swizzle for the the parameter w component, default w (0 -> x , 1 -> y , 2 -> z , 3 -> w).
 *  @param modifier The source parameter modifier, default none.
 *
 *  @return The assembled a D3D9 shader bytecode source parameter token.
 *
 */

DWORD src_tk(D3DRegisterId reg, 
             BYTE swz_x = 0, BYTE swz_y = 1, BYTE swz_z = 2, BYTE swz_w = 3,
             D3DSHADER_PARAM_SRCMOD_TYPE modifier = D3DSPSM_NONE );

/**
 *
 *  Assemble a D3D9 shader bytecode destination parameter token.
 *
 *  @param reg Identifier of a D3D9 shader register.
 *  @param wmx Mask for the parameter x component, default write (0 -> do not write, 1 -> write).
 *  @param wmy Mask for the parameter x component, default write (0 -> do not write, 1 -> write).
 *  @param wmz Mask for the parameter x component, default write (0 -> do not write, 1 -> write).
 *  @param wmw Mask for the parameter x component, default write (0 -> do not write, 1 -> write).
 *  @param modifier The destination parameter modifier, default none (?).
 *  @param shift The destination parameter shift, default none (?)
 *
 *  @return The assembled a D3D9 shader bytecode destination parameter token.
 *
 */

DWORD dst_tk(D3DRegisterId reg,  BYTE wmx = 1, BYTE wmy = 1, BYTE wmz = 1, BYTE wmw = 1, DWORD modifier = 0, DWORD shift = 0);


/**
 *
 *  Assemble a D3D9 shader bytecode semantic token.
 *
 *  @param usage The D3D9 semantic usage.
 *  @param usage_index The usage index.
 *
 *  @return The assembled a D3D9 shader bytecode semantic token.
 *
 */

DWORD sem_tk(UINT usage, UINT usage_index = 0);


/**
 *
 *  Assemble a D3D9 shader bytecode float point value token.
 *
 *  @param value A float point value.
 *
 *  @return The assembled a D3D9 shader bytecode float point value token.
 *
 */

DWORD flt_tk(FLOAT value);

/**
 *
 *  Assemble a D3D9 shader bytecode sampler definition token.
 *
 *  @param type The sampler type.
 *
 *  @return The assembled a D3D9 shader sampler definition token.
 *
 */

DWORD sam_tk(D3DSAMPLER_TEXTURE_TYPE type);

/**
 *
 *  Assemble a D3D9 shader bytecode comment token.
 *
 *  @param size Number of tokens/dwords in the comment.
 *
 *  @return The assembled a D3D9 shader comment token.
 *
 */

DWORD com_tk(DWORD size);

/**
 *
 *  Defines fixed function constant declarations in the generated shaders.
 *
 */
struct FFConstRegisterDeclaration
{
    D3DRegisterId constant;     /**<  D3D9 register identifier for the defined constant.  */
    FFUsageId usage;            /**<  Fixed function usage for the defined constant.  */
    
    /**
     *
     *  Constructor.  Sets defined values.
     *
     *  @param _constant The D3D9 register identifier for the constant to define.
     *  @param _usage The fixed function usage for the constant.
     *
     */
     
    FFConstRegisterDeclaration(D3DRegisterId _constant, FFUsageId _usage): constant(_constant), usage(_usage) {}
    
    /**
     *
     *  Constructor.  Empty.
     *
     */
     
    FFConstRegisterDeclaration() {}
};

/**
 *
 *  Stores the D3D9 shader generated to emulate fixed function and related information.
 *
 *
 */
struct FFGeneratedShader
{
    std::list<FFConstRegisterDeclaration> const_declaration;        /**<  List with the defined constants with fixed function usage.  */
    DWORD *code;    /**<  Pointer to the generated D3D9 shader bytecode.  */

    /**
     *
     *  Constructor.  Set with the defined values.
     *
     *  @param _const_declaration The list of constant declarations with fixed function usage.
     *  @param code Pointer to the generated D3D9 shader bytecode.
     * 
     */
    FFGeneratedShader(std::list<FFConstRegisterDeclaration> _const_declaration,
                      DWORD *_code): const_declaration(_const_declaration), code(_code) {}

    /**
     *
     *  Constructor.  Empty.
     *
     */                      
    FFGeneratedShader(): code(0) {}
    
    /**
     *
     *  Destructor.  Deletes code array if required.
     *
     */
     
    ~FFGeneratedShader()
    {
        if (code !=0)
            delete[] code;
    }
};

/**
 *
 *  Fixed function D3D9 shader generator.
 *
 *  Generates D3D9 shaders in bytecode format based on the defined D3D9 fixed function state.
 *
 */
class FFShaderGenerator
{
public:
    
    /**
     *
     *  Generate a D3D9 vertex shader (vs 3.0) for the defined D3D9 fixed function state.
     *
     *  @param _ff_state Defined fixed function state.
     *
     *  @return A pointer to the structure storing the generated vertex shader in shader bytecode format and related information.
     *
     */
     
    FFGeneratedShader *generate_vertex_shader(FFState _ff_state);

    /**
     *
     *  Generate a D3D9 pixel shader (ps 3.0) for the defined D3D9 fixed function state.
     *
     *  @param _ff_state Defined fixed function state.
     *
     *  @return A pointer to the structure storing the generated pixel shader in shader bytecode format and related information.
     *
     */

    FFGeneratedShader *generate_pixel_shader(FFState _ff_state);

private:

    FFState ff_state;       /**<  The current fixed function state.  */

    std::list<DWORD> def;   /**<  List with the D3D9 shader definition tokens.  */
    std::list<DWORD> dec;   /**<  List with the D3D9 shader declaration tokens.  */
    std::list<DWORD> cod;   /**<  List with the D3D9 shader code (instruction, comments?) tokens.  */
    
    RegisterBank<D3DRegisterId, D3DUsageId> input;      /**<  Register bank for inputs.  */
    RegisterBank<D3DRegisterId, D3DUsageId> output;     /**<  Register bank for outputs.  */
    RegisterBank<D3DRegisterId, D3DUsageId> samplers;   /**<  Register bank for samplers.  */
    RegisterBank<D3DRegisterId> temp;                   /**<  Register bank for temporary registers.  */
    RegisterBank<D3DRegisterId, FFUsageId> constant;    /**<  Register bank for constant registers.  */
    
    D3DRegisterId literals;     /**<  D3D9 register identifier for some basic constant values (0, 1, 0, 0).  */

    std::list<FFConstRegisterDeclaration> const_declaration;

    /**
     *
     *  Initialize register banks for vertex shader generation.
     *
     */
     
    void vsInitializeRegisterBanks();

    /**
     *
     *  Initialize register banks for pixel shader generation.
     *
     */
     
    void psInitializeRegisterBanks();

    /**
     *
     *  Generate vertex shader code for space transformations.
     *
     *  @param dst_N D3D9 register identifier for the normal.
     *  @param dst_V D3D9 register identifier for the position in viewer space (?).
     *  @param dst_P D3D9 regsiter identifier for the position in clip space.
     *
     */
     
    void vs_transform(D3DRegisterId dst_N, D3DRegisterId dst_V, D3DRegisterId dst_P);
    
    /**
     *
     *  Generate vertex shader code for vertex lighting.
     *
     *  @param src_N D3D9 register identifier for the normal.
     *  @param src_V D3D9 register identifier for the position in viewer space (?).
     *
     */
     
    void vs_lighting(D3DRegisterId src_N, D3DRegisterId src_V);
    
    /**
     *  
     *  Generate the vertex shader input declaration.
     *
     */
          
    void vs_input_declaration();
    
    /**
     *
     *  Generate vertex shader code for pretransformed position (usage POSITIONT).
     *
     */
     
    void vs_transformed_position();

    /**
     *
     *  Generate a comment.
     *
     *  @param text Pointer to a char string with the comment.
     *
     */
    
    void comment(char *text);
    
    /**
     *
     *  Generate code to negate a value.
     *
     *  @param dst D3D9 register where to store the negated value.
     *  @param src D3D9 register with the value to negate.
     *
     */
     
    void negate(D3DRegisterId dst, D3DRegisterId src);
    
    /**
     *
     *  Generate code to copy a value.
     *
     *  @param dst D3D9 register where to store the copied value.
     *  @param src D3D9 register with the value to copy.
     *
     */
     
    void mov(D3DRegisterId dst, D3DRegisterId src);
    
    /**
     *
     *  Generate code to normalize a 3-component vector.
     *
     *  @param dst D3D9 register where to store the normalized 3-component vector.
     *  @param src D3D9 register with the 3-component vector to normalize.
     *
     */

    void normalize(D3DRegisterId res, D3DRegisterId vec);
    
    /**
     *
     *  Generate code for a 3-component vector matrix multiplication.
     *
     *  @param dst D3D9 register where to store the result 3-component vector.
     *  @param src_vec D3D9 register with the 3-component input vector.
     *  @param src_mat0 D3D9 register with the first row of the matrix with whic to multiply.
     *  @param src_mat1 D3D9 register with the second row of the matrix with which to multiply.
     *  @param src_mat2 D3D9 register with the third row of the matrix with which to multiply.
     *
     */
     
    void mul_mat3_vec3(D3DRegisterId dst, D3DRegisterId src_vec, D3DRegisterId src_mat0,
                       D3DRegisterId src_mat1, D3DRegisterId src_mat2);

    /**
     *
     *  Generate code for a 4-component vector matrix multiplication.
     *
     *  @param dst D3D9 register where to store the result 4-component vector.
     *  @param src_vec D3D9 register with the 4-component input vector.
     *  @param src_mat0 D3D9 register with the first row of the matrix with whic to multiply.
     *  @param src_mat1 D3D9 register with the second row of the matrix with which to multiply.
     *  @param src_mat2 D3D9 register with the third row of the matrix with which to multiply.
     *  @param src_mat3 D3D9 register with the fourth row of the matrix with which to multiply.
     *
     */

    void mul_mat4_vec4(D3DRegisterId dst, D3DRegisterId src_vec, D3DRegisterId src_mat0,
                       D3DRegisterId src_mat1, D3DRegisterId src_mat2, D3DRegisterId src_mat3);
                       
    /**
     *
     *  Generate code for a 3-component vector 4x4 matrix multiplication.
     *
     *  @param dst D3D9 register where to store the result 3-component vector.
     *  @param src_vec D3D9 register with the 3-component input vector.
     *  @param src_mat0 D3D9 register with the first row of the matrix with whic to multiply.
     *  @param src_mat1 D3D9 register with the second row of the matrix with which to multiply.
     *  @param src_mat2 D3D9 register with the third row of the matrix with which to multiply.
     *  @param src_mat3 D3D9 register with the fourth row of the matrix with which to multiply.
     *
     */

    void mul_mat4_vec3(D3DRegisterId dst, D3DRegisterId src_vec, D3DRegisterId src_mat0,
                       D3DRegisterId src_mat1, D3DRegisterId src_mat2, D3DRegisterId src_mat3);
        
    /**
     *
     *  Checks if a usage is defined in the vertex declaration.
     *
     *  @param usage The D3D9 usage to check if it's defined in the current vertex declaration.
     *
     *  @return If the D3D9 usage is defined in the current vertex declaration.
     *
     */
     
    bool checkUsageInVertexDeclaration(D3DUsageId usage);


    /**
     *
     *  Generates a D3D9 shader bytecode source parameter token corresponding with the defined
     *  texture stage operation argument.
     *
     *  @param arg The texture stage operation argument.
     *  @param constant The texture stage constant color.
     *  @param current The temporary register holding the current color.
     *  @param defaultColor A constant register used for default oclor.
     *  @param tempReg A temporary register corresponding with the texture stages temp register.
     *  @param texture A temporary register holding the stag texture color.
     *  @param textFactorColor A constant register holding the texture factor color.
     *  @param textureFactorDefined A reference to a boolean register that stores if the texture factor
     *  constant was defined.  If the constant was not defined the function will define it and set
     *  the variable to true.
     *  @param diffuseDefined Vertex diffuse color output defined.
     *  @param specularDefined Vertex specular color output defined.
     * 
     *  @return A source parameter token corresponding with the defined texture stage operation
     *  argument and parameters.
     *
     */  
     
     
    DWORD genSourceTokenForTextureStageArg(u32bit arg, D3DCOLORVALUE constant, D3DRegisterId current,
                                           D3DRegisterId defaultColor, D3DRegisterId tempReg, D3DRegisterId texture,
                                           D3DRegisterId textFactorColor, bool &textureFactorDefined,
                                           bool diffuseDefined, bool specularDefined);
    

    /**
     *
     *  Generates a D3D9 shader bytecode destination parameter token for the corresponding result argument
     *  for the texture stage.
     *
     *  @param stage The texture stage.
     *  @param current The temporary register that holds the current color.
     *  @param tempReg The temporary register corresponding with texture stages temp register.
     *  @param maskX The write mask for the x component, default 1 (0 -> do not write, 1 -> write).
     *  @param maskY The write mask for the y component, default 1 (0 -> do not write, 1 -> write).
     *  @param maskZ The write mask for the z component, default 1 (0 -> do not write, 1 -> write).
     *  @param maskW The write mask for the w component, default 1 (0 -> do not write, 1 -> write).
     *
     *  @return A destination parameter token corresponding with the defined texture stage result argument.
     *
     */
     
    DWORD genDestTokenForTextureStage(u32bit stage, D3DRegisterId current, D3DRegisterId tempReg,
                                      BYTE maskX = 1, BYTE maskY = 1, BYTE maskZ = 1, BYTE maskW = 1);


    /**
     *
     *  Generates D3D9 shader bytecode to emulate the texture stage operation.
     *
     *  @param op The texture stage operation to emulate.
     *  @param arg1 The texture stage first operation argument.
     *  @param arg2 The texture stage second operation argument.
     *  @param arg3 The texture stage third operation argument.
     *  @param sourceToken1 The D3D9 shader bytecode token for the first operation argument.
     *  @param sourceToken2 The D3D9 shader bytecode token for the second operation argument.
     *  @param sourceToken3 The D3D9 shader bytecode token for the third operation argument.
     *  @param destToken The D3D9 shader bytecode token for the result argument.
     *  @param defaultColor A constant register with immediate value.
     *
     */

    void genCodeForTextureStageOp(D3DTEXTUREOP op, u32bit arg1, u32bit arg2, u32bit arg3,
                                  DWORD sourceToken1, DWORD sourceToken2, DWORD sourceToken3, DWORD destToken,
                                  D3DRegisterId defaultColor);
};

#endif

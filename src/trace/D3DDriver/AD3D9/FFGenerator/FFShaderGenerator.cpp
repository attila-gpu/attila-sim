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

#include "FFShaderGenerator.h"
#include "Utils.h"

#include <cstring>

using namespace std;

//  Initialize Texture Stage state with default values.
TextureStageState::TextureStageState()
{
    colorOp = D3DTOP_MODULATE;
    colorArg0 = D3DTA_CURRENT;
    colorArg1 = D3DTA_TEXTURE;
    colorArg2 = D3DTA_CURRENT;
    alphaOp = D3DTOP_SELECTARG1;
    alphaArg0 = D3DTA_CURRENT;
    alphaArg1 = D3DTA_DIFFUSE;
    alphaArg2 = D3DTA_CURRENT;
    bumpEnvMatrix[0][0] = bumpEnvMatrix[0][1] = bumpEnvMatrix[1][0] = bumpEnvMatrix[1][1] = 0.0f;
    bumpEnvLScale = 0.0f;
    bumpEnvLOffset = 0.0f;
    index = 0;
    transformFlags = D3DTTFF_DISABLE;
    resultArg = D3DTA_CURRENT;
    constant.r = constant.g = constant.b = constant.a = 0.0f;
}   


//  Initialize fixed function state with default values.
FFState::FFState()
{
    //  Set default fixed function vertex declaration.
    fvf = 0;

    //  Set transform matrices to identity.
    identityMatrix(&world);
    identityMatrix(&view);
    identityMatrix(&projection);
    
    //  Set texture coordinate matrices to identity.
    for(u32bit t = 0; t < 8; t++)
        identityMatrix(&texture[t]);

    // Vertex blending/tweening.
    vertexBlend = D3DVBF_DISABLE;
    indexedVertexBlend = false;
    tweenFactor = 0.0f;
    
    //  Set lighting and vertex color to default values.
    lightingEnabled = true;
    vertexColor = true;
    specularEnable = false;
    localViewer = true;
    normalizeNormals = false;
    diffuseMaterialSource = D3DMCS_COLOR1;
    specularMaterialSource = D3DMCS_COLOR2;
    ambientMaterialSource = D3DMCS_MATERIAL;
    emissiveMaterialSource = D3DMCS_MATERIAL;
    ambient.r = ambient.g = ambient.b = ambient.a = 0.0f;
    material.Ambient.r = material.Ambient.b = material.Ambient.g = material.Ambient.a = 0.0f;
    material.Diffuse.r = material.Diffuse.b = material.Diffuse.g = material.Diffuse.a = 0.0f;
    material.Specular.r = material.Specular.b = material.Specular.g = material.Specular.a = 0.0f;
    material.Emissive.r = material.Emissive.b = material.Emissive.g = material.Emissive.a = 0.0f;
    material.Power = 0.0f;
    for(u32bit i = 0; i < 8; i ++)
    {
        lightsEnabled[i] = false;
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
    
    //  Set fog to default state.
    fogEnable = false;
    fogColor.r = fogColor.g = 
    fogColor.b = fogColor.a = 0;
    fogPixelMode = D3DFOG_NONE;
    fogVertexMode = D3DFOG_NONE;
    fogDensity = 1.0f;
    fogStart = 0.0f;
    fogEnd = 1.0f;
    fogRange = false;

    //  Reset the defined textures.
    for(u32bit t = 0; t < 8; t++)    
        settedTexture[t] = false;    

    //  Reset the texture stages state.
    for(u32bit ts = 1; ts < 8; ts++)    
    {
        textureStage[ts].colorOp = D3DTOP_DISABLE;
        textureStage[ts].alphaOp = D3DTOP_DISABLE;
        textureStage[ts].index = ts;
    }
    
    //  Reset value for the render state texture factor.
    textureFactor.r = textureFactor.g = textureFactor.b = textureFactor.a = 1.0f;
    
}

//  Create identity matrix.
void identityMatrix(D3DMATRIX *dest)
{
    for (UINT i = 0; i < 4; i ++)
        for (UINT j = 0; j < 4; j ++)
            if (i == j)
                dest->m[i][j] = 1.0f;
            else
                dest->m[i][j] = 0.0f;
}

//  Copy matrix.
void copyMatrix(D3DMATRIX *dest, const D3DMATRIX* source)
{
    for (UINT i = 0; i < 4; i ++)
        for (UINT j = 0; j < 4; j ++)
            dest->m[i][j] = source->m[i][j];
}

//  Multiply two matrices.
void multiplyMatrix(D3DMATRIX *dest, const D3DMATRIX* source_a, const D3DMATRIX* source_b)
{
    D3DMATRIX temp;

    for (UINT i = 0; i < 4; i ++)
    {
        for (UINT j = 0; j < 4; j ++)
        {
            float dot = 0;
            for (UINT k = 0; k < 4; k ++)
                dot += source_a->m[i][k] * source_b->m[k][j];
            temp.m[i][j] = dot;
        }
    }
    
    copyMatrix(dest, &temp);
}

//  Transpose a matrix.
void transposeMatrix(D3DMATRIX *dest, const D3DMATRIX *source)
{
    D3DMATRIX temp;
    for (UINT i = 0; i < 4; i ++)
        for (UINT j = 0; j < 4; j ++)
            temp.m[j][i] = source->m[i][j];
            
    copyMatrix(dest, &temp);
}

//  Invert a matrix.
void invertMatrix(D3DMATRIX *dest, const D3DMATRIX *source)
{
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

  copyMatrix(dest, &temp);
}


//  Assemble pixel shader version token.
DWORD  ver_ps_tk(UINT _Major,UINT _Minor)
{
    return (0xFFFF0000 | (_Major << 8) | _Minor);
}

//  Assemble vertex shader version token.
DWORD  ver_vs_tk(UINT _Major,UINT _Minor)
{
    return (0xFFFE0000 | (_Major << 8) | _Minor);
}

//  Create end token.
DWORD  end_tk()
{
    return 0x0000FFFF;
}

//  Create instruction token.
DWORD  ins_tk(D3DSHADER_INSTRUCTION_OPCODE_TYPE opcode, BYTE length)
{
    return (0x00000000 | opcode | (D3DSI_INSTLENGTH_MASK & (length << D3DSI_INSTLENGTH_SHIFT)));
}

//  Create source parameter token.
DWORD  src_tk(D3DRegisterId reg, 
             BYTE swz_x, BYTE swz_y, BYTE swz_z, BYTE swz_w,
             D3DSHADER_PARAM_SRCMOD_TYPE modifier)
{    
    return (0x80000000 |
            (reg.num & D3DSP_REGNUM_MASK) |
            ((reg.type << D3DSP_REGTYPE_SHIFT)  & D3DSP_REGTYPE_MASK) |
            ((reg.type << D3DSP_REGTYPE_SHIFT2) & D3DSP_REGTYPE_MASK2) |
            (swz_x << D3DSP_SWIZZLE_SHIFT) |
            (swz_y << (D3DSP_SWIZZLE_SHIFT + 2)) |
            (swz_z << (D3DSP_SWIZZLE_SHIFT + 4)) |
            (swz_w << (D3DSP_SWIZZLE_SHIFT + 6)) |
            (modifier  & D3DSP_SRCMOD_MASK));
}

//  Create destination parameter token.
DWORD  dst_tk(D3DRegisterId reg, BYTE wmx, BYTE wmy, BYTE wmz, BYTE wmw,
              DWORD modifier, DWORD shift)
{
    return (0x80000000 |
            (reg.num & D3DSP_REGNUM_MASK) |
            ((reg.type << D3DSP_REGTYPE_SHIFT)  & D3DSP_REGTYPE_MASK) |
            ((reg.type << D3DSP_REGTYPE_SHIFT2) & D3DSP_REGTYPE_MASK2) |
            (wmx << 16) | (wmy << (16 + 1)) | (wmz << (16 + 2)) | (wmw << (16 + 3)) |
            (modifier  & D3DSP_DSTMOD_MASK));
}

//  Create semantic usage token.
DWORD  sem_tk(UINT usage, UINT usage_index)
{
    return (0x80000000 | (usage << D3DSP_DCL_USAGE_SHIFT) | (usage_index <<  D3DSP_DCL_USAGEINDEX_SHIFT));
}

//  Create float point value token.
DWORD  flt_tk(FLOAT value)
{
    return (*reinterpret_cast<DWORD *>(&value));
}

//  Create sampler descriptor token.
DWORD  sam_tk(D3DSAMPLER_TEXTURE_TYPE type)
{
    return (0x80000000 | type);
}

//  Create comment token.
DWORD  com_tk(DWORD size)
{
    return(0x0000FFFE | ((size << D3DSI_COMMENTSIZE_SHIFT) & D3DSI_COMMENTSIZE_MASK));
}


//  Generate a vertex shader for the current fixed function state.
FFGeneratedShader *FFShaderGenerator::generate_vertex_shader(FFState _ff_state)
{
    ff_state = _ff_state;

    def.clear();
    dec.clear();
    cod.clear();

    const_declaration.clear();

    vsInitializeRegisterBanks();

    //  Add vs 3.0 version token.
    dec.push_back(ver_vs_tk(3,0));
    
    vs_input_declaration();

    //  Check for pre-transformed position.
    if ((ff_state.fvf & D3DFVF_XYZRHW) || checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_POSITIONT, 0)))
    {
        vs_transformed_position();
    }
    else
    {
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
    }
    
    // Propagate texcoords
    for(BYTE i = 0; i < 8; i ++)
    {
        if (fvf_has_texcoord(i, ff_state.fvf) || checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i)))
        {
            cod.push_back(ins_tk(D3DSIO_MOV, 2));
            cod.push_back(dst_tk(output.findUsage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i))));
            cod.push_back(src_tk(input.findUsage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i))));
        }
    }

    //  Add end token.
    cod.push_back(end_tk());

    //  Assemble the whole shader program.
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

//  Generate a pixel shader for the current fixed function state.
FFGeneratedShader *FFShaderGenerator::generate_pixel_shader(FFState _ff_state)
{

    ff_state = _ff_state;

    psInitializeRegisterBanks();

    const_declaration.clear();

    D3DRegisterId reg;

    dec.clear();
    def.clear();
    cod.clear();

    dec.push_back(ver_ps_tk(3, 0));

    //  Declare the samplers used.
    bool endStage = false;
    D3DRegisterId sampler;
    for(u32bit s = 0; !endStage && (s < 8); s++)
    {
        //  Check if the stage is disabled.
        endStage = (ff_state.textureStage[s].colorOp == D3DTOP_DISABLE);
        
        //  Check if the texture is defined and the stage is not disabled.
        if (ff_state.settedTexture[s] && !endStage)
        {
            sampler = samplers.reserve();
            dec.push_back(ins_tk(D3DSIO_DCL, 2));            
            dec.push_back(sam_tk(ff_state.textureType[s]));
            dec.push_back(dst_tk(sampler));
        }
    }
    
    bool diffuseDefined = false;
    
    //  Check vertex diffuse color if present in the vertex declaration.
    if ((ff_state.fvf & D3DFVF_DIFFUSE) || checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_COLOR, 0)))
    {
        reg = input.reserveUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0));
        dec.push_back(ins_tk(D3DSIO_DCL, 2));
        dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 0));
        dec.push_back(dst_tk(reg));
        
        diffuseDefined = true;
    }
    
    bool specularDefined = false;
    
    //  Check for vertex specular color.
    if (ff_state.specularEnable)
    {
        reg = input.reserveUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 1));
        dec.push_back(ins_tk(D3DSIO_DCL, 2));
        dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 1));
        dec.push_back(dst_tk(reg));
        
        specularDefined = true;
    }
    
    //  Declare texture coordinates.
    for(u32bit s = 0; s < 8; s++)
    {
        //  Check for the texture coordinate in the vertex declaration.
        if (fvf_has_texcoord(s, ff_state.fvf) || checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_TEXCOORD, s)))
        {
            reg = input.reserveUsage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, s));
            dec.push_back(ins_tk(D3DSIO_DCL, 2));
            dec.push_back(sem_tk(D3DDECLUSAGE_TEXCOORD, s));
            dec.push_back(dst_tk(reg));
        }
    }

    // Define a constant for default colors.
    D3DRegisterId defaultColor = constant.reserve();
    def.push_back(ins_tk(D3DSIO_DEF, 5));
    def.push_back(dst_tk(defaultColor));
    def.push_back(flt_tk(0.0f));
    def.push_back(flt_tk(0.0f));
    def.push_back(flt_tk(0.0f));
    def.push_back(flt_tk(1.0f));
    
    //  Reserve a temporary register for the current stage color.
    D3DRegisterId current = temp.reserve();

    //  Reserve a temporary register for the current texture color. 
    D3DRegisterId texture = temp.reserve();

    //  Reserve a temporary register for the temp fixed function register.
    D3DRegisterId tempReg = temp.reserve();
    
    //  Check vertex diffuse color if present in the declaration.
    if ((ff_state.fvf & D3DFVF_DIFFUSE) || checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_COLOR, 0)))
    {
        //  mov current, diffuseColor
        cod.push_back(ins_tk(D3DSIO_MOV, 2));
        cod.push_back(dst_tk(current));
        cod.push_back(src_tk(input.findUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));
    }
    else
    {
        //  mov current, (1, 1, 1, 1)
        cod.push_back(ins_tk(D3DSIO_MOV, 2));
        cod.push_back(dst_tk(current));
        cod.push_back(src_tk(defaultColor, 3, 3, 3, 3));
    }
    
    bool textureFactorDefined = false;
    D3DRegisterId textFactorColor;
    
    u32bit currentStage = 0;
    
    endStage = (ff_state.textureStage[currentStage].colorOp == D3DTOP_DISABLE);;
    
    while (!endStage)
    {
        //  Check for triadic operations that enable the third argument.
        bool threeArgsForColorOp = (ff_state.textureStage[currentStage].colorOp == D3DTEXOPCAPS_MULTIPLYADD) ||
                                   (ff_state.textureStage[currentStage].colorOp == D3DTEXOPCAPS_LERP);
        bool threeArgsForAlphaOp = (ff_state.textureStage[currentStage].alphaOp == D3DTEXOPCAPS_MULTIPLYADD) ||
                                   (ff_state.textureStage[currentStage].alphaOp == D3DTEXOPCAPS_LERP);
                                   

        //  Check if the texture is read in the current stage.
        if ((ff_state.textureStage[currentStage].colorArg1 == D3DTA_TEXTURE) ||
            (ff_state.textureStage[currentStage].colorArg2 == D3DTA_TEXTURE) ||
            (ff_state.textureStage[currentStage].alphaArg1 == D3DTA_TEXTURE) ||
            (ff_state.textureStage[currentStage].alphaArg2 == D3DTA_TEXTURE) ||
            (threeArgsForColorOp && (ff_state.textureStage[currentStage].colorArg0 == D3DTA_TEXTURE)) ||
            (threeArgsForAlphaOp && (ff_state.textureStage[currentStage].colorArg0 == D3DTA_TEXTURE)))
        {
        
            //  Check if a texture was set for the stage.
            if (ff_state.settedTexture[currentStage])
            {
                //  tex texture, texcoord[stage], sample[stage]
                cod.push_back(ins_tk(D3DSIO_TEX, 3));
                cod.push_back(dst_tk(texture));
                cod.push_back(src_tk(input.findUsage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, (ff_state.textureStage[currentStage].index & 0x07)))));
                cod.push_back(src_tk(D3DRegisterId(currentStage, D3DSPR_SAMPLER)));
            }
            else
            {
                //  mov texture, (0, 0, 0, 1)
                cod.push_back(ins_tk(D3DSIO_MOV, 2));
                cod.push_back(dst_tk(texture));
                cod.push_back(src_tk(defaultColor));
            }
        }
        
        //  Check if the same operation is defined for color and alpha components.
        if ((ff_state.textureStage[currentStage].colorOp == ff_state.textureStage[currentStage].alphaOp) &&
            (ff_state.textureStage[currentStage].colorArg1 == ff_state.textureStage[currentStage].alphaArg1) &&
            (ff_state.textureStage[currentStage].colorArg2 == ff_state.textureStage[currentStage].alphaArg2) &&
            (!threeArgsForColorOp || (ff_state.textureStage[currentStage].colorArg0 == ff_state.textureStage[currentStage].alphaArg0)))
        {
            //  Create the source parameters for the operation.
            DWORD source1Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].colorArg1,
                                                                  ff_state.textureStage[currentStage].constant,
                                                                  current, defaultColor, tempReg, texture,
                                                                  textFactorColor, textureFactorDefined,
                                                                  diffuseDefined, specularDefined);
                                                                  
            DWORD source2Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].colorArg2,
                                                                  ff_state.textureStage[currentStage].constant,
                                                                  current, defaultColor, tempReg, texture,
                                                                  textFactorColor, textureFactorDefined,
                                                                  diffuseDefined, specularDefined);
            DWORD source3Token = 0;            
            if (threeArgsForColorOp)
            {
                source3Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].colorArg0,
                                                                ff_state.textureStage[currentStage].constant,
                                                                current, defaultColor, tempReg, texture,
                                                                textFactorColor, textureFactorDefined,
                                                                diffuseDefined, specularDefined);
            }

            //  Create destination token for the operation.
            DWORD destToken = genDestTokenForTextureStage(currentStage, current, texture);
            
            //  Generate the corresponding instructions for the operation.
            genCodeForTextureStageOp(ff_state.textureStage[currentStage].colorOp,
                                     ff_state.textureStage[currentStage].colorArg1,
                                     ff_state.textureStage[currentStage].colorArg2,
                                     ff_state.textureStage[currentStage].colorArg0,
                                     source1Token, source2Token, source3Token, destToken,
                                     defaultColor);
            
        }
        else
        {
            //  Create the source parameters for the color operation.
            DWORD source1Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].colorArg1,
                                                                  ff_state.textureStage[currentStage].constant,
                                                                  current, defaultColor, tempReg, texture,
                                                                  textFactorColor, textureFactorDefined,
                                                                  diffuseDefined, specularDefined);
                                                                  
            DWORD source2Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].colorArg2,
                                                                  ff_state.textureStage[currentStage].constant,
                                                                  current, defaultColor, tempReg, texture,
                                                                  textFactorColor, textureFactorDefined,
                                                                  diffuseDefined, specularDefined);
            DWORD source3Token = 0;            
            if (threeArgsForColorOp)
            {
                source3Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].colorArg0,
                                                                ff_state.textureStage[currentStage].constant,
                                                                current, defaultColor, tempReg, texture,
                                                                textFactorColor, textureFactorDefined,
                                                                diffuseDefined, specularDefined);
            }

            //  Create destination token for the color operation.
            DWORD destToken = genDestTokenForTextureStage(currentStage, current, texture, 1, 1, 1, 0);
            
            //  Generate the corresponding instructions for the operation.
            genCodeForTextureStageOp(ff_state.textureStage[currentStage].colorOp,
                                     ff_state.textureStage[currentStage].colorArg1,
                                     ff_state.textureStage[currentStage].colorArg2,
                                     ff_state.textureStage[currentStage].colorArg0,
                                     source1Token, source2Token, source3Token, destToken,
                                     defaultColor);

            //  Create the source parameters for the alpha operation.
            source1Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].alphaArg1,
                                                                  ff_state.textureStage[currentStage].constant,
                                                                  current, defaultColor, tempReg, texture,
                                                                  textFactorColor, textureFactorDefined,
                                                                  diffuseDefined, specularDefined);
                                                                  
            source2Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].alphaArg2,
                                                                  ff_state.textureStage[currentStage].constant,
                                                                  current, defaultColor, tempReg, texture,
                                                                  textFactorColor, textureFactorDefined,
                                                                  diffuseDefined, specularDefined);
            source3Token = 0;            
            if (threeArgsForColorOp)
            {
                source3Token = genSourceTokenForTextureStageArg(ff_state.textureStage[currentStage].alphaArg0,
                                                                ff_state.textureStage[currentStage].constant,
                                                                current, defaultColor, tempReg, texture,
                                                                textFactorColor, textureFactorDefined,
                                                                diffuseDefined, specularDefined);
            }

            //  Create destination token for the color operation.
            destToken = genDestTokenForTextureStage(currentStage, current, texture, 0, 0, 0, 1);
            
            //  Generate the corresponding instructions for the operation.
            genCodeForTextureStageOp(ff_state.textureStage[currentStage].alphaOp,
                                     ff_state.textureStage[currentStage].alphaArg1,
                                     ff_state.textureStage[currentStage].alphaArg2,
                                     ff_state.textureStage[currentStage].alphaArg0,
                                     source1Token, source2Token, source3Token, destToken,
                                     defaultColor);
        }

        currentStage++;
        
        endStage = (currentStage < 7) && (ff_state.textureStage[currentStage].colorOp == D3DTOP_DISABLE);;
    }
                
    //  Check if specular color is enabled.
    if (specularDefined)
    {
        //  Add the specular color to the result of the texture stages.  The result of the texture stages
        //  is expected to be in the current register.
        cod.push_back(ins_tk(D3DSIO_ADD, 3));
        cod.push_back(dst_tk(D3DRegisterId(0, D3DSPR_COLOROUT)));
        cod.push_back(src_tk(current));
        cod.push_back(src_tk(input.findUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 1))));
    }
    else
    {
        //  Copy result to output color register.  The result of the texture stages is expected to be
        //  in the current register.
        cod.push_back(ins_tk(D3DSIO_MOV, 2));
        cod.push_back(dst_tk(D3DRegisterId(0, D3DSPR_COLOROUT)));
        cod.push_back(src_tk(current));
    }
        
    cod.push_back(end_tk());

    //  Release temporary registers.
    temp.release(current);
    temp.release(texture);
    temp.release(tempReg);


    //  Assemble the whole shader program.
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

//  Generate a source token for a texture stage argument.
DWORD FFShaderGenerator::genSourceTokenForTextureStageArg(u32bit arg, D3DCOLORVALUE texStageConstantColor, D3DRegisterId current,
                                                          D3DRegisterId defaultColor, D3DRegisterId tempReg, D3DRegisterId texture,
                                                          D3DRegisterId textFactorColor, bool &textureFactorDefined,
                                                          bool diffuseDefined, bool specularDefined)
{
    DWORD sourceToken = 0;
    D3DRegisterId constantColor;
    
    switch(arg)
    {
        case D3DTA_CONSTANT:
        
            //  Define a constant for the texture stage constant color if required.
            constantColor = constant.reserve();
            def.push_back(ins_tk(D3DSIO_DEF, 5));
            def.push_back(dst_tk(constantColor));
            def.push_back(flt_tk(texStageConstantColor.r));
            def.push_back(flt_tk(texStageConstantColor.g));
            def.push_back(flt_tk(texStageConstantColor.b));
            def.push_back(flt_tk(texStageConstantColor.a));
            
            sourceToken = src_tk(constantColor);
            
            break;

        case D3DTA_CURRENT:
        
            sourceToken = src_tk(current);
            break;
            
        case D3DTA_DIFFUSE:
        
            if (diffuseDefined)
                sourceToken = src_tk(input.findUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0)));
            else
                sourceToken = src_tk(defaultColor, 3, 3, 3, 3);
                
            break;
            
        case D3DTA_SPECULAR:

            if (specularDefined)
                sourceToken = src_tk(input.findUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 1)));
            else
                sourceToken = src_tk(defaultColor, 3, 3, 3, 3);

            break;

        case D3DTA_TEMP:
        
            sourceToken = src_tk(tempReg);
            break;
            
        case D3DTA_TEXTURE:
        
            sourceToken = src_tk(texture);
            break;
            
        case D3DTA_TFACTOR:
        
            //  Check if the constant for the texture factor was already defined.
            if (!textureFactorDefined)
            {
                //  Define a constant for the texture factor color.
                textFactorColor = constant.reserve();
                def.push_back(ins_tk(D3DSIO_DEF, 5));
                def.push_back(dst_tk(textFactorColor));
                def.push_back(flt_tk(ff_state.textureFactor.r));
                def.push_back(flt_tk(ff_state.textureFactor.g));
                def.push_back(flt_tk(ff_state.textureFactor.b));
                def.push_back(flt_tk(ff_state.textureFactor.a));
            
                textureFactorDefined = true;
            }
            
            sourceToken = src_tk(textFactorColor);
            
            break;
        
        default:
            panic("FFShaderGenerator", "genSourceTokenForTextureStageArg", "Undefined operation argument.");
            break;
    }

    return sourceToken;
}

//  Generate result token for texture stage.
DWORD FFShaderGenerator::genDestTokenForTextureStage(u32bit stage, D3DRegisterId current, D3DRegisterId tempReg,
                                                     BYTE maskX, BYTE maskY, BYTE maskZ, BYTE maskW)
{
    DWORD destToken = 0;
    
    switch(ff_state.textureStage[stage].resultArg)
    {
        case D3DTA_CURRENT:
        
            destToken = dst_tk(current, maskX, maskY, maskZ, maskW);
            break;
            
        case D3DTA_TEMP:
        
            destToken = dst_tk(tempReg, maskX, maskY, maskZ, maskW);
            break;
            
        default:
            panic("FFShaderGenerator", "genDestTokenForTextureStage", "Undefined operation result argument.");
            break;
    }

    return destToken;
}

//  Generate code to emulate a texture stage operation.
void FFShaderGenerator::genCodeForTextureStageOp(D3DTEXTUREOP op, u32bit arg1, u32bit arg2, u32bit arg3,
                                                 DWORD sourceToken1, DWORD sourceToken2, DWORD sourceToken3, DWORD destToken,
                                                 D3DRegisterId defaultColor)
{

    bool threeArgOperation = (op == D3DTEXOPCAPS_MULTIPLYADD) || (op == D3DTEXOPCAPS_LERP);
    
    D3DRegisterId tempArg1;
    D3DRegisterId tempArg2;
    D3DRegisterId tempArg3;
    bool tempArg1Defined = false;
    bool tempArg2Defined = false;
    bool tempArg3Defined = false;
    
    D3DRegisterId tempRes;
    bool tempResDefined = false;
    
    //  Check for alpha replication argument modifier for first argument.
    if ((arg1 & D3DTA_ALPHAREPLICATE) != 0)
    {
        //  Change the swizzling of the source token to replicate the alpha component.
        sourceToken1 = (sourceToken1 & ~D3DSP_SWIZZLE_MASK) | D3DSP_REPLICATEALPHA;
    }
    
    //  Check for complement argument modifier for first argument.
    if ((arg1 & D3DTA_COMPLEMENT) != 0)
    {
        //  Reserve a temporary register for the complement.
        tempArg1 = temp.reserve();
        tempArg1Defined = true;
   
        //  Compute the complement: 1 - arg.
        cod.push_back(ins_tk(D3DSIO_ADD, 3));
        cod.push_back(dst_tk(tempArg1));
        cod.push_back(src_tk(defaultColor, 3, 3, 3, 3));
        cod.push_back((sourceToken1 & ~D3DSP_SRCMOD_MASK) | D3DSPSM_NEG);
        
        //  Make the source token point to the temporary register.
        sourceToken1 = src_tk(tempArg1);
    }

    //  Check for alpha replication argument modifier for second argument.
    if ((arg2 & D3DTA_ALPHAREPLICATE) != 0)
    {
        //  Change the swizzling of the source token to replicate the alpha component.
        sourceToken2 = (sourceToken2 & ~D3DSP_SWIZZLE_MASK) | D3DSP_REPLICATEALPHA;
    }
    
    //  Check for complement argument modifier for first argument.
    if ((arg2 & D3DTA_COMPLEMENT) != 0)
    {
        //  Reserve a temporary register for the complement.
        tempArg2 = temp.reserve();
        tempArg2Defined = true;
   
        //  Compute the complement: 1 - arg.
        cod.push_back(ins_tk(D3DSIO_ADD, 3));
        cod.push_back(dst_tk(tempArg2));
        cod.push_back(src_tk(defaultColor, 3, 3, 3, 3));
        cod.push_back((sourceToken2 & ~D3DSP_SRCMOD_MASK) | D3DSPSM_NEG);
        
        //  Make the source token point to the temporary register.
        sourceToken2 = src_tk(tempArg2);
    }

    //  Check for alpha replication argument modifier for third argument.
    if (threeArgOperation && ((arg2 & D3DTA_ALPHAREPLICATE) != 0))
    {
        //  Change the swizzling of the source token to replicate the alpha component.
        sourceToken3 = (sourceToken3 & ~D3DSP_SWIZZLE_MASK) | D3DSP_REPLICATEALPHA;
    }
    
    //  Check for complement argument modifier for first argument.
    if (threeArgOperation && ((arg3 & D3DTA_COMPLEMENT) != 0))
    {
        //  Reserve a temporary register for the complement.
        tempArg3 = temp.reserve();
        tempArg3Defined = true;
   
        //  Compute the complement: 1 - arg.
        cod.push_back(ins_tk(D3DSIO_ADD, 3));
        cod.push_back(dst_tk(tempArg3));
        cod.push_back(src_tk(defaultColor, 3, 3, 3, 3));
        cod.push_back((sourceToken3 & ~D3DSP_SRCMOD_MASK) | D3DSPSM_NEG);
        
        //  Make the source token point to the temporary register.
        sourceToken3 = src_tk(tempArg3);
    }
    
    //  Generate the code for the operation.
    switch(op)
    {
        case D3DTOP_DISABLE:
            panic("FFShaderGenerator", "genCodeForTextureStageOp", "Disable is not a correct operation.");
            break;
            
        case D3DTOP_SELECTARG1:

            //  mov dest, arg1            
            cod.push_back(ins_tk(D3DSIO_MOV, 2));
            cod.push_back(destToken);
            cod.push_back(sourceToken1);
            
            break;
            
        case D3DTOP_SELECTARG2:

            //  mov dest, arg2
            cod.push_back(ins_tk(D3DSIO_MOV, 2));
            cod.push_back(destToken);
            cod.push_back(sourceToken2);
            
            break;
            
        case D3DTOP_MODULATE:
        
            //  mul dest, arg1, arg2
            cod.push_back(ins_tk(D3DSIO_MUL, 3));
            cod.push_back(destToken);
            cod.push_back(sourceToken1);
            cod.push_back(sourceToken2);
            
            break;
            
        case D3DTOP_MODULATE2X:
        
            //  Reserve a temporary register for the result.
            tempRes = temp.reserve();
            tempResDefined = true;
            
            //  mul temp, arg1, arg2
            cod.push_back(ins_tk(D3DSIO_MUL, 3));
            cod.push_back(dst_tk(tempRes));
            cod.push_back(sourceToken1);
            cod.push_back(sourceToken2);
            
            //  add dest, temp, temp
            cod.push_back(ins_tk(D3DSIO_ADD, 3));
            cod.push_back(destToken);
            cod.push_back(src_tk(tempRes));
            cod.push_back(src_tk(tempRes));
            
            break;
            
        case D3DTOP_MODULATE4X:
        
            //  Reserve a temporary register for the result.
            tempRes = temp.reserve();
            tempResDefined = true;
            
            //  mul temp, arg1, arg2
            cod.push_back(ins_tk(D3DSIO_MUL, 3));
            cod.push_back(dst_tk(tempRes));
            cod.push_back(sourceToken1);
            cod.push_back(sourceToken2);
            
            //  add temp, temp, temp
            cod.push_back(ins_tk(D3DSIO_ADD, 3));
            cod.push_back(dst_tk(tempRes));
            cod.push_back(src_tk(tempRes));
            cod.push_back(src_tk(tempRes));

            //  add dest, temp, temp
            cod.push_back(ins_tk(D3DSIO_ADD, 3));
            cod.push_back(destToken);
            cod.push_back(src_tk(tempRes));
            cod.push_back(src_tk(tempRes));
            
            break;            

        case D3DTOP_ADD:
            
            //  add dest, arg1, arg2
            cod.push_back(ins_tk(D3DSIO_ADD, 3));
            cod.push_back(destToken);
            cod.push_back(sourceToken1);
            cod.push_back(sourceToken2);
            
            break;
            
        case D3DTOP_SUBTRACT:
        
            //  add dest, arg1, -arg2
            cod.push_back(ins_tk(D3DSIO_ADD, 3));
            cod.push_back(destToken);
            cod.push_back(sourceToken1);
            cod.push_back((sourceToken2 & ~D3DSP_SRCMOD_MASK) | D3DSPSM_NEG);
            break;

        case D3DTOP_ADDSIGNED:
        case D3DTOP_ADDSIGNED2X:
        case D3DTOP_ADDSMOOTH:
        case D3DTOP_BLENDDIFFUSEALPHA:
        case D3DTOP_BLENDTEXTUREALPHA:
        case D3DTOP_BLENDFACTORALPHA:
        case D3DTOP_BLENDTEXTUREALPHAPM:
        case D3DTOP_BLENDCURRENTALPHA:
        case D3DTOP_PREMODULATE:
        case D3DTOP_MODULATEALPHA_ADDCOLOR:
        case D3DTOP_MODULATECOLOR_ADDALPHA:
        case D3DTOP_MODULATEINVALPHA_ADDCOLOR:
        case D3DTOP_MODULATEINVCOLOR_ADDALPHA:
        case D3DTOP_BUMPENVMAP:
        case D3DTOP_BUMPENVMAPLUMINANCE:
        case D3DTOP_DOTPRODUCT3:
        case D3DTOP_MULTIPLYADD:
        case D3DTOP_LERP:
            panic("FFShaderGenerator", "genCodeForTextureStageOp", "Unimplemented operation");
            break;

        default:
            panic("FFShaderGenerator", "genCodeForTextureStageOp", "Undefined texture operation");
            break;
    }
    
    
    //  Release temp registers.
    if (tempArg1Defined)
        temp.release(tempArg1);
    if (tempArg2Defined)
        temp.release(tempArg2);
    if (tempArg3Defined)
        temp.release(tempArg3);
    if (tempResDefined)
        temp.release(tempRes);
    
}


void FFShaderGenerator::vsInitializeRegisterBanks()
{
    input.clear();
    output.clear();
    temp.clear();
    constant.clear();

    for (DWORD i = 0; i < 16; i++)
        input.insert(D3DRegisterId(i, D3DSPR_INPUT));

    for (DWORD i = 0; i < 12; i++)
        output.insert(D3DRegisterId(i, D3DSPR_OUTPUT));

    for (DWORD i = 0; i < 32; i++)
        temp.insert(D3DRegisterId(i, D3DSPR_TEMP));

    //  Use constant registers not corrently in use by the implementation shader pipeline.
    for (DWORD i = 288; i < 512; i++)
        constant.insert(D3DRegisterId(i, D3DSPR_CONST));
}

void FFShaderGenerator::psInitializeRegisterBanks()
{
    input.clear();
    temp.clear();
    constant.clear();
    for (DWORD i = 0; i < 10; i++)
        input.insert(D3DRegisterId(i, D3DSPR_INPUT));
    for (DWORD i = 0; i < 32; i++)
        temp.insert(D3DRegisterId(i, D3DSPR_TEMP));
    for (DWORD i = 0; i < 8; i++)
        samplers.insert(D3DRegisterId(i, D3DSPR_SAMPLER));

    //  Use constant registers not corrently in use by the implementation shader pipeline.
    for (DWORD i = 288; i < 512; i++)
        constant.insert(D3DRegisterId(i, D3DSPR_CONST));
}



void FFShaderGenerator :: vs_transform(D3DRegisterId dst_N, D3DRegisterId dst_V, D3DRegisterId dst_P) {

    // Declare output
    dec.push_back(ins_tk(D3DSIO_DCL, 2));
    dec.push_back(sem_tk(D3DDECLUSAGE_POSITION, 0));
    dec.push_back(dst_tk(output.reserveUsage(D3DUsageId(D3DDECLUSAGE_POSITION, 0))));

    // Declare constants needed
    FFUsageId const_usage;

    for(DWORD i = 0; i < 4; i ++) {
        const_usage = FFUsageId(FF_WORLDVIEWPROJ, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
            constant.reserveUsage(const_usage), const_usage));
    }

    for(DWORD i = 0; i < 4; i ++)  {
        const_usage = FFUsageId(FF_WORLDVIEW, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
            constant.reserveUsage(const_usage), const_usage));
    }

    for(DWORD i = 0; i < 4; i ++)  {
        const_usage = FFUsageId(FF_WORLDVIEW_IT, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
            constant.reserveUsage(const_usage), const_usage));
    }

    mul_mat4_vec4(output.findUsage(D3DUsageId(D3DDECLUSAGE_POSITION)),
        input.findUsage(D3DUsageId(D3DDECLUSAGE_POSITION)),
        constant.findUsage(FFUsageId(FF_WORLDVIEWPROJ, 0)),
        constant.findUsage(FFUsageId(FF_WORLDVIEWPROJ, 1)),
        constant.findUsage(FFUsageId(FF_WORLDVIEWPROJ, 2)),
        constant.findUsage(FFUsageId(FF_WORLDVIEWPROJ, 3)));

    mul_mat4_vec3(dst_P,
        input.findUsage(D3DUsageId(D3DDECLUSAGE_POSITION)),
        constant.findUsage(FFUsageId(FF_WORLDVIEW, 0)),
        constant.findUsage(FFUsageId(FF_WORLDVIEW, 1)),
        constant.findUsage(FFUsageId(FF_WORLDVIEW, 2)),
        constant.findUsage(FFUsageId(FF_WORLDVIEW, 3)));

    mul_mat3_vec3(dst_N,
        input.findUsage(D3DUsageId(D3DDECLUSAGE_NORMAL)),
        constant.findUsage(FFUsageId(FF_WORLDVIEW_IT, 0)),
        constant.findUsage(FFUsageId(FF_WORLDVIEW_IT, 1)),
        constant.findUsage(FFUsageId(FF_WORLDVIEW_IT, 2)));

    normalize(dst_V, dst_P);
    cod.push_back(ins_tk(D3DSIO_MOV, 2));
    cod.push_back(dst_tk(dst_V, 1, 1, 1, 0));
    cod.push_back(src_tk(dst_V, 0, 1, 2, 3, D3DSPSM_NEG));
}

void FFShaderGenerator :: vs_lighting(D3DRegisterId src_N, D3DRegisterId src_V) {

    // Declare output registers

    dec.push_back(ins_tk(D3DSIO_DCL, 2));
    dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 0));
    dec.push_back(dst_tk(output.reserveUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));

    dec.push_back(ins_tk(D3DSIO_DCL, 2));
    dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 1));
    dec.push_back(dst_tk(output.reserveUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 1))));

    // Declare constants required

    FFUsageId const_usage;

    for(DWORD i = 0; i < 4; i ++) {
        const_usage = FFUsageId(FF_WORLD, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
            constant.reserveUsage(const_usage), const_usage));
    }


    for(DWORD i = 0; i < 4; i ++) {
        const_usage = FFUsageId(FF_VIEW_IT, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
            constant.reserveUsage(const_usage), const_usage));
    }

    const_usage = FFUsageId(FF_MATERIAL_EMISSIVE);
    const_declaration.push_back( FFConstRegisterDeclaration(
    constant.reserveUsage(const_usage), const_usage));
    const_usage = FFUsageId(FF_MATERIAL_SPECULAR);
    const_declaration.push_back( FFConstRegisterDeclaration(
    constant.reserveUsage(const_usage), const_usage));
    const_usage = FFUsageId(FF_MATERIAL_DIFFUSE);
    const_declaration.push_back( FFConstRegisterDeclaration(
    constant.reserveUsage(const_usage), const_usage));
    const_usage = FFUsageId(FF_MATERIAL_AMBIENT);
    const_declaration.push_back( FFConstRegisterDeclaration(
    constant.reserveUsage(const_usage), const_usage));
    const_usage = FFUsageId(FF_MATERIAL_POWER);
    const_declaration.push_back( FFConstRegisterDeclaration(
    constant.reserveUsage(const_usage), const_usage));

    const_usage = FFUsageId(FF_AMBIENT);
    const_declaration.push_back( FFConstRegisterDeclaration(
    constant.reserveUsage(const_usage), const_usage));

    for(DWORD i = 0; i < 8; i ++) {
        const_usage = FFUsageId(FF_LIGHT_AMBIENT, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
        constant.reserveUsage(const_usage), const_usage));
        const_usage = FFUsageId(FF_LIGHT_DIFFUSE, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
        constant.reserveUsage(const_usage), const_usage));
        const_usage = FFUsageId(FF_LIGHT_SPECULAR, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
        constant.reserveUsage(const_usage), const_usage));
        const_usage = FFUsageId(FF_LIGHT_DIRECTION, i);
        const_declaration.push_back( FFConstRegisterDeclaration(
        constant.reserveUsage(const_usage), const_usage));
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
    cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_AMBIENT))));

    cod.push_back(ins_tk(D3DSIO_MUL, 3));
    cod.push_back(dst_tk(r0));
    cod.push_back(src_tk(r0));
    cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_MATERIAL_AMBIENT))));

    cod.push_back(ins_tk(D3DSIO_ADD, 3));
    cod.push_back(dst_tk(Color));
    cod.push_back(src_tk(r0));
    cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_MATERIAL_EMISSIVE))));

    temp.release(r0);

    for(DWORD i = 0; i < 8; i ++) {
        if(ff_state.lightsEnabled[i]) {

            D3DRegisterId L;
            D3DRegisterId r0, r1, r2, r3, r4, r5;


            L = temp.reserve();
            r0 = temp.reserve();
            r1 = temp.reserve();
            r2 = temp.reserve();

            
            mov(r2, constant.findUsage(FFUsageId(FF_LIGHT_DIRECTION, i)));
            normalize(r0, r2);
            negate(r1, r0);
            mul_mat3_vec3(L, r1, constant.findUsage(FFUsageId(FF_VIEW_IT, 0)),
                constant.findUsage(FFUsageId(FF_VIEW_IT, 1)),
                constant.findUsage(FFUsageId(FF_VIEW_IT, 2)));

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
            cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_LIGHT_DIFFUSE, i))));


            cod.push_back(ins_tk(D3DSIO_MUL, 3));
            cod.push_back(dst_tk(r1));
            cod.push_back(src_tk(r0));
            cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_MATERIAL_DIFFUSE))));

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
            cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_MATERIAL_POWER)), 0, 0, 0, 0));

            cod.push_back(ins_tk(D3DSIO_EXP, 2));
            cod.push_back(dst_tk(r4, 0, 0, 0, 1));
            cod.push_back(src_tk(r3, 3, 3, 3, 3));


            cod.push_back(ins_tk(D3DSIO_MUL, 3));
            cod.push_back(dst_tk(r4));
            cod.push_back(src_tk(r4, 3, 3, 3, 3));
            cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_LIGHT_SPECULAR, i))));

            cod.push_back(ins_tk(D3DSIO_MUL, 3));
            cod.push_back(dst_tk(r5));
            cod.push_back(src_tk(r4));
            cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_MATERIAL_SPECULAR))));

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
    cod.push_back(dst_tk(output.findUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0))));
    cod.push_back(src_tk(Color));    

    cod.push_back(ins_tk(D3DSIO_MOV, 2));
    cod.push_back(dst_tk(output.findUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 1))));
    cod.push_back(src_tk(ColorSpecular));

    temp.release(Color);
    temp.release(ColorSpecular);


}

void FFShaderGenerator::vs_transformed_position()
{
    // Declare constants to hold the viewport mapping required for pre-transformed addresses.
    FFUsageId const_usage;

    //  Declare viewport0 = {2/width, -2/height, 1, 1}
    const_usage = FFUsageId(FF_VIEWPORT, 0);
    const_declaration.push_back(FFConstRegisterDeclaration(constant.reserveUsage(const_usage), const_usage));

    // Define constant viewport1 = {-1, +1, 0, 0}
    D3DRegisterId viewport1 = constant.reserve();
    def.push_back(ins_tk(D3DSIO_DEF, 5));
    def.push_back(dst_tk(viewport1));
    def.push_back(flt_tk(-1.0f));
    def.push_back(flt_tk(1.0f));
    def.push_back(flt_tk(0.0f));
    def.push_back(flt_tk(0.0f));

    //  Check if the vertex declaration specifies a diffuse input color as input/output.
    if ((ff_state.fvf & D3DFVF_DIFFUSE) || checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_COLOR, 0)))
    {
        cod.push_back(ins_tk(D3DSIO_MOV, 2));
        cod.push_back(dst_tk(output.findUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0)), 1, 1, 1, 1));
        cod.push_back(src_tk(input.findUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0)), 0, 1, 2, 3, D3DSPSM_NONE));    
    }

    cod.push_back(ins_tk(D3DSIO_MAD, 3));
    cod.push_back(dst_tk(output.findUsage(D3DUsageId(D3DDECLUSAGE_POSITION, 0)), 1, 1, 1, 1));
    cod.push_back(src_tk(input.findUsage(D3DUsageId(D3DDECLUSAGE_POSITION, 0)), 0, 1, 2, 3, D3DSPSM_NONE));
    cod.push_back(src_tk(constant.findUsage(FFUsageId(FF_VIEWPORT, 0)), 0, 1, 2, 3, D3DSPSM_NONE));
    cod.push_back(src_tk(viewport1, 0, 1, 2, 3, D3DSPSM_NONE));
}

void FFShaderGenerator :: vs_input_declaration()
{
    D3DRegisterId reg;

    //  Check if the vertex declaration specifies a position.
    if ((ff_state.fvf & D3DFVF_XYZ) || (ff_state.fvf & D3DFVF_XYZW) || (ff_state.fvf & D3DFVF_XYZRHW) ||
        checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_POSITION, 0)) ||
        checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_POSITIONT, 0)))
    {
        //  Reserve and declare an input register for the vertex position.
        reg = input.reserveUsage(D3DUsageId(D3DDECLUSAGE_POSITION, 0));
        dec.push_back(ins_tk(D3DSIO_DCL, 2));
        dec.push_back(sem_tk(D3DDECLUSAGE_POSITION, 0));
        dec.push_back(dst_tk(reg));
        
        // Reserve and declare an output register for the vertex position.
        reg = output.reserveUsage(D3DUsageId(D3DDECLUSAGE_POSITION, 0));
        dec.push_back(ins_tk(D3DSIO_DCL, 2));
        dec.push_back(sem_tk(D3DDECLUSAGE_POSITION, 0));
        dec.push_back(dst_tk(reg));
    }

    //  Check if the vertex declaration specifies a normal as input/output.
    if ((ff_state.fvf & D3DFVF_NORMAL) || checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_NORMAL, 0)))
    {
        //  Reserve and declara an output register for the normal.
        reg = input.reserveUsage(D3DUsageId(D3DDECLUSAGE_NORMAL, 0));
        dec.push_back(ins_tk(D3DSIO_DCL, 2));
        dec.push_back(sem_tk(D3DDECLUSAGE_NORMAL, 0));
        dec.push_back(dst_tk(reg));
    }
    
    //  Check if the vertex declaration specifies a diffuse input color as input/output.
    if ((ff_state.fvf & D3DFVF_DIFFUSE) || checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_COLOR, 0)))
    {
        //  Reserve and declare an input register for vertex diffuse color.
        reg = input.reserveUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0));
        dec.push_back(ins_tk(D3DSIO_DCL, 2));
        dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 0));
        dec.push_back(dst_tk(reg));
        
        //  Reserve and declare an output register for the vertex diffuse color.
        reg = output.reserveUsage(D3DUsageId(D3DDECLUSAGE_COLOR, 0));
        dec.push_back(ins_tk(D3DSIO_DCL, 2));
        dec.push_back(sem_tk(D3DDECLUSAGE_COLOR, 0));
        dec.push_back(dst_tk(reg));
    }   

    for(DWORD i = 0; i < 8; i ++)
    {
        //  Check if the vertex declaration specifies a texture coordinate as input/output.
        if (fvf_has_texcoord(i, ff_state.fvf) || (checkUsageInVertexDeclaration(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i))))
        {
            //  Reserve and declare an input register for the input texture coordinate.
            reg = input.reserveUsage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i));
            dec.push_back(ins_tk(D3DSIO_DCL, 2));
            dec.push_back(sem_tk(D3DDECLUSAGE_TEXCOORD, i));
            dec.push_back(dst_tk(reg));
            
            //  Reserve and declare an output register for the output texture coordinate.
            reg = output.reserveUsage(D3DUsageId(D3DDECLUSAGE_TEXCOORD, i));
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

//  Check usage in the current vertex declaration.
bool FFShaderGenerator::checkUsageInVertexDeclaration(D3DUsageId usage)
{
    bool found = false;
    
    for(u32bit u = 0; !found && (u < ff_state.vertexDeclaration.size()); u++)
        found = (ff_state.vertexDeclaration[u].Usage == usage.usage) && (ff_state.vertexDeclaration[u].UsageIndex == usage.index);

    return found;
}


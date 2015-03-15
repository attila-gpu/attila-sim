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

#include "ContextStateAdapter.h"
#include "GPU.h"
#include "glext.h"
#include "MathLib.h"

using namespace gpu3d;

using namespace libgl;
    
ContextStateAdapter::ContextStateAdapter(GPUDriver* driver)
    : cullFaceMode(GL_BACK), previousEnabledCullFace(false), previousEnabledPolygonOffset(false), 
      slopeFactor(0.0f), unitsOffset(0.0f), driver(driver)
{
}

#define PANIC_WITH_CALLER(cName , fName, Message) {\
                                                    char msg[80];\
                                                    sprintf(msg,"%s called by %s GLLib module",fName, functionCallerName.c_str());\
                                                    panic(cName, msg, Message);\
                                                  }
                                                    

void ContextStateAdapter::setPrimitiveMode(GLenum primitive)
{
    GPURegData data;
        
    switch ( primitive )
    {
        case GL_POINTS:
            PANIC_WITH_CALLER("ContextStateAdapter", "sendPrimitiveMode", "Primitive mode GL_POINTS not supported.");
            break;
        
        case GL_LINES:
            PANIC_WITH_CALLER("ContextStateAdapter", "sendPrimitiveMode", "Primitive mode GL_LINES not supported.");
            break;
        
        case GL_LINE_LOOP:
            PANIC_WITH_CALLER("ContextStateAdapter", "sendPrimitiveMode", "Primitive mode GL_LINE_LOOP not supported.");
            break;
        
        case GL_LINE_STRIP:
            PANIC_WITH_CALLER("ContextStateAdapter", "sendPrimitiveMode", "Primitive mode GL_LINE_STRIP not supported.");
            break;
        
        case GL_TRIANGLES:
            data.primitive = TRIANGLE;
            break;
        
        case GL_TRIANGLE_STRIP:
            data.primitive = TRIANGLE_STRIP;
            break;
            
        case GL_TRIANGLE_FAN:
            data.primitive = TRIANGLE_FAN;
            break;
        
        case GL_QUADS:
            data.primitive = QUAD;
            break;
        
        case GL_QUAD_STRIP:
            data.primitive = QUAD_STRIP;
            break;
        
        case GL_POLYGON:
            PANIC_WITH_CALLER("ContextStateAdapter", "sendPrimitiveMode", "Primitive mode GL_POLYGON not supported.");
            break;
        
        default:
            
            PANIC_WITH_CALLER("ContextStateAdapter", "sendPrimitiveMode", "Unknown primitive type");

    }
    driver->writeGPURegister(GPU_PRIMITIVE, data);

}

void ContextStateAdapter::setTextureTarget(GLuint textureUnit, GLenum target, GLboolean enable)
{
    GPURegData data;
    
    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureTarget","Texture Unit number out of bounds");
        
    data.booleanVal = (enable != 0 ? true : false);
    driver->writeGPURegister(GPU_TEXTURE_ENABLE, textureUnit, data);
    
    switch ( target )
    {
        case GL_TEXTURE_1D:
            data.txMode = GPU_TEXTURE1D;
            break;
        case GL_TEXTURE_2D:
            data.txMode = GPU_TEXTURE2D;
            break;
        case GL_TEXTURE_3D:
            data.txMode = GPU_TEXTURE3D;
            break;
        case GL_TEXTURE_CUBE_MAP:
            data.txMode = GPU_TEXTURECUBEMAP;
            break;
        default:
            PANIC_WITH_CALLER("ContextStateAdapter", "setTextureTarget", "Unknown texture target");
    }
    driver->writeGPURegister(GPU_TEXTURE_MODE, textureUnit, data);

}

void ContextStateAdapter::setTextureMaxMinLevels(GLuint textureUnit, GLuint minLevel, GLuint maxLevel)
{
    GPURegData data;

    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureMaxMinLevels","Texture Unit number out of bounds");
        
    if ( minLevel > MAX_TEXTURE_SIZE || maxLevel > MAX_TEXTURE_SIZE )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureMaxMinLevels","MipMap level greater than maximum level allowed");
        
    if ( minLevel > maxLevel )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureMaxMinLevels","MinLevel is greater than MaxLevel parameter");    
        
    data.uintVal = minLevel;
    driver->writeGPURegister(GPU_TEXTURE_MIN_LEVEL, textureUnit, data);
    
    data.uintVal = maxLevel;
    driver->writeGPURegister(GPU_TEXTURE_MAX_LEVEL, textureUnit, data);
}

void ContextStateAdapter::setTextureLodBias(GLuint textureUnit, GLfloat lodBias)
{
    GPURegData data;
    
    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureLodBias","Texture Unit number out of bounds");
        
    data.f32Val = lodBias;
    driver->writeGPURegister(GPU_TEXTURE_LOD_BIAS, textureUnit, data);
}

void ContextStateAdapter::setTextureUnitLodBias(GLuint textureUnit, GLfloat lodBias)
{
    GPURegData data;
    
    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureUnitLodBias","Texture Unit number out of bounds");
    
    data.f32Val = lodBias;
    driver->writeGPURegister(GPU_TEXT_UNIT_LOD_BIAS, textureUnit, data);
}

void ContextStateAdapter::setTextureDimensions(GLuint textureUnit, GLuint width, GLuint height)
{
    GPURegData data;
    
    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureDimensions","Texture Unit number out of bounds");
    
    if ( (unsigned int)mathlib::log2((double)width) > MAX_TEXTURES ||
         (unsigned int)mathlib::log2((double)height) > MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureDimensions","Dimensions exceed the maximum size allowed");
    
    data.uintVal = width;
    driver->writeGPURegister(GPU_TEXTURE_WIDTH, textureUnit, data);

    data.uintVal = height;
    driver->writeGPURegister(GPU_TEXTURE_HEIGHT, textureUnit, data);
}

void ContextStateAdapter::setBaseMipMapLog2Dimensions(GLuint textureUnit, GLuint log2_width, GLuint log2_height)
{
    GPURegData data;
    
    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setBaseMipMapLog2Dimensions","Texture Unit number out of bounds");
    
    if ( log2_width > MAX_TEXTURES || log2_height > MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setBaseMipMapLog2Dimensions","Log2 dimensions exceed the maximum size allowed");
    
    data.uintVal = log2_width;
    driver->writeGPURegister(GPU_TEXTURE_WIDTH2, textureUnit, data);

    data.uintVal = log2_height;
    driver->writeGPURegister(GPU_TEXTURE_HEIGHT2, textureUnit, data);
}

void ContextStateAdapter::setTextureInternalFormat(GLuint textureUnit, GLenum internalformat)
{
    GPURegData data;
    GPURegData compression;
    
    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureInternalFormat","Texture Unit number out of bounds");
        
    switch ( internalformat )
    {
        case GL_RGBA:
            data.txFormat = GPU_RGBA8888;
            compression.txCompression = GPU_NO_TEXTURE_COMPRESSION;
            break;
        case GL_INTENSITY8:
            data.txFormat = GPU_INTENSITY8;
            compression.txCompression = GPU_NO_TEXTURE_COMPRESSION;
            break;
        case GL_ALPHA:
            data.txFormat = GPU_ALPHA8;
            compression.txCompression = GPU_NO_TEXTURE_COMPRESSION;
            break;
        case GL_LUMINANCE:
            data.txFormat = GPU_LUMINANCE8;
            compression.txCompression = GPU_NO_TEXTURE_COMPRESSION;
            break;
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            data.txFormat = GPU_RGB888;
            compression.txCompression = GPU_S3TC_DXT1_RGB;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            data.txFormat = GPU_RGBA8888;
            compression.txCompression = GPU_S3TC_DXT1_RGBA;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            data.txFormat = GPU_RGBA8888;
            compression.txCompression = GPU_S3TC_DXT3_RGBA;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            data.txFormat = GPU_RGBA8888;
            compression.txCompression = GPU_S3TC_DXT5_RGBA;
            break;
        default:
            PANIC_WITH_CALLER("AuxFuncsLib", "setupTexture()", "Unsupported texture format");
    }
    driver->writeGPURegister(GPU_TEXTURE_COMPRESSION, textureUnit, compression);
    driver->writeGPURegister(GPU_TEXTURE_FORMAT, textureUnit, data);
}

void ContextStateAdapter::setTextureReverseData(GLuint textureUnit, GLboolean reverse)
{
    GPURegData data;
    
    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureReverseData","Texture Unit number out of bounds");
        
    data.booleanVal = (reverse != 0 ? true : false);
    driver->writeGPURegister(GPU_TEXTURE_REVERSE, textureUnit, data);
}

void ContextStateAdapter::setTextureWrap(GLuint textureUnit, GLenum wrapS, GLenum wrapT)
{
     GPURegData data;
     
     if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setTextureWrap","Texture Unit number out of bounds");
        
#define CASE_BRANCH_CLAMP(x) case GL_##x: data.txClamp = GPU_TEXT_##x; break;

    switch ( wrapS )
    {
        CASE_BRANCH_CLAMP(CLAMP)
        CASE_BRANCH_CLAMP(CLAMP_TO_EDGE)
        CASE_BRANCH_CLAMP(REPEAT)
        CASE_BRANCH_CLAMP(CLAMP_TO_BORDER)
        CASE_BRANCH_CLAMP(MIRRORED_REPEAT)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter", "setTextureWrap", "Unexpected WRAP_S mode");
    }
    driver->writeGPURegister(GPU_TEXTURE_WRAP_S, textureUnit, data);

    switch ( wrapT )
    {
        CASE_BRANCH_CLAMP(CLAMP)
        CASE_BRANCH_CLAMP(CLAMP_TO_EDGE)
        CASE_BRANCH_CLAMP(REPEAT)
        CASE_BRANCH_CLAMP(CLAMP_TO_BORDER)
        CASE_BRANCH_CLAMP(MIRRORED_REPEAT)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter", "setTextureWrap", "Unexpected WRAP_T mode");
    }
    driver->writeGPURegister(GPU_TEXTURE_WRAP_T, textureUnit, data);

#undef CASE_BRANCH_CLAMP 
}

void ContextStateAdapter::setMinMagFilters(GLuint textureUnit, GLenum minFilter, GLenum magFilter, GLfloat maxAnisotropy )
{
    GPURegData data;
    
    if ( textureUnit >= MAX_TEXTURES )
        PANIC_WITH_CALLER("ContextStateAdapter","setMinMagFilters","Texture Unit number out of bounds");
    
    if ( maxAnisotropy < 1.0f || maxAnisotropy > MAX_ANISOTROPY )
        PANIC_WITH_CALLER("ContextStateAdapter","setMinMagFilters","Unexpected anisotropic ratio parameter");
        
#define CASE_BRANCH_FILTER(x) case GL_##x: data.txFilter = GPU_##x; break;

    switch ( minFilter )
    {
        CASE_BRANCH_FILTER(NEAREST)
        CASE_BRANCH_FILTER(LINEAR)
        CASE_BRANCH_FILTER(NEAREST_MIPMAP_NEAREST)
        CASE_BRANCH_FILTER(NEAREST_MIPMAP_LINEAR)
        CASE_BRANCH_FILTER(LINEAR_MIPMAP_NEAREST)
        CASE_BRANCH_FILTER(LINEAR_MIPMAP_LINEAR)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter","setMinMagFilters", "Unexpected MIN_FILTER");
    }
    driver->writeGPURegister(GPU_TEXTURE_MIN_FILTER, textureUnit, data);

    switch ( magFilter )
    {
        CASE_BRANCH_FILTER(NEAREST)
        CASE_BRANCH_FILTER(LINEAR)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter","setMinMagFilters", "Unexpected MAG_FILTER");
    }
    driver->writeGPURegister(GPU_TEXTURE_MAG_FILTER, textureUnit, data);

    data.uintVal = u32bit(maxAnisotropy);
    driver->writeGPURegister(GPU_TEXTURE_MAX_ANISOTROPY, textureUnit, data);

#undef CASE_BRANCH_FILTER

}

void ContextStateAdapter::setColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha)
{
    GPURegData data;
    
    data.booleanVal = (red != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_MASK_R, data);
    
    data.booleanVal = (green != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_MASK_G, data);
    
    data.booleanVal = (blue != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_MASK_B, data);
    
    data.booleanVal = (alpha != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_MASK_A, data);
}

void ContextStateAdapter::setClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
    GPURegData data;
    
    if (red   > 1.0f || red   < 0.0f ||
        green > 1.0f || green < 0.0f ||
        blue  > 1.0f || blue  < 0.0f ||
        alpha > 1.0f || alpha < 0.0f)
        
        PANIC_WITH_CALLER("ContextStateAdapter","setClearColor","Some color values not clamped to [0,1]");
    
    data.qfVal[0] = red;
    data.qfVal[1] = green;
    data.qfVal[2] = blue;
    data.qfVal[3] = alpha;

    driver->writeGPURegister( GPU_COLOR_BUFFER_CLEAR, data );
}

void ContextStateAdapter::setShadeModel(GLenum shadeMode)
{
    GPURegData data;
    
    if (shadeMode != GL_FLAT && shadeMode != GL_SMOOTH)
        PANIC_WITH_CALLER("ContextStateAdapter","setShadeModel","Unexpected shade model");

    data.booleanVal = ( shadeMode == GL_FLAT ? false : true );
    driver->writeGPURegister( GPU_INTERPOLATION, COLOR_ATTRIBUTE, data );

}

void ContextStateAdapter::setViewportArea(GLint iniX, GLint iniY, GLuint width, GLuint height)
{
    GPURegData data;
    
    if ( iniX > MAX_VIEWPORT_X || iniY > MAX_VIEWPORT_Y)
        PANIC_WITH_CALLER("ContextStateAdapter","setViewportArea","Initial x and y exceed the maximum viewport area allowed");
        
    if ( width > MAX_VIEWPORT_X || height > MAX_VIEWPORT_Y)
        PANIC_WITH_CALLER("ContextStateAdapter","setViewportArea","width and height exceed the maximum viewport area allowed");

    data.intVal = iniX;
    driver->writeGPURegister( GPU_VIEWPORT_INI_X, data );
    data.intVal = iniY;
    driver->writeGPURegister( GPU_VIEWPORT_INI_Y, data );
    data.uintVal = width;
    driver->writeGPURegister( GPU_VIEWPORT_WIDTH, data );
    data.uintVal = height;
    driver->writeGPURegister( GPU_VIEWPORT_HEIGHT, data );

}

void ContextStateAdapter::setScissorArea(GLint iniX, GLint iniY, GLsizei width, GLsizei height)
{
    GPURegData data;
    
    if ( iniX > MAX_VIEWPORT_X || iniY > MAX_VIEWPORT_Y)
        PANIC_WITH_CALLER("ContextStateAdapter","setScissorArea","Initial x and y exceed the maximum viewport area allowed");
        
    if ( width > MAX_VIEWPORT_X || height > MAX_VIEWPORT_Y)
        PANIC_WITH_CALLER("ContextStateAdapter","setScissorArea","width and height exceed the maximum viewport area allowed");

    data.intVal = iniX;
    driver->writeGPURegister( GPU_SCISSOR_INI_X, data );
    data.intVal = iniY;
    driver->writeGPURegister( GPU_SCISSOR_INI_Y, data );
    data.uintVal = width;
    driver->writeGPURegister( GPU_SCISSOR_WIDTH, data );
    data.uintVal = height;
    driver->writeGPURegister( GPU_SCISSOR_HEIGHT, data );

}

void ContextStateAdapter::setCullingFace(GLboolean enable)
{
    GPURegData data;

    if (enable) /* Enable culling */
    {
        if (!previousEnabledCullFace) /* Needed register update to enable GPU culling */
        {
        #define CASE_BRANCH(x)  case GL_##x: data.culling = x; break;
            
            switch(cullFaceMode)
            {
                CASE_BRANCH(FRONT)
                CASE_BRANCH(BACK)
                CASE_BRANCH(FRONT_AND_BACK)
            }
        #undef CASE_BRANCH 

            driver->writeGPURegister(GPU_CULLING, data);
            
            previousEnabledCullFace = true;
        }

    }
    else /* Disable culling */
    {
        if (previousEnabledCullFace) /* Needed register update to disable GPU culling */
        {
            data.culling = NONE;
            driver->writeGPURegister(GPU_CULLING, data);
            
            previousEnabledCullFace = false;
        }
    }

}

void ContextStateAdapter::setCullFaceMode(GLenum cullFace)
{
    GPURegData data;
    
    if (cullFaceMode != GL_FRONT && cullFaceMode != GL_BACK && cullFaceMode != GL_FRONT_AND_BACK)
        PANIC_WITH_CALLER("ContextStateAdapter","setCullFaceMode","Unexpected culling face mode");
    
    cullFaceMode = cullFace;
    
    if (previousEnabledCullFace)/* Needed GPU culling mode update */
    {    
    #define CASE_BRANCH(x)  case GL_##x: data.culling = x; break;
        
        switch(cullFaceMode)
        {
            CASE_BRANCH(FRONT)
            CASE_BRANCH(BACK)
            CASE_BRANCH(FRONT_AND_BACK)
        }
    #undef CASE_BRANCH 
    
        driver->writeGPURegister(GPU_CULLING, data);
    }
}

void ContextStateAdapter::setStencilTest(GLboolean enable)
{
    GPURegData data;
    
    data.booleanVal = (enable != 0 ? true : false);
    driver->writeGPURegister(GPU_STENCIL_TEST, data);
}

void ContextStateAdapter::setColorBlend(GLboolean enable)
{
    GPURegData data;
    
    data.booleanVal = (enable != 0 ? true : false);
    driver->writeGPURegister(GPU_COLOR_BLEND, data);
}

void ContextStateAdapter::setScissorTest(GLboolean enable)
{   
    GPURegData data;
    
    data.booleanVal = (enable != 0 ? true : false);
    driver->writeGPURegister(GPU_SCISSOR_TEST, data);
}

void ContextStateAdapter::setPolygonOffset(GLboolean enable)
{
    GPURegData data;
    
    if (enable)
    {
        if (!previousEnabledPolygonOffset) /* Needed register update to set Polygon Offset parameters */
        {
            data.f32Val = slopeFactor;
            driver->writeGPURegister( GPU_DEPTH_SLOPE_FACTOR, data );
            data.f32Val = unitsOffset;
            driver->writeGPURegister( GPU_DEPTH_UNIT_OFFSET, data );
            
            previousEnabledPolygonOffset = true;
        }
    }
    else
    {
        if (previousEnabledPolygonOffset) /* Needed register update to reset Polygon Offset parameters */
        {
            data.f32Val = 0.0f;
            driver->writeGPURegister( GPU_DEPTH_SLOPE_FACTOR, data );
            data.f32Val = 0.0f;
            driver->writeGPURegister( GPU_DEPTH_UNIT_OFFSET, data );
            
            previousEnabledPolygonOffset = false;
        }
    };
}

void ContextStateAdapter::setPolygonOffsetFactors(GLfloat slope, GLfloat units)
{
    GPURegData data;
    
    slopeFactor = slope;
    unitsOffset = units;
    
    if (!previousEnabledPolygonOffset) /* Needed GPU polygon offset mode update */
    {    
        data.f32Val = slopeFactor;
        driver->writeGPURegister( GPU_DEPTH_SLOPE_FACTOR, data );
        data.f32Val = unitsOffset;
        driver->writeGPURegister( GPU_DEPTH_UNIT_OFFSET, data );
    }
}

void ContextStateAdapter::setDepthTest(GLboolean enable)
{
    GPURegData data;
    
    data.booleanVal = (enable != 0 ? true : false);
    driver->writeGPURegister(GPU_DEPTH_TEST, data);

}

void ContextStateAdapter::setDepthFunction(GLenum depthFunc)
{
    GPURegData data;
    
#define CASE_BRANCH(x) case GL_##x: data.compare = GPU_##x; break;

    switch ( depthFunc )
    {
        CASE_BRANCH(NEVER)
        CASE_BRANCH(ALWAYS)
        CASE_BRANCH(LESS)
        CASE_BRANCH(LEQUAL)
        CASE_BRANCH(EQUAL)
        CASE_BRANCH(GEQUAL)
        CASE_BRANCH(GREATER)
        CASE_BRANCH(NOTEQUAL)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter", "setDepthFunction", "Unknown depth function");
    }

    #undef CASE_BRANCH

    driver->writeGPURegister(GPU_DEPTH_FUNCTION, data);
    
}

void ContextStateAdapter::setDepthMask(GLboolean enable)
{
    GPURegData data;

    data.booleanVal = (enable != 0 ? true : false);
    driver->writeGPURegister(GPU_DEPTH_MASK, data);
}

void ContextStateAdapter::setDepthClearValue(GLclampd clearValue)
{
    GPURegData data;

    if (clearValue > 1.0 || clearValue < 0.0)
        PANIC_WITH_CALLER("ContextStateAdapter","setDepthClearValue","clear value must be clamped to [0,1]");
        
    //  The float point depth value in the 0 .. 1 range must be converted to an integer
    //  value of the selected depth bit precission.
    //
    //  WARNING: This function should depend on the current depth bit precission.  Should be
    //  configurable using the wgl functions that initialize the framebuffer.  By default
    //  should be 24 (stencil 8 depth 24).  Currently the simulator only supports 24.
    //

    data.uintVal = u32bit((f32bit(clearValue) * f32bit((1 << 24) - 1)));
    driver->writeGPURegister(GPU_Z_BUFFER_CLEAR, data);

}

void ContextStateAdapter::setFrontFace(GLenum mode)
{    
    GPURegData data;

    switch ( mode )
    {
        case GL_CW:
            data.faceMode = GPU_CW;
            break;
        case GL_CCW:
            data.faceMode = GPU_CCW;
            break;
        default:
            PANIC_WITH_CALLER("ContextStateAdapter", "setFrontFace", "Unexpected front face mode");
    }

    driver->writeGPURegister(GPU_FACEMODE, data);

}

void ContextStateAdapter::setStencilFunc(GLenum func, GLint ref, GLuint mask)
{    
    GPURegData data;

    #define BRANCH_CASE(c) case GL_##c : data.compare = GPU_##c; break;

    switch ( func )
    {
        BRANCH_CASE(NEVER)
        BRANCH_CASE(ALWAYS)
        BRANCH_CASE(LESS)
        BRANCH_CASE(LEQUAL)
        BRANCH_CASE(EQUAL)
        BRANCH_CASE(GEQUAL)
        BRANCH_CASE(GREATER)
        BRANCH_CASE(NOTEQUAL)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter", "setStencilFunc", "Unexpected stencil function");
    }

    #undef BRANCH_CASE

    driver->writeGPURegister(GPU_STENCIL_FUNCTION, data);

    data.uintVal = (u32bit)ref;
    driver->writeGPURegister(GPU_STENCIL_REFERENCE, data);

    data.uintVal = mask;
    driver->writeGPURegister(GPU_STENCIL_COMPARE_MASK, data);

}

void ContextStateAdapter::setStencilOp( GLenum fail, GLenum zfail, GLenum zpass )
{
    GPURegData data;

    #define _SWITCH(fail_, msgError) \
        switch ( fail_ )\
        {\
            case GL_KEEP : data.stencilUpdate = STENCIL_KEEP; break;\
            case GL_ZERO : data.stencilUpdate = STENCIL_ZERO; break;\
            case GL_REPLACE : data.stencilUpdate = STENCIL_REPLACE; break;\
            case GL_INCR : data.stencilUpdate = STENCIL_INCR; break;\
            case GL_DECR : data.stencilUpdate = STENCIL_DECR; break;\
            case GL_INVERT : data.stencilUpdate = STENCIL_INVERT; break;\
            case GL_INCR_WRAP : data.stencilUpdate = STENCIL_INCR_WRAP; break;\
            case GL_DECR_WRAP : data.stencilUpdate = STENCIL_DECR_WRAP; break;\
            default: PANIC_WITH_CALLER("ContextStateAdapter", "setStencilOp", msgError);\
        }

    _SWITCH(fail, "Unexpected first parameter (fail)")
    driver->writeGPURegister(GPU_STENCIL_FAIL_UPDATE, data);

    _SWITCH(zfail, "Unexpected second parameter (zfail)")
    driver->writeGPURegister(GPU_DEPTH_FAIL_UPDATE, data);

    _SWITCH(zpass, "Unexpected third parameter (zpass)")
    driver->writeGPURegister(GPU_DEPTH_PASS_UPDATE, data);

    #undef _SWITCH
}

void ContextStateAdapter::setStencilClearValue(GLint s)
{
    GPURegData data;
    
    data.intVal = s;
    driver->writeGPURegister(GPU_STENCIL_BUFFER_CLEAR, data);
}

void ContextStateAdapter::setStencilMask(GLuint mask)
{   
    GPURegData data;

    data.uintVal = mask;
    driver->writeGPURegister(GPU_STENCIL_UPDATE_MASK, data);
}

void ContextStateAdapter::setBlendFactors( GLenum sRGBFactor, GLenum dRGBFactor, GLenum sAlphaFactor, GLenum dAlphaFactor )
{
    GPURegData data;

    #define SWITCH_BR(v) case GL_##v: data.blendFunction = BLEND_##v; break;

    switch ( sRGBFactor )
    {
        SWITCH_BR(ZERO)
        SWITCH_BR(ONE)
        SWITCH_BR(SRC_COLOR)
        SWITCH_BR(ONE_MINUS_SRC_COLOR)
        SWITCH_BR(DST_COLOR)
        SWITCH_BR(ONE_MINUS_DST_COLOR)
        SWITCH_BR(SRC_ALPHA)
        SWITCH_BR(ONE_MINUS_SRC_ALPHA)
        SWITCH_BR(DST_ALPHA)
        SWITCH_BR(ONE_MINUS_DST_ALPHA)
        SWITCH_BR(CONSTANT_COLOR)
        SWITCH_BR(ONE_MINUS_CONSTANT_COLOR)
        SWITCH_BR(CONSTANT_ALPHA)
        SWITCH_BR(ONE_MINUS_CONSTANT_ALPHA)
        SWITCH_BR(SRC_ALPHA_SATURATE)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter","setBlendFactors","Unexpected source RGB factor");
    }
    driver->writeGPURegister(GPU_BLEND_SRC_RGB, data);
    
    switch ( dRGBFactor )
    {
        SWITCH_BR(ZERO)
        SWITCH_BR(ONE)
        SWITCH_BR(SRC_COLOR)
        SWITCH_BR(ONE_MINUS_SRC_COLOR)
        SWITCH_BR(DST_COLOR)
        SWITCH_BR(ONE_MINUS_DST_COLOR)
        SWITCH_BR(SRC_ALPHA)
        SWITCH_BR(ONE_MINUS_SRC_ALPHA)
        SWITCH_BR(DST_ALPHA)
        SWITCH_BR(ONE_MINUS_DST_ALPHA)
        SWITCH_BR(CONSTANT_COLOR)
        SWITCH_BR(ONE_MINUS_CONSTANT_COLOR)
        SWITCH_BR(CONSTANT_ALPHA)
        SWITCH_BR(ONE_MINUS_CONSTANT_ALPHA)
        SWITCH_BR(SRC_ALPHA_SATURATE)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter","setBlendFactors","Unexpected destination RGB factor");
    }
    driver->writeGPURegister(GPU_BLEND_DST_RGB, data);
    
    switch ( sAlphaFactor )
    {
        SWITCH_BR(ZERO)
        SWITCH_BR(ONE)
        SWITCH_BR(SRC_COLOR)
        SWITCH_BR(ONE_MINUS_SRC_COLOR)
        SWITCH_BR(DST_COLOR)
        SWITCH_BR(ONE_MINUS_DST_COLOR)
        SWITCH_BR(SRC_ALPHA)
        SWITCH_BR(ONE_MINUS_SRC_ALPHA)
        SWITCH_BR(DST_ALPHA)
        SWITCH_BR(ONE_MINUS_DST_ALPHA)
        SWITCH_BR(CONSTANT_COLOR)
        SWITCH_BR(ONE_MINUS_CONSTANT_COLOR)
        SWITCH_BR(CONSTANT_ALPHA)
        SWITCH_BR(ONE_MINUS_CONSTANT_ALPHA)
        SWITCH_BR(SRC_ALPHA_SATURATE)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter","setBlendFactors","Unexpected source Alpha factor");
    }
    driver->writeGPURegister(GPU_BLEND_SRC_ALPHA, data);
    
    switch ( dAlphaFactor )
    {
        SWITCH_BR(ZERO)
        SWITCH_BR(ONE)
        SWITCH_BR(SRC_COLOR)
        SWITCH_BR(ONE_MINUS_SRC_COLOR)
        SWITCH_BR(DST_COLOR)
        SWITCH_BR(ONE_MINUS_DST_COLOR)
        SWITCH_BR(SRC_ALPHA)
        SWITCH_BR(ONE_MINUS_SRC_ALPHA)
        SWITCH_BR(DST_ALPHA)
        SWITCH_BR(ONE_MINUS_DST_ALPHA)
        SWITCH_BR(CONSTANT_COLOR)
        SWITCH_BR(ONE_MINUS_CONSTANT_COLOR)
        SWITCH_BR(CONSTANT_ALPHA)
        SWITCH_BR(ONE_MINUS_CONSTANT_ALPHA)
        SWITCH_BR(SRC_ALPHA_SATURATE)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter","setBlendFactors","Unexpected destination Alpha factor");
    }
    driver->writeGPURegister(GPU_BLEND_DST_ALPHA, data);

    #undef SWITCH_BR
}

void ContextStateAdapter::setBlendEquation( GLenum mode )
{
    GPURegData data;
    
#define BRANCH_CASE(c) case GL_##c: data.blendEquation = BLEND_##c; break;

    switch ( mode )
    {
        BRANCH_CASE(FUNC_ADD)
        BRANCH_CASE(FUNC_SUBTRACT)
        BRANCH_CASE(FUNC_REVERSE_SUBTRACT)
        BRANCH_CASE(MIN)
        BRANCH_CASE(MAX)
        default:
            PANIC_WITH_CALLER("ContextStateAdapter", "setBlendEquation", "Unexpected blend mode equation");
    }
    driver->writeGPURegister(GPU_BLEND_EQUATION, data);

#undef BRANCH_CASE

}

void ContextStateAdapter::setBlendColor( GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha )
{
    GPURegData data;
    
    if (red   > 1.0f || red   < 0.0f ||
        green > 1.0f || green < 0.0f ||
        blue  > 1.0f || blue  < 0.0f ||
        alpha > 1.0f || alpha < 0.0f)
        
        PANIC_WITH_CALLER("ContextStateAdapter","setBlendColor","Some color values not clamped to [0,1]");
        
    data.qfVal[0] = red;
    data.qfVal[1] = green;
    data.qfVal[2] = blue;
    data.qfVal[3] = alpha;
    driver->writeGPURegister(GPU_BLEND_COLOR, data);
}

void ContextStateAdapter::setDepthRange( GLclampd near_val, GLclampd far_val )
{
    GPURegData data;
    
    if ( near_val > 1.0f || near_val < 0.0f ||
         far_val  > 1.0f || far_val  < 0.0f )
         
         PANIC_WITH_CALLER("ContextStateAdapter","setDepthRange","Some color values not clamped to [0,1]");

    data.f32Val = near_val;
    driver->writeGPURegister( GPU_DEPTH_RANGE_NEAR, data );
    data.f32Val = far_val;
    driver->writeGPURegister( GPU_DEPTH_RANGE_FAR, data );
}

void ContextStateAdapter::setCurrentColor( GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha )
{
    GPURegData data;

    data.qfVal[0] = red;
    data.qfVal[1] = green;
    data.qfVal[2] = blue;
    data.qfVal[3] = alpha;
    driver->writeGPURegister(GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE, (u32bit)GPUDriver::VS_COLOR, data); 
}

void ContextStateAdapter::setTwoSidedLighting( GLboolean enable )
{   
    GPURegData data;
    
    data.booleanVal = (enable != 0 ? true : false);
    driver->writeGPURegister(GPU_TWOSIDED_LIGHTING, data);
}
#undef PANIC_WITH_CALLER

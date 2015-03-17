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

#include "GL2ACD.h"
#include <stdio.h>

using namespace agl;
using namespace acdlib;

ACD_TEXTURE_ADDR_MODE agl::getACDTextureAddrMode(GLenum texAddrMode)
{
    switch (texAddrMode)
    {
        case (GL_CLAMP):
            return ACD_TEXTURE_ADDR_CLAMP;
            break;
        case (GL_CLAMP_TO_EDGE):
            return ACD_TEXTURE_ADDR_CLAMP_TO_EDGE;
            break;
        case (GL_REPEAT):
            return ACD_TEXTURE_ADDR_REPEAT;
            break;
        case (GL_CLAMP_TO_BORDER):
            return ACD_TEXTURE_ADDR_CLAMP_TO_BORDER;
            break;
        /*case ():
            return ACD_TEXTURE_ADDR_MIRRORED_REPEAT;
            break;*/
        default:
            panic("GLContext","getACDTextureFilter","Filter type not found");
            return static_cast<ACD_TEXTURE_ADDR_MODE>(0);
    }
}

ACD_TEXTURE_FILTER agl::getACDTextureFilter(GLenum texFilter)
{
    switch (texFilter)
    {
        case (GL_NEAREST):
            return ACD_TEXTURE_FILTER_NEAREST;
            break;
        case (GL_LINEAR):
            return ACD_TEXTURE_FILTER_LINEAR;
            break;
        case (GL_NEAREST_MIPMAP_NEAREST):
            return ACD_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST;
            break;
        case (GL_NEAREST_MIPMAP_LINEAR):
            return ACD_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR;
            break;
        case (GL_LINEAR_MIPMAP_NEAREST):
            return ACD_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST;
            break;
        case (GL_LINEAR_MIPMAP_LINEAR):
            return ACD_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR;
            break;
        default:
            panic("GLContext","getACDTextureFilter","Filter type not found");
            return static_cast<ACD_TEXTURE_FILTER>(0);
    }
}

ACD_STREAM_DATA agl::getACDStreamType (GLenum type, GLboolean normalized)
{
    if (normalized)
    {
        switch (type)
        {
            case GL_UNSIGNED_BYTE:
                    return ACD_SD_UNORM8;
                    break;

            case GL_UNSIGNED_SHORT:
                    return ACD_SD_UNORM16;
                    break;

            case GL_SHORT:
                    return ACD_SD_SNORM16;
                    break;

            case GL_UNSIGNED_INT:
                    return ACD_SD_UNORM32;
                    break;

            case GL_FLOAT:
                    return ACD_SD_FLOAT32;
                    break;

            default:
                panic("GLContext","getACDStreamType", "Stream Type not supported yet");
                return static_cast<ACD_STREAM_DATA>(0);
        }
    }
    else
    {
        switch (type)
        {
            case GL_UNSIGNED_BYTE:
                    return ACD_SD_UINT8;
                    break;

            case GL_UNSIGNED_SHORT:
                    return ACD_SD_UINT16;
                    break;

            case GL_SHORT:
                    return ACD_SD_SINT16;
                    break;

            case GL_UNSIGNED_INT:
                    return ACD_SD_UINT32;
                    break;

            case GL_FLOAT:
                    return ACD_SD_FLOAT32;
                    break;

            default:
                panic("GLContext","getACDStreamType", "Stream Type not supported yet");
                return static_cast<ACD_STREAM_DATA>(0);
        }
    }
}

acdlib::ACD_CULL_MODE agl::getACDCullMode(GLenum cullMode)
{
    switch(cullMode)
    {
        case GL_FRONT:
            return ACD_CULL_FRONT;
        case GL_BACK:
            return ACD_CULL_BACK;
        case GL_FRONT_AND_BACK:
            return ACD_CULL_FRONT_AND_BACK;
        default:
            panic("GLContext","getACDCullMode","Cull Mode not found");
            return static_cast<acdlib::ACD_CULL_MODE>(0);
   }
}


acdlib::ACD_COMPARE_FUNCTION agl::getACDDepthFunc(GLenum func)
{
    switch (func)
    {
        case GL_NEVER:
            return acdlib::ACD_COMPARE_FUNCTION_NEVER;

        case GL_LESS:
            return ACD_COMPARE_FUNCTION_LESS;

        case GL_EQUAL:
            return ACD_COMPARE_FUNCTION_EQUAL;

        case GL_LEQUAL:
            return ACD_COMPARE_FUNCTION_LESS_EQUAL;

        case GL_GREATER:
            return ACD_COMPARE_FUNCTION_GREATER;

        case GL_NOTEQUAL:
            return ACD_COMPARE_FUNCTION_NOT_EQUAL;

        case GL_GEQUAL:
            return ACD_COMPARE_FUNCTION_GREATER_EQUAL;

        case GL_ALWAYS:
            return ACD_COMPARE_FUNCTION_ALWAYS;

        default:
            panic("GLContext","getACDDepthFunc","function not found");
            return static_cast<acdlib::ACD_COMPARE_FUNCTION>(0);
    }
}


ACD_COMPARE_FUNCTION agl::getACDCompareFunc (GLenum compareFunc)
{
    switch (compareFunc)
    {
        case GL_NEVER:
            return ACD_COMPARE_FUNCTION_NEVER;

        case GL_LESS:
            return ACD_COMPARE_FUNCTION_LESS;

        case GL_EQUAL:
            return ACD_COMPARE_FUNCTION_EQUAL;

        case GL_LEQUAL:
            return ACD_COMPARE_FUNCTION_LESS_EQUAL;

        case GL_GREATER:
            return ACD_COMPARE_FUNCTION_GREATER;

        case GL_NOTEQUAL:
            return ACD_COMPARE_FUNCTION_NOT_EQUAL;

        case GL_GEQUAL:
            return ACD_COMPARE_FUNCTION_GREATER_EQUAL;

        case GL_ALWAYS:
            return ACD_COMPARE_FUNCTION_ALWAYS;

        default:
            panic("GLContext","getACDCompareFunc","Compare Function incorrect");
            return static_cast<acdlib::ACD_COMPARE_FUNCTION>(0);
    }
}

ACD_STENCIL_OP agl::getACDStencilOp (GLenum stencilOp)
{
    switch(stencilOp)
    {
        case GL_KEEP :
            return ACD_STENCIL_OP_KEEP;

        case GL_ZERO :
            return ACD_STENCIL_OP_ZERO;

        case GL_REPLACE :
            return ACD_STENCIL_OP_REPLACE;

        case GL_INCR_WRAP :
            return ACD_STENCIL_OP_INCR_SAT;

        case GL_DECR_WRAP :
            return ACD_STENCIL_OP_DECR_SAT;

        case GL_INVERT :
            return ACD_STENCIL_OP_INVERT;

        case GL_INCR :
            return ACD_STENCIL_OP_INCR;

        case GL_DECR :
            return ACD_STENCIL_OP_DECR;

        default:
            panic("GLContext","getACDStencilOp","Stencil Op incorrect");
            return static_cast<ACD_STENCIL_OP>(0);
    }
}

ACD_BLEND_FUNCTION agl::getACDBlendFunction(GLenum blendFunc)
{
    switch ( blendFunc )
    {
        case GL_FUNC_ADD:
            return ACD_BLEND_ADD;

        case GL_FUNC_SUBTRACT:
            return ACD_BLEND_SUBTRACT;

        case GL_FUNC_REVERSE_SUBTRACT:
            return ACD_BLEND_REVERSE_SUBTRACT;

        case GL_MIN:
            return ACD_BLEND_MIN;

        case GL_MAX:
            return ACD_BLEND_MAX;

        default:
            panic("GLContext", "getACDBlendFunction", "Unexpected blend mode equation");
            return static_cast<ACD_BLEND_FUNCTION>(0);
    }
}

acdlib::ACD_BLEND_OPTION agl::getACDBlendOption(GLenum blendOption)
{
    switch (blendOption)
    {
        case GL_ZERO:
            return ACD_BLEND_ZERO;
            break;
        case GL_ONE:
            return ACD_BLEND_ONE;
            break;
        case GL_SRC_COLOR:
            return ACD_BLEND_SRC_COLOR;
            break;
        case GL_ONE_MINUS_SRC_COLOR:
            return ACD_BLEND_INV_SRC_COLOR;
            break;
        case GL_SRC_ALPHA:
            return ACD_BLEND_SRC_ALPHA;
            break;
        case GL_ONE_MINUS_SRC_ALPHA:
            return ACD_BLEND_INV_SRC_ALPHA;
            break;
        case GL_DST_ALPHA:
            return ACD_BLEND_DEST_ALPHA;
            break;
        case GL_ONE_MINUS_DST_ALPHA:
            return ACD_BLEND_INV_DEST_ALPHA;
            break;
        case GL_DST_COLOR:
            return ACD_BLEND_DEST_COLOR;
            break;
        case GL_ONE_MINUS_DST_COLOR:
            return ACD_BLEND_INV_DEST_COLOR;
            break;
        case GL_SRC_ALPHA_SATURATE:
            return ACD_BLEND_SRC_ALPHA_SAT;
            break;
        /*case :
            return ACD_BLEND_BLEND_FACTOR;
            break;
        case :
            return ACD_BLEND_INV_BLEND_FACTOR;
            break;
        case :
            return ACD_BLEND_CONSTANT_COLOR;
            break;
        case :
            return ACD_BLEND_INV_CONSTANT_COLOR;
            break;
        case :
            return ACD_BLEND_CONSTANT_ALPHA;
            break;
        case :
            return ACD_BLEND_INV_CONSTANT_ALPHA;
            break;*/
        default:
            panic("GLContext","getBlendOption","Blend option not found");
            return static_cast<acdlib::ACD_BLEND_OPTION>(0);
    }

}

acdlib::ACD_FORMAT agl::getACDTextureFormat (GLenum acdFormat)
{
    switch (acdFormat)
    {
        // Uncompressed formats
        case GL_RGB:
        case GL_RGB8:
            return ACD_FORMAT_RGB_888;
            break;

        case GL_RGBA:
        case GL_RGBA8:
            return ACD_FORMAT_RGBA_8888;
            break;

        case GL_UNSIGNED_INT_8_8_8_8:
        
            return ACD_FORMAT_UNSIGNED_INT_8888;
            break;
            
        case GL_INTENSITY:
        case GL_INTENSITY8:
            return ACD_FORMAT_INTENSITY_8;
            break;
        case GL_INTENSITY12:
            return ACD_FORMAT_INTENSITY_12;
            break;
        case GL_INTENSITY16:
            return ACD_FORMAT_INTENSITY_16;
            break;

        case GL_ALPHA:
        case GL_ALPHA8:
            return ACD_FORMAT_ALPHA_8;
            break;
        case GL_ALPHA12:
            return ACD_FORMAT_ALPHA_12;
            break;
        case GL_ALPHA16:
            return ACD_FORMAT_ALPHA_16;
            break;

        case GL_LUMINANCE:
        case GL_LUMINANCE8:
            return ACD_FORMAT_LUMINANCE_8;
            break;
        case GL_LUMINANCE12:
            return ACD_FORMAT_LUMINANCE_12;
            break;
        case GL_LUMINANCE16:
            return ACD_FORMAT_LUMINANCE_16;
            break;

        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            return ACD_COMPRESSED_S3TC_DXT1_RGB;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
            return ACD_COMPRESSED_S3TC_DXT1_RGBA;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            return ACD_COMPRESSED_S3TC_DXT3_RGBA;
            break;
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return ACD_COMPRESSED_S3TC_DXT5_RGBA;
            break;

        case GL_COMPRESSED_LUMINANCE_LATC1_EXT:
            return ACD_COMPRESSED_LUMINANCE_LATC1_EXT;
            break;

        case GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            return ACD_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT;
            break;

        case GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
            return ACD_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT;
            break;

        case GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            return ACD_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT;
            break;

        case GL_DEPTH_COMPONENT16:
            return ACD_FORMAT_DEPTH_COMPONENT_16;
            break;
        case GL_DEPTH_COMPONENT24:
            return ACD_FORMAT_DEPTH_COMPONENT_24;
            break;
        case GL_DEPTH_COMPONENT32:
            return ACD_FORMAT_DEPTH_COMPONENT_32;

            break;

        default:
            char error[128];
            sprintf(error, "ACD format unknown: %d", acdFormat);
            panic ("GLTextureObject","getACDformat",error);
            return ACD_FORMAT_UNKNOWN;
            break;
    }
}

acdlib::ACD_CUBEMAP_FACE agl::getACDCubeMapFace(GLenum cubeFace)
{
    switch (cubeFace)
    {
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            return ACD_CUBEMAP_POSITIVE_X;
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            return ACD_CUBEMAP_NEGATIVE_X;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            return ACD_CUBEMAP_POSITIVE_y;
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            return ACD_CUBEMAP_NEGATIVE_Y;
            break;
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            return ACD_CUBEMAP_POSITIVE_Z;
            break;
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            return ACD_CUBEMAP_NEGATIVE_Z;
            break;
        default:
            char error[128];
            sprintf(error, "Cube map face not supported: %d", cubeFace);
            panic("GLTextureObject","getACDcubeMapFace",error);
            return static_cast<acdlib::ACD_CUBEMAP_FACE>(0);
    }
}

acdlib::ACD_FACE agl::getACDFace(GLenum face)
{
    switch(face)
    {
        case GL_NONE:
            return ACD_FACE_NONE;

        case GL_FRONT:
            return ACD_FACE_FRONT;

        case GL_BACK:
            return ACD_FACE_BACK;

        case GL_FRONT_AND_BACK:
            return ACD_FACE_FRONT_AND_BACK;
        
        default:
            char error[128];
            sprintf(error, "Face mode not supported: %d", face);
            panic("GL2ACD","getACDFace",error);
            return static_cast<acdlib::ACD_FACE>(0);
    }
}

GLuint agl::getACDTexelSize(GLenum format)
{
    switch ( format ) 
    {
        case GL_RGB8:
            return 3;
        case GL_RGBA:
        case GL_DEPTH_COMPONENT24:
            return 4;
        case GL_ALPHA:
        case GL_ALPHA8:
        case GL_LUMINANCE:
        case GL_LUMINANCE8:
            return 1;
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return 1;
        default:
            char error[128];
            sprintf(error, "Unknown format size: %d", format);
            panic("ACDDeviceImp", "getTexelSize", error);
            return 0;
    }
}


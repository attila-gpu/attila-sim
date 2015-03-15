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

#include "GL2ACDX.h"

using namespace agl;
using namespace acdlib;

acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_TARGET agl::getACDXTextureTarget(GLenum target)
{
    switch (target)
    {
        case GL_TEXTURE_2D:
        case GL_TEXTURE_RECTANGLE:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_2D;

        case GL_TEXTURE_3D:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_3D;
            
        case GL_TEXTURE_CUBE_MAP:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_CUBE_MAP;

        default:
            panic("GLContext","getACDXTextureTarget","texture target not found");
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_2D;
    }
}

ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_MODE agl::getACDXFogMode(GLenum fogMode)
{
    switch (fogMode)
    {
        case GL_LINEAR:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_LINEAR;

        case GL_EXP:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_EXP;

        case GL_EXP2:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_EXP2;

        default:
            panic("GLContext","getACDFogMode","Fog mode not supported");
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_LINEAR;
    }
}

ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_COORDINATE_SOURCE agl::getACDXFogCoordinateSource(GLenum fogCoordinate)
{

    switch (fogCoordinate)
    {
        case GL_FRAGMENT_DEPTH:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FRAGMENT_DEPTH;

        case GL_FOG_COORD:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_COORD;

        default:
            panic("GLContext","","Fog Coordinate Source not found");
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FRAGMENT_DEPTH;;
    }
}

ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_STAGE_FUNCTION agl::getACDXTextureStageFunction (GLenum texFunction)
{

    switch (texFunction)
    {
        case GL_ADD:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_ADD;
            break;
        case GL_MODULATE:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_MODULATE;
            break;
        case GL_REPLACE:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_REPLACE;
            break;
        case GL_DECAL:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_DECAL;
            break;
        case GL_BLEND:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_BLEND;
            break;
        case GL_COMBINE:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_COMBINE;
            break;
        case GL_COMBINE4_NV:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_COMBINE_NV;
            break;
        default:
            panic ("GLContext","getACDTextureStageFunction","Texture Function not found");
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_MODULATE;
    }
}

ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_BASE_INTERNAL_FORMAT agl::getACDXTextureInternalFormat (GLenum textureInternalFormat)
{

    switch (textureInternalFormat)
    {
        case GL_ALPHA:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_ALPHA;
            break;
        case GL_LUMINANCE:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_LUMINANCE;
            break;
        case GL_LUMINANCE_ALPHA:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_LUMINANCE_ALPHA;
            break;
        case GL_DEPTH:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_DEPTH;
            break;
        case GL_INTENSITY:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_INTENSITY;
            break;
        case GL_RGB:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_RGB;
            break;
        case GL_RGBA:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_RGBA;
            break;
        default:
            panic ("GLContext","getACDTextureInternalFormat","Texture internal Format not found");
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_ALPHA;
    }
}

ACDX_COMBINE_SETTINGS::ACDX_COMBINE_FUNCTION agl::getACDXCombinerFunction (GLenum combineFunction)
{

    switch (combineFunction)
    {
        case GL_REPLACE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_REPLACE;
            break;
        case GL_MODULATE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE;
            break;
        case GL_ADD:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD;
            break;
        case GL_ADD_SIGNED:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD_SIGNED;
            break;
        case GL_INTERPOLATE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_INTERPOLATE;
            break;
        /*case GL_SUBSTRACT:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SUBSTRACT;
            break;*/
        case GL_DOT3_RGB:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_DOT3_RGB;
            break;
        case GL_DOT3_RGBA:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_DOT3_RGBA;
            break;
        /*case GL_MODULATE_ADD:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_ADD;
            break;
        case GL_MODULATE_SIGNED_ADD:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SIGNED_ADD;
            break;
        case GL_MODULATE_SUBSTRACT:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SUBTRACT;
            break;*/
        default:
            panic ("GLContext","getACDXCombinerFunction","Combine function not found");
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SUBTRACT;
    }
}

ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SOURCE agl::getACDXCombineSource (GLenum combineSource)
{

    /*switch (combineSource)
    {
        case GL_REPLACE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO;
            break;
        case GL_REPLACE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE;
            break;
        case GL_REPLACE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTURE;
            break;
        case GL_REPLACE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTUREn;
            break;
        case GL_REPLACE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_CONSTANT;
            break;
        case GL_REPLACE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PRIMARY_COLOR;
            break;
        case GL_REPLACE:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PREVIOUS;
            break;
        default:
            panic ("GLContext","getACDXCombineSource","Combine source not found");
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO;
    }*/
    return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO;
}

ACDX_COMBINE_SETTINGS::ACDX_COMBINE_OPERAND agl::getACDXCombineOperand (GLenum combineOperand)
{

    /*switch (combineOperand)
    {
        case ACDX_COMBINE_SRC_COLOR:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR;
            break;
        case ACDX_COMBINE_ONE_MINUS_SRC_COLOR:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_COLOR;
            break;
        case ACDX_COMBINE_SRC_ALPHA:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA;
            break;
        case ACDX_COMBINE_ONE_MINUS_SRC_ALPHA:
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_ALPHA;
            break;
        default:
            panic ("GLContext","getACDXCombineOperand","Combine operand not found");
            return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR;
    }*/
    return ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR;
}

ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_TEXTURE_COORDINATE_MODE agl::getACDXTexGenMode (GLenum texGenMode)
{
    switch (texGenMode)
    {
        case GL_OBJECT_LINEAR:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_OBJECT_LINEAR;
            break;
        case GL_EYE_LINEAR:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_EYE_LINEAR;
            break;
        case GL_SPHERE_MAP:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_SPHERE_MAP;
            break;
        case GL_REFLECTION_MAP:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_REFLECTION_MAP;
            break;
        case GL_NORMAL_MAP:
            return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_NORMAL_MAP;
            break;
        default:
            panic("GLContext","getACDTexGenMode","Undefined texture gen mode");
            return static_cast<ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_TEXTURE_COORDINATE_MODE>(0);
    }
}


ACDX_FIXED_PIPELINE_SETTINGS::ACDX_COLOR_MATERIAL_ENABLED_FACE agl::getACDXColorMaterialFace (GLenum face)
{
    switch (face)
    {
        case GL_FRONT:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_FRONT;

        case GL_BACK:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_BACK;

        case GL_FRONT_AND_BACK:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_FRONT_AND_BACK;

        default:
            panic("GLContext", "getACDXColorMaterialFace", "Unexpected face");
            return static_cast<ACDX_FIXED_PIPELINE_SETTINGS::ACDX_COLOR_MATERIAL_ENABLED_FACE>(0);
    }
}

ACDX_FIXED_PIPELINE_SETTINGS::ACDX_COLOR_MATERIAL_MODE agl::getACDXColorMaterialMode (GLenum mode)
{

    switch (mode)
    {
        case GL_EMISSION:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_EMISSION;

        case GL_AMBIENT:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_AMBIENT;

        case GL_DIFFUSE:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_DIFFUSE;

        case GL_SPECULAR:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_SPECULAR;

        case GL_AMBIENT_AND_DIFFUSE:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_AMBIENT_AND_DIFFUSE;

        default:
            panic("GLContext", "getACDXColorMaterialFace", "Unexpected face");
            return static_cast<ACDX_FIXED_PIPELINE_SETTINGS::ACDX_COLOR_MATERIAL_MODE>(0);
    }
}

ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_TEST_FUNCTION agl::getACDXAlphaFunc(GLenum func)
{
    switch (func)
    {
        case GL_NEVER:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_NEVER;

        case GL_ALWAYS:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_ALWAYS;

        case GL_LESS:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_LESS;

        case GL_LEQUAL:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_LEQUAL;

        case GL_EQUAL:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_EQUAL;

        case GL_GEQUAL:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_GEQUAL;

        case GL_GREATER:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_GREATER;

        case GL_NOTEQUAL:
            return acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_NOTEQUAL;

        default:
            panic("GLContext", "getACDXColorMaterialFace", "Unexpected face");
            return static_cast<ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_TEST_FUNCTION>(0);
    }
}

ACDX_TEXTURE_COORD agl::getACDXTexCoordPlane(GLenum texCoordPlane)
{
    switch (texCoordPlane)
    {
        case GL_S:
            return ACDX_COORD_S;
            break;
        case GL_T:
            return ACDX_COORD_T;
            break;
        case GL_R:
            return ACDX_COORD_R;
            break;
        case GL_Q:
            return ACDX_COORD_Q;
            break;
        default:
            panic("GLContext","getACDTexcoorPlane","Unexpected plane coordinate");
            return static_cast<ACDX_TEXTURE_COORD>(0);
    }
}


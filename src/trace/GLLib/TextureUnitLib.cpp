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

#include "TextureUnitLib.h"
#include "glext.h"
#include <sstream>
#include <cstdio>
#include <iostream>


using namespace libgl;
using namespace std;


TextureUnit::TextureUnit() : texFunc(GL_MODULATE), rgbCombinerFunc(GL_MODULATE),
                                      alphaCombinerFunc(GL_MODULATE), rgbScale(1), alphaScale(1),
                                      activeTexGenS(false), activeTexGenT(false), activeTexGenR(false),
                                      activeTexGenQ(false),
                                      texGenModeS(GL_EYE_LINEAR), texGenModeT(GL_EYE_LINEAR),
                                      texGenModeR(GL_EYE_LINEAR), texGenModeQ(GL_EYE_LINEAR)
{
    for ( int i = 0; i < 4; i++ )
    {
        flags[i] = 0;
        texObjs[i] = 0;
        //envColor[i] = 0.0f;
    }

    srcRGB[0] = srcAlpha[0] = GL_TEXTURE;
    srcRGB[1] = srcAlpha[1] = GL_PREVIOUS;
    srcRGB[2] = srcAlpha[2] = GL_CONSTANT;
    srcRGB[3] = srcAlpha[3] = GL_ZERO;

    operandRGB[0] = operandRGB[1] = GL_SRC_COLOR;
    operandRGB[2] = GL_SRC_ALPHA;
    operandAlpha[0] = operandAlpha[1] = operandAlpha[2] = GL_SRC_ALPHA;
    operandRGB[3] = GL_ONE_MINUS_SRC_COLOR;
    operandAlpha[3] = GL_ONE_MINUS_SRC_ALPHA;

    lodBias = 0.0f;
}

void TextureUnit::enableTarget(GLenum target)
{
    if ( target == GL_TEXTURE_1D )
        flags[0] = true;
    else if ( target == GL_TEXTURE_2D )
        flags[1] = true;
    else if ( target == GL_TEXTURE_3D )
        flags[2] = true;
    else if ( target == GL_TEXTURE_CUBE_MAP )
        flags[3] = true;
    else
        panic("TextureUnit", "enableTarget", "Target not supported");
}

void TextureUnit::disableTarget(GLenum target)
{
    if ( target == GL_TEXTURE_1D )
        flags[0] = false;
    else if ( target == GL_TEXTURE_2D )
        flags[1] = false;
    else if ( target == GL_TEXTURE_3D )
        flags[2] = false;
    else if ( target == GL_TEXTURE_CUBE_MAP )
        flags[3] = false;
    else
        panic("TextureUnit", "disableTarget", "Target not supported");
}


GLenum TextureUnit::getTarget() const
{
    // Texture target priority
    if ( flags[3] )
        return GL_TEXTURE_CUBE_MAP;
    if ( flags[2] )
        return GL_TEXTURE_3D;
    if ( flags[1] )
        return GL_TEXTURE_2D;
    if ( flags[0] )
        return GL_TEXTURE_1D;
    return 0; // textures disabled
}


TextureObject* TextureUnit::getTextureObject() const
{
    if ( flags[3] )
        return texObjs[3];
    if ( flags[2] )
        return texObjs[2];
    if ( flags[1] )
        return texObjs[1];
    if ( flags[0] )
        return texObjs[0];
    return 0; // textures disabled
}

TextureObject* TextureUnit::getTextureObject(GLenum target) const
{
    if ( target == GL_TEXTURE_1D )
        return texObjs[0];
    else if ( target == GL_TEXTURE_2D )
        return texObjs[1];
    else if ( target == GL_TEXTURE_3D )
        return texObjs[2];
    else if ( target == GL_TEXTURE_CUBE_MAP )
        return texObjs[3];
    else
        return 0;   
}

void TextureUnit::setTextureObject(GLenum target, TextureObject* to)
{
    if ( target == GL_TEXTURE_1D )
        texObjs[0] = to;
    else if ( target == GL_TEXTURE_2D )
        texObjs[1] = to;
    else if ( target == GL_TEXTURE_3D )
        texObjs[2] = to;
    else if ( target == GL_TEXTURE_CUBE_MAP )
        texObjs[3] = to;
    else
        panic("TextureUnit", "setTextureObject", "Target unknown");
}

void TextureUnit::setParameter(GLenum target, GLenum pname, const GLfloat* param)
{
    if ( target == GL_TEXTURE_FILTER_CONTROL )
    {
        if ( pname != GL_TEXTURE_LOD_BIAS )
            panic("TextureUnit", "setParameter", "GL_TEXTURE_FILTER_CONTROL target with unexpected pname");

        lodBias = param[0];
        return ;
    }

    if ( target != GL_TEXTURE_ENV )
        panic("libgl::TextureUnit", "setParameter", "Only target == GL_TEXTURE_ENV supported for now");

//    if ( pname == GL_TEXTURE_ENV_COLOR )
//    {
//        envColor[0] = param[0];
//        envColor[1] = param[1];
//        envColor[2] = param[2];
//        envColor[3] = param[3];
//        return ;
//    }

    GLint pInt = GLint(param[0]); // the param is used like an integer
    setParameter(target, pname, &pInt); // call integer version

}


void TextureUnit::setParameter(GLenum target, GLenum pname, const GLint* param)
{
    if ( target != GL_TEXTURE_ENV )
        panic("libgl::TextureUnit", "setParameter", "Only target == GL_TEXTURE_ENV supported for now");

    if ( pname == GL_TEXTURE_ENV_MODE )
    {
        switch ( param[0] )
        {
            case GL_MULT:
                texFunc = GL_MODULATE;
                return;
            case GL_DECAL:
            case GL_REPLACE:
            case GL_MODULATE:
            case GL_BLEND:
            case GL_ADD:
            case GL_COMBINE:
            /* For "NV_texture_env_combine4" extension support */
            case GL_COMBINE4_NV:
                texFunc = param[0];
                return;
        }
        panic("TextureUnit", "setParameter", "Unknown texture function");
    }
//    else if ( pname == GL_TEXTURE_ENV_COLOR )
//    {
//        envColor[0] = GLfloat(param[0]);
//        envColor[1] = GLfloat(param[1]);
//        envColor[2] = GLfloat(param[2]);
//        envColor[3] = GLfloat(param[3]);
//        return ;
//    }
    else if ( pname == GL_COMBINE_RGB )
    {
        switch ( param[0] )
        {
            case GL_REPLACE:
            case GL_MODULATE:
            case GL_ADD:
            case GL_ADD_SIGNED:
            case GL_INTERPOLATE:
            case GL_SUBTRACT:
            case GL_DOT3_RGB:
            case GL_DOT3_RGBA:
            /* For "ATI_texture_env_combine3" extension support */
            case GL_MODULATE_ADD_ATI:
            case GL_MODULATE_SIGNED_ADD_ATI:
            case GL_MODULATE_SUBTRACT_ATI:
                rgbCombinerFunc = param[0];
                return ;
        }
        char msg[64];
        sprintf(msg, "Unknown RGB combine function: 0x%x", param[0]);
        panic("TextureUnit", "setParameter", msg);
    }
    else if ( pname == GL_COMBINE_ALPHA )
    {
        switch ( param[0] )
        {
            case GL_REPLACE:
            case GL_MODULATE:
            case GL_ADD:
            case GL_ADD_SIGNED:
            case GL_INTERPOLATE:
            case GL_SUBTRACT:
            /* For "ATI_texture_env_combine3" extension support */
            case GL_MODULATE_ADD_ATI:
            case GL_MODULATE_SIGNED_ADD_ATI:
            case GL_MODULATE_SUBTRACT_ATI:
                alphaCombinerFunc = param[0];
                return ;
        }
        panic("TextureUnit", "setParameter", "Unknown Alpha combine function");
    }
    else if ( pname == GL_RGB_SCALE )
    {
        if ( rgbScale != 1 && rgbScale != 2 && rgbScale != 4 )
            panic("TextureUnit", "setParameter", "GL_RGB_SCALE Must be 1, 2 or 4");
        rgbScale = param[0];
        return;
    }
    else if ( pname == GL_ALPHA_SCALE )
    {
        if ( alphaScale != 1 && alphaScale != 2 && alphaScale != 4 )
            panic("TextureUnit", "setParameter", "GL_ALPHA_SCALE Must be 1, 2 or 4");
        alphaScale = param[0];
        return;
    }
    else if ( pname == GL_SOURCE0_RGB )
    {
        if ( (param[0] == GL_TEXTURE)  || (GL_TEXTURE0 <= param[0] && param[0] <= GL_TEXTURE31 ) ||
             (param[0] == GL_CONSTANT) || (param[0] == GL_PRIMARY_COLOR) || (param[0] == GL_PREVIOUS) ||
             (param[0] == GL_ZERO)     || (param[0] == GL_ONE)) /* Lasts ones for "NV_texture_env_combine4" and
                                                                   "ATI_texture_env_combine3" extensions support */
        {
            srcRGB[0] = param[0];
            return;
        }
        else
            panic("TextureUnit", "setParameter", "GL_SOURCE0_RGB unexpected value");
    }
    else if ( pname == GL_SOURCE1_RGB )
    {
        if ( (param[0] == GL_TEXTURE)  || (GL_TEXTURE0 <= param[0] && param[0] <= GL_TEXTURE31 ) ||
             (param[0] == GL_CONSTANT) || (param[0] == GL_PRIMARY_COLOR) || (param[0] == GL_PREVIOUS) ||
             (param[0] == GL_ZERO)     || (param[0] == GL_ONE)) /* Lasts ones for "NV_texture_env_combine4" and
                                                                   "ATI_texture_env_combine3" extensions support */
        {
            srcRGB[1] = param[0];
            return;
        }
        else
            panic("TextureUnit", "setParameter", "GL_SOURCE1_RGB unexpected value");
    }
    else if ( pname == GL_SOURCE2_RGB )
    {
        if ( (param[0] == GL_TEXTURE)  || (GL_TEXTURE0 <= param[0] && param[0] <= GL_TEXTURE31 ) ||
             (param[0] == GL_CONSTANT) || (param[0] == GL_PRIMARY_COLOR) || (param[0] == GL_PREVIOUS) ||
             (param[0] == GL_ZERO)     || (param[0] == GL_ONE)) /* Lasts ones for "NV_texture_env_combine4" and
                                                                   "ATI_texture_env_combine3" extensions support */
        {
            srcRGB[2] = param[0];
            return;
        }
        else
            panic("TextureUnit", "setParameter", "GL_SOURCE2_RGB unexpected value");
    }
    /* For "NV_texture_env_combine4" extension support */
    else if ( pname == GL_SOURCE3_RGB_NV )
    {
        if ( (param[0] == GL_TEXTURE)  || (GL_TEXTURE0 <= param[0] && param[0] <= GL_TEXTURE31 ) ||
             (param[0] == GL_CONSTANT) || (param[0] == GL_PRIMARY_COLOR) || (param[0] == GL_PREVIOUS) ||
             (param[0] == GL_ZERO))
        {
            srcRGB[3] = param[0];
            return;
        }
        else
            panic("TextureUnit", "setParameter", "GL_SOURCE3_RBG_NV unexpected value");
    }

    else if ( pname == GL_SOURCE0_ALPHA )
    {
        if ( (param[0] == GL_TEXTURE)  || (GL_TEXTURE0 <= param[0] && param[0] <= GL_TEXTURE31 ) ||
             (param[0] == GL_CONSTANT) || (param[0] == GL_PRIMARY_COLOR) || (param[0] == GL_PREVIOUS) ||
             (param[0] == GL_ZERO)     || (param[0] == GL_ONE)) /* Lasts ones for "NV_texture_env_combine4" and
                                                                   "ATI_texture_env_combine3" extensions support */
        {
            srcAlpha[0] = param[0];
            return;
        }
        else
            panic("TextureUnit", "setParameter", "GL_SOURCE0_ALPHA unexpected value");
    }
    else if ( pname == GL_SOURCE1_ALPHA )
    {
        if ( (param[0] == GL_TEXTURE)  || (GL_TEXTURE0 <= param[0] && param[0] <= GL_TEXTURE31 ) ||
             (param[0] == GL_CONSTANT) || (param[0] == GL_PRIMARY_COLOR) || (param[0] == GL_PREVIOUS) ||
             (param[0] == GL_ZERO)     || (param[0] == GL_ONE)) /* Lasts ones for "NV_texture_env_combine4" and
                                                                   "ATI_texture_env_combine3" extensions support */
        {
            srcAlpha[1] = param[0];
            return;
        }
        else
            panic("TextureUnit", "setParameter", "GL_SOURCE1_ALPHA unexpected value");
    }
    else if ( pname == GL_SOURCE2_ALPHA )
    {
        if ( (param[0] == GL_TEXTURE)  || (GL_TEXTURE0 <= param[0] && param[0] <= GL_TEXTURE31 ) ||
             (param[0] == GL_CONSTANT) || (param[0] == GL_PRIMARY_COLOR) || (param[0] == GL_PREVIOUS) ||
             (param[0] == GL_ZERO)     || (param[0] == GL_ONE)) /* Lasts ones for "NV_texture_env_combine4" and
                                                                   "ATI_texture_env_combine3" extensions support */
        {
            srcAlpha[2] = param[0];
            return;
        }
        else
            panic("TextureUnit", "setParameter", "GL_SOURCE2_ALPHA unexpected value");
    }
    /* For "NV_texture_env_combine4" extension support */
    else if ( pname == GL_SOURCE3_ALPHA_NV )
    {
        if ( (param[0] == GL_TEXTURE)  || (GL_TEXTURE0 <= param[0] && param[0] <= GL_TEXTURE31 ) ||
             (param[0] == GL_CONSTANT) || (param[0] == GL_PRIMARY_COLOR) || (param[0] == GL_PREVIOUS) ||
             (param[0] == GL_ZERO))
        {
            srcAlpha[3] = param[0];
            return;
        }
        else
            panic("TextureUnit", "setParameter", "GL_SOURCE3_ALPHA_NV unexpected value");
    }
        
    else if ( pname == GL_OPERAND0_RGB )
    {
        switch ( param[0] )
        {
            case GL_SRC_COLOR:
            case GL_ONE_MINUS_SRC_COLOR:
            case GL_SRC_ALPHA:
            case GL_ONE_MINUS_SRC_ALPHA:
                operandRGB[0] = param[0];
                return ;
        }
        panic("TextureUnit", "setParameter", "GL_OPERAND0_RGB unexpected value");
    }
    else if ( pname == GL_OPERAND1_RGB )
    {
        switch ( param[0] )
        {
            case GL_SRC_COLOR:
            case GL_ONE_MINUS_SRC_COLOR:
            case GL_SRC_ALPHA:
            case GL_ONE_MINUS_SRC_ALPHA:
                operandRGB[1] = param[0];
                return ;
        }
        panic("TextureUnit", "setParameter", "GL_OPERAND1_RGB unexpected value");
    }
    else if ( pname == GL_OPERAND2_RGB )
    {
        switch ( param[0] )
        {
            case GL_SRC_COLOR:
            case GL_ONE_MINUS_SRC_COLOR:
            case GL_SRC_ALPHA:
            case GL_ONE_MINUS_SRC_ALPHA:
                operandRGB[2] = param[0];
                return ;
        }
        panic("TextureUnit", "setParameter", "GL_OPERAND2_RGB unexpected value");
    }
    /* For "NV_texture_env_combine4" extension support */
    else if ( pname == GL_OPERAND3_RGB_NV )
    {
        switch ( param[0] )
        {
            case GL_SRC_COLOR:
            case GL_ONE_MINUS_SRC_COLOR:
            case GL_SRC_ALPHA:
            case GL_ONE_MINUS_SRC_ALPHA:
                operandRGB[3] = param[0];
                return ;
        }
        panic("TextureUnit", "setParameter", "GL_OPERAND3_RGB_NV unexpected value");
    }
    else if ( pname == GL_OPERAND0_ALPHA )
    {
        switch( param[0] )
        {
            case GL_SRC_ALPHA:
            case GL_ONE_MINUS_SRC_ALPHA:
                operandAlpha[0] = param[0];
                return;
        }
        panic("TextureUnit", "setParameter", "GL_OPERAND0_ALPHA unexpected value");
    }
    else if ( pname == GL_OPERAND1_ALPHA )
    {
        switch( param[0] )
        {
            case GL_SRC_ALPHA:
            case GL_ONE_MINUS_SRC_ALPHA:
                operandAlpha[1] = param[0];
                return;
        }
        panic("TextureUnit", "setParameter", "GL_OPERAND1_ALPHA unexpected value");
    }
    else if ( pname == GL_OPERAND2_ALPHA )
    {
        switch( param[0] )
        {
            case GL_SRC_ALPHA:
            case GL_ONE_MINUS_SRC_ALPHA:
                operandAlpha[2] = param[0];
                return;
        }
        panic("TextureUnit", "setParameter", "GL_OPERAND2_ALPHA unexpected value");
    }
    /* For "NV_texture_env_combine4" extension support */
    else if ( pname == GL_OPERAND3_ALPHA_NV )
    {
        switch( param[0] )
        {
            case GL_SRC_ALPHA:
            case GL_ONE_MINUS_SRC_ALPHA:
                operandAlpha[3] = param[0];
                return;
        }
        panic("TextureUnit", "setParameter", "GL_OPERAND3_ALPHA_NV unexpected value");
    }
    panic("TextureUnit", "setParameter", "Unexpected Parameter");

}

GLenum TextureUnit::getSrcRGB(GLuint n) const
{
    if ( n >= 4 )
        panic("TextureUnit", "getSrcRGB", "only RGB sources from 0 to 3 are available");
    return srcRGB[n];
}

GLenum TextureUnit::getSrcAlpha(GLuint n) const
{
    if ( n >= 4 )
        panic("TextureUnit", "getSrcAlpha", "only alpha sources from 0 to 3 are available");
    return srcAlpha[n];
}

GLenum TextureUnit::getOperandRGB(GLuint n) const
{
    if ( n >= 4 )
        panic("TextureUnit", "getOperandRGB", "only RGB operands from 0 to 3 are available");
    return operandRGB[n];
}

GLenum TextureUnit::getOperandAlpha(GLuint n) const
{
    if ( n >= 4 )
        panic("TextureUnit", "getOperandAlpha", "only alpha operands from 0 to 3 are available");
    return operandAlpha[n];
}

void TextureUnit::setTexGenMode(GLenum coord, GLint mode)
{
    if ( mode != GL_OBJECT_LINEAR && mode != GL_EYE_LINEAR && mode != GL_REFLECTION_MAP &&
         mode != GL_SPHERE_MAP && mode != GL_NORMAL_MAP )
         panic("TextureUnit", "setTexGenMode", "Unexpected mode");

    if ( coord == GL_S )
        texGenModeS = mode;
    else if ( coord == GL_T )
        texGenModeT = mode;
    else if ( coord == GL_R )
        texGenModeR = mode;
    else if ( coord == GL_Q )
        texGenModeQ = mode;
    else
        panic("TextureUnit", "setTexGenMode", "Unexpected coordinate");
}

GLint TextureUnit::getTexGenMode(GLenum coord) const
{
    if ( coord == GL_S )
        return texGenModeS;
    else if ( coord == GL_T )
        return texGenModeT;
    else if ( coord == GL_R )
        return texGenModeR;
    else if ( coord == GL_Q )
        return texGenModeQ;
    else
        panic("TextureUnit", "getTexGenMode", "Unexpected coordinate");
    return 0;
}

void TextureUnit::setEnabledTexGen(GLenum coord, bool active)
{
    if ( coord == GL_S )
        activeTexGenS = active;
    else if ( coord == GL_T )
        activeTexGenT = active;
    else if ( coord == GL_R )
        activeTexGenR = active;
    else if ( coord == GL_Q )
        activeTexGenQ = active;
    else
        panic("TextureUnit", "setEnabledTexGen", "Unexpected coordinate");
}


bool TextureUnit::isEnabledTexGen(GLenum coord) const
{
    if ( coord == GL_S )
        return activeTexGenS;
    else if ( coord == GL_T )
        return activeTexGenT;
    else if ( coord == GL_R )
        return activeTexGenR;
    else if ( coord == GL_Q )
        return activeTexGenQ;
    else
        panic("TextureUnit", "isEnabledTexGen", "Unexpected coordinate");
    return false;
}

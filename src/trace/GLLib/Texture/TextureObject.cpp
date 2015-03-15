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

#include "TextureObject.h"
#include "AuxFuncsLib.h"
#include "TextureTarget.h"
#include "MathLib.h"
#include "GPUMath.h"
#include "GPULibInternals.h"
#include "glext.h"
#include <iostream>

using namespace libgl;

TextureObject::Mipmap::Mipmap() :
originalData(0),
data(0),
size(0),
imageCompression(false),
width(0),
height(0),
depth(0),
border(0),
ifmt(1)
{

    // Hack for default textures
    GLubyte temp[3];
    temp[0] = 255;
    temp[1] = 255;
    temp[2] = 255;
    set2D(GL_RGBA, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, temp, 0);

}

TextureObject::Mipmap::Mipmap(const Mipmap &mip)
{
    data = new GLubyte[mip.size];
    data = new GLubyte[mip.size];
    memcpy(data, mip.data, mip.size);
    size = mip.size;
    width = mip.width;
    height = mip.height;
    depth  = mip.depth;
    border = mip.border;
    ifmt = mip.ifmt;
    red  = mip.red;
    green = mip.green;
    blue  = mip.blue;
    alpha  = mip.alpha;
    luminance  = mip.luminance;
    intensity  = mip.intensity;
    imageCompression  = mip.imageCompression;
    format = mip.format;
    type  = mip.type;

}

TextureObject::Mipmap::~Mipmap()
{
    //delete[] originalData;
    delete[] data;
    data = 0;
    originalData = 0;
}

void TextureObject::Mipmap::setWithDefaults()
{
    /*
    width = 0;
    height = 0;
    depth = 0;
    border = 0;
    ifmt = GL_LUMINANCE;
    red = 0;
    green = 0;
    blue = 0;
    alpha = 0;
    luminance = 0;
    intensity = 0;
    imageCompression = false;
    */
}

GLuint TextureObject::Mipmap::getImageSize()
{
    return width * height * afl::getSize(format);
}



void TextureObject::Mipmap::set3D(GLint ifmt_, GLint width_, GLint height_, GLint depth_, GLint border_, GLenum format_,
                            GLenum type_, const GLvoid* data_, GLint compressedSize_)
{
    panic("TextureObject::Mipmap", "set3D", "Texture object with 3D dimensionality are not supported for now");
}




void TextureObject::Mipmap::set1D(GLint ifmt_, GLint width_, GLint border_, GLenum format_,
                                  GLenum type_, const GLvoid* data_, GLint compressedSize_)
{
    if ( format_ != GL_RGBA )
        panic("TextureObject::Mipmap", "set1D", "only GL_RGBA format supported for now");
    if ( type_ != GL_UNSIGNED_BYTE )
        panic("TextureObject::Mipmap", "set1D", "only GL_UNSIGNED_BYTE type supported for now");
    if ( border_ != 0 )
        panic("TextureObject::Mipmap", "set1D", "border different of 0 is not supported for now");

    ifmt = ifmt_;
    width = width_;
    height = 1;
    depth = 1;
    border = border_;
    size = compressedSize_;
    format = format_;
    type = type_;

    size = width * height * afl::getSize(type_);
    data = new GLubyte[size];
    memcpy(data, data_, size); // no special formating required
}

void TextureObject::Mipmap::convertALPHAtoALPHA(const GLubyte* input, GLubyte* output,
                                          GLsizei ics, GLsizei ocs,
                                          GLsizei width, GLint height)
{
    if ( ics != 1 && ics != 2 && ics != 4 )
        panic("TextureObject", "convertALPHAtoALPHA", "inputComponent size must be 1, 2 or 4");

    if ( ocs != 1 && ocs != 2 && ocs != 4 )
        panic("TextureObject", "convertALPHAtoALPHA", "outputComponent size must be 1, 2 or 4");

    const GLubyte* ub_iptr = input;
    const GLushort* us_iptr = (GLushort*)input;
    const GLuint* ui_iptr = (GLuint*)input;

    GLubyte* ub_optr = output;
    GLushort* us_optr = (GLushort*)output;
    GLuint* ui_optr = (GLuint*)output;

    GLsizei texels = width*height;

    GLdouble a;

    for ( GLint i = 0; i < texels; i++ )
    {
        if ( ics == 1 )
        {
            //cout << "(" << GLint(ub_iptr[i*4]) << "," << GLint(ub_iptr[i*4+1]) << "," << GLint(ub_iptr[i*4+2]) << "," << GLint(ub_iptr[i*4+3]) << ")" << endl;
            a = GLdouble(ub_iptr[i*4]) / 0xFF;
        }
        else if ( ics == 2 )
            a = GLdouble(us_iptr[i*4]) / 0xFFFF;
        else if ( ics == 4 )
            a = GLdouble(ui_iptr[i*4]) / 0xFFFFFFFF;

        if ( ocs == 1 )
            ub_optr[i] = GLubyte(a * 0xFF);
        else if ( ocs == 2 )
            us_optr[i] = GLushort(a * 0xFFFF);
        else if ( ocs == 4 )
            ui_optr[i] = GLuint(a * 0xFFFFFFFF);
    }
}

void TextureObject::Mipmap::convertLUMINANCEtoLUMINANCE(const GLubyte* input, GLubyte* output,
                                                        GLsizei ics, GLsizei ocs,
                                                        GLsizei width, GLint height)
{
    if ( ics != 1 && ics != 2 && ics != 4 )
        panic("TextureObject", "convertLUMINANCEtoLUMINANCE", "inputComponent size must be 1, 2 or 4");

    if ( ocs != 1 && ocs != 2 && ocs != 4 )
        panic("TextureObject", "convertLUMINANCEtoLUMINANCE", "outputComponent size must be 1, 2 or 4");

    const GLubyte* ub_iptr = input;
    const GLushort* us_iptr = (GLushort*)input;
    const GLuint* ui_iptr = (GLuint*)input;

    GLubyte* ub_optr = output;
    GLushort* us_optr = (GLushort*)output;
    GLuint* ui_optr = (GLuint*)output;

    GLsizei texels = width*height;

    GLdouble l;

    for ( GLint i = 0; i < texels; i++ )
    {
        if ( ics == 1 )
        {
            //cout << "(" << GLint(ub_iptr[i*4]) << "," << GLint(ub_iptr[i*4+1]) << "," << GLint(ub_iptr[i*4+2]) << "," << GLint(ub_iptr[i*4+3]) << ")" << endl;
            l = GLdouble(ub_iptr[i*4]) / 0xFF;
        }
        else if ( ics == 2 )
            l = GLdouble(us_iptr[i*4]) / 0xFFFF;
        else if ( ics == 4 )
            l = GLdouble(ui_iptr[i*4]) / 0xFFFFFFFF;

        if ( ocs == 1 )
            ub_optr[i] = GLubyte(l * 0xFF);
        else if ( ocs == 2 )
            us_optr[i] = GLushort(l * 0xFFFF);
        else if ( ocs == 4 )
            ui_optr[i] = GLuint(l * 0xFFFFFFFF);
    }
}

void TextureObject::Mipmap::convertRGBAtoINTENSITY(const GLubyte* input, GLubyte* output,
                                         GLsizei ics, GLsizei ocs,
                                         GLsizei width, GLint height)
{
    if ( ics != 1 && ics != 2 && ics != 4 )
        panic("TextureObject", "convertRGBAtoINTENSITY", "inputComponent size must be 1, 2 or 4");

    if ( ocs != 1 && ocs != 2 && ocs != 4 )
        panic("TextureObject", "convertRGBAtoINTENSITY", "outputComponent size must be 1, 2 or 4");

    const GLubyte* ub_iptr = input;
    const GLushort* us_iptr = (GLushort*)input;
    const GLuint* ui_iptr = (GLuint*)input;

    GLubyte* ub_optr = output;
    GLushort* us_optr = (GLushort*)output;
    GLuint* ui_optr = (GLuint*)output;

    GLsizei texels = width*height;

    GLdouble r;

    for ( GLint i = 0; i < texels; i++ )
    {
        if ( ics == 1 )
        {
            //cout << "(" << GLint(ub_iptr[i*4]) << "," << GLint(ub_iptr[i*4+1]) << "," << GLint(ub_iptr[i*4+2]) << "," << GLint(ub_iptr[i*4+3]) << ")" << endl;
            r = GLdouble(ub_iptr[i*4]) / 0xFF;
        }
        else if ( ics == 2 )
            r = GLdouble(us_iptr[i*4]) / 0xFFFF;
        else if ( ics == 4 )
            r = GLdouble(ui_iptr[i*4]) / 0xFFFFFFFF;

        if ( ocs == 1 )
            ub_optr[i] = GLubyte(r * 0xFF);
        else if ( ocs == 2 )
            us_optr[i] = GLushort(r * 0xFFFF);
        else if ( ocs == 4 )
            ui_optr[i] = GLuint(r * 0xFFFFFFFF);
    }
}


void TextureObject::Mipmap::convertRGBtoRGB(const GLubyte* input, GLubyte* output,
                                    GLsizei ics, GLsizei ocs,
                                    GLsizei width, GLsizei height)
{
    if ( ics != 1 && ics != 2 && ics != 4 )
        panic("TextureObject", "convertRGBtoRGB", "inputComponent size must be 1, 2 or 4");

    if ( ocs != 1 && ocs != 2 && ocs != 4 )
        panic("TextureObject", "convertRGBtoRGB", "outputComponent size must be 1, 2 or 4");

    const GLubyte* ub_iptr = input;
    const GLushort* us_iptr = (GLushort*)input;
    const GLuint* ui_iptr = (GLuint*)input;

    GLubyte* ub_optr = output;
    GLushort* us_optr = (GLushort*)output;
    GLuint* ui_optr = (GLuint*)output;

    GLsizei texels = width*height; // number of texels

    GLdouble r, g, b; // temporary values

    for ( GLint i = 0; i < texels; i++ )
    {
        if ( ics == 1 )
        {
            r = GLdouble(ub_iptr[i*3]) / 0xFF;
            g = GLdouble(ub_iptr[i*3+1]) / 0xFF;
            b = GLdouble(ub_iptr[i*3+2]) / 0xFF;
        }
        else if ( ics == 2 )
        {
            r = GLdouble(us_iptr[i*3]) / 0xFFFF;
            g = GLdouble(us_iptr[i*3+1]) / 0xFFFF;
            b = GLdouble(us_iptr[i*3+2]) / 0xFFFF;
        }
        else if ( ics == 4 )
        {
            r = GLdouble(ui_iptr[i*3]) / 0xFFFFFFFF;
            g = GLdouble(ui_iptr[i*3+1]) / 0xFFFFFFFF;
            b = GLdouble(ui_iptr[i*3+2]) / 0xFFFFFFFF;
        }

        // r, g and b contains values in the range [0..1]

        if ( ocs == 1 )
        {
            ub_optr[i*3] = GLubyte(r * 0xFF);
            ub_optr[i*3+1] = GLubyte(g * 0xFF);
            ub_optr[i*3+2] = GLubyte(b * 0xFF);

        }
        else if ( ocs == 2 )
        {
            us_optr[i*3] = GLushort(r * 0xFFFF);
            us_optr[i*3+1] = GLushort(g * 0xFFFF);
            us_optr[i*3+2] = GLushort(b * 0xFFFF);
        }
        else if ( ocs == 4 )
        {
            ui_optr[i*3] = GLuint(r * 0xFFFFFFFF);
            ui_optr[i*3+1] = GLuint(g * 0xFFFFFFFF);
            ui_optr[i*3+2] = GLuint(b * 0xFFFFFFFF);
        }
    }
}


void TextureObject::Mipmap::convertBGRtoRGBA(const GLubyte* input, GLubyte* output,
                                                    GLsizei ics, GLsizei ocs,
                                                    GLsizei width, GLint height)
{

    if ( ics != 1 && ics != 2 && ics != 4 )
        panic("TextureObject", "convertBGRtoRGBA", "inputComponent size must be 1, 2 or 4");

    if ( ocs != 1 && ocs != 2 && ocs != 4 )
        panic("TextureObject", "convertBGRtoRGBA", "outputComponent size must be 1, 2 or 4");

    const GLubyte* ub_iptr = input;
    const GLushort* us_iptr = (GLushort*)input;
    const GLuint* ui_iptr = (GLuint*)input;

    GLubyte* ub_optr = output;
    GLushort* us_optr = (GLushort*)output;
    GLuint* ui_optr = (GLuint*)output;

    GLsizei texels = width*height; // number of texels

    GLdouble r, g, b; // temporary values

    for ( GLint i = 0; i < texels; i++ )
    {
        if ( ics == 1 )
        {
            b = GLdouble(ub_iptr[i*3]) / 0xFF;
            g = GLdouble(ub_iptr[i*3+1]) / 0xFF;
            r = GLdouble(ub_iptr[i*3+2]) / 0xFF;
        }
        else if ( ics == 2 )
        {
            b = GLdouble(us_iptr[i*3]) / 0xFFFF;
            g = GLdouble(us_iptr[i*3+1]) / 0xFFFF;
            r = GLdouble(us_iptr[i*3+2]) / 0xFFFF;
        }
        else if ( ics == 4 )
        {
            b = GLdouble(ui_iptr[i*3]) / 0xFFFFFFFF;
            g = GLdouble(ui_iptr[i*3+1]) / 0xFFFFFFFF;
            r = GLdouble(ui_iptr[i*3+2]) / 0xFFFFFFFF;
        }

        // r, g and b values contain values in the range [0..1]

        if ( ocs == 1 )
        {
            ub_optr[i*4] = GLubyte(r * 0xFF);
            ub_optr[i*4+1] = GLubyte(g * 0xFF);
            ub_optr[i*4+2] = GLubyte(b * 0xFF);
            ub_optr[i*4+3] = 0xFF;
        }
        else if ( ocs == 2 )
        {
            us_optr[i*4] = GLushort(r * 0xFFFF);
            us_optr[i*4+1] = GLushort(g * 0xFFFF);
            us_optr[i*4+2] = GLushort(b * 0xFFFF);
            us_optr[i*4+3] = 0xFFFF;
        }
        else if ( ocs == 4 )
        {
            ui_optr[i*4] = GLuint(r * 0xFFFFFFFF);
            ui_optr[i*4+1] = GLuint(g * 0xFFFFFFFF);
            ui_optr[i*4+2] = GLuint(b * 0xFFFFFFFF);
            ui_optr[i*4+3] = 0xFFFFFFFF;
        }
    }
}

void TextureObject::Mipmap::convertBGRAtoRGBA(const GLubyte* input, GLubyte* output,
                                                     GLsizei ics, GLsizei ocs,
                                                     GLsizei width, GLint height)
{
    if ( ics != 1 && ics != 2 && ics != 4 )
        panic("TextureObject", "convertBGRAtoRGBA", "inputComponent size must be 1, 2 or 4");

    if ( ocs != 1 && ocs != 2 && ocs != 4 )
        panic("TextureObject", "convertBGRAtoRGBA", "outputComponent size must be 1, 2 or 4");

    const GLubyte* ub_iptr = input;
    const GLushort* us_iptr = (GLushort*)input;
    const GLuint* ui_iptr = (GLuint*)input;

    GLubyte* ub_optr = output;
    GLushort* us_optr = (GLushort*)output;
    GLuint* ui_optr = (GLuint*)output;

    GLsizei texels = width*height; // number of texels

    GLdouble r, g, b, a; // temporary values

    for ( GLint i = 0; i < texels; i++ )
    {
        if ( ics == 1 )
        {
            b = GLdouble(ub_iptr[i*4]) / 0xFF;
            g = GLdouble(ub_iptr[i*4+1]) / 0xFF;
            r = GLdouble(ub_iptr[i*4+2]) / 0xFF;
            a = GLdouble(ub_iptr[i*4+3]) / 0xFF;
        }
        else if ( ics == 2 )
        {
            b = GLdouble(us_iptr[i*4]) / 0xFFFF;
            g = GLdouble(us_iptr[i*4+1]) / 0xFFFF;
            r = GLdouble(us_iptr[i*4+2]) / 0xFFFF;
            a = GLdouble(us_iptr[i*4+3]) / 0xFFFF;
        }
        else if ( ics == 4 )
        {
            b = GLdouble(ui_iptr[i*4]) / 0xFFFFFFFF;
            g = GLdouble(ui_iptr[i*4+1]) / 0xFFFFFFFF;
            r = GLdouble(ui_iptr[i*4+2]) / 0xFFFFFFFF;
            a = GLdouble(ui_iptr[i*4+3]) / 0xFFFFFFFF;
        }

        // r, g, b and a contains values in the range [0..1]

        if ( ocs == 1 )
        {
            ub_optr[i*4] = GLubyte(r * 0xFF);
            ub_optr[i*4+1] = GLubyte(g * 0xFF);
            ub_optr[i*4+2] = GLubyte(b * 0xFF);
            ub_optr[i*4+3] = GLubyte(a * 0xFF);
        }
        else if ( ocs == 2 )
        {
            us_optr[i*4] = GLushort(r * 0xFFFF);
            us_optr[i*4+1] = GLushort(g * 0xFFFF);
            us_optr[i*4+2] = GLushort(b * 0xFFFF);
            us_optr[i*4+3] = GLushort(a * 0xFFFF);
        }
        else if ( ocs == 4 )
        {
            ui_optr[i*4] = GLuint(r * 0xFFFFFFFF);
            ui_optr[i*4+1] = GLuint(g * 0xFFFFFFFF);
            ui_optr[i*4+2] = GLuint(b * 0xFFFFFFFF);
            ui_optr[i*4+3] = GLuint(a * 0xFFFFFFFF);
        }
    }
}


void TextureObject::Mipmap::convertRGBtoRGBA(const GLubyte* input, GLubyte* output,
                                    GLsizei ics, GLsizei ocs,  /* inputComponentSize and outputComponentSize */
                                    GLsizei width, GLsizei height)
{

    if ( ics != 1 && ics != 2 && ics != 4 )
        panic("TextureObject", "convertRGBtoRGBA", "inputComponent size must be 1, 2 or 4");

    if ( ocs != 1 && ocs != 2 && ocs != 4 )
        panic("TextureObject", "convertRGBtoRGBA", "outputComponent size must be 1, 2 or 4");

    const GLubyte* ub_iptr = input;
    const GLushort* us_iptr = (GLushort*)input;
    const GLuint* ui_iptr = (GLuint*)input;

    GLubyte* ub_optr = output;
    GLushort* us_optr = (GLushort*)output;
    GLuint* ui_optr = (GLuint*)output;

    GLsizei texels = width*height; // number of texels

    GLdouble r, g, b; // temporary values

    for ( GLint i = 0; i < texels; i++ )
    {
        if ( ics == 1 )
        {
            r = GLdouble(ub_iptr[i*3]) / 0xFF;
            g = GLdouble(ub_iptr[i*3+1]) / 0xFF;
            b = GLdouble(ub_iptr[i*3+2]) / 0xFF;
        }
        else if ( ics == 2 )
        {
            r = GLdouble(us_iptr[i*3]) / 0xFFFF;
            g = GLdouble(us_iptr[i*3+1]) / 0xFFFF;
            b = GLdouble(us_iptr[i*3+2]) / 0xFFFF;
        }
        else if ( ics == 4 )
        {
            r = GLdouble(ui_iptr[i*3]) / 0xFFFFFFFF;
            g = GLdouble(ui_iptr[i*3+1]) / 0xFFFFFFFF;
            b = GLdouble(ui_iptr[i*3+2]) / 0xFFFFFFFF;
        }

        // r, g and b contains values in the range [0..1]

        if ( ocs == 1 )
        {
            ub_optr[i*4] = GLubyte(r * 0xFF);
            ub_optr[i*4+1] = GLubyte(g * 0xFF);
            ub_optr[i*4+2] = GLubyte(b * 0xFF);
            ub_optr[i*4+3] = 0xFF;
        }
        else if ( ocs == 2 )
        {
            us_optr[i*4] = GLushort(r * 0xFFFF);
            us_optr[i*4+1] = GLushort(g * 0xFFFF);
            us_optr[i*4+2] = GLushort(b * 0xFFFF);
            us_optr[i*4+3] = 0xFFFF;
        }
        else if ( ocs == 4 )
        {
            ui_optr[i*4] = GLuint(r * 0xFFFFFFFF);
            ui_optr[i*4+1] = GLuint(g * 0xFFFFFFFF);
            ui_optr[i*4+2] = GLuint(b * 0xFFFFFFFF);
            ui_optr[i*4+3] = 0xFFFFFFFF;
        }
    }
}


void TextureObject::Mipmap::convertRGBAtoRGBA(const GLubyte* input, GLubyte* output,
                                    GLsizei ics, GLsizei ocs,  /* inputComponentSize and outputComponentSize */
                                    GLsizei width, GLsizei height)
{

    if ( ics != 1 && ics != 2 && ics != 4 )
        panic("TextureObject", "convertRGBAtoRGBA", "inputComponent size must be 1, 2 or 4");

    if ( ocs != 1 && ocs != 2 && ocs != 4 )
        panic("TextureObject", "convertRGBAtoRGBA", "outputComponent size must be 1, 2 or 4");

    const GLubyte* ub_iptr = input;
    const GLushort* us_iptr = (GLushort*)input;
    const GLuint* ui_iptr = (GLuint*)input;

    GLubyte* ub_optr = output;
    GLushort* us_optr = (GLushort*)output;
    GLuint* ui_optr = (GLuint*)output;

    GLsizei texels = width*height; // number of texels

    GLdouble r, g, b, a; // temporary values

    for ( GLint i = 0; i < texels; i++ )
    {
        if ( ics == 1 )
        {
            r = GLdouble(ub_iptr[i*3]) / 0xFF;
            g = GLdouble(ub_iptr[i*3+1]) / 0xFF;
            b = GLdouble(ub_iptr[i*3+2]) / 0xFF;
            a = GLdouble(ub_iptr[i*3+3]) / 0xFF;
        }
        else if ( ics == 2 )
        {
            r = GLdouble(us_iptr[i*3]) / 0xFFFF;
            g = GLdouble(us_iptr[i*3+1]) / 0xFFFF;
            b = GLdouble(us_iptr[i*3+2]) / 0xFFFF;
            a = GLdouble(us_iptr[i*3+3]) / 0xFFFF;
        }
        else if ( ics == 4 )
        {
            r = GLdouble(ui_iptr[i*3]) / 0xFFFFFFFF;
            g = GLdouble(ui_iptr[i*3+1]) / 0xFFFFFFFF;
            b = GLdouble(ui_iptr[i*3+2]) / 0xFFFFFFFF;
            a = GLdouble(ui_iptr[i*3+3]) / 0xFFFFFFFF;
        }

        // r, g, b and a contains values in the range [0..1]

        if ( ocs == 1 )
        {
            ub_optr[i*4] = GLubyte(r * 0xFF);
            ub_optr[i*4+1] = GLubyte(g * 0xFF);
            ub_optr[i*4+2] = GLubyte(b * 0xFF);
            ub_optr[i*4+3] = GLubyte(a * 0xFF);
        }
        else if ( ocs == 2 )
        {
            us_optr[i*4] = GLushort(r * 0xFFFF);
            us_optr[i*4+1] = GLushort(g * 0xFFFF);
            us_optr[i*4+2] = GLushort(b * 0xFFFF);
            us_optr[i*4+3] = GLushort(a * 0xFFFF);
        }
        else if ( ocs == 4 )
        {
            ui_optr[i*4] = GLuint(r * 0xFFFFFFFF);
            ui_optr[i*4+1] = GLuint(g * 0xFFFFFFFF);
            ui_optr[i*4+2] = GLuint(b * 0xFFFFFFFF);
            ui_optr[i*4+3] = GLuint(a * 0xFFFFFFFF);
        }
    }
}

void TextureObject::Mipmap::set2D(GLint ifmt_, GLint width_, GLint height_, GLint border_, GLenum format_,
                                  GLenum type_, const GLvoid* data_, GLint compressedSize_)
{
    u32bit tileLevel1Sz;
    u32bit tileLevel2Sz;
    bool formatConversion = false;
    GLubyte *convertedData = NULL;
    GLuint originalSize = 0;

    if ( data != 0 ) // Delete previous texture mipmap image contents
    {
        //delete[] originalData;
        delete[] data;
        originalData = 0;
        data = 0;
        size = 0;
    }

    if ( ifmt_ == 4 ) ifmt_ = GL_RGBA; // Patch for backward compatibility
    if ( ifmt_ == 3 ) ifmt_ = GL_RGB; // Patch for backward compatibility

    // Copy parameters
    ifmt = ifmt_;
    width = width_;
    height = height_;
    depth = 1;
    border = border_;
    format = format_;
    type = type_;
    size = compressedSize_;

    // Check border (for now borders are not supported)
    if ( border != 0 )
        panic("TextureObject::Mipmap", "set2D", "border different of 0 is not supported for now");

    /*  Get the texture tiling parameters from the driver.  Note:  May be the parameters should come
        from another class higher in the Texture Object hierarchy rather than directly from the driver.  */
    driver->getTextureTilingParameters(tileLevel1Sz, tileLevel2Sz);

    if ( size != 0 ) // Compressed texture (glCompressedTexImage2D)
    {
        imageCompression = true;

        GLint i;
        GLint j;
        u32bit k;
        u32bit w2;
        u32bit a;
        u32bit s3tcBlockSz;

        switch ( ifmt )
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                s3tcBlockSz = 8;
                break; // OK
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                s3tcBlockSz = 16;
                break; // OK

            default:
                panic("TextureObject", "set2D", "Unexpected compressed internal format");
        }

        if ( width_ < 4 || height_ < 4 )
        {
            popup("TextureObject", "Warning. S3TC textures width and height must be a multiple of 4 (see spec).\n"
                                   "Setting a not compressed default 1x1 black texture instead in mipmap level.\n"
                                   "Caution: This can trigger a texture completeness panic.");
            GLubyte temp[3];
            temp[0] = 0;
            temp[1] = 0;
            temp[2] = 0;
            set2D(GL_RGBA, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, temp, 0);
            return ;
        }

        originalData = new GLubyte[compressedSize_];
        memcpy(originalData, data_, compressedSize_); // In order to save the original Texture data

        /*originalData = new GLubyte[size];
        memcpy(originalData, data_, size); // In order to save the original Texture data

        w2 = (u32bit) mathlib::ceil2(mathlib::log2(width_ >> 2));
        size = s3tcBlockSz * (texel2address(w2, tileLevel1Sz - 2, tileLevel2Sz, (width >> 2) - 1, (height >> 2) - 1) + 1);
        data = new GLubyte[size];

        // Format texture data (morton)
        for(i = 0; i < (height_ >> 2); i++)
        {
            for(j = 0; j < (width_ >> 2); j++)
            {
                a = texel2address(w2, tileLevel1Sz - 2, tileLevel2Sz, j, i);
                for(k = 0; k < (s3tcBlockSz >> 2); k ++)
                    ((u32bit *) data)[(a * (s3tcBlockSz >> 2)) + k] = ((u32bit *) data_)[(i * (width_ >> 2) + j) * (s3tcBlockSz >> 2) + k];
            }
        }*/

        //NEWW

        gpu3d::TextureCompression compressed;
        switch (ifmt)
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
                compressed = gpu3d::GPU_S3TC_DXT1_RGB;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                compressed = gpu3d::GPU_S3TC_DXT1_RGBA;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                compressed = gpu3d::GPU_S3TC_DXT3_RGBA;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                compressed = gpu3d::GPU_S3TC_DXT5_RGBA;
                break;
            default:
                compressed = gpu3d::GPU_NO_TEXTURE_COMPRESSION;
        }
        data = driver->getDataInMortonOrder((u8bit*)data_, width, height, depth, compressed, 0, size);


        //memcpy(data, data_, size); // Use compressed texture directly

        return ; // bye
    }

    // Uncompressed texture (glTexImage2D)

    if ( type == GL_FLOAT )
        panic("TextureObject::Mipmap", "set2D", "GL_FLOAT type not yet supported");

    switch ( format ) // Check format
    {
        case GL_RGBA8:
        case GL_RGBA:
        case GL_RGB:
        case GL_RGB8:
        case GL_BGR:
        case GL_BGRA:
        case GL_ALPHA:
        case GL_LUMINANCE:
            break;
        default:
            panic("TextureObject::Mipmap", "set2D", "Format not supported");
    }


    switch ( ifmt )
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RGBA:
            // Compress texture image
            //popup("TextureObject", "Warning. defining a Mipmap to be compressed and compression is not supported. This texture will not be compressed (used as regular RGBA)");
            //cout << "Warning. defining a Mipmap to be compressed and compression is not supported. This texture will not be compressed (used as regular RGBA)" << endl;
            ifmt = GL_RGBA;
            break; // OK
        case GL_RGBA:
        case GL_RGB:
            break;
        case GL_COMPRESSED_RGB:
        case GL_RGB8:
            ifmt = GL_RGB;
        case GL_RGBA8:
            ifmt = GL_RGBA;
            break;
        case GL_INTENSITY8:
            break; // OK (native support)
        case GL_LUMINANCE:
            break;
        case GL_ALPHA:
        case GL_ALPHA8:
            break; // OK (native support)
        default:
            panic("TextureObject::Mipmap", "set2D", "Unexpected internal format");
    }

    // Compute bytes per component
    GLsizei bytesPerComponent = afl::getSize(type);

    GLsizei texelsSize = 4; // RGBA size

    if ( bytesPerComponent != 1 && bytesPerComponent != 2 && bytesPerComponent != 4 )
        panic("TextureObject::Mipmap", "set2D", "Bytes per component must be 1, 2 or 4");

    if ( format == GL_RGB && (ifmt == GL_RGB || ifmt == GL_RGBA)) // Convert to RGBA
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* rgba = new GLubyte[width*height*4]; // Assumes 1 byte per component
        originalSize = width*height*4;
        convertRGBtoRGBA((const GLubyte*)data_, rgba, bytesPerComponent, 1, width,height ); // Support RGB coverting RGB to RGBA
        data_ = convertedData = rgba;
        ifmt = GL_RGBA;
    }
    else if ( format == GL_RGBA && ifmt == GL_RGB )
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* rgba = new GLubyte[width*height*4];
        originalSize = width*height*4;
        if ( bytesPerComponent != afl::getSize(GL_UNSIGNED_BYTE) )
            convertRGBAtoRGBA((const GLubyte*)data_, rgba, bytesPerComponent, 1, width, height);
        else
            memcpy(rgba, data_, width*height*4); // Assumes UNSIGNED_BYTE
        data_ = convertedData = rgba;
        ifmt = GL_RGBA;
    }
    else if ( (format == GL_BGR || format == GL_BGRA) && (ifmt == GL_RGB || ifmt == GL_RGBA) )
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* rgba = new GLubyte[width*height*4];
        originalSize = width*height*4;
        if ( format_ == GL_BGR )
            convertBGRtoRGBA((const GLubyte*)data_, rgba, bytesPerComponent, 1, width, height);
        else
            convertBGRAtoRGBA((const GLubyte*)data_, rgba, bytesPerComponent, 1, width, height);
        data_ = convertedData = rgba;
        ifmt = GL_RGBA;
    }
    else if ( format == GL_RGBA && ifmt == GL_INTENSITY8 )
    {
        //  Format conversion performed
        formatConversion = true;

        // GL_INTENSITY8 = 1 byte
        GLubyte* intensity = new GLubyte[width*height*1];
        originalSize = width*height*1;
        convertRGBAtoINTENSITY((const GLubyte*)data_, intensity, bytesPerComponent,1, width, height);
        data_ = convertedData = intensity;
        texelsSize = 1;
    }
    else if ( format == GL_ALPHA && (ifmt == GL_ALPHA || ifmt == GL_ALPHA8 ))
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* alpha = new GLubyte[width*height];
        originalSize = width*height;
        if ( bytesPerComponent != afl::getSize(GL_UNSIGNED_BYTE) )
            convertALPHAtoALPHA((const GLubyte*)data_, alpha, bytesPerComponent,1, width, height);
        else
            memcpy(alpha, data_, width * height);

        data_ = convertedData = alpha;
        texelsSize = 1;
    }

    else if ( format == GL_LUMINANCE && ifmt == GL_LUMINANCE )
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* luminance = new GLubyte[width*height*1];
        originalSize = width*height*1;
        memcpy(luminance, data_, width*height*1); // Assumes UNSIGNED_BYTE
        data_ = convertedData = luminance;
        texelsSize = 1;
    }

    originalData = new GLubyte[originalSize];
    memcpy(originalData, data_, originalSize); // In order to save the original Texture data

    /*GLint i;
    GLint j;
    u32bit w2;
    u32bit a;


    originalData = new GLubyte[size];
    memcpy(originalData, data_, size); // In order to save the original Texture data

    w2 = (u32bit) mathlib::ceil2(mathlib::log2(width_));
    size = texelsSize*(texel2address(w2, tileLevel1Sz, tileLevel2Sz, width-1, height-1) + 1);

    data = new GLubyte[size];

    // Format texture data (morton)
    if ( texelsSize == 4 )
    {
        for(i = 0; i < height; i++)
        {
            for(j = 0; j < width; j++)
            {
                a = texel2address(w2, tileLevel1Sz, tileLevel2Sz, j, i);
                ((u32bit *) data)[a] = ((u32bit *) data_)[i * width_ + j];
            }
        }
    }
    else if ( texelsSize == 1 )
    {
        for(i = 0; i < height; i++)
        {
            for(j = 0; j < width; j++)
            {
                a = texel2address(w2, tileLevel1Sz, tileLevel2Sz, j, i);
                ((u8bit *) data)[a] = ((u8bit *) data_)[i * width_ + j];
            }
        }
    }*/

        
        //NEWW

        gpu3d::TextureCompression compressed;
        switch (ifmt)
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
                compressed = gpu3d::GPU_S3TC_DXT1_RGB;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                compressed = gpu3d::GPU_S3TC_DXT1_RGBA;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                compressed = gpu3d::GPU_S3TC_DXT3_RGBA;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                compressed = gpu3d::GPU_S3TC_DXT5_RGBA;
                break;
            default:
                compressed = gpu3d::GPU_NO_TEXTURE_COMPRESSION;
        }
        data = driver->getDataInMortonOrder((u8bit*)data_, width, height, depth, compressed, texelsSize, size);



    //  If a format conversion was performed delete the auxiliar buffer
    if (formatConversion)
        delete[] convertedData;
}


void TextureObject::Mipmap::setPartial2D(GLint xoffset, GLint yoffset, GLint width_, GLint height_,
                                         GLenum fmt_, GLenum type_, const GLvoid* subData, GLint compressedSize_)
{
    if ( fmt_ != format )
        panic("TextureObject::Mipmap", "setPartial2D", "format previously defined was diferent (maybe is not a panic)");

    if ( type_ != type )
        panic("TextureObject::Mipmap", "setPartial2D", "type previously defined was diferent (maybe is not a panic)");
    
    if (xoffset < 0)
        panic("TextureObject::Mipmap", "setPartial2D", "negative xoffset (maybe is not a panic)");
        
    if (yoffset < 0)
        panic("TextureObject::Mipmap", "setPartial2D", "negative yoffset (maybe is not a panic)");    
        
    if (xoffset >= width || yoffset >= height)
        panic("TextureObject::Mipmap", "setPartial2D", "No texture image area is updated. xoffset or yoffset lay outside the texture dimensions. (maybe is not a panic)");    

    if (width_ > width)
        panic("TextureObject::Mipmap", "setPartial2D", "previously defined width was smaller (maybe is not a panic)");
    
    if (height_ > height)
        panic("TextureObject::Mipmap", "setPartial2D", "previously defined height was smaller (maybe is not a panic)");

    if (compressedSize_ != 0)
        panic("TextureObject::Mipmap", "setPartial2D", "set partial contents of compressed textures not supported yet");

    if ( type == GL_FLOAT )
        panic("TextureObject::Mipmap", "setPartial2D", "GL_FLOAT type not yet supported");
        
    u32bit tileLevel1Sz;
    u32bit tileLevel2Sz;
    bool formatConversion = false;
    GLubyte *convertedData = NULL;

    // Check border (for now borders are not supported)
    if ( border != 0 )
        panic("TextureObject::Mipmap", "setPartial2D", "border different of 0 is not supported for now");

    /*  Get the texture tiling parameters from the driver.  Note:  May be the parameters should come
        from another class higher in the Texture Object hierarchy rather than directly from the driver.  */
    driver->getTextureTilingParameters(tileLevel1Sz, tileLevel2Sz);

/*    
    if ( size != 0 ) // Compressed texture (glCompressedTexImage2D)
    {
        imageCompression = true;

        GLint i;
        GLint j;
        u32bit k;
        u32bit w2;
        u32bit a;
        u32bit s3tcBlockSz;

        switch ( ifmt )
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                s3tcBlockSz = 8;
                break; // OK
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                s3tcBlockSz = 16;
                break; // OK

            default:
                panic("TextureObject", "set2D", "Unexpected compressed internal format");
        }

        if ( width_ < 4 || height_ < 4 )
        {
            popup("TextureObject", "Warning. S3TC textures width and height must be a multiple of 4 (see spec).\n"
                                   "Setting a not compressed default 1x1 black texture instead in mipmap level.\n"
                                   "Caution: This can trigger a texture completeness panic.");
            GLubyte temp[3];
            temp[0] = 0;
            temp[1] = 0;
            temp[2] = 0;
            set2D(GL_RGBA, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, temp, 0);
            return ;
        }

        w2 = (u32bit) mathlib::ceil2(mathlib::log2(width_ >> 2));
        size = s3tcBlockSz * (texel2address(w2, tileLevel1Sz - 2, tileLevel2Sz, (width >> 2) - 1, (height >> 2) - 1) + 1);
        data = new GLubyte[size];

        // Format texture data (morton)
        for(i = 0; i < (height_ >> 2); i++)
        {
            for(j = 0; j < (width_ >> 2); j++)
            {
                a = texel2address(w2, tileLevel1Sz - 2, tileLevel2Sz, j, i);
                for(k = 0; k < (s3tcBlockSz >> 2); k ++)
                    ((u32bit *) data)[(a * (s3tcBlockSz >> 2)) + k] = ((u32bit *) data_)[(i * (width_ >> 2) + j) * (s3tcBlockSz >> 2) + k];
            }
        }

        //memcpy(data, data_, size); // Use compressed texture directly

        return ; // bye
    }
*/
    // Uncompressed texture (glTexSubImage2D)


    switch ( format ) // Check format
    {
        case GL_RGBA8:
        case GL_RGBA:
        case GL_RGB:
        case GL_RGB8:
        case GL_BGR:
        case GL_BGRA:
        case GL_ALPHA:
        case GL_LUMINANCE:
            break;
        default:
            panic("TextureObject::Mipmap", "set2PartialD", "Format not supported");
    }


    switch ( ifmt )
    {
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
        case GL_COMPRESSED_RGBA:
            // Compress texture image
            //popup("TextureObject", "Warning. defining a Mipmap to be compressed and compression is not supported. This texture will not be compressed (used as regular RGBA)");
            //cout << "Warning. defining a Mipmap to be compressed and compression is not supported. This texture will not be compressed (used as regular RGBA)" << endl;
            ifmt = GL_RGBA;
            break; // OK
        case GL_RGBA:
        case GL_RGB:
            break;
        case GL_COMPRESSED_RGB:
        case GL_RGB8:
            ifmt = GL_RGB;
        case GL_RGBA8:
            ifmt = GL_RGBA;
            break;
        case GL_INTENSITY8:
            break; // OK (native support)
        case GL_LUMINANCE:
            break;
        case GL_ALPHA:
        case GL_ALPHA8:
            break; // OK (native support)
        default:
            panic("TextureObject::Mipmap", "setPartial2D", "Unexpected internal format");
    }

    // Compute bytes per component
    GLsizei bytesPerComponent = afl::getSize(type);

    GLsizei texelsSize = 4; // RGBA size

    if ( bytesPerComponent != 1 && bytesPerComponent != 2 && bytesPerComponent != 4 )
        panic("TextureObject::Mipmap", "setPartial2D", "Bytes per component must be 1, 2 or 4");

        
    if ( format == GL_RGB && (ifmt == GL_RGB || ifmt == GL_RGBA)) // Convert to RGBA
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* rgba = new GLubyte[width_*height_*4]; // Assumes 1 byte per component
        convertRGBtoRGBA((const GLubyte*)subData, rgba, bytesPerComponent, 1, width_,height_ ); // Support RGB coverting RGB to RGBA
        subData = convertedData = rgba;
        ifmt = GL_RGBA;
    }
    else if ( format == GL_RGBA && ifmt == GL_RGB )
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* rgba = new GLubyte[width_*height_*4];
        if ( bytesPerComponent != afl::getSize(GL_UNSIGNED_BYTE) )
            convertRGBAtoRGBA((const GLubyte*)subData, rgba, bytesPerComponent, 1, width_, height_);
        else
            memcpy(rgba, subData, width_*height_*4); // Assumes UNSIGNED_BYTE
        subData = convertedData = rgba;
        ifmt = GL_RGBA;
    }
    else if ( (format == GL_BGR || format == GL_BGRA) && (ifmt == GL_RGB || ifmt == GL_RGBA) )
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* rgba = new GLubyte[width_*height_*4];
        if ( fmt_ == GL_BGR )
            convertBGRtoRGBA((const GLubyte*)subData, rgba, bytesPerComponent, 1, width_, height_);
        else
            convertBGRAtoRGBA((const GLubyte*)subData, rgba, bytesPerComponent, 1, width_, height_);
        subData = convertedData = rgba;
        ifmt = GL_RGBA;
    }
    else if ( format == GL_RGBA && ifmt == GL_INTENSITY8 )
    {
        //  Format conversion performed
        formatConversion = true;

        // GL_INTENSITY8 = 1 byte
        GLubyte* intensity = new GLubyte[width_*height_*1];
        convertRGBAtoINTENSITY((const GLubyte*)subData, intensity, bytesPerComponent,1, width_, height_);
        subData = convertedData = intensity;
        texelsSize = 1;
    }
    else if ( format == GL_ALPHA && (ifmt == GL_ALPHA || ifmt == GL_ALPHA8 ))
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* alpha = new GLubyte[width_*height_];

        if ( bytesPerComponent != afl::getSize(GL_UNSIGNED_BYTE) )
            convertALPHAtoALPHA((const GLubyte*)subData, alpha, bytesPerComponent,1, width_, height_);
        else
            memcpy(alpha, subData, width_ * height_);

        subData = convertedData = alpha;
        texelsSize = 1;
    }

    else if ( format == GL_LUMINANCE && ifmt == GL_LUMINANCE )
    {
        //  Format conversion performed
        formatConversion = true;

        GLubyte* luminance = new GLubyte[width_*height_*1];
        memcpy(luminance, subData, width_*height_*1); // Assumes UNSIGNED_BYTE
        subData = convertedData = luminance;
        texelsSize = 1;
    }
/*
    GLint i;
    GLint j;
    u32bit w2;
    u32bit a;

    w2 = (u32bit) mathlib::ceil2(mathlib::log2(width));

    // Format texture data (morton)
    
    if ( texelsSize == 4 )
    {
        for(i = yoffset; i < yoffset + height_; i++)
        {
            for(j = xoffset; j < xoffset + width_; j++)
            {
                a = texel2address(w2, tileLevel1Sz, tileLevel2Sz, j, i);
                ((u32bit *) data)[a] = ((u32bit *) subData)[(i - yoffset) * width_ + (j - xoffset)];
            }
        }
    }
    else if ( texelsSize == 1 )
    {
        for(i = yoffset; i < yoffset + height_; i++)
        {
            for(j = xoffset; j < xoffset + width_; j++)
            {
                a = texel2address(w2, tileLevel1Sz, tileLevel2Sz, j, i);
                ((u8bit *) data)[a] = ((u8bit *) subData)[(i - yoffset) * width_ + (j - xoffset)];
            }
        }
    }*/


    GLuint heightCompr;
    //heightCompr = max(height_, GLuint(4)); // Texture compress default value

        //NEWW

        gpu3d::TextureCompression compressed;
        switch (ifmt)
        {
            case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
                compressed = gpu3d::GPU_S3TC_DXT1_RGB;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
                compressed = gpu3d::GPU_S3TC_DXT1_RGBA;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
                compressed = gpu3d::GPU_S3TC_DXT3_RGBA;
                break;
            case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
                compressed = gpu3d::GPU_S3TC_DXT5_RGBA;
                break;
            default:
                compressed = gpu3d::GPU_NO_TEXTURE_COMPRESSION;
                heightCompr = height_;
                break;
        }

        GLuint tonto;
        // Update mipmap data
        GLuint rowPitchOriginal = width * texelsSize;
        GLuint rowPitchUpdate = width_ * texelsSize;
        const GLuint offset = xoffset * texelsSize + yoffset*rowPitchOriginal;
        for ( GLuint i = 0; i < heightCompr; ++i )
            memcpy(originalData + offset + i*rowPitchOriginal, (GLubyte*)subData + i*rowPitchUpdate, rowPitchUpdate);

        data = driver->getDataInMortonOrder(originalData, width, height, 1, compressed, texelsSize, size);



    //  If a format conversion was performed delete the auxiliar buffer
    if (formatConversion)
        delete[] convertedData;

    //panic("TextureObject::Mipmap", "setPartial2D", "Not implemented yet");
}

/*  Determines if the texture data must be reversed (little to big endianess?).  */
bool TextureObject::Mipmap::reverseData() const
{
    /*  Only this type currently supported.  More should be added.  */
    if (type == GL_UNSIGNED_INT_8_8_8_8)
        return true;
    else
        return false;
}

TextureObject::TextureObject(GLuint name, GLenum targetName) :
BaseObject(name, targetName), /* Base object initialization */
/* Mipmap array initialization called by default constructor */
/* Texture object parameters default values */
wrapS(GL_REPEAT), wrapT(GL_REPEAT), wrapR(GL_REPEAT),
magFilter(GL_LINEAR), minFilter(GL_NEAREST_MIPMAP_LINEAR), maxAnisotropy(1.0f),
texturePriority(1.0f), minLOD(-1000), maxLOD(1000),
baseLevel(0), maxLevel(mipmapArraySize-1), biasLOD(0.0f),
depthTextureMode(GL_LUMINANCE), compareMode(GL_NONE), compareFunc(GL_LEQUAL),
generateMipmap(false), alignment(1), useMipmaps(false)
{
    GLuint nSets = 0;
    if ( targetName == GL_TEXTURE_1D || targetName == GL_TEXTURE_2D || targetName == GL_TEXTURE_3D )
    {
        nSets = 1;
        mips = new MipmapArray[1]; // 1 set of mipmaps
    }
    else if ( targetName == GL_TEXTURE_CUBE_MAP )
    {
        nSets = 6;
        mips = new MipmapArray[6]; // 6 faces per cubemap (6 sets of mipmaps)
    }
    else
    {
        char msg[256];
        sprintf(msg, "Target name 0x%X unsupported", targetName);
        panic("TextureObject", "TextureObject", msg);
    }
    for ( GLuint i = 0; i < nSets; i++ ) // init mipmap(s) array(s)
        mips[i].resize(mipmapArraySize);
}

TextureObject::~TextureObject()
{
    delete[] mips;
}

GLsizei TextureObject::getWidth() const
{
    return mips[0][0].getWidth();
}

GLsizei TextureObject::getHeight() const
{
    return mips[0][0].getHeight();
}

GLsizei TextureObject::getDepth() const
{
    return mips[0][0].getDepth();
}

GLenum TextureObject::getFormat() const
{
    return mips[0][0].getFormat();
}

GLint TextureObject::getInternalFormat() const
{
    return mips[0][baseLevel].getInternalFormat();
}

GLenum TextureObject::getBaseInternalFormat() const
{
    GLint ifmt = mips[0][baseLevel].getInternalFormat();
    switch ( ifmt )
    {
        case GL_ALPHA:
        case GL_ALPHA4:
        case GL_ALPHA8:
        case GL_ALPHA12:
        case GL_ALPHA16:
            return GL_ALPHA;
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
            return GL_DEPTH_COMPONENT;
        case GL_LUMINANCE:
        case GL_LUMINANCE4:
        case GL_LUMINANCE8:
        case GL_LUMINANCE12:
        case GL_LUMINANCE16:
            return GL_LUMINANCE;
        case GL_LUMINANCE_ALPHA:
        case GL_LUMINANCE4_ALPHA4:
        case GL_LUMINANCE6_ALPHA2:
        case GL_LUMINANCE8_ALPHA8:
        case GL_LUMINANCE12_ALPHA4:
        case GL_LUMINANCE12_ALPHA12:
        case GL_LUMINANCE16_ALPHA16:
            return GL_LUMINANCE_ALPHA;
        case GL_INTENSITY:
        case GL_INTENSITY4:
        case GL_INTENSITY8:
        case GL_INTENSITY12:
        case GL_INTENSITY16:
            return GL_INTENSITY;
        case GL_RGB:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16:
        case GL_COMPRESSED_RGB_S3TC_DXT1_EXT:
            return GL_RGB;
        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGB10_A2:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_COMPRESSED_RGBA_S3TC_DXT1_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT3_EXT:
        case GL_COMPRESSED_RGBA_S3TC_DXT5_EXT:
            return GL_RGBA;
        default:
            panic("TextureObject", "getBaseInternalFormat()", "Cannot find a base ifmt for this format");
    }
    return 0;
}

bool TextureObject::getReverseTextureData() const
{
    return mips[0][baseLevel].reverseData();
}

TextureObject::Mipmap& TextureObject::getMipmap(GLenum targetFace, GLint level) const
{

if (level >= mipmapArraySize)
{
printf("Mipmap::getMipmap() => face %d level %d\n", targetFace, level);
panic("Mipmap", "getMipmap", "level out of range");
}
    switch ( targetFace )
    {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:
            if ( getTargetName() != targetFace )
            {
                char msg[256];
                sprintf(msg, "Incompatible TextureTarget/TextureObject dimensionality (%d / %d)",targetFace, getTargetName());
                panic("TextureObject", "getMipmap", msg);
            }
            return mips[0][level];
        case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            if ( getTargetName() != GL_TEXTURE_CUBE_MAP )
                panic("TextureObject", "getMipmap", "Only cube maps have faces");
            return mips[0][level];
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            if ( getTargetName() != GL_TEXTURE_CUBE_MAP )
                panic("TextureObject", "getMipmap", "Only cube maps have faces");
            return mips[1][level];
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            if ( getTargetName() != GL_TEXTURE_CUBE_MAP )
                panic("TextureObject", "getMipmap", "Only cube maps have faces");
            return mips[2][level];
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            if ( getTargetName() != GL_TEXTURE_CUBE_MAP )
                panic("TextureObject", "getMipmap", "Only cube maps have faces");
            return mips[3][level];
        case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            if ( getTargetName() != GL_TEXTURE_CUBE_MAP )
                panic("TextureObject", "getMipmap", "Only cube maps have faces");
            return mips[4][level];
        case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            if ( getTargetName() != GL_TEXTURE_CUBE_MAP )
                panic("TextureObject", "getMipmap", "Only cube maps have faces");
            return mips[5][level];
        default:
            panic("TextureObject", "getMipmap", "targetFace unknown");
            return mips[0][level]; // avoids compile time warnings
    }

}

void TextureObject::setPartialContents(
        GLenum targetFace,
        GLint level,
        GLint xoffset,
        GLint yoffset,
        GLint zoffset,
        GLsizei width,
        GLsizei height,
        GLsizei depth,
        GLenum fmt,
        GLenum type,
        const GLubyte* subData,
        GLint compressedSize
        )
{
    Mipmap& mip = getMipmap(targetFace, level);

    if ( targetFace== GL_TEXTURE_1D )
        panic("TextureObject", "setPartialContents", "Textures 1D not implemented");
    else if ( targetFace == GL_TEXTURE_3D )
        panic("TextureObject", "setPartialContents", "Textures 3D not implemented");
    else // 2D texture or CM face texture (both types are TEXTURE_2D)
        mip.setPartial2D(xoffset, yoffset, width, height, fmt, type, subData, compressedSize);

    // basic synchronization per TextureObject
    // a per Mipmap synchronization could be "easily" implemented in the future (if more "precarios" are hired)

    // Entire reallocation of the texture object is only allowed if previous contents of the texture images
    // are not result of a bit blit operation (in this case forceRealloc() would erase the valid contents in memory).
    
    if (getState() != BaseObject::Blit) forceRealloc();

}


void TextureObject::setContents(
        GLenum targetFace,
        GLint level,
        GLint ifmt,
        GLsizei width,
        GLsizei height,
        GLsizei depth,
        GLint border,
        GLenum format,
        GLenum type,
        const GLubyte* data,
        GLsizei compressedSize
        )
{
    if ( level != 0 )
        useMipmaps = true;

    Mipmap& mip = getMipmap(targetFace, level);

    if ( targetFace== GL_TEXTURE_1D )
        mip.set1D(ifmt, width, border, format, type, data, compressedSize);
    else if ( targetFace == GL_TEXTURE_3D )
        mip.set3D(ifmt, width, height, depth, border, format, type, data, compressedSize);
    else // 2D texture or CM face texture (both types are TEXTURE_2D)
        mip.set2D(ifmt, width, height, border, format, type, data, compressedSize);

    // basic synchronization per TextureObject
    // a per Mipmap synchronization could be "easily" implemented in the future (if more "precarios" are hired)

    forceRealloc(); // new contents defined, reallocate this object (or allocate it for first time)
}


void TextureObject::setParameter(GLenum pname, const GLint* params)
{
    int parameter = params[0];
    switch ( pname )
    {
        case GL_TEXTURE_WRAP_S:
            wrapS = parameter;
            break;
        case GL_TEXTURE_WRAP_T:
            wrapT = parameter;
            break;
        case GL_TEXTURE_WRAP_R:
            wrapR = parameter;
            break;
        case GL_TEXTURE_MAG_FILTER:
            magFilter = parameter;
            break;
        case GL_TEXTURE_MIN_FILTER:
            switch (parameter)
            {
                case GL_NEAREST: break;
                case GL_LINEAR:  break;
                case GL_NEAREST_MIPMAP_NEAREST: break;
                case GL_NEAREST_MIPMAP_LINEAR: break;
                case GL_LINEAR_MIPMAP_NEAREST: break;
                case GL_LINEAR_MIPMAP_LINEAR: break;
                default:
                    panic("TextureObject","setParameter","Unknown texture min filter");
            }
            minFilter = parameter;
            break;
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:

            //
            // Patch to force max aniso 16 for all the traces that were captured in the old NVidia GeForce5950XT
            // that only supported max aniso 8.
            //
            maxAnisotropy = (parameter == 8)?16.0f:GLfloat(parameter);

            //maxAnisotropy = GLfloat(parameter);

            break;
        case GL_TEXTURE_BASE_LEVEL:
            if ( baseLevel < 0 )
                panic("TextureObject", "setParameter", "GL_TEXTURE_BASE_LEVEL value must be non-negative");
            baseLevel = parameter;
            break;
        case GL_TEXTURE_MAX_LEVEL:
            if ( maxLevel < 0 )
                panic("TextureObject", "setParameter", "GL_TEXTURE_MAX_LEVEL value must be non-negative");
            maxLevel = parameter;
            break;
        case GL_TEXTURE_MIN_LOD:
            minLOD = static_cast<GLfloat>(parameter);
            break;
        case GL_TEXTURE_MAX_LOD:
            maxLOD = static_cast<GLfloat>(parameter);
            break;
        default:
            panic("TextureObject", "setParameter", "Parameter name not supported");
    }
}

void TextureObject::setParameter(GLenum pname, const GLfloat* params)
{
    switch ( pname )
    {
        case GL_TEXTURE_WRAP_S:
        case GL_TEXTURE_WRAP_T:
        case GL_TEXTURE_MAG_FILTER:
        case GL_TEXTURE_MIN_FILTER:
        case GL_TEXTURE_BASE_LEVEL:
        case GL_TEXTURE_MAX_LEVEL:
            {
                GLint param = GLint(params[0]);
                setParameter(pname, &param);
            }
            break;
        case GL_TEXTURE_MIN_LOD:
            minLOD = params[0];
            break;
        case GL_TEXTURE_MAX_LOD:
            maxLOD = params[0];
            break;
        case GL_TEXTURE_MAX_ANISOTROPY_EXT:

            //
            // Patch to force max aniso 16 for all the traces that were captured in the old NVidia GeForce5950XT
            // that only supported max aniso 8.
            //
            maxAnisotropy = (params[0] == 8.0f)?16.0f:params[0];

            //maxAnisotropy = params[0];

            break;
        case GL_TEXTURE_LOD_BIAS:
            //popup("TextureObject", "Warning. GL_TEXTURE_LOD_BIAS parameter ignored");
            //cout << "Warning. GL_TEXTURE_LOD_BIAS parameter ignored" << endl;
            biasLOD = params[0];
            break;
        default:
            {
                char msg[128];
                sprintf(msg, "Parameter not supported: 0x%x", pname);
                panic("TextureObject", "setParameter", msg);
            }
    }
}

void TextureObject::Mipmap::dump2PPM(GLubyte* filename)
{
    FILE *fout;

    u8bit red;
    u8bit green;
    u8bit blue;

    if (format != GL_RGBA8) return;

        //sprintf(filename, "sampler.ppm", frameCounter);

    if (!originalData) return;

    fout = fopen((const char*)filename, "wb");

    /*  Write magic number.  */
    fprintf(fout, "P6\n");

    /*  Write frame size.  */
    fprintf(fout, "%d %d\n", width, height);

    /*  Write color component maximum value.  */
    fprintf(fout, "255\n");

    // Supose unsigned byte

    GLubyte* index;
    index = originalData;
 
    for (  int i = 0 ; i < height; i++ )
    {
        for ( int j = 0; j < width; j++ )
        {
            fputc( char(*index), fout ); index++;
            fputc( char(*index), fout ); index++;
            fputc( char(*index), fout ); index++;
            index++;
        }
    }

    fclose(fout);
}


void TextureObject::dump(GLubyte* filename)
{
    switch(getTargetName())
    {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_2D:
        case GL_TEXTURE_3D:

            for (int i = getMinActiveMipmap();i < getMaxActiveMipmap(); i++)
            {
                sprintf((char*)(filename+8), "_Face0_Mipmap%d.ppm", i);
                mips[0][i].dump2PPM(filename);
            }
        break;
        case GL_TEXTURE_CUBE_MAP:
            for (int j = 0; j < 6; j++)
                for (int i = getMinActiveMipmap();i < getMaxActiveMipmap(); i++)
                {
                    sprintf((char*)(filename+8), "_Face%d_Mipmap%d.ppm", j,i);
                    mips[j][i].dump2PPM(filename);
                }
        break;
        default:
            cout << "ERROR: TextureObject::Dump" << endl;
    }
}


void TextureObject::getMipmapPosition(GLuint portion, GLuint& face, GLuint& pos)
{
    // assumes getNumberOfPortions() is updated correctly according to baseLevel & maxLevel
    GLenum tn = getTargetName();

    GLuint mod = getMaxActiveMipmap() - baseLevel + 1;
    GLuint set = (portion / mod);
    GLuint offset = (portion % mod) + baseLevel;


    if ( offset > getMaxActiveMipmap() )
        panic("TextureObject", "getMipmapPosition", "Mipmap selected greater than maxLevel");

    if ( tn == GL_TEXTURE_1D || tn == GL_TEXTURE_2D || tn == GL_TEXTURE_3D )
    {
        if ( set != 0 )
            panic("TextureObject", "getMipmapPosition", "(NOT a CubeMap) Mipmap out of range [baseLevel..maxLevel]");
    }
    else // CUBE_MAP
    {
        if ( set >= 6 )
            panic("TextureObject", "getMipmapPosition", "CubeMap: Mipmap out of range [baseLevel..maxLevel]");
    }

    face = set;
    pos = offset;
}

void TextureObject::getMipmapPortion(GLuint face, GLuint pos, GLuint& portion)
{
    // assumes getNumberOfPortions() is updated correctly according to baseLevel & maxLevel
    GLenum tn = getTargetName();
    
    if ( (tn == GL_TEXTURE_1D || tn == GL_TEXTURE_2D || tn == GL_TEXTURE_3D) && face != 0 )
    {
        panic("TextureObject","getMipmapPortion","NOT a CubeMap and face different from face 0");
    }
    else if ( (tn == GL_TEXTURE_CUBE_MAP) && face >= 6 )
    {
        panic("TextureObject","getMipmapPortion","CubeMap and face greater than 6");
    }
    
    if ( pos > getMaxActiveMipmap())
    {
        panic("TextureObject","getMipmapPortion","mipmap position greater than maximum active mipmap");
    }
    
    portion = face * (getMaxActiveMipmap() + 1) + pos;
}

GLuint TextureObject::binarySize(GLuint portion)
{
    GLuint set, offset;
    getMipmapPosition(portion, set, offset);
    return mips[set][offset].getSize();
}


const GLubyte* TextureObject::binaryData(GLuint portion)
{
    GLuint set, offset;
    getMipmapPosition(portion, set, offset);
    return mips[set][offset].getData();
}

/**
 * see OpenGL spec 2.0 -> section 3.8.10 (pag 177) - Texture Completeness
 */
bool TextureObject::checkCompleteness() const
{
    if ( baseLevel > maxLevel ) // 4th condition
    {
        panic("TextureObject", "checkCompleteness()", "baseLevel > maxLevel");
        return false;
    }

    GLuint sets = ( getTargetName() == GL_TEXTURE_CUBE_MAP ? 6 : 1 );
    GLuint i, j;

    GLint ifmt = mips[0][baseLevel].getInternalFormat();
    GLint border = mips[0][baseLevel].getBorder();

    for ( i = 0; i < sets; i++ ) // check mipmap set_i completeness
    {
        GLsizei w = mips[i][baseLevel].getWidth();
        GLsizei h = mips[i][baseLevel].getHeight();
        // next expected dimensions
        GLsizei wNext = w;
        GLsizei hNext = h;

        for ( j = baseLevel; j <= getMaxActiveMipmap(); j++ )
        {
            Mipmap& m = mips[i][j];
            if ( m.getInternalFormat() != ifmt )
                return false;
            if ( m.getBorder() != border )
                return false;
            if ( m.getWidth() != wNext )
                return false;
            if ( m.getHeight() != hNext )
                return false;
            if ( getTargetName() == GL_TEXTURE_CUBE_MAP && wNext != hNext)
                return false;

            // compute next expected mipmap size
            hNext = mathlib::max(1, GLsizei(mathlib::ceil2(mathlib::log2(hNext))));
            wNext = mathlib::max(1, GLsizei(mathlib::ceil2(mathlib::log2(wNext))));
        }
    }

    return true;
}

GLsizei TextureObject::getMipmapWidth(GLenum targetFace, GLint level) const
{
    return getMipmap(targetFace, level).getWidth();
}

GLsizei TextureObject::getMipmapHeight(GLenum targetFace, GLint level) const
{
    return getMipmap(targetFace, level).getHeight();
}

GLsizei TextureObject::getMipmapDepth(GLenum targetFace, GLint level) const
{
    return getMipmap(targetFace, level).getDepth();
}


TextureObject::Mipmap& TextureObject::getBaseMipmap() const
{
    return mips[0][baseLevel];
}

bool TextureObject::isCompressed() const
{
    return mips[0][baseLevel].isCompressed();
}

GLuint TextureObject::getMaxActiveMipmap() const
{
    using namespace std;
    if ( !useMipmaps )
        return baseLevel;

    Mipmap& mip = getBaseMipmap();
    GLint maxSize = mathlib::max(mathlib::max(mip.getHeight(), mip.getWidth()), mip.getDepth());

    GLint p = GLint(mathlib::ceil2(mathlib::log2(maxSize)));

    if ( isCompressed() )
    {
        GLenum targetName = getTargetName();
        if (  targetName == GL_TEXTURE_2D || targetName == GL_TEXTURE_CUBE_MAP )

            p = GLint(mathlib::ceil2(mathlib::log2(mathlib::min(mip.getHeight(), mip.getWidth())))) - 2;
        else
            panic("TextureObject", "getMaxActiveMipmap()", "GL_TEXTURE_2D or CUBE_MAP expected");
    }

    if (  p < 0 )
        panic("TextureObject", "getMaxActiveMipmap", "Max active mipmap cannot be less than 0");

    GLint ret = mathlib::min(p,(GLint)maxLevel);
    //cout << "mathlib::min(p,maxLevel) = " << ret << endl;
    return GLuint(ret);
}

GLuint TextureObject::getMinActiveMipmap() const
{
    return getBaseLevel();
}

void TextureObject::sync()
{
    // check completeness
    checkCompleteness();

    // recompute NumberOfPortions
    int min = getMinActiveMipmap();
    int max = getMaxActiveMipmap();

    if ( min > max )
        panic("TextureObject", "sync", "min mipmap is greater than max mipmap");

    if ( getTargetName() == GL_TEXTURE_CUBE_MAP )
        setNumberOfPortions((max-min+1)*6);
    else
        setNumberOfPortions(max-min+1);
}

GLfloat TextureObject::getMaxAnisotropy() const
{
    return maxAnisotropy;
}

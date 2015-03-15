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

#ifndef AUXFUNCSLIB_H
    #define AUXFUNCSLIB_H

#include "gl.h"
#include "GPU.h"
#include "QuadReg.h"
#include "DataBuffer.h"
#include "GPUDriver.h"
#include "TextureObject.h"
#include "BaseManager.h"
#include "BufferObject.h"
#include "GLContext.h"
#include "MathLib.h"
#include <typeinfo>
#include <vector>
#include <set>
#include <map>

//#define GLLIB_DUMP_STREAMS
//#define GLLIB_DUMP_SAMPLERS

namespace libgl
{

class GLContext;

namespace afl /* Auxiliar functions library namespace */
{

void getIndicesSet(std::set<GLuint>& iSet, GLenum type, const GLvoid* indices, GLsizei count);

/* Function that computes the highest number in a given array of numbers */
GLuint computeHighestIndex(GLenum type, const GLvoid* indices, GLsizei count);

/**
 * Function that computes the highest and the lowest number in a given array of numbers
 */
void computeIndexBounds(GLenum type, const GLvoid* indices, GLsizei count, GLuint& min, GLuint& max);

/* Obtains the size in bytes of a OGL type */
GLuint getSize(GLenum openGLType);

/* Obtains the corresponding StreamData type for a given GL type */
gpu3d::StreamData getStreamDataType(GLenum openGLType);

/* Obtains the corresponding GPU primitive enum for a given GL primitive enum */
gpu3d::PrimitiveMode translatePrimitive(GLenum primitive);

void normalizeQuad(QuadReg<float>& qr);

/**
 * Generic glVertex call used to implement all different flavors of glVertex command
 */
void glVertex(GLContext* ctx, GLfloat x, GLfloat y, GLfloat z = 0.0f, GLfloat w = 1.0f);


GLuint configureStream(GLContext* ctx, ::GPUDriver* driver,
                            GLContext::VArray& info,
                            ::GPUDriver::ShAttrib attrib,
                            GLuint stream, bool colorAttrib,
                            GLuint start, GLuint count,
                            std::set<GLuint>& iSet);

void dumpBuffer(std::ofstream& out, const GLubyte* data, GLsizei components, GLenum type, GLsizei start, GLsizei stride, GLsizei count );

std::vector<GLuint> setVertexArraysBuffers(GLContext* ctx, ::GPUDriver* driver,
                                                GLuint start, GLuint count,
                                                std::set<GLuint>& iSet, GLuint& nextFreeStream);


/**
 * Generic drawElements: used to implement drawElements & drawRangeElements
 */
void drawElements(GLContext* ctx, ::GPUDriver* driver, GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);


/**
 * Entry point for glCompressedTexImage2D & glCompressedTexImage2DARB
 */
void compressedTexImage2D( GLenum target, GLint level, GLenum internalFormat, GLsizei width,
                           GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data );


GLfloat clamp(GLfloat v);
GLdouble clamp(GLdouble d);

/* SQRT supporting all numeric types */
template<typename T>
float sqrt(T x) { return (float)::sqrt((float)x); }

void fillDBColor(DataBuffer<float>& db, GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count);

void fillDB(DataBuffer<float>& db, GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, GLsizei start, GLsizei count);

void fillDBColor_ind(DataBuffer<GLfloat>& db, GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet);

void fillDB_ind(DataBuffer<GLfloat>& db, GLenum type, const GLvoid* buf, GLsizei size, GLsizei stride, const std::set<GLuint>& iSet);

u32bit sendDB(DataBuffer<float>& db, ::GPUDriver* drv);

/*
std::vector<unsigned int> setStreamBuffers(GLContext* ctx, ::GPUDriver* driver, unsigned int start, unsigned int count);

// indexed version
std::vector<unsigned int> setStreamBuffers(GLContext* ctx, ::GPUDriver* driver, std::set<GLuint>& iSet );

// VBO version + indexed
int setStreamBuffersVBO(GLContext* ctx, ::GPUDriver* driver, BufferObject* bo);
*/

int sendIndices(::GPUDriver* driver, const GLvoid* indices, GLenum indicesType, GLsizei count, GLint offset);

int sendIndices(::GPUDriver* driver, const GLvoid* indices, GLenum indicesType, GLsizei count, const std::set<GLuint>& usedIndices);

int sendRawIndices(::GPUDriver* driver, const GLvoid* indices, GLenum type, GLsizei count);

void setFragmentAttributes(::GPUDriver* driver, GLContext* ctx);

/**
 * Sets all required state for this texture
 */
//void setupTexture(GLContext* ctx, ::GPUDriver* driver, TextureObject& to);
void setupTexture(GLContext* ctx, ::GPUDriver* driver, TextureObject& to, GLuint texUnit, GLenum kindOfAccess = 0);

/**
 * Synchronizes all required texture images into the local GPUMemory
 *
 * Must be called inside a OpenGL Call that performs a GPU_DRAW command: glEnd, glDrawElements, glDrawArrays
 * And must be called before the GPU_DRAW command is sent to the GPU
 */
void synchronizeTextureImages(GLContext* ctx, ::GPUDriver* driver, bool& disableColorMask);


/**
 * Construct required shaders or uses user shaders
 * and set all input/output attributes
 */
void setupShaders(GLContext* ctx, ::GPUDriver* driver);


/**
 *
 *  Configures the Hierarchical Z Test.
 *
 *  @param ctx Pointer to the OpenGL context.
 *  @param driver Pointer to the GPU driver.
 *
 */

void setupHZTest(GLContext* ctx, ::GPUDriver* driver);

/**
 * Release object from a given Manager
 */
void releaseObjects(BaseManager& bm, GLsizei n, const GLuint* names);


/**
 * Returns the string associated with the internal format
 */
const char* strIFMT(GLint ifmt);

// Normalize an attribute array
void normalizeData(const GLvoid* dataIn, GLfloat* dataOut, GLsizei count, GLenum type);


//u32bit writeTexture(GLContext* ctx, GPUDriver* driver, TextureObject& to);

/**
 * Emulates glClear(GL_STENCIL_BUFFER_BIT) rendering a quad that fills entire stencil buffer
 * without modify color and z buffer.
 */
void clearStencilOnly(GLContext *ctx);

/**
 * Emulates glClear(GL_DEPTH_BUFFER_BIT) rendering a quad that fills z buffer
 * with depth clear value in position z of fragments.
 */
void clearDepthOnly(GLContext *ctx);

/**
 *  Emulates glClear()
 */
void clear(GLContext *ctx,  GLbitfield mask);


/**
 * Performs a dump of the Stencil Buffer State in a frame output image
 * CAUTION: After this call, the Color Buffer is modified and final rendered image
 *          will be not correct.
 */
void dumpStencilBuffer(GLContext *ctx);

/**
 * Performs a blit operation copying the framebuffer region within ( x, y ) 
 * and ( x + width, y + height ) corners into the texture image region starting 
 * at (xoffset, yoffset) bottom-left corner of a texture with texWidth width.
 *
 * The texture image is specified by the corresponding memory descriptor and the destination
 * internal texture format in 'ifmt'.
 *
 * In addition, the total size in bytes occupied by the blitted region at the destination texture is returned.
 *
 */
void performBlitOperation(GLint xoffset, GLint yoffset, GLsizei texWidth, GLint x, GLint y, GLsizei width, GLsizei height, u32bit md, GLenum ifmt, GLsizei &size);


/***************************************************************************
 *                     TEMPLATIZED AUXILIAR FUNCTIONS                      *
 *                                                                         *
 * @note: are private, placed here by limitation in templates compilation  *
 *                                                                         *
 ***************************************************************************/

template<typename T>
void fillDB(DataBuffer<GLfloat>& db, const T* buf, GLsizei nc, GLsizei stride, GLsizei start, GLsizei count)
{
    int i, j;

    if ( stride % sizeof(T) != 0 )
        panic("AuxFuncLib", "fillDB()", "stride must be a multiple of T");

    if ( stride == 0 )
        stride = nc;
    else
        stride /= sizeof(T);

    j = start;
    i = start * stride;
    count += start;



    for ( ; j < count; i += stride, j++ )
    {
        if ( nc == 2 )
            db.add(GLfloat(buf[i]), GLfloat(buf[i+1]));
        else if ( nc == 3 )
            db.add(GLfloat(buf[i]), GLfloat(buf[i+1]), GLfloat(buf[i+2]));
        else
            db.add(GLfloat(buf[i]), GLfloat(buf[i+1]), GLfloat(buf[i+2]), GLfloat(buf[i+3]));
    }
}

int valueCal();

template<typename T>
void fillDB_ind(DataBuffer<GLfloat>& db, const T* buf, GLsizei nc, GLsizei stride, const std::set<GLuint>& iSet)
{
    using namespace std;

    int j;

    if ( stride % sizeof(T) != 0 )
        panic("AuxFuncLib", "fillDB()", "stride must be a multiple of T");

    if ( stride == 0 )
        stride = nc;
    else
        stride /= sizeof(T);

    set<GLuint>::const_iterator it = iSet.begin();

    for ( ; it != iSet.end(); it++ )
    {
        j = stride * (*it);

        if ( nc == 2 )
            db.add(GLfloat(buf[j]), GLfloat(buf[j+1]));
        else if ( nc == 3 )
            db.add(GLfloat(buf[j]), GLfloat(buf[j+1]), GLfloat(buf[j+2]));
        else
            db.add(GLfloat(buf[j]), GLfloat(buf[j+1]), GLfloat(buf[j+2]), GLfloat(buf[j+3]));
    }
}


template<typename T>
void fillDBColor_ind(DataBuffer<GLfloat>& db, const T* buf, GLsizei nc, GLsizei stride, const std::set<GLuint>& iSet)
{
    if ( nc != 3 && nc != 4 )
        panic("AuxFuncsLib", "fillDBColor_ind()", "size (nc), must be 3 or 4");

    if ( stride % sizeof(T) != 0 )
        panic("AuxFuncLib", "fillDBColor_ind()", "stride must be a multiple of T");

#define __U_TO_FLOAT(divValue) {\
    r = GLfloat(buf[i])/divValue;\
    g = GLfloat(buf[i+1])/divValue;\
    b = GLfloat(buf[i+2])/divValue;\
    if ( nc == 4 )\
        a = GLfloat(buf[i+3])/divValue;}

#define __TO_FLOAT(divValue) {\
    r = (2.0f * float(buf[i])+1)/divValue;\
    g = (2.0f * float(buf[i+1])+1)/divValue;\
    b = (2.0f * float(buf[i+2])+1)/divValue;\
    if ( nc == 4 )\
        a = (2.0f * float(buf[i+3])) / divValue;}

    if ( stride == 0 )
        stride = nc; // item size
    else
        stride /= sizeof(T);

    GLfloat r, g, b, a;

    int i;

    std::set<GLuint>::const_iterator it = iSet.begin();

    for ( ; it != iSet.end(); it++ )
    {
        i = stride * (*it); // position in the buffer for element *it

        if ( typeid(T) == typeid(GLubyte) )
            __U_TO_FLOAT(0xFF)

        else if ( typeid(T) == typeid(GLbyte) )
            __TO_FLOAT(0xFF)

        else if ( typeid(T) == typeid(GLushort) )
            __U_TO_FLOAT(0xFFFF)

        else if ( typeid(T) == typeid(GLshort) )
            __TO_FLOAT(0xFFFF)

        else if ( typeid(T) == typeid(GLuint) )
            __U_TO_FLOAT(0xFFFFFFFF)

        else if ( typeid(T) == typeid(GLint) )
            __TO_FLOAT(0xFFFFFFFF)

        else if ( typeid(T) == typeid(GLfloat) )
        {
            r = float(buf[i]);
            g = float(buf[i+1]);
            b = float(buf[i+2]);
            if ( nc == 4 )
                a = float(buf[i+3]);
        }

        else if ( typeid(T) == typeid(GLdouble) )
        {
            r = GLfloat(buf[i]);
            g = GLfloat(buf[i+1]);
            b = GLfloat(buf[i+2]);
            if ( nc == 4 )
                a = GLfloat(buf[i+3]);
        }

        else
            panic("AuxFuncsLib","fillDBColor_ind()","Unsupported 'templatized' type");

        if ( nc == 3 )
            db.add(r,g,b,1.0f);
        else
            db.add(r,g,b,a);
    }

    #undef __U_TO_FLOAT
    #undef __TO_FLOAT
}

/**
 *
 * @param db DataBuffer where data will be stored (in floating point mode)
 * @param buf Buffer containing input data
 * @param stride offset in bytes where to find the next item in the buffer
 * @param count number of items in the buffer
 */
template<typename T>
void fillDBColor(DataBuffer<GLfloat>& db, const T* buf, GLsizei nc, GLsizei stride, GLsizei start, GLsizei count)
{
    if ( nc != 3 && nc != 4 )
        panic("AuxFuncsLib", "fillDBColor()", "size (nc), must be 3 or 4");

    if ( stride % sizeof(T) != 0 )
        panic("AuxFuncLib", "fillDBColor()", "stride must be a multiple of T");

#define __U_TO_FLOAT(divValue) {\
    r = GLfloat(buf[i])/divValue;\
    g = GLfloat(buf[i+1])/divValue;\
    b = GLfloat(buf[i+2])/divValue;\
    if ( nc == 4 )\
        a = GLfloat(buf[i+3])/divValue;}

#define __TO_FLOAT(divValue) {\
    r = (2.0f * GLfloat(buf[i])+1)/divValue;\
    g = (2.0f * GLfloat(buf[i+1])+1)/divValue;\
    b = (2.0f * GLfloat(buf[i+2])+1)/divValue;\
    if ( nc == 4 )\
        a = (2.0f * GLfloat(buf[i+3])) / divValue;}

    if ( stride == 0 )
        stride = nc; // item size
    else
        stride /= sizeof(T);

    GLfloat r, g, b, a;

    int i, j;

    j = start;
    i = start * stride;
    count += start;


    for ( ; j < count; i+= stride, j++ )
    {
        if ( typeid(T) == typeid(GLubyte) )
            __U_TO_FLOAT(0xFF)

        else if ( typeid(T) == typeid(GLbyte) )
            __TO_FLOAT(0xFF)

        else if ( typeid(T) == typeid(GLushort) )
            __U_TO_FLOAT(0xFFFF)

        else if ( typeid(T) == typeid(GLshort) )
            __TO_FLOAT(0xFFFF)

        else if ( typeid(T) == typeid(GLuint) )
            __U_TO_FLOAT(0xFFFFFFFF)

        else if ( typeid(T) == typeid(GLint) )
            __TO_FLOAT(0xFFFFFFFF)

        else if ( typeid(T) == typeid(GLfloat) )
        {
            r = (float)buf[i];
            g = (float)buf[i+1];
            b = (float)buf[i+2];
            if ( nc == 4 )
                a = (float)buf[i+3];
        }

        else if ( typeid(T) == typeid(GLdouble) )
        {
            r = GLfloat(buf[i]);
            g = GLfloat(buf[i+1]);
            b = GLfloat(buf[i+2]);
            if ( nc == 4 )
                a = GLfloat(buf[i+3]);
        }

        else
            panic("AuxFuncsLib","fillDBColor()","Unsupported 'templatized' type");

        if ( nc == 3 )
            db.add(r,g,b,1.0f);
        else
            db.add(r,g,b,a);
    }


    #undef __U_TO_FLOAT
    #undef __TO_FLOAT
}


template<typename T>
void computeIndexBounds2(const T* indices, GLsizei count, GLuint& min, GLuint& max)
{
    GLint iMax = 0;
    GLint iMin = 0;
    GLint i;
    for ( i = 0; i < count; i++ )
    {
        if ( indices[i] > indices[iMax] )
            iMax = i;
        if ( indices[i] < indices[iMin] )
            iMin = i;
    }
    min = indices[iMin];
    max = indices[iMax];
}


/* Templatized version that finds the highest number in a numeric array */
template<typename T>
GLuint computeHighestIndex2(const T* indices,GLsizei count)
{
    GLuint max = 0;
    GLint i;
    for (i = 0; i < count; i++ )
    {
        if ( indices[i] > indices[max] )
            max = i;
    }
    return indices[max];
}




} // namespace afl

} // namespace libgl


#endif // AUXFUNCSLIB_H

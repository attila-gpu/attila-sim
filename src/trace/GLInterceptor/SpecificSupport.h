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

#ifndef SPECIFICSUPPORT_H
    #define SPECIFICSUPPORT_H

#include "support.h"
#include "glAll.h"
#include "BufferDescriptor.h"
//#include "BufferObj.h"
#include <map>
#include <string>

#define BM GLInterceptor::getBufferManager()

#define LOG_BUFFERS_MACRO\
    (  GLInterceptor::isLogFlagEnabled(GLInterceptor::LOG_BUFFERS) \
    && GLInterceptor::isLogFlagEnabled(GLInterceptor::LOG_TRACE)  )

/* TraceWriter */
#define WRITER GLInterceptor::tw 

/* Jump Table containing original OpenGL calls */
#define _JT GLInterceptor::jt 

/* Jump Table containing pointers to wrapper functions */
#define _JTW GLInterceptor::jtWrapper 

#define RESOLVE_CALL(call) ((unsigned long *)(&_JT))[APICall_##call] = (unsigned long)_JT.wglGetProcAddress(#call)
#define CHECKCALL(call) if (_JT.##call == NULL) { RESOLVE_CALL(call); if (_JT.##call == NULL) { panic("Wrapper.cpp", "Call cannot be resolved", #call); }}


/* identify a data pointer in 'pointer' array */
enum PointerTag
{
    VERTEX = 0,
    COLOR,
    INDEX,
    NORMAL,
    TEXTURE_COORD, // Texture coord 0
    TEXTURE_COORD1,
    TEXTURE_COORD2,
    TEXTURE_COORD3,
    TEXTURE_COORD4,
    TEXTURE_COORD5,
    TEXTURE_COORD6,
    TEXTURE_COORD_MAX,
// Add more if required...

    EDGE_FLAG,
    /* Add here new enums for support another buffers */
    MAX_POINTERS
};

#define MAX_GENERIC_POINTERS 16


/* Macro for initialize PointerStruct componets */
#define INIT_POINTER {0,0,0,NULL,false,false,true,false,false}

/* Represents an OpenGl data pointer ( vertexes, colors, textures, etc ) */
/* Support to vertex arrays ( OpenGL 1.1 ) */
struct PointerStruct
{
    GLint size;
    GLenum type;
    GLsizei stride;
    //BufferObj* buf;
    BufferDescriptor* buf;
    bool enabled;
    bool modified;
    bool isNew;
    bool isPrevious;
    bool bufferBound; // for supporting vertex buffer object in vertex arrays
    GLboolean normalized;
};


/* Last indices used */
struct IndicesStruct
{
    //BufferObj* indices; /* stores index buffer and count */
    BufferDescriptor* indices; /* stores index buffer and count */
    GLint maxIndex; /* Store last max index used */
    GLenum type; /* Type of indices */
};

class SpecificSupportInitializer
{
public:
    SpecificSupportInitializer();
};


struct BufferObjectStruct
{
    struct BindInfo
    {
        GLsizei size;
        GLenum usage;
        GLenum target;
    };

    std::map<GLuint,BindInfo> binds;
    GLuint currentBind; // 0 means disabled
    GLvoid* addr; // third parameter used in last glBufferData call
    GLuint dataBind; // bind selected in last glBufferData call
    //BufferObj* contents; // BufferObject associated with last glBufferData call
    BufferDescriptor* contents; // BufferObject associated with last glBufferData call
    GLvoid* mappedAddr;

    bool isBound() { return currentBind != 0; }
};

extern std::string globalMsg;

extern bool vertexProgramEnabled;
extern GLuint clientActiveTexture;

/* State used for controlling Buffer Objects with target GL_ARRAY_BUFFER */
extern BufferObjectStruct boArray;

/* State used for controlling Buffer Objects with target GL_ELEMENT_ARRAY_BUFFER */
extern BufferObjectStruct boElementArray;

/* Array of data pointers */
extern PointerStruct arrayPointer[];

/* Array of data pointers for Attrib pointers */
extern PointerStruct genericPointer[];

/* Object storing last index buffer used */
static IndicesStruct prevIndices = {NULL, 0, 0};

/* generic call for all glXXXPointer calls */
void pointerArrayCall( const char* callName, PointerTag pt, GLint size, GLenum type, GLsizei stride, const GLvoid* ptr );

/* Configure one vertex array pointer */
void interleavedArrayCall(PointerTag pt, GLint size, GLenum type, GLsizei stride, const GLvoid* ptr, GLuint offset);

/* Configure all vertex arrays (includes mixing) */
int configureVertexArrays(GLuint start, GLuint count);

/* Computes the maximum index value in an indices array */
int findMaxIndex( GLsizei count, GLenum type, const void *indices );

/* gets pixel type size */
int getPixelTypeSize( GLenum pixelTypeConstant );

/* gets pixel format size */
int getPixelFormatSize( GLenum formatConstant );

/* gets the size given an openGL type */
int getTypeSize( GLenum openGLType );


/*
 * Combine all buffers that can share its MemReg internal object
 * That is all buffers than are mapped in the same memory chunk
 */
// Deprecated
//void combineBuffers( BufferObj** bObj, uint bsize[], uint n );


/*
 * ps.size : how many components per vertex data element
 * ps.type : type of each individual components in a vertex data element
 * ps.stride : buffer stride
 * count : number of vertex data elements in buffer
 */
int computeBufferSize( int size, GLenum type, int stride, int count );

/**
 * Checks and (if required) generate glXXXPointer calls to support
 * modified contents via user pointers (vertex arrays)
 */
void generateExtraArrayPointers(GLuint count);

/**
 * Auxiliar functions used to implement generateExtraArrayPointers
 */
void generateExtraArrayPointersAux(GLuint count, PointerStruct& ps, GLuint psIndex, bool isGeneric);
void generateArrayPointerCall(GLuint pointerID, bool isGeneric);


#endif // SPECIFICSUPPORT_H

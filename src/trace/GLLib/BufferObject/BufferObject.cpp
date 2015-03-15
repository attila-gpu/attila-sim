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

#include "BufferObject.h"
#include "BufferTarget.h"
#include "glext.h"
#include <iostream>
#include <cstring>

using namespace std;
using namespace libgl;


#define CHECK_SIZE(func)\
if ( this->size == 0 ) { panic("BufferObject", #func, "Buffer Object undefined yet"); return; }

#define CHECK_SIZE_R(func)\
if ( this->size == 0 ) { panic("BufferObject", #func, "Buffer Object undefined yet"); return 0; }

bool BufferObject::checkUsage(GLenum u)
{
    switch ( u )
    {
        case GL_STREAM_DRAW:
        case GL_STREAM_READ:
        case GL_STREAM_COPY:
        case GL_STATIC_DRAW:
        case GL_STATIC_READ:
        case GL_STATIC_COPY:
        case GL_DYNAMIC_DRAW:
        case GL_DYNAMIC_READ:
        case GL_DYNAMIC_COPY:
            return true;
        default:
            panic("BufferObject", "checkUsage", "Unsupported usage HINT");
            return false;
    }
}


BufferObject::BufferObject(GLuint name, GLenum targetName) : BaseObject(name, targetName),
size(0), usage(0), data(0)
{

}

GLenum BufferObject::getUsage() const
{
    CHECK_SIZE_R(getUsage)

    return usage;

}

GLuint BufferObject::binarySize(GLuint portion)
{
    if ( portion != 0 )
        panic("BufferObject", "binarySize", "A buffer object contains only one portion");

    return size;
}

const GLubyte* BufferObject::binaryData(GLuint portion)
{
    if ( portion != 0 )
        panic("BufferObject", "binaryData", "A buffer object contains only one portion");

    return getData();
}


void BufferObject::setContents(GLsizei size, const GLubyte* data, GLenum usage)
{
    if ( size == 0 )
    {
        panic("BufferObject", "setContents", "Size cannot be 0");
        return ;
    }

    checkUsage(usage);

    if ( usage == GL_STATIC_DRAW || usage == GL_STATIC_READ || usage == GL_STATIC_COPY )
        setPreferredMemoryHint(BaseObject::GPUMemory);
    else
        setPreferredMemoryHint(BaseObject::SystemMemory);

    forceRealloc();

    this->size = size;
    this->usage = usage;
    delete[] this->data;
    if ( data == 0 )
    {
        this->data = 0;
        return ;
    }
    this->data = new GLubyte[size];
    memcpy(this->data, data, size);
}

void BufferObject::setContents(GLsizei size, GLenum usage)
{
    setContents(size, 0, usage);
}

void BufferObject::setPartialContents(GLsizei offset, GLsizei subSize, const void* subData)
{
    CHECK_SIZE(setPartialContents)

    if ( subSize + offset > size )
    {
        panic("BufferObject", "setPartialContents", "size + offset out of range!");
        return ;
    }

    //setSynchronized(false);

    if ( data == 0 ) // force space creation
        data = new GLubyte[size]; // create uninitialized contents


    /*cout << "Buffer: " << getName() << endl;
    cout << "ANtes: " << endl;
    printfmt(3, GL_FLOAT, 0, 60, 6);
    cout << endl;
    printfmt(2, GL_FLOAT, 12, 60, 6);*/


    memcpy(data+offset, subData, subSize); // copy subregion

    /*cout << "\nDespues: " << endl;
    printfmt(3, GL_FLOAT, 0, 60, 6);
    cout << endl;
    printfmt(2, GL_FLOAT, 12, 60, 6);
    cout << "----------" << endl;*/


    addModifiedRange(offset, subSize); // mark this range as modified
//***************************
    //forceRealloc();  // debug
//******************
    //setState(BaseObject::NotSync); // requires synchronization with GPUMemory
}


GLubyte* BufferObject::getData()
{
    CHECK_SIZE_R(getData)

    if ( !data  )
        data = new GLubyte[size];
    return data;
}

BufferObject::~BufferObject()
{
    delete[] data;
}


void BufferObject::printfmt(GLsizei components, GLenum type, GLsizei start, GLsizei stride, GLsizei count )
{
    printfmt(data, components, type, start, stride, count);
}

void BufferObject::printfmt(const GLubyte* data, GLsizei components, GLenum type, GLsizei start, GLsizei stride, GLsizei count )
{
    const GLubyte* ptr_ub = (data + start);
    // ...
    const GLfloat* ptr_f = 0;
    const GLuint* ptr_ui = 0;
    cout << "{";
    GLint i, j;
    switch ( type )
    {
        case GL_UNSIGNED_INT:
            for ( i = 0; i < count; i++, ptr_ub += stride )
            {
                ptr_ui = (const GLuint*)ptr_ub;
                if ( components != 1 )
                    cout << "{";
                for ( j = 0; j < components; j++ )
                {
                    cout << ptr_ui[j];
                    if ( j < components - 1 )
                        cout << ",";
                }
                if ( components != 1 )
                    cout << "}";
                if ( i < count -1 )
                    cout << ",";
            }
            break;
        case GL_FLOAT:

            for ( i = 0; i < count; i++, ptr_ub += stride )
            {
                ptr_f = (const GLfloat*)ptr_ub;
                if ( components != 1 )
                    cout << "{";
                for ( j = 0; j < components; j++ )
                {
                    cout << ptr_f[j];
                    if ( j < components - 1 )
                        cout << ",";
                }
                if ( components != 1 )
                    cout << "}";
                if ( i < count -1 )
                    cout << ",";
            }
            break;
        default:
            panic("BufferObject", "printfmt", "Format unknown or not yet implemented");

    }
    cout << "}";
}

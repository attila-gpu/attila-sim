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

#include "AGLBufferObject.h"
#include "AGLBufferTarget.h"
#include "glext.h"
#include <iostream>

using namespace agl;
using namespace std;

void BufferObject::attachDevice(acdlib::ACDDevice* device)
{
    if (_acddev == 0)
    {
        _acddev = device;
        _data = _acddev->createBuffer();
    }

}

BufferObject::BufferObject(GLuint name, GLenum targetName) : BaseObject(name, targetName)
{
    _acddev = 0;
}

GLenum BufferObject::getUsage() const
{
    return _data->getUsage();
}

GLsizei BufferObject::getSize()
{
    return _data->getSize();
}

void BufferObject::setContents(GLsizei size, const GLubyte* data, GLenum usage)
{
    if ( size == 0 )
    {
        panic("BufferObject", "setContents", "Size cannot be 0");
        return ;
    }

    _data->clear();

    _data->setUsage(getACDUsage(usage));

   
    if (data == 0) _data->resize(size, true);
    else 
    {
        _data->clear();
        _data->pushData(data, size);
    }   
}

void BufferObject::setContents(GLsizei size, GLenum usage)
{
    setContents(size, 0, usage);
}

void BufferObject::setPartialContents(GLsizei offset, GLsizei subSize, const void* subData)
{

    if ( _data->getSize() == 0 )
    {
        _acddev->destroy(_data);
        _data = _acddev->createBuffer(subSize + offset, 0);
        //panic("BufferObject", "setPartialContents", "Buffer Object undefined yet");
    }


    if ( subSize + offset > _data->getSize())
    {
        _data->resize(subSize+offset, true);
        //panic("BufferObject", "setPartialContents", "size + offset out of range!");
    }

     _data->updateData(offset, subSize, (const acdlib::acd_ubyte*) subData);
}


acdlib::ACDBuffer* BufferObject::getData()
{
    return _data;
}

acdlib::ACD_USAGE BufferObject::getACDUsage (GLenum usage)
{
    switch(usage)
    {
        case (GL_STATIC_DRAW_ARB):
        case (GL_STATIC_READ_ARB):
        case (GL_STATIC_COPY_ARB):
            return acdlib::ACD_USAGE_STATIC;
            break;

        case (GL_DYNAMIC_DRAW_ARB):
        case (GL_DYNAMIC_READ_ARB):
        case (GL_DYNAMIC_COPY_ARB):
        case (GL_STREAM_DRAW_ARB):
        case (GL_STREAM_READ_ARB):
        case (GL_STREAM_COPY_ARB):
            return acdlib::ACD_USAGE_DYNAMIC;
            break;

        default:
            panic("BufferObject","getACDUsage","Type not supported yet");
            return static_cast<acdlib::ACD_USAGE>(0);
    }
}

BufferObject::~BufferObject()
{

}

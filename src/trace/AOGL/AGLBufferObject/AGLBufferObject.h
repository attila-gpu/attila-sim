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

#ifndef AGL_BUFFEROBJECT_H
    #define AGL_BUFFEROBJECT_H

#include "AGLBaseObject.h"
#include "ACDBuffer.h"
#include "ACDDevice.h"
#include "GPUTypes.h"
#include "gl.h"

namespace agl
{

class BufferTarget;

class BufferObject : public BaseObject
{

private:    
          
    acdlib::ACDBuffer* _data; // asociated data 

    acdlib::ACDDevice* _acddev;
    
public:     

    BufferObject(GLuint name, GLenum targetName);
    
    void attachDevice(acdlib::ACDDevice* device);

    GLenum getUsage() const;
    
    acdlib::ACDBuffer* getData();

    GLsizei getSize();
    
    void setContents(GLsizei size, const GLubyte* data, GLenum usage);
    
    void setContents(GLsizei size, GLenum usage); // change just size & usage (discard previous data)
   
    void setPartialContents(GLsizei offset, GLsizei size, const void* data);

    acdlib::ACD_USAGE getACDUsage (GLenum usage);
    
    ~BufferObject();
    
};

} // namespace libgl

#endif // BUFFEROBJECT_H

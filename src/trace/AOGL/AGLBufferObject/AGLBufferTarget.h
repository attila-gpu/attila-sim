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

#ifndef AGL_BUFFERTARGET_H
    #define AGL_BUFFERTARGET_H

#include "AGLBaseTarget.h"
#include "AGLBufferObject.h"
#include "gl.h"

namespace agl
{

class BufferTarget : public BaseTarget
{

protected:    

    BufferObject* createObject(GLuint name);

public:

    BufferTarget(GLenum target);
    
    BufferObject& getCurrent() const
    {
        return static_cast<BufferObject&>(BaseTarget::getCurrent());
    }

};

} // namespace libgl


#endif // BUFFERTARGET_H

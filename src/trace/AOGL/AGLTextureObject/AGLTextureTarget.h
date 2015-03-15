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

#ifndef AGL_TEXTURETARGET
    #define AGL_TEXTURETARGET

#include "gl.h"
#include "glext.h"
#include "AGLBaseTarget.h"
#include "AGLTextureObject.h"
#include "support.h"

namespace agl
{

class GLTextureTarget : public BaseTarget
{
private:

    GLenum envMode;
    
protected:
    
    GLTextureObject* createObject(GLuint name);
    
public:

    GLTextureTarget(GLuint target);    
    
    GLTextureObject& getCurrent() const
    {
        return static_cast<GLTextureObject&>(BaseTarget::getCurrent());
    }
            
};

} // namespace libgl

#endif // AGL_TEXTURETARGET

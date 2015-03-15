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

#ifndef TEXTURETARGET_H
    #define TEXTURETARGET_H

#include "BaseTarget.h"
#include "TextureObject.h"
#include <map>
#include "gl.h"

namespace libgl
{

class TextureTarget : public BaseTarget
{
private:

    GLenum envMode;
    
protected:
    
    TextureObject* createObject(GLuint name);
    
public:

    TextureTarget(GLuint target);    
    
    // Wraps cast
    TextureObject& getCurrent() const
    {
        return static_cast<TextureObject&>(BaseTarget::getCurrent());
    }
            
};

} // namespace libgl

#endif // TEXTURETARGET_H

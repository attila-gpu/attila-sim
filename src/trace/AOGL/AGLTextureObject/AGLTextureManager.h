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

#ifndef AGL_TEXTUREMANAGER
    #define AGL_TEXTUREMANAGER

#include "gl.h"
#include "glext.h"
#include "AGLBaseManager.h"
#include "AGLTextureTarget.h"
#include "AGLTextureObject.h"

namespace agl
{

class GLTextureManager: public BaseManager
{

private:

    static GLTextureManager* tm;

public:

    GLTextureManager(GLenum textureUnits = 0);

    GLTextureManager(const GLTextureManager&);

    static GLTextureManager& instance();

    GLTextureTarget& getTarget(GLenum target) const
    {
        return static_cast<GLTextureTarget&>(BaseManager::getTarget(target));
    }
    
    /*GLTextureObject& bindObject(GLenum target, GLuint name)
    {
        return static_cast<GLTextureObject&>(BaseManager::bindObject(target, name));
    }*/
    
    bool removeObjects(GLsizei n, const GLuint* names)
    {
        return BaseManager::removeObjects(n, names);
    }           

}; // class GLTextureManager

} // namespace agl

#endif // AGL_TEXTUREMANAGER

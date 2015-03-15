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

#ifndef AGL_BUFFERMANAGER_H
    #define AGL_BUFFERMANAGER_H

#include "AGLBaseManager.h"
#include "AGLBufferTarget.h"
#include "gl.h"

namespace agl
{

class BufferManager : public BaseManager
{

private:

    
    
    static BufferManager* bom;
    
public:

    BufferManager();
    BufferManager(const BufferManager&);

    BufferObject& getBuffer(GLenum target, GLenum name);

    BufferTarget& target(GLenum target) const
    {
        return static_cast<BufferTarget&>(BaseManager::getTarget(target));
    }    
    
    static BufferManager& instance();
    
    bool removeObjects(GLsizei n, const GLuint* names)
    {
        return BaseManager::removeObjects(n, names);
    }
    
};

}

#endif // BUFFERMANAGER_H

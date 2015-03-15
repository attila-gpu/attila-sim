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

#include "BufferManager.h"
#include "support.h"
#include "glext.h"

using namespace libgl;

BufferManager* BufferManager::bom = 0;

/* Create singleton */
BufferManager& BufferManager::instance()
{
    if ( !bom )
        bom = new BufferManager;
    return *bom;
}

BufferManager::BufferManager()
{
    // Create targets
    addTarget(new BufferTarget(GL_ARRAY_BUFFER));
    addTarget(new BufferTarget(GL_ELEMENT_ARRAY_BUFFER)); 
}

BufferObject& BufferManager::getBuffer(GLenum target, GLenum name)
{
    BufferObject* bo = static_cast<BufferObject*>(findObject(name));
    if ( !bo )
        panic("BufferManager", "getBuffer", "Buffer object not found");
    if ( bo->getTargetName() != target )
        panic("BufferManager", "getBuffer", "target does not match with target name of buffer object looked for");
    
    return *bo;
}

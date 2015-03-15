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

#include "BufferTarget.h"

using namespace libgl;

BufferTarget::BufferTarget(GLuint target) : BaseTarget(target)
{
    resetCurrent(); // no current (unbound)
}

BufferObject* BufferTarget::createObject(GLuint name)
{
    BufferObject* bo = new BufferObject(name, getName());
    bo->setTarget(*this);
    return bo;
}


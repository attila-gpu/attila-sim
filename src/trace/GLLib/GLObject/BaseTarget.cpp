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

#include "BaseTarget.h"
#include "support.h"

using namespace libgl;

BaseTarget::BaseTarget(GLuint target) : target(target), current(0), def(0)
{
    // empty
}

GLuint BaseTarget::getName() const
{
    return target;
}

BaseObject& BaseTarget::getCurrent() const
{
    if ( current == 0 )
        panic("BaseTarget","getCurrent", "Current does not exist");

    return *current;
}

bool BaseTarget::setCurrent(BaseObject& bo)
{
    current = &bo;
    return true;
}

bool BaseTarget::resetCurrent()
{
    current = 0;
    return true;
}

bool BaseTarget::hasCurrent() const
{
    return (current != 0);
}

BaseTarget::~BaseTarget()
{
    delete def;
}

void BaseTarget::setDefault(BaseObject* d)
{
    if ( d->getName() != 0 )
        panic("BaseTarget", "setDefault", "Default object must have name equal to zero");
    if ( def != 0 )
        panic("BaseTarget", "setDefault", "this method cannot be called more than once");
    def = d;
}

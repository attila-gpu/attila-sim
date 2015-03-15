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

#include "AGLBaseTarget.h"
#include "support.h"

using namespace agl;

BaseTarget::BaseTarget(GLuint target) : _target(target), _current(0), _def(0)
{
    // empty
}

GLuint BaseTarget::getName() const
{
    return _target;
}

BaseObject& BaseTarget::getCurrent() const
{
    if ( _current == 0 )
        panic("BaseTarget","getCurrent", "Current does not exist");

    return *_current;
}

bool BaseTarget::setCurrent(BaseObject& bo)
{
    _current = &bo;
    return true;
}

bool BaseTarget::resetCurrent()
{
    _current = 0;
    return true;
}

bool BaseTarget::hasCurrent() const
{
    return (_current != 0);
}

BaseTarget::~BaseTarget()
{
    delete _def;
}

void BaseTarget::setDefault(BaseObject* d)
{
    if ( d->getName() != 0 )
        panic("BaseTarget", "setDefault", "Default object must have name equal to zero");
    if ( _def != 0 )
        panic("BaseTarget", "setDefault", "this method cannot be called more than once");
    _def = d;
}

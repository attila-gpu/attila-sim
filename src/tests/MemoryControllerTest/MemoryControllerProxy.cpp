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

#include "MemoryControllerProxy.h"

using namespace gpu3d;

MemoryControllerProxy::MemoryControllerProxy(const Box* mc) : mc(mc)
{}

string MemoryControllerProxy::getDebugInfo() const
{
    string debugInfo;
    mc->getDebugInfo(debugInfo);
    return debugInfo;
}


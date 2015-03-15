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

#ifndef MEMORYCONTROLLERPROXY_H
    #define MEMORYCONTROLLERPROXY_H

#include "MemoryControllerV2.h"
#include "MemoryController.h"
#include <string>

namespace gpu3d
{

// Simplified memory controller interface to query memory controllers state
class MemoryControllerProxy
{
private:

    const Box* mc;

public:

    MemoryControllerProxy(const Box* mc);

    std::string getDebugInfo() const;

};

}

#endif // MEMORYCONTROLLERPROXY_H

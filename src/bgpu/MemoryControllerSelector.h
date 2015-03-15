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

#ifndef MEMORYCONTROLLERSELECTOR_H
    #define MEMORYCONTROLLERSELECTOR_H

#include "Box.h"
#include "ConfigLoader.h"

namespace gpu3d
{

Box* createMemoryController(SimParameters& simP, 
                            const char** tuPrefixes, 
                            const char** suPrefix,
                            const char** slPrefixes,
                            const char* memoryControllerName,
                            Box* parentBox);

}

#endif // MEMORYCONTROLLERSELECTOR_H

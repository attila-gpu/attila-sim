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
 * Global Profiler implementation file.
 *
 */

/**
 *
 *  @file GlobalProfiler.cpp
 *
 *  This file implements a Global Profiler class that can be used for
 *  profiling in a whole program.
 *
 */

#include "GlobalProfiler.h"

namespace gpu3d
{

GlobalProfiler::GlobalProfiler() : Profiler()
{
}

GlobalProfiler& GlobalProfiler::instance()
{
    static GlobalProfiler *gp = NULL;
    
    if (gp == NULL)
        gp = new GlobalProfiler;
    
    return *gp;
}

}   // Namespace gpu3d


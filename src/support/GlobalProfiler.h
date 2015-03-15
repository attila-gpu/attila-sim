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
 * Global Profiler definition and implementation file.
 *
 */

/**
 *
 *  @file GlobalProfiler.cpp
 *
 *  This file defines and implements a Global Profiler class that can be used for
 *  profiling in a whole program.
 *
 */

#include "Profiler.h"

#undef GLOBALPROFILER_ENTERREGION
#undef GLOBALPROFILER_EXITREGION
#undef GLOBALPROFILER_GENERATEREPORT

#ifdef ENABLE_GLOBAL_PROFILER
    #define GLOBALPROFILER_ENTERREGION(a, b, c) gpu3d::GlobalProfiler::instance().enterRegion((a), (b), (c));
    #define GLOBALPROFILER_EXITREGION() gpu3d::GlobalProfiler::instance().exitRegion();
    #define GLOBALPROFILER_GENERATEREPORT(a) gpu3d::GlobalProfiler::instance().generateReport((a));
#else
    #define GLOBALPROFILER_ENTERREGION(a, b, c)
    #define GLOBALPROFILER_EXITREGION()
    #define GLOBALPROFILER_GENERATEREPORT(a)
#endif

#ifndef _GLOBALPROFILER_

#define _GLOBALPROFILER_


namespace gpu3d
{

class GlobalProfiler : public Profiler
{
private:

    //  Avoid explicit constructor and copies.
    GlobalProfiler();
    GlobalProfiler(const gpu3d::GlobalProfiler&);
    GlobalProfiler& operator=(const GlobalProfiler&);

public:

    static GlobalProfiler& instance();
};

}   // namespace gpu3d


#endif


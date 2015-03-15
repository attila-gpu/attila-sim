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
 * Parser definition file.
 *
 */

/**
 *
 *  @file Profiler.h
 *
 *  This file defines the Profiler class used as a profiling tool.
 *
 */

#include "GPUTypes.h"

#include <map>
#include <vector>

using namespace std;

#ifndef _PROFILER_
#define _PROFILER_

// Macros for use within code so it gets compiled out

// Function prototypes
namespace gpu3d
{

class Profiler
{

private:

    struct ProfileRegion
    {
	    u64bit ticks;
	    u64bit visits;
	    u64bit ticksTotal;
	    u64bit visitsTotal;
	    string regionName;
	    string className;
	    string functionName;
    };

    map<string, ProfileRegion *> profileRegions;
    
    vector<ProfileRegion *> regionStack;
    
    u32bit regionStackSize;
    
    u64bit lastTickSample;
    
    u64bit allTicks;
    
    u64bit allTicksTotal;
    
    ProfileRegion *currentProfileRegion;
    
    static u64bit sampleTickCounter();
    
    u32bit lastProcessor;
    bool differentProcessors;
    u32bit processorChanges;
    
public:

    Profiler();
    
    ~Profiler();
    
    void reset();
    
    void enterRegion(char *regionName, char *className, char *functionName);
        
    void exitRegion();
    
    void generateReport(char *fileName);
    
};

}   // namespace gpu3d


#endif	// _PROFILER_

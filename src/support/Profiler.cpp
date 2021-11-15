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
 * Profiler implementation file.
 *
 */

/**
 *
 *  @file Profiler.cpp
 *
 *  This file implements the Profiler class used as a profiling tool.
 *
 */

#include "Profiler.h"

#include <cstdio>

#ifdef WIN32
    #define _WIN32_WINNT 0x400
    #include "windows.h"
    #include <intrin.h>
    
    #define FORCE_PROCESSOR
    //#define CHECK_PROCESSOR
    
#endif

#ifdef WIN32
    #pragma intrinsic(__rdtsc)
#endif

namespace gpu3d
{

Profiler::Profiler()
{
    profileRegions.clear();
    regionStack.clear();
    regionStackSize = 0;
    allTicks = 0;
    allTicksTotal = 0;

#ifdef WIN32

#ifdef FORCE_PROCESSOR
    SetThreadAffinityMask(GetCurrentThread(), 0x01);
#endif

#ifdef CHECK_PROCESSOR    
    lastProcessor = GetCurrentProcessorNumber();
    printf("Profiler => Initialized at processor %d\n", lastProcessor);
#endif
    
#endif    
}

Profiler::~Profiler()
{
    map<string, ProfileRegion*>::iterator it;
    it = profileRegions.begin();
    while (it != profileRegions.end())
    {
        delete it->second;
        it++;
    }

    profileRegions.clear();
    regionStack.clear();
}

u64bit Profiler::sampleTickCounter()
{
#ifdef WIN32

    //  This may not work in a multiprocessor if the process
    //  moves from one processor to another.
    return __rdtsc();	

#else

    u64bit tickSample;
    
    //  Not sure if this code works on Linux or Cygwin.
    
    __asm__ __volatile__ ("rdtsc" : "=&A" (tickSample));
    
    return tickSample;
    
#endif
}

void Profiler::enterRegion(char *regionName, char *className, char *functionName)
{
    //  Update the tick count for the current region.
    u64bit ticks = sampleTickCounter() - lastTickSample;

#ifdef WIN32    
#ifdef CHECK_PROCESSOR    
    u32bit proc = GetCurrentProcessorNumber();           
    differentProcessors = (lastProcessor != proc);
    if (differentProcessors)
    {
        processorChanges++;
        lastProcessor = proc;
    }
#endif    
#endif
                
    //  Check if inside a region.
    if (regionStackSize != 0)
    {
        regionStack[regionStackSize - 1]->ticks += ticks;
        allTicks += ticks;
    }
    
    //  Search the region in the map of profile regions.
    map<string, ProfileRegion*>::iterator it;
    it = profileRegions.find(string(regionName));
    if (it != profileRegions.end())
    {
        //  Found.  Set as current profile region.
        it->second->visits++;
        regionStack.push_back(it->second);
        regionStackSize++;
    }
    else
    {
        //  Not found.  Create new region.
        ProfileRegion *region = new ProfileRegion;
        region->ticks = 0;
        region->ticksTotal = 0;
        region->visits = 1;
        region->visitsTotal = 0;
        region->regionName = string(regionName);
        region->className = string(className);
        region->functionName = string(functionName);
        
        //  Add region to the map.
        profileRegions[string(regionName)] = region;
        
        //  Set as current profile region.
        regionStack.push_back(region);
        regionStackSize++;
    }
    
    //  Sample tick counter.
    lastTickSample = sampleTickCounter();;
}

void Profiler::exitRegion()
{
    //  Update the tick count for the region.
    u64bit ticks = sampleTickCounter() - lastTickSample;

#ifdef WIN32
#ifdef CHECK_PROCESSOR    
    u32bit proc = GetCurrentProcessorNumber();           
    differentProcessors = (lastProcessor != proc);
    if (differentProcessors)
    {
        processorChanges++;
        lastProcessor = proc;
    }  
#endif
#endif    

    //  Check if inside a region.
    if (regionStackSize != 0)
    {
        regionStack[regionStackSize - 1]->ticks += ticks;
        allTicks += ticks;
     
        //  Exit this region and move to the previous one.
        regionStack.pop_back();
        regionStackSize--;
    }
    
    //  Sample tick counter.
    lastTickSample = sampleTickCounter();
}

void Profiler::reset()
{
    map<string, ProfileRegion*>::iterator it;
    it = profileRegions.begin();
    while (it != profileRegions.end())
    {
        it->second->ticksTotal += it->second->ticks;
        it->second->ticks = 0;
        it->second->visitsTotal = it->second->visits;
        it->second->visits = 0;
        
        it++;
    }
    allTicksTotal += allTicks;
    allTicks = 0;
}

void Profiler::generateReport(char *filename)
{

    FILE *outFile = fopen(filename, "w");
    
    if (outFile == NULL)
        panic("Profiler", "generateReport", "Error opening profiler report file.");
    
    u32bit maxLength[3];
    maxLength[0] = 15;
    maxLength[1] = 10;
    maxLength[2] = 15;
    
    //  Order by the number of ticks spend in the region.
    multimap<u64bit, ProfileRegion *> orderedRegions;
    
    map<string, ProfileRegion*>::iterator it;
    it = profileRegions.begin();
    while (it != profileRegions.end())
    {
        orderedRegions.insert(pair<u64bit, ProfileRegion*>(it->second->ticks, it->second));
        maxLength[0] = (maxLength[0] < it->second->regionName.size()) ? (u32bit) it->second->regionName.size() : maxLength[0];
        maxLength[1] = (maxLength[1] < it->second->className.size()) ? (u32bit) it->second->className.size() : maxLength[1];
        maxLength[2] = (maxLength[2] < it->second->functionName.size()) ? (u32bit) it->second->functionName.size() : maxLength[2];
        it++;
    }
    
    fprintf(outFile, "Total Ticks Accounted = " U64FMT "\n", allTicks);
    fprintf(outFile, "\n");

#ifdef WIN32    
#ifdef CHECK_PROCESSOR
    fprintf(outFile, "Sampled cycle counter from different processors? = %s.  Processor changes = %d\n",
        (differentProcessors ? "yes" : "false"), processorChanges);
    fprintf(outFile, "\n");
#endif CHECK_PROCESSOR    
#endif
    
    u32bit paddingSpaces = (maxLength[0] - 11) >> 1;
    for(u32bit c = 0; c < paddingSpaces; c++)
        fprintf(outFile, " ");
    fprintf(outFile, "Region Name");
    paddingSpaces = ((maxLength[0] - 11) >> 1) + ((maxLength[0] - 11) & 0x01);
    for(u32bit c = 0; c < paddingSpaces; c++)
        fprintf(outFile, " ");
    fprintf(outFile,"   ");
    paddingSpaces = (maxLength[1] + maxLength[2] - 23) >> 1;
    for(u32bit c = 0; c < paddingSpaces; c++)
        fprintf(outFile, " ");
    fprintf(outFile, "ClassName::FunctionName");
    paddingSpaces = ((maxLength[1] + maxLength[2] - 23) >> 1) + ((maxLength[1] + maxLength[2] - 23) & 0x01);
    for(u32bit c = 0; c < paddingSpaces; c++)
        fprintf(outFile, " ");
    fprintf(outFile, "       Visits                Ticks              Time\n");
    fprintf(outFile, "\n");
    
    //  Print info for all the regions ordered from most ticks to less ticks.
    multimap<u64bit, ProfileRegion*>::reverse_iterator it2;
    it2 = orderedRegions.rbegin();
    while (it2 != orderedRegions.rend())
    {
        u32bit paddingSpaces = maxLength[0] - it2->second->regionName.size();
        for(u32bit c = 0; c < paddingSpaces; c++)
            fprintf(outFile, " ");
        fprintf(outFile, "%s", it2->second->regionName.c_str());
        fprintf(outFile, "  ");
        paddingSpaces = maxLength[1] + maxLength[2] - it2->second->className.size() - it2->second->functionName.size();        
        for(u32bit c = 0; c < paddingSpaces; c++)
            fprintf(outFile, " ");
        fprintf(outFile,"%s::%s",
            it2->second->className.c_str(),
            it2->second->functionName.c_str());
#ifdef WIN32        
        fprintf(outFile, "    %12I64d    %24I64d    %3.2f\n",
#else
        fprintf(outFile, "    %12lld    %24lld    %3.2f\n",
#endif        
            it2->second->visits,
            it2->second->ticks,
            100.0f * f64bit(it2->second->ticks) / f64bit(allTicks));
            
        it2++;
    }
    
    fclose(outFile);
}

}   // namespace gpu3d



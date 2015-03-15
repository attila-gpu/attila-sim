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

#ifndef PROGRAMOBJECTCACHE_H
    #define PROGRAMOBJECTCACHE_H

#include <map>
#include <list>
#include <vector>
#include "ProgramObject.h"
#include "gl.h"

namespace libgl
{

class ProgramObjectCache
{
public:

    ProgramObjectCache(u32bit maxCacheablePrograms, u32bit startPID, GLenum targetName);
    /**
     * Try to find a previous program with the same source code, if it is found the program
     * is returned, if not, a new program is created and added to the program cache
     */
    ProgramObject* get(const std::string& source, bool &cached);
    
    /**
     * Returns the  most recently used program
     */
    ProgramObject* getLastUsed() const;
    
    /**
     * Clear the program object from cache
     * the cleared programs are returned in a std::vector structure
     */
    std::vector<ProgramObject*> clear();
    
    bool full() const;
    
    // Returns the amount of programs stored currently in the cache
    u32bit size() const;    
    
    void dumpStatistics() const;
    
private:

    typedef std::multimap<u32bit,ProgramObject*> ProgramCache;
    typedef ProgramCache::iterator PCIt;
    typedef std::pair<PCIt,PCIt> Range;
    
    ProgramObject* lastUsed;

    GLenum targetName;
    u32bit maxPrograms;
    u32bit startPID;
    u32bit nextPID;
    ProgramCache cache;
    
    // STATISTICS
    u32bit hits;
    u32bit misses;
    u32bit clears;
    
    u32bit computeChecksum(const std::string& source) const;
};

} // namespace libgl

#endif // PROGRAMOBJECTCACHE_H

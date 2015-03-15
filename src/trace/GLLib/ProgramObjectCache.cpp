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

#include "ProgramObjectCache.h"
#include "gl.h"
#include "glext.h"

using namespace std;
using namespace libgl;

ProgramObjectCache::ProgramObjectCache(u32bit maxCacheablePrograms, u32bit startPID, GLenum targetName) : 
    maxPrograms(maxCacheablePrograms), startPID(startPID), nextPID(startPID), targetName(targetName), lastUsed(0),
    // Statistics
    hits(0), misses(0), clears(0)
{}

u32bit ProgramObjectCache::computeChecksum(const std::string& source) const
{
    u32bit sz = source.length();
    const char* sourceAscii = source.c_str();
    
    u32bit cs = 0;
    for ( u32bit i = 0; i < sz; i++ ) // trivial checksum
        cs += u32bit(sourceAscii[i]);
    
    return cs;
}

ProgramObject* ProgramObjectCache::getLastUsed() const
{
    return lastUsed;    
}


ProgramObject* ProgramObjectCache::get(const std::string& source, bool &cached)
{
    u32bit cs = computeChecksum(source);
    Range range = cache.equal_range(cs);

    
    if ( range.first != range.second )
    {
        for ( PCIt matching = range.first; matching != range.second; ++matching )
        {
            if ( matching->second->getSource() == source )
            {
                hits++;
                cached = true;
                return (lastUsed = matching->second);
            }
        }
    }
    
    if ( cache.size() == maxPrograms )
        return 0;
        
    misses++;
    
    lastUsed = new ProgramObject(nextPID, targetName);
    nextPID++;
    cache.insert(make_pair(cs, lastUsed));
    cached = false;
    return lastUsed;
}

vector<ProgramObject*> ProgramObjectCache::clear()
{
    clears++;

    vector<ProgramObject*> v;
    v.reserve(cache.size()); // Simple optimization
    
    PCIt it = cache.begin();
    for ( ; it != cache.end(); it++ )
        v.push_back(it->second);

    // Reset internal state 
    lastUsed = 0;
    nextPID = startPID;
    cache.clear();
    return v;
}

bool ProgramObjectCache::full() const
{
    return cache.size() == maxPrograms;
}

u32bit ProgramObjectCache::size() const
{
    return cache.size();
}

void ProgramObjectCache::dumpStatistics() const
{
    if ( targetName == GL_VERTEX_PROGRAM_ARB )
        cout << "ProgramObjectCache - VERTEX programs\n";
    else
        cout << "ProgramObjectCache - FARGMENT programs\n";
    
    cout << "hits: " << hits << " (" << (u32bit)(100.0f * ((f32bit)hits/(f32bit)(hits + misses))) << "%)\n";
    cout << "misses: " << misses << " (" << (u32bit)(100.0f * ((f32bit)misses/(f32bit)(hits + misses))) << "%)\n";
    cout << "clears: " << clears << "\n";
    cout << "programs in cache: " << cache.size() << endl;
}

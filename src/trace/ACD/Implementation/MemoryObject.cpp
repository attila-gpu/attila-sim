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

#include "MemoryObject.h"
#include "support.h"

using namespace acdlib;
using namespace std;


MemoryObject::MemoryObject() : _globalReallocs(0), _preferredMemory(MT_LocalMemory)
{}

void MemoryObject::setPreferredMemory(MemoryType mem)
{
    _preferredMemory = mem;
}

MemoryObject::MemoryType MemoryObject::getPreferredMemory() const
{
    return _preferredMemory;
}

void MemoryObject::getUpdateRange(acd_uint region, acd_uint& startByte, acd_uint& lastByte)
{
    map<acd_uint, MemoryObjectRegion>::const_iterator it = mor.find(region);
    if ( it == mor.end() )
        panic("MemoryObject", "getUpdateRange", "Region not found/defined");

    startByte = it->second.firstByteToUpdate;
    lastByte = it->second.lastByteToUpdate;
}

vector<acd_uint> MemoryObject::getDefinedRegions() const
{
    vector<acd_uint> regs;//(mor.size());

    map<acd_uint, MemoryObjectRegion>::const_iterator it = mor.begin();
    map<acd_uint, MemoryObjectRegion>::const_iterator end = mor.end();

    for ( ; it != end; ++it )
        regs.push_back(it->first);

    return regs;
}

acd_uint MemoryObject::trackRealloc(acd_uint region) const
{
    map<acd_uint, MemoryObjectRegion>::const_iterator it = mor.find(region);
    if ( it == mor.end() )
        panic("MemoryObject", "trackRealloc", "Tracking reallocation of an undefined region");

    return it->second.reallocs;
}

void MemoryObject::lock(acd_uint region, acd_bool lock)
{
	map<acd_uint, MemoryObjectRegion>::iterator it = mor.find(region);
	if ( it == mor.end() )
		panic("MemoryObject", "lock", "Searching an undefined region");

	it->second.locked = lock;
}

acd_bool MemoryObject::isLocked(acd_uint region) const
{
	map<acd_uint, MemoryObjectRegion>::const_iterator it = mor.find(region);
	if ( it == mor.end() )
		panic("MemoryObject", "isLocked", "Searching if it is lock an undefined region");

	return it->second.locked;
}

void MemoryObject::preload(acd_uint region, acd_bool preloadData)
{
	map<acd_uint, MemoryObjectRegion>::iterator it = mor.find(region);
	if ( it == mor.end() )
		panic("MemoryObject", "preload", "Searching an undefined region");

	it->second.preload = preloadData;
}

acd_bool MemoryObject::isPreload(acd_uint region) const
{
	map<acd_uint, MemoryObjectRegion>::const_iterator it = mor.find(region);
	if ( it == mor.end() )
		panic("MemoryObject", "isPreload", "Searching if it is lock an undefined region");

	return it->second.preload;
}

acd_uint MemoryObject::trackRealloc() const
{
    return _globalReallocs;
}

void MemoryObject::defineRegion(acd_uint region)
{
    // Discard previous region definition (if exists) and define a new region
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.find(region);
    if ( it == mor.end() )
    {
        MemoryObjectRegion info;
        info.state = MOS_ReAlloc;
        info.reallocs = 0;
		info.locked = false;
		info.preload = false;
        mor.insert(make_pair(region,info));
    }
    else
    {
        if ( it->second.state != MOS_ReAlloc ) 
        {
            it->second.state = MOS_ReAlloc;
            it->second.reallocs++;
			it->second.locked = false;
			it->second.preload = false;
            _globalReallocs++;
        }
    }
}

void MemoryObject::_postUpdate(acd_uint moID, MemoryObjectRegion& moRegion, acd_uint startByte, acd_uint lastByte)
{ 
    if ( lastByte == 0 ) { // Update from start to last region byte
        memoryData(moID, lastByte); // Get memory object region size in 'lastByte' parameter
        --lastByte;
    }

    switch ( moRegion.state )
    {
        case MOS_ReAlloc:
            return ; // ignore partial update, this region will be reallocated completely
        case MOS_Blit:
            //panic("MemoryObject", "_postUpdate", "Blitted memory regions can not be updated (not implemented)");
            break;
        case MOS_RenderBuffer:
            //panic("MemoryObject", "_postUpdate", "Render buffer regions can not be updated (not implemented).");            
            break;
        case MOS_Sync:
            moRegion.state = MOS_NotSync;
            moRegion.firstByteToUpdate = startByte;
            moRegion.lastByteToUpdate = lastByte;
            break;
        case MOS_NotSync:
            // Enlarge "update range" if required
            if ( moRegion.firstByteToUpdate > startByte )
                moRegion.firstByteToUpdate = startByte;
            if ( moRegion.lastByteToUpdate < lastByte )
                moRegion.lastByteToUpdate = lastByte;
            break;
        default:
            panic("MemoryObject", "_postUpdate", "Unknown memory object state");
    }
}

void MemoryObject::postUpdate(acd_uint region, acd_uint startByte, acd_uint lastByte)
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.find(region);
    if ( it == mor.end() )
        panic("MemoryObject", "postUpdate", "Posting an update region in a region that does not exist");

    _postUpdate(it->first, it->second, startByte, lastByte);    
}

void MemoryObject::postUpdateAll()
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.begin();
    const map<acd_uint, MemoryObjectRegion>::iterator itEnd = mor.end();
    for ( ; it != itEnd; ++it )
        _postUpdate(it->first, it->second , 0, 0);
}

void MemoryObject::postReallocate(acd_uint region)
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.find(region);
    if ( it == mor.end() )
        panic("MemoryObject", "postReallocate", "Posting ReAlloc in a region that does not exist");

    if ( it->second.state != MOS_ReAlloc ) {
        it->second.state = MOS_ReAlloc;
        it->second.reallocs++; // increment reallocs if the object/region was not already in state realloc
		it->second.locked = false;
        _globalReallocs++;
    }
}

void MemoryObject::postReallocateAll()
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.begin();
    const map<acd_uint, MemoryObjectRegion>::iterator itEnd = mor.end();
    for ( ; it != itEnd; ++it ) {
        if ( it->second.state != MOS_ReAlloc ) {
            it->second.state = MOS_ReAlloc;
            it->second.reallocs++; // increment reallocs if the object/region was not already in state realloc
			it->second.locked = false;
            _globalReallocs++;
        }
    }
}

void MemoryObject::postBlit(acd_uint region)
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.find(region);
    if ( it == mor.end() )
        panic("MemoryObject", "postBlit", "Posting Blit in a region that does not exist");

    it->second.state = MOS_Blit;
}

void MemoryObject::postBlitAll()
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.begin();
    const map<acd_uint, MemoryObjectRegion>::iterator itEnd = mor.end();
    for ( ; it != itEnd; ++it )
        it->second.state = MOS_Blit;
}

void MemoryObject::postRenderBuffer(acd_uint region)
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.find(region);
    if ( it == mor.end() )
        panic("MemoryObject", "postRenderBuffer", "Posting Blit in a region that does not exist");

    it->second.state = MOS_RenderBuffer;
}

void MemoryObject::postRenderBufferAll()
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.begin();
    const map<acd_uint, MemoryObjectRegion>::iterator itEnd = mor.end();
    for ( ; it != itEnd; ++it )
        it->second.state = MOS_RenderBuffer;
}

const acd_char* MemoryObject::stringType() const
{
    return "MEMORY_OBJECT_STRING_NOT_DEFINED";
}


MemoryObjectState MemoryObject::getState(acd_uint region) const
{
    map<acd_uint, MemoryObjectRegion>::const_iterator it = mor.find(region);
    
    if ( it == mor.end() )
        panic("MemoryObject", "getState", "Region not found/defined");

    return it->second.state;
}

void MemoryObject::changeState(acd_uint region, MemoryObjectState newState)
{
    map<acd_uint, MemoryObjectRegion>::iterator it = mor.find(region);
    
    if ( it == mor.end() )
        panic("MemoryObject", "changeState", "Region not found/defined");

    it->second.state = newState;
}

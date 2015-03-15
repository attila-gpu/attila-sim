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

#include "MemoryObjectAllocator.h"
#include "MemoryObject.h"
#include "support.h"
#include "GPUDriver.h"
#include <sstream>
#include "ACDMacros.h"

using namespace acdlib;
using namespace std;

MemoryObjectAllocator::MemoryObjectAllocator(GPUDriver* driver) : _driver(driver)
{    
    lockOutMem = &lockedMemory[0];
	lockInMem = &lockedMemory[1];
}

MemoryObjectAllocator::MemoryObjectInfo* MemoryObjectAllocator::_findMOI(MemoryObject* mo)
{
    MemoryObjectMap::iterator it = maps.find(mo);
    if ( it != maps.end() )
        return &(it->second);
    return 0;
}


// This method requires that the MemoryObject passed as parameter holds:
//  1. The region passed as parameter is defined
//  2. The region passed as parameter its in state MOS_NotSync
// This method also requires that the MemoryObjectInfo corresponds to
// the memory object passed as argument
void MemoryObjectAllocator::_update(MemoryObject* mo, MemoryObjectInfo* moi, acd_uint region)
{
    // Gets the memory data of this object
    acd_uint memorySizeDummy;
    const acd_ubyte* data = mo->memoryData(region, memorySizeDummy);

    if ( !data || memorySizeDummy == 0 )
        panic("MemoryObjectAllocator", "_update", "MemoryObject data NULL or size 0 ");
    
    // Get the range of the memory to update from the selected region
    acd_uint startByte, lastByte;
    mo->getUpdateRange(region, startByte, lastByte);

    //  Check if the region contains data to be preloaded into GPU memory.
    if (mo->isPreload(region))
    {
        // Update the GPU memory corresponding to the region 
	    _driver->writeMemoryPreload((*moi)[region].md, startByte, data + startByte, lastByte - startByte + 1);
    }
    else
    {    
        // Update the GPU memory corresponding to the region 
	    _driver->writeMemory((*moi)[region].md, startByte, data + startByte, lastByte - startByte + 1, mo->isLocked(region));
    }
    
    mo->changeState(region, MOS_Sync);
}

MemoryObjectAllocator::MemoryObjectInfo* MemoryObjectAllocator::_createMOI(MemoryObject* mo)
{
    pair< map<MemoryObject*, MemoryObjectInfo>::iterator, acd_bool> info = 
        maps.insert(make_pair(mo, MemoryObjectInfo()));

    ACD_ASSERT
    (
        if ( !info.second )
            panic("MemoryObjectAllocator", "_createMOI", "MOI already exists for the memory object");
    )

    return &(info.first->second);
}


void MemoryObjectAllocator::_addRegionToMOI(MemoryObjectInfo* moi, acd_uint region, acd_uint md, acd_uint size)
{
    pair<MemoryObjectInfo::iterator, acd_bool> info =
        moi->insert(make_pair(region, RegionInfo(md, size)));

    ACD_ASSERT
    (
        if ( !info.second )
            panic("MemoryObjectAllocator", "addRegionToMOI", "The memory region already exists for the memory object");
    )
}


void MemoryObjectAllocator::_alloc( MemoryObject* mo, MemoryObjectInfo* moi, acd_uint region )
{
    // Get the memory data of the memory object and its size in bytes
    acd_uint size;
    const acd_ubyte* data = mo->memoryData(region, size);

    if ( !data || size == 0 )
        panic("MemoryObjectAllocator", "_alloc", "MemoryObject data NULL or size 0 ");
    
    // Get the amount of memory required to allocate the memory object region
    acd_uint md;
    if ( mo->getPreferredMemory() == MemoryObject::MT_LocalMemory )
        md = _driver->obtainMemory(size, GPUDriver::GPUMemoryFirst);
    else if ( mo->getPreferredMemory() == MemoryObject::MT_SystemMemory )
        md = _driver->obtainMemory(size, GPUDriver::SystemMemoryFirst); 
    else
        panic("MemoryObjectAllocator", "_alloc", "Unknown memory type");

    //  Check if the memory region contains data to be preloaded into GPU memory.
    if (mo->isPreload(region))
    {
        // Update the GPU memory
        _driver->writeMemoryPreload(md, 0, data, size);
    }
    else
    {
        // Update the GPU memory
        _driver->writeMemory(md, data, size, mo->isLocked(region));
    }
    
    // Update the "memory object information" with the new allocated region information
    _addRegionToMOI(moi, region, md, size);

    mo->changeState(region, MOS_Sync);
}

// The region exists in the MOI passed as parameter
void MemoryObjectAllocator::_dealloc(MemoryObject* mo, MemoryObjectInfo* moi, acd_uint region)
{
    // Get an iterator pointing to the RegionInfo to deallocate
    MemoryObjectInfo::iterator it = moi->find(region);

    // Release the associated GPU/system memory
    _driver->releaseMemory(it->second.md);

    // Remove RegionInfo pointed by 'it'
    moi->erase(it); 

    // Mark this object region as not allocated (requiring ReAlloc)
    mo->postReallocate(region); 
}

void MemoryObjectAllocator::syncGPU(MemoryObject* mo)
{
    vector<acd_uint> regs = mo->getDefinedRegions();
    vector<acd_uint>::const_iterator it = regs.begin();
    vector<acd_uint>::const_iterator end = regs.end();
    
    for ( ; it != end; ++it ) // synchronize all regions
        syncGPU(mo, *it);
}

acd_bool MemoryObjectAllocator::syncGPU(MemoryObject* mo, acd_uint region)
{
    ACD_ASSERT
    (
        { 
            acd_uint size;
            if ( !mo->memoryData(region, size) || size == 0) {
                stringstream ss;
                ss << "Trying to synchronize a memory region (" << region << ") in GPU memory that does not exist";
                panic("MemoryObjectAllocator", "syncGPU", ss.str().c_str());
            }
        }
    )

    MemoryObjectInfo* moi = _findMOI(mo);
    switch ( mo->getState(region) )
    {
        case MOS_Sync:
            break;
        case MOS_RenderBuffer:
            //panic("MemoryObjectAllocator", "syncGPU", "The region is defined as a render buffer, sync from CPU not supported");
            break;            
        case MOS_Blit:
            //panic("MemoryObjectAllocator", "syncGPU", "The region has previously blit, sync from CPU not supported");
            break;
        case MOS_ReAlloc:
            if ( !moi )
                moi = _createMOI(mo); // create MOI if there is not exist yet
            else if ( moi->find(region) != moi->end() )
                _dealloc(mo, moi, region); // deallocate previous region

            _alloc(mo, moi, region);

			mo->lock(region, true);
			lockInMem->push_back(lockedRegion(mo,region));
            break;
        case MOS_NotSync:            
            _update(mo, moi, region);

			mo->lock(region, true);
			lockInMem->push_back(lockedRegion(mo,region));

            break;
        case MOS_NotFound:
        default:
            panic("MemoryObjectAllocator", "syncGPU", "The region to synchronize does not exist");
    }
    return true;
}

void MemoryObjectAllocator::assign(MemoryObject *mo, acd_uint region, acd_uint md)
{
    MemoryObjectInfo* moi = _findMOI(mo);

    switch(mo->getState(region))
    {
        case MOS_Sync:
            break;
        case MOS_Blit:
            panic("MemoryObjectAllocator", "assign", "Blit not supported.");

        case MOS_RenderBuffer:
        
            //  Check if the memory object was already defined.
            if (!moi)
                moi = _createMOI(mo); // create MOI if there is not exist yet
            else if (moi->find(region) != moi->end())
                _dealloc(mo, moi, region); // deallocate previous region
                           
            // Update the "memory object information" with the new allocated region information
            _addRegionToMOI(moi, region, md, 0);

            break;
                        
        case MOS_ReAlloc:
            panic("MemoryObjectAllocator", "assign", "Allocate not supported.");
        case MOS_NotSync:            
            panic("MemoryObjectAllocator", "assign", "Update not supported.");
            break;
        case MOS_NotFound:
        default:
            panic("MemoryObjectAllocator", "assign", "The region to synchronize does not exist");
    }
}



acd_bool MemoryObjectAllocator::deallocate(MemoryObject* mo, acd_uint subregion)
{
    MemoryObjectInfo* moi = _findMOI(mo);
    if ( !moi )
        return false; // Nothing to deallocate

    // Deallocate the subregion
    _dealloc(mo, moi, subregion);

    // If the memory object info is empty, remove it from the main 'maps' hash
    if ( moi->empty() )
        maps.erase(mo);

    return true;
}

acd_bool MemoryObjectAllocator::deallocate(MemoryObject* mo)
{
    MemoryObjectInfo* moi = _findMOI(mo);
    if ( !moi )
        return false; // Nothing to deallocate

    vector<acd_uint> regs = mo->getDefinedRegions();
    vector<acd_uint>::const_iterator it = regs.begin();
    vector<acd_uint>::const_iterator end = regs.end();
    
    for ( ; it != end; ++it ) // synchronize all regions
        _dealloc(mo, moi, *it);

	std::vector <std::vector<lockedRegion>::iterator > release;

	std::vector<lockedRegion>::iterator iter = lockedMemory[0].begin();
	for (; iter != lockedMemory[0].end(); iter++)
		if ((*iter).mo == mo)
			release.push_back(iter);

	for (acd_uint i = 0; i < release.size(); i++)
		lockedMemory[0].erase(release[i]);

	release.clear();

	iter = lockedMemory[1].begin();
	for (; iter != lockedMemory[1].end(); iter++)
		if ((*iter).mo == mo)
			release.push_back(iter);

	for (acd_uint i = 0; i < release.size(); i++)
		lockedMemory[1].erase(release[i]);

	release.clear();

    maps.erase(mo);

    return true;
}

acd_uint MemoryObjectAllocator::md(MemoryObject* mo, acd_uint region)
{
    if ((mo->getState(region) != MOS_Sync) && 
        (mo->getState(region) != MOS_Blit) &&
        (mo->getState(region) != MOS_RenderBuffer)) return 0;
        //panic("MemoryObjectAllocator", "md", "Memory object must be synchronized before calling this method");
        

    MemoryObjectInfo* moi = _findMOI(mo);

    ACD_ASSERT(
        if ( !moi )
            panic("MemoryObjectAllocator", "md", "Unexpected error. MOI not found and the MemoryObject has a synchronized region");
    )

    MemoryObjectInfo::iterator it = moi->find(region);
    
    ACD_ASSERT(
        if ( it == moi->end() )
            panic("MemoryObjectAllocator", "md", "Unexpected error. Region information not found"); 
    )

    return it->second.md;        
}

acd_bool MemoryObjectAllocator::hasMD(MemoryObject* mo, acd_uint region)
{
    return _findMOI(mo);
}

void MemoryObjectAllocator::realeaseLockedMemoryRegions()
{
	std::vector<lockedRegion>::iterator it = lockOutMem->begin();

	for(; it != lockOutMem->end(); it++)
		(*it).mo->lock((*it).region, false);

	lockOutMem->clear();

	std::vector<lockedRegion>* aux = lockOutMem;
	lockOutMem = lockInMem;
	lockInMem = aux;
}

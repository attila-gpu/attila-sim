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

#include "GPUMemory.h"
#include "GPUDriver.h"
#include <iostream>
#include "support.h"

using namespace std;
using namespace libgl;

GPUMemory::GPUMemory(GPUDriver* driver) : driver(driver)
{    
    // empty
}

GPUMemory::BaseObjectInfo* GPUMemory::_find(BaseObject* bo)
{
    map<BaseObject*, BaseObjectInfo>::iterator it = maps.find(bo);
    if ( it != maps.end() )
    {
        return &(it->second);
    }    
    return 0;
}

void GPUMemory::_update(BaseObject* bo, BaseObjectInfo* boi)
{    
    for ( GLuint i = 0; i < bo->getNumberOfPortions(); i++ )
    {
        vector<pair<GLuint, GLuint> > modifRanges = bo->getModifiedRanges();
        u8bit* binData = (u8bit*)bo->binaryData(i);
        u32bit binSize = bo->binarySize(i);
        
        for ( GLuint j = 0; j < modifRanges.size(); j++ ) // update regions
        {
            GLuint offset = modifRanges[j].first;
            GLuint size = modifRanges[j].second;
            // check memory bounds
            if ( offset + size > binSize )
                panic("GPUMemory", "_update", "Trying to write farther from memory bounds");
            driver->writeMemory(boi->md[i], offset, binData, size);            
        }
    }
    bo->setSynchronized();
    //bo->setState(BaseObject::Sync);
}


void GPUMemory::_alloc(BaseObject* bo)
{
    //cout << "start\n _alloc: " << bo->toString() << endl;
    BaseObjectInfo boi;
    GLuint portions = bo->getNumberOfPortions();
    
    for ( GLuint i = 0; i < portions; i++ )
    {
        //cout << "Sync portion: " << i;        
        cout.flush();
        boi.size.push_back(bo->binarySize(i));
        u32bit desc;
        if ( bo->getPreferredMemoryHint() == BaseObject::GPUMemory ) // Try to use GPUMemory for this base object
            desc = driver->obtainMemory(boi.size[i], GPUDriver::GPUMemoryFirst);
        else
            desc = driver->obtainMemory(boi.size[i], GPUDriver::SystemMemoryFirst); // Try to use System memory
        boi.md.push_back(desc);
        driver->writeMemory(boi.md[i], bo->binaryData(i), boi.size[i]);
        //cout << " OK" << endl;
    }
    maps.insert(make_pair(bo, boi));
    //bo->setState(BaseObject::Sync);
    bo->setSynchronized();
    //cout << "end of _alloc" << endl;
}

void GPUMemory::_dealloc(BaseObject* bo, BaseObjectInfo* boi)
{
    vector<u32bit>::iterator it = boi->md.begin();
    for ( ; it != boi->md.end(); it++ )
        driver->releaseMemory(*it);  
    maps.erase(bo);
    //bo->setState(BaseObject::ReAlloc); // object not allocated (must be allocated to be used)
    bo->forceRealloc();
}


bool GPUMemory::allocate(BaseObject& bo)
{
    GPUMemory::BaseObjectInfo* boInfo = _find(&bo);    
    BaseObject::State state = bo.getState();
    if ( boInfo )
    {
        if ( state == BaseObject::ReAlloc )
        {
            //  reallocate
            _dealloc(&bo, boInfo);
            _alloc(&bo);
        }
        else if ( state == BaseObject::NotSync )
        {
            _update(&bo, boInfo);
        }
        else // { ( state == BaseObject::Sync ) -> already synchronized } ||
            //  { ( state == BaseObject::Blit ) -> updated data is already and exclusively in memory  }
        {
            return false;
        }
    }
    else
    {
        if ( state != BaseObject::ReAlloc )
            panic("GPUMemory", "allocate", "Should not happen."
                                           "GPUMemory internal info not found for an allocated BaseObject");
        _alloc(&bo);
    }
    return true;
}

bool GPUMemory::deallocate(BaseObject& bo)
{
    BaseObjectInfo* boInfo = _find(&bo);
    BaseObject::State state = bo.getState();

    if ( boInfo )
        _dealloc(&bo, boInfo);
    else
    {
        panic("GPUMemory", "deallocate", "Trying to deallocate a not allocated object");
        return false;
    }
            
    return true;
}


bool GPUMemory::isPresent(BaseObject& bo)
{
    return ( _find(&bo) != 0 );
}

u32bit GPUMemory::md(BaseObject& bo, GLuint portion)
{
    if ( !bo.isSynchronized() )
    {
        panic("GPUMemory", "md", "Base object must be synchronized before calling this method");
        return 0;
    }
    
    BaseObjectInfo* boi = _find(&bo);
    if ( !boi )
    {
        panic("GPUMemory", "md", "The object is not currently allocated into GPU Memory");
        return 0;
    }
    
    if ( portion >= boi->md.size() )
    {
        char msg[256];
        sprintf(msg, "It does not exist the portion %d in this BaseObject", portion);
        panic("GPUMemory", "md", msg);
    }
    return boi->md[portion];
}


void GPUMemory::dump() const
{
    cout << "------ GPUMemory allocated objects ------\n\n";
    map<BaseObject*,BaseObjectInfo>::const_iterator it = maps.begin();
    for ( ; it != maps.end(); it++ )
    {
        BaseObject* bo = it->first;
        BaseObjectInfo boi = it->second;

        cout << "State: ";
        
        BaseObject::State state = bo->getState();
        switch(state)
        {
            case BaseObject::ReAlloc: cout << "ReAlloc"; break;
            case BaseObject::NotSync: cout << "NotSync"; break;
            case BaseObject::Sync: cout << "Sync"; break;
            case BaseObject::Blit: cout << "Blit"; break;
            default:
                panic("GPUMemory","dump","Unknown state");
        }
        
        cout << endl;
        
        //cout << "ID_STRING: " << bo->toString().c_str() << " :\n";
        cout << "   md's: ";
        
        for (u32bit i = 0; i < boi.md.size(); i++ )
        {
            
            cout << boi.md[i] << endl;
            
            driver->dumpMD(boi.md[i]);
            
            if ( i < boi.md.size() - 1 )
                cout << ", ";
        }
        cout << "\n   size's: ";
        for (u32bit i = 0; i < boi.size.size(); i++ )
        {
            cout << boi.size[i];
            if ( i < boi.size.size() - 1 )
                cout << ", ";
        }
        cout << "\n   Number of portions: " << bo->getNumberOfPortions() << endl << endl;
        
    }
    
    //driver->dumpMemoryAllocation();
    
    cout << "\n-----------------------------------------" << endl;
}

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

#include "MemoryRegion.h"
#include "BufferDescriptor.h"
#include "support.h"
#include <algorithm>
#include <iostream>
#include <sstream>
#include <cstring>

using namespace std;

MemoryManager::MemoryManager() : nextFreeID(0), nextFileOffset(0), cacheSz(10)
{}


void MemoryManager::_loadMemoryInfo(MemoryRegion* mr)
{
    char rawData[12];
    file.read(rawData, 12);
    mr->id = *((uint*)rawData);
    mr->startAddress = *((uint*)(rawData+4));
    mr->size = *((uint*)(rawData+8));
}

bool MemoryManager::open(const char* memoryRegionsFile, MM_Mode mode)
{
    if ( mode == MM_ReadOnly  )
    {
        fileReadOnly = true;
        file.open(memoryRegionsFile, ios::in | ios::binary);
        if ( !file.is_open() )
        {
            stringstream ss;
            ss << "File \"" << memoryRegionsFile << "\" could not be opened in read only mode";
            panic("MemoryManager", "open", ss.str().c_str());
        }
        if ( !mrv.empty() )
            panic("MemoryManager", "open", "Previous contents were loaded");

        // Load memory regions (only descriptors, maintain data in file)
        uint nextOffset = 0;
        while ( !file.eof() )
        {
            MemoryRegion* m = new MemoryRegion(*this);
            _loadMemoryInfo(m);
            if ( file.gcount() == 0 )
            {
                delete m;
                break;
            }

            if ( m->size == 0 )
            {
                stringstream ss;
                ss << "Memory region: " << m->id << ". Field with data size equals to 0";
                panic("MemoryManager", "open", ss.str().c_str());
            }

            m->fileOffset = nextOffset;
            mrv.push_back(m);
            nextOffset = nextOffset + MemoryRegionFixedBytes + m->size;
            file.seekg(nextOffset);
        }
    }
    else
    {
        fileReadOnly = false; //  file can be read
        file.open(memoryRegionsFile, ios::out | ios::in | ios::binary | ios::trunc);
        if ( !file.is_open() )
        {
            stringstream ss;
            ss << "File \"" << memoryRegionsFile << "\" could not be opened in read/write mode";
            panic("MemoryManager", "open", ss.str().c_str());
        }
    }

    return true;
}

void MemoryManager::_storeDataMemory(MemoryRegion* mr)
{
    // cout << "MemoryManager::_storeDataMemory() -> ID = " << mr->id << endl;

    if ( fileReadOnly )
        panic("MemoryManager", "_storeDataMemory", "File was opened in read-only mode");

    //if ( mr->data == 0 )
    if ( mr->data.empty() )
    {
        stringstream ss;
        ss << "Memory region with ID: " << mr->id << " is unloaded. Cannot be stored";
        panic("MemoryManager", "_storeDataMemory", ss.str().c_str());
    }

    mr->stored = true;

    file.seekp(mr->fileOffset); // find memory region in file
    
    // Write ID
    file.write((const char*)&mr->id, sizeof(uint));
    
    // Write start address
    // It is used to compute Buffer offsets (bd.startAddress - mr.startAddress) = offset
    // It must be computed this way for compatibility reasons with previous memory files
    file.write((const char*)&mr->startAddress, sizeof(uint));
    
    // Write Size
    file.write((const char*)&mr->size, sizeof(uint));
    
    // Write memory contents
    file.write((const char*)mr->data, mr->size);

    file.flush();
}

void MemoryManager::_loadDataMemory(MemoryRegion* mr)
{
    if ( !mr->data.empty() )
    {
        stringstream ss;
        ss << "Memory region " << mr->id << " is already loaded";
        panic("MemoryManager", "_loadDataMemory", ss.str().c_str());
    }

    file.clear(); // Clear previous soft errors (eofbit) (patch for VS.net && gcc 2.96)

    file.seekg(mr->fileOffset + MemoryRegionFixedBytes); // find memory region in file

    /*
    // Enable this code to check information of each memory region
    uint oldId = mr->id;

    _loadMemoryInfo(mr);

    if ( oldId != mr->id )
    {
        stringstream ss;
        ss << "Memory region with ID = " << mr->id << " Not found. "
            << " ID found: " << oldId;
        panic("MemoryManager", "_loadDataMemory", ss.str().c_str());
    }
    */

    //mr->data = new DArray(mr->size);
    mr->data.set(mr->size);
    file.read(mr->data, mr->size);

    //mr->data = new unsigned char[mr->size];
    //mr->data = (MPtr)memAlloc.get(mr->size);
    //file.read((char*)mr->data, mr->size);

}

void MemoryManager::_unloadDataMemory(MemoryRegion* mr)
{
    if ( mr->data.empty() )
        return ;

    mr->data.clear();
}


void MemoryManager::_updateCacheMemory(MemoryRegion* mr)
{
    // cout << "MemoryManager::_updateCacheMemory() -> " << mr->id << endl;
    list<MemoryRegion*>::iterator it = cache.begin();
    for ( ; it != cache.end(); it++ )
    {
        if ( *it == mr )
        {
            // move to front
            cache.erase(it);
            cache.push_front(mr);
            return ;
        }
    }

    // not found in cache implies Memory Region contents are not loaded
    if ( cache.size() < cacheSz )
    {
        if ( mr->data.empty() )
        {
            if ( !fileReadOnly )
                panic("MemoryManager", "_updateCacheMemory", "Prog error!");
            _loadDataMemory(mr);
        }

        cache.push_front(mr);
    }
    else
    {
        // some memory region must be erased from cache
        if ( fileReadOnly || cache.back()->stored )
            _unloadDataMemory(cache.back());
        else
        {
            _storeDataMemory(cache.back()); // store victim if it was not yet stored
            _unloadDataMemory(cache.back());
        }
        cache.pop_back();

        if ( mr->data.empty() )
            _loadDataMemory(mr);
        cache.push_front(mr);
    }

}

MemoryRegion* MemoryManager::create(const MPtr startAddress, uint bytes)
{
    if ( !file.is_open() )
        panic("MemoryManager", "create", "Memory file not opened");
    if ( fileReadOnly )
        panic("MemoryManager", "create", "In read-only mode Memory Regions cannot be created");

    MemoryRegion* mr = new MemoryRegion(*this, startAddress, bytes);
    mr->id = nextFreeID++;
    mr->fileOffset = nextFileOffset;
    nextFileOffset += (MemoryRegionFixedBytes + bytes);
    mrv.push_back(mr);
    
    _updateCacheMemory(mr);


    stringstream ss;
    uint cs = _generateChecksum(startAddress,bytes);

    _addToHashtable(cs, mr);

    return mr;
}

void MemoryManager::_addToHashtable(uint cs, MemoryRegion* mr)
{
    map<uint,vector<MemoryRegion*> >::iterator it = hashtable.find(cs);
    if ( it == hashtable.end() )
    {

        // vector not yet created, create a new one
        vector<MemoryRegion*> temp;
        temp.push_back(mr);
        hashtable.insert(make_pair(cs,temp));
    }
    else
    {
        // another memory regions with equal checksum exist
        vector<MemoryRegion*>& v = it->second;
        vector<MemoryRegion*>::iterator it2 = v.begin();

        for ( ; it2 != v.end(); it2++ )
        {  
            // check if the memory region already exists
            if ( (*it2)->compare(*mr) )
            {
                stringstream ss;
                ss << "Memory region " << (*it2)->id << " is equals to Memory region " << mr->id;
                panic("MemoryManager", "_addToHashtable", ss.str().c_str());
            }
        }
        v.push_back(mr);
    }
}

uint MemoryManager::_generateChecksum(const MPtr data, uint bytes)
{
    if ( sizeof(unsigned short) != 2 )
        panic("MemoryManager", "_generateChecksum", "sizeof(unsigned short) != 2");

    uint cs = 0;
    uint trailingBytes = bytes % 2;
    uint items = bytes / 2;
    
    const unsigned short* ptr = (const unsigned short*)data;
    if ( trailingBytes == 1 )
        cs = (uint)data[bytes-1];

    for ( uint i = 0; i < items; i++ )
        cs += ptr[i];

    return cs;
}


MemoryRegion* MemoryManager::find(uint id)
{
    if ( id >= mrv.size() )
    {

        stringstream ss;
        ss << "ID: " << id << " too high. Maximum ID found is: " << (mrv.size() - 1);
        panic("MemoryManager", "find", ss.str().c_str());
    }
    
    if ( mrv[id]->id != id )
    {
        stringstream ss;
        ss << "ID: " << id << " not found. Instead it was found ID: " << mrv[id]->id;
        panic("MemoryManager", "find", ss.str().c_str());
    }

    // DO not update cache, update cache only when contents are accessed (getData())
    //_updateCacheMemory(mrv[id]);
    return mrv[id];
}


MemoryRegion* MemoryManager::find(const MPtr data, uint bytes)
{
    if ( fileReadOnly )
        panic("MemoryManager", "find", "Searching by contents not available when read-only mode is selected");

    uint cs = _generateChecksum(data, bytes);
    vector<MemoryRegion*> matches = _findByChecksum(cs);
    if ( matches.empty() )
        return 0;

    vector<MemoryRegion*>::const_iterator it = matches.begin();
    for ( ; it != matches.end(); it++ )
    {
        MemoryRegion* mr = *it;
        //if ( mr->data ) 
        if ( !mr->data.empty() )
        {
            // Memory contents are in cache (available)
            if ( mr->compare(data,bytes) )
            {
                _updateCacheMemory(mr);
                return mr;
            }
        }
        else
        {
            // Contents not available (not in cache)
            _loadDataMemory(mr);
            if ( mr->compare(data,bytes) )
            {
                _updateCacheMemory(mr);
                return mr;
            }
            _unloadDataMemory(mr);
        }
    }
    return 0;
}

vector<MemoryRegion*> MemoryManager::_findByChecksum(uint cs) const
{
    map<uint,vector<MemoryRegion*> >::const_iterator it = hashtable.find(cs);
    
    if ( it != hashtable.end() )
        return it->second;

    return vector<MemoryRegion*>(); // matches not found
}

void MemoryManager::setCacheSize(uint size)
{
    if ( size >= cacheSz )
        cacheSz = size;
    else
    {
        if ( size >= cache.size() )
            cacheSz = size;
        else 
        {
            cacheSz = size;
            // flush excess buffers            
            list<MemoryRegion*>::iterator it = cache.begin();
            for ( uint i = 0; i < size; i++ )
                it++;

            list<MemoryRegion*>::iterator it2 = it;

            for ( ; it != cache.end(); it++ )
            {   // flush if it is required
                if ( !(*it)->stored )
                    _storeDataMemory(*it);
                _unloadDataMemory(*it);
            }
            cache.erase(it2, cache.end());
        }
    }
}

void MemoryManager::flushCache()
{
    //popup("MemoryManager::flushCache", "Starting to flush the Memory Regions cache");
    list<MemoryRegion*>::iterator it = cache.begin();
    for ( ; it != cache.end(); it++ )
    {
        if ( !(*it)->stored )
            _storeDataMemory(*it);
        _unloadDataMemory(*it);
    }
    cache.clear();
    //popup("MemoryManager::flushCache", "Memory Region cache flushed");
}



uint MemoryManager::getCacheSize() const
{
    return cacheSz;
}

uint MemoryManager::getCacheUsage() const
{
    return (uint) cache.size();
}

void MemoryManager::touch(MemoryRegion* mr)
{
    _updateCacheMemory(mr);
}

MPtr MemoryManager::obtainDataMemoryCopy( MemoryRegion& mr)
{
    if ( !mr.data.empty() )
    {
        stringstream ss;
        ss << "MRid = " << mr.id << " is in RAM memory. This method should not be called";
        panic("MemoryManager", "obtainDataMemoryCopy", ss.str().c_str());
    }
    _loadDataMemory(&mr); // allocate
    char* temp = new char[mr.size];
    memcpy(temp, (const char*)mr.data, mr.size); // copy
    _unloadDataMemory(&mr); // deallocate
    return (MPtr)temp;
}

bool MemoryManager::close()
{
    flushCache();

    vector<MemoryRegion*>::iterator it = mrv.begin();
    for ( ; it != mrv.end(); it++ )
    {
        delete (*it);
    }

    mrv.clear();

    file.close();
    if ( file.is_open() )
    {
        panic("MemoryManager", "close", "Memory file could not be closed");
        return false;
    }
    return true;
}

void MemoryManager::dump() const
{
    cout << "MemoryManager::dump():" << endl;
    vector<MemoryRegion*>::const_iterator it = mrv.begin();
    for ( ; it != mrv.end(); it++ )
        (*it)->dump();
    cout.flush();
}

uint MemoryManager::countMemoryRegions() const
{
    return (uint) mrv.size();
}

uint MemoryManager::getDataBytesInCache() const
{
    uint totalBytes = 0;
    list<MemoryRegion*>::const_iterator it = cache.begin();
    for ( ; it != cache.end(); it++ )
        totalBytes += (*it)->size;
    return totalBytes;
}

uint MemoryManager::getTotalDataBytes() const
{
    uint totalBytes = 0;
    vector<MemoryRegion*>::const_iterator it = mrv.begin();
    for ( ; it != mrv.end(); it++ )
        totalBytes += (*it)->size;
    return totalBytes;
}

MemoryManager::~MemoryManager()
{
    close();
}




//////////////////////////////////////////
////////// MemoryRegion methods //////////
//////////////////////////////////////////

MemoryRegion::MemoryRegion(MemoryManager& m, const MPtr ptr, uint bytes) : 
    size(bytes), stored(false), manager(m)
{
    if ( ptr == 0 || bytes == 0 )
        panic("MemoryRegion", "constructor", "Trying to create an empty memory region");

    if ( ptr != 0 )
        data.set(ptr, bytes);
    else
        data.set(bytes);

    //data = new DArray(ptr, bytes);

    /*
    data = (MPtr) new char[bytes];
    //data = (MPtr) m.memAlloc.get(bytes);
    memcpy(data, ptr, bytes);
    */
#ifndef WIN32    
    startAddress = (((unsigned long) ptr) & 0xFFFFFFFF) ;
#else
    startAddress = (uint) ptr;
#endif
}

// used to re-create memory regions that are in memory file
MemoryRegion::MemoryRegion(MemoryManager& m) : manager(m), stored(true)
{}

uint MemoryRegion::getID() const
{
    return id;
}

uint MemoryRegion::getStartAddress() const
{
    return startAddress;
}

uint MemoryRegion::getSize() const
{
    return size;
}

const MPtr MemoryRegion::getData(uint offset)
{
    if ( offset >= size )
    {
        stringstream ss;
        ss << "MRid = " << id << ". ";
        ss << "Offset (" << offset << ") is greater than memory region size ("
           << size << ")";
        panic("MemoryRegion", "getData", ss.str().c_str());
    }
    manager.touch(this); // update internal memory manager cache
    return ( const MPtr) & data[offset];
}

void MemoryRegion::dump() const
{
    cout << "ID: " << id << "\n";
    cout << "Start address: "  << startAddress << "\n";
    cout << "size (bytes): " << size << "\n";
    cout << "File offset: " << fileOffset << "\n";
    if ( !data.empty() )
        cout << "Memory data available in RAM" << endl;
    else
        cout << "Memory data stored in HD" << endl;
}


bool MemoryRegion::compare(const MemoryRegion& mr) const
{
    if ( size != mr.size )
        return false;
    
    MemoryRegion* mr1 = (MemoryRegion*)this;
    MemoryRegion* mr2 = (MemoryRegion*)&mr;
    
    bool empty1 = mr1->data.empty();
    bool empty2 = mr2->data.empty();

    char* mr1Data;
    char* mr2Data;

    if ( empty1 )
        mr1Data = (char *)manager.obtainDataMemoryCopy(*mr1);
    else
        mr1Data = mr1->data;

    if ( empty2 )
        mr2Data = (char *)manager.obtainDataMemoryCopy(*mr2);
    else
        mr2Data = mr2->data;

    if ( mr1 == 0 || mr2 == 0 )
    {
        stringstream ss;
        ss << "MRid = " << mr1->id << "(@ = " << mr1->startAddress << ") compared with ";
        ss << "MRid = " << mr2->id << "(@ = " << mr2->startAddress << ")";
        panic("MemoryRegion", "compare", ss.str().c_str());
    }

    bool comp =  memcmp((const char*)mr1Data, (const char*)mr2Data, size) == 0;


    if ( empty1 )
        delete[] mr1Data;
    if ( empty2 )
        delete[] mr2Data;

    return comp;
}

bool MemoryRegion::compare(const MPtr data, uint bytes) const
{
    if ( size != bytes )
        return false;

    manager.touch((MemoryRegion*)this);

    return memcmp((const char*)this->data, data, bytes) == 0;
}

MemoryRegion::~MemoryRegion()
{
    data.clear();
}


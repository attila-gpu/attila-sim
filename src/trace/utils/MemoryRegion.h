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

#ifndef MEMORYREGION_H
    #define MEMORYREGION_H

#include <vector>
#include <list>
#include <map>
#include <fstream>
#include "DArray.h"

typedef unsigned int uint;
typedef unsigned char* MPtr;

class BufferDescriptor;
class MemoryManager;

class MemoryRegion
{

public:

    friend class MemoryManager;

    uint getID() const;
    void resize(uint bytes);
    uint getSize() const;
    uint getStartAddress() const;
    //uint getLastAddress() const;
    const MPtr getData(uint offset = 0);

    MemoryManager& getManager() const;

    bool compare(const MemoryRegion& mr) const;
    bool compare(const MPtr data, uint size) const;

    void dump() const;

    static uint computeChecksum(const MPtr data, uint bytes);

    ~MemoryRegion();

private:

    MemoryRegion(MemoryManager& m);
    MemoryRegion(MemoryManager& m, const MPtr ptr, uint bytes);
    MemoryRegion(const MemoryRegion&);
    MemoryRegion& operator=(const MemoryRegion&);

    uint id;
    uint startAddress; // compatibility reasons
    uint size;
    //MPtr data;
    DArray data;
    MemoryManager& manager;

    bool stored;
    uint fileOffset;

    uint getFileOffset();
    void setFileOffset(uint offset);
    void deallocateData();
    void allocateData(const MPtr data, uint bytes);
    
};


class MemoryManager
{
public:

    MemoryManager();
    ~MemoryManager();

    // Can be used directly
    //MemoryAllocator memAlloc;

    enum MM_Mode
    {
        MM_ReadOnly,
        MM_ReadWrite
    };

    uint countMemoryRegions() const;
    uint getDataBytesInCache() const;
    uint getTotalDataBytes() const;


    bool open(const char* memoryRegionsFile, MM_Mode mode);

    bool close();
    void setCacheSize(uint size);
    uint getCacheSize() const;
    uint getCacheUsage() const;

    void flushCache(); // called implicitly by close and Memory Manager destructor

    MemoryRegion* create(const MPtr startAddress, uint bytes);
    MemoryRegion* find(uint id);
    MemoryRegion* find(const MPtr data, uint bytes);

    /**
     * Data returned must be deallocated by method caller
     *
     * This method does not updates internal cache
     * Is is useful to obtain data from a MemoryRegion with data stored in file
     * but without caching that data or polluting the internal cache
     */
    MPtr obtainDataMemoryCopy(MemoryRegion& mr);

    /**
     * Force memory region to be in cache
     */
    void touch(MemoryRegion* mr);

    void dump() const;

    // temporary public
    void store(MemoryRegion* mr)
    {
        _storeDataMemory(mr);
    }

    void load(MemoryRegion* mr)
    {
        _loadDataMemory(mr);
    }

    void unload(MemoryRegion* mr)
    {
        _unloadDataMemory(mr);
    }

private:
    
    /**
     * memory region ID
     * startAddress
     * data size (bytes)
     */ 
    enum { MemoryRegionFixedBytes = 12 };

    bool fileReadOnly;

    
    uint nextFreeID;
    uint nextFileOffset;

    MemoryManager(const MemoryManager&);
    MemoryManager& operator=(const MemoryManager&);

    std::vector<MemoryRegion*> mrv; // Direct access by ID
    std::map<uint,std::vector<MemoryRegion*> > hashtable; // checksum hashtable

    uint cacheSz; // maximum number of memory regions with loaded data
    std::list<MemoryRegion*> cache; // contain memory regions with data loaded

    void _updateCacheMemory(MemoryRegion* mr);

    std::fstream file;

    /**
     * Reads { id, startAddress, size } from current file position
     */
    void _loadMemoryInfo(MemoryRegion* mr);

    void _loadDataMemory(MemoryRegion* mr);
    void _unloadDataMemory(MemoryRegion* mr);
    void _storeDataMemory(MemoryRegion* mr);
    void _unloadAll();

    static uint _generateChecksum(const MPtr data, uint bytes);
    
    std::vector<MemoryRegion*> _findByChecksum(uint cs) const;

    void _addToHashtable(uint cs, MemoryRegion* mr);

    

};

#endif // MEMORYREGION_H

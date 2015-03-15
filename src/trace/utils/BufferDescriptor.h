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

#ifndef BUFFERDESCRIPTOR_H
    #define BUFFERDESCRIPTOR_H

#include <vector>
#include <string>

typedef unsigned int uint;
typedef unsigned char* BPtr;

class MemoryManager;
class MemoryRegion;
class BufferPrinter;
class BufferManager;

class BufferDescriptor
{
public:

    friend class BufferManager;

    uint getID() const;
    uint getChecksum() const;
    bool hasContents() const;
    uint getSize() const;

    /**
     * This method does not allocate any storage
     * This method is used to define which amount of memory is required to
     * store the data that will be placed in this object
     */
    void setSize(uint bytes);

    uint getStartAddress() const;
    uint getLastAddress() const;
    const BPtr getData() const;

    bool isDeferred() const;

    // These methods require a defined memory region
    bool equals(const BufferDescriptor* bd) const;
    bool equals(const BPtr ptr, uint bytes) const;

    void setMemory(const BPtr data, uint bytes);

    void dump(BufferPrinter* printer = 0) const;

    void show() const;


private:

    BufferManager& bm;
    MemoryRegion* mr;
    uint startAddress;
    uint size;
    uint id;
    uint offset;

    BufferDescriptor(BufferManager& manager);
    BufferDescriptor(BufferManager& manager, const BPtr ptr, uint bytes);
    
    // Protect copy and copy-creation
    BufferDescriptor(const BufferDescriptor&);
    BufferDescriptor& operator=(const BufferDescriptor&);

    void setMemory(MemoryRegion* mr, uint offset);
    MemoryRegion* getMemory() const;
    uint getOffset() const;
};

class BufferManager
{
public:

    //static BufferManager& instance();

    BufferManager();

    ~BufferManager();

    enum WorkingMode
    {
        Loading,
        Storing,
        NotSelected
    };

    WorkingMode getMode() const;

    bool open(const char* bufferDescriptorsFile, const char* memoryFile, WorkingMode mode);
    bool close();
    
    /**
     * Memory is immediately allocated and size set
     */
    BufferDescriptor* create(const BPtr ptr, uint bytes);

    BufferDescriptor* createDeferred(const BPtr ptr);

    /**
     * Memory inmediatelly allocated and size set (but contents are undefined)
     */
    BufferDescriptor* createDeferred(uint bytes);

    BufferDescriptor* find(uint id) const;

    // Configure fast dynamic memory allocator
    void initDynMemory(uint dynAllocs, uint memoryPool, uint clusterSz);

    void setCacheMemorySize(uint cacheableMemoryRegions);
    uint getCacheMemorySize() const;
    void flushCache();

    void setMemory(BufferDescriptor* bd, const BPtr data, uint bytes);
    
    
    /**
     * Allocate memory for a set of buffer descriptors that may share parts of
     * their address spaces
     *
     * Buffer descriptors must be deferred but with a defined size
     */
    void setMemory(std::vector<BufferDescriptor*> bds);    

    void dump(BufferPrinter* printer = 0) const;

    void printMemUsage() const;

    void acceptDeferredBuffers(bool accept);
    bool acceptDeferredBuffers() const;

    uint count() const;

    uint countDeferredBuffers() const;

private:

    /**
     * Accepts loading/storing of deferred buffers
     *
     * This property is false by default
     */
    bool acceptDeferredBuffersFlag;

    WorkingMode workingMode;

    uint nextFreeID;

    std::vector<BufferDescriptor*> bdv;

    MemoryManager* mem;

    std::string outputFile;

    void _printFormatedSize(std::ostream& os, uint sizeInBytes);

    BufferManager(const BufferManager&);
    BufferManager& operator=(const BufferManager&);
};

class BufferPrinter
{
public:

    virtual void print(std::ostream& out, const BufferDescriptor& bd) = 0;
};



#endif // BUFFERDESCRIPTOR_H

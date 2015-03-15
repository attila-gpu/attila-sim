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

#include "BufferDescriptor.h"
#include "MemoryRegion.h"
#include "support.h"
#include <sstream>
#include <algorithm>
#include <iostream>
#include "DArray.h"
#include <fstream>
#include "IncludeLog.h"
#include <cstring>    

using namespace std;
using includelog::logfile; // Make log object visible

struct BufferComparator
{
    bool operator()(const BufferDescriptor* bd1, const BufferDescriptor* bd2)
    {
        return (bd1->getStartAddress() > bd2->getStartAddress());
    }
};


BufferManager::BufferManager() : nextFreeID(0), acceptDeferredBuffersFlag(false),
                                 workingMode(NotSelected)
{
    mem = new MemoryManager;
}

void BufferManager::initDynMemory(uint dynAllocs, uint memoryPool, uint clusterSz)
{
    DArray::init(0, memoryPool, clusterSz);
    DArray::coreDump();
}

BufferDescriptor* BufferManager::find(uint id) const
{
    if ( workingMode == NotSelected )
    {
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Panic, "Working mode not selected yet\n");
        logfile().popInfo();
        panic("BufferManager", "find", "Working mode not selected yet");
    }

    if ( id >= bdv.size() )
    {
        stringstream ss;
        ss << "ID: " << id << " not found. Maximum ID found is: " << bdv.back()->id;
        logfile().pushInfo(__FILE__, __FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferManager", "find", ss.str().c_str());
    }
    if ( bdv[id]->id != id )
    {
        stringstream ss;
        ss << "ID: " << id << " not found. ID: " << bdv[id]->id << " was found instead";
        logfile().pushInfo(__FILE__, __FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferManager", "find", ss.str().c_str());
    }
    
    return bdv[id];
}


void BufferManager::setMemory(vector<BufferDescriptor*> bds)
{
    if ( workingMode == NotSelected )
    {
        logfile().pushInfo(__FILE__, __FUNCTION__);
        logfile().write(includelog::Panic, "Working mode not selected yet\n");
        logfile().popInfo();
        panic("BufferManager", "setMemory", "Working mode not selected yet");
    }

    {   //// Check code ////
        vector<BufferDescriptor*>::const_iterator it = bds.begin();
        for ( ; it != bds.end(); it++ )
        {
            if ( !(*it)->isDeferred() )
            {
                stringstream ss;
                ss << "Buffer descriptor " << (*it)->getID() << " has contents already";
                logfile().pushInfo(__FILE__, __FUNCTION__);
                logfile().write(includelog::Panic, ss.str());
                logfile().write(includelog::Panic, "\n", false);
                logfile().popInfo();
                panic("BufferManager", "setMemory", ss.str().c_str());
            }
            else if ( (*it)->startAddress == 0 )
            {
                stringstream ss;
                ss << "Buffer " << (*it)->id << " with startAddress equals to zero";
                logfile().pushInfo(__FILE__, __FUNCTION__);
                logfile().write(includelog::Panic, ss.str());
                logfile().write(includelog::Panic, "\n", false);
                logfile().popInfo();
                panic("BufferManager", "setMemory", ss.str().c_str());
            }
            else if ( (*it)->getSize() == 0 )
            {
                stringstream ss;
                ss << "Buffer descriptor " << (*it)->getID() << " has size 0 bytes";
                logfile().pushInfo(__FILE__, __FUNCTION__);
                logfile().write(includelog::Panic, ss.str());
                logfile().write(includelog::Panic, "\n", false);
                logfile().popInfo();
                panic("BufferManager", "setMemory", ss.str().c_str());
            }
        }
    }

    // { List of Deferred buffer descriptors }

    sort(bds.begin(), bds.end(), BufferComparator());
    // { Buffer descriptors sorted using start address }
    // addr0 > addr1 > addr2 ... so, addrN is the buffer with the lowest start address


    // Create groups of buffer descriptors that share memory spaces
    vector<vector<BufferDescriptor*> > groups;
    vector<uint> maxAddressInGroup;
    while ( !bds.empty() )
    {
        vector<BufferDescriptor*> newGroup;
        BufferDescriptor* current = bds.back();
        bds.pop_back();
        newGroup.push_back(current);
        maxAddressInGroup.push_back(current->getLastAddress());
        while ( !bds.empty() )
        {
            if ( current->getLastAddress() >= bds.back()->getStartAddress() )
            {
                current = bds.back();
                newGroup.push_back(current); // add it to current group
                bds.pop_back(); // remove it from global vector
                // update max group address
                if ( current->getLastAddress() > maxAddressInGroup.back() )
                    maxAddressInGroup.back() = current->getLastAddress();
            }
            else
                break; // process a new group
        }
        groups.push_back(newGroup);
    }

    if ( groups.size() != maxAddressInGroup.size() )
    {
        logfile().pushInfo(__FILE__, __FUNCTION__);
        logfile().write(includelog::Panic, "Programming error. Shouldn't happen ever\n");
        logfile().popInfo();
        panic("BufferManager", "setMemory", "Programming error. Shouldn't happen ever");
    }

    // treat groups
    // { Each vector group contain buffers ordered in ascending order by address

    for ( uint i = 0; i < groups.size(); i++ )
    {
        vector<BufferDescriptor*> group = groups[i];
        uint startAddress = group.front()->getStartAddress();
        uint lastAddress = maxAddressInGroup[i];
        uint memSz = lastAddress - startAddress + 1;

        MemoryRegion* mr = mem->find((const MPtr)startAddress, memSz);
        if ( !mr )
        {
            mr = mem->create((const MPtr)startAddress, memSz);
            
            char buffer[256];
            logfile().pushInfo(__FILE__, __FUNCTION__);
            sprintf(buffer, "Creating memory region for BufferObject group starting at %x with size %d\n", startAddress, memSz);
            logfile().write(includelog::Debug, buffer);
            logfile().popInfo();
        }
        
        for ( uint j = 0; j < group.size(); j++ )
        {
            // fix for vertex arrays?! uint offset = group[j]->getStartAddress() - startAddress;
            uint offset = 0;
            
            group[j]->setMemory(mr, offset);
            
            char buffer[256];
            logfile().pushInfo(__FILE__, __FUNCTION__);
            sprintf(buffer, "Updating Buffer Descriptor in group with old start address %x with new offset %x\n",
                group[j]->getStartAddress(), offset);                
            logfile().write(includelog::Debug, buffer);
            logfile().popInfo();

            // update BufferDescriptor pointer to the previous created memory region
            // Must be done, since memory regions match based on contents not based on
            // start addresses...
            // fix for vertex arrays?! group[j]->startAddress = mr->getStartAddress();            
        }
    }
}

BufferDescriptor* BufferManager::create(const BPtr ptr, uint bytes)
{
    if ( workingMode != Storing )
    {
        if ( workingMode == NotSelected )
        {
            logfile().pushInfo(__FILE__, __FUNCTION__);
            logfile().write(includelog::Panic, "Working mode not selected yet\n");
            logfile().popInfo();
            panic("BufferManager", "create", "Working mode not selected yet");
        }
        else // Working mode == Loading
        {
            logfile().pushInfo(__FILE__, __FUNCTION__);
            logfile().write(includelog::Panic, "Method call not allowed in \"Loading\" mode\n");
            logfile().popInfo();
            panic("BufferManager", "create", "Method call not allowed in \"Loading\" mode");
        }
    }

    if ( bytes == 0 && ptr == 0 )
    {
        logfile().pushInfo(__FILE__, __FUNCTION__);
        logfile().write(includelog::Panic, "Data pointer & size cannot be both zero\n");
        logfile().popInfo();
        panic("BufferManager", "create", "Data pointer & size cannot be both zero");
    }

    BufferDescriptor* bd = new BufferDescriptor(*this, ptr, bytes);
    bd->id = nextFreeID++;
    bdv.push_back(bd);

    // Not a deferred buffer
    if ( ptr != 0 && bytes != 0 ) 
    {   // initialized memory
        MemoryRegion* m = mem->find(ptr, bytes); // Try to find a previous region
        if ( !m ) // This chunk of memory was not found, create new one
            m = mem->create(ptr, bytes);
        else
        {
            // update BufferDescriptor pointer to the previous created memory region
            // Must be done, since memory regions match based on contents not based on
            // start addresses...
            bd->startAddress = m->getStartAddress();
        }
        bd->setMemory(m, 0);
    }
    //else //(deferred buffer)


    return bd;
}

BufferDescriptor* BufferManager::createDeferred(uint bytes)
{
    return create(0, bytes);
}

BufferDescriptor* BufferManager::createDeferred(const BPtr ptr)
{
    return create(ptr, 0);
}


void BufferManager::acceptDeferredBuffers(bool accept)
{
    acceptDeferredBuffersFlag = accept;
}

bool BufferManager::acceptDeferredBuffers() const
{
    return acceptDeferredBuffersFlag;
}


void BufferManager::dump(BufferPrinter* printer) const
{
    cout << "BufferManager::dump() -> Buffer Descriptors:\n";
    vector<BufferDescriptor*>::const_iterator it = bdv.begin();
    cout << "--------------------------------\n";
    for ( ; it != bdv.end(); it++ )
    {
        (*it)->dump(printer);
        cout << "--------------------------------\n";
    }
}


bool BufferManager::open(const char* bufferDescriptorsFile, const char* memoryFile,
                         WorkingMode mode)
{
    workingMode = mode;

    if ( mode == Loading )
    {        
        // setup memory regions cache
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Init, "Buffer Manager -> Loading mode selected\n");
        
        mem->open(memoryFile, MemoryManager::MM_ReadOnly);

        logfile().write(includelog::Init, "Memory Regions file opened\n");
        logfile().popInfo();

        // Assumes that memory regions are loaded before calling this method
        ifstream loadFile(bufferDescriptorsFile, ios::binary | ios::in );
        if ( !loadFile.is_open() )
        {
            stringstream ss;
            ss << "Buffer descriptors input file: \"" << bufferDescriptorsFile
            << "\" could not be opened or found";
            logfile().pushInfo(__FILE__, __FUNCTION__);
            logfile().write(includelog::Panic, ss.str());
            logfile().write(includelog::Panic, "\n", false);
            logfile().popInfo();
            panic("BufferManager", "load", ss.str().c_str());
        }
        if ( !bdv.empty() )
        {
            logfile().pushInfo(__FILE__, __FUNCTION__);
            logfile().write(includelog::Panic, "Previous buffer descriptors loaded\n");
            logfile().popInfo();
            panic("BufferManager", "load", "Previous buffer descriptors loaded");
        }

        uint next = 0;
        while ( !loadFile.eof() )
        {
            uint v;
            BufferDescriptor* bd = new BufferDescriptor(*this);

            // Buffer Descriptor ID
            loadFile.read((char*)&v, sizeof(uint));
            if ( loadFile.gcount() == 0 )
            {
                delete bd;
                break;
            }
            if ( v != next )
            {
                stringstream ss;
                ss << "Unexpected ID found: " << v << ". It was expected: " << next;
                logfile().pushInfo(__FILE__, __FUNCTION__);
                logfile().write(includelog::Panic, ss.str());
                logfile().write(includelog::Panic, "\n", false);
                logfile().popInfo();
                panic("BufferManager", "load", ss.str().c_str());
            }
            bd->id = v;

            // BufferDescriptor startAddress
            loadFile.read((char*)&v, sizeof(uint));
            bd->startAddress = v;

            // Buffer descriptor size
            loadFile.read((char*)&v, sizeof(uint));
            bd->size = v;
            
            // Memory region ID
            loadFile.read((char*)&v, sizeof(uint)); // Memory region ID
            
            if ( bd->size != 0 )
            {
                bd->mr = mem->find(v);
                if ( bd->mr == 0 )
                {
                    stringstream ss;
                    ss << "Memory region with ID: " << v << " not found";
                    logfile().pushInfo(__FILE__, __FUNCTION__);
                    logfile().write(includelog::Panic, ss.str());
                    logfile().write(includelog::Panic, "\n", false);
                    logfile().popInfo();
                    panic("BufferManager", "load", ss.str().c_str());
                }
                // Compute buffer descriptor offset (legacy compatibility)
                bd->offset = bd->startAddress - bd->mr->getStartAddress();
            }            
            else // deferred buffer (bd->size == 0)
            {
                if ( !acceptDeferredBuffersFlag )
                {
                    stringstream ss;
                    ss << "Buffer descriptor with ID: " << bd->id << " is deferred";
                    logfile().pushInfo(__FILE__, __FUNCTION__);
                    logfile().write(includelog::Panic, ss.str());
                    logfile().write(includelog::Panic, "\n", false);
                    logfile().popInfo();
                    panic("BufferManager", "load", ss.str().c_str());
                }
                if ( v != 0 )
                {
                    stringstream ss;
                    ss << "Memory region ID = " << v 
                       << " and must be 0 in a deferred buffer";
                    logfile().pushInfo(__FILE__, __FUNCTION__);
                    logfile().write(includelog::Panic, ss.str());
                    logfile().write(includelog::Panic, "\n", false);
                    logfile().popInfo();
                    panic("MemoryManager", "open", ss.str().c_str());
                }
                bd->mr = 0;
                bd->offset = 0;
            }

            bdv.push_back(bd);

            next++; // next expected ID
        }
    }
    else  if ( mode == Storing )
    {
        mem->open(memoryFile, MemoryManager::MM_ReadWrite);
        outputFile = bufferDescriptorsFile;
    }

    return true;
}

bool BufferManager::close()
{    
    if ( workingMode == Storing )
    {
        // save buffer descriptors in outputFile
        ofstream storeFile(outputFile.c_str(), ios::binary);
        if ( !storeFile.is_open() )
        {
            stringstream ss;
            ss << "Buffer descriptors output file: \"" << outputFile
            << "\" could not be created";
            logfile().pushInfo(__FILE__,__FUNCTION__);
            logfile().write(includelog::Panic, ss.str());
            logfile().write(includelog::Panic, "\n", false);
            logfile().popInfo();
            panic("BufferManager", "close", ss.str().c_str());
        }

        vector<BufferDescriptor*>::iterator it = bdv.begin();
        for ( ; it != bdv.end(); it++ )
        {
            BufferDescriptor& bd = *(*it);
            if ( bd.mr != 0 )
            {    
                storeFile.write((const char*)&bd.id, sizeof(uint));
                storeFile.write((const char*)&bd.startAddress, sizeof(uint));
                storeFile.write((const char*)&bd.size, sizeof(uint));
                uint mrid = bd.mr->getID();
                storeFile.write((const char*)&mrid, sizeof(uint));
            }
            else
            {    
                if ( !acceptDeferredBuffersFlag )
                {
                    stringstream ss;
                    ss << "Buffer descriptor " << bd.id << " is deferred";
                    logfile().pushInfo(__FILE__,__FUNCTION__);
                    logfile().write(includelog::Panic, ss.str());
                    logfile().write(includelog::Panic, "\n", false);
                    logfile().popInfo();
                    panic("MemoryManager", "close", ss.str().c_str());
                }
                uint zero = 0;
                storeFile.write((const char*)&bd.id, sizeof(uint));
                storeFile.write((const char*)&bd.startAddress, sizeof(uint));
                storeFile.write((const char*)&zero, sizeof(uint));
                storeFile.write((const char*)&zero, sizeof(uint));
            }
        }
        storeFile.close();
        
        vector<BufferDescriptor*>::iterator it2 = bdv.begin();
        for ( ; it2 != bdv.end(); it2++ )
            delete *it2;

        bdv.clear();
        
        mem->close(); // flush memory file cache
    }
    else if ( workingMode == Loading )
    {
        vector<BufferDescriptor*>::iterator it = bdv.begin();
        for ( ; it != bdv.end(); it++ )
            delete *it;
        
        bdv.clear();
    }

    workingMode = NotSelected;

    return true;

}

void BufferManager::setCacheMemorySize(uint cacheableMemoryRegions)
{
    mem->setCacheSize(cacheableMemoryRegions);
}

uint BufferManager::getCacheMemorySize() const
{
    return mem->getCacheSize();
}

void BufferManager::flushCache()
{
    mem->flushCache();
}

BufferManager::~BufferManager()
{
    close();
    delete mem; // Delete memory manager and managed memory regions
}

void BufferManager::printMemUsage() const
{
    uint structuresBytes = sizeof(vector<BufferDescriptor*>) 
                         + sizeof(BufferDescriptor*) * (uint) bdv.size();
    uint bufferDescriptorsBytes = (uint) bdv.size()*  sizeof(BufferDescriptor);

    uint memoryRegionsBytes = mem->countMemoryRegions() * sizeof(MemoryRegion);

    uint memoryCacheBytes = mem->getDataBytesInCache();

    cout << "Internal BufferManager structures: " << structuresBytes << " bytes" << endl;
    cout << "Buffer descriptors (" << bdv.size() << "): " 
         << bufferDescriptorsBytes << " bytes" << endl;
    cout << "Memory region descriptors (" << mem->countMemoryRegions() << "): "
         << memoryRegionsBytes << " bytes" << endl;
    cout << "Cached memory file data: " << memoryCacheBytes << " bytes" << endl;


    uint totalMemory = structuresBytes + bufferDescriptorsBytes +
                       memoryRegionsBytes + memoryCacheBytes;
    
    cout << "Total memory usage: " << totalMemory << " bytes" << endl;
}

void BufferManager::setMemory(BufferDescriptor* bd, const BPtr data, uint bytes)
{
    if ( bd->mr != 0 )
    {
        stringstream ss;
        ss << "Trying to write data in not deferred buffer with ID: " << bd->id;
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferDescriptor", "write", ss.str().c_str());
    }

    if ( bd->startAddress != 0 )
    {
        stringstream ss;
        ss << "Trying to write data in a buffer (ID: " << bd->id << ") with start address defined";
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferDescriptor", "write", ss.str().c_str());
    }
    
    if ( bytes > bd->size )
    {
        stringstream ss;
        ss << "Trying to write " << bytes << " bytes but only can be written as many as "
            << bd->size << " bytes";
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferDescriptor", "write", ss.str().c_str());
    }


    MemoryRegion* m = mem->find(data, bytes);
    if ( !m )
        m = mem->create(data, bytes);
    
    bd->setMemory(m, 0);
    bd->startAddress = m->getStartAddress(); // patch start address
}

BufferManager::WorkingMode BufferManager::getMode() const
{
    return workingMode;
}


////////////////////////////////////////////////
////////// Buffer Descriptor methods ///////////
////////////////////////////////////////////////

BufferDescriptor::BufferDescriptor(BufferManager& manager, const BPtr ptr, uint bytes) : 
    bm(manager), mr(0), size(bytes), id(0), offset(0)
{
   //  Check pointer size.
   if (sizeof(void *) == 8)
      panic("BufferDescriptor", "BufferDescriptor", "Error: only 32-bit pointers supported");

#ifndef WIN32   
   unsigned long t = (unsigned long) ptr;
   startAddress = t & 0xFFFFFFFF; 
#else
   startAddress = (uint) ptr;
#endif
}

// Constructor used to re-create buffer descriptors read from a file
BufferDescriptor::BufferDescriptor(BufferManager& manager) : bm(manager)
{}


void BufferDescriptor::setMemory(MemoryRegion* mr, uint offset)
{
    this->mr = mr;
    this->offset = offset;
}

uint BufferDescriptor::getID() const
{
    return id;
}

void BufferDescriptor::setSize(uint bytes)
{
    if ( mr )
    {
        stringstream ss;
        ss << "Redefining size of buffer: " << id << " when memory was previously allocated ->"
            " prev. size: " << size << " new size: " << bytes;
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferDescriptor", "setSize", ss.str().c_str());
    }
    size = bytes;
}

uint BufferDescriptor::getSize() const
{
    return size;
}

void BufferDescriptor::show() const
{
    stringstream ss;
    ss << "Buffer descriptor information:\n";
    ss << "ID = " << id << "\n"
        "Start Address: " << startAddress << "\n"
        "Offset (computed from start address): " << offset << "\n"
        "Size: " << size << "\n";
    if ( mr )
    {
        ss << "Attached to Memory Region with id = " << mr->getID();
        // show info from MR
        ss << "\n  MR startAddress = " << mr->getStartAddress() << "\n"
              "  MR size = " << mr->getSize();
    }
    else
        ss << "Not attached to any Memory Region yet";
    popup("BufferDescriptor::show", ss.str().c_str());
}



const BPtr BufferDescriptor::getData() const
{
    if ( mr == 0)
    {
        if ( bm.getMode() == BufferManager::Loading && bm.acceptDeferredBuffers() )
            return 0;

        stringstream ss;
        ss << "Buffer data in buffer descriptor " << id << " not defined yet";
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferDescriptor", "getData", ss.str().c_str());
    }


    if ( offset >= mr->getSize() )
        show();

    return mr->getData(offset);
}




bool BufferDescriptor::isDeferred() const
{
    return mr == 0;
}


uint BufferDescriptor::getStartAddress() const
{
    return startAddress;
}

uint BufferDescriptor::getLastAddress() const
{
    return startAddress + size - 1;
}

bool BufferDescriptor::equals(const BPtr ptr, uint bytes) const
{
    if ( mr == 0 )
    {
        stringstream ss;
        ss << "Buffer " << id << " is deferred and cannot be compared (1)";
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferDescriptor", "equals", ss.str().c_str());
    }

    if ( bytes != size )
        return false;

    return (memcmp(ptr, mr->getData(offset), bytes) == 0);
}

bool BufferDescriptor::equals(const BufferDescriptor* bd) const
{
    if ( bd->mr == 0 )
    {
        stringstream ss;
        ss << "Buffer " << bd->id << " is deferred and cannot be compared (2)";
        logfile().pushInfo(__FILE__,__FUNCTION__);
        logfile().write(includelog::Panic, ss.str());
        logfile().write(includelog::Panic, "\n", false);
        logfile().popInfo();
        panic("BufferDescriptor", "equals", ss.str().c_str());
    }

    if ( size != bd->size )
        return false;

    const MPtr d = bd->mr->getData(offset);

    return equals(d, size);
}


void BufferDescriptor::dump(BufferPrinter* printer) const
{
    cout << dec;
    cout << "ID: " << id << "\n"
         << "Start Address: " << startAddress << "\n"
         << "Size (bytes): " << size << "\n"
         << "Offset (bytes): " << offset << "\n";

    if ( mr )
    {
        cout << "Memory:\n ";
        mr->dump();
        if ( printer )
        {
            cout << "Formated buffer: ";
            printer->print(cout, *this);
            cout << endl;
        }

    }
    else
        cout << "Memory:\n NOT DEFINED YET" << endl;
}

uint BufferManager::countDeferredBuffers() const
{
    uint count = 0;
    vector<BufferDescriptor*>::const_iterator it = bdv.begin();
    for ( ; it != bdv.end(); it++ )
    {
        BufferDescriptor& bd = *(*it);
        if ( bd.mr == 0 ) ++count;
    }
    return count;
}

uint BufferManager::count() const
{
    return (uint) bdv.size();
}


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

#include "DArray.h"
#include "support.h"
#include <iostream>
#include <sstream>
#include <algorithm>
#include <string>
#include <cstring>

// Comment out this macro to use fast internal allocation for array data
#define USE_DEFAULT_NEW_DELETE

using namespace std;

char* DArray::memory;
vector<bool> DArray::bitmap;
bool DArray::memSet = false;
unsigned int DArray::darrayCounting = 0;
char* DArray::objectMemory; // memory for objects (darrays)
vector<bool> DArray::objectBitmap;
unsigned int DArray::ClusterSize = 1024;


void DArray::init(size_t maxDArrays, size_t memoryAmount, size_t clusterSize)
{
    #ifdef USE_DEFAULT_NEW_DELETE
        return ;
    #endif

    if ( memSet )
        panic("DArray", "setMemory", "Can be called only once");
    memSet = true;

    objectBitmap.resize(maxDArrays, true);
    objectMemory = new char[maxDArrays*sizeof(DArray)];

    ClusterSize = (uint) clusterSize;

    if ( memoryAmount % clusterSize != 0 )
        panic("DArray", "setMemory", "Memory size must be multiple of cluster size");

    memory = new char[memoryAmount];
    //memset(memory, (int)'X', memoryAmount);
    bitmap.resize(memoryAmount/ClusterSize, true);

}

void DArray::coreDump()
{
    #ifdef USE_DEFAULT_NEW_DELETE
        return ; // ignore this call
    #endif

    stringstream ss;

    uint dynObjects = (uint) count(objectBitmap.begin(), objectBitmap.end(), false);
    uint stackObjects = darrayCounting - dynObjects;

    ss << "Total DArray objects alive: " << darrayCounting << "\n";
    ss << "Alive DArray objects (dynamic created): " <<  dynObjects << "\n";
    ss << "   Maximum number allowed: " << objectBitmap.size() << "\n";
    ss << "Alive DArray objects (created in the stack or statically): " << stackObjects << "\n";

    uint clusters = (uint) count(bitmap.begin(), bitmap.end(), false);
    ss << "Data allocated:\n";
    ss << " Clusters: " << clusters << " clusters of " << bitmap.size() << "\n";
    ss << " Bytes: " << clusters*ClusterSize << " bytes of " 
       << bitmap.size()*ClusterSize << "\n";

    popup("DArray::coreDump", ss.str().c_str());
}

void DArray::dumpState()
{
    #ifdef USE_DEFAULT_NEW_DELETE
        cout << "Using standard new & delete operator. Dump not available" << endl;
        return ;
    #endif

    cout << "Objects memory ( " << sizeof(DArray) << " bytes per DArray object)" << endl;
    for ( uint i = 0; i < objectBitmap.size(); i++ )
    {
        if ( objectBitmap[i] )
            cout << i << ": FREE" << endl;
        else
        {
            DArray* a = (DArray*)&objectMemory[i*sizeof(DArray)];
            cout << i << ": Occupied ( pointing to " << a->bytes << " of static data)" << endl;
        }
    }
    cout << "Data memory: " << endl;
    for ( uint i = 0; i < bitmap.size(); i++ )
    {
        cout << "Cluster " << i << (bitmap[i] ? " (FREE)" : " (OCCUPIED)") << ": ";
        cout.write(memory+ClusterSize*i, ClusterSize);
        cout << endl;
    }
}

void* DArray::operator new(size_t size_) throw(std::bad_alloc)
{
    #ifdef USE_DEFAULT_NEW_DELETE
        return ::new char[size_];
    #endif

    if ( !memSet )
        panic("DArray", "new", "setMemory must be called before any DArray object creation");

    uint size = (uint) objectBitmap.size();
    for ( uint i = 0; i < size; i++ )
    {
        if ( objectBitmap[i] )
        {
            objectBitmap[i] = false; // mark as used
            return (void *)&objectMemory[i*sizeof(DArray)];
        }
    }
    panic("DArray", "new", "Memory for DArray objects exhaused, no more DArrays can be created");
    throw bad_alloc();
    return 0;
}

void DArray::operator delete(void* ptr)
{
    #ifdef USE_DEFAULT_NEW_DELETE
        ::delete []ptr;
        return;
    #endif

    // fast deallocation
    uint id = ((unsigned long) ptr - (unsigned long) objectMemory)/sizeof(DArray);
    if ( id >= objectBitmap.size() )
        panic("DArray", "delete", "trying to deleting a pointer not obtained using new");

    objectBitmap[id] = true; // free
}

void DArray::clear()
{
    #ifdef USE_DEFAULT_NEW_DELETE
        ::delete[] (char *) ptr;
        bytes = 0;
        ptr = 0;
        return ;
    #endif

    if ( bytes != 0 )
        freeMyClusters();
}

bool DArray::empty() const
{
    return (bytes == 0);
}

void DArray::set(size_t bytes_)
{
    #ifdef USE_DEFAULT_NEW_DELETE
        if ( bytes != 0 )
            ::delete[] ptr;
        bytes = bytes_;
        ptr = new char[bytes];
        return ;
    #endif

    if ( bytes != 0 )
        freeMyClusters();

    bytes = bytes_;
    
    uint howMany = (uint) bytes / ClusterSize;
    if ( bytes % ClusterSize != 0 )
        howMany++;

    uint start = findFreeClusters(howMany); // mark returned clusters as used
    if ( start == ClusterNotFound )
        panic("DArray", "set", "Data Memory exhausted!");
    
    ptr = (void *) &memory[start*ClusterSize];
}

void DArray::set(const void* ptr_, size_t bytes_)
{
    #ifdef USE_DEFAULT_NEW_DELETE

        if ( bytes != 0 )
            ::delete[] ptr;

        bytes = bytes_;
        ptr = new char[bytes];
        memcpy(ptr, ptr_, bytes);
        return ;
    #endif

    if ( bytes != 0 )
        freeMyClusters();

    bytes = bytes_;

    uint howMany = (uint) bytes / ClusterSize;
    if ( bytes % ClusterSize != 0 )
        howMany++;

    uint start = findFreeClusters(howMany); // mark returned clusters as used
    if ( start == ClusterNotFound )
        panic("DArray", "set", "Data Memory exhausted!");
    
    ptr = (void *) &memory[start*ClusterSize];
    memcpy(ptr, ptr_, bytes);
}

void DArray::dump() const
{
    cout << "Bytes: " << bytes;
    if ( bytes == 0 )
        cout << " - No data" << endl;
    else
    {
        cout << " - Contents: ";
        cout.write((const char *)ptr, (uint) bytes);
    }
}

void DArray::freeMyClusters()
{
    // Deallocate
    if ( bytes == 0 )
        return ;
    uint howMany = (uint) bytes / ClusterSize;
    if ( bytes % ClusterSize != 0 )
        howMany++;
#ifndef WIN32    
    uint start = ((unsigned long)ptr - (unsigned long)memory) / ClusterSize;    
#else
    uint start = ((uint) ptr - (uint) memory) / ClusterSize;
#endif
    freeClusters(start, howMany);
    bytes = 0;
}

DArray::DArray() : ptr(0), bytes(0)
{
    darrayCounting++;
}

DArray::DArray(const void* ptr_, size_t bytes_)
{
    #ifdef USE_DEFAULT_NEW_DELETE
        bytes = bytes_;
        ptr = new char[bytes];
        memcpy(ptr, ptr_, bytes);
        return ;
    #endif

    darrayCounting++;

    if ( !memSet )
        panic("DArray", "ctor", "setMemory must be called before allocating any DArray object");

    bytes = bytes_;

    uint howMany = (uint) bytes / ClusterSize;
    if ( bytes % ClusterSize != 0 )
        howMany++;

    cout << "Clusters required: " << howMany << endl;

    uint start = findFreeClusters(howMany);
    if ( start == ClusterNotFound )
    {
        panic("DArray", "ctor", "bad_alloc (Memory exhausted)");
        throw bad_alloc();
    }
    
    ptr = (void *) &memory[start*ClusterSize];
    memcpy(ptr, ptr_, bytes);
}

DArray::~DArray()
{
    darrayCounting--;

    #ifdef USE_DEFAULT_NEW_DELETE
        delete [] (char *)ptr; // ptr == 0 does not hurt
        return ;
    #endif

    freeMyClusters();

}

void DArray::freeClusters(uint start, uint howMany)
{

    if ( start + howMany >= bitmap.size() )
    {
        stringstream ss;
        ss << "start(" << start << ") + howMany(" << howMany << ") = " << start + howMany << " >= bitmap.size() (" << 
            bitmap.size() << ")";
        popup("DArray::freeClusters", ss.str().c_str());
    }

    for ( uint i = 0; i < howMany; i++ )
        bitmap[start++] = true;
}


DArray::uint DArray::findFreeClusters(uint howMany)
{
    uint bitmapSz = (uint) bitmap.size();
    uint consecutive = 0;
    uint start = 0;
    for ( uint i = 0; i < bitmapSz && consecutive < howMany; i++ )
    {
        if ( bitmap[i] )
            consecutive++;
        else
        {
            consecutive = 0;
            start = i + 1;
        }
    }

    if ( consecutive != howMany )
        return ClusterNotFound;

    // Mark clusters as used
    
    uint j = start;
    for ( uint i = 0; i < consecutive; i++ )
        bitmap[j++] = false;

    return start;
}


DArray::operator char*() const
{
    return (char *)ptr;
}


DArray::operator const char*() const
{
    return (const char *)ptr;
}

DArray::operator unsigned char*() const
{
    return (unsigned char *)ptr;
}


DArray::operator const unsigned char*() const
{
    return (const unsigned char *)ptr;
}


char& DArray::operator[](int subscript)
{
    return (((char*)ptr)[subscript]);
}
char& DArray::operator[](unsigned int subscript)
{
    return (((char*)ptr)[subscript]);
}

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

#ifndef DARRAY_H
    #define DARRAY_H

#include <cstddef>
#include <new>
#include <vector>

class DArray
{
private:

    typedef unsigned int uint;

    enum { ClusterNotFound = 0xFFFFFFFF };

    /**
     * 8 state bytes per DArray
     */
    size_t bytes;
    void* ptr;

    static bool memSet;

    static char* objectMemory; // memory for objects (darrays)
    static std::vector<bool> objectBitmap;

    static char* memory; // memory for data pointed by darrays
    static std::vector<bool> bitmap; // false means occupied, true means free

    static uint ClusterSize;

    static uint findFreeClusters(uint howMany);
    static void freeClusters(uint start, uint howMany);
    static uint darrayCounting;

    void freeMyClusters();

public:

    static void init(size_t maxDarrays, size_t memoryAmount, size_t clusterSize = 1024);

    static void dumpState();

    // Message showing memory usage for DArray objects
    static void coreDump();

    DArray();
    DArray(size_t bytes);
    DArray(const void* ptr, size_t bytes);
    void clear();
    void set(const void* ptr, size_t bytes);
    void set(size_t bytes);
    bool empty() const;
    void dump() const;
    
    ~DArray();

    void* operator new(size_t size) throw(std::bad_alloc);
    void operator delete(void* ptr);

    operator const char*() const;
    operator char*() const;
    operator const unsigned char*() const;
    operator unsigned char*() const;

    char& operator[](int subscript);
    char& operator[](unsigned int subscript);

    size_t size() const { return bytes; }
};

#endif // DARRAY_H


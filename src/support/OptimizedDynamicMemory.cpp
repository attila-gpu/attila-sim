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
#include "OptimizedDynamicMemory.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ostream>
#include <iostream>
#include "support.h"
#include <fstream>
#include <map>
#include <string>
#include <sstream>
#include <list>
#include <typeinfo>

using namespace std;
using namespace gpu3d;

#ifndef GPU_LOG2
    #define GPU_LOG2(x) (log(f64bit(x))/log(2.0))
#endif

#ifndef GPU_CEIL
   #define GPU_CEIL(x) ceil(x)
#endif

#define FAST_NEW_DELETE

OptimizedDynamicMemory::Bucket OptimizedDynamicMemory::bucket[NUM_BUCKETS]; //  Allocate the bucket structures.

bool OptimizedDynamicMemory::wasCalled = false;             // one initialize call allowed only
u32bit OptimizedDynamicMemory::freqSize[24];
u64bit OptimizedDynamicMemory::timeStamp = 0;

using namespace std;
using namespace gpu3d;

void OptimizedDynamicMemory::Bucket::init(u32bit maxObjectSize, u32bit maxObjects)
{
    _maxObjects = maxObjects;
    _chunkSzLg2 = (u32bit) GPU_CEIL(GPU_LOG2(maxObjectSize));
    _chunkSize = 1 << _chunkSzLg2;
    maxUsage = 0;
    _memSize = _chunkSize * _maxObjects;

    mem = (char*)malloc( _maxObjects * _chunkSize );
    
    GPU_ASSERT(
        if ( mem == 0 )
            panic("OptimizedDynamicMemory", "Bucket::init", "Error allocating dynamic memory.");
    )

    //  Allocate memory for the free dynamic memory list.
    map = (u32bit*)malloc( _maxObjects * sizeof(u32bit) );

    //  Check free list allocation.
    GPU_ASSERT(
        if ( map == 0 )
            panic("OptimizeDynamicMemory", "Bucket::init", "Error allocating dynamic memory free list.");
    )

    // Allocate usage of each chunk of dynamic memory.
    sizes = (u32bit*)malloc( _maxObjects * sizeof(u32bit) );

    // Check usage of each chunk map allocation. 
    GPU_ASSERT(
        if (sizes == NULL)
            panic("OptimizedDynamicMemory", "Bucket::init", "Error allocating used dynamic memory map.");
    )

    // Clear dynamic memory.
    memset(mem, 0, _memSize);

    //  Initialize the free list and the used memory map.
    for ( u32bit j = 0; j < _maxObjects; j++ ) {
        map[j] = j; // all maps available
        sizes[j] = 0; // no ocupation in any chunk
    }

    /*  Set free list first free pointer to 0.  */
    nextFree = 0; // setting first free chunk


    // precompute max address
    _maxMemAddress = mem + _memSize; 
}


// { P: chunkSize is power of two && realObject is always lower than chunkSize }

void OptimizedDynamicMemory::initialize( u32bit maxObjectSize1, u32bit capacity1, u32bit maxObjectSize2, u32bit capacity2,
    u32bit maxObjectSize3, u32bit capacity3 )
{
    // integrity test
    GPU_ASSERT(
        if ( capacity1 <= 0 || maxObjectSize1 <= 0 || capacity2 <= 0 || maxObjectSize2 <= 0 ||
             capacity3 <= 0 || maxObjectSize3 <= 0)
            panic("OptimizedDynamicMemory", "initialize", "Object size and object capacity can not be 0.");
        if ( wasCalled )
            panic("OptimizedDynamicMemory", "initialize", "Dynamic memory system already initialized.");
    )

#ifdef AM_FAST_NEW_DELETE
        if (maxObjectSize1 >= maxObjectSize2)
            panic("OptimizedDynamicMemory", "initialize", "Object sizes for the buckets must be ordered for fast new/delete.");
#endif

    wasCalled = true;

    bucket[0].init(maxObjectSize1, capacity1);
    bucket[1].init(maxObjectSize2, capacity2);
    bucket[2].init(maxObjectSize3, capacity3);

#ifdef FAST_NEW_DELETE 
        cout << "OptimizedDynamicMemory => FAST_NEW_DELETE enabled.  Ignoring third bucket!" << endl;
#endif

    for(u32bit i = 0; i < 24; i++)
        freqSize[i] = 0;
}

void* OptimizedDynamicMemory::operator new( size_t objectSize ) throw()
{

#ifdef FAST_NEW_DELETE

    u32bit b;
    u32bit *p;

    /*  Check the requested object size against the chunk ranges.  */
    b = ((objectSize + 16) <= bucket[0].ChunkSize())?0:((objectSize + 16) <= bucket[1].ChunkSize())?1:2;

    if ((b == 2) || (bucket[b].nextFree == bucket[b].MaxObjects()))
    {
        if ( b == 2 ) 
            cout << "(b == 2)\n";

        if (bucket[b].nextFree == bucket[b].MaxObjects())
            cout << "bucket[b].nextFree == bucket[b].MaxObjects()\n";

        cout << "Bucket = " << b << " nextFree = " << bucket[b].nextFree << " MaxObjects() = " << bucket[b].MaxObjects() << "\n";
        panic("OptimizedDynamicMemory", "new", "Error allocating object.");
    }

    p = (u32bit *) &bucket[b].mem[bucket[b].map[bucket[b].nextFree] << bucket[b].ChunkSzLg2()];

    p[0] = b;
    p[1] = bucket[b].map[bucket[b].nextFree];
    //p[2] = 0xCCCCDDDD;

    //printf("p %p b %d pos %d\n", p, b, bucket[b].map[bucket[b].nextFree]);

    bucket[b].nextFree++;
    return &p[4];

#else // !FAST_NEW_DELETE
    
    u32bit b;
    u32bit currentSize;
    u32bit i;

    GPU_ASSERT(
        if ( !wasCalled )
            panic("OptimizedDynamicMemory", "new", "Initialization must be done before any allocation occurs.");
        if (( objectSize > bucket[0].ChunkSize() ) && ( objectSize > bucket[1].ChunkSize() ) && ( objectSize > bucket[2].ChunkSize() ))
        {
            printf("objectSize %d\n", objectSize);
            panic("OptimizedDynamicMemory", "new", "Object larger than maximum dynamic object size.");
        }
    )

    /*  Select the smaller bucket required for the object.  */
    for(i = 0, currentSize = u32bit(-1); i < NUM_BUCKETS; i++)
    {
        /*  Check if the object can be stored in the bucket and the chunk size is the smaller selected until now.  */
        if (((objectSize + 16) <= bucket[i].ChunkSize()) && (currentSize > bucket[i].ChunkSize()))
        {
            b = i;
            currentSize = bucket[i].ChunkSize();
        }
    }

    // param ignored, size is fixed in initialization
    // find available map
    if ( bucket[b].nextFree == bucket[b].MaxObjects() )
    {
        stringstream ss;
        ss << "No free objects available in bucket " << b << " (numObjects " << bucket[b].MaxObjects()
           << " ChunkSize() " << bucket[b].ChunkSize() << ")";
        panic("OptimizedDynamicMemory", "new", ss.str().c_str());            
    }

    freqSize[static_cast<u32bit>(ceil(GPU_LOG2(objectSize)))]++;

    if ((bucket[b].nextFree + 1) > bucket[b].maxUsage)
        bucket[b].maxUsage = bucket[b].nextFree + 1;

    bucket[b].sizes[bucket[b].map[bucket[b].nextFree]] = static_cast<u32bit>(objectSize); // set used memory map entry with this object size

    timeStamp++;

    return &bucket[b].mem[bucket[b].map[bucket[b].nextFree++]*bucket[b].ChunkSize()]; // fast allocation ( product is a shift )

#endif // FAST_NEW_DELETE  
}

void OptimizedDynamicMemory::operator delete ( void* obj )
{
    if ( !obj ) // Standard behaviour
        return ;
#ifdef FAST_NEW_DELETE

    u32bit b;
    u32bit idx;

    //if (*(((u32bit*) obj) - 2) != 0xCCCCDDDD)
    //    panic("OptimizedDynamicMemory", "delete", "Error in delete.");
    //printf("obj %p b %d pos %d\n", obj, b, idx);

    b = *(((u32bit*) obj) - 4);
    idx = *(((u32bit*) obj) - 3);

    Bucket& bu = bucket[b]; // alias to avoid multiple computations of bucket[b]
    bu.map[--bu.nextFree] = idx; // release map, new available chunk

#else // !FAST_NEW_DELETE

    u32bit b;

    //  Check OptimizedDynamicMemory initialization. 
    GPU_ASSERT(
        if ( !wasCalled )
            panic("OptimizedDynamicMemory", "delete", "Initialization must be done before any de-allocation occurs.");
    )

    // Check null object.  
    GPU_ASSERT(
        if ( !obj ) // delete of null pointer
            panic("OptimizedDynamicMemory", "delete", "Trying to delete a null pointer.");
    )

    // Convenient cast to clarify code
    const char* const objptr = reinterpret_cast<const char* const>(obj);

    // Select bucket for the object to delete.
    if ( objptr >= bucket[0].mem && objptr <= bucket[0].MaxMemAddress() )
        b = 0;
    else if ( objptr >= bucket[1].mem && objptr <= bucket[1].MaxMemAddress() )
        b = 1;
    else if ( objptr >= bucket[2].mem && objptr <= bucket[2].MaxMemAddress() )
        b = 2;
    else
        panic("OptimizedDynamicMemory", "delete", "Object address outside dynamic memory range.");

    // Assume it was created in the pool of dynamic memory
    // calculate which block number is
    u32bit iMap = static_cast<u32bit>(objptr - bucket[b].mem);

    GPU_ASSERT(
        if ( iMap % bucket[b].ChunkSize() != 0 )
            panic("OptimizedDynamicMemory","delete", "Object address not correctly aligned.");
    )

    // if needed in DEBUG mode, test if the obj owns to the heap
    iMap /= bucket[b].ChunkSize(); // ( div is a shift )

    //  Check that the object chunk hasn't been already deleted. 
    GPU_ASSERT(
        if(bucket[b].sizes[iMap] == 0)
        {
            printf("Block %d address %x allocated %d bytes tag %s timeStamp %llx\n", iMap,
                    reinterpret_cast<u64bit>(bucket[b].mem + iMap*bucket[b].ChunkSize()),
                    bucket[b].sizes[iMap],
                    &(((char *) (bucket[b].mem + iMap * bucket[b].ChunkSize()))[bucket[b].ChunkSize() - 16]),
                    ((u64bit *) (bucket[b].mem + iMap * bucket[b].ChunkSize()))[(bucket[b].ChunkSize() >> 3) - 1]
                    );

            panic("OptimizedDynamicMemory", "delete", "Deleting an already deleted chunk.\n");
        }

        if (bucket[b].sizes[iMap] > bucket[b].ChunkSize())
        {
            panic("OptimizedDynamicMemory", "delete", "Deleting a chunk from the wrong bucket.\n");
        }
    )

    bucket[b].nextFree--;

    bucket[b].sizes[iMap] = 0; // free chunk, no consumed space then

    bucket[b].map[bucket[b].nextFree] = iMap; // release map, new available chunk

#endif // FAST_NEW_DELETE
    
}


void OptimizedDynamicMemory::setTag(char *tag)
{
#ifdef FAST_NEW_DELETE
        return ;
#endif

    u32bit b;

    const char* const objptr = reinterpret_cast<const char* const>(this);

    /*  Select bucket for the object to delete.  */
    if (objptr >= bucket[0].mem && objptr <= bucket[0].MaxMemAddress())
    {
        b = 0;
    }
    if (objptr >= bucket[1].mem && objptr <= bucket[1].MaxMemAddress())
    {
        b = 1;
    }
    if (objptr >= bucket[2].mem && objptr <= bucket[2].MaxMemAddress())
    {
        b = 2;
    }
    else
    {
return;
        panic("OptimizedDynamicMemory", "setTag", "Object address outside dynamic memory range.");
    }

    ((u64bit *) this)[(bucket[b].ChunkSize() >> 3) - 2] = *((u64bit *) tag);
    ((u64bit *) this)[(bucket[b].ChunkSize() >> 3) - 1] = timeStamp;
}


std::string OptimizedDynamicMemory::getClass() const
{
    return string(typeid(*this).name());
}

string OptimizedDynamicMemory::toString() const
{
    return getClass();
}

void OptimizedDynamicMemory::dumpDynamicMemoryState( bool dumpMemoryContentsToo, bool cooked )
{
    for(u32bit i = 0; i < NUM_BUCKETS; i++)
    {
        cout << "Dump mem Statistics...  " << endl;
        cout << "True size ocuppied by an object ( ChunkSize() ): " << bucket[i].ChunkSize() << " bytes" << endl;
        cout << "Max number of objects allowed DynMem: " << bucket[i].MaxObjects() << endl;
        cout << "Objects allocated in DynMem (outstanding deletes): " << bucket[i].nextFree << endl;
        cout << "Available storage (in objects): " << bucket[i].MaxObjects() - bucket[i].nextFree << endl;
        cout << "Available storage (in bytes): " << bucket[i].ChunkSize()*( bucket[i].MaxObjects() - bucket[i].nextFree ) <<
            " bytes" << endl;

        cout << "Mapping representation ( free blocks ): " << endl;
        if ( bucket[i].nextFree == bucket[i].MaxObjects() )
            cout << "No free maps available..." << endl;
        else {
            for ( u32bit j = bucket[i].nextFree; j < bucket[i].MaxObjects(); j++ ) {
                if ( j == bucket[i].nextFree )
                    cout << "i: " << j << "   chunkIndex: " << bucket[i].map[j] << "  <-- nextFree" << endl;
                else
                    cout << "i: " << j << "   chunkIndex: " << bucket[i].map[j] << endl;
            }
        }

        printf("Allocated blocks.  Last time stamp: %llx.\n", timeStamp);

        /*  Print used blocks.  */
        for(u32bit j = 0; j < bucket[i].MaxObjects(); j++)
        {
            if (bucket[i].sizes[j] != 0)
                printf("Block %d address %p allocated %d bytes tag %s timeStamp %llx\n", j,
                    bucket[i].mem+j*bucket[i].ChunkSize(),
                    bucket[i].sizes[j],
                    &(((char *) (bucket[i].mem + j * bucket[i].ChunkSize()))[bucket[i].ChunkSize() - 16]),
                    ((u64bit *) (bucket[i].mem + j * bucket[i].ChunkSize()))[(bucket[i].ChunkSize() >> 3) - 1]
                    );
        }
    }

printf("Frequencies:\n");
for(u32bit i = 0; i < 24; i++)
printf("Size %d -> %d\n", u32bit(pow(2.0, double(i))), freqSize[i]);

    if ( !dumpMemoryContentsToo )
        return ;

    u32bit inc = sizeof( char );
    if ( cooked ) // cooked means grouping 4 bytes in an u32bit
        inc = sizeof( u32bit );

    for (u32bit i = 0; i < NUM_BUCKETS; i++)
    {
        for ( u32bit j = 0; j < bucket[i].ChunkSize()*bucket[i].MaxObjects(); j+=inc ) {
            if ( j % bucket[i].ChunkSize() == 0 ) {
                if ( isOcupied(i, j/bucket[i].ChunkSize() ) )
                    cout << "--- Reserved block ( real ocupation: " << bucket[i].sizes[j/bucket[i].ChunkSize()]
                        << " bytes ) ------------------------" << endl;
                else
                    cout << "--- Free block -------------------------------------------------------" << endl;
            }
            printf( "%04i: ", j );
            if ( cooked )
                cout << *(u32bit*)(bucket[i].mem+j)  << endl;
            else
                cout << bucket[i].mem[i] << endl;
        }
    }
}

void OptimizedDynamicMemory::usage()
{
#ifdef FAST_NEW_DELETE
    printf("Bucket 0: Size %d Last %d Max %d | Bucket 1: Size %d Last %d Max %d | Bucket 2: Size %d Last %d Max %d\n",
            bucket[0].MaxObjects(), bucket[0].nextFree, bucket[0].maxUsage,
            bucket[1].MaxObjects(), bucket[1].nextFree, bucket[1].maxUsage,
            bucket[2].MaxObjects(), bucket[2].nextFree, bucket[2].maxUsage
          );
#else // !FAST_NEW_DELETE
    cout << "\n-----------------------------------------------------------------\n";
    cout << "------------------ OPTIMIZED DYNAMIC MEMORY USAGE ---------------\n";
    cout << "-----------------------------------------------------------------\n";
    for ( u32bit i = 0; i < NUM_BUCKETS; ++i ) {
        const Bucket& b = bucket[i];
        cout << "Bucket " << i << ": Size " << b.MaxObjects() << " Last " << b.nextFree << " Max " << b.maxUsage 
            << " (object size "  <<  b.ChunkSize() <<  " bytes)\n";
    }
    cout << "-----------------------------------------------------------------\n";
    for ( u32bit i = 0; i < NUM_BUCKETS; ++i ) {
        map<string,u32bit> mapCounts;
        cout << "Objects in BUCKET " << i << " -> ALLOCATIONS" << endl;
        u32bit accum = 0;
        for(u32bit j = 0; j < bucket[i].MaxObjects(); ++j) 
        {
            if ( bucket[i].sizes[j] != 0 ) {
                ++accum;
                char* address = bucket[i].mem+j*bucket[i].ChunkSize();
                OptimizedDynamicMemory* dynObj = reinterpret_cast<OptimizedDynamicMemory*>(address);
                string className = typeid(*dynObj).name();
                map<string,u32bit>::iterator it = mapCounts.find(className);
                if ( it == mapCounts.end() )
                    mapCounts.insert(make_pair(className,0));
                ++mapCounts[className];
            }
        }
        list<pair<u32bit,string> > pendingList;
        for ( map<string,u32bit>::iterator it = mapCounts.begin(); it != mapCounts.end(); ++it ) {
            pendingList.push_back(make_pair(it->second, it->first));
            // cout << "  " << it->first << " -> " << it->second << "\n";
        }
        pendingList.sort(); // Sort by number of allocatations
        for ( list<pair<u32bit,string> >::iterator it = pendingList.begin(); it != pendingList.end(); ++it ) {
            cout << "  " << it->second << " -> " << it->first << "\n";
        }

        cout << "  TOTAL -> " << accum << " Allocated objects in BUCKET " << i << endl;
    }
    cout << "-----------------------------------------------------------------\n";
#endif // FAST_NEW_DELETE
}

bool OptimizedDynamicMemory::isOcupied( u32bit b, u32bit chunk )
{
    for ( u32bit i = bucket[b].nextFree; i < bucket[b].MaxObjects(); i++ ) {
        if ( bucket[b].map[i] == chunk )
            return false;
    }
    return true;
}

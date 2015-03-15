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
#ifndef _OPTIMIZED_DYNAMIC_MEMORY_
   #define _OPTIMIZED_DYNAMIC_MEMORY_

#include "GPUTypes.h"
#include <cstddef> // size_t definition
#include <new> // bad_alloc definition
#include <string>

namespace gpu3d
{

/**
 * @b Optimized class is a fast controler of dynamic memory.
 *
 * @b files: OptimizedDynamicMemory.h OptimizedDynamicMemory.cpp
 *
 * OptimizedDynamicMemory class :
 * - Implements a memory manager for fast allocation and deallocation ( overloads new and delete operators )
 * - to use this feature you must inherit from this class. All methods are static.
 *
 * @note It is MANDATORY to call initialize method before using new and delete operators of wichever class
 *       devired from OptimizedDynamicMemory.
 *
 * Example of use:
 *
 *    @code
 *       // suppose VSInstruction is a derived class of OptimizedDynamicMemory
 *
 *       // Capacity for 1000 derived objects of OptimizedDynamicMemory, max size of every object 256 bytes
 *       // we assume sizeof( VSInstruction ) is equal or lower than 256
 *       OptimizedDynamicMemory::initialize( 256, 1000 );
 *       // now you can use new and delete as ever
 *
 *       // using OptimizedDynamicMemory new operator
 *       // if VSInstruction size were greater than 256 a bad_alloc() exception would be thrown
 *       VSInstruction* vsi1 = new VSInstruction( ... );
 *
 *       // dumps memory including contents in raw mode
 *       VSInstruction::dumpDynamicMemoryState( true );
 *
 *       delete vsi1; // using OptimizedDynamicMemory delete operator
 *    @endcode
 *
 * @version 1.1
 * @date 05/02/2003 ( previous )23/01/2003
 * @author Carlos González Rodríguez - cgonzale@ac.upc.es
 */
class OptimizedDynamicMemory {

private:


    /**
     *
     *  Defines an optimized memory bucket.
     *
     */
    struct Bucket
    {
    private:

        u32bit _maxObjects;  ///< maximum number of allocations allowed
        u32bit _chunkSize;   ///< size of a chunk of memory ( is a power of 2 )
        u32bit _chunkSzLg2;  /**<  Log 2 of the size of the chunk.  */
        char* _maxMemAddress;
        u32bit _memSize;

    public:

        char* mem;          ///< mem available ( raw memory )
        u32bit* sizes;      /// tracing of sizes ( real ocupation within a chunk )
        u32bit* map;        ///< fifo with the available identifiers of free chunks
        u32bit nextFree;    ///< next block free ( number of current maps also )
        u32bit maxUsage;    /**<  Stores the largest number of chunks used. */

        void init(u32bit maxObjectSize, u32bit maxObjects);

        // These methos are as fast as accessing the contents directly, they are provided to
        // expose some bucket attributes as constants
        inline u32bit MaxObjects() const { return _maxObjects; }
        inline u32bit ChunkSize() const { return _chunkSize; }
        inline u32bit ChunkSzLg2() const { return _chunkSzLg2; }
        inline const char* const MaxMemAddress() const { return _maxMemAddress; }
        inline u32bit MemSize() const { return _memSize; }

    };

    /**
     *  Defines the number of available memory buckets.
     *
     */
    static const u32bit NUM_BUCKETS = 3;

    static Bucket bucket[NUM_BUCKETS];  /**<  Buckets of fixed sized blocks for memory allocation.  */

    static bool wasCalled;      ///< controls that only one call to initialize is performed in the life of the class
    static u64bit timeStamp;    /**<  Timestamp counter.  */
    static u32bit freqSize[];   /**<  Stores how frequent are different object sizes (power of 2).  */


    /**
     * Check if a chunk is already ocupied ( used for print memory status )
     *
     * @param bucket to be tested
     * @param chunk chunk to be tested
     * @return true if chunk is ocupied, false is chunk is available ( free )
     */
    static bool isOcupied( u32bit bucket, u32bit chunk );

    OptimizedDynamicMemory( const OptimizedDynamicMemory& );

protected:

    void setTag(char *tag);

    // Inherit classes call it implicitly, so It can not be private
    // It has an empty definition
    OptimizedDynamicMemory() {}

public:

    /**
     * Converts a class into its name based on RTTI information kept by C++
     */
    virtual std::string toString() const;

    /**
     * Obtains a string representation of the name of the class object
     */
    std::string getClass() const;

    /**
     * Mandatory ( only once at the beginning )
     *
     *
     * @param maxObjectSize1 maximum size of wichever object who inherits from this class (for first bucket)
     * @param capacity1 maximum capacity of objects that can be allocated at the same time (for first bucket)
     * @param maxObjectSize2 maximum size of wichever object who inherits from this class (for second bucket)
     * @param capacity2 maximum capacity of objects that can be allocated at the same time (for second bucket)
     * @param maxObjectSize3 maximum size of wichever object who inherits from this class (for third bucket)
     * @param capacity3 maximum capacity of objects that can be allocated at the same time (for third bucket)
     *
     * @return true if all goes well, false otherwise ( a second call or any param is equal or lower than 0 )
     *
     * @note if maxObjectSize is not power of 2 then maxObjectSize is set as the lower power of two greater
     * than maxObjectSize
     */
    static void initialize( u32bit maxObjectSize1, u32bit capacity1, u32bit maxObjectSize2, u32bit capacity2,
        u32bit maxObjectSize3, u32bit capacity3 );

    /**
     * Called by the compiler when new operator is used, size_t param is discarded
     *
     * @param size discarded in our implementation since size is fixed at the beginning
     */
    void* operator new( size_t size) throw();

    /**
     * Called by the compiler when delete operator is performed
     *
     * @param obj passed by the compiler
     */
    void operator delete( void *obj );

    /**
     * Dumps debug information about the usage of dynamic memory
     *
     * @param dumpMemoryContentsToo contents of memory are dumpped
     * @param cooked if dumpMemoryContentsToo is true it is possible to specify a cooked or raw mode for dumping
     *
     * @note formated printing of memory contents is implemented packing 4 bytes in a u32bit
     */
    static void dumpDynamicMemoryState( bool dumpMemoryContentsToo = false, bool cooked = false );

    /**
     *
     *  Dumps usage information about the two dynamic memory buckets.
     *
     */

    static void usage();

    // static void printNotDeletedObjects();

};

} // namespace gpu3d

#endif

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
 * $RCSfile: GPUSignal.h,v $
 * $Revision: 1.2 $
 * $Author: vmoya $
 * $Date: 2006-12-12 17:41:13 $
 *
 * Signal class definition file.
 *
 */

#ifndef __SIGNAL__
    #define __SIGNAL__

#include "GPUTypes.h"
#include "QuadFloat.h"
#include "DynamicObject.h"
#include <cstring>
#include <ostream>
#include <cstdio>

namespace gpu3d
{

/**
 * @b Signal class implements the Signal concept
 *
 * @b Files:  Signal.h, Signal.cpp
 *
 * - Implements the Signals concept. Only pointer are used ( fast )
 * - Limitations:
 *    - Latency 0 in writes NOT SUPPORTED
 *
 * - About implementation:
 *    - FIFO - Circular queue. Support arbitrary ordering in
 *      the same cycle for read and writes.
 *      - read(CYCLE,data1), write(CYCLE,data2)
 *      - write(CYCLE,data2),read(CYCLE,data1)
 *      - Both sequences has exact behaviour
 *    - Supports variable latency ( latency must be equal or less than maxLatency )
 *    - Latency 0 unsupported
 *    - Supports pre-initialization ( since version 1.1 )
 *
 * @version 1.1
 * @date 07/02/2003 ( previous 28/11/2002 )
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 *
 */
class Signal {


    static const u32bit CAPACITY = 512;
    static const u32bit CAPACITY_MASK = CAPACITY - 1;

private:

    char* name; ///< Name identifier for the signal
    u32bit bandwidth; ///< Bandwidth allowed ( writes per cycle ), 0 implies undefined
    u32bit maxLatency;   ///< Signal's latency ( 0 implies undefined )

    u32bit capacity;        ///< Precalculated: 2^(ceil(log2(maxLatency + 1)))
    u32bit capacityMask;    ///< Bitmask used for the capacity module.

    // char*** data; ///< Matrix data
    DynamicObject*** data; ///< Matrix data

    u32bit nWrites; ///< Number of writes in actual cycle
    u32bit *nReads; ///< Reads that must be done in every cycle to avoid data losses
    u32bit readsDone; ///< reads in actual cycle

    u64bit lastCycle; ///< Last cycle with a read or write operation
    u64bit lastRead; ///< Last cycle with a read operation
    u64bit lastWrite; ///< Last cycle with a write operation

    u32bit nextRead;
    u32bit nextWrite;
    u32bit pendentReads;

    /**
     * Position in the matrix where we are going to write or read.
     *
     * Calculated in every read/write operation
     */
    u32bit in;

    std::string strFormatObject(const DynamicObject* dynObj);

    /**
     * Check data losses in signal ( only used if GPU_ERROR_CHECK is enabled )
     *
     * @param cycle current cycle
     * @param index1 first position with valid data
     * @param index2 last position with valid data
     *
     * @return true if there are not data loss, false otherwise
     */
    bool checkReadsMissed( u64bit cycle, u32bit index1, u32bit index2 );


    /**
     * Called for constructor and setParameters to create necesary dynamic structures
     *
     * Dynamic memory allocation for atribute 'name' is not responsibility of this method
     * NO initialization performed for not-dynamic structures and atributes
     * @warning maxLatency and bandwidth must be defined before calling this method
     */
    void create();

    /**
     * Called by destructor and setParameters for release dynamic memory
     *
     * Dynamic memory deallocation for atribute 'name' is not responsibility of this method
     */
    void destroy();

    /**
     * Generic write
     *
     * Implements all the work, specific write is only an interface to avoid
     * explicit casting
     */
    // bool writeGen( u64bit cycle, char* dataW, u32bit latency );
    bool writeGen( u64bit cycle, DynamicObject* dataW, u32bit latency );

    /**
     * Generic write using maximum latency allowed
     *
     * Implements all the work, specific write is only an interface to avoid
     * explicit casting
     */
    //bool writeGen( u64bit cycle, char* dataW );
    bool writeGen( u64bit cycle, DynamicObject* dataW );

    /**
     * Generic read
     *
     * Implements all the work, specific read is only an interface to avoid
     * explicit casting
     */
    // bool readGen( u64bit cycle, char* &dataR );
    bool readGen( u64bit cycle, DynamicObject* &dataR );

    /**
     * Generic write
     *
     * Implements all the work, specific write is only an interface to avoid
     * explicit casting
     */
    // bool writeGenFast( u64bit cycle, char* dataW, u32bit latency );
    bool writeGenFast( u64bit cycle, DynamicObject* dataW, u32bit latency );

    /**
     * Generic write using maximum latency allowed
     *
     * Implements all the work, specific write is only an interface to avoid
     * explicit casting
     */
    // bool writeGenFast( u64bit cycle, char* dataW );
    bool writeGenFast( u64bit cycle, DynamicObject* dataW );

    /**
     * Generic read
     *
     * Implements all the work, specific read is only an interface to avoid
     * explicit casting
     */
    // bool readGenFast( u64bit cycle, char* &dataR );
    bool readGenFast( u64bit cycle, DynamicObject* &dataR );

    /**
     *
     *  Updates the signal clock.
     *
     *  @param cycle The next signal cycle.
     *
     */

    void clock(u64bit cycle);
    void clockFast(u64bit cycle);

public:


    /**
     * Creates a new Signal
     *
     * Creates a new Signal. If bandwidth or latency are not specified creates an
     * undefined signal
     *
     * @param name signal's name
     * @param bandwidth maximum signal's bandwidth supported
     * @param maxLatency maximum signal's latency
     */
    Signal( const char* name, u32bit bandwidth = 0, u32bit maxLatency = 0 );

    /// Destructor
    ~Signal();


    /**
     * Sets a default contents ( it only should be used at the beginning, before reading or writing in the Signal )
     *
     * @param initialData array of data used to fill the signal
     * @param firstCycleForReadOrWrite first cycle with data available ( defaults to 0 )
     *
     * @note setData is typically used without specifying firstCycleForReadOrWrite, 0 it is assumed.
     *
     * @code
     *    // example of use of setData(), general use ( specifying a firstCycleForReadOrWrite greater than 0 )
     *
     *    // initialContents + firstCycle represents this logical initialization for the signal:
     *    // <-----> maxLatency ( 3 )
     *    // |5|3|1|  |
     *    // |6|4|2|  | bandwidth ( 2 )
     *    //
     *    //  data 1 and 2 must be read in cycle 1 ( assumption: data written in cycle -1 in this case )
     *    //  data 3 and 4 must be read in cycle 2 ( assumption: data written in cycle 0 in this case )
     *    //  data 5 and 6 must be read in cycle 3 ( assumption: data written in cycle 1 in this case )
     *    //  The assumption of when data was written ( with maxLatency ) is deduced like this :
     *    //     ( firstCycleForReadOrWrite - maxLatency + 1 )
     *    //  Note that is possible "the assumption" of negative cycles, but this is supported.
     *    //  User of this method must not worry about that.
     *
     *    // Asumptions based on initial cycle specified ( 1 in this case )
     *    // 1,2 -> data available in cycle 1
     *    // 3,4 -> data available in cycle 2
     *    // 5,6 -> data available in cycle 3
     *    void* initialContents[6] = { (void*)1, (void*)2, (void*)3, (void*)4, (void*)5 ,(void*)6 };
     *
     *    Signal s( "fooSignal", 2, 3 ); // Signal with bandwith 2 and maxLatency 3
     *
     *    s.setData( initialContents, 1 ); // cycle 1 with data available ( 1 and 2 )
     *
     *    // now when can read from the signal
     *    s.read( 1, myData ); // OK !
     *
     * @endcode
     */
    bool setData( DynamicObject* initialData[], u64bit firstCycleForReadOrWrite = 0 );


    /**
     * Write interface for void pointers ( using max latency allowed )
     *
     * @param cycle cycle in which write is performed
     * @param dataW pointer to the data
     *
     * @return returns true if the write was successfully done, false otherwise
     */
    bool write( u64bit cycle, DynamicObject* dataW );

    /**
     * Write interface for void pointers
     *
     * @param cycle cycle in which write is performed
     * @param dataW pointer to the data
     * @param latency Latency for the data to be written
     *
     * @return returns true if the write was successfully done, false otherwise
     */
    bool write( u64bit cycle, DynamicObject* dataW, u32bit latency );

    /**
     * Read interface for void pointers
     *
     * @param cycle cycle in which read is performed
     * @param dataR a reference to a pointer to the Quadfloat read ( not a pointer but a pointer reference )
     *
     * @return returns true if the read was successfully done, false otherwise
     */
    bool read( u64bit cycle, DynamicObject* &dataR );

    /**
     * Obtains signal's name identifier
     *
     * @return signal's name
     */
    const char* getName() const;

    /**
     * Obtains max signal's bandwidth ( writes per cycle allowed )
     *
     * @return signal's bandwidth
     *
     * @note If signal's bandwidth is not yet defined returns 0
     */
    u32bit getBandwidth() const;

    /**
     * Obtains max signal's latency
     *
     * @return signal's latency
     *
     * @note If signal's maximum latency is not yet defined returns 0
     */
    u32bit getLatency() const;

    /**
     *
     *  Dumps to the trace file the signal trace for the cycle.
     *
     *  @param traceFile A pointer to the trace file output stream object.
     *  @param cycle The simulation cycle for which to dump
     *  the trace signal.
     *
     */

    void traceSignal(std::ostream *traceFile, u64bit cycle);
    //void traceSignal(gzofstream *traceFile, u64bit cycle);

    /// For debug purpose
    void dump() const;

    /**
     * Test if signal is fully defined
     *
     * Tests if signal has bandwidth and latency defined
     *
     * @return true if signal is fully defined, false otherwise
     *
     * @note If signal is not defined, all operations except setParameters are undefined
     */
    bool isSignalDefined() const;

    /**
     * Test if bandwidth is defined
     *
     * @return true if signal's bandwidth is WELL-DEFINED
     *
     * @note BANDWIDTH is WELL-DEFINED ( bandwidth > 0 )
     */
    bool isBandwidthDefined() const;

    /**
     * Test if latency is defined
     *
     * @return true if signal's latency is WELL-DEFINED
     *
     * @note LATENCY is WELL-DEFINED ( latency > 0 )
     */
    bool isLatencyDefined() const;


   /**
     * Sets new bandwidth and latency ( hard reset )
     *
     * @return true if SIGNAL is WELL-DEFINED, false ( 0 ) otherwise
     * @note SIGNAL WELL-DEFINED : latency > 0 && bandwidth > 0
     */
    bool setParameters( u32bit newBandwidth, u32bit newLatency );

    /**
     * Sets new bandwidth
     *
     * Sets new signal bandwidth ( hard reset )
     *
     * @return true if SIGNAL is WELL-DEFINED, false ( 0 ) otherwise
     */
    bool setBandwidth( u32bit newBandwidth );

    /**
     * Sets new signal latency
     *
     * Sets new latency's signal ( hard reset )
     *
     * @return true if signal is WELL-DEFINED, false ( 0 ) otherwise
     */
    bool setLatency( u32bit newLatency );

};

}

#endif

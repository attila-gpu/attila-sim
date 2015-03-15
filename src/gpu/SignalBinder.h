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
 * $RCSfile: SignalBinder.h,v $
 * $Revision: 1.8 $
 * $Author: vmoya $
 * $Date: 2006-12-12 17:41:13 $
 *
 * Signal Binder class definition file.
 *
 */

#ifndef __SIGNAL_BINDER__
   #define __SIGNAL_BINDER__

#include "GPUTypes.h"
#include "GPUSignal.h"
#include <cstdio>
#include <ostream>

namespace gpu3d
{

/**
 * @b SignalBinder class manages with Signal connections [SINGLETON]
 *
 * @b Files: SignalBinder.h, SignalBinder.cpp
 * - Description: Object reposible of all signal bindings and integrity of conections
 * - About implementation: Simple array of Signal pointers, with sequential access by name
 *
 * - Latest modifications:
 *    - No maximum capacity limitation ( automatic growth, since 1.1 )
 *
 *
 * @version 1.1
 * @date 2/12/2002 ( previous 25/11/2002 )
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 */
class SignalBinder {

private:

    enum { growth = 50 }; ///< Increase capacity when needed

    Signal** signals; ///< Signals registered
    flag* bindingState; ///< Binding control
    u32bit elements; ///< Count of signals registered
    u32bit capacity; ///< Max capacity allowed

    std::ostream *traceFile;    ///< Trace file handle.

    /// Aux method for finding positions in the binder
    s32bit find( const char* name ) const;

    /// SignalBinder can't be instanciated directly and can't be copied
    SignalBinder( u32bit capacity = growth );

    /// Copy constructor ( private avoid binder1 = binder2 )
    SignalBinder( const SignalBinder& );

    /// The unique SignalBinder object
    static SignalBinder binder;

    static const flag BIND_MODE_RW; ///< Signal successfully binded

public:

    static const flag BIND_MODE_READ; ///< Binding read mode
    static const flag BIND_MODE_WRITE; ///< Binding write mode

    /**
     * Gets a reference to the binder ( an alias )
     * You must get and alias before doing something else
     *
     * @return A SignalBinder reference to the global SignalBinder
     */
    static SignalBinder& getBinder();

    /**
     * Registers a name for a Signal
     * Type indicates if the signal is for reading or writing
     * Bandwidth and latency are registered only once, the first time they are specified.
     *
     * @param name Signal's name ( must be unique )
     * @param type kind of binding, possible values are { BIND_MODE_READ, BIND_MODE_WRITE )
     * @param bw bandwidth for this signal ( it can be left unspecified )
     * @param latency latency for this signal ( it can be left unspecified )
     *
     * @return A pointer to the Signal with name 'name', if the Signal did not exist, this methods creates one
     *         with the specified characteristics. If the signal exist previously 'bw' and  'latency' must
     *         match with previous values ( note: undefined values always match )
     */
    Signal* registerSignal( char* name, flag type, u32bit bw = 0, u32bit latency = 0 );

    /**
     * Obtains the signal with name 'name'
     *
     * @param name Obtains a pointer to the Signal identified with 'name'
     *
     * @return Signal's pointer, the Signal with name 'name'
     */
    Signal* getSignal( const char* name ) const;

    /**
     * Obtains maximum capacity allowed within binder without 'growing'
     *
     * @return maximum capacity supported before the binder grows
     */
    u32bit getCapacity() const;

    /**
     * Obtains how many Signals are inside the binder
     *
     * @return number of signal's registered in the binder
     */
    u32bit getElements() const;

    /**
     * Checks if all signals registered has 1 and only 1 registration for reading
     * and another for writing and latency and bandwidth are well defined
     *
     * @return true if the checking is satisfactory, false otherwise
     */
    bool checkSignalBindings() const;


    /**
     *
     *  Start signal tracing.
     *
     *  Initializes the signal tracing and creates the signal trace
     *  file.
     *
     *  @param traceFile A pointer to the trace file output stream object.
     *
     */

    void initSignalTrace(std::ostream *traceFile);

    /**
     *
     *  End signal tracing.
     *
     *  Finishes the signal tracing and closes the signal trace file.
     *
     */

    void endSignalTrace();

    /**
     *
     *  Dumps the signal trace for a simulation cycle.
     *
     *  @param cycle The simulation cycle for which to dump the
     *  signal trace.
     *
     */

    void dumpSignalTrace(u64bit cycle);

    /// Debug purpose only
    void dump(bool showOnlyNotBoundSignals = false) const;


};

} // namespace gpu3d

#endif

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

#ifndef _TRACE_DRIVER_INTERFACE_
    #define _TRACE_DRIVER_INTERFACE_

#include "AGPTransaction.h"
#include "GPUDriver.h"

/**
 *
 *  Trace Driver class.
 *
 *  Generates AGP transactions to the GPU from API traces.
 *
 */
class TraceDriverInterface
{

private:

public:

    /**
     * Starts the trace driver.
     *
     * Verifies if a TraceDriver object is correctly created and available for use
     *
     * @return  0 if all is ok.
     *         -1 opengl functions cannot be loaded
     *         -2 if traceFile could not be opened
     *         -3 if bufferFile could not be opened
     */
    virtual int startTrace() = 0;

    /**
     *
     *  Generates the next AGP transaction from the API trace
     *  file.
     *
     *  @return A pointer to the new AGP transaction, NULL if
     *  there are no more AGP transactions.
     *
     */
    virtual gpu3d::AGPTransaction* nextAGPTransaction() = 0;
    
    
    /**
     *
     *  Obtain the current position inside the trace.
     *
     *  @return The current position inside the trace.
     *
     */
     
    virtual u32bit getTracePosition() = 0;
};

extern "C" TraceDriverInterface *createTraceDriver(char *traceFile, GPUDriver* driver, u32bit startFrame);
extern "C" void destroyTraceDriver(TraceDriverInterface *trDriver);

#endif

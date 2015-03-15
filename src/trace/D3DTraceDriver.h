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

#ifndef _D3DTRACEDRIVER_
#define _D3DTRACEDRIVER_

class D3DTrace;

namespace gpu3d {
    class AGPTransaction;
}

/**
 *  Generates AGP transactions from an D3D API trace file.
 */
class D3DTraceDriver : public TraceDriverInterface
{
private:

    D3DTrace *trace; // Equivalent to GLExec
    u32bit startFrame;
    u32bit currentFrame;

public:
    int startTrace();
    D3DTraceDriver(char *traceFile, u32bit start_frame);
    gpu3d::AGPTransaction* nextAGPTransaction();
    u32bit getTracePosition();
    ~D3DTraceDriver();
};


#endif

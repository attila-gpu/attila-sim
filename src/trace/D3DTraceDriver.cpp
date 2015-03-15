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

#include "TraceDriverInterface.h"
#include "D3DTraceDriver.h"
#include "D3DTrace.h"
#include "D3DDriver/D3DDriver.h"
#include "GPUDriver.h"
using namespace gpu3d;

/*  TraceDriver constructor.  */
D3DTraceDriver::D3DTraceDriver(char *trace_file, u32bit _startFrame)
{
    trace = create_d3d_trace(trace_file);
    D3DDriver::initialize(trace);
    startFrame = _startFrame;
    currentFrame = 0;
}

AGPTransaction* D3DTraceDriver::nextAGPTransaction()
{
    AGPTransaction* agpt = 0;
    
    // Execute trace calls until GPUDriver receives
    // some transaction or end of trace reached.
    bool end = false;
    
    //  Execute D3D trace calls until an AGP Transaction is 
    //  generated or the trace ends.
    while ((agpt == 0) & !end)
    {
        //  Check if there is a transaction pending in the GPU Driver buffer.
        agpt = GPUDriver::getGPUDriver()->nextAGPTransaction();
                    
        if (agpt == NULL)
        {
            //  Execute the next D3D trace call.
            if(trace->next())
            {
                //  Check if the current frame finished.
                if(trace->getFrameEnded())
                {
                    GPUDriver::getGPUDriver()->printMemoryUsage();
                    
                    if (currentFrame < startFrame)
                    {
                        cout << "Skipped frame " << currentFrame << endl;
                    }                       
                    else
                    {
                        cout << "Rendered frame " << currentFrame << endl;
                        
                        //  Clear cycles spent on swap flush and DAC.
                        GPUDriver::getGPUDriver()->signalEvent(GPU_END_OF_FRAME_EVENT, "");
                    }
                    
                    // Disable preload memory before first renderable frame
                    if(currentFrame == (startFrame - 1))
                    {
                        GPUDriver::getGPUDriver()->setPreloadMemory(false);
                        cout << "D3DTRACEDRIVER: Disabling preload memory on frame " << (int)currentFrame << endl;
                        
                        //  Clear end of frame event register.
                        GPUDriver::getGPUDriver()->signalEvent(GPU_END_OF_FRAME_EVENT, "");
                        
                        //  Signal start of simulation.
                        GPUDriver::getGPUDriver()->signalEvent(GPU_UNNAMED_EVENT, "End of preload phase.  Starting simulation.");
                    }
                    
                    currentFrame ++;
                }
            }
            else
            {
                //  Reached the end of the D3D trace.
                end = true;
            }
        }
    }

    return agpt;
}

int D3DTraceDriver::startTrace() {
    //  Set preload memory mode
    if (startFrame > 0) {
        GPUDriver::getGPUDriver()->setPreloadMemory(true);
        cout << "D3DTRACEDRIVER: Enabling preload memory" << endl;

    }
    currentFrame = 0;
    return 0;
}

//  Return the current position inside the trace file.
u32bit D3DTraceDriver::getTracePosition()
{
    return trace->getCurrentEventID();
}


D3DTraceDriver::~D3DTraceDriver()
{
    D3DDriver::finalize();
    delete trace;
}

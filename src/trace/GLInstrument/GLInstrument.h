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

#ifndef GLINSTRUMENT_H
    #define GLINSTRUMENT_H

#include "GLJumpTable.h"
#include "GLResolver.h"

struct GLInstrument;

struct EventHandler
{
    virtual void action(GLInstrument& gli)=0;
};


// interface
struct GLInstrument
{  
    enum Instant { Before, After };

    // Access to read GLCalls
    // Allows to modify real gl calls
    // ie. insert your own implementation for a concrete call
    // note: gli.glCall().glNormal3f = NULL; // skip execution of glNormal3f
    virtual GLJumpTable& glCall()=0;

    // Example of use gli.registerBeforeFunc().glBegin = glBeginCount;
    virtual GLJumpTable& registerBeforeFunc()=0;
    virtual GLJumpTable& registerAfterFunc()=0;
    
    virtual void registerStartHandler(EventHandler& sah)=0;
    virtual void unregisterStartHandler()= 0;
    virtual void registerEndHandler(EventHandler& sah)=0;
    virtual void unregisterEndHandler()=0;
    
    virtual void registerGlobalHandler(Instant when, EventHandler& gh)=0;
    virtual void unregisterGlobalHandler(Instant when)=0;
    virtual void registerTimeHandler(long time, EventHandler& handler)=0;  
    virtual void unregisterTimeHandler()=0;    
    virtual void registerBatchHandler(Instant when, EventHandler& handler)=0;
    virtual void unregisterBatchHandler(Instant when)=0;
    virtual void registerFrameHandler(Instant when, EventHandler& handler)=0;
    virtual void unregisterFrameHandler(Instant when)=0;
    virtual void registerFrameHandler(int numberOfCalls, EventHandler& ec)=0;    
    virtual void unregisterFrameHandler()=0;
    
    virtual void doSwapBuffers()=0;
   
    /**
     * Query methods
     */
    virtual void resolution(int& h, int& w)=0;
    virtual int currentFrame()=0;
    virtual int currentBatch()=0;
    virtual int executedCalls()=0;
    virtual long executionTime()=0;
    virtual int vertexCount()=0; // in last batch
    virtual int frameVertexCount()= 0; // in current frame
    virtual int totalVertexCount()=0; // total vertex count (global)
    virtual int triangleCount()=0;
    virtual int frameTriangleCount()=0;
    virtual int totalTriangleCount()=0;
    virtual int callsCount()=0; // in this frame (or batch)
    virtual APICall lastCall()=0;

};


#endif // GLINSTRUMENT_H

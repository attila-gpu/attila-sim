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

#ifndef GLINSTRUMENTIMP_H
    #define GLINSTRUMENTIMP_H

#include "GLInstrument.h"
#include "ConfigManager.h"

/**
 * GLInstrument implementation
 */
class GLInstrumentImp : public GLInstrument
{
public:

    // Not present in GLInstrument interface. Once method used to initialize
    // GLInstrumentImp object
    void init();

    GLInstrumentImp();

    GLJumpTable& glCall();

    // Example of use gli.registerBeforeFunc().glBegin = glBeginCount;
    GLJumpTable& registerBeforeFunc();
    GLJumpTable& registerAfterFunc();
    
    void registerStartHandler(EventHandler& sah);
    void unregisterStartHandler();
    void registerEndHandler(EventHandler& sah);
    void unregisterEndHandler();
    
    void registerGlobalHandler(Instant when, EventHandler& gh);
    void unregisterGlobalHandler(Instant when);
    void registerTimeHandler(long time, EventHandler& handler);  
    void unregisterTimeHandler();
    void registerBatchHandler(Instant when, EventHandler& handler);
    void unregisterBatchHandler(Instant when);
    void registerFrameHandler(Instant when, EventHandler& handler);
    void unregisterFrameHandler(Instant when);
    void registerFrameHandler(int numberOfCalls, EventHandler& ec);   
    void unregisterFrameHandler();

    void doSwapBuffers();
    
    void resolution(int& h, int& w);
    int currentFrame();
    int currentBatch();
    int executedCalls();
    long executionTime();

    void incVertexCount(int vc=1);
    int vertexCount(); // in this frame (or batch)
    int frameVertexCount(); // in current frame
    int totalVertexCount(); // total vertex count (global)
    void setPrimitive(GLenum primitive);
    int triangleCount();
    int frameTriangleCount();
    int totalTriangleCount();
    int callsCount(); // in this frame (or batch)
    APICall lastCall();

    ~GLInstrumentImp();

    /**
     * Methods added to the interface defined in GLInstrument
     */
    void doStartBatchEvent();
    void doEndBatchEvent();
    void doStartFrameEvent();
    void doEndFrameEvent();

    void setHDC(HDC hdc);
    HDC getHDC() const;

    bool isForcedSwap() const;

private:

    bool forcedSwap;
    HDC hdc;

    int frame;
    int batch;

    int batchVC;
    int frameVC;
    int totalVC;

    GLenum primitive;
    int batchTC;
    int frameTC;
    int totalTC;

    ConfigManager config;

    GLJumpTable glCalls; // contains real opengl function pointers
    GLJumpTable prevUserCall; // contains user call pointers called before real gl call
    GLJumpTable postUserCall; // contains user call pointers called after real gl call

    /**
     * 0: previous, 1: after
     */
    EventHandler* batchHandler[2];
    EventHandler* frameHandler[2];
    EventHandler* countHandler;
    int count;
    EventHandler* timeHandler;
    long alarm;
    EventHandler* globalHandler[2];
    EventHandler* onceActionHandler[2]; // 0: start, 1: end

    bool loadGLCalls();

};

#endif // GLINSTRUMENTIMP_H

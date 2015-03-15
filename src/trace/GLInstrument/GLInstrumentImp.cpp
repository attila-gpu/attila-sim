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

#include "MainGLInstrument.h"
#include "GLInstrumentImp.h"
#include "DLLLoader.h"
#include "GLITool.h"
#include "support.h"
#include <string>

using namespace std;

#define PREV 0
#define POST 1

typedef GLITool* (APIENTRY *PFNGETTOOL)();
typedef void (APIENTRY *PFNRELEASETOOL)(GLITool* glit);

bool GLInstrumentImp::loadGLCalls()
{
    char openGLPath[1024];    
    (void)GetSystemDirectory(openGLPath,sizeof(openGLPath));
    strcat(openGLPath,"\\opengl32.dll");        
    return loadGLJumpTable(glCalls, openGLPath);
}

GLInstrumentImp::GLInstrumentImp() : frame(0), batch(0), 
    batchVC(0), frameVC(0), totalVC(0),
    batchTC(0), frameTC(0), totalTC(0), hdc(0), forcedSwap(false)
{
    batchHandler[0] = 0;
    batchHandler[1] = 0;
    frameHandler[0] = 0;
    frameHandler[1] = 0;
}


void GLInstrumentImp::init()
{
    static bool initCalled = false;
    
    if ( initCalled ) 
        return;
    initCalled = true;

    hdc = GetDC(0); // capture HDC

    memset(&glCalls,0,sizeof(GLJumpTable));
    loadGLCalls();
    memset(&prevUserCall,0,sizeof(GLJumpTable));
    memset(&postUserCall,0,sizeof(GLJumpTable));

    config.loadConfigFile("GLInstrument.ini");
    string toolName = config.getOption("tool");

    if ( toolName == "null" || toolName == "NULL" )
    {
        MessageBox(NULL, "Running without any tool attached", "GLInstrument", MB_OK);
        return ;
    }

    DLLLoader dll;
    if ( dll.open(toolName) )
    {
        PFNGETTOOL getToolProc = (PFNGETTOOL)dll.getFunction("getTool");
        GLITool* tool = getToolProc();
        if ( tool )
            tool->init(*this);
        else
            MessageBox(NULL, "getTool returned NULL. Running without any tool attached", "GLInstrument", MB_OK);
    }
    else
    {
        char msg[256];
        sprintf(msg, "Error finding tool: \"%s\"", toolName.c_str());
        panic("GLInstrumentImp", "init()", msg);
    }
}

GLInstrumentImp::~GLInstrumentImp()
{
}

GLJumpTable& GLInstrumentImp::glCall()
{
    return glCalls;
}

GLJumpTable& GLInstrumentImp::registerBeforeFunc()
{
    return prevUserCall;
}

GLJumpTable& GLInstrumentImp::registerAfterFunc()
{
    return postUserCall;
}

void GLInstrumentImp::registerStartHandler(EventHandler& sah)
{
    onceActionHandler[PREV] = &sah;
}

void GLInstrumentImp::unregisterStartHandler()
{
    onceActionHandler[PREV] = 0;
}

void GLInstrumentImp::registerEndHandler(EventHandler& sah)
{
    onceActionHandler[POST] = &sah;
}

void GLInstrumentImp::unregisterEndHandler()
{
    onceActionHandler[POST] = 0;
}

void GLInstrumentImp::registerGlobalHandler(Instant when, EventHandler& gh)
{
    if ( when == Before )
        globalHandler[PREV] = &gh;
    else
        globalHandler[POST] = &gh;
}

void GLInstrumentImp::unregisterGlobalHandler(Instant when)
{
    if ( when == Before )
        globalHandler[PREV] = 0;
    else
        globalHandler[POST] = 0;
}

void GLInstrumentImp::registerTimeHandler(long time, EventHandler& handler)
{
    alarm = time;
    timeHandler = &handler;
}

void GLInstrumentImp::unregisterTimeHandler()
{
    timeHandler = 0;
}



void GLInstrumentImp::registerBatchHandler(Instant when, EventHandler& handler)
{
    if ( when == Before )
        batchHandler[PREV] = &handler;
    else
        batchHandler[POST] = &handler;
}

void GLInstrumentImp::unregisterBatchHandler(Instant when)
{
    if ( when == Before )
        batchHandler[PREV] = 0;
    else
        batchHandler[POST] = 0;
}


void GLInstrumentImp::registerFrameHandler(Instant when, EventHandler& handler)
{
    if ( when == Before )
        frameHandler[PREV] = &handler;
    else
        frameHandler[POST] = &handler;
}

void GLInstrumentImp::unregisterFrameHandler(Instant when)
{
    if ( when == Before )
        frameHandler[PREV] = 0;
    else
        frameHandler[POST] = 0;
}

void GLInstrumentImp::registerFrameHandler(int numberOfCalls, EventHandler& handler)
{
    count = numberOfCalls;
    countHandler = &handler;
}

void GLInstrumentImp::unregisterFrameHandler()
{
    countHandler = 0;
}

void GLInstrumentImp::doSwapBuffers()
{
    //glCalls.wglSwapBuffers(hdc);
    forcedSwap = true;
    SwapBuffers(hdc);
    forcedSwap = false;
}


/********************************
 * QUERY METHODS IMPLEMENTATION *
 ********************************/

void GLInstrumentImp::resolution(int& h, int& w)
{
}

int GLInstrumentImp::currentFrame()
{
    return frame;
}

int GLInstrumentImp::currentBatch()
{
    return batch;
}
 
int GLInstrumentImp::executedCalls()
{
    return 0;
}
 
long GLInstrumentImp::executionTime()
{
    return 0;
}

void GLInstrumentImp::incVertexCount(int vc)
{
    batchVC += vc;
}
 
int GLInstrumentImp::vertexCount()
{
    return batchVC;
}

int GLInstrumentImp::frameVertexCount()
{
    return frameVC + batchVC;
}
int GLInstrumentImp::totalVertexCount()
{
    return totalVC + frameVC + batchVC;
}

int GLInstrumentImp::triangleCount()
{
    panic("GLInstrumentImp", "triangleCount()", "Not implemented yet");
    return 0;
}

int GLInstrumentImp::frameTriangleCount()
{
    return frameTC + batchTC;
}

int GLInstrumentImp::totalTriangleCount()
{
    return totalTC + frameTC + batchTC;
}


int GLInstrumentImp::callsCount()
{
    return 0;
}

APICall GLInstrumentImp::lastCall()
{
    return APICall_glBegin;
}


/**
 * METHODS ADDED TO THE BASE INTERFACE DEFINED IN GLInstrument
 */

void GLInstrumentImp::doEndBatchEvent()
{
    if ( batchHandler[1] != 0 )
        batchHandler[1]->action(*this);
    // update counters
    frameVC += batchVC;
    batchVC = 0;
    frameTC += batchTC; 
    batchTC = 0;
    batch++; // new batch
}


void GLInstrumentImp::doEndFrameEvent()
{
    if ( frameHandler[1] != 0 )
        frameHandler[1]->action(*this);
    // update counters
    totalVC += frameVC;
    frameVC = 0;
    totalTC += frameTC;
    frameTC = 0;
    batch = 0;
    frame++; // new frame
}


void GLInstrumentImp::setHDC(HDC hdc)
{
    this->hdc = hdc;
}

HDC GLInstrumentImp::getHDC() const
{
    return hdc;
}

bool GLInstrumentImp::isForcedSwap() const
{
    return forcedSwap;
}

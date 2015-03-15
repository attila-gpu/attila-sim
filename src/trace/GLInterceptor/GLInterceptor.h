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

#ifndef GLINTERCEPTOR_H
    #define GLINTERCEPTOR_H

/* Include standard openGL declaration file */
#include "support.h" /* include windows if required */
#include "GLJumpTable.h"
#include "TraceWriter.h"
//#include "BufferObj.h"
#include "BufferDescriptor.h"
#include "GLIStatsManager.h"
#include <string>

#ifdef _MSC_VER
#pragma warning (disable:4201)
#pragma warning (disable:4214)
#pragma warning (disable:4115)
#pragma warning (disable:4514)
#pragma warning (disable:4127)
#pragma warning (disable:4273)   /* No complaints about DLL linkage... */
#endif /* _MSC_VER */


/* Macro for fast access to flag "executeRealCalls" */
//#define DO_REAL_CALL(call) ( GLInterceptor::executeRealGLCalls() ? GLInterceptor::jt.call : NULL )
#define DO_REAL_CALL(call) GLInterceptor::jt.call

/*
 * GLInterceptor
 *
 * Traces OpenGL programs
 *
 * @note: Some components of this program are generated automatically, this unavailable parts
 *        are mark with tag //<START>
 * 
 *
 * @author Carlos Gonzalez Rodriguez - cgonzale@ac.upc.es
 * @version 0.1 Beta
 * @date 19/11/2003
 */
class GLInterceptor
{
private:

    GLInterceptor();
    GLInterceptor(const GLInterceptor&);


    static BufferManager bm;

    static bool initCalled;

    //static bool traceCall[];
    static bool isStateCall[];

    /* config options */
    static bool GLILogFlags[]; // 3 flags

    static char outputTraceFile[];
    static char statsPerFrameFile[];
    static char statsTotalFile[];
    static char statsPerBatchFile[];

    static int currentFrame;
    static int firstFrame;
    static int lastFrame;

    static int splitLevel;

    static int cacheSz;

    static bool acceptDeferredBuffers;

    static bool vertical;

    /**
     * This flag indicates if we are supporting all extension even those
     * not supported by the current hardware. That means we can log the
     * commands but we cannot execute them ( maybe with this feature enabled
     * some crashes or unexpected behaviour can be "achieved" )
     */
    static bool hackMode; 

    /**
     * Indicates if OpenGL real calls must be executed
     * If true the calls are traced and executed, if value is false
     * then only trace calls is done, but they're not executed
     */
    static bool executeCalls;

    /**
     * Return is traced/not traced (just a "NOT TRACED" is placed)
     */
    static bool traceReturn;

    /**
     * Application resolution x-y
     */
    static int resWidth;
    static int resHeight;

    /**
     *  This flags selects between uncompressed output and compressed output for the
     *  tracefile.
     *
     */

    static bool compressedOutput;

public:

    enum GLI_LOGFLAG
    {
        LOG_TRACE,
        LOG_BUFFERS,
        LOG_STATS_PER_FRAME,
        LOG_STATS_TOTAL
    };

    enum GLI_OFILEFLAG
    {
        FILE_TRACEFILE,
        FILE_STATS_PER_FRAME,
        FILE_STATS_PER_BATCH,
        FILE_STATS_TOTAL
    };

    static BufferManager& getBufferManager() { return bm; }

    static std::string getInternalInfo();

    static void initStateCallsArray();
    static void startTWCallMode(APICall apicall);
    static void restoreTWMode(APICall apicall);

    static void incFrame() { currentFrame++; }
    static void setFrame( int frame ) { currentFrame = frame; }
    static int getFrame() { return currentFrame; }

    static bool isReturnTraced() { return traceReturn; }
    static void setReturnTraced(bool t) { traceReturn = t; }

    static int getSplitLevel();

    static bool isVerticalDumpMode();

    static void setLogFlag( GLI_LOGFLAG flag, bool enabled );
    static bool isLogFlagEnabled( GLI_LOGFLAG flag );
    static const char* getOutputFile( GLI_OFILEFLAG flag );

    static int getFirstFrame();
    static int getLastFrame();

    static bool isHackMode();
    static void setHackMode( bool mode );

    static bool executeRealGLCalls();
    static void setExecuteRealCalls( bool executeCalls );


    /** 
     * TraceWriter object were the trace is written
     */
    static TraceWriter tw;

    /**
     * Initial TraceWriter mode
     */
    static TraceWriter::TW_MODE initialTWMode;

    /**
     * Jump table where pointers to real OpenGL functions are stored
     */
    static GLJumpTable jt;

    /**
     * GLInterceptor statistics manager
     */
    static GLIStatsManager statManager;


    /**
     * Jump table where pointer to wrapper functions are stored
     *
     * It is required to return pointers to wrapped functions
     * instead of pointers to original opengl function
     *
     * for example when wglGetProcAddress is being used
     */
    static GLJumpTable jtWrapper;

    /**
     * init method ( load real OpenGL dll function pointers )
     * it is a ONCE method, calls after first call do not do anything. Only return
     */
    static void init();

    static void updateStats( int frame );

    static bool parseConfigFile( const char* configFileName );

    //static void resetPartialCounters();

    static TraceWriter::TW_MODE getInitialTWMode();

    static void setAppResolution(int width, int height);
    static void getAppResolution(int& width, int& height);


};

#endif // GLINTERCEPTOR_H

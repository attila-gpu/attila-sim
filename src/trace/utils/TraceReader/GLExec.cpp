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

#include "GLExec.h"
#include "StubApiCalls.h" // Stubs
#include <iostream>
#include "GLExecStats.h"
#include "IncludeLog.h"
#include <cstring>

using namespace std;
using includelog::logfile; // Make log file visible

GLExec::GLExec() : currentExec(true), current(APICall_UNDECLARED), savedTracePos(0)
{}

Param GLExec::getCurrentParam(unsigned int iParam)
{
    return getLastCallParameter(iParam);
}

void GLExec::setCurrentParam(unsigned int iParam, const Param& p)
{
    setLastCallParameter(iParam, p);
}

void GLExec::getTraceResolution(unsigned int& width, unsigned int& height)
{
    tr.readResolution(width, height);
}



int GLExec::init( const char* trace, const char* buffersFile, const char* regionsFile,
                   bool enableStats )
{
    logfile().pushInfo(__FILE__,__FUNCTION__);

    this->enableStats = enableStats;
    if ( enableStats )
    {
        statsFile.open("glexecStats.csv");
        if ( !statsFile.is_open() )
        {
            logfile().write(includelog::Panic, "\nStats file could not be created");
            panic("GLExec", "init", "Stats file could not be created");
        }
        statsFile << "Vertexes" << sepStats << "Batches" << endl; // eol + flush
    }

    char msg[256];
    char openGLPath[1024] = "\0";
    //(void)GetSystemDirectory(openGLPath, sizeof(openGLPath));
    //strcat(openGLPath,"\\");
    strcat(openGLPath,"opengl32.dll"); // LOAD LOCAL DLL

    memset(&jt,0,sizeof(GLJumpTable));
    
    memset(&uct,0,sizeof(GLJumpTable));
    
    if ( !loadGLJumpTable(jt,openGLPath) )
    {
        logfile().write(includelog::Panic, "\nError loading GL JumpTable");
        panic("GLExec","init()","Error loading GL JumpTable");
        return -1;
    }

    ifstream bufTest(buffersFile, ios::in | ios::binary);
    ifstream memTest(regionsFile, ios::in | ios::binary);

    if ( bufTest.is_open() && bufTest.is_open() )
    {
        bufTest.close();
        memTest.close();
        if ( !tr.open(trace, buffersFile, regionsFile) )
        {        
            sprintf(msg, "Error opening trace file: %s", trace);
            logfile().write(includelog::Panic, "\n");
            logfile().write(includelog::Panic, msg);
            panic("GLExec", "init()", msg);
            return -2;
        }
    }
    else
    {
        bufTest.close();
        memTest.close();
        if ( !tr.open(trace) )
        {
            sprintf(msg, "Error opening trace file: %s", trace);
            logfile().write(includelog::Panic, "\n");
            logfile().write(includelog::Panic, msg);
            panic("GLExec", "init()", msg);
            return -2;
        }
    }

    addUserCalls();

    logfile().popInfo();
    
    return 0; // OK
}

APICall GLExec::getCurrentCall()
{    
    // if call has not been executed return it as current
    if ( !currentExec ) // not consumed yet
        return current; // return current again
    
    // else : parse APICall from TraceReader
    currentExec = false; // new call (not executed)
    current = tr.parseApiCall();
    return current;
}

void GLExec::skipCurrentCall()
{
    tr.skipCall();
    currentExec = true; // Assumes it was executed
}


APICall GLExec::executeCurrentCall()
{   
    if ( currentExec )
        getCurrentCall(); // get another call    

    if ( current == APICall_UNDECLARED )
    {
        cout << "Executing APICall_UNDECLARED (execution ignored)" << endl;
        return APICall_UNDECLARED; // APICall_UNDECLARED is never executed
    }
    
    currentExec = true; // mark as executed ( consumed )    
    
    if ( current == APICall_SPECIAL )
        return APICall_SPECIAL;
    
    switch ( current )
    {
        /*
         * Example of branch added:
         *
         * case APICall_glBegin:
         *     STUB_glBegin(tr);
         *     break;
         *
         *      .
         *      .
         *      .
         */
        #include "SwitchBranches.gen"

        default:
        {
            cout << "Current: " << int(current) << endl;
            cout << "APICall_UNDECLARED: " << (int)APICall_UNDECLARED << endl;
            panic("GLExec", "executeCurrentCall", "Unknown OpenGL Call");
        }
    }

    if ( current == APICall_wglSwapBuffers )
    {
        // process per frame stats
        processPerFrameStats();
    }
    
    return current;
}


void GLExec::setDirectivesEnabled(bool enabled)
{
    tr.setDirectivesEnabled(enabled);
}

   
bool GLExec::areDirectivesEnabled() const
{
    return tr.areDirectivesEnabled();
}


long GLExec::saveTracePosition()
{
    savedTracePos = tr.getCurrentTracePos();
    /*
    char msg[256];
    sprintf(msg, "Saving pos: %d", savedTracePos);
    MessageBox(NULL, msg, "SAVE", MB_OK);
    */
    return savedTracePos;
}

void GLExec::restoreTracePosition()
{
    //long curPos = tr.getCurrentTracePos();
    tr.setCurrentTracePos(savedTracePos);
    /*
    char msg[256];
    sprintf(msg, "Current pos: %d Restoring pos: %d", curPos, savedTracePos);
    MessageBox(NULL, msg, "RESTORE", MB_OK);
    */
}


void GLExec::addUserCalls()
{
    // empty by default

    
    // Example: (your instrumentation calls)
    //uct.glEnd = end_USER;
    uct.glDrawArrays = drawArrays_USER;
    uct.glDrawElements =  drawElements_USER;
    uct.glDrawRangeElements = drawRangeElements_USER;
}

void GLExec::processPerFrameStats()
{
    if ( enableStats )
    {
        GLExecStats& stats = GLExecStats::instance();
        // write stats + eol + flush
        statsFile << stats.getVertexCount() << sepStats << stats.getBatchCount() << endl;
        stats.resetBatchCount();
        stats.resetVertexCount();
    }
}

bool GLExec::isCallAvailable(APICall apicall)
{
    unsigned int* ptr = reinterpret_cast<unsigned int*>(&jt);
    return ( ptr[apicall] != 0 );
}

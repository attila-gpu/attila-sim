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

#include "GLTraceDriver.h"
/*#include "GPULib.h"*/
#include <cstdio>
#include <iostream>
#include "GLContext.h"
//#include "ACDDevice.h"
#include "AuxFuncsLib.h"
#include "GPULibInternals.h"
#include "AGLEntryPoints.h"
#include "AGL.h"
#include "GLResolver.h"
#include "BufferDescriptor.h"

#include "GlobalProfiler.h"

using namespace gpu3d;
using namespace std;

/*  TraceDriver constructor.  */
GLTraceDriver::GLTraceDriver(char *traceFile, bool useACD, GPUDriver* driver, bool triangleSetupOnShader,
                             u32bit startFrame) :
    startFrame(startFrame), currentFrame(0), _useACD(useACD)
    // initialize AGPTransaction buffer control variables
{
    this->driver = driver;

    GLOBALPROFILER_ENTERREGION("GLExec", "", "")
    BufferManager& bm = gle.getBufferManager();
    GLOBALPROFILER_EXITREGION()
        
    // Must be selected before opening Buffer Descriptors file
    bm.acceptDeferredBuffers(true);

    GLOBALPROFILER_ENTERREGION("GLExec", "", "")
    initValue = gle.init(traceFile, "BufferDescriptors.dat", "MemoryRegions.dat");
    GLOBALPROFILER_EXITREGION()

    if ( _useACD ) {
        _setAGLFunctions(); // overwrite jump table with AGL versions
        agl::initAGL(driver, startFrame);
    }
    else
        initOGLLibrary(driver, triangleSetupOnShader);
}

GLTraceDriver::GLTraceDriver( const char* traceFile, bool useACD, const char* bufferFile, const char* memFile,
    GPUDriver* driver, bool triangleSetupOnShader, u32bit startFrame ) : 
        startFrame(startFrame), currentFrame(0), _useACD(useACD)
{
    this->driver = driver;

    GLOBALPROFILER_ENTERREGION("GLExec", "", "")
    BufferManager& bm = gle.getBufferManager();
    
    // Must be selected before opening Buffer Descriptors file
    bm.acceptDeferredBuffers(true);

    initValue = gle.init(traceFile, bufferFile, memFile);
    GLOBALPROFILER_EXITREGION()
    
    if ( _useACD ) {
        _setAGLFunctions(); // overwrite jump table with AGL versions
        agl::initAGL(driver, startFrame);
    }
    else
        initOGLLibrary(driver, triangleSetupOnShader);
}

int GLTraceDriver::startTrace()
{
    // do not do anything :-)
    return initValue; // return state of TraceDriver object ( 0 means ready )
}


// new version to allow hotStart with identical memory footprint
AGPTransaction* GLTraceDriver::nextAGPTransaction()
{
    AGPTransaction* agpt;
    APICall oglCommand;

    agpt = NULL;

    static bool resInit = false;
    if ( !resInit )
    {
        GLOBALPROFILER_ENTERREGION("GLExec", "", "")
        if ( startFrame != 0 )
            gle.setDirectivesEnabled(false); // Disable directives
        else
            gle.setDirectivesEnabled(true);
        resInit = true;
        u32bit width, height;
        gle.getTraceResolution(width, height);
        GLOBALPROFILER_EXITREGION()
        if ( width != 0 && height != 0)
        {
            cout << "TraceDriver: Setting resolution to " << width << "x" << height << endl;
            
	    if (_useACD)
            {
                agl::setResolution(width, height);
            }
            else
            {
                driver->setResolution(width, height);
                driver->initBuffers();
            }
        }
        //  Set preload memory mode
        if (startFrame > 0)
            driver->setPreloadMemory(true);
    }

    // Try to drain an AGPTransaction from driver buffer
    while ( !( agpt = driver->nextAGPTransaction() ) )
    {
        bool interpretBatchAsFrame = false; // Interpret this batch as a frame

        GLOBALPROFILER_ENTERREGION("GLExec", "", "")
        oglCommand = gle.getCurrentCall();
        GLOBALPROFILER_EXITREGION()
        
        // Enable this code to dump GL calls ignored by GLExec
        //if ( !gle.isCallAvailable(oglCommand) && oglCommand != APICall_UNDECLARED )
        //    cout << "Warning - OpenGL call " << GLResolver::getFunctionName(oglCommand) << " NOT AVAILABLE" << endl;

        switch ( oglCommand )
        {
            /*
            case APICall_glClear:
                if ( currentFrame < startFrame )
                {
                    gle.skipCurrentCall();
                    continue;
                }
                */
            case APICall_glEnd:
            case APICall_glDrawArrays:
            case APICall_glDrawElements:
            case APICall_glDrawRangeElements:
                GLOBALPROFILER_ENTERREGION("GLExec", "", "")
                interpretBatchAsFrame = gle.checkBatchesAsFrames();
                GLOBALPROFILER_EXITREGION()
                break;
            case APICall_SPECIAL:
            {
                GLOBALPROFILER_ENTERREGION("GLExec", "", "")
                string str = gle.getSpecialString();
                GLOBALPROFILER_EXITREGION()
                if ( str == "DUMP_CTX" )
                {
                    // process DUMP_CTX
                    if ( driver->getContext() == 0 )
                        cout << "output:> GL Context not found (command igbnored)" << endl;
                    else {
                        if ( _useACD ) {
                            //acdlib::ACDDevice* acddev = (acdlib::ACDDevice*)driver->getContext();
                            cout << "output:> Performing DUMP_CTX:" << endl;
                            //acddev->DBG_dump("", 0);
                        }
                        else {
                            libgl::GLContext* ctx = (libgl::GLContext*)driver->getContext();
                            cout << "output:> Performing DUMP_CTX:" << endl;
                            ctx->dump();
                        }
                    }                    
                }
                else if ( str == "DUMP_STENCIL" )
                {
                    // process DUMP_STENCIL
                    if ( _useACD ) {
                        cout << "output:> ACDX does not implement yet stencil buffer dump" << endl;
                    }
                    else {
                        libgl::GLContext* ctx = (libgl::GLContext*)driver->getContext();
                        if ( ctx )
                        {
                            cout << "output:> Performing Dump of Stencil Buffer." << endl;
                            libgl::afl::dumpStencilBuffer(ctx);
                            interpretBatchAsFrame = true;
                        }
                        else
                            cout << "output:> GL Context not found (command igbnored)" << endl;
                    }
                }
                else
                    cout << "output:> '" << str << "' unknown command" << endl;
                gle.resetSpecialString();
            }

            case APICall_UNDECLARED:
                return 0; // End of trace
        }
        
        GLOBALPROFILER_ENTERREGION("GLExec", "", "")
        gle.executeCurrentCall(); // Perform call execution
        GLOBALPROFILER_EXITREGION()

        if ( oglCommand == APICall_wglSwapBuffers || interpretBatchAsFrame )
        {
            APICall nextOGLCall = gle.getCurrentCall();
            if ( interpretBatchAsFrame && nextOGLCall == APICall_wglSwapBuffers )
                continue; // DO NOT DO ANYTHING. Swap will be performed by wglSwapBuffers (including currentFrame++)

            if ( currentFrame >= startFrame )
            {
                cout << "Dumping frame " << currentFrame;
                if ( oglCommand != APICall_wglSwapBuffers )
                    cout << "  (It is a BATCH)";
                cout << endl;
            }
            else
            {
                cout << "Frame " << currentFrame << " Skipped";
                if ( oglCommand != APICall_wglSwapBuffers )
                    cout << "  (It is a BATCH)";
                cout << endl;
            }
            
            if ( _useACD )
                agl::swapBuffers();
            else
                privateSwapBuffers();
                
            //  Clean end of frame event to remove swap (flush) and DAC cycles.                
            if (currentFrame >= startFrame)
                driver->signalEvent(GPU_END_OF_FRAME_EVENT, "");

            currentFrame++;
            
            //  Disable preload memory mode just before ending the last skipped frame
            if (currentFrame == startFrame)
            {
                driver->setPreloadMemory(false);
                
                cout << "TraceDriver::nextAGPTransaction() -> Disabling preload..." << endl;
               
                gle.setDirectivesEnabled(true);
           
                //  Reset the end of frame event.
                driver->signalEvent(GPU_END_OF_FRAME_EVENT, "");
                
                //  Signal that the preload phased has finished.
                driver->signalEvent(GPU_UNNAMED_EVENT, "End of preload phase.  Starting simulation.");
            }

            if ( _useACD ) {}
            else
                ctx->currentFrame(currentFrame);

            driver->printMemoryUsage();
            //driver->dumpStatistics();
        }
    }
    
    return agpt;
}

//  Return the current position inside the trace file.
u32bit GLTraceDriver::getTracePosition()
{
    return gle.currentTracefileLine();
}

/*
AGPTransaction* TraceDriver::nextAGPTransaction()
{
    AGPTransaction* agpt;
    APICall oglCommand;

    static bool resInit = false;
    if ( !resInit )
    {
        if ( startFrame != 0 )
            gle.setDirectivesEnabled(false); // Disable directives
        else
            gle.setDirectivesEnabled(true);
        resInit = true;
        u32bit width, height;
        gle.getTraceResolution(width, height);
        if ( width != 0 && height != 0)
        {
            cout << "TraceDriver: Setting resolution to " << width << "x" << height << endl;
            driver->setResolution(width, height);
            driver->initBuffers();
        }
    }

    // Try to drain an AGPTransaction from driver buffer
    while ( !( agpt = driver->nextAGPTransaction() ) )
    {
        bool interpretBatchAsFrame = false; // Interpret this batch as a frame
        bool skipCall = false; // Skip this call

        // If there is not AGPTransactions read an execute a new OGL Command from traceFile


        oglCommand = gle.getCurrentCall();

        switch ( oglCommand )
        {
            case APICall_glClear:
            case APICall_glBegin:
            case APICall_glVertex2f:
            case APICall_glVertex3f:
            case APICall_glVertex3fv:
            case APICall_glVertex2i:
                if ( currentFrame < startFrame )
                    skipCall = true;
                break;
            case APICall_glDrawArrays:
            case APICall_glEnd:
                if  ( currentFrame < startFrame )
                {
                    skipCall = true;
                    if ( hotStart )
                        ctx->preloadMemory();
                }
                if ( gle.checkBatchesAsFrames() )
                    interpretBatchAsFrame = true;
                break;

            case APICall_glDrawElements:
            case APICall_glDrawRangeElements:
                if  ( currentFrame < startFrame )
                {
                    skipCall = true;
                    if ( hotStart )
                        ctx->preloadMemory(true);
                }
                if ( gle.checkBatchesAsFrames() )
                    interpretBatchAsFrame = true;
                break;
            case APICall_UNDECLARED:
                return NULL;
            case APICall_SPECIAL:
            {
                string str = gle.getSpecialString();
                if ( str == "DUMP_CTX" )
                {
                    // process DUMP_CTX
                    libgl::GLContext* ctx = (libgl::GLContext*)driver->getContext();
                    if ( ctx )
                    {
                        cout << "output:> Performing DUMP_CTX:" << endl;
                        ctx->dump();
                    }
                    else
                        cout << "output:> GL Context not found (command igbnored)" << endl;
                }
                else if ( str == "DUMP_STENCIL" )
                {
                    // process DUMP_STENCIL
                    libgl::GLContext* ctx = (libgl::GLContext*)driver->getContext();
                    if ( ctx )
                    {
                        cout << "output:> Performing Dump of Stencil Buffer." << endl;
                        libgl::afl::dumpStencilBuffer(ctx);
                        interpretBatchAsFrame = true;
                    }
                    else
                        cout << "output:> GL Context not found (command igbnored)" << endl;
                }
                else
                    cout << "output:> '" << str << "' unknown command" << endl;
                gle.resetSpecialString();
            }
        }

        APICall nextOGLCall;

        if ( skipCall ) // Current (BATCH) call must be skipped
        {
            gle.skipCurrentCall();
            nextOGLCall = gle.getCurrentCall();
            if ( interpretBatchAsFrame && nextOGLCall != APICall_wglSwapBuffers ) // Batch that must be counted
            {
                cout << "Frame " << currentFrame << " Skipped (it is a BATCH)" << endl;
                currentFrame++;
                ctx->currentFrame(currentFrame); // DEBUG
            }
            // if nextOGLCall == APICall_wglSwapBuffers then frame count will be performed by wglSwapBuffers
            continue; // go to next loop iteration
        }

        gle.executeCurrentCall(); // Perform call execution

        if ( oglCommand == APICall_wglSwapBuffers || interpretBatchAsFrame )
        {
            if ( currentFrame >= startFrame )
            {
                if ( interpretBatchAsFrame && nextOGLCall == APICall_wglSwapBuffers )
                {
                    continue; // DO NOT DO ANYTHING. Swap will be performed by wglSwapBuffers (including currentFrame++)
                }

                if ( currentFrame >= startFrame )
                    gle.setDirectivesEnabled(true);

                cout << "Dumping frame " << currentFrame; // << " (trace line " << gle.currentTracefileLine() << ")";
                if ( oglCommand != APICall_wglSwapBuffers )
                    cout << "  (It is a BATCH)";
                cout << endl;

                privateSwapBuffers();
            }
            else
            {
                if ( interpretBatchAsFrame )
                    panic("TraceDriver", "nextAGPTransaction", "PROG ERROR...");
                cout << "Frame " << currentFrame << " Skipped" << endl;
            }
            currentFrame++;
            ctx->currentFrame(currentFrame); // DEBUG
            driver->printMemoryUsage();
        }
    }
    return agpt;
}
*/

void GLTraceDriver::_setAGLFunctions()
{
    GLJumpTable& jt = gle.jTable();

    memset(&jt, 0, sizeof(GLJumpTable)); // reset previous pointers

    // Set jumptable pointers to point new "GLLib on ACD" entry points
    jt.glBegin = AGL_glBegin;
    jt.glBindBufferARB = AGL_glBindBufferARB;
    jt.glBindProgramARB = AGL_glBindProgramARB;
    jt.glBufferDataARB = AGL_glBufferDataARB;
    jt.glBufferSubDataARB = AGL_glBufferSubDataARB;
    jt.glClear = AGL_glClear;
    jt.glClearColor = AGL_glClearColor;
    jt.glClearDepth = AGL_glClearDepth;
    jt.glColor3f = AGL_glColor3f;
    jt.glColor4f = AGL_glColor4f;
    jt.glColorPointer = AGL_glColorPointer;
    jt.glDepthFunc = AGL_glDepthFunc;
    jt.glDisableClientState = AGL_glDisableClientState;
    jt.glDrawArrays = AGL_glDrawArrays;
    jt.glDrawElements = AGL_glDrawElements;
    jt.glDrawRangeElements = AGL_glDrawRangeElements;
    jt.glDeleteBuffersARB = AGL_glDeleteBuffersARB;
    jt.glEnable = AGL_glEnable;
    jt.glEnableClientState = AGL_glEnableClientState;
    jt.glEnd = AGL_glEnd;
    jt.glFlush = AGL_glFlush;
    jt.glLightfv = AGL_glLightfv;
    jt.glLightf = AGL_glLightf;
    jt.glLightModelfv = AGL_glLightModelfv;
    jt.glLightModelf = AGL_glLightModelf;
    jt.glLightModeli = AGL_glLightModeli;
    jt.glLoadIdentity = AGL_glLoadIdentity;
    jt.glMaterialfv = AGL_glMaterialfv;
    jt.glMaterialf = AGL_glMaterialf;
    jt.glMatrixMode = AGL_glMatrixMode;
    jt.glMultMatrixd = AGL_glMultMatrixd;
    jt.glMultMatrixf = AGL_glMultMatrixf;
    jt.glNormal3f = AGL_glNormal3f;
    jt.glNormal3fv = AGL_glNormal3fv;
    jt.glNormalPointer = AGL_glNormalPointer;
    jt.glOrtho = AGL_glOrtho;
    jt.glFrustum = AGL_glFrustum;
    jt.glPopMatrix = AGL_glPopMatrix;
    jt.glProgramLocalParameter4fARB = AGL_glProgramLocalParameter4fARB;
    jt.glProgramStringARB = AGL_glProgramStringARB;
    jt.glPushMatrix = AGL_glPushMatrix;
    jt.glRotatef = AGL_glRotatef;
    jt.glScalef = AGL_glScalef;
    jt.glShadeModel = AGL_glShadeModel;
    jt.glTranslatef = AGL_glTranslatef;
    jt.glTranslated = AGL_glTranslated;
    jt.glVertex3f = AGL_glVertex3f;
    jt.glVertex3fv = AGL_glVertex3fv;
    jt.glVertexPointer = AGL_glVertexPointer;
    jt.glViewport = AGL_glViewport;
    
    jt.glTexCoord2f = AGL_glTexCoord2f;
    jt.glTexParameteri = AGL_glTexParameteri;
    jt.glTexParameterf = AGL_glTexParameterf;
    jt.glBindTexture = AGL_glBindTexture;
    jt.glTexImage2D = AGL_glTexImage2D;
    jt.glLoadMatrixf = AGL_glLoadMatrixf;
    jt.glCompressedTexSubImage2DARB = AGL_glCompressedTexSubImage2DARB;
    jt.glCompressedTexImage2DARB = AGL_glCompressedTexImage2DARB;
    jt.glCompressedTexImage2D = AGL_glCompressedTexImage2D;
    jt.glTexEnvi = AGL_glTexEnvi;
    jt.glTexEnvf = AGL_glTexEnvf;
    jt.glTexEnvfv = AGL_glTexEnvfv;
    jt.glTexGenfv = AGL_glTexGenfv;
    jt.glTexGeni = AGL_glTexGeni;
    jt.glTexGenf = AGL_glTexGenf;
    jt.glActiveTextureARB = AGL_glActiveTextureARB;
    jt.glFogf = AGL_glFogf;
    jt.glFogi = AGL_glFogi;
    jt.glFogfv = AGL_glFogfv;
    jt.glFogiv = AGL_glFogiv;
    jt.glTexCoordPointer = AGL_glTexCoordPointer;
    jt.glBlendFunc = AGL_glBlendFunc;
    jt.glFrontFace = AGL_glFrontFace;
    jt.glCullFace = AGL_glCullFace;
    jt.glVertex2i = AGL_glVertex2i;
    jt.glVertex2f = AGL_glVertex2f;
    jt.glDisable =  AGL_glDisable;
    jt.glProgramLocalParameter4fvARB  = AGL_glProgramLocalParameter4fvARB ;
    jt.glEnableVertexAttribArrayARB  = AGL_glEnableVertexAttribArrayARB ;
    jt.glDisableVertexAttribArrayARB  = AGL_glDisableVertexAttribArrayARB ;
    jt.glVertexAttribPointerARB = AGL_glVertexAttribPointerARB;
    jt.glCopyTexImage2D = AGL_glCopyTexImage2D;
    jt.glClientActiveTextureARB = AGL_glClientActiveTextureARB;
    jt.glDepthMask = AGL_glDepthMask;
    jt.glRotated = AGL_glRotated;
  
    jt.glStencilFunc = AGL_glStencilFunc;
    jt.glStencilOp = AGL_glStencilOp;
    jt.glClearStencil = AGL_glClearStencil;
    jt.glStencilMask = AGL_glStencilMask;
    jt.glColorMask = AGL_glColorMask;
    jt.glAlphaFunc = AGL_glAlphaFunc;
    jt.glBlendEquation = AGL_glBlendEquation;
    jt.glBlendColor = AGL_glBlendColor;
    jt.glColorMaterial = AGL_glColorMaterial;
    jt.glScissor = AGL_glScissor;
    jt.glDepthRange = AGL_glDepthRange;
    jt.glPolygonOffset = AGL_glPolygonOffset;
    jt.glPushAttrib = AGL_glPushAttrib;
    jt.glPopAttrib = AGL_glPopAttrib;
    
    jt.glIndexPointer = AGL_glIndexPointer;
    jt.glEdgeFlagPointer = AGL_glEdgeFlagPointer;
    jt.glArrayElement = AGL_glArrayElement;
    jt.glInterleavedArrays = AGL_glInterleavedArrays;
    jt.glGetString = AGL_glGetString;
    jt.glProgramEnvParameter4fvARB  = AGL_glProgramEnvParameter4fvARB ;
    jt.glColor4ub = AGL_glColor4ub;
    jt.glColor4fv = AGL_glColor4fv;
    jt.glDeleteTextures = AGL_glDeleteTextures;
    jt.glPixelStorei = AGL_glPixelStorei;
    jt.glTexSubImage2D = AGL_glTexSubImage2D;
    jt.glCopyTexSubImage2D = AGL_glCopyTexSubImage2D;
    jt.glMultiTexCoord2f = AGL_glMultiTexCoord2f;
    jt.glMultiTexCoord2fv = AGL_glMultiTexCoord2fv;

    jt.glProgramLocalParameters4fvEXT = AGL_glProgramLocalParameters4fvEXT;
    jt.glProgramEnvParameters4fvEXT = AGL_glProgramEnvParameters4fvEXT;
    jt.glPolygonMode = AGL_glPolygonMode;
    jt.glBlendEquationEXT = AGL_glBlendEquationEXT;
    jt.glStencilOpSeparateATI = AGL_glStencilOpSeparateATI;
    jt.glTexImage3D = AGL_glTexImage3D;
    
}

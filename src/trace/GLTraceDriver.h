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



#ifndef _GLTRACEDRIVER_
    #define _GLTRACEDRIVER_

#include "GPUTypes.h"
#include "AGPTransaction.h"
#include "TraceDriverInterface.h"
#include "GPUDriver.h"
#include "GLExec.h"
#include "zfstream.h"

void privateSwapBuffers();

/**
 *
 *  GL Trace Driver class.
 *
 *  Generates AGP transactions for the ATTILA GPU from an OpenGL API trace file.
 *
 */
class GLTraceDriver : public TraceDriverInterface
{

private:

    GLExec gle; ///< OpenGL command manager ( executes ogl commands in a tracefile )
    GPUDriver* driver; ///< GPU Driver
    char *traceFile; ///< API trace file
    int initValue; ///< 0 if creation was ok, < 0 value if not
    u32bit startFrame;
    u32bit currentFrame;
    bool _useACD;

    void _setAGLFunctions();
 
public:

    /**
     *
     * GL Trace Driver constructor
     *
     * Creates a GL Trace Driver object and initializes it.
     *
     * @param traceFile The name of the API trace file from which to read 3D commands.
     *
     * @return An initialized GL Trace Driver object.
     *
     * @note Remains available for backwards compatibility (implicity sets bufferFile to "GLIBuffers.dat"),
     *       use TraceDriver constructor with 3 parameters instead
     */
    GLTraceDriver( char *traceFile, bool useACD, GPUDriver* driver, bool triangleSetupOnShader, u32bit startFrame = 0);

    /**
     * GL Trace driver constructor
     *
     * Creates a GL Trace Driver object and initializes it.
     *
     * @param traceFile The name of the API trace file from which to read 3D commands.
     *
     * @param bufferFile File containing all buffers ( binary ) required por
     *        executing the trace
     *
     * @return An initialized GL Trace Driver object.
     */
    GLTraceDriver( const char* traceFile, bool useACD, const char* bufferFile, const char* memFile, GPUDriver* driver,
                   bool triangleSetupOnShader, u32bit startFrame = 0);

    /**
     * Starts the trace driver.
     *
     * Verifies if a GL TraceDriver object is correctly created and available for use
     *
     * @return  0 if all is ok.
     *         -1 opengl functions cannot be loaded
     *         -2 if traceFile could not be opened
     *         -3 if bufferFile could not be opened
     */
    int startTrace();

    /**
     *
     *  Generates the next AGP transaction from the API trace file.
     *
     *  @return A pointer to the new AGP transaction, NULL if there are no more AGP transactions.
     *
     */
    gpu3d::AGPTransaction* nextAGPTransaction();

    
    /**
     *
     *  Returns the current position (line) inside the OpenGL trace file.
     *
     *  @return The current position inside the OpenGL trace file (line).
     *
     */

    u32bit getTracePosition();

    // NOT AVAILABLE IN THE PUBLIC INTERFACE
    /**
     *
     * Call the specified OpenGL library command encoded in a TraceElement
     *
     * Indirectly called by nextAGPTransactions if there are not AGPTransactions
     * in GPUDriver buffer
     *
     */
    //void performOGLLibraryCall( TraceElement* te );

};


#endif

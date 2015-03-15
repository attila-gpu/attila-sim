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
 * $RCSfile: GPULibInternals.h,v $
 * $Revision: 1.34 $
 * $Author: jroca $
 * $Date: 2007-06-22 15:36:44 $
 *
 * OpenGL API library internal implementation definitions.
 *
 */

#ifndef GPULIBINTERNALS_H
    #define GPULIBINTERNALS_H

//#define SHOW_WARNINGS
//#define PRINT_CALLS
#define ENABLE_FRAGMENT_SHADER

/*
#define ENABLE_TEXTURING

#ifdef ENABLE_TEXTURING
    #define TEXTURING_CODE(code) code
#else
    #define TEXTURING_CODE(code)
#endif
*/

#ifdef ENABLE_FRAGMENT_SHADER
    #define FSHADER_CODE(code) code
#else
    #define FSHADER_CODE(code)
#endif


#ifdef PRINT_CALLS
    #define PRINT_CALL(call) /*driver->printMemoryUsage();*/ cout << call;
#else
    #define PRINT_CALL(call)
#endif

#ifdef SHOW_WARNINGS
    #define WARNING_CODE(code) { code }
#else
    #define WARNING_CODE(code)
#endif

#include "gl.h"
#include "glext.h"
#include "GPUDriver.h"
#include "GLContext.h"
#include "BufferManager.h"
#include "TextureManager.h"
#include "ProgramManager.h"

extern u32bit frame;
#define START_FRAME 9

/**
 * Static object responsible of library initialization
 */
/*
class GPULibInitializer
{
private:

    GPULibInitializer( const GPULibInitializer& );

public:

    GPULibInitializer();
};
*/

// static GPULibInitializer gpuLibInitializer;

void initOGLLibrary(GPUDriver* driver, bool triangleSetupOnShader);

#define RESET_BEGIN_MODE_SUPPORT supportedMode = true;

#define SET_BATCH_MODE_SUPPORT(m)\
    if (m==GL_TRIANGLES || m==GL_TRIANGLE_FAN || m==GL_TRIANGLE_STRIP || m==GL_QUADS || m==GL_QUAD_STRIP)\
        supportedMode = true;\
    else supportedMode = false;

#define CHECK_BATCH_MODE_SUPPORT if (supportedMode==false) return ;
#define CHECK_BATCH_MODE_SUPPORT_MSG(msg) if (supportedMode==false) {cout << msg; return ;}


/**
 * Constants
 */
#define GPULIB_PI 3.1415926536
#define GPULIB_DEG2RAD (GPULIB_PI/180.0)

#define CHECK_NOT_BEGIN_MODE(func) \
    if ( ctx->testFlags(GLContext::flagBeginEnd) )\
        panic("GPULib",#func"()","Not allowed function in a glBegin/glEnd block");

#define CHECK_BEGIN_MODE(func) \
    if ( !ctx->testFlags(GLContext::flagBeginEnd) )\
        panic("GPULib",#func"()","Function must be called within a glBegin/glEnd block");


#define PRINTFR(frame,msg) if ( ctx->currentFrame() == frame ) cout << msg << endl;

#define ENABLE_USENORMALS ctx->setFlags(GLContext::flagUsingNormals)
#define IS_USENORMALS ctx->testFlags(GLContext::flagUsingNormals)


#define IS_BUFFER_BOUND(target) (BufferManager::instance().getTarget(target).hasCurrent())
#define GET_BUFFER(target) (BufferManager::instance().getTarget(target).getCurrent())
#define GET_ELEMENT_ARRAY_BUFFER GET_BUFFER(GL_ELEMENT_ARRAY_BUFFER)
#define GET_ARRAY_BUFFER GET_BUFFER(GL_ARRAY_BUFFER)
#define GET_BUFFER_BY_NAME(target,name) (BufferManager::instance().getBuffer(target,name))

#define GET_PROGRAM(target) (ProgramManager::instance().getTarget(target).getCurrent())
#define GET_VERTEX_PROGRAM GET_PROGRAM(GL_VERTEX_PROGRAM_ARB)
#define GET_FRAGMENT_PROGRAM GET_PROGRAM(GL_FRAGMENT_PROGRAM_ARB)

#define GET_TEXTURE(target) (TextureManager::instance().getTarget(target).getCurrent())
#define GET_TEXTURE_1D GET_TEXTURE(GL_TEXTURE_1D)
#define GET_TEXTURE_2D GET_TEXTURE(GL_TEXTURE_2D)
#define GET_TEXTURE_3D GET_TEXTURE(GL_TEXTURE_3D)
#define GET_TEXTURE_CUBE_MAP GET_TEXTURE(GL_TEXTURE_CUBE_MAP)

#define SET_LIB_DIRTY ctx->setLibStateDirty(true)
#define SET_LIB_NO_DIRTY ctx->setLibStateDirty(false)
#define IS_LIB_DIRTY ctx->isLibStateDirty()


class ClientDescriptorManager
{
public:

    static ClientDescriptorManager* instance();
    void save(const std::vector<u32bit>&  newClientDescriptors);
    void release();

private:

    ClientDescriptorManager();
    ClientDescriptorManager(const ClientDescriptorManager&);
    ClientDescriptorManager& operator=(const ClientDescriptorManager&);

    std::vector<u32bit> descs;
};


class LibBufferObjectManager
{
public:

    // Save current bind and enabled buffer
    void clear();
    void add(u32bit vboName);
    bool used(u32bit vboName);

private:

    std::vector<u32bit> vboNames;
};


void privateSwapBuffers();

/*
 * Driver access
 */
extern GPUDriver* driver;

/*
 * Current context ( only one context in this version )
 */
extern libgl::GLContext* ctx;


extern bool supportedMode;

extern std::vector<u32bit> prevDescriptors;

//extern ClientDescriptorManager* libClientDescriptors; // has a pointer to the unique GlobalMemoryDescriptorManager

//extern LibBufferObjectManager* libBufferObjects;

#endif

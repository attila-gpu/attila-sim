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
 * $RCSfile: GPULibInternals.cpp,v $
 * $Revision: 1.17 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:33:46 $
 *
 * OpenGL API library internal implementation.
 *
 */

#include "ProgramManager.h"
#include "GPULibInternals.h"
#include "VSLoader.h"
#include "Matrixf.h"

// Don't open namespace (it clashes with other enums used here)
//using namespace glsNS;

GPUDriver* driver = 0;
libgl::GLContext* ctx = 0;
bool supportedMode = true;
// ClientDescriptorManager* libClientDescriptors = ClientDescriptorManager::instance();
ClientDescriptorManager* libClientDescriptors = 0;

ClientDescriptorManager::ClientDescriptorManager()
{}

ClientDescriptorManager* ClientDescriptorManager::instance()
{
    static ClientDescriptorManager gmdm;
    return &gmdm;
}

void ClientDescriptorManager::save(const std::vector<u32bit>& memDescs)
{
    descs = memDescs;
}

void ClientDescriptorManager::release()
{
    std::vector<u32bit>::iterator it = descs.begin();
    for ( ; it != descs.end(); it++ )
        driver->releaseMemory(*it);
    descs.clear(); // throw away descriptors
}

void initOGLLibrary(GPUDriver* d, bool triangleSetupOnShader)
{
    driver = d;
    ctx = new libgl::GLContext(driver, triangleSetupOnShader);
    libClientDescriptors = ClientDescriptorManager::instance();
}


//  Our own swap buffer function.
void privateSwapBuffers()
{
//if (frame >= START_FRAME)
    //  Request a framebuffer swap to the driver.
    driver->sendCommand(gpu3d::GPU_SWAPBUFFERS);
//else
//printf("LIB >> Skipping frame %d\n", frame);

//frame++;
}

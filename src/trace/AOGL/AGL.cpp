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

#include "AGL.h"
#include "support.h"
#include <iostream>
#include "ACDMatrix.h"
#include <sstream>

// Define the global GLContext
agl::GLContext* agl::_ctx;

acdlib::ACD_PRIMITIVE agl::trPrimitive(GLenum primitive)
{
    switch ( primitive ) {
        case GL_POINTS:
            return acdlib::ACD_POINTS;
        case GL_LINES:
            return acdlib::ACD_LINES;
        case GL_LINE_STRIP:
            return acdlib::ACD_LINE_STRIP;
        case GL_LINE_LOOP:
            return acdlib::ACD_LINE_LOOP;
        case GL_TRIANGLES:
            return acdlib::ACD_TRIANGLES;
        case GL_TRIANGLE_STRIP:
            return acdlib::ACD_TRIANGLE_STRIP;
        case GL_TRIANGLE_FAN:
            return acdlib::ACD_TRIANGLE_FAN;
        case GL_QUADS:
            return acdlib::ACD_QUADS;
        case GL_QUAD_STRIP:
            return acdlib::ACD_QUAD_STRIP;
        case GL_POLYGON:
            panic("agl", "trPrimitive", "Attila does not support POLIGON PRIMITIVES directly (emulation not implemented)");
        default:
            panic("agl", "trPrimitive", "Other primitive than triangles not implemented yet");
    }
    return acdlib::ACD_TRIANGLES;

}

acdlib::ACD_INTERPOLATION_MODE agl::trInterpolationMode(GLenum iMode)
{
    switch ( iMode ) {
        case GL_SMOOTH:
            return acdlib::ACD_INTERPOLATION_LINEAR;
        case GL_FLAT:
            return acdlib::ACD_INTERPOLATION_NONE;
        default:
            panic("AGL.cpp", "acdlib::trInterpolationMode", "Unknown OpenGL interpolation mode");
            return acdlib::ACD_INTERPOLATION_NONE;
    }
}

void agl::initAGL(GPUDriver* driver, acdlib::acd_uint startFrame)
{
    using namespace acdlib;

    acdlib::ACDDevice* acddev = acdlib::createDevice(driver);

    acddev->setStartFrame(startFrame);

    _ctx = new GLContext(acddev);
    
    //cout << "*** initAGL completed ***" << endl;
}

void agl::setResolution(acdlib::acd_uint width, acdlib::acd_uint height)
{
    _ctx->acd().setResolution(width, height);
}


void agl::swapBuffers() { _ctx->acd().swapBuffers(); }


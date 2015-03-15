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

#ifndef AGL
    #define AGL

#include "gl.h"
#include "ACD.h"
#include "AGLContext.h"
#include "ACDRasterizationStage.h"

namespace agl
{
    // GL current context
    extern GLContext* _ctx;

    // Called by GLTraceDriver to initialize OpenGL library build on top of ACD
    // The starting frame is passed through to allow the tracking of the
    // current frame number in the library.
    void initAGL(GPUDriver* driver, acdlib::acd_uint startFrame);

    void setResolution(acdlib::acd_uint width, acdlib::acd_uint height);
    
    // Called by GLTraceDriver to perform a swap buffers on the acddev attached to the current GLContext
    void swapBuffers();

    acdlib::ACD_PRIMITIVE trPrimitive(GLenum primitive);

    // Must be moved to a common ACD place
    acdlib::acd_ubyte trClamp(acdlib::acd_float v);

    acdlib::ACD_INTERPOLATION_MODE trInterpolationMode(GLenum iMode);

}

#endif // AGL

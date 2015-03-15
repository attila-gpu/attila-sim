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

#ifndef AGL_GL2ACD
    #define AGL_GL2ACD

#include "gl.h"
#include "glext.h"
#include "ACDDevice.h"
#include "ACDBlendingStage.h"
#include "ACDRasterizationStage.h"
#include "ACDZStencilStage.h"
#include "ACDSampler.h"
#include "ACDStream.h"

namespace agl
{

    acdlib::ACD_BLEND_FUNCTION getACDBlendFunction(GLenum blendFunction);
    acdlib::ACD_BLEND_OPTION getACDBlendOption(GLenum blendOption);
    acdlib::ACD_CULL_MODE getACDCullMode(GLenum cullMode);
    acdlib::ACD_COMPARE_FUNCTION getACDDepthFunc(GLenum func);
    acdlib::ACD_TEXTURE_ADDR_MODE getACDTextureAddrMode(GLenum texAddrMode);
    acdlib::ACD_TEXTURE_FILTER getACDTextureFilter(GLenum texFilter);
    acdlib::ACD_COMPARE_FUNCTION getACDCompareFunc (GLenum compareFunc);
    acdlib::ACD_STENCIL_OP getACDStencilOp (GLenum texGenMode);
    acdlib::ACD_STREAM_DATA getACDStreamType (GLenum type, GLboolean normalized);
    acdlib::ACD_FORMAT getACDTextureFormat (GLenum acdFormat);
    acdlib::ACD_CUBEMAP_FACE getACDCubeMapFace(GLenum cubeFace);
    acdlib::ACD_FACE getACDFace(GLenum face);
    GLuint getACDTexelSize(GLenum format);

}

#endif


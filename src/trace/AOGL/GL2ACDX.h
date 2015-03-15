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

#ifndef AGL_GL2ACDX
    #define AGL_GL2ACDX

#include "gl.h"
#include "glext.h"
#include "ACDDevice.h"
#include "ACDX.h"

namespace agl
{

    acdlib::ACDX_TEXTURE_COORD getACDXTexCoordPlane(GLenum texCoordPlane);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_MODE getACDXFogMode(GLenum fogMode);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_COORDINATE_SOURCE getACDXFogCoordinateSource(GLenum fogCoordinate);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_STAGE_FUNCTION getACDXTextureStageFunction (GLenum texFunction);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_BASE_INTERNAL_FORMAT getACDXTextureInternalFormat (GLenum textureInternalFormat);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_TEXTURE_COORDINATE_MODE getACDXTexGenMode (GLenum texGenMode);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_COLOR_MATERIAL_ENABLED_FACE getACDXColorMaterialFace (GLenum face);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_COLOR_MATERIAL_MODE getACDXColorMaterialMode (GLenum mode);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_TEST_FUNCTION getACDXAlphaFunc(GLenum func);
    acdlib::ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_TARGET getACDXTextureTarget(GLenum target);
    acdlib::ACDX_COMBINE_SETTINGS::ACDX_COMBINE_FUNCTION getACDXCombinerFunction (GLenum combineFunction);
    acdlib::ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SOURCE getACDXCombineSource (GLenum combineSource);
    acdlib::ACDX_COMBINE_SETTINGS::ACDX_COMBINE_OPERAND getACDXCombineOperand (GLenum combineOperand);

}

#endif


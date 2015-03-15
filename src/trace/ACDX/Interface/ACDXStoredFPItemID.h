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

#ifndef ACDX_STORED_FP_ITEM_ID_H
    #define ACDX_STORED_FP_ITEM_ID_H

#include "ACDXFixedPipelineConstantLimits.h"

namespace acdlib
{

/**
 * Fixed Pipeline state identifiers
 */ 
enum ACDX_STORED_FP_ITEM_ID
{
    //////////////////////
    // Surface Material //
    //////////////////////

    ACDX_MATERIAL_FIRST_ID           = 0,

    ACDX_MATERIAL_FRONT_AMBIENT    = ACDX_MATERIAL_FIRST_ID,        ///< Material front ambient color
    ACDX_MATERIAL_FRONT_DIFFUSE    = ACDX_MATERIAL_FIRST_ID + 1,    ///< Material front diffuse color
    ACDX_MATERIAL_FRONT_SPECULAR   = ACDX_MATERIAL_FIRST_ID + 2,    ///< Material front specular color
    ACDX_MATERIAL_FRONT_EMISSION   = ACDX_MATERIAL_FIRST_ID + 3,    ///< Material front emission color
    ACDX_MATERIAL_FRONT_SHININESS  = ACDX_MATERIAL_FIRST_ID + 4,    ///< Material front shininess factor
    ACDX_MATERIAL_BACK_AMBIENT     = ACDX_MATERIAL_FIRST_ID + 5,    ///< Material back ambient color
    ACDX_MATERIAL_BACK_DIFFUSE     = ACDX_MATERIAL_FIRST_ID + 6,    ///< Material back diffuse color
    ACDX_MATERIAL_BACK_SPECULAR    = ACDX_MATERIAL_FIRST_ID + 7,    ///< Material back specular color
    ACDX_MATERIAL_BACK_EMISSION    = ACDX_MATERIAL_FIRST_ID + 8,    ///< Material back emission color
    ACDX_MATERIAL_BACK_SHININESS   = ACDX_MATERIAL_FIRST_ID + 9,    ///< Material back shininess factor
    
    ACDX_MATERIAL_PROPERTIES_COUNT = 10,

    /////////////////////////////////////
    // Colors and properties of lights //
    /////////////////////////////////////

    ACDX_LIGHT_FIRST_ID               = ACDX_MATERIAL_FIRST_ID + ACDX_MATERIAL_PROPERTIES_COUNT,

    ACDX_LIGHT_AMBIENT             = ACDX_LIGHT_FIRST_ID,        ///< Light ambient color
    ACDX_LIGHT_DIFFUSE             = ACDX_LIGHT_FIRST_ID + 1,    ///< Light diffuse color
    ACDX_LIGHT_SPECULAR            = ACDX_LIGHT_FIRST_ID + 2,    ///< Light specular color
    ACDX_LIGHT_POSITION            = ACDX_LIGHT_FIRST_ID + 3,    ///< Light position (point/spot lights only)
    ACDX_LIGHT_DIRECTION           = ACDX_LIGHT_FIRST_ID + 4,    ///< Light direction (directional lights only)
    ACDX_LIGHT_ATTENUATION         = ACDX_LIGHT_FIRST_ID + 5,    ///< Light attenuation (point/spot lights only)
    ACDX_LIGHT_SPOT_DIRECTION      = ACDX_LIGHT_FIRST_ID + 6,    ///< Light spot direction (spot lights only)
    ACDX_LIGHT_SPOT_CUTOFF           = ACDX_LIGHT_FIRST_ID + 7,    ///< Light spot cut-off cosine angle (Phi angle in D3D)
    ACDX_LIGHT_SPOT_EXPONENT       = ACDX_LIGHT_FIRST_ID + 8,    ///< Light spot attenuation exponent (Falloff factor in D3D)

    ACDX_LIGHT_PROPERTIES_COUNT    = 9,
    // ... Implicit enums for lights 1, 2, ..., up to ACDX_FP_MAX_LIGHTS_LIMIT - 1

    ////////////////////////////
    // Light model properties //
    ////////////////////////////

    ACDX_LIGHTMODEL_FIRST_ID         = ACDX_LIGHT_FIRST_ID + ACDX_LIGHT_PROPERTIES_COUNT * ACDX_FP_MAX_LIGHTS_LIMIT,
    
    ACDX_LIGHTMODEL_AMBIENT             = ACDX_LIGHTMODEL_FIRST_ID,        ///< Lightmodel ambient color
    ACDX_LIGHTMODEL_FRONT_SCENECOLOR = ACDX_LIGHTMODEL_FIRST_ID + 1,    ///< Lightmodel front face scence color
    ACDX_LIGHTMODEL_BACK_SCENECOLOR  = ACDX_LIGHTMODEL_FIRST_ID + 2,    ///< Lightmodel back face scene color

    ACDX_LIGHTMODEL_PROPERTIES_COUNT = 3,

    //////////////////////////////////////////
    // Texture coordinate generation planes //
    //////////////////////////////////////////

    ACDX_TEXGEN_FIRST_ID = ACDX_LIGHTMODEL_FIRST_ID + ACDX_LIGHTMODEL_PROPERTIES_COUNT,

    ACDX_TEXGEN_EYE_S     = ACDX_TEXGEN_FIRST_ID,        ///< Texture coordinate S Eye plane coeffients
    ACDX_TEXGEN_EYE_T     = ACDX_TEXGEN_FIRST_ID + 1,    ///< Texture coordinate T Eye plane coeffients
    ACDX_TEXGEN_EYE_R     = ACDX_TEXGEN_FIRST_ID + 2,    ///< Texture coordinate R Eye plane coeffients
    ACDX_TEXGEN_EYE_Q     = ACDX_TEXGEN_FIRST_ID + 3,    ///< Texture coordinate Q Eye plane coeffients
    ACDX_TEXGEN_OBJECT_S = ACDX_TEXGEN_FIRST_ID + 4,    ///< Texture coordinate S Object plane coeffients
    ACDX_TEXGEN_OBJECT_T = ACDX_TEXGEN_FIRST_ID + 5,    ///< Texture coordinate T Object plane coeffients
    ACDX_TEXGEN_OBJECT_R = ACDX_TEXGEN_FIRST_ID + 6,    ///< Texture coordinate R Object plane coeffients
    ACDX_TEXGEN_OBJECT_Q = ACDX_TEXGEN_FIRST_ID + 7,    ///< Texture coordinate Q Object plane coeffients
    // ... implicit enums for texgen 1, 2, ... up to ACDX_FP_MAX_TEXTURE_STAGES_LIMIT - 1
    
    ACDX_TEXGEN_PROPERTIES_COUNT = 8,

    ////////////////////
    // Fog properties //
    ////////////////////

    ACDX_FOG_FIRST_ID     = ACDX_TEXGEN_FIRST_ID + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT * ACDX_TEXGEN_PROPERTIES_COUNT,

    ACDX_FOG_COLOR        = ACDX_FOG_FIRST_ID,        ///< FOG combine color
    ACDX_FOG_DENSITY      = ACDX_FOG_FIRST_ID + 1,    ///< FOG exponent (only for non-linear modes)
    ACDX_FOG_LINEAR_START = ACDX_FOG_FIRST_ID + 2,    ///< FOG start position (only for linear mode)
    ACDX_FOG_LINEAR_END   = ACDX_FOG_FIRST_ID + 3,    ///< FOG end position (only for linear mode)

    ACDX_FOG_PROPERTIES_COUNT = 4,

    //////////////////////////////
    // Texture stage properties //
    //////////////////////////////

    ACDX_TEXENV_FIRST_ID = ACDX_FOG_FIRST_ID + ACDX_FOG_PROPERTIES_COUNT,

    ACDX_TEXENV_COLOR = ACDX_TEXENV_FIRST_ID, ///< Texture stage environment color
    // ... Implicit enums for texenv 1, 2, ..., up to ACDX_FP_MAX_TEXTURE_STAGES_LIMIT - 1

    ACDX_TEXENV_PROPERTIES_COUNT = 1,

    ////////////////////////////
    // Depth range properties //
    ////////////////////////////

    ACDX_DEPTH_FIRST_ID = ACDX_TEXENV_FIRST_ID + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT * ACDX_TEXENV_PROPERTIES_COUNT,

    ACDX_DEPTH_RANGE_NEAR = ACDX_DEPTH_FIRST_ID,    ///< The depth range near distance
    ACDX_DEPTH_RANGE_FAR = ACDX_DEPTH_FIRST_ID + 1,    ///< The depth range far distance

    ACDX_DEPTH_RANGE_PROPERTIES_COUNT = 2,

    ///////////////////////////
    // Alpha test properties //
    ///////////////////////////

    ACDX_ALPHA_TEST_FIRST_ID = ACDX_DEPTH_FIRST_ID + ACDX_DEPTH_RANGE_PROPERTIES_COUNT,
    
    ACDX_ALPHA_TEST_REF_VALUE = ACDX_ALPHA_TEST_FIRST_ID,    ///< The alpha test reference value

    ACDX_ALPHA_TEST_PROPERTIES_COUNT = 1,

    /////////////////////////////
    // Transformation matrices //
    /////////////////////////////

    ACDX_MATRIX_FIRST_ID = ACDX_MATERIAL_PROPERTIES_COUNT + ACDX_FP_MAX_LIGHTS_LIMIT * ACDX_LIGHT_PROPERTIES_COUNT + ACDX_LIGHTMODEL_PROPERTIES_COUNT + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT * ACDX_TEXGEN_PROPERTIES_COUNT + ACDX_FOG_PROPERTIES_COUNT + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT * ACDX_TEXENV_PROPERTIES_COUNT + ACDX_DEPTH_RANGE_PROPERTIES_COUNT + ACDX_ALPHA_TEST_PROPERTIES_COUNT,

    ACDX_M_MODELVIEW  = ACDX_MATRIX_FIRST_ID,    ///< The modelview transformation matrix
    // ... Implicit enums for modelview 1, 2, ..., up to ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT - 1
    ACDX_M_PROJECTION = ACDX_M_MODELVIEW + ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT, ///< The projection matrix
    ACDX_M_TEXTURE    = ACDX_M_PROJECTION + 1,  ///< The texture coordinate generation matrix

    ACDX_MATRIX_COUNT = ACDX_FP_MAX_MODELVIEW_MATRICES_LIMIT + 1 + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT,
    
    //////////////////////
    // Last Dummy state //
    //////////////////////

    ACDX_LAST_STATE = ACDX_MATERIAL_PROPERTIES_COUNT + ACDX_FP_MAX_LIGHTS_LIMIT * ACDX_LIGHT_PROPERTIES_COUNT + ACDX_LIGHTMODEL_PROPERTIES_COUNT + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT * ACDX_TEXGEN_PROPERTIES_COUNT + ACDX_FOG_PROPERTIES_COUNT + ACDX_FP_MAX_TEXTURE_STAGES_LIMIT * ACDX_TEXENV_PROPERTIES_COUNT + ACDX_DEPTH_RANGE_PROPERTIES_COUNT + ACDX_ALPHA_TEST_PROPERTIES_COUNT + ACDX_MATRIX_COUNT,
};

} // namespace acdlib

#endif // ACDX_STORED_FP_ITEM_ID_H

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

#ifndef ARB_IMPLEMENTATIONLIMITS
    #define ARB_IMPLEMENTATIONLIMITS

#include "gl.h"

namespace agl
{

// Library implementation Limits

static const GLenum MAX_PROGRAM_INSTRUCTIONS_ARB        = 128;
static const GLenum MAX_PROGRAM_PARAMETERS_ARB          = 96;
static const GLenum MAX_PROGRAM_ENV_PARAMETERS_ARB      = 96;
static const GLenum MAX_PROGRAM_LOCAL_PARAMETERS_ARB    = 96;


//static const GLenum MAX_PROGRAM_MATRICES_ARB          = 8;
static const GLenum MAX_PROGRAM_MATRICES_ARB            = 2;

static const GLenum MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB  = 1;
static const GLenum MAX_PROGRAM_TEMPORARIES_ARB         = 32;
static const GLenum MAX_PROGRAM_ATTRIBS_ARB             = 16;
static const GLenum MAX_PROGRAM_ADDRESS_REGISTERS_ARB   = 1;

// Native Implementation Limits (Not used)
static const GLenum MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB = 0;
static const GLenum MAX_PROGRAM_NATIVE_TEMPORARIES_ARB  = 0;
static const GLenum MAX_PROGRAM_NATIVE_PARAMETERS_ARB   = 0;
static const GLenum MAX_PROGRAM_NATIVE_ATTRIBS_ARB      = 0;
static const GLenum MAX_PROGRAM_NATIVE_ADDRESS_ARB      = 0;

static const GLenum MAX_VERTEX_ATTRIBS_ARB              = 16; 
static const GLenum MAX_VERTEX_UNITS_ARB                = 4;
static const GLenum MAX_TEXTURE_UNITS_ARB               = 16;

static const GLenum MAX_TOTAL_VERTEX_ATTRIBS_ARB = (MAX_VERTEX_ATTRIBS_ARB - 8 + MAX_TEXTURE_UNITS_ARB);
static const GLenum MAX_LIGHTS_ARB               = 8;
static const GLenum MAX_CLIP_PLANES_ARB          = 6;
//static const GLenum MAX_PALETTE_MATRICES_ARB   = 10;
static const GLenum MAX_PALETTE_MATRICES_ARB     = 3;

// Infinite supported
static const GLenum MAX_VERTEX_PROGRAM_OBJECTS_ARB = 0;

static const GLenum CONSTANT_BANK_SIZE          = MAX_PROGRAM_PARAMETERS_ARB;
static const GLenum STATE_BANK_SIZE             = MAX_PROGRAM_PARAMETERS_ARB;
static const GLenum ENV_PARAMETERS_BANK_SIZE    = MAX_PROGRAM_ENV_PARAMETERS_ARB;
static const GLenum LOCAL_PARAMETERS_BANK_SIZE  = MAX_PROGRAM_LOCAL_PARAMETERS_ARB;
static const GLenum TEMPORARIES_BANK_SIZE       = MAX_PROGRAM_TEMPORARIES_ARB;
static const GLenum ATTRIBS_BANK_SIZE           = MAX_PROGRAM_ATTRIBS_ARB;
static const GLenum VERTEX_RESULT_BANK_SIZE     = (MAX_VERTEX_ATTRIBS_ARB - 8);
static const GLenum ADDRESS_REGISTERS_BANK_SIZE = MAX_PROGRAM_ADDRESS_REGISTERS_ARB;

// Implementation GL State Offsets

static const GLenum BASE_MATERIAL     = 0;
static const GLenum BASE_LIGHTS       = 10;
static const GLenum BASE_LIGHTMODEL   = (10 + MAX_LIGHTS_ARB * 7);
static const GLenum BASE_LIGHTPROD    = (13 + MAX_LIGHTS_ARB * 7);
static const GLenum BASE_TEXGEN       = (13 + MAX_LIGHTS_ARB * 13);
static const GLenum BASE_FOG          = (13 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8);
static const GLenum BASE_CLIP         = (15 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8);
static const GLenum BASE_POINT        = (15 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8 + MAX_CLIP_PLANES_ARB);
static const GLenum BASE_TEXENV       = (17 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8 + MAX_CLIP_PLANES_ARB);
static const GLenum BASE_DEPTH        = (17 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 9 + MAX_CLIP_PLANES_ARB);
static const GLenum BASE_STATE_MATRIX = (18 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 9 + MAX_CLIP_PLANES_ARB);
/*
static const GLenum BASE_STATE_MATRIX = (17 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8 + MAX_CLIP_PLANES_ARB);
static const GLenum BASE_TEXENV       = (25 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 16 + MAX_CLIP_PLANES_ARB + MAX_VERTEX_UNITS_ARB + MAX_PALETTE_MATRICES_ARB * 4 + MAX_PROGRAM_MATRICES_ARB * 4);
static const GLenum BASE_DEPTH        = (25 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 16 + MAX_CLIP_PLANES_ARB + MAX_VERTEX_UNITS_ARB * 2 + MAX_PALETTE_MATRICES_ARB * 4 + MAX_PROGRAM_MATRICES_ARB * 4);
*/


static const GLenum MATERIAL_FRONT    = BASE_MATERIAL;
static const GLenum MATERIAL_BACK     = (BASE_MATERIAL + 5);

static const GLenum MATERIAL_AMBIENT_OFFSET    = 0;
static const GLenum MATERIAL_DIFFUSE_OFFSET    = 1;
static const GLenum MATERIAL_SPECULAR_OFFSET   = 2;
static const GLenum MATERIAL_EMISSION_OFFSET   = 3;
static const GLenum MATERIAL_SHINNINESS_OFFSET = 4;

static const GLenum LIGHT_AMBIENT_OFFSET       = 0;
static const GLenum LIGHT_DIFFUSE_OFFSET       = 1;
static const GLenum LIGHT_SPECULAR_OFFSET      = 2;
static const GLenum LIGHT_POSITION_OFFSET      = 3;
static const GLenum LIGHT_ATTENUATION_OFFSET   = 4;
static const GLenum LIGHT_SPOTDIRECTION_OFFSET = 5;
static const GLenum LIGHT_HALF_OFFSET          = 6;

static const GLenum LIGHTMODEL_AMBIENT          = (BASE_LIGHTMODEL);
static const GLenum LIGHTMODEL_FRONT_SCENECOLOR = (BASE_LIGHTMODEL + 1);
static const GLenum LIGHTMODEL_BACK_SCENECOLOR  = (BASE_LIGHTMODEL + 2);

static const GLenum LIGHTPROD_FRONT_OFFSET    = 0;
static const GLenum LIGHTPROD_BACK_OFFSET     = 3;

static const GLenum LIGHTPROD_AMBIENT_OFFSET  = 0;
static const GLenum LIGHTPROD_DIFFUSE_OFFSET  = 1;
static const GLenum LIGHTPROD_SPECULAR_OFFSET = 2;

static const GLenum TEXGEN_EYE_OFFSET         = 0;
static const GLenum TEXGEN_OBJECT_OFFSET      = 4;

static const GLenum TEXGEN_S_COORD_OFFSET     = 0;
static const GLenum TEXGEN_T_COORD_OFFSET     = 1;
static const GLenum TEXGEN_R_COORD_OFFSET     = 2;
static const GLenum TEXGEN_Q_COORD_OFFSET     = 3;

static const GLenum FOG_COLOR                 = BASE_FOG;
static const GLenum FOG_PARAMS                = (BASE_FOG + 1);

static const GLenum POINT_SIZE                = BASE_POINT;
static const GLenum POINT_ATTENUATION         = (BASE_POINT + 1);

static const GLenum STATE_MATRIX_MODELVIEW_OFFSET  = 0;
static const GLenum STATE_MATRIX_PROJECTION_OFFSET = (MAX_VERTEX_UNITS_ARB * 4);
static const GLenum STATE_MATRIX_MVP_OFFSET        = ((MAX_VERTEX_UNITS_ARB + 1) * 4);
static const GLenum STATE_MATRIX_TEXTURE_OFFSET    = ((MAX_VERTEX_UNITS_ARB + 2) * 4);
static const GLenum STATE_MATRIX_PALETTE_OFFSET    = ((MAX_VERTEX_UNITS_ARB + 2 + MAX_TEXTURE_UNITS_ARB) * 4);
static const GLenum STATE_MATRIX_PROGRAM_OFFSET    = ((MAX_VERTEX_UNITS_ARB + 2 + MAX_TEXTURE_UNITS_ARB + MAX_PALETTE_MATRICES_ARB) * 4);

static const GLenum STATE_MATRIX_NORMAL_OFFSET     = 0;
static const GLenum STATE_MATRIX_INVERSE_OFFSET    = 1;
static const GLenum STATE_MATRIX_TRANSPOSE_OFFSET  = 2;
static const GLenum STATE_MATRIX_INVTRANS_OFFSET   = 3;

static const GLenum DEPTH_RANGE_OFFSET             = 0;

} // namespace agl


#endif // ARB_IMPLEMENTATIONLIMITS

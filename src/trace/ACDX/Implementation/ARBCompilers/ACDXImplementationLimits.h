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

#ifndef ACDX_IMPLEMENTATIONLIMITS_H
    #define ACDX_IMPLEMENTATIONLIMITS_H

/* Library implementation Limits */
#define MAX_PROGRAM_INSTRUCTIONS_ARB        128
#define MAX_PROGRAM_PARAMETERS_ARB          250
#define MAX_PROGRAM_ENV_PARAMETERS_ARB      96
#define MAX_PROGRAM_LOCAL_PARAMETERS_ARB    96

//#define MAX_PROGRAM_MATRICES_ARB          8
#define MAX_PROGRAM_MATRICES_ARB            2

#define MAX_PROGRAM_MATRIX_STACK_DEPTH_ARB  1
#define MAX_PROGRAM_TEMPORARIES_ARB         32
#define MAX_PROGRAM_ATTRIBS_ARB             16
#define MAX_PROGRAM_ADDRESS_REGISTERS_ARB   1

/* Native Implementation Limits */
#define MAX_PROGRAM_NATIVE_INSTRUCTIONS_ARB
#define MAX_PROGRAM_NATIVE_TEMPORARIES_ARB
#define MAX_PROGRAM_NATIVE_PARAMETERS_ARB
#define MAX_PROGRAM_NATIVE_ATTRIBS_ARB
#define MAX_PROGRAM_NATIVE_ADDRESS_ARB

#define MAX_VERTEX_ATTRIBS_ARB          16 /* Include the minimum texture units */
#define MAX_VERTEX_UNITS_ARB            4
#define MAX_TEXTURE_UNITS_ARB           16

#define MAX_TOTAL_VERTEX_ATTRIBS_ARB    (MAX_VERTEX_ATTRIBS_ARB - 8 + MAX_TEXTURE_UNITS_ARB)
#define MAX_LIGHTS_ARB                  8
#define MAX_CLIP_PLANES_ARB             6
//#define MAX_PALETTE_MATRICES_ARB      10
#define MAX_PALETTE_MATRICES_ARB        3

#define MAX_VERTEX_PROGRAM_OBJECTS_ARB

#define CONSTANT_BANK_SIZE              MAX_PROGRAM_PARAMETERS_ARB
#define STATE_BANK_SIZE                 MAX_PROGRAM_PARAMETERS_ARB
#define ENV_PARAMETERS_BANK_SIZE        MAX_PROGRAM_ENV_PARAMETERS_ARB
#define LOCAL_PARAMETERS_BANK_SIZE      MAX_PROGRAM_LOCAL_PARAMETERS_ARB
#define TEMPORARIES_BANK_SIZE           MAX_PROGRAM_TEMPORARIES_ARB
#define ATTRIBS_BANK_SIZE               MAX_PROGRAM_ATTRIBS_ARB
#define VERTEX_RESULT_BANK_SIZE         (MAX_VERTEX_ATTRIBS_ARB - 8)
#define ADDRESS_REGISTERS_BANK_SIZE     MAX_PROGRAM_ADDRESS_REGISTERS_ARB


/* Implementation GL State Offsets */

#define BASE_MATERIAL                   0
#define BASE_LIGHTS                     10
#define BASE_LIGHTMODEL                 (10 + MAX_LIGHTS_ARB * 7)
#define BASE_LIGHTPROD                  (13 + MAX_LIGHTS_ARB * 7)
#define BASE_TEXGEN                     (13 + MAX_LIGHTS_ARB * 13)
#define BASE_FOG                        (13 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8)
#define BASE_CLIP                       (15 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8)
#define BASE_POINT                      (15 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8 + MAX_CLIP_PLANES_ARB)
#define BASE_TEXENV                     (17 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8 + MAX_CLIP_PLANES_ARB)
#define BASE_DEPTH                      (17 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 9 + MAX_CLIP_PLANES_ARB)
#define BASE_STATE_MATRIX               (18 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 9 + MAX_CLIP_PLANES_ARB)
/*
#define BASE_STATE_MATRIX               (17 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 8 + MAX_CLIP_PLANES_ARB)
#define BASE_TEXENV                     (25 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 16 + MAX_CLIP_PLANES_ARB + MAX_VERTEX_UNITS_ARB + MAX_PALETTE_MATRICES_ARB * 4 + MAX_PROGRAM_MATRICES_ARB * 4)
#define BASE_DEPTH                      (25 + MAX_LIGHTS_ARB * 13 + MAX_TEXTURE_UNITS_ARB * 16 + MAX_CLIP_PLANES_ARB + MAX_VERTEX_UNITS_ARB * 2 + MAX_PALETTE_MATRICES_ARB * 4 + MAX_PROGRAM_MATRICES_ARB * 4)
*/

#define MATERIAL_FRONT                  BASE_MATERIAL
#define MATERIAL_BACK                   (BASE_MATERIAL + 5)

#define MATERIAL_AMBIENT_OFFSET     0
#define MATERIAL_DIFFUSE_OFFSET     1
#define MATERIAL_SPECULAR_OFFSET    2
#define MATERIAL_EMISSION_OFFSET    3
#define MATERIAL_SHINNINESS_OFFSET  4

#define LIGHT_AMBIENT_OFFSET        0
#define LIGHT_DIFFUSE_OFFSET        1
#define LIGHT_SPECULAR_OFFSET       2
#define LIGHT_POSITION_OFFSET       3
#define LIGHT_ATTENUATION_OFFSET    4
#define LIGHT_SPOTDIRECTION_OFFSET  5
#define LIGHT_HALF_OFFSET           6


#define LIGHTMODEL_AMBIENT          (BASE_LIGHTMODEL)
#define LIGHTMODEL_FRONT_SCENECOLOR (BASE_LIGHTMODEL + 1)
#define LIGHTMODEL_BACK_SCENECOLOR  (BASE_LIGHTMODEL + 2)

#define LIGHTPROD_FRONT_OFFSET      0
#define LIGHTPROD_BACK_OFFSET       3

#define LIGHTPROD_AMBIENT_OFFSET    0
#define LIGHTPROD_DIFFUSE_OFFSET    1
#define LIGHTPROD_SPECULAR_OFFSET   2

#define TEXGEN_EYE_OFFSET           0
#define TEXGEN_OBJECT_OFFSET        4

#define TEXGEN_S_COORD_OFFSET       0
#define TEXGEN_T_COORD_OFFSET       1
#define TEXGEN_R_COORD_OFFSET       2
#define TEXGEN_Q_COORD_OFFSET       3

#define FOG_COLOR                   BASE_FOG
#define FOG_PARAMS                  (BASE_FOG + 1)

#define POINT_SIZE                  BASE_POINT
#define POINT_ATTENUATION           (BASE_POINT + 1)

#define STATE_MATRIX_MODELVIEW_OFFSET   0
#define STATE_MATRIX_PROJECTION_OFFSET  (MAX_VERTEX_UNITS_ARB * 4)
#define STATE_MATRIX_MVP_OFFSET         ((MAX_VERTEX_UNITS_ARB + 1) * 4)
#define STATE_MATRIX_TEXTURE_OFFSET     ((MAX_VERTEX_UNITS_ARB + 2) * 4)
#define STATE_MATRIX_PALETTE_OFFSET     ((MAX_VERTEX_UNITS_ARB + 2 + MAX_TEXTURE_UNITS_ARB) * 4)
#define STATE_MATRIX_PROGRAM_OFFSET     ((MAX_VERTEX_UNITS_ARB + 2 + MAX_TEXTURE_UNITS_ARB + MAX_PALETTE_MATRICES_ARB) * 4)

#define STATE_MATRIX_NORMAL_OFFSET      0
#define STATE_MATRIX_INVERSE_OFFSET     1
#define STATE_MATRIX_TRANSPOSE_OFFSET   2
#define STATE_MATRIX_INVTRANS_OFFSET    3

#define DEPTH_RANGE_OFFSET              0

#endif // ACDX_IMPLEMENTATIONLIMITS_H

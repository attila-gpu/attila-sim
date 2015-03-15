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

#ifndef ACD_STORED_ITEM_ID_H
    #define ACD_STORED_ITEM_ID_H

#include "ACDConstantLimits.h"
#include "GPU.h"

namespace acdlib
{

/**
 * Storable/restorable state identifiers
 */ 
enum ACD_STORED_ITEM_ID
{
    ///////////////////
    // Rasterization //
    ///////////////////

    ACD_RASTER_FIRST_ID = 0,
    
    ACD_RASTER_FILLMODE         =   ACD_RASTER_FIRST_ID,
    ACD_RASTER_CULLMODE         =   ACD_RASTER_FIRST_ID + 1,
    ACD_RASTER_VIEWPORT         =   ACD_RASTER_FIRST_ID + 2,
    ACD_RASTER_FACEMODE         =   ACD_RASTER_FIRST_ID + 3,
    ACD_RASTER_INTERPOLATION    =   ACD_RASTER_FIRST_ID + 4,
    ACD_RASTER_SCISSOR_TEST     =   ACD_RASTER_INTERPOLATION + gpu3d::MAX_FRAGMENT_ATTRIBUTES,
    ACD_RASTER_SCISSOR_X        =   ACD_RASTER_SCISSOR_TEST + 1,
    ACD_RASTER_SCISSOR_Y        =   ACD_RASTER_SCISSOR_TEST + 2,
    ACD_RASTER_SCISSOR_WIDTH    =   ACD_RASTER_SCISSOR_TEST + 3,
    ACD_RASTER_SCISSOR_HEIGHT   =   ACD_RASTER_SCISSOR_TEST + 4,

    ACD_RASTER_PROPERTIES_COUNT = 10 + gpu3d::MAX_FRAGMENT_ATTRIBUTES,

    ACD_RASTER_LAST =   ACD_RASTER_FIRST_ID + ACD_RASTER_PROPERTIES_COUNT,


    //////////////
    // ZStencil //
    //////////////

    ACD_ZST_FIRST_ID = ACD_RASTER_LAST,

    ACD_ZST_Z_ENABLED                   =   ACD_ZST_FIRST_ID,
    ACD_ZST_Z_FUNC                      =   ACD_ZST_FIRST_ID + 1,
    ACD_ZST_Z_MASK                      =   ACD_ZST_FIRST_ID + 2,
    ACD_ZST_STENCIL_ENABLED             =   ACD_ZST_FIRST_ID + 3,
    ACD_ZST_STENCIL_BUFFER_DEF          =   ACD_ZST_FIRST_ID + 4,
    ACD_ZST_FRONT_STENCIL_OPS           =   ACD_ZST_FIRST_ID + 5,
    ACD_ZST_FRONT_COMPARE_FUNC          =   ACD_ZST_FIRST_ID + 6,
    ACD_ZST_FRONT_STENCIL_REF_VALUE     =   ACD_ZST_FIRST_ID + 7,
    ACD_ZST_FRONT_STENCIL_COMPARE_MASK  =   ACD_ZST_FIRST_ID + 8,
    ACD_ZST_BACK_STENCIL_OPS            =   ACD_ZST_FIRST_ID + 9,
    ACD_ZST_BACK_COMPARE_FUNC           =   ACD_ZST_FIRST_ID + 10,
    ACD_ZST_BACK_STENCIL_REF_VALUE      =   ACD_ZST_FIRST_ID + 11,
    ACD_ZST_BACK_STENCIL_COMPARE_MASK   =   ACD_ZST_FIRST_ID + 12,
    ACD_ZST_RANGE_NEAR                  =   ACD_ZST_FIRST_ID + 13,
    ACD_ZST_RANGE_FAR                   =   ACD_ZST_FIRST_ID + 14,
    ACD_ZST_D3D9_DEPTH_RANGE            =   ACD_ZST_FIRST_ID + 15,
    ACD_ZST_SLOPE_FACTOR                =   ACD_ZST_FIRST_ID + 16, 
    ACD_ZST_UNIT_OFFSET                 =   ACD_ZST_FIRST_ID + 17,
    ACD_ZST_DEPTH_UPDATE_MASK           =   ACD_ZST_FIRST_ID + 18, 

    ACD_ZST_PROPERTIES_COUNT = 18,

    ACD_ZST_LAST = ACD_ZST_FIRST_ID + ACD_ZST_PROPERTIES_COUNT,


    //////////////
    // Blending //
    //////////////
    
    ACD_BLENDING_FIRST_ID       =   ACD_ZST_LAST,

    ACD_BLENDING_ENABLED        =   ACD_BLENDING_FIRST_ID,
    // ... Implicit enums for render targets 1, 2, ..., up to ACD_MAX_RENDER_TARGETS - 1

    ACD_BLENDING_SRC_RGB                =       ACD_BLENDING_FIRST_ID + ACD_MAX_RENDER_TARGETS,
    ACD_BLENDING_DST_RGB                =       ACD_BLENDING_SRC_RGB + 1,
    ACD_BLENDING_FUNC_RGB               =       ACD_BLENDING_SRC_RGB + 2,
    ACD_BLENDING_SRC_ALPHA              =       ACD_BLENDING_SRC_RGB + 3,
    ACD_BLENDING_DST_ALPHA              =       ACD_BLENDING_SRC_RGB + 4,
    ACD_BLENDING_FUNC_ALPHA             =       ACD_BLENDING_SRC_RGB + 5,
    ACD_BLENDING_COLOR                  =       ACD_BLENDING_SRC_RGB + 6,
    ACD_BLENDING_COLOR_WRITE_ENABLED    =       ACD_BLENDING_SRC_RGB + 7,
    ACD_BLENDING_COLOR_MASK_R           =       ACD_BLENDING_SRC_RGB + 8,
    ACD_BLENDING_COLOR_MASK_G           =       ACD_BLENDING_SRC_RGB + 9,
    ACD_BLENDING_COLOR_MASK_B           =       ACD_BLENDING_SRC_RGB + 10,
    ACD_BLENDING_COLOR_MASK_A           =       ACD_BLENDING_SRC_RGB + 11,

    ACD_BLENDING_PROPERTIES_COUNT = 12 + ACD_MAX_RENDER_TARGETS,

    ACD_BLENDING_LAST = ACD_BLENDING_FIRST_ID + ACD_BLENDING_PROPERTIES_COUNT,

    //////////////
    //  Stream  //
    //////////////

    ACD_STREAM_FIRST_ID     =   ACD_BLENDING_LAST,

    ACD_STREAM_ELEMENTS     =   ACD_STREAM_FIRST_ID,
    ACD_STREAM_FREQUENCY    =   ACD_STREAM_FIRST_ID + 1,
    ACD_STREAM_DATA_TYPE    =   ACD_STREAM_FIRST_ID + 2,
    ACD_STREAM_D3D9_COLOR   =   ACD_STREAM_FIRST_ID + 3,
    ACD_STREAM_STRIDE       =   ACD_STREAM_FIRST_ID + 4,
    ACD_STREAM_BUFFER       =   ACD_STREAM_FIRST_ID + 5,

    ACD_STREAM_PROPERTIES_COUNT = 5,

    ACD_STREAM_LAST = ACD_STREAM_FIRST_ID + ACD_STREAM_PROPERTIES_COUNT,



    //////////////
    // Sampler  //
    //////////////

    ACD_SAMPLER_FIRST_ID     =  ACD_STREAM_LAST,

    ACD_SAMPLER_ENABLED         =   ACD_SAMPLER_FIRST_ID,
    ACD_SAMPLER_CLAMP_S         =   ACD_SAMPLER_FIRST_ID + 1,
    ACD_SAMPLER_CLAMP_T         =   ACD_SAMPLER_FIRST_ID + 2,
    ACD_SAMPLER_CLAMP_R         =   ACD_SAMPLER_FIRST_ID + 3,
    ACD_SAMPLER_NON_NORMALIZED  =   ACD_SAMPLER_FIRST_ID + 4,
    ACD_SAMPLER_MIN_FILTER      =   ACD_SAMPLER_FIRST_ID + 5,
    ACD_SAMPLER_MAG_FILTER      =   ACD_SAMPLER_FIRST_ID + 6,
    ACD_SAMPLER_ENABLE_COMP     =   ACD_SAMPLER_FIRST_ID + 7,
    ACD_SAMPLER_COMP_FUNCTION   =   ACD_SAMPLER_FIRST_ID + 8,
    ACD_SAMPLER_SRGB_CONVERSION =   ACD_SAMPLER_FIRST_ID + 9,
    ACD_SAMPLER_MIN_LOD         =   ACD_SAMPLER_FIRST_ID + 10,
    ACD_SAMPLER_MAX_LOD         =   ACD_SAMPLER_FIRST_ID + 11,
    ACD_SAMPLER_MAX_ANISO       =   ACD_SAMPLER_FIRST_ID + 12,
    ACD_SAMPLER_LOD_BIAS        =   ACD_SAMPLER_FIRST_ID + 13,
    ACD_SAMPLER_UNIT_LOD_BIAS   =   ACD_SAMPLER_FIRST_ID + 14,
    ACD_SAMPLER_TEXTURE         =   ACD_SAMPLER_FIRST_ID + 15,

    ACD_SAMPLER_PROPERTIES_COUNT = 15,

    ACD_SAMPLER_LAST = ACD_SAMPLER_FIRST_ID + ACD_SAMPLER_PROPERTIES_COUNT,



    //////////////
    //  Device  //
    //////////////

    ACD_DEVICE_FIRST_ID    =    ACD_SAMPLER_LAST,
    
    ACD_DEV_PRIMITIVE                   =   ACD_DEVICE_FIRST_ID,    
    ACD_DEV_CURRENT_COLOR               =   ACD_DEVICE_FIRST_ID + 1,
    ACD_DEV_HIERARCHICALZ               =   ACD_DEVICE_FIRST_ID + 2,
    ACD_DEV_STREAM_START                =   ACD_DEVICE_FIRST_ID + 3,
    ACD_DEV_STREAM_COUNT                =   ACD_DEVICE_FIRST_ID + 4,
    ACD_DEV_STREAM_INSTANCES            =   ACD_DEVICE_FIRST_ID + 5,
    ACD_DEV_INDEX_MODE                  =   ACD_DEVICE_FIRST_ID + 6,
    ACD_DEV_EARLYZ                      =   ACD_DEVICE_FIRST_ID + 7,
    ACD_DEV_VERTEX_THREAD_RESOURCES     =   ACD_DEVICE_FIRST_ID + 8,
    ACD_DEV_FRAGMENT_THREAD_RESOURCES   =   ACD_DEVICE_FIRST_ID + 9,
    ACD_DEV_VSH                         =   ACD_DEVICE_FIRST_ID + 10,
    ACD_DEV_FSH                         =   ACD_DEVICE_FIRST_ID + 11,
    ACD_DEV_FRONT_BUFFER                =   ACD_DEVICE_FIRST_ID + 12,
    ACD_DEV_BACK_BUFFER                 =   ACD_DEVICE_FIRST_ID + 13,
    ACD_DEV_VERTEX_ATTRIBUTE_MAP        =   ACD_DEVICE_FIRST_ID + 14, 
    ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE     =   ACD_DEV_VERTEX_ATTRIBUTE_MAP + gpu3d::MAX_VERTEX_ATTRIBUTES ,
    ACD_DEV_FRAGMENT_INPUT_ATTRIBUTES   =   ACD_DEV_VERTEX_OUTPUT_ATTRIBUTE + gpu3d::MAX_VERTEX_ATTRIBUTES,


    ACD_DEV_PROPERTIES_COUNT = 15 + gpu3d::MAX_VERTEX_ATTRIBUTES + gpu3d::MAX_VERTEX_ATTRIBUTES + gpu3d::MAX_FRAGMENT_ATTRIBUTES,

    ACD_DEV_LAST = ACD_DEVICE_FIRST_ID + ACD_DEV_PROPERTIES_COUNT,
    // ... Put here new state item identifiers

    ACD_LAST_STATE = ACD_DEV_LAST,
};

} // namespace acdlib

#endif // ACD_STORED_ITEM_ID_H

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

#ifndef ACDX_FIXED_PIPELINE_SETTINGS_H
    #define ACDX_FIXED_PIPELINE_SETTINGS_H

#include "ACDXGlobalTypeDefinitions.h"
#include "ACDXFixedPipelineConstantLimits.h"

namespace acdlib
{
/**
 * Notes on Direct3D Fixed Pipeline compatibility:
 *
 * The following D3D Fixed Function Pipeline properties are not still supported
 * by the FF emulation engine:
 *
 *    1. The "Theta" angle in the spot/cone lights. Therefore, spot attenuation 
 *       starts at the cone center (Theta = 0.0).
 *
 *    2. The Falloff factor in D3D spot lights corresponds exactly to the 
 *       "Spot exponent" of Transform & Lighting State interface.
 *
 *    3. The "Range" distance of point lights. Therefore, there is not 
 *       cut-off distance in point lights.
 *
 *  4. The "Enable Specular lighting" property, that allows enabling/disabling
 *     the per-vertex specular computation in the vertex shader as
 *       long as the specular component of light won´t be used to form the final 
 *       color of the surface. The unalike "Separate Specular" property inherited 
 *       from the OpenGL Pipeline allows the specular contribution of lights to be
 *       added to the final fragment color after texturing.
 *
 * Some unknown parameters in the Direct3D world:
 *
 *    1. The "twoSidedLighting" parameter stands for a second/complementary 
 *       color computation for back faced polygons. The computation 
 *       is the same as front-faced polygons but using the opposite normal vector. 
 *       Thus, if two sided lighting is enabled, two surface colors are actually 
 *       computed for each vertex: the colors for either the front face and the 
 *     back faces that the vertex belongs to.
 *
 *    2. TO EXPLAIN: The difference in the Culling modes and OpenGL equivalence.
 *
 *  3. TO EXPLAIN: The texture stage combine functions and NVIDIA and ATI extensions supported.
 */

/**
 * The ACDX_COMBINE_SETTINGS struct
 *
 * Defines the combine function settings
 */
struct ACDX_COMBINE_SETTINGS
{
    /**
     * The combine function
     */
    enum ACDX_COMBINE_FUNCTION 
    { 
        ACDX_COMBINE_REPLACE, ///< Replace with the current stage sampled value
        ACDX_COMBINE_MODULATE, ///< Modulate the previous stage color with the current sampled one
        ACDX_COMBINE_ADD, ///< Add the previous stage and the current sampled color
        ACDX_COMBINE_ADD_SIGNED, 
        ACDX_COMBINE_INTERPOLATE, 
        ACDX_COMBINE_SUBTRACT,
        ACDX_COMBINE_DOT3_RGB, ///< Only applicable to Combine RGB function
        ACDX_COMBINE_DOT3_RGBA,///< Only applicable to Combine RGB function
        
        ACDX_COMBINE_MODULATE_ADD, ///< MODULATE_ADD function included in the "ATI_texture_env_combine3" extension.
        ACDX_COMBINE_MODULATE_SIGNED_ADD, ///< MODULATED_SIGNED_ADD function included in the "ATI_texture_env_combine3" extension.
        ACDX_COMBINE_MODULATE_SUBTRACT, ///< MODULATE_SUBTRACT function included in the "ATI_texture_env_combine3" extension.
    };

    /**
     * The combine sources
     */
    enum ACDX_COMBINE_SOURCE
    {
        ACDX_COMBINE_ZERO,  ///<  ZERO combine source included in the "NV_texture_env_combine4" extension.
        ACDX_COMBINE_ONE,   ///<  ONE combine source included in the "ATI_texture_env_combine3" extension.
        ACDX_COMBINE_TEXTURE,
        ACDX_COMBINE_TEXTUREn,
        ACDX_COMBINE_CONSTANT,
        ACDX_COMBINE_PRIMARY_COLOR,
        ACDX_COMBINE_PREVIOUS,
    };

    /**
     * The combine source operand/modifier
     */
    enum ACDX_COMBINE_OPERAND
    {
        ACDX_COMBINE_SRC_COLOR,
        ACDX_COMBINE_ONE_MINUS_SRC_COLOR,
        ACDX_COMBINE_SRC_ALPHA,
        ACDX_COMBINE_ONE_MINUS_SRC_ALPHA
    };

    ACDX_COMBINE_FUNCTION RGBFunction, ALPHAFunction; ///< The combine functions for RGB and Alpha components

    ACDX_COMBINE_SOURCE srcRGB[4], srcALPHA[4]; ///< 4th source extended for "NV_texture_env_combine4" extension support.
    ACDX_COMBINE_OPERAND operandRGB[4], operandALPHA[4]; ///< 4th source extended for "NV_texture_env_combine4" extension support.

    acd_int srcRGB_texCrossbarReference[4]; ///< The RGB crossbar reference (What the stage source number when specifying ACDX_COMBINE_TEXTUREn as RGB combine source).
    acd_int srcALPHA_texCrossbarReference[4]; ///< The ALPHA crossbar reference (What the stage source number when specifying ACDX_COMBINE_TEXTUREn as ALPHA combine source).

    acd_int RGBScale; ///< The Final RGB component scale factor.
    acd_int ALPHAScale; ///< The Final Alpha component scale factor.

};

/**
 * The ACDX_FIXED_PIPELINE_SETTINGS struct.
 * 
 * This struct is used as input of the ACDXGeneratePrograms() function
 * to generate the shader programs that emulate the Fixed Pipeline. 
 * This struct gathers all the fixed pipeline settings concerning states
 * for which a single feature change requires rebuilding of both generated programs.
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @version 0.8
 * @date 03/09/2007
 */
struct ACDX_FIXED_PIPELINE_SETTINGS
{
    ///////////////////////
    // Lighting settings //
    ///////////////////////

    acd_bool lightingEnabled;    ///< Vertex Lighting is enabled (if disabled the vertex color attribute is copied as vertex color output)
    acd_bool localViewer;        ///< The view point is not located at an infinite distance (enabling this feature increases the vertex computation cost)
    
    enum ACDX_NORMALIZATION_MODE
    { 
        ACDX_NO_NORMALIZE,    ///< The vertex normal is used directly for lighting computations
        ACDX_RESCALE,        ///< The vertex normal is rescaled (this is a cheaper normalization mode but is only applicable to modelview transformation matrices not distortioning objects to any dimension)
        ACDX_NORMALIZE,        ///< The full cost vertex normal normalization.
    
    } normalizeNormals; ///< The normal normalization mode

    acd_bool separateSpecular;    ///< Computes separately the specular contribution of lights in order to be added after surface fragment texturing.
    acd_bool twoSidedLighting;  ///< Computes colors for each front and back faces that the vertex belongs to

    static const acd_uint ACDX_MAX_LIGHTS = ACDX_FP_MAX_LIGHTS_LIMIT;    ///< Max number of lights supported by the implementation

    /**
     * The ACDX_LIGHT_SETTINGS struct defines the settings for a light source
     */
    struct ACDX_LIGHT_SETTINGS
    {
        acd_bool enabled;    ///< Light enabled/disabled.

        enum ACDX_LIGHT_TYPE 
        { 
            ACDX_DIRECTIONAL,    ///< The light is at an infinite distance from the scene, so, the light rays incide all the surface points from the same direction.
            ACDX_POINT,            ///< The light is at a not infinite concrete coordinate in the scene, so, the light rays incide from different directions to each surface point.
            ACDX_SPOT,            ///< A point light with the light rays constrained to the cut-off angle of a cone pointing in a concrete direction (the spot direction).
        
        } lightType;    ///< The light type
    };

    ACDX_LIGHT_SETTINGS lights[ACDX_MAX_LIGHTS];    ///< The array of lights

    //////////////////////
    // Culling Settings //
    //////////////////////

    acd_bool cullEnabled;    ///< Face culling is enabled/disabled.

    enum ACDX_CULL_MODE 
    { 
        ACDX_CULL_NONE,                ///< Do not cull any face. The same as culling disabled
        ACDX_CULL_FRONT,            ///< Front face culling. Front faces are those specified in the counter-clock wise order by default.
        ACDX_CULL_BACK,                ///< Back face culling. Back faces are those specified in the clock wise order by default.
        ACDX_CULL_FRONT_AND_BACK,    ///< Cull both faces.
    
    } cullMode;

    /////////////////////////////
    // Color Material Settings //
    /////////////////////////////

    enum ACDX_COLOR_MATERIAL_MODE 
    { 
        ACDX_CM_EMISSION,                ///< Color material emission mode
        ACDX_CM_AMBIENT,                ///< Color material ambient mode
        ACDX_CM_DIFFUSE,                ///< Color material diffuse mode
        ACDX_CM_SPECULAR,                ///< Color material specular mode
        ACDX_CM_AMBIENT_AND_DIFFUSE,    ///< Color material ambient and diffuse mode
    
    } colorMaterialMode; ///< The color material mode. Only emission is currently implemented.

    enum ACDX_COLOR_MATERIAL_ENABLED_FACE 
    { 
        ACDX_CM_NONE,            ///< Color material any face enabled
        ACDX_CM_FRONT,            ///< Color material front face enabled
        ACDX_CM_BACK,            ///< Color material back face enabled
        ACDX_CM_FRONT_AND_BACK, ///< Color material front and back faces enabled
    
    } colorMaterialEnabledFace;  ///< The color material enabled face

    //////////////////
    // FOG Settings //
    //////////////////

    acd_bool fogEnabled;    ///< FOG enabled/disabled

    enum ACDX_FOG_COORDINATE_SOURCE
    { 
        ACDX_FRAGMENT_DEPTH, ///< The FOG coordinate is taken from the fragment depth.
        ACDX_FOG_COORD,         ///< The FOG coordinate is taken from the vertex fog attribute.
    
    } fogCoordinateSource; ///< The FOG coordinate source
    
    enum ACDX_FOG_MODE 
    { 
        ACDX_FOG_LINEAR, ///< The FOG factor is computed linearly across the segment between the start and the end distances.
        ACDX_FOG_EXP,     ///< The FOG factor is an exponential function of the FOG coordinate.
        ACDX_FOG_EXP2,     ///< The FOG factor is a second order exponential function of the FOG coordinate.
    
    } fogMode;  ///< The FOG mode

    ////////////////////////////////////////////
    // Texture Coordinate Generation Settings //
    ////////////////////////////////////////////

    static const acd_uint ACDX_MAX_TEXTURE_STAGES = ACDX_FP_MAX_TEXTURE_STAGES_LIMIT; ///< Max number of texture stages supported by the implementation

    /**
     * The ACDX_TEXTURE_COORDINATE_SETTINGS struct defines the settings of the texture coordinate generation
     */
    struct ACDX_TEXTURE_COORDINATE_SETTINGS
    {
        enum ACDX_TEXTURE_COORDINATE_MODE 
        { 
            ACDX_VERTEX_ATTRIB,        ///< The texture coordinate is taken/copied directly from the vertex input texture coordinate
            ACDX_OBJECT_LINEAR,        ///< The texture coordinate is computed as the normalized distance to the object linear plane
            ACDX_EYE_LINEAR,        ///< The texture coordinate is computed as the normalized distance to the eye linear plane
            ACDX_SPHERE_MAP,        ///< The texture coordinate is computed using the sphere map reflection technique (environmental mapping)
            ACDX_REFLECTION_MAP,    ///< The texture coordinate is computed using the reflection map technique (environmental mapping)
            ACDX_NORMAL_MAP            ///< The texture coordinate is computed using the normal map technique (environmental mapping)
        };

        ACDX_TEXTURE_COORDINATE_MODE coordS; ///< The texture S coordinate mode
        ACDX_TEXTURE_COORDINATE_MODE coordT; ///< The texture T coordinate mode
        ACDX_TEXTURE_COORDINATE_MODE coordR; ///< The texture R coordinate mode
        ACDX_TEXTURE_COORDINATE_MODE coordQ; ///< The texture Q coordinate mode

        acd_bool textureMatrixIsIdentity; ///< The texture coordinate matrix is the identity matrix (no coordinate transformation is needed)
    };

    ACDX_TEXTURE_COORDINATE_SETTINGS textureCoordinates[ACDX_MAX_TEXTURE_STAGES]; ///< The array of texture coordinate generation settings

    ////////////////////////////
    // Texture Stage Settings //
    ////////////////////////////

    /**
     * The ACDX_TEXTURE_STAGE_SETTINGS struct defines the settings of a texture stage
     */
    struct ACDX_TEXTURE_STAGE_SETTINGS
    {
        acd_bool enabled;    ///< Texture stage enabled/disabled
        
        enum ACDX_TEXTURE_TARGET 
        { 
            ACDX_TEXTURE_1D,        ///< 1D Texture target
            ACDX_TEXTURE_2D,        ///< 2D Texture target
            ACDX_TEXTURE_3D,        ///< 3D Texture target
            ACDX_TEXTURE_CUBE_MAP,    ///< Cube Map texture target
            ACDX_TEXTURE_RECT        ///< Rectangular texture target
        
        } activeTextureTarget;    /// The active texture target

        enum ACDX_TEXTURE_STAGE_FUNCTION 
        { 
            ACDX_REPLACE,    ///< Replace texture combine function 
            ACDX_MODULATE,  ///< Modulate texture combine function 
            ACDX_DECAL,        ///< Decal texture combine function 
            ACDX_BLEND,        ///< Blend texture combine function 
            ACDX_ADD,        ///< Add texture combine function 
            ACDX_COMBINE,    ///< Combine texture combine function 
            ACDX_COMBINE_NV  ///< Combine texture combine function (For "NV_texture_env_combine4")
        
        } textureStageFunction;    ///< The texture combine function

        ACDX_COMBINE_SETTINGS combineSettings;

        enum ACDX_BASE_INTERNAL_FORMAT 
        { 
            ACDX_ALPHA,                ///<  The alpha internal format
            ACDX_LUMINANCE,            ///<  The luminance internal format
            ACDX_LUMINANCE_ALPHA,    ///<  The luminance alpha internal format
            ACDX_DEPTH,                ///<  The depth internal format
             ACDX_INTENSITY,            ///<  The intensity internal format
            ACDX_RGB,                ///<  The RGB internal format
            ACDX_RGBA,                ///<  The RGBA internal format
        
        } baseInternalFormat;    ///< The base internal format
    };

    ACDX_TEXTURE_STAGE_SETTINGS textureStages[ACDX_MAX_TEXTURE_STAGES];  ///< The array of texture stage settings

    /////////////////////////
    // Alpha Test Settings //
    /////////////////////////

    acd_bool alphaTestEnabled;    ///< Alpha test enabled/disabled

    enum ACDX_ALPHA_TEST_FUNCTION
    { 
        ACDX_ALPHA_NEVER,        ///< Alpha test never function
        ACDX_ALPHA_ALWAYS,        ///< Alpha test always function
        ACDX_ALPHA_LESS,        ///< Alpha test less function
        ACDX_ALPHA_LEQUAL,        ///< Alpha test lequal function
        ACDX_ALPHA_EQUAL,        ///< Alpha test equal function
        ACDX_ALPHA_GEQUAL,        ///< Alpha test gequal function
        ACDX_ALPHA_GREATER,        ///< Alpha test greater function
        ACDX_ALPHA_NOTEQUAL        ///< Alpha test notequal function
    
    } alphaTestFunction;    ///< The alpha test function
};

} // namespace acdlib

#endif // ACDX_FIXED_PIPELINE_SETTINGS_H

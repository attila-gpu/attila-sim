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

#include "SettingsAdapter.h"
#include "support.h"

using namespace acdlib;

SettingsAdapter::SettingsAdapter(const ACDX_FIXED_PIPELINE_SETTINGS& fpSettings)
: settings(fpSettings), fpState(this), tlState(this)
{
}

/////////////////////////////////////////////////////////////////////////
///////// Implementation of inner class TLStateImplementation ///////////
/////////////////////////////////////////////////////////////////////////

#define TLSImp SettingsAdapter::TLStateImplementation

TLSImp::TLStateImplementation(SettingsAdapter* parent) : parent(parent)
{}


bool TLSImp::normalizeNormals()
{
    return (parent->settings.normalizeNormals == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_NORMALIZE);
}

bool TLSImp::isLighting()
{
    return (parent->settings.lightingEnabled);
}

bool TLSImp::isRescaling()
{
    return (parent->settings.normalizeNormals == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_RESCALE);
}


bool TLSImp::infiniteViewer()
{
    return !(parent->settings.localViewer);
}

bool TLSImp::localLightSource(Light light)
{
    if (light >= ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS)
        panic("Settings Adapter ACDXTLState Implementation","localLightSource()","Requested light out of range");
    
    return ((parent->settings.lights[light].lightType == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_POINT) ||
           (parent->settings.lights[light].lightType == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_SPOT));
}

void TLSImp::attenuationCoeficients(Light , float& , float& , float&)
{
    panic("Settings Adapter ACDXTLState Implementation","attenuationCoeficients()","Not implemented. Coefs shouldn´t be required by the FF emulation");
}

bool TLSImp::isSpotLight(Light light)
{
    if (light >= ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS)
        panic("Settings Adapter ACDXTLState Implementation","isSpotLight()","Requested light out of range");

    return (parent->settings.lights[light].lightType == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_SPOT);
}

bool TLSImp::isLightEnabled(Light light)
{
    if (light >= ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS)
        panic("Settings Adapter ACDXTLState Implementation","isLightEnabled()","Requested light out of range");

    return (parent->settings.lights[light].enabled);
}

int TLSImp::maxLights()
{
    return (ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS);
}

bool TLSImp::separateSpecular()
{
    return (parent->settings.separateSpecular);
}

bool TLSImp::twoSidedLighting()
{
    return (parent->settings.twoSidedLighting);
}

bool TLSImp::anyLightEnabled()
{
    int i=0;
    bool found = false;

    while (i<ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS && !found)
    {
        if (parent->settings.lights[i].enabled) found = true;
        i++;
    }

    return found;
}

bool TLSImp::anyLocalLight()
{
    int i=0;
    bool found = false;

    while (i<ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_LIGHTS && !found)
    {
        if ((parent->settings.lights[i].lightType == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_POINT) ||
           (parent->settings.lights[i].lightType == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_LIGHT_SETTINGS::ACDX_SPOT))
            found = true;

        i++;
    }

    return found;
}

bool TLSImp::isCullingEnabled()
{
    return (parent->settings.cullEnabled);
}

bool TLSImp::isFrontFaceCulling()
{
    return ((parent->settings.cullMode == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_FRONT) ||
            (parent->settings.cullMode == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_FRONT_AND_BACK));
}

bool TLSImp::isBackFaceCulling()
{
    return ((parent->settings.cullMode == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_BACK) ||
            (parent->settings.cullMode == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CULL_FRONT_AND_BACK));
}

ACDXTLState::ColorMaterialMode TLSImp::colorMaterialMode()
{
    switch(parent->settings.colorMaterialMode)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_EMISSION: return ACDXTLState::EMISSION;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_AMBIENT: return ACDXTLState::AMBIENT;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_DIFFUSE: return ACDXTLState::DIFFUSE;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_SPECULAR: return ACDXTLState::SPECULAR;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_AMBIENT_AND_DIFFUSE: return ACDXTLState::AMBIENT_AND_DIFFUSE;
        default:
            panic("TLSImp", "colorMaterialMode", "Unsupported color material mode");
            return ACDXTLState::AMBIENT_AND_DIFFUSE; // to avoid stupid compiler warnings
    }
}

bool TLSImp::colorMaterialFrontEnabled()
{
    return ((parent->settings.colorMaterialEnabledFace == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_FRONT) ||
            (parent->settings.colorMaterialEnabledFace == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_FRONT_AND_BACK));
}

bool TLSImp::colorMaterialBackEnabled()
{
    return ((parent->settings.colorMaterialEnabledFace == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_BACK) ||
            (parent->settings.colorMaterialEnabledFace == ACDX_FIXED_PIPELINE_SETTINGS::ACDX_CM_FRONT_AND_BACK));
}

bool TLSImp::isTextureUnitEnabled(GLuint texUnit)
{
    if (texUnit >= ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES)
        panic("Settings Adapter ACDXTLState Implementation","isTextureUnitEnable()","Requested texture stage out of range");

    return (parent->settings.textureStages[texUnit].enabled);
}

bool TLSImp::anyTextureUnitEnabled()
{
    int i=0;
    bool found = false;

    while ( i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES && !found)
    {
        if (parent->settings.textureStages[i].enabled) found = true;
        i++;
    }

    return found;
}


int TLSImp::maxTextureUnits()
{
    return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES;
}


ACDXTLState::TextureUnit TLSImp::getTextureUnit(GLuint unit)
{
    if (unit >= ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES)
        panic("Settings Adapter ACDXTLState Implementation","getTextureUnit()","Requested texture stage out of range");

    ACDXTLState::TextureUnit tu;

    tu.unitId = unit;

    tu.activeTexGenS = (parent->settings.textureCoordinates[unit].coordS != ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB);
    tu.activeTexGenT = (parent->settings.textureCoordinates[unit].coordT != ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB);
    tu.activeTexGenR = (parent->settings.textureCoordinates[unit].coordR != ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB);
    tu.activeTexGenQ = (parent->settings.textureCoordinates[unit].coordQ != ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB);
    
    switch (parent->settings.textureCoordinates[unit].coordS)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB: break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_OBJECT_LINEAR: tu.texGenModeS = ACDXTLState::TextureUnit::OBJECT_LINEAR; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_EYE_LINEAR: tu.texGenModeS = ACDXTLState::TextureUnit::EYE_LINEAR; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_SPHERE_MAP: tu.texGenModeS = ACDXTLState::TextureUnit::SPHERE_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_REFLECTION_MAP: tu.texGenModeS = ACDXTLState::TextureUnit::REFLECTION_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_NORMAL_MAP: tu.texGenModeS = ACDXTLState::TextureUnit::NORMAL_MAP; break;
    }
    
    switch (parent->settings.textureCoordinates[unit].coordT)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB: break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_OBJECT_LINEAR: tu.texGenModeT = ACDXTLState::TextureUnit::OBJECT_LINEAR; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_EYE_LINEAR: tu.texGenModeT = ACDXTLState::TextureUnit::EYE_LINEAR; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_SPHERE_MAP: tu.texGenModeT = ACDXTLState::TextureUnit::SPHERE_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_REFLECTION_MAP: tu.texGenModeT = ACDXTLState::TextureUnit::REFLECTION_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_NORMAL_MAP: tu.texGenModeT = ACDXTLState::TextureUnit::NORMAL_MAP; break;
    }

    switch (parent->settings.textureCoordinates[unit].coordR)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB: break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_OBJECT_LINEAR: tu.texGenModeR = ACDXTLState::TextureUnit::OBJECT_LINEAR; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_EYE_LINEAR: tu.texGenModeR = ACDXTLState::TextureUnit::EYE_LINEAR; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_SPHERE_MAP: tu.texGenModeR = ACDXTLState::TextureUnit::SPHERE_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_REFLECTION_MAP: tu.texGenModeR = ACDXTLState::TextureUnit::REFLECTION_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_NORMAL_MAP: tu.texGenModeR = ACDXTLState::TextureUnit::NORMAL_MAP; break;
    }

    switch (parent->settings.textureCoordinates[unit].coordQ)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_VERTEX_ATTRIB: break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_OBJECT_LINEAR: tu.texGenModeQ = ACDXTLState::TextureUnit::OBJECT_LINEAR; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_EYE_LINEAR: tu.texGenModeQ = ACDXTLState::TextureUnit::EYE_LINEAR; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_SPHERE_MAP: tu.texGenModeQ = ACDXTLState::TextureUnit::SPHERE_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_REFLECTION_MAP: tu.texGenModeQ = ACDXTLState::TextureUnit::REFLECTION_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_COORDINATE_SETTINGS::ACDX_NORMAL_MAP: tu.texGenModeQ = ACDXTLState::TextureUnit::NORMAL_MAP; break;
    }

    tu.textureMatrixIsIdentity = parent->settings.textureCoordinates[unit].textureMatrixIsIdentity;

    return tu;
}

ACDXTLState::FogCoordSrc TLSImp::fogCoordSrc()
{
    switch(parent->settings.fogCoordinateSource)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FRAGMENT_DEPTH: return ACDXTLState::FRAGMENT_DEPTH;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_COORD: return ACDXTLState::FOG_COORD;
        default:
            panic("TLSImp", "fogCoordSrc", "Unsupported fog coord source");
            return ACDXTLState::FOG_COORD; // to avoid stupid compiler warnings 
    }
}

bool TLSImp::fogEnabled()
{
    return (parent->settings.fogEnabled);
}


/////////////////////////////////////////////////////////////////////////
///////// Implementation of inner class FPStateImplementation ///////////
/////////////////////////////////////////////////////////////////////////

#define FPSImp SettingsAdapter::FPStateImplementation

FPSImp::FPStateImplementation(SettingsAdapter* parent) : parent(parent)
{}

ACDXFPState::AlphaTestFunc FPSImp::alphaTestFunc()
{
    switch(parent->settings.alphaTestFunction)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_NEVER: return ACDXFPState::NEVER;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_ALWAYS: return ACDXFPState::ALWAYS;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_LESS: return ACDXFPState::LESS;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_LEQUAL: return ACDXFPState::LEQUAL;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_EQUAL: return ACDXFPState::EQUAL;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_GEQUAL: return ACDXFPState::GEQUAL;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_ALPHA_GREATER: return ACDXFPState::GREATER;
        default:
            panic("FPSImp", "alphaTestFunc", "Unsupported Alpha test function");
            return ACDXFPState::NOTEQUAL;
    }
}

bool FPSImp::alphaTestEnabled()
{
    return (parent->settings.alphaTestEnabled);
}

bool FPSImp::separateSpecular()
{
    return (parent->settings.separateSpecular);
}

bool FPSImp::anyTextureUnitEnabled()
{
    int i=0;

    bool found = false;

    while ( i < ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES && !found)
    {
        if (parent->settings.textureStages[i].enabled) found = true;
        i++;
    }

    return found;
}

int FPSImp::maxTextureUnits()
{
    return ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES;
}

bool FPSImp::isTextureUnitEnabled(GLuint unit)
{
    if (unit >= ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES)
        panic("Settings Adapter ACDXFPState Implementation","isTextureUnitEnabled()","Requested texture stage out of range");

    return (parent->settings.textureStages[unit].enabled);
}

ACDXFPState::TextureUnit FPSImp::getTextureUnit(GLuint unit)
{
    if (unit >= ACDX_FIXED_PIPELINE_SETTINGS::ACDX_MAX_TEXTURE_STAGES)
        panic("Settings Adapter ACDXFPState Implementation","getTextureUnit()","Requested texture stage out of range");
    
    ACDXFPState::TextureUnit tu(unit);

    switch(parent->settings.textureStages[unit].activeTextureTarget)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_1D: tu.activeTarget = ACDXFPState::TextureUnit::TEXTURE_1D; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_2D: tu.activeTarget = ACDXFPState::TextureUnit::TEXTURE_2D; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_3D: tu.activeTarget = ACDXFPState::TextureUnit::TEXTURE_3D; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_CUBE_MAP: tu.activeTarget = ACDXFPState::TextureUnit::CUBE_MAP; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_TEXTURE_RECT: panic("Settings Adapter ACDXFPState Implementation","getTextureUnit()","RECT Texture target not yet supported");
    }

    switch(parent->settings.textureStages[unit].textureStageFunction)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_REPLACE: tu.function = ACDXFPState::TextureUnit::REPLACE; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_MODULATE: tu.function = ACDXFPState::TextureUnit::MODULATE; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_DECAL: tu.function = ACDXFPState::TextureUnit::DECAL; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_BLEND: tu.function = ACDXFPState::TextureUnit::BLEND; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_ADD: tu.function = ACDXFPState::TextureUnit::ADD; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_COMBINE: tu.function = ACDXFPState::TextureUnit::COMBINE; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_COMBINE_NV: tu.function = ACDXFPState::TextureUnit::COMBINE4_NV; break;
    }

    switch(parent->settings.textureStages[unit].baseInternalFormat)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_ALPHA: tu.format = ACDXFPState::TextureUnit::ALPHA; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_LUMINANCE: tu.format = ACDXFPState::TextureUnit::LUMINANCE; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_LUMINANCE_ALPHA: tu.format = ACDXFPState::TextureUnit::LUMINANCE_ALPHA; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_DEPTH: tu.format = ACDXFPState::TextureUnit::DEPTH; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_INTENSITY: tu.format = ACDXFPState::TextureUnit::INTENSITY; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_RGB: tu.format = ACDXFPState::TextureUnit::RGB; break;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_TEXTURE_STAGE_SETTINGS::ACDX_RGBA: tu.format = ACDXFPState::TextureUnit::RGBA; break;
    }

    /* Combine parameters */
    
    switch(parent->settings.textureStages[unit].combineSettings.RGBFunction)
    {
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_REPLACE: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::REPLACE; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::MODULATE; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::ADD; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD_SIGNED: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::ADD_SIGNED; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_INTERPOLATE: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::INTERPOLATE; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SUBTRACT: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::SUBTRACT; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_DOT3_RGB: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::DOT3_RGB; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_DOT3_RGBA: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::DOT3_RGBA; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_ADD: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::MODULATE_ADD_ATI; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SIGNED_ADD: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::MODULATE_SIGNED_ADD_ATI; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SUBTRACT: tu.combineMode.combineRGBFunction = ACDXFPState::TextureUnit::CombineMode::MODULATE_SUBTRACT_ATI; break;
    }

    switch(parent->settings.textureStages[unit].combineSettings.ALPHAFunction)
    {
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_REPLACE: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::REPLACE; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::MODULATE; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::ADD; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ADD_SIGNED: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::ADD_SIGNED; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_INTERPOLATE: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::INTERPOLATE; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SUBTRACT: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::SUBTRACT; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_ADD: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::MODULATE_ADD_ATI; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SIGNED_ADD: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::MODULATE_SIGNED_ADD_ATI; break;
        case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_MODULATE_SUBTRACT: tu.combineMode.combineALPHAFunction = ACDXFPState::TextureUnit::CombineMode::MODULATE_SUBTRACT_ATI; break;
    }

    for (int j=0; j<4; j++)
    {
        switch(parent->settings.textureStages[unit].combineSettings.srcRGB[j])
        {
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO: tu.combineMode.srcRGB[j] = ACDXFPState::TextureUnit::CombineMode::ZERO; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE: tu.combineMode.srcRGB[j] = ACDXFPState::TextureUnit::CombineMode::ONE; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTURE: ACDXFPState::TextureUnit::CombineMode::TEXTURE; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTUREn: ACDXFPState::TextureUnit::CombineMode::TEXTUREn; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_CONSTANT: ACDXFPState::TextureUnit::CombineMode::CONSTANT; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PRIMARY_COLOR: ACDXFPState::TextureUnit::CombineMode::PRIMARY_COLOR; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PREVIOUS: ACDXFPState::TextureUnit::CombineMode::PREVIOUS; break;
        }

        switch(parent->settings.textureStages[unit].combineSettings.srcALPHA[j])
        {
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ZERO: tu.combineMode.srcALPHA[j] = ACDXFPState::TextureUnit::CombineMode::ZERO; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE: tu.combineMode.srcALPHA[j] = ACDXFPState::TextureUnit::CombineMode::ONE; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTURE: tu.combineMode.srcALPHA[j] = ACDXFPState::TextureUnit::CombineMode::TEXTURE; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_TEXTUREn: tu.combineMode.srcALPHA[j] = ACDXFPState::TextureUnit::CombineMode::TEXTUREn; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_CONSTANT: tu.combineMode.srcALPHA[j] = ACDXFPState::TextureUnit::CombineMode::CONSTANT; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PRIMARY_COLOR: tu.combineMode.srcALPHA[j] = ACDXFPState::TextureUnit::CombineMode::PRIMARY_COLOR; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_PREVIOUS: tu.combineMode.srcALPHA[j] = ACDXFPState::TextureUnit::CombineMode::PREVIOUS; break;
        }

        switch(parent->settings.textureStages[unit].combineSettings.operandRGB[j])
        {
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR: tu.combineMode.operandRGB[j] = ACDXFPState::TextureUnit::CombineMode::SRC_COLOR; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_COLOR: tu.combineMode.operandRGB[j] = ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_COLOR; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA: tu.combineMode.operandRGB[j] = ACDXFPState::TextureUnit::CombineMode::SRC_ALPHA; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_ALPHA: tu.combineMode.operandRGB[j] = ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_ALPHA; break;
        }

        switch(parent->settings.textureStages[unit].combineSettings.operandALPHA[j])
        {
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_COLOR: tu.combineMode.operandALPHA[j] = ACDXFPState::TextureUnit::CombineMode::SRC_COLOR; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_COLOR: tu.combineMode.operandALPHA[j] = ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_COLOR; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_SRC_ALPHA: tu.combineMode.operandALPHA[j] = ACDXFPState::TextureUnit::CombineMode::SRC_ALPHA; break;
            case ACDX_COMBINE_SETTINGS::ACDX_COMBINE_ONE_MINUS_SRC_ALPHA: tu.combineMode.operandALPHA[j] = ACDXFPState::TextureUnit::CombineMode::ONE_MINUS_SRC_ALPHA; break;
        }

        tu.combineMode.srcRGB_texCrossbarReference[j]  = parent->settings.textureStages[unit].combineSettings.srcRGB_texCrossbarReference[j];
        tu.combineMode.srcALPHA_texCrossbarReference[j] = parent->settings.textureStages[unit].combineSettings.srcALPHA_texCrossbarReference[j];
    }    

    tu.combineMode.rgbScale = parent->settings.textureStages[unit].combineSettings.RGBScale;
    tu.combineMode.alphaScale = parent->settings.textureStages[unit].combineSettings.ALPHAScale;

    return tu;
}

bool FPSImp::fogEnabled()
{
    return (parent->settings.fogEnabled);
}

ACDXFPState::FogMode FPSImp::fogMode()
{
    switch (parent->settings.fogMode)
    {
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_LINEAR: return ACDXFPState::LINEAR;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_EXP: return ACDXFPState::EXP;
        case ACDX_FIXED_PIPELINE_SETTINGS::ACDX_FOG_EXP2: return ACDXFPState::EXP2;
        default:
            panic("ACDXFPState::FogMode", "fogMode", "Unsupported fog mode");
            return ACDXFPState::EXP2; // to avoid stupid compiler warnings
    }
}

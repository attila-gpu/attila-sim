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

#ifndef ACDX_TRANSFORM_AND_LIGHTING_STAGE_H
    #define ACDX_TRANSFORM_AND_LIGHTING_STAGE_H

#include "ACDXGlobalTypeDefinitions.h"

namespace acdlib
{

/**
 * Interface to configure a light state parameters.
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @date 03/13/2007
 */
class ACDXLight
{
public:
    
    /**
     * Sets the light ambient color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /**
     * Sets the light diffuse color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightDiffuseColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /**
     * Sets the light specular color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightSpecularColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /**
     * Sets the light position (only for point and spot lights).
     *
     * @param x        The x position coordinate.
     * @param y        The y position coordinate.
     * @param z        The z position coordinate.
     * @param w        The w position coordinate (for homogeneous cordinate system).
     */
    virtual void setLightPosition(acd_float x, acd_float y, acd_float z, acd_float w) = 0;

    /**
     * Sets the light direction (only for directional lights).
     *
     * @param x        The x direction coordinate.
     * @param y        The y direction coordinate.
     * @param z        The z direction coordinate.
     */
    virtual void setLightDirection(acd_float x, acd_float y, acd_float z) = 0;

    /**
     * Sets the light attenuation coefficients (only for point and spot lights).
     *
     * @param constant        The constant attenuation coefficient.
     * @param linear        The linear attenuation coefficient.
     * @param quadratic        The quadratic attenuation coefficient.
     */
    virtual void setAttenuationCoeffs(acd_float constant, acd_float linear, acd_float quadratic) = 0;

    /**
     * Gets the constant light attenuation coefficient (only for point and spot lights).
     */
    virtual acd_float getConstantAttenuation () = 0;

    /**
     * Gets the linear light attenuation coefficient (only for point and spot lights).
     */
    virtual acd_float getLinearAttenuation () = 0;

    /**
     * Gets the quadratic light attenuation coefficient (only for point and spot lights).
     */
    virtual acd_float getQuadraticAttenuation () = 0;

    /**
     * Sets the spot light direction (only for spot lights).
     *
     * @param x        The x direction coordinate.
     * @param y        The y direction coordinate.
     * @param z        The z direction coordinate.
     */
    virtual void setSpotLightDirection(acd_float x, acd_float y, acd_float z) = 0;

    /**
     * Sets the spot light cut-off angle (outer Phi cut-off angle in D3D).
     *
     * @param cos_angle The cosine of the cut-off angle.
     */
    virtual void setSpotLightCutOffAngle(acd_float cos_angle) = 0;

    /**
     * Sets the spot exponent or D3D falloff factor (only for spot lights).
     *
     * @param exponent The spot exponent value
     */
    virtual void setSpotExponent(acd_float exponent) = 0;
};

/**
 * Interface to configure a material state parameters.
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @date 03/13/2007
 */
class ACDXMaterial
{
public:
    
    /**
     * Sets the material ambient color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setMaterialAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /**
     * Sets the material diffuse color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setMaterialDiffuseColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /**
     * Sets the material specular color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setMaterialSpecularColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /**
     * Sets the material emission color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setMaterialEmissionColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /**
     * Sets the shininess factor of the material.
     *
     * @param shininess    The shininess factor of the material.
     */
    virtual void setMaterialShininess(acd_float shininess) = 0;

};


/**
 * Faces 
 */
enum ACDX_FACE
{
    ACDX_FRONT,            ///< Front face idenfier
    ACDX_BACK,            ///< Back face identifier
    ACDX_FRONT_AND_BACK ///< Front and back face identifier
};

/**
 * Interface to configure the ACDX Transform & Lighting state
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @date 03/13/2007
 */
class ACDXTransformAndLightingStage
{
public:

    /**
     * Select an light object to be configured
     *
     * @param light The light number.
     * @return        The corresponding ACDXLight object.
     */
    virtual ACDXLight& light(acd_uint light) = 0;

    /**
     * Select a face material object to be configured
     *
     * @param face  The material properties face.
     * @return        The corresponding ACDXMaterial object.
     */
    virtual ACDXMaterial& material(ACDX_FACE face) = 0;

    /**
     * Sets the light model ambient color
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightModelAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0;
        
    /**
     * Sets the light model scene color
     *
     * @param face  The face where to set the scene color.
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightModelSceneColor(ACDX_FACE face, acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /* TO DO: Include Transformation matrix set methods */

    /**
     * Sets the Modelview Matrix.
     *
     * @param unit     The vertex unit matrix(Vertex blending support).
     * @param matrix The input matrix.
     */
    virtual void setModelviewMatrix(acd_uint unit, const ACDXFloatMatrix4x4& matrix) = 0;

    /**
     * Gets the Modelview Matrix
     *
     * @param unit The vertex unit matrix
     */
    virtual ACDXFloatMatrix4x4 getModelviewMatrix(acd_uint unit) const = 0;

    /**
     * Sets the Projection Matrix.
     *
     * @param matrix The input matrix.
     */
    virtual void setProjectionMatrix(const ACDXFloatMatrix4x4& matrix) = 0;

    /**
     * Gets the Projection Matrix
     *
     * @return A copy of the current Projection Matrix
     */
    virtual ACDXFloatMatrix4x4 getProjectionMatrix() const = 0;

};

} // namespace acdlib

#endif // ACDX_TRANSFORM_AND_LIGHTING_STAGE_H

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

#ifndef ACDX_LIGHT_IMP_H
    #define ACDX_LIGHT_IMP_H

#include "ACDXGlobalTypeDefinitions.h"
#include "ACDXTransformAndLightingStage.h"

namespace acdlib
{

class ACDXFixedPipelineStateImp;

class ACDXLightImp: public ACDXLight
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
    virtual void setLightAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a);

    /**
     * Sets the light diffuse color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightDiffuseColor(acd_float r, acd_float g, acd_float b, acd_float a);

    /**
     * Sets the light specular color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setLightSpecularColor(acd_float r, acd_float g, acd_float b, acd_float a);

    /**
     * Sets the light position (only for point and spot lights).
     *
     * @param x        The x position coordinate.
     * @param y        The y position coordinate.
     * @param z        The z position coordinate.
     * @param w        The w position coordinate (for homogeneous cordinate system).
     */
    virtual void setLightPosition(acd_float x, acd_float y, acd_float z, acd_float w);

    /**
     * Sets the light direction (only for directional lights).
     *
     * @param x        The x direction coordinate.
     * @param y        The y direction coordinate.
     * @param z        The z direction coordinate.
     */
    virtual void setLightDirection(acd_float x, acd_float y, acd_float z);

    /**
     * Sets the light attenuation coefficients (only for point and spot lights).
     *
     * @param constant        The constant attenuation coefficient.
     * @param linear        The linear attenuation coefficient.
     * @param quadratic        The quadratic attenuation coefficient.
     */
    virtual void setAttenuationCoeffs(acd_float constant, acd_float linear, acd_float quadratic);

    /**
     * Gets the constant light attenuation coefficient (only for point and spot lights).
     */
    virtual acd_float getConstantAttenuation ();

    /**
     * Gets the linear light attenuation coefficient (only for point and spot lights).
     */
    virtual acd_float getLinearAttenuation ();

    /**
     * Gets the quadratic light attenuation coefficient (only for point and spot lights).
     */
    virtual acd_float getQuadraticAttenuation ();
    /**
     * Sets the spot light direction (only for spot lights).
     *
     * @param x        The x direction coordinate.
     * @param y        The y direction coordinate.
     * @param z        The z direction coordinate.
     */
    virtual void setSpotLightDirection(acd_float x, acd_float y, acd_float z);

    /**
     * Sets the spot light cut-off angle (outer Phi cut-off angle).
     *
     * @param cos_angle The cosine of the cut-off angle.
     */
    virtual void setSpotLightCutOffAngle(acd_float cos_angle);

    /**
     * Sets the spot exponent or D3D falloff factor (only for spot lights).
     *
     * @param exponent The spot exponent value
     */
    virtual void setSpotExponent(acd_float exponent);

//////////////////////////
//  interface extension //
//////////////////////////

public:

    ACDXLightImp(acd_uint lightNumber, ACDXFixedPipelineStateImp* fpStateImp);

private:

    ACDXFixedPipelineStateImp* _fpStateImp;
    acd_uint _lightNumber;

};

} // namespace acdlib

#endif // ACDX_LIGHT_IMP_H

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

#ifndef ACDX_MATERIAL_IMP_H
    #define ACDX_MATERIAL_IMP_H

#include "ACDXTransformAndLightingStage.h"
#include "ACDXGlobalTypeDefinitions.h"

namespace acdlib
{

class ACDXFixedPipelineStateImp;

class ACDXMaterialImp: public ACDXMaterial
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
    virtual void setMaterialAmbientColor(acd_float r, acd_float g, acd_float b, acd_float a);

    /**
     * Sets the material diffuse color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setMaterialDiffuseColor(acd_float r, acd_float g, acd_float b, acd_float a);

    /**
     * Sets the material specular color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setMaterialSpecularColor(acd_float r, acd_float g, acd_float b, acd_float a);

    /**
     * Sets the material emission color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
    virtual void setMaterialEmissionColor(acd_float r, acd_float g, acd_float b, acd_float a);

    /**
     * Sets the shininess factor of the material.
     *
     * @param shininess    The shininess factor of the material.
     */
    virtual void setMaterialShininess(acd_float shininess);

//////////////////////////
//  interface extension //
//////////////////////////

public:

    ACDXMaterialImp(acd_uint face, ACDXFixedPipelineStateImp* fpStateImp);

private:

    ACDXFixedPipelineStateImp* _fpStateImp;
    acd_uint _face;

};

} // namespace acdlib

#endif // ACDX_MATERIAL_IMP_H

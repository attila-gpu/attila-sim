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

#ifndef ACDX_FRAGMENT_SHADING_STAGE_H
    #define ACDX_FRAGMENT_SHADING_STAGE_H

#include "ACDXGlobalTypeDefinitions.h"

namespace acdlib
{

/**
 * Interface to configure the ACDX Fragment Shading Stage state
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @date 03/13/2007
 */
class ACDXFragmentShadingStage
{
public:

    /**
     * Sets the texture environment color used for each texture stage.
     *
     * @param textureStage    The corresponding texture stage.
     * @param r                The red component of the color.
     * @param g                The green component of the color.
     * @param b                The    blue component of the color.
     * @param a                The alpha component of the color.
     */
     virtual void setTextureEnvColor(acd_uint textureStage, acd_float r, acd_float g, acd_float b, acd_float a) = 0;

    /**
     * Sets the depth range (near and far plane distances)
     *
     * @param near           The distance of the near plane.
     * @param far           The distance of the far plane.
     */
     virtual void setDepthRange(acd_float near, acd_float far) = 0;
};

} // namespace acdlib

#endif // ACDX_FRAGMENT_SHADING_STAGE_H

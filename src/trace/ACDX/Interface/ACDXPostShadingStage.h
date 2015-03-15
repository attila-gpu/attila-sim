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

#ifndef ACDX_POST_SHADING_STAGE_H
    #define ACDX_POST_SHADING_STAGE_H

#include "ACDXGlobalTypeDefinitions.h"

namespace acdlib
{

/**
 * Interface to configure the ACDX Post Fragment Shading Stages state (FOG and Alpha).
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @date 03/13/2007
 */
class ACDXPostShadingStage
{
public:

    /////////
    // FOG //
    /////////

    /**
     * Sets the FOG blending color.
     *
     * @param r        The red component of the color.
     * @param g        The green component of the color.
     * @param b        The blue component of the color.
     * @param a        The alpha component of the color.
     */
     virtual void setFOGBlendColor(acd_float r, acd_float g, acd_float b, acd_float a) = 0; 

    /**
     * Sets the FOG density exponent
     *
     * @param exponent The density exponent
     */
     virtual void setFOGDensity(acd_float exponent) = 0;

    /**
     * Sets the FOG linear mode start and end distances.
     *
     * @param start The FOG linear start distance.
     * @param end   The FOG linear end distance.
     */
     virtual void setFOGLinearDistances(acd_float start, acd_float end) = 0;

     ////////////////
     // ALPHA TEST //
     ////////////////

     /**
      * Sets the Alpha Test Reference Value.
      *
      * @param refValue The reference value.
      */
     virtual void setAlphaTestRefValue(acd_float refValue) = 0;
};

} // namespace acdlib

#endif // ACDX_POST_SHADING_STAGE_H

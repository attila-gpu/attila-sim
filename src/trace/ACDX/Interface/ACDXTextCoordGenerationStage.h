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

#ifndef ACDX_TEXT_COORD_GENERATION_STAGE_H
    #define ACDX_TEXT_COORD_GENERATION_STAGE_H

#include "ACDXGlobalTypeDefinitions.h"

namespace acdlib
{
/**
 * Texture coordinate names
 */
enum ACDX_TEXTURE_COORD
{
    ACDX_COORD_S,    ///< S texture coordinate
    ACDX_COORD_T,    ///< T texture coordinate
    ACDX_COORD_R,    ///< R texture coordinate
    ACDX_COORD_Q,    ///< Q texture coordinate
};

/**
 * Texture coordinate generation planes
 */
enum ACDX_TEXTURE_COORD_PLANE
{
    ACDX_OBJECT_PLANE,    ///< The object linear coordinate plane 
    ACDX_EYE_PLANE,        ///< The eye linear coordinate plane
};

/**
 * Interface to configure the ACDX Texture Coordinate Generation Stage state
 *
 * @author Jordi Roca Monfort (jroca@ac.upc.edu)
 * @date 03/13/2007
 */
class ACDXTextCoordGenerationStage
{
public:

    /**
     * Sets the texture coordinate generation object and eye planes.
     *
     * @param textureStage The texture stage affected.
     * @param plane           The Eye or Object plane.
     * @param coord           The texture coordinate affected.
     * @param a               The a coefficient of the plane equation.
     * @param b               The b coefficient of the plane equation.
     * @param c               The c coefficient of the plane equation.
     * @param d               The d coefficient of the plane equation.
     */
    virtual void setTextureCoordPlane(acd_uint textureStage, ACDX_TEXTURE_COORD_PLANE plane, ACDX_TEXTURE_COORD coord, acd_float a, acd_float b, acd_float c, acd_float d) = 0;

    /**
     * Sets the texture coordinate matrix
     *
     * @param textureStage The texture stage affected.
     * @param matrix       The texture coordinate matrix.
     */
    virtual void setTextureCoordMatrix(acd_uint textureStage, const ACDXFloatMatrix4x4& matrix) = 0;

    /**
     * Gets the texture coordinate matrix
     *
     * @param textureStage The texture stage matrix selected
     */
    virtual ACDXFloatMatrix4x4 getTextureCoordMatrix(acd_uint textureStage) const = 0;
};

} // namespace acdlib

#endif // ACDX_TEXT_COORD_GENERATION_STAGE_H

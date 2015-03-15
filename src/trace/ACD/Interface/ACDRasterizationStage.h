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

#ifndef ACD_RASTERIZATIONSTAGE
    #define ACD_RASTERIZATIONSTAGE

#include "ACDTypes.h"

namespace acdlib
{

/**
 * Fill modes identifiers definition
 */
enum ACD_FILL_MODE
{
    ACD_FILL_SOLID,
    ACD_FILL_WIREFRAME,
};

/**
 * Cull modes identifiers definition
 */
enum ACD_CULL_MODE
{
    ACD_CULL_NONE,
    ACD_CULL_FRONT,
    ACD_CULL_BACK,
    ACD_CULL_FRONT_AND_BACK
};

enum ACD_FACE_MODE
{
    ACD_FACE_CW,
    ACD_FACE_CCW
};

enum ACD_INTERPOLATION_MODE
{
    ACD_INTERPOLATION_NONE,
    ACD_INTERPOLATION_LINEAR, // Default value
    ACD_INTERPOLATION_CENTROID,
    ACD_INTERPOLATION_NOPERSPECTIVE,
};

/**
 * Interface to configure the Attila rasterization stage
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @date 02/07/2007
 */
class ACDRasterizationStage
{
public:

    /**
     * Sets the interpolation mode for a fragment shader input attribute
     *
     * @param The fragment shader input attribute identifier
     * @param mode The interpolation mode
     */
    virtual void setInterpolationMode(acd_uint fshInputAttribute, ACD_INTERPOLATION_MODE mode) = 0;

    /**
     * Enables the scissor test
     *
     * @param enable: enable/disable the scissor test
     */
    virtual void enableScissor(acd_bool enable) = 0;

    /**
     * Sets the fill mode to use when rendering [default: ACD_FILL_MODE_SOLID]
     *
     * @param fillMode The fill mode to use when rendering
     *
     * @note ACD_WIREFRAME is not currently supported
     */
    virtual void setFillMode(ACD_FILL_MODE fillMode) = 0;

    /**
     * Sets the triangle culling mode [default: ACD_CULL_NONE]
     *
     * @param cullMode the triangle culling mode
     */
    virtual void setCullMode(ACD_CULL_MODE cullMode) = 0;

    /**
     *Sets the triangle facing mode [default: ACD_CW]
     *
     * @param faceMode the triangle facing mode
     */
    virtual void setFaceMode(ACD_FACE_MODE faceMode) = 0;

    /**
     *
     *  Sets if the D3D9 rasterization rules will be used, otherwise the OpenGL rules will be used
     *
     *  @param use TRUE for D3D9 rasterization rules, FALSE for OGL rasterization rules.
     *
     */
     
    virtual void useD3D9RasterizationRules(acd_bool use) = 0;
    
    /**
     *
     *  Sets if the D3D9 pixel coordinates convention will be used (top left is [0,0]),
     *  otherwise the OpenGL convention will be used (bottom left is [0,0]).
     *
     *  @param use TRUE for D3D9 pixel coordinates convention, FALSE for OpenGL convention.
     *
     */
     
    virtual void useD3D9PixelCoordConvention(acd_bool use) = 0;
    
    /**
     * Sets the viewport rectangle
     *
     * @param x
     * @param y
     * @param width
     * @param height
     */
    virtual void setViewport(acd_int x, acd_int y, acd_uint width, acd_uint height) = 0;

    /**
     * Sets the scissor rectangle
     *
     * @param x
     * @param y
     * @param width
     * @param height
     */
    virtual void setScissor(acd_int x, acd_int y, acd_uint width, acd_uint height) = 0;

    /**
     *
     *  Gets the viewport rectange.
     *
     *  @param x
     *  @param y
     *  @param width
     *  @param height
     *
     */
     
    virtual void getViewport(acd_int &x, acd_int &y, acd_uint &width, acd_uint &height) = 0;

    /**
     *  Gets the scissor rectangle
     *
     *  @param enabled
     *  @param x
     *  @param y
     *  @param width
     *  @param height
     *
     */
     
    virtual void getScissor(acd_bool &enabled, acd_int &x, acd_int &y, acd_uint &width, acd_uint &height) = 0;
    
};

}

#endif // ACD_RASTERIZATIONSTAGE

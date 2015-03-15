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

#ifndef ACD_BLENDINGSTAGE
    #define ACD_BLENDINGSTAGE

#include "ACDTypes.h"

namespace acdlib
{

/**
 * Blend options. A blend option identifies the data source and an optional pre-blend operation
 */
enum ACD_BLEND_OPTION
{
    ACD_BLEND_ZERO,
    ACD_BLEND_ONE,
    ACD_BLEND_SRC_COLOR,
    ACD_BLEND_INV_SRC_COLOR,
    ACD_BLEND_SRC_ALPHA,
    ACD_BLEND_INV_SRC_ALPHA,
    ACD_BLEND_DEST_ALPHA,
    ACD_BLEND_INV_DEST_ALPHA,
    ACD_BLEND_DEST_COLOR,
    ACD_BLEND_INV_DEST_COLOR,
    ACD_BLEND_SRC_ALPHA_SAT,
    ACD_BLEND_BLEND_FACTOR,
    ACD_BLEND_INV_BLEND_FACTOR,
    ACD_BLEND_CONSTANT_COLOR,
    ACD_BLEND_INV_CONSTANT_COLOR,
    ACD_BLEND_CONSTANT_ALPHA,
    ACD_BLEND_INV_CONSTANT_ALPHA,
};


/**
 * Blend operations available
 */
enum ACD_BLEND_FUNCTION
{
    ACD_BLEND_ADD,
    ACD_BLEND_SUBTRACT,
    ACD_BLEND_REVERSE_SUBTRACT,
    ACD_BLEND_MIN,
    ACD_BLEND_MAX,
};

/**
 * Interface to configure the Attila color/blending stage
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @date 02/07/2007
 */
class ACDBlendingStage
{
public:

    /**
     * Enable or disable blending for one of the render targets attached to color blending stage
     *
     * @param renderTargetID the render target position
     * @param enable True to enable blending, false to disable it
     */
    virtual void setEnable(acd_uint renderTargetID, acd_bool enable) = 0;

    /**
     * Sets the first RGB data source and includes an optional pre-blend operation
     *
     * @param renderTargetID the render target position
     * @param srcBlend the first RGB data source and an optional pre-blend operation
     */
    virtual void setSrcBlend(acd_uint renderTargetID, ACD_BLEND_OPTION srcBlend) = 0;

    /**
     * Sets the second RGB data source and includes an optional pre-blend operation
     *
     * @param renderTargetID the render target position
     * @param destBlend the second RGB data source and an optional pre-blend operation
     */
    virtual void setDestBlend(acd_uint renderTargetID, ACD_BLEND_OPTION destBlend) = 0;

    /**
     * Sets how to combine the RGB data sources
     *
     * @param renderTargetID the render target position
     * @param blendOp operation to combine RGB data sources
     */
    virtual void setBlendFunc(acd_uint renderTargetID, ACD_BLEND_FUNCTION blendOp) = 0;

    /**
     * Sets the first alpha data source and an optional pre-blend operation
     *
     * @param renderTargetID the render target position
     * @param srcBlendAlpha the first alpha data source and the optional pre-blend operation
     *
     * @note Blending operations ended with _COLOR are not allowed as a parameter for this method
     */
    virtual void setSrcBlendAlpha(acd_uint renderTargetID, ACD_BLEND_OPTION srcBlendAlpha) = 0;

    /**
     * Sets the second alpha data source and an optional pre-blend operation
     *
     * @param renderTargetID the render target position
     * @param destBlendAlpha the second alpha data source and the optional pre-blend operation
     *
     * @note Blending operations ended with _COLOR are not allowed as a parameter for this method
     */
    virtual void setDestBlendAlpha(acd_uint renderTargetID, ACD_BLEND_OPTION destBlendAlpha) = 0;
    
    /**
     * Sets how to combine the alpha data sources
     *
     * @param renderTargetID the render target position
     * @param blendOpAlpha operation to combine the alpha data sources
     */
    virtual void setBlendFuncAlpha(acd_uint renderTargetID, ACD_BLEND_FUNCTION blendOpAlpha) = 0;

    /**
     * Sets the blending color
     *
     * @param renderTargetID the render target position
     * @param red Red component of the blend color
     * @param green Green component of the blend color
     * @param blue Blue component of the blend color
     * @param alpha Alpha value of the blend color
     */
    virtual void setBlendColor(acd_uint renderTargetID, acd_float red, acd_float green, acd_float blue, acd_float alpha) = 0;

    /**
     * Sets the blending color
     *
     * @param renderTargetID the render target position
     * @param rgba 4-component vector with the RGBA blend components
     */
    virtual void setBlendColor(acd_uint renderTargetID, const acd_float* rgba) = 0;

    /**
     * Sets the color mask
     *
     * @param renderTargetID the render target position
     * @param red Active the red component
     * @param green Active the green component
     * @param blue Active the blue component
     * @param alpha Active the alpha component
     */
    virtual void setColorMask(acd_uint renderTargetID, acd_bool red, acd_bool green, acd_bool blue, acd_bool alpha) = 0;
    
    /**
     *
     *  Disables color write (render target undefined).
     *
     * @param renderTargetID the render target position
     *
     */
  
    virtual void disableColorWrite(acd_uint renderTargetID) = 0;
    
    /**
     *
     *  Disables color write (render target defined).
     *
     * @param renderTargetID the render target position
     *
     */

    virtual void enableColorWrite(acd_uint renderTargetID) = 0;
    
    /**
     *
     *  Returns if color write is enabled or disabled.
     *
     * @param renderTargetID the render target position
     *
     */

    virtual acd_bool getColorEnable(acd_uint renderTargetID) = 0;
};

}

#endif // ACD_BLENDINGSTAGE

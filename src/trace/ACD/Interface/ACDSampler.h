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

#ifndef ACD_SAMPLER
    #define ACD_SAMPLER

#include "ACDTypes.h"
#include "ACDTexture.h"
#include "ACDTexture2D.h"

namespace acdlib
{


/**
 * Defines the texture address modes supported by the ACD
 */
enum ACD_TEXTURE_ADDR_MODE
{
    ACD_TEXTURE_ADDR_CLAMP,
    ACD_TEXTURE_ADDR_CLAMP_TO_EDGE,
    ACD_TEXTURE_ADDR_REPEAT,
    ACD_TEXTURE_ADDR_CLAMP_TO_BORDER,
    ACD_TEXTURE_ADDR_MIRRORED_REPEAT
};

/**
 * Texture coordinates tokens
 */
enum ACD_TEXTURE_COORD
{
    ACD_TEXTURE_S_COORD,
    ACD_TEXTURE_T_COORD,
    ACD_TEXTURE_R_COORD
};

/**
 * Native texture filters
 */
enum ACD_TEXTURE_FILTER
{
    ACD_TEXTURE_FILTER_NEAREST,
    ACD_TEXTURE_FILTER_LINEAR,
    ACD_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST,
    ACD_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR,
    ACD_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST,
    ACD_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR
};

/**
 *
 * Native comparison functions.
 *
 */
enum ACD_TEXTURE_COMPARISON
{
    ACD_TEXTURE_COMPARISON_NEVER,
    ACD_TEXTURE_COMPARISON_ALWAYS,
    ACD_TEXTURE_COMPARISON_LESS,
    ACD_TEXTURE_COMPARISON_LEQUAL,
    ACD_TEXTURE_COMPARISON_EQUAL,
    ACD_TEXTURE_COMPARISON_GEQUAL,
    ACD_TEXTURE_COMPARISON_GREATER,
    ACD_TEXTURE_COMPARISON_NOTEQUAL
};
 

/**
 * This interface represent an Atila texture sampler
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @version 1.0
 * @date 01/23/2007
 */
class ACDSampler
{
public:

    /**
     * Enables or disables the sampler unit
     */
    virtual void setEnabled(acd_bool enable) = 0;

    /**
     * Checks if the texture sampler is enabled or disabled
     */
    virtual acd_bool isEnabled() const = 0;

    /**
     * Sets the texture used by this sampler
     *
     * @param texture The set texture
     */
    virtual void setTexture(ACDTexture* texture) = 0;

    /**
     * Sets texture address mode
     */
    virtual void setTextureAddressMode(ACD_TEXTURE_COORD coord, ACD_TEXTURE_ADDR_MODE mode) = 0;

    /**
     *
     *  Set texture non-normalized coordinates mode.
     *
     *  @param enable Boolean value to enable or disable non-normalized texture coordinates.
     *
     */
     
    virtual void setNonNormalizedCoordinates(acd_bool enable) = 0;
         
    /**
     * Sets the minification filter
     *
     * @param minFilter the minification filter
     */
    virtual void setMinFilter(ACD_TEXTURE_FILTER minFilter) = 0;

    /**
     * Sets the magnification filter
     *
     * @param magFilter he magnification filter
     */
    virtual void setMagFilter(ACD_TEXTURE_FILTER magFilter) = 0;

    /**
     *
     *  Sets if comparison filter (PCF) is enabled for this sampler.
     *
     *  @param enable Boolean value that defines if the comparison filter is
     *  enabled for this sampler.
     *
     */
     
    virtual void setEnableComparison(acd_bool enable) = 0;
    
    /**
     *
     *  Sets the comparison function (PCF) for this sampler.
     *
     *  @param function The comparison function defined for the sampler.
     *
     */
     
    virtual void setComparisonFunction(ACD_TEXTURE_COMPARISON function) = 0;
     
    /**
     *
     *  Sets the conversion from sRGB space to linear space of the texture data for the sampler.
     *
     *  @param enable Boolean value that defines if the conversion is enabled for this sampler.
     *  
     */
     
    virtual void setSRGBConversion(acd_bool enable) = 0;    
     
    virtual void setMinLOD(acd_float minLOD) = 0;

    virtual void setMaxLOD(acd_float maxLOD) = 0;

    virtual void setMaxAnisotropy(acd_uint maxAnisotropy) = 0;

    virtual void setLODBias(acd_float lodBias) = 0;

    virtual void setUnitLODBias(acd_float unitLodBias) = 0;

    virtual void setMinLevel(acd_uint minLevel) = 0;
    
    /**
     * Gets the magnification filter
     *
     * @returns the magnification filter
     */
    virtual ACD_TEXTURE_FILTER getMagFilter() const = 0;
    
    /**
     * Gets the current texture of the sampler
     *
     * @returns the current texture attached to the sampler
     */
    virtual ACDTexture* getTexture() const = 0;

    /**
     * Gets the minification filter
     *
     * @returns the minification filter value
     */
    virtual ACD_TEXTURE_FILTER getMinFilter() const = 0;

    /**
     * Performs the blito operation
     *
     * 
     */
    virtual void performBlitOperation2D(acd_uint xoffset, acd_uint yoffset, acd_uint x, acd_uint y, acd_uint width, acd_uint height, acd_uint textureWidth, ACD_FORMAT internalFormat, ACDTexture2D* tex2D, acd_uint level) = 0;
};

}

#endif // ACD_SAMPLER

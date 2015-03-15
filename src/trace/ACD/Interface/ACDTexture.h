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

#ifndef ACD_TEXTURE
    #define ACD_TEXTURE

#include "ACDTypes.h"
#include "ACDResource.h"

namespace acdlib
{

static const acd_uint ACD_MIN_TEXTURE_LEVEL =  0;
static const acd_uint ACD_MAX_TEXTURE_LEVEL = 12;

/**
 * Base interface to handle Atila textures
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @version 1.0
 * @date 28/03/2007
 */
class ACDTexture : public ACDResource
{
public:

    virtual acd_uint getBaseLevel() const = 0;

    virtual acd_uint getMaxLevel() const = 0;

    /**
     * Sets the base level of detail of the texture
     *
     * By default = 0
     *
     * Equivalent to setLOD in DirectX 9
     */
    virtual void setBaseLevel(acd_uint minMipLevel) = 0;

    /**
     * Sets the maximum level of detail of the texture (clamp to maximum defined)
     *
     * By default the maximum level defined
     */ 
    virtual void setMaxLevel(acd_uint maxMipLevel) = 0;

    /**
     * Get the number of mipmaps actually setted
     *
     * 
     */ 
	virtual acd_uint getSettedMipmaps() = 0;
};

}

#endif // ACD_TEXTURE

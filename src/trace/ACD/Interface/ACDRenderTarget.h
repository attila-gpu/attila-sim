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

#ifndef ACD_RENDERTARGET
    #define ACD_RENDERTARGET

#include "ACDTypes.h"

namespace acdlib
{
/**
 * Render target interface is an abstraction of a generic render target subresource
 *
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @date 02/12/2007
 */
class ACDRenderTarget
{

public:

    /**
     *
     *  Get the width in pixels of the render target.
     *
     *  @return The width in pixels of the render target.
     */
     
    virtual acd_uint getWidth() const = 0;

    /**
     *
     *  Get the height in pixels of the render target.
     *
     *  @return The height in pixels of the render target.
     */

    virtual acd_uint getHeight() const = 0;
    
    /** 
     *
     *  Get the format of the render target.
     *
     *  @return The format of the render target.
     *
     */
     
    virtual ACD_FORMAT getFormat() const = 0;
    
    /**
     *
     *  Get if the render target supports multisampling.
     *
     *  @return If the render target supports multisampling.
     *
     */
     
    virtual acd_bool isMultisampled() const = 0;
    
    /** 
     *
     *  Get the number of samples supported per pixel for the render target.
     *
     *  @return The number of samples supporter per pixel for the render target.
     *
     */
     
    virtual acd_uint getSamples() const = 0;
    
    /**
     *
     *  Get if the render targets allows compression.
     *
     *  @return If the render target allows compression.
     *
     */
     
    virtual acd_bool allowsCompression() const = 0;

    /**
     *
     *  Get the texture that holds the renderTarget
     *
     *  @return The thexture that holds the renderTarget
     *
     */
    virtual ACDTexture* getTexture() = 0;

};

}

#endif // ACD_RENDERTARGET

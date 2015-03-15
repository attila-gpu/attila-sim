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

#ifndef ACD_TEXTURE3D
    #define ACD_TEXTURE3D

#include "ACDTexture.h"

namespace acdlib
{

/**
 * This interface represents 3D dimensional texture (interface not published yet)
 */
class ACDTexture3D : public ACDTexture
{
public:

    /**
     * Returns the width in texels of the most detailed mipmap level texture
     *
     * @returns the texture width in texels 
     */
    virtual acd_uint getWidth(acd_uint mipmap) const = 0;

    /**
     * Returns the height in texels of the most detailed mipmap level of the texture
     *
     * @returns the texture height in texels
     *
     * @note Always 0 for 1D textures
     */
    virtual acd_uint getHeight(acd_uint mipmap) const = 0;

    /**
     * Returns the depth in texels of the most detailed mipmap level of the texture
     *
     * @returns the texture depth in texels
     *
     * @note Always 0 for 1D textures
     */
    virtual acd_uint getDepth(acd_uint mipmap) const = 0;

    /**
     * Gets the texture format
     *
     * @returns The texture format
     */
    virtual ACD_FORMAT getFormat(acd_uint mipmap) const = 0;

    /**
     *
     *  Get if the texture is multisampled.
     *
     *  @param mipmap Mipmap of the texture for which information is requested.
     * 
     *  @return If the texture is multisampled.
     *
     */
     
    virtual acd_bool isMultisampled(acd_uint mipmap) const = 0;
    
    /**
     *
     *  Get the number of samples per pixel defined for the texture.
     *
     *  @param mipmap Mipmap of the texture for which information is requested.
     *
     *  @return The samples per pixel defiend for the texture.
     *
     */     
     
    virtual acd_uint getSamples(acd_uint mipmap) const = 0;     
     
    /**
     * Returns the size of the texels in the texture (in bytes)
     *
     * @returns the size of one texel of the texture
     */
    virtual acd_uint getTexelSize(acd_uint mipmap) const = 0;

    /**
     * Sets/resets data into a texture mipmap
     *
     * @note It is assumed that source texel data contains 'height' rows of size width * getTexelSize()
     *       each initial texel of each two consecutive rows is separated by rowPitch bytes
     */
    virtual void setData( acd_uint mipLevel,
                          acd_uint width,
                          acd_uint height,
                          acd_uint depth,
                          ACD_FORMAT format,
                          acd_uint rowPitch,
                          const acd_ubyte* srcTexelData,
                          acd_uint texSize,
                          acd_bool preloadData = false) = 0;

    /**
     *
     *  Defines a texture mipmap.
     *
     *  @param mipLevel Mipmap level in the texture to be defined.
     *  @param width Width in pixels of the mipmap.
     *  @param height Height in pixels of the mipmap.
     *  @param multisampling Defines if the 
     */
    virtual void setData(acd_uint mipLevel,
                         acd_uint width,
                         acd_uint height,
                         acd_uint depth,
                         acd_bool multisampling,
                         acd_uint samples,
                         ACD_FORMAT format) = 0;
    /**
     * Update texture contents of a subresource region
     *
     * @param mipLevel Mipmap level to update
     * @param x left coordinate of the 2D rectangle to update
     * @param y top coordinate of the 2D rectangle to update
     * @param width Width of the 2D rectagle to update
     * @param height Height of the 2D rectangle to update
     * @param rowPitch Total bytes of a row including the memory layout padding
     * @param srcTexelData Texel data to update the texture
     * @param preloadData Defines if the data is preloaded (zero simulation cost).
     *
     * @note the srcData is expected to contain height rows and width columns
     *       after each row an amount of bytes (rowPaddingBytes)
     *       must be skipped before access the next row
     *
     * @note Use update(miplevel,0,0,0,0,rowPaddingBytes,srcData) to update
     *       the whole mipmap level
     *        
     */
    virtual void updateData( acd_uint mipLevel,
                             acd_uint x,
                             acd_uint y,
                             acd_uint z,
                             acd_uint width,
                             acd_uint height,
                             acd_uint depth,
                             ACD_FORMAT format,
                             acd_uint rowPitch,
                             const acd_ubyte* srcTexelData,
                             acd_bool preloadData = false) = 0;

    /**
     * Allows to map a texture subresource to the user address space
     *
     * @note Only 
     *
     * @param mipLevel to map
     * @param mapType type of mapping requested
     * @retval pData pointer to the texture mipmap data
     * @retval dataRowPitch
     *
     * @code
     *
     *    // Reset a texture mipmap
     * 
     *    acd_uint rowPitch;
     *    acd_ubyte* pData;
     *
     *    t->map(0, ACD_MAP_WRITE_DISCARDPREV, pData, rowPitch);
     *         memZero(pData, t->getHeight() * rowPitch);
     *    t->unmap();
     *
     * @endcode
     */
    virtual acd_bool map( acd_uint mipLevel,
                      ACD_MAP mapType,
                      acd_ubyte*& pData,
                      acd_uint& dataRowPitch,
                      acd_uint& dataPlanePitch) = 0;

    /**
     * Unmaps a mipmap subresource from a 3D texture
     *
     * @param mipLevel the mipmap level to unmap 
     */
    virtual acd_bool unmap(acd_uint mipLevel, acd_bool preloadData = false) = 0; 
};

}

#endif // ACD_TEXTURE3D

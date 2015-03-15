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

#ifndef ACD_TEXTURECUBEMAP
    #define ACD_TEXTURECUBEMAP

#include "ACDTexture.h"

namespace acdlib
{

/**
 * Cube map faces definition
 */
enum ACD_CUBEMAP_FACE
{
    ACD_CUBEMAP_POSITIVE_X = 0, ///< The X positive face
    ACD_CUBEMAP_NEGATIVE_X, ///< The X negative face
    ACD_CUBEMAP_POSITIVE_y, ///< The Y positive face
    ACD_CUBEMAP_NEGATIVE_Y, ///< The Y negative face
    ACD_CUBEMAP_POSITIVE_Z, ///< The Z positive face
    ACD_CUBEMAP_NEGATIVE_Z, ///< The Z negative face
};

/**
 * This interface represents a 6-face 2D dimensional texture
 */
class ACDTextureCubeMap : public ACDTexture
{
public:

    /**
     * Returns the width in texels of the most detailed mipmap level texture
     *
     * @returns the texture width in texels 
     */
    virtual acd_uint getWidth(ACD_CUBEMAP_FACE face, acd_uint mipmap) const = 0;

    /**
     * Returns the height in texels of the most detailed mipmap level of the texture
     *
     * @returns the texture height in texels
     *
     * @note Always 0 for 1D textures
     */
    virtual acd_uint getHeight(ACD_CUBEMAP_FACE face, acd_uint mipmap) const = 0;

    /**
     * Gets the texture format
     *
     * @returns The texture format
     */
    virtual ACD_FORMAT getFormat(ACD_CUBEMAP_FACE face, acd_uint mipmap) const = 0;

    /**
     * Returns the size of the texels in the texture (in bytes)
     *
     * @returns the size of one texel of the texture
     */
    virtual acd_uint getTexelSize(ACD_CUBEMAP_FACE face, acd_uint mipmap) const = 0;

    /**
     * Sets/resets data into a texture mipmap
     */
    virtual void setData( ACD_CUBEMAP_FACE face,
                          acd_uint mipLevel,
                          acd_uint width,
                          acd_uint height,
                          ACD_FORMAT format,
                          acd_uint rowPitch,
                          const acd_ubyte* srcTexelData,
                          acd_uint texSize,
                          acd_bool preloadData = false) = 0;

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
    virtual void updateData( ACD_CUBEMAP_FACE face,
                             acd_uint mipLevel,
                             acd_uint x,
                             acd_uint y,
                             acd_uint width,
                             acd_uint height,
                             ACD_FORMAT format,
                             acd_uint rowPitch,
                             const acd_ubyte* srcTexelData,
                             acd_bool preloadData = false) = 0;
    

    virtual acd_bool map( ACD_CUBEMAP_FACE face,
                          acd_uint mipLevel,
                          ACD_MAP mapType,
                          acd_ubyte*& pData,
                          acd_uint& dataRowPitch ) = 0;

    virtual acd_bool unmap(ACD_CUBEMAP_FACE face, acd_uint mipLevel, acd_bool preloadData = false) = 0;

};

}

#endif // ACD_TEXTURECUBEMAP

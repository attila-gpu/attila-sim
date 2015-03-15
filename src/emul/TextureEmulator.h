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
 * $RCSfile: TextureEmulator.h,v $
 * $Revision: 1.20 $
 * $Author: vmoya $
 * $Date: 2008-03-02 19:09:16 $
 *
 * Texture Emulator class definition file.
 *
 */

/**
 *
 *  @file TextureEmulator.h
 *
 *  This file defines the Texture Emulator class.
 *
 *  This class defines functions that emulate the
 *  texture unit functionality of a GPU.
 *
 */

#ifndef _TEXTUREEMULATOR_

#define _TEXTUREEMULATOR_

#include "GPUTypes.h"
#include "GPU.h"
#include "support.h"
#include "OptimizedDynamicMemory.h"
#include "PixelMapper.h"

namespace gpu3d
{

/***  Defines the maximum texture lod bias.  */
static const f32bit MAX_TEXTURE_LOD_BIAS = 1000.0f;

/***  Defines the magnification to minification transition point.
      The first value is used when the magnification filter is LINEAR and
      the minification filter is GPU_NEAREST_MIPMAP_NEAREST or
      GPU_NEAREST_MIPMAP_LINEAR.  The second is used for all other filter
      modes.  */
static const f32bit C1 = 0.5f;
static const f32bit C2 = 0.0f;

/**
 *
 *  Defines the address space for uncompressed textures.
 *
 */
//#define UNCOMPRESSED_TEXTURE_SPACE 0x0000000000000000ULL
static const u64bit UNCOMPRESSED_TEXTURE_SPACE = 0x0000000000000000ULL;

/**
 *
 *  Defines the address space for compressed textures using DXT1 RGB mode (1:8 ratio).
 *
 */

//#define COMPRESSED_TEXTURE_SPACE_DXT1_RGB 0x1000000000000000ULL
static const u64bit COMPRESSED_TEXTURE_SPACE_DXT1_RGB = 0x1000000000000000ULL;


/**
 *
 *  Defines the address space for compressed textures using DXT1 RGBA mode (1:8 ratio).
 *
 */

// #define COMPRESSED_TEXTURE_SPACE_DXT1_RGBA 0x2000000000000000ULL
static const u64bit COMPRESSED_TEXTURE_SPACE_DXT1_RGBA = 0x2000000000000000ULL;

/**
 *
 *  Defines the address space for compressed textures using DXT3 RGBA mode (1:4 ratio).
 *
 */

static const u64bit COMPRESSED_TEXTURE_SPACE_DXT3_RGBA = 0x3000000000000000ULL;

/**
 *
 *  Defines the address space for compressed textures using DXT5 RGBA mode (1:4 ratio).
 *
 */

static const u64bit COMPRESSED_TEXTURE_SPACE_DXT5_RGBA = 0x4000000000000000ULL;

/**
 *
 *  Defines the address space for compressed textures using LATC1 mode (1:2 ratio).
 *
 */

static const u64bit COMPRESSED_TEXTURE_SPACE_LATC1 = 0x5000000000000000ULL;

/**
 *
 *  Defines the address space for compressed textures using LATC1 signed mode (1:2 ratio).
 *
 */

static const u64bit COMPRESSED_TEXTURE_SPACE_LATC1_SIGNED = 0x6000000000000000ULL;

/**
 *
 *  Defines the address space for compressed textures using LATC2 mode (1:2 ratio).
 *
 */

static const u64bit COMPRESSED_TEXTURE_SPACE_LATC2 = 0x7000000000000000ULL;

/**
 *
 *  Defines the address space for compressed textures using LATC2 signed mode (1:2 ratio).
 *
 */

static const u64bit COMPRESSED_TEXTURE_SPACE_LATC2_SIGNED = 0x8000000000000000ULL;

/**
 *
 *  Defines the black texel address.
 *
 */
static const u64bit BLACK_TEXEL_ADDRESS = 0x00ffffffffffffffULL;

/**
 *
 *  Defines the texture address space mask.
 *
 */

static const u64bit TEXTURE_ADDRESS_SPACE_MASK = 0xff00000000000000ULL;

/**
 *
 *  Defines the size of a S3TC DXT1 compressed 4x4 block in bytes.
 *
 */

static const u32bit S3TC_DXT1_BLOCK_SIZE_SHIFT = 3;

/**
 *
 *  Defines the size of a S3TC DXT3/DXT5 compressed 4x4 block in bytes.
 *
 */

static const u32bit S3TC_DXT3_DXT5_BLOCK_SIZE_SHIFT = 4;

/**
 *
 *  Defines the size of a LATC1 compressed 4x4 block in bytes.
 *
 */
static const u32bit LATC1_BLOCK_SIZE_SHIFT = 3;

/**
 *
 *  Defines the size of a LATC2 compressed 4x4 block in bytes.
 *
 */ 
static const u32bit LATC2_BLOCK_SIZE_SHIFT = 4;
 
/**
 *
 *  Defines the shift to translate a GPU memory address to/from DXT1 RGB/RGBA address space.
 *
 */

static const u32bit DXT1_SPACE_SHIFT = 3;

/**
 *
 *  Defines the shift to translate GPU memory address to/from DXT3/DXT5 RGBA address space.
 *
 */

static const u32bit DXT3_DXT5_SPACE_SHIFT = 2;

/**
 *
 *  Defines the shift to translate a GPU memory address to/from LATC1/LATC2 RGBA address space.
 *
 */
 
static const u32bit LATC1_LATC2_SPACE_SHIFT = 1;
 
/**
 *
 *  Defines the maximum amount of data required for an attribute.
 *
 */

static const u32bit MAX_ATTRIBUTE_DATA = 16; 


/**
 *
 *  Defines the type of the operation requested to the Texture Unit.
 */
enum TextureOperation
{
    TEXTURE_READ,
    TEXTURE_READ_WITH_LOD,
    ATTRIBUTE_READ
};

/**
 *
 *  This class stores information for a fragment stamp (quad)
 *  texture access.
 *
 *  Inherits from the OptimizedDynamicMemory class that offers
 *  cheap memory management.
 *
 */

class TextureAccess: public OptimizedDynamicMemory
{
public:

    /**
     *
     *  Defines the main axis for anisotropic filtering.
     *
     */
    enum AnisotropyAxis
    {
        X_AXIS,      /**<  Horizontal axis.  */
        Y_AXIS,      /**<  Vertical axis.  */
        XY_AXIS,     /**<  Lineal combination.  */
        YX_AXIS
    };

    /**
     *
     *  Defines a trilinear texture sample.
     *
     *      - for 1D: up to two texels.
     *      - for 2D: up to eight texels.
     *      - for 3D: up to sixteen texels.
     *
     */

    class Trilinear : public OptimizedDynamicMemory
    {
        public:

        u32bit i[STAMP_FRAGMENTS][16];          /**<  Texel i coordinate (in texels) for the trilinear sample.  */
        u32bit j[STAMP_FRAGMENTS][16];          /**<  Texel j coordinate (in texels) for the trilinear sample.  */
        u32bit k[STAMP_FRAGMENTS][16];          /**<  Texel k coordinate (in texels) for the trilinear sample.  */
        f32bit a[STAMP_FRAGMENTS][2];           /**<  Weight factors for horizontal texture dimension.  */
        f32bit b[STAMP_FRAGMENTS][2];           /**<  Weight factors for vertical texture dimension.  */
        f32bit c[STAMP_FRAGMENTS][2];           /**<  Weight factor for the depth texture dimension.  */
        u64bit address[STAMP_FRAGMENTS][16];    /**<  Memory address for each texel in the the trilinear sample.  */
        u32bit way[STAMP_FRAGMENTS][16];        /**<  Texture cache way for each texel in the trilinear sample.  */
        u32bit line[STAMP_FRAGMENTS][16];       /**<  Texture cache line for each texel the trilinear sample.  */
        u64bit tag[STAMP_FRAGMENTS][16];        /**<  Stores the tag for each texel in the trilinear sample.  */
        bool fetched[STAMP_FRAGMENTS][16];      /**<  Flag storing if the texel has been fetched.  */
        bool ready[STAMP_FRAGMENTS][16];        /**<  Flag storing if the texel data is available in the cache.  */
        bool read[STAMP_FRAGMENTS][16];         /**<  Flag storing if the texel has been read.  */
        bool sampleFromTwoMips[STAMP_FRAGMENTS];    /**<  Flag storing if two mips have to be sampled.  */
        u32bit loops[STAMP_FRAGMENTS];          /**<  Number of fetch/read/filter loops required to calculate the trilinear texture sample.  */
        u32bit texelsLoop[STAMP_FRAGMENTS];     /**<  Number of texels to fetch/read for each loop.  */
        u32bit fetchLoop[STAMP_FRAGMENTS];      /**<  Current fetch loop.  */
        u32bit readLoop[STAMP_FRAGMENTS];       /**<  Current read/filter loop.  */
        QuadFloat texel[STAMP_FRAGMENTS][16];   /**<  Read texels for the trilinear sample.  */
        QuadFloat sample[STAMP_FRAGMENTS];      /**<  Filtered texture sample for the trilinear sample.  */
        u8bit attrData[STAMP_FRAGMENTS][MAX_ATTRIBUTE_DATA];    /**<  Maximum size of an attribute read.  */
        u32bit attrFirstOffset[STAMP_FRAGMENTS];                /**<  Offset for the first attribute data byte in the first cache read.  */
        u32bit attrFirstSize[STAMP_FRAGMENTS];                  /**<  Bytes from the first cache read to store.  */
        u32bit attrSecondSize[STAMP_FRAGMENTS];                 /**<  Bytes from the first cache read to store.  */
    };

    TextureOperation texOperation;          /**<  Type of texture operation to perform.  */
    QuadFloat coordinates[STAMP_FRAGMENTS]; /**<  The fragment texture access coordinates.  */
    QuadFloat originalCoord[STAMP_FRAGMENTS];   /**<  Stores the original fragment texture access coordinates.  */
    f32bit parameter[STAMP_FRAGMENTS];      /**<  Fragment parameter (lod/bias).  */
    f32bit reference[STAMP_FRAGMENTS];      /**<  Reference value for comparison filter (PCF).  */
    u32bit textUnit;                        /**<  Texture unit where the access is performed.  */
    f32bit lod[STAMP_FRAGMENTS];            /**<  Calculated LOD for each fragment.  */
    CubeMapFace cubemap;                    /**<  Cubemap image to be accessed by the texture access.  */
    u32bit level[STAMP_FRAGMENTS][2];       /**<  Mipmaps for each fragment.  */
    u32bit anisoSamples;                    /**<  Number of anisotropic samples to take.  */
    u32bit currentAnisoSample;              /**<  Current anisotropic sample.  */
    f32bit anisodsOffset;                   /**<  Per anisotropic sample offset for texture coordinate s component.  */
    f32bit anisodtOffset;                   /**<  Per anisotropic sample offset for texture coordinate t component.  */
    FilterMode filter[STAMP_FRAGMENTS];     /**<  Filter to apply to the sampled texels.  */
    u32bit magnified;                       /**<  Number of pixels in the quad that are magnified.  */
    u32bit accessID;                        /**<  Identifier of the texture access/Pointer to the entry in the shader emulator texture queue.  */
    u64bit cycle;                           /**<  Stores the cycle when the texture access was created.  */
    QuadFloat sample[STAMP_FRAGMENTS];      /**<  Final filtered texture sample.  */
    Trilinear *trilinear[MAX_ANISOTROPY];   /**<  Stores the trilinear samples to take for the texture access.  */
    u32bit nextTrilinearFetch;              /**<  Next trilinear sample in the texture access for which to fetch texels.  */
    u32bit trilinearFiltered;               /**<  Number of trilinear samples already filtered.  */
    u32bit trilinearToFilter;               /**<  Number of trilinear samples remaining to be filtered.  */
    bool addressCalculated;                 /**<  All the trilinear addresses have been calculated.  */
    u32bit texelSize[STAMP_FRAGMENTS];      /**<  Bytes per texel.  */

    /**
     *
     *  Texture Access constructor.
     *
     *  @param texOp Type of texture operation to perform.
     *  @param coord Pointer to texture coordinates being accessed by the fragments in
     *  the stamp (quad).
     *  @param parameter Pointer to a parameter (lod/bias) of the fragments in the stamp (quad).
     *  @param textUnit Identifier of the texture unit being accessed by the texture
     *  access.
     *
     *  @return A new texture access object.
     *
     */

    TextureAccess(u32bit id, TextureOperation texOp, QuadFloat *coord, f32bit *parameter, u32bit textUnit);

    /**
     *
     *  Texture access destructor.
     *
     */

    ~TextureAccess();

};

//  Defines the different anisotropy algorithm available in the emulator
enum AnisotropyAlgorithm
{
    ANISO_TWO_AXIS    = 0,  /**<  Detect anisotropy on the X and Y screen axis.  */
    ANISO_FOUR_AXIS   = 1,  /**<  Detect anisotropy on the X and Y and 45 degree rotated X and Y screen axis.  */
    ANISO_RECTANGULAR = 2,  /**<  Detect anisotropy based on a rectangular approximation of
                                  the pixel projection on texture space along the X and Y and
                                  45 degree rotated X and Y screen axis.  */
    ANISO_EWA          = 3, /**<  Detect anisotropy using Heckbert's EWA algorithm.  */
    ANISO_EXPERIMENTAL = 4  /**<  Experimental anisotropy algorithm.  */
};

/**
 *
 *  This class implements emulation functions that implement
 *  the texture unit in a GPU.
 *
 */


class TextureEmulator
{

private:

    //  Parameters.
    u32bit stampFragments;      /**<  Fragments per stamp.  */
    u32bit textCacheBlockDim;   /**<  Dimension of a texture cache block.  Dimension is the n exponent in a block with a size of 2^n x 2^n texels.  */
    u32bit textCacheBlockSize;  /**<  Texels in a block (derived from textCacheBlockDim).  */
    u32bit textCacheSBlockDim;  /**<  Dimension of a texture cache superblock (in blocks).  Dimension is the n exponent in a block with the size of 2^n x 2^n blocks.  */
    u32bit textCacheSBlockSize; /**<  Size in blocks of a texture cache superblock.  */
    u32bit anisoAlgorithm;      /**<  Selected anisotropy algorithm.  */
    bool forceMaxAnisotropy;    /**<  Forces the anisotropy of all textures to the maximum anisotropy from the configuration file.  */
    u32bit confMaxAniso;        /**<  Maximum anisotropy allowed by the configuration file.  */
    u32bit trilinearPrecision;  /**<  Bits of precision for the fractional part of the trilinear LOD.  */
    u32bit brilinearThreshold;  /**<  Threshold used to decide when to enable trilinear (sample from two mipmaps).  Based on trilinear precision.  */
    u32bit anisoRoundPrecision; /**<  Bits of precision for the fractional part of the aniso ratio.  Used to compute rounding.  */
    u32bit anisoRoundThreshold; /**<  Threshold used to round the computed aniso ratio to the next supported integral aniso ratio.  */
    bool anisoRatioMultOfTwo;   /**<  Flag that selects if the final aniso ratio must be a multiple of 2.  */
    u32bit overScanTileWidth;   /**<  Over scan tile width in scan tiles.  */
    u32bit overScanTileHeight;  /**<  Over scan tile height in scan tiles.  */
    u32bit scanTileWidth;       /**<  Scan tile width in generation tiles.  */
    u32bit scanTileHeight;      /**<  Scan tile height in generation tiles.  */
    u32bit genTileWidth;        /**<  Scan tile width in pixels/texels.  */
    u32bit genTileHeight;       /**<  Scan tile height in pixels/texels.  */

    //  Precomputed constants.
    f32bit trilinearRangeMin;   /**<  Minimum LOD fractional value at which trilinear (sampling from two mipmaps) is triggered.  */
    f32bit trilinearRangeMax;   /**<  Maximum LOD fractional value at which trilinear (sampling from two mipmaps) is triggered.  */
    f32bit anisoRoundUp;        /**<  Point at which the next valid number of aniso samples is used.  */    
    
    /*  GPU Texture Unit registers.  */
    bool textureEnabled[MAX_TEXTURES];      /**<  Texture unit enable flag.  */
    TextureMode textureMode[MAX_TEXTURES];  /**<  Current texture mode active in the texture unit.  */
    u32bit textureAddress[MAX_TEXTURES][MAX_TEXTURE_SIZE][CUBEMAP_IMAGES];  /**<  Address in GPU memory of the active texture mipmaps.  */
    u32bit textureWidth[MAX_TEXTURES];      /**<  Active texture width in texels.  */
    u32bit textureHeight[MAX_TEXTURES];     /**<  Active texture height in texels.  */
    u32bit textureDepth[MAX_TEXTURES];      /**<  Active texture depth in texels.  */
    u32bit textureWidth2[MAX_TEXTURES];     /**<  Log2 of the texture width (base mipmap size).  */
    u32bit textureHeight2[MAX_TEXTURES];    /**<  Log2 of the texture height (base mipmap size).  */
    u32bit textureDepth2[MAX_TEXTURES];     /**<  Log2 of the texture depth (base mipmap size).  */
    u32bit textureBorder[MAX_TEXTURES];     /**<  Texture border in texels.  */
    TextureFormat textureFormat[MAX_TEXTURES];  /**<  Texture format of the active texture.  */
    bool textureReverse[MAX_TEXTURES];          /**<  Reverses texture data (from little to big endian).  */
    bool textD3D9ColorConv[MAX_TEXTURES];       /**<  Sets the color component order to the order defined by D3D9.  */
    bool textD3D9VInvert[MAX_TEXTURES];         /**<  Forces the u coordinate to be inverted as specified by D3D9.  */
    TextureCompression textureCompr[MAX_TEXTURES];  /**<  Texture compression mode of the active texture.  */
    TextureBlocking textureBlocking[MAX_TEXTURES];  /**<  Texture blocking mode for the texture.  */
    QuadFloat textBorderColor[MAX_TEXTURES];    /**<  Texture border color.  */
    ClampMode textureWrapS[MAX_TEXTURES];       /**<  Texture wrap mode for s coordinate.  */
    ClampMode textureWrapT[MAX_TEXTURES];       /**<  Texture wrap mode for t coordinate.  */
    ClampMode textureWrapR[MAX_TEXTURES];       /**<  Texture wrap mode for r coordinate.  */
    bool textureNonNormalized[MAX_TEXTURES];    /**<  Texture coordinates are non-normalized.  */
    FilterMode textureMinFilter[MAX_TEXTURES];  /**<  Texture minification filter.  */
    FilterMode textureMagFilter[MAX_TEXTURES];  /**<  Texture Magnification filter.  */
    bool textureEnableComparison[MAX_TEXTURES]; /**<  Texture Enable Comparison filter (PCF).  */
    CompareMode textureComparisonFunction[MAX_TEXTURES];  /**<  Texture Comparison function (PCF).  */
    bool textureSRGB[MAX_TEXTURES];             /**<  Texture sRGB space to linear space conversion.  */
    f32bit textureMinLOD[MAX_TEXTURES];         /**<  Texture minimum lod.  */
    f32bit textureMaxLOD[MAX_TEXTURES];         /**<  Texture maximum lod.  */
    f32bit textureLODBias[MAX_TEXTURES];        /**<  Texture lod bias.  */
    u32bit textureMinLevel[MAX_TEXTURES];       /**<  Texture minimum mipmap level.  */
    u32bit textureMaxLevel[MAX_TEXTURES];       /**<  Texture maximum mipmap level.  */
    f32bit textureUnitLODBias[MAX_TEXTURES];    /**<  Texture unit lod bias (not texture lod!!).  */
    u32bit maxAnisotropy[MAX_TEXTURES];         /**<  Maximum anisotropy for the texture.  */

    //  Pixel mapper (for framebuffer textures).
    PixelMapper texPixelMapper[MAX_TEXTURES];   /**<  Maps texels/pixels to addresses.  */
    bool pixelMapperConfigured[MAX_TEXTURES];   /**<  Stores if the corresponding pixel mapper has been properly configured.  */
    
    //  Stream input registers.
    u32bit attributeMap[MAX_VERTEX_ATTRIBUTES];     /**<  Mapping from vertex input attributes and vertex streams.  */
    QuadFloat attrDefValue[MAX_VERTEX_ATTRIBUTES];  /**<  Defines the vertex attribute default values.  */
    u32bit streamAddress[MAX_STREAM_BUFFERS];       /**<  Address in GPU memory for the vertex stream buffers.  */
    u32bit streamStride[MAX_STREAM_BUFFERS];        /**<  Stride for the stream buffers.  */
    StreamData streamData[MAX_STREAM_BUFFERS];      /**<  Data type for the stream buffer.  */
    u32bit streamElements[MAX_STREAM_BUFFERS];      /**<  Number of stream data elements (vectors) per stream buffer entry.  */
    bool d3d9ColorStream[MAX_STREAM_BUFFERS];       /**<  Read components of the color attributes in the order defined by D3D9.  */

    //  Derived from the registers.
    u32bit streamDataSize[MAX_STREAM_BUFFERS];      /**<  Bytes per stream read.  */
    u32bit streamElementSize[MAX_STREAM_BUFFERS];   /**<  Bytes per stream element.  */
    
    
    /*  Private functions.  */

    /**
     *
     *  Expands the texel coordinates for a bilinear texture access from the first
     *  texel coordinates.
     *
     *  @param textUnit The texture unit where the bilinear access is being done.
     *  @param textAccess A reference to a TextureAccess object describing a stamp
     *  of fragments performing a texture access.  The expanded texel coordinates
     *  are stored for a fragment and a mipmap level inside this object.
     *  @param trilinearAccess The trilinear access for which to expand the texel coordinates.
     *  @param frag The fragment for which to expand the texel coordinates.
     *  @param level The mipmap level for the bilinear access.
     *  @param i Horizontal texel coordinate.
     *  @param j Vertical texel coordinate.
     *  @param k Depth texel coordinate.
     *  @param l Identifies one of two bilinear access for trilinear.
     *
     */

    void genBilinearTexels(u32bit textUnit, TextureAccess &textAccess, u32bit trilinearAccess,
        u32bit frag, u32bit level, u32bit i, u32bit j, u32bit k, u32bit l);

    /**
     *
     *  Calculates the derivatives in x and y for fragment stamp.
     *
     *  @param coordinates Pointer to the texture coordinates of the fragments in the stamp
     *  accessing the texture unit.
     *  @param textUnit Texture unit/stage that is being accessed.
     *  @param dudx Reference to a float variable where to store the derivative.
     *  @param dudy Reference to a float variable where to store the derivative.
     *  @param dvdx Reference to a float variable where to store the derivative.
     *  @param dvdy Reference to a float variable where to store the derivative.
     *  @param dwdx Reference to a float variable where to store the derivative.
     *  @param dwdy Reference to a float variable where to store the derivative.
     *
     */
    void  derivativesXY(QuadFloat *coordinates, u32bit textUnit, f32bit &dudx, f32bit &dudy,
        f32bit &dvdx, f32bit &dvdy, f32bit &dwdx, f32bit &dwdy);

    /**
     *
     *  Calculates the level of detail scale factor for GPU_TEXTURE1D textures.
     *
     *  @param dudx Derivative on x.
     *  @param dudy Derivative on y.
     *
     *  @return The scale factor for the level of detail equation.
     *
     */

    f32bit calculateScale1D(f32bit dudx, f32bit dudy);

    /**
     *
     *  Calculates the level of detail scale factor for GPU_TEXTURE2D textures.
     *
     *  @param textUnit Texture unit being accessed.
     *  @param dudx Derivative on x.
     *  @param dudy Derivative on y.
     *  @param dvdx Derivative on x.
     *  @param dvdy Derivative on y.
     *  @param maxAniso Maximum anisotropy supported for the texture.
     *  @param anisoSamples Reference to a variable where to store the number of anisotropic samples
     *  to take.
     *  @param dsOffset Increment offset for texture coordinate s component per anisotropic sample in the
     *  anisotropy axis direction.
     *  @param dtOffsett Increment offset for texture coordinate t component per anisotropic sample in the
     *  anisotropy axis direction.
     *
     *
     *  @return The scale factor for the level of detail equation.
     *
     */

    f32bit calculateScale2D(u32bit textUnit, f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, u32bit maxAniso,
        u32bit &anisoSamples, f32bit &dsOffset, f32bit &dtOffset);

    /**
     *
     *  Computes the number of anisotropic samples based on the anisotropic ratio.
     *
     *  @param anisoRatio The compute anisotropic ratio.
     *
     *  @return The number of aniso samples to process based on the aniso rounding parameters.
     *
     */
     
    u32bit computeAnisoSamples(f32bit anisoRatio);

    /**
     *
     *  Computes anisotropy for the texture access based on the projection of the pixel
     *  over the texture area in terms of the screen aligned X and Y axis.
     *
     *  @param dudx Derivative on x.
     *  @param dudy Derivative on y.
     *  @param dvdx Derivative on x.
     *  @param dvdy Derivative on y.
     *  @param maxAniso Maximum anisotropy supported for the texture.
     *  @param anisoSamples Reference to a variable where to store the number of anisotropic samples
     *  to take.
     *  @param dsOffset Increment offset for texture coordinate s component per anisotropic sample in the
     *  anisotropy axis direction.
     *  @param dtOffsett Increment offset for texture coordinate t component per anisotropic sample in the
     *  anisotropy axis direction.
     *
     *
     *  @return The scale factor for the level of detail equation.
     *
     */

    f32bit anisoTwoAxis(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, u32bit maxAniso,
        u32bit &anisoSamples, f32bit &dsOffset, f32bit &dtOffset);

    /**
     *
     *  Computes anisotropy for the texture access based on the projection of the pixel
     *  over the texture area in terms of the screen aligned X and Y axis and the 45 degree
     *  rotation of the screen X and Y axis.
     *
     *  @param dudx Derivative on x.
     *  @param dudy Derivative on y.
     *  @param dvdx Derivative on x.
     *  @param dvdy Derivative on y.
     *  @param maxAniso Maximum anisotropy supported for the texture.
     *  @param anisoSamples Reference to a variable where to store the number of anisotropic samples
     *  to take.
     *  @param dsOffset Increment offset for texture coordinate s component per anisotropic sample in the
     *  anisotropy axis direction.
     *  @param dtOffsett Increment offset for texture coordinate t component per anisotropic sample in the
     *  anisotropy axis direction.
     *
     *
     *  @return The scale factor for the level of detail equation.
     *
     */

    f32bit anisoFourAxis(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, u32bit maxAniso,
        u32bit &anisoSamples, f32bit &dsOffset, f32bit &dtOffset);

    /**
     *
     *  Computes anisotropy for the texture access based on a rectangular approximation of
     *  the projection of a pixel over the texture.  The screen X and Y axis and the 45 degree
     *  rotated screen X and Y axis are used to select the directing/major edge of the rectangle.
     *
     *
     *  @param dudx Derivative on x.
     *  @param dudy Derivative on y.
     *  @param dvdx Derivative on x.
     *  @param dvdy Derivative on y.
     *  @param maxAniso Maximum anisotropy supported for the texture.
     *  @param anisoSamples Reference to a variable where to store the number of anisotropic samples
     *  to take.
     *  @param dsOffset Increment offset for texture coordinate s component per anisotropic sample in the
     *  anisotropy axis direction.
     *  @param dtOffsett Increment offset for texture coordinate t component per anisotropic sample in the
     *  anisotropy axis direction.
     *
     *
     *  @return The scale factor for the level of detail equation.
     *
     */

    f32bit anisoRectangle(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, u32bit maxAniso,
        u32bit &anisoSamples, f32bit &dsOffset, f32bit &dtOffset);

    /**
     *
     *  Computes anisotropy for the texture access based on Heckbert's EWA algorithm.
     *
     *  @param dudx Derivative on x.
     *  @param dudy Derivative on y.
     *  @param dvdx Derivative on x.
     *  @param dvdy Derivative on y.
     *  @param maxAniso Maximum anisotropy supported for the texture.
     *  @param anisoSamples Reference to a variable where to store the number of anisotropic samples
     *  to take.
     *  @param dsOffset Increment offset for texture coordinate s component per anisotropic sample in the
     *  anisotropy axis direction.
     *  @param dtOffsett Increment offset for texture coordinate t component per anisotropic sample in the
     *  anisotropy axis direction.
     *
     *
     *  @return The scale factor for the level of detail equation.
     *
     */

    f32bit anisoEWA(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, u32bit maxAniso,
        u32bit &anisoSamples, f32bit &dsOffset, f32bit &dtOffset);

    /**
     *
     *  Computes anisotropy for the texture access based on an experimental algorithm that takes into account
     *  the angle between the X and Y axis.
     *
     *  @param dudx Derivative on x.
     *  @param dudy Derivative on y.
     *  @param dvdx Derivative on x.
     *  @param dvdy Derivative on y.
     *  @param maxAniso Maximum anisotropy supported for the texture.
     *  @param anisoSamples Reference to a variable where to store the number of anisotropic samples
     *  to take.
     *  @param dsOffset Increment offset for texture coordinate s component per anisotropic sample in the
     *  anisotropy axis direction.
     *  @param dtOffsett Increment offset for texture coordinate t component per anisotropic sample in the
     *  anisotropy axis direction.
     *
     *
     *  @return The scale factor for the level of detail equation.
     *
     */

    f32bit anisoExperimentalAngle(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, u32bit maxAniso,
        u32bit &anisoSamples, f32bit &dsOffset, f32bit &dtOffset);

    /**
     *
     *  Calculates the level of detail scale factor for GPU_TEXTURE3D textures.
     *
     *  @param dudx Derivative on x.
     *  @param dudy Derivative on y.
     *  @param dvdx Derivative on x.
     *  @param dvdy Derivative on y.
     *  @param dwdx Derivative on x.
     *  @param dwdy Derivative on y.
     *
     *  @return The scale factor for the level of detail equation.
     *
     */

    f32bit calculateScale3D(f32bit dudx, f32bit dudy, f32bit dvdx, f32bit dvdy, f32bit dwdx, f32bit dwdy);

    /**
     *
     *  Calculates, biases and clamps the lod for a fragment.
     *
     *  @param textUnit The texture unit being accessed by the fragment.
     *  @param fragmentBias The fragment lod bias.
     *  @param scaleFactor The scale factor calculated from the derivatives.
     *
     *  @return The clamped lod for the fragment.
     *
     */

    f32bit calculateLOD(u32bit textUnit, f32bit fragmentBias, f32bit scaleFactor);

    /**
     *
     *  Calculates texel coordinates for the first texel and weight factors for the
     *  filter for a fragment using accessing a texture unit at coordinates {s, t, r}.
     *
     *  @param textUnit The texture unit that is being accessed by the fragment.
     *  @param l The mipmap level where the access is performed.
     *  @param filter The filter that is going to be used at the mipmap level
     *  (either GPU_LINEAR or GPU_NEAREST).
     *  @param currentSample Current anisotropic sample for which to calculate the texture coordinates.
     *  @param samples Number of total anisotropic samples to take for the texture access.
     *  @param dsOffset Texture coordinate component s offset in the anisotropy axis direction per anisotropic sample.
     *  @param dtOffset Texture coordinate component t offset in the anisotropy axis direction per anisotropic sample.
     *  @param s Fragment s texture coordinate for the access.
     *  @param t Fragment t texture coordinate for the access.
     *  @param r Fragment r texture coordinate for the access.
     *  @param i Reference to an integer variable where to store the texel horizontal coordinate.
     *  @param j Reference to an integer variable where to store the texel vertical coordinate.
     *  @param k Reference to an integer variable where to store the texel depth coordinate.
     *  @param a Reference to a float point variable where to store the horizontal weight factor for the linear filter.
     *  @param b Reference to a float point variable where to store the vertical weight factor for the linear filter.
     *  @param c Reference to a float point variable where to store the depth weight factor for the linear filter.
     *
     */

    void texelCoord(u32bit textUnit, u32bit l, FilterMode filter, u32bit currentSample, u32bit samples,
        f32bit dsOffset, f32bit dtOffset, f32bit s, f32bit t, f32bit r,
        u32bit &i, u32bit &j, u32bit &k, f32bit &a, f32bit &b, f32bit &c);

    /**
     *
     *  Calculates the first texel cooordinates for a mipmap level of a GPU_TEXTURE1D texture
     *  being accesed by a fragment.
     *
     *  @param textUnit The texture unit being accssed.
     *  @param l The mipmap level being accessed.
     *  @param filter The filter being used at the mipmap level (GPU_LINEAR or GPU_NEAREST).
     *  @param s Fragment s texture coordinate for the access.
     *  @param i Reference an integer variable where to store the texel horizontal coordinate.
     *  @param a Reference to a float point variable where to store the vertical weight factor for the linear filter.
     *
     */

    void texelCoord1D(u32bit textUnit, u32bit l, FilterMode filter, f32bit s, u32bit &i, f32bit &a);

    /**
     *
     *  Calculates the first texel cooordinates for a mipmap level of a GPU_TEXTURE2D texture
     *  being accesed by a fragment.
     *
     *  @param textUnit The texture unit being accssed.
     *  @param l The mipmap level being accessed.
     *  @param filter The filter being used at the mipmap level (GPU_LINEAR or GPU_NEAREST).
     *  @param currentSample Current anisotropic sample for which to calculate the texture coordinates.
     *  @param samples Number of total anisotropic samples to take for the texture access.
     *  @param dsOffset Texture coordinate component s offset in the anisotropy axis direction per anisotropic sample.
     *  @param dtOffset Texture coordinate component t offset in the anisotropy axis direction per anisotropic sample.
     *  @param s Fragment s texture coordinate for the access.
     *  @param t Fragment t texture coordinate for the access.
     *  @param i Reference an integer variable where to store the texel horizontal coordinate.
     *  @param j Reference an integer variable where to store the texel vertical coordinate.
     *  @param a Reference to a float point variable where to store the horizontal weight factor for the linear filter.
     *  @param b Reference to a float point variable where to store the vertical weight factor for the linear filter.
     *
     */

    void texelCoord2D(u32bit textUnit, u32bit l, FilterMode filter, u32bit currentSample, u32bit samples,
        f32bit dsOffset, f32bit dtOffset, f32bit s, f32bit t, u32bit &i, u32bit &j, f32bit &a, f32bit &b);

    /**
     *
     *  Calculates the first texel cooordinates for a mipmap level of a GPU_TEXTURE3D texture
     *  being accesed by a fragment.
     *
     *  @param textUnit The texture unit being accssed.
     *  @param l The mipmap level being accessed.
     *  @param filter The filter being used at the mipmap level (GPU_LINEAR or GPU_NEAREST).
     *  @param s Fragment s texture coordinate for the access.
     *  @param t Fragment t texture coordinate for the access.
     *  @param r Fragment r texture coordinate for the access.
     *  @param i Reference an integer variable where to store the texel horizontal coordinate.
     *  @param j Reference an integer variable where to store the texel vertical coordinate.
     *  @param k Reference an integer variable where to store the texel depth coordinate.
     *  @param a Reference to a float point variable where to store the horizontal weight factor for the linear filter.
     *  @param b Reference to a float point variable where to store the vertical weight factor for the linear filter.
     *  @param c Reference to a float point variable where to store the vertical depth factor for the linear filter.
     *
     */

    void texelCoord3D(u32bit textUnit, u32bit l, FilterMode filter,
        f32bit s, f32bit t, f32bit r, u32bit &i, u32bit &j, u32bit &k, f32bit &a, f32bit &b, f32bit &c);

    /**
     *
     *  Applies a texture wrapping mode to a texture coordinate component.
     *
     *  @param wrapMode Wrapping mode to be applied to the texture coordinate component.
     *  @param a The texture coordinate componet being wrapped.
     *  @param size Size in texels of the mipmap level being accessed.
     *
     *  @return The wrapped texture coordinate.
     *
     */

    f32bit applyWrap(ClampMode wrapMode, f32bit a, u32bit size);

    /**
     *
     *  Applies a texture wrapping mode to a texel coordinate.
     *
     *  @param wrapMode Wrapping mode to be applied to the texel coordinate.
     *  @param i The texel coordinate component being wrapped.
     *  @param size Size in texels of the mipmap level being accessed.
     *
     *  @return The wrapped texel coordinate.
     *
     */

    u32bit applyWrap(ClampMode wrapMode, u32bit i, u32bit size);

    /**
     *
     *  Performs a bilinear filtering at a mipmap level for a fragment in a stamp texture access.
     *
     *  @param textAccess The stamp texture access for which a bilinear sample must be calculated.
     *  @param trilinearAccess The trilinear access for which a bilinear sample must be calculated.
     *  @param level Which mipmap level (for trilinear) is being sampled.
     *  @param frag The fragment inside the stamp for which the bilinear sample must be calculated.
     *
     *  @return The filtered value.
     *
     */

    QuadFloat bilinearFilter(TextureAccess &textAccess, u32bit trilinearAccess, u32bit level, u32bit frag);

    /**
     *
     *  Adjust a texel address to the size and type of the texture format.
     *
     *  @param textUnit Identifier of the texture unit being addressed.
     *  @param texelAddress The linear address of the texel inside the texture.
     *
     *  @return The modified address of the texel taking into account the size,
     *  components and type of the texture format.
     *
     */

    u64bit adjustToFormat(u32bit textUnit, u64bit texelAddres);

    /**
     *
     *  Calculates the size (as 2^n exponent) of the texture mipmap level dimension.
     *
     *  @param topSize Logarithm of 2 of the largest mipmap level for the texture in
     *  a given dimension.
     *  @param level The mipmap level for which we want to calculate the logarithm of 2 of
     *  the dimension size.
     *
     *  @return The logarithm of 2 of the size of a given dimension of the mipmap level.
     *
     */

    u32bit mipmapSize(u32bit topSize, u32bit level);

    /**
     *
     *  Translates a texture texel address to a physical texture memory address for 1D textures.
     *
     *  @param textUnit The texture unit that is being accessed.
     *  @param level The mipmap level that is being accessed.
     *  @param i The texel coordinate inside the mipmap level of the linear 1D accessed texture.
     *
     *  @return The memory address of the addressed texel.
     *
     */

    u64bit texel2address(u32bit textUnit, u32bit level, u32bit i);

    /**
     *
     *  Translates a texture texel address to a physical texture memory address for 2D textures.
     *
     *  @param textUnit The texture unit of the texture being accessed.
     *  @param level The mipmap level of the texture being accessed.
     *  @param cubemap Cubemap image index for the texel.
     *  @param i The horizontal coordinate of the texel inside the 2D mipmap level.
     *  @param j The vertical coordinate of the texel inside the 2D mipmap level.
     *
     *  @return The memory address of the texel.
     *
     */

    u64bit texel2address(u32bit textUnit, u32bit level, u32bit cubemap, u32bit i, u32bit j);

    /**
     *
     *  Translates a texture texel address to a physical texture memory address for 3D textures.
     *
     *  @param textUnit The texture unit of the texture being accessed.
     *  @param level The mipmap level of the texture being accessed.
     *  @param cubemap Cubemap image index for the texel.
     *  @param i The horizontal coordinate of the texel inside the 3D mipmap level.
     *  @param j The vertical coordinate of the texel inside the 3D mipmap level.
     *  @param k The depth coordinate of the texel inside the 3D mipmap level.
     *
     *  @return The memory address of the texel.
     *
     */

    u64bit texel2address(u32bit textUnit, u32bit level, u32bit cubemap, u32bit i, u32bit j, u32bit k);

    /**
     *
     *  Translates a attribute stream index to a serie of physical memory addresses.
     *
     *  @param attribute The attribute stream to read.
     *  @param textAccess Texture Access where to store the addresses to read.
     *  @param trilinearAccess Trilinear element inside the Texture Access object where to store the addresses.
     *  @param frag The element in the 4-group for which the addresses are computed.
     *  @param index The stream index used to read from the atttribute stream.
     *
     */

    void index2address(u32bit attribute, TextureAccess &textAccess, u32bit trilinearAccess, u32bit frag, u32bit index);

    /**
     *
     *  Converts the data read for an attribute into the attribute final format (SIMD4 float32) and fills
     *  the elements of the attribute vector with valid or default data based on the attribute/stream
     *  register settings.
     *
     *  @param texAccess Reference to the Texture Access object where to store the attribute data.
     *  @param trilinearAccess Identifier of the TrilinearAccess object inside the TextureAccess object
     *  where to store the attribute data.
     *  @param frag The element in the 4-group for which the attribute data is being converted/filled.
     *
     */
     
    void loadAttribute(TextureAccess &texAccess, u32bit trilinearAccess, u32bit frag);

    /**
     *
     *  Converts data read for an attribute element to the attribute final format (float32).
     *
     *  @param format Format of the attribute.
     *  @param data Pointer to the data read for the attribute element.
     *
     *  @result The attribute element value converted to float32.
     *
     */
     
    f32bit attributeDataConvert(StreamData format, u8bit *data);

    /**
     *
     *  Selects the face of a cubemap being accessed by a set of texture coordinates for
     *  a stamp and converts those coordinates into coordinates to the selected 2D texture
     *  image.
     *
     *  @param coord QuadFloat array with the texture coordinates of the texture access to
     *  a cubemap for a stamp.  The coordinates are recalculated to address the selected
     *  2D texture image.
     *
     *  @return The identifier of the cubemap face selected.
     *
     */

    CubeMapFace selectCubeMapFace(QuadFloat *coord);

    /**
     *
     *  Converts a QuadFloat color into a given format.
     *
     *  @param format Texture format to which the input QuadFloat color is to be converted.
     *  @param color QuadFloat source color.
     *
     *  @return A 32 bit unsigned integer that stores the color converted into the destination
     *  format.
     *
     */

    static u32bit format(TextureFormat format, QuadFloat color);

    /**
     *
     *  Decodes and selects the proper alpha value for S3TC alpha encoding.
     *
     *  @param code The 3 bit alpha code for the texel.
     *  @param alpha0 First reference alpha value.
     *  @param alpha1 Second reference alpha value.
     *
     *  @return The decoded alpha value.
     *
     */

    static f32bit decodeS3TCAlpha(u32bit code, f32bit alpha0, f32bit alpha1);


    /**
     *
     *  Decodes and selects the proper color for non-transparent S3TC color encoding.
     *
     *  @param code The 2 bit color code for the texel.
     *  @param RGB0 The first reference color.
     *  @param RGB1 The second reference color.
     *  @param output A reference to the QuadFloat variable where to store the decoded
     *  color.
     *
     */

    static void nonTransparentS3TCRGB(u32bit code, QuadFloat RGB0, QuadFloat RGB1, QuadFloat &output);

    /**
     *
     *  Decodes and selects the proper color for transparent ST3C color encoding.
     *
     *  @param code The 2 bit color code for the texel.
     *  @param RGB0 The first reference color.
     *  @param RGB1 The second reference color.
     *  @param output A reference to the QuadFloat variable where to store the decoded
     *  color.
     *
     */

    static void transparentS3TCRGB(u32bit code, QuadFloat RGBA0, QuadFloat RGBA1, QuadFloat &output);


public:

    /**
     *
     *  Texture Emulator constructor.
     *
     *  @param stampFrags Number of fragments per stamp.
     *  @param blockDim Dimension of a texture cache block (the exponent in a block of 2^n x 2^n texels).
     *  @param superBlockDim Dimension of a texture cache super block (the exponent in a superblock of 2^n x 2^n blocks).
     *  @param anisoAlgo Selects the anisotropy algorithm to be used by the Texture Emulator.
     *  @param forceAniso Flag used to force the maximum anisotropy from the configuration file to all textures.
     *  @param maxAniso Maximum anisotropy defined in the configuration file.
     *  @param triPrecision Bits of precision of the fractional part of the LOD used to decide when to trigger trilinear.
     *  @param brilinearThreshold Threshold, based on the trilinear precision, used to trigger trilinear (sampling from two mipmaps).
     *  @param anisoRoundPrecision Bits of precision of the fractional part of the aniso ratio used to round to the final integer
     *  number of samples to use for anisotropic filtering.
     *  @param anisoRoundThreshold Threshold, based on the trilinear precision, used to round to the next valid integer number of
     *  samples to use for anisotropic filtering.
     *  @param anisoRatioMultOfTwo Flag that selects if the number of samples for anisotropic filtering must be a multiple of two.
     *  @param overScanWidth Width of a frame buffer over scan tile (in scan tiles).
     *  @param overScanHeight Height of a frame buffer over scan tile (in scan tiles).
     *  @param scanWidth Width of a frame buffer scan tile (in pixels).
     *  @param scanHeight Height of a frame buffer scan tile (in pixels).
     *  @param genWidth Width of a frame buffer generation tile (in pixels).
     *  @param genHeight Height of a frame buffer generation tile (in pixels).     
     *
     *  @return A new texture emulator object.
     *
     */

    TextureEmulator(u32bit stampFrags, u32bit blockDim, u32bit superBlockDim, u32bit anisoAlgo,
                    bool forceAniso, u32bit maxAniso, u32bit triPrecision, u32bit brilinearThreshold,
                    u32bit anisoRoundPrecision, u32bit anisoRoundThreshold, bool anisoRatioMultOfTwo,
                    u32bit overScanWidth, u32bit overScanHeight, u32bit scanWidth, u32bit scanHeight,
                    u32bit genWidth, u32bit genHeight);

    /**
     *
     *  Calculates the texel address for a stamp.
     *
     *  @param id Identifier of the texture access in the Shader Emulator.
     *  @param texOp Operation requested to the texture emulator.
     *  @param stampCoord  Pointer to the array of texture coordinates generated by the
     *  fragments in the stamp accessing the texture unit.
     *  @param stampParameter Per fragment (in the stamp) parameter (lod/bias).
     *  @param textUnit Texture unit that is being accessed by the fragments in the stamp.
     *
     *  @return A TextureAccess object that describes the current texture access for
     *  for the given fragment stamp and texture unit.
     *
     */

    TextureAccess *textureOperation(u32bit id, TextureOperation texOp, QuadFloat *stampCoord, f32bit *stampParameter, u32bit textUnit);

    /**
     *
     *  Calculates the addresses for all the texels in the texture access.
     *
     *  @param textAccess Texture access for which to calculate the texel addresses.
     *
     */

    void calculateAddress(TextureAccess *textAccess);

    /**
     *  Filters the texels read for a stamp texture access and generates the final sample
     *  value.
     *
     *  @param textAccess Reference to the stamp texture access that is going to be
     *  filtered.
     *  @param nextTrilinear Trilinear sample to filter.
     *
     */

    void filter(TextureAccess &textAccess, u32bit nextTrilinear);

    /**
     *  Filters the texels read for a trilinear and generates the final sample
     *  value.
     *
     *  @param textAccess Reference to the stamp texture access that is going to be filtered.
     *  @param trilinearAccess Trilinear access in the texture access that is to be filtered.
     *
     */

    void filterTrilinear(TextureAccess &textAccess, u32bit trilinearAccess);

    /**
     *  Converts and stores texel data from their native source to float point format.
     *
     *  @param textAccess Reference to a texture access object.
     *  @param trilinearAccess Trilinear access in the texture access to be converted.
     *  @param frag The fragment in the texture access.
     *  @param tex  The texel in the texture access.
     *  @param data Pointer to the texel data.
     *
     */

    void convertFormat(TextureAccess &textAccess, u32bit trilinearAccess, u32bit frag, u32bit tex, u8bit *data);


    /**
     *
     *  Compares the texel value with the reference value using the defined comparison function
     *  to implement PCF (Percentage Close Filtering).
     *  Returns 0.0f (false) or 1.0f (true) based on the result of the comparison.
     *
     *  @param function The comparison function to use.
     *  @param reference The reference value to compare the texel value with.
     *  @param texel The texel value.
     *
     *  @return If the result of the comparison is TRUE return 1.0f, otherwise return 0.0f.
     *
     */
     
    f32bit comparisonFilter(CompareMode function, f32bit reference, f32bit texel);

    /**
     *
     *  Decodes a S3TC compressed 4x4 block using DXT1 encoding for RGB format.
     *  The output block texels are stored in Morton order.
     *
     *  @param inBuffer Pointer to an array of bytes where the compressed input data
     *  is stored.
     *  @param outBuffer Pointer to an array of bytes where the decompressed block
     *  is to be stored.
     *
     */

    static void decodeBlockDXT1RGB(u8bit *inBuffer, u8bit *outBuffer);

    /**
     *
     *  Decodes a S3TC compressed 4x4 block using DXT1 encoding for RGBA format.
     *  The output block texels are stored in Morton order.
     *
     *  @param inBuffer Pointer to an array of bytes where the compressed input data
     *  is stored.
     *  @param outBuffer Pointer to an array of bytes where the decompressed block
     *  is to be stored.
     *
     */

    static void decodeBlockDXT1RGBA(u8bit *inBuffer, u8bit *outBuffer);

    /**
     *
     *  Decodes a S3TC compressed 4x4 block using DXT3 encoding for RGBA format.
     *  The output block texels are stored in Morton order.
     *
     *  @param inBuffer Pointer to an array of bytes where the compressed input data
     *  is stored.
     *  @param outBuffer Pointer to an array of bytes where the decompressed block
     *  is to be stored.
     *
     */

    static void decodeBlockDXT3RGBA(u8bit *inBuffer, u8bit *outBuffer);

    /**
     *
     *  Decodes a S3TC compressed 4x4 block using DXT5 encoding for RGBA format.
     *  The output block texels are stored in Morton order.
     *
     *  @param inBuffer Pointer to an array of bytes where the compressed input data
     *  is stored.
     *  @param outBuffer Pointer to an array of bytes where the decompressed block
     *  is to be stored.
     *
     */

    static void decodeBlockDXT5RGBA(u8bit *inBuffer, u8bit *outBuffer);

    /**
     *
     *  Decodes a LATC1 compressed 4x4 block.
     *  The output block texels are stored in Morton order.
     *
     *  @param inBuffer Pointer to an array of bytes where the compressed input data
     *  is stored.
     *  @param outBuffer Pointer to an array of bytes where the decompressed block
     *  is to be stored.
     *
     */

    static void decodeBlockLATC1(u8bit *inBuffer, u8bit *outBuffer);

    /**
     *
     *  Decodes a LATC1_SIGNED compressed 4x4 block.
     *  The output block texels are stored in Morton order.
     *
     *  @param inBuffer Pointer to an array of bytes where the compressed input data
     *  is stored.
     *  @param outBuffer Pointer to an array of bytes where the decompressed block
     *  is to be stored.
     *
     */

    static void decodeBlockLATC1Signed(u8bit *inBuffer, u8bit *outBuffer);

    /**
     *
     *  Decodes a LATC2 compressed 4x4 block.
     *  The output block texels are stored in Morton order.
     *
     *  @param inBuffer Pointer to an array of bytes where the compressed input data
     *  is stored.
     *  @param outBuffer Pointer to an array of bytes where the decompressed block
     *  is to be stored.
     *
     */

    static void decodeBlockLATC2(u8bit *inBuffer, u8bit *outBuffer);

    /**
     *
     *  Decodes a LATC2_SIGNED compressed 4x4 block.
     *  The output block texels are stored in Morton order.
     *
     *  @param inBuffer Pointer to an array of bytes where the compressed input data
     *  is stored.
     *  @param outBuffer Pointer to an array of bytes where the decompressed block
     *  is to be stored.
     *
     */

    static void decodeBlockLATC2Signed(u8bit *inBuffer, u8bit *outBuffer);
    
    /**
     *
     *  Decompresses a number of S3TC DXT1 RGB compressed blocks.
     *
     *  @param input Pointer to an input byte array with the compressed data.
     *  @param output Pointer to an output byte array where to store the uncompressed data.
     *  @param size Size of the input data.
     *
     */

    static void decompressDXT1RGB(u8bit *input, u8bit *output, u32bit size);

    /**
     *
     *  Decompresses a number of S3TC DXT1 RGBA compressed blocks.
     *
     *  @param input Pointer to an input byte array with the compressed data.
     *  @param output Pointer to an output byte array where to store the uncompressed data.
     *  @param size Size of the input data.
     *
     */

    static void decompressDXT1RGBA(u8bit *input, u8bit *output, u32bit size);

    /**
     *
     *  Decompresses a number of S3TC DXT3 RGBA compressed blocks.
     *
     *  @param input Pointer to an input byte array with the compressed data.
     *  @param output Pointer to an output byte array where to store the uncompressed data.
     *  @param size Size of the input data.
     *
     */

    static void decompressDXT3RGBA(u8bit *input, u8bit *output, u32bit size);

    /**
     *
     *  Decompresses a number of S3TC DXT5 RGBA compressed blocks.
     *
     *  @param input Pointer to an input byte array with the compressed data.
     *  @param output Pointer to an output byte array where to store the uncompressed data.
     *  @param size Size of the input data.
     *
     */

    static void decompressDXT5RGBA(u8bit *input, u8bit *output, u32bit size);

    /**
     *
     *  Decompresses a number of LATC1 compressed blocks.
     *
     *  @param input Pointer to an input byte array with the compressed data.
     *  @param output Pointer to an output byte array where to store the uncompressed data.
     *  @param size Size of the input data.
     *
     */

    static void decompressLATC1(u8bit *input, u8bit *output, u32bit size);

    /**
     *
     *  Decompresses a number of LATC1_SIGNED compressed blocks.
     *
     *  @param input Pointer to an input byte array with the compressed data.
     *  @param output Pointer to an output byte array where to store the uncompressed data.
     *  @param size Size of the input data.
     *
     */

    static void decompressLATC1Signed(u8bit *input, u8bit *output, u32bit size);

    /**
     *
     *  Decompresses a number of LATC2 compressed blocks.
     *
     *  @param input Pointer to an input byte array with the compressed data.
     *  @param output Pointer to an output byte array where to store the uncompressed data.
     *  @param size Size of the input data.
     *
     */

    static void decompressLATC2(u8bit *input, u8bit *output, u32bit size);

    /**
     *
     *  Decompresses a number of LATC2_SIGNED compressed blocks.
     *
     *  @param input Pointer to an input byte array with the compressed data.
     *  @param output Pointer to an output byte array where to store the uncompressed data.
     *  @param size Size of the input data.
     *
     */

    static void decompressLATC2Signed(u8bit *input, u8bit *output, u32bit size);

    
    
    /**
     *
     *  Reset the Texture Emulator internal state.
     *
     */

    void reset();

    /**
     *
     *  Writes a texture emulator register.
     *
     *  @param reg Texture Emulator register identifier.
     *  @param subreg Texture Emulator register subregister.
     *  @param data Data to write into the Texture Emulator register.
     *
     */

    void writeRegister(GPURegister reg, u32bit subReg, GPURegData data);

};

} // namespace gpu3d

#endif

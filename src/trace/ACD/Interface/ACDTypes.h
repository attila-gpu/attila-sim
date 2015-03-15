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

#ifndef ACD_TYPES
    #define ACD_TYPES

namespace acdlib
{
    typedef void acd_void;
    typedef bool acd_bool;
    typedef char acd_char;

    typedef char acd_byte;
    typedef short acd_short;
    typedef int acd_int;
    
    typedef unsigned char acd_ubyte;
    typedef unsigned short acd_ushort;
    typedef unsigned int acd_uint;

    typedef unsigned long long acd_ulonglong;

    typedef float acd_float;
    typedef double acd_double;
    
    typedef acd_ushort acd_enum;   

    /**
     * Represents/defines a 3D bounding box
     */
    struct ACD_BOX
    {
        acd_uint left; ///< x position of the left handside of the box
        acd_uint top; ///< y position of the top of the box
        acd_uint front; ///< z position of the front of the box
        acd_uint right; ///< x position of the right handside of the box
        acd_uint bottom; ///< y position of the bottom of the box 
        acd_uint back; ///< z position of the back of the box
    };

    /**
     * Types of mapping
     */
    enum ACD_MAP
    {
        ACD_MAP_READ,
        ACD_MAP_WRITE,
        ACD_MAP_READ_WRITE,
        ACD_MAP_WRITE_DISCARDPREV,
        ACD_MAP_WRITE_NOOVERWRITE
    };

    /**
     * Data formats supported
     *
     * @note More to be defined.
     */
    enum ACD_FORMAT
    {
        ACD_FORMAT_UNKNOWN,

        // Uncompressed formats
        ACD_FORMAT_RGBA_8888,
        ACD_FORMAT_RGB_565,
        ACD_FORMAT_INTENSITY_8,
        ACD_FORMAT_INTENSITY_12,
        ACD_FORMAT_INTENSITY_16,
        ACD_FORMAT_ALPHA_8,
        ACD_FORMAT_ALPHA_12,
        ACD_FORMAT_ALPHA_16,
        ACD_FORMAT_LUMINANCE_8,
        ACD_FORMAT_LUMINANCE_12,
        ACD_FORMAT_LUMINANCE_16,
        ACD_FORMAT_LUMINANCE8_ALPHA8,
        ACD_FORMAT_DEPTH_COMPONENT_16,
        ACD_FORMAT_DEPTH_COMPONENT_24,
        ACD_FORMAT_DEPTH_COMPONENT_32,
        ACD_FORMAT_RGB_888,
        ACD_FORMAT_ARGB_8888,
        ACD_FORMAT_ABGR_161616,
        ACD_FORMAT_XRGB_8888,
        ACD_FORMAT_QWVU_8888,
        ACD_FORMAT_ARGB_1555,
        ACD_FORMAT_UNSIGNED_INT_8888,
        ACD_FORMAT_RG16F,
        ACD_FORMAT_R32F,
        ACD_FORMAT_S8D24,
        ACD_FORMAT_RGBA16F,
        ACD_FORMAT_RGBA32F,

        ACD_FORMAT_NULL,

        // Compressed formats
        ACD_COMPRESSED_S3TC_DXT1_RGB,
        ACD_COMPRESSED_S3TC_DXT1_RGBA,
        ACD_COMPRESSED_S3TC_DXT3_RGBA,
        ACD_COMPRESSED_S3TC_DXT5_RGBA,
        ACD_COMPRESSED_LUMINANCE_LATC1_EXT,
        ACD_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT,
        ACD_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT,
        ACD_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT
        // Add more compressed formats here
    };

    /**
     * Render target dimension identifier
     */
    enum ACD_RT_DIMENSION
    {
        ACD_RT_DIMENSION_UNKNOWN,
        ACD_RT_DIMENSION_BUFFER,
        ACD_RT_DIMENSION_TEXTURE1D,
        ACD_RT_DIMENSION_TEXTURE2D,
        ACD_RT_DIMENSION_TEXTURE2DMS,
        ACD_RT_DIMENSION_TEXTURECM,
        ACD_RT_DIMENSION_TEXTURE3D,
    };


    /**
     * Render target parameters for a buffer used as a render target
     */
    struct ACD_RT_BUFFER 
    {
        acd_uint elementOffset; ///< Number of bytes from the beginning of the buffer to the first element to be accessed
        acd_uint elementWidth; ///< Number of bytes in each element in the buffer
    };

    /**
     * Render target parameters for a texture 1D mipmap
     */
    struct ACD_RT_TEXTURE1D
    {
        acd_uint mipmap;
    };

    /**
     * Render target parameters for a texture 2D mipmap
     */
    struct ACD_RT_TEXTURE2D 
    { 
        acd_uint mipmap; 
    };

    /**
     * Render target parameters for a texture 3D mipmap
     */
    struct ACD_RT_TEXTURE3D 
    { 
        acd_uint mipmap;
        acd_uint depth;
    };

    /**
     * Descriptor to define how a render target is created
     */
    struct ACD_RT_DESC
    {
        ACD_FORMAT format;
        ACD_RT_DIMENSION dimension;
        union // Per dimension type informaton
        {
            ACD_RT_BUFFER buffer;
            ACD_RT_TEXTURE1D texture1D;
            ACD_RT_TEXTURE2D texture2D;
            ACD_RT_TEXTURE3D texture3D;
        };
    };

}





#endif // ACD_TYPES

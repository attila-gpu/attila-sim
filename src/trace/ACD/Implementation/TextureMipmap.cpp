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

#include "TextureMipmap.h"
#include "support.h"
#include <cstring>
#include "ACDMath.h"
#include "GPUDriver.h"
#include <sstream>

#include "GlobalProfiler.h"

using namespace acdlib;
using namespace std;

acd_uint TextureMipmap::_tileLevel2Sz = 0;
acd_uint TextureMipmap::_tileLevel1Sz = 0;
GPUDriver* TextureMipmap::_driver = 0;

void TextureMipmap::setTextureTiling(acd_uint tileLevel1Sz, acd_uint tileLevel2Sz)
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")
    static acd_bool alreadyCalled = false;

    if ( !alreadyCalled )
    {
        alreadyCalled = true;
        _tileLevel1Sz = tileLevel1Sz;
        _tileLevel2Sz = tileLevel2Sz;
        GLOBALPROFILER_EXITREGION()
    }
}

void TextureMipmap::setDriver(GPUDriver* driver)
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")
    static acd_bool alreadyCalled = false;

    if ( !alreadyCalled )
    {
        alreadyCalled = true;
        _driver = driver;
        GLOBALPROFILER_EXITREGION()
    }
}


void TextureMipmap::getTextureTiling(acd_uint& tileLevel1Sz, acd_uint& tileLevel2Sz)
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")
    tileLevel1Sz = _tileLevel1Sz;
    tileLevel2Sz = _tileLevel2Sz;
    GLOBALPROFILER_EXITREGION()
}

TextureMipmap::TextureMipmap() : _data(0), _dataSize(0), _rowPitch(0), _planePitch(0), _mortonData(0), 
                                 _width(0), _height(0), _depth(0), _format(ACD_FORMAT_RGBA_8888),
                                 _multisampling(false), _samples(1)
{
    if ( _tileLevel1Sz == 0 || _tileLevel1Sz == 0 )
        panic("TextureMipmap", "ctor", "Tile level sizes not defined (must be defined before any ctor call)");
    
}

acd_uint TextureMipmap::getWidth() const { return _width; }

acd_uint TextureMipmap::getHeight() const { return _height; }

acd_uint TextureMipmap::getDepth() const { return _depth; }

ACD_FORMAT TextureMipmap::getFormat() const { return _format; }

acd_bool TextureMipmap::isMultisampled() const { return _multisampling; }

acd_uint TextureMipmap::getSamples() const { return _samples; }

acd_float TextureMipmap::getTexelSize() const 
{
    return getTexelSize(_format);
}

acd_float TextureMipmap::getTexelSize(ACD_FORMAT format) 
{
    switch ( format ) 
    {
        case ACD_FORMAT_RGB_565:
            return 2;
            break;
        case ACD_FORMAT_RGB_888:
            return 3;
        case ACD_FORMAT_RGBA_8888:
        case ACD_FORMAT_ARGB_8888:
        case ACD_FORMAT_XRGB_8888:
        case ACD_FORMAT_QWVU_8888:
        case ACD_FORMAT_UNSIGNED_INT_8888:
        case ACD_FORMAT_DEPTH_COMPONENT_24:
            return 4;
        case ACD_FORMAT_RG16F:
            return 4;
            break;  
        case ACD_FORMAT_S8D24:
            return 4;
        case ACD_FORMAT_R32F:
            return 4;
            break;
        case ACD_FORMAT_RGBA32F:
            return 16;
            break;
        case ACD_FORMAT_ABGR_161616:
        case ACD_FORMAT_RGBA16F:
            return 8;
        case ACD_FORMAT_ARGB_1555:
            return 2;
        case ACD_FORMAT_INTENSITY_8:
        case ACD_FORMAT_ALPHA_8:
        case ACD_FORMAT_LUMINANCE_8:
            return 1;
        case ACD_FORMAT_LUMINANCE8_ALPHA8:
            return 2;
        case ACD_FORMAT_NULL:
            return 0;
        case ACD_COMPRESSED_S3TC_DXT1_RGB:
        case ACD_COMPRESSED_S3TC_DXT1_RGBA:
        case ACD_COMPRESSED_LUMINANCE_LATC1_EXT:
        case ACD_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            return 0.5;
        case ACD_COMPRESSED_S3TC_DXT3_RGBA:
        case ACD_COMPRESSED_S3TC_DXT5_RGBA:
        case ACD_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case ACD_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            return 1;
        default:
            char error[128];
            sprintf(error, "Unknown format size: %d", format);
            panic("TextureMipmap", "getTexelSize", error);
            return 0;
    }
}

bool TextureMipmap::getData(acd_ubyte*& pData, acd_uint& rowPitch, acd_uint& planePitch)
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")

    pData = _data;
    rowPitch = _rowPitch;
    planePitch = _planePitch;

    GLOBALPROFILER_EXITREGION()
    return true;
}

void TextureMipmap::dump2PPM (acd_ubyte* filename)
{
    FILE *fout;

    u8bit red;
    u8bit green;
    u8bit blue;
    acd_bool alpha = false;

    switch (_format)
    {
        case ACD_FORMAT_RGBA_8888:
        case ACD_FORMAT_DEPTH_COMPONENT_32:
            alpha = true;

        case ACD_FORMAT_RGB_888:
        case ACD_FORMAT_DEPTH_COMPONENT_24:
        case ACD_COMPRESSED_S3TC_DXT1_RGB:

            fout = fopen((const char*)filename, "wb");

            /*  Write magic number.  */
            fprintf(fout, "P6\n");

            /*  Write frame size.  */
            fprintf(fout, "%d %d\n", _width, _height);

            /*  Write color component maximum value.  */
            fprintf(fout, "255\n");

            // Supose unsigned byte

            acd_ubyte* index;
            index = _data;
         
            for (  int i = 0 ; i < _height; i++ )
            {
                for ( int j = 0; j < _width; j++ )
                {
                    fputc( char(*index), fout ); index++;
                    fputc( char(*index), fout ); index++;
                    fputc( char(*index), fout ); index++;
                    if (alpha) index++;
                }
            }

            fclose(fout);

            break;

        default:
            break;
    }

}

bool TextureMipmap::getData(const acd_ubyte*& pData, acd_uint& rowPitch, acd_uint& planePitch) const
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")

    pData = _data;
    rowPitch = _rowPitch;
    planePitch = _planePitch;

    GLOBALPROFILER_EXITREGION()

    return true;    
}

void TextureMipmap::setData( acd_uint width, acd_uint height, acd_uint depth, ACD_FORMAT format, 
                             acd_uint rowPitch, const acd_ubyte* srcTexelData, acd_uint texSize )
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")

    // Delete previous mipmap contents
    delete[] _data;
    delete[] _mortonData;

    // Mipmap data not converted to morton order.
    _mortonData = 0;
    _mortonDataSize = 0;

    // Update mipmap properties
    _format = format;
    _width = width;
    _height = height;
    _depth = depth;

    acd_bool compressed;

    switch ( format ) 
    {
        case ACD_COMPRESSED_S3TC_DXT1_RGB:
        case ACD_COMPRESSED_S3TC_DXT1_RGBA:
        case ACD_COMPRESSED_LUMINANCE_LATC1_EXT:
        case ACD_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:

            _dataSize = texSize;
            compressed = true;
            _rowPitch = max((acd_uint) 4, width) / 2;
            _planePitch = max((acd_uint) 16, width*height) / 2;
            break;

        case ACD_COMPRESSED_S3TC_DXT3_RGBA:
        case ACD_COMPRESSED_S3TC_DXT5_RGBA:
        case ACD_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
        case ACD_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:

            _dataSize = texSize;
            compressed = true;
            _rowPitch = max((acd_uint) 4, width);
            _planePitch = max((acd_uint) 16, width*height);
            break;

        default:
            _dataSize = width * height * depth * getTexelSize();
            _rowPitch = width * getTexelSize();
            _planePitch = height * width * getTexelSize();
            compressed = false;
    }

    // Allocate mipmap data
    _data = new acd_ubyte[_dataSize];

    if ( srcTexelData != 0 ) {
        if ( compressed || rowPitch == 0 || rowPitch == width )
            memcpy(_data, srcTexelData, _dataSize);
        else 
        {
            // Copy texture memory contiguous
            for ( acd_uint i = 0; i < (height*depth); ++i )
                memcpy(_data + i*_rowPitch, srcTexelData + i*rowPitch, _dataSize);
        }
    }

    GLOBALPROFILER_EXITREGION()
}

void TextureMipmap::setData(acd_uint width, acd_uint height, acd_uint depth, acd_bool multisampling, acd_uint samples, ACD_FORMAT format)
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")

    // Delete previous mipmap contents
    delete[] _data;
    delete[] _mortonData;

    // Mipmap data not converted to morton order.
    _mortonData = 0;
    _mortonDataSize = 0;

    //  Create mipmap with no defined data.
    _data = 0;
    _dataSize = 0;
    _rowPitch = 0;
    _planePitch = 0;

    // Update mipmap properties
    _format = format;
    _width = width;
    _height = height;
    _depth = depth;
    _multisampling = multisampling;
    _samples = samples;   

    GLOBALPROFILER_EXITREGION()
}

void TextureMipmap::updateData(const acd_ubyte* srcTexelData)
{
    // Invalidate current precomputed morton data
    if (_mortonData != NULL)
        delete[] _mortonData;

    _mortonData = NULL;

    memcpy(_data, srcTexelData, _dataSize);    
}

void TextureMipmap::copyData( acd_uint mipLevel, ACDTexture2D* destTexture)
{
    destTexture->updateData(mipLevel,_data);
}

void TextureMipmap::updateData( acd_uint xoffset, acd_uint yoffset, acd_uint zoffset, acd_uint width, acd_uint height, acd_uint depth, ACD_FORMAT format, 
                                acd_uint rowPitch, const acd_ubyte* srcTexelData )
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")
    if ( _data == 0 )
        panic("TextureMipmap", "updateData", "Mipmap has no defined data yet");

    if ( format != _format )
        panic("TextureMipmap", "updateData", "Incompatible formats");

    if ( xoffset + width > _width )
        panic("TextureMipmap", "updateData", "xoffset + width out of mipmap width bounds");

    if ( yoffset + height > _height )
        panic("TextureMipmap", "updateData", "yoffset + height out of mipmap height bounds");

    if ( srcTexelData == 0 )
        panic("TextureMipmap", "updateData", "updateData requires to supply data (not NULL)");

    // Invalidate current precomputed morton data
    if (_mortonData != NULL)
        delete[] _mortonData;

    _mortonData = NULL;

    //  Clamp the 
    acd_uint heightCompr;
    acd_uint depthCompr = 1;
    acd_uint blockSize = 0;

    switch ( _format)
    {
        case ACD_COMPRESSED_S3TC_DXT1_RGB:
        case ACD_COMPRESSED_S3TC_DXT1_RGBA:
            blockSize = 8;
            break;
        case ACD_COMPRESSED_S3TC_DXT3_RGBA:
        case ACD_COMPRESSED_S3TC_DXT5_RGBA:
            blockSize = 16;
            break;
        default:
            break;
    }

    switch ( _format )
    {
        case ACD_COMPRESSED_S3TC_DXT1_RGB:
        case ACD_COMPRESSED_S3TC_DXT1_RGBA:
        case ACD_COMPRESSED_S3TC_DXT3_RGBA:
        case ACD_COMPRESSED_S3TC_DXT5_RGBA:

            {

                if ((width % 4 != 0) && (width > 4))
                    panic("TextureMipmap","updateData","Width is not multiple of 4");

                if ((height % 4 != 0) && (height > 4))
                    panic("TextureMipmap","updateData","Height is not multiple of 4");

                if (xoffset % 4 != 0)
                    panic("TextureMipmap","updateData","Xoffset is not multiple of 4");

                if (yoffset % 4 != 0)
                    panic("TextureMipmap","updateData","Yoffset is not multiple of 4");

                acd_uint widthBlockSize = max(_width / 4, acd_uint(1));
                acd_uint heightBlockSize = max(_height / 4, acd_uint(1));
                acd_uint copyWidthBlockSize = max(width / 4, acd_uint(1));
                acd_uint copyHeightBlockSize = max(height / 4, acd_uint(1));

                acd_uint firstBlock = yoffset/4 * widthBlockSize  + xoffset/4;
                acd_uint lastBlock = (copyHeightBlockSize * widthBlockSize + copyWidthBlockSize);

                acd_ubyte* _initialData = _data + ( blockSize * firstBlock);

                acd_uint blockRowPitch = blockSize * widthBlockSize;
                acd_uint copyBlockRowPitch = blockSize * copyWidthBlockSize;

                for (acd_uint h = 0; h < copyHeightBlockSize; h++)
                        memcpy(_initialData + blockRowPitch*h, srcTexelData + copyBlockRowPitch*h, blockSize*copyWidthBlockSize);

            }
            break;

        default:
            heightCompr = height;
            depthCompr = depth;
   
            // Update mipmap data
            const acd_uint offset = xoffset * getTexelSize() + yoffset*_rowPitch + zoffset*_planePitch;

            acd_uint k = 0;

            for ( acd_uint j = 0; j < depthCompr; ++j)
                for ( acd_uint i = 0; i < heightCompr; ++i )
                {
                    memcpy(_data + offset + i*_rowPitch + j*_planePitch, srcTexelData + k*rowPitch, rowPitch);
                    k++;
                }

            break;
    }


    GLOBALPROFILER_EXITREGION()
}

const acd_ubyte* TextureMipmap::getDataInMortonOrder( acd_uint& sizeInBytes ) const
{
    GLOBALPROFILER_ENTERREGION("TextureMipmap", "", "")

    //  Check if the mipmap was already converted to morton order.
    if (!_mortonData)
    {
        gpu3d::TextureCompression compressed;
        switch ( _format )
        {
            case ACD_COMPRESSED_S3TC_DXT1_RGB:
                compressed = gpu3d::GPU_S3TC_DXT1_RGB;
                break;
            case ACD_COMPRESSED_S3TC_DXT1_RGBA:
                compressed = gpu3d::GPU_S3TC_DXT1_RGBA;
                break;
            case ACD_COMPRESSED_S3TC_DXT3_RGBA:
                compressed = gpu3d::GPU_S3TC_DXT3_RGBA;
                break;
            case ACD_COMPRESSED_S3TC_DXT5_RGBA:
                compressed = gpu3d::GPU_S3TC_DXT5_RGBA;
                break;

            case ACD_COMPRESSED_LUMINANCE_LATC1_EXT:
                compressed = gpu3d::GPU_LATC1;
                break;
            case ACD_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
                compressed = gpu3d::GPU_LATC2_SIGNED;
                break;
            case ACD_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
                compressed = gpu3d::GPU_LATC2;
                break;
            case ACD_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
                compressed = gpu3d::GPU_LATC2_SIGNED;
                break;
            default:
                compressed = gpu3d::GPU_NO_TEXTURE_COMPRESSION;
        }

        _mortonData = _driver->getDataInMortonOrder(_data, _width, _height, _depth, compressed, getTexelSize(), _mortonDataSize);

    }

    sizeInBytes = _mortonDataSize;
    GLOBALPROFILER_EXITREGION()
    return _mortonData;
}

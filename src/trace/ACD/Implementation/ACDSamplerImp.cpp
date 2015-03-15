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

#include <iostream>
#include <fstream>
#include <sstream>
#include "ACDMath.h"
#include "ACDDeviceImp.h"
#include "ACDSamplerImp.h"
#include "ACDMacros.h"
#include "ACDTexture2DImp.h"
#include "ACDTextureCubeMapImp.h"
#include "TextureAdapter.h"
#include "GlobalProfiler.h"

using namespace acdlib;
using namespace std;

ACDSamplerImp::ACDSamplerImp(ACDDeviceImp* device, GPUDriver* driver, acd_uint samplerID) :
    _device(device), _driver(driver), _SAMPLER_ID(samplerID), _requiredSync(true),
    _enabled(false),
    _texture(0),
    _textureType(ACD_RESOURCE_UNKNOWN),
    _textureWidth(0),
    _textureHeight(0),
    _textureDepth(0),
    _textureWidth2(0),
    _textureHeight2(0),
    _textureDepth2(0),
    _sCoord(ACD_TEXTURE_ADDR_CLAMP),
    _tCoord(ACD_TEXTURE_ADDR_CLAMP),
    _rCoord(ACD_TEXTURE_ADDR_CLAMP),
    _nonNormalizedCoords(false),
    _minFilter(ACD_TEXTURE_FILTER_NEAREST),
    _magFilter(ACD_TEXTURE_FILTER_NEAREST),
    _enableComparison(false),
    _comparisonFunction(ACD_TEXTURE_COMPARISON_LEQUAL),
    _sRGBConversion(false),
    _minLOD(0.0f),
    _maxLOD(12.0f),
    _baseLevel(0),
    _maxLevel(12),
    _maxAniso(1),
    _lodBias(0.0f),
    _unitLodBias(0.0f),
    _minLevel(0),
    _blitTextureBlocking(ACD_LAYOUT_TEXTURE)
{
    forceSync(); // init GPU sampler(i) state registers
}

void ACDSamplerImp::setEnabled(acd_bool enable) { _enabled = enable; }

acd_bool ACDSamplerImp::isEnabled() const { return _enabled; }

void ACDSamplerImp::setTexture(ACDTexture* texture)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACD_ASSERT(
        if ( texture->getType() == ACD_RESOURCE_TEXTURE1D )
            panic("ACDSamplerImp", "setTexture", "1D textures are not supported yet");
        /*if ( texture->getType() == ACD_RESOURCE_TEXTURE3D )
            panic("ACDSamplerImp", "setTexture", "3D textures are not supported yet");*/
    )
    _texture = texture;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setTextureAddressMode(ACD_TEXTURE_COORD coord, ACD_TEXTURE_ADDR_MODE mode)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    switch ( coord )
    {
        case ACD_TEXTURE_S_COORD:
            _sCoord = mode; break;
        case ACD_TEXTURE_T_COORD:
            _tCoord = mode; break;
        case ACD_TEXTURE_R_COORD:
            _rCoord = mode; break;
        default:
            panic("ACDSamplerImp", "setTextureAddressMode", "Unknown texture address coordinate");
    }
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setNonNormalizedCoordinates(acd_bool enable)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _nonNormalizedCoords = enable;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setMinFilter(ACD_TEXTURE_FILTER minFilter)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _minFilter = minFilter;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setMagFilter(ACD_TEXTURE_FILTER magFilter)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _magFilter = magFilter;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setMinLOD(acd_float minLOD)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _minLOD = minLOD;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setEnableComparison(acd_bool enable)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _enableComparison = enable;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setComparisonFunction(ACD_TEXTURE_COMPARISON function)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _comparisonFunction = function;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setSRGBConversion(acd_bool enable)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _sRGBConversion = enable;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setMaxLOD(acd_float maxLOD)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _maxLOD = maxLOD;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setMaxAnisotropy(acd_uint maxAnisotropy)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _maxAniso = maxAnisotropy;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setLODBias(acd_float lodBias)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _lodBias = lodBias;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setUnitLODBias(acd_float unitLodBias)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _unitLodBias = unitLodBias;
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::setMinLevel(acd_uint minLevel)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    _minLevel = minLevel;
    GLOBALPROFILER_EXITREGION()
}

ACD_TEXTURE_FILTER ACDSamplerImp::getMagFilter() const
{
    return _magFilter;
}

ACDTexture* ACDSamplerImp::getTexture() const
{
    return _texture;
}

ACD_TEXTURE_FILTER ACDSamplerImp::getMinFilter() const
{
    return _minFilter;
}

string ACDSamplerImp::getInternalState() const
{
    stringstream ss, ssAux;

    ssAux << "SAMPLER" << _SAMPLER_ID;

    ss << ssAux.str() << stateItemString(_enabled,"_ENABLED",false,&boolPrint);

    ss << ssAux.str() << stateItemString(_sCoord,"_S_COORD_ADDR_MODE",false,&addressModePrint);
    ss << ssAux.str() << stateItemString(_tCoord,"_T_COORD_ADDR_MODE",false,&addressModePrint);
    ss << ssAux.str() << stateItemString(_rCoord,"_R_COORD_ADDR_MODE",false,&addressModePrint);

    ss << ssAux.str() << stateItemString(_nonNormalizedCoords, "_NON_NORMALIZED", false, &boolPrint);

    ss << ssAux.str() << stateItemString(_minFilter,"_MIN_FILTER",false,&filterPrint);
    ss << ssAux.str() << stateItemString(_magFilter,"_MAG_FILTER",false,&filterPrint);

    ss << ssAux.str() << stateItemString(_minLOD,"_MIN_LOD",false);
    ss << ssAux.str() << stateItemString(_maxLOD,"_MAX_LOD",false);

    ss << ssAux.str() << stateItemString(_maxAniso,"_MAX_ANISO",false);
    ss << ssAux.str() << stateItemString(_lodBias,"_LOD_BIAS",false);

    return ss.str().c_str();
}

const char* ACDSamplerImp::_getFormatString(ACD_FORMAT textureFormat)
{
    switch ( textureFormat )
    {
        //Noncompressed Formats
        case ACD_FORMAT_RGBA_8888:
            return "RGBA_8888";
        case ACD_FORMAT_INTENSITY_8:
            return "INTENSITY_8";
        case ACD_FORMAT_INTENSITY_12:
            return "INTENSITY_12";
        case ACD_FORMAT_INTENSITY_16:
            return "INTENSITY_16";
        case ACD_FORMAT_ALPHA_8:
            return "ALPHA_8";
        case ACD_FORMAT_ALPHA_12:
            return "ALPHA_12";
        case ACD_FORMAT_ALPHA_16:
            return "ALPHA_16";
        case ACD_FORMAT_LUMINANCE_8:
            return "LUMINANCE_8";
        case ACD_FORMAT_LUMINANCE_12:
            return "LUMINANCE_12";
        case ACD_FORMAT_LUMINANCE_16:
            return "LUMINANCE_16";
        case ACD_FORMAT_LUMINANCE8_ALPHA8:
            return "LUMINANCE8_ALPHA8";            
        case ACD_FORMAT_DEPTH_COMPONENT_16:
            return "DEPTH_COMPONENT_16";
        case ACD_FORMAT_DEPTH_COMPONENT_24:
            return "DEPTH_COMPONENT_24";
        case ACD_FORMAT_DEPTH_COMPONENT_32:
            return "DEPTH_COMPONENT_32";
        case ACD_FORMAT_RGB_888:
            return "RGB_888";
        case ACD_FORMAT_ARGB_8888:
            return "ARGB_8888";
        case ACD_FORMAT_ABGR_161616:
            return "ARGR_161616";
        case ACD_FORMAT_XRGB_8888:
            return "XRGB_8888";
        case ACD_FORMAT_QWVU_8888:
            return "QWVU_8888";
        case ACD_FORMAT_ARGB_1555:
            return "ARGB_1555";
        case ACD_FORMAT_UNSIGNED_INT_8888:
            return "UNSIGNED_INT_8888";
        case ACD_FORMAT_RG16F:
            return "RG16F";
        case ACD_FORMAT_R32F:
            return "R32F";
        case ACD_FORMAT_RGBA32F:
            return "RGBA32F";
        case ACD_FORMAT_S8D24:
            return "S8D24";
        case ACD_FORMAT_RGBA16F:
            return "RGBA16F";
        case ACD_FORMAT_RGB_565:
            return "RGB_565";
        /// Add more here...

        // Compressed formats
        case ACD_COMPRESSED_S3TC_DXT1_RGB:
            return "COMPRESSED_S3TC_DXT1_RGB";
        case ACD_COMPRESSED_S3TC_DXT1_RGBA:
            return "COMPRESSED_S3TC_DXT1_RGBA";
        case ACD_COMPRESSED_S3TC_DXT3_RGBA:
            return "COMPRESSED_S3TC_DXT3_RGBA";
        case ACD_COMPRESSED_S3TC_DXT5_RGBA:
            return "COMPRESSED_S3TC_DXT5_RGBA";
        case ACD_COMPRESSED_LUMINANCE_LATC1_EXT:
            return "COMPRESSED_LUMINANCE_LATC1_EXT";
        case ACD_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            return "COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT";
        case ACD_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
            return "COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT";
        case ACD_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            return "COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT";

        default:
            return "UNKNOWN";
    }
}

void ACDSamplerImp::_getGPUTextureFormat(ACD_FORMAT textureFormat,
                                         gpu3d::GPURegData* dataFormat,
                                         gpu3d::GPURegData* dataCompression,
                                         gpu3d::GPURegData* dataInverted)
{
    using namespace gpu3d;
    switch ( textureFormat )
    {

        // Non-compressed formats
        case ACD_FORMAT_RGB_565:
            dataFormat->txFormat = GPU_RGB565;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = true;
            break;
        case ACD_FORMAT_RGBA_8888:
        case ACD_FORMAT_UNSIGNED_INT_8888:
            dataFormat->txFormat = GPU_RGBA8888;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_RGB_888:
            dataFormat->txFormat = GPU_RGB888;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_XRGB_8888:
            dataFormat->txFormat = GPU_RGBA8888;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = true;
            break;
        case ACD_FORMAT_ARGB_8888:
        case ACD_FORMAT_QWVU_8888:
            dataFormat->txFormat = GPU_RGBA8888;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = true;
            break;
        case ACD_FORMAT_RG16F:
            dataFormat->txFormat = GPU_RG16F;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_R32F:
            dataFormat->txFormat = GPU_R32F;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;                    
        case ACD_FORMAT_RGBA32F:
            dataFormat->txFormat = GPU_RGBA32F;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_S8D24:
            dataFormat->txFormat = GPU_DEPTH_COMPONENT24;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;                    
        case ACD_FORMAT_ABGR_161616:
            dataFormat->txFormat = GPU_RGBA16;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_RGBA16F:
            dataFormat->txFormat = GPU_RGBA16F;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;

        case ACD_FORMAT_ARGB_1555:
            dataFormat->txFormat = GPU_RGBA5551;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = true;
            break;

        case ACD_FORMAT_INTENSITY_8:
            dataFormat->txFormat = GPU_INTENSITY8;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_INTENSITY_12:
            dataFormat->txFormat = GPU_INTENSITY12;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_INTENSITY_16:
            dataFormat->txFormat = GPU_INTENSITY16;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;


        case ACD_FORMAT_ALPHA_8:
            dataFormat->txFormat = GPU_ALPHA8;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_ALPHA_12:
            dataFormat->txFormat = GPU_ALPHA12;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_ALPHA_16:
            dataFormat->txFormat = GPU_ALPHA16;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;


        case ACD_FORMAT_LUMINANCE_8:
            dataFormat->txFormat = GPU_LUMINANCE8;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_LUMINANCE_12:
            dataFormat->txFormat = GPU_LUMINANCE12;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_LUMINANCE_16:
            dataFormat->txFormat = GPU_LUMINANCE16;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;

        case ACD_FORMAT_LUMINANCE8_ALPHA8:
            dataFormat->txFormat = GPU_LUMINANCE8_ALPHA8;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;

        case ACD_FORMAT_DEPTH_COMPONENT_16:
            dataFormat->txFormat = GPU_DEPTH_COMPONENT16;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_DEPTH_COMPONENT_24:
            dataFormat->txFormat = GPU_DEPTH_COMPONENT24;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;
        case ACD_FORMAT_DEPTH_COMPONENT_32:
            dataFormat->txFormat = GPU_DEPTH_COMPONENT32;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            break;

        // Compressed Formats
        case ACD_COMPRESSED_S3TC_DXT1_RGB:
            dataFormat->txFormat = GPU_RGB888;
            dataCompression->txCompression = GPU_S3TC_DXT1_RGB;
            dataInverted->booleanVal = false;
            break;
        case ACD_COMPRESSED_S3TC_DXT1_RGBA:
            dataFormat->txFormat = GPU_RGBA8888;
            dataCompression->txCompression = GPU_S3TC_DXT1_RGBA;
            dataInverted->booleanVal = false;
            break;
        case ACD_COMPRESSED_S3TC_DXT3_RGBA:
            dataFormat->txFormat = GPU_RGBA8888;
            dataCompression->txCompression = GPU_S3TC_DXT3_RGBA;
            dataInverted->booleanVal = false;
            break;
        case ACD_COMPRESSED_S3TC_DXT5_RGBA:
            dataFormat->txFormat = GPU_RGBA8888;
            dataCompression->txCompression = GPU_S3TC_DXT5_RGBA;
            dataInverted->booleanVal = false;
            break;


        case ACD_COMPRESSED_LUMINANCE_LATC1_EXT:
            dataFormat->txFormat = GPU_LUMINANCE8;
            dataCompression->txCompression = GPU_LATC1;
            dataInverted->booleanVal = false;
            break;
        case ACD_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT:
            dataFormat->txFormat = GPU_LUMINANCE8_SIGNED;
            dataCompression->txCompression = GPU_LATC1_SIGNED;
            dataInverted->booleanVal = false;
            break;
        case ACD_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT:
            dataFormat->txFormat = GPU_LUMINANCE8_ALPHA8;
            dataCompression->txCompression = GPU_LATC2;
            dataInverted->booleanVal = false;
            break;
        case ACD_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT:
            dataFormat->txFormat = GPU_LUMINANCE8_ALPHA8_SIGNED;
            dataCompression->txCompression = GPU_LATC2_SIGNED;
            dataInverted->booleanVal = false;
            break;

        default:
        {
            dataFormat->txFormat = GPU_RGBA8888;
            dataCompression->txCompression = GPU_NO_TEXTURE_COMPRESSION;
            dataInverted->booleanVal = false;
            stringstream ss;
            ss << "Texture format \"" << _getFormatString(textureFormat) << "\" not supported";
            panic("ACDSamplerImp", "_getGPUTextureFormat", ss.str().c_str());
        }
    }
}

void ACDSamplerImp::_getGPUTextureMode(ACD_RESOURCE_TYPE textureType, gpu3d::GPURegData* data)
{
    using namespace gpu3d;
    switch ( textureType )
    {
        case ACD_RESOURCE_TEXTURE1D:
            data->txMode = GPU_TEXTURE1D;
            break;
        case ACD_RESOURCE_TEXTURE2D:
            data->txMode = GPU_TEXTURE2D;
            break;
        case ACD_RESOURCE_TEXTURE3D:
            data->txMode = GPU_TEXTURE3D;
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            data->txMode = GPU_TEXTURECUBEMAP;
            break;
        case ACD_RESOURCE_BUFFER:
            panic("ACDSamplerImp", "_getGPUTextureMode", "ACD_RESOURCE_BUFFER is not a valid texture type");
            break;
        case ACD_RESOURCE_UNKNOWN:
            panic("ACDSamplerImp", "_getGPUTextureMode", "ACD_RESOURCE_UNKNOWN is not a valid texture type");
            break;
        default:
            panic("ACDSamplerImp", "_getGPUTextureMode", "Unknown texture type token");
    }
}

void ACDSamplerImp::_getGPUTexMemoryLayout(ACD_MEMORY_LAYOUT memLayout, gpu3d::GPURegData* data)
{
    //  Set texture blocking/memory layout mode.
    switch(memLayout)
    {
        case ACD_LAYOUT_TEXTURE:
            data->txBlocking = gpu3d::GPU_TXBLOCK_TEXTURE;
            break;
        case ACD_LAYOUT_RENDERBUFFER:
            data->txBlocking = gpu3d::GPU_TXBLOCK_FRAMEBUFFER;
            break;
        default:
            panic("ACDSamplerImp", "_getGPUTexMemoryLayout", "Undefined memory layout mode.");
            break;
    }
}

void ACDSamplerImp::_syncTexture()
{
    using namespace gpu3d;

    GPURegData data;

    // Create a texture adapter object
    TextureAdapter texAdapter(_texture);

    bool renderBufferTexture = (_texture->getMemoryLayout() == ACD_LAYOUT_RENDERBUFFER);

    // Compute efective base mipmap level
    acd_uint baseLevel = _minLevel;
    _baseLevel = max(_texture->getBaseLevel(), baseLevel);
    if ( _baseLevel.changed() || _requiredSync )
    {
        data.uintVal = _baseLevel;
        _driver->writeGPURegister(GPU_TEXTURE_MIN_LEVEL, _SAMPLER_ID, data);
        _baseLevel.restart();
    }

    // Compute max mipmal level
    _maxLevel = _texture->getMaxLevel();   
    if ( _maxLevel.changed() || _requiredSync ) 
    {
        data.uintVal = _maxLevel;
        _driver->writeGPURegister(GPU_TEXTURE_MAX_LEVEL, _SAMPLER_ID, data);
        _maxLevel.restart();
    }

    // Compute texture width
    _textureWidth = texAdapter.getWidth(static_cast<ACD_CUBEMAP_FACE>(0), 0);
    if ( _textureWidth.changed() || _requiredSync ) 
    {
        data.uintVal = _textureWidth;
        _driver->writeGPURegister(GPU_TEXTURE_WIDTH, _SAMPLER_ID, data);
        _textureWidth.restart();
    }


    // Compute texture height
    _textureHeight = texAdapter.getHeight(static_cast<ACD_CUBEMAP_FACE>(0), 0);
    if ( _textureHeight.changed() || _requiredSync ) 
    {
        data.uintVal = _textureHeight;
        _driver->writeGPURegister(GPU_TEXTURE_HEIGHT, _SAMPLER_ID, data);
        _textureHeight.restart();
    }

    // Compute texture depth
    _textureDepth = texAdapter.getDepth(static_cast<ACD_CUBEMAP_FACE>(0), 0);
    if ( _textureDepth.changed() || _requiredSync ) 
    {
        data.uintVal = _textureDepth;
        _driver->writeGPURegister(GPU_TEXTURE_DEPTH, _SAMPLER_ID, data);
        _textureHeight.restart();
    }

    // Compute texture log two width
    _textureWidth2 = static_cast<acd_uint>(acdlib::logTwo(texAdapter.getWidth(static_cast<ACD_CUBEMAP_FACE>(0),_baseLevel)));
    if ( _textureWidth2.changed() || _requiredSync ) 
    {   
        data.uintVal = _textureWidth2;
        _driver->writeGPURegister(GPU_TEXTURE_WIDTH2, _SAMPLER_ID, data);
        _textureWidth2.restart();
    }

    // Compute texture log two height
    _textureHeight2  = static_cast<acd_uint>(acdlib::logTwo(texAdapter.getHeight(static_cast<ACD_CUBEMAP_FACE>(0),_baseLevel)));
    if ( data.uintVal != _textureHeight2 || _requiredSync ) 
    {
        data.uintVal = _textureHeight2;
        _driver->writeGPURegister(GPU_TEXTURE_HEIGHT2, _SAMPLER_ID, data);
        _textureHeight2.restart();
    }

    // Compute texture log two depth
    _textureDepth2  = static_cast<acd_uint>(acdlib::logTwo(texAdapter.getDepth(static_cast<ACD_CUBEMAP_FACE>(0),_baseLevel)));
    if ( data.uintVal != _textureDepth2 || _requiredSync ) 
    {
        data.uintVal = _textureDepth2;
        _driver->writeGPURegister(GPU_TEXTURE_DEPTH2, _SAMPLER_ID, data);
        _textureDepth2.restart();
    }

    _textureFormat = texAdapter.getFormat(static_cast<ACD_CUBEMAP_FACE>(0), _baseLevel);
    if (_textureFormat.changed() || _requiredSync)
    {
        GPURegData dataInverted;
        GPURegData dataCompression;

        _getGPUTextureFormat(texAdapter.getFormat(static_cast<ACD_CUBEMAP_FACE>(0), _baseLevel), &data, &dataCompression, &dataInverted);
        _driver->writeGPURegister(GPU_TEXTURE_FORMAT, _SAMPLER_ID, data);
        _driver->writeGPURegister(GPU_TEXTURE_COMPRESSION, _SAMPLER_ID, dataCompression);

        data.booleanVal = dataInverted.booleanVal && !renderBufferTexture;
        _driver->writeGPURegister(GPU_TEXTURE_D3D9_COLOR_CONV, _SAMPLER_ID, data);

        // Update texture texels reversing
        data.booleanVal = (_textureFormat == ACD_FORMAT_UNSIGNED_INT_8888 ? true : false);
        _driver->writeGPURegister(GPU_TEXTURE_REVERSE, _SAMPLER_ID, data);

        _textureFormat.restart();
    }

    _textureType = _texture->getType();
    if ( _textureType.changed() || _requiredSync ) {
        _getGPUTextureMode(_textureType, &data);
        _driver->writeGPURegister(GPU_TEXTURE_MODE, _SAMPLER_ID, data);
        _textureType.restart();
    }

    _memoryLayout = _texture->getMemoryLayout();
    if (_memoryLayout.changed() || _requiredSync)
    {
       _getGPUTexMemoryLayout(_memoryLayout, &data);
        _driver->writeGPURegister(GPU_TEXTURE_BLOCKING, _SAMPLER_ID, data);
        _memoryLayout.restart();
    }

    // Compute texture faces
    const acd_uint totalFaces = ( _texture->getType() == ACD_RESOURCE_TEXTURECUBEMAP ? 6 : 1 );

    // Compute the texture unit start register
    const acd_uint samplerUnitOffset = _SAMPLER_ID * (ACD_MAX_TEXTURE_LEVEL+1) * 6;



    ACDTexture2DImp* texture2D = 0;
	ACDTexture3DImp* texture3D = 0;
    ACDTextureCubeMapImp* textureCM = 0;

    if(_texture->getType() == ACD_RESOURCE_TEXTURE2D)
        texture2D = static_cast<ACDTexture2DImp*>(_texture);
    else if (_texture->getType() == ACD_RESOURCE_TEXTURE3D)
        texture3D = static_cast<ACDTexture3DImp*>(_texture);
	else
        textureCM = static_cast<ACDTextureCubeMapImp*>(_texture);

    MemoryObjectAllocator& moa = _device->allocator();
    for ( acd_uint face = 0; face < totalFaces; ++face ) 
    {
        for ( acd_uint level = _baseLevel; level <= _maxLevel; ++level ) 
        {
            acd_uint mipRegion = texAdapter.region(static_cast<ACD_CUBEMAP_FACE>(face),level); // compute the associated region to this mipmap

            acd_uint md;
            if (texture2D)
            {
                moa.syncGPU(texture2D, mipRegion);
                md = moa.md(texture2D, mipRegion);

#ifdef ACD_DUMP_SAMPLERS
                acd_ubyte filename[30];
                sprintf((char*)filename, "Frame_%d_Batch_%d_Sampler%d_Mipmap%d_Format_%d.ppm", _device->getCurrentFrame(), _device->getCurrentBatch(), _SAMPLER_ID, level, _getFormatString(_textureFormat));
                texture2D->dumpMipmap(mipRegion, filename);
#endif
            }
            else if (textureCM)
            {
                moa.syncGPU(textureCM, mipRegion);
                md = moa.md(textureCM, mipRegion);

#ifdef ACD_DUMP_SAMPLERS
                    acd_ubyte filename[30];
                    sprintf((char*)filename, "Frame_%d_Batch_%d_Sampler%d_Face%d_Mipmap%d_Format_%d.ppm", _device->getCurrentFrame(), _device->getCurrentBatch(), _SAMPLER_ID, face, level, _getFormatString(_textureFormat));
                    textureCM->dumpMipmap((ACD_CUBEMAP_FACE)face, level, filename);
#endif
            }
			else
			{
                moa.syncGPU(texture3D, mipRegion);
                md = moa.md(texture3D, mipRegion);
			}

            acd_uint registerIndex = samplerUnitOffset + 6*level + face; // Compute the corresponding register index
            _driver->writeGPUAddrRegister(GPU_TEXTURE_ADDRESS, registerIndex, md);
        }
    }
}

void ACDSamplerImp::performBlitOperation2D(acd_uint xoffset, acd_uint yoffset, acd_uint x, acd_uint y, acd_uint width, acd_uint height, acd_uint textureWidth, ACD_FORMAT internalFormat, ACDTexture2D* texture, acd_uint level)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")

    using namespace gpu3d;
    GPURegData data;

    if (xoffset < 0 || yoffset < 0)
        panic("ACDSamplerImp","performBlitOperation2D","Texture subregions are not allowed to start at negative x,y coordinates");

    if (x < 0 || y < 0 )
        panic("ACDSamplerImp","performBlitOperation2D","Framebuffer regions to blit are not allowed to start at negative x,y coordinates");

    if (width < 0 || height < 0 )
        panic("ACDSamplerImp","performBlitOperation2D","Blitting of negative regions of the framebuffer not allowed");


    _blitFormat = internalFormat;
    if (_blitFormat.changed() || _requiredSync)
    {
	    if (internalFormat == ACD_FORMAT_ARGB_8888 || internalFormat == ACD_FORMAT_RGBA_8888 || 
            internalFormat == ACD_FORMAT_RGB_888 || internalFormat == ACD_FORMAT_DEPTH_COMPONENT_24)
        {
            data.txFormat = GPU_RGBA8888;
            _driver->writeGPURegister(GPU_BLIT_DST_TX_FORMAT, 0, data);
        }
        else
        {
            char error[128];
            sprintf(error, "Unsupported Blit operation texture format. Format used: %d", internalFormat);
            panic("ACDSamplerImp","performBlitOperation2D", error);
        }

        _blitFormat.restart();
    }

    // Compute the ceiling of the texture width power of two.
    _blitWidth2 = static_cast<acd_uint>(acdlib::ceil(acdlib::logTwo(textureWidth)));
    if (_blitWidth2.changed() || _requiredSync)
    {
        data.uintVal = _blitWidth2;
        _driver->writeGPURegister(GPU_BLIT_DST_TX_WIDTH2, 0, data);
        _blitWidth2.restart();
    }
    
    _blitIniX = x;
    if (_blitIniX.changed() || _requiredSync)
    {
        data.uintVal = _blitIniX;
        _driver->writeGPURegister(GPU_BLIT_INI_X, 0, data);
        _blitIniX.restart();
    }

    _blitIniY = y;
    if (_blitIniY.changed() || _requiredSync)
    {
        data.uintVal = _blitIniY;
        _driver->writeGPURegister(GPU_BLIT_INI_Y, 0, data);
        _blitIniY.restart();
    }

    _blitXoffset = xoffset;
    if (_blitXoffset.changed() || _requiredSync)
    {
        data.uintVal = _blitXoffset;
        _driver->writeGPURegister(GPU_BLIT_X_OFFSET, 0, data);
        _blitXoffset.restart();
    }

    _blitYoffset = yoffset;
    if (_blitYoffset.changed() || _requiredSync)
    {
        data.uintVal = _blitYoffset;
        _driver->writeGPURegister(GPU_BLIT_Y_OFFSET, 0, data);
        _blitYoffset.restart();
    }

    _blitWidth = width;
    if (_blitWidth.changed() || _requiredSync)
    {
        data.uintVal = _blitWidth;
        _driver->writeGPURegister(GPU_BLIT_WIDTH, 0, data);
        _blitWidth.restart();
    }

    _blitHeight = height;
    if (_blitHeight.changed() || _requiredSync)
    {
        data.uintVal = _blitHeight;
        _driver->writeGPURegister(GPU_BLIT_HEIGHT, 0, data);
        _blitHeight.restart();
    }

    if (!_requiredSync)
    {
        ACDTexture2DImp* tex2D = (ACDTexture2DImp*) texture;
        MemoryObjectAllocator& moa = _device->allocator();
        moa.syncGPU(tex2D, level);

        _blitTextureBlocking = tex2D->getMemoryLayout();
        if (_blitTextureBlocking.changed())
        {
            _getGPUTexMemoryLayout(_blitTextureBlocking, &data);
            _driver->writeGPURegister(GPU_BLIT_DST_TX_BLOCK, 0, data);
            _blitTextureBlocking.restart();
        }

        acd_uint md = moa.md(tex2D, level);
        _driver->writeGPUAddrRegister(GPU_BLIT_DST_ADDRESS, 0, md);

        _driver->sendCommand(GPU_BLIT);

        tex2D->postBlit(level);
    }
    
    GLOBALPROFILER_EXITREGION()
}

void ACDSamplerImp::sync()
{
    ACD_ASSERT(
        // Check texture completeness before synchronize texture in GPU/system memory
        if ( _enabled && _texture && !_texture->wellDefined() )
            panic("ACDSamplerImp", "_syncTextureState", "Texture is not well defined (texture completness required)");
    )

    gpu3d::GPURegData data;

    if ( _enabled.changed() ) {
        data.booleanVal = _enabled;
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_ENABLE, _SAMPLER_ID, data);
        _enabled.restart();
    }

    /// If the sampler is disabled we are done (no sync required)
    if ( !_enabled && !_requiredSync)
        return;

    /// { S: The texture sampler is enabled }

    /// Synchronize texture unit state with the current bound texture
    if ( _texture )
        _syncTexture();

    /// Update SAMPLER STATE independent of the texture
    if ( _sCoord.changed() || _requiredSync ) {
        _getGPUClampMode(_sCoord, &data);
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_WRAP_S, _SAMPLER_ID, data);
        _sCoord.restart();
    }
    if ( _tCoord.changed() || _requiredSync ) {
        _getGPUClampMode(_tCoord, &data);
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_WRAP_T, _SAMPLER_ID, data);
        _tCoord.restart();
    }
    if ( _rCoord.changed() || _requiredSync ) {
        _getGPUClampMode(_rCoord, &data);
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_WRAP_R, _SAMPLER_ID, data);
        _rCoord.restart();
    }

    if ( _nonNormalizedCoords.changed() || _requiredSync )
    {
        data.booleanVal = _nonNormalizedCoords;
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_NON_NORMALIZED, _SAMPLER_ID, data);
        _nonNormalizedCoords.restart();
    }

    if ( _minFilter.changed() || _requiredSync ) {
        _getGPUTexFilter(_minFilter, &data);
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_MIN_FILTER, _SAMPLER_ID, data);
        _minFilter.restart();
    }
    if ( _magFilter.changed() || _requiredSync ) {
        _getGPUTexFilter(_magFilter, &data);
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_MAG_FILTER, _SAMPLER_ID, data);
        _magFilter.restart();
    }

    if (_enableComparison.changed() || _requiredSync)
    {
        data.booleanVal = _enableComparison;
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_ENABLE_COMPARISON, _SAMPLER_ID, data);
        _enableComparison.restart();
    }
    
    if (_comparisonFunction.changed() || _requiredSync)
    {
        _getGPUTexComp(_comparisonFunction, &data);
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_COMPARISON_FUNCTION, _SAMPLER_ID, data);
        _comparisonFunction.restart();        
    }
    
    if (_sRGBConversion.changed() || _requiredSync)
    {
        data.booleanVal = _sRGBConversion;
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_SRGB, _SAMPLER_ID, data);
        _sRGBConversion.restart();
    }

    if ( _minLOD.changed() || _requiredSync ) {
        data.f32Val = _minLOD;
        data.f32Val = 0.0f;
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_MIN_LOD, _SAMPLER_ID, data);
        _minLOD.restart();
    }
    if ( _maxLOD.changed() || _requiredSync ) {
        data.f32Val = _maxLOD;
        data.f32Val = 12.0f;
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_MAX_LOD, _SAMPLER_ID, data);
        _maxLOD.restart();
    }
    if ( _maxAniso.changed() || _requiredSync ) {
        data.uintVal = _maxAniso;
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_MAX_ANISOTROPY, _SAMPLER_ID, data);
        _maxAniso.restart();
    }
    if ( _lodBias.changed() || _requiredSync ) {
        data.f32Val = _lodBias;
        _driver->writeGPURegister(gpu3d::GPU_TEXTURE_LOD_BIAS, _SAMPLER_ID, data);
        _lodBias.restart();
    }
    if ( _unitLodBias.changed() || _requiredSync ) {
        data.f32Val = _unitLodBias;
        _driver->writeGPURegister(gpu3d::GPU_TEXT_UNIT_LOD_BIAS, _SAMPLER_ID, data);
        _unitLodBias.restart();
    }
}

void ACDSamplerImp::forceSync()
{
    _requiredSync = true;
    sync();
    performBlitOperation2D(0, 0, 0, 0, 0, 0, 0, ACD_FORMAT_RGBA_8888, 0, 0);
    _requiredSync = false;
}

void ACDSamplerImp::_getGPUClampMode(ACD_TEXTURE_ADDR_MODE mode, gpu3d::GPURegData* data)
{
    switch ( mode ) {
        case ACD_TEXTURE_ADDR_CLAMP:
            data->txClamp = gpu3d::GPU_TEXT_CLAMP;
            break;
        case ACD_TEXTURE_ADDR_CLAMP_TO_EDGE:
            data->txClamp = gpu3d::GPU_TEXT_CLAMP_TO_EDGE;
            break;
        case ACD_TEXTURE_ADDR_REPEAT:
            data->txClamp = gpu3d::GPU_TEXT_REPEAT;
            break;
        case ACD_TEXTURE_ADDR_CLAMP_TO_BORDER:
            data->txClamp = gpu3d::GPU_TEXT_CLAMP_TO_BORDER;
            break;
        case ACD_TEXTURE_ADDR_MIRRORED_REPEAT:
            data->txClamp = gpu3d::GPU_TEXT_MIRRORED_REPEAT;
            break;
        default:
            panic("ACDSamplerImp", "_getGPUClampMode", "ACD_TEXTURE_ADDR_MODE unknown");
    }
}

void ACDSamplerImp::_getGPUTexFilter(ACD_TEXTURE_FILTER filter, gpu3d::GPURegData* data)
{
    switch ( filter ) {
        case ACD_TEXTURE_FILTER_NEAREST:
            data->txFilter = gpu3d::GPU_NEAREST;
            break;
        case ACD_TEXTURE_FILTER_LINEAR:
            data->txFilter = gpu3d::GPU_LINEAR;
            break;
        case ACD_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            data->txFilter = gpu3d::GPU_NEAREST_MIPMAP_NEAREST;
            break;
        case ACD_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            data->txFilter = gpu3d::GPU_NEAREST_MIPMAP_LINEAR;
            break;
        case ACD_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            data->txFilter = gpu3d::GPU_LINEAR_MIPMAP_NEAREST;
            break;
        case ACD_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            data->txFilter = gpu3d::GPU_LINEAR_MIPMAP_LINEAR;
            break;
        default:
            panic("ACDSamplerImp", "_getGPUTexFilter", "Unknown ACD_TEXTURE_FILTER");
    }
}

void ACDSamplerImp::_getGPUTexComp(ACD_TEXTURE_COMPARISON function, gpu3d::GPURegData* data)
{
    switch(function)    
    {
        case ACD_TEXTURE_COMPARISON_NEVER:
            data->compare = gpu3d::GPU_NEVER;
            break;
        case ACD_TEXTURE_COMPARISON_ALWAYS:
            data->compare = gpu3d::GPU_ALWAYS;
            break;
        case ACD_TEXTURE_COMPARISON_LESS:
            data->compare = gpu3d::GPU_LESS;
            break;
        case ACD_TEXTURE_COMPARISON_LEQUAL:
            data->compare = gpu3d::GPU_LEQUAL;
            break;
        case ACD_TEXTURE_COMPARISON_EQUAL:
            data->compare = gpu3d::GPU_EQUAL;
            break;
        case ACD_TEXTURE_COMPARISON_GEQUAL:
            data->compare = gpu3d::GPU_GEQUAL;
            break;
        case ACD_TEXTURE_COMPARISON_GREATER:
            data->compare = gpu3d::GPU_GREATER;
            break;
        case ACD_TEXTURE_COMPARISON_NOTEQUAL:
            data->compare = gpu3d::GPU_NOTEQUAL;
            break;
        default:
            panic("ACDSamplerImp", "_getGPUTexComp", "Unknown ACD_TEXTURE_COMPARISON");
            break;            
    }
}

const char* ACDSamplerImp::AddressModePrint::print(const ACD_TEXTURE_ADDR_MODE &var) const
{
    switch(var)
    {
        case ACD_TEXTURE_ADDR_CLAMP:
            return "CLAMP"; break;
        case ACD_TEXTURE_ADDR_CLAMP_TO_EDGE:
            return "CLAMP_TO_EDGE"; break;
        case ACD_TEXTURE_ADDR_REPEAT:
            return "REPEAT"; break;
        case ACD_TEXTURE_ADDR_CLAMP_TO_BORDER:
            return "CLAMP_TO_BORDER"; break;
        case ACD_TEXTURE_ADDR_MIRRORED_REPEAT:
            return "MIRRORED_REPEAT"; break;
        default:
            return "UNDEFINED";
    }
}

const char* ACDSamplerImp::FilterPrint::print(const acdlib::ACD_TEXTURE_FILTER &var) const
{
    switch(var)
    {
        case ACD_TEXTURE_FILTER_NEAREST:
            return "NEAREST"; break;
        case ACD_TEXTURE_FILTER_LINEAR:
            return "LINEAR"; break;
        case ACD_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST:
            return "NEAREST_MIPMAP_NEAREST"; break;
        case ACD_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR:
            return "NEAREST_MIPMAP_LINEAR"; break;
        case ACD_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST:
            return "LINEAR_MIPMAP_NEAREST"; break;
        case ACD_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR:
            return "LINEAR_MIPMAP_LINEAR"; break;
        default:
            return "UNDEFINED";
    };
}

const char* ACDSamplerImp::CompFuncPrint::print(const acdlib::ACD_TEXTURE_COMPARISON &var) const
{
    switch(var)
    {
        case ACD_TEXTURE_COMPARISON_NEVER:
            return "NEVER";
            break;
        case ACD_TEXTURE_COMPARISON_ALWAYS:
            return "ALWAYS";
            break;
        case ACD_TEXTURE_COMPARISON_LESS:
            return "LESS";
            break;
        case ACD_TEXTURE_COMPARISON_LEQUAL:
            return "LEQUAL";
            break;
        case ACD_TEXTURE_COMPARISON_EQUAL:
            return "EQUAL";
            break;
        case ACD_TEXTURE_COMPARISON_GEQUAL:
            return "GEQUAL";
            break;
        case ACD_TEXTURE_COMPARISON_GREATER:
            return "GREATER";
            break;
        case ACD_TEXTURE_COMPARISON_NOTEQUAL:
            return "NOTEQUAL";
            break;
        default:
            return "UNDEFINED";
            break;
    }
}


const StoredStateItem* ACDSamplerImp::createStoredStateItem(ACD_STORED_ITEM_ID stateId) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACDStoredStateItem* ret;
    acd_uint rTarget;

    if (stateId >= ACD_SAMPLER_FIRST_ID && stateId < ACD_SAMPLER_LAST)
    {    
        // It´s a blending state
        switch(stateId)
        {
            case ACD_SAMPLER_ENABLED:           ret = new ACDSingleBoolStoredStateItem(_enabled);           break;
            case ACD_SAMPLER_CLAMP_S:           ret = new ACDSingleEnumStoredStateItem(_sCoord);            break;
            case ACD_SAMPLER_CLAMP_T:           ret = new ACDSingleEnumStoredStateItem(_tCoord);            break;
            case ACD_SAMPLER_CLAMP_R:           ret = new ACDSingleEnumStoredStateItem(_rCoord);            break;
            case ACD_SAMPLER_NON_NORMALIZED:    ret = new ACDSingleBoolStoredStateItem(_nonNormalizedCoords); break;
            case ACD_SAMPLER_MIN_FILTER:        ret = new ACDSingleEnumStoredStateItem(_minFilter);         break;
            case ACD_SAMPLER_MAG_FILTER:        ret = new ACDSingleEnumStoredStateItem(_magFilter);         break;
            case ACD_SAMPLER_ENABLE_COMP:       ret = new ACDSingleBoolStoredStateItem(_enableComparison);  break;
            case ACD_SAMPLER_COMP_FUNCTION:     ret = new ACDSingleEnumStoredStateItem(_comparisonFunction);break;
            case ACD_SAMPLER_SRGB_CONVERSION:   ret = new ACDSingleBoolStoredStateItem(_sRGBConversion);    break;
            case ACD_SAMPLER_MIN_LOD:           ret = new ACDSingleFloatStoredStateItem(_minLOD);           break;
            case ACD_SAMPLER_MAX_LOD:           ret = new ACDSingleFloatStoredStateItem(_maxLOD);           break;
            case ACD_SAMPLER_MAX_ANISO:         ret = new ACDSingleUintStoredStateItem(_maxAniso);          break;
            case ACD_SAMPLER_LOD_BIAS:          ret = new ACDSingleFloatStoredStateItem(_lodBias);          break;
            case ACD_SAMPLER_UNIT_LOD_BIAS:     ret = new ACDSingleFloatStoredStateItem(_unitLodBias);      break;
            case ACD_SAMPLER_TEXTURE:           ret = new ACDSingleVoidStoredStateItem(_texture);           break;
            
            // case ACD_STREAM_... <- add here future additional blending states.

            default:
                panic("ACDStreamImp","createStoredStateItem","Unexpected blending state id");
        }          
        // else if (... <- write here for future additional defined blending states.
    }
    else
        panic("ACDStreamImp","createStoredStateItem","Unexpected blending state id");

    ret->setItemId(stateId);

    GLOBALPROFILER_EXITREGION()

    return ret;
}

#define CAST_TO_UINT(X)         *(static_cast<const ACDSingleUintStoredStateItem*>(X))
#define CAST_TO_BOOL(X)         *(static_cast<const ACDSingleBoolStoredStateItem*>(X))
#define CAST_TO_FLOAT(X)        *(static_cast<const ACDSingleFloatStoredStateItem*>(X))
#define CAST_TO_ENUM(X)         *(static_cast<const ACDSingleEnumStoredStateItem*>(X))
#define CAST_TO_VOID(X)         static_cast<void*>(*(static_cast<const ACDSingleVoidStoredStateItem*> (X)))

void ACDSamplerImp::restoreStoredStateItem(const StoredStateItem* ssi)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    acd_uint rTarget;

    const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(ssi);

    ACD_STORED_ITEM_ID stateId = acdssi->getItemId();

    if (stateId >= ACD_SAMPLER_FIRST_ID && stateId < ACD_SAMPLER_LAST)
    {    

        // It´s a blending state
        switch(stateId)
        {
            case ACD_SAMPLER_ENABLED:           _enabled = CAST_TO_BOOL(acdssi);                                        break;
            //case ACD_SAMPLER_CLAMP_S:           _sCoord = static_cast<ACD_TEXTURE_ADDR_MODE>(CAST_TO_ENUM(acdssi));     break;
            //case ACD_SAMPLER_CLAMP_T:           _tCoord = static_cast<ACD_TEXTURE_ADDR_MODE>(CAST_TO_ENUM(acdssi));     break;
            //case ACD_SAMPLER_CLAMP_R:           _rCoord = static_cast<ACD_TEXTURE_ADDR_MODE>(CAST_TO_ENUM(acdssi));     break;
            case ACD_SAMPLER_NON_NORMALIZED:    _nonNormalizedCoords = CAST_TO_BOOL(acdssi);                            break;
            //case ACD_SAMPLER_MIN_FILTER:        _minFilter = static_cast<ACD_TEXTURE_FILTER>(CAST_TO_ENUM(acdssi));     break;
            //case ACD_SAMPLER_MAG_FILTER:        _magFilter = static_cast<ACD_TEXTURE_FILTER>(CAST_TO_ENUM(acdssi));     break;
            case ACD_SAMPLER_ENABLE_COMP:       _enableComparison = CAST_TO_BOOL(acdssi);                               break;
            //case ACD_SAMPLER_COMP_FUNCTION:     _comparisonFunction = static_cast<ACD_TEXTURE_COMPARISON>(CAST_TO_ENUM(acdssi));  break;
            case ACD_SAMPLER_SRGB_CONVERSION:   _sRGBConversion = CAST_TO_BOOL(acdssi);                                 break;
            case ACD_SAMPLER_MIN_LOD:           _minLOD = CAST_TO_FLOAT(acdssi);                                        break;
            case ACD_SAMPLER_MAX_LOD:           _maxLOD = CAST_TO_FLOAT(acdssi);                                        break;
            case ACD_SAMPLER_MAX_ANISO:         _maxAniso = CAST_TO_UINT(acdssi);                                       break;
            case ACD_SAMPLER_LOD_BIAS:          _lodBias = CAST_TO_FLOAT(acdssi);                                       break;
            case ACD_SAMPLER_UNIT_LOD_BIAS:     _unitLodBias = CAST_TO_FLOAT(acdssi);                                   break;
            case ACD_SAMPLER_TEXTURE:           _texture = static_cast<ACDTexture*>(CAST_TO_VOID(acdssi));              break;

            // case ACD_STREAM_... <- add here future additional blending states.
            default:
                panic("ACDStreamImp","createStoredStateItem","Unexpected blending state id");
        }           
        // else if (... <- write here for future additional defined blending states.
    }
    else
        panic("ACDStreamImp","restoreStoredStateItem","Unexpected blending state id");

    GLOBALPROFILER_EXITREGION()
}

#undef CAST_TO_UINT
#undef CAST_TO_BOOL             
#undef CAST_TO_FLOAT
#undef CAST_TO_ENUM
#undef CAST_TO_VOID


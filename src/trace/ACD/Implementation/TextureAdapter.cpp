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

#include "TextureAdapter.h"
#include "support.h"

using namespace std;
using namespace acdlib;

TextureAdapter::TextureAdapter(ACDTexture* tex)
{
    if (tex == 0) return;

    _type = tex->getType();

    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            _tex2D = static_cast<ACDTexture2DImp*>(tex); 
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            _texCM = static_cast<ACDTextureCubeMapImp*>(tex);
            break;

        case ACD_RESOURCE_TEXTURE3D:
            _tex3D = static_cast<ACDTexture3DImp*>(tex);
            break;

        //default:
            //panic("TextureAdapter", "ctor", "Adapter only supports 2D and CM textures for now");
    }
}

ACDTexture* TextureAdapter::getTexture()
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return static_cast<ACDTexture*>(_tex2D); 
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return static_cast<ACDTexture*>(_texCM);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return static_cast<ACDTexture*>(_tex3D);
            break;
        default:
            panic("TextureAdapter", "getTexture", "Adapter only supports 2D and CM textures for now");
            return 0;
            break;
    }
}

const acd_ubyte* TextureAdapter::getData(ACD_CUBEMAP_FACE face, acd_uint mipmap, acd_uint& memorySizeInBytes, acd_uint& rowPitch) const
{
    acd_uint rowPlane;

    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->getData(mipmap, memorySizeInBytes, rowPitch);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return _texCM->getData(face, mipmap, memorySizeInBytes, rowPitch);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->getData(mipmap, memorySizeInBytes, rowPitch, rowPlane);
            break;
        default:
            panic("TextureAdapter", "getData", "Adapter only supports 2D and CM textures for now");
            return 0;
            break;
    }
}

const acd_ubyte* TextureAdapter::getMemoryData(ACD_CUBEMAP_FACE face, acd_uint mipmap, acd_uint& memorySizeInBytes) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->memoryData(mipmap, memorySizeInBytes);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            /*return _texCM->memoryData(face, mipmap, memorySizeInBytes);*/
            panic("TextureAdapter", "getMemoryData", "CubeMap texture not supported");
            return 0;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->memoryData(mipmap, memorySizeInBytes);
            break;
        default:
            panic("TextureAdapter", "getMemoryData", "Adapter only supports 2D and CM textures for now");
            return 0;
    }
}

acd_uint TextureAdapter::getWidth(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->getWidth(mipmap);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return _texCM->getWidth(face, mipmap);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->getWidth(mipmap);
            break;
        default:
            panic("TextureAdapter", "getWidth", "Adapter only supports 2D and CM textures for now");
            return 0;
    }
}

acd_uint TextureAdapter::getHeight(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->getHeight(mipmap);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return _texCM->getHeight(face, mipmap);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->getHeight(mipmap);
            break;
        default:
            panic("TextureAdapter", "getHeight", "Adapter only supports 2D and CM textures for now");
            return 0;
    }
}

acd_uint TextureAdapter::getDepth(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return 0;
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return 0;
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->getDepth(mipmap);
            break;
        default:
            panic("TextureAdapter", "getDepth", "Adapter only supports 2D and CM textures for now");
            return 0;
    }
}

acd_uint TextureAdapter::getTexelSize(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->getTexelSize(mipmap);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return _texCM->getTexelSize(face, mipmap);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->getTexelSize(mipmap);
            break;
        default:
            panic("TextureAdapter", "getTexelSize", "Adapter only supports 2D and CM textures for now");
            return 0;
    }
}

ACD_FORMAT TextureAdapter::getFormat(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->getFormat(mipmap);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return _texCM->getFormat(face, mipmap);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->getFormat(mipmap);
            break;
        default:
            panic("TextureAdapter", "getFormat", "Adapter only supports 2D and CM textures for now");
            return ACD_FORMAT_UNKNOWN;
    }
}


acd_bool TextureAdapter::getMultisampled(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->isMultisampled(mipmap);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            /*return _texCM->isMultisampled(face, mipmap);*/
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->isMultisampled(mipmap);
            break;
        default:
            panic("TextureAdapter", "getMultisampled", "Adapter only supports 2D and CM textures for now");
            return false;
    }
}

acd_uint TextureAdapter::getSamples(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->getSamples(mipmap);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            /*return _texCM->getSamples(face, mipmap);*/
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->getSamples(mipmap);
            break;
        default:
            panic("TextureAdapter", "getSamples", "Adapter only supports 2D and CM textures for now");
            return 0;
    }
}

MemoryObjectState TextureAdapter::getState(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->getState(mipmap);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            /*return _texCM->getState(face, mipmap);*/
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->getState(mipmap);
            break;
        default:
            panic("TextureAdapter", "getState", "Adapter only supports 2D and CM textures for now");
            return MOS_NotFound;
    }
}

acd_bool TextureAdapter::getMultisampling(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return _tex2D->isMultisampled(mipmap);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return _texCM->isMultisampled(face, mipmap);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return _tex3D->isMultisampled(mipmap);
            break;
        default:
            panic("TextureAdapter", "getState", "Adapter only supports 2D and CM textures for now");
            return false;
    }
}

acd_uint TextureAdapter::region(ACD_CUBEMAP_FACE face, acd_uint mipmap) const
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            return mipmap;
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            return ACDTextureCubeMapImp::translate2region(face, mipmap);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            return mipmap;
            break;
        default:
            panic("TextureAdapter", "region", "Adapter only supports 2D and CM textures for now");
            return 0;
    }
}

void TextureAdapter::updateMipmap(  ACD_CUBEMAP_FACE face, 
                                    acd_uint mipLevel, 
                                    acd_uint x, 
                                    acd_uint y, 
                                    acd_uint z, 
                                    acd_uint width, 
                                    acd_uint height,
                                    acd_uint depth,
                                    ACD_FORMAT format, 
                                    acd_uint rowPitch, 
                                    const acd_ubyte* srcTexelData)
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            _tex2D->updateData(mipLevel, x, y, width, height, format, rowPitch, srcTexelData);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            _texCM->updateData(face, mipLevel, x, y, width, height, format, rowPitch, srcTexelData);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            _tex3D->updateData(mipLevel, x, y, z, width, height, depth, format, rowPitch, srcTexelData);
            break;
        default:
            panic("TextureAdapter", "region", "Adapter only supports 2D and CM textures for now");
            break;
    }
}

void TextureAdapter::setMemoryLayout(ACD_MEMORY_LAYOUT layout)
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            _tex2D->setMemoryLayout(layout);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            _texCM->setMemoryLayout(layout);
            break;
        case ACD_RESOURCE_TEXTURE3D:
            _tex3D->setMemoryLayout(layout);
            break;
        default:
            panic("TextureAdapter", "setMemoryLayout", "Adapter only supports 2D and CM textures for now");
            break;
    }
}


void TextureAdapter::postRenderBuffer(ACD_CUBEMAP_FACE face, acd_uint mipLevel)
{
    switch (_type) 
    {
        case ACD_RESOURCE_TEXTURE2D:
            _tex2D->postRenderBuffer(mipLevel);
            break;
        case ACD_RESOURCE_TEXTURECUBEMAP:
            _texCM->postRenderBuffer(ACDTextureCubeMapImp::translate2region(face, mipLevel));
            break;
        case ACD_RESOURCE_TEXTURE3D:
            _tex3D->postRenderBuffer(mipLevel);
            break;
        default:
            panic("TextureAdapter", "setMemoryLayout", "Adapter only supports 2D and CM textures for now");
            break;
    }
}


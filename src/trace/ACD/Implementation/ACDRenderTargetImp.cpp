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

#include "ACDRenderTargetImp.h"
#include "ACDDeviceImp.h"
#include "ACDTexture2DImp.h"
#include "ACDSampler.h"

using namespace acdlib;

ACDRenderTargetImp::ACDRenderTargetImp(ACDDeviceImp *device, ACDTexture* resource, ACD_RT_DIMENSION dimension, ACD_CUBEMAP_FACE face, acd_uint mipLevel) :
    _device(device), _surface(resource), _dimension(dimension), _face(face), _mipLevel(mipLevel)
{

    //TextureAdapter _surface(resource);

    switch(dimension)
    {
        case ACD_RT_DIMENSION_TEXTURE2D:
        case ACD_RT_DIMENSION_TEXTURECM:
            {    
                _surface.setMemoryLayout(ACD_LAYOUT_RENDERBUFFER);
                
                MemoryObjectAllocator& moa = _device->allocator();
                
                //  Check if the texture was already defined as a render buffer and allocated.
                if (_surface.getState(face, _mipLevel) != MOS_RenderBuffer)
                {
                    //  Get the GPU driver from the ACD device.
                    GPUDriver& driver = device->driver();
                    
                    gpu3d::TextureFormat format;
                    
                    switch(_surface.getFormat(face, _mipLevel))
                    {
                        case ACD_FORMAT_XRGB_8888:
                        case ACD_FORMAT_ARGB_8888:
                            format = gpu3d::GPU_RGBA8888;
                            break;
                            
                        case ACD_FORMAT_RG16F:
                            format = gpu3d::GPU_RG16F;
                            break;
                            
                        case ACD_FORMAT_R32F:
                            format = gpu3d::GPU_R32F;
                            break;
                            
                        case ACD_FORMAT_RGBA16F:
                            format = gpu3d::GPU_RGBA16F;
                            break;

                        case ACD_FORMAT_ABGR_161616:
                            format = gpu3d::GPU_RGBA16;
                            break;
                            
                        case ACD_FORMAT_S8D24:
                            format = gpu3d::GPU_DEPTH_COMPONENT24;
                            break;
                            
                        default:
                            char error[128];
                            sprintf(error, "Format not supported for render buffers: %d", _surface.getFormat(face, _mipLevel));
                            panic("ACDRenderTargetImp", "ACDRenderTargetImp", error);
                            break;
                    }


                    acd_uint textureMD, renderTargetMD;

                    if (moa.hasMD(static_cast<ACDTexture2DImp*>(_surface.getTexture()), _mipLevel) || moa.hasMD(static_cast<ACDTexture2DImp*>(_surface.getTexture()), _surface.region(_face, _mipLevel))) // Discover if the texture is sync with the GPU
                    {
                        //If the texture is not sync with the GPU we don't have to copy the information that saves the texture
                        // to the new renderTarget memory region.

                        renderTargetMD = driver.createRenderBuffer( _surface.getWidth(face, _mipLevel), 
                                                                    _surface.getHeight(face, _mipLevel), 
                                                                    _surface.getMultisampling(face, _mipLevel), 
                                                                    _surface.getSamples(face, _mipLevel),
                                                                    format);

                    }
                    else 
                    {
                        // Obtain the MD used by the texture mipmap we want to convert
                        if (dimension == ACD_RT_DIMENSION_TEXTURECM)
                            textureMD = moa.md(static_cast<ACDTextureCubeMapImp*>(_surface.getTexture()), _surface.region(_face, _mipLevel));
                        else
                            textureMD = moa.md(static_cast<ACDTexture2DImp*>(_surface.getTexture()), _mipLevel);
                        
                        // Obtain the MD that will hold the renderTarget
                        renderTargetMD = driver.createRenderBuffer( _surface.getWidth(face, _mipLevel), 
                                                                    _surface.getHeight(face, _mipLevel), 
                                                                    _surface.getMultisampling(face, _mipLevel), 
                                                                    _surface.getSamples(face, _mipLevel),
                                                                    format);

                        ACDRenderTargetImp* auxRT = new ACDRenderTargetImp(_device,
                                                                           _surface.getWidth(face, _mipLevel), 
                                                                           _surface.getHeight(face, _mipLevel), 
                                                                           _surface.getMultisampling(face, _mipLevel), 
                                                                           _surface.getSamples(face, _mipLevel),
                                                                           _surface.getFormat(face, _mipLevel),
                                                                            false,
                                                                            renderTargetMD);

                        // Transfer the actual texture content to the renderTarget
                        //_device->copyTexture2RenderTarget(static_cast<ACDTexture2DImp*>(_surface.getTexture()), auxRT);

                        delete auxRT;

                        // Deallocate previous texture memory
                        moa.deallocate(static_cast<ACDTexture2DImp*>(_surface.getTexture()));
                    }

                    _surface.postRenderBuffer(face, _mipLevel);

                    // Update the memory descriptor used by the renderTarget
                    if (dimension == ACD_RT_DIMENSION_TEXTURECM)
                        moa.assign(static_cast<ACDTextureCubeMapImp*>(_surface.getTexture()), _surface.region(_face, _mipLevel), renderTargetMD);
                    else
                        moa.assign(static_cast<ACDTexture2DImp*>(_surface.getTexture()), _mipLevel, renderTargetMD);

                    //  Redefine the texture mipmap as a render buffer.
                    _surface.postRenderBuffer(face, _mipLevel);
                }
            }               
            break;
        case ACD_RT_DIMENSION_UNKNOWN:
        case ACD_RT_DIMENSION_BUFFER:
        case ACD_RT_DIMENSION_TEXTURE1D:
        case ACD_RT_DIMENSION_TEXTURE2DMS:
        case ACD_RT_DIMENSION_TEXTURE3D:
            panic("ACDRenderTargetImp", "ACDRenderTargetImp", "Unimplement ACD_RT_DIMENSION");
            break;
    }

    //  Render targets created from normal resources don't support compression.
    _compression = false;
}

ACDRenderTargetImp::ACDRenderTargetImp(ACDDeviceImp* device, acd_uint width, acd_uint height, acd_bool multisampling, acd_uint samples, ACD_FORMAT format, bool compression, acd_uint md) :
    _device(device), _compression(compression), _surface(static_cast<ACDTexture2D*>(0))
{

    ACDTexture2DImp* renderTarget = static_cast<ACDTexture2DImp*>(_device->createTexture2D());

    _mipLevel = 0;
    renderTarget->setData(_mipLevel, width, height, format, 0, 0, 0);

    renderTarget->setMemoryLayout(ACD_LAYOUT_RENDERBUFFER);

    renderTarget->postRenderBuffer(_mipLevel);

    MemoryObjectAllocator& moa = _device->allocator();

    moa.assign(renderTarget, _mipLevel, md);

    TextureAdapter surface (renderTarget);
    _surface = surface;

    _dimension = ACD_RT_DIMENSION_TEXTURE2D;

    _face = static_cast<ACD_CUBEMAP_FACE>(0);


}

acd_uint ACDRenderTargetImp::getWidth() const
{
    return _surface.getWidth(_face, _mipLevel);
}

acd_uint ACDRenderTargetImp::getHeight() const
{

    return _surface.getHeight(_face, _mipLevel);
}

acd_bool ACDRenderTargetImp::isMultisampled() const
{
    return _surface.getMultisampled(_face, _mipLevel);
}

acd_uint ACDRenderTargetImp::getSamples() const
{
    return _surface.getSamples(_face, _mipLevel);
}

ACD_FORMAT ACDRenderTargetImp::getFormat() const
{
    return _surface.getFormat(_face, _mipLevel);
}

acd_uint ACDRenderTargetImp::md() 
{
    MemoryObjectAllocator& moa = _device->allocator();
    if (_dimension == ACD_RT_DIMENSION_TEXTURE2D)
        return moa.md(static_cast<ACDTexture2DImp*>(_surface.getTexture()) , _mipLevel);
    else if (_dimension == ACD_RT_DIMENSION_TEXTURECM)
        return moa.md(static_cast<ACDTextureCubeMapImp*>(_surface.getTexture()) , _surface.region( _face, _mipLevel));
    else
    {
        panic("ACDRenderTargetImp", "md", "Unexpected texture type.");
        return 0;
    }
}

acd_bool ACDRenderTargetImp::allowsCompression() const
{
    // Only for renderTarget
    return _compression;
}

ACDTexture* ACDRenderTargetImp::getTexture()
{
    return _surface.getTexture();
}



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

#ifndef ACD_RENDER_TARGET_IMP
    #define ACD_RENDER_TARGET_IMP

#include "ACDDeviceImp.h"
#include "ACDRenderTarget.h"
#include "ACDResource.h"
#include "ACDTexture.h"
#include "ACDTextureCubeMap.h"
#include "TextureAdapter.h"

namespace acdlib
{

class ACDRenderTargetImp : public ACDRenderTarget
{
public:

    ACDRenderTargetImp(ACDDeviceImp* device, ACDTexture* resource, ACD_RT_DIMENSION dimension, ACD_CUBEMAP_FACE face, acd_uint mipLevel);
    ACDRenderTargetImp(ACDDeviceImp* device, acd_uint width, acd_uint height, acd_bool multisampling, acd_uint samples, ACD_FORMAT format, bool compression, acd_uint md);

    virtual acd_uint getWidth() const;
    virtual acd_uint getHeight() const;
    virtual acd_bool isMultisampled() const;
    virtual acd_uint getSamples() const;
    virtual ACD_FORMAT getFormat() const;
    virtual acd_bool allowsCompression() const;
    virtual ACDTexture* getTexture();

    acd_uint md();

private:
    
    ACDDeviceImp* _device;
    ACD_RT_DIMENSION _dimension;
    TextureAdapter _surface;
    ACD_CUBEMAP_FACE _face;
    acd_uint _mipLevel;
    acd_bool _compression;
    
};

} 

#endif // ACD_RENDER_TARGET_IMP

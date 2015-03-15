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

#ifndef TEXTUREADAPTER
    #define TEXTUREADAPTER

#include "ACDTexture2DImp.h"
#include "ACDTextureCubeMapImp.h"
#include "ACDTexture3DImp.h"
#include "TextureMipmap.h"
#include "ACDResource.h"
#include "MemoryObject.h"

namespace acdlib
{

/*
 * Texture adapter class adapts all texture types defined in ACD to a common interface
 * This allows to perform several query schemes independently of the texture type
 *
 * @note It is mandatory that the texture is "well-defined"
 */
class TextureAdapter
{
public:
    
    /**
     * MANDATORY: The texture to be "adapted" must be in state "well-defined"
     */
    TextureAdapter(ACDTexture* tex);


    ACDTexture* getTexture();


    const acd_ubyte* getData(ACD_CUBEMAP_FACE face, acd_uint mipmap, acd_uint& memorySizeInBytes, acd_uint& rowPitch) const;


    const acd_ubyte* getMemoryData(ACD_CUBEMAP_FACE face, acd_uint mipmap, acd_uint& memorySizeInBytes) const;


    acd_uint getWidth(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    acd_uint getHeight(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    acd_uint getDepth(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    acd_uint getTexelSize(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    ACD_FORMAT getFormat(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    acd_bool getMultisampled(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    acd_uint getSamples(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    acd_uint region(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    MemoryObjectState getState(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;


    acd_bool getMultisampling(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;

    void postRenderBuffer(ACD_CUBEMAP_FACE face, acd_uint mipLevel);


    void setMemoryLayout(ACD_MEMORY_LAYOUT layout);


    void updateMipmap(ACD_CUBEMAP_FACE face,
                             acd_uint mipLevel,
                             acd_uint x,
                             acd_uint y,
                             acd_uint z,
                             acd_uint width,
                             acd_uint height,
                             acd_uint depth,
                             ACD_FORMAT format,
                             acd_uint rowPitch,
                             const acd_ubyte* srcTexelData);

private:

    ACD_RESOURCE_TYPE _type;

    ACDTexture2DImp* _tex2D;

    ACDTexture3DImp* _tex3D;

    ACDTextureCubeMapImp* _texCM;
};

}

#endif // TEXTUREADAPTER

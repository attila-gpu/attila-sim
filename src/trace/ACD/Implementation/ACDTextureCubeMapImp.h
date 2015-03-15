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

#ifndef ACD_TEXTURECUBEMAP_IMP
    #define ACD_TEXTURECUBEMAP_IMP

#include "ACDTextureCubeMap.h"
#include "TextureMipmapChain.h"
#include "MemoryObject.h"
#include <set>

namespace acdlib
{

class ACDTextureCubeMapImp : public ACDTextureCubeMap, public MemoryObject
{
public:

    ACDTextureCubeMapImp();

    /// Methods inherited from ACDResource interface

    virtual void setUsage(ACD_USAGE usage); // Afegit

    virtual ACD_USAGE getUsage() const; // Afegit

    virtual void setMemoryLayout(ACD_MEMORY_LAYOUT layout);
    
    virtual ACD_MEMORY_LAYOUT getMemoryLayout() const;

    virtual ACD_RESOURCE_TYPE getType() const;

    virtual void setPriority(acd_uint prio);

    virtual acd_uint getPriority() const;

    virtual acd_uint getSubresources() const;

    virtual acd_bool wellDefined() const;

    /// Methods inherited form ACDTexture interface

    virtual acd_uint getBaseLevel() const;

    virtual acd_uint getMaxLevel() const;

    virtual void setBaseLevel(acd_uint minMipLevel);

    virtual void setMaxLevel(acd_uint maxMipLevel);

	virtual acd_uint getSettedMipmaps();

    /// Methods inherited from ACDTextureCubeMap interface

    virtual acd_uint getWidth(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;

    virtual acd_uint getHeight(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;

    virtual ACD_FORMAT getFormat(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;

    virtual acd_bool isMultisampled(ACD_CUBEMAP_FACE face, acd_uint mipLevel) const;

    virtual acd_uint getTexelSize(ACD_CUBEMAP_FACE face, acd_uint mipmap) const;

    virtual void setData( ACD_CUBEMAP_FACE face,
                          acd_uint mipLevel,
                          acd_uint width,
                          acd_uint height,
                          ACD_FORMAT format,
                          acd_uint rowPitch,
                          const acd_ubyte* srcTexelData,
                          acd_uint texSize,
                          acd_bool preloadData = false);

    virtual void updateData( ACD_CUBEMAP_FACE face,
                             acd_uint mipLevel,
                             acd_uint x,
                             acd_uint y,
                             acd_uint width,
                             acd_uint height,
                             ACD_FORMAT format,
                             acd_uint rowPitch,
                             const acd_ubyte* srcTexelData,
                             acd_bool preloadData = false);

    virtual acd_bool map( ACD_CUBEMAP_FACE face,
                          acd_uint mipLevel,
                          ACD_MAP mapType,
                          acd_ubyte*& pData,
                          acd_uint& dataRowPitch);

    virtual acd_bool unmap(ACD_CUBEMAP_FACE face, acd_uint mipLevel, acd_bool preloadData = false);

    /// Method required by MemoryObject derived classes
    virtual const acd_ubyte* memoryData(acd_uint region, acd_uint& memorySizeInBytes) const;

    const acd_char* stringType() const;


    /// Extended interface ACDTextureCubeMapImp

    // Obtains the corresponding memoy region for a given face/mipmap pair
    static acd_uint translate2region(ACD_CUBEMAP_FACE face, acd_uint mipLevel);

    // Obtains the corresponding face/mipmap pair for a given memory region
    static void translate2faceMipmap(acd_uint mipRegion, ACD_CUBEMAP_FACE& face, acd_uint& mipLevel);

    void dumpMipmap(ACD_CUBEMAP_FACE face, acd_uint region, acd_ubyte* mipName);

    const acd_ubyte* getData(ACD_CUBEMAP_FACE face, acd_uint mipLevel, acd_uint& memorySizeInBytes, acd_uint& rowPitch) const;

private:

    const TextureMipmap* _getMipmap(ACD_CUBEMAP_FACE, acd_uint mipLevel, const acd_char* methodStr) const;

    acd_uint _baseLevel;
    acd_uint _maxLevel;
    std::set<acd_uint> _mappedMips[6]; // Mapped mipmaps
    TextureMipmapChain _mips[6]; // Define 6 Mipmap chains

    ACD_MEMORY_LAYOUT layout;
};

}

#endif // ACD_TEXTURECUBEMAP_IMP

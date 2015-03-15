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

#ifndef ACD_TEXTURE2D_IMP
    #define ACD_TEXTURE2D_IMP

#include "ACDTexture2D.h"
#include "TextureMipmapChain.h"
#include "TextureMipmap.h"
#include "MemoryObject.h"
#include <set>

namespace acdlib
{

class ACDTexture2DImp : public ACDTexture2D, public MemoryObject
{
public:

    ACDTexture2DImp();

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

    /// Methods inherited from ACDTexture2D interface

    virtual acd_uint getWidth(acd_uint mipmap) const;

    virtual acd_uint getHeight(acd_uint mipmap) const;

    virtual ACD_FORMAT getFormat(acd_uint mipmap) const;

    virtual acd_bool isMultisampled(acd_uint mipmap) const;
    
    virtual acd_uint getSamples(acd_uint mipmap) const;
    
    virtual acd_uint getTexelSize(acd_uint mipmap) const;

    virtual void copyData( acd_uint srcMipLevel, acd_uint dstMipLevel, ACDTexture2D* destTexture);

    virtual void setData( acd_uint mipLevel,
                          acd_uint width,
                          acd_uint height,
                          ACD_FORMAT format,
                          acd_uint rowPitch,
                          const acd_ubyte* srcTexelData,
                          acd_uint texSize,
                          acd_bool preloadData = false);

    virtual void setData(acd_uint mipLevel,
                         acd_uint width,
                         acd_uint height,
                         acd_bool multisampling,
                         acd_uint samples,
                         ACD_FORMAT format);

    virtual void updateData( acd_uint mipLevel, const acd_ubyte* srcTexelData, acd_bool preloadData);

    virtual void updateData( acd_uint mipLevel,
                             acd_uint x,
                             acd_uint y,
                             acd_uint width,
                             acd_uint height,
                             ACD_FORMAT format,
                             acd_uint rowPitch,
                             const acd_ubyte* srcTexelData,
                             acd_bool preloadData = false);

    virtual acd_bool map( acd_uint mipLevel,
                      ACD_MAP mapType,
                      acd_ubyte*& pData,
                      acd_uint& dataRowPitch );

    virtual acd_bool unmap(acd_uint mipLevel, acd_bool preloadData = false);

    /// Method required by MemoryObject derived classes
    virtual const acd_ubyte* memoryData(acd_uint region, acd_uint& memorySizeInBytes) const;

    virtual const acd_char* stringType() const;

    void dumpMipmap(acd_uint region, acd_ubyte* mipName);

    const acd_ubyte* getData(acd_uint mipLevel, acd_uint& memorySizeInBytes, acd_uint& rowPitch) const;

    static acd_ubyte* compressTexture(ACD_FORMAT originalFormat, ACD_FORMAT compressFormat, acd_uint width, acd_uint height, acd_ubyte* originalData);

    static acd_uint compressionDifference(acd_uint texel, acd_uint compare, acd_uint rowSize, acd_ubyte* data);

    static acd_ushort convertR5G6B5(acd_uint originalRGB);

    static acd_uint getTexelSize(ACD_FORMAT format) ;

private:

    const TextureMipmap* _getMipmap(acd_uint mipLevel, const acd_char* methodStr) const;

    acd_uint _baseLevel;
    acd_uint _maxLevel;
    TextureMipmapChain _mips;
    std::set<acd_uint> _mappedMips;
    
    ACD_MEMORY_LAYOUT layout;
    
}; // class ACDTexture2DImp


}

#endif // ACD_TEXTURE2D_IMP

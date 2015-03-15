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

#include "ACDTextureCubeMapImp.h"
#include "ACDMacros.h"
#include "ACDMath.h"
#include "support.h"
#include <iostream>
#include <sstream>


using namespace std;
using namespace acdlib;

ACDTextureCubeMapImp::ACDTextureCubeMapImp() : _baseLevel(0), _maxLevel(0), layout(ACD_LAYOUT_TEXTURE)
{

}

ACD_RESOURCE_TYPE ACDTextureCubeMapImp::getType() const
{
    return ACD_RESOURCE_TEXTURECUBEMAP;
}

void ACDTextureCubeMapImp::setUsage(ACD_USAGE usage)
{
    cout << "ACDTexture2DImp::setUsage() -> Not implemented yet" << endl;
}

ACD_USAGE ACDTextureCubeMapImp::getUsage() const
{
    cout << "ACDTexture2DImp::getUsage() -> Not implemented yet" << endl;
    return ACD_USAGE_STATIC;
}

void ACDTextureCubeMapImp::setMemoryLayout(ACD_MEMORY_LAYOUT _layout)
{
    layout = _layout;
}

ACD_MEMORY_LAYOUT ACDTextureCubeMapImp::getMemoryLayout() const
{
    return layout;
}

void ACDTextureCubeMapImp::setPriority(acd_uint prio)
{
    cout << "ACDTextureCubeMapImp::setPriority() -> Not implemented yet" << endl;
}

acd_uint ACDTextureCubeMapImp::getPriority() const
{
    cout << "ACDTextureCubeMapImp::getPriority() -> Not implemented yet" << endl;
    return 0;
}

acd_bool ACDTextureCubeMapImp::wellDefined() const
{
    /// Check texture completeness ///
    
    if ( _baseLevel > _maxLevel ) 
    {
        panic("ACDTexture2DImp", "welldDefined", "Base mipmap level > Max mipmap level");
        return false;
    }

    /// Get BaseLevel mipmap of face 0
    const TextureMipmap* baseMip = _mips[0].find(_baseLevel);
    if ( baseMip == 0 ) 
    {
        stringstream ss;
        ss << "BaseLevel mipmap " << _baseLevel << " of face 0 is not defined";
        panic("ACDTextureCubeMapImp", "welldDefined", ss.str().c_str());
        return false;
    }

    acd_uint wNext = baseMip->getWidth();
    acd_uint hNext = baseMip->getHeight();
    ACD_FORMAT format = baseMip->getFormat();

    // These three variables are defined just to be able to call getData()
    const acd_ubyte* dummyPData;
    acd_uint dummyRowPitch;
    acd_uint dummyPlanePitch;

    // Compare the BaseLevel mipmap of each cubemap face
    for ( acd_uint i = 0; i < 6; ++i ) 
    {
        const TextureMipmap* tm = _mips[i].find(_baseLevel);
        if ( tm == 0 ) 
        {
            stringstream ss;
            ss << "BaseLevel mipmap " << _baseLevel << " of face " << i << " is not defined";
            panic("ACDTextureCubeMapImp", "welldDefined", ss.str().c_str());
            return false;
        }

        if ( wNext != tm->getWidth() )
            panic("ACDTextureCubeMapImp", "welldDefined", "BaseLevel mipmap of all faces have not the same width");

        if ( hNext != tm->getHeight() )
            panic("ACDTextureCubeMapImp", "welldDefined", "BaseLevel mipmap of all faces have not the same height");

        if ( format != tm->getFormat() )
            panic("ACDTextureCubeMapImp", "welldDefined", "BaseLevel mipmap of all faces have not the same format");

        if ( tm->getData(dummyPData, dummyRowPitch, dummyPlanePitch) == 0 ) 
        {
            stringstream ss;
            ss << "Baselevel mipmap " << _baseLevel << " has not defined data";
            panic("ACDTexture2DImp", "welldDefined", ss.str().c_str());
            return false;
        }

        if ( _mappedMips[i].count(_baseLevel) != 0 ) {
            stringstream ss;
            ss << "Baselevel mipmap " << _baseLevel << " is mapped";
            panic("ACDTexture2DImp", "welldDefined", ss.str().c_str());
            return false;
        }
    }

    /// Check mipmap chain size of each cubemap face
    vector<const TextureMipmap*> mips[6];
    for ( acd_uint i = 0; i < 6; ++i ) 
    {
        mips[i] = _mips[i].getMipmaps(_baseLevel, _maxLevel);

        if ( _maxLevel - _baseLevel + 1 != mips[i].size() )
            panic("ACDTextureCubeMapImp", "wellDefined", "Some required mipmaps are not defined");
    }

    for ( acd_uint i = 0; i < mips[0].size(); ++i ) 
    {
        for ( acd_uint face = 0; face < 6; ++face ) 
        {
            const TextureMipmap& mip = *(mips[face][i]); // create a reference alias
            if ( _mappedMips[face].count(i+_baseLevel) != 0 ) 
            {
                stringstream ss;
                ss << "Mipmap " << i + _baseLevel << " of face " << face << " is mapped";
                panic("ACDTextureCubeMapImp", "welldDefined", ss.str().c_str());
                return false;
            }

            if ( mip.getData(dummyPData, dummyRowPitch, dummyPlanePitch) == 0 ) {
                stringstream ss;
                ss << "Mipmap " << i + _baseLevel << " has not defined data";
                panic("ACDTextureCubeMapImp", "welldDefined", ss.str().c_str());
                return false;
            }

            // Check format
            if ( mip.getFormat() != format ) 
            {
                stringstream ss;
                ss << "Mipmap " << i << " format is different from base level format";
                panic("ACDTextureCubeMapImp", "welldDefined", ss.str().c_str());
                return false;
            }

            // Check width & height
            if ( mip.getWidth() != wNext ) 
            {
                stringstream ss;
                ss << "Mipmap " << i << " width is " << mips[face][i]->getWidth() << " and " << wNext << " was expected";
                panic("ACDTextureCubeMapImp", "welldDefined", ss.str().c_str());
                return false;
            }

            if ( mip.getHeight() != hNext ) 
            {
                stringstream ss;
                ss << "Mipmap " << i << " height is " << mips[face][i]->getHeight() << " and " << hNext << " was expected";
                panic("ACDTextureCubeMapImp", "welldDefined", ss.str().c_str());
                return false;
            }
        }

        // compute next expected mipmap size
        hNext = acdlib::max(static_cast<acd_uint>(1), hNext/2);
        wNext = acdlib::max(static_cast<acd_uint>(1), wNext/2);
    }

    return true;
}

acd_uint ACDTextureCubeMapImp::getWidth(ACD_CUBEMAP_FACE face, acd_uint mipLevel) const
{
    return _getMipmap(face, mipLevel, "getWidth")->getWidth();
}

acd_uint ACDTextureCubeMapImp::getHeight(ACD_CUBEMAP_FACE face, acd_uint mipLevel) const
{
    return _getMipmap(face, mipLevel, "getHeight")->getHeight();
}

acd_uint ACDTextureCubeMapImp::getTexelSize(ACD_CUBEMAP_FACE face, acd_uint mipLevel) const
{

    return _getMipmap(face, mipLevel, "getTexelSize")->getTexelSize();
}

ACD_FORMAT ACDTextureCubeMapImp::getFormat(ACD_CUBEMAP_FACE face, acd_uint mipLevel) const
{
    return _getMipmap(face, mipLevel, "getFormat")->getFormat();
}

acd_bool ACDTextureCubeMapImp::isMultisampled(ACD_CUBEMAP_FACE face, acd_uint mipLevel) const
{
    return _getMipmap(face, mipLevel, "isMultisampled")->isMultisampled();
}

const TextureMipmap* ACDTextureCubeMapImp::_getMipmap(ACD_CUBEMAP_FACE face, acd_uint mipLevel, 
                                                      const acd_char* methodStr) const
{
    const TextureMipmap* mipmap = _mips[face].find(mipLevel);
    if ( mipmap == 0 ) {
        stringstream ss;
        ss << "Mipmap level " << mipLevel << " of face " << static_cast<acd_uint>(face) << " is not defined";
        panic("ACDTextureCubeMapImp", methodStr, ss.str().c_str());
    }
    return mipmap;
}

const acd_ubyte* ACDTextureCubeMapImp::memoryData(acd_uint region, acd_uint& memorySizeInBytes) const
{
    acd_uint mipLevel;
    ACD_CUBEMAP_FACE face;

    // Obtain the mipmap face & level identified by region
    translate2faceMipmap(region, face, mipLevel);

    const TextureMipmap* mip = _mips[face].find(mipLevel);
    if ( mip == 0 ) {
        stringstream ss;
        ss << "Mipmap " << region << " not found (not defined) - face=" << static_cast<acd_uint>(face)
           << " mipLevel = " << mipLevel << "\n";
        panic("ACDTextureCubeMapImp", "memoryData", ss.str().c_str());
    }

    return mip->getDataInMortonOrder(memorySizeInBytes);
}

const acd_char* ACDTextureCubeMapImp::stringType() const
{
    return "TEXTURE_CUBEMAP_OBJECT";
}

acd_uint ACDTextureCubeMapImp::getSubresources() const
{
    ACD_ASSERT(
        if ( _baseLevel > _maxLevel )
            panic("ACDTexture2DImp", "getSubresources", "Base mipmap level > Max mipmap level");
    )
    return 6*(_maxLevel - _baseLevel + 1);
}

void ACDTextureCubeMapImp::setData( ACD_CUBEMAP_FACE face,
                                    acd_uint mipLevel,
                                    acd_uint width,
                                    acd_uint height,
                                    ACD_FORMAT format,
                                    acd_uint rowPitch,
                                    const acd_ubyte* srcTexelData,
                                    acd_uint texSize,
                                    acd_bool preloadData)
{
    ACD_ASSERT(
        if ( face > ACD_CUBEMAP_NEGATIVE_Z )
            panic("ACDTextureCubeMapImp", "setData", "Invalid face");
    )

    if ( _mappedMips[face].count(mipLevel) != 0 )
        panic("ACDTexture2DImp", "setData", "Cannot call setData on a mapped mipmap");

    //  Adjust the base (minimum) mip level as new mip levels are added to the texture.
    if (_baseLevel > mipLevel)
        _baseLevel = mipLevel;

    //  Adjust the top (maximum) mip level as new mip levels are added to the texture.
    if (mipLevel > _maxLevel)
        _maxLevel = mipLevel;

    // Create or redefine the mipmap level
    TextureMipmap* mip = _mips[face].create(mipLevel);
    
    // Set texel data into the mipmap
    mip->setData(width, height, 1, format, rowPitch, srcTexelData, texSize);

    // Convert the mipmap/face identifier to a unique region identifier 
    acd_uint mipRegion = translate2region(face, mipLevel);

    // Add the mipmap to the tracking mechanism supported by all ACD Resources
    defineRegion(mipRegion);

    // New contents defined, texture must be reallocated before being used by the GPU
    postReallocate(mipRegion);

    preload(mipRegion, preloadData);
}

void ACDTextureCubeMapImp::updateData( ACD_CUBEMAP_FACE face,
                                       acd_uint mipLevel,
                                       acd_uint x,
                                       acd_uint y,
                                       acd_uint width,
                                       acd_uint height,
                                       ACD_FORMAT format,
                                       acd_uint rowPitch,
                                       const acd_ubyte* srcTexelData,
                                       acd_bool preloadData)
{
    ACD_ASSERT(
        if ( face > ACD_CUBEMAP_NEGATIVE_Z )
            panic("ACDTextureCubeMapImp", "updateData", "Invalid face");
    )

    if ( _mappedMips[face].count(mipLevel) != 0 )
        panic("ACDTexture2DImp", "updateData", "Cannot call updateData on a mapped mipmap");

    // Create or redefine the mipmap level
    //TextureMipmap* mip = _mips[face].create(mipLevel);
    TextureMipmap* mip = _mips[face].find(mipLevel);

    ACD_ASSERT(
        if ( mip == 0 )
            panic("ACDTexture2DImp", "updateData", "Trying to update an undefined mipmap");
    )

    // Update texel data into the mipmap
    mip->updateData(x, y, 0, width, height, 1, format, rowPitch, srcTexelData);

    // Convert the mipmap/face identifier to a unique region identifier 
    acd_uint mipRegion = translate2region(face, mipLevel);

    // Mark all the texture mipmap as pendent to be updated
    postUpdate(mipRegion, 0, mip->getTexelSize() * mip->getWidth() * mip->getHeight() - 1);

    preload(mipRegion, preloadData);
}

acd_bool ACDTextureCubeMapImp::map( ACD_CUBEMAP_FACE face,
                                    acd_uint mipLevel,
                                    ACD_MAP mapType,
                                    acd_ubyte*& pData,
                                    acd_uint& dataRowPitch )
{
    ACD_ASSERT(
        if ( face > ACD_CUBEMAP_NEGATIVE_Z )
            panic("ACDTextureCubeMapImp", "map", "Invalid face");
        if ( _mappedMips[face].count(mipLevel) != 0 )
            panic("ACDTextureCubeMapImp", "map", "Cannot map an already mapped mipmap");
    )

    TextureMipmap* mip = _mips[face].find(mipLevel);

    if ( mip == 0 ) {
        stringstream ss;
        ss << "Mipmap " << mipLevel << " not defined";
        panic("ACDTextureCubeMapImp", "map", ss.str().c_str());
    }

    acd_uint mipRegion = translate2region(face, mipLevel);

    if ( getState(mipRegion) == MOS_Blit && (mapType == ACD_MAP_READ || ACD_MAP_READ_WRITE) )
        panic("ACDTextureCubeMapImp", "map", "Map for reading not supported with memory object region in BLIT state");

    // Mark this mipmap as mapped
    _mappedMips[face].insert(mipLevel);

    acd_uint dataPlanePitch;

    return mip->getData(pData, dataRowPitch, dataPlanePitch);
}

acd_bool ACDTextureCubeMapImp::unmap(ACD_CUBEMAP_FACE face, acd_uint mipLevel, acd_bool preloadData)
{
    ACD_ASSERT(
        if ( face > ACD_CUBEMAP_NEGATIVE_Z )
            panic("ACDTextureCubeMapImp", "map", "Invalid face");
        if ( _mappedMips[face].count(mipLevel) != 0 )
            panic("ACDTextureCubeMapImp", "unmap", "Unmapping a not mapped texture mipmap");
    )

    // remove the mipmap level from the list of mapped mipmaps
    _mappedMips[face].erase(mipLevel);

    acd_uint mipRegion = translate2region(face, mipLevel);

    // Update all (conservative)
    postUpdate(mipRegion, 0, getTexelSize(face, mipLevel) * getWidth(face, mipLevel) * getHeight(face, mipLevel) - 1);
    
    preload(mipRegion, preloadData);
    
    return true;
}


acd_uint ACDTextureCubeMapImp::getBaseLevel() const { return _baseLevel; }

acd_uint ACDTextureCubeMapImp::getMaxLevel() const { return _maxLevel; }

void ACDTextureCubeMapImp::setBaseLevel(acd_uint minMipLevel) 
{ 
    // Clamp if required
    _baseLevel = ( minMipLevel > ACD_MAX_TEXTURE_LEVEL ? ACD_MAX_TEXTURE_LEVEL : minMipLevel );
}

void ACDTextureCubeMapImp::setMaxLevel(acd_uint maxMipLevel) 
{
    // Clamp if required
    _maxLevel = ( maxMipLevel > ACD_MAX_TEXTURE_LEVEL ? ACD_MAX_TEXTURE_LEVEL : maxMipLevel );
}

acd_uint ACDTextureCubeMapImp::getSettedMipmaps()
{
    return _mips[0].size();
}

acd_uint ACDTextureCubeMapImp::translate2region(ACD_CUBEMAP_FACE face, acd_uint mipLevel)
{
    return mipLevel + face * (ACD_MAX_TEXTURE_LEVEL + 1);
}

void ACDTextureCubeMapImp::translate2faceMipmap(acd_uint mipRegion, ACD_CUBEMAP_FACE& face, acd_uint& mipLevel)
{
    acd_uint faceID = mipRegion / (ACD_MAX_TEXTURE_LEVEL + 1);
    
    ACD_ASSERT(
        if ( faceID > ACD_CUBEMAP_NEGATIVE_Z )
            panic("ACDTextureCubeMapImp", "translate2faceMipmap", "mipRegion face out of bounds");
    )

    face = static_cast<ACD_CUBEMAP_FACE>(faceID);
    mipLevel = mipRegion % (ACD_MAX_TEXTURE_LEVEL + 1);
}

void ACDTextureCubeMapImp::dumpMipmap(ACD_CUBEMAP_FACE face, acd_uint region, acd_ubyte* mipName)
{
    TextureMipmap* mip = _mips[face].find(region);

    if ( mip == 0 ) {
        stringstream ss;
        ss << "Mipmap " << region << " not found (not defined)";
        panic("ACDTextureCubeMapImp", "dumpMipmap", ss.str().c_str());
    }

    mip->dump2PPM(mipName);

}

const acd_ubyte* ACDTextureCubeMapImp::getData(ACD_CUBEMAP_FACE face, acd_uint mipLevel, acd_uint& memorySizeInBytes, acd_uint& rowPitch) const
{
    acd_ubyte* data;
    acd_uint planePitch;
    TextureMipmap* mip = (TextureMipmap*)(_mips[face].find(mipLevel));
    mip->getData(data, rowPitch, planePitch);

    memorySizeInBytes = mip->getTexelSize() * mip->getHeight() * mip->getWidth();

    return data;
    
}

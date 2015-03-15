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
#include <sstream>
#include <vector>

#include "ACDTexture3DImp.h"
#include "support.h"
#include "ACDMacros.h"
#include "ACDMath.h"

#include "GlobalProfiler.h"

using namespace acdlib;

using std::vector;
using std::cout;
using std::endl;
using std::stringstream;

ACDTexture3DImp::ACDTexture3DImp() : 
    _baseLevel(0), _maxLevel(0), layout(ACD_LAYOUT_TEXTURE)
{
    _mips.destroyMipmaps();
}

ACD_RESOURCE_TYPE ACDTexture3DImp::getType() const
{
    return ACD_RESOURCE_TEXTURE3D;
}

void ACDTexture3DImp::setUsage(ACD_USAGE usage)
{
    cout << "ACDTexture3DImp::setUsage() -> Not implemented yet" << endl;
}

ACD_USAGE ACDTexture3DImp::getUsage() const
{
    cout << "ACDTexture3DImp::getUsage() -> Not implemented yet" << endl;
    return ACD_USAGE_STATIC;
}

void ACDTexture3DImp::setMemoryLayout(ACD_MEMORY_LAYOUT _layout)
{
    layout = _layout;
}

ACD_MEMORY_LAYOUT ACDTexture3DImp::getMemoryLayout() const
{
    return layout;
}

void ACDTexture3DImp::setPriority(acd_uint prio)
{
    cout << "ACDTexture3DImp::setPriority() -> Not implemented yet" << endl;
}

acd_uint ACDTexture3DImp::getPriority() const
{
    cout << "ACDTexture3DImp::getPriority() -> Not implemented yet" << endl;
    return 0;
}

acd_uint ACDTexture3DImp::getSubresources() const
{
    ACD_ASSERT(
        if ( _baseLevel > _maxLevel )
            panic("ACDTexture3DImp", "getSubresources", "Base mipmap level > Max mipmap level");
    )
    return _maxLevel - _baseLevel + 1;
}

acd_bool ACDTexture3DImp::wellDefined() const
{
    GLOBALPROFILER_ENTERREGION("ACDTexture3DImp", "", "")
    
    /// Check texture completeness ///
    
    if ( _baseLevel > _maxLevel ) {
        panic("ACDTexture3DImp", "welldDefined", "Base mipmap level > Max mipmap level");
        return false;
    }

    acd_uint wNext, hNext, dNext;

    acd_uint baseLevel = _baseLevel;

    const TextureMipmap* baseMip = _mips.find(baseLevel);
    if ( baseMip == 0 ) 
    {
        stringstream ss;
        ss << "Mipmap " << baseLevel << " is not defined";
        panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
        return false;
    }
    
    vector<const TextureMipmap*> mips = _mips.getMipmaps(_baseLevel, _maxLevel);
   
    //Check if all required mipmaps are defined
    if ( _maxLevel - _baseLevel + 1 != mips.size() )
        panic("ACDTexture3DImp", "wellDefined", "Some required mipmaps are not defined");

    wNext = baseMip->getWidth();
    hNext = baseMip->getHeight();
    dNext = baseMip->getDepth();
    ACD_FORMAT format = baseMip->getFormat();

    // These three variables are defined just to be able to call getData()
    const acd_ubyte* dummyPData;
    acd_uint dummyRowPitch;
    acd_uint dummyPlanePitch;

    if ( baseMip->getData(dummyPData, dummyRowPitch, dummyPlanePitch) == 0 ) 
    {
        stringstream ss;
        ss << "Baselevel mipmap " << baseLevel << " has not defined data";
        panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
        return false;
    }

    if ( _mappedMips.count(baseLevel) != 0 ) 
    {
        stringstream ss;
        ss << "Baselevel mipmap " << baseLevel << " is mapped";
        panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
        return false;
    }

    for ( acd_uint i = 0; i < mips.size(); ++i ) 
    {
        const TextureMipmap& mip = *mips[i]; // create a reference alias

       /* if ( _mappedMips.count(i+baseLevel) != 0 ) 
        {
            stringstream ss;
            ss << "Mipmap " << i + baseLevel << " is mapped";
            panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
            return false;
        }

        // Check data
        if ( mip.getData(dummyPData, dummyRowPitch, dummyPlanePitch) == 0 ) 
        {
            stringstream ss;
            ss << "Mipmap " << i + baseLevel << " has not defined data";
            panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
            return false;
        }*/

        // Check format
        /*if ( mip.getFormat() != format ) 
        {
            stringstream ss;
            ss << "Mipmap " << i << " format is different from base level format";
            panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
            return false;
        }*/

        // Check width & height & depth
        /*if ( mip.getWidth() != wNext ) 
        {
            stringstream ss;
            ss << "Mipmap " << i << " width is " << mips[i]->getWidth() << " and " << wNext << " was expected";
            panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
            return false;
        }

        if ( mip.getHeight() != hNext ) 
        {
            stringstream ss;
            ss << "Mipmap " << i << " height is " << mips[i]->getHeight() << " and " << hNext << " was expected";
            panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
            return false;
        }

        if ( mip.getDepth() != dNext ) 
        {
            stringstream ss;
            ss << "Mipmap " << i << " height is " << mips[i]->getDepth() << " and " << dNext << " was expected";
            panic("ACDTexture3DImp", "welldDefined", ss.str().c_str());
            return false;
        }
*/
        // compute next expected mipmap size
        hNext = acdlib::max(static_cast<acd_uint>(1), hNext/2);
        wNext = acdlib::max(static_cast<acd_uint>(1), wNext/2);
        dNext = acdlib::max(static_cast<acd_uint>(1), dNext/2);
    }

    GLOBALPROFILER_EXITREGION()
    return true;
}

acd_uint ACDTexture3DImp::getWidth(acd_uint mipLevel) const 
{ 
    return _getMipmap(mipLevel, "getWidth")->getWidth();
}

acd_uint ACDTexture3DImp::getHeight(acd_uint mipLevel) const 
{
    return _getMipmap(mipLevel, "getHeight")->getHeight();
}

acd_uint ACDTexture3DImp::getDepth(acd_uint mipLevel) const
{
    return _getMipmap(mipLevel, "getDepth")->getDepth();
}

acd_uint ACDTexture3DImp::getTexelSize(acd_uint mipLevel) const 
{
    return _getMipmap(mipLevel, "getTexelSize")->getTexelSize();
}

ACD_FORMAT ACDTexture3DImp::getFormat(acd_uint mipLevel) const
{
    return _getMipmap(mipLevel, "getFormat")->getFormat();
}

acd_bool ACDTexture3DImp::isMultisampled(acd_uint mipLevel) const
{
    return _getMipmap(mipLevel, "isMultisampled")->isMultisampled();
}

acd_uint ACDTexture3DImp::getSamples(acd_uint mipLevel) const
{
    return _getMipmap(mipLevel, "getSamples")->getSamples();
}

const TextureMipmap* ACDTexture3DImp::_getMipmap(acd_uint mipLevel, const acd_char* methodStr) const
{
    GLOBALPROFILER_ENTERREGION("ACDTexture3DImp", "", "")

    const TextureMipmap* mip = _mips.find(mipLevel);

    if ( mip == 0 ) 
    {
        stringstream ss;
        ss << "Mipmap " << mipLevel << " not found (not defined)";
        panic("ACDTexture3DImp", methodStr, ss.str().c_str());
    }

    GLOBALPROFILER_EXITREGION()
    return mip;
}

const acd_ubyte* ACDTexture3DImp::memoryData(acd_uint region, acd_uint& memorySizeInBytes) const
{
    const TextureMipmap* mip = _mips.find(region);

    if ( mip == 0 ) 
    {
        stringstream ss;
        ss << "Mipmap " << region << " not found (not defined)";
        panic("ACDTexture3DImp", "memoryData", ss.str().c_str());
    }

    return mip->getDataInMortonOrder(memorySizeInBytes);
}

const acd_char* ACDTexture3DImp::stringType() const
{
    return "TEXTURE_3D_OBJECT";
}

void ACDTexture3DImp::dumpMipmap(acd_uint region, acd_ubyte* mipName)
{
    TextureMipmap* mip = _mips.find(region);

    if ( mip == 0 ) {
        stringstream ss;
        ss << "Mipmap " << region << " not found (not defined)";
        panic("ACDTexture3DImp", "dumpMipmap", ss.str().c_str());
    }
    
    mip->dump2PPM(mipName);

}

void ACDTexture3DImp::setData( acd_uint mipLevel,
                               acd_uint width,
                               acd_uint height,
                               acd_uint depth,
                               ACD_FORMAT format,
                               acd_uint rowPitch,
                               const acd_ubyte* srcTexelData,
                               acd_uint texSize,
                               acd_bool preloadData)
{
    GLOBALPROFILER_ENTERREGION("ACDTexture3DImp", "", "")
    
    if ( _mappedMips.count(mipLevel) != 0 )
        panic("ACDTexture3DImp", "setData", "Cannot call setData on a mapped mipmap");

    //  Adjust the base (minimum) mip level as new mip levels are added to the texture.
    if (_baseLevel > mipLevel)
        _baseLevel = mipLevel;

    //  Adjust the top (maximum) mip level as new mip levels are added to the texture.
    if (mipLevel > _maxLevel)
        _maxLevel = mipLevel;

    // Create or redefine the mipmap level
    TextureMipmap* mip = _mips.create(mipLevel);
    
    // Set texel data into the mipmap
    mip->setData(width, height, depth, format, rowPitch, srcTexelData, texSize);

    // Add the mipmap to the tracking mechanism supported by all ACD Resources
    defineRegion(mipLevel);

    // New contents defined, texture must be reallocated before being used by the GPU
    postReallocate(mipLevel);

    GLOBALPROFILER_EXITREGION()
}

void ACDTexture3DImp::setData ( acd_uint mipLevel, 
                                acd_uint width, 
                                acd_uint height,
                                acd_uint depth,
                                acd_bool multisampling, 
                                acd_uint samples, 
                                ACD_FORMAT format)
{
    GLOBALPROFILER_ENTERREGION("ACDTexture3DImp", "", "")

    if ( _mappedMips.count(mipLevel) != 0 )
        panic("ACDTexture3DImp", "setData", "Cannot call setData on a mapped mipmap");

    //  Adjust the base (minimum) mip level as new mip levels are added to the texture.
    if (_baseLevel > mipLevel)
        _baseLevel = mipLevel;

    //  Adjust the top (maximum) mip level as new mip levels are added to the texture.
    if (mipLevel > _maxLevel)
        _maxLevel = mipLevel;
    
    // Create or redefine the mipmap level
    TextureMipmap* mip = _mips.create(mipLevel);
    
    // Set texel data into the mipmap
    mip->setData(width, height, depth, multisampling, samples, format);

    // Add the mipmap to the tracking mechanism supported by all ACD Resources
    defineRegion(mipLevel);

    GLOBALPROFILER_EXITREGION()
}

void ACDTexture3DImp::updateData(   acd_uint mipLevel,
                                    acd_uint x,
                                    acd_uint y,
                                    acd_uint z,
                                    acd_uint width,
                                    acd_uint height,
                                    acd_uint depth,
                                    ACD_FORMAT format,
                                    acd_uint rowPitch,
                                    const acd_ubyte* srcTexelData,
                                    acd_bool preloadData)
{
    GLOBALPROFILER_ENTERREGION("ACDTexture3DImp", "", "")

    if ( _mappedMips.count(mipLevel) != 0 )
        panic("ACDTexture3DImp", "map", "Cannot call updateData on a mapped mipmap");
    
    // Create or redefine the mipmap level
    TextureMipmap* mip = _mips.find(mipLevel);

    ACD_ASSERT(
        if ( mip == 0 )
            panic("ACDTexture3DImp", "updateData", "Trying to update an undefined mipmap");
    )

    // Update texel data into the mipmap
    mip->updateData(x, y, z, width, height, depth, format, rowPitch, srcTexelData);

    // Mark all the texture mipmap as pendent to be updated
    postUpdate(mipLevel, 0, mip->getTexelSize() * mip->getWidth() * mip->getHeight() - 1);

    preload(mipLevel, preloadData);

    GLOBALPROFILER_EXITREGION()
}

acd_bool ACDTexture3DImp::map( acd_uint mipLevel,
                               ACD_MAP mapType,
                               acd_ubyte*& pData,
                               acd_uint& dataRowPitch,
                               acd_uint& dataPlanePitch)
{
    GLOBALPROFILER_ENTERREGION("ACDTexture3DImp", "", "")

    if ( _mappedMips.count(mipLevel) != 0 )
        panic("ACDTexture3DImp", "map", "Cannot map an already mapped mipmap");

    TextureMipmap* mip = _mips.find(mipLevel);

    if ( mip == 0 ) {
        stringstream ss;
        ss << "Mipmap " << mipLevel << " not defined";
        panic("ACDTexture3DImp", "map", ss.str().c_str());
    }

    if ( getState(mipLevel) == MOS_Blit && (mapType == ACD_MAP_READ || ACD_MAP_READ_WRITE) )
        panic("ACDTexture3DImp", "map", "Map for reading not supported with memory object region in BLIT state");
    
    // Mark this mipmap as mapped
    _mappedMips.insert(mipLevel);

    GLOBALPROFILER_EXITREGION()

    return mip->getData(pData, dataRowPitch, dataPlanePitch);
}

acd_bool ACDTexture3DImp::unmap(acd_uint mipLevel, acd_bool preloadData)
{
    GLOBALPROFILER_ENTERREGION("ACDTexture3DImp", "", "")

    ACD_ASSERT
    (
        if ( _mappedMips.count(mipLevel) != 0 ) 
        {
            panic("ACDTexture3DImp", "unmap", "Unmapping a not mapped texture mipmap");
        }
        GLOBALPROFILER_EXITREGION()
        return false;
    )
    
    // remove the mipmap level from the list of mapped mipmaps
    _mappedMips.erase(mipLevel);

    // Update all (conservative)
    postUpdate(mipLevel, 0, getTexelSize(mipLevel) * getWidth(mipLevel) * getHeight(mipLevel) * getDepth(mipLevel) - 1);

    preload(mipLevel, preloadData);

    GLOBALPROFILER_EXITREGION()
    
    return true;
}

acd_uint ACDTexture3DImp::getBaseLevel() const { return _baseLevel; }

acd_uint ACDTexture3DImp::getMaxLevel() const { return _maxLevel; }

void ACDTexture3DImp::setBaseLevel(acd_uint minMipLevel) 
{ 
    // Clamp if required
    _baseLevel = ( minMipLevel > ACD_MAX_TEXTURE_LEVEL ? ACD_MAX_TEXTURE_LEVEL : minMipLevel );
}

void ACDTexture3DImp::setMaxLevel(acd_uint maxMipLevel) 
{
    // Clamp if required
    _maxLevel = ( maxMipLevel > ACD_MAX_TEXTURE_LEVEL ? ACD_MAX_TEXTURE_LEVEL : maxMipLevel );
}

acd_uint ACDTexture3DImp::getSettedMipmaps()
{
    return _mips.size();
}

const acd_ubyte* ACDTexture3DImp::getData(acd_uint mipLevel, acd_uint& memorySizeInBytes, acd_uint& rowPitch, acd_uint& planePitch) const
{
    acd_ubyte* data;

    TextureMipmap* mip = (TextureMipmap*)(_mips.find(mipLevel));
    mip->getData(data, rowPitch, planePitch);

    memorySizeInBytes = mip->getTexelSize() * mip->getHeight() * mip->getWidth() * mip->getDepth();

    return data;
    
}


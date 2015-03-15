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

#ifndef TEXTUREMIPMAP
    #define TEXTUREMIPMAP

#include "ACDTypes.h"
#include "ACDTexture2D.h"

class GPUDriver;

namespace acdlib
{

class TextureMipmap
{
private:

    friend class ACDDeviceImp;

    static void setTextureTiling(acd_uint tileLevel1Sz, acd_uint tileLevel2Sz);
    static void getTextureTiling(acd_uint& tileLevel1Sz, acd_uint& tileLevel2Sz);
    static void setDriver(GPUDriver* driver);

    static acd_uint _tileLevel1Sz;
    static acd_uint _tileLevel2Sz;
    static GPUDriver* _driver;

    // Temporary all is mutable... (bad practise)
    mutable acd_ubyte* _data;
    mutable acd_uint _dataSize;
    mutable acd_uint _rowPitch;
    mutable acd_uint _planePitch;

    mutable acd_uint _width;
    mutable acd_uint _height;
    mutable acd_uint _depth;
    mutable acd_bool _multisampling;
    mutable acd_uint _samples;
    mutable ACD_FORMAT _format;

    mutable acd_ubyte* _mortonData;
    mutable acd_uint _mortonDataSize;
    
    public:

    TextureMipmap();

    acd_uint getWidth() const;
    acd_uint getHeight() const;
    acd_uint getDepth() const;
    ACD_FORMAT getFormat() const;   
    acd_bool isMultisampled() const;    
    acd_uint getSamples() const;

    // Returns the texel size in bytes (only for uncompressed textures)
    acd_float getTexelSize () const;
    static acd_float getTexelSize(ACD_FORMAT format);

    // To implement MAP operation on textures
    // Return true if data has been defined
    bool getData(acd_ubyte*& pData, acd_uint& rowPitch, acd_uint& planePitch);
    bool getData(const acd_ubyte*& pData, acd_uint& rowPitch, acd_uint& planePitch) const;

    void setData( acd_uint width, acd_uint height, acd_uint depth, ACD_FORMAT format, 
                  acd_uint rowPitch, const acd_ubyte* srcTexelData, acd_uint texSize );

    virtual void setData(acd_uint width, acd_uint height, acd_uint depth, acd_bool multisampling, acd_uint samples, ACD_FORMAT format);

    void updateData(const acd_ubyte* srcTexelData);

    void copyData( acd_uint mipLevel, ACDTexture2D* destTexture);

    void updateData( acd_uint xoffset, acd_uint yoffset, acd_uint zoffset, acd_uint width, acd_uint height, acd_uint depth,
                        ACD_FORMAT format, acd_uint rowPitch, const acd_ubyte* srcTexelData );

    void clear();

    const acd_ubyte* getDataInMortonOrder(acd_uint& sizeInBytes) const;

    void dump2PPM (acd_ubyte* filename);
};

}

#endif

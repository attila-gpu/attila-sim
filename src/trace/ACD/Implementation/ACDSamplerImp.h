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

#ifndef ACD_SAMPLER_IMP
    #define ACD_SAMPLER_IMP

#include "ACDSampler.h"
#include "StateComponent.h"
#include "StateItem.h"
#include "GPUDriver.h"
#include "ACDResource.h"
#include "TextureMipmap.h"
#include "GPU.h"

namespace acdlib
{

class ACDDeviceImp;

class ACDSamplerImp : public ACDSampler, public StateComponent
{
public:

    ACDSamplerImp(ACDDeviceImp* device, GPUDriver* driver, acd_uint samplerID);

    virtual void setEnabled(acd_bool enable);

    virtual acd_bool isEnabled() const;

    virtual void setTexture(ACDTexture* texture);

    virtual void setTextureAddressMode(ACD_TEXTURE_COORD coord, ACD_TEXTURE_ADDR_MODE mode);

    virtual void setNonNormalizedCoordinates(acd_bool enable);
    
    virtual void setMinFilter(ACD_TEXTURE_FILTER minFilter);

    virtual void setMagFilter(ACD_TEXTURE_FILTER magFilter);
    
    virtual void setEnableComparison(acd_bool enable);
    
    virtual void setComparisonFunction(ACD_TEXTURE_COMPARISON function);

    virtual void setSRGBConversion(acd_bool enable);

    virtual void setMinLOD(acd_float minLOD);

    virtual void setMaxLOD(acd_float maxLOD);

    virtual void setMaxAnisotropy(acd_uint maxAnisotropy);

    virtual void setLODBias(acd_float lodBias);

    virtual void setUnitLODBias(acd_float unitLodBias);

    virtual void setMinLevel(acd_uint minLevel);
    
    virtual ACD_TEXTURE_FILTER getMagFilter() const;

    virtual ACDTexture* getTexture() const;

    virtual ACD_TEXTURE_FILTER getMinFilter() const;

    virtual std::string getInternalState() const;

    virtual void sync();

    virtual void forceSync();

    void performBlitOperation2D(acd_uint xoffset, acd_uint yoffset, acd_uint x, acd_uint y, acd_uint width, acd_uint height, acd_uint textureWidth, ACD_FORMAT internalFormat, ACDTexture2D* texture, acd_uint level);
    
    const StoredStateItem* createStoredStateItem(ACD_STORED_ITEM_ID stateId) const;

    void restoreStoredStateItem(const StoredStateItem* ssi);

private:


    static const acd_uint _MAX_TEXTURE_MIPMAPS = gpu3d::MAX_TEXTURE_SIZE;

    ACDDeviceImp* _device;
    GPUDriver* _driver;
    acd_bool _requiredSync;

    // Maximum number of 2D images per texture
    std::vector<acd_uint> _gpuMemTrack; // MIPMAPS * CUBEMAP_FACES
    
    const acd_uint _SAMPLER_ID;

    ACDTexture* _texture;

    /// Last texture-dependant values used in this sampler
    StateItem<ACD_RESOURCE_TYPE> _textureType;
    StateItem<ACD_FORMAT> _textureFormat;
    StateItem<acd_uint> _minLevel;
    StateItem<acd_uint> _baseLevel;
    StateItem<acd_uint> _maxLevel;
    StateItem<acd_uint> _textureWidth;
    StateItem<acd_uint> _textureHeight;
    StateItem<acd_uint> _textureDepth;
    StateItem<acd_uint> _textureWidth2;
    StateItem<acd_uint> _textureHeight2;
    StateItem<acd_uint> _textureDepth2;
    StateItem<ACD_MEMORY_LAYOUT> _memoryLayout;
    
    // Sampler dependant values
    StateItem<acd_bool> _enabled;
    StateItem<ACD_TEXTURE_ADDR_MODE> _sCoord;
    StateItem<ACD_TEXTURE_ADDR_MODE> _tCoord;
    StateItem<ACD_TEXTURE_ADDR_MODE> _rCoord;
    StateItem<acd_bool> _nonNormalizedCoords;
    StateItem<ACD_TEXTURE_FILTER> _minFilter;
    StateItem<ACD_TEXTURE_FILTER> _magFilter;
    StateItem<acd_bool> _enableComparison;
    StateItem<ACD_TEXTURE_COMPARISON> _comparisonFunction;
    StateItem<acd_bool> _sRGBConversion;
    StateItem<acd_float> _minLOD;
    StateItem<acd_float> _maxLOD;
    StateItem<acd_uint> _maxAniso;
    StateItem<acd_float> _lodBias;
    StateItem<acd_float> _unitLodBias;

    // Blit dependent values
    StateItem<acd_int> _blitXoffset;
    StateItem<acd_int> _blitYoffset;
    StateItem<acd_int >_blitIniX;
    StateItem<acd_int >_blitIniY;
    StateItem<acd_int> _blitHeight;
    StateItem<acd_int> _blitWidth;
    StateItem<acd_int> _blitWidth2;
    StateItem<ACD_FORMAT> _blitFormat;
    StateItem<ACD_MEMORY_LAYOUT> _blitTextureBlocking;

    static void _getGPUTextureFormat(ACD_FORMAT textureFormat, 
                                         gpu3d::GPURegData* dataFormat,
                                         gpu3d::GPURegData* dataCompression,
                                         gpu3d::GPURegData* dataInverted);

    static void _getGPUClampMode(ACD_TEXTURE_ADDR_MODE mode, gpu3d::GPURegData* data);
    static void _getGPUTexFilter(ACD_TEXTURE_FILTER, gpu3d::GPURegData* data);
    static void _getGPUTexComp(ACD_TEXTURE_COMPARISON, gpu3d::GPURegData* data);
    static void _getGPUTextureMode(ACD_RESOURCE_TYPE, gpu3d::GPURegData* data);
    static void _getGPUTexMemoryLayout(ACD_MEMORY_LAYOUT memLayout, gpu3d::GPURegData* data);

    static const char* _getFormatString(ACD_FORMAT format);

    void _syncTexture();



    // print help classes

    BoolPrint boolPrint;

    class AddressModePrint: public PrintFunc<ACD_TEXTURE_ADDR_MODE>
    {
    public:

        virtual const char* print(const ACD_TEXTURE_ADDR_MODE& var) const;
    }
    addressModePrint;

    class FilterPrint: public PrintFunc<ACD_TEXTURE_FILTER>
    {
    public:

        virtual const char* print(const ACD_TEXTURE_FILTER& var) const;
    }
    filterPrint;

    class CompFuncPrint: public PrintFunc<ACD_TEXTURE_COMPARISON>
    {
    public:
        virtual const char* print(const ACD_TEXTURE_COMPARISON& var) const;
    }
    compFuncPrint;
    
};


}

#endif // ACD_SAMPLER_IMP

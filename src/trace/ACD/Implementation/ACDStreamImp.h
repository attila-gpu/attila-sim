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

#ifndef ACD_STREAM_IMP
    #define ACD_STREAM_IMP

#include "ACDStream.h"
#include "StateItem.h"
#include "StateComponent.h"
#include "GPUDriver.h"
#include "StateItemUtils.h"
#include "ACDStoredStateImp.h"
#include "GPU.h"

namespace acdlib
{
class ACDDeviceImp;
class ACDBufferImp;

class ACDStreamImp : public ACDStream, public StateComponent
{
public:

    ACDStreamImp(ACDDeviceImp* device, GPUDriver* driver, acd_uint streamID);

    virtual void set( ACDBuffer* buffer, const ACD_STREAM_DESC& desc );

    virtual void get( ACDBuffer*& buffer,
                      acd_uint& offset,
                      acd_uint& components,
                      ACD_STREAM_DATA& componentsType,
                      acd_uint& stride,
                      acd_uint& frequency) const;

    virtual void setBuffer(ACDBuffer* buffer);

    virtual void setOffset(acd_uint offset);

    virtual void setComponents(acd_uint components);

    virtual void setType(ACD_STREAM_DATA componentsType);
    
    virtual void setStride(acd_uint stride);
    
    virtual void setFrequency(acd_uint stride);
    
    virtual ACDBuffer* getBuffer() const;
    
    virtual acd_uint getOffset() const;
    
    virtual acd_uint getComponents() const;
    
    virtual ACD_STREAM_DATA getType() const;
    
    virtual acd_uint getStride() const;
    
    virtual acd_uint getFrequency() const;

    virtual std::string DBG_getState() const;

    void sync();

    void forceSync();

    std::string getInternalState() const;

    void ACDDumpStream();

    const StoredStateItem* createStoredStateItem(ACD_STORED_ITEM_ID stateId) const;

    void restoreStoredStateItem(const StoredStateItem* ssi);

    ACDStoredState* saveAllState() const;

    void restoreAllState(const ACDStoredState* ssi);

private:

    const acd_uint _STREAM_ID;

    GPUDriver* _driver;
    ACDDeviceImp* _device;

    acd_bool _requiredSync;
    acd_uint _gpuMemTrack;
    
    StateItem<ACDBufferImp*> _buffer;
    StateItem<acd_uint> _offset;
    StateItem<acd_uint> _components;
    StateItem<ACD_STREAM_DATA> _componentsType;
    StateItem<acd_uint> _stride;
    StateItem<acd_uint> _frequency;

    StateItem<acd_bool> _invertStreamType;

    static void _convertToGPUStreamData(ACD_STREAM_DATA componentsType, gpu3d::GPURegData* data);

    // print help classes
    
    class ComponentTypePrint: public PrintFunc<ACD_STREAM_DATA>
    {
    public:

        virtual const char* print(const ACD_STREAM_DATA& var) const;        
    }
    componentTypePrint;
};

} // namespace acdlib


#endif // ACD_STREAM_IMP


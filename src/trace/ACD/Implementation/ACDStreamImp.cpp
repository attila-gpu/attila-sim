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

#include "ACDStreamImp.h"
#include "ACDDeviceImp.h"
#include "ACDBufferImp.h"
#include "ACDMacros.h"
#include <sstream>
#include "StateItemUtils.h"

#include "GlobalProfiler.h"

using namespace std;
using namespace acdlib;


ACDStreamImp::ACDStreamImp(ACDDeviceImp* device, GPUDriver* driver, acd_uint streamID) : 
    _driver(driver), _device(device), _STREAM_ID(streamID), _requiredSync(true), _gpuMemTrack(0),
    _buffer(0),
    _offset(0),
    _components(0),
    _componentsType(ACD_SD_UINT8),
    _stride(0),
    _frequency(0),
    _invertStreamType(false)
{
    forceSync(); // init GPU streamer(i) state registers    
}

void ACDStreamImp::set( ACDBuffer* buffer, const ACD_STREAM_DESC& desc )
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    
    ACDBufferImp* buf = static_cast<ACDBufferImp*>(buffer);
    _buffer = buf;
    _gpuMemTrack = 0; // Init buffer track

    _offset = desc.offset;
    _components = desc.components;
    _componentsType = desc.type;
    _stride = desc.stride;
    _frequency = desc.frequency;

    switch(_componentsType)
    {
        case ACD_SD_UNORM8:
        case ACD_SD_SNORM8:
        case ACD_SD_UNORM16:
        case ACD_SD_SNORM16:
        case ACD_SD_UNORM32:
        case ACD_SD_SNORM32:
        case ACD_SD_FLOAT16:
        case ACD_SD_FLOAT32:
        case ACD_SD_UINT8:
        case ACD_SD_SINT8:
        case ACD_SD_UINT16:
        case ACD_SD_SINT16:
        case ACD_SD_UINT32:
        case ACD_SD_SINT32:
            _invertStreamType = false;
            break;
        case ACD_SD_INV_UNORM8:
        case ACD_SD_INV_SNORM8:
        case ACD_SD_INV_UNORM16:
        case ACD_SD_INV_SNORM16:
        case ACD_SD_INV_UNORM32:
        case ACD_SD_INV_SNORM32:
        case ACD_SD_INV_FLOAT16:
        case ACD_SD_INV_FLOAT32:
        case ACD_SD_INV_UINT8:
        case ACD_SD_INV_SINT8:
        case ACD_SD_INV_UINT16:
        case ACD_SD_INV_SINT16:
        case ACD_SD_INV_UINT32:
        case ACD_SD_INV_SINT32:
            _invertStreamType = true;
            break;
        default:
            panic("ACDStreampImp", "set", "Undefined stream data format.");            
        
    }
        
    GLOBALPROFILER_EXITREGION()
}

void ACDStreamImp::get( ACDBuffer*& buffer,
                        acd_uint& offset,
                        acd_uint& components,
                        ACD_STREAM_DATA& componentsType,
                        acd_uint& stride,
                        acd_uint &frequency) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    buffer = _buffer;
    offset = _offset;
    components = _components;
    componentsType = _componentsType;
    stride = _stride;
    frequency = _frequency;
    GLOBALPROFILER_EXITREGION()
}

void ACDStreamImp::setBuffer(ACDBuffer* buffer) 
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACDBufferImp* buf = static_cast<ACDBufferImp*>(buffer);
    _buffer = buf;
    _gpuMemTrack = 0; // Init buffer track
    GLOBALPROFILER_EXITREGION()
}

void ACDStreamImp::setOffset(acd_uint offset)
{
    _offset = offset;
}

void ACDStreamImp::setComponents(acd_uint components)
{
_components = components;
}

void ACDStreamImp::setType(ACD_STREAM_DATA componentsType)
{
    _componentsType = componentsType;
}
    
void ACDStreamImp::setStride(acd_uint stride)
{
    _stride = stride;
}

void ACDStreamImp::setFrequency(acd_uint frequency)
{
    _frequency = frequency;
}

ACDBuffer* ACDStreamImp::getBuffer() const
{
    return _buffer;
}
    
acd_uint ACDStreamImp::getOffset() const
{
    return _offset;
}

acd_uint ACDStreamImp::getComponents() const
{
    return _components;
}
    
ACD_STREAM_DATA ACDStreamImp::getType() const
{
    return _componentsType;
}
    
acd_uint ACDStreamImp::getStride() const
{
    return _stride;
}

acd_uint ACDStreamImp::getFrequency() const
{
    return _frequency;
}

void ACDStreamImp::sync()
{
    gpu3d::GPURegData data;
    
    // Buffer synchronization is always required
    ACDBufferImp* buf = _buffer;

    if ( buf != 0 ) {
        _device->allocator().syncGPU(buf); // syncronize buffer contents

        //if ( _buffer.changed() || (_buffer.initial()->getUID()!=buf->getUID()) || _offset.changed() || _gpuMemTrack != buf->trackRealloc(0) || _requiredSync ) {
            // Stream address requires to be recomputed
            acd_uint md = _device->allocator().md(buf);
            _driver->writeGPUAddrRegister(gpu3d::GPU_STREAM_ADDRESS, _STREAM_ID, md, _offset);
            // reset IF conditions
            _buffer.restart();
            _offset.restart();
            _gpuMemTrack = buf->trackRealloc(0); // restart buffer tracking
        //}

        #ifdef ACD_DUMP_STREAMS
            ACDDumpStream();
        #endif
    }
  
    if ( _components.changed() || _requiredSync )
    {
        data.uintVal = _components;
        _driver->writeGPURegister(gpu3d::GPU_STREAM_ELEMENTS, _STREAM_ID, data);
        _components.restart();
    }

    if (_frequency.changed() || _requiredSync)
    {
        data.uintVal = _frequency;
        _driver->writeGPURegister(gpu3d::GPU_STREAM_FREQUENCY, _STREAM_ID, data);
        _frequency.restart();
    }
    
    if ( _componentsType.changed() || _requiredSync )
    {
        _convertToGPUStreamData(_componentsType, &data);
        _driver->writeGPURegister(gpu3d::GPU_STREAM_DATA, _STREAM_ID, data);
        _componentsType.restart();
    }

    if ( _invertStreamType.changed() || _requiredSync )
    {
        data.booleanVal = _invertStreamType;
        _driver->writeGPURegister(gpu3d::GPU_D3D9_COLOR_STREAM, _STREAM_ID, data);
        _invertStreamType.restart();
    }

    if ( _stride.changed() || _requiredSync )
    {
        data.uintVal = _stride;
        _driver->writeGPURegister(gpu3d::GPU_STREAM_STRIDE, _STREAM_ID, data);
        _stride.restart();
    }
}

const StoredStateItem* ACDStreamImp::createStoredStateItem(ACD_STORED_ITEM_ID stateId) const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    ACDStoredStateItem* ret;
    acd_uint rTarget;

    if (stateId >= ACD_STREAM_FIRST_ID && stateId < ACD_STREAM_LAST)
    {    
        // It큦 a blending state
        switch(stateId)
        {
            case ACD_STREAM_ELEMENTS:       ret = new ACDSingleEnumStoredStateItem((acd_enum)_components);          break;
            case ACD_STREAM_FREQUENCY:      ret = new ACDSingleEnumStoredStateItem((acd_enum) _frequency);          break;
            case ACD_STREAM_DATA_TYPE:      ret = new ACDSingleEnumStoredStateItem((acd_enum)_componentsType);      break;
            case ACD_STREAM_D3D9_COLOR:     ret = new ACDSingleEnumStoredStateItem((acd_enum)_invertStreamType);    break;
            case ACD_STREAM_STRIDE:         ret = new ACDSingleEnumStoredStateItem((acd_enum)_stride);              break;
            case ACD_STREAM_BUFFER:         ret = new ACDSingleVoidStoredStateItem(_buffer);                        break;
            // case ACD_STREAM_... <- add here future additional blending states.

            default:
                panic("ACDStreamImp","createStoredStateItem","Unexpected blending state id");
        }          
        // else if (... <- write here for future additional defined blending states.
    }
    else
        panic("ACDStreamImp","createStoredStateItem","Unexpected blending state id");

    ret->setItemId(stateId);

    GLOBALPROFILER_EXITREGION()

    return ret;
}

#define CAST_TO_UINT(X)         *(static_cast<const ACDSingleUintStoredStateItem*>(X))
#define CAST_TO_BOOL(X)         *(static_cast<const ACDSingleBoolStoredStateItem*>(X))
#define CAST_TO_VOID(X)         static_cast<void *>(*(static_cast<const ACDSingleVoidStoredStateItem*> (X)))
#define STREAM_DATA(DST,X)      { const ACDSingleEnumStoredStateItem* aux = static_cast<const ACDSingleEnumStoredStateItem*>(X); acd_enum value = *aux; DST = static_cast<ACD_STREAM_DATA>(value); }

void ACDStreamImp::restoreStoredStateItem(const StoredStateItem* ssi)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")
    acd_uint rTarget;

    const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(ssi);

    ACD_STORED_ITEM_ID stateId = acdssi->getItemId();

    if (stateId >= ACD_STREAM_FIRST_ID && stateId < ACD_STREAM_LAST)
    {    
        // It큦 a blending state
        switch(stateId)
        {
            case ACD_STREAM_ELEMENTS:       _components = CAST_TO_UINT(acdssi);                             break;
            case ACD_STREAM_FREQUENCY:      _frequency = CAST_TO_UINT(acdssi);                              break;
            case ACD_STREAM_DATA_TYPE:      STREAM_DATA(_componentsType, acdssi);                           break;
            case ACD_STREAM_D3D9_COLOR:     _invertStreamType = CAST_TO_BOOL(acdssi);                       break;
            case ACD_STREAM_STRIDE:         _stride = CAST_TO_UINT(acdssi);                                 break;
            case ACD_STREAM_BUFFER:         _buffer = static_cast<ACDBufferImp*>(CAST_TO_VOID(acdssi));     break;

            // case ACD_STREAM_... <- add here future additional blending states.
            default:
                panic("ACDStreamImp","createStoredStateItem","Unexpected blending state id");
        }           
        // else if (... <- write here for future additional defined blending states.
    }
    else
        panic("ACDStreamImp","restoreStoredStateItem","Unexpected blending state id");

    GLOBALPROFILER_EXITREGION()
}

#undef CAST_TO_UINT
#undef CAST_TO_BOOL
#undef CAST_TO_VOID
#undef STREAM_DATA

ACDStoredState* ACDStreamImp::saveAllState() const
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    
    ACDStoredStateImp* ret = new ACDStoredStateImp();

    for (acd_uint i = 0; i < ACD_LAST_STATE; i++)
    {
        if (i < ACD_LAST_STATE)
        {
            // It큦 a rasterization stage
            ret->addStoredStateItem(createStoredStateItem(ACD_STORED_ITEM_ID(i)));
        }
        else
            panic("ACDStreamImp","saveState","Unexpected Stored Item Id");
    }

    GLOBALPROFILER_EXITREGION()
    
    return ret;
}

void ACDStreamImp::restoreAllState(const ACDStoredState* state)
{
    GLOBALPROFILER_ENTERREGION("ACD", "", "")    

    const ACDStoredStateImp* ssi = static_cast<const ACDStoredStateImp*>(state);

    std::list<const StoredStateItem*> ssiList = ssi->getSSIList();

    std::list<const StoredStateItem*>::const_iterator iter = ssiList.begin();

    while ( iter != ssiList.end() )
    {
        const ACDStoredStateItem* acdssi = static_cast<const ACDStoredStateItem*>(*iter);

        if (acdssi->getItemId() < ACD_LAST_STATE)
        {
            // It큦 a blending stage
            restoreStoredStateItem(acdssi);
        }
        else
            panic("ACDStreamImp","restoreState","Unexpected Stored Item Id");

        iter++;
    }

    GLOBALPROFILER_EXITREGION()
}

void ACDStreamImp::ACDDumpStream()
{

    ofstream out;
    
    out.open("ACDDumpStream.txt",ios::app);

    if ( !out.is_open() )
        panic("ACDDumpStream", "ACDStreamImp", "Dump failed (output file could not be opened)");


    out << " Dumping Stream " << _STREAM_ID << endl;
    out << "\t Stride: " << _stride << endl;
    out << "\t Data type: " << _componentsType << endl;
    out << "\t Num. elemets: " << _components << endl;
    out << "\t Frequency: " << _frequency << endl;
    out << "\t Offset: " << _offset << endl;
    out << "\t Dumping stream content: " << endl;

    ((ACDBufferImp*)_buffer)->dumpBuffer(out, _componentsType, _components, _stride, _offset);

    out.close();

    

}
string ACDStreamImp::DBG_getState() const
{
    return getInternalState();
}

void ACDStreamImp::_convertToGPUStreamData(ACD_STREAM_DATA componentsType, gpu3d::GPURegData* data)
{
    switch ( componentsType )
    {
        case ACD_SD_UNORM8:
        case ACD_SD_INV_UNORM8:
            data->streamData = gpu3d::SD_UNORM8;
            break;
        case ACD_SD_SNORM8:
        case ACD_SD_INV_SNORM8:
            data->streamData = gpu3d::SD_SNORM8;
            break;
        case ACD_SD_UNORM16:
        case ACD_SD_INV_UNORM16:
            data->streamData = gpu3d::SD_UNORM16;
            break;
        case ACD_SD_SNORM16:
        case ACD_SD_INV_SNORM16:
            data->streamData = gpu3d::SD_SNORM16;
            break;
        case ACD_SD_UNORM32:
        case ACD_SD_INV_UNORM32:
            data->streamData = gpu3d::SD_UNORM32;
            break;
        case ACD_SD_SNORM32:
        case ACD_SD_INV_SNORM32:
            data->streamData = gpu3d::SD_SNORM32;
            break;
        case ACD_SD_FLOAT16:
        case ACD_SD_INV_FLOAT16:
            data->streamData = gpu3d::SD_FLOAT16;
            break;
        case ACD_SD_FLOAT32:
        case ACD_SD_INV_FLOAT32:
            data->streamData = gpu3d::SD_FLOAT32;
            break;
        case ACD_SD_UINT8:
        case ACD_SD_INV_UINT8:
            data->streamData = gpu3d::SD_UINT8;
            break;
        case ACD_SD_SINT8:
        case ACD_SD_INV_SINT8:
            data->streamData = gpu3d::SD_SINT8;
            break;
        case ACD_SD_UINT16:
        case ACD_SD_INV_UINT16:
            data->streamData = gpu3d::SD_UINT16;
            break;
        case ACD_SD_SINT16:
        case ACD_SD_INV_SINT16:
            data->streamData = gpu3d::SD_SINT16;
            break;
        case ACD_SD_UINT32:
        case ACD_SD_INV_UINT32:
            data->streamData = gpu3d::SD_UINT32;
            break;
        case ACD_SD_SINT32:
        case ACD_SD_INV_SINT32:
            data->streamData = gpu3d::SD_SINT32;
            break;                        
        default:
            panic("ACDStreamImp", "_convertToGPUStreamData", "Unknown ACD_STREAM_DATA value");
    }
}

void ACDStreamImp::forceSync()
{
    _requiredSync = true;
    sync();
    _requiredSync = false;
}

string ACDStreamImp::getInternalState() const
{
    stringstream ss, ssAux;

    ssAux << "STREAM" << _STREAM_ID;

    ss << ssAux.str() << "_GPU_MEM_TRACK = " << _gpuMemTrack << "\n";
    ss << ssAux.str() << "_BUFFER = " << hex << static_cast<ACDBufferImp*>(_buffer) << dec;
    
    if ( _requiredSync )
        ss << " NotSync (?)\n";
    else {
        if ( _buffer.changed() )
            ss << " NotSync (" << hex << static_cast<ACDBufferImp*>(_buffer.initial()) << dec << ")\n";
        else
            ss << " Sync\n";
    }

    ss << ssAux.str() << stateItemString(_offset,"_OFFSET",false);

    ss << ssAux.str() << stateItemString(_components,"_COMPONENTS",false);

    ss << ssAux.str() << stateItemString(_componentsType,"_COMPONENTS_TYPE",false,&componentTypePrint);
    
    ss << ssAux.str() << stateItemString(_stride,"_STRIDE",false);

    ss << ssAux.str() << stateItemString(_frequency,"_FREQUENCY",false);

    return ss.str();
}

const char* ACDStreamImp::ComponentTypePrint::print(const ACD_STREAM_DATA& var) const
{
    switch ( var ) 
    {
        case ACD_SD_UNORM8:
            return "SD_UNORM8"; break;
        case ACD_SD_SNORM8:
            return "SD_SNORM8"; break;
        case ACD_SD_UNORM16:
            return "SD_UNORM16"; break;
        case ACD_SD_SNORM16:
            return "SD_SNORM16"; break;
        case ACD_SD_UNORM32:
            return "SD_UNORM32"; break;
        case ACD_SD_SNORM32:
            return "SD_SNORM32"; break;
        case ACD_SD_FLOAT16:
            return "SD_FLOAT16"; break;
        case ACD_SD_FLOAT32:
            return "SD_FLOAT32"; break;
        case ACD_SD_UINT8:
            return "SD_UINT8"; break;
        case ACD_SD_SINT8:
            return "SD_SINT8"; break;
        case ACD_SD_UINT16:
            return "SD_UINT16"; break;
        case ACD_SD_SINT16:
            return "SD_SINT16"; break;
        case ACD_SD_UINT32:
            return "SD_UINT32"; break;
        case ACD_SD_SINT32:
            return "SD_SINT32"; break;
                                                
        case ACD_SD_INV_UNORM8:
            return "SD_INV_UNORM8"; break;
        case ACD_SD_INV_SNORM8:
            return "SD_INV_SNORM8"; break;
        case ACD_SD_INV_UNORM16:
            return "SD_INV_UNORM16"; break;
        case ACD_SD_INV_SNORM16:
            return "SD_INV_SNORM16"; break;
        case ACD_SD_INV_UNORM32:
            return "SD_INV_UNORM32"; break;
        case ACD_SD_INV_SNORM32:
            return "SD_INV_SNORM32"; break;
        case ACD_SD_INV_FLOAT16:
            return "SD_INV_FLOAT16"; break;
        case ACD_SD_INV_FLOAT32:
            return "SD_INV_FLOAT16"; break;
        case ACD_SD_INV_UINT8:
            return "SD_INV_UINT8"; break;
        case ACD_SD_INV_SINT8:
            return "SD_INV_SINT8"; break;
        case ACD_SD_INV_UINT16:
            return "SD_INV_UINT16"; break;
        case ACD_SD_INV_SINT16:
            return "SD_INV_SINT16"; break;
        case ACD_SD_INV_UINT32:
            return "SD_INV_UINT32"; break;
        case ACD_SD_INV_SINT32:
            return "SD_INV_SINT32"; break;

        default:
            return "UNKNOWN";
    }
}

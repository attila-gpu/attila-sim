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

#include "ACDBufferImp.h"
#include "ACDMacros.h"
#include "support.h"
#include <iostream>
#include <fstream>
#include <cstring>

using namespace std;
using namespace acdlib;

const acd_float ACDBufferImp::GROWTH_FACTOR = 1.10f;
acd_uint ACDBufferImp::_nextUID = 0;

#define BURST_SIZE 32

ACDBufferImp::ACDBufferImp(acd_uint size, const acd_ubyte* data) : _UID(_nextUID)
{
    _nextUID++;

    // Start buffer memory tracking
    defineRegion(0);

    if ( size == 0 ) {
        ACD_ASSERT(
            if ( data != 0 )
                panic("ACDBufferImp", "ctor", "Data pointer NOT null with size == 0");
        )
        _data = 0;
        _size = 0;
        _capacity = 0;
    }
    else {
        _size = _capacity = size;
        _data = new acd_ubyte[size];

        if ( data != 0 )
            memcpy(_data, data, size);

        // Notify reallocate requeriment
        postReallocate(0);
    }
}

ACDBufferImp::~ACDBufferImp () {
    delete[] _data;
}

void ACDBufferImp::setUsage(ACD_USAGE usage)
{
    ACD_ASSERT(
        if ( _data )
            panic("ACDBufferImp", "setUsage", "Buffer with defined contents cannot redefine its usage");
    )

    _usage = usage;

    if (usage == ACD_USAGE_STATIC)
        setPreferredMemory(MemoryObject::MT_LocalMemory);
    else
        setPreferredMemory(MemoryObject::MT_SystemMemory);
}

ACD_USAGE ACDBufferImp::getUsage() const
{
    return _usage;
}

void ACDBufferImp::setMemoryLayout(ACD_MEMORY_LAYOUT _layout)
{
    layout = _layout;
}

ACD_MEMORY_LAYOUT ACDBufferImp::getMemoryLayout() const
{
    return layout;
}

ACD_RESOURCE_TYPE ACDBufferImp::getType() const
{
    return ACD_RESOURCE_BUFFER;
}

void ACDBufferImp::setPriority(acd_uint prio)
{
    cout << "ACDBufferImp::setPriority() -> Not implemented yet" << endl;
}

acd_uint ACDBufferImp::getPriority() const
{
    cout << "CDBufferImp::getPriority() -> Not implemented yet" << endl;
    return 0;
}

acd_uint ACDBufferImp::getSubresources() const
{
    return 1;
}

acd_bool ACDBufferImp::wellDefined() const
{
    return (_data != 0 && _size != 0 );
}

const acd_ubyte* ACDBufferImp::getData() const
{
    return _data;
}

acd_uint ACDBufferImp::getSize() const
{
    return _size;
}

void ACDBufferImp::resize(acd_uint size, bool discard)
{
    acd_uint originalCapacity = _capacity;

    if ( size > _capacity ) {
        acd_ubyte* chunk = new acd_ubyte[size];
        if ( !discard )
            memcpy(chunk, _data, _size);
        delete[] _data;
        _data = chunk;
        _capacity = _size = size; // updates new capacity and size
    }
    else // size <= _capacity
        _size = size;

    // Reallocation required in GPU memory
    if ( discard )
        postReallocate(0);
    else
        postUpdate(0, originalCapacity, size);
}

void ACDBufferImp::clear()
{
    // Implementando clear de los buffers para soportard glacd::discarBuffers()
    delete[] _data;

    _size = 0;
    _capacity = 0;
    _data = 0;

    // Next synchronization will release previous buffer contents on GPU memory or system memory
    postReallocate(0);
}

void ACDBufferImp::pushData(const acd_void* data, acd_uint size)
{
    if ( _size + size > _capacity ) { // buffer growth is required
        if ( _size + size > static_cast<acd_uint>(_size * GROWTH_FACTOR) )
            _capacity = _size + size;
        else
            _capacity = static_cast<acd_uint>(_size * GROWTH_FACTOR);
        // growth is required
        acd_ubyte* tmp = new acd_ubyte[_capacity];
        memcpy(tmp, _data, _size); // Copy previous contents
        delete[] _data; // Delete old contents
        _data = tmp; // reassign data pointer
    }

    // Push new data at the end of the buffer
    memcpy(_data + _size, data, size);
    _size += size;

            
    postReallocate(0);
}

void ACDBufferImp::updateData( acd_uint offset, acd_uint size, const acd_ubyte* data, acd_bool preloadData)
{
    ACD_ASSERT
    (
        if ( offset + size > _size )
            panic("ACDBufferImp", "updateData", "Buffer overflow offset + size is greater than total buffer size");
    )

    memcpy(_data + offset, data, size); 

    
    if ( (offset % BURST_SIZE) != 0) // Check if buffer realloc is properly aligned
    {
        size = size + (offset % BURST_SIZE);
        offset = offset - (offset % BURST_SIZE);
    }
    
    // Mark the memory range as dirty
    postUpdate(0, offset, offset + size - 1);
    preload(0, preloadData);
}

const acd_ubyte* ACDBufferImp::memoryData(acd_uint region, acd_uint& memorySizeInBytes) const
{
    // Buffers are placed equally in CPU and GPU memory (linear), so no translation is required

    ACD_ASSERT(
        if ( _data == 0 || _size == 0 )
            panic("ACDBufferImp", "memoryData", "Memory data or size are NULL");
    )

    memorySizeInBytes = _size;
    return _data;
}

const acd_char* ACDBufferImp::stringType() const
{
    return "MEMORY_OBJECT_BUFFER";
}

void ACDBufferImp::shrink()
{
    // Shrink does not require to POST any message, it has only CPU effects (not GPU)

    if ( _size == _capacity )
        return ;
    acd_ubyte* chunk = new acd_ubyte[_size];
    memcpy(chunk, _data, _size);
    delete[] _data;
    _data = chunk;
}

acd_uint ACDBufferImp::capacity() const
{
    return _capacity;
}


acd_uint ACDBufferImp::getUID ()
{
    return _UID;
}

void ACDBufferImp::dumpBuffer (ofstream& out, ACD_STREAM_DATA type, acd_uint components, acd_uint stride, acd_uint offset)
{
    acd_uint numElem = _size/getSize(type);
    acd_uint i = offset/getSize(type);
    acd_uint strideElem = stride/getSize(type);

    out << "{";
    switch ( type )
    {
        case ACD_SD_UNORM8:
        case ACD_SD_SNORM8:
        case ACD_SD_UINT8:
        case ACD_SD_SINT8:
        case ACD_SD_INV_UNORM8:
        case ACD_SD_INV_SNORM8:
        case ACD_SD_INV_UINT8:
        case ACD_SD_INV_SINT8:
            for ( ; i < numElem; )
            {
                out << "(";
                for (int j = 0; j < components; j++)
                {
                    if (j != 0) out << ",";

                    out << ((acd_ubyte*)_data)[i];

                    i++;
                }

                if (strideElem != 0)
                    i = i + (strideElem - components);

                out << ")";

                if(i<numElem) out << ",";

            }
        case ACD_SD_UNORM16:
        case ACD_SD_SNORM16:
        case ACD_SD_UINT16:
        case ACD_SD_SINT16:
        case ACD_SD_FLOAT16:
        case ACD_SD_INV_UNORM16:
        case ACD_SD_INV_SNORM16:
        case ACD_SD_INV_UINT16:
        case ACD_SD_INV_SINT16:
        case ACD_SD_INV_FLOAT16:
            for ( ; i < numElem; )
            {
                out << "(";
                for (int j = 0; j < components; j++)
                {
                    if (j != 0) out << ",";

                    out << ((acd_ushort*)_data)[i];

                    i++;
                }

                if (strideElem != 0)
                    i = i + (strideElem - components);

                out << ")";

                if(i<numElem) out << ",";

            }
        case ACD_SD_UNORM32:
        case ACD_SD_SNORM32:
        case ACD_SD_UINT32:
        case ACD_SD_SINT32:
        case ACD_SD_INV_UNORM32:
        case ACD_SD_INV_SNORM32:
        case ACD_SD_INV_UINT32:
        case ACD_SD_INV_SINT32:
            for ( ; i < numElem; )
            {
                out << "(";
                for (int j = 0; j < components; j++)
                {
                    if (j != 0) out << ",";

                    out << ((acd_uint*)_data)[i];

                    i++;
                }

                if (strideElem != 0)
                    i = i + (strideElem - components);

                out << ")";

                if(i<numElem) out << ",";

            }
        case ACD_SD_FLOAT32:
        case ACD_SD_INV_FLOAT32:
            for ( ; i < numElem; )
            {
                out << "(";
                for (int j = 0; j < components; j++)
                {
                    if (j != 0) out << ",";

                    out << ((acd_float*)_data)[i];

                    i++;
                }

                if (strideElem != 0)
                    i = i + (strideElem - components);

                out << ")";

                if(i<numElem) out << ",";

            }
            break;
        default:
            panic("GLContext", "rawIndices", "Unsupported indices type");
    }
    out << "}" << endl;

}

acd_uint ACDBufferImp::getSize (ACD_STREAM_DATA type)
{
    switch ( type )
    {
        case ACD_SD_UNORM8:
        case ACD_SD_SNORM8:
        case ACD_SD_UINT8:
        case ACD_SD_SINT8:
        case ACD_SD_INV_UNORM8:
        case ACD_SD_INV_SNORM8:
        case ACD_SD_INV_UINT8:
        case ACD_SD_INV_SINT8:
            return sizeof(acd_ubyte);
            break;

        case ACD_SD_UNORM16:
        case ACD_SD_SNORM16:
        case ACD_SD_UINT16:
        case ACD_SD_SINT16:
        case ACD_SD_FLOAT16:
        case ACD_SD_INV_UNORM16:
        case ACD_SD_INV_SNORM16:
        case ACD_SD_INV_UINT16:
        case ACD_SD_INV_SINT16:
        case ACD_SD_INV_FLOAT16:
            return sizeof(acd_ushort);
            break;

        case ACD_SD_UNORM32:
        case ACD_SD_SNORM32:
        case ACD_SD_UINT32:
        case ACD_SD_SINT32:
        case ACD_SD_INV_UNORM32:
        case ACD_SD_INV_SNORM32:
        case ACD_SD_INV_UINT32:
        case ACD_SD_INV_SINT32:
            return sizeof(acd_uint);
            break;

        case ACD_SD_FLOAT32:
        case ACD_SD_INV_FLOAT32:
            return sizeof(acd_float);
            break;

        default:
            panic("ACDBufferImp", "getSize", "Type size not available");
            return 0;  // To remove compiler warning;
            break;
    }
}

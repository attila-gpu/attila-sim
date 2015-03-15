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

#ifndef ACD_BUFFER_IMP
    #define ACD_BUFFER_IMP

#include "ACDBuffer.h"
#include "MemoryObject.h"
#include "ACDStream.h"
#include <iostream>
#include <fstream>

using namespace std;
namespace acdlib
{

/**
 * ACDBuffer interface is the common interface to declare API independent data buffers.
 * API-dependant vertex and index buffers are mapped onto this interface
 *
 * This objects are created from ResourceManager factory
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @version 1.0
 * @date 01/19/2007
 */
class ACDBufferImp : public ACDBuffer, public MemoryObject
{
public:

    ACDBufferImp(acd_uint size, const acd_ubyte* data);
    ~ACDBufferImp ();

    void dumpBuffer (ofstream& out, ACD_STREAM_DATA type, acd_uint components, acd_uint stride, acd_uint offset);


    acd_uint getSize (ACD_STREAM_DATA type);

    ///////////////////////////////////////////////////////
    /// ACDResource methods that required implemenation ///
    ///////////////////////////////////////////////////////

    virtual void setUsage(ACD_USAGE usage);

    virtual ACD_USAGE getUsage() const;

    virtual ACD_RESOURCE_TYPE getType() const;

    virtual void setMemoryLayout(ACD_MEMORY_LAYOUT layout);
    
    virtual ACD_MEMORY_LAYOUT getMemoryLayout() const;

    virtual void setPriority(acd_uint prio);

    virtual acd_uint getPriority() const;

    virtual acd_uint getSubresources() const;

    virtual acd_bool wellDefined() const;

    /////////////////////////////////////////////////////////
    /// ACDBuffer methods that require implementationfrom ///
    /////////////////////////////////////////////////////////

    virtual const acd_ubyte* getData() const;

    virtual acd_uint getSize() const;

    virtual void resize(acd_uint size, bool discard);

    virtual void clear();

    virtual void pushData(const acd_void* data, acd_uint size);

    virtual void updateData( acd_uint offset, acd_uint size, const acd_ubyte* data, acd_bool preload = false);
 
    ////////////////////////////////////////////////////////
    /// MemoryObject methods that require implementation ///
    ////////////////////////////////////////////////////////
    virtual const acd_ubyte* memoryData(acd_uint region, acd_uint& memorySizeInBytes) const;

    virtual const acd_char* stringType() const;


    ///////////////////////////////////////////////////
    /// Methods only available for ACD implementors ///
    ///////////////////////////////////////////////////

    /**
     * Reallocates the buffer to use the minimum required space
     */
    virtual void shrink();

    /**
     * Gets the capacity of the buffer to grow without reallocate new memory
     */
    virtual acd_uint capacity() const;

    virtual acd_uint getUID ();

private:

    static const acd_float GROWTH_FACTOR;
    static acd_uint _nextUID; // Debug info

    const acd_uint _UID;

    acd_uint _size;
    acd_uint _capacity;
    acd_ubyte* _data;
    ACD_USAGE _usage;

    ACD_MEMORY_LAYOUT layout;
};

} // namespace acdlib

#endif // ACD_BUFFER_IMP

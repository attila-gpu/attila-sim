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

#ifndef ACD_STREAM
    #define ACD_STREAM

#include "ACDTypes.h"
#include "ACDBuffer.h"
#include <string>

namespace acdlib
{

/**
 * Supported components data types in Attila (other type could be supported in future versions)
 */
enum ACD_STREAM_DATA
{
    ACD_SD_UNORM8,
    ACD_SD_SNORM8,
    ACD_SD_UNORM16,
    ACD_SD_SNORM16,
    ACD_SD_UNORM32,
    ACD_SD_SNORM32,
    ACD_SD_FLOAT16,
    ACD_SD_FLOAT32,
    ACD_SD_UINT8,
    ACD_SD_SINT8,
    ACD_SD_UINT16,
    ACD_SD_SINT16,
    ACD_SD_UINT32,
    ACD_SD_SINT32,
    
    ACD_SD_INV_UNORM8,
    ACD_SD_INV_SNORM8,
    ACD_SD_INV_UNORM16,
    ACD_SD_INV_SNORM16,
    ACD_SD_INV_UNORM32,
    ACD_SD_INV_SNORM32,
    ACD_SD_INV_FLOAT16,
    ACD_SD_INV_FLOAT32,
    ACD_SD_INV_UINT8,
    ACD_SD_INV_SINT8,
    ACD_SD_INV_UINT16,
    ACD_SD_INV_SINT16,
    ACD_SD_INV_UINT32,
    ACD_SD_INV_SINT32
};

struct ACD_STREAM_DESC
{
    acd_uint offset;
    acd_uint components;
    ACD_STREAM_DATA type;
    acd_uint stride;
    acd_uint frequency;
};

/**
 * This interface represents an Atila data stream processed by the Atila Streamer unit
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @version 1.0
 * @date 01/22/2007
 */
class ACDStream
{
public:

    /**
     * Sets all the parameters of the stream
     *
     * Sets all the parametersn of the stream
     *
     * @param buffer The data buffer used to feed this stream
     * @param desc Descriptor with 
     */
    virtual void set( ACDBuffer* buffer, const ACD_STREAM_DESC& desc ) = 0;

    /**
     * Gets all the parameters of the stream
     *
     * @retval buffer the buffer used to feed this stream
     * @retval offset Offset from the beginning og the data buffer (in bytes)
     * @retval components The number of components of each item in the stream (in the buffer)
     * @retval componentsType Type of the components
     * @retval stride Stride in bytes between two consecutive items in the data buffer
     * @retval frequency Frenquency of the stream: 0-> per vertex/index, >0 -> per n instances.
     */
    virtual void get( ACDBuffer*& buffer,
                      acd_uint& offset,
                      acd_uint& components,
                      ACD_STREAM_DATA& componentsType,
                      acd_uint& stride,
                      acd_uint &frequency) const = 0;

    /**
     * Sets the buffer used to feed this stream
     *
     * @param buffer the buffer attached to this stream
     */
    virtual void setBuffer(ACDBuffer* buffer) = 0;

    /**
     * Sets an offset in the data buffer
     *
     * @param offset Offset from the beginning of the data buffer (in bytes)
     */
    virtual void setOffset(acd_uint offset) = 0;

    /**
     * Sets the number of components per item
     *
     * @param components The number of components of each item in the stream (in the buffer)
     */
    virtual void setComponents(acd_uint components) = 0;


    virtual void setType(ACD_STREAM_DATA componentsType) = 0;   
    virtual void setStride(acd_uint stride) = 0;
    
    virtual void setFrequency(acd_uint frequency) = 0;

    virtual ACDBuffer* getBuffer() const = 0;
    virtual acd_uint getOffset() const = 0;
    virtual acd_uint getComponents() const = 0;
    virtual ACD_STREAM_DATA getType() const = 0;
    virtual acd_uint getStride() const = 0;
    virtual acd_uint getFrequency() const = 0;

    virtual std::string DBG_getState() const = 0;

};

}

#endif // ACD_STREAM

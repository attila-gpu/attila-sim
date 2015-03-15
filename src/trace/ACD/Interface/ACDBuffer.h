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

#ifndef ACD_BUFFER
    #define ACD_BUFFER

#include "ACDTypes.h"
#include "ACDResource.h"

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
class ACDBuffer : public ACDResource
{
public:

    /**
     * Retrieves a pointer to the internal data of the ACDBuffer object
     *
     * @returns the pointer to the internal data (read-only) of the ACDBufferData object
     */
    virtual const acd_ubyte* getData() const = 0;

    /**
     * Gets the size in bytes of the internal vertex buffer data
     *
     * @returns The vertex buffer size in bytes
     */
    virtual acd_uint getSize() const = 0;

    /**
     * Sets a new data buffer size
     *
     * @param size New data buffer size in bytes
     * @param discard If true, discards previous data. If false, the buffer is resized maintaing previous data
     */
    virtual void resize(acd_uint size, bool discard) = 0;

    /**
     * Discard previous buffer contents
     */
    virtual void clear() = 0;

    /**
     * Adds data to the end of the current accesible data (the buffer will grow accordingly)
     */
    virtual void pushData(const acd_void* data, acd_uint size) = 0;

    /**
     * Updates the buffer or a specified range of the data buffer
     *
     * @param offset offset in the buffer (where to start updating)
     * @param size amount of data in the supplied pointer
     * @param data pointer to the data to update the buffer
     * @param preload boolean flag that defines if the data is to be preloaded into GPU memory (no cycles
     * spent on simulation).
     *
     */
    virtual void updateData( acd_uint offset, acd_uint size, const acd_ubyte* data, acd_bool preload = false ) = 0;

};

} // namespace acdlib

#endif // ACD_BUFFER

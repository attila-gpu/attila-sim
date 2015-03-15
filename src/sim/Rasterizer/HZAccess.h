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
 * $RCSfile: HZAccess.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:53 $
 *
 * Hierarchical Z Buffer Access definition file.
 *
 */

/**
 *
 *  @file HZAccess.h
 *
 *  This file defines the HZAccess class.
 *
 *  The Hierarchical Z Access class is used to simulate
 *  the latency of access operations in the Hierarchical
 *  Z Buffer.
 *
 */

#ifndef _HZACCESS_
#define _HZACCESS_

#include "DynamicObject.h"

namespace gpu3d
{

/**
 *
 *  Defines the Hierarchical Z buffer access operations.
 *
 */
enum HZOperation
{
    HZ_WRITE,   /**<  A write operation over the Hierarchical Z Buffer.  */
    HZ_READ     /**<  A read operation over the Hierarchical Z Buffer.  */
};


/**
 *
 *  This class is used to simulate the operation latency of
 *  accesses to the Hierarchical Z Buffer.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class HZAccess : public DynamicObject
{
private:

    HZOperation operation;  /**<  The operation to be performed over the HZ Buffer.  */
    u32bit address;         /**<  The HZ buffer block to be accessed.  */
    u32bit data;            /**<  Data to write over the HZ Buffer.  */
    u32bit cacheEntry;      /**<  The HZ Cache entry where to store the HZ Buffer data.  */

public:

    /**
     *
     *  Creates a new HZAccess object.
     *
     *  @param op Operation to perform.
     *  @param address The block address to be updated.
     *  @param data The z to write into the block Z (for
     *  HZ_WRITE) or the cache entry where to store the HZ
     *  block data (HZ_READ).
     *
     *  @return A new initialized HZAccess object.
     *
     */

    HZAccess(HZOperation op, u32bit address, u32bit data);

    /**
     *
     *  Returns the access operation to perform.
     *
     *  @return Access operation.
     *
     */

    HZOperation getOperation();

    /**
     *
     *  Returns the address of the HZ block to be accessed.
     *
     *  @return The block address of the HZ buffer block to
     *  be accessed.
     *
     */

    u32bit getAddress();

    /**
     *
     *  Returns the new z for the HZ buffer block to be written.
     *
     *  @return The new Z for the block to be written.
     *
     */

    u32bit getData();

    /**
     *
     *  Returns the HZ Cache entry where to store the read HZ
     *  block Z.
     *
     *  @param HZ Cache entry where to perform the read access
     *  to the HZ buffer.
     *
     */

    u32bit getCacheEntry();

};

} // namespace gpu3d

#endif

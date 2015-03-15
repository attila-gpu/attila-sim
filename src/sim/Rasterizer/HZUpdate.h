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
 * $RCSfile: HZUpdate.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:54 $
 *
 * Hierarchical Z Update definition file.
 *
 */

/**
 *
 *  @file HZUpdate.h
 *
 *  This file defines the HZUpdate class.
 *
 *  The Hieararchical Z Update class is used to block updates
 *  from Z Test to the Hierarchical Z buffer.
 *
 */

#ifndef _HZUPDATE_
#define _HZUPDATE_

#include "DynamicObject.h"

namespace gpu3d
{

/**
 *
 *  This class defines a container for the block updates
 *  that the Z Test box sends to the Hierarchical Z buffer.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class HZUpdate : public DynamicObject
{
private:

    u32bit blockAddress;    /**<  The HZ buffer block to be updated.  */
    u32bit blockZ;          /**<  The new z for the HZ buffer block to be updated.  */

public:

    /**
     *
     *  Creates a new HZUpdate object.
     *
     *  @param address The block address to be updated.
     *  @param z The block Z value.
     *
     *  @return A new initialized HZUpdate object.
     *
     */

    HZUpdate(u32bit address, u32bit z);

    /**
     *
     *  Returns the address of the HZ block to be updated.
     *
     *  @return The block address of the HZ buffer block to
     *  be updated.
     *
     */

    u32bit getBlockAddress();

    /**
     *
     *  Returns the new z for the HZ buffer block to be updated.
     *
     *  @return The new Z for the block to be updated.
     *
     */

    u32bit getBlockZ();

};

} // namespace gpu3d

#endif

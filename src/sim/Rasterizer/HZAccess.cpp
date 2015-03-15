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
 * $RCSfile: HZAccess.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:28 $
 *
 * Hierarchical Z Access implementation file.
 *
 */

/**
 *
 *  @file HZAccess.cpp
 *
 *  This file implements the HZAccess class.
 *
 *  The HZAccess class is used to simulate the access
 *  over the Hierarchical Z Buffer.
 *
 */


#include "HZAccess.h"

using namespace gpu3d;

/*  Creates a new HZAccess object.  */
HZAccess::HZAccess(HZOperation op, u32bit addr, u32bit d) :
operation(op), address(addr)
{
    /*  Select access operation.  */
    switch(operation)
    {
        case HZ_READ:

            cacheEntry = d;

            break;

        case HZ_WRITE:

            data = d;

            break;

        default:
            panic("HZAccess", "HZAccess", "Unsupported access operation.");
            break;
    }

    setTag("HZAcc");
}

/*  Returns the address of the HZ Buffer block to be accessed.  */
u32bit HZAccess::getAddress()
{
    return address;
}

/*  Returns the Z for HZ Buffer block to be written.  */
u32bit HZAccess::getData()
{
    GPU_ASSERT(
        if (operation != HZ_WRITE)
            panic("HZAccess", "getData", "Not a write operation.");
    )

    return data;
}

/*  Returns the HZ Cache entry where to perform the read access to the HZ Buffer.  */
u32bit HZAccess::getCacheEntry()
{
    GPU_ASSERT(
        if (operation != HZ_READ)
            panic("HZAccess", "getCacheEntry", "Not a read operation.");
    )

    return cacheEntry;
}

/*  Returns the access operation to perform.  */
HZOperation HZAccess::getOperation()
{
    return operation;
}

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
 * $RCSfile: HZUpdate.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:29 $
 *
 * Hierarchical Z Update implementation file.
 *
 */

/**
 *
 *  @file HZUpdate.cpp
 *
 *  This file implements the HZUpdate class.
 *
 *  The HZUpdate class carries block updates from Z Test
 *  to the Hierarchical Z buffer.
 *
 */


#include "HZUpdate.h"

using namespace gpu3d;



/*  Creates a new HZUpdate object.  */
HZUpdate::HZUpdate(u32bit address, u32bit z) :
blockAddress(address), blockZ(z)
{
    setTag("HZUpd");
}

/*  Returns the address of the HZ Buffer block to be updated.  */
u32bit HZUpdate::getBlockAddress()
{
    return blockAddress;
}

/*  Returns the new z for the HZ Buffer block to be updated.  */
u32bit HZUpdate::getBlockZ()
{
    return blockZ;
}

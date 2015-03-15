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
 * $RCSfile: HZStateInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:28 $
 *
 * Hierarchical Z State Info implementation file.
 *
 */

/**
 *
 *  @file HZStateInfo.cpp
 *
 *  This file implements the HZStateInfo class.
 *
 *  The HZStateInfo class carries state information
 *  between Hierarchical Z early test and Triangle Traversal.
 *
 */


#include "HZStateInfo.h"

using namespace gpu3d;


/*  Creates a new HZStateInfo object.  */
HZStateInfo::HZStateInfo(HZState newState) :

    state(newState)
{
    /*  Set color for tracing.  */
    setColor(state);

    setTag("HZStIn");
}


/*  Returns the Hierarchical Z early test state carried by the object.  */
HZState HZStateInfo::getState()
{
    return state;
}

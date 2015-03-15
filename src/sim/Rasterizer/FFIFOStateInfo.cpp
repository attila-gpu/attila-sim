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
 * $RCSfile: FFIFOStateInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:27 $
 *
 * Fragment FIFO State Info implementation file.
 *
 */

/**
 *
 *  @file FFIFOStateInfo.cpp
 *
 *  This file implements the FFIFOStateInfo class.
 *
 *  The FFIFOStateInfo class carries state information from Fragment FIFO to Hierarchical/Early Z.
 *
 */


#include "FFIFOStateInfo.h"

using namespace gpu3d;


/*  Creates a new FFIFOStateInfo object.  */
FFIFOStateInfo::FFIFOStateInfo(FFIFOState newState) :

    state(newState)
{
    /*  Set color for tracing.  */
    setColor(state);

    setTag("FFStIn");
}


/*  Returns the Fragment FIFO state carried by the object.  */
FFIFOState FFIFOStateInfo::getState()
{
    return state;
}

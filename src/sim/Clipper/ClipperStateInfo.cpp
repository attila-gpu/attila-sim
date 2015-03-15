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
 * $RCSfile: ClipperStateInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:25 $
 *
 * Clipper State Info implementation file.
 *
 */

/**
 *
 *  @file ClipperStateInfo.cpp
 *
 *  This file implements the Clipper State Info class.
 *
 *  This class carries state information from the Clipper
 *  to the Command Processor.
 *
 */


#include "ClipperStateInfo.h"

using namespace gpu3d;

/*  Creates a new ClipperStateInfo object.  */
ClipperStateInfo::ClipperStateInfo(ClipperState newState) :
    state(newState)
{
    /*  Set color for tracing.  */
    setColor(state);

    setTag("ClSteIn");
}


/*  Returns the clipper state carried by the object.  */
ClipperState ClipperStateInfo::getState()
{
    return state;
}

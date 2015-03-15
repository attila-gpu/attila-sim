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
 * $RCSfile: StreamerStateInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:33 $
 *
 * Streamer State Info implementation file.
 *
 */

#include "StreamerStateInfo.h"

using namespace gpu3d;

/*  Creates a new StreamerStateInfo object.  */
StreamerStateInfo::StreamerStateInfo(StreamerState newState) : state(newState)
{
    /*  Set object color for tracing.  */
    setColor(state);

    setTag("stStIn");
}


/*  Returns the streamer state carried by the object.  */
StreamerState StreamerStateInfo::getState()
{
    return state;
}

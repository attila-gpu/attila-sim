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
 * $RCSfile: TriangleSetupStateInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:30 $
 *
 * Triangle Setup State Info implementation file.
 *
 */

/**
 *
 *  @file TriangleSetupStateInfo.cpp
 *
 *  This file implements the TriangleSetupStateInfo class.
 *
 *  The TriangleSetupStateInfo class carries state information
 *  between Triangle Setup and Primitive Assembly.
 *
 */


#include "TriangleSetupStateInfo.h"


using namespace gpu3d;

/*  Creates a new TriangleSetupStateInfo object.  */
TriangleSetupStateInfo::TriangleSetupStateInfo(TriangleSetupState newState) :

    state(newState)
{
    /*  Set color for tracing.  */
    setColor(state);

    setTag("TrSStIn");
}


/*  Returns the triangle setup state carried by the object.  */
TriangleSetupState TriangleSetupStateInfo::getState()
{
    return state;
}

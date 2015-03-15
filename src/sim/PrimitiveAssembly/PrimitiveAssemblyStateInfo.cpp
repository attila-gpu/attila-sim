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
 * $RCSfile: PrimitiveAssemblyStateInfo.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:27 $
 *
 * Primitive Assembly State Info implementation file.
 *
 */


/**
 *
 *  @file PrimitiveAssemblyStateInfo.h
 *
 *  This file implements the Primitive Assembly State Info class.
 *
 *  This class carries state information from Primitive Assembly to
 *  the Command Processor.
 *
 */

#include "PrimitiveAssemblyStateInfo.h"

using namespace gpu3d;

/*  Creates a new PrimitiveAssemblyStateInfo object.  */
PrimitiveAssemblyStateInfo::PrimitiveAssemblyStateInfo(AssemblyState newState) :
    state(newState)
{
    /*  Set color for tracing.  */
    setColor(state);

    setTag("PAsStIn");
}


/*  Returns the primitive assembly state carried by the object.  */
AssemblyState PrimitiveAssemblyStateInfo::getState()
{
    return state;
}

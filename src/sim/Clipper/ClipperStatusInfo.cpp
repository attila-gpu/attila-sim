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
 * $RCSfile: ClipperStatusInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:25 $
 *
 * Clipper Status Info implementation file.
 *
 */

/**
 *
 *  @file ClipperStatusInfo.cpp
 *
 *  This file implements the CliperStatusInfo class.
 *
 *  This class objects carries status information from
 *  the Clipper to Primitive Assembly
 *
 */

#include "ClipperStatusInfo.h"

using namespace gpu3d;

/*  Creates a new ClipperStatusInfo object.  */
ClipperStatusInfo::ClipperStatusInfo(ClipperStatus newStatus) :
    status(newStatus)
{
    /*  Set color for tracing.  */
    setColor(status);

    setTag("ClStuIn");
}


/*  Returns the clipper status carried by the object.  */
ClipperStatus ClipperStatusInfo::getStatus()
{
    return status;
}

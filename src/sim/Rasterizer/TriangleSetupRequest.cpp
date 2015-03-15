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
 * $RCSfile: TriangleSetupRequest.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:30 $
 *
 * Triangle Setup Request implementation file.
 *
 */

/**
 *
 *  @file TriangleSetupRequest.cpp
 *
 *  This file implements the TriangleSetupRequest class.
 *
 *  The TriangleSetupRequest class carries requests  to Triangle Setup
 *  for new setup triangles.
 *
 */


#include "TriangleSetupRequest.h"

using namespace gpu3d;

/*  Creates a new TriangleSetupRequest object.  */
TriangleSetupRequest::TriangleSetupRequest(u32bit trianglesRequested)
{
    /*  Set number of triangles requested.  */
    request = trianglesRequested;

    /*  Set color for tracing.  */
    setColor(0);

    setTag("TrSReq");
}

/*  Return the number of triangles requested.  */
u32bit TriangleSetupRequest::getRequest()
{
    return request;
}


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
 * $RCSfile: PrimitiveAssemblyRequest.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:27 $
 *
 * Primitive Assembly Request implementation file.
 *
 */

/**
 *
 *  @file PrimitiveAssemblyRequest.cpp
 *
 *  This file implements the PrimitiveAssemblyRequest class.
 *
 *  The PrimitiveAssemblyRequest class carries requests from
 *  the Primitive Assembly unit to the Streamer Commmit for
 *  transformed vertexes.
 *
 */


#include "PrimitiveAssemblyRequest.h"

using namespace gpu3d;

/*  Creates a new PrimitiveAssemblyRequest object.  */
PrimitiveAssemblyRequest::PrimitiveAssemblyRequest(u32bit trianglesRequested)
{
    /*  Set number of triangles requested.  */
    request = trianglesRequested;

    /*  Set color for tracing.  */
    setColor(request);

    setTag("PAsReq");
}

/*  Return the number of triangles requested.  */
u32bit PrimitiveAssemblyRequest::getRequest()
{
    return request;
}


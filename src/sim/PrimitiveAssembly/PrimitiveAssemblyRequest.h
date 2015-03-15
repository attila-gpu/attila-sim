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
 * $RCSfile: PrimitiveAssemblyRequest.h,v $
 * $Revision: 1.3 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:52 $
 *
 * Primitive Assembly Request definition file.
 *
 */

/**
 *
 *  @file PrimitiveAssemblyRequest.h
 *
 *  This file defines the PrimitiveAssemblyRequest class.
 *
 *  The Primitive Assembly Request class is used to
 *  to request a vertex by the Primitive Assembly unit to the
 *  Streamer Commit unit.
 *
 */

#ifndef _PRIMITIVEASSEMBLYREQUEST_
#define _PRIMITIVEASSEMBLYREQUEST_

#include "DynamicObject.h"

namespace gpu3d
{

/**
 *
 *  This class is used to request transformed vertexes to the
 *  Streamer Commit box.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class PrimitiveAssemblyRequest : public DynamicObject
{
private:

    u32bit request;     /**<  Number of requested triangles.  */

public:

    /**
     *
     *  Creates a new PrimitiveAssemblyRequest object.
     *
     *  @param request Number of requested triangles.
     *
     *  @return A new initialized PrimitiveAssemblyRequest object.
     *
     */

    PrimitiveAssemblyRequest(u32bit request);

    /**
     *
     *  Get the number of requested triangles.
     *
     *  @return The number of requested triangles.
     *
     */

    u32bit getRequest();

};

} // namespace gpu3d


#endif

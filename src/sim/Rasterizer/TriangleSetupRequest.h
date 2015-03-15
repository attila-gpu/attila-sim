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
 * $RCSfile: TriangleSetupRequest.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:58 $
 *
 * Triangle Setup Request definition file.
 *
 */

/**
 *
 *  @file TriangleSetupRequest.h
 *
 *  This file defines the TriangleSetupRequest class.
 *
 *  The Triangle Setup Request class is used to
 *  to request a setup triangle to the Triangle Setup unit.
 *
 */

#ifndef _TRIANGLESETUPREQUEST_
#define _TRIANGLESETUPREQUEST_

#include "DynamicObject.h"

namespace gpu3d
{

/**
 *
 *  This class is used to request setup triangles to the
 *  Triangle Setup box and request bound triangles to the
 *  Triangle Bound box.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class TriangleSetupRequest : public DynamicObject
{
private:
     
    u32bit request;     /**<  Number of requested triangles.  */

public:

    /**
     *
     *  Creates a new TriangleSetupRequest object.
     *
     *
     *  @param trianglesRequested The number of requested triangles.
     *  @return A new initialized TriangleSetupRequest object.
     *
     */
     
    TriangleSetupRequest(u32bit trianglesRequested);

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

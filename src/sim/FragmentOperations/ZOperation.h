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
 * $RCSfile: ZOperation.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:49 $
 *
 * Z Operation definition file.
 *
 */

/**
 *
 *  @file ZOperation.h
 *
 *  This file defines the Z Operation class.
 *
 *  This class defines the Z and Stencil tests started in the
 *  Z Stencil Test box that are sent through the test unit (signal).
 *
 */

#include "DynamicObject.h"

#ifndef _ZOPERATION_

#define _ZOPERATION_

namespace gpu3d
{

/**
 *
 *  This class stores the information about Z and stencil tests
 *  issued to the test unit in the Z Stencil Test box.  The objects
 *  of this class circulate through the test signal to simulate
 *  the test operation latency.
 *
 *  This class inherits from the DynamicObject class that offers
 *  basic dynamic memory management and statistic gathering capabilities.
 *
 */

class ZOperation: public DynamicObject
{
private:

    u32bit id;          /**<  Operation identifier.  */

public:

    /**
     *
     *  Z Operation constructor.
     *
     *  @param id The operation identifier.
     *
     *  @return A new Z Operation object.
     *
     */

    ZOperation(u32bit id);

    /**
     *
     *  Gets the Z operation identifier.
     *
     *  @return The Z operation identifier.
     *
     */

    u32bit getID();

};

} // namespace gpu3d

#endif

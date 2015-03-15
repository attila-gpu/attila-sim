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
 * $RCSfile: ROPOperation.h,v $
 * $Revision: 1.1 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:46 $
 *
 * ROP Operation definition file.
 *
 */

/**
 *
 *  @file ROPOperation.h
 *
 *  This file defines the ROP Operation class.
 *
 *  This class defines the ROP operations started in a Generic ROP box that
 *  sent through the ROP operation unit (latency signal).
 *
 */

#include "DynamicObject.h"
#include "GenericROP.h"

#ifndef _ROPOPERATION_

#define _ROPOPERATION_

namespace gpu3d
{

/**
 *
 *  This class stores the information about ROP operations issued to the operation unit
 *  of a Generic ROP box.  The objects of this class circulate through the operation
 *  latency signal to simulate the operation latency.
 *
 *  This class inherits from the DynamicObject class that offers
 *  basic dynamic memory management and statistic gathering capabilities.
 *
 */

class ROPOperation: public DynamicObject
{
private:

    ROPQueue *operatedStamp;        /**<  Pointer to an operated stamp object.  */

public:

    /**
     *
     *  ROP Operation constructor.
     *
     *  @param opStamp A pointer to the stamp object that is being operated.
     *
     *  @return A new ROP Operation object.
     *
     */

    ROPOperation(ROPQueue *opStamp);

    /**
     *
     *  Gets the pointer to the stamp object being operated
     *
     *  @return The pointer to the stamp object being operated.
     *
     */

    ROPQueue *getROPStamp();

};

} // namespace gpu3d

#endif

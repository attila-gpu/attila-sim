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
 * $RCSfile: ROPStatusInfo.h,v $
 * $Revision: 1.1 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:46 $
 *
 * ROP Status Info definition file.
 *
 */

/**
 *
 *  @file ROPStatusInfo.h
 *
 *  This file defines the ROP Status Info class.
 *
 *  This class defines objects that carry state information
 *  from a Generic ROP box to a producer stage that sends fragments
 *  to the Generic ROP box.
 *
 */

#ifndef _ROPSTATUSINFO_

#define _ROPSTATUSINFO_

#include "DynamicObject.h"
#include "GenericROP.h"

namespace gpu3d
{

/**
 *
 *  This class defines the objects that carry state information from
 *  a Generic ROP box to a producer stage that sends fragments to the
 *  Geneneric ROP box.
 *
 *  The class inherits from the DynamicObject class that offers basic
 *  dynamic memory and tracing features.
 *
 */

class ROPStatusInfo: public DynamicObject
{
private:

    ROPState state;     /**<  The state information carried by the object.  */

public:

    /**
     *
     *  ROPStatusInfo constructor.
     *
     *  @param state The state information the object will carry.
     *
     *  @return A new ROPStatusInfo object.
     *
     */

    ROPStatusInfo(ROPState state);


    /**
     *
     *  Get the state information carried by the object.
     *
     *  @return The state information carried by the object.
     *
     */

    ROPState getState();

};

} // namespace gpu3d

#endif

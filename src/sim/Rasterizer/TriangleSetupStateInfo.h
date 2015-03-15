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
 * $RCSfile: TriangleSetupStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:58 $
 *
 * Triangle Setup State Info definition file.
 *
 */

/**
 *
 *  @file TriangleSetupStateInfo.h
 *
 *  This file defines the TriangleSetupStateInfo class.
 *
 *  The Triangle Setup State Info class is used to
 *  carry state information between Triangle Setup
 *  and Primitive Assembly.
 *
 */

#ifndef _TRIANGLESETUPSTATEINFO_
#define _TRIANGLESETUPSTATEINFO_

#include "DynamicObject.h"
#include "TriangleSetup.h"

namespace gpu3d
{

/**
 *
 *  This class defines a container for the state signals
 *  that the Triangle Setup box sends to the Primitive Assembly box.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class TriangleSetupStateInfo : public DynamicObject
{
private:
    
    TriangleSetupState state;    /**<  The Triangle Setup state.  */

public:

    /**
     *
     *  Creates a new TriangleSetupStateInfo object.
     *
     *  @param state The triangle setup state carried by this
     *  triangle setup state info object.
     *
     *  @return A new initialized TriangleSetupStateInfo object.
     *
     */
     
    TriangleSetupStateInfo(TriangleSetupState state);
    
    /**
     *
     *  Returns the triangle setup state carried by the triangle setup
     *  state info object.
     *
     *  @return The triangle setup state carried in the object.
     *
     */
     
    TriangleSetupState getState();
};

} // namespace gpu3d

#endif

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
 * $RCSfile: ZStencilStatusInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:49 $
 *
 * Z Stencil Status Info definition file.
 *
 */

/**
 *
 *  @file ZStencilStatusInfo.h
 *
 *  This file defines the Z Stencil Status Info class.
 *
 *  This class defines objects that carry state information
 *  from Z Stencil Test to Fragment FIFO (Interpolator).
 *
 */

#ifndef _ZSTENCILSTATUSINFO_

#define _ZSTENCILSTATUSINFO_

#include "DynamicObject.h"
#include "ZStencilTest.h"

namespace gpu3d
{

/**
 *
 *  This class defines the objects that carry state information from
 *  the Z Stencil Test box to Fragment FIFO (Interpolator) box.
 *
 *  The class inherits from the DynamicObject class that offers basic
 *  dynamic memory and tracing features.
 *
 */

class ZStencilStatusInfo: public DynamicObject
{
private:

    ZStencilTestState state;    /**<  The state information carried by the object.  */

public:

    /**
     *
     *  ZStencilStatusInfo constructor.
     *
     *  @param state The state information the object will carry.
     *
     *  @return A new ZStencilStatusInfo object.
     *
     */

    ZStencilStatusInfo(ZStencilTestState state);


    /**
     *
     *  Get the state information carried by the object.
     *
     *  @return The state information carried by the object.
     *
     */

    ZStencilTestState getState();

};

} // namespace gpu3d

#endif

/**************************************************************************
 *
 * Copyright (c) 2002 - 2004 by Computer Architecture Department,
 * Universitat Politecnica de Catalunya.
 * All rights reserved.
 *
 * The contents of this file may not be disclosed to third parties,
 * copied or duplicated in any form, in whole or in part, without the
 * prior permission of the authors, Computer Architecture Department
 * and Universitat Politecnica de Catalunya.
 *
 * $RCSfile: ColorWriteStatusInfo.h,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:46 $
 *
 * Color Write Status Info definition file.
 *
 */

/**
 *
 *  @file ColorWriteStatusInfo.h
 *
 *  This file defines the Color Write Status Info class.
 *
 *  This class defines objects that carry state information
 *  from Color Write to Z Test.
 *
 */

#ifndef _COLORWRITESTATUSINFO_

#define _COLORWRITESTATUSINFO_

#include "DynamicObject.h"
#include "ColorWrite.h"

namespace gpu3d
{

/**
 *
 *  This class defines the objects that carry state information from
 *  the Color Write box to the Z Test box.
 *
 *  The class inherits from the DynamicObject class that offers basic
 *  dynamic memory and tracing features.
 *
 */

class ColorWriteStatusInfo: public DynamicObject
{
private:

    ColorWriteState state;      /**<  The state information carried by the object.  */

public:

    /**
     *
     *  ColorWriteStatusInfo constructor.
     *
     *  @param state The state information the object will carry.
     *
     *  @return A new ColorWriteStatusInfo object.
     *
     */

    ColorWriteStatusInfo(ColorWriteState state);


    /**
     *
     *  Get the state information carried by the object.
     *
     *  @return The state information carried by the object.
     *
     */

    ColorWriteState getState();

};

} // namespace gpu3d

#endif

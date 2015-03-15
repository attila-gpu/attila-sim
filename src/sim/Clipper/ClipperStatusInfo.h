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
 * $RCSfile: ClipperStatusInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:46 $
 *
 * Clipper Status Info definition file.
 *
 */


#ifndef _CLIPPERSTATUSINFO_
#define _CLIPPERSTATUSINFO_

#include "DynamicObject.h"
#include "Clipper.h"

namespace gpu3d
{

/**
 *
 *  @file ClipperStatusInfo.h
 *
 *  This file defines the Clipper Stateus Info class.
 *
 *  Carries the Clipper status to Primitive Assembly
 *
 */
 
/**
 *
 *  This class defines a container for the state signals
 *  that the Clipper sends to Primitive Assembly.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class ClipperStatusInfo : public DynamicObject
{
private:
    
    ClipperStatus status;   /**<  The clipper status.  */

public:

    /**
     *
     *  Creates a new ClipperStatusInfo object.
     *
     *  @param state The clipper status carried by this clipper status
     *  info object.
     *
     *  @return A new initialized ClipperStatusInfo object.
     *
     */
     
    ClipperStatusInfo(ClipperStatus state);
    
    /**
     *
     *  Returns the clipper status carried by the clipper status info object.
     *
     *  @return The clipper status carried in the object.
     *
     */
     
    ClipperStatus getStatus();
};

} // namespace gpu3d

#endif

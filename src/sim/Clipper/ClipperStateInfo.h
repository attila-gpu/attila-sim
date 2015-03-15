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
 * $RCSfile: ClipperStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:46 $
 *
 * Clipper State Info definition file.
 *
 */


#ifndef _CLIPPERSTATEINFO_
#define _CLIPPERSTATEINFO_

#include "DynamicObject.h"
#include "Clipper.h"

namespace gpu3d
{

/**
 *
 *  @file ClipperStateInfo.h
 *
 *  This file defines the Clipper State Info class.
 *
 */
 
/**
 *
 *  This class defines a container for the state signals
 *  that the Clipper sends to the Command Processor
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class ClipperStateInfo : public DynamicObject
{
private:
    
    ClipperState state;     /**<  The clipper state.  */

public:

    /**
     *
     *  Creates a new ClipperStateInfo object.
     *
     *  @param state The clipper state carried by this clipper state info
     *  object.
     *
     *  @return A new initialized ClipperStateInfo object.
     *
     */
     
    ClipperStateInfo(ClipperState state);
    
    /**
     *
     *  Returns the clipper state carried by the clipper state info object.
     *
     *  @return The clipper state carried in the object.
     *
     */
     
    ClipperState getState();
};

} // namespace gpu3d

#endif

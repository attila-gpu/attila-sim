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
 * $RCSfile: FFIFOStateInfo.h,v $
 * $Revision: 1.3 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:52 $
 *
 * Fragment FIFO State Info definition file.
 *
 */

/**
 *
 *  @file FFIFOStateInfo.h
 *
 *  This file defines the FFIFOStateInfo class.
 *
 *  The Fragment FIFO State Info class is used to carry state information
 *  from the Fragment FIFO to the Hierarchical/Early Z unit.
 *
 */

#ifndef _FFIFOSTATEINFO_
#define _FFIFOSTATEINFO_

#include "DynamicObject.h"
#include "FragmentFIFOState.h"

namespace gpu3d
{

/**
 *
 *  This class defines a container for the state signals
 *  that the Fragment FIFO box sends to the Hierarchical/Early Z.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class FFIFOStateInfo : public DynamicObject
{
private:

    FFIFOState state;       /**<  The Fragment FIFO state.  */

public:

    /**
     *
     *  Creates a new FFIFOStateInfo object.
     *
     *  @param state The Fragment FIFO state carried by this fragment FIFO state info object.
     *
     *  @return A new initialized FFIFOStateInfo object.
     *
     */

    FFIFOStateInfo(FFIFOState state);

    /**
     *
     *  Returns the Fragment FIFO state carried by the Fragment FIFO
     *  state info object.
     *
     *  @return The Fragment FIFO carried in the object.
     *
     */

    FFIFOState getState();
};

} // namespace gpu3d

#endif

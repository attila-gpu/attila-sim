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
 * $RCSfile: RasterizerStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:56 $
 *
 * Rasterizer State Info definition file.
 *
 */


#ifndef _RASTERIZERSTATEINFO_
#define _RASTERIZERSTATEINFO_

#include "DynamicObject.h"
#include "Rasterizer.h"

namespace gpu3d
{

/**
 *
 *  This class defines a container for the state signals
 *  that the rasterizer sends to the Streamer.
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class RasterizerStateInfo : public DynamicObject
{
private:
    
    RasterizerState state;    /**<  The rasterizer state.  */

public:

    /**
     *
     *  Creates a new RasterizerStateInfo object.
     *
     *  @param state The rasterizer state carried by this
     *  rasterizer state info object.
     *
     *  @return A new initialized RasterizerStateInfo object.
     *
     */
     
    RasterizerStateInfo(RasterizerState state);
    
    /**
     *
     *  Returns the rasterizer state carried by the rasterizer
     *  state info object.
     *
     *  @return The rasterizer state carried in the object.
     *
     */
     
    RasterizerState getState();
};

} // namespace gpu3d

#endif

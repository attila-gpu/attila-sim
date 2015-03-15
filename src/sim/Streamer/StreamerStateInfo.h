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
 * $RCSfile: StreamerStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:29:05 $
 *
 * Streamer State Info definition file.
 *
 */


#ifndef _STREAMERSTATEINFO_
#define _STREAMERSTATEINFO_

#include "DynamicObject.h"
#include "Streamer.h"

namespace gpu3d
{

/**
 *
 *  This class defines a container for the state signals
 *  that the streamer sends to the Command Processor.
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class StreamerStateInfo : public DynamicObject
{
private:
    
    StreamerState state;    /**<  The streamer state.  */

public:

    /**
     *
     *  Creates a new StreamerStateInfo object.
     *
     *  @param state The Streamer state carried by this
     *  streamer state info object.
     *
     *  @return A new initialized StreamerStateInfo object.
     *
     */
     
    StreamerStateInfo(StreamerState state);
    
    /**
     *
     *  Returns the streamer state carried by the stream
     *  state info object.
     *
     *  @return The streamer state carried in the object.
     *
     */
     
    StreamerState getState();
};

} // namespace gpu3d

#endif

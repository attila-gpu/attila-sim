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
 * $RCSfile: ConsumerStateInfo.h,v $
 * $Revision: 1.3 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:59 $
 *
 * Consumer State Info definition file.
 *
 */


#ifndef _CONSUMERSTATEINFO_
#define _CONSUMERSTATEINFO_

#include "DynamicObject.h"
#include "Streamer.h"

namespace gpu3d
{

/**  This describes the shader consumer state signal states.  */
enum ConsumerState
{
    CONS_READY,               /**<  The consumer can receive ouputs from the shaders.  */
    CONS_BUSY,                /**<  The consumer can not receive outputs from the shaders.  */

    /* Specific to the Streamer Unit. */

    CONS_LAST_VERTEX_COMMIT,  /**<  Streamer has committed the last transformed vertex. 
                                    Hence, no more vertex shading inputs can be expected 
                                    from Streamer Loader.  */
    CONS_FIRST_VERTEX_IN,     /**<  Streamer is processing the first index of the new input stream.
                                    Hence, new vertex shading inputs are expected from
                                    Streamer Loader.  */
};


/**
 *
 *  This class defines a container for the state signals
 *  that the streamer sends to the Shader.
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class ConsumerStateInfo : public DynamicObject
{
private:

    ConsumerState state;    /**<  The streamer (consumer) state.  */

public:

    /**
     *
     *  Creates a new ConsumerStateInfo object.
     *
     *  @param state The Streamer (consumer) state carried by this
     *  consumer state info object.
     *
     *  @return A new initialized ConsumerStateInfo object.
     *
     */

    ConsumerStateInfo(ConsumerState state);

    /**
     *
     *  Returns the streamer (consumer) state carried by the
     *  consumer state info object.
     *
     *  @return The streamer (consumer) state carried in the object.
     *
     */

    ConsumerState getState();
};

} // namespace gpu3d

#endif

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
 * $RCSfile: ShaderStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:29:02 $
 *
 * Shader State Info definition file.
 *
 */


#ifndef _SHADERSTATEINFO_
#define _SHADERSTATEINFO_

#include "DynamicObject.h"
#include "ShaderFetch.h"

/**
 *
 *  This class defines a container for the state signals
 *  that the shader sends to the Streamer.
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */

namespace gpu3d
{

class ShaderStateInfo : public DynamicObject
{
private:
    
    ShaderState state;    /**<  The Shader state.  */
    
public:

    /**
     *
     *  Creates a new ShaderStateInfo object.
     *
     *  @param state The Shader state carried by this
     *  shader state info object.
     *
     *  @return A new initialized ShaderStateInfo object.
     *
     */
     
    ShaderStateInfo(ShaderState state);

    /**
     *
     *  Returns the shader state carried by the shader
     *  state info object.
     *
     *  @return The shader state carried in the object.
     *
     */
     
    ShaderState getState();

};

} // namespace gpu3d

#endif

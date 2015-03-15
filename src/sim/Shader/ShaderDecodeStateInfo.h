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
 * $RCSfile: ShaderDecodeStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:29:00 $
 *
 * Shader Decode State Info definition file.
 *
 */


#ifndef _SHADERDECODESTATEINFO_
#define _SHADERDECODESTATEINFO_

#include "DynamicObject.h"
#include "ShaderDecodeExecute.h"

namespace gpu3d
{

/**
 *
 *  This class defines a container for the state signals
 *  that the shader decode box sends to the shader fetch box.
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class ShaderDecodeStateInfo : public DynamicObject
{

private:
    
    ShaderDecodeState state;    /**<  The Shader decode state.  */

public:

    /**
     *
     *  Creates a new ShaderDecodeStateInfo object.
     *
     *  @param state The Shader decode state carried by this
     *  shader decode state info object.
     *
     *  @return A new initialized ShaderDecodeStateInfo object.
     *
     */
     
    ShaderDecodeStateInfo(ShaderDecodeState state);
    
    /**
     *
     *  Returns the shader decode state carried by the shader
     *  decode state info object.
     *
     *  @return The shader decode state carried in the object.
     *
     */
     
    ShaderDecodeState getState();
};

} // namespace gpu3d

#endif

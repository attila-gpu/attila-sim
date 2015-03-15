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
 * $RCSfile:$
 * $Revision:$
 * $Author:$
 * $Date:$
 *
 * Shader State.
 *
 */

/**
 *
 *  @file ShaderState.h
 *
 *  Defines enumeration types for the shader external and internal states.
 *
 */

#ifndef _SHADERSTATE_

#define _SHADERSTATE_

namespace gpu3d
{

/**
 *
 *  Shader States.
 *
 */


enum ShaderState
{
    SH_RESET,   /**<  The shader is in reset state.  */
    SH_READY,   /**<  The Shader accepts new input data.  */
    SH_EMPTY,   /**<  The Shader has all its thread and input buffers empty.  Accepts programs and parameters.  */
    SH_BUSY     /**<  The Shader has all the input buffers filled.  Does not accepts commands.  */
};

/**
 *
 *  This defines the different states in which the Shader Decode stage
 *  can be.
 *
 */

enum ShaderDecodeState
{
    SHDEC_READY,    /**<  The Shader Decode stage can receive instructions.  */
    SHDEC_BUSY      /**<  The Shader Decode stage can not receive instructions.  */
};

}  // namespace gpu3d

#endif

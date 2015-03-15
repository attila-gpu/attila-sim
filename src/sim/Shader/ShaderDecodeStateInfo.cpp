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
 * $RCSfile: ShaderDecodeStateInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:30 $
 *
 * Shader Decode State Info implementation file.
 *
 */

#include "ShaderDecodeStateInfo.h"

using namespace gpu3d;

/*  Creates a new Shader State Info object.  */
ShaderDecodeStateInfo::ShaderDecodeStateInfo(ShaderDecodeState newState) : state(newState)
{
    /*  Set color for tracing.  */
    setColor(state);

    setTag("ShDStIn");
}


/*  Returns the shader decode state carried by the object.  */
ShaderDecodeState ShaderDecodeStateInfo::getState()
{
    return state;
}

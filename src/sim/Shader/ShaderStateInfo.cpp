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
 * $RCSfile: ShaderStateInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:31 $
 *
 * Shader State Info implementation file.
 *
 */

#include "ShaderStateInfo.h"


using namespace gpu3d;

/*  Creates a new Shader State Info object.  */
ShaderStateInfo::ShaderStateInfo(ShaderState newState) : state(newState)
{
    /*  Set object color for tracing.  */
    setColor(state);

    setTag("ShStIn");
}

/*  Returns the shader state carried by the object.  */
ShaderState ShaderStateInfo::getState()
{
    return state;
}


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
 * $RCSfile: TextureUnitStateInfo.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:32 $
 *
 * Texture Unit State Info implementation file.
 *
 */

/**
 *
 *  @file TextureUnitStateInfo.cpp
 *
 *  This file implements the Texture Unit State Info class.
 *  This class carries state information from the Texture Unit
 *  to the shader to which it is attached.
 *
 */

#include "TextureUnitStateInfo.h"

using namespace gpu3d;

/*  Creates a new TextureUnit State Info object.  */
TextureUnitStateInfo::TextureUnitStateInfo(TextureUnitState newState) : state(newState)
{
    /*  Set object color for tracing.  */
    setColor(state);

    setTag("TexStIn");
}


/*  Returns the texture unit state carried by the object.  */
TextureUnitState TextureUnitStateInfo::getState()
{
    return state;
}

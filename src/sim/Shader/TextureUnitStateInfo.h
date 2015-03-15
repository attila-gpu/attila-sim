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
 * $RCSfile: TextureUnitStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:29:03 $
 *
 * Texture Unit State Info definition file.
 *
 */


#ifndef _TEXTUREUNITSTATEINFO_
#define _TEXTUREUNITSTATEINFO_

#include "DynamicObject.h"
#include "TextureUnit.h"

namespace gpu3d
{

/**
 *
 *  @file TextureUnitStateInfo.h
 *
 *  This file defines the TextureUnitStateInfo class that is used
 *  to carry state information from the Texture Unit to the Shader.
 *
 */

/**
 *
 *  This class defines a container for the state signals that the Texture Unit sends to the Shader.
 *  Inherits from Dynamic Object that offers dynamic memory support and tracing capabilities.
 *
 */


class TextureUnitStateInfo : public DynamicObject
{
private:

    TextureUnitState state;    /**<  The Texture Unit state.  */

public:

    /**
     *
     *  Creates a new TextureUnitStateInfo object.
     *
     *  @param state The Texture Unit state carried by this  texture state info object.
     *
     *  @return A new initialized Texture Unit StateInfo object.
     *
     */

    TextureUnitStateInfo(TextureUnitState state);

    /**
     *
     *  Returns the texture unit state carried by the texture unit state info object.
     *
     *  @return The texture unit state carried in the object.
     *
     */

    TextureUnitState getState();
};

} // namespace gpu3d

#endif

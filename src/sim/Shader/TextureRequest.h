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
 * $RCSfile: TextureRequest.h,v $
 * $Revision: 1.3 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:29:02 $
 *
 * Texture Request definition file.
 *
 */

/**
 *
 *  @file TextureRequest.h
 *
 *  Defines the Texture Request class.  This class describes a request from
 *  the Shader Decode Execute unit to the Texture Unit.
 *
 */

#include "GPUTypes.h"
#include "DynamicObject.h"
#include "TextureEmulator.h"

#ifndef _TEXTUREREQUEST_

#define _TEXTUREREQUEST_

namespace gpu3d
{

/**
 *
 *  Defines a texture access request from the Shader Decode Execute Unit
 *  to the Texture Unit.
 *
 *  This class inherits from the DynamicObject class which offers
 *  dynamic memory management and tracing support.
 *
 *
 */


class TextureRequest : public DynamicObject
{

private:

    TextureAccess *textAccess;      /**<  Pointer to the texture access carried by the object.  */

public:

    /**
     *
     *  Class constructor.
     *
     *  Creates and initializes a new Texture Request object.
     *
     *  @param txAccess Pointer to a Texture Access to be carried by the object from
     *  the shader to the texture unit.
     *
     *  @return A new Texture Request object.
     *
     */

    TextureRequest(TextureAccess *txAccess);

    /**
     *
     *  Returns the texture access being carried by the object.
     *
     *  @return A pointer to the texture access being carried by the object.
     *
     */

    TextureAccess *getTextAccess();

};

} // namespace gpu3d

#endif

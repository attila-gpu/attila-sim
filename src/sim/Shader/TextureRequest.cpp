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
 * $RCSfile: TextureRequest.cpp,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2006-01-31 12:54:39 $
 *
 * Texture Request implementation file.
 *
 */


/**
 *
 *  @file TextureRequest.cpp
 *
 *  This file implements the Texture Request class.
 *
 *  The Texture Request class carries texture access requests from
 *  the Shader Decode Execute unit to the Texture Unit.
 *
 */

#include "TextureRequest.h"

using namespace gpu3d;

/*  Texture Request constructor.  */
TextureRequest::TextureRequest(TextureAccess *txAccess)
{
    /*  Initialize Texture Request attributes.  */
    textAccess = txAccess;

    setTag("TexReq");
}


/*  Gets the texture coordinates.  */
TextureAccess *TextureRequest::getTextAccess()
{
    return textAccess;
}

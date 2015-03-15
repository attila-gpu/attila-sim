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
 */

#include "TextureTarget.h"

using namespace libgl;

TextureTarget::TextureTarget(GLuint target) : BaseTarget(target)
{    
    if ( target != GL_TEXTURE_1D && target != GL_TEXTURE_2D && target != GL_TEXTURE_3D && 
         target != GL_TEXTURE_CUBE_MAP )
         panic("TextureTarget", "TextureTarget", "Unexpected texture target");
         
    TextureObject* defTex = new TextureObject(0, target); // create a default object
    defTex->setTarget(*this);
    setCurrent(*defTex);
    setDefault(defTex);
    
    /*****************************************
     * CONFIGURE DEFAULT TEXTURE OBJECT HERE *
     *****************************************/
}


TextureObject* TextureTarget::createObject(GLuint name)
{
    
    TextureObject* to = new TextureObject(name, getName());    
    to->setTarget(*this);
    return to;
}


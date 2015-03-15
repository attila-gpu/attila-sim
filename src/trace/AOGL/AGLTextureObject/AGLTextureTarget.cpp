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

#include "AGLTextureTarget.h"

using namespace agl;

GLTextureTarget::GLTextureTarget(GLuint target) : BaseTarget(target)
{    
    if ( target != GL_TEXTURE_1D && target != GL_TEXTURE_2D && target != GL_TEXTURE_3D && 
         target != GL_TEXTURE_CUBE_MAP && target != GL_TEXTURE_RECTANGLE )
         panic("GLTextureTarget", "GLTextureTarget", "Unexpected texture target");
         
    GLTextureObject* defTex = new GLTextureObject(0, target); // create a default object
    defTex->setTarget(*this);
    setCurrent(*defTex);
    setDefault(defTex);
    
    /*****************************************
     * CONFIGURE DEFAULT TEXTURE OBJECT HERE *
     *****************************************/
}


GLTextureObject* GLTextureTarget::createObject(GLuint name)
{
    
    GLTextureObject* to = new GLTextureObject(name, getName());    
    to->setTarget(*this);
    return to;
}


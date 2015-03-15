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

#include "AGLTextureManager.h"
#include <iostream>


using namespace std;
using namespace agl;

GLTextureManager* GLTextureManager::tm = 0;


/* Create singleton */
GLTextureManager& GLTextureManager::instance()
{
    if ( !tm )
        tm = new GLTextureManager(16); // MAX_TEXTURE_UNITS_ARB groups
    return *tm;
}


GLTextureManager::GLTextureManager(GLenum textureUnits) : BaseManager(textureUnits)
{
    for ( GLuint i = 0; i < textureUnits; i++ )
    {
        // Add targets
        selectGroup(i);
        addTarget(new GLTextureTarget(GL_TEXTURE_1D));
        addTarget(new GLTextureTarget(GL_TEXTURE_2D));
        addTarget(new GLTextureTarget(GL_TEXTURE_3D));
        addTarget(new GLTextureTarget(GL_TEXTURE_CUBE_MAP));
        addTarget(new GLTextureTarget(GL_TEXTURE_RECTANGLE));
    }
    selectGroup(0);    
}

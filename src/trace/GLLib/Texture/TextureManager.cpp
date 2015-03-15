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

#include "TextureManager.h"
#include "support.h"
#include "ImplementationLimits.h"
#include <iostream>


using namespace std;
using namespace libgl;

TextureManager* TextureManager::tm = 0;


/* Create singleton */
TextureManager& TextureManager::instance()
{
    if ( !tm )
        tm = new TextureManager(MAX_TEXTURE_UNITS_ARB); // MAX_TEXTURE_UNITS_ARB groups
    return *tm;
}


TextureManager::TextureManager(GLenum textureUnits) : BaseManager(textureUnits)
{
    for ( GLuint i = 0; i < textureUnits; i++ )
    {
        // Add targets
        selectGroup(i);
        addTarget(new TextureTarget(GL_TEXTURE_1D));
        addTarget(new TextureTarget(GL_TEXTURE_2D));
        addTarget(new TextureTarget(GL_TEXTURE_3D));
        addTarget(new TextureTarget(GL_TEXTURE_CUBE_MAP));
    }
    selectGroup(0);    
}


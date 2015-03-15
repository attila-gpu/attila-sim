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

#include "ARBProgramManager.h"
#include "support.h"
#include "glext.h"

using namespace agl;


ARBProgramManager::ARBProgramManager() 
{
    // Program objects define one group with two targets
    addTarget(new ARBProgramTarget(GL_VERTEX_PROGRAM_ARB));
    addTarget(new ARBProgramTarget(GL_FRAGMENT_PROGRAM_ARB));
}

ARBProgramTarget& ARBProgramManager::target(GLenum target) 
{ 
    return static_cast<ARBProgramTarget&>(getTarget(target));
}

void ARBProgramManager::programString(GLenum targetName, GLenum format, GLsizei len, const void* program)
{
    target(targetName).getCurrent().setSource(string((const char*)program, len), format);
}


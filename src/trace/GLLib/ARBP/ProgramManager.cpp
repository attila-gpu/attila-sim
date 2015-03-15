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

#include "ProgramManager.h"
#include "support.h"
#include "glext.h"

using namespace libgl;

ProgramManager* ProgramManager::pm = 0;

ProgramManager& ProgramManager::instance()
{
    if ( !pm )
        pm = new ProgramManager;;
                
    return *pm;
}


ProgramManager::ProgramManager() 
{
    // Program objects define one group with two targets
    addTarget(new ProgramTarget(GL_VERTEX_PROGRAM_ARB));
    addTarget(new ProgramTarget(GL_FRAGMENT_PROGRAM_ARB));
}


bool ProgramManager::programString(GLenum target, GLenum format, GLsizei len, const void* program)
{
    ProgramObject& po = dynamic_cast<ProgramObject&>(getTarget(target).getCurrent());    
    
    po.setSource(program, len);
    po.setFormat(format);

    return true; //po.compile();
}


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

#ifndef GLITOOLEXPORTS_H
    #define GLITOOLEXPORTS_H

#include "GLITool.h"

/**
 * This two methods must be exported in every GLITool DLL implemented
 */
extern "C"
{
    GLITool* APIENTRY getTool();
    void APIENTRY releaseTool(GLITool* tool);
}

#endif // GLITOOLEXPORTS_H

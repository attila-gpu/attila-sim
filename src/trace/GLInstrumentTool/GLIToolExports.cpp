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

#include "GLIToolExports.h"
#include "GLIToolManager.h"

GLITool* APIENTRY getTool()
{
    static GLITool* tool = 0;
    if ( !tool )
        tool = GLIToolManager::instance().getTool();
    return tool;
}

void APIENTRY releaseTool(GLITool* tool)
{
    delete tool;
}

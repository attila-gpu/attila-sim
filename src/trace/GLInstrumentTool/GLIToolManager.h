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

#ifndef GLITOOLMANAGER_H
    #define GLITOOLMANAGER_H

#include "GLITool.h"
#include <string>
#include <map>


class GLIToolManager
{
private:

    GLIToolManager();
    GLIToolManager(const GLIToolManager&);
    GLIToolManager& operator=(const GLIToolManager&);

public:

    static GLIToolManager& instance();
    GLITool* getTool();
};

#endif // GLITOOLMANAGER_H

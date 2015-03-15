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

#ifndef DEMOTOOL_H
    #define DEMOTOOL_H

#include "GLITool.h"

class DemoTool : public GLITool
{
public:
    
    bool init(GLInstrument& gli);  
    bool release();
    
    DemoTool();
    ~DemoTool();
};

#endif DEMOTOOL_H

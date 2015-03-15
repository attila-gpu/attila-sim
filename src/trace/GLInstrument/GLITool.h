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

#ifndef GLITOOL_H
    #define GLITOOL_H

#include "GLInstrument.h"

struct GLITool
{
    /**
     * This method must have the code required to instrument OpenGL app
     * inserting handlers to GLInstrument object
     *
     * This method will be called just after retrieve the tool from GLInstrument
     */
    virtual bool init(GLInstrument& gli)=0;

};


#endif // GLITOOL_H

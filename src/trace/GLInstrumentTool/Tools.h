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

#ifndef TOOLS_H
    #define TOOLS_H

#include "GLITool.h"

/////////////////////////////////////////////////////////
//// TESTEAR DOSWAPBUFFERS
//////////////////////////////////////////////////////



class FrameHandlerWakeup : public EventHandler
{
    void action(GLInstrument& gli);
};

class FrameHandlerInfo : public EventHandler
{
    void action(GLInstrument& gli);
};

// Tool Every N Frames Info tool
class EveryNInfoTool : public GLITool
{
private:

    int every;

public:
    
    EveryNInfoTool(int every);
    bool init(GLInstrument& gli);
};

#endif // TOOLS_H


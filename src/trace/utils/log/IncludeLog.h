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

#ifndef INCLUDELOG_H
    #define INCLUDELOG_H

#include "LogObject.h"

namespace includelog
{

enum LogType
{
    Init = 0x1,
    Panic = 0x2,
    Warning = 0x4,
    Debug = 0x8,
};

// Access to the global LogObject
LogObject& logfile();

} // namespace 

#endif // INCLUDELOG_H

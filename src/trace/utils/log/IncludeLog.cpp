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

#include "IncludeLog.h"
//#include "support.h"

LogObject& includelog::logfile()
{
    static LogObject* log = 0;
    if ( log == 0 )
    {
        //popup("includelog::logfile()", "Creating LogObject");
        log = new LogObject;
    }
    return *log;
}

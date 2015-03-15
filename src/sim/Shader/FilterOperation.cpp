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
 * $RCSfile: FilterOperation.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:30 $
 *
 * FilterOperation implementation file.
 *
 */

/**
 *
 *  @file FilterOperation.cpp
 *
 *  This file implements the FilterOperation class that carrier information
 *  about the current operation in the texture unit filter pipeline.
 *
 */

#include "FilterOperation.h"

using namespace gpu3d;

/*  Creates a new Filter Operation object.  */
FilterOperation::FilterOperation(u32bit entry) : filterEntry(entry)
{
    setTag("FiltOp");
}


/*  Returns the queue entry for the filter operation.  */
u32bit FilterOperation::getFilterEntry()
{
    return filterEntry;
}

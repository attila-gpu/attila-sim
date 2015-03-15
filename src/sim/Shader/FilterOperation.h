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
 * $RCSfile: FilterOperation.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:59 $
 *
 * Filter Operation definition file.
 *
 */


#ifndef _FILTEROPERATION_
#define _FILTEROPERATION_

#include "DynamicObject.h"

namespace gpu3d
{

/**
 *
 *  @file FilterOperation.h
 *
 *  This file defines the FilterOperation class that carrier information
 *  about the current operation in the texture unit filter pipeline.
 *
 */

/**
 *
 *  This class defines a container for filtering operations that circulate through the
 *  filter pipeline in a Texture Unit.
 *  Inherits from Dynamic Object that offers dynamic memory support and tracing capabilities.
 *
 */


class FilterOperation : public DynamicObject
{
private:

    u32bit filterEntry;     /**<  The entry in the filter queue being processed.  */

public:

    /**
     *
     *  Creates a new FilterOperation object.
     *
     *  @param filterEntry The queue entry for the filter operation being
     *  carried by this texture state info object.
     *
     *  @return A new initialized FilterOperation object.
     *
     */

    FilterOperation(u32bit filterEntry);

    /**
     *
     *  Returns the queue entry for the filter operation being carried by this
     *  FilterOperation object.
     *
     *  @return The filter queue entry for this operation.
     *
     */

    u32bit getFilterEntry();
};

} // namespace gpu3d

#endif

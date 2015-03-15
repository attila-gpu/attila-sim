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
 * $RCSfile: HZStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:54 $
 *
 * Hierarchical Z State Info definition file.
 *
 */

/**
 *
 *  @file HZStateInfo.h
 *
 *  This file defines the HZStateInfo class.
 *
 *  The Hieararchical Z State Info class is used to carry state information
 *  between Hierarchical Z early test and Triangle Traversal.
 *
 */

#ifndef _HZSTATEINFO_
#define _HZSTATEINFO_

#include "DynamicObject.h"
#include "HierarchicalZ.h"

namespace gpu3d
{

/**
 *
 *  This class defines a container for the state signals
 *  that the Hierarchical Z box sends to the Triangle Traversal box.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class HZStateInfo : public DynamicObject
{
private:

    HZState state;    /**<  The Hiearchical Z early test state.  */

public:

    /**
     *
     *  Creates a new HZStateInfo object.
     *
     *  @param state The Hierarchical Z Early test state carried by this
     *  hierarchical Z state info object.
     *
     *  @return A new initialized HZStateInfo object.
     *
     */

    HZStateInfo(HZState state);

    /**
     *
     *  Returns the hiearchical Z early test state carried by the hierarchical z
     *  state info object.
     *
     *  @return The Hierarchical Z state carried in the object.
     *
     */

    HZState getState();
};

} // namespace gpu3d

#endif

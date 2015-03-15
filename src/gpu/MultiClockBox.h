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
 * Multi Clock Box class definition file.
 *
 */

#ifndef __MULTICLOCKBOX__
   #define __MULTICLOCKBOX__

#include "Box.h"

namespace gpu3d
{

/**
 *
 *  Specialization of the Box class for boxes that require multiple clock domains.
 *
 */
 
class MultiClockBox : public Box
{

private:


public:

    /**
     *
     *  Multi Clock Box constructor.
     *
     *  Current implementation just calls Box constructor.
     *
     *  @param name Name of the multi clocked box.
     *  @param parent Pointer to the parent box of the multi clocked box.
     * 
     *  @return A new MultiClockBox instance.
     *
     */
     
    MultiClockBox(const char *name, Box *parent);
     
    /**
     *
     *  Pure virtual clock function for a single clock domain.
     *
     *  Updates the state of one of the single clock domain implemented in the box.
     *
     *  @param cycle Current simulation cycle for the clock domain to update.
     *
     */
     
    virtual void clock(u64bit cycle) = 0;

    /**
     *
     *  Pure virtual clock function for multiple clock domains.
     *
     *  Updates the state of one of the clock domains implemented in the box.
     *
     *  @param domain Clock domain from the box to update.
     *  @param cycle Current simulation cycle for the clock domain to update.
     *
     */
     
    virtual void clock(u32bit domain, u64bit cycle) = 0;
};

} // namespace gpu3d

#endif

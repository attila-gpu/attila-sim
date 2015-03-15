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
 * $RCSfile: ColorBlockStateInfo.h,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:45 $
 *
 * ColorBlockStateInfo definition file.
 *
 */

/**
 *
 *  @file ColorBlockStateInfo.h
 *
 *  This file defines the ColorBlockStateInfo class.
 *
 *  This class is used to transmit the color buffer block
 *  state memory to the DAC unit.
 *
 */

#ifndef _COLORBLOCKSTATEINFO_

#include "DynamicObject.h"
#include "ColorCacheV2.h"

namespace gpu3d
{

/**
 *
 *  The class is used to carry the color block state memory
 *  from the ColorWrite/ColorCache box to the DAC unit.
 *
 *  Inherits from the dynamic object class.
 *
 */

class ColorBlockStateInfo : public DynamicObject
{

private:

    ROPBlockState *stateMemory;         /**<  The color buffer state memory being transmited.  */
    u32bit blocks;                      /**<  Number of blocks of the state memory being transmited.  */

public:

    /**
     *
     *  Constructor.
     *
     *  @param state Pointer to the color buffer block state memory to transmit.
     *  @param numBlocks  Number of block states to transmit.
     *
     *  @return A colorblockstateinfo object.
     *
     */

    ColorBlockStateInfo(ROPBlockState *state, u32bit numBlocks);

    /**
     *
     *  Retrieves the pointer to the state memory.
     *
     *  @return The pointer to the transmited state memory.
     *
     */

    ROPBlockState *getColorBufferState();

    /**
     *
     *  Get the number of blocks being transmited.
     *
     *  @return Number of block states transmited.
     *
     */

    u32bit getNumBlocks();

};

} // namespace gpu3d

#endif

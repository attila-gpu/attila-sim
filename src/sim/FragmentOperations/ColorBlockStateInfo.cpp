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
 * $RCSfile: ColorBlockStateInfo.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2006-08-25 06:57:45 $
 *
 * ColorBlockStateInfo implementation file.
 *
 */

/**
 *
 *  @file ColorBlockStateInfo.cpp
 *
 *  This file implements the ColorBlockStateInfo class.
 *
 *  This class is used to transmit the color buffer block
 *  state memory from the ColorWrite/ColorCache box to the
 *  DAC box.
 *
 */

#include "ColorBlockStateInfo.h"

using namespace gpu3d;

/*  Constructor.  */
ColorBlockStateInfo::ColorBlockStateInfo(ROPBlockState *state, u32bit numBlocks)
{

    /*  Set state memory pointer.  */
    stateMemory = state;

    /*  Set number of blocks.  */
    blocks = numBlocks;

    setTag("CBStIn");
}


/*  Gets the pointer to the state memory.  */
ROPBlockState *ColorBlockStateInfo::getColorBufferState()
{
    return stateMemory;
}


/*  Gets the number of blocks transmited.  */
u32bit ColorBlockStateInfo::getNumBlocks()
{
    return blocks;
}

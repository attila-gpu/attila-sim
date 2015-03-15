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
 * $RCSfile: TileEvaluatorStateInfo.cpp,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:57 $
 *
 * Tile Evaluator State Info implementation file.
 *
 */

/**
 *
 *  @file TileEvaluatorStateInfo.cpp
 *
 *  This file implements the TileEvaluatorStateInfo class.
 *
 *  The TileEvaluatorStateInfo class carries state information
 *  from the Tile Evaluators to the Tile FIFO.
 *
 */


#include "TileEvaluatorStateInfo.h"

using namespace gpu3d;

/*  Creates a new TileEvaluatorStateInfo object.  */
TileEvaluatorStateInfo::TileEvaluatorStateInfo(TileEvaluatorState newState) : 

    state(newState)
{
    /*  Set color for tracing.  */
    setColor(state);
}


/*  Returns the tile evaluator state carried by the object.  */
TileEvaluatorState TileEvaluatorStateInfo::getState()
{
    return state;
}

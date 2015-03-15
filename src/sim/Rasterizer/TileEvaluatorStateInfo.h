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
 * $RCSfile: TileEvaluatorStateInfo.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:57 $
 *
 * Tile Evaluator State Info definition file.
 *
 */

/**
 *
 *  @file TileEvaluatorStateInfo.h
 *
 *  This file defines the TileEvaluatorStateInfo class.
 *
 *  The Tile Evaluator State Info class is used to carry state information 
 *  from the Tile Evaluators to the Tile FIFO.
 *
 */

#ifndef _TILEEVALUATORSTATEINFO_
#define _TILEEVALUATORSTATEINFO_

#include "DynamicObject.h"
#include "TileEvaluator.h"

namespace gpu3d
{

/**
 *
 *  This class defines a container for the state signals
 *  that the Tile Evaluator boxes send to the Tile FIFO box.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class TileEvaluatorStateInfo : public DynamicObject
{
private:
    
    TileEvaluatorState state;   /**<  The Tile Evaluator state.  */

public:

    /**
     *
     *  Creates a new TileEvaluatorStateInfo object.
     *
     *  @param state The Tile Evaluator state carried by this
     *  Tile Evaluator state info object.
     *
     *  @return A new initialized TileEvaluatorStateInfo object.
     *
     */
     
    TileEvaluatorStateInfo(TileEvaluatorState state);
    
    /**
     *
     *  Returns the Tile Evaluator state carried by the Tile Evaluator
     *  state info object.
     *
     *  @return The Tile Evaluator state carried in the object.
     *
     */
     
    TileEvaluatorState getState();
};

} // namespace gpu3d

#endif

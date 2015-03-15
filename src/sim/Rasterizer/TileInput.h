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
 * $RCSfile: TileInput.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:57 $
 *
 * Tile Input definition file. 
 *
 */

#ifndef _TILEINPUT_

#define _TILEINPUT_

#include "support.h"
#include "GPUTypes.h"
#include "DynamicObject.h"
#include "Tile.h"

namespace gpu3d
{

/**
 * 
 *  @file TileInput.h 
 *
 *  This file defines the Tile Input class.
 *
 */
  
/**
 *
 *  This class defines objects that carry tiles for recursive
 *  rasterization between the Tile FIFO and the Tile Evaluators.
 *
 *  This class inherits from the DynamicObject class that
 *  offers basic dynamic memory and signal tracing support
 *  functions.
 *
 */
 
class TileInput : public DynamicObject
{

private:

    u32bit triangleID;          /**<  Triangle identifier.  */
    u32bit setupID;             /**<  Setup triangle identifier.  */
    Tile *tile;                 /**<  Pointer to the tile being carried.  */
    bool end;                   /**<  Last tile generated from the parent tile.  */
    bool last;                  /**<  Last triangle signal.  */

public:

    /**
     *
     *  Tile Input constructor.
     *
     *  Creates and initializes a tile input.
     *
     *  @param id The triangle identifier being rasterized in the tile.
     *  @param setupID The setup triangle identifier.
     *  @param tile Pointer to the tile being carried for evaluation.
     *  @param end Last generated tile from the input parent tile.
     *  @param last If it is the last triangle/tile for the batch.
     *
     *  @return An initialized tile input.
     *
     */
     
    TileInput(u32bit id, u32bit setupID, Tile *tile, bool end, bool last);
    
    /**
     *
     *  Gets the triangle identifier.
     *
     *  @return The triangle identifier.
     *
     */

    u32bit getTriangleID();
    
    /**
     *
     *  Gets the setup triangle identifier.
     *
     *  @return The setup triangle identifier.
     *
     */

    u32bit getSetupTriangleID();
    
    /**
     *
     *  Gets the tile being carried by the Tile Input object.
     *
     *  @return The pointer to the Tile being carried by the
     *  Tile Input object.
     *
     */
     
    Tile *getTile();
    
    /**
     *
     *  Asks if the current tile is the last tile generated from
     *  its parent tile.
     *
     *  @return If the current tile is the last generated from the
     *  parent tile.
     *
     */
     
    bool isEndTile();
    
    /**
     * 
     *  Asks if the last triangle flag is enabled for this triangle.
     *
     *  @return If the last triangle signal is enabled.
     *
     */
    
    bool isLast();
    
};

} // namespace gpu3d

#endif

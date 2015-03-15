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
 * $RCSfile: TileInput.cpp,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:57 $
 *
 * Tile Input definition file. 
 *
 */

/**
 *
 *  @file TileInput.cpp
 *
 *  This file implements the Tile Input class.
 *
 */
 
#include "TileInput.h"

using namespace gpu3d;

/*  Creates a new TileInput.  */
TileInput::TileInput(u32bit ID, u32bit setID, Tile *t, bool endTile,
    bool lastTriangle)
{
    /*  Set vertex parameters.  */
    triangleID = ID;
    setupID = setID; 
    tile = t;
    end = endTile;
    last = lastTriangle;
}

/*  Gets the triangle identifier.  */
u32bit TileInput::getTriangleID()
{
    return triangleID;
}


/*  Gets the setup triangle identifier.  */
u32bit TileInput::getSetupTriangleID()
{
    return setupID;
}

/*  Gets the tile being carried.  */
Tile *TileInput::getTile()
{
    return tile;
}

/*  Returns if it is the last tile from the input parent tile.  */
bool TileInput::isEndTile()
{
    return end;
}

/*  Returns if it is a valid tile.  Marks the end of a parent tile without
    any */

/*  Gets if it is the last triangle/tile in the pipeline.  */
bool TileInput::isLast()
{
    return last;
}

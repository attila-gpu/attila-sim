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
 * $RCSfile: Tile.h,v $
 * $Revision: 1.4 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:41 $
 *
 * Tile definition file.
 *
 */

/**
 *
 *  @file Tile.h
 *
 *  Defines the Tile class.
 *
 *  This class describes and stores the data associated with
 *  a 2D tile inside the 3D graphic pipeline.
 *
 *
 */

#ifndef _TILE_

#define _TILE_

#include "GPUTypes.h"
#include "SetupTriangle.h"
#include "OptimizedDynamicMemory.h"

namespace gpu3d
{

/**
 *
 *  Defines the maximum number triangles supported for evaluation in a tile.
 *
 */

static const u32bit MAX_TRIANGLES = 32;

/**
 *
 *  Defines a 2D tile of fragments.
 *
 *  This class stores information about tiles of the viewport
 *  that are evaluated in the rasterization stage of 3D GPU
 *  simulator.
 *
 *  This class inherits from the OptimizedDynamicMemory object that provides custom optimized
 *  memory allocation and deallocation support
 *
 */

class Tile : public OptimizedDynamicMemory
{

private:

    u32bit x;                           /**<  Tile start x position.  */
    u32bit y;                           /**<  Tile start y position.  */
    u32bit level;                       /**<  Tile level/size.  */
    f64bit sEq[MAX_TRIANGLES * 3];      /**<  Storage for the three edge equation values per triangle.  */
    f64bit *edgeEq[MAX_TRIANGLES];      /**<  Three edge equations value at the tile start position for the triangles being evaluated.  */
    f64bit zEq[MAX_TRIANGLES];          /**<  z/w equation value at the tile start point for the triangles being evaluated.  */
    SetupTriangle *triangle[MAX_TRIANGLES]; /**<  Pointer to the triangle or triangles to be evaluated in the tile.  */
    u32bit numTriangles;                /**<  Number of triangles being evaluated.  */
    bool inside[MAX_TRIANGLES];         /**<  Stores if the tile is inside each of the triangles being rasterized.  */
    u32bit nextTriangle;                /**<  Stores the next triangle for which to generate fragments.  */

public:

    /**
     *
     *  Tile constructor.
     *
     *  Creates and initializes a new Tile.
     *
     *  @param triangle Pointer to the triangle(s) to be evaluated/rasterized in the tile.
     *  @param x Start horizontal coordinate of the tile inside the viewport.
     *  @param y Start vertical coordinate of the tile inside the viewport.
     *  @param edgeq Array with the value of the triangle edge equations at the
     *  tile start point.
     *  @param zeq Value of the z/w equation at the tile start point.
     *  @param level Level/Size of the tile inside the viewport.
     *
     *  @return A new initialized Tile.
     *
     */

    Tile(SetupTriangle *triangle, u32bit x, u32bit y, f64bit *edgeq, f64bit zeq,
        u32bit level);

    /**
     *
     *  Tile constructor.
     *
     *  Creates and initializes a new Tile.
     *
     *  @param triangle Pointer to an array of triangles to be evaluated/rasterized in the tile.
     *  @param numTriangles Number of triangles to evaluate in the tile.
     *  @param x Start horizontal coordinate of the tile inside the viewport.
     *  @param y Start vertical coordinate of the tile inside the viewport.
     *  @param edgeq Array of float point arrays with the value of the triangle edge equations at the
     *  tile start point for each triangle being evaluated.
     *  @param zeq Array with the value of the z/w equation at the tile start point for each triangle
     *  being evaluated.
     *  @param level Level/Size of the tile inside the viewport.
     *
     *  @return A new initialized Tile.
     *
     */

    Tile(SetupTriangle **triangle, u32bit numTriangles, u32bit x, u32bit y, f64bit **edgeq, f64bit *zeq,
        u32bit level);

    /**
     *
     *  Tile copy constructor.
     *
     *  @param input Reference to the source Tile to be copied into the new Tile.
     *
     *  @return A new initialized Tile that is a copy of the source Tile.
     *
     */

    Tile(const Tile &input);

    /**
     *
     *  Tile destructor.
     *
     */

    ~Tile();

    /**
     *
     *  Return the tile start horizontal position.
     *
     *  @return The tile start horizontal position.
     *
     */

    u32bit getX();

    /**
     *
     *  Returns the tile start vertical position.
     *
     *  @return The tile start vertical position.
     *
     */

    u32bit getY();

    /**
     *
     *  Gets the pointer to the Setup Triangle (first) that is being evaluated/rasterized in the current tile.
     *
     *  @return Pointer to the (first) setup triangle being evaluated/rasterized in the tile.
     *
     */

    SetupTriangle *getTriangle();

    /**
     *
     *  Gets the pointer to the specified setup triangle that is being evaluated/rasterized in the current tile.
     *
     *  @param id Identifier (order/position) of the triangle requested.
     *
     *  @return Pointer to the setup triangle being evaluated/rasterized in the tile.
     *
     */

    SetupTriangle *getTriangle(u32bit id);

    /**
     *
     *  Gets the value of the triangle edge equations at the tile start point of the first triangle being evaluated.
     *
     *  @return A pointer to the value of the triangle edge equations at the tile start point.
     *
     */

    f64bit *getEdgeEquations();

    /**
     *
     *  Gets the value of the triangle edge equations at the tile start point of the specified triangle being evaluated.
     *
     *  @param id Triangle identifier inside the tile which edge equations are requested.
     *
     *  @return A pointer to the value of the triangle edge equations at the tile start point.
     *
     */

    f64bit *getEdgeEquations(u32bit id);

    /**
     *
     *  Gets the value of the z/w triangle equation at the tile start point for the first triangle in the tile.
     *
     *  @return The value of the z/w equation at the tile start point.
     *
     */

    f64bit getZEquation();

    /**
     *
     *  Gets the value of the z/w triangle equation at the tile start point for the specified triangle in the tile.
     *
     *  @param id Identifier of the triangle inside the tile which z equation value is requested.
     *
     *  @return The value of the z/w equation at the tile start point.
     *
     */

    f64bit getZEquation(u32bit id);


    /**
     *
     *  Get the current tile size/level.
     *
     *  @return The tile size/level.
     *
     */

    u32bit getLevel();

    /**
     *
     *  Get the number of triangles being evaluated in the tile.
     *
     *  @eturn The number of triangles being evaluated in the tile.
     *
     */

    u32bit getNumTriangles();

    /**
     *
     *  Get the vector with the tile inside triangle flags.
     *
     *  @return Pointer to the vector with the tile inside triangle flag.
     *
     */

    bool *getInside();

    /**
     *
     *  Get identifier of the next triangle in the tile for which to generate
     *  fragments.
     *
     *  @return Identifier of the next triangle in the tile for which to generate
     *  fragments.
     *
     */

    u32bit getNextTriangle();

    /**
     *
     *  Sets the identifier of the next triangle in the tile for which to generate
     *  fragments.
     *
     *  @param next Identifier of the next triangle for which fragments will be
     *  generated.
     *
     */

    void setNextTriangle(u32bit next);
};

} // namespace gpu3d

#endif

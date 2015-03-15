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
 * $RCSfile: Fragment.h,v $
 * $Revision: 1.10 $
 * $Author: vmoya $
 * $Date: 2007-09-21 14:56:32 $
 *
 * Fragment definition file.
 *
 */

/**
 *
 *  @file Fragment.h
 *
 *  Defines the Fragment class.
 *
 *  This class describes and stores the data associated with
 *  fragments inside the 3D graphic pipeline.
 *
 *
 */

#include "GPUTypes.h"
#include "support.h"
#include "SetupTriangle.h"
#include "OptimizedDynamicMemory.h"
#include "GPU.h"

#ifndef _FRAGMENT_

#define _FRAGMENT_

namespace gpu3d
{

/**
 *
 *  Defines an object that identifies a tile in the framebuffer.
 *
 */
class TileIdentifier
{
private:

    s32bit x;   /**<  Tile x coordinate inside the framebuffer (in tiles).  */
    s32bit y;   /**<  Tile y coordinate inside the framebuffer (in tiles).  */

public:

    /**
     *
     *  TileIdentifier constructor.
     *
     *  @param x The tile x coordinate in the framebuffer in tiles.
     *  @param y The Tile y coordinate in the framebuffer in tiles.
     *
     *  @return A new TileIdentifier object.
     *
     */

    TileIdentifier(s32bit x = 0, s32bit y = 0);

    /**
     *
     *  Gets the tile x coordinate.
     *
     *  @return The tile x coordinate.
     *
     */

    s32bit getX() const;

    /**
     *
     *  Gets the tile y coordinate.
     *
     *  @return The tile y coordinate.
     *
     */

    s32bit getY() const;

    /**
     *
     *  Redefines the equality operator for TileIdentifier objects.
     *
     *  @param a TileIdentifier object which with to compare for equality.
     *
     *  @return If the current TileIdentifier object is equal to the passed
     *  TileIdentifier object.
     *
     */

    bool operator==(TileIdentifier a);

    /**
     *
     *  Redefines the inequality operator for TileIdentifier objects.
     *
     *  @param a TileIdentifier object which with to compare for inequality.
     *
     *  @return If the current TileIdentifier object is not equal to the passed
     *  TileIdentifier object.
     *
     */

    bool operator!=(TileIdentifier a);

    /**
     *
     *  Assigns the tile to a processing based on the tile horizontal coordinate.
     *
     *  @param numUnits Number of available processing units to which the tile
     *  can be assigned.
     *
     *  @return The assigned processing unit based on the tile horizontal
     *  coordinate.
     *
     */

    u32bit assignOnX(u32bit numUnits);

    /**
     *
     *  Assigns the tile to a processing based on the tile vertical coordinate.
     *
     *  @param numUnits Number of available processing units to which the tile
     *  can be assigned.
     *
     *  @return The assigned processing unit based on the tile Vertical
     *  coordinate.
     *
     */

    u32bit assignOnY(u32bit numUnits);

    /**
     *
     *  Assigns the tile to a processing interleaving in the processing
     *  units.
     *
     *  @param numUnits Number of available processing units to which the tile
     *  can be assigned.
     *
     *  @return The assigned processing unit.
     *
     */

    u32bit assignInterleaved(u32bit numUnits);

    /*
     *  Assigns tile to a processing unit interleaved in morton/Z orden.
     *
     *  @param numUnits Number of available processing units to which the tile
     *  can be assigned.
     *
     *  @return The assigned processing unit.
     *
     */

    u32bit assignMorton(u32bit numUnits);

};

/**
 *
 *  Defines a Fragment and its associated data.
 *
 *  This class carries the information associated with a fragment
 *  along the rasterization and fragment stages of a hardware 3D
 *  graphic pipeline.
 *
 *  This class inherits from the OptimizedDynamicMemory object that provides custom optimized
 *  memory allocation and deallocation support
 *
 */

class Fragment : public OptimizedDynamicMemory
{

private:

    s32bit x;               /**<  The fragment x coordinate in screen space.  */
    s32bit y;               /**<  The fragment y coordinate in screen space.  */
    u32bit z;               /**<  The fragment z coordinate in screen space.  */
    SetupTriangle *triangle;/**<  Pointer to the Triangle that generated the fragment.  */
    f64bit coordinates[3];  /**<  The fragment edge or barycentric coordinates inside the triangle.  */
    f64bit zw;              /**<  Stores the calculated z/w for near/far plane clipping.  */
    bool insideTriangle;    /**<  Keeps if the fragment is inside the triangle (for tiled rasterization).  */
    bool isLast;            /**<  Keeps if the fragment is the last fragment in the triangle.  */

    u32bit msaaZ[MAX_MSAA_SAMPLES];         /**<  Z samples for multisampling antialiasing.  */
    bool msaaCoverage[MAX_MSAA_SAMPLES];    /**<  Stores the coverage flags for the MSAA samples.  */

public:

    /**
     *
     *  Fragment constructor.
     *
     *  Creates and initializes a fragment in (x, y) screen space coordinates
     *  for a given setup triangle.
     *
     *  @param triangle Pointer to the setup triangle for which to generate the fragment.
     *  @param x X coordinate in screen space of the triangle fragment to generate.
     *  @param y Y coordinate in screen space of the triangle fragment to generate.
     *  @param z Fragment depth (z/w).
     *  @param coord The edge/barycentric fragment coordinates and the z/w for the fragment.
     *  @param inside Boolean value that sets if the fragment is culled (inside triangle and
     *  clip space).
     *
     *  @return A new initialized fragment for the given triangle and
     *  screen coordinates.
     *
     */

    Fragment(SetupTriangle *triangle, s32bit x, s32bit y, u32bit z, f64bit *coord, bool inside);

    /**
     *
     *  Fragment constructor (simplified version to construct bypass fragments).
     *
     *  Creates and initializes a fragment in (x, y) screen space coordinates
     *
     *  @param x X coordinate in screen space of the triangle fragment to generate.
     *  @param y Y coordinate in screen space of the triangle fragment to generate.
     *
     *  @return A new initialized fragment for the given screen coordinates.
     *
     */

    Fragment(s32bit x, s32bit y);

    /**
     *
     *  Fragment destructor.
     *
     */

    ~Fragment();

    /**
     *
     *  Get the fragment coordinates inside the triangle (edge
     *  coordinates or barycentric coordinates).
     *
     *  @return The fragment coordinates inside the triangle.
     *
     */

    f64bit *getCoordinates();

    /**
     *
     *  Get the z/w interpolated value for the fragment.
     *
     *  @return The z/w interpolated value.
     *
     */
     
    const f64bit getZW();

    /**
     *
     *  Indicates if the fragment is inside the triangle.
     *
     *  @return If the fragment is inside the triangle.
     *
     */

    const bool isInsideTriangle();

    /**
     *
     *  Gets the fragment x raster coordinate.
     *
     *  @return The fragment x raster coordinate
     *
     */

    const s32bit getX();


    /**
     *
     *  Gets the fragment y raster coordinate.
     *
     *  @return The fragment y raster coordinate.
     *
     */

    const s32bit getY();

    /**
     *
     *  Gets the fragment depth (z/w).
     *
     *  @return The fragment depth.
     *
     */

    const u32bit getZ();

    /**
     *
     *  Sets the fragment depth (z/w).
     *
     *  @param zValue The fragment depth value.
     *
     */
    void setZ(u32bit zValue);

    /**
     *
     *  Gets the pointer to the setup triangle from which the fragment
     *  was generated.
     *
     *  @return A pointer to the fragment triangle.
     *
     */

    SetupTriangle *getTriangle();

    /**
     *
     *  Sets the fragment as the last triangle fragment.
     *
     *
     */

    void lastFragment();

    /**
     *
     *  Asks if it is the last fragment in the triangle
     *
     *  @return If the fragment is the last fragment in the
     *  triangle.
     *
     */

    const bool isLastFragment();

    /**
     *
     *  Determines if a fragment is inside the three
     *  triangle edge equations.
     *
     *  @return If the fragment is inside the triangle edge equations.
     *
     */

    //  NOTE:  Moved to RasterizerEmulator.
    //
    //bool insideEdgeEquations();


    /**
     *
     *  Sets the z samples for multisampling antialiasing.
     *
     *  @param samples Number of samples computed for the fragment.
     *  @param msaaZ An array with the fragment z samples for MSAA.
     *  @param coverage An array storing for each MSAA sample if it's covered by the triangle.
     *  @param centroid Sample coordinates computed with the centroid sampling method.
     *  @param anyInside A boolean value that stores if any of the MSAA samples of the fragment
     *  is inside the triangle.
     *
     */
     
     void setMSAASamples(u32bit samples, u32bit *msaaZ, bool *coverage, f64bit *centroid, bool anyInside);
     
    /**
     *
     *  Gets a pointer to an array storing the z samples for multisampling antialising.
     *
     *  @return A pointer to the array storing the fragment Z samples for MSAA.
     *
     */
     
    u32bit *getMSAASamples();

    /**
     *
     *  Gets a pointer to an array storing the coverage flags for z samples for multisampling antialising.
     *
     *  @return A pointer to the array storing coverage information about the fragment MSAA samples
     *
     */
     
    bool *getMSAACoverage();
     
     
}; // Fragment

} // namespace gpu3d

#endif

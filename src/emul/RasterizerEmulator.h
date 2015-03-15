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
 * $RCSfile: RasterizerEmulator.h,v $
 * $Revision: 1.14 $
 * $Author: vmoya $
 * $Date: 2006-11-03 15:19:22 $
 *
 * Rasterizer Emulator definition file.
 *
 */

/**
 *
 *  @file RasterizerEmulator.h
 *
 *  Defines the RasterizerEmulator class.
 *  This class provides services to emulate the functional behaviour
 *  Rasterizer unit.
 *
 *
 */

#include "GPUTypes.h"
#include "support.h"
#include "GPU.h"
#include "SetupTriangle.h"
#include "Fragment.h"
#include "Tile.h"

#ifndef _RASTERIZEREMULATOR_

#define _RASTERIZEREMULATOR_

namespace gpu3d
{

/**
 *
 *  Defines the fragment width/height threshold for the scan tile that determines which method
 *  is used for scanning the triangle (tile based or line based).
 *
 */

//#define SCAN_METHOD_THRESHOLD 1

/**
 *
 *  Defines the maximum number of stacked tile levels for recursive
 *  rasterization (for 1 tile evaluation = log(max(vp_width, vp_height))) + 1.
 *
 */

static const u32bit MAX_LEVELS = 13;

/**
 *
 *  Defines the number of tile testers available per cycle.  Those testers
 *  can only work on tiles of the same level and triangle/batch and they produce
 *  as much as 4x subtiles.
 *
 */

//#define TILE_TESTERS 4
static const u32bit TILE_TESTERS = 4;


/**
 *
 *  This class implements the emulator for a Rasterization hardware
 *  unit.
 *
 *  This class offers functions for performing the triangle setup,
 *  triangle traversal, fragment generation and fragment attribute
 *  interpolation operations.
 *
 */

class RasterizerEmulator
{
private:

    /*  Parameters.  */
    u32bit activeTriangles;         /**<  Number of triangles active inside the raterizer.  */
    u32bit fragmentAttributes;      /**<  Number of fragment attributes.  */
    u32bit scanTileWidth;           /**<  Width of the triangle scan tile.  */
    u32bit scanTileHeight;          /**<  Height of the triangle scan tile.  */
    u32bit scanOverTileWidth;       /**<  Width of the triangle scan over tile (second tile level).  */
    u32bit scanOverTileHeight;      /**<  Height of the triangle scan over tile (second tile level).  */
    u32bit genTileWidth;            /**<  Generation tile width.  */
    u32bit genTileHeight;           /**<  Generation tile height.  */
    bool   useBBOptimization;       /**<  Use the Bounding Box optimization pass.  */
    u32bit subPixelPrecision;       /**<  Number of bits for the subpixel decimal representation in fixed-point.  */ 

    /*  Triangle Storage.  */
    SetupTriangle **setupTriangles; /**<  Stores the triangles in setup process or already stored.  */
    u32bit triangles;               /**<  Number of setup triangles stored.  */
    u32bit *freeSetupList;          /**<  Free setup triangle entry list.  */
    u32bit nextFreeSetup;           /**<  Next free entry in the setup triangle list.  */
    u32bit freeSetups;              /**<  Number of free setup entries.  */

    /*  Storage for tiled generation.  */
    Tile ***trGenTiles;             /**<  Stores the current generation tiles for each setup triangle.  */
    Tile ***trStamps;               /**<  Stores the current stamps for each setup triangle.  */
    Fragment ***trFragments;        /**<  Stores the current generated fragments for each setup triangle.  */
    u32bit **frTriangleID;          /**<  Identifier of the triangle that generated the fragment.  */
    u32bit *trStoredFragments;      /**<  Number of stored fragments per setup triangle.  */
    u32bit *trStoredGenTiles;       /**<  Number of stored generation tiles per setup triangle.  */
    u32bit *trStoredStamps;         /**<  Number of stored stamps for a setup triangle.  */

    /*  Storage for recursive evaluation.  */
    u32bit level;                       /**<  Current recursive level for the triangle.  */
    Tile *testTiles[MAX_LEVELS][4 * TILE_TESTERS];  /**<  Subtiles to evaluate for the triangle and recursive level.  */
    Tile *scanTiles[4 * 4 * TILE_TESTERS];          /**<  Stores scan level tiles.  */
    u32bit nextTile[MAX_LEVELS];        /**<  Next subtile to evaluate for the triangle and recursive level.  */
    u32bit numTiles[MAX_LEVELS];        /**<  Number of subtiles to evaluate for the triangle and recursive level.  */
    u32bit triangleBatch[MAX_TRIANGLES];/**<  Stores the triangle batch being rasterized.  */
    u32bit batchSize;                   /**<  Size of the triangle batch being rasterized.  */

    /*  Viewport.  */
    bool d3d9PixelCoordinates;      /**<  Use D3D9 pixel coordinates convention -> top left is (0, 0).  */
    s32bit x0;                      /**<  Viewport start x coordinate.  */
    s32bit y0;                      /**<  Viewport start y coordinate.  */
    u32bit w;                       /**<  Viewport width in pixels.  */
    u32bit h;                       /**<  Viewport height in pixels.  */
    f32bit n;                       /**<  Depth range near value.  */
    f32bit f;                       /**<  Depth range far value.  */
    bool d3d9DepthRange;            /**<  Use D3D9 depth range in clip space [0, 1].  */

    /*  Scissor.  */
    u32bit windowWidth;             /**<  OpenGL window horizontal resolution.  */
    u32bit windowHeight;            /**<  OpenGL window vertical resolution.  */
    s32bit scissorX0;               /**<  Scissor start x coordinate (left).  */
    s32bit scissorY0;               /**<  Scissor start y coordinate (bottom).  */
    u32bit scissorW;                /**<  Scissor width in pixels.  */
    u32bit scissorH;                /**<  Scissor height in pixels.  */

    /*  Polygon offset.  */
    f32bit slopeFactor;             /**<  Depth factor offset based on the triangle z slope.  */
    f32bit unitOffset;              /**<  Depth unit offset based on the depth precission granurality.  */

    /*  Z Buffer.  */
    u32bit depthBitPrecission;      /**<  Z Buffer bit precission (usually 16 or 24).  */

    /*  Front face mode.  */
    FaceMode faceMode;              /**<  Front face selection mode.  */

    //  Rasterization rules.
    bool useD3D9RasterizationRules; /**<  Use D3D9 rasterization rules.  */
    
    /*  Precalculated constants.  */
    u32bit genTileFragments;        /**<  Fragments per generation tile.  */
    u32bit scanTileGenTiles;        /**<  Generation tiles per scan tile.  */
    u32bit genLevel;                /**<  Level (power of 2) of the generation tile dimension (2^genLevel * 2^genLevel).  */
    u32bit scanLevel;               /**<  Level (power of 2) of the scan tile dimension (2^scanLevel * 2^scanLevel).  */

    //  Fixed point subpixel limits of the current MSAA sampling box (microtriangle generation optimization).
    FixedPoint sampleBBXMin;
    FixedPoint sampleBBYMin;
    FixedPoint sampleBBXMax;
    FixedPoint sampleBBYMax;


    //  Defines an struct that stores the offsets for a MSAA sample position
    struct MSAAOffset
    {
        f64bit xOff;        /**<  Horizontal offset of the MSAA sample position (from the fragment start point in OpenGL).  */
        f64bit yOff;        /**<  Vertical offset of the MSAA sample position (from the fragment start point in OpenGL).  */
    };
    
    /*  Private funtions.  */

    /**
     *
     *  Scans the scan tile to the left from the current position.
     *
     *  @param edgeC Pointer to the three edge and z interpolation
     *  equation values at the current rasterization position.  It is
     *  updated with the value at the scan tile start point (bottom left) at
     *  the left from the current one.
     *  @param edge1 Pointer to the triangle first edge equation coefficientes.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param offset Pointer to the offset to add for the correct
     *  rasterization of thin triangles.
     *  @param x Current horizontal rasterization coordinate.
     *  @param leftBound Left bound from the triangle bounding box.
     *  @param extended Performs an extended scan (2x) of the left-up area.
     *
     *  @return TRUE if the scan tile to the left has a fragmnt inside
     *  the triangle.  FALSE if no scan tile fragment is inside the triangle.
     *
     */

    bool scanTileLeft(f64bit *edgeC, f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit leftBound,bool extended = FALSE);

    /**
     *
     *  Scans the scan tile to the right from the current position.
     *
     *  @param edgeC Pointer to the three edge and z interpolation
     *  equation values at the current rasterization position.  It is
     *  updated with the value at the scan tile start point (bottom left) at
     *  the right from the current one.
     *  @param edge1 Pointer to the triangle first edge equation coefficientes.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param offset Pointer to the offset to add for the correct
     *  rasterization of thin triangles.
     *  @param x Current horizontal rasterization coordinate.
     *  @param rightBound Right bound from the triangle bounding box.
     *  @param extended Performs an extended scan (2x) of the right-down area.
     *
     *  @return TRUE if the scan tile to the right has a fragment inside
     *  the triangle.  FALSE if no scan tile fragment is inside the triangle.
     *
     */

    bool scanTileRight(f64bit *edgeC, f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit rightBound, bool extended = FALSE);

    /**
     *
     *  Scans the scan tile over the current position.
     *
     *  @param edgeC Pointer to the three edge and z interpolation
     *  equation values at the current rasterization position.  It is
     *  updated with the value at the scan tile start point (bottom left)
     *  over the current one.
     *  @param edge1 Pointer to the triangle first edge equation coefficientes.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param offset Pointer to the offset to add for the correct
     *  rasterization of thin triangles.
     *  @param y Current vertical rasterization coordinate.
     *  @param topBound Top bound from the triangle bounding box.
     *  @param extended Performs an extended scan (2x) of the up-right area.
     *
     *  @return TRUE if the scan tile over the current has a fragment inside
     *  the triangle.  FALSE if no scan tile fragment is inside the triangle.
     *
     */

    bool scanTileUp(f64bit *edgeC, f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit y, s32bit topBound, bool extended = FALSE);

    /**
     *
     *  Scans the scan tile below the current position.
     *
     *  @param edgeC Pointer to the three edge and z interpolation
     *  equation values at the current rasterization position.  It is
     *  updated with the value at the scan tile start point (bottom left)
     *  below the current one.
     *  @param edge1 Pointer to the triangle first edge equation coefficientes.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param offset Pointer to the offset to add for the correct
     *  rasterization of thin triangles.
     *  @param y Current vertical rasterization coordinate.
     *  @param bottomBound Bottom bound from the triangle bounding box.
     *  @param extended Performs an extended scan (2x) of the down-left area.
     *
     *  @return TRUE if the scan tile below has a fragment inside
     *  the triangle.  FALSE if no scan tile fragment is inside the triangle.
     *
     */

    bool scanTileDown(f64bit *edgeC, f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit y, s32bit bottomBound, bool extended = FALSE);


    /**
     *
     *  Tries to restore a left saved over tile position.
     *
     *  @param triangle Pointer to the setup triangle which saved position
     *  must be restored.
     *
     *  @return TRUE if the left saved over tile position was restored.  FALSE
     *  if there is no left saved position to restore.
     *
     */

    bool restoreTileLeft(SetupTriangle *triangle);

    /**
     *
     *  Tries to restore a right saved over tile position.
     *
     *  @param triangle Pointer to the setup triangle which saved position
     *  must be restored.
     *
     *  @return TRUE if the right saved over tile position was restored.  FALSE
     *  if there is no right saved position to restore.
     *
     */

    bool restoreTileRight(SetupTriangle *triangle);

    /**
     *
     *  Tries to restore a up saved over tile position.
     *
     *  @param triangle Pointer to the setup triangle which saved position
     *  must be restored.
     *
     *  @return TRUE if the up saved over tile position was restored.  FALSE
     *  if there is no up saved position to restore.
     *
     */

    bool restoreTileUp(SetupTriangle *triangle);

    /**
     *
     *  Tries to restore a down saved over tile position.
     *
     *  @param triangle Pointer to the setup triangle which saved position
     *  must be restored.
     *
     *  @return TRUE if the down saved over tile position was restored.  FALSE
     *  if there is no down saved position to restore.
     *
     */

    bool restoreTileDown(SetupTriangle *triangle);


    /**
     *
     *  Tries to restore a left saved scan tile position.
     *
     *  @param triangle Pointer to the triangle for which to restore
     *  the saved position.
     *
     *  @return TRUE if the saved position could be restored.
     *
     */

    bool restoreLeft(SetupTriangle *triangle);

    /**
     *
     *  Tries to restore a right saved scan tile position.
     *
     *  @param triangle Pointer to the triangle for which to restore
     *  the saved position.
     *
     *  @return TRUE if the saved position could be restored.
     *
     */

    bool restoreRight(SetupTriangle *triangle);

    /**
     *
     *  Tries to restore a up saved scan tile position.
     *
     *  @param triangle Pointer to the triangle for which to restore
     *  the saved position.
     *
     *  @return TRUE if the saved position could be restored.
     *
     */

    bool restoreUp(SetupTriangle *triangle);

    /**
     *
     *  Tries to restore a down saved scan tile position.
     *
     *  @param triangle Pointer to the triangle for which to restore
     *  the saved position.
     *
     *  @return TRUE if the saved position could be restored.
     *
     */

    bool restoreDown(SetupTriangle *triangle);

    /**
     *
     *  Sets the position of the next scan tile position for over tile saves.
     *  Priority: left over tile save > right over tile save > up over tile save > down over tile save.
     *
     *  @param triangle Pointer to the triangle for which to restor an over tile save.
     *
     *  @return TRUE if an over tile save position could be restored.
     *
     */

    bool restoreTile(SetupTriangle *triangle);

    /**
     *
     *  Sets the position of the next scan tile to scan/rasterize
     *  Priority: right save > up save > down save > tile saves.
     *
     *  @param triangle Pointer to the triangle for which to determine
     *  the next rasterization position.
     *  @param fr Pointer to a fragment of the current scan tile.  It is
     *  set to last fragment if it is determined that there are no more
     *  fragments to rasterize for the triangle.
     *
     *  @return If any of the saved positions was restored.
     *
     */

    bool nextTileBorder(SetupTriangle *triangle, Fragment *fr);

    /**
     *
     *  Sets the position of the next scan tile to scan/rasterize
     *  Priority: right save > up save > down save > tile saves.
     *
     *  @param triangle Pointer to the triangle for which to determine
     *  the next rasterization position.
     *  @param fr Pointer to a fragment of the current scan tile.  It is
     *  set to last fragment if it is determined that there are no more
     *  fragments to rasterize for the triangle.
     *
     */

    void nextTileRUD(SetupTriangle *triangle, Fragment *fr);

    /**
     *
     *  Sets the position of the next scan tile to scan/rasterize
     *  Priority: up save > down save > tile saves.
     *
     *  @param triangle Pointer to the triangle for which to determine
     *  the next rasterization position.
     *  @param fr Pointer to a fragment of the current scan tile.  It is
     *  set to last fragment if it is determined that there are no more
     *  fragments to rasterize for the triangle.
     *
     */

    void nextTileUD(SetupTriangle *triangle, Fragment *fr);

    /**
     *
     *  Sets the position of the next scan tile to scan/rasterize
     *  Priority: right save > down save > tile saves.
     *
     *  @param triangle Pointer to the triangle for which to determine
     *  the next rasterization position.
     *  @param fr Pointer to a fragment of the current scan tile.  It is
     *  set to last fragment if it is determined that there are no more
     *  fragments to rasterize for the triangle.
     *
     */

    void nextTileRD(SetupTriangle *triangle, Fragment *fr);

    /**
     *
     *  Sets the position of the next scan tile to scan/rasterize
     *  Priority: down save > tile saves.
     *
     *  @param triangle Pointer to the triangle for which to determine
     *  the next rasterization position.
     *  @param fr Pointer to a fragment of the current scan tile.  It is
     *  set to last fragment if it is determined that there are no more
     *  fragments to rasterize for the triangle.
     *
     */

    void nextTileD(SetupTriangle *triangle, Fragment *fr);

    /**
     *
     *  Scans the scan tile over the current and determines if it
     *  must be saved as a scan tile save or as an over tile save (if it is
     *  outside the current over tile and no previous scan tile over the current
     *  over tile was saved).
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param topBound Left bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *  @param extended Use extended probe area.
     *
     */

    void saveUp(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit topBound,
        SetupTriangle *triangle, bool extended = FALSE);

    /**
     *
     *  Scans the scan tile below the current and determines if it
     *  must be saved as a scan tile save or as an over tile save (if it is
     *  outside the current over tile and no previous scan tile below the current
     *  over tile was saved).
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param bottomBound Bottom bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *  @param extended Use extended probe area.
     *
     */

    void saveDown(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit bottomBound,
        SetupTriangle *triangle, bool extended = FALSE);


    /**
     *
     *  Scans the scan tile to the right from the current and determines if it
     *  must be saved as a scan tile save or as an over tile save (if it is
     *  outside the current over tile and no previous scan tile right from the current
     *  over tile was saved).
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param rightBound Right bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *  @param extended Use extended probe area.
     *
     */

    void saveRight(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit rightBound,
        SetupTriangle *triangle, bool extended);


    /**
     *
     *  Scans the scan tile over the current and determines if it
     *  must be saved as a scan tile save.
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param topBound Left bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *  @param extended Use extended probe area.
     *
     */

    void saveUpBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit topBound,
        SetupTriangle *triangle, bool extended = FALSE);

    /**
     *
     *  Scans the scan tile below the current and determines if it
     *  must be saved as a scan tile save.
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param bottomBound Bottom bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *  @param extended Use extended probe area.
     *
     */

    void saveDownBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit bottomBound,
        SetupTriangle *triangle, bool extended = FALSE);


    /**
     *
     *  Scans the scan tile to the right from the current and determines if it
     *  must be saved as a scan tile save.
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param rightBound Right bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *  @param extended Use extended probe area.
     *
     */

    void saveRightBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit rightBound,
        SetupTriangle *triangle, bool extended);

    /**
     *
     *  Scans the stamp to the left from the current.  If the
     *  scan tile is outside the current over tile and no previous scan tile
     *  to the left of the current over tile was saved it is saved
     *  as left over tile save.
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param leftBound Left bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *  @param extended Extended probe area.
     *
     *  @return TRUE if the scan tile to the left has fragments from the
     *  triangle and the scan tile isn't outside the current tile.  FALSE
     *  if there are no fragments in the scan tile to the left or if the
     *  scan tile to the left is outside the current tile.
     *
     */

    bool rasterizeLeft(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit leftBound,
        SetupTriangle *triangle, bool extended = FALSE);

    /**
     *
     *  Scans the scan tile to the right from the current.  If the
     *  scan tile is outside the current over tile and no previous scan tile
     *  to the right of the current over tile was saved it is saved
     *  as left over tile save.
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param rightBound Right bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *  @param extended Extended probe area.
     *
     *  @return TRUE if the scan tile to the right has fragments from the
     *  triangle and the scan tile isn't outside the current over tile.  FALSE
     *  if there are no fragments in the scan tile to the right or if the
     *  scan tile to the right is outside the current over tile.
     *
     */

    bool rasterizeRight(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit rightBound,
        SetupTriangle *triangle, bool extended = FALSE);

    /**
     *
     *  Search the bottom border for triangle fragments.
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param rightBound Right bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *
     *  @return TRUE if the right border hasn't been reached yet.
     *
     */

    bool rasterizeBottomBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit rightBound, SetupTriangle *triangle);

    /**
     *
     *  Search the top border for triangle fragments.
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param leftBound Left bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *
     */

    void rasterizeTopBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit leftBound, SetupTriangle *triangle);

    /**
     *
     *  Search the left border for triangle fragments.
     *
     *  @param edge1 Pointer to the triangle first edge equation coefficients.
     *  @param edge2 Pointer to the triangle second edge equation coefficients.
     *  @param edge3 Pointer to the triangle third edge equation coefficients.
     *  @param zeq Pointer to the Z interpolation equation coefficients.
     *  @param x Current horizontal rasterization position.
     *  @param y Current vertical rasterization position.
     *  @param bottomBound Bottom bound from the triangle bounding box.
     *  @param triangle Setup triangle being rasterized.
     *
     */

    void rasterizeLeftBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3, f64bit *zeq,
        f64bit *offset, s32bit x, s32bit y, s32bit bottomBound, SetupTriangle *triangle);


    /**
     *
     *  Generates nine subtile sample points (4 subtiles) from a tile sample point.
     *
     *  @param tile Tile for which to generate the subtile sample points.
     *  @param triId Identifier of the triangle inside the tile for which to generate the sample
     *  points.
     *  @param s0 Pointer to the first sample point.
     *  @param s1 Pointer to the second sample point.
     *  @param s2 Pointer to the third sample point.
     *  @param s3 Pointer to the fourth sample point.
     *  @param s4 Pointer to the fifth sample point.
     *  @param s5 Pointer to the sixth sample point.
     *  @param s6 Pointer to the seventh sample point.
     *  @param s7 Pointer to the eight sample point.
     *  @param s8 Pointer to the nineth samplepoint.
     *
     */

    void generateSubTileSamples(Tile *tile, u32bit triId, f64bit *s0, f64bit *s1, f64bit *s2, f64bit *s3,
        f64bit *s4, f64bit *s5, f64bit *s6, f64bit *s7, f64bit *s8);

    /**
     *
     *  Generates four subtile sample points that correspond with the four subtile start points
     *  from a tile defined by a start sample point.
     *
     *  @param tile Tile for which to generate the subtile sample points.
     *  @param triId Identifier of the triangle inside the tile for which to generate the sample
     *  points.
     *  @param s0 Pointer to the first sample point.
     *  @param s1 Pointer to the second sample point.
     *  @param s2 Pointer to the third sample point.
     *  @param s3 Pointer to the fourth sample point.
     *
     */

    void generateSubTileSamples(Tile *tile, u32bit triId, f64bit *s0, f64bit *s1, f64bit *s2, f64bit *s3);

    /**
     *
     *  Generates four subtiles from the input tile.
     *
     *  @param tile Input tile to subdivide into lower level tiles.
     *  @param outputTiles Pointer to an array of tile pointers where to store the
     *  generated for subtiles.
     *
     */

    void generateTiles(Tile *tile, Tile **outputTiles);

    /**
     *
     *  Expands a tile into tiles of the given level.
     *
     *  @param tile The input tile to expand.
     *  @param tilesOut Reference to an array of Tile pointers where to store the
     *  generated tiles.
     *  @param level Level of the generated tiles.
     *
     *  @return Number of generated tiles.
     *
     */

    u32bit expandTile(Tile *tile, Tile **&tilesOut, u32bit level);


    /**************************************************************************
     *
     *  Functions no longer supported.  May not work.
     *
     **************************************************************************/

    /**
     *
     *  Probes the pixel to the left from the current rasterization
     *  position.
     *
     *  @param edgeC Pointer to the three edge and z interpolation equation
     *  values at the current rasterization position.  It is updated if the
     *  pixel to the left is inside the triangle zone.
     *  @param a1 Triangle first edge equation x coefficient (a).
     *  @param a2 Triangle second edge equation x coefficient (a).
     *  @param a3 Triangle third edge equation x coefficient (a).
     *  @param az Triangle z interpolation equation x coefficient (a).
     *  @param offset Pointer with the offset to add to the three triangle
     *  edge equations for rasterizing thin triangles.
     *  @param x The current horizontal rasterization position.
     *
     *  @return TRUE if the pixel to left is inside the triangle zone.
     *  FALSE if it is outside.
     *
     */

    bool scanLeft(f64bit *edgeC, f64bit a1, f64bit a2, f64bit a3, f64bit az,
        f64bit *offset, s32bit x);

    /**
     *
     *  Probes the pixel to the right from the current rasterization
     *  position.
     *
     *  @param edgeC Pointer to the three edge and z interpolation equation
     *  values at the current rasterization position.  It is updated if the
     *  pixel to the right is inside the triangle zone.
     *  @param a1 Triangle first edge equation x coefficient (a).
     *  @param a2 Triangle second edge equation x coefficient (a).
     *  @param a3 Triangle third edge equation x coefficient (a).
     *  @param az Triangle z interpolation equation x coefficient (a).
     *  @param offset Pointer with the offset to add to the three triangle
     *  edge equations for rasterizing thin triangles.
     *  @param x The current horizontal rasterization position.
     *
     *  @return TRUE if the pixel to right is inside the triangle zone.
     *  FALSE if it is outside.
     *
     */

    bool scanRight(f64bit *edgeC, f64bit a1, f64bit a2, f64bit a3, f64bit az,
        f64bit *offset, s32bit x);

    /**
     *
     *  Probes the pixel down from the current rasterization
     *  position.
     *
     *  @param edgeC Pointer to the three edge and z interpolation equation
     *  values at the current rasterization position.  It is updated if the
     *  pixel down from the current position is inside the triangle zone.
     *  @param b1 Triangle first edge equation y coefficient (b).
     *  @param b2 Triangle second edge equation y coefficient (b).
     *  @param b3 Triangle third edge equation y coefficient (b).
     *  @param bz Triangle z interpolation equation y coefficient (b).
     *  @param offset Pointer with the offset to add to the three triangle
     *  edge equations for rasterizing thin triangles.
     *  @param y The current horizontal rasterization position.
     *
     *  @return TRUE if the pixel down from the current is inside
     *  the triangle zone.  FALSE if it is outside.
     *
     */

    bool scanDown(f64bit *edgeC, f64bit b1, f64bit b2, f64bit b3, f64bit bz,
        f64bit *offset, s32bit y);


    /**
     *
     *  Probes the pixel up from the current rasterization
     *  position.
     *
     *  @param edgeC Pointer to the three edge and z interpolation equation
     *  values at the current rasterization position.  It is updated if the
     *  pixel up from the current position is inside the triangle zone.
     *  @param b1 Triangle first edge equation y coefficient (b).
     *  @param b2 Triangle second edge equation y coefficient (b).
     *  @param b3 Triangle third edge equation y coefficient (b).
     *  @param bz Triangle z interpolation equation y coefficient (b).
     *  @param offset Pointer with the offset to add to the three triangle
     *  edge equations for rasterizing thin triangles.
     *  @param y The current horizontal rasterization position.
     *
     *  @return TRUE if the pixel up from the current is inside
     *  the triangle zone.  FALSE if it is outside.
     *
     */

    bool scanUp(f64bit *edgeC, f64bit b1, f64bit b2, f64bit b3, f64bit bz,
        f64bit *offset, s32bit y);

    /**
     *
     *  Generates the next fragment of a triangle using scanline
     *  based rasterization.  Old deprecatated version that works
     *  on a fragment basis.
     *
     *  @param triangle The triangle identifier for the triangle
     *  from which to generate the next fragment
     *
     *  @return The generated fragment.
     *
     */

    Fragment *nextScanlineFragment(u32bit triangle);

    /**
     *
     *  Generates the next stamp using a tiled scanline rasterization
     *  algorithm.  Old deprecated version that works on a single stamp basis.
     *
     *  @param triangle The triangle identifier for the triangle
     *  from which to generate the next fragment.
     *
     *  @return A pointer to an array of fragments (stamp).
     *
     */

    Fragment **nextScanlineStampTiledOld(u32bit triangle);

    static MSAAOffset MSAAPatternTable2[];
    static MSAAOffset MSAAPatternTable4[];
    static MSAAOffset MSAAPatternTable6[];
    static MSAAOffset MSAAPatternTable8[];
    

public:

    /**
     *
     *  Rasterizer Emulator constructor.
     *
     *  Creates and initializes a new Rasterizer Emulator.
     *
     *  @param activeTriangles Maximum number of active triangles
     *  inside the Rasterizer Emulator at a given time.
     *  @param fragmentAttributes Maximum number of attributes
     *  per fragment.
     *  @param scanTileW Scan tile width (fragments).
     *  @param scanTileH Scan tile height (fragments).
     *  @param overTileW Scan over tile width (in scan tiles).
     *  @param overTileH Scan over tile height (in scan tiles).
     *  @param genTileW Generation tile width (fragments).
     *  @param genTileH Generation tile height (fragments).
     *  @param useBBOpt Use the Bounding Box optimization pass.
     *  @param subPxBBbit Number of precision bits for the subpixel operations.
     *
     *  @return A new initialized Rasterizer Emulator.
     *
     */

    RasterizerEmulator(u32bit activeTriangles, u32bit fragmentAttributes,
        u32bit scanTileW, u32bit scanTileH, u32bit overTileW, u32bit overTileH, u32bit genTileW, u32bit genTileH,
        bool useBBOpt, u32bit subPxBBbit);

    /**
     *
     *  Sets the rasterization viewport.
     *
     *  @param d3d9PixelCoords Use D3D9 pixel coordinates convention -> top left is (0, 0).
     *  @param x The origin x coordinate of the viewport.
     *  @param y The origin y coordinate of the viewport.
     *  @param w The viewport width (in pixels).
     *  @param h The viewport height (in pixels).
     *
     */

    void setViewport(bool d3d9PixelCoords, s32bit x, s32bit y, u32bit w, u32bit h);

    /**
     *
     *  Sets the rasterization scissor rectangle.
     *
     *
     *  @param wWidth GL Window width.
     *  @param wHeight GL Window height.
     *  @param scissor Enable or disable scissor test.
     *  @param x The origin x coordinate for the scissor rectangle (left).
     *  @param y The origin y coordinate for the scissor rectangle (bottom).
     *  @param w The scissor rectangle width (in pixels).
     *  @param h The scissor rectangle height (in pixels).
     *
     */

    void setScissor(u32bit wWidth, u32bit wHeight, bool scissor, s32bit x, s32bit y, u32bit w, u32bit h);

    /**
     *
     *  Sets the depth range.
     *
     *  @param d3d9DepthRange Use the D3D9 range for depth clip space [0, 1].
     *  @param n Near depth range (must be clamped to [0..1]!).
     *  @param f Far depth range (must be clamped to [0..1]!).
     *
     */

    void setDepthRange(bool d3d9DepthRange, f32bit n, f32bit f);

    /**
     *
     *  Sets the polygon offset.
     *
     *  @param factor Depth offset as a factor of the triangle z slope.
     *  @param unit Depth offset in depth precission units.
     *
     */

    void setPolygonOffset(f32bit factor, f32bit unit);

    /**
     *
     *  Sets the Z/Depth buffer bit precission.
     *
     *  @param zPrecission The Depth Buffer bit precission.
     *
     */

    void setDepthPrecission(u32bit zPrecission);

    /**
     *
     *  Sets the face mode that determines which triangles are front facing
     *  based on its vertices order.
     *
     *  @param mode Front face selection mode.
     *
     */

    void setFaceMode(FaceMode mode);

    /**
     *
     *  Sets if the D3D9 rasterization rules will be used by the rasterizer emulator.
     *
     *  @param d3d9RasterizationRules Boolean value that defines if the D3D9 rules for
     *  rasterization will be used.
     *
     */
     
    void setD3D9RasterizationRules(bool d3d9RasterizationRules);
         
    /**
     *
     *  Setups a new triangle using the 2DH rasterization method.
     *
     *  Performs the triangle setup operations for a triangle defined
     *  by the three passed vertices.
     *
     *  @param vertex1 The triangle first vertex attribute array.
     *  @param vertex2 The triangle second vertex attribute array.
     *  @param vertex3 The triangle third vertex attribute array.
     *
     *  @return An identifier for the the new setup triangle.
     *
     */

    u32bit setup(QuadFloat *vertex1, QuadFloat *vertex2, QuadFloat *vertex3);

    /**
     *
     *  Creates a setup triangle from precalculated data.
     *
     *  Creates a new setup triangle in the rasterizer emulator using the
     *  edge and z equations provided.
     *
     *  @param vertex1 The triangle first vertex attribute array.
     *  @param vertex2 The triangle second vertex attribute array.
     *  @param vertex3 The triangle third vertex attribute array.
     *  @param A QuadFloat variable storing the a coefficients for
     *  the triangle edge and z equations.
     *  @param B QuadFloat variable storing the b coefficients for
     *  the triangle edge and z equations.
     *  @param C QuadFloat variable storing the c coefficients for
     *  the triangle edge and z equations.
     *  @param tArea Variable storing the estimated 'area' for the
     *  triangle.
     *
     *  @return An identifier for the the new setup triangle.
     *
     */

    u32bit setup(QuadFloat *vAttr1, QuadFloat *vAttr2, QuadFloat *vAttr3,
        QuadFloat A, QuadFloat B, QuadFloat C, f32bit tArea);

    /**
     *
     *  Sets up edge equations for a previously bound triangle, 
     *  from precalculated data.
     *
     *  Sets the provided edge equations and z interpolation equation for a
     *  previously pre-bound triangle. Used in the micropolygon rasterizer.
     *
     *  @param triangle The triangle identifier of a valid pre-bound triangle
     *  @param A QuadFloat variable storing the a coefficients for
     *  the triangle edge and z equations.
     *  @param B QuadFloat variable storing the b coefficients for
     *  the triangle edge and z equations.
     *  @param C QuadFloat variable storing the c coefficients for
     *  the triangle edge and z equations.
     *  @param tArea Variable storing the estimated 'area' for the
     *  triangle.
     *
     */

    void setupEdgeEquations(u32bit triangle, QuadFloat A, QuadFloat B, 
        QuadFloat C, f32bit tArea);

    /**
     *
     *  Pre-Bounds a new triangle. 
     *
     *  Allocates a new Rasterizer emulator entry for the triangle
     *  and computes the triangle BB. Used in the micropolygon
     *  rasterizer.
     *
     *  @param vAttr1 The triangle first vertex attribute array.
     *  @param vAttr2 The triangle second vertex attribute array.
     *  @param vAttr3 The triangle third vertex attribute array.
     *
     *  @return An identifier for the new pre-setup triangle.
     */

    u32bit preBound(QuadFloat *vAttr1, QuadFloat *vAttr2, QuadFloat *vAttr3);

    /**
     *
     *  Sets up edge equatios for a pre-bound triangle. 
     *
     *  Computes edge equations and z interpolation equation for a 
     *  previously pre-bound triangle. Used in the micropolygon rasterizer.
     *
     *  @param triangle The triangle identifier of a valid pre-bound triangle
     */

    void setupEdgeEquations(u32bit triangle);

    /**
     *
     *  Destroys a setup triangle.
     *
     *  @param triangle The identifier of the triangle to destroy.
     *
     */

    void destroyTriangle(u32bit triangle);

    /**
     *
     *  Returns the corresponding setup triangle.
     *
     *  @param triangle The identifier of the triangle to return.
     *  @return The corresponding setup triagle.
     */

    SetupTriangle* getSetupTriangle(u32bit triangle);

    /**
     *
     *  Calculates a signed aproximation of the triangle area using
     *  the 2DH rasterization method (so it isn't the *real* area).
     *
     *  @param triangle The triangle identifier.
     *
     *  @return The triangle area calculated using 2DH rasterization
     *  method (the vertex coordinates matrix determinant).
     *
     */

    f64bit triangleArea(u32bit triangle);

    /**
     *
     *  Returns the triangle size in percentage of total screen area.
     * 
     *
     *  @param triangle The triangle identifier.
     *
     *  @return The percentage of total screen area.
     *
     */

    f64bit triScreenPercent(u32bit triangle);

    /**
     *
     *  Returns the Triangle Bounding Box.
     *
     *
     *  @param triangle The triangle identifier.
     *  @param x0 Reference to a variable where to store the minimum x for the bounding box.
     *  @param y0 Reference to a variable where to store the minimum y for the bounding box.
     *  @param x1 Reference to a variable where to store the maximum x for the bounding box.
     *  @param y1 Reference to a variable where to store the maximum y for the bounding box.
     *
     */

    void triangleBoundingBox(u32bit triangle, s32bit &x0, s32bit &y0, s32bit &x1, s32bit &y1);

    /**
     *
     *  Updates the MSAA subpixel sampling box according to the specified MSAA mode.
     *
     *  @param multiSamplingEnabled Multi sampling enabled or disabled.
     *  @param msaaSamples Multi sampling mode.
     *  
     */
    void computeMSAABoundingBox(bool multiSamplingEnabled, u32bit msaaSamples);

    /**
     *
     *  Tests if meets microtriangle dimensions. Additionally returns adjusted BB 
     *  and covered pixels and stamps information in X and Y directions.
     *
     *  @param triangle The triangle identifier.
     *  @param microTriSzLimit 0: 1-pixel bound triangles only, 1: 1-pixel and 1-stamp bound triangles, 
     *                         2: Any triangle bound to 2x2, 1x4 or 4x1 stamps.
     *  @param minX The adjusted microtriangle BB left bound.
     *  @param minY The adjusted microtriangle BB lower bound.
     *  @param maxX The adjusted microtriangle BB right bound.
     *  @param maxY The adjusted microtriangle BB upper bound.
     *  @param pixelsX Reference to store covered pixels in the X direction.
     *  @param pixelsY Reference to store covered pixels in the Y direction.
     *  @param stampsX Reference to store covered stamps in the X direction.
     *  @param stampsY Reference to store covered stamps in the Y direction.
     *
     *  @return The triangle meets the microtriangle dimensions according to the specified limit. 
     *
     */

    bool testMicroTriangle(u32bit triangle, u32bit microTriSzLimit, s32bit& minX, s32bit& minY, s32bit& maxX, s32bit& maxY, u32bit& pixelsX, u32bit& pixelsY, u32bit& stampsX, u32bit& stampsY);

    /**
     *
     *  Inverts the triangle facing direction (changes triangle edge
     *  equation signs).
     *
     *  @param triangle The triangle identifier for which to change
     *  the facing direction.
     *
     */

    void invertTriangleFacing(u32bit triangle);

    /**
     *
     *  Performs the color selection for two sided lighting based on
     *  the triangle facing (front -> positive area, back -> negative area).
     *
     *  @param triangle The triangle identifier for which to select the
     *  colors.
     *
     */

    void selectTwoSidedColor(u32bit triangle);

    /**
     *
     *  Calculates the triangle start rasterization position for
     *  scanline based rasterization and adjust the triangle edge
     *  equations to that start point.
     *
     *  @param triangle The setup triangle for which calculate the
     *  start rasterization position.
     *  @param msaaEnabled Boolean parameter that is set to TRUE for MSAA to change the position
     *  of the base sample point.
     *
     */

    void startPosition(u32bit triangle, bool msaaEnabled);

    /**
     *
     *  Changes the current raster position for the triangle updating the edge and z equations.
     *
     *  @param triangle Triangle identifier for which to change the raster position.
     *  @param x The new raster position (pixel).
     *  @param y The new raster position (pixel).
     *
     */

    void changeRasterPosition(u32bit triangle, s32bit x, s32bit y);

    /**
     *
     *  Creates the start (top) tile for the triangle.  Used for recursive rasterization
     *  intialization.
     *
     *  @param triangle The setup triangle for which calculate the start rasterization position.
     *  @param msaaEnabled Boolean parameter that is set to TRUE for MSAA to change the position
     *  of the base sample point.
     *
     */

    void startRecursive(u32bit triangle, bool msaaEnabled);

    /**
     *
     *  Creates the start (top) tile for the batch of triangles.  Used for recursive
     *  rasterization initialization.
     *
     *  @param triangleID An array with the identifiers of the batch of setup triangles
     *  to rasterize.
     *  @param numTris Number of triangles in the batch.
     *  @param msaaEnabled Boolean parameter that is set to TRUE for MSAA to change the position
     *  of the base sample point.
     *
     *  @return Identifier of the recursive rasterization batch that was initialized.
     *
     */

    u32bit startRecursiveMulti(u32bit *triangleID, u32bit numTris, bool msaaEnabled);

    /**
     *
     *  Converts a fragment interpolated z (depth) value into
     *  a fixed point value clamped to [0..1] using the full
     *  depth buffer bit precission range.
     *
     *  @param z Interpolated z value for the fragment.
     *
     *  @return The z value mapped to fixed point into the
     *  depth buffer bit precission range.
     *
     */

    u32bit convertZ(f64bit z);

    /**
     *
     *  Generates the next stamp using a tiled scanline rasterization
     *  algorithm.  Works on a generation tile basis.
     *
     *  @param triangle The triangle identifier for the triangle
     *  from which to generate the next fragment.
     *
     *  @return A pointer to an array of fragments (stamp).
     *
     */

    Fragment **nextScanlineStampTiled(u32bit triangle);

    /**
     *
     *  Request to the recursive rasterization for the generated stamp.
     *  Works on a generation tile basis.
     *
     *  @param triangle The triangle identifier for the triangle
     *  from which to generate the next fragment.
     *
     *  @return A pointer to an array of fragments (stamp).
     *
     */

    Fragment **nextStampRecursive(u32bit triangleID);

    /**
     *
     *  Generates the next stamp for recursive rasterization of a batch of triangles.
     *
     *  @param batchID Identifier of the batch of triangles being rasterized.
     *  @param genTriangle Reference to an integer variable where to store the identifier
     *  of the triangle that generated the stamp/fragments.
     *
     *  @return A stamp of fragments generated by the recursive rasterization of the
     *  batch of triangles.  It returns NULL if no stamp/fragments could be generated.
     *
     */

    Fragment **nextStampRecursiveMulti(u32bit batchID, u32bit &genTriangle);

    /**
     *
     *  Tells if the are no more fragments to generate in the
     *  triangle using scaline based rasterization.
     *
     *  @param triangle The identifier of the triangle for which
     *  to ask if all its fragments have been generated.
     *
     *  @return If all the fragments in the triangle have been
     *  generated.
     *
     */

    bool lastFragment(u32bit triangle);

    /**
     *
     *  Interpolates a fragment attribute (4 32-bit float components)
     *  using barycentric coordinates.
     *
     *  @param f Fragment for which to interpolate the attribute.
     *  @param attribute The identifier for the attribute to interpolate.
     *
     *  @return The interpolated attribute.
     *
     */

    QuadFloat interpolate(Fragment *f, u32bit attribute);

    /**
     *
     *  Interpolates the qfragment attribute array using barycentric
     *  coordinates.
     *
     *  @param f Fragment for which to interpolate the attribute.
     *  @param attribute A pointer to a QuadFloat array where to
     *  store the fragment interpolated attributes.
     *
     */

    void interpolate(Fragment *f, QuadFloat *attribute);

    /**
     *
     *  Copies the fragment attribute from one of the setup
     *  triangle vertex attribute.
     *
     *  @param f The fragment for which to copy the attribute.
     *  @param attribute The attribute to copy.
     *  @param vertex The triangle vertex from which to copy the
     *  attribute.
     *
     *  @return The copied attribute.
     *
     */

    QuadFloat copy(Fragment *f, u32bit attribute, u32bit vertex);

    /**
     *
     *  Copies all the fragment attributes from one of the
     *  setup triangle vertex attributes.
     *
     *  @param f The fragment for which to copy the attributes.
     *  @param attribute A pointer to the QuadFloat array where to
     *  store the fragment interpolated attributes.
     *  @param vertex The triangle vertex from which to copy the
     *  attributes.
     *
     */

    void copy(Fragment *f, QuadFloat *attribute, u32bit vertex);

    /**
     *
     *  Generates the top most level tile for the triangle to be
     *  rasterized using recursive descent rasterization.
     *
     *  @param triangle Identifier of the setup triangle in the rasterizer emulator for which to create
     *  the top most tile
     *  @param msaaEnabled Boolean parameter that is set to TRUE for MSAA to change the position
     *  of the base sample point.
     *
     *  @return The top most level tile for the triangle.
     *
     */

    Tile *topLevelTile(u32bit triangle, bool msaaEnabled);

    /**
     *
     *  Generates the top most level tile for the triangle to be
     *  rasterized using recursive descent rasterization.
     *
     *  @param triangles Pointer to an array of setup triangles.
     *  @param numTris Number of triangles in the batch.
     *  @param msaaEnabled Boolean parameter that is set to TRUE for MSAA to change the position
     *  of the base sample point.
     *
     *  @return The top most level tile for the triangle.
     *
     */

    Tile *topLevelTile(u32bit *triangles, u32bit numTris, bool msaaEnabled);

    /**
     *
     *  Creates the specified tile for the batch of setup triangles.
     *
     *  @param triangles Pointer to an array of setup triangles.
     *  @param numTris Number of triangles in the batch.
     *  @param x Horizontal start coordinate of the tile to create.
     *  @param y Vertical start coordinate of the tile to create.
     *  @param level Level of the tile to create.
     *
     *  @return The specified tile for the batch of triangles.
     *
     */

    Tile *createTile(u32bit *triangles, u32bit numTris, s32bit x, s32bit y, u32bit level);

    /**
     *
     *  Evaluates a tile using recursive descent rasterization.
     *  Generates up to 4 subtiles from the input tile if there
     *  are triangle fragments inside the tile.
     *
     *  @param tile Pointer to the tile to be evaluated.
     *  @param outputTiles Pointer to an array of tile pointers where to store the generated tiles.
     *
     *  @return The number of generated tiles.
     *
     */

    u32bit evaluateTile(Tile *tile, Tile **outputTiles);

    /**
     *
     *  Evaluates a tile using recursive descent rasterization until it reached
     *  a given tile level.
     *  It generates up to 4 subtiles per level from the input tile if there
     *  are triangle fragments inside the tile.
     *
     *  @param tile Pointer to the tile to be evaluated.
     *  @param outputTiles Reference to pointer to an array of tile pointers where to store the generated tiles.
     *  @param level Tile level of the tiles down to evaluate and generate.
     *
     *  @return The number of generated tiles.
     *
     */

    u32bit evaluateTile(Tile *tile, Tile **&tilesOut, u32bit level);

    /**
     *
     *  Generates the fragments in a stamp tile (lowest level tile).
     *
     *  @param tile Pointer to the stamp tile for which to generate the fragments.
     *
     *  @return A pointer to an array of pointer with the generated fragments for the stamp tile.
     *
     */

    Fragment **generateStamp(Tile* tile);

    /**
     *
     *  Generates the fragments of a stamp level tile (preallocated fragment array version).
     *
     *  @param tile Pointer to the stamp tile for which to generate the fragments.
     *  @param fragments Pointer to an array of fragment pointers where to store the generated
     *  fragments.
     *
     */

    void generateStamp(Tile *tile, Fragment **fragments);

    /**
     *
     *  Generates fragments from a stamp level tile and the next triangle from the
     *  batch of triangles being rasterized.
     *
     *  @param tile Pointer to the stamp tile for which to generate teh fragments.
     *  @param fragments Pointer to an array of fragment pointers where to store the
     *  generated fragments.
     *  @param genTriID Reference to an integer variable where to store the identifier
     *  of the setup triangle in the batch that generated the fragments.
     *
     *  @return TRUE if the stamp tile can generate more fragments from another
     *  triangle in the batch.  FALSE if all the fragments for all the triangles
     *  where generated.
     *
     */
    bool generateStampMulti(Tile *tile, Fragment **fragments, u32bit &genTriID);

    /**
     *
     *  Generates the fragments in the generation tile for the triangle current
     *  generation position.
     *
     *  @param triangleID Identifier for the triangle for which to generate the fragments.
     *
     *  @return Pointer to an array of fragment pointer storing the generated fragments.
     *
     */

    //Fragment **generateFragments(u32bit triangleID);
    void generateFragments(u32bit triangleID);

    /**
     *
     *  Generates the fragments in the current generation tile for a batch of triangles.
     *
     *  @param triangleID Identifier of the triangle/batch for which to generate the fragments.
     *  @param stamp Reference to a pointer to an array of fragment pointers where to return
     *  the generated fragments.
     *
     *  @return If the generation tile can generate fragments for other triangles in the
     *  batch.
     *
     */

    //bool generateFragmentsMulti(u32bit triangleID, Fragment **&stamp);
    bool generateFragmentsMulti(u32bit triangleID);

    /**
     *
     *  Scans the current triangle and updates the current generation position.
     *
     *  The scan is performed in a per scan tile basis and with a second tile level
     *  (over tile).
     *
     *  @param triangleID Identifier of the triangle to scan.
     *
     */

    void scanTiled(u32bit triangleID);

    /**
     *
     *  Expands a scan tile into generation tiles for the triangle current scan position.
     *
     *  @param triangleID The identifier of the triangle for which to expand the
     *  current scan tile into generation tiles.
     *  @param genTiles Reference to an integer variable where to store the number of
     *  generation tiles produced.
     *
     *  @return An array of Tile pointers with the produced generation tiles.
     *
     */

    Tile **expandScanTile(u32bit triangleID, u32bit &genTiles);

    /**
     *
     *  Updates the recursive search of fragments.
     *
     *  @param triangleID Identifier of the triangle for which the recursive search
     *  is going to be updated.
     *
     */

    void updateRecursive(u32bit triangleID);

    /**
     *
     *  Updates the recursive fragment generation process for the given triangle batch.
     *
     *  @param batchID Identifier of the batch of triangles being rasterized using
     *  the recursive algorithm.
     *
     */

    void updateRecursiveMulti(u32bit batchID);
    void updateRecursiveMultiv2(u32bit batchID);

    /**
     *
     *  Calculates the tile identifier for the fragment based on the scan tile size.
     *
     *  @param fr Pointer to the fragment for which to calculate the assigned unit.
     *
     *  @return The fragment TileIdentifier.
     *
     */

    TileIdentifier calculateTileID(Fragment *fr);

    /**
     *
     *  Computes the Z samples for MultiSampling AntiAliasing for the input fragment.
     *
     *  @param fr Pointer to the fragment for which to compute the MSAA Z samples.
     *  @param samples Number of Z samples to compute.
     *
     */
     
    void computeMSAASamples(Fragment *fr, u32bit samples);
    
    /**
     *
     *  Perform the sample inside triangle test given the provided edge and z equation values for
     *  the sample.  Includes culling due to far and near clip planes.
     *
     *  @param triangle Pointer to the setup triangle that holds the edge and z equation data.
     *  @param coords Pointer to a 64-bit float point array storing the values of the edge and
     *  z equations for the sample.
     *
     *  @return TRUE if the sample is inside the triangled and inside the far/near clip space,
     *  FALSE otherwise.
     *
     */
     
    bool testInsideTriangle(SetupTriangle *triangle, f64bit *coords);
         
}; // RasterizerEmulator

} // namespace gpu3d

#endif

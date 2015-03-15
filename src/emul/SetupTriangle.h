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
 * $RCSfile: SetupTriangle.h,v $
 * $Revision: 1.9 $
 * $Author: vmoya $
 * $Date: 2005-09-22 09:05:08 $
 *
 * Setup Triangle definition file.
 *
 */

/**
 *
 *  @file SetupTriangle.h
 *
 *  Defines the Setup Triangle class.
 *  This class defines a Triangle and its associated data
 *  in the Rasterization and Triangle Setups stages of the
 *  3D graphic pipeline.
 *
 *
 */

#include "GPUTypes.h"
#include "support.h"
#include "OptimizedDynamicMemory.h"
#include "FixedPoint.h"

#ifndef _SETUPTRIANGLE_

#define _SETUPTRIANGLE_

namespace gpu3d
{

/**  Maximum saved positions per triangle.  */
static const u32bit MAX_SAVED_POSITIONS = 8;


/**  Defines the rasterization directions.  */
//enum RasterDirection
//{
//   CENTER_DIR,     /**<  Rasterize from a center fragment of the triangle.  */
//    UP_DIR,         /**<  Rastarize triangle up.  */
//    DOWN_DIR,       /**<  Rasterize triangle down.  */
//    DOWN_LEFT_DIR,  /**<  Rasterize triangle to the left and down.  */
//    DOWN_RIGHT_DIR, /**<  Rasterize triangle to the right and down.  */
//    UP_LEFT_DIR,    /**<  Rasterize triangle to the left and up.  */
//    UP_RIGHT_DIR,   /**<  Rasterize triangle to the right and up.  */
//    CENTER_LEFT_DIR,    /**<  Rasterize triangle to the left up and down.  */
//    CENTER_RIGHT_DIR    /**<  Rasterize triangle to the right up and down.  */
//};

enum RasterDirection
{
    /*  Basic directions.  */
    UP_DIR      = 0x01,         /**<  Rastarize triangle up.  */
    DOWN_DIR    = 0x02,         /**<  Rasterize triangle down.  */
    LEFT_DIR    = 0x04,         /**<  Rasterize triangle to the left.  */
    RIGHT_DIR   = 0x08,         /**<  Rasterize triangle to the right.  */

    /*  Composite directions.  */
    CENTER_DIR          = 0x03, /**<  Rasterize in the center (up and down).  */
    CENTER_LEFT_DIR     = 0x0f, /**<  Rasterize to the center left (up and down, left and right).  */
    CENTER_RIGHT_DIR    = 0x0b, /**<  Rasterize to the center (up, down, right).  */
    UP_LEFT_DIR         = 0x0d, /**<  Rasterize up and left (up, left, right).  */
    UP_RIGHT_DIR        = 0x09, /**<  Rasterize up and right (up, right).  */
    DOWN_LEFT_DIR       = 0x0e, /**<  Rasterize down and left (down, left, right).  */
    DOWN_RIGHT_DIR      = 0x0a, /**<  Rasterize down and right (down, right).  */

    /*  Border traversal.  */
    BORDER_DIR          = 0x10, /**<  Traverse the bounding box border.  */
    TOP_BORDER_DIR      = 0x11, /**<  Traverse the bounding box top border.  */
    BOTTOM_BORDER_DIR   = 0x12, /**<  Traverse the bounding box bottom border.  */
    LEFT_BORDER_DIR     = 0x13, /**<  Traverse the bounding box left border.  */
    RIGHT_BORDER_DIR    = 0x14  /**<  Traverse the bounding box right border.  */
};


/**
 *
 *  Defines the saved position identifiers.
 *
 */
enum SavedPosition
{
    RIGHT_SAVE = 0,     /**<  Right save position.  */
    UP_SAVE,            /**<  Up save position.  */
    DOWN_SAVE,          /**<  Down save position.  */
    TILE_LEFT_SAVE,     /**<  Tile left save position.  */
    TILE_RIGHT_SAVE,    /**<  Tile right save position.  */
    TILE_DOWN_SAVE,     /**<  Tile down save position.  */
    TILE_UP_SAVE,       /**<  Tile up save position.  */
    RASTER_START_SAVE   /**<  Rasterization start position.  */
};



/**
 *
 *  Defines a Setup Triangle and its associated data.
 *
 *  This class stores the data associated with a triangle
 *  in the setup and rasterization stages of a hardware
 *  3D graphic pipeline.
 *
 *  This class inherits from the OptimizedDynamicMemory object that provides custom optimized
 *  memory allocation and deallocation support
 *
 */

class SetupTriangle : public OptimizedDynamicMemory
{

private:

    QuadFloat *vertex1;     /**<  First triangle vertex attribute array.  */
    QuadFloat *vertex2;     /**<  Second triangle vertex attribute array.  */
    QuadFloat *vertex3;     /**<  Third triangle vertex attribute array.  */
    f64bit area;            /**<  The triangle signed area (or an approximation to the area).  */
    f64bit edge1[3];        /**<  First edge coefficients.  */
    f64bit edge2[3];        /**<  Second edge coefficients.  */
    f64bit edge3[3];        /**<  Third edge coefficients.  */
    f64bit zEq[3];          /**<  Z interpolation equation coefficients.  */
    f64bit savedEdge[MAX_SAVED_POSITIONS][4];   /**<  Saved edge and z interpolation position.  */
    s32bit savedRaster[MAX_SAVED_POSITIONS][2]; /**<  Saved (x,y) raster position.  */
    bool isSaved[MAX_SAVED_POSITIONS];      /**<  Stores if a saved position has valid data.  */
    bool lastFragmentFlag;  /**<  Flag storing if the last triangle fragment has been generated.  */
    s32bit x;               /**<  Current x raster position inside the triangle.  */
    s32bit y;               /**<  Current y raster position inside the triangle.  */
    u8bit direction;        /**<  The current rasterization direction for the triangle.  */
    u8bit tileDirection;    /**<  The current rasterization direction for tiles inside the triangle.  */
    u32bit references;      /**<  Setup Triangle references counter.  Used for 'garbage collection'/object destruction.  */
    bool firstStamp;        /**<  Flag signaling if the first stamp of the triangle hasn't been generated yet.  */
    s32bit minX;            /**<  Defines the triangle bounding box (minimum x) inside the viewport.  */
    s32bit minY;            /**<  Defines the triangle bounding box (minimum y) inside the viewport.  */
    s32bit maxX;            /**<  Defines the triangle bounding box (maximum x) inside the viewport.  */
    s32bit maxY;            /**<  Defines the triangle bounding box (maximum y) inside the viewport.  */
    f64bit screenArea;      /**<  The triangle size (in percentage of total screen area). */
    QuadFloat nHVtxPos[3];  /**<  The non-homogeneous position coordinates for the three vertices (fourth component is each vertex 1/w).  */
    bool preBoundTriangle;  /**<  Triangle is pre-bound.  */
    FixedPoint subXMin;     /**<  The subpixel bounding box (minimum x) stored as a fixed-point fractional value. */
    FixedPoint subYMin;     /**<  The subpixel bounding box (minimum y) stored as a fixed-point fractional value. */
    FixedPoint subXMax;     /**<  The subpixel bounding box (maximum x) stored as a fixed-point fractional value. */
    FixedPoint subYMax;     /**<  The subpixel bounding box (maximum y) stored as a fixed-point fractional value. */
    
public:

    /**
     *
     *  Setup Triangle constructor.
     *
     *  Creates and initializes a new setup triangle from the
     *  three given vertices and their attributes.  The vertices
     *  are passed in CCW for front facing triangles (default face mode).
     *
     *  @param vert1 The triangle first vertex attributes.
     *  @param vert2 The triangle second vertex attributes.
     *  @param vert3 The triangle third vertex attributes.
     *
     */

    SetupTriangle(QuadFloat *vert1, QuadFloat *vert2, QuadFloat *vert3);

    /**
     *
     *  Setup Triangle destructor.
     *
     */

    ~SetupTriangle();

    /**
     *
     *  Sets the triangle three edge equations.
     *
     *  @param edgeEq1 The first edge equation coefficients.
     *  @param edgeEq2 The second edge equation coefficients.
     *  @param edgeEq3 The third edge equation coefficients.
     *
     */

    void setEdgeEquations(f64bit *edgeEq1, f64bit *edgeEq2, f64bit *edgeEq3);

    /**
     *
     *  Sets the triangle z interpolation edge equation.
     *
     *  @param zeq The z interpolation egde equation coefficients.
     *
     */

    void setZEquation(f64bit *zeq);

    /**
     *
     *  Gets the triangle three edge equations.
     *
     *  @param edgeEq1 Pointer to the 64 bit float array where to store
     *  the first edge equation coefficients.
     *  @param edgeEq2 Pointer to the 64 bit float array where to store
     *  the second edge equation coefficients.
     *  @param edgeEq1 Pointer to the 64 bit float array where to store
     *  the third edge equation coefficients.
     *
     */

    void getEdgeEquations(f64bit *edgeEq1, f64bit *edgeEq2, f64bit *edgeEq3);

    /**
     *
     *  Gets the triangle z interpolation equation.
     *
     *  @param zeq A pointer to array where to stored the z intepolation
     *  equation coefficients.
     *
     */

    void getZEquation(f64bit *zeq);


    /**
     *
     *  Updates the current fragment generation value.
     *
     *  @param updateVal The value with which to update the current position.
     *  @param xP New x raster position.
     *  @param yP New y raster position.
     *
     */

    void updatePosition(f64bit *update, s32bit xP, s32bit yP);

    /**
     *
     *  Saves a rasterization position.
     *
     *  @param save Pointer to the edge and z interpolation
     *  equation values to save.
     *  @param x Horizontal raster position to save.
     *  @param y Vertical raster position to save.
     *  @param savePos Save position where to save the data.
     *
     */

    void save(f64bit *save, s32bit x, s32bit y, SavedPosition savePos);

    /**
     *
     *  Saves the raster start position.
     *
     */

    void saveRasterStart();

    /**
     *
     *  Saves right fragment position state.
     *
     *  @param right Right save position rasterization state.
     *
     */

    void saveRight(f64bit *right);

    /**
     *
     *  Saves down fragment position state.
     *
     *  @param down Down save position rasterization state.
     *
     */

    void saveDown(f64bit *down);

    /**
     *
     *  Saves up fragment position state.
     *
     *  @param up Up save position rasterization state.
     *
     */

    void saveUp(f64bit *up);

    /**
     *
     *  Restores a saved rasterization position.
     *
     *  @param save The saved position identifier.
     *
     */

    void restore(SavedPosition save);


    /**
     *
     *  Restores right saved position state.
     *
     *
     */

    void restoreRight();

    /**
     *
     *  Restores down saved position state.
     *
     *
     */

    void restoreDown();


    /**
     *
     *  Restores up saved position state.
     *
     *
     */

    void restoreUp();


    /**
     *
     *  Restores left tile saved position state.
     *
     *
     */

    void restoreLeftTile();

    /**
     *
     *  Restores right tile saved position state.
     *
     *
     */

    void restoreRightTile();

    /**
     *
     *  Restores down tile saved position state.
     *
     *
     */

    void restoreDownTile();


    /**
     *
     *  Restores up tile saved position state.
     *
     *
     */

    void restoreUpTile();

    /**
     *
     *  Asks if the right position has been already saved.
     *
     *  @return If the right position is saved.
     *
     */

    bool isRightSaved();

    /**
     *
     *  Asks if the down position has been already saved.
     *
     *  @return If the down position is saved.
     *
     */

    bool isDownSaved();

    /**
     *
     *  Asks if the up position has been already saved.
     *
     *  @return If the up position is saved.
     *
     */

    bool isUpSaved();

    /**
     *
     *  Asks if the left tile position has been already saved.
     *
     *  @return If the left tile position is saved.
     *
     */

    bool isLeftTileSaved();

    /**
     *
     *  Asks if the right tile position has been already saved.
     *
     *  @return If the right tile position is saved.
     *
     */

    bool isRightTileSaved();

    /**
     *
     *  Asks if the down tile position has been already saved.
     *
     *  @return If the down tile position is saved.
     *
     */

    bool isDownTileSaved();

    /**
     *
     *  Asks if the up tile position has been already saved.
     *
     *  @return If the up tile position is saved.
     *
     */

    bool isUpTileSaved();

    /**
     *
     *  Sets triangle last fragment flag.
     *
     *
     */

    void lastFragment();

    /**
     *
     *  Asks if the triangle last fragment has been already generated.
     *
     *  @return If the triangle last fragment has been generated.
     *
     */

    bool isLastFragment();

    /**
     *
     *  Sets the triangle as pre-bound triangle.
     *
     */
    void setPreBound();

    /**  
     *
     *  Returns if the triangle is a pre-bound triangle.
     *
     */
    bool isPreBoundTriangle();

    /**
     *
     *  Sets the triangle current raster position.
     *
     *  @param xP The x coordinate of the raster position.
     *  @param yP The y coordinate of the raster position.
     *
     */

    void setRasterPosition(s32bit x, s32bit y);

    /**
     *
     *  Gets the triangle current raster position.
     *
     *  @param xP Reference to the 32 bit integer where to store the triangle
     *  current rasterization position.
     *  @param yP Reference to the 32bit integer where to store the triangle
     *  current rasterization position.
     *
     */

    void getRasterPosition(s32bit &x, s32bit &y);

    /**
     *
     *  Changes the triangle rasterization direction.
     *
     *  @param dir The triangle new rasterization direction.
     *
     */

    void setDirection(RasterDirection dir);

    /**
     *
     *  Gets the triangle current rasterization direction.
     *
     *  @return The triangle current rasterization direction.
     *
     */

    u8bit getDirection();

    /**
     *
     *  Changes the triangle tile rasterization direction.
     *
     *  @param dir The triangle new tile rasterization direction.
     *
     */

    void setTileDirection(RasterDirection dir);

    /**
     *
     *  Returns if a tile rasterization direction is 'active' (must
     *  save positions).
     *
     *  @param dir The rasterization direction to check.
     *
     *  @return If the rasterization algorithm must save tile positions
     *  in the given rasterization direction.
     *
     */

    bool getTileDirection(RasterDirection dir);

    /**
     *
     *  Gets a vertex attribute from the setup triangle.
     *
     *  @param vertex The triangle vertex from which to get
     *  the attribute.
     *  @param attribute The vertex attribute to get.
     *
     *  @return A pointer to the vertex attribute requested.
     *
     */

    QuadFloat *getAttribute(u32bit vertex, u32bit attribute);


    /**
     *
     *  Gets the triangle three vertex attribute arrays.
     *
     *  @param vAttr1 Reference to a QuadFloat pointer where to
     *  store the address of the first vertex attribute array.
     *  @param vAttr2 Reference to a QuadFloat pointer where to
     *  store the address of the second vertex attribute array.
     *  @param vAttr2 Reference to a QuadFloat pointer where to
     *  store the address of the third vertex attribute array.
     *
     */

    void getVertexAttributes(QuadFloat *&vAttr1, QuadFloat *&vAttr2,
        QuadFloat *&vAttr3);


    /**
     *
     *  Sets the triangle signed area.
     *
     *  @param area The triangle signed area (or an approximation).
     *
     */

    void setArea(f64bit area);

     /**
     *  Sets the triangle size in percentage of screen area
     *
     *  @param size The triangle size. 
     *
     */
     void setScreenPercent(f64bit size);

    /**
     *
     *  Gets the triangle signed area.
     *
     *  @return The triangle signed area (or an approximation to that area).
     *
     */

    f64bit getArea();

   /**
    *
    *  Gets the triangle size in percentage of screen area.
    *
    *  @return The triangle size. 
    *
    */

    f64bit getScreenPercent();

    /**
     *
     *  Sets the vertex non-homogeneous position coordinates (divided by W).
     *
     *  @param vtx  The vertex identifier [0-2].
     *  @param nHPos The vertex's non-homogeneous position coordinates.
     *
     */

    void setNHVtxPosition(u32bit vtx, QuadFloat nHPos);

    /**
     *
     *  Gets the vertex non-homogeneous position coordinates (divided by W).
     *
     *  @param vtx  The vertex identifier [0-2].
     *  @param nHPosOut The returned vertex's non-homogeneous position coordinates.
     *
     */

    void getNHVtxPosition(u32bit vtx, QuadFloat& nHPosOut);

    /**
     *
     *  Inverts the sign of the triangle edge equations.
     *
     */

    void invertEdgeEquations();

    /**
     *
     *  Increments the setup triangle reference counter.
     *
     *
     */

    void newReference();


    /**
     *
     *  Decrements the setup triangle reference counter.
     *
     *  If the number of objects referencing the setup triangle object
     *  is 0 the object is deleted.
     *
     */

    void deleteReference();

    /**
     *
     *  Sets the first stamp generated flag.
     *
     *  @param flag The value that the first stamp flag will take.
     *
     */

    void setFirstStamp(bool flag);

    /**
     *
     *  Returns the value of the first stamp flag.
     *
     *  @return The current value of the first stamp flag.
     *
     */

   bool isFirstStamp();

    /**
     *
     *  Sets the triangle bounding box.
     *
     *  @param x0 Minimum x for the bounding box.
     *  @param y0 Minimum y for the bounding box.
     *  @param x1 Maximum x for the bounding box.
     *  @param y1 Maximum y for the bounding box.
     *
     */

    void setBoundingBox(s32bit x0, s32bit y0, s32bit x1, s32bit y1);

    /**
     *
     *  Sets the subpixel triangle bounding box in fixed-point fractional values.
     *
     *  @param subXMin Minimum x for the bounding box (fixed-point).
     *  @param subYMin Minimum y for the bounding box (fixed-point).
     *  @param subXMax Maximum x for the bounding box (fixed-point).
     *  @param subYMax Maximum y for the bounding box (fixed-point).
     *
     */

    void setSubPixelBoundingBox(FixedPoint subXMin, FixedPoint subYMin, FixedPoint subXMax, FixedPoint subYMax);

    /**
     *
     *  Gets the triangle bounding box.
     *
     *  @param x0 Reference to a variable where to store the minimum x for the bounding box.
     *  @param y0 Reference to a variable where to store the minimum y for the bounding box.
     *  @param x1 Reference to a variable where to store the maximum x for the bounding box.
     *  @param y1 Reference to a variable where to store the maximum y for the bounding box.
     *
     */

    void getBoundingBox(s32bit &x0, s32bit &y0, s32bit &x1, s32bit &y1);

    /**
     *
     *  Gets the subpixel triangle bounding box in fixed-point fractional values.
     *
     *  @param subXMin Reference to a variable where to store the minimum x for the bounding box (fixed-point).
     *  @param subYMin Reference to a variable where to store the minimum y for the bounding box (fixed-point).
     *  @param subXMax Reference to a variable where to store the maximum x for the bounding box (fixed-point).
     *  @param subYMax Reference to a variable where to store the maximum y for the bounding box (fixed-point).
     *
     */

    void getSubPixelBoundingBox(FixedPoint& subXmin, FixedPoint& subYmin, FixedPoint& subXmax, FixedPoint& subYmax);
        
}; // SetupTriangle

} // namespace gpu3d


#endif

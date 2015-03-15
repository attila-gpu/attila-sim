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
 * $RCSfile: RasterizerEmulator.cpp,v $
 * $Revision: 1.25 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:41 $
 *
 * Rasterizer Emulator implementation file.
 *
 */

/**
 *
 *  @file RasterizerEmulator.cpp
 *
 *  Implements the RasterizerEmulator class.
 *  This class provides services to emulate the functional behaviour
 *  Rasterizer unit.
 *
 *
 */


#include "RasterizerEmulator.h"
#include "GPUMath.h"
#include <cstdio>
#include <cstring>
#include <iostream>
#include "FixedPoint.h"

using namespace gpu3d;
using namespace std;

RasterizerEmulator::MSAAOffset RasterizerEmulator::MSAAPatternTable2[4] =
{
    {  8.0, 120.0},
    {120.0,   8.0},

    // MSAA Pattern precomputed bounding box.
    {  8.0,   8.0}, // minX, minY
    {120.0, 120.0}, // maxX, maxY
};

RasterizerEmulator::MSAAOffset RasterizerEmulator::MSAAPatternTable4[6] =
{
    { 12.0,  44.0},
    { 44.0, 108.0},
    { 76.0,  12.0},
    {108.0,  76.0},

    // MSAA Pattern precomputed bounding box.
    { 12.0,  12.0}, // minX, minY
    {108.0, 108.0}, // maxX, maxY
};

RasterizerEmulator::MSAAOffset RasterizerEmulator::MSAAPatternTable6[8] =
{
    { 12.0,  12.0},
    { 32.0,  52.0},
    { 52.0,  96.0},
    { 76.0,  32.0},
    { 96.0, 116.0},
    {116.0,  76.0},

    // MSAA Pattern precomputed bounding box.
    { 12.0,  12.0}, // minX, minY
    {116.0, 116.0}, // maxX, maxY
};

RasterizerEmulator::MSAAOffset RasterizerEmulator::MSAAPatternTable8[10] =
{
    {  8.0,  56.0},
    { 24.0, 104.0},
    { 40.0,  24.0},
    { 56.0,  88.0},
    { 72.0,  40.0},
    { 88.0, 120.0},
    {104.0,  72.0},
    {120.0,   8.0},

    // MSAA Pattern precomputed bounding box.
    {  8.0,   8.0}, // minX, minY
    {120.0, 120.0}, // maxX, maxY

};


/*  Rasterizer Emulator constructor.  */
RasterizerEmulator::RasterizerEmulator(u32bit numActiveTriangles,
    u32bit attrPerFragment, u32bit scanW, u32bit scanH, u32bit overW,
    u32bit overH, u32bit genW, u32bit genH, bool useBBOpt, u32bit subPxBits) :

    scanTileWidth(scanW), scanTileHeight(scanH),
    scanOverTileWidth(overW), scanOverTileHeight(overH),
    genTileWidth(genW), genTileHeight(genH),
    genTileFragments(genW * genH), useBBOptimization(useBBOpt),
    subPixelPrecision(subPxBits)
{
    u32bit i;

    /*  Set the maximum number of active triangles in the
        rasterizer. */
    activeTriangles = numActiveTriangles;

    /*  Set the maximum number of attributes per fragment. */
    fragmentAttributes = attrPerFragment;

    /*  Allocate space of the active setup triangles.  */
    setupTriangles = new SetupTriangle *[activeTriangles];

    /*  Check if allocation worked.  */
    GPU_ASSERT(
        if (setupTriangles == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the setup triangles.");
    )

    /*  Initialize the setup array to NULL.  */
    for(i = 0; i < activeTriangles; i++)
        setupTriangles[i] = NULL;

    /*  Allocate the setup triangle generation tile array.  */
    trGenTiles = new Tile **[activeTriangles];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (trGenTiles == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the triangle tile array.");
    )

    /*  Initialize generation tile pointers.  */
    for(i = 0; i < activeTriangles; i++)
        trGenTiles[i] = NULL;

    /*  Allocate the setup triangle stamp array.  */
    trStamps = new Tile **[activeTriangles];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (trStamps == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the triangle stamp array.");
    )

    /*  Initialize generation tile pointers.  */
    for(i = 0; i < activeTriangles; i++)
        trStamps[i] = NULL;

    /*  Allocate the setup triangle fragment array.  */
    trFragments = new Fragment **[activeTriangles];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (trFragments == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the triangle fragment array.");
    )

    /*  Initialize fragment pointers.  */
    for(i = 0; i < activeTriangles; i++)
    {
        trFragments[i] = new Fragment*[genTileWidth * genTileHeight];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (trFragments[i] == NULL)
                panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating per triangle fragment buffer.");
        )
    }

    /*  Allocate the fragment triangle identifier array.  */
    frTriangleID = new u32bit *[activeTriangles];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (frTriangleID == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the fragment triangle identifier array.");
    )

    /*  Initialize triangle identifier pointers.  */
    for(i = 0; i < activeTriangles; i++)
    {
        frTriangleID[i] = new u32bit[genTileWidth * genTileHeight];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (frTriangleID[i] == NULL)
                panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating per triangle fragment triangle identifier buffer.");
        )
    }

    /*  Allocate setup triangle generation tile counters.  */
    trStoredGenTiles = new u32bit[activeTriangles];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (trStoredGenTiles == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the triangle tile counters.");
    )

    /*  Initialize tile counters.  */
    for(i = 0; i < activeTriangles; i++)
        trStoredGenTiles[i] = 0;

    /*  Allocate setup triangle generation stamp counters.  */
    trStoredStamps = new u32bit[activeTriangles];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (trStoredStamps == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the triangle stamp counters.");
    )

    /*  Initialize tile counters.  */
    for(i = 0; i < activeTriangles; i++)
        trStoredStamps[i] = 0;

    /*  Allocate setup triangle fragment counters.  */
    trStoredFragments = new u32bit[activeTriangles];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (trStoredFragments == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the triangle fragment counters.");
    )

    /*  Initialize fragment counters.  */
    for(i = 0; i < activeTriangles; i++)
        trStoredFragments[i] = 0;


    /*  Allocate space for the free setup triangle entry list.  */
    freeSetupList = new u32bit[activeTriangles];

    /*  Check memory allocation.  */
    GPU_ASSERT(
        if (freeSetupList == NULL)
            panic("RasterizerEmulator", "RasterizerEmulator", "Error allocating memory for the free setup entry list.");
    )

    /*  Initialize the free setup entry list.  */
    for(i = 0; i < activeTriangles; i++)
        freeSetupList[i] = i;

    /*  Reset the pointer to the next free setup entry.  */
    nextFreeSetup = 0;

    /*  Reset the free setup entries counter.  */
    freeSetups = activeTriangles;

    /*  Reset stored setup triangles counter.  */
    triangles = 0;

    /*  Set default viewport origin coordinates and sizes.  */
    d3d9PixelCoordinates = false;
    x0 = 0;
    y0 = 0;
    w = 400;
    h = 400;
    n = 0.0f;
    f = 1.0f;
    d3d9DepthRange = false;
    windowWidth = 400;
    windowHeight = 400;
    scissorX0 = 0;
    scissorY0 = 0;
    scissorW = 400;
    scissorH = 400;
    slopeFactor = 0.0f;
    unitOffset = 0.0f;
    faceMode = GPU_CCW;
    depthBitPrecission = 24;
    useD3D9RasterizationRules = false;

    /*  Precalculated values.  */

    /*  Calculate the scan tile level.  */
    scanLevel = (u32bit) GPU_CEIL2(GPU_LOG2(GPU_MAX(scanTileWidth, scanTileHeight)));

    /*  Calculate generation tile level.  */
    genLevel = (u32bit) GPU_CEIL2(GPU_LOG2(GPU_MAX(genTileWidth, genTileHeight)));

    //  Calculate the number of gen tiles per scan level tile.
    scanTileGenTiles = (1 << (scanLevel - genLevel)) * (1 << (scanLevel - genLevel));

    //  Calculate the initial MSAA sampling box (multisampling disabled).
    computeMSAABoundingBox(false, 1);
}

//  Sets the rasterizer viewport.
void RasterizerEmulator::setViewport(bool useD3D9PixelCoords, s32bit x, s32bit y, u32bit width, u32bit height)
{
    //  Set pixel coordinate convention to use.
    d3d9PixelCoordinates = useD3D9PixelCoords;

    //  Set viewport start coordinates.
    x0 = x;
    y0 = y;

    //  Set viewport sizes.
    w = width;
    h = height;
}

//  Sets the depth range.
void RasterizerEmulator::setDepthRange(bool d3d9Mode, f32bit near, f32bit far)
{
    //  Set depth range mode.
    d3d9DepthRange = d3d9Mode;

    //  Set near range.
    n = near;

    //  Set far range.
    f = far;
}

/*  Sets the depth buffer bit precission.  */
void RasterizerEmulator::setDepthPrecission(u32bit zBitPrecission)
{
    depthBitPrecission = zBitPrecission;
}

/*  Set scissor.  */
void RasterizerEmulator::setScissor(u32bit wWidth, u32bit wHeight, bool scissor, s32bit scx0, s32bit scy0, u32bit scw, u32bit sch)
{
    /*  Set current GL window resolution.  */
    windowWidth = wWidth;
    windowHeight = wHeight;

    /*  Check if scissor test is enabled.  */
    if (scissor)
    {
        /*  Calculate the scissor bounding box.  Clamp to the GL window.  */
        scissorX0 = GPU_MAX(scx0, 0);
        scissorY0 = GPU_MAX(scy0, 0);
        scissorW = GPU_MIN(scw, windowWidth - scissorX0);
        scissorH = GPU_MIN(sch, windowHeight - scissorY0);
    }
    else
    {
        /*  The GL window is the bounding box.  */
        scissorX0 = 0;
        scissorY0 = 0;
        scissorW = windowWidth;
        scissorH = windowHeight;
    }
}

/*  Set polygon offset.  */
void RasterizerEmulator::setPolygonOffset(f32bit factor, f32bit unit)
{
    slopeFactor = factor;
    unitOffset = unit;
}

/*  Set front face mode.  */
void RasterizerEmulator::setFaceMode(FaceMode mode)
{
    faceMode = mode;
}

//  Set if D3D9 rasterization rules will be used.
void RasterizerEmulator::setD3D9RasterizationRules(bool d3d9RasterizationRules)
{
    useD3D9RasterizationRules = d3d9RasterizationRules;
}

/*  Pre-bounds a new triangle (Allocates the triangle and computes its BB).*/
u32bit RasterizerEmulator::preBound(QuadFloat *vAttr1, QuadFloat *vAttr2, QuadFloat *vAttr3)
{
    SetupTriangle *triangle;
    u32bit triangleID;
    s32bit xMin, yMin, zMin;
    s32bit xMax, yMax, zMax;
    FixedPoint subXMin, subYMin;
    FixedPoint subXMax, subYMax;
    f64bit screenPercent;
    QuadFloat nH_vtx1, nH_vtx2, nH_vtx3;

    /*  Check if there is a free entry in the setup array.  */
    GPU_ASSERT(
        if (triangles == activeTriangles)
            panic("RasterizerEmulator", "preBound", "No free setup triangle entry.");
    )

    /*  Create the new setup triangle.  */
    triangle = new SetupTriangle(vAttr1, vAttr2, vAttr3);

    /*  Get next free setup entry.  */
    triangleID = freeSetupList[nextFreeSetup];

    /*  Check if the position in the setup triangle table is empty.  */
    GPU_ASSERT(
        if (setupTriangles[triangleID] != NULL)
           panic("RasterizerEmulator", "preBound", "Setup Triangle table entry not empty.");
    )

    /*  Store it in the next setup array entry.  */
    setupTriangles[triangleID] = triangle;

    /*  Update next free entry pointer.  */
    nextFreeSetup = GPU_MOD(nextFreeSetup + 1, activeTriangles);

    /*  Update free setup entries counter.  */
    freeSetups--;

    /*  Update setup triangles counter.  */
    triangles++;

    //  Invert vertical coordinate when D3D9 pixel coordinates convention is selected.
    if (d3d9PixelCoordinates)
    {
        vAttr1[POSITION_ATTRIBUTE][1] = -vAttr1[POSITION_ATTRIBUTE][1];
        vAttr2[POSITION_ATTRIBUTE][1] = -vAttr2[POSITION_ATTRIBUTE][1];
        vAttr3[POSITION_ATTRIBUTE][1] = -vAttr3[POSITION_ATTRIBUTE][1];
    }

    /*  Calculate the subpixel bounding box for the triangle. */
    GPUMath::subPixelBoundingBox(vAttr1[POSITION_ATTRIBUTE], vAttr2[POSITION_ATTRIBUTE], vAttr3[POSITION_ATTRIBUTE], x0, y0, w, h,
        subXMin, subXMax, subYMin, subYMax, zMin, zMax, subPixelPrecision);

    /*  Calculate bounding box for the triangle.  */
    GPUMath::boundingBox(vAttr1[POSITION_ATTRIBUTE], vAttr2[POSITION_ATTRIBUTE], vAttr3[POSITION_ATTRIBUTE], x0, y0, w, h,
        xMin, xMax, yMin, yMax, zMin, zMax);

//printf("Bounding box => {%d, %d}  {%d, %d}\n", xMin, yMin, xMax, yMax);

    /*  Bound the bounding box to the scissor box.  */
    xMin = GPU_MIN(GPU_MAX(xMin, scissorX0), scissorX0 + s32bit(scissorW));
    xMax = GPU_MAX(GPU_MIN(xMax, scissorX0 + s32bit(scissorW)), scissorX0);
    yMin = GPU_MIN(GPU_MAX(yMin, scissorY0), scissorY0 + s32bit(scissorH));
    yMax = GPU_MAX(GPU_MIN(yMax, scissorY0 + s32bit(scissorH)), scissorY0);

//printf("Scissor BB => start = (%d, %d) size = (%d, %d)\n", scissorX0, scissorY0, scissorW, scissorH);
//printf("Bounding box (+ Scissor) => {%d, %d}  {%d, %d}\n", xMin, yMin, xMax, yMax);

    /*  Compute screen area (percentage) and non-homogeneous coordinates.  */
    screenPercent = GPUMath::computeTriangleScreenArea(vAttr1[POSITION_ATTRIBUTE],
                                                       vAttr2[POSITION_ATTRIBUTE],
                                                       vAttr3[POSITION_ATTRIBUTE],
                                                       nH_vtx1, nH_vtx2, nH_vtx3);

    /*  Set the vertex´s non-homogeneous position coordinates.  */
    triangle->setNHVtxPosition(0, nH_vtx1);
    triangle->setNHVtxPosition(1, nH_vtx2);
    triangle->setNHVtxPosition(2, nH_vtx3);

    /*  Set the triangle screen area.  */
    triangle->setScreenPercent(screenPercent);

    /*  Set setup triangle bounding box.  */
    triangle->setBoundingBox(xMin, yMin, xMax, yMax);

    /*  Set subpixel bounding box.  */
    triangle->setSubPixelBoundingBox(subXMin, subYMin, subXMax, subYMax);

    /*  Reset stored generation tiles counter.  */
    trStoredGenTiles[triangleID] = 0;

    /*  Reset stored fragments counter.  */
    trStoredFragments[triangleID] = 0;

    /*  Set as a pre-bound triangle.  */
    triangle->setPreBound();

    return triangleID;
}

/*  Sets up edge equations for a previously bound triangle.  */
void RasterizerEmulator::setupEdgeEquations(u32bit triangleID)
{
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    SetupTriangle *triangle;
    f64bit area;
    QuadFloat *vAttr1;
    QuadFloat *vAttr2;
    QuadFloat *vAttr3;

    /*  Check if it is a valid triangle identifier.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "setupEdgeEquations", "Wrong setup triangle identifier.");
    )

    /*  Check if non-empty triangle entry.  */
    GPU_ASSERT(
        if (setupTriangles[triangleID] == NULL)
           panic("RasterizerEmulator", "setupEdgeEquations", "Setup Triangle table entry is empty.");
    )

    /*  Check if entry is a pre-bound triangle.  */
    GPU_ASSERT(
        if (!setupTriangles[triangleID]->isPreBoundTriangle())
           panic("RasterizerEmulator", "setupEdgeEquations", "Triangle identifier returns a non pre-bound triangle.");
    )

    /*  Retrieve pointer to the setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Get vertex attributes.  */
    setupTriangles[triangleID]->getVertexAttributes(vAttr1, vAttr2, vAttr3);

//vAttr1[POSITION_ATTRIBUTE][2], vAttr1[POSITION_ATTRIBUTE][3]);
//printf("Vertex 2 Position = {%f, %f, %f, %f}\n", vAttr2[POSITION_ATTRIBUTE][0], vAttr2[POSITION_ATTRIBUTE][1],
//vAttr2[POSITION_ATTRIBUTE][2], vAttr2[POSITION_ATTRIBUTE][3]);
//printf("Vertex 3 Position = {%f, %f, %f, %f}\n", vAttr3[POSITION_ATTRIBUTE][0], vAttr3[POSITION_ATTRIBUTE][1],
//vAttr3[POSITION_ATTRIBUTE][2], vAttr3[POSITION_ATTRIBUTE][3]);

    /*  Calculate setup matrix.  */
    GPUMath::setupMatrix(vAttr1[POSITION_ATTRIBUTE], vAttr2[POSITION_ATTRIBUTE], vAttr3[POSITION_ATTRIBUTE],
        edge1, edge2, edge3);

//printf("Edge Eq 1 => {%f, %f, %f}\n", edge1[0], edge1[1], edge1[2]);
//printf("Edge Eq 2 => {%f, %f, %f}\n", edge2[0], edge2[1], edge2[2]);
//printf("Edge Eq 3 => {%f, %f, %f}\n", edge3[0], edge3[1], edge3[2]);

    //
    //  Note: Inverting the vertical coordinate changes the vertex ordering.
    //
    if (((faceMode == GPU_CW) && !d3d9PixelCoordinates) || ((faceMode == GPU_CCW) && d3d9PixelCoordinates))
    {
        /*  Invert edge equations.  */
        edge1[0] = -edge1[0];
        edge1[1] = -edge1[1];
        edge1[2] = -edge1[2];
        edge2[0] = -edge2[0];
        edge2[1] = -edge2[1];
        edge2[2] = -edge2[2];
        edge3[0] = -edge3[0];
        edge3[1] = -edge3[1];
        edge3[2] = -edge3[2];
    }

    /*  Calculate Z interpolation equation.  */
    GPUMath::interpolationEquation(edge1, edge2, edge3,
        vAttr1[POSITION_ATTRIBUTE][2], vAttr2[POSITION_ATTRIBUTE][2], vAttr3[POSITION_ATTRIBUTE][2], zeq);

    /*  Adjust the edge equations to the current viewport.  */
    area = GPUMath::viewport(vAttr1[POSITION_ATTRIBUTE], vAttr2[POSITION_ATTRIBUTE], vAttr3[POSITION_ATTRIBUTE],
        edge1, edge2, edge3, zeq, x0, y0, w, h);

    /*  Offset sample point half a pixel (OpenGL specification).  */
    edge1[2] += edge1[0] * 0.5f + edge1[1] * 0.5f;
    edge2[2] += edge2[0] * 0.5f + edge2[1] * 0.5f;
    edge3[2] += edge3[0] * 0.5f + edge3[1] * 0.5f;
    zeq[2] += zeq[0] * 0.5f  + zeq[1] * 0.5f;

    /*  Set the triangle area.  */
    triangle->setArea(area);

    /*  Set the setup triangle three edge equations.  */
    triangle->setEdgeEquations(edge1, edge2, edge3);

    /*  Set the z interpolation equation of the triangle.  */
    triangle->setZEquation(zeq);
}

/*  Setups a new triangle.  */
u32bit RasterizerEmulator::setup(QuadFloat *vAttr1, QuadFloat *vAttr2, QuadFloat *vAttr3)
{
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    SetupTriangle *triangle;
    u32bit triangleID;
    f64bit area;
    s32bit xMin, yMin, zMin;
    s32bit xMax, yMax, zMax;
    f64bit screenPercent;
    QuadFloat nH_vtx1, nH_vtx2, nH_vtx3;

    /*  Check if there is a free entry in the setup array.  */
    GPU_ASSERT(
        if (triangles == activeTriangles)
            panic("RasterizerEmulator", "setup", "No free setup triangle entry.");
    )

    /*  Create the new setup triangle.  */
    triangle = new SetupTriangle(vAttr1, vAttr2, vAttr3);

    /*  Get next free setup entry.  */
    triangleID = freeSetupList[nextFreeSetup];

    /*  Check if the position in the setup triangle table is empty.  */
    GPU_ASSERT(
        if (setupTriangles[triangleID] != NULL)
           panic("RasterizerEmulator", "setup", "Setup Triangle table entry not empty.");
    )

    /*  Store it in the next setup array entry.  */
    setupTriangles[triangleID] = triangle;

    /*  Update next free entry pointer.  */
    nextFreeSetup = GPU_MOD(nextFreeSetup + 1, activeTriangles);

    /*  Update free setup entries counter.  */
    freeSetups--;

    /*  Update setup triangles counter.  */
    triangles++;

//printf("Vertex 1 Position = {%f, %f, %f, %f}\n", vAttr1[POSITION_ATTRIBUTE][0], vAttr1[POSITION_ATTRIBUTE][1],
//vAttr1[POSITION_ATTRIBUTE][2], vAttr1[POSITION_ATTRIBUTE][3]);
//printf("Vertex 2 Position = {%f, %f, %f, %f}\n", vAttr2[POSITION_ATTRIBUTE][0], vAttr2[POSITION_ATTRIBUTE][1],
//vAttr2[POSITION_ATTRIBUTE][2], vAttr2[POSITION_ATTRIBUTE][3]);
//printf("Vertex 3 Position = {%f, %f, %f, %f}\n", vAttr3[POSITION_ATTRIBUTE][0], vAttr3[POSITION_ATTRIBUTE][1],
//vAttr3[POSITION_ATTRIBUTE][2], vAttr3[POSITION_ATTRIBUTE][3]);

    //  Invert vertical coordinate when D3D9 pixel coordinates convention is selected.
    if (d3d9PixelCoordinates)
    {
        vAttr1[POSITION_ATTRIBUTE][1] = -vAttr1[POSITION_ATTRIBUTE][1];
        vAttr2[POSITION_ATTRIBUTE][1] = -vAttr2[POSITION_ATTRIBUTE][1];
        vAttr3[POSITION_ATTRIBUTE][1] = -vAttr3[POSITION_ATTRIBUTE][1];
    }

    /*  Calculate setup matrix.  */
    GPUMath::setupMatrix(vAttr1[POSITION_ATTRIBUTE], vAttr2[POSITION_ATTRIBUTE], vAttr3[POSITION_ATTRIBUTE],
        edge1, edge2, edge3);

//printf("Edge Eq 1 => {%f, %f, %f}\n", edge1[0], edge1[1], edge1[2]);
//printf("Edge Eq 2 => {%f, %f, %f}\n", edge2[0], edge2[1], edge2[2]);
//printf("Edge Eq 3 => {%f, %f, %f}\n", edge3[0], edge3[1], edge3[2]);

    //  Check front face mode.
    //
    //  Note: Inverting the vertical coordinate changes the vertex ordering.
    //
    if (((faceMode == GPU_CW) && !d3d9PixelCoordinates) ||
        ((faceMode == GPU_CCW) && d3d9PixelCoordinates))
    {
        /*  Invert edge equations.  */
        edge1[0] = -edge1[0];
        edge1[1] = -edge1[1];
        edge1[2] = -edge1[2];

        edge2[0] = -edge2[0];
        edge2[1] = -edge2[1];
        edge2[2] = -edge2[2];

        edge3[0] = -edge3[0];
        edge3[1] = -edge3[1];
        edge3[2] = -edge3[2];
    }

    /*  Calculate Z interpolation equation.  */
    GPUMath::interpolationEquation(edge1, edge2, edge3,
        vAttr1[POSITION_ATTRIBUTE][2], vAttr2[POSITION_ATTRIBUTE][2], vAttr3[POSITION_ATTRIBUTE][2], zeq);

    /*  Adjust the edge equations to the current viewport.  */
    area = GPUMath::viewport(vAttr1[POSITION_ATTRIBUTE], vAttr2[POSITION_ATTRIBUTE], vAttr3[POSITION_ATTRIBUTE],
        edge1, edge2, edge3, zeq, x0, y0, w, h);


//printf("Area => %f\n", area);

    //  Check what rules will be used for rasterization.
    if (!useD3D9RasterizationRules)
    {
        //  Offset sample point by half a pixel (OpenGL specification).
        edge1[2] += edge1[0] * 0.5f + edge1[1] * 0.5f;
        edge2[2] += edge2[0] * 0.5f + edge2[1] * 0.5f;
        edge3[2] += edge3[0] * 0.5f + edge3[1] * 0.5f;
        zeq[2] += zeq[0] * 0.5f  + zeq[1] * 0.5f;
    }

    /*  Calculate bounding box for the triangle.  */
    GPUMath::boundingBox(vAttr1[POSITION_ATTRIBUTE], vAttr2[POSITION_ATTRIBUTE], vAttr3[POSITION_ATTRIBUTE], x0, y0, w, h,
        xMin, xMax, yMin, yMax, zMin, zMax);

//printf("Bounding box => {%d, %d}  {%d, %d}\n", xMin, yMin, xMax, yMax);

    /*  Bound the bounding box to the scissor box.  */
    xMin = GPU_MIN(GPU_MAX(xMin, scissorX0), scissorX0 + s32bit(scissorW));
    xMax = GPU_MAX(GPU_MIN(xMax, scissorX0 + s32bit(scissorW)), scissorX0);
    yMin = GPU_MIN(GPU_MAX(yMin, scissorY0), scissorY0 + s32bit(scissorH));
    yMax = GPU_MAX(GPU_MIN(yMax, scissorY0 + s32bit(scissorH)), scissorY0);

//printf("Scissor BB => start = (%d, %d) size = (%d, %d)\n", scissorX0, scissorY0, scissorW, scissorH);
//printf("Bounding box (+ Scissor) => {%d, %d}  {%d, %d}\n", xMin, yMin, xMax, yMax);

    /*  Set the triangle area.  */
    triangle->setArea(area);

    /*  Compute screen area (percentage) and non-homogeneous coordinates.  */
    screenPercent = GPUMath::computeTriangleScreenArea(vAttr1[POSITION_ATTRIBUTE],
                                                       vAttr2[POSITION_ATTRIBUTE],
                                                       vAttr3[POSITION_ATTRIBUTE],
                                                       nH_vtx1, nH_vtx2, nH_vtx3);

    /*  Set the vertex´s non-homogeneous position coordinates.  */
    triangle->setNHVtxPosition(0, nH_vtx1);
    triangle->setNHVtxPosition(1, nH_vtx2);
    triangle->setNHVtxPosition(2, nH_vtx3);

    /*  Set the triangle screen area.  */
    triangle->setScreenPercent(screenPercent);

    /*  Set the setup triangle three edge equations.  */
    triangle->setEdgeEquations(edge1, edge2, edge3);

    /*  Set the z interpolation equation of the triangle.  */
    triangle->setZEquation(zeq);

    /*  Set setup triangle bounding box.  */
    triangle->setBoundingBox(xMin, yMin, xMax, yMax);

    /*  Reset stored generation tiles counter.  */
    trStoredGenTiles[triangleID] = 0;

    /*  Reset stored fragments counter.  */
    trStoredFragments[triangleID] = 0;

    return triangleID;
}

/*  Creates a setup triangle from precalculated data.  */
u32bit RasterizerEmulator::setup(QuadFloat *vAttr1, QuadFloat *vAttr2, QuadFloat *vAttr3,
    QuadFloat A, QuadFloat B, QuadFloat C, f32bit tArea)
{
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    SetupTriangle *triangle;
    u32bit triangleID;
    f64bit area;
    s32bit xMin, yMin, zMin;
    s32bit xMax, yMax, zMax;
    f64bit screenPercent;
    QuadFloat nH_vtx1, nH_vtx2, nH_vtx3;

    /*  Check if there is a free entry in the setup array.  */
    GPU_ASSERT(
        if (triangles == activeTriangles)
            panic("RasterizerEmulator", "setup", "No free setup triangle entry.");
    )

    /*  Create the new setup triangle.  */
    triangle = new SetupTriangle(vAttr1, vAttr2, vAttr3);

    /*  Get next free setup entry.  */
    triangleID = freeSetupList[nextFreeSetup];

    /*  Check if the position in the setup triangle table is empty.  */
    GPU_ASSERT(
        if (setupTriangles[triangleID] != NULL)
           panic("RasterizerEmulator", "setup", "Setup Triangle table entry not empty.");
    )

    /*  Store it in the next setup array entry.  */
    setupTriangles[triangleID] = triangle;

    /*  Update next free entry pointer.  */
    nextFreeSetup = GPU_MOD(nextFreeSetup + 1, activeTriangles);

    /*  Update free setup entries counter.  */
    freeSetups--;

    /*  Update setup triangles counter.  */
    triangles++;

//printf("Vertex 1 Position = {%f, %f, %f, %f}\n", vAttr1[POSITION_ATTRIBUTE][0], vAttr1[POSITION_ATTRIBUTE][1],
//vAttr1[POSITION_ATTRIBUTE][2], vAttr1[POSITION_ATTRIBUTE][3]);
//printf("Vertex 2 Position = {%f, %f, %f, %f}\n", vAttr2[POSITION_ATTRIBUTE][0], vAttr2[POSITION_ATTRIBUTE][1],
//vAttr2[POSITION_ATTRIBUTE][2], vAttr2[POSITION_ATTRIBUTE][3]);
//printf("Vertex 3 Position = {%f, %f, %f, %f}\n", vAttr3[POSITION_ATTRIBUTE][0], vAttr3[POSITION_ATTRIBUTE][1],
//vAttr3[POSITION_ATTRIBUTE][2], vAttr3[POSITION_ATTRIBUTE][3]);

    /*  Unpack and convert to 64 bits precalculated edge and z equations.  */
    edge1[0] = f64bit(A[0]);
    edge1[1] = f64bit(B[0]);
    edge1[2] = f64bit(C[0]);
    edge2[0] = f64bit(A[1]);
    edge2[1] = f64bit(B[1]);
    edge2[2] = f64bit(C[1]);
    edge3[0] = f64bit(A[2]);
    edge3[1] = f64bit(B[2]);
    edge3[2] = f64bit(C[2]);
    zeq[0] = f64bit(A[3]);
    zeq[1] = f64bit(B[3]);
    zeq[2] = f64bit(C[3]);

//printf("Edge Eq 1 => {%f, %f, %f}\n", edge1[0], edge1[1], edge1[2]);
//printf("Edge Eq 2 => {%f, %f, %f}\n", edge2[0], edge2[1], edge2[2]);
//printf("Edge Eq 3 => {%f, %f, %f}\n", edge3[0], edge3[1], edge3[2]);
//printf("Z Interpolation Eq => {%f, %f, %f}\n", zeq[0], zeq[1], zeq[2]);

    /*  Convert area to 64 bits.  */
    area = f64bit(tArea);

    /*  Calculate bounding box for the triangle.  */
    GPUMath::boundingBox(vAttr1[POSITION_ATTRIBUTE], vAttr2[POSITION_ATTRIBUTE], vAttr3[POSITION_ATTRIBUTE], x0, y0, w, h,
        xMin, xMax, yMin, yMax, zMin, zMax);

    /*  Bound the bounding box to the scissor box.  */
    xMin = GPU_MIN(GPU_MAX(xMin, scissorX0), scissorX0 + s32bit(scissorW));
    xMax = GPU_MAX(GPU_MIN(xMax, scissorX0 + s32bit(scissorW)), scissorX0);
    yMin = GPU_MIN(GPU_MAX(yMin, scissorY0), scissorY0 + s32bit(scissorH));
    yMax = GPU_MAX(GPU_MIN(yMax, scissorY0 + s32bit(scissorH)), scissorY0);

//printf("Bounding box => {%d, %d}  {%d, %d}\n", xMin, yMin, xMax, yMax);

    /*  Set the triangle area.  */
    triangle->setArea(area);

    /*  Compute screen area (percentage) and non-homogeneous coordinates.  */
    screenPercent = GPUMath::computeTriangleScreenArea(vAttr1[POSITION_ATTRIBUTE],
                                                       vAttr2[POSITION_ATTRIBUTE],
                                                       vAttr3[POSITION_ATTRIBUTE],
                                                       nH_vtx1, nH_vtx2, nH_vtx3);

    /*  Set the vertex´s non-homogeneous position coordinates.  */
    triangle->setNHVtxPosition(0, nH_vtx1);
    triangle->setNHVtxPosition(1, nH_vtx2);
    triangle->setNHVtxPosition(2, nH_vtx3);

    /*  Set the triangle screen area.  */
    triangle->setScreenPercent(screenPercent);

    /*  Set the setup triangle three edge equations.  */
    triangle->setEdgeEquations(edge1, edge2, edge3);

    /*  Set the z interpolation equation of the triangle.  */
    triangle->setZEquation(zeq);

    /*  Set setup triangle bounding box.  */
    triangle->setBoundingBox(xMin, yMin, xMax, yMax);

    /*  Reset stored generation tiles counter.  */
    trStoredGenTiles[triangleID] = 0;

    /*  Reset stored fragments counter.  */
    trStoredFragments[triangleID] = 0;

    return triangleID;
}

/*  Sets up edge equations for a previously bound triangle, from precalculated data.  */
void RasterizerEmulator::setupEdgeEquations(u32bit triangleID, QuadFloat A, QuadFloat B, QuadFloat C, f32bit tArea)
{
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    SetupTriangle *triangle;
    f64bit area;
    QuadFloat *vAttr1;
    QuadFloat *vAttr2;
    QuadFloat *vAttr3;

    /*  Check if it is a valid triangle identifier.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "setupEdgeEquations", "Wrong setup triangle identifier.");
    )

    /*  Check if non-empty triangle entry.  */
    GPU_ASSERT(
        if (setupTriangles[triangleID] == NULL)
           panic("RasterizerEmulator", "setupEdgeEquations", "Setup Triangle table entry is empty.");
    )

    /*  Check if entry is a pre-bound triangle.  */
    GPU_ASSERT(
        if (!setupTriangles[triangleID]->isPreBoundTriangle())
           panic("RasterizerEmulator", "setupEdgeEquations", "Triangle identifier returns a non pre-bound triangle.");
    )

    /*  Retrieve pointer to the setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Get vertex attributes.  */
    setupTriangles[triangleID]->getVertexAttributes(vAttr1, vAttr2, vAttr3);

//printf("Vertex 1 Position = {%f, %f, %f, %f}\n", vAttr1[POSITION_ATTRIBUTE][0], vAttr1[POSITION_ATTRIBUTE][1],
//vAttr1[POSITION_ATTRIBUTE][2], vAttr1[POSITION_ATTRIBUTE][3]);
//printf("Vertex 2 Position = {%f, %f, %f, %f}\n", vAttr2[POSITION_ATTRIBUTE][0], vAttr2[POSITION_ATTRIBUTE][1],
//vAttr2[POSITION_ATTRIBUTE][2], vAttr2[POSITION_ATTRIBUTE][3]);
//printf("Vertex 3 Position = {%f, %f, %f, %f}\n", vAttr3[POSITION_ATTRIBUTE][0], vAttr3[POSITION_ATTRIBUTE][1],
//vAttr3[POSITION_ATTRIBUTE][2], vAttr3[POSITION_ATTRIBUTE][3]);

    /*  Unpack and convert to 64 bits precalculated edge and z equations.  */
    edge1[0] = f64bit(A[0]);
    edge1[1] = f64bit(B[0]);
    edge1[2] = f64bit(C[0]);
    edge2[0] = f64bit(A[1]);
    edge2[1] = f64bit(B[1]);
    edge2[2] = f64bit(C[1]);
    edge3[0] = f64bit(A[2]);
    edge3[1] = f64bit(B[2]);
    edge3[2] = f64bit(C[2]);
    zeq[0] = f64bit(A[3]);
    zeq[1] = f64bit(B[3]);
    zeq[2] = f64bit(C[3]);

//printf("Edge Eq 1 => {%f, %f, %f}\n", edge1[0], edge1[1], edge1[2]);
//printf("Edge Eq 2 => {%f, %f, %f}\n", edge2[0], edge2[1], edge2[2]);
//printf("Edge Eq 3 => {%f, %f, %f}\n", edge3[0], edge3[1], edge3[2]);
//printf("Z Interpolation Eq => {%f, %f, %f}\n", zeq[0], zeq[1], zeq[2]);

    /*  Convert area to 64 bits.  */
    area = f64bit(tArea);

    /*  Set the triangle area.  */
    triangle->setArea(area);

    /*  Set the setup triangle three edge equations.  */
    triangle->setEdgeEquations(edge1, edge2, edge3);

    /*  Set the z interpolation equation of the triangle.  */
    triangle->setZEquation(zeq);
}

/*  Calculates the start position for scanline rasterization of the setup triangle.  */
void RasterizerEmulator::startPosition(u32bit triangle, bool msaaEnabled)
{
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    QuadFloat *vertex1, *vertex2, *vertex3;
    QuadFloat topVertex;
    s32bit startX;
    s32bit startY;
    RasterDirection startDir;
    s32bit leftBound, bottomBound, rightBound, upperBound;

    /*  Check if it is a valid triangle identifier.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "startPosition", "Wrong setup triangle identifier.");
    )

    /*  Check if the triangle ID points to an active setup triangle.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "startPosition", "Setup triangle not in the table.");
    )

    /*  Get triangle vertices.  */
    vertex1 = setupTriangles[triangle]->getAttribute(0, POSITION_ATTRIBUTE);
    vertex2 = setupTriangles[triangle]->getAttribute(1, POSITION_ATTRIBUTE);
    vertex3 = setupTriangles[triangle]->getAttribute(2, POSITION_ATTRIBUTE);

    /*  Get triangle edge equations.  */
    setupTriangles[triangle]->getEdgeEquations(edge1, edge2, edge3);

    /*  Get the triangle z interpolation equation.  */
    setupTriangles[triangle]->getZEquation(zeq);

    /*  Get the triangle bounding box.  */
    setupTriangles[triangle]->getBoundingBox(leftBound, bottomBound, rightBound, upperBound);

//printf("RastEmu => Start Position\n");
//printf("V1 = { %f, %f, %f, %f } -> { %f, %f, %f}\n",
//(*vertex1)[0], (*vertex1)[1], (*vertex1)[2], (*vertex1)[3],
//(*vertex1)[0] / (*vertex1)[3], (*vertex1)[1] / (*vertex1)[3], (*vertex1)[2] / (*vertex1)[3]);
//printf("V2 = { %f, %f, %f, %f } -> { %f, %f, %f}\n",
//(*vertex2)[0], (*vertex2)[1], (*vertex2)[2], (*vertex2)[3],
//(*vertex2)[0] / (*vertex2)[3], (*vertex2)[1] / (*vertex2)[3], (*vertex2)[2] / (*vertex2)[3]);
//printf("V3 = { %f, %f, %f, %f } -> { %f, %f, %f}\n",
//(*vertex3)[0], (*vertex3)[1], (*vertex3)[2], (*vertex3)[3],
//(*vertex3)[0] / (*vertex3)[3], (*vertex3)[1] / (*vertex3)[3], (*vertex3)[2] / (*vertex3)[3]);

    /*  Calculate the rasterization start position.  */
    GPUMath::startPosition(*vertex1, *vertex2, *vertex3, edge1, edge2, edge3,
        x0, y0, w, h, scissorX0, scissorY0, scissorW, scissorH,
        leftBound, bottomBound, rightBound, upperBound, startX, startY, startDir);

//printf("start = { %d, %d }\n", startX, startY);

    /*  Adjust the start position to the stamp grid.  */
    startX = startX - GPU_MOD(startX, scanTileWidth);
    startY = startY - GPU_MOD(startY, scanTileHeight);

//printf("start (tiled) = { %d, %d }\n", startX, startY);

    //  Select a sample point that is (-0.5f, -0.5f) from the pixel position for
    //  when MSAA is enabled.
    f32bit xSampleOffset = msaaEnabled ? -0.5f : 0.0f;
    f32bit ySampleOffset = msaaEnabled ? -0.5f : 0.0f;

    /*  Store in c component the edge equation values for
        top most vertex.  */
    edge1[2] = edge1[0] * (f64bit) (startX + xSampleOffset) + edge1[1] * (f64bit) (startY + ySampleOffset) + edge1[2];
    edge2[2] = edge2[0] * (f64bit) (startX + xSampleOffset) + edge2[1] * (f64bit) (startY + ySampleOffset) + edge2[2];
    edge3[2] = edge3[0] * (f64bit) (startX + xSampleOffset) + edge3[1] * (f64bit) (startY + ySampleOffset) + edge3[2];

    /*  Calculate the z interpolation equation value into c coefficient.  */
    zeq[2] = zeq[0] * (f64bit) (startX + xSampleOffset) + zeq[1] * (f64bit) (startY + ySampleOffset) + zeq[2];

    /*  Save raster start position.  */
    setupTriangles[triangle]->saveRasterStart();

    /*  Set the setup triangle three edge equations.  */
    setupTriangles[triangle]->setEdgeEquations(edge1, edge2, edge3);

    /*  Set the triangle z interpolation equation.  */
    setupTriangles[triangle]->setZEquation(zeq);

    /*  Set triangle start rasterization position => top most vertex.  */
    setupTriangles[triangle]->setRasterPosition(startX, startY);

    /*  Set triangle start rasterization direction.  */
    setupTriangles[triangle]->setDirection(startDir);

    /*  Set triangle start tile rasterization direction.  */
    setupTriangles[triangle]->setTileDirection(CENTER_DIR);

    /*  Set as next stamp as not first.  */
    setupTriangles[triangle]->setFirstStamp(TRUE);
}

/*  Changes the current raster position for the triangle updating the edge and z equations.  */
void RasterizerEmulator::changeRasterPosition(u32bit triangle, s32bit x, s32bit y)
{
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];

    /*  Check if it is a valid triangle identifier.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "startPosition", "Wrong setup triangle identifier.");
    )

    /*  Check if the triangle ID points to an active setup triangle.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "startPosition", "Setup triangle not in the table.");
    )

    /*  Restored saved start raster position.  */
    setupTriangles[triangle]->restore(RASTER_START_SAVE);

    /*  Save start raster position.  */
    setupTriangles[triangle]->saveRasterStart();

    /*  Get triangle edge equations.  */
    setupTriangles[triangle]->getEdgeEquations(edge1, edge2, edge3);

    /*  Get the triangle z interpolation equation.  */
    setupTriangles[triangle]->getZEquation(zeq);

    /*  Adjust the start position to the stamp grid.  */
    x = x - GPU_MOD(x, scanTileWidth);
    y = y - GPU_MOD(y, scanTileHeight);

    /*  Store in c component the edge equation values for
        top most vertex.  */
    edge1[2] = edge1[0] * (f64bit) x + edge1[1] * (f64bit) y + edge1[2];
    edge2[2] = edge2[0] * (f64bit) x + edge2[1] * (f64bit) y + edge2[2];
    edge3[2] = edge3[0] * (f64bit) x + edge3[1] * (f64bit) y + edge3[2];

    /*  Calculate the z interpolation equation value into c coefficient.  */
    zeq[2] = zeq[0] * (f64bit) x + zeq[1] * (f64bit) y + zeq[2];

    /*  Set the setup triangle three edge equations.  */
    setupTriangles[triangle]->setEdgeEquations(edge1, edge2, edge3);

    /*  Set the triangle z interpolation equation.  */
    setupTriangles[triangle]->setZEquation(zeq);

    /*  Set triangle start rasterization position => top most vertex.  */
    setupTriangles[triangle]->setRasterPosition(x, y);
}


/*  Destroys a setup triangle.  */
void RasterizerEmulator::destroyTriangle(u32bit triangle)
{
    //QuadFloat *vAttr1, *vAttr2, *vAttr3;

    /*  Check if it is a valid triangle identifier.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "destroyTriangle", "Wrong setup triangle identifier.");
    )

    /*  Check if the triangle ID points to an active setup triangle.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "destroyTriangle", "Setup triangle already destroyed.");
    )

    /*  Check we are not deleting too many setup entries.  */
    GPU_ASSERT(
        if (freeSetups == activeTriangles)
            panic("RasterizerEmulator", "destroyTriangle", "All setup triangles already deleted.");
    )

    /*  Delete the triangle.  */
    //delete setupTriangles[triangle];
    setupTriangles[triangle]->deleteReference();

    /*  Set setup triangle table entry to NULL.  */
    setupTriangles[triangle] = NULL;

    /*  Add triangle to the free setup list.  */
    freeSetupList[GPU_MOD(nextFreeSetup + freeSetups, activeTriangles)] = triangle;

    /*  Update free setup entries counter.  */
    freeSetups++;

    /*  Update setup triangles counter.  */
    triangles--;
}


/*  Returns the corresponding setup triangle.  */
SetupTriangle* RasterizerEmulator::getSetupTriangle(u32bit triangle)
{
    /*  Check if it is a valid triangle identifier.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "getSetupTriangle", "Wrong setup triangle identifier.");
    )

    /*  Check if the triangle ID points to an active setup triangle.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "getSetupTriangle", "Non-valid setup triangle entry.");
    )

    return setupTriangles[triangle];
}

//  Maps a fragment z value from float point to fixed point into the
//  [0..2n] range, where n is the Z buffer bit precission.
u32bit RasterizerEmulator::convertZ(f64bit z)
{
    f64bit zw;
    u32bit zc;

    //  Check if the z comes in the [0, 1] range (D3D9) or the [-1, 1] range (OpenGL).
    if (d3d9DepthRange)
    {
        //  Map the allocated depth range to [0, 1] range.
        zw = f64bit(f - n) * z + f64bit(n);
    }
    else
    {
        //  Map the allocated depth range to [0, 1] range.
        zw = f64bit((f - n) / 2.0f) * z + f64bit((n + f) / 2.0f);
    }

    //  Second convert to fixed point using allocated depth buffer bit precission.
    zc = u32bit(zw * f64bit((1 << depthBitPrecission) - 1));

    return zc;
}

/*  Scan a stamp to the left.  */
bool RasterizerEmulator::scanTileLeft(f64bit *edgeC, f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit x, s32bit leftBound, bool extended)
{
    f64bit leftC[4][4];
    bool continueToLeft;
    //u32bit i;

    /*  Update horizontal position.  */
    x -= scanTileWidth;

    /*  Check against the triangle left bound.  */
    if (x < (leftBound - s32bit(GPU_MOD(leftBound, scanTileWidth))))
        return FALSE;

    /*  Check if sample outside the scissor box (adjusted to scan tile grid).  */
    if (x < (scissorX0 - s32bit(GPU_MOD(scissorX0, scanTileWidth))))
        return FALSE;

    /*  Selects scan method (tile or linial based) based on the defined threshold.  */
    //if (scanTileHeight > SCAN_METHOD_THRESHOLD)
    //{
    /*  Get the 4 tile corners as sample points.  */

    /*  Check if extended scan is used.  */
    if (extended)
    {
        /*  Rigth bottom corner.  */
        leftC[0][0] = edgeC[0] + offset[0];
        leftC[0][1] = edgeC[1] + offset[1];
        leftC[0][2] = edgeC[2] + offset[2];
        leftC[0][3] = edgeC[3];

        /*  Rigth top corner.  */
        leftC[1][0] = edgeC[0] + edge1[1] * 2 * scanTileHeight + offset[0];
        leftC[1][1] = edgeC[1] + edge2[1] * 2 * scanTileHeight + offset[1];
        leftC[1][2] = edgeC[2] + edge3[1] * 2 * scanTileHeight + offset[2];
        leftC[1][3] = edgeC[3] + zeq[1] * 2 * scanTileHeight;

        /*  Left bottom corner.  */
        leftC[2][0] = edgeC[0] - edge1[0] * 2 * scanTileWidth + offset[0];
        leftC[2][1] = edgeC[1] - edge2[0] * 2 * scanTileWidth + offset[1];
        leftC[2][2] = edgeC[2] - edge3[0] * 2 * scanTileWidth + offset[2];
        leftC[2][3] = edgeC[3] - zeq[0] * 2 * scanTileWidth;

        /*  Left top corner.  */
        leftC[3][0] = edgeC[0] - edge1[0] * 2 * scanTileWidth + edge1[1] * 2 * scanTileHeight + offset[0];
        leftC[3][1] = edgeC[1] - edge2[0] * 2 * scanTileWidth + edge2[1] * 2 * scanTileHeight + offset[1];
        leftC[3][2] = edgeC[2] - edge3[0] * 2 * scanTileWidth + edge3[1] * 2 * scanTileHeight + offset[2];
        leftC[3][3] = edgeC[3] - zeq[0] * 2 * scanTileWidth + zeq[1] * 2 * scanTileHeight;
    }
    else
    {
        /*  Rigth bottom corner.  */
        leftC[0][0] = edgeC[0] + offset[0];
        leftC[0][1] = edgeC[1] + offset[1];
        leftC[0][2] = edgeC[2] + offset[2];
        leftC[0][3] = edgeC[3];

        /*  Rigth top corner.  */
        leftC[1][0] = edgeC[0] + edge1[1] * scanTileHeight + offset[0];
        leftC[1][1] = edgeC[1] + edge2[1] * scanTileHeight + offset[1];
        leftC[1][2] = edgeC[2] + edge3[1] * scanTileHeight + offset[2];
        leftC[1][3] = edgeC[3] + zeq[1] * scanTileHeight;

        /*  Left bottom corner.  */
        leftC[2][0] = edgeC[0] - edge1[0] * scanTileWidth + offset[0];
        leftC[2][1] = edgeC[1] - edge2[0] * scanTileWidth + offset[1];
        leftC[2][2] = edgeC[2] - edge3[0] * scanTileWidth + offset[2];
        leftC[2][3] = edgeC[3] - zeq[0] * scanTileWidth;

        /*  Left top corner.  */
        leftC[3][0] = edgeC[0] - edge1[0] * scanTileWidth + edge1[1] * scanTileHeight + offset[0];
        leftC[3][1] = edgeC[1] - edge2[0] * scanTileWidth + edge2[1] * scanTileHeight + offset[1];
        leftC[3][2] = edgeC[2] - edge3[0] * scanTileWidth + edge3[1] * scanTileHeight + offset[2];
        leftC[3][3] = edgeC[3] - zeq[0] * scanTileWidth + zeq[1] * scanTileHeight;
    }

    /*  Evaluate if there may be a triangle fragment inside the extended scan tile.  */
    continueToLeft = GPUMath::evaluateTile(leftC[0], leftC[1], leftC[2], leftC[3]);
    //}
    //else
    //{
    //    /*  Get the scanTileHeight samples to the left.  */

    //    /*  Set continue left to false until a fragment inside the triangle is found.  */
    //    continueToLeft = FALSE;

    //    /*  Sample the right side of the left scan tile.  */
    //    for(i = 0; (i < scanTileHeight) && !continueToLeft; i++)
    //    {
    //        /*  Sample a fragment from the right side of the left scan tile.  */
    //        leftC[0][0] = edgeC[0] - edge1[0] + edge1[1] * i;
    //        leftC[0][1] = edgeC[1] - edge2[0] + edge2[1] * i;
    //        leftC[0][2] = edgeC[2] - edge3[0] + edge3[1] * i;
    //        leftC[0][3] = edgeC[3] - zeq[0] + zeq[1] * i;

            /*  Test if the fragment is inside the triangle.  */
    //        continueToLeft = GPU_IS_POSITIVE(leftC[0][0] + offset[0]) &&
    //            GPU_IS_POSITIVE(leftC[0][1] + offset[1]) &&
    //            GPU_IS_POSITIVE(leftC[0][2] + offset[2]);
    //    }
    //}

    /*  Calculate edge and z interpolation equation values one scan tile to the
        left from the current position.  */
    leftC[0][0] = edgeC[0] - edge1[0] * scanTileWidth;
    leftC[0][1] = edgeC[1] - edge2[0] * scanTileWidth;
    leftC[0][2] = edgeC[2] - edge3[0] * scanTileWidth;
    leftC[0][3] = edgeC[3] - zeq[0] * scanTileWidth;

    /*  If continue rasterization to the left store new edge and z interpolation
        equation values and return.  */
    //if (continueToLeft)
    //{
        edgeC[0] = leftC[0][0];
        edgeC[1] = leftC[0][1];
        edgeC[2] = leftC[0][2];
        edgeC[3] = leftC[0][3];
    //}

    return (continueToLeft);
}


/*  Scan a stamp to the right.  */
bool RasterizerEmulator::scanTileRight(f64bit *edgeC, f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit x, s32bit rightBound, bool extended)
{
    f64bit rightC[4][4];
    bool continueToRight;
    //u32bit i;

    /*  Update horizontal position.  */
    x += scanTileWidth;

    /*  Check against the triangle right bound.  */
    if (x > rightBound)
        return FALSE;

    /*  Check if sample outside the scissor box (adjusted to scan tile grid).  */
    if (x >= (scissorX0 + s32bit(scissorW)))
        return FALSE;

    /*  Selects scan method (tile or linial based) based on the defined threshold.  */
    //if (scanTileHeight > SCAN_METHOD_THRESHOLD)
    //{
    /*  Get the 4 corners sample points for the scan tile to the right.  */

    /*  Check if extended scan is used.  */
    if (extended)
    {
        /*  Left bottom corner.  */
        rightC[0][0] = edgeC[0] + edge1[0] * scanTileWidth - edge1[1] * scanTileHeight + offset[0];
        rightC[0][1] = edgeC[1] + edge2[0] * scanTileWidth - edge2[1] * scanTileHeight + offset[1];
        rightC[0][2] = edgeC[2] + edge3[0] * scanTileWidth - edge3[1] * scanTileHeight + offset[2];
        rightC[0][3] = edgeC[3] + zeq[0] * scanTileWidth - zeq[1] * scanTileHeight;

        /*  Left top corner.  */
        rightC[1][0] = edgeC[0] + edge1[0] * scanTileWidth + edge1[1] * scanTileHeight + offset[0];
        rightC[1][1] = edgeC[1] + edge2[0] * scanTileWidth + edge2[1] * scanTileHeight + offset[1];
        rightC[1][2] = edgeC[2] + edge3[0] * scanTileWidth + edge3[1] * scanTileHeight + offset[2];
        rightC[1][3] = edgeC[3] + zeq[0] * scanTileWidth + zeq[1] * scanTileHeight;

        /*  Right bottom corner.  */
        rightC[2][0] = edgeC[0] + edge1[0] * 3 * scanTileWidth - edge1[1] * scanTileHeight + offset[0];
        rightC[2][1] = edgeC[1] + edge2[0] * 3 * scanTileWidth - edge2[1] * scanTileHeight + offset[1];
        rightC[2][2] = edgeC[2] + edge3[0] * 3 * scanTileWidth - edge3[1] * scanTileHeight + offset[2];
        rightC[2][3] = edgeC[3] + zeq[0] * 3 * scanTileWidth - zeq[1] * scanTileHeight;

        /*  Right top corner.  */
        rightC[3][0] = edgeC[0] + edge1[0] * 3 * scanTileWidth + edge1[1] * scanTileHeight + offset[0];
        rightC[3][1] = edgeC[1] + edge2[0] * 3 * scanTileWidth + edge2[1] * scanTileHeight + offset[1];
        rightC[3][2] = edgeC[2] + edge3[0] * 3 * scanTileWidth + edge3[1] * scanTileHeight + offset[2];
        rightC[3][3] = edgeC[3] + zeq[0] * 3 * scanTileWidth + zeq[1] * scanTileHeight;
    }
    else
    {
        /*  Left bottom corner.  */
        rightC[0][0] = edgeC[0] + edge1[0] * scanTileWidth + offset[0];
        rightC[0][1] = edgeC[1] + edge2[0] * scanTileWidth + offset[1];
        rightC[0][2] = edgeC[2] + edge3[0] * scanTileWidth + offset[2];
        rightC[0][3] = edgeC[3] + zeq[0] * scanTileWidth;

        /*  Left top corner.  */
        rightC[1][0] = edgeC[0] + edge1[0] * scanTileWidth + edge1[1] * scanTileHeight + offset[0];
        rightC[1][1] = edgeC[1] + edge2[0] * scanTileWidth + edge2[1] * scanTileHeight + offset[1];
        rightC[1][2] = edgeC[2] + edge3[0] * scanTileWidth + edge3[1] * scanTileHeight + offset[2];
        rightC[1][3] = edgeC[3] + zeq[0] * scanTileWidth + zeq[1] * scanTileHeight;

        /*  Right bottom corner.  */
        rightC[2][0] = edgeC[0] + edge1[0] * 2 * scanTileWidth + offset[0];
        rightC[2][1] = edgeC[1] + edge2[0] * 2 * scanTileWidth + offset[1];
        rightC[2][2] = edgeC[2] + edge3[0] * 2 * scanTileWidth + offset[2];
        rightC[2][3] = edgeC[3] + zeq[0] * 2 * scanTileWidth;

        /*  Right top corner.  */
        rightC[3][0] = edgeC[0] + edge1[0] * 2 * scanTileWidth + edge1[1] * scanTileHeight + offset[0];
        rightC[3][1] = edgeC[1] + edge2[0] * 2 * scanTileWidth + edge2[1] * scanTileHeight + offset[1];
        rightC[3][2] = edgeC[2] + edge3[0] * 2 * scanTileWidth + edge3[1] * scanTileHeight + offset[2];
        rightC[3][3] = edgeC[3] + zeq[0] * 2 * scanTileWidth + zeq[1] * scanTileHeight;
    }

    /*  Evaluate if there may be a triangle fragment inside the scan tile.  */
    continueToRight = GPUMath::evaluateTile(rightC[0], rightC[1], rightC[2], rightC[3]);
    //}
    //else
    //{
    //    /*  Get the scanTileHeight samples to the right.  */

    //    /*  Set continue right to false until a fragment inside the triangle is found.  */
    //    continueToRight = FALSE;

    //   /*  Sample the left side of the right scan tile.  */
    //    for(i = 0; (i < scanTileHeight) && !continueToRight; i++)
    //    {
    //        /*  Sample a fragment from the left side of the right scan tile.  */
    //        rightC[0][0] = edgeC[0] + edge1[0] * scanTileWidth + edge1[1] * i;
    //        rightC[0][1] = edgeC[1] + edge2[0] * scanTileWidth + edge2[1] * i;
    //        rightC[0][2] = edgeC[2] + edge3[0] * scanTileWidth + edge3[1] * i;
    //        rightC[0][3] = edgeC[3] + zeq[0] * scanTileWidth + zeq[1] * i;

    //        /*  Test if the fragment is inside the triangle.  */
    //        continueToRight = GPU_IS_POSITIVE(rightC[0][0] + offset[0]) &&
    //            GPU_IS_POSITIVE(rightC[0][1] + offset[1]) &&
    //            GPU_IS_POSITIVE(rightC[0][2] + offset[2]);
    //    }
    //}

    /*  Calculate edge and z interpolation equation values one scan tile to the
        right from the current position.  */
    rightC[0][0] = edgeC[0] + edge1[0] * scanTileWidth;
    rightC[0][1] = edgeC[1] + edge2[0] * scanTileWidth;
    rightC[0][2] = edgeC[2] + edge3[0] * scanTileWidth;
    rightC[0][3] = edgeC[3] + zeq[0] * scanTileWidth;

    /*  If continue rasterization to the right store new edge and z interpolation
        equation values and return.  */
    //if (continueToRight)
    //{
        edgeC[0] = rightC[0][0];
        edgeC[1] = rightC[0][1];
        edgeC[2] = rightC[0][2];
        edgeC[3] = rightC[0][3];
    //}

    return (continueToRight);
}

/*  Scan one stamp down.  */
bool RasterizerEmulator::scanTileDown(f64bit *edgeC, f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit y, s32bit bottomBound, bool extended)
{
    f64bit downC[4][4];
    bool continueDown;
    //u32bit i;


    /*  Update vertical position.  */
    y -= scanTileHeight;

    /*  Check against the triangle bottom bound.  */
    if (y < (bottomBound - s32bit(GPU_MOD(bottomBound, scanTileHeight))))
        return FALSE;

    /*  Check if sample outside the scissor box (adjusted to scan tile grid).  */
    if (y < (scissorY0 - (s32bit) GPU_MOD(scissorY0, scanTileHeight)))
        return FALSE;

    ///*  Selects scan method (tile or linial based) based on the defined threshold.  */
    //if (scanTileWidth > SCAN_METHOD_THRESHOLD)
    //{
    /*  Get the 4 corners as sample points for the scan tile to the bottom.  */

    /*  Check if extended scan is to be used.  */
    if (extended)
    {
        /*  Left bottom corner.  */
        downC[0][0] = edgeC[0] - edge1[0] * scanTileWidth - edge1[1] * 2 * scanTileHeight + offset[0];
        downC[0][1] = edgeC[1] - edge2[0] * scanTileWidth - edge2[1] * 2 * scanTileHeight + offset[1];
        downC[0][2] = edgeC[2] - edge3[0] * scanTileWidth - edge3[1] * 2 * scanTileHeight + offset[2];
        downC[0][3] = edgeC[3] - zeq[0] * scanTileWidth - zeq[1] * 2 * scanTileHeight;

        /*  Left top corner.  */
        downC[1][0] = edgeC[0] - edge1[0] * scanTileWidth + offset[0];
        downC[1][1] = edgeC[1] - edge2[0] * scanTileWidth + offset[1];
        downC[1][2] = edgeC[2] - edge3[0] * scanTileWidth + offset[2];
        downC[1][3] = edgeC[3] - zeq[0] * scanTileWidth;

        /*  Right bottom corner.  */
        downC[2][0] = edgeC[0] + edge1[0] * scanTileWidth - edge1[1] * 2 * scanTileHeight + offset[0];
        downC[2][1] = edgeC[1] + edge2[0] * scanTileWidth - edge2[1] * 2 * scanTileHeight + offset[1];
        downC[2][2] = edgeC[2] + edge3[0] * scanTileWidth - edge3[1] * 2 * scanTileHeight + offset[2];
        downC[2][3] = edgeC[3] + zeq[0] * scanTileWidth - zeq[1] * 2 * scanTileHeight;

        /*  Right top corner.  */
        downC[3][0] = edgeC[0] + edge1[0] * scanTileWidth + offset[0];
        downC[3][1] = edgeC[1] + edge2[0] * scanTileWidth + offset[1];
        downC[3][2] = edgeC[2] + edge3[0] * scanTileWidth + offset[2];
        downC[3][3] = edgeC[3] + zeq[0] * scanTileWidth;
    }
    else
    {
        /*  Left bottom corner.  */
        downC[0][0] = edgeC[0] - edge1[1] * scanTileHeight + offset[0];
        downC[0][1] = edgeC[1] - edge2[1] * scanTileHeight + offset[1];
        downC[0][2] = edgeC[2] - edge3[1] * scanTileHeight + offset[2];
        downC[0][3] = edgeC[3] - zeq[1] * scanTileHeight;

        /*  Left top corner.  */
        downC[1][0] = edgeC[0] + offset[0];
        downC[1][1] = edgeC[1] + offset[1];
        downC[1][2] = edgeC[2] + offset[2];
        downC[1][3] = edgeC[3];

        /*  Right bottom corner.  */
        downC[2][0] = edgeC[0] + edge1[0] * scanTileWidth - edge1[1] * scanTileHeight + offset[0];
        downC[2][1] = edgeC[1] + edge2[0] * scanTileWidth - edge2[1] * scanTileHeight + offset[1];
        downC[2][2] = edgeC[2] + edge3[0] * scanTileWidth - edge3[1] * scanTileHeight + offset[2];
        downC[2][3] = edgeC[3] + zeq[0] * scanTileWidth - zeq[1] * scanTileHeight;

        /*  Right top corner.  */
        downC[3][0] = edgeC[0] + edge1[0] * scanTileWidth + offset[0];
        downC[3][1] = edgeC[1] + edge2[0] * scanTileWidth + offset[1];
        downC[3][2] = edgeC[2] + edge3[0] * scanTileWidth + offset[2];
        downC[3][3] = edgeC[3] + zeq[0] * scanTileWidth;
    }

    /*  Evaluate if there may be a triangle fragment inside the scan tile.  */
    continueDown = GPUMath::evaluateTile(downC[0], downC[1], downC[2], downC[3]);
    //}
    //else
    //{
    //    /*  Get the scanTileWidth samples above the current scan tile.  */

    //    /*  Set continue down to false until a fragment inside the triangle is found.  */
    //    continueDown = FALSE;

    //    /*  Sample the top side of the scan tile below.  */
    //    for(i = 0; (i < scanTileWidth) && !continueDown; i++)
    //    {
    //        /*  Sample a fragment from the top side of the scan tile below.  */
    //        downC[0][0] = edgeC[0] - edge1[1] + edge1[0] * i;
    //        downC[0][1] = edgeC[1] - edge2[1] + edge2[0] * i;
    //        downC[0][2] = edgeC[2] - edge3[1] + edge3[0] * i;
    //        downC[0][3] = edgeC[3] - zeq[1] + zeq[0] * i;

    //        /*  Test if the fragment is inside the triangle.  */
    //        continueDown = GPU_IS_POSITIVE(downC[0][0] + offset[0]) &&
    //            GPU_IS_POSITIVE(downC[0][1] + offset[1]) &&
    //            GPU_IS_POSITIVE(downC[0][2] + offset[2]);
    //    }
    //}

    /*  Calculate edge and z interpolation equation values one scan tile
        down from the current position.  */
    downC[0][0] = edgeC[0] - edge1[1] * scanTileHeight;
    downC[0][1] = edgeC[1] - edge2[1] * scanTileHeight;
    downC[0][2] = edgeC[2] - edge3[1] * scanTileHeight;
    downC[0][3] = edgeC[3] - zeq[1] * scanTileHeight;

    /*  If continue rasterization down store new edge and z interpolation
        equation values and return.  */
    //if (continueDown)
    //{
        edgeC[0] = downC[0][0];
        edgeC[1] = downC[0][1];
        edgeC[2] = downC[0][2];
        edgeC[3] = downC[0][3];
    //}

    return continueDown;
}

/*  Scan one stamp up.  */
bool RasterizerEmulator::scanTileUp(f64bit *edgeC, f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit y, s32bit upperBound, bool extended)
{
    f64bit upC[4][4];
    bool continueUp;
    //u32bit i;

    /*  Update vertical position.  */
    y += scanTileHeight;

    /*  Check against the triangle upper bound.  */
    if (y >= upperBound)
        return FALSE;

    /*  Check if sample outside the scissor box (adjusted to scan tile grid).  */
    if (y >= (scissorY0 + (s32bit) scissorH))
        return FALSE;

    ///*  Selects scan method (tile or linial based) based on the defined threshold.  */
    //if (scanTileWidth > SCAN_METHOD_THRESHOLD)
    //{
    /*  Get the 4 corners as sample points for the scan tile to the bottom.  */

    /*  Check if extended scan is enabled.  */
    if (extended)
    {
        /*  Left top corner.  */
        upC[0][0] = edgeC[0] + edge1[1] * 3 * scanTileHeight;
        upC[0][1] = edgeC[1] + edge2[1] * 3 * scanTileHeight;
        upC[0][2] = edgeC[2] + edge3[1] * 3 * scanTileHeight;
        upC[0][3] = edgeC[3] + zeq[1] * 3 * scanTileHeight;

        /*  Left bottom corner.  */
        upC[1][0] = edgeC[0] + edge1[1] * scanTileHeight;
        upC[1][1] = edgeC[1] + edge2[1] * scanTileHeight;
        upC[1][2] = edgeC[2] + edge3[1] * scanTileHeight;
        upC[1][3] = edgeC[3] + zeq[1] * scanTileHeight;

        /*  Right top corner.  */
        upC[2][0] = edgeC[0] + edge1[0] * 2 * scanTileWidth + edge1[1] * 3 * scanTileHeight;
        upC[2][1] = edgeC[1] + edge2[0] * 2 * scanTileWidth + edge2[1] * 3 * scanTileHeight;
        upC[2][2] = edgeC[2] + edge3[0] * 2 * scanTileWidth + edge3[1] * 3 * scanTileHeight;
        upC[2][3] = edgeC[3] + zeq[0] * 2 * scanTileWidth + zeq[1] * 3 * scanTileHeight;

        /*  Right bottom corner.  */
        upC[3][0] = edgeC[0] + edge1[0] * 2 * scanTileWidth + edge1[1] * scanTileHeight;
        upC[3][1] = edgeC[1] + edge2[0] * 2 * scanTileWidth + edge2[1] * scanTileHeight;
        upC[3][2] = edgeC[2] + edge3[0] * 2 * scanTileWidth + edge3[1] * scanTileHeight;
        upC[3][3] = edgeC[3] + zeq[0] * 2 * scanTileWidth + zeq[1] * scanTileHeight;
    }
    else
    {
        /*  Left top corner.  */
        upC[0][0] = edgeC[0] + edge1[1] * 2 * scanTileHeight;
        upC[0][1] = edgeC[1] + edge2[1] * 2 * scanTileHeight;
        upC[0][2] = edgeC[2] + edge3[1] * 2 * scanTileHeight;
        upC[0][3] = edgeC[3] + zeq[1] * 2 * scanTileHeight;

        /*  Left bottom corner.  */
        upC[1][0] = edgeC[0] + edge1[1] * scanTileHeight;
        upC[1][1] = edgeC[1] + edge2[1] * scanTileHeight;
        upC[1][2] = edgeC[2] + edge3[1] * scanTileHeight;
        upC[1][3] = edgeC[3] + zeq[1] * scanTileHeight;

        /*  Right top corner.  */
        upC[2][0] = edgeC[0] + edge1[0] * scanTileWidth + edge1[1] * 2 * scanTileHeight;
        upC[2][1] = edgeC[1] + edge2[0] * scanTileWidth + edge2[1] * 2 * scanTileHeight;
        upC[2][2] = edgeC[2] + edge3[0] * scanTileWidth + edge3[1] * 2 * scanTileHeight;
        upC[2][3] = edgeC[3] + zeq[0] * scanTileWidth + zeq[1] * 2 * scanTileHeight;

        /*  Right bottom corner.  */
        upC[3][0] = edgeC[0] + edge1[0] * scanTileWidth + edge1[1] * scanTileHeight;
        upC[3][1] = edgeC[1] + edge2[0] * scanTileWidth + edge2[1] * scanTileHeight;
        upC[3][2] = edgeC[2] + edge3[0] * scanTileWidth + edge3[1] * scanTileHeight;
        upC[3][3] = edgeC[3] + zeq[0] * scanTileWidth + zeq[1] * scanTileHeight;
    }

    /*  Evaluate if there may be a triangle fragment inside the scan tile.  */
    continueUp = GPUMath::evaluateTile(upC[0], upC[1], upC[2], upC[3]);
    //}
    //else
    //{
    //    /*  Get the scanTileWidth samples over the current scan tile.  */

    //    /*  Set continue up to false until a fragment inside
    //        the triangle is found.  */
    //    continueUp = FALSE;

    //    /*  Sample the bottom side of the scan tile over the current.  */
    //    for(i = 0; (i < scanTileWidth) && !continueUp; i++)
    //    {
    //        /*  Sample a fragment from the bottom side of the scan tile over the current.  */
    //        upC[0][0] = edgeC[0] + edge1[1] * scanTileHeight + edge1[0] * i;
    //        upC[0][1] = edgeC[1] + edge2[1] * scanTileHeight + edge2[0] * i;
    //        upC[0][2] = edgeC[2] + edge3[1] * scanTileHeight + edge3[0] * i;
    //        upC[0][3] = edgeC[3] + zeq[1] * scanTileHeight + zeq[0] * i;

    //        /*  Test if the fragment is inside the triangle.  */
    //        continueUp = GPU_IS_POSITIVE(upC[0][0] + offset[0]) &&
    //            GPU_IS_POSITIVE(upC[0][1] + offset[1]) &&
    //            GPU_IS_POSITIVE(upC[0][2] + offset[2]);
    //    }
    //}

    /*  Calculate edge and z interpolation equation values one scan tile
        over the current position.  */
    upC[0][0] = edgeC[0] + edge1[1] * scanTileHeight;
    upC[0][1] = edgeC[1] + edge2[1] * scanTileHeight;
    upC[0][2] = edgeC[2] + edge3[1] * scanTileHeight;
    upC[0][3] = edgeC[3] + zeq[1] * scanTileHeight;

    /*  If continue rasterization up store new edge and z interpolation
        equation values and return.  */
    //if (continueUp)
    //{
        edgeC[0] = upC[0][0];
        edgeC[1] = upC[0][1];
        edgeC[2] = upC[0][2];
        edgeC[3] = upC[0][3];
    //}

    return continueUp;
}

/*  Tries to restore a left saved tile position.  */
bool RasterizerEmulator::restoreTileLeft(SetupTriangle *triangle)
{
    /*  Check if there is a left saved tile position.  */
    if (triangle->isLeftTileSaved())
    {
        /*  Restore position stored in left tile save.  */
        triangle->restoreLeftTile();

        return TRUE;
    }

    return FALSE;
}

/*  Tries to restore a right saved tile position.  */
bool RasterizerEmulator::restoreTileRight(SetupTriangle *triangle)
{
    /*  Check if there is a right saved tile position.  */
    if (triangle->isRightTileSaved())
    {
        /*  Restore position stored in right tile save.  */
        triangle->restoreRightTile();

        return TRUE;
    }

    return FALSE;
}

/*  Tries to restore a up saved tile position.  */
bool RasterizerEmulator::restoreTileUp(SetupTriangle *triangle)
{
    /*  Check if there is a up saved tile position.  */
    if (triangle->isUpTileSaved())
    {
        /*  Restore position stored in up tile save.  */
        triangle->restoreUpTile();

        return TRUE;
    }

    return FALSE;
}

/*  Tries to restore a down saved tile position.  */
bool RasterizerEmulator::restoreTileDown(SetupTriangle *triangle)
{
    /*  Check if there is a down saved tile position.  */
    if (triangle->isDownTileSaved())
    {
        /*  Restore position stored in down tile save.  */
        triangle->restoreDownTile();

        return TRUE;
    }

    return FALSE;
}

/*  Tries to restore a right saved stamp position.  */
bool RasterizerEmulator::restoreRight(SetupTriangle *triangle)
{
    /*  Check if there is a right saved position.  */
    if (triangle->isRightSaved())
    {
        /*  Restore position stored in right save.  */
        triangle->restoreRight();

        /*  Change rasterization direction.  */
        triangle->setDirection(RIGHT_DIR);

        return TRUE;
    }

    return FALSE;
}

/*  Tries to restore a up saved stamp position.  */
bool RasterizerEmulator::restoreUp(SetupTriangle *triangle)
{
    /*  Check if there is a up saved position.  */
    if (triangle->isUpSaved())
    {
        /*  Restore position stored in up save.  */
        triangle->restoreUp();

        /*  Change drawing direction to up.  */
        triangle->setDirection(UP_DIR);

        return TRUE;
    }

    return FALSE;
}

/*  Tries to restore a down saved stamp position.  */
bool RasterizerEmulator::restoreDown(SetupTriangle *triangle)
{
    /*  Check if there is a down saved position.  */
    if (triangle->isDownSaved())
    {
        /*  Restore position stored in down save.  */
        triangle->restoreDown();

        /*  Change drawing direction to down.  */
        triangle->setDirection(DOWN_DIR);

        return TRUE;
    }

    return FALSE;
}

/*  Sets the position of the next scan tile position for over tile saves. Priority: left > right > up > down.  */
bool RasterizerEmulator::restoreTile(SetupTriangle *triangle)
{
    /*  Try to restore a left saved over tile position.  */
    if (!restoreTileLeft(triangle))
    {
        /*  Try to restore the right saved over tile position.  */
        if (!restoreTileRight(triangle))
        {
            /*  Try to restore the up saved over tile position.  */
            if (!restoreTileUp(triangle))
            {
                /*  Try to restore the down saved over tile position.  */
                return restoreTileDown(triangle);
            }
        }
    }

    return TRUE;
}

/*  Sets the position of the next scan tile to scan/rasterize (right > up > down > tile priority).  */
void RasterizerEmulator::nextTileRUD(SetupTriangle *triangle, Fragment *fr)
{
    /*  Try to restore right saved scan tile position.  */
    if (!restoreRight(triangle))
    {
        /*  Try to restore a up saved scan tile position.  */
        if (!restoreUp(triangle))
        {
            /*  Try to restore a down saved scan tile position.  */
            if (!restoreDown(triangle))
            {
                /*  Try to restore from an over tile save position.  */
                if (!restoreTile(triangle))
                {
                    /*  Stop scan/rasterization.  */

                    /*  No more fragments in the triangle.  */
                    triangle->lastFragment();

                    /*  Set as last fragment.  */
                    fr->lastFragment();
                }
            }
        }
    }
}

/*  Sets the position of the next scan tile to scan/rasterize (right > up > down > tile priority).  */
bool RasterizerEmulator::nextTileBorder(SetupTriangle *triangle, Fragment *fr)
{
    /*  Try to restore right saved scan tile position.  */
    if (!restoreRight(triangle))
    {
        /*  Try to restore a up saved scan tile position.  */
        if (!restoreUp(triangle))
        {
            /*  Try to restore a down saved scan tile position.  */
            if (!restoreDown(triangle))
            {
                /*  Try to restore from an over tile save position.  */
                if (!restoreTile(triangle))
                {
                    return false;
                }
            }
        }
    }

    triangle->setDirection(CENTER_DIR);
    return true;
}


/*  Sets the position of the next scan tile to scan/rasterize (up > down > tile priority).  */
void RasterizerEmulator::nextTileUD(SetupTriangle *triangle, Fragment *fr)
{
    /*  Try to restore a up saved scan tile position.  */
    if (!restoreUp(triangle))
    {
        /*  Try to restore a down saved scan tile position.  */
        if (!restoreDown(triangle))
        {
            /*  Try to restore from an over tile save position.  */
            if (!restoreTile(triangle))
            {
                /*  Stop scan/rasterization.  */

                /*  No more fragments in the triangle.  */
                triangle->lastFragment();

                /*  Set as last fragment.  */
                fr->lastFragment();
            }
        }
    }
}

/*  Sets the position of the next scan tile to scan/rasterize (right > down > tile priority).  */
void RasterizerEmulator::nextTileRD(SetupTriangle *triangle, Fragment *fr)
{
    /*  Try to restore right saved scan tile position.  */
    if (!restoreRight(triangle))
    {
        /*  Try to restore a down saved scan tile position.  */
        if (!restoreDown(triangle))
        {
            /*  Try to restore from an over tile save position.  */
            if (!restoreTile(triangle))
            {
                /*  Stop scan/rasterization.  */

                /*  No more fragments in the triangle.  */
                triangle->lastFragment();

                /*  Set as last fragment.  */
                fr->lastFragment();
            }
        }
    }
}

/*  Sets the position of the next scan tile to scan/rasterize (down > tile priority).  */
void RasterizerEmulator::nextTileD(SetupTriangle *triangle, Fragment *fr)
{
    /*  Try to restore a down saved scan tile position.  */
    if (!restoreDown(triangle))
    {
        /*  Try to restore from an over tile position.  */
        if (!restoreTile(triangle))
        {
            /*  Stop rasterization.  */

            /*  No more fragments in the triangle.  */
            triangle->lastFragment();

            /*  Set as last fragment.  */
            fr->lastFragment();
        }
    }
}


/*  Scan and saves the scan tile position over the current.  */
void RasterizerEmulator::saveUp(f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit upperBound,
    SetupTriangle *triangle, bool extended)
{
    f64bit upSave[4];

    /*  Initialize up save equation values.  */
    upSave[0] = edge1[2];
    upSave[1] = edge2[2];
    upSave[2] = edge3[2];
    upSave[3] = zeq[2];

    /*  Check if is the start of a new over tile.  */
    if (GPU_MOD(y/scanTileHeight + 1, scanOverTileHeight) == 0)
    {

        /*  Check if scan tile position in the over tile over the current must be saved.  */
        if (triangle->getTileDirection(UP_DIR) && (!triangle->isUpTileSaved()))
        {
            /*  Test scan tile over the current.  */
            if (scanTileUp(upSave, edge1, edge2, edge3, zeq, offset, y, upperBound, extended))
            {
                /*  Save up over tile.  */
                triangle->save(upSave, x, y + scanTileHeight, TILE_UP_SAVE);
            }
        }
    }
    else
    {
        /*  Check if the up save isn't already saved.  */
        if (!triangle->isUpSaved())
        {
            /*  First test up and save.  */
            if (scanTileUp(upSave, edge1, edge2, edge3, zeq, offset, y, upperBound, extended))
            {
                /*  Save down position.  */
                triangle->save(upSave, x, y + scanTileHeight, UP_SAVE);
            }
        }
    }
}

/*  Scan and saves the scan tile position over the current.  */
void RasterizerEmulator::saveUpBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit upperBound,
    SetupTriangle *triangle, bool extended)
{
    f64bit upSave[4];

    /*  Initialize up save equation values.  */
    upSave[0] = edge1[2];
    upSave[1] = edge2[2];
    upSave[2] = edge3[2];
    upSave[3] = zeq[2];

    /*  Check if the up save isn't already saved.  */
    if (!triangle->isUpSaved())
    {
        /*  First test up and save.  */
        if (scanTileUp(upSave, edge1, edge2, edge3, zeq, offset, y, upperBound, extended))
        {
            /*  Save down position.  */
            triangle->save(upSave, x, y + scanTileHeight, UP_SAVE);
        }
    }
}

/*  Scans and saves the scan tile position below the current.  */
void RasterizerEmulator::saveDown(f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit bottomBound,
    SetupTriangle *triangle, bool extended)
{
    f64bit downSave[4];

    /*  Initialize down save equation values.  */
    downSave[0] = edge1[2];
    downSave[1] = edge2[2];
    downSave[2] = edge3[2];
    downSave[3] = zeq[2];

    /*  Check if is the start of a new over tile.  */
    if (GPU_MOD(y/scanTileHeight - 1, scanOverTileHeight) == (scanOverTileHeight - 1))
    {

        /*  Check if scan tile position below the current over tile must be saved.  */
        if (triangle->getTileDirection(DOWN_DIR) && (!triangle->isDownTileSaved()))
        {
            /*  Test scan tile below the current.  */
            if (scanTileDown(downSave, edge1, edge2, edge3, zeq, offset, y, bottomBound, extended))
            {
                /*  Save tile below.  */
                triangle->save(downSave, x, y - scanTileHeight, TILE_DOWN_SAVE);
            }
        }
    }
    else
    {
        /*  Check if down save is already saved.  */
        if (!triangle->isDownSaved())
        {
            /*  Second test down and save.  */
            if (scanTileDown(downSave, edge1, edge2, edge3, zeq, offset, y, bottomBound, extended))
            {
                /*  Save down position.  */
                triangle->save(downSave, x, y - scanTileHeight, DOWN_SAVE);
            }
        }
    }
}

/*  Scans and saves the scan tile position below the current.  */
void RasterizerEmulator::saveDownBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit bottomBound,
    SetupTriangle *triangle, bool extended)
{
    f64bit downSave[4];

    /*  Initialize down save equation values.  */
    downSave[0] = edge1[2];
    downSave[1] = edge2[2];
    downSave[2] = edge3[2];
    downSave[3] = zeq[2];

    /*  Check if down save is already saved.  */
    if (!triangle->isDownSaved())
    {
        /*  Second test down and save.  */
        if (scanTileDown(downSave, edge1, edge2, edge3, zeq, offset, y, bottomBound, extended))
        {
            /*  Save down position.  */
            triangle->save(downSave, x, y - scanTileHeight, DOWN_SAVE);
        }
    }
}

/*  Scans and saves the scan tile position right from the current.  */
void RasterizerEmulator::saveRight(f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit rightBound,
    SetupTriangle *triangle, bool extended)
{
    f64bit rightSave[4];

    /*  Initialize right save equation values.  */
    rightSave[0] = edge1[2];
    rightSave[1] = edge2[2];
    rightSave[2] = edge3[2];
    rightSave[3] = zeq[2];

    /*  Check if is the start of a new over tile.  */
    if (GPU_MOD(x/scanTileWidth + 1, scanOverTileWidth) == 0)
    {
        /*  Check if right over tile position must be saved.  */
        if(triangle->getTileDirection(RIGHT_DIR) && (!triangle->isRightTileSaved()))
        {
            /*  Test scan tile right of the current.  */
            if (scanTileRight(rightSave, edge1, edge2, edge3, zeq, offset, x, rightBound, extended))
            {
                /*  Save right tile.  */
                triangle->save(rightSave, x + scanTileWidth, y, TILE_RIGHT_SAVE);
            }
        }
    }
    else
    {
        /*  Check if right is already saved.  */
        if(!triangle->isRightSaved())
        {
            /*  Third test right and save.*/
            if (scanTileRight(rightSave, edge1, edge2, edge3, zeq, offset, x, rightBound, extended))
            {
                /*  Save right position.  */
                triangle->save(rightSave, x + scanTileWidth, y, RIGHT_SAVE);
            }
        }
    }
}

/*  Scans and saves the scan tile position right from the current.  */
void RasterizerEmulator::saveRightBorder(f64bit *edge1, f64bit *edge2, f64bit *edge3,
    f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit rightBound,
    SetupTriangle *triangle, bool extended)
{
    f64bit rightSave[4];

    /*  Initialize right save equation values.  */
    rightSave[0] = edge1[2];
    rightSave[1] = edge2[2];
    rightSave[2] = edge3[2];
    rightSave[3] = zeq[2];

    /*  Check if right is already saved.  */
    if(!triangle->isRightSaved())
    {
        /*  Third test right and save.*/
        if (scanTileRight(rightSave, edge1, edge2, edge3, zeq, offset, x, rightBound, extended))
        {
            /*  Save right position.  */
            triangle->save(rightSave, x + scanTileWidth, y, RIGHT_SAVE);
        }
    }
}

/*  Scans, rasterizes and saves left scan tile.  */
bool RasterizerEmulator::rasterizeLeft(f64bit *edge1, f64bit *edge2,
    f64bit *edge3, f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit leftBound,
    SetupTriangle *triangle, bool extended)
{
    f64bit leftPos[4];

    /*  Initialize left position equation values.  */
    leftPos[0] = edge1[2];
    leftPos[1] = edge2[2];
    leftPos[2] = edge3[2];
    leftPos[3] = zeq[2];

    /*  Check if is the start of a new over tile.  */
    if (GPU_MOD(x/scanTileWidth - 1, scanOverTileWidth) == (scanOverTileWidth - 1))
    {
        /*  Check if scan tile position left to the current over tile must be saved. */
        if (triangle->getTileDirection(LEFT_DIR) && (!triangle->isLeftTileSaved()))
        {
            /*  Test scan tile left of the current.  */
            if (scanTileLeft(leftPos, edge1, edge2, edge3, zeq, offset, x, leftBound, extended))
            {
                /*  Save left over tile.  */
                triangle->save(leftPos, x - scanTileWidth, y, TILE_LEFT_SAVE);
            }
        }

        /*  No continue rasterizing to the left.  */
        return FALSE;
    }
    else
    {
        /*  Fourth, test left.  */
        if (scanTileLeft(leftPos, edge1, edge2, edge3, zeq, offset, x, leftBound, extended))
        {
            /*  Store raster position state for next fragment.  */
            triangle->updatePosition(leftPos, x - scanTileWidth, y);

            /*  Change rasterization direction to the left.  */
            triangle->setDirection(LEFT_DIR);

            /*  Continue rasterization to the left.  */
            return TRUE;
        }
        else
        {
            /*  No continue to the left.  */
            return FALSE;
        }
    }
}

/*  Scans, rasterizes and saves right scan tile.  */
bool RasterizerEmulator::rasterizeRight(f64bit *edge1, f64bit *edge2,
    f64bit *edge3, f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit rightBound,
    SetupTriangle *triangle, bool extended)
{
    f64bit rightPos[4];

    /*  Initialize right position equation values.  */
    rightPos[0] = edge1[2];
    rightPos[1] = edge2[2];
    rightPos[2] = edge3[2];
    rightPos[3] = zeq[2];

    /*  Check if is the start of a new over tile.  */
    if (GPU_MOD(x/scanTileWidth + 1, scanOverTileWidth) == 0)
    {
        /*  Check if scan tile position right to the current over tile must be saved.  */
        if (triangle->getTileDirection(RIGHT_DIR) && (!triangle->isRightTileSaved()))
        {
            /*  Test scan tile right of the current.  */
            if (scanTileRight(rightPos, edge1, edge2, edge3, zeq, offset, x, rightBound, extended))
            {
                /*  Save right over tile.  */
                triangle->save(rightPos, x + scanTileWidth, y, TILE_RIGHT_SAVE);
            }
        }

        /*  No continue rasterizing to the right.  */
        return FALSE;
    }
    else
    {
        /*  Second, test right.  */
        if (scanTileRight(rightPos, edge1, edge2, edge3, zeq, offset, x, rightBound, extended))
        {
            /*  Store raster position state for next fragment.  */
            triangle->updatePosition(rightPos, x + scanTileWidth, y);

            /*  Change rasterization direction to the right.  */
            triangle->setDirection(RIGHT_DIR);

            /*  Continue to the right.  */
            return TRUE;
        }
        else
        {
            /*  Stop rasterizing to the right.  */
            return FALSE;
        }
    }
}

/*  Searches for triangle fragments along the top border of the bounding box.  */
void RasterizerEmulator::rasterizeTopBorder(f64bit *edge1, f64bit *edge2,
    f64bit *edge3, f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit leftBound,
    SetupTriangle *triangle)
{
    f64bit leftPos[4];

    /*  Initialize left position equation values.  */
    leftPos[0] = edge1[2];
    leftPos[1] = edge2[2];
    leftPos[2] = edge3[2];
    leftPos[3] = zeq[2];

    /*  Scan to the next tile to the left.  */
    if (scanTileLeft(leftPos, edge1, edge2, edge3, zeq, offset, x, leftBound, false))
    {
        /*  Store raster position state for next fragment.  */
        triangle->updatePosition(leftPos, x - scanTileWidth, y);

        /*  Change rasterization direction to the left.  */
        triangle->setDirection(CENTER_DIR);
    }
    else
    {
        /*  Check if left bounding box border reached.  */
        if ((x - s32bit(scanTileWidth)) < (leftBound - s32bit(GPU_MOD(leftBound, scanTileWidth))))
        {
            /*  Change to left border direction.  */
            triangle->setDirection(LEFT_BORDER_DIR);
        }
        else
        {
            /*  Store raster position state for next fragment.  */
            triangle->updatePosition(leftPos, x - scanTileWidth, y);
        }
    }
}

/*  Searches for triangle fragments along the left border of the bounding box.  */
void RasterizerEmulator::rasterizeLeftBorder(f64bit *edge1, f64bit *edge2,
    f64bit *edge3, f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit bottomBound,
    SetupTriangle *triangle)
{
    f64bit downPos[4];

    /*  Initialize down position equation values.  */
    downPos[0] = edge1[2];
    downPos[1] = edge2[2];
    downPos[2] = edge3[2];
    downPos[3] = zeq[2];

    /*  Scan to the next tile down.  */
    if (scanTileDown(downPos, edge1, edge2, edge3, zeq, offset, y, bottomBound, false))
    {
        /*  Store raster position state for next fragment.  */
        triangle->updatePosition(downPos, x, y - scanTileHeight);

        /*  Change rasterization direction to down.  */
        triangle->setDirection(CENTER_DIR);
    }
    else
    {
        /*  Check if bottom bounding box border reached.  */
        if ((y - s32bit(scanTileHeight)) < (bottomBound - s32bit(GPU_MOD(bottomBound, scanTileHeight))))
        {
            /*  Change to bottom border direction.  */
            triangle->setDirection(BOTTOM_BORDER_DIR);
        }
        else
        {
            /*  Store raster position state for next fragment.  */
            triangle->updatePosition(downPos, x, y - scanTileHeight);
        }
    }
}

/*  Searches for triangle fragments along the bottom border of the bounding box.  */
bool RasterizerEmulator::rasterizeBottomBorder(f64bit *edge1, f64bit *edge2,
    f64bit *edge3, f64bit *zeq, f64bit *offset, s32bit x, s32bit y, s32bit rightBound,
    SetupTriangle *triangle)
{
    f64bit rightPos[4];

    /*  Initialize right position equation values.  */
    rightPos[0] = edge1[2];
    rightPos[1] = edge2[2];
    rightPos[2] = edge3[2];
    rightPos[3] = zeq[2];

    /*  Scan to the next tile to the right.  */
    if (scanTileRight(rightPos, edge1, edge2, edge3, zeq, offset, x, rightBound, false))
    {
        /*  Store raster position state for next fragment.  */
        triangle->updatePosition(rightPos, x + scanTileWidth, y);

        /*  Change rasterization direction to right.  */
        triangle->setDirection(CENTER_DIR);
    }
    else
    {
        /*  Check if right bounding box border reached.  */
        if ((x + s32bit(scanTileWidth)) > rightBound)
        {
            return false;
        }
        else
        {
            /*  Store raster position state for next fragment.  */
            triangle->updatePosition(rightPos, x + scanTileWidth, y);
        }
    }

    return true;
}


/*  Scans the current triangle and updates the current generation position.  */
void RasterizerEmulator::scanTiled(u32bit triangleID)
{
    SetupTriangle *triangle;
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    s32bit x, y;
    f64bit offset[3];
    Fragment *fr;
    bool firstStamp;
    s32bit leftBound, rightBound, bottomBound, upperBound;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "scanTiled", "Triangle identifier out of range.");
    )

    /*  Get setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "scanTiled", "Triangle is not a setup triangle.");
    )

    /*  Get triangle current edge equations.  */
    triangle->getEdgeEquations(edge1, edge2, edge3);

    /*  Get triangle current z interpolation equation.  */
    triangle->getZEquation(zeq);

    /*  Get the triangle bounding box.  */
    triangle->getBoundingBox(leftBound, bottomBound, rightBound, upperBound);

    /*  Get triangle current raster position.  */
    triangle->getRasterPosition(x, y);

    /*  Get if it's the first stamp of the triangle to probe.  */
    firstStamp = triangle->isFirstStamp();

    if ((triangle->getDirection() & BORDER_DIR) == 0)
    {
        /*  Set as next stamp as not first.  */
        triangle->setFirstStamp(FALSE);
    }

    /*  Calculate pixel offset (aprox) for the edge equations.  This is used
        for scaning triangles that are thiner than a pixel.  */
    //offset[0] = GPU_MAX(fabs(edge1[0]), fabs(edge1[1]));
    //offset[1] = GPU_MAX(fabs(edge2[0]), fabs(edge2[1]));
    //offset[2] = GPU_MAX(fabs(edge3[0]), fabs(edge3[1]));
    offset[0] = 0.0f;
    offset[1] = 0.0f;
    offset[2] = 0.0f;
    offset[3] = 0.0f;

    /*  Dummy fragment because nextTile functions mark the last fragment.  */
    fr = new Fragment(triangle, 0, 0, 0, offset, testInsideTriangle(triangle, offset));

    /*  Get rasterization direction.  */
    switch(triangle->getDirection())
    {
        case CENTER_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, upperBound, triangle, firstStamp);

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle, firstStamp);

            /*  Save the stamp position right from the current.  */
            saveRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle, firstStamp);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle, firstStamp))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRUD(triangle, fr);
            }

            break;

        case UP_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, upperBound, triangle, firstStamp);

            /*  Save the stamp position right from the current.  */
            saveRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle, firstStamp);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle, firstStamp))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRUD(triangle, fr);
            }

            break;

        case DOWN_DIR:

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle, firstStamp);

            /*  Save the stamp position right from the current.  */
            saveRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle, firstStamp);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRD(triangle, fr);
            }

            break;

        case DOWN_LEFT_DIR:

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRD(triangle, fr);
            }

            break;

        case DOWN_RIGHT_DIR:

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);

            /*  Moves or saves right stamp position.  Returns if continue
                rasterizing to the right.  */
            if (!rasterizeRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileD(triangle, fr);
            }

            break;

        case UP_LEFT_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, upperBound, triangle);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRUD(triangle, fr);
            }

            break;

        case UP_RIGHT_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, upperBound, triangle);

            /*  Moves or saves right stamp position.  Returns if continue
                rasterizing to the right.  */
            if (!rasterizeRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileUD(triangle, fr);
            }

            break;

        case CENTER_LEFT_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, upperBound, triangle);

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRUD(triangle, fr);
            }

            break;

        case CENTER_RIGHT_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, upperBound, triangle);

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);

            /*  Moves or saves right stamp position.  Returns if continue
                rasterizing to the right.  */
            if (!rasterizeRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileUD(triangle, fr);
            }

            break;

        case TOP_BORDER_DIR:

            /*  Save the stamp position below the current.  */
            saveDownBorder(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle, firstStamp);

            /*  Check for any saved position.  */
            if (!nextTileBorder(triangle, fr))
            {
                /*  Continue border search.  */
                rasterizeTopBorder(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle);
            }

            break;

        case LEFT_BORDER_DIR:

            /*  Save the stamp position right to the current.  */
            saveRightBorder(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle, firstStamp);

            /*  Check for any saved position.  */
            if (!nextTileBorder(triangle, fr))
            {
                /*  Continue diagonal search.  */
                rasterizeLeftBorder(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);
            }

            break;

        case BOTTOM_BORDER_DIR:

            /*  Save the stamp position over the current.  */
            saveUpBorder(edge1, edge2, edge3, zeq, offset, x, y, upperBound, triangle, firstStamp);

            /*  Check for any saved position.  */
            if (!nextTileBorder(triangle, fr))
            {
                /*  Continue border search.  */
                if (!rasterizeBottomBorder(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle))
                {
                    /*  Stop scan/rasterization.  */

                    /*  No more fragments in the triangle.  */
                    triangle->lastFragment();

                    /*  Set as last fragment.  */
                    fr->lastFragment();
                }
            }

            break;

        default:
            panic("RasterizerEmulator", "scanTiled", "Invalid scan direction.");
            break;
    }

    delete fr;
}

/*  Generates the fragments in the generation tile for the triangle current generation position.  */
//Fragment **RasterizerEmulator::generateFragments(u32bit triangleID)
void RasterizerEmulator::generateFragments(u32bit triangleID)
{
    //Fragment **stamp;
    Tile **tilesOut;
    u32bit outTiles;
    u32bit i;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "generateFragments", "Triangle identifier out of range.");
    )

    /*  Allocate the stamp.  */
    //stamp = new (Fragment *)[genTileWidth * genTileHeight];

    /*  Check allocation.  */
    //GPU_ASSERT(
    //    if (stamp == NULL)
    //        panic("RasterizerEmulator", "generateTileFragments", "Error allocating the stamp.");
    //)

    /*  Expand the input tile into stamps.  */
    outTiles = expandTile(trGenTiles[triangleID][scanTileGenTiles - trStoredGenTiles[triangleID]], tilesOut, 1);

    /*  Generate fragments from all the stamp level tiles.  */
    for(i = 0; i < outTiles; i++)
    {
        /*  Generate the fragments for the current stamp.  */
        //generateStamp(tilesOut[i], &stamp[i << 2]);
        generateStamp(tilesOut[i], &trFragments[triangleID][i << 2]);

        /*  Delete the current tile.  */
        delete tilesOut[i];
    }

    /*  Delete tile arrays.  */
    delete[] tilesOut;

    //return stamp;
}

/*  Generates the fragments in the generation tile for the triangle current generation position.  */
//bool RasterizerEmulator::generateFragmentsMulti(u32bit triangleID, Fragment **&stamp)
bool RasterizerEmulator::generateFragmentsMulti(u32bit triangleID)
{
    Tile **tilesOut;
    u32bit outTiles;
    u32bit i;
    //u32bit *fragmentTrID;
    u32bit genTriID;
    bool moreFragments;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "generateFragments", "Triangle identifier out of range.");
    )

    /*  Allocate the stamp.  */
    //stamp = new (Fragment *)[genTileWidth * genTileHeight];

    /*  Check allocation.  */
    //GPU_ASSERT(
    //    if (stamp == NULL)
    //        panic("RasterizerEmulator", "generateTileFragments", "Error allocating the stamp.");
    //)

    /*  Allocate the triangle identifier list.  */
    //fragmentTrID = new (u32bit)[genTileWidth * genTileHeight];

    /*  Check allocation.  */
    //GPU_ASSERT(
    //    if (fragmentTrID == NULL)
    //        panic("RasterizerEmulator", "generateTileFragments", "Error allocating the triangle identifier array.");
    //)

    /*  Check if the stamps were already generated.  */
    if(trStoredStamps[triangleID] > 0)
    {
        /*  Get the already generated stamps.  */
        tilesOut = trStamps[triangleID];
        outTiles = trStoredStamps[triangleID];
    }
    else
    {
//printf("Generating fragments from gen tile at %d | scanTileGenTiles %d\n", scanTileGenTiles - trStoredGenTiles[triangleID], scanTileGenTiles);
        /*  Expand the input tile into stamps.  */
        outTiles = expandTile(trGenTiles[triangleID][scanTileGenTiles - trStoredGenTiles[triangleID]], tilesOut, 1);
    }

    /*  Generate fragments from all the stamp level tiles.  */
    for(i = 0, moreFragments = FALSE; i < outTiles; i++)
    {
        /*  Generate the fragments for the current stamp.  */
        //moreFragments = moreFragments | generateStampMulti(tilesOut[i], &stamp[i << 2], genTriID);
        moreFragments = moreFragments | generateStampMulti(tilesOut[i], &trFragments[triangleID][i << 2], genTriID);

        /*  Set fragment triangle identifier.  */
        //fragmentTrID[i << 2] = genTriID;
        //fragmentTrID[(i << 2) + 1] = genTriID;
        //fragmentTrID[(i << 2) + 2] = genTriID;
        //fragmentTrID[(i << 2) + 3] = genTriID;
        frTriangleID[triangleID][i << 2] = genTriID;
        frTriangleID[triangleID][(i << 2) + 1] = genTriID;
        frTriangleID[triangleID][(i << 2) + 2] = genTriID;
        frTriangleID[triangleID][(i << 2) + 3] = genTriID;
    }

    /*  Check if the stamps in this generation tile will generate more stamps.  */
    if (moreFragments)
    {
        /*  Keep the generated stamps.  */
        trStamps[triangleID] = tilesOut;
        trStoredStamps[triangleID] = outTiles;
    }
    else
    {
        /*  Reset stamp storage.  */
        trStamps[triangleID] = NULL;
        trStoredStamps[triangleID] = 0;

        /*  Delete all the quad level tiles.  */
        for(i = 0; i < outTiles; i++)
            delete tilesOut[i];

        /*  Delete tile arrays.  */
        delete[] tilesOut;
    }

    /*  Set triangle identifier array.  */
    //frTriangleID[triangleID] = fragmentTrID;

    return moreFragments;
}



/*  New version of nextScanlineStamp.  */
Fragment **RasterizerEmulator::nextScanlineStampTiled(u32bit triangleID)
{
    Fragment **stamp;
    SetupTriangle *triangle;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "nextScanlineStampTiled", "Triangle identifier out of range.");
    )

    /*  Get setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "nextScanlineStampTiled", "Triangle is not a setup triangle.");
    )

    /*  Check if there are no generation tiles.  */
    if ((trStoredGenTiles[triangleID] == 0) && (!triangle->isLastFragment()))
    {
        /*  Delete the old list.  */
        delete[] trGenTiles[triangleID];

        /*  Expand current scan tile into generation tiles.  */
        trGenTiles[triangleID] = expandScanTile(triangleID, trStoredGenTiles[triangleID]);

        /*  Continue scanning the triangle.  */
        scanTiled(triangleID);
    }

    /*  Check if there are no stamps stored.  */
    if ((trStoredFragments[triangleID] == 0) && (trStoredGenTiles[triangleID] > 0))
    {
        /*  Delete the old list.  */
        //delete[] trFragments[triangleID];

        /*  Generate the fragments in the current generation tile.  */
        //trFragments[triangleID] = generateFragments(triangleID);
        generateFragments(triangleID);

        /*  Update number of stored generation tiles.  */
        trStoredGenTiles[triangleID]--;

        /*  Update number of stored fragments.  */
        trStoredFragments[triangleID] = genTileFragments;
    }

    /*  Check if there are stamps stored.  */
    if (trStoredFragments[triangleID] > 0)
    {
        /*  Allocate the stamp.  */
        stamp = new Fragment*[4];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (stamp == NULL)
                panic("RasterizerEmulator", "nextScanlineStampTiled", "Error allocating stamp.");
        )

        /*  Get next stored stamp.  */
        stamp[0] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID]];
        stamp[1] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 1];
        stamp[2] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 2];
        stamp[3] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 3];

        /*  Update number of stored fragments.  */
        trStoredFragments[triangleID] -= 4;

        /*  Check if last fragment flag and no more stored fragments.  */
        if (triangle->isLastFragment() && (trStoredGenTiles[triangleID] == 0) && (trStoredFragments[triangleID] == 0))
        {
            /*  Set stamp last fragment flag.  */
            stamp[3]->lastFragment();
        }
    }
    else
    {
        /*  No stamps generated.  */
        stamp = NULL;
    }

    return stamp;
}


/*  Initializes the recursive rasterization engine.  */
void RasterizerEmulator::startRecursive(u32bit triangleID, bool msaaEnabled)
{
    u32bit i;
    Tile *tile;

    /*  Set counters and pointers.  */
    for(i = 0; i < MAX_LEVELS; i++)
    {
        nextTile[i] = 0;
        numTiles[i] = 0;
    }

    /*  Create triangle top tile.  */
    tile = topLevelTile(triangleID, msaaEnabled);

    /*  Get current recursive level.  */
    level = tile->getLevel();

    /*  Update number of tiles to evaluate in current level.  */
    numTiles[level] = 1;

    /*  Update next tile to evaluate in the current level.  */
    nextTile[level] = 0;

    /*  Store top level tile.  */
    testTiles[level][0] = tile;
}

/*  Initializes the recursive rasterization engine for a batch of triangles.  */
u32bit RasterizerEmulator::startRecursiveMulti(u32bit *triangleID, u32bit numTris, bool msaaEnabled)
{
    u32bit i;
    Tile *tile;

    /*  Check the size of the triangle batch.  */
    GPU_ASSERT(
        if(numTris == 0)
            panic("RasterizerEmulator", "startRecursiveMulti", "The triangle batch requires at least a triangle.");
        if(numTris > MAX_TRIANGLES)
            panic("RasterizerEmulator", "startRecursiveMulti", "Triangle batch is too large.");
    )

    /*  Store the triangle batch to rasterize.  */
    for(i = 0; i < numTris; i++)
        triangleBatch[i] = triangleID[i];

    /*  Set number of triangles in the batch.  */
    batchSize = numTris;

    /*  Set counters and pointers.  */
    for(i = 0; i < MAX_LEVELS; i++)
    {
        nextTile[i] = 0;
        numTiles[i] = 0;
    }

    /*  Create triangle top tile.  */
    tile = topLevelTile(triangleBatch, batchSize, msaaEnabled);

    /*  Get current recursive level.  */
    level = tile->getLevel();

    /*  Update number of tiles to evaluate in current level.  */
    numTiles[level] = 1;

    /*  Update next tile to evaluate in the current level.  */
    nextTile[level] = 0;

//printf("RastEmu => topLevel = %d\n", level);

    /*  Check if the top level tile for the triangle batch is a scan level tile.  */
    if (level == scanLevel)
    {
        /*  Store in the scan tile queue.  */
        scanTiles[0] = tile;

        /*  Stop recursive search.  */
        level = MAX_LEVELS;
    }
    else
    {
        /*  Store top level tile.  */
        testTiles[level][0] = tile;
    }

//printf("startRecursive => startTile %d\n", level);

    return 0xFAFA;
}

/*  Updates the recursive search of fragments.  */
void RasterizerEmulator::updateRecursive(u32bit triangleID)
{
    SetupTriangle *triangle;
    u32bit i;
    bool isLast;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "updateRecursive", "Triangle identifier out of range.");
    )

    /*  Get setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "updateRecursive", "Triangle is not a setup triangle.");
    )

    /*  Check current recursive level.  */
    if (level == scanLevel)
    {
//printf("Level %d scan tile\n", level);
        /*  Check if there are no generation tiles.  */
        if (trStoredGenTiles[triangleID] == 0)
        {
            /*  Delete the old list.  */
            delete[] trGenTiles[triangleID];

            /*  Expand current scan tile into generation tiles.  */
            trStoredGenTiles[triangleID] = expandTile(testTiles[level][nextTile[level]], trGenTiles[triangleID], genLevel);

            /*  Update next scan tile to expand.  */
            nextTile[level]++;

            /*  Check if all scan tiles were expanded.  */
            if (nextTile[level] == numTiles[level])
            {
                /*  Level up.  */
                level++;
            }
        }
    }
    else
    {
        /*  Check if last fragment flag.  */
        if (!triangle->isLastFragment())
        {
//printf("Level %d\n", level);
            /*  Check if all tiles for the current level were evaluated.  */
            if (nextTile[level] == numTiles[level])
            {
                /*  Level up.  */
                level++;
            }
            else
            {
                /*  Evaluate N tiles from the same level.  */
                for(i = 0, numTiles[level - 1] = 0; (i < TILE_TESTERS) && (numTiles[level] > nextTile[level]); i++)
                {
                    /*  Evaluate current tile in the current recursive level.  */
                    numTiles[level - 1] += evaluateTile(testTiles[level][nextTile[level]],
                        &testTiles[level - 1][numTiles[level - 1]]);

                    /*  Delete evaluated tile.  */
                    delete testTiles[level][nextTile[level]];

                    /*  Update next tile to expand in the current level.  */
                    nextTile[level]++;
                }

                /*  Check if any subtile was generated.  */
                if (numTiles[level - 1] != 0)
                {
                    /*  Level down.  */
                    level--;

                    /*  Reset next tile pointer.  */
                    nextTile[level] = 0;
                }
                else
                {
                    /*  Check if all tiles for the current level were evaluated.  */
                    if (nextTile[level] == numTiles[level])
                    {
                        /*  Level up.  */
                        level++;
                    }
                }
            }

            /*  Set last fragment flag.  */
            for(i = level, isLast = TRUE; (i < MAX_LEVELS) && isLast; i++)
            {
                /*  Check if there tiles to evalute in the upper levels.  */
                if (nextTile[i] != numTiles[i])
                    isLast = FALSE;
            }

            if (isLast)
                triangle->lastFragment();
        }
    }
}

/*  Updates the recursive search of fragments.  */
void RasterizerEmulator::updateRecursiveMulti(u32bit batchID)
{
    SetupTriangle *triangle;
    u32bit triangleID;
    u32bit i;
    bool isLast;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (batchID != 0xFAFA)
            panic("RasterizerEmulator", "updateRecursiveMulti", "Batch identifier out of range.");
    )

    /*  Get identifier of the first triangle in the batch.  */
    triangleID = triangleBatch[0];

    /*  Get first setup triangle in the batch.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "updateRecursiveMulti", "Triangle is not a setup triangle.");
    )

    /*  Check current recursive level.  */
    if (level == scanLevel)
    {
//printf("Level %d scan tile\n", level);
        /*  Check if there are no generation tiles.  */
        if (trStoredGenTiles[triangleID] == 0)
        {
            /*  Delete the old list.  */
            delete[] trGenTiles[triangleID];

            /*  Expand current scan tile into generation tiles.  */
            trStoredGenTiles[triangleID] = expandTile(testTiles[level][nextTile[level]], trGenTiles[triangleID], genLevel);

            /*  Update next scan tile to expand.  */
            nextTile[level]++;


            /*  Check if all scan tiles were expanded.  */
            if (nextTile[level] == numTiles[level])
            {
                /*  Level up.  */
                level++;
            }
        }
    }
    else
    {
        /*  Check if last fragment flag.  */
        if (!triangle->isLastFragment())
        {
//printf("Level %d\n", level);
            /*  Check if all tiles for the current level were evaluated.  */
            if (nextTile[level] == numTiles[level])
            {
                /*  Level up.  */
                level++;
            }
            else
            {
                /*  Evaluate N tiles from the same level.  */
                for(i = 0, numTiles[level - 1] = 0; (i < TILE_TESTERS) && (numTiles[level] > nextTile[level]); i++)
                {
                    /*  Evaluate current tile in the current recursive level.  */
                    numTiles[level - 1] += evaluateTile(testTiles[level][nextTile[level]],
                        &testTiles[level - 1][numTiles[level - 1]]);

                    /*  Delete evaluated tile.  */
                    delete testTiles[level][nextTile[level]];

                    /*  Update next tile to expand in the current level.  */
                    nextTile[level]++;
                }

                /*  Check if any subtile was generated.  */
                if (numTiles[level - 1] != 0)
                {
                    /*  Level down.  */
                    level--;

                    /*  Reset next tile pointer.  */
                    nextTile[level] = 0;
                }
                else
                {
                    /*  Check if all tiles for the current level were evaluated.  */
                    if (nextTile[level] == numTiles[level])
                    {
                        /*  Level up.  */
                        level++;
                    }
                }
            }

            /*  Set last fragment flag.  */
            for(i = level, isLast = TRUE; (i < MAX_LEVELS) && isLast; i++)
            {
                /*  Check if there tiles to evalute in the upper levels.  */
                if (nextTile[i] != numTiles[i])
                    isLast = FALSE;
            }

            if (isLast)
                triangle->lastFragment();
        }
    }
}

void RasterizerEmulator::updateRecursiveMultiv2(u32bit batchID)
{
    SetupTriangle *triangle;
    u32bit triangleID;
    u32bit i;
    Tile *aux[4];
    u32bit subTiles;
    u32bit tileBufferSize;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (batchID != 0xFAFA)
            panic("RasterizerEmulator", "updateRecursiveMulti", "Batch identifier out of range.");
    )

    /*  Get identifier of the first triangle in the batch.  */
    triangleID = triangleBatch[0];

    /*  Get first setup triangle in the batch.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "updateRecursiveMulti", "Triangle is not a setup triangle.");
    )

//printf("in scanlevel %d numTiles %d nextTile %d storedGenTiles %d\n",
//    scanLevel, numTiles[scanLevel], nextTile[scanLevel], trStoredGenTiles[triangleID]);

    /*  Check if there are no generation tiles.  */
    if ((numTiles[scanLevel] > 0) && (trStoredGenTiles[triangleID] == 0))
    {
        /*  Delete the old list.  */
        delete[] trGenTiles[triangleID];

        /*  Expand current scan tile into generation tiles.  */
        trStoredGenTiles[triangleID] = expandTile(scanTiles[nextTile[scanLevel]], trGenTiles[triangleID], genLevel);

        /*  Update next scan tile to expand.  */
        nextTile[scanLevel] = GPU_MOD(nextTile[scanLevel] + 1, 4 * 4 * TILE_TESTERS);

        /*  Update number of tiles stored.  */
        numTiles[scanLevel]--;
//printf("out scanlevel %d numTiles %d nextTile %d storedGenTiles %d\n",
//    scanLevel, numTiles[scanLevel], nextTile[scanLevel], trStoredGenTiles[triangleID]);
    }


    /*  Check if last fragment flag.  */
    if (level < MAX_LEVELS)
    {
//printf("in level %d numTiles %d nextTile %d\n", level, numTiles[level], nextTile[level]);

        /*  Determine the size of the tile buffer for the subtiles.  */
        if ((level - 1) == scanLevel)
        {
            tileBufferSize = 4 * (4 * TILE_TESTERS - 1);
        }
        else
        {
            tileBufferSize = 4 * (TILE_TESTERS - 1);
        }

        /*  Evaluate N tiles from the same level.  */
        for(i = 0; (i < TILE_TESTERS) && (numTiles[level] > 0) && (numTiles[level - 1] <= tileBufferSize); i++)
        {
//printf("in sublevel %d numTiles %d nextTile %d\n", level - 1, numTiles[level - 1], nextTile[level - 1]);

            /*  Evaluate current tile in the current recursive level.  */
            subTiles = evaluateTile(testTiles[level][nextTile[level]], aux);

            /*  Check subtile level storage.  */
            if ((level - 1) == scanLevel)
            {
                scanTiles[GPU_MOD(nextTile[level - 1] + numTiles[level - 1], 4 * 4 * TILE_TESTERS)] = aux[0];
                scanTiles[GPU_MOD(nextTile[level - 1] + numTiles[level - 1] + 1, 4 * 4 * TILE_TESTERS)] = aux[1];
                scanTiles[GPU_MOD(nextTile[level - 1] + numTiles[level - 1] + 2, 4 * 4 * TILE_TESTERS)] = aux[2];
                scanTiles[GPU_MOD(nextTile[level - 1] + numTiles[level - 1] + 3, 4 * 4 * TILE_TESTERS)] = aux[3];
            }
            else
            {
                testTiles[level - 1][GPU_MOD(nextTile[level - 1] + numTiles[level - 1], 4 * TILE_TESTERS)] = aux[0];
                testTiles[level - 1][GPU_MOD(nextTile[level - 1] + numTiles[level - 1] + 1, 4 * TILE_TESTERS)] = aux[1];
                testTiles[level - 1][GPU_MOD(nextTile[level - 1] + numTiles[level - 1] + 2, 4 * TILE_TESTERS)] = aux[2];
                testTiles[level - 1][GPU_MOD(nextTile[level - 1] + numTiles[level - 1] + 3, 4 * TILE_TESTERS)] = aux[3];
            }

            /*  Update number of tiles in the sublevel.  */
            numTiles[level - 1] += subTiles;

            /*  Delete evaluated tile.  */
            delete testTiles[level][nextTile[level]];

            /*  Update number of tiles to process.  */
            numTiles[level]--;

            /*  Update next tile to expand in the current level.  */
            nextTile[level] = GPU_MOD(nextTile[level] + 1, 4 * TILE_TESTERS);

//printf("out sublevel %d numTiles %d nextTile %d\n", level - 1, numTiles[level - 1], nextTile[level - 1]);
        }

        /*  Check if any subtile was generated.  */
        if ((numTiles[level - 1] != 0) && ((level - 1) > scanLevel))
        {
            /*  Level down.  */
            level--;

            /*  Reset next tile pointer.  */
            //nextTile[level] = 0;
        }
        else
        {
            /*  Check if all tiles for the current level were evaluated.  */
            if (numTiles[level] == 0)
            {
                /*  Search for the next level up with tiles to evaluate.  */
                for(i = level; (i < MAX_LEVELS) && (numTiles[i] == 0); i++);

                /*  Level up.  */
                level = i;
            }
        }
//printf("out level %d numTiles %d nextTile %d\n", level, numTiles[level], nextTile[level]);
    }

    /*  Determine if all space has been searched.  */
    if ((level == MAX_LEVELS) && (numTiles[scanLevel] == 0))
        triangle->lastFragment();
}

/*  Recursive version of nextScanlineStamp.  */
Fragment **RasterizerEmulator::nextStampRecursive(u32bit triangleID)
{
    Fragment **stamp;
    SetupTriangle *triangle;
    //u32bit i;
    //bool isLast;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "nextStampRecursive", "Triangle identifier out of range.");
    )

    /*  Get setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "nextStampRecursive", "Triangle is not a setup triangle.");
    )

//printf("stored fragments %d stored gen tiles %d\n", trStoredFragments[triangleID], trStoredGenTiles[triangleID]);

    /*  Check if there are no stamps stored.  */
    if ((trStoredFragments[triangleID] == 0) && (trStoredGenTiles[triangleID] > 0))
    {
        /*  Delete the old list.  */
        //delete[] trFragments[triangleID];

        /*  Generate the fragments in the current generation tile.  */
        //trFragments[triangleID] = generateFragments(triangleID);
        generateFragments(triangleID);

        /*  Update number of stored generation tiles.  */
        trStoredGenTiles[triangleID]--;

        /*  Update number of stored fragments.  */
        trStoredFragments[triangleID] = genTileFragments;
    }

    /*  Check if there are stamps stored.  */
    if (trStoredFragments[triangleID] > 0)
    {
        /*  Allocate the stamp.  */
        stamp = new Fragment*[STAMP_FRAGMENTS];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (stamp == NULL)
                panic("RasterizerEmulator", "nextScanlineStampTiled", "Error allocating stamp.");
        )

        /*  Get next stored stamp.  */
        stamp[0] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID]];
        stamp[1] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 1];
        stamp[2] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 2];
        stamp[3] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 3];

        /*  Update number of stored fragments.  */
        trStoredFragments[triangleID] -= STAMP_FRAGMENTS;

//printf("Gen stamp\n");
        /*  Check if last fragment flag and no more stored fragments.  */
        if (triangle->isLastFragment() && (trStoredGenTiles[triangleID] == 0) && (trStoredFragments[triangleID] == 0))
        {
//printf("Last fragment\n");
            /*  Set stamp last fragment flag.  */
            stamp[3]->lastFragment();
        }
    }
    else
    {
//printf("No stamp\n");
        /*  No stamps generated.  */
        stamp = NULL;
    }

    return stamp;
}

/*  Recursive version of nextScanlineStamp.  */
Fragment **RasterizerEmulator::nextStampRecursiveMulti(u32bit batchID, u32bit &genTriangle)
{
    Fragment **stamp;
    SetupTriangle *triangle;
    //u32bit i;
    //bool isLast;
    bool moreFragments;
    u32bit triangleID;
    //u32bit numEvStamps;
    //Tile **evStamps;
    //Tile *auxTile;


    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (batchID != 0xFAFA)
            panic("RasterizerEmulator", "nextStampRecursiveMulti", "Batch identifier out of range.");
    )

    /*  Get first triangle in the batch.  */
    triangleID = triangleBatch[0];

    /*  Get setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "nextStampRecursiveMulti", "Triangle is not a setup triangle.");
    )

//printf("stored fragments %d stored gen tiles %d\n", trStoredFragments[triangleID], trStoredGenTiles[triangleID]);

    /*  Check if there are no stamps stored.  */
    if ((trStoredFragments[triangleID] == 0) && (trStoredGenTiles[triangleID] > 0))
    {
        //if (trStoredStamps[triangleID] == 0)
        //{
        //    auxTile = new Tile(*trGenTiles[triangleID][scanTileGenTiles - trStoredGenTiles[triangleID]]);

//printf("Evaluating gen tile level %d x %d y %d\n", auxTile->getLevel(), auxTile->getX(), auxTile->getY());

            /*  First check if the tile generates any stamp.  */
            //numEvStamps = evaluateTile(auxTile, evStamps, 1);

//printf("Num stamps evaluated %d\n", numEvStamps);

            //for(i = 0; i < numEvStamps; i++)
            //    delete evStamps[i];
            //delete[] evStamps;

            //if (numEvStamps == 0)
            //    delete trGenTiles[triangleID][scanTileGenTiles - trStoredGenTiles[triangleID]];
        //}
        //else
        //{
        //    numEvStamps = 1;
        //}

        //if (numEvStamps > 0)
        //{
            /*  Delete the old list.  */
            //delete[] trFragments[triangleID];
            //delete[] frTriangleID[triangleID];

            /*  Generate the fragments in the current generation tile.  */
            //moreFragments = generateFragmentsMulti(triangleID, trFragments[triangleID]);
            moreFragments = generateFragmentsMulti(triangleID);

            /*  Check if the generation tile can produce more fragments (from other triangles).  */
            if (!moreFragments)
            {
                /*  Update number of stored generation tiles.  */
                trStoredGenTiles[triangleID]--;
            }

            /*  Update number of stored fragments.  */
            trStoredFragments[triangleID] = genTileFragments;
        //}
        //else
        //{
            /*  Update number of stored generation tiles.  */
        //    trStoredGenTiles[triangleID]--;
        //}
    }

    /*  Check if there are stamps stored.  */
    if (trStoredFragments[triangleID] > 0)
    {
        /*  Allocate the stamp.  */
        stamp = new Fragment*[STAMP_FRAGMENTS];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (stamp == NULL)
                panic("RasterizerEmulator", "nextScanlineStampTiled", "Error allocating stamp.");
        )

        /*  Get next stored stamp.  */
        stamp[0] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID]];
        stamp[1] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 1];
        stamp[2] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 2];
        stamp[3] = trFragments[triangleID][genTileFragments - trStoredFragments[triangleID] + 3];

        /*  Get identifier of the triangle that generated the stamp (should be same for all the stamp).  */
        genTriangle = frTriangleID[triangleID][genTileFragments - trStoredFragments[triangleID]];
//printf("triangleID %d genTriangle %d\n", triangleID, genTriangle);

        /*  Update number of stored fragments.  */
        trStoredFragments[triangleID] -= STAMP_FRAGMENTS;

//printf("Gen stamp\n");
        /*  Check if last fragment flag and no more stored fragments.  */
        if (triangle->isLastFragment() && (trStoredGenTiles[triangleID] == 0) && (trStoredFragments[triangleID] == 0))
        {
//printf("Last fragment\n");
            /*  Set stamp last fragment flag.  */
            stamp[3]->lastFragment();
        }
    }
    else
    {
//printf("No stamp\n");
        /*  Set default identifier.  */
        genTriangle = (u32bit) -1;

        /*  No stamps generated.  */
        stamp = NULL;
    }

    return stamp;
}

/*  Expands a tile into tiles of the given level.  */
u32bit RasterizerEmulator::expandTile(Tile *tile, Tile **&tilesOut, u32bit level)
{
    Tile **tilesIn;
    Tile **auxTilesOut;
    u32bit inTiles;
    u32bit outTiles;
    u32bit i;

//printf("In tile level %d out tile level %d\n", level, tile->getLevel());

    /*  Allocate space for the tiles down to the target level.  */
    tilesIn = new Tile*[1 << (2 * (tile->getLevel() - level + 1))];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (tilesIn == NULL)
            panic("RasterizerEmulator", "expandTile", "Error allocating input tile array.");
    )

    /*  Allocate space for the tiles down to the target level.  */
    tilesOut = new Tile*[1 << (2 * (tile->getLevel() - level + 1))];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (tilesOut == NULL)
            panic("RasterizerEmulator", "expandTile", "Error allocating output tile array.");
    )

    /*  Set input tile.  */
    tilesIn[0] = tile;

    /*  Initialize generation variables.  */
    inTiles = 1;
    outTiles = 0;

    /*  Generate the tiles until the target level is reached.  */
    while(tilesIn[0]->getLevel() > level)
    {
//printf("Expanding %d tiles of level %d\n", inTiles, tilesIn[0]->getLevel());
        /*  Generate the new tiles.  */
        for(i = 0, outTiles = 0; i < inTiles; i++)
        {
            /*  Create new tiles from the current tile.  */
            generateTiles(tilesIn[i], &tilesOut[outTiles]);

            /*  Update number of generated tiles.  */
            outTiles += 4;

            /*  Delete current tile.  */
            delete tilesIn[i];
        }

        /*  Set output tiles as input tiles.  */
        auxTilesOut = tilesIn;
        tilesIn = tilesOut;
        tilesOut = auxTilesOut;

        inTiles = outTiles;
    }

    delete[] tilesOut;

    tilesOut = tilesIn;
    outTiles = inTiles;

    return outTiles;
}

/*  Evaluate a tile into tiles of a level.  */
u32bit RasterizerEmulator::evaluateTile(Tile *tile, Tile **&tilesOut, u32bit level)
{
    Tile **tilesIn;
    Tile **auxTilesOut;
    u32bit inTiles;
    u32bit outTiles;
    u32bit genTiles;
    u32bit i;

    /*  Allocate space for the tiles down to the target level.  */
    tilesIn = new Tile*[1 << (2 * (tile->getLevel() - level + 1))];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (tilesIn == NULL)
            panic("RasterizerEmulator", "evaluateTile", "Error allocating input tile array.");
    )

    /*  Allocate space for the tiles down to the target level.  */
    tilesOut = new Tile*[1 << (2 * (tile->getLevel() - level + 1))];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (tilesOut == NULL)
            panic("RasterizerEmulator", "evaluateTile", "Error allocating output tile array.");
    )

    /*  Set input tile.  */
    tilesIn[0] = tile;

    /*  Initialize generation variables.  */
    inTiles = 1;
    outTiles = 0;

    /*  Generate the tiles until the stamp level is reached.  */
    while((inTiles > 0) && (tilesIn[0]->getLevel() > level))
    {
        /*  Generate the new tiles.  */
        for(i = 0, outTiles = 0; i < inTiles; i++)
        {
            /*  Create new tiles from the current tile.  */
            genTiles = evaluateTile(tilesIn[i], &tilesOut[outTiles]);

            /*  Update number of generated tiles.  */
            outTiles += genTiles;

            /*  Delete current tile.  */
            delete tilesIn[i];
        }

        /*  Set output tiles as input tiles.  */
        auxTilesOut = tilesIn;
        tilesIn = tilesOut;
        tilesOut = auxTilesOut;

        inTiles = outTiles;
    }

    delete[] tilesOut;

    tilesOut = tilesIn;
    outTiles = inTiles;

    return outTiles;
}

/*  Expands a scan tile into generation tiles for the triangle current scan position.  */
Tile **RasterizerEmulator::expandScanTile(u32bit triangleID, u32bit &genTiles)
{
    SetupTriangle *triangle;
    f64bit scanPos[4];
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    s32bit x, y;
    Tile *tileIn;
    Tile **tilesOut;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "expandScanTile", "Triangle identifier out of range.");
    )

    /*  Get setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "expandScanTile", "Triangle is not a setup triangle.");
    )

    /*  Get triangle current edge equations.  */
    triangle->getEdgeEquations(edge1, edge2, edge3);

    /*  Get triangle current z interpolation equation.  */
    triangle->getZEquation(zeq);

    /*  Get triangle current raster position.  */
    triangle->getRasterPosition(x, y);

    /*  Get tile start position.  */
    scanPos[0] = edge1[2];
    scanPos[1] = edge2[2];
    scanPos[2] = edge3[2];
    scanPos[3] = zeq[2];

    /*  Create top tile.  */
    tileIn = new Tile(triangle, x, y, scanPos, scanPos[3], scanLevel);

    /*  Expand scan tile into generation tiles.  */
    genTiles = expandTile(tileIn, tilesOut, genLevel);

    return tilesOut;
}

/*  Returns if the last fragment of the triangle was generated using scanline rasterization.  */
bool RasterizerEmulator::lastFragment(u32bit triangle)
{
    /*  Check triangle ID range.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "lastFragment", "Triangle identifier out of range.");
    )

    /*  Check that the triangle ID points to a setup triangle.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "lastFragment", "Triangle identifier points to an unsetup triangle.");
    )

    return (setupTriangles[triangle]->isLastFragment()) &&
        (trStoredGenTiles[triangle] == 0) && (trStoredFragments[triangle] == 0);
}

/*  Interpolates an attribute of a fragment.  */
QuadFloat RasterizerEmulator::interpolate(Fragment *fr, u32bit attribute)
{
    f64bit r;
    f64bit f[3];
    f64bit p[3];
    QuadFloat attrib;
    QuadFloat *vAttr[3];

    /*  Check a fragment is passed.  */
    GPU_ASSERT(
        if (fr == NULL)
            panic("RasterizerEmulator", "interpolate", "NULL Fragment pointer.");
    )

    /*  Check attribute index range.  */
    GPU_ASSERT(
        if (attribute >= fragmentAttributes)
            panic("RasterizerEmulator", "interpolate2DH", "Fragment attribute index out of range.");
    )


    /*  Fragment attributes are interpolated from the vertex attributes
        using a method based in barycentric/edge coordinates:

        r = 1 / (F0(x, y) + F1(x, y) + F2(x, y))

        f0(x, y) = r * F0(x, y)
        f1(x, y) = r * F1(x, y)
        f2(x, y) = 1 - f0(x, y) - f1(x, y) = r * F2(x, y)

        pk(x, y) = pk0 * f0(x, y) + pk1 * f1(x, y) + pk2 * f2(x, y)


        Where:

            - F0, F1, F2 are the edge equation values at the fragment.
            - pk0, pk1, pk2 are the attribute value at each of the
              three vertices.


     */

    /*  Get the fragment edge/barycentric coordinates.  */
    f[0] = fr->getCoordinates()[0];
    f[1] = fr->getCoordinates()[1];
    f[2] = fr->getCoordinates()[2];

    /*  Calculate reciproque of the edge/barycentric coordinates sum.  */
    r = 1.0 / (f[0] + f[1] + f[2]);

    /*  Calculate fragment coordinate factors for interpolation.  */
    f[0] = r * f[0];
    f[1] = r * f[1];
    f[2] = r * f[2];

    /*  Each attribute is a 4 element vector of 32 bit fp values.  */

    /*  Get the three vertex attributes.  */
    vAttr[0] = fr->getTriangle()->getAttribute(0, attribute);
    vAttr[1] = fr->getTriangle()->getAttribute(1, attribute);
    vAttr[2] = fr->getTriangle()->getAttribute(2, attribute);

    /*  Get parameter vector for first attribute component.  */
    p[0] = (*vAttr[0])[0];
    p[1] = (*vAttr[1])[0];
    p[2] = (*vAttr[2])[0];

    /*  Interpolate attribute first component.  */
    attrib[0] = f32bit(p[0] * f[0] + p[1] * f[1] + p[2] * f[2]);

    /*  Get parameter vector for second attribute component.  */
    p[0] = (*vAttr[0])[1];
    p[1] = (*vAttr[1])[1];
    p[2] = (*vAttr[2])[1];


    /*  Interpolate attribute second component.  */
    attrib[1] = f32bit(p[0] * f[0] + p[1] * f[1] + p[2] * f[2]);

    /*  Get parameter vector for third attribute component.  */
    p[0] = (*vAttr[0])[2];
    p[1] = (*vAttr[1])[2];
    p[2] = (*vAttr[2])[2];

    /*  Interpolate attribute third component.  */
    attrib[2] = f32bit(p[0] * f[0] + p[1] * f[1] + p[2] * f[2]);

    /*  Get parameter vector for fourth attribute component.  */
    p[0] = (*vAttr[0])[3];
    p[1] = (*vAttr[1])[3];
    p[2] = (*vAttr[2])[3];

    /*  Interpolate attribute fourth component.  */
    attrib[3] = f32bit(p[0] * f[0] + p[1] * f[1] + p[2] * f[2]);

    return attrib;
}

/*  Interpolates all the attributes of a fragment.  */
void RasterizerEmulator::interpolate(Fragment *fr, QuadFloat *attribute)
{
    f64bit r;
    f64bit f[3];
    f64bit p[3];
    QuadFloat *vAttr[3];
    u32bit i;

    /*  Check a fragment is passed.  */
    GPU_ASSERT(
        if (fr == NULL)
            panic("RasterizerEmulator", "interpolate", "NULL Fragment pointer.");
    )

    /*  Fragment attributes are interpolated from the vertex attributes
        using a method based in barycentric/edge coordinates:

        r = 1 / (F0(x, y) + F1(x, y) + F2(x, y))

        f0(x, y) = r * F0(x, y)
        f1(x, y) = r * F1(x, y)
        f2(x, y) = 1 - f0(x, y) - f1(x, y) = r * F2(x, y)

        pk(x, y) = pk0 * f0(x, y) + pk1 * f1(x, y) + pk2 * f2(x, y)


        Where:

            - F0, F1, F2 are the edge equation values at the fragment.
            - pk0, pk1, pk2 are the attribute value at each of the
              three vertices.


     */

    /*  Get the fragment edge/barycentric coordinates.  */
    f[0] = fr->getCoordinates()[0];
    f[1] = fr->getCoordinates()[1];
    f[2] = fr->getCoordinates()[2];

    /*  Calculate reciproque of the edge/barycentric coordinates sum.  */
    r = 1.0 / (f[0] + f[1] + f[2]);

    /*  Calculate fragment coordinate factors for interpolation.  */
    f[0] = r * f[0];
    f[1] = r * f[1];
    f[2] = r * f[2];

    /*  Each attribute is a 4 element vector of 32 bit fp values.  */

    /*  Get the three vertex attributes.  */
    fr->getTriangle()->getVertexAttributes(vAttr[0], vAttr[1], vAttr[2]);

    /*  Interpolate all fragment attributes.  */
    for(i = 0; i < fragmentAttributes; i++)
    {
        /*  Each attribute is a 4 element vector of 32 bit fp values.  */

        /*  Get parameter vector for first attribute component.  */
        p[0] = vAttr[0][i][0];
        p[1] = vAttr[1][i][0];
        p[2] = vAttr[2][i][0];

        /*  Interpolate attribute first component.  */
        attribute[i][0] = f32bit(p[0] * f[0] + p[1] * f[1] + p[2] * f[2]);

        /*  Get parameter vector for second attribute component.  */
        p[0] = vAttr[0][i][1];
        p[1] = vAttr[1][i][1];
        p[2] = vAttr[2][i][1];

        /*  Interpolate attribute second component.  */
        attribute[i][1] = f32bit(p[0] * f[0] + p[1] * f[1] + p[2] * f[2]);

        /*  Get parameter vector for third attribute component.  */
        p[0] = vAttr[0][i][2];
        p[1] = vAttr[1][i][2];
        p[2] = vAttr[2][i][2];

        /*  Interpolate attribute third component.  */
        attribute[i][2] = f32bit(p[0] * f[0] + p[1] * f[1] + p[2] * f[2]);

        /*  Get parameter vector for fourth attribute component.  */
        p[0] = vAttr[0][i][3];
        p[1] = vAttr[1][i][3];
        p[2] = vAttr[2][i][3];

        /*  Interpolate attribute fourth component.  */
        attribute[i][3] = f32bit(p[0] * f[0] + p[1] * f[1] + p[2] * f[2]);
    }
}


/*  Return an aproximation of the triangle signed area.  */
f64bit RasterizerEmulator::triangleArea(u32bit triangle)
{
    /*  Check triangle ID range.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "triangleArea", "Out of range triangle identifier.");
    )

    /*  Check if the specified triangle exists.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "triangleArea", "Not a setup triangle.");
    )

    return setupTriangles[triangle]->getArea();
}

f64bit RasterizerEmulator::triScreenPercent(u32bit triangle)
{
    /*  Check triangle ID range.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "triScreenPercent", "Out of range triangle identifier.");
    )

    /*  Check if the specified triangle exists.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "triScreenPercent", "Not a setup triangle.");
    )

    return setupTriangles[triangle]->getScreenPercent();
}

/*  Returns the triangle Bounding Box.  */
void RasterizerEmulator::triangleBoundingBox(u32bit triangle, s32bit &x0, s32bit &y0, s32bit &x1, s32bit &y1)
{
    /*  Check triangle ID range.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "triangleBoundingBox", "Out of range triangle identifier.");
    )

    /*  Check if the specified triangle exists.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "triangleBoundingBox", "Not a setup triangle.");
    )

    setupTriangles[triangle]->getBoundingBox(x0, y0, x1, y1);
}

void RasterizerEmulator::computeMSAABoundingBox(bool multiSamplingEnabled, u32bit msaaSamples)
{
    //  Compute the MSAA sample bounding box.

    if (!multiSamplingEnabled)
    {
        sampleBBXMin = FixedPoint(0.5f, 16, subPixelPrecision);
        sampleBBYMin = FixedPoint(0.5f, 16, subPixelPrecision);
        sampleBBXMax = FixedPoint(0.5f, 16, subPixelPrecision);
        sampleBBYMax = FixedPoint(0.5f, 16, subPixelPrecision);
    }
    else
    {
        switch(msaaSamples)
        {
            case 1:

                sampleBBXMin = FixedPoint(0.5f, 16, subPixelPrecision);
                sampleBBYMin = FixedPoint(0.5f, 16, subPixelPrecision);
                sampleBBXMax = FixedPoint(0.5f, 16, subPixelPrecision);
                sampleBBYMax = FixedPoint(0.5f, 16, subPixelPrecision);

                break;

            case 2:

                sampleBBXMin = FixedPoint((MSAAPatternTable2[2].xOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBYMin = FixedPoint((MSAAPatternTable2[2].yOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBXMax = FixedPoint((MSAAPatternTable2[3].xOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBYMax = FixedPoint((MSAAPatternTable2[3].yOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);

                break;

            case 4:

                sampleBBXMin = FixedPoint((MSAAPatternTable4[4].xOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBYMin = FixedPoint((MSAAPatternTable4[4].yOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBXMax = FixedPoint((MSAAPatternTable4[5].xOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBYMax = FixedPoint((MSAAPatternTable4[5].yOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);

                break;

            case 6:

                sampleBBXMin = FixedPoint((MSAAPatternTable6[6].xOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBYMin = FixedPoint((MSAAPatternTable6[6].yOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBXMax = FixedPoint((MSAAPatternTable6[7].xOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBYMax = FixedPoint((MSAAPatternTable6[7].yOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);

                break;

            case 8:

                sampleBBXMin = FixedPoint((MSAAPatternTable8[8].xOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBYMin = FixedPoint((MSAAPatternTable8[8].yOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBXMax = FixedPoint((MSAAPatternTable8[9].xOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);
                sampleBBYMax = FixedPoint((MSAAPatternTable8[9].yOff / MSAA_SUBPIXEL_PRECISSION), 16, subPixelPrecision);

                break;

            default:
                panic("RasterizerEmulator", "computeMSAABoundingBox", "Unsupported MSAA mode.");
                break;
        }
    }
}


bool RasterizerEmulator::testMicroTriangle(u32bit triangle, u32bit microTriSzLimit, s32bit& minX, s32bit& minY, s32bit& maxX, s32bit& maxY, u32bit& pixelsX, u32bit& pixelsY, u32bit& stampsX, u32bit& stampsY)
{
    FixedPoint fxpMinX, fxpMinY, fxpMaxX, fxpMaxY;
    s32bit intXMin, intYMin, intXMax, intYMax;
    bool xMinInBorder;
    bool xMaxInBorder;
    bool yMinInBorder;
    bool yMaxInBorder;
    u32bit viewportShiftX;
    u32bit viewportShiftY;
    bool alignedX, alignedY;
    SetupTriangle* setupTriangle;
    QuadFloat nHVtx0Pos, nHVtx1Pos, nHVtx2Pos;

    //  Get the setup triangle object.
    setupTriangle = getSetupTriangle(triangle);

    //  Get the triangle bounding box.
    setupTriangle->getBoundingBox(minX, minY, maxX, maxY);

//printf("Testing triangle %d. Original BB: (%d,%d)->(%d,%d)\n",triangle, minX, minY, maxX, maxY);
    //  Get the subpixel precision bounding box.
    setupTriangle->getSubPixelBoundingBox(fxpMinX, fxpMinY, fxpMaxX, fxpMaxY);

    //  Get integer value (entire pixels bounding box).
    intXMin = s32bit(fxpMinX.integer().toFloat32());
    intYMin = s32bit(fxpMinY.integer().toFloat32());
    intXMax = s32bit(fxpMaxX.integer().toFloat32());
    intYMax = s32bit(fxpMaxY.integer().toFloat32());

    //  Check in-border limits.
    xMinInBorder = (intXMin == 0);
    xMaxInBorder = (intXMax == (w - 1));
    yMinInBorder = (intYMin == 0);
    yMaxInBorder = (intYMax == (h - 1));

    //  Initialize viewport border shifts.
    viewportShiftX = 0;
    viewportShiftY = 0;

    //  Adjust (shrink) triangle bounding box based on the MSAA sample bounding box area test (optimization).

    //  Determine if the subpixel left bound of the microtriangle is greater than
    //  the right limit of the pixel sampling area.
    if ((fxpMinX.fractional() > sampleBBXMax) && useBBOptimization)
    {
        //  All the pixel samples lay out of the microtriangle left bound.
        //  Adjust this pixel as the new BB left bound.
        minX = intXMin;
    }
    else
    {
        //  Set the previous pixel as the BB left bound clamped to the left viewport border.
        minX = xMinInBorder ? 0 : intXMin - 1;

        //  Annotate that clamp to the left viewport border avoided to
        //  let the proper one pixel margin to the left.
        if (xMinInBorder)
            viewportShiftX++;
    }

    //  Determine if the subpixel right bound of the microtriangle is lesser than
    //  the left limit of the pixel sampling area.
    if ((sampleBBXMin > fxpMaxX.fractional()) && useBBOptimization)
    {
        //  All the pixel samples lay out of the microtriangle right bound.
        //  Adjust this pixel as the new BB right bound.
        maxX = intXMax;
    }
    else
    {
        //  Set the posterior pixel as the BB right bound clamped to the right viewport border.
        maxX = xMaxInBorder ? (w - 1) : intXMax + 1;

        //  Annotate that clamp to the right viewport border avoided to
        //  let the proper one pixel margin to the right.
        if (xMaxInBorder)
            viewportShiftX++;

    }

    //  Determine if the subpixel lower bound of the microtriangle is greater than the
    //  upper limit of the pixel sampling area.
    if ((fxpMinY.fractional() > sampleBBYMax) && useBBOptimization)
    {
        //  All the pixel samples lay out of the microtriangle lower bound.
        //  Adjust this pixel as the new BB lower bound.
        minY = intYMin;
    }
    else
    {
        //  Set the previous pixel as the BB lower bound clamped to the bottom viewport border.
        minY = yMinInBorder ? 0 : intYMin - 1;

        //  Annotate that clamp to the bottom viewport border avoided to
        //  let the proper one pixel margin at bottom.
        if (yMinInBorder)
            viewportShiftY++;
    }

    //  Determine if the subpixel upper bound of the microtriangle is lesser than
    //  the lower limit of the pixel sampling area.
    if ((sampleBBYMin > fxpMaxY.fractional()) && useBBOptimization)
    {
        //  All the pixel samples lay out of the microtriangle upper bound.
        //  Adjust this pixel as the new BB upper bound.
        maxY = intYMax;
    }
    else
    {
        //  Set the posterior pixel as the BB upper bound clamped to the top viewport border.
        maxY = yMaxInBorder ? (h - 1) : intYMax + 1;

        //  Annotate that clamp to the top viewport border avoided to
        //  let the proper one pixel margin at top.
        if (yMaxInBorder)
            viewportShiftY++;
    }

    //  Compute pixels covered in the X direction.
    pixelsX = maxX - (minX + 1) + viewportShiftX;

    //  Compute pixels covered in the Y direction
    pixelsY = maxY - (minY + 1) + viewportShiftY;

    //  Compute if triangle lower X limit is aligned to the stamp.
    alignedX = xMinInBorder ? (GPU_MOD(minX, 2) == 0) : (GPU_MOD(minX + 1, 2) == 0);

    //  Compute if triangle lower Y limit is aligned to the stamp.
    alignedY = yMinInBorder ? (GPU_MOD(minY, 2) == 0) : (GPU_MOD(minY + 1, 2) == 0);

    //  Compute required number of entire stamps in X and Y direction.
    stampsX = (u32bit) GPU_FLOOR( (f32bit)pixelsX / 2.0f );
    stampsY = (u32bit) GPU_FLOOR( (f32bit)pixelsY / 2.0f );

    //  Account for an additional stamp if not aligned or covered pixels is not multiple of stamp pixels.
    if (!alignedX || (GPU_MOD( pixelsX, 2 ) > 0)) stampsX++;
    if (!alignedY || (GPU_MOD( pixelsY, 2 ) > 0)) stampsY++;

//printf("Adjusted BB: (%d,%d)->(%d,%d)\n",minX, minY, maxX, maxY);
//printf("PixelsX,Y: %d,%d, StampsX,Y: %d,%d, alignedX,Y: %d,%d\n\n", pixelsX, pixelsY, stampsX, stampsY, alignedX, alignedY);

    //  Decide if can become a microtriangle because of covered dimensions and size limit.
    if ((pixelsX <= 1) && (pixelsY <= 1))
    {
         return true;
    }
    else if ((stampsX == 1) && (stampsY == 1) && microTriSzLimit > 0)
    {
         return true;
    }
    else if ((stampsX * stampsY <= 4) && microTriSzLimit > 1)
    {
         return true;
    }
    else if ((stampsX * stampsY <= 16) && microTriSzLimit > 2)
    {
        return true;
    }
    else
    {
         return false;
    }
}

/*  Inverts the triangle facing direction (edge eq signs).  */
void RasterizerEmulator::invertTriangleFacing(u32bit triangle)
{
    /*  Check triangle ID range.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "inverTriangleFacing", "Out of range triangle identifier.");
    )

    /*  Check if the specified triangle exists.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "invertTriangleFacing", "Not a setup triangle.");
    )

    /*  Changes the sign of the triangle edge equations coefficients.  */
    setupTriangles[triangle]->invertEdgeEquations();
}

/*  Selects the triangle colors based on its facing (front or back).  */
void RasterizerEmulator::selectTwoSidedColor(u32bit triangle)
{
    QuadFloat *v1;
    QuadFloat *v2;
    QuadFloat *v3;

    /*  Check triangle ID range.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "twoSidedColor", "Out of range triangle identifier.");
    )

    /*  Check if the specified triangle exists.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "twoSidedColor", "Not a setup triangle.");
    )

    /*  Get the triangle vertex attributes.  */
    setupTriangles[triangle]->getVertexAttributes(v1, v2, v3);

    /*  Check triangle facing.  */
    if (setupTriangles[triangle]->getArea() > 0)
    {
        /*  Nothing to do.  Color for front faces already in correct attribute position.  */
    }
    else
    {
        /*  Back facing triangle.  Copy from back facing color attributes.  */
        v1[COLOR_ATTRIBUTE] = v1[COLOR_ATTRIBUTE_BACK_PRI];
        v2[COLOR_ATTRIBUTE] = v2[COLOR_ATTRIBUTE_BACK_PRI];
        v3[COLOR_ATTRIBUTE] = v3[COLOR_ATTRIBUTE_BACK_PRI];

        v1[COLOR_ATTRIBUTE_SEC] = v1[COLOR_ATTRIBUTE_BACK_SEC];
        v2[COLOR_ATTRIBUTE_SEC] = v2[COLOR_ATTRIBUTE_BACK_SEC];
        v3[COLOR_ATTRIBUTE_SEC] = v3[COLOR_ATTRIBUTE_BACK_SEC];
    }
}

/*  Copies a fragment attribute from a vertex.  */
QuadFloat RasterizerEmulator::copy(Fragment *fr, u32bit attribute, u32bit vertex)
{
    QuadFloat qf;

    /*  Copy the request attribute from the triangle vertex.  */
    qf = *(fr->getTriangle()->getAttribute(vertex, attribute));

    return qf;
}

/*  Copies the attributes from one of the triangle vertices to the fragment attributes.  */
void RasterizerEmulator::copy(Fragment *fr, QuadFloat *attributes, u32bit vertex)
{
    u32bit i;

    /*  Check the vertex identifier.  */
    GPU_ASSERT(
        if (vertex > 2)
            panic("RasterizerEmulator", "copy", "Vertex identifier out of range.");
    )

    /*  Copy all vertex attributes to the fragment attribute.  */
    for(i = 0; i < fragmentAttributes; i++)
        attributes[i] = *(fr->getTriangle()->getAttribute(vertex, i));
}

/*  Creates a top level tile for the setup triangle.  */
Tile *RasterizerEmulator::topLevelTile(u32bit triangle, bool msaaEnabled)
{
    Tile *topTile;
    f64bit edge1[3], edge2[3], edge3[3];
    f64bit zeq[3];
    f64bit startC[3];
    f64bit startZ;
    u32bit level;
    s32bit startX, startY;
    s32bit xMin, yMin, xMax, yMax;

    /*  Check triangle ID range.  */
    GPU_ASSERT(
        if (triangle >= activeTriangles)
            panic("RasterizerEmulator", "topLevelTile", "Triangle identifier out of range.");
    )

    /*  Check that the triangle ID points to a setup triangle.  */
    GPU_ASSERT(
        if (setupTriangles[triangle] == NULL)
            panic("RasterizerEmulator", "topLevelTile", "Triangle identifier points to an unsetup triangle.");
    )

    /*  Get triangle edge equations.  */
    setupTriangles[triangle]->getEdgeEquations(edge1, edge2, edge3);

    /*  Get triangle z/w equation.  */
    setupTriangles[triangle]->getZEquation(zeq);

    /*  Get triangle bounding box.  */
    setupTriangles[triangle]->getBoundingBox(xMin, yMin, xMax, yMax);

    /*  Adjust start of the bounding box to scan tile grid.  */
    startX = xMin - GPU_MOD(xMin, scanTileWidth);
    startY = yMin - GPU_MOD(yMin, scanTileHeight);

    /*  Calculate level (dimension) of the tile covering the triangle bounding box.  */
    level = GPU_MAX(u32bit(GPU_CEIL2(GPU_LOG2(GPU_MAX(xMax - startX + 1, yMax - startY + 1)))), scanLevel);

    /*  Calculate top level tile size.  */
    //level = (u32bit) GPU_CEIL2(GPU_LOG2(GPU_MAX(w, h)));

    /*  Adjust initial viewport coordinates to stamp/tile grid.  */
    //startX = x0 - GPU_MOD(x0, scanTileWidth);
    //startY = y0 - GPU_MOD(y0, scanTileHeight);

    //  Select a sample point that is (-0.5f, -0.5f) from the pixel position for
    //  when MSAA is enabled.
    f32bit xSampleOffset = msaaEnabled ? -0.5f : 0.0f;
    f32bit ySampleOffset = msaaEnabled ? -0.5f : 0.0f;

    /*  Calculate initial value for edge equations.  */
    startC[0] = edge1[0] * (startX + xSampleOffset) + edge1[1] * (startY + ySampleOffset) + edge1[2];
    startC[1] = edge2[0] * (startX + xSampleOffset) + edge2[1] * (startY + ySampleOffset) + edge2[2];
    startC[2] = edge3[0] * (startX + xSampleOffset) + edge3[1] * (startY + ySampleOffset) + edge3[2];

    /*  Calculate initial value for the z/w equation.  */
    startZ = zeq[0] * (startX + xSampleOffset) + zeq[1] * (startY + ySampleOffset) + zeq[2];

    /*  Create top level tile for the triangle.  */
    topTile = new Tile(setupTriangles[triangle], startX, startY, startC, startZ, level);

    return topTile;
}


/*  Creates a top level tile for the setup triangle.  */
Tile *RasterizerEmulator::topLevelTile(u32bit *triangles, u32bit numTris, bool msaaEnabled)
{
    Tile *topTile;
    SetupTriangle *setupTris[MAX_TRIANGLES];
    SetupTriangle *setTri;
    f64bit edge1[3], edge2[3], edge3[3];
    f64bit zeq[3];
    f64bit eC[MAX_TRIANGLES * 3];
    f64bit *startC[MAX_TRIANGLES];
    f64bit startZ[MAX_TRIANGLES];
    u32bit level;
    s32bit startX, startY;
    s32bit xMin, yMin, xMax, yMax;
    s32bit xMinAux, yMinAux, xMaxAux, yMaxAux;
    u32bit i;

    /*  Initialize the batch bound box (inverted scissor box).  */
    xMin = scissorX0 + scissorW;
    yMin = scissorY0 + scissorH;
    xMax = scissorX0;
    yMax = scissorY0;

    /*  Calculate the bounding box for the triangle batch.  */
    for(i = 0; i < numTris; i++)
    {
        /*  Check triangle ID range.  */
        GPU_ASSERT(
            if (triangles[i] >= activeTriangles)
                panic("RasterizerEmulator", "topLevelTile", "Triangle identifier out of range.");
        )

       /*  Get setup triangle.  */
        setTri = setupTriangles[triangles[i]];

        /*  Check that the triangle ID points to a setup triangle.  */
        GPU_ASSERT(
            if (setTri == NULL)
                panic("RasterizerEmulator", "topLevelTile", "Triangle identifier points to an unsetup triangle.");
        )

        /*  Get triangle bounding box.  */
        setTri->getBoundingBox(xMinAux, yMinAux, xMaxAux, yMaxAux);

        /*  Update bounding box for the triangle batch.  */
        if (xMinAux < xMin)
            xMin = xMinAux;
        if (yMinAux < yMin)
            yMin = yMinAux;
        if (xMaxAux > xMax)
            xMax = xMaxAux;
        if (yMaxAux > yMax)
            yMax = yMaxAux;
    }

    /*  Adjust initial viewport coordinates to stamp/tile grid.  */
    //startX = x0 - GPU_MOD(x0, scanTileWidth);
    //startY = y0 - GPU_MOD(y0, scanTileHeight);

    /*  Calculate top level tile size.  */
    //level = (u32bit) GPU_CEIL2(GPU_LOG2(GPU_MAX(w, h)));

    /*  Adjust start of the bounding box to scan tile grid.  */
    startX = xMin - GPU_MOD(xMin, scanTileWidth);
    startY = yMin - GPU_MOD(yMin, scanTileHeight);

    /*  Calculate level (dimension) of the tile covering the triangle bounding box.  */
    level = GPU_MAX(u32bit(GPU_CEIL2(GPU_LOG2(GPU_MAX(xMax - startX + 1, yMax - startY + 1)))), scanLevel);

//printf("xMin %d xMax %d yMin %d yMax %d\n", xMin, xMax, yMin, yMax);
//printf("startX %d startY %d level %d\n", startX, startY, level);

    //  Select a sample point that is (-0.5f, -0.5f) from the pixel position for
    //  when MSAA is enabled.
    f32bit xSampleOffset = msaaEnabled ? -0.5f : 0.0f;
    f32bit ySampleOffset = msaaEnabled ? -0.5f : 0.0f;

    /*  Get all the triangle equation values for the start position.  */
    for(i = 0; i < numTris; i++)
    {
       /*  Get setup triangle.  */
        setTri = setupTriangles[triangles[i]];

        /*  Get triangle edge equations.  */
        setTri->getEdgeEquations(edge1, edge2, edge3);

        /*  Get triangle z/w equation.  */
        setTri->getZEquation(zeq);

        /*  Initialize pointer to the array of values.  */
        startC[i] = &eC[i * 3];

        /*  Calculate initial value for edge equations.  */
        startC[i][0] = edge1[0] * (startX + xSampleOffset) + edge1[1] * (startY + ySampleOffset) + edge1[2];
        startC[i][1] = edge2[0] * (startX + xSampleOffset) + edge2[1] * (startY + ySampleOffset) + edge2[2];
        startC[i][2] = edge3[0] * (startX + xSampleOffset) + edge3[1] * (startY + ySampleOffset) + edge3[2];

        /*  Calculate initial value for the z/w equation.  */
        startZ[i] = zeq[0] * (startX + xSampleOffset) + zeq[1] * (startY + ySampleOffset) + zeq[2];

        /*  Set pointer to setup triangle.  */
        setupTris[i] = setTri;
    }

    /*  Create top level tile for the triangle.  */
    topTile = new Tile(setupTris, numTris, startX, startY, startC, startZ, level);

    return topTile;
}

/*  Creates the specified tile for the batch of setup triangle.  */
Tile *RasterizerEmulator::createTile(u32bit *triangles, u32bit numTris, s32bit x, s32bit y, u32bit level)
{
    Tile *topTile;
    SetupTriangle *setupTris[MAX_TRIANGLES];
    SetupTriangle *setTri;
    f64bit edge1[3], edge2[3], edge3[3];
    f64bit zeq[3];
    f64bit eC[MAX_TRIANGLES * 3];
    f64bit *startC[MAX_TRIANGLES];
    f64bit startZ[MAX_TRIANGLES];
    s32bit startX, startY;
    u32bit i;

    /*  Check the tile level.  */
    GPU_ASSERT
    (
        /*  Calculate the scan tile level.  */
        if ((level >= MAX_LEVELS) || (level <= scanLevel))
            panic("RasterizerEmulator", "createTile", "Tile level out of range.");
    )

    /*  Adjust tile coordinates to stamp/tile grid.  */
    startX = x - GPU_MOD(x, scanTileWidth);
    startY = y - GPU_MOD(y, scanTileHeight);

    /*  Get all the triangle equation values for the start position.  */
    for(i = 0; i < numTris; i++)
    {
        /*  Check triangle ID range.  */
        GPU_ASSERT(
            if (triangles[i] >= activeTriangles)
                panic("RasterizerEmulator", "createTile", "Triangle identifier out of range.");
        )

        /*  Get setup triangle.  */
        setTri = setupTriangles[triangles[i]];

        /*  Check that the triangle ID points to a setup triangle.  */
        GPU_ASSERT(
            if (setTri == NULL)
                panic("RasterizerEmulator", "createTile", "Triangle identifier points to an unsetup triangle.");
        )

        /*  Get triangle edge equations.  */
        setTri->getEdgeEquations(edge1, edge2, edge3);

        /*  Get triangle z/w equation.  */
        setTri->getZEquation(zeq);

        /*  Initialize pointer to the array of values.  */
        startC[i] = &eC[i * 3];

        /*  Calculate initial value for edge equations.  */
        startC[i][0] = edge1[0] * startX + edge1[1] * startY + edge1[2];
        startC[i][1] = edge2[0] * startX + edge2[1] * startY + edge2[2];
        startC[i][2] = edge3[0] * startX + edge3[1] * startY + edge3[2];

        /*  Calculate initial value for the z/w equation.  */
        startZ[i] = zeq[0] * startX + zeq[1] * startY + zeq[2];

        /*  Set pointer to setup triangle.  */
        setupTris[i] = setTri;
    }

    /*  Create top level tile for the triangle.  */
    topTile = new Tile(setupTris, numTris, startX, startY, startC, startZ, level);

    return topTile;
}


/*  Generates nine subtile sample points (4 subtiles) from a tile sample point (start point).  */
void RasterizerEmulator::generateSubTileSamples(Tile *tile, u32bit triId, f64bit *s0, f64bit *s1, f64bit *s2, f64bit *s3,
    f64bit *s4, f64bit *s5, f64bit *s6, f64bit *s7, f64bit *s8)
{
    SetupTriangle *triangle;
    f64bit e1[3], e2[3], e3[3], zeq[3];
    f64bit sa[4], sb[4], s2a[4], s2b[4];
    f64bit *edgeEq;
    u32bit level;

    /*  Get tile level.  */
    level = tile->getLevel();


    /*  Get tile start point edge and z equation values. */
    edgeEq = tile->getEdgeEquations(triId);
    s0[0] = edgeEq[0];
    s0[1] = edgeEq[1];
    s0[2] = edgeEq[2];
    s0[3] = tile->getZEquation(triId);

    /*  Get tile setup triangle.  */
    triangle = tile->getTriangle(triId);

    /*  Get the triangle edge equations.  */
    triangle->getEdgeEquations(e1, e2, e3);

    /*  Get the triangle z interpolation equation.  */
    triangle->getZEquation(zeq);

    /*  Calculate the sample points for the four subtiles to be
        tested inside the tile.

        2b  s6--s7--s8
            |   |   |
         b  s3--s4--s5
            |   |   |
            s0--s1--s2

                a   2a

        s0 : tile start point.
        s1 - s8 : subtile sample points.
        a : equation horizontal coefficient
        b : equation vertical coefficient


        Vertical and horizontal coefficients are scaled to the
        tile size (level).

        a' = a << (level - 1)
        b' = a << (level - 1)
        2a' = a << level
        2b' = b << level

        A subtile is generated if there is any fragment of the
        triangle(s) being rasterized inside it.

     */

    /*  Calculate scaled equation coefficients.  */
    sa[0] = e1[0] * GPU_POWER2OF(level - 1);
    sa[1] = e2[0] * GPU_POWER2OF(level - 1);
    sa[2] = e3[0] * GPU_POWER2OF(level - 1);
    sa[3] = zeq[0] * GPU_POWER2OF(level - 1);

    //s2a[0] = e1[0] * GPU_POWER2OF(level);
    //s2a[1] = e2[0] * GPU_POWER2OF(level);
    //s2a[2] = e3[0] * GPU_POWER2OF(level);
    //s2a[3] = zeq[0] * GPU_POWER2OF(level);
    s2a[0] = e1[0] * GPU_POWER2OF(level) + e1[0];
    s2a[1] = e2[0] * GPU_POWER2OF(level) + e2[0];
    s2a[2] = e3[0] * GPU_POWER2OF(level) + e3[0];
    s2a[3] = zeq[0] * GPU_POWER2OF(level + zeq[0]);

    sb[0] = e1[1] * GPU_POWER2OF(level - 1);
    sb[1] = e2[1] * GPU_POWER2OF(level - 1);
    sb[2] = e3[1] * GPU_POWER2OF(level - 1);
    sb[3] = zeq[1] * GPU_POWER2OF(level - 1);

    //s2b[0] = e1[1] * GPU_POWER2OF(level);
    //s2b[1] = e2[1] * GPU_POWER2OF(level);
    //s2b[2] = e3[1] * GPU_POWER2OF(level);
    //s2b[3] = zeq[1] * GPU_POWER2OF(level);
    s2b[0] = e1[1] * GPU_POWER2OF(level) + e1[1];
    s2b[1] = e2[1] * GPU_POWER2OF(level) + e2[1];
    s2b[2] = e3[1] * GPU_POWER2OF(level) + e3[1];
    s2b[3] = zeq[1] * GPU_POWER2OF(level) + zeq[1];

    /*  Calculate the subtiles sample points.  */
    GPUMath::ADD64(s1, s0, sa);
    GPUMath::ADD64(s2, s0, s2a);
    GPUMath::ADD64(s3, s0, sb);
    GPUMath::ADD64(s4, s1, sb);
    GPUMath::ADD64(s5, s2, sb);
    GPUMath::ADD64(s6, s0, s2b);
    GPUMath::ADD64(s7, s1, s2b);
    GPUMath::ADD64(s8, s2, s2b);
}

/*  Generates 4 subtile sample points (4 subtile start points) from a tile sample point (start point).  */
void RasterizerEmulator::generateSubTileSamples(Tile *tile, u32bit triId, f64bit *s0, f64bit *s1, f64bit *s2, f64bit *s3)
{
    SetupTriangle *triangle;
    f64bit e1[3], e2[3], e3[3], zeq[3];
    f64bit sa[4], sb[4];
    f64bit *edgeEq;
    u32bit level;

    /*  Get tile level.  */
    level = tile->getLevel();

    /*  Get tile start point edge and z equation values. */
    edgeEq = tile->getEdgeEquations(triId);
    s0[0] = edgeEq[0];
    s0[1] = edgeEq[1];
    s0[2] = edgeEq[2];
    s0[3] = tile->getZEquation(triId);

    /*  Get tile setup triangle.  */
    triangle = tile->getTriangle(triId);

    /*  Get the triangle edge equations.  */
    triangle->getEdgeEquations(e1, e2, e3);

    /*  Get the triangle z interpolation equation.  */
    triangle->getZEquation(zeq);

    /*  Calculate the sample points for the four subtiles to be
        tested inside the tile.

        2b  *---*---*
            |   |   |
         b  s2--s3--*
            |   |   |
            s0--s1--*

                a   2a

        s0 : tile start point.
        s1 - s3 : subtile sample points (tile start points).
        a : equation horizontal coefficient
        b : equation vertical coefficient


        Vertical and horizontal coefficients are scaled to the
        tile size (level).

        a' = a << (level - 1)
        b' = a << (level - 1)
        2a' = a << level
        2b' = b << level

        A subtile is generated if there is any fragment of the
        triangle(s) being rasterized inside it.

     */

    /*  Calculate scaled equation coefficients.  */
    sa[0] = e1[0] * GPU_POWER2OF(level - 1);
    sa[1] = e2[0] * GPU_POWER2OF(level - 1);
    sa[2] = e3[0] * GPU_POWER2OF(level - 1);
    sa[3] = zeq[0] * GPU_POWER2OF(level - 1);

    sb[0] = e1[1] * GPU_POWER2OF(level - 1);
    sb[1] = e2[1] * GPU_POWER2OF(level - 1);
    sb[2] = e3[1] * GPU_POWER2OF(level - 1);
    sb[3] = zeq[1] * GPU_POWER2OF(level - 1);

    /*  Calculate the subtiles sample points (subtile start points).  */
    GPUMath::ADD64(s1, s0, sa);
    GPUMath::ADD64(s2, s0, sb);
    GPUMath::ADD64(s3, s1, sb);
}



/*  Evaluates a tile.  */
u32bit RasterizerEmulator::evaluateTile(Tile *tile, Tile **outputTiles)
{
    SetupTriangle *triangles[MAX_TRIANGLES];
    f64bit es0[MAX_TRIANGLES * 4], es1[MAX_TRIANGLES * 4], es3[MAX_TRIANGLES * 4], es4[MAX_TRIANGLES * 4];
    f64bit *s0[MAX_TRIANGLES];
    f64bit *s1[MAX_TRIANGLES];
    f64bit *s3[MAX_TRIANGLES];
    f64bit *s4[MAX_TRIANGLES];
    f64bit s2[4], s5[4], s6[4], s7[4], s8[4];
    u32bit level;
    s32bit x, y;
    bool evTile[4];
    u32bit generatedTiles;
    u32bit i;
    bool *inside;
    bool insideTile[MAX_TRIANGLES][4];
    u32bit numTriangles;
    f64bit zeq[MAX_TRIANGLES];

    /*  Get tile level.  */
    level = tile->getLevel();

    /*  Check if it is a stamp/fragment level tile!!  */
    GPU_ASSERT(
        if (level <= 1)
            panic("RasterizerEmulator", "evaluateTile", "Trying to evaluate a fragment level tile.");
    )

    /*  Get the number of triangles in the tile.  */
    numTriangles = tile->getNumTriangles();

    /*  Initialize arrays for the tile start points.  */
    for(i = 0; i < numTriangles; i++)
    {
        s0[i] = &es0[i * 4];
        s1[i] = &es1[i * 4];
        s3[i] = &es3[i * 4];
        s4[i] = &es4[i * 4];
    }

    /*  Get tile start x position.  */
    x = tile->getX();

    /*  Get tile start y position.  */
    y = tile->getY();

    /*  Initialize evaluation array for the 4 tiles.  */
    evTile[0] = evTile[1] = evTile[2] = evTile[3] = FALSE;

    /*  Get array with triangle that are inside the tile.  */
    inside = tile->getInside();

    /*  Evaluate all triangles in the tile.  */
    for(i = 0; i < numTriangles; i++)
    {
        /*  Get tile setup triangle.  */
        triangles[i] = tile->getTriangle(i);

        /*  Check if the triangle is inside the tile.  */
        if (inside[i])
        {

            /*  Generate the nine sample points for the 4 subtiles.  */
            generateSubTileSamples(tile, i, s0[i], s1[i], s2, s3[i], s4[i], s5, s6, s7, s8);

            /*  Evaluate if there are triangle fragments inside the 4 subtiles:

                    +---+---+
                    | 2 | 3 |
                    +---+---+
                    | 0 | 1 |
                    +---+---+

             */

            /*  Evaluate first subtile.  */
            insideTile[i][0] = GPUMath::evaluateTile(s0[i], s1[i], s4[i], s3[i]);

            /*  Evaluate second subtile.  */
            insideTile[i][1] = GPUMath::evaluateTile(s1[i], s2, s5, s4[i]);

            /*  Evaluate third subtile.  */
            insideTile[i][2] = GPUMath::evaluateTile(s3[i], s4[i], s7, s6);

            /*  Evaluate fourth subtile.  */
            insideTile[i][3] = GPUMath::evaluateTile(s4[i], s5, s8, s7);

            /*  Update global evaluation results.  */
            evTile[0] = evTile[0] || insideTile[i][0];
            evTile[1] = evTile[1] || insideTile[i][1];
            evTile[2] = evTile[2] || insideTile[i][2];
            evTile[3] = evTile[3] || insideTile[i][3];
        }
        else
        {
            /*  Set that the triangle is outside all the subtiles.  */
            insideTile[i][0] = FALSE;
            insideTile[i][1] = FALSE;
            insideTile[i][2] = FALSE;
            insideTile[i][3] = FALSE;

            /*  Zero start positions for the triangle that doesn't have fragments inside the tile.  */
            s0[i][0] = s0[i][1] = s0[i][2] = s0[i][3] = s1[i][0] = s1[i][1] = s1[i][2] = s1[i][3] =
            s3[i][0] = s3[i][1] = s3[i][2] = s3[i][3] = s4[i][0] = s4[i][1] = s4[i][2] = s4[i][3] = 0.0f;
        }
    }

    /*  Reset generated tiles counter.  */
    generatedTiles = 0;

    /*  Generate first subtile if fragments inside.  */
    if (evTile[0] && (x <= (x0 + s32bit(w))) && (y <= (y0 + s32bit(w))))
    {
        /*  Create array with the z equation values.  */
        for(i = 0; i < numTriangles; i++)
                zeq[i] = s0[i][3];

        /*  Create subtile from top tile data.  */
        outputTiles[generatedTiles] = new Tile(triangles, numTriangles, x, y, s0, zeq, level - 1);

        /*  Get inside tile flag array.  */
        inside = outputTiles[generatedTiles]->getInside();

        /*  Update inside flag array for the nex tile.  */
        for(i = 0; i < numTriangles; i++)
            inside[i] = insideTile[i][0];

        /*  Update generated tiles counter.  */
        generatedTiles++;
    }

    /*  Generate second subtile if fragments inside.  */
    if (evTile[1] && ((x + GPU_2N(s32bit(level) - 1)) <= (x0 + s32bit(w))) && (y <= (y0 + s32bit(w))))
    {
        /*  Create array with the z equation values.  */
        for(i = 0; i < numTriangles; i++)
            zeq[i] = s1[i][3];

        /*  Create subtile from top tile data.  */
        outputTiles[generatedTiles] = new Tile(triangles, numTriangles, x + GPU_2N(level - 1), y, s1, zeq, level - 1);

        /*  Get inside tile flag array.  */
        inside = outputTiles[generatedTiles]->getInside();

        /*  Update inside flag array for the nex tile.  */
        for(i = 0; i < numTriangles; i++)
            inside[i] = insideTile[i][1];

        /*  Update generated tiles counter.  */
        generatedTiles++;
    }

    /*  Generate third subtile if fragments inside.  */
    if (evTile[2] && (x <= (x0 + s32bit(w))) && ((y + GPU_2N(s32bit(level) - 1)) <= (y0 + s32bit(w))))
    {
        /*  Create array with the z equation values.  */
        for(i = 0; i < numTriangles; i++)
            zeq[i] = s3[i][3];

        /*  Create subtile from top tile data.  */
        outputTiles[generatedTiles] = new Tile(triangles, numTriangles, x, y + GPU_2N(level - 1), s3, zeq, level - 1);

        /*  Get inside tile flag array.  */
        inside = outputTiles[generatedTiles]->getInside();

        /*  Update inside flag array for the nex tile.  */
        for(i = 0; i < numTriangles; i++)
            inside[i] = insideTile[i][2];

        /*  Update generated tiles counter.  */
        generatedTiles++;
    }

    /*  Generate fourth subtile if fragments inside.  */
    if (evTile[3] && ((x + GPU_2N(level - 1)) <= (x0 + w)) && ((y + GPU_2N(level - 1)) <= (y0 + w)))
    {
        /*  Create array with the z equation values.  */
        for(i = 0; i < numTriangles; i++)
            zeq[i] = s4[i][3];

        /*  Create subtile from top tile data.  */
        outputTiles[generatedTiles] = new Tile(triangles, numTriangles, x + GPU_2N(level - 1), y + GPU_2N(level - 1),
            s4, zeq, level - 1);

        /*  Get inside tile flag array.  */
        inside = outputTiles[generatedTiles]->getInside();

        /*  Update inside flag array for the nex tile.  */
        for(i = 0; i < numTriangles; i++)
            inside[i] = insideTile[i][3];

        /*  Update generated tiles counter.  */
        generatedTiles++;
    }

    return generatedTiles;
}

/*  Generates four subtiles from the input tile.  */
void RasterizerEmulator::generateTiles(Tile *tile, Tile **outputTiles)
{
    SetupTriangle *triangles[MAX_TRIANGLES];
    f64bit es0[MAX_TRIANGLES * 4], es1[MAX_TRIANGLES * 4], es2[MAX_TRIANGLES * 4], es3[MAX_TRIANGLES * 4];
    f64bit *s0[MAX_TRIANGLES], *s1[MAX_TRIANGLES], *s2[MAX_TRIANGLES], *s3[MAX_TRIANGLES];
    u32bit level;
    s32bit x, y;
    //u32bit generatedTiles;
    u32bit numTriangles;
    u32bit i;
    f64bit zeq[MAX_TRIANGLES];
    bool *oldInside;
    bool *inside;

    /*  Get tile level.  */
    level = tile->getLevel();

    /*  Check if it is a stamp/fragment level tile!!  */
    GPU_ASSERT(
        if (level <= 1)
            panic("RasterizerEmulator", "generateTile", "Trying to subdivide a stamp/fragment level tile.");
    )

    /*  Get number of triangles in the tile.  */
    numTriangles = tile->getNumTriangles();

    /*  Get tile triangle inside tile flag array.  */
    oldInside = tile->getInside();

    /*  Initialize arrays for the edge equation samples.  */
    for(i = 0; i < numTriangles; i++)
    {
        s0[i] = &es0[i * 4];
        s1[i] = &es1[i * 4];
        s2[i] = &es2[i * 4];
        s3[i] = &es3[i * 4];
    }

    /*  Get tile start x position.  */
    x = tile->getX();

    /*  Get tile start y position.  */
    y = tile->getY();

    /*  Generate the samples for all the triangles in the tile.  */
    for(i = 0; i < numTriangles; i++)
    {
        /*  Get tile setup triangle.  */
        triangles[i] = tile->getTriangle(i);

        /*  Generate the 4 sample start points for the 4 subtiles.  */
        generateSubTileSamples(tile, i, s0[i], s1[i], s2[i], s3[i]);
    }

    /*  Gather the z equation sample values for the triangles.  */
    for(i = 0; i < numTriangles; i++)
        zeq[i] = s0[i][3];

    /*  Create subtile from top tile data.  */
    outputTiles[0] = new Tile(triangles, numTriangles, x, y, s0, zeq, level - 1);

    /*  Get subtile inside flag array.  */
    inside = outputTiles[0]->getInside();

    /*  Copy inside flag array for the sub tile.  */
    for(i = 0; i < numTriangles; i++)
        inside[i] = oldInside[i];


    /*  Gather the z equation sample values for the triangles.  */
    for(i = 0; i < numTriangles; i++)
        zeq[i] = s1[i][3];

    /*  Create subtile from top tile data.  */
    outputTiles[1] = new Tile(triangles, numTriangles, x + GPU_2N(level - 1), y, s1, zeq, level - 1);

    /*  Get subtile inside flag array.  */
    inside = outputTiles[1]->getInside();

    /*  Copy inside flag array for the sub tile.  */
    for(i = 0; i < numTriangles; i++)
        inside[i] = oldInside[i];


    /*  Gather the z equation sample values for the triangles.  */
    for(i = 0; i < numTriangles; i++)
        zeq[i] = s2[i][3];

    /*  Create subtile from top tile data.  */
    outputTiles[2] = new Tile(triangles, numTriangles, x, y + GPU_2N(level - 1), s2, zeq, level - 1);

    /*  Get subtile inside flag array.  */
    inside = outputTiles[2]->getInside();

    /*  Copy inside flag array for the sub tile.  */
    for(i = 0; i < numTriangles; i++)
        inside[i] = oldInside[i];


    /*  Gather the z equation sample values for the triangles.  */
    for(i = 0; i < numTriangles; i++)
        zeq[i] = s3[i][3];

    /*  Create subtile from top tile data.  */
    outputTiles[3] = new Tile(triangles, numTriangles, x + GPU_2N(level - 1), y + GPU_2N(level - 1), s3, zeq, level - 1);

    /*  Get subtile inside flag array.  */
    inside = outputTiles[3]->getInside();

    /*  Copy inside flag array for the sub tile.  */
    for(i = 0; i < numTriangles; i++)
        inside[i] = oldInside[i];
}

/*  Generates the fragments of a stamp level tile.  */
void RasterizerEmulator::generateStamp(Tile *tile, Fragment **fragments)
{
    f64bit s0[4], s1[4], s2[4], s3[4];
    f64bit e1[3], e2[3], e3[3], zeq[3];
    f64bit a[4], b[4];
    SetupTriangle *triangle;
    s32bit x, y;

    /*  Stamp/quad size is fixed as 2x2 fragments.  */

    /*  Check level of the input tile.  */
    GPU_ASSERT(
        if (tile->getLevel() != 1)
            panic("RasterizerEmulator", "generateStamp", "Trying to generate fragments for a non stamp level tile.");
    )

    /*  Get stamp start point equations values.  */
    s0[0] = tile->getEdgeEquations()[0];
    s0[1] = tile->getEdgeEquations()[1];
    s0[2] = tile->getEdgeEquations()[2];
    s0[3] = tile->getZEquation();

    /*  Get tile/stamp start coordinates.  */
    x = tile->getX();
    y = tile->getY();

    /*  Get tile setup triangle.  */
    triangle = tile->getTriangle();

    /*  Get triangle edge equations.  */
    triangle->getEdgeEquations(e1, e2, e3);

    /*  Get triangle Z equation.  */
    triangle->getZEquation(zeq);

    /*  Current implementation generates just a 2x2 fragment stamp:

        s3 s4
        s0 s1

     */

    /*  Create horizontal coefficient vector.  */
    a[0] = e1[0];
    a[1] = e2[0];
    a[2] = e3[0];
    a[3] = zeq[0];

    /*  Create vertical coefficient vector.  */
    b[0] = e1[1];
    b[1] = e2[1];
    b[2] = e3[1];
    b[3] = zeq[1];

    /*  Calculate stamp fragment samples.  */
    GPUMath::ADD64(s1, s0, a);
    GPUMath::ADD64(s2, s0, b);
    GPUMath::ADD64(s3, s1, b);

    /*  Generate stamp fragments.  */
    fragments[0] = new Fragment(triangle, x, y, convertZ(s0[3]), s0, testInsideTriangle(triangle, s0));
    fragments[1] = new Fragment(triangle, x + 1, y, convertZ(s1[3]), s1, testInsideTriangle(triangle, s1));
    fragments[2] = new Fragment(triangle, x, y + 1, convertZ(s2[3]), s2, testInsideTriangle(triangle, s2));
    fragments[3] = new Fragment(triangle, x + 1, y + 1, convertZ(s3[3]), s3, testInsideTriangle(triangle, s3));
}

/*  Generates the fragments of a stamp level tile.  */
bool RasterizerEmulator::generateStampMulti(Tile *tile, Fragment **fragments, u32bit &genTriID)
{
    f64bit s0[4], s1[4], s2[4], s3[4];
    f64bit e1[3], e2[3], e3[3], zeq[3];
    f64bit a[4], b[4];
    f64bit *edgeEq;
    SetupTriangle *triangle;
    u32bit nextTriangle;
    s32bit x, y;
    bool *inside;
    u32bit i;
    u32bit numTriangles;

    /*  Stamp/quad size is fixed as 2x2 fragments.  */

    /*  Check level of the input tile.  */
    GPU_ASSERT(
        if (tile->getLevel() != 1)
            panic("RasterizerEmulator", "generateStampMulti", "Trying to generate fragments for a non stamp level tile.");
    )

    /*  Get next triangle in the tile.  */
    nextTriangle = tile->getNextTriangle();

    /*  Get number of triangles in the tile.  */
    numTriangles = tile->getNumTriangles();

    /*  Check if next triangle isn't a triangle in the tile.  */
    GPU_ASSERT(
        if (nextTriangle >= numTriangles)
            panic("RasterizerEmulator", "generateStampMulti", "Next tile triangle out of range.");
    )

    /*  Get the array of triangle in tile flags.  */
    inside = tile->getInside();

    /*  Check if current triangle is inside the stamp and search for the first one inside.  */
    for(; (nextTriangle < numTriangles) && (!inside[nextTriangle]); nextTriangle++);

    /*  Check if there is no triangles inside the stamp.  */
    GPU_ASSERT(
        if (nextTriangle == numTriangles)
            panic("RasterizerEmulator", "generateStampMulti", "No triangles in the batch are inside the tile.");
    )

    /*  Get stamp start point equations values.  */
    edgeEq = tile->getEdgeEquations(nextTriangle);
    s0[0] = edgeEq[0];
    s0[1] = edgeEq[1];
    s0[2] = edgeEq[2];
    s0[3] = tile->getZEquation(nextTriangle);

    /*  Get tile/stamp start coordinates.  */
    x = tile->getX();
    y = tile->getY();

    /*  Get tile setup triangle.  */
    triangle = tile->getTriangle(nextTriangle);

    /*  Get triangle edge equations.  */
    triangle->getEdgeEquations(e1, e2, e3);

    /*  Get triangle Z equation.  */
    triangle->getZEquation(zeq);

    /*  Current implementation generates just a 2x2 fragment stamp:

        s3 s4
        s0 s1

     */

    /*  Create horizontal coefficient vector.  */
    a[0] = e1[0];
    a[1] = e2[0];
    a[2] = e3[0];
    a[3] = zeq[0];

    /*  Create vertical coefficient vector.  */
    b[0] = e1[1];
    b[1] = e2[1];
    b[2] = e3[1];
    b[3] = zeq[1];

    /*  Calculate stamp fragment samples.  */
    GPUMath::ADD64(s1, s0, a);
    GPUMath::ADD64(s2, s0, b);
    GPUMath::ADD64(s3, s1, b);

    /*  Generate stamp fragments.  */
    fragments[0] = new Fragment(triangle, x, y, convertZ(s0[3]), s0, testInsideTriangle(triangle, s0));
    fragments[1] = new Fragment(triangle, x + 1, y, convertZ(s1[3]), s1, testInsideTriangle(triangle, s1));
    fragments[2] = new Fragment(triangle, x, y + 1, convertZ(s2[3]), s2, testInsideTriangle(triangle, s2));
    fragments[3] = new Fragment(triangle, x + 1, y + 1, convertZ(s3[3]), s3, testInsideTriangle(triangle, s3));

    /*  Search for the next triangle with fragments in the tile.  */
    for(i = nextTriangle + 1; (i < numTriangles) && (!inside[i]); i++);

    /*  If there is another triangle with fragments update the pointer in the tile.  */
    if (i < numTriangles)
    {
        /*  Set next triangle.  */
        tile->setNextTriangle(i);
    }

    /*  Return the identifier of the triangle that generated this stamp.  */
    genTriID = nextTriangle;

    /*  Return if the tile will produce fragments for another triangle.  */
    return (i < numTriangles);
}


/*  Generates the fragments of a stamp level tile.  */
Fragment **RasterizerEmulator::generateStamp(Tile *tile)
{
    Fragment **fragments;

    /*  Allocate fragment pointer array.  */
    fragments = new Fragment*[4];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (fragments == NULL)
            panic("RasterizerEmulator", "generateStamp", "Error allocating fragment pointer array.");
    )

    /*  Generate the fragments.  */
    generateStamp(tile, fragments);

    return fragments;
}

/*  Calculates the stamp unit to which the fragment will be assigned.  */
TileIdentifier RasterizerEmulator::calculateTileID(Fragment *fr)
{
    s32bit scanX;
    s32bit scanY;

    /*  Calculate the fragment scan tile coordinates.  */
    scanX = fr->getX() / scanTileWidth;
    scanY = fr->getY() / scanTileHeight;

    return TileIdentifier(scanX, scanY);
}

/*************************************************************************
 *
 *  Functions no longer supported.  May work or not.
 *
 *  They are here for reference and historical purposes.
 *
 *************************************************************************/

/*  Scan a pixel to the left.  */
bool RasterizerEmulator::scanLeft(f64bit *edgeC, f64bit a1, f64bit a2, f64bit a3,
    f64bit az, f64bit *offset, s32bit x)
{
    f64bit leftC[4];
    bool continueToLeft;

    /*  Calculate edge and z interpolation equation values one pixel to the
        left from the current position.  */
    leftC[0] = edgeC[0] - a1;
    leftC[1] = edgeC[1] - a2;
    leftC[2] = edgeC[2] - a3;
    leftC[3] = edgeC[3] - az;

    /*  Update horizontal position.  */
    x--;

    /*  Determine if we must continue rasterizing to the left.  */
    continueToLeft =
        GPU_IS_POSITIVE(leftC[0] + offset[0]) &&
        GPU_IS_POSITIVE(leftC[1] + offset[1]) &&
        GPU_IS_POSITIVE(leftC[2] + offset[2]) &&
        (x >= x0);

    /*  If continue rasterization to the left store new edge and z interpolation
        equation values and return.  */
    if (continueToLeft)
    {
        edgeC[0] = leftC[0];
        edgeC[1] = leftC[1];
        edgeC[2] = leftC[2];
        edgeC[3] = leftC[3];
    }

    return continueToLeft;
}

/*  Scan a pixel to the right.  */
bool RasterizerEmulator::scanRight(f64bit *edgeC, f64bit a1, f64bit a2, f64bit a3,
    f64bit az, f64bit *offset, s32bit x)
{
    f64bit rightC[4];
    bool continueToRight;

    /*  Calculate edge and z interpolation equation values one pixel to the
        right from the current position.  */
    rightC[0] = edgeC[0] + a1;
    rightC[1] = edgeC[1] + a2;
    rightC[2] = edgeC[2] + a3;
    rightC[3] = edgeC[3] + az;

    /*  Update horizontal position.  */
    x++;

    /*  Determine if we must continue rasterizing to the right.  */
    continueToRight =
        GPU_IS_POSITIVE(rightC[0] + offset[0]) &&
        GPU_IS_POSITIVE(rightC[1] + offset[1]) &&
        GPU_IS_POSITIVE(rightC[2] + offset[2]) &&
        (x < (x0 + (s32bit) w));

    /*  If continue rasterization to the right store new edge and z interpolation
        equation values and return.  */
    if (continueToRight)
    {
        edgeC[0] = rightC[0];
        edgeC[1] = rightC[1];
        edgeC[2] = rightC[2];
        edgeC[3] = rightC[3];
    }

    return continueToRight;
}

/*  Scan one pixel down.  */
bool RasterizerEmulator::scanDown(f64bit *edgeC, f64bit b1, f64bit b2, f64bit b3,
    f64bit bz, f64bit *offset, s32bit y)
{
    f64bit downC[4];
    bool continueDown;

    /*  Calculate edge and z interpolation equation values one pixel down from
        the current position.  */
    downC[0] = edgeC[0] - b1;
    downC[1] = edgeC[1] - b2;
    downC[2] = edgeC[2] - b3;
    downC[3] = edgeC[3] - bz;

    /*  Update vertical position.  */
    y--;

    /*  Evaluate if down pixel must be scanned. */
    continueDown =
        GPU_IS_POSITIVE(downC[0] + offset[0]) &&
        GPU_IS_POSITIVE(downC[1] + offset[1]) &&
        GPU_IS_POSITIVE(downC[2] + offset[2]) &&
        (y >= y0);

    /*  If continue rasterization down store new edge and z interpolation
        equation values and return.  */
    if (continueDown)
    {
        edgeC[0] = downC[0];
        edgeC[1] = downC[1];
        edgeC[2] = downC[2];
        edgeC[3] = downC[3];
    }

    return continueDown;
}

/*  Scan one pixel up.  */
bool RasterizerEmulator::scanUp(f64bit *edgeC, f64bit b1, f64bit b2, f64bit b3,
    f64bit bz, f64bit *offset, s32bit y)
{
    f64bit upC[4];
    bool continueUp;

    /*  Calculate edge and z interpolation equation values one pixel down from
        the current position.  */
    upC[0] = edgeC[0] + b1;
    upC[1] = edgeC[1] + b2;
    upC[2] = edgeC[2] + b3;
    upC[3] = edgeC[3] + bz;

    /*  Update vertical position.  */
    y++;

    /*  Evaluate if up pixel must be scanned.  */
    continueUp = GPU_IS_POSITIVE(upC[0] + offset[0]) &&
        GPU_IS_POSITIVE(upC[1] + offset[1]) &&
        GPU_IS_POSITIVE(upC[2] + offset[2]) &&
        (y < (y0 + (s32bit) h));

    /*  If continue rasterization up store new edge and z interpolation equation
        values and return.  */
    if (continueUp)
    {
        edgeC[0] = upC[0];
        edgeC[1] = upC[1];
        edgeC[2] = upC[2];
        edgeC[3] = upC[3];
    }

    return continueUp;
}

/*  New rasterizer algorithm. */
Fragment *RasterizerEmulator::nextScanlineFragment(u32bit triangleID)
{
    SetupTriangle *triangle;
    f64bit rightSave[4];
    f64bit downSave[4];
    f64bit upSave[4];
    f64bit updatedC[4];
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    Fragment *fr;
    s32bit x, y;
    f64bit offset[3];

    /*  NOTE: UNTIL IT CAN BE TESTED THAT WORKS FOR SINGLE FRAGMENT STAMPS.  */
    GPU_ASSERT(
        if ((scanTileWidth != 1) || (scanTileHeight != 1))
            panic("RasterizerEmulator", "nextScanlineFragment", "Stamp based rasterization not supported.");
    )

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "nextScanlineFragment", "Triangle identifier out of range.");
    )

    /*  Get setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "nextScanlineFragment", "Triangle is not a setup triangle.");
    )

    /*  Get triangle current edge equations.  */
    triangle->getEdgeEquations(edge1, edge2, edge3);

    /*  Get triangle current z interpolation equation.  */
    triangle->getZEquation(zeq);

    /*  Get triangle current raster position.  */
    triangle->getRasterPosition(x, y);

    /*  Initialize next fragment equation values.  */
    updatedC[0] = edge1[2];
    updatedC[1] = edge2[2];
    updatedC[2] = edge3[2];
    updatedC[3] = zeq[2];

    /*  Calculate pixel offset (aprox) for the edge equations.  This is used
        for scaning triangles that are thiner than a pixel.  */
    offset[0] = fabs(edge1[0]) + fabs(edge1[1]);
    offset[1] = fabs(edge2[0]) + fabs(edge2[1]);
    offset[2] = fabs(edge3[0]) + fabs(edge3[1]);

    /*  Create current fragment.  */
    fr = new Fragment(triangle, x, y, convertZ(zeq[2]), updatedC, testInsideTriangle(triangle, updatedC));

    /*  Get rasterization direction.  */
    switch(triangle->getDirection())
    {

        case CENTER_DIR:

            /*  Initialize right save equation values.  */
            rightSave[0] = edge1[2];
            rightSave[1] = edge2[2];
            rightSave[2] = edge3[2];
            rightSave[3] = zeq[2];

            /*  Initialize up save equation values.  */
            upSave[0] = edge1[2];
            upSave[1] = edge2[2];
            upSave[2] = edge3[2];
            upSave[3] = zeq[2];

            /*  First test up and save.  */
            if (scanUp(upSave, edge1[1], edge2[1], edge3[1], zeq[1], offset, y))
            {
                /*  Save down position.  */
                triangle->saveUp(upSave);
            }

            /*  Initialize down save equation values.  */
            downSave[0] = edge1[2];
            downSave[1] = edge2[2];
            downSave[2] = edge3[2];
            downSave[3] = zeq[2];

            /*  Second test down and save.  */
            if (scanDown(downSave, edge1[1], edge2[1], edge3[1], zeq[1],
                offset, y))
            {
                /*  Save down position.  */
                triangle->saveDown(downSave);
            }

            /*  Third test right and save.*/
            if (scanRight(rightSave, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Save right position.  */
                triangle->saveRight(rightSave);
            }

            /*  Fourth, test left.  */
            if (scanLeft(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x - 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(CENTER_LEFT_DIR);
            }
            else
            {
                /*  Check if there is a right saved position.  */
                if (triangle->isRightSaved())
                {
                    /*  Restore position stored in right save.  */
                    triangle->restoreRight();

                    /*  Change rasterization direction.  */
                    triangle->setDirection(CENTER_RIGHT_DIR);
                }
                else
                {
                    /*  Check if there is a up saved position.  */
                    if (triangle->isUpSaved())
                    {
                        /*  Restore position stored in up save.  */
                        triangle->restoreUp();

                        /*  Change drawing direction to up.  */
                        triangle->setDirection(UP_DIR);
                    }
                    else
                    {
                        /*  Check if there is a down saved position.  */
                        if (triangle->isDownSaved())
                        {
                            /*  Restore position stored in down save.  */
                            triangle->restoreDown();

                            /*  Change drawing direction to down.  */
                            triangle->setDirection(DOWN_DIR);
                        }
                        else
                        {
                            /*  Stop rasterization.  */

                            /*  No more fragments in the triangle.  */
                            triangle->lastFragment();

                            /*  Set as last fragment.  */
                            fr->lastFragment();
                        }
                    }
                }
            }

            break;

        case UP_DIR:

            /*  Initialize right save equation values.  */
            rightSave[0] = edge1[2];
            rightSave[1] = edge2[2];
            rightSave[2] = edge3[2];
            rightSave[3] = zeq[2];

            /*  Initialize up save equation values.  */
            upSave[0] = edge1[2];
            upSave[1] = edge2[2];
            upSave[2] = edge3[2];
            upSave[3] = zeq[2];

            /*  First test up and save.  */
            if (scanUp(upSave, edge1[1], edge2[1], edge3[1], zeq[1],
                offset, y))
            {
                /*  Save down position.  */
                triangle->saveUp(upSave);
            }

            /*  Second test right and save.*/
            if (scanRight(rightSave, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Save right position.  */
                triangle->saveRight(rightSave);
            }

            /*  Third, test left.  */
            if (scanLeft(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x - 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(UP_LEFT_DIR);
            }
            else
            {
                /*  Check if there is a right saved position.  */
                if (triangle->isRightSaved())
                {
                    /*  Restore position stored in right save.  */
                    triangle->restoreRight();

                    /*  Change rasterization direction.  */
                    triangle->setDirection(UP_RIGHT_DIR);
                }
                else
                {
                    /*  Check if there is a up saved position.  */
                    if (triangle->isUpSaved())
                    {
                        /*  Restore position stored in up save.  */
                        triangle->restoreUp();

                        /*  Change drawing direction to up.  */
                        triangle->setDirection(UP_DIR);
                    }
                    else
                    {
                        /*  Check if there is a down saved position.  */
                        if (triangle->isDownSaved())
                        {
                            /*  Restore position stored in down save.  */
                            triangle->restoreDown();

                            /*  Change drawing direction to down.  */
                            triangle->setDirection(DOWN_DIR);
                        }
                        else
                        {
                            /*  Stop rasterization.  */

                            /*  No more fragments in the triangle.  */
                            triangle->lastFragment();

                            /*  Set as last fragment.  */
                            fr->lastFragment();
                        }
                    }
                }
            }

            break;

        case DOWN_DIR:

            /*  Initialize right save equation values.  */
            rightSave[0] = edge1[2];
            rightSave[1] = edge2[2];
            rightSave[2] = edge3[2];
            rightSave[3] = zeq[2];

            /*  Initialize down save equation values.  */
            downSave[0] = edge1[2];
            downSave[1] = edge2[2];
            downSave[2] = edge3[2];
            downSave[3] = zeq[2];

            /*  First test down and save.  */
            if (scanDown(downSave, edge1[1], edge2[1], edge3[1], zeq[1],
                offset, y))
            {
                /*  Save down position.  */
                triangle->saveDown(downSave);
            }

            /*  Second test right and save.*/
            if (scanRight(rightSave, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Save right position.  */
                triangle->saveRight(rightSave);
            }

            /*  Third, test left.  */
            if (scanLeft(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x - 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(DOWN_LEFT_DIR);
            }
            else
            {
                /*  Check if there is a right saved position.  */
                if (triangle->isRightSaved())
                {
                    /*  Restore position stored in right save.  */
                    triangle->restoreRight();

                    /*  Change rasterization direction.  */
                    triangle->setDirection(DOWN_RIGHT_DIR);
                }
                else
                {
                    /*  Check if there is a down saved position.  */
                    if (triangle->isDownSaved())
                    {
                        /*  Restore position stored in down save.  */
                        triangle->restoreDown();

                        /*  Change drawing direction to down.  */
                        triangle->setDirection(DOWN_DIR);
                    }
                    else
                    {
                        /*  Stop rasterization.  */

                        /*  No more fragments in the triangle.  */
                        triangle->lastFragment();

                        /*  Set as last fragment.  */
                        fr->lastFragment();
                    }
                }
            }

            break;

        case DOWN_LEFT_DIR:

            /*  Initialize down save equation values.  */
            downSave[0] = edge1[2];
            downSave[1] = edge2[2];
            downSave[2] = edge3[2];
            downSave[3] = zeq[2];

            /*  First test down and save.  */
            if (!triangle->isDownSaved())
            {
                if (scanDown(downSave, edge1[1], edge2[1], edge3[1], zeq[1],
                    offset, y))
                {
                    /*  Save down position.  */
                    triangle->saveDown(downSave);
                }
            }

            /*  Second, test left.  */
            if (scanLeft(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x - 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(DOWN_LEFT_DIR);
            }
            else
            {
                /*  Check if there is a right saved position.  */
                if (triangle->isRightSaved())
                {
                    /*  Restore position stored in right save.  */
                    triangle->restoreRight();

                    /*  Change rasterization direction.  */
                    triangle->setDirection(DOWN_RIGHT_DIR);
                }
                else
                {
                    /*  Check if there is a down saved position.  */
                    if (triangle->isDownSaved())
                    {
                        /*  Restore position stored in down save.  */
                        triangle->restoreDown();

                        /*  Change drawing direction to down.  */
                        triangle->setDirection(DOWN_DIR);
                    }
                    else
                    {
                        /*  Stop rasterization.  */

                        /*  No more fragments in the triangle.  */
                        triangle->lastFragment();

                        /*  Set as last fragment.  */
                        fr->lastFragment();
                    }
                }
            }

            break;

        case DOWN_RIGHT_DIR:

            /*  Initialize down save equation values.  */
            downSave[0] = edge1[2];
            downSave[1] = edge2[2];
            downSave[2] = edge3[2];
            downSave[3] = zeq[2];

            /*  First test down and save.  */
            if (!triangle->isDownSaved())
            {
                if (scanDown(downSave, edge1[1], edge2[1], edge3[1], zeq[1],
                    offset, y))
                {
                    /*  Save down position.  */
                    triangle->saveDown(downSave);
                }
            }

            /*  Second, test right.  */
            if (scanRight(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x + 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(DOWN_RIGHT_DIR);
            }
            else
            {
                /*  Check if there is a down saved position.  */
                if (triangle->isDownSaved())
                {
                    /*  Restore position stored in down save.  */
                    triangle->restoreDown();

                    /*  Change drawing direction to down.  */
                    triangle->setDirection(DOWN_DIR);
                }
                else
                {
                    /*  Stop rasterization.  */

                    /*  No more fragments in the triangle.  */
                    triangle->lastFragment();

                    /*  Set as last fragment.  */
                    fr->lastFragment();
                }
            }

            break;

        case UP_LEFT_DIR:

            /*  Initialize up save equation values.  */
            upSave[0] = edge1[2];
            upSave[1] = edge2[2];
            upSave[2] = edge3[2];
            upSave[3] = zeq[2];

            /*  First test up and save.  */
            if (!triangle->isUpSaved())
            {
                if (scanUp(upSave, edge1[1], edge2[1], edge3[1], zeq[1],
                    offset, y))
                {
                    /*  Save up position.  */
                    triangle->saveUp(upSave);
                }
            }

            /*  Second, test left.  */
            if (scanLeft(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x - 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(UP_LEFT_DIR);
            }
            else
            {
                /*  Check if there is a right saved position.  */
                if (triangle->isRightSaved())
                {
                    /*  Restore position stored in right save.  */
                    triangle->restoreRight();

                    /*  Change rasterization direction.  */
                    triangle->setDirection(UP_RIGHT_DIR);
                }
                else
                {
                    /*  Check if there is a up saved position.  */
                    if (triangle->isUpSaved())
                    {
                        /*  Restore position stored in up save.  */
                        triangle->restoreUp();

                        /*  Change drawing direction to up.  */
                        triangle->setDirection(UP_DIR);
                    }
                    else
                    {
                        /*  Check if there is a down saved position.  */
                        if (triangle->isDownSaved())
                        {
                            /*  Restore position stored in down save.  */
                            triangle->restoreDown();

                            /*  Change drawing direction to down.  */
                            triangle->setDirection(DOWN_DIR);
                        }
                        else
                        {
                            /*  Stop rasterization.  */

                            /*  No more fragments in the triangle.  */
                            triangle->lastFragment();

                            /*  Set as last fragment.  */
                            fr->lastFragment();
                        }
                    }
                }
            }

            break;

        case UP_RIGHT_DIR:

            /*  Initialize up save equation values.  */
            upSave[0] = edge1[2];
            upSave[1] = edge2[2];
            upSave[2] = edge3[2];
            upSave[3] = zeq[2];

            /*  First test up and save.  */
            if (!triangle->isUpSaved())
            {
                if (scanUp(upSave, edge1[1], edge2[1], edge3[1], zeq[1],
                    offset, y))
                {
                    /*  Save up position.  */
                    triangle->saveUp(upSave);
                }
            }

            /*  Second, test right.  */
            if (scanRight(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x + 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(UP_RIGHT_DIR);
            }
            else
            {
                /*  Check if there is a up saved position.  */
                if (triangle->isUpSaved())
                {
                    /*  Restore position stored in up save.  */
                    triangle->restoreUp();

                    /*  Change drawing direction to up.  */
                    triangle->setDirection(UP_DIR);
                }
                else
                {
                    /*  Check if there is a down saved position.  */
                    if (triangle->isDownSaved())
                    {
                        /*  Restore position stored in down save.  */
                        triangle->restoreDown();

                        /*  Change drawing direction to down.  */
                        triangle->setDirection(DOWN_DIR);
                    }
                    else
                    {
                        /*  Stop rasterization.  */

                        /*  No more fragments in the triangle.  */
                        triangle->lastFragment();

                        /*  Set as last fragment.  */
                        fr->lastFragment();
                    }
                }
            }

            break;

        case CENTER_LEFT_DIR:

            /*  Initialize up save equation values.  */
            upSave[0] = edge1[2];
            upSave[1] = edge2[2];
            upSave[2] = edge3[2];
            upSave[3] = zeq[2];

            /*  First test up and save.  */
            if (!triangle->isUpSaved())
            {
                if (scanUp(upSave, edge1[1], edge2[1], edge3[1], zeq[1],
                    offset, y))
                {
                    /*  Save up position.  */
                    triangle->saveUp(upSave);
                }
            }

            /*  Initialize down save equation values.  */
            downSave[0] = edge1[2];
            downSave[1] = edge2[2];
            downSave[2] = edge3[2];
            downSave[3] = zeq[2];

            /*  Second test down and save.  */
            if (!triangle->isDownSaved())
            {
                if (scanDown(downSave, edge1[1], edge2[1], edge3[1], zeq[1],
                    offset, y))
                {
                    /*  Save down position.  */
                    triangle->saveDown(downSave);
                }
            }

            /*  Third, test left.  */
            if (scanLeft(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x - 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(CENTER_LEFT_DIR);
            }
            else
            {
                /*  Check if there is a right saved position.  */
                if (triangle->isRightSaved())
                {
                    /*  Restore position stored in right save.  */
                    triangle->restoreRight();

                    /*  Change rasterization direction.  */
                    triangle->setDirection(CENTER_RIGHT_DIR);
                }
                else
                {
                    /*  Check if there is a up saved position.  */
                    if (triangle->isUpSaved())
                    {
                        /*  Restore position stored in up save.  */
                        triangle->restoreUp();

                        /*  Change drawing direction to up.  */
                        triangle->setDirection(UP_DIR);
                    }
                    else
                    {

                        /*  Check if there is a down saved position.  */
                        if (triangle->isDownSaved())
                        {
                            /*  Restore position stored in down save.  */
                            triangle->restoreDown();

                            /*  Change drawing direction to down.  */
                            triangle->setDirection(DOWN_DIR);
                        }
                        else
                        {
                            /*  Stop rasterization.  */

                            /*  No more fragments in the triangle.  */
                            triangle->lastFragment();

                            /*  Set as last fragment.  */
                            fr->lastFragment();
                        }
                    }
                }
            }
            break;

        case CENTER_RIGHT_DIR:

            /*  Initialize up save equation values.  */
            upSave[0] = edge1[2];
            upSave[1] = edge2[2];
            upSave[2] = edge3[2];
            upSave[3] = zeq[2];

            /*  First test up and save.  */
            if (!triangle->isUpSaved())
            {
                if (scanUp(upSave, edge1[1], edge2[1], edge3[1], zeq[1],
                    offset, y))
                {
                    /*  Save up position.  */
                    triangle->saveUp(upSave);
                }
            }

            /*  Initialize down save equation values.  */
            downSave[0] = edge1[2];
            downSave[1] = edge2[2];
            downSave[2] = edge3[2];
            downSave[3] = zeq[2];

            /*  Second test down and save.  */
            if (!triangle->isDownSaved())
            {
                if (scanDown(downSave, edge1[1], edge2[1], edge3[1], zeq[1],
                    offset, y))
                {
                    /*  Save down position.  */
                    triangle->saveDown(downSave);
                }
            }

            /*  Third, test right.  */
            if (scanRight(updatedC, edge1[0], edge2[0], edge3[0], zeq[0],
                offset, x))
            {
                /*  Store raster position state for next fragment.  */
                triangle->updatePosition(updatedC, x + 1, y);

                /*  Change rasterization direction to the left.  */
                triangle->setDirection(CENTER_RIGHT_DIR);
            }
            else
            {
                /*  Check if there is a up saved position.  */
                if (triangle->isUpSaved())
                {
                    /*  Restore position stored in up save.  */
                    triangle->restoreUp();

                    /*  Change drawing direction to up.  */
                    triangle->setDirection(UP_DIR);
                }
                else
                {
                    /*  Check if there is a down saved position.  */
                    if (triangle->isDownSaved())
                    {
                        /*  Restore position stored in down save.  */
                        triangle->restoreDown();

                        /*  Change drawing direction to down.  */
                        triangle->setDirection(DOWN_DIR);
                    }
                    else
                    {
                        /*  Stop rasterization.  */

                        /*  No more fragments in the triangle.  */
                        triangle->lastFragment();

                        /*  Set as last fragment.  */
                        fr->lastFragment();
                    }
                }
            }
            break;

        default:
            panic("RasterizerEmulator", "nextScanlineFragmentNew", "Invalid scan direction.");
            break;
    }

    return fr;
}

/*  Returns the next fragment stamp using tiled scanline based rasterization.  */
Fragment **RasterizerEmulator::nextScanlineStampTiledOld(u32bit triangleID)
{
    SetupTriangle *triangle;
    f64bit stampPos[4];
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    s32bit x, y;
    f64bit offset[3];
    Fragment **stamp;
    Fragment *fr;
    bool firstStamp;
    u32bit i, j;
    s32bit leftBound, rightBound, bottomBound, topBound;

    /*  Check it is a correct triangle ID.  */
    GPU_ASSERT(
        if (triangleID >= activeTriangles)
            panic("RasterizerEmulator", "nextScanlineStampTiled", "Triangle identifier out of range.");
    )

    /*  Get setup triangle.  */
    triangle = setupTriangles[triangleID];

    /*  Check the triangle id points to a setup triangle.  */
    GPU_ASSERT(
        if (triangle == NULL)
            panic("RasterizerEmulator", "nextScanlineStampTiled", "Triangle is not a setup triangle.");
    )

    /*  Allocate the stamp.  */
    stamp = new Fragment*[scanTileWidth * scanTileHeight];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (stamp == NULL)
            panic("RasterizerEmulator", "nextScanlineStampTiled", "Error allocating the stamp.");
    )

    /*  Get triangle current edge equations.  */
    triangle->getEdgeEquations(edge1, edge2, edge3);

    /*  Get triangle current z interpolation equation.  */
    triangle->getZEquation(zeq);

    /*  Get triangle current raster position.  */
    triangle->getRasterPosition(x, y);

    /*  Get triangle bounding box.  */
    triangle->getBoundingBox(leftBound, bottomBound, rightBound, topBound);

    /*  Get if it's the first stamp of the triangle to probe.  */
    firstStamp = triangle->isFirstStamp();

    /*  Set as next stamp as not first.  */
    triangle->setFirstStamp(FALSE);

    /*  Calculate pixel offset (aprox) for the edge equations.  This is used
        for scaning triangles that are thiner than a pixel.  */
    //offset[0] = GPU_MAX(fabs(edge1[0]), fabs(edge1[1]));
    //offset[1] = GPU_MAX(fabs(edge2[0]), fabs(edge2[1]));
    //offset[2] = GPU_MAX(fabs(edge3[0]), fabs(edge3[1]));
    offset[0] = 0.0f;
    offset[1] = 0.0f;
    offset[2] = 0.0f;

    stampPos[0] = edge1[2];
    stampPos[1] = edge2[2];
    stampPos[2] = edge3[2];
    stampPos[3] = zeq[2];

    /*  Create current stamp.  */
    for(i = 0; i < scanTileHeight; i++)
    {
        for(j = 0; j < scanTileWidth; j++)
        {
            stamp[i * scanTileWidth + j] = new Fragment(triangle, x + j, y + i,
                convertZ(stampPos[3]), stampPos, testInsideTriangle(triangle, stampPos));

            /*  Update stamp position.  */
            stampPos[0] += edge1[0];
            stampPos[1] += edge2[0];
            stampPos[2] += edge3[0];
            stampPos[3] += zeq[0];
        }

        /*  Update stamp position.  */
        stampPos[0] += edge1[1] - scanTileWidth * edge1[0];
        stampPos[1] += edge2[1] - scanTileWidth * edge2[0];
        stampPos[2] += edge3[1] - scanTileWidth * edge3[0];
        stampPos[3] += zeq[1] - scanTileWidth * zeq[0];
    }

    /*  Used to mark the last fragment.  */
    fr = stamp[3];

    /*  Get rasterization direction.  */
    switch(triangle->getDirection())
    {

        case CENTER_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, topBound, triangle, firstStamp);

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle, firstStamp);

            /*  Save the stamp position right from the current.  */
            saveRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle, firstStamp);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle, firstStamp))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRUD(triangle, fr);
            }

            break;

        case UP_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, topBound, triangle, firstStamp);

            /*  Save the stamp position right from the current.  */
            saveRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle, firstStamp);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle, firstStamp))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRUD(triangle, fr);
            }

            break;

        case DOWN_DIR:

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle, firstStamp);

            /*  Save the stamp position right from the current.  */
            saveRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle, firstStamp);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRD(triangle, fr);
            }

            break;

        case DOWN_LEFT_DIR:

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRD(triangle, fr);
            }

            break;

        case DOWN_RIGHT_DIR:

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);

            /*  Moves or saves right stamp position.  Returns if continue
                rasterizing to the right.  */
            if (!rasterizeRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileD(triangle, fr);
            }

            break;

        case UP_LEFT_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, topBound, triangle);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRUD(triangle, fr);
            }

            break;

        case UP_RIGHT_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, topBound, triangle);

            /*  Moves or saves right stamp position.  Returns if continue
                rasterizing to the right.  */
            if (!rasterizeRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileUD(triangle, fr);
            }

            break;

        case CENTER_LEFT_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, topBound, triangle);

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);

            /*  Moves or saves left stamp position.  Returns if continue
                rasterizing to the left.  */
            if (!rasterizeLeft(edge1, edge2, edge3, zeq, offset, x, y, leftBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileRUD(triangle, fr);
            }

            break;

        case CENTER_RIGHT_DIR:

            /*  Save the stamp position over the current.  */
            saveUp(edge1, edge2, edge3, zeq, offset, x, y, topBound, triangle);

            /*  Save the stamp position below the current.  */
            saveDown(edge1, edge2, edge3, zeq, offset, x, y, bottomBound, triangle);

            /*  Moves or saves right stamp position.  Returns if continue
                rasterizing to the right.  */
            if (!rasterizeRight(edge1, edge2, edge3, zeq, offset, x, y, rightBound, triangle))
            {
                /*  Select new stamp to rasterize.  */
                nextTileUD(triangle, fr);
            }

            break;

        default:
            panic("RasterizerEmulator", "nextScanlineStampTiled", "Invalid scan direction.");
            break;
    }

    return stamp;
}


#define INSIDE_EQUATION(e, a, b) ((GPU_IS_POSITIVE(e) && !GPU_IS_ZERO(e)) || (GPU_IS_ZERO(e) &&\
((a > 0.0f) || ((a == 0.0f) && (b >= 0.0f)))))

//  Test if the sample is inside of the triangle and the clip space (near/far).
bool RasterizerEmulator::testInsideTriangle(SetupTriangle *triangle, f64bit *coordinates)
{
    f64bit edge1[3], edge2[3], edge3[3];
    bool inside;

    triangle->getEdgeEquations(edge1, edge2, edge3);

    //  Rules to avoid multiple generation of fragments at the edges of adjoint triangles:
    //
    //  - if the coordinate is positive and outside the zero region it is considered inside for
    //    that edge equation.
    //  - if the coordinate is in the zero region and the edge equation first coeficient is
    //    positive and outside the zero region it is considered inside for that edge equation.
    //  - if the coordinate is in the zero region and the edge equation first coefficient is
    //    inside the zero region and the edge equation second coefficient is positive it is
    //    considered inside for the edge equation.
    //  - otherwise it is considered outside for the edge equation.
    //

    inside = INSIDE_EQUATION(coordinates[0], edge1[0], edge1[1]) &&
        INSIDE_EQUATION(coordinates[1], edge2[0], edge2[1]) &&
        INSIDE_EQUATION(coordinates[2], edge3[0], edge3[1]) &&
        (d3d9DepthRange ? (GPU_IS_POSITIVE(coordinates[3]) && GPU_IS_LESS_EQUAL(coordinates[3], f64bit(1))) :
                          GPU_IS_LESS_EQUAL(GPU_ABS(coordinates[3]), f64bit(1)));

    return inside;
}

//  Computes

//  Function that computes the fragment Z samples for MSAA
void RasterizerEmulator::computeMSAASamples(Fragment *fr, u32bit samples)
{
    f64bit edge1[3];
    f64bit edge2[3];
    f64bit edge3[3];
    f64bit zeq[3];
    f64bit sample[4];
    f64bit centroid[4];
    f64bit *fragCoord;
    f64bit zw;
    SetupTriangle *triangle;
    bool coverage[MAX_MSAA_SAMPLES];
    u32bit z[MAX_MSAA_SAMPLES];
    bool anySampleInsideTriangle = false;
    f32bit centroidSamples;

    //  Get the triangle associated with the fragment.
    triangle = fr->getTriangle();

    //  Get triangle current edge equations.
    triangle->getEdgeEquations(edge1, edge2, edge3);

    //  Get triangle current z interpolation equation.
    triangle->getZEquation(zeq);

    //  Get fragment values for the edge equations and the z/w interpolation equation.
    fragCoord = fr->getCoordinates();
    zw = fr->getZW();

/*if ((fr->getX() == 204) && (fr->getY() == 312))
{
printf("computeMSAASamples:\n");
QuadFloat *vpos = triangle->getAttribute(0, 0);
printf("vpos1 = %f %f %f %f\n", (*vpos)[0], (*vpos)[1], (*vpos)[2], (*vpos)[3]);
vpos = triangle->getAttribute(1, 0);
printf("vpos2 = %f %f %f %f\n", (*vpos)[0], (*vpos)[1], (*vpos)[2], (*vpos)[3]);
vpos = triangle->getAttribute(2, 0);
printf("vpos3 = %f %f %f %f\n", (*vpos)[0], (*vpos)[1], (*vpos)[2], (*vpos)[3]);
printf("edge1 = %f %f %f\n", edge1[0], edge1[1], edge1[2]);
printf("edge2 = %f %f %f\n", edge2[0], edge2[1], edge2[2]);
printf("edge3 = %f %f %f\n", edge3[0], edge3[1], edge3[2]);
printf("zeq = %f %f %f\n", zeq[0], zeq[1], zeq[2]);
printf("current = %f %f %f %f\n", fragCoord[0], fragCoord[1], fragCoord[2], zw);
}*/

    //  Initilize centroid sampling computation.
    centroid[0] = 0.0f;
    centroid[1] = 0.0f;
    centroid[2] = 0.0f;
    centroid[3] = 0.0f;
    centroidSamples = 0.0f;

    switch(samples)
    {
        case 2:
            for(u32bit i = 0; i < samples; i++)
            {
                //  Compute the edge and z equations for the current fragment sample.
                sample[0] = fragCoord[0] + edge1[0] * (MSAAPatternTable2[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge1[1] * (MSAAPatternTable2[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[1] = fragCoord[1] + edge2[0] * (MSAAPatternTable2[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge2[1] * (MSAAPatternTable2[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[2] = fragCoord[2] + edge3[0] * (MSAAPatternTable2[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge3[1] * (MSAAPatternTable2[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[3] = zw + zeq[0] * (MSAAPatternTable2[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            zeq[1] * (MSAAPatternTable2[i].yOff / MSAA_SUBPIXEL_PRECISSION);

                //  Convert the computed z for the sample from 64-bit fp to 24 bit integer.
                z[i] = convertZ(sample[3]);

                //  Determine if the sample is inside the triangle.
                //coverage[i] = INSIDE_EQUATION(sample[0], edge1[0], edge1[1]) &&
                //    INSIDE_EQUATION(sample[1], edge2[0], edge2[1]) &&
                //    INSIDE_EQUATION(sample[2], edge3[0], edge3[1]) &&
                //    GPU_IS_LESS_EQUAL(GPU_ABS(sample[3]), f64bit(1));
                coverage[i] = testInsideTriangle(triangle, sample);

                //  Update centroid sample computation.
                if (coverage[i])
                {
                    centroid[0] += sample[0];
                    centroid[1] += sample[1];
                    centroid[2] += sample[2];
                    centroid[3] += sample[3];
                    centroidSamples++;
                }

                //  Compute if any of the fragment samples is inside the triangle.
                anySampleInsideTriangle = anySampleInsideTriangle || coverage[i];
            }

            break;

        case 4:

            for(u32bit i = 0; i < samples; i++)
            {
                //  Compute the edge and z equations for the current fragment sample.
                sample[0] = fragCoord[0] + edge1[0] * (MSAAPatternTable4[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge1[1] * (MSAAPatternTable4[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[1] = fragCoord[1] + edge2[0] * (MSAAPatternTable4[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge2[1] * (MSAAPatternTable4[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[2] = fragCoord[2] + edge3[0] * (MSAAPatternTable4[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge3[1] * (MSAAPatternTable4[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[3] = zw + zeq[0] * (MSAAPatternTable4[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            zeq[1] * (MSAAPatternTable4[i].yOff / MSAA_SUBPIXEL_PRECISSION);

                //  Convert the computed z for the sample from 64-bit fp to 24 bit integer.
                z[i] = convertZ(sample[3]);

                //  Determine if the sample is inside the triangle.
                //coverage[i] = INSIDE_EQUATION(sample[0], edge1[0], edge1[1]) &&
                //    INSIDE_EQUATION(sample[1], edge2[0], edge2[1]) &&
                //    INSIDE_EQUATION(sample[2], edge3[0], edge3[1]) &&
                //    (GPU_ABS(sample[3]) <= 1);
                coverage[i] = testInsideTriangle(triangle, sample);

                //  Update centroid sample computation.
                if (coverage[i])
                {
                    centroid[0] += sample[0];
                    centroid[1] += sample[1];
                    centroid[2] += sample[2];
                    centroid[3] += sample[3];
                    centroidSamples++;
                }

                //  Compute if any of the fragment samples is inside the triangle.
                anySampleInsideTriangle = anySampleInsideTriangle || coverage[i];
//if ((fr->getX() == 204) && (fr->getY() == 312))
//{
//printf("sample = %d -> %f %f %f %f -> coverage = %d | anySampleInsideTriangle = %d\n",
//i, sample[0], sample[1], sample[2], sample[3], coverage[i], anySampleInsideTriangle);
//}
            }

            break;

        case 6:

            for(u32bit i = 0; i < samples; i++)
            {
                //  Compute if any of the fragment samples is inside the triangle.
                sample[0] = fragCoord[0] + edge1[0] * (MSAAPatternTable6[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge1[1] * (MSAAPatternTable6[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[1] = fragCoord[1] + edge2[0] * (MSAAPatternTable6[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge2[1] * (MSAAPatternTable6[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[2] = fragCoord[2] + edge3[0] * (MSAAPatternTable6[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge3[1] * (MSAAPatternTable6[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[3] = zw + zeq[0] * (MSAAPatternTable6[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            zeq[1] * (MSAAPatternTable6[i].yOff / MSAA_SUBPIXEL_PRECISSION);

                //  Convert the computed z for the sample from 64-bit fp to 24 bit integer.
                z[i] = convertZ(sample[3]);

                //  Determine if the sample is inside the triangle.
                //coverage[i] = INSIDE_EQUATION(sample[0], edge1[0], edge1[1]) &&
                //    INSIDE_EQUATION(sample[1], edge2[0], edge2[1]) &&
                //    INSIDE_EQUATION(sample[2], edge3[0], edge3[1]) &&
                //    (GPU_ABS(sample[3]) <= 1);
                coverage[i] = testInsideTriangle(triangle, sample);

                //  Update centroid sample computation.
                if (coverage[i])
                {
                    centroid[0] += sample[0];
                    centroid[1] += sample[1];
                    centroid[2] += sample[2];
                    centroid[3] += sample[3];
                    centroidSamples++;
                }

                //  Compute if any of the fragment samples is inside the triangle.
                anySampleInsideTriangle = anySampleInsideTriangle || coverage[i];
            }

            break;

        case 8:

            for(u32bit i = 0; i < samples; i++)
            {
                //  Compute if any of the fragment samples is inside the triangle.
                sample[0] = fragCoord[0] + edge1[0] * (MSAAPatternTable8[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge1[1] * (MSAAPatternTable8[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[1] = fragCoord[1] + edge2[0] * (MSAAPatternTable8[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge2[1] * (MSAAPatternTable8[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[2] = fragCoord[2] + edge3[0] * (MSAAPatternTable8[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            edge3[1] * (MSAAPatternTable8[i].yOff / MSAA_SUBPIXEL_PRECISSION);
                sample[3] = zw + zeq[0] * (MSAAPatternTable8[i].xOff / MSAA_SUBPIXEL_PRECISSION) +
                            zeq[1] * (MSAAPatternTable8[i].yOff / MSAA_SUBPIXEL_PRECISSION);

                //  Convert the computed z for the sample from 64-bit fp to 24 bit integer.
                z[i] = convertZ(sample[3]);

                //  Determine if the sample is inside the triangle.
                //coverage[i] = INSIDE_EQUATION(sample[0], edge1[0], edge1[1]) &&
                //    INSIDE_EQUATION(sample[1], edge2[0], edge2[1]) &&
                //    INSIDE_EQUATION(sample[2], edge3[0], edge3[1]) &&
                //    (GPU_ABS(sample[3]) <= 1);
                coverage[i] = testInsideTriangle(triangle, sample);

                //  Update centroid sample computation.
                if (coverage[i])
                {
                    centroid[0] += sample[0];
                    centroid[1] += sample[1];
                    centroid[2] += sample[2];
                    centroid[3] += sample[3];
                    centroidSamples++;
                }

                //  Compute if any of the fragment samples is inside the triangle.
                anySampleInsideTriangle = anySampleInsideTriangle || coverage[i];
            }

            break;

        default:
            panic("RasterizerEmulator", "computeMSAASamples", "Unsupported MSAA mode.");
            break;
    }

    //  Finish centroid sampling computation.
    if (anySampleInsideTriangle)
    {
        centroid[0] = centroid[0] / centroidSamples;
        centroid[1] = centroid[1] / centroidSamples;
        centroid[2] = centroid[2] / centroidSamples;
        centroid[3] = centroid[3] / centroidSamples;
    }

    //  Set fragment MSAA data.
    fr->setMSAASamples(samples, z, coverage, centroid, anySampleInsideTriangle);
}


/*************************************************************************
 *
 *  Non functions.  This are pieces of code that could be used as reference
 *  for other things.  They aren't supposed to work or compile.
 *
 *************************************************************************/


/*  !! TO BE USED ONLY AS A REFERENCE FOR USING A PROGRAMMABLE UNIT
    (SIMILAR TO A VERTEX SHADER) FOR RASTERIZATION MATRIX CALCULATION!!  */
void oldSetupMatrix()
{
    /*
    f32bit X1[3], X2[3], Y1[3], Y2[3], W1[3], W2[3];
    QuadFloat *vAttr1, *vAttr2, *vAttr3;
    f32bit A[3], B[3], C[3];
    f32bit auxA[3], auxB[3], auxC[3];

      Create the X, Y and W vectors from three vertex coordinates.
        Vertex position MUST be stored in the first vertex attribute.
        Vertex coordinates must be unprojected clip space coordinates
        for 2D homogeneous rasterization.  */

    /*  X, Y and W vectors use two different swizzles: yzx and zxy.
        Create both. */

    /*  Get the three vertex x coordinates.  Use yzx swizzle.  *
    X1[2] = vAttr1[0][0];
    X1[0] = vAttr2[0][0];
    X1[1] = vAttr3[0][0];

    /*  Get the three vertex x coordinates.  Use zxy swizzle.  *
    X2[1] = vAttr1[0][0];
    X2[2] = vAttr2[0][0];
    X2[0] = vAttr3[0][0];

    /*  Get the three vertices y coordinates.  Use yzx swizzle.  *
    Y1[2] = vAttr1[0][1];
    Y1[0] = vAttr2[0][1];
    Y1[1] = vAttr3[0][1];

    /*  Get the three vertices y coordinates.  Use zxy swizzle.  *
    Y2[1] = vAttr1[0][1];
    Y2[2] = vAttr2[0][1];
    Y2[0] = vAttr3[0][1];

    /*  Get the three vertices w coordinates.  Use yzx swizzle.  *
    W1[2] = vAttr1[0][3];
    W1[0] = vAttr2[0][3];
    W1[1] = vAttr3[0][3];

    /*  Get the three vertices w coordinates.  Use zxy swizzle.  *
    W2[1] = vAttr1[0][3];
    W2[2] = vAttr2[0][3];
    W2[0] = vAttr3[0][3];

    /*  Calculate the adjoint matrix for the [X Y W] matrix (stored
        as rows) (three vertices 2DH coordinate matrix) and store it
        in A, B and C vectors (adjoint matrix columns).  */

    /*  The adjoint matrix is calculated this way
        (using a shader program):

          mul rC.xyz, rX.zxy, rY.yzx
          mul rB.xyz, rX.yzx, rW.zxy
          mul rA.xyz, rY.zxy, rW.yzx
          mad rC.xyz, rX.yzx, rY.zxy, -rC
          mad rB.xyz, rX.zxy, rW.yzx, -rB
          mad rA.xyz, rY.yzx, rW.zxy, -rA

     *

    GPUMath::MUL(X2, Y1, C);
    GPUMath::MUL(X1, W2, B);
    GPUMath::MUL(Y2, W1, A);

    //GPUMath::NEG(C, auxC);
    //GPUMath::NEG(B, auxB);
    //GPUMath::NEG(A, auxA);

    auxC[0] = - C[0];
    auxC[1] = - C[1];
    auxC[2] = - C[2];

    auxB[0] = - B[0];
    auxB[1] = - B[1];
    auxB[2] = - B[2];

    auxA[0] = - A[0];
    auxA[1] = - A[1];
    auxA[2] = - A[2];

    GPUMath::MAD(X1, Y2, auxC, C);
    GPUMath::MAD(X2, W1, auxB, B);
    GPUMath::MAD(Y1, W2, auxA, A);
    */

}


/*  !! TO BE USED ONLY AS A REFERENCE FOR USING A PROGRAMMABLE UNIT
    (SIMILAR TO A VERTEX SHADER) FOR RASTERIZATION MATRIX ADJUST TO
    THE VIEWPORT!!  */
void viewportAdjust()
{
    panic("RasterizerEmulator.cpp", "viewportAdjust()", "Function with non-compilable code");
    /*
    QuadFloat *vAttr1, *vAttr2, *vAttr3;
    f32bit auxA[3], auxB[3];//, auxC[3];
    f32bit A[3], B[3], C[3];
    f32bit edge1[3], edge2[3], edge3[3];
    f32bit area;
    s32bit x0, y0;
    u32bit w,h;

    /*  Adjust the three edge equations to the viewport:

        a' = a * 2/w
        b' = b * 2/h
        c' = c - a * ((x0 * 2)/w + 1) - b * ((y0 * 2)/h + 1)

     *

    /*  Create auxiliary vector { 2/w, 2/w, 2/w }.  *
    auxA[0] = 2.0f / w;
    auxA[1] = 2.0f / w;
    auxA[2] = 2.0f / w;

    /*  Create auxiliary vector { 2/h, 2/h, 2/h }.  *
    auxB[0] = 2.0f / h;
    auxB[1] = 2.0f / h;
    auxB[2] = 2.0f / h;

    /*  Adjust c coefficients to the viewport.
    edge1[2] = C[0] - A[0] * (((f32bit) (x0 * 2.0)/w) + 1.0) -
        B[0] * (((f32bit) (y0 * 2.0)/h) + 1.0);
    edge2[2] = C[1] - A[1] * (((f32bit) (x0 * 2.0)/w) + 1.0) -
        B[1] * (((f32bit) (y0 * 2.0)/h) + 1.0);
    edge3[2] = C[2] - A[2] * (((f32bit) (x0 * 2.0)/w) + 1.0) -
        B[2] * (((f32bit) (y0 * 2.0)/h) + 1.0);

    /*  Adjust a coefficients to the viewport.  *
    GPUMath::MUL(A, auxA, A);

    /*  Adjust b coefficients to the viewport.  *
    GPUMath::MUL(B, auxB, B);

    /*  Create triangle edge equations from the adjoint matrix:

            A B C (rows) -> edge1 edge2 edge3 (columns)

     *

    /*  Create first edge equation.  *
    edge1[0] = A[0];
    edge1[1] = B[0];

    /*  Create second edge equation.  *
    edge2[0] = A[1];
    edge2[1] = B[1];

    /*  Create third edge equation.  *
    edge3[0] = A[2];
    edge3[1] = B[2];

    /*  Calculate the triangle matrix determinant (an approximation
        to the triangle signed area:

            det(M) = dot3(C, W) = c0 * w0 + c1 * w1 + c2 * w2

     *
    area = edge1[2] * vAttr1[0][3] + edge2[2] * vAttr2[0][3] +
        edge3[2] * vAttr3[0][3];


        */
}


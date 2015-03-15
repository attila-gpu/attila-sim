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
 * $RCSfile: Fragment.cpp,v $
 * $Revision: 1.14 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:40 $
 *
 * Fragment implementation file.
 *
 */

/**
 *
 *  @file Fragment.cpp
 *
 *  Implements the Fragment class.
 *
 *  This class describes and stores the associated data of
 *  fragment inside the 3D graphic pipeline.
 *
 *
 */

#include "Fragment.h"
#include "GPUMath.h"
#include <cstdio>

using namespace gpu3d;

/*  Tile Identifier constructor.  */
TileIdentifier::TileIdentifier(s32bit xIn, s32bit yIn):

    x(xIn), y(yIn)
{
}

/*  Gets the tile identifier x coordinate.  */
s32bit TileIdentifier::getX() const
{
    return x;
}

/*  Gets the tile identifier y coordinate.  */
s32bit TileIdentifier::getY() const
{
    return y;
}

/*  Assigns tile to a processing unit based on the horizontal coordinate.  */
u32bit TileIdentifier::assignOnX(u32bit numUnits)
{
    /*  Units are distributed per tile columns:

        0 1 2 3 0 1 2 3
        0 1 2 3 0 1 2 3
        0 1 2 3 0 1 2 3
        0 1 2 3 0 1 2 3

     */

    return u32bit(GPU_PMOD(x, s32bit(numUnits)));
}

/*  Assigns tile to a processing unit based on the vertical coordinate.  */
u32bit TileIdentifier::assignOnY(u32bit numUnits)
{
    /*  Units are distributed per tile lines:

        0 0 0 0 0 0 0 0
        1 1 1 1 1 1 1 1
        2 2 2 2 2 2 2 2
        3 3 3 3 3 3 3 3
        0 0 0 0 0 0 0 0
        1 1 1 1 1 1 1 1
        2 2 2 2 2 2 2 2
        3 3 3 3 3 3 3 3

    */

    return u32bit(GPU_PMOD(y, s32bit(numUnits)));
}

/*  Assigns tile to a processing unit interleaved the horizontal coordinate.  */
u32bit TileIdentifier::assignInterleaved(u32bit numUnits)
{
    /*  Units are distributed on a line basis and displaced one position for each line:

        0 1 2 3 0 1 2 3
        1 2 3 0 1 2 3 0
        2 3 0 1 2 3 0 1
        3 0 1 2 3 0 1 2

      */

    return u32bit(GPU_PMOD(x + GPU_PMOD(y, s32bit(numUnits)), s32bit(numUnits)));
}

/*  Assigns tile to a processing unit interleaved in morton/Z orden.  */
u32bit TileIdentifier::assignMorton(u32bit numUnits)
{
    /*  Tiles are distributed per unit on a morton orden basis:

        0 1 0 1 0 1 0 1
        2 3 2 3 2 3 2 3
        0 1 0 1 0 1 0 1
        2 3 2 3 2 3 2 3

    */

    GPU_ASSERT(
        if ((numUnits != 1) && (numUnits != 2) && (numUnits != 4) && (numUnits != 8) && (numUnits != 16))
            panic("TileIdentifier", "assignMorton", "Tile to unit Morton assignation mode only supported for 1, 2, 4 or 8 units.");
    )

    if (numUnits == 1)
        return 0;

    if (numUnits == 2)
        return ((x + y) & 0x01);

    if (numUnits == 4)
        return  GPU_MOD((x & 0x01) + ((y & 0x01) << 1), numUnits);
        
    if ((numUnits == 8) || (numUnits == 16))
        return  GPU_MOD((x & 0x01) + ((y & 0x01) << 1) + ((x & 0x02) << 1) + ((y & 0x02) << 2), numUnits);

    return 0; // put it here to avoid stupid compiler errors (carlos)
}


/*  Equality comparator redefined.  */
bool TileIdentifier::operator==(TileIdentifier a)
{
    return (x == a.x) && (y == a.y);
}

/*  Inequality comparator redefined.  */
bool TileIdentifier::operator!=(TileIdentifier a)
{
    return (x != a.x) || (y != a.y);
}

/*  Fragment constructor.  */
Fragment::Fragment(SetupTriangle *setupTri, s32bit xCoord, s32bit yCoord,
    u32bit depth, f64bit *coord, bool inside)
{
    //  Store the setup triangle that generated the fragment.
    triangle = setupTri;

    //  Set fragment screen coordinates.
    x = xCoord;
    y = yCoord;

    //  Set fragment depth (z/w).
    z = depth;

    //  Set fragment edge/barycentric coordinates.
    coordinates[0] = coord[0];
    coordinates[1] = coord[1];
    coordinates[2] = coord[2];

    //  Set the fragment z/w.
    zw = coord[3];

    //  Calculate if the fragment is inside the triangle.
    //  NOTE: ONLY FOR FRONT FACING TRIANGLES.  BACK FACING IS IMPLEMENTED
    //  INVERTING THE TRIANGLE EDGE EQUATIONS SIGNS AT SETUP.
    //
    //  Clips against the near and far clip planes ( -1 <= zw <= 1).
    //insideTriangle = insideEdgeEquations() && GPU_IS_LESS_EQUAL(GPU_ABS(zw), f64bit(1));
    insideTriangle = inside;

    /*  Update setup triangle references.  */
    triangle->newReference();

    /*  Set as not last fragment.  */
    isLast = FALSE;

    setTag("Frag");
}

/*  Fragment constructor (simplified version for bypass triangle´s fragments).  */
Fragment::Fragment(s32bit xCoord, s32bit yCoord)
{
    /*  Set fragment screen coordinates.  */
    x = xCoord;
    y = yCoord;

    /*  Set as not last fragment.  */
    isLast = FALSE;

    /*  Set fake setup triangle.  */
    triangle = new SetupTriangle(0,0,0);

    setTag("Frag");
}

/*  Fragment destructor.  */
Fragment::~Fragment()
{
    /*  Decrement setup triangle references.  */
    triangle->deleteReference();
}


/*  Returns the fragment coordinates (barycentric or edge equation values)
    inside the triangle. */
f64bit *Fragment::getCoordinates()
{
    return coordinates;
}

//  Returns the interpolated z/w value for the fragment.
const f64bit Fragment::getZW()
{
    return zw;
}

/*  Returns if the fragment is inside the triangle.  */
const bool Fragment::isInsideTriangle()
{
    return insideTriangle;
}

/*  Returns the fragment x coordinate.  */
const s32bit Fragment::getX()
{
    return x;
}

/*  Returns the fragment y coordinate.  */
const s32bit Fragment::getY()
{
    return y;
}

/*  Returns the fragment depth.  */
const u32bit Fragment::getZ()
{
    return z;
}

/*  Sets the fragment depth (z/w). */
void Fragment::setZ(u32bit zValue)
{
    z = zValue;
}

/*  Returns the fragment triangle.  */
SetupTriangle *Fragment::getTriangle()
{
    return triangle;
}

/*  Sets the fragment as the last one.  */
void Fragment::lastFragment()
{
    isLast = TRUE;
}

/*  Returns if it is the last fragment in the triangle.  */
const bool Fragment::isLastFragment()
{
    return isLast;
}


//
//  NOTE: Moved to Rasterizer Emulator.
// 

//#define INSIDE_EQUATION(e, a, b) ((GPU_IS_POSITIVE(e) && !GPU_IS_ZERO(e)) || (GPU_IS_ZERO(e) &&\
//((a > 0.0f) || ((a == 0.0f) && (b >= 0.0f)))))

/*  Test if the fragment is inside one of the triangle edge equations.  */
/*bool Fragment::insideEdgeEquations()
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

    inside = INSIDE_EQUATION(coordinates[0], edge1[0], edge1[1]) &&
        INSIDE_EQUATION(coordinates[1], edge2[0], edge2[1]) &&
        INSIDE_EQUATION(coordinates[2], edge3[0], edge3[1]);

    return inside;
}*/

//  Set the fragment Z samples for Multi-sampling Anti-Aliasing (MSAA)
void Fragment::setMSAASamples(u32bit samples, u32bit *msaaZSamples, bool *coverage, f64bit *centroid, bool anyInsideTriangle)
{
    for(u32bit i = 0; i < samples; i++)
    {
        msaaZ[i] = msaaZSamples[i];
        msaaCoverage[i] = coverage[i];
    }
    
    //  The fragment is inside the triangle if any of the samples is inside the triangle.
    insideTriangle = anyInsideTriangle;
    
    if (anyInsideTriangle && (centroid != NULL))
    {
        coordinates[0] = centroid[0];
        coordinates[1] = centroid[1];
        coordinates[2] = centroid[2];
        coordinates[3] = centroid[3];
    }
}

//  Return the pointer to the array storing the fragment Z samples for MSAA
u32bit *Fragment::getMSAASamples()
{
    return msaaZ;
}

//  Return the pointer to the array storing the coverage flags for the fragment MSAA samples
bool *Fragment::getMSAACoverage()
{
    return msaaCoverage;
}




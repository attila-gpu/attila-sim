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
 * $RCSfile: Tile.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2005-05-12 08:50:40 $
 *
 * Tile implementation file.
 *
 */

/**
 *
 *  @file Tile.cpp
 *
 *  Defines the Tile class.
 *
 *  This class stores the data of viewport tiles being evaluated
 *  over a triangle or triangles to rasterize in a 3D GPU simulator.
 *
 *
 */

#include "Tile.h"
#include <cstring>

using namespace gpu3d;

/*  Tile constructor.  */
Tile::Tile(SetupTriangle *tri, u32bit startX, u32bit startY, f64bit *edgEq, f64bit zeq, u32bit l)
{
    u32bit i;

    /*  Initialize pointers to edge equation value arrays.  */
    for(i = 0; i < MAX_TRIANGLES; i++)
    {
        edgeEq[i] = &sEq[i * 3];
    }

    /*  Set tile parameters.  */
    triangle[0] = tri;
    x = startX;
    y = startY;
    edgeEq[0][0] = edgEq[0];
    edgeEq[0][1] = edgEq[1];
    edgeEq[0][2] = edgEq[2];
    zEq[0] = zeq;
    level = l;
    numTriangles = 1;
    nextTriangle = 0;
    inside[0] = TRUE;

    /*  Reset tile inside triangle flags.  */
    for(i = numTriangles; i < MAX_TRIANGLES; i++)
        inside[i] = FALSE;

    /*  Increment setup triangle references.  */
    triangle[0]->newReference();

    setTag("Tile");
}

/*  Tile constructor.  */
Tile::Tile(SetupTriangle **tri, u32bit nTris, u32bit startX, u32bit startY, f64bit **edgEq, f64bit *zeq, u32bit l)
{
    u32bit i;

    GPU_ASSERT(
        if (nTris == 0)
            panic("Tile", "Tile", "At least a triangle is required.");
    )

    /*  Initialize pointers to edge equation value arrays.  */
    for(i = 0; i < MAX_TRIANGLES; i++)
    {
        edgeEq[i] = &sEq[i * 3];
    }

    /*  Set all the triangle parameters in the tile.  */
    for(i = 0; i < nTris; i++)
    {
        /*  Set per triangle parameters.  */
        triangle[i] = tri[i];
        edgeEq[i][0] = edgEq[i][0];
        edgeEq[i][1] = edgEq[i][1];
        edgeEq[i][2] = edgEq[i][2];
        zEq[i] = zeq[i];
        inside[i] = TRUE;

        /*  Increment setup triangle references.  */
        triangle[i]->newReference();
    }

    /*  Set common tile parameters.  */
    x = startX;
    y = startY;
    level = l;
    numTriangles = nTris;
    nextTriangle = 0;

    /*  Reset tile inside triangle flags.  */
    for(i = numTriangles; i < MAX_TRIANGLES; i++)
        inside[i] = FALSE;

    setTag("Tile");
}

/*  Tile copy function.  */
Tile::Tile(const Tile &input)
{
    u32bit i;

    /*  Copy all the data from the input tile.  */
    memcpy(this, &input, sizeof(Tile));

    /*  Update the references to the setup triangles in the tile.  */
    for(i = 0; i < numTriangles; i++)
    {
            triangle[i]->newReference();
    }

    setTag("Tile");
}

/*  Tile destructor.  */
Tile::~Tile()
{
    u32bit i;

    /*  Unreference the references to the tile triangles.  */
    for(i = 0; i < numTriangles; i++)
    {
        /*  Decrement setup triangle references.  */
        triangle[i]->deleteReference();
    }
}

/*  Returns the tile start x position.  */
u32bit Tile::getX()
{
    return x;
}

/*  Returns the tile start y position.  */
u32bit Tile::getY()
{
    return y;
}


/*  Returns a pointer to the triangle(s) being evaluated in the tile.  */
SetupTriangle *Tile::getTriangle()
{
    return triangle[0];
}

/*  Returns a pointer to the triangle(s) being evaluated in the tile.  */
SetupTriangle *Tile::getTriangle(u32bit id)
{
    GPU_ASSERT(
        if (id >= numTriangles)
            panic("Tile", "getTriangle", "Wrong triangle identifier for the tile.");
    )

    return triangle[id];
}

/*  Returns the value of the triangle(s) edge equation at the tile start point.  */
f64bit *Tile::getEdgeEquations()
{
    return edgeEq[0];
}

/*  Returns the value of the triangle(s) edge equation at the tile start point.  */
f64bit *Tile::getEdgeEquations(u32bit id)
{
    GPU_ASSERT(
        if (id >= numTriangles)
            panic("Tile", "getEdgeEquations", "Wrong triangle identifier for the tile.");
    )

    return edgeEq[id];
}


/*  Returns the value of the z/w equation at the tile start point.  */
f64bit Tile::getZEquation()
{
    return zEq[0];
}

/*  Returns the value of the z/w equation at the tile start point.  */
f64bit Tile::getZEquation(u32bit id)
{
    GPU_ASSERT(
        if (id >= numTriangles)
            panic("Tile", "getEdgeEquations", "Wrong triangle identifier for the tile.");
    )

    return zEq[id];
}

/*  Returns the tile level/size.  */
u32bit Tile::getLevel()
{
    return level;
}

/*  Returns the number of triangles being evaluated in the tile.  */
u32bit Tile::getNumTriangles()
{
    return numTriangles;
}

/*  Returns a pointer to the array of triangle inside tile flags.  */
bool *Tile::getInside()
{
    return inside;
}

/*  Returns the next triangle in the tile for which to generate fragments.  */
u32bit Tile::getNextTriangle()
{
    return nextTriangle;
}

/*  Sets the next triangle pointer.  */
void Tile::setNextTriangle(u32bit triangle)
{
    GPU_ASSERT(
        if (triangle >= numTriangles)
            panic("Tile", "setNextTriangle", "Triangle identifier outside range.");
    )

    nextTriangle = triangle;
}


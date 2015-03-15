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
 * $RCSfile: TriangleSetupInput.cpp,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:29 $
 *
 * Triangle Setup Input definition file.
 *
 */

/**
 *
 *  @file TriangleSetupInput.cpp
 *
 *  This file implements the Triangle Setup Input class.
 *
 */

#include "TriangleSetupInput.h"

using namespace gpu3d;

/*  Creates a new TriangleSetupInput.  */
TriangleSetupInput::TriangleSetupInput(u32bit ID, QuadFloat *attrib1,
    QuadFloat *attrib2, QuadFloat *attrib3, bool lastTriangle)
{
    /*  Set vertex parameters.  */
    triangleID = ID;
    attributes[0] = attrib1;
    attributes[1] = attrib2;
    attributes[2] = attrib3;
    rejected = FALSE;
    last = lastTriangle;
    preBound = false;

    setTag("TrSIn");
}

/*  Gets the triangle identifier.  */
u32bit TriangleSetupInput::getTriangleID()
{
    return triangleID;
}

/*  Gets the triangle vertex attributes.  */
QuadFloat *TriangleSetupInput::getVertexAttributes(u32bit vertex)
{
    /*  Check vertex number.  */
    GPU_ASSERT(
        if (vertex > 2)
            panic("TriangleSetupInput", "getVertexAttributes", "Undefined vertex number.");
    )

    return attributes[vertex];
}

/*  Sets the triangle as rejected/culled.  */
void TriangleSetupInput::reject()
{
    rejected = TRUE;
}


/*  Gets if the triangle was rejected/culled.  */
bool TriangleSetupInput::isRejected()
{
    return rejected;
}

/*  Gets if it is the last triangle in the pipeline.  */
bool TriangleSetupInput::isLast()
{
    return last;
}

/*  Sets the triangle as pre-bound triangle.  */
void TriangleSetupInput::setPreBound()
{
    preBound = true;
}

/*  Returns if pre-bound triangle.  */
bool TriangleSetupInput::isPreBound()
{
    return preBound;
}

/*  Sets the triangle ID in Rasterizer emulator.  */
void TriangleSetupInput::setSetupID(u32bit setupId)
{
    setupID = setupId;
}

/*  Gets the triangle ID in Rasterizer emulator.  */
u32bit TriangleSetupInput::getSetupID()
{
    return setupID;
}


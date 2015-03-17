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
 * $RCSfile: TriangleSetupOutput.cpp,v $
 * $Revision: 1.6 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:30 $
 *
 * Triangle Setup Output implementation file.
 *
 */

/**
 *
 *  @file TriangleSetupInput.cpp
 *
 *  This file implements the Triangle Setup Output class.
 *
 *  This class transports setup triangles from Triangle
 *  Setup to Fragment Generation/Triangle Traversal.
 *
 */

#include "TriangleSetupOutput.h"
#include <stdio.h>
using namespace gpu3d;

/*  Creates a new TriangleSetupOutput.  */
TriangleSetupOutput::TriangleSetupOutput(u32bit ID, u32bit setupID, bool lastTri)
{
    /*  Set triangle parameters.  */
    triangleID = ID;
    triSetupID = setupID;
    culled = FALSE;
    last = lastTri;

    sprintf((char *) getInfo(), "triID %d triSetID %d", ID, setupID);

    setTag("TrSOut");
}

/*  Gets the triangle identifier (batch).  */
u32bit TriangleSetupOutput::getTriangleID()
{
    return triangleID;
}

/*  Gets the setup triangle identifier (rasterizer emulator).  */
u32bit TriangleSetupOutput::getSetupTriangleID()
{
    return triSetupID;
}

/*  Gets if the triangle has been culled in previous stage.  */
bool TriangleSetupOutput::isCulled()
{
    return culled;
}

/*  Returns if the triangle is marked as the last in the batch.  */
bool TriangleSetupOutput::isLast()
{
    return last;
}




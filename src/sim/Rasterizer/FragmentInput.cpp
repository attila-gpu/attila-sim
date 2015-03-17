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
 * Fragment Input definition file.
 *
 */

/**
 *
 *  @file FragmentInput.cpp
 *
 *  This file implements the Fragment Input class.
 *
 *  This class carries fragment data between Triangle Traversal,
 *  the Interpolator, the fragment FIFO and the Pixel Shaders.
 *
 */

#include "FragmentInput.h"
#include "stdio.h"

using namespace gpu3d;

/*  Creates a new FragmentInput.  */
FragmentInput::FragmentInput(u32bit ID, u32bit setupID, Fragment *fragment, TileIdentifier id, u32bit stampUnitID):

    tileID(id)
{
    /*  Set fragment parameters.  */
    triangleID = ID;
    setupTriangle = setupID;
    fr = fragment;
    stampUnit = stampUnitID;
    tileID = id;

    /*  Write fragment information.  */
    if (fr != NULL)
        sprintf((char *) getInfo(), "SU:%d %d, %d", stampUnitID, fr->getX(), fr->getY());
    else
        sprintf((char *) getInfo(), "SU:%d last fragment", stampUnitID);

    /*  Mark the fragment as not culled.  */
    culled = FALSE;

    /*  The fragment has no interpolated attributes yet.  */
    attributes = NULL;

    setTag("FrIn");
}

/*  Gets the fragment triangle identifier.  */
u32bit FragmentInput::getTriangleID() const
{
    return triangleID;
}

/*  Gets the fragment setup triangle identifier.  */
u32bit FragmentInput::getSetupTriangle() const
{
    return setupTriangle;
}

/*  Gets the fragment interpolated attributes.  */
QuadFloat *FragmentInput::getAttributes() const
{
    return attributes;
}

/*  Sets the fragment interpolated attributes array.  */
void FragmentInput::setAttributes(QuadFloat *attrib)
{
    attributes = attrib;
}

/*  Gets the fragment input Fragment object.  */
Fragment *FragmentInput::getFragment() const
{
    return fr;
}

/*  Sets the fragment new cull flag value.  */
void FragmentInput::setCull(bool cull)
{
    /*  Set the cull flag.  */
    culled = cull;
}

/*  Returns the fragment cull flag.  */
bool FragmentInput::isCulled() const
{
    return culled;
}

/*  Returns the fragment assigned stamp unit.  */
u32bit FragmentInput::getStampUnit() const
{
    return stampUnit;
}

/*  Sets the fragment start cycle.  */
void FragmentInput::setStartCycle(u64bit cycle)
{
    startCycle = cycle;
}

/*  Returns the fragment start cycle.  */
u64bit FragmentInput::getStartCycle() const
{
    return startCycle;
}

/*  Sets the fragment start cycle in the shader.  */
void FragmentInput::setStartCycleShader(u64bit cycle)
{
    startCycleShader = cycle;
}

/*  Returns the fragment start cycle in the shader unit.  */
u64bit FragmentInput::getStartCycleShader() const
{
    return startCycleShader;
}

/*  Sets the number of cycles the fragment spent in the shader.  */
void FragmentInput::setShaderLatency(u32bit latency)
{
    shaderLatency = latency;
}

/*  Returns the number of cycles the fragment spent inside the shader unit .  */
u32bit FragmentInput::getShaderLatency() const
{
    return shaderLatency;
}


/*  Sets the fragment tile identifier.  */
void FragmentInput::setTileID(TileIdentifier id)
{
    tileID = id;
}

/*  Gets the fragment tile identifier.  */
TileIdentifier FragmentInput::getTileID() const
{
    return tileID;
}

/*  Sets the shader unit to which the fragment is assigned.  */
void FragmentInput::setShaderUnit(u32bit unit)
{
    shaderUnit = unit;
}

/*  Gets the shader unit to which  the fragment was assigned.  */
u32bit FragmentInput::getShaderUnit() const
{
    return shaderUnit;
}


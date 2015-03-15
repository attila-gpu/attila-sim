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
 * $RCSfile: ShaderInput.cpp,v $
 * $Revision: 1.8 $
 * $Author: vmoya $
 * $Date: 2006-01-31 12:54:39 $
 *
 * Shader Input definition file.
 *
 */

#include "ShaderInput.h"

using namespace gpu3d;

/*  Creates a new ShaderInput.  */
ShaderInput::ShaderInput(u32bit ID, u32bit unitID, u32bit stEntry, QuadFloat *attrib)
{
    /*  Set shader input parameters.  */
    id = ID;
    unit = unitID;
    entry = stEntry;
    attributes = attrib;
    kill = false;
    isAFragment = false;
    last = false;

    setTag("ShInp");
}

/*  Creates a new ShaderInput.  */
ShaderInput::ShaderInput(u32bit ID, u32bit unitID, u32bit stEntry, QuadFloat *attrib, ShaderInputMode shInMode)
{
    /*  Set shader input parameters.  */
    id = ID;
    unit = unitID;
    entry = stEntry;
    attributes = attrib;
    kill = false;
    isAFragment = (shInMode == SHIM_FRAGMENT || shInMode == SHIM_MICROTRIANGLE_FRAGMENT);
    mode = shInMode;
    last = false;

    setTag("ShInp");
}

/*  Creates a new ShaderInput.  */
ShaderInput::ShaderInput(ShaderInputID ID, u32bit unitID, u32bit stEntry, QuadFloat *attrib)
{
    /*  Set shader input parameters.  */
    id = ID;
    unit = unitID;
    entry = stEntry;
    attributes = attrib;
    kill = false;
    isAFragment = false;
    last = false;

    setTag("ShInp");
}

/*  Creates a new ShaderInput.  */
ShaderInput::ShaderInput(ShaderInputID ID, u32bit unitID, u32bit stEntry, QuadFloat *attrib, ShaderInputMode shInMode)
{
    /*  Set shader input parameters.  */
    id = ID;
    unit = unitID;
    entry = stEntry;
    attributes = attrib;
    kill = false;
    isAFragment = (shInMode == SHIM_FRAGMENT || shInMode == SHIM_MICROTRIANGLE_FRAGMENT);
    mode = shInMode;
    last = false;

    setTag("ShInp");
}

/*  Gets the shader input identifier.  */
u32bit ShaderInput::getID()
{
    return id.id.inputID;
}

/*  Gets the shader input identifier.  */
ShaderInputID ShaderInput::getShaderInputID()
{
    return id;
}

/*  Gets the shader input post shading storage entry.  */
u32bit ShaderInput::getEntry()
{
    return entry;
}

/*  Gets the shader input post shading storage unit.  */
u32bit ShaderInput::getUnit()
{
    return unit;
}

/*  Gets the shader input attributes.  */
QuadFloat *ShaderInput::getAttributes()
{
    return attributes;
}

/*  Sets the shader input kill flag.  */
void ShaderInput::setKilled()
{
    kill = true;
}

/*  Returns is the shader input was killed/aborted.  */
bool ShaderInput::getKill()
{
    return kill;
}

/*  Set shader input as being a fragment.  */
void ShaderInput::setAsFragment()
{
    isAFragment = true;
    mode = SHIM_FRAGMENT;
}

/*  Set shader input as being a vertex.  */
void ShaderInput::setAsVertex()
{
    isAFragment = false;
    mode = SHIM_VERTEX;
}

/*  Returns if the shader input is for a fragment.  */
bool ShaderInput::isFragment()
{
    return isAFragment;
}

/*  Returns if the shader input is for a vertex.  */
bool ShaderInput::isVertex()
{
    return !isAFragment;
}

/*  Set last index in batch flag.  */
void ShaderInput::setLast(bool lastInBatch)
{
    last = lastInBatch;
}

/*  Return if the index is the last in the batch.  */
bool ShaderInput::isLast()
{
    return last;
}

/*  Return the shader input mode.  */
ShaderInputMode ShaderInput::getInputMode()
{
    return mode;
}

/*  Sets the start cycle in the shader.  */
void ShaderInput::setStartCycleShader(u64bit cycle)
{
    startCycleShader = cycle;
}

/*  Returns the start cycle in the shader unit.  */
u64bit ShaderInput::getStartCycleShader() const
{
    return startCycleShader;
}

/*  Sets the number of cycles spent in the shader.  */
void ShaderInput::setShaderLatency(u32bit latency)
{
    shaderLatency = latency;
}

/*  Returns the number of cycles spent inside the shader unit .  */
u32bit ShaderInput::getShaderLatency() const
{
    return shaderLatency;
}

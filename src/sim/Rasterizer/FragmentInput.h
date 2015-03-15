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
 * Fragment Input implementation file.
 *
 */

#ifndef _FRAGMENTINPUT_

#define _FRAGMENTINPUT_

#include "support.h"
#include "GPUTypes.h"
#include "DynamicObject.h"
#include "Fragment.h"

namespace gpu3d
{

/**
 *
 *  @file FragmentInput.h
 *
 *  This file defines the Fragment Input class.
 *
 *  This class is used to carry fragment information between
 *  Triangle Traversal and the Interpolator unit up to the
 *  Fragment FIFO and Pixel Shader.
 *
 */

/**
 *
 *  This class defines objects that carry fragment data
 *  from the Fragment Generator/Triangle Traversal to the
 *  Fragment FIFO and Pixel Shader unit.
 *
 *  This class inherits from the DynamicObject class that
 *  offers basic dynamic memory and signal tracing support
 *  functions.
 *
 */

class FragmentInput : public DynamicObject
{

private:

    u32bit triangleID;          /**<  Triangle identifier (inside the current batch).  */
    u32bit setupTriangle;       /**<  The setup triangle identifier (in the rasterizer emulator).  */
    Fragment *fr;               /**<  Pointer to the fragment object with the fragment data.  */
    QuadFloat *attributes;      /**<  Pointer to the fragment interpolated attributes.  */
    bool culled;                /**<  Flag that keeps if the fragment has been culled in any of the test stages (alpha, Z, stencil, ...).  */
    TileIdentifier tileID;      /**<  The fragment tile identifier.  */
    u32bit stampUnit;           /**<  Identifier of the stamp unit to which the fragment is assigned.  */
    u32bit shaderUnit;          /**<  Identifier of the shader unit to which the fragment is assigned.  */
    u64bit startCycle;          /**<  Stores the cycle when the fragment was created (or any other reference initial cycle).  */
    u64bit startCycleShader;    /**<  Cycle in which the fragment was issued into the shader unit.  */
    u32bit shaderLatency;       /**<  Stores the cycles spent inside the shader unit by the fragment.  */

public:

    /**
     *
     *  Fragment Input constructor.
     *
     *  Creates and initializes a fragment input.
     *
     *  @param id The triangle identifier.
     *  @param setupID The setup triangle identifier.
     *  @param fr Pointer to the fragment object with the data.
     *  @param tileID The fragment tile identifier.
     *  @param stampUnitID Identifier of the stamp unit to which the fragment has been assigned.
     *
     *  @return An initialized fragment input.
     *
     */

    FragmentInput(u32bit id, u32bit setupID, Fragment *fr, TileIdentifier tid, u32bit stampUnitID);

    /**
     *
     *  Gets the fragment triangle identifier (inside the batch).
     *
     *  @return The triangle identifier.
     *
     */

    u32bit getTriangleID() const;

    /**
     *
     *  Gets the fragment setup triangle identifier (rasterizer emulator).
     *
     *  @return The setup triangle identifier.
     *
     */

    u32bit getSetupTriangle() const;

    /**
     *
     *  Gets the fragment input Fragment object.
     *
     *  @return The fragment input Fragment object.
     *
     */

    Fragment *getFragment() const;

    /**
     *
     *  Gets the fragment interpolated attributes.
     *
     *  @return A pointer to the fragment interpolated attribute array.
     *
     */

    QuadFloat *getAttributes() const;

    /**
     *
     *  Sets the fragment interpolated attributes.
     *
     *  @param attr The interpolated fragment attributes.
     *
     */

    void setAttributes(QuadFloat *attr);

    /**
     *
     *  Sets the fragment cull flag.
     *
     *  @param cull The new value of the fragment cull flag.
     *
     */

    void setCull(bool cull);

    /**
     *
     *  Returns if the fragment has been culled in any stage
     *  of the fragment pipeline (scissor, alpha, z, stencil, ...).
     *
     *  @return The fragment cull flag.
     *
     */

    bool isCulled() const;

    /**
     *
     *  Returns the identifier of the stamp unit/pipe to which the fragment has
     *  been assigned.
     *
     *  @return The identifier of the stamp unit for the fragment.
     *
     */

    u32bit getStampUnit() const;

    /**
     *
     *  Sets the fragment start cycle.
     *
     *  @param startCycle The initial cycle for the fragment.
     *
     */

    void setStartCycle(u64bit cycle);

    /**
     *
     *  Gets the fragment start cycle.
     *
     *  @return The fragment start cycle.
     *
     */

    u64bit getStartCycle() const;

    /**
     *
     *  Sets the fragment start cycle in the shader.
     *
     *  @param cycle The initial cycle for the fragment in the shader.
     *
     */

    void setStartCycleShader(u64bit cycle);

    /**
     *
     *  Gets the fragment start cycle in the shader.
     *
     *  @return The fragment start cycle in the shader.
     *
     */

    u64bit getStartCycleShader() const;

    /**
     *
     *  Sets the fragment shader latency (cycles spent in the shader).
     *
     *  @param cycle Number of cycles the fragment spent in the shader.
     *
     */

    void setShaderLatency(u32bit cycle);

    /**
     *
     *  Gets the fragment shader latency (cycles spent in the shader).
     *
     *  @return The number of cycles that the fragment spent inside the shader.
     *
     */

    u32bit getShaderLatency() const;

    /**
     *
     *  Sets the fragment tile identifier.
     *
     *  @param tileID The tile identifier for the fragment
     *
     */

    void setTileID(TileIdentifier tileID);

    /**
     *
     *  Gets the fragment tile identifier.
     *
     *  @return The fragment tile identifier.
     *
     */

    TileIdentifier getTileID() const;

    /**
     *
     *  Sets the shader unit to which the fragment is going to be assigned.
     *
     *  @param unit Identifier of the shader unit the fragment is assigned to.
     *
     */

     void setShaderUnit(u32bit unit);

     /**
      *
      *  Gets the shader unit to which the fragment is assigned.
      *
      *  @return The identifier of the shader unit to which the fragment has been assigned.
      *
      */

    u32bit getShaderUnit() const;

};

} // namespace gpu3d

#endif

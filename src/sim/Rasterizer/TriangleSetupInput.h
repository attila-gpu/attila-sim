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
 * $RCSfile: TriangleSetupInput.h,v $
 * $Revision: 1.4 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:58 $
 *
 * Triangle Setup Input implementation file. 
 *
 */

#ifndef _TRIANGLESETUPINPUT_

#define _TRIANGLESETUPINPUT_

#include "support.h"
#include "GPUTypes.h"
#include "DynamicObject.h"

namespace gpu3d
{

/**
 * 
 *  @file TriangleSetupInput.h 
 *
 *  This file defines the Triangle Setup Input class.
 *
 */
  
/**
 *
 *  This class defines objects that carry triangle data
 *  from the Primitive Assembly to the Triangle Setup
 *  unit.
 *
 *  It is also used by the MicroPolygon Rasterizer to
 *  carry data from Primitive Assembly to Triangle Bound,
 *  and from Triangle Bound to Triangle Setup.
 *   
 *
 *  This class inherits from the DynamicObject class that
 *  offers basic dynamic memory and signal tracing support
 *  functions.
 *
 */
 
class TriangleSetupInput : public DynamicObject
{

private:

    u32bit triangleID;          /**<  Triangle identifier.  */
    QuadFloat *attributes[3];   /**<  Triangle vertex attributes.  */
    bool rejected;              /**<  The triangle has been rejected/culled in a previous stage.  */
    bool last;                  /**<  Last triangle signal.  */
    u32bit setupID;             /**<  The triangle identifier in Rasterizer emulator.  */
    bool preBound;

public:

    /**
     *
     *  Triangle Setup Input constructor.
     *
     *  Creates and initializes a triangle setup input.
     *
     *  @param id The triangle identifier.
     *  @param attrib1 The triangle first vertex attributes.
     *  @param attrib2 The triangle second vertex attributes.
     *  @param attrib3 The triangle third vertex attributes.
     *  @param last Last triangle signal.
     *
     *  @return An initialized triangle setup input.
     *
     */
     
    TriangleSetupInput(u32bit id, QuadFloat *attrib1, QuadFloat *attrib2, 
        QuadFloat *attrib3, bool last);
    
    /**
     *
     *  Gets the triangle identifier.
     *
     *  @return The triangle identifier.
     *
     */

    u32bit getTriangleID();
    
    /**
     *
     *  Gets the triangle vertex attributes.
     *
     *  @param vertex The triangle vertex number (0, 1 or 2) for
     *  which the vertex attributes are requested.
     *
     *  @return A pointer to the triangle vertex attribute array.
     *
     */

    QuadFloat *getVertexAttributes(u32bit vertex);
    
    /**
     *
     *  Sets the triangle as rejected/culled.
     *
     *
     */
    
    void reject();
   
    /**
     *
     *  Get if the triangle has been rejected/culled in a previous
     *  pipeline stage.
     *
     *  @return If the triangle has been rejected.
     *
     */
     
    bool isRejected();
    
    /**
     * 
     *  Check if the last triangle flag is enabled for this triangle.
     *
     *  @return If the last triangle signal is enabled.
     *
     */
    
    bool isLast();

    /**
     *
     *  Sets the triangle as pre-bound triangle.
     *
     */

    void setPreBound();

    /**
     *
     *  Returns if pre-bound triangle.
     *
     *  @return If pre-bound triangle.
     */
     
    bool isPreBound();

    /**
     *
     *  Sets the triangle ID in Rasterizer emulator.
     *
     *  @param setupID The triangle ID in Rasterizer emulator.
     *
     */

    void setSetupID(u32bit setupID);

    /**
     *
     *  Gets the triangle ID in Rasterizer emulator.
     *
     *  @return The triangle ID
     *
     */

    u32bit getSetupID();
 
};

} // namespace gpu3d

#endif

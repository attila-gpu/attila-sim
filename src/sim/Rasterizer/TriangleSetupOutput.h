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
 * $RCSfile: TriangleSetupOutput.h,v $
 * $Revision: 1.4 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:58 $
 *
 * Triangle Setup Output definition file. 
 *
 */

/**
 *
 *  @file TriangleSetupOutput.h
 *
 *  This file defines the Triangle Setup Output class.
 *
 *  This class is used to send setup triangles from the
 *  Triangle Setup box to the Fragment Generator box.
 *
 */
 
#ifndef _TRIANGLESETUPOUTPUT_

#define _TRIANGLESETUPOUTPUT_

#include "support.h"
#include "GPUTypes.h"
#include "DynamicObject.h"

namespace gpu3d
{

/**
 *
 *  This class defines objects that carry the information
 *  about setup triangles from the Triangle Setup box
 *  to the Fragment Generator box.
 *
 *  This class inherits from the DynamicObject class that
 *  offers basic dynamic memory and signal tracing support
 *  functions.
 *
 */
 
class TriangleSetupOutput : public DynamicObject
{
private:

    u32bit triangleID;      /**<  Triangle identifier (inside the batch).  */
    u32bit triSetupID;      /**<  The setup triangle ID (rasterizer emulator).  */
    bool culled;            /**<  The triangle has been culled.  */
    bool last;              /**<  Last triangle mark.  */

public:

    /**
     *
     *  Triangle Setup Output constructor.
     *
     *  Creates and initializes a triangle setup output.
     *
     *  @param id The triangle identifier (inside the batch).
     *  @param setupID The setup triangle identifier for the
     *  triangle inside the Rasterizer Emulator.
     *  @param last Triangle marked as last triangle.
     *
     *  @return An initialized triangle setup output.
     *
     */
     
    TriangleSetupOutput(u32bit id, u32bit setupID, bool last);
    
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
     *  Gets the setup triangle identifier.
     *
     *  @return The setup triangle identifier.
     *
     */

    u32bit getSetupTriangleID();

    /**
     *
     *  Gets if the triangle has been culled in primitive assembly
     *  or triangle setup.
     *
     *  @param If the triangle has been culled.
     *
     */
     
    bool isCulled();
    
    /**
     *
     *  Gets if the triangle is marked as last triangle.
     *
     *  @param If the triangle is marked as the last one in the batch.
     *
     */
     
    bool isLast();

};

} // namespace gpu3d

#endif

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
 * $RCSfile: BlendOperation.h,v $
 * $Revision: 1.2 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:48 $
 *
 * Blend Operation definition file. 
 *
 */
 
/**
 *
 *  @file BlendOperation.h 
 *
 *  This file defines the Blend Operation class.
 *
 *  This class defines the blend operations started in the
 *  ColorWrite box that are sent through the blend unit (signal).
 *
 */

#include "DynamicObject.h"
 
#ifndef _BLENDOPERATION_

#define _BLENDOPERATION_

namespace gpu3d
{

/**
 *
 *  This class stores the information about blend operations
 *  issued to the blend unit in the Color Write box.  The objects
 *  of this class circulate through the Blend signal to simulate
 *  the blend operation latency.
 *
 *  This class inherits from the DynamicObject class that offers
 *  basic dynamic memory management and statistic gathering capabilities.
 *
 */

class BlendOperation: public DynamicObject
{
private:

    u32bit id;          /**<  Operation identifier.  */
    
public:

    /**
     *
     *  Blend Operation constructor.
     *
     *  @param id The operation identifier.
     *
     *  @return A new Blend Operation object.
     *
     */
     
    BlendOperation(u32bit id);
    
    /**
     *
     *  Gets the blend operation identifier.
     *
     *  @return The blend operation identifier.
     *
     */
     
    u32bit getID();
    
};

} // namespace gpu3d

#endif

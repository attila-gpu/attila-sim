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
 * $RCSfile: PrimitiveAssemblyStateInfo.h,v $
 * $Revision: 1.3 $
 * $Author: cgonzale $
 * $Date: 2005-04-26 17:28:52 $
 *
 * Primitive Assembly State Info definition file.
 *
 */


#ifndef _PRIMITIVEASSEMBLYSTATEINFO_
#define _PRIMITIVEASSEMBLYSTATEINFO_

#include "DynamicObject.h"
#include "PrimitiveAssembly.h"

namespace gpu3d
{
/**
 *
 *  @file PrimitiveAssemblyStateInfo.h
 *
 *  This file defines the Primitive Assembly State Info class.
 *
 */
 
/**
 *
 *  This class defines a container for the state signals
 *  that the Primitive Assembly sends to the Command Processor.
 *
 *  Inherits from Dynamic Object that offers dynamic memory
 *  support and tracing capabilities.
 *
 */


class PrimitiveAssemblyStateInfo : public DynamicObject
{
private:
    
    AssemblyState state;    /**<  The primitive assembly state.  */

public:

    /**
     *
     *  Creates a new PrimitiveAssemblyStateInfo object.
     *
     *  @param state The primitive assembly state carried by this
     *  primitive assembly state info object.
     *
     *  @return A new initialized PrimitiveAssemblyStateInfo object.
     *
     */
     
    PrimitiveAssemblyStateInfo(AssemblyState state);
    
    /**
     *
     *  Returns the primitive assembly state carried by the primitive
     *  assembly state info object.
     *
     *  @return The primitive assembly state carried in the object.
     *
     */
     
    AssemblyState getState();
};

} // namespace gpu3d

#endif

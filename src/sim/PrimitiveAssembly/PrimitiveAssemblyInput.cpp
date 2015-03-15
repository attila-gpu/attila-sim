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
 * $RCSfile: PrimitiveAssemblyInput.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:27 $
 *
 * Primitive Assembly Input definition file.
 *
 */

#include "PrimitiveAssemblyInput.h"

using namespace gpu3d;

/*  Creates a new PrimitiveAssemblyInput.  */
PrimitiveAssemblyInput::PrimitiveAssemblyInput(u32bit ID, QuadFloat *attrib)
{
    /*  Set vertex parameters.  */
    id = ID;
    attributes = attrib;

    setTag("PAsIn");
}

/*  Gets the Primitive Assembly input identifier (vertex index).  */
u32bit PrimitiveAssemblyInput::getID()
{
    return id;
}

/*  Gets the Primitive Assembly input attributes.  */
QuadFloat *PrimitiveAssemblyInput::getAttributes()
{
    return attributes;
}

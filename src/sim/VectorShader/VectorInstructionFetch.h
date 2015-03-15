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
 * $RCSfile:$
 * $Revision:$
 * $Author:$
 * $Date:$
 *
 * Vector Shader Instruction Fetch definition file.
 *
 */

/**
 *
 *  @file VectorInstructioFetch.h
 *
 *  Defines the VectorInstructionFetch class.  The VectorInstructionFetch is a Dynamic Object (signal traceable)
 *  that carries a vector instruction fetch from the Vector Shader fetch stage to the Vector Shader decode stage.  A vector fetch
 *  contains a number of ShaderExecInstruction objects.
 *
 */

#include "GPUTypes.h"
#include "DynamicObject.h"
#include "ShaderExecInstruction.h"

#ifndef _VECTORINSTRUCTIONFETCH_

#define _VECTORINSTRUCTIONFETCH_

namespace gpu3d
{

/** 
 *
 *  Container for the ShaderExecInstruction objects that a vector instruction fetch produces and sends to the decode stage.
 *
 */

    class VectorInstructionFetch : public DynamicObject
    {
    private:

        ShaderExecInstruction **vectorInstruction;     /**<  Pointer to an array of pointers to ShaderExecInstruction objects.  Stores the vector instruction.  */

    public:

        /**
         *
         *  VectorInstructionFetch class constructor.
         *
         *  Creates a new VectorInstructionFetch objects from an array of pointers to ShaderExecInstruction objects that corresponds with a
         *  vector instruction fetch performed in the Vector Shader fetch stage.
         *
         *  @param vectorFetch Pointer to an array of pointers to ShaderExecInstruction objects, corresponding to a vector instruction fetch.
         *
         *  @return A new VectorInstructionFetch object.
         *
         */

        VectorInstructionFetch(ShaderExecInstruction **vectorFetch);

        /**
         *
         *  Get the pointer to the array of pointers to ShaderExecInstruction objects that corresponds with a vector instruction fetch.
         *
         *  @return The vector fetch as a pointer to an array of pointers to ShaderExecInstruction objects.
         *
         */

        ShaderExecInstruction **getVectorFetch();
    };

}

#endif

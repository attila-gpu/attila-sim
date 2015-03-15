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
 * Vector Shader Instruction Fetch implementation file.
 *
 */

/**
 *
 *  @file VectorInstructioFetch.cpp
 *
 *  Implements the VectorInstructionFetch class.  The VectorInstructionFetch is a Dynamic Object (signal traceable)
 *  that carries a vector instruction fetch from the Vector Shader fetch stage to the Vector Shader decode stage.  A vector fetch
 *  contains a number of ShaderExecInstruction objects.
 *
 */


#include "VectorInstructionFetch.h"
#include <cstring>

using namespace gpu3d;

//  VectorShaderInstruction constructor.
VectorInstructionFetch::VectorInstructionFetch(ShaderExecInstruction **vectorFetch)
{
    //  Set pointer to the vector instruction.
    vectorInstruction = vectorFetch;

    //  Get the first vector element shader instruction to retrieve the information required to fill
    //  the dynamic object trace information.
    ShaderInstruction *shInstr = vectorInstruction[0]->getShaderInstruction().getShaderInstruction();

    //  Set color from the opcode of the first vector element instruction.
    setColor(shInstr->getOpcode());

    //  Set instruction disassembled string from first vector instruction element as info.
    //shInstr->disassemble((char *) getInfo());
    strcpy((char *) getInfo(), (char *) vectorFetch[0]->getInfo());

    //  Set dynamic object tag.
    setTag("VecInsF");
}

//  Return the vector fetch array.
ShaderExecInstruction **VectorInstructionFetch::getVectorFetch()
{
    return vectorInstruction;
}


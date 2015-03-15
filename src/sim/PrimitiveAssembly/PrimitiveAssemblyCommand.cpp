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
 * $RCSfile: PrimitiveAssemblyCommand.cpp,v $
 * $Revision: 1.5 $
 * $Author: vmoya $
 * $Date: 2005-11-11 15:41:47 $
 *
 * Primitive Assembly Command class implementation file.
 *
 */

/**
 *
 *  @file PrimitiveAssemblyCommand.h
 *
 *  This file implements the Primitive Assembly Command class.
 *
 *  This class carries commands from the Command Processor to
 *  Primitive Assembly.
 *
 */

#include "PrimitiveAssemblyCommand.h"

using namespace gpu3d;

/*  Primitive Assembly Command constructor.  For PACOM_RESET, PACOM_DRAW,
    and PACOM_END.  */
PrimitiveAssemblyCommand::PrimitiveAssemblyCommand(AssemblyComm comm)
{
    /*  Check the type rasterizer command.  */
    GPU_ASSERT(
        if ((comm != PACOM_RESET) && (comm != PACOM_DRAW) &&
            (comm != PACOM_END))
            panic("PrimitiveAssemblyCommand", "PrimitiveAssemblyCommand", "Illegal primitive assembly command for this constructor.");
    )

    /*  Set the command.  */
    command = comm;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("PAsCom");
}


/*  Primitive Assembly Command constructor.  For PACOM_REG_WRITE.  */
PrimitiveAssemblyCommand::PrimitiveAssemblyCommand(GPURegister rReg, u32bit rSubReg, GPURegData rData)
{
    /*  Set the command.  */
    command = PACOM_REG_WRITE;

    /*  Set the command parameters.  */
    reg = rReg;
    subReg = rSubReg;
    data = rData;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("PAsCom");
}

/*  Returns the primitive assembly command type.  */
AssemblyComm PrimitiveAssemblyCommand::getCommand()
{
    return command;
}


/*  Returns the register identifier to write/read.  */
GPURegister PrimitiveAssemblyCommand::getRegister()
{
    return reg;
}

/*  Returns the register subregister number to write/read.  */
u32bit PrimitiveAssemblyCommand::getSubRegister()
{
    return subReg;
}

/*  Returns the register data to write.  */
GPURegData PrimitiveAssemblyCommand::getRegisterData()
{
    return data;
}

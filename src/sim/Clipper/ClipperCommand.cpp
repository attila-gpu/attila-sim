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
 * $RCSfile: ClipperCommand.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2005-11-11 15:41:47 $
 *
 * Clipper Command class implementation file.
 *
 */

/**
 *
 *  @file ClipperCommand.cpp
 *
 *  This file implements the Clipper Command class.
 *
 *  This class objects are used to carry command from the
 *  Command Processor to the Clipper unit.
 *
 */


#include "ClipperCommand.h"

using namespace gpu3d;

/*  Clipper Command constructor.  For CLPCOM_RESET, CLPCOM_START,
    and CLPCOM_END.  */
ClipperCommand::ClipperCommand(ClipCommand comm)
{
    /*  Check the type rasterizer command.  */
    GPU_ASSERT(
        if ((comm != CLPCOM_RESET) && (comm != CLPCOM_START) &&
            (comm != CLPCOM_END))
            panic("ClipperCommand", "ClipperCommand", "Illegal primitive assembly command for this constructor.");
    )

    /*  Set the command.  */
    command = comm;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("ClpCom");
}


/*  Clipper Command constructor.  For CLPCOM_REG_WRITE.  */
ClipperCommand::ClipperCommand(GPURegister rReg, u32bit rSubReg, GPURegData rData)
{
    /*  Set the command.  */
    command = CLPCOM_REG_WRITE;

    /*  Set the command parameters.  */
    reg = rReg;
    subReg = rSubReg;
    data = rData;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("ClpCom");
}

/*  Returns the clipper command type.  */
ClipCommand ClipperCommand::getCommand()
{
    return command;
}


/*  Returns the register identifier to write/read.  */
GPURegister ClipperCommand::getRegister()
{
    return reg;
}

/*  Returns the register subregister number to write/read.  */
u32bit ClipperCommand::getSubRegister()
{
    return subReg;
}

/*  Returns the register data to write.  */
GPURegData ClipperCommand::getRegisterData()
{
    return data;
}

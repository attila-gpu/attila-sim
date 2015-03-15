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
 * $RCSfile: RasterizerCommand.cpp,v $
 * $Revision: 1.7 $
 * $Author: jroca $
 * $Date: 2007-01-19 19:33:59 $
 *
 * Rasterizer Command class implementation file.
 *
 */

#include "RasterizerCommand.h"

using namespace gpu3d;

/*  Rasterizer Command constructor.  For RSCOM_RESET, RSCOM_DRAW,
    RSCOM_CLEAR_COLOR_BUFFER, RSCOM_CLEAR_ZSTENCIL_BUFFER, RSCOM_SWAP, RSCOM_END.  */
RasterizerCommand::RasterizerCommand(RastComm comm)
{
    /*  Check the type rasterizer command.  */
    GPU_ASSERT(
        if ((comm != RSCOM_RESET) && (comm != RSCOM_SWAP) && (comm != RSCOM_FRAME_CHANGE) && 
            (comm != RSCOM_DUMP_COLOR) && (comm != RSCOM_DUMP_DEPTH) && (comm != RSCOM_DUMP_STENCIL) &&
            (comm != RSCOM_BLIT) && (comm != RSCOM_FLUSH) && (comm != RSCOM_DRAW) && 
            (comm != RSCOM_SAVE_STATE) && (comm != RSCOM_RESTORE_STATE) &&  (comm != RSCOM_RESET_STATE) &&
            (comm != RSCOM_CLEAR_COLOR_BUFFER) && (comm!= RSCOM_CLEAR_ZSTENCIL_BUFFER) && (comm != RSCOM_END))
            panic("RasterizerCommand", "RasterizerCommand", "Illegal rasterizer command for this constructor.");
    )

    /*  Set the command.  */
    command = comm;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("RasCom");
}


/*  Rasterizer Command constructor.  For RSCOM_REG_WRITE.  */
RasterizerCommand::RasterizerCommand(GPURegister rReg, u32bit rSubReg, GPURegData rData)
{
    /*  Set the command.  */
    command = RSCOM_REG_WRITE;

    /*  Set the command parameters.  */
    reg = rReg;
    subReg = rSubReg;
    data = rData;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("RasCom");
}

/*  Returns the rasterizer command type.  */
RastComm RasterizerCommand::getCommand()
{
    return command;
}


/*  Returns the register identifier to write/read.  */
GPURegister RasterizerCommand::getRegister()
{
    return reg;
}

/*  Returns the register subregister number to write/read.  */
u32bit RasterizerCommand::getSubRegister()
{
    return subReg;
}

/*  Returns the register data to write.  */
GPURegData RasterizerCommand::getRegisterData()
{
    return data;
}

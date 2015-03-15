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
 * $RCSfile: StreamerCommand.cpp,v $
 * $Revision: 1.4 $
 * $Author: vmoya $
 * $Date: 2005-11-11 15:41:48 $
 *
 * StreamerCommand class implementation file.
 *
 */

#include "StreamerCommand.h"

using namespace gpu3d;

/*  Streamer Command Constructor for reset, start and end streaming
    commands.  */
StreamerCommand::StreamerCommand(StreamComm comm)
{
    /*  Check the type of Streamer Command to create.  */
    GPU_ASSERT(
        if ((comm != STCOM_RESET) && (comm != STCOM_START) &&
            (comm != STCOM_END))
            panic("StreamerCommand", "StreamerCommand", "Incorrect Streamer command.");
    )

    command = comm;

    /*  Set object color for tracing.  */
    setColor(command);

    setTag("stCom");
}


/*  Streamer Command constructor for register writes.  */
StreamerCommand::StreamerCommand(GPURegister reg, u32bit subReg, GPURegData regData)
{
    /*  Create a STCOM_REG_WRITE command.  */
    command = STCOM_REG_WRITE;

    /*  Set command parameters.  */
    streamReg = reg;
    streamSubReg = subReg;
    data = regData;

    /*  Set object color for tracing.  */
    setColor(command);

    setTag("stCom");
}

/*  Gets the Streamer command type.  */
StreamComm StreamerCommand::getCommand()
{
    return command;
}


/*  Gets the Streamer destination register of the command.  */
GPURegister StreamerCommand::getStreamerRegister()
{
    return streamReg;
}

/*  Gets the Streamer destination register subregister of the command.  */
u32bit StreamerCommand::getStreamerSubRegister()
{
    return streamSubReg;
}

/*  Gets the data to write to the Streamer register.  */
GPURegData StreamerCommand::getRegisterData()
{
    return data;
}


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
 * $RCSfile: ShaderDecodeCommand.cpp,v $
 * $Revision: 1.3 $
 * $Author: vmoya $
 * $Date: 2005-05-02 10:26:30 $
 *
 * Shader Decode Command.
 *
 */


/**
 *
 *  @file ShaderDecodeCommand.cpp
 *
 *  This file store the implementation for the ShaderDecodeCommand class.
 *  The ShaderDecodeCommand class stores information about commands sent
 *  to the Fetch box from the Decode/Execute box.
 *
 */

#include "ShaderDecodeCommand.h"
#include <stdio.h>

using namespace gpu3d;

//  Creates the DecodeCommand initialized.
ShaderDecodeCommand::ShaderDecodeCommand(DecodeCommand comm, u32bit threadID, u32bit newPC):
command(comm), numThread(threadID), PC(newPC)
{
    //  Set color for tracing.
    setColor(command);

    //  Set information string.
    switch(command)
    {
        case UNBLOCK_THREAD:
            sprintf((char *) getInfo(), "UNBLOCK_THREAD ThID %03d PC %04x ", numThread, PC);
            break;
        case BLOCK_THREAD:
            sprintf((char *) getInfo(), "BLOCK_THREAD ThID %03d PC %04x ", numThread, PC);
            break;
        case END_THREAD:
            sprintf((char *) getInfo(), "END_THREAD ThID %03d PC %04x ", numThread, PC);
            break;
        case REPEAT_LAST:
            sprintf((char *) getInfo(), "REPEAT_LAST ThID %03d PC %04x ", numThread, PC);
            break;
        case NEW_PC:
            sprintf((char *) getInfo(), "NEW_PC ThID %03d PC %04x ", numThread, PC);
            break;
        default:
            break;
    }
    
    setTag("shDCom");
}

//  Returns the type of the command.
DecodeCommand ShaderDecodeCommand::getCommandType()
{
    return command;
}

//  Returns the size of the shader program code in bytes.
u32bit ShaderDecodeCommand::getNewPC()
{
    return PC;
}

//  Returns the thread number for which this command is send.
u32bit ShaderDecodeCommand::getNumThread()
{
    return numThread;
}

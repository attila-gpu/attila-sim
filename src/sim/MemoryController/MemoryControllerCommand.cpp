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
 */

#include "MemoryControllerCommand.h"

using namespace gpu3d;

MemoryControllerCommand::MemoryControllerCommand(MCCommand command, GPURegister reg, 
                                                 GPURegData data) :
   command(command), reg(reg), data(data)
{}

   MemoryControllerCommand::MemoryControllerCommand(MCCommand cmd) :command(cmd)
{}


MemoryControllerCommand* MemoryControllerCommand::createRegWrite(
                                                  GPURegister reg, GPURegData data)
{
    return new MemoryControllerCommand(MCCOM_REG_WRITE, reg, data);
}

MemoryControllerCommand* MemoryControllerCommand::createLoadMemory()
{
    return new MemoryControllerCommand(MCCOM_LOAD_MEMORY);
}

MemoryControllerCommand* MemoryControllerCommand::createSaveMemory()
{
    return new MemoryControllerCommand(MCCOM_SAVE_MEMORY);
}


MCCommand MemoryControllerCommand::getCommand() const
{
    return command;
}


GPURegister MemoryControllerCommand::getRegister() const
{
    return reg;
}

GPURegData MemoryControllerCommand::getRegisterData() const
{
    return data;
}

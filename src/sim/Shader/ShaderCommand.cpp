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
 * $RCSfile: ShaderCommand.cpp,v $
 * $Revision: 1.7 $
 * $Author: vmoya $
 * $Date: 2005-12-16 12:35:21 $
 *
 * Shader Command.
 *
 */


/**
 *
 *  @file ShaderCommand.cpp
 *
 *  This file store the implementation for the ShaderCommand class.
 *  The ShaderCommand class stores information about commands sent
 *  to a Shader Unit from a Command Processor.
 *
 */

#include "ShaderCommand.h"
#include <cstring>

using namespace gpu3d;

/*  Creates the ShaderCommand and sets the command type.  */
ShaderCommand::ShaderCommand(Command com)
{
    command = com;     /*  Set the command type.  */

    /*  Default (empty) values for the command parameters.*/
    code = NULL;
    sizeCode = 0;
    values = NULL;
    numValues = 0;
    firstAddr = 0;
    iniPC = 0;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("ShCom");
}

/*  Craete an initialiced ShaderCommand of type NEW_PROGRAM.  */
ShaderCommand::ShaderCommand(u32bit pc, u8bit *progCode, u32bit size)
{
    command = LOAD_PROGRAM;
    code = new u8bit[size];
    memcpy(code, progCode, size);
    sizeCode = size;
    loadPC = pc;

    /*  Default (empty) values for the unused parameters.  */
    values = NULL;
    numValues = 0;
    firstAddr = 0;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("ShCom");
}

/*  Creates an initialized ShaderCommand of  type PARAM_WRITE.  */
ShaderCommand::ShaderCommand(QuadFloat *buffer, u32bit first, u32bit size)
{
    command = PARAM_WRITE;
    values = buffer;
    numValues = size;
    firstAddr = first;

    /*  Default (empty) values for the unused parameters.  */
    code = NULL;
    sizeCode = 0;
    iniPC = 0;

    /*  Set color for tracing.  */
    setColor(command);

    setTag("ShCom");
}

/*  Creates an initialized ShaderCommand of  type SET_INIT_PC.  */
ShaderCommand::ShaderCommand(Command com, ShaderTarget shTarget, u32bit data)
{
    /*  Default (empty) values for the unused parameters.  */
    code = NULL;
    sizeCode = 0;
    values = NULL;
    numValues = 0;
    firstAddr = 0;
    iniPC = 0;
    thResources = 0;

    /*  Select command.  */
    switch (com)
    {
        case SET_INIT_PC:

            /*  Configure command for setting the shader program initial PC.  */
            command = SET_INIT_PC;
            iniPC = data;
            target = shTarget;

            break;

        case SET_THREAD_RES:

            /*  Configure command for setting the per thread resource usage.  */
            command = SET_THREAD_RES;
            thResources = data;
            target = shTarget;

            break;

        default:
            panic("ShaderCommand", "ShaderCommand", "Unsupported command for single parameter 32 bit unsigned constructor.");
            break;
    }

    /*  Set color for tracing.  */
    setColor(command);

    setTag("ShCom");
}

/*  Creates an initialized ShaderCommand of type SET_IN_ATTR or SET_OUT_ATTR.  */
ShaderCommand::ShaderCommand(Command com, u32bit attr, bool active)
{
    GPU_ASSERT(
        if ((com != SET_IN_ATTR) && (com != SET_OUT_ATTR))
            panic("ShaderCommand", "ShaderCommand", "Unsupported command for configuring input/output shader attributes constructor.");
    )

    /*  Default (empty) values for the unused parameters.  */
    code = NULL;
    sizeCode = 0;
    values = NULL;
    numValues = 0;
    firstAddr = 0;
    iniPC = 0;
    thResources = 0;
    
    /*  Set command.  */
    command = com;

    /*  Shader input/output attribute to configure.  */
    attribute = attr;

    /*  Set if the input/output attribute is enabled.  */
    attrActive = active;

    /*  Set color for signal tracing.  */
    setColor(command);

    setTag("ShCom");
}

/*  Creates an initialized ShaderCommand of type TX_REG_WRITE.  */
ShaderCommand::ShaderCommand(GPURegister reg, u32bit subReg, GPURegData data)
{

    /*  Default (empty) values for the unused parameters.  */
    code = NULL;
    sizeCode = 0;
    values = NULL;
    numValues = 0;
    firstAddr = 0;
    iniPC = 0;
    thResources = 0;

    /*  Command for texture unit register write.  */
    command = TX_REG_WRITE;

    /*  Set register and data to write.  */
    txReg = reg;
    txSubReg = subReg;
    txData = data;

    /*  Set color for signal tracing.  */
    setColor(command);

    setTag("ShCom");
}

/*  Creates an initialized ShaderCommand of the type SET_MULTISAMPLING.  */
ShaderCommand::ShaderCommand(bool enabled)
{
    /*  Default (empty) values for the unused parameters.  */
    code = NULL;
    sizeCode = 0;
    values = NULL;
    numValues = 0;
    firstAddr = 0;
    iniPC = 0;
    thResources = 0;
    txSubReg = 0;

    /*  Command for multisampling register write.  */
    command = SET_MULTISAMPLING;

    /*  Set register and data to write.  */
    multiSampling = enabled;

    /*  Set color for signal tracing.  */
    setColor(command);

    setTag("ShCom");
}

/*  Creates an initialized ShaderCommand of the type SET_MSAA_SAMPLES.  */
ShaderCommand::ShaderCommand(u32bit samples)
{
    /*  Default (empty) values for the unused parameters.  */
    code = NULL;
    sizeCode = 0;
    values = NULL;
    numValues = 0;
    firstAddr = 0;
    iniPC = 0;
    thResources = 0;
    txSubReg = 0;

    /*  Command for multisampling register write.  */
    command = SET_MSAA_SAMPLES;

    /*  Set register and data to write.  */
    msaaSamples = samples;

    /*  Set color for signal tracing.  */
    setColor(command);

    setTag("ShCom");
}

//  Destructor.
ShaderCommand::~ShaderCommand()
{
    //  Check if this Shader Command allocated memory for the shader code (LOAD_PROGRAM command).
    if (code != NULL)
        delete [] code;
}


/*  Returns the type of the command.  */
Command ShaderCommand::getCommandType() const
{
    return command;
}

/*  Returns a pointer to the shader program code.  */
u8bit *ShaderCommand::getProgramCode()
{

    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != LOAD_PROGRAM)
            panic("ShaderCommand", "getProgramCode", "Unsuported command parameters for this command type.");
    )

    return code;
}

/*  Returns the size of the shader program code in bytes.  */
u32bit ShaderCommand::getProgramCodeSize() const
{
    return sizeCode;
}

/*  Returns the reference to the array of QuadFloats.  */
QuadFloat *ShaderCommand::getValues()
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != PARAM_WRITE)
            panic("ShaderCommand", "setValues", "Unsupported command parameter for this command type.");
    )

    return values;
}

/*  Returns the size of the array of QuadFloats.  */
u32bit ShaderCommand::getNumValues() const
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != PARAM_WRITE)
            panic("ShaderCommand", "setValues", "Unsupported command parameter for this command type.");
    )

    return numValues;
}

/*  Returns the first element to write in parameter memory.  */
u32bit ShaderCommand::getParamFirstAddress() const
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != PARAM_WRITE)
            panic("ShaderCommand", "setParamFirstAddress", "Unsupported command parameter for this command type.");
    )

    return firstAddr;
}

/*  Return the new shader program initial PC.  */
u32bit ShaderCommand::getInitPC() const
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != SET_INIT_PC)
            panic("ShaderCommand", "getInitPC", "Unsupported command parameter for this command type.");
    )

    return iniPC;
}

/*  Return the per thread resource usage.  */
u32bit ShaderCommand::getThreadResources() const
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != SET_THREAD_RES)
            panic("ShaderCommand", "getThreadResources", "Unsupported command parameter for this command type.");
    )

    return thResources;
}

//  Return the address in shader instruction memory where to load the shader program.
u32bit ShaderCommand::getLoadPC() const
{
    GPU_ASSERT(
        if (command != LOAD_PROGRAM)
            panic("ShaderCommand", "getThreadResources", "Unsupported command parameter for this command type.");
    )
    
    return loadPC;
}

//  Return the shader target for which the shader command was issued.
ShaderTarget ShaderCommand::getShaderTarget() const
{

    GPU_ASSERT(
        if ((command != SET_INIT_PC) && (command != SET_THREAD_RES))
            panic("ShaderCommand", "getShaderTarget", "Unsupported command parameter for this command type.");
    )

    return target;
}


/*  Return the shader input/output attribute to configure.  */
u32bit ShaderCommand::getAttribute() const
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if ((command != SET_OUT_ATTR) && (command != SET_IN_ATTR))
            panic("ShaderCommand", "getAttribute", "Unsupported command parameter for this command type.");
    )

    return attribute;
}

/*  Return if the shader input/output is active.  */
bool ShaderCommand::isAttributeActive() const
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if ((command != SET_OUT_ATTR) && (command != SET_IN_ATTR))
            panic("ShaderCommand", "isAttributeActive", "Unsupported command parameter for this command type.");
    )

    return attrActive;
}

/*  Returns the texture unit register to write.  */
GPURegister ShaderCommand::getRegister() const
{
    GPU_ASSERT(
        if (command != TX_REG_WRITE)
            panic("ShaderCommand", "getRegister", "Unsupported command parameter for this command type.");
    )

    return txReg;
}

/*  Returns the texture unit register data to write.  */
GPURegData ShaderCommand::getRegisterData() const
{
    GPU_ASSERT(
        if (command != TX_REG_WRITE)
            panic("ShaderCommand", "getRegisterData", "Unsupported command parameter for this command type.");
    )

    return txData;
}

/*  Returns the texture unit subregister to write.  */
u32bit ShaderCommand::getSubRegister() const
{
    GPU_ASSERT(
        if (command != TX_REG_WRITE)
            panic("ShaderCommand", "getSubRegister", "Unsupported command parameter for this command type.");
    )

    return txSubReg;
}

/*  Returns if multisampling enabled/disabled for SET_MULTISAMPLING commands.  */
bool ShaderCommand::multisamplingEnabled() const
{
    return multiSampling;
}

/*  Returns the number of MSAA sampels for SET_MSAA_SAMPLES commands.  */
u32bit ShaderCommand::samplesMSAA() const
{
    return msaaSamples;
}

/*  Sets the pointer to a shader program code and the code aize.  */
void ShaderCommand::setProgramCode(u8bit *progCode, u32bit size)
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != LOAD_PROGRAM)
            panic("ShaderCommand", "setProgramCode", "Unsuported command parameters for this command type.");
    )

    code = progCode;
    sizeCode = size;
}

/*  Sets pointer to the array of values and its size.  */
void ShaderCommand::setValues(QuadFloat *valueArray, u32bit size)
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != PARAM_WRITE)
            panic("ShaderCommand", "setValues", "Unsupported command parameters for this command.");
    )

    values = valueArray;
    numValues = size;
}

/*  Sets the address for the first element to write in parameter memory.  */
void ShaderCommand::setParamFirstAddress(u32bit first)
{
    /*  Check correct command type.  */
    GPU_ASSERT(
        if (command != PARAM_WRITE)
            panic("ShaderCommand", "setParamFirstAddress", "Unsupported command parameters for this command.");
    )

    firstAddr = first;
}


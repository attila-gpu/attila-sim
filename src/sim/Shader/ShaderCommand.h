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
 * $RCSfile: ShaderCommand.h,v $
 * $Revision: 1.7 $
 * $Author: vmoya $
 * $Date: 2005-12-16 12:35:21 $
 *
 * Shader Command.
 *
 */

/**
 *
 *  @file ShaderCommand.h
 *
 *  Defines ShaderCommand class.   This class stores commands
 *  sent to the Shader Unit from a Command Processor.
 *
 */

#include "GPUTypes.h"
#include "GPU.h"
#include "DynamicObject.h"

#ifndef _SHADERCOMMAND_

#define _SHADERCOMMAND_

namespace gpu3d
{

/**  Commands that can recieve the Shader.  */
enum Command
{
    RESET,              /**<  Reset the shader.  */
    PARAM_WRITE,        /**<  Request to write the parameter (constant) bank.  */
    LOAD_PROGRAM,       /**<  Request to load a new shader program.  */
    SET_INIT_PC,        /**<  Request to set initial PC.  */
    SET_THREAD_RES,     /**<  Request to set the per thread resource usage.  */
    SET_IN_ATTR,        /**<  Request to configure shader input attributes.  */
    SET_OUT_ATTR,       /**<  Request to configure shader output attributes.  */
    SET_MULTISAMPLING,  /**<  Request to set the MSAA enabled flag. (used for microtriangle stamps).  */
    SET_MSAA_SAMPLES,   /**<  Request to set the MSAA number of samples. (used for microtriangle stamps).  */
    TX_REG_WRITE        /**<  Texture unit register write.  */
};

/**
 *
 *  Defines a Shader Command from the Command Processor to the Shader.
 *
 *  This class stores information about commands sent to a Shader Unit
 *  from a Command Processor.
 *  The class inherits from DynamicObject class that offers dynamic
 *  memory management and trace support.
 *
 */

class ShaderCommand : public DynamicObject
{

private:

    Command command;    /**<  Type of the command.  */
    u8bit *code;        /**<  For NEW_PROGRAM command, program code.  */
    u32bit sizeCode;    /**<  For NEW_PROGRAM command, size of program code.  */
    QuadFloat *values;  /**<  For PARAM_WRITE array of QuadFloats.  */
    u32bit firstAddr;   /**<  For PARAM_WRITE address of the first register/position to write.  */
    u32bit numValues;   /**<  For PARAM_WRITE number of values in the QuadFloat array.  */
    u32bit iniPC;       /**<  For SET_INIT_PC, new initial PC for the shader program.  */
    u32bit thResources; /**<  For SET_THREAD_RES, per thread resource usage.  */
    u32bit attribute;   /**<  For SET_IN_ATTR, SET_OUT_ATTR, which shader input/output is going to be configured.  */
    bool attrActive;    /**<  For SET_IN_ATTR, SET_OUT_ATTR, if the shader input/output is active for the current program.  */
    bool multiSampling; /**<  For SET_MULTISAMPLING, if multisampling is enabled. (used for microtriangle stamps) */
    u32bit msaaSamples; /**<  For SET_MSAA_SAMPLES, the number of MSAA samples. (used for microtriangle stamps) */
    GPURegister txReg;  /**<  For TX_REG_WRITE, the texture unit register to write.  */
    u32bit txSubReg;    /**<  For TX_REG_WRITE, the registe subregister to write.  */
    GPURegData txData;  /**<  For TX_REG_WRITE, the texture unit register data to write.  */
    ShaderTarget target;    /**<  For SET_INIT_PC and SET_THREAD_RES the shader target for which the command is issued.  */
    u32bit loadPC;          /**<  Direction in the shader instruction memory where to load the shader program.  */
    
public:

    /**
     *
     *  ShaderCommand constructor.
     *
     *  Creates a ShaderCommand without any parameters.
     *
     *  @param comm  Command type for the ShaderCommand.
     *  @return A ShaderCommand object.
     *
     */

    ShaderCommand(Command comm);

    /**
     *
     *  ShaderCommand constructor.
     *
     *  Creates a ShaderCommand of type NEW_PROGRAM.
     *
        @param pc Direction in the shader instruction memory where to load the shader program.
     *  @param progCode Pointer to a shader program in binary format.
     *  @param size Size of the shader program (bytes).
     *  @return A ShaderCommand object of type NEW_PROGRAM.
     *
     */

    ShaderCommand(u32bit pc, u8bit *progCode, u32bit size);

    /**
     *
     *  ShaderCommand constructor.
     *
     *  Creates a ShaderCommand of type PARAM_WRITE.
     *
     *  @param input Pointer to an array of QuadFloats that stores the
     *  values to write in the parameter bank/memory.
     *  @param firstAddr Address/position of the first element
     *  to write in the parameter bank/memory.
     *  @param inputSize Number of elements in the array.
     *  @return A ShaderCommand object of type PARAM_WRITE.
     *
     */

    ShaderCommand(QuadFloat *input, u32bit firstAddr, u32bit inputSize);

    /**
     *
     *  Shader Command constructor.
     *
     *  Creates a ShaderCommand for commands with a single unsigned 32 bit parameter.
     *
     *  @param com Shader command (SET_INI_PC, SET_THREAD_RES).
     *  @param target Shader target for which the command is applied.
     *  @param data Parameter of the shader command
     *
     *  @return A ShaderCommand object.
     *
     */

    ShaderCommand(Command com, ShaderTarget target, u32bit data);

    /**
     *
     *  Shader Command constructor.
     *
     *  Creates a ShaderCommand of type SET_IN_ATTR or SET_OUT_ATTR.
     *
     *  @param com Shader command (SET_IN_ATTR or SET_OUT_ATTR).
     *  @param attrib The shader output attribute to configure.
     *  @param active If the shader output attribute is going to be written.
     *
     *  @return A ShaderCommand object of type SET_IN_ATTR or SET_OUT_ATTR.
     *
     */

    ShaderCommand(Command com, u32bit attr, bool active);

    /**
     *
     *  Shader Command constructor.
     *
     *  Creates a ShaderCommand of type TX_REG_WRITE.
     *
     *  @param reg Register of the Texture Unit to write to.
     *  @param subReg Subregister of the register to write to.
     *  @param data Data to write in the Texture Unit register.
     *
     *  @return A ShaderCommand object of type TX_REG_WRITE.
     *
     */

    ShaderCommand(GPURegister reg, u32bit subReg, GPURegData data);

    /**
     *
     *  Shader Command constructor.
     *
     *  Creates a ShaderCommand of the type SET_MULTISAMPLING. (used for microtriangle stamps).
     *
     *  @param enabled Multisampling is enabled.
     *
     *  @return A ShaderCommand object of type SET_MULTISAMPLING.
     *
     */

    ShaderCommand(bool enabled);

    /**
     *
     *  Shader Command constructor.
     *
     *  Creates a ShaderCommand of the type SET_MSAA_SAMPLES. (used for microtriangle stamps).
     *
     *  @param samples The number of MSAA samples.
     *
     *  @return A ShaderCommand object of type SET_MSAA_SAMPLES.
     *
     */

    ShaderCommand(u32bit samples);


    /**
     *
     *  ShaderComamnd destructor.
     *
     */
     
    ~ShaderCommand();
    
    /**
     *
     *  Returns the type of the command.
     *
     *  @return  Type of the command.
     *
     */

    Command getCommandType() const;

    /**
     *
     *  Returns the pointer to the shader program code.
     *
     *  @return A pointer to the shader program code.
     *
     */

    u8bit *getProgramCode();

    /**
     *
     *  Returns the size of the shader program code in bytes.
     *
     *  @return Shader program code size.
     *
     */

    u32bit getProgramCodeSize() const;

    /**
     *
     *  Returns a pointer to an array of QuadFloat values.
     *
     *  @return A pointer to an array of QuadFloats.
     *
     */

    QuadFloat *getValues();

    /**
     *
     *  Returns the number of elements in the QuadFloat array.
     *
     *  @return Number of values in the QuadFloat array.
     *
     */

    u32bit getNumValues() const;

    /**
     *
     *  Returns the address of the first element to write
     *  in parameter memory.
     *
     *  @return Address of the first element to write.
     *
     */

    u32bit getParamFirstAddress() const;

    /**
     *
     *  Returns the new initial PC for the shader program.
     *
     *  @return Initial shader program PC.
     *
     */

    u32bit getInitPC() const;

    /**
     *
     *  Returns the per thread resource usage to set.
     *
     *  @return Per shader thread resource usage.
     *
     */

    u32bit getThreadResources() const;

    /**
     *
     *  Returns the address in the shader instruction memory where to load the shader program.
     *
     *  @return Address in the shader instruction memory where to load the shader program.
     *
     */
    u32bit getLoadPC() const;
    
    /**
     *
     *  Returns the shader target for the command.
     *
     *  @return The shader target for which the command was issued.
     *
     */
     
     ShaderTarget getShaderTarget() const;
    
    /**
     *
     *  Returns the shader input/output attribute to be configured.
     *
     *  @return The shader input/output to configure.
     *
     */

    u32bit getAttribute() const;

    /**
     *
     *  Returns if the shader input/output is active in the current shader program.
     *
     *  @return If the shader input/output is active
     *
     */

    bool isAttributeActive() const;

    /**
     *
     *  Returns the texture unit register to write to.
     *
     *  @return The register identifier to write to.
     *
     */

    GPURegister getRegister() const;

    /**
     *
     *  Returns the texture unit register subregister to write.
     *
     *  @return The subregister to write to.
     *
     */

    u32bit getSubRegister() const;

    /**
     *
     *  Returns the data to write in the texture unit register.
     *
     *  @return Data to write into the texture unit register.
     *
     */

    GPURegData getRegisterData() const;

    /**
     *
     *  Returns if multisampling enabled/disabled for SET_MULTISAMPLING commands
     *
     *  @return true if multisampling enabled.
     *
     */

    bool multisamplingEnabled() const;

    /**
     *
     *  Returns the number of MSAA sampels for SET_MSAA_SAMPLES commands
     *
     *  @return true if multisampling enabled.
     *
     */

    u32bit samplesMSAA() const;

    /**
     *
     *  Stores the pointer a shader program code and its size.
     *
     *  @param code  Pointer to a shader program code in binary format.
     *  @param size  Size of the shader program code in bytes.
     *
     */

    void setProgramCode(u8bit *code, u32bit size);

    /**
     *
     *  Stores a pointer to an array of QuadFloat values and
     *  the size of the array.
     *
     *  @param values  Pointer to an array of QuadFloats.
     *  @param size  Size of the array of QuadFloats.
     *
     */

     void setValues(QuadFloat *values, u32bit size);

    /**
     *
     *  Sets the address for the first element to write in the
     *  parameter memory.
     *
     *  @param firstAddress  Address of the first element to write
     *  in the parameter memory.
     *
     */

     void setParamFirstAddress(u32bit firstAddress);

};

} // namespace gpu3d

#endif

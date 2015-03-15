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

#ifndef ACD_SHADERPROGRAM
    #define ACD_SHADERPROGRAM

#include "ACDTypes.h"
#include <ostream>

namespace acdlib
{

/**
 * Common interface for shader programas (vertex,fragment and geometry shader programs)
 *
 * @author Carlos González Rodríguez (cgonzale@ac.upc.edu)
 * @date 02/27/2007
 */
class ACDShaderProgram
{
public:

    /**
     * Sets the Attila bytecode for this program
     */
    virtual void setCode(const acd_ubyte* attilaByteCode, acd_uint sizeInBytes) = 0;

    /**
     *
     *  Set the Shader Program using a shader program written in ATTILA Shader Assembly.
     *
     *  @param attilaASM Pointer to the shader program assembly code.
     *
     */

    virtual void setProgram(acd_ubyte *attilaASM) = 0;
         
    /**
     * Gets the bytecode of this program (previously set with setCode)
     */
    virtual const acd_ubyte* getCode() const = 0;

    /**
     * Gets the size in bytes of the program
     */
    virtual acd_uint getSize() const = 0;

    /**
     * Sets the value of a constant register exposed by the shader architecture
     *
     * @param index constant register index
     * @param vect4 vector of 4 floats to write into a constant register
     */
    virtual void setConstant(acd_uint index, const acd_float* vect4) = 0;

    /**
     * Gets the current value of a constant register exposed by the shader architecture
     *
     * @param index constant register index
     * @retval vect4 4-float out with the values of the constant register
     */
    virtual void getConstant(acd_uint index, acd_float* vect4) const = 0;

    /**
     * Sets if a texture unit is being used in that shader.
     *
     * @param tu Texture Unit number
     * @param usage == 0 Texture Unit not used // != 0 Type of Texture used in that Texture Unit
     */
    virtual void setTextureUnitsUsage(acd_uint tu, acd_enum usage) = 0;

    /**
     * Gets if a texture unit is being used in that shader.
     *
     * @param tu Texture Unit number
     * @retval usage == 0 Texture Unit not used // != 0 Type of Texture used in that Texture Unit
     */
    virtual acd_enum getTextureUnitsUsage(acd_uint tu) = 0;

    /**
     * Sets if a shader have Kil instructions.
     *
     * @param kill?
     */
    virtual void setKillInstructions(acd_bool kill) = 0;

    /**
     * Gets if a shader have Kil instructions.
     *
     * @retval kill?
     */
    virtual acd_bool getKillInstructions() = 0;

    /**
     * Prints the disassembled attila bytecode.
     *
     * @retval    os    The output stream.
     */
    virtual void printASM(std::ostream& os) const = 0;

    /**
     * Prints the attila bytecode in hex format.
     *
     * @retval    os    The output stream.
     */
    virtual void printBinary(std::ostream& os) const = 0;

    /**
     * Prints the program constants
     *
     * @retval os The output stream.
     */
    virtual void printConstants(std::ostream& os) const = 0;

};

}

#endif // ACD_SHADERPROGRAM

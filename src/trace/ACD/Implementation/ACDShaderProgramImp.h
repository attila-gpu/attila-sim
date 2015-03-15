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

#ifndef ACD_SHADERPROGRAM_IMP
    #define ACD_SHADERPROGRAM_IMP

#include "ACDShaderProgram.h"
#include "ACDStoredStateImp.h"
#include "MemoryObject.h"
#include "GPUDriver.h"
#include "StateItem.h"
#include <set>
#include <bitset>

namespace acdlib
{


class ACDShaderProgramImp : public ACDShaderProgram, public MemoryObject
{
public:

    ACDShaderProgramImp();

    //////////////////////////////////////////////////////
    /// Methods required by ACDShaderProgram interface ///
    //////////////////////////////////////////////////////

    virtual void setCode(const acd_ubyte* attilaByteCode, acd_uint sizeInBytes);

    virtual void setProgram(acd_ubyte *attilaASM);
    
    virtual const acd_ubyte* getCode() const;

    virtual acd_uint getSize() const;

    virtual void setConstant(acd_uint index, const acd_float* vect4);

    virtual void getConstant(acd_uint index, acd_float* vect4) const;

    virtual void printASM(std::ostream& os) const;

    virtual void printBinary(std::ostream& os) const;

    virtual void printConstants(std::ostream& os) const;

    ////////////////////////////////////////////////////////
    /// Methods required by MemoryObject derived classes ///
    ////////////////////////////////////////////////////////

    virtual const acd_ubyte* memoryData(acd_uint region, acd_uint& memorySizeInBytes) const;

    virtual const acd_char* stringType() const;

    ~ACDShaderProgramImp();

    ///////////////////////////////////////////////////////
    // Method only available for backend Attila compiler //
    ///////////////////////////////////////////////////////
    void setOptimizedCode(const acd_ubyte* optimizedCode, acd_uint sizeInBytes);

    bool isOptimized() const;

    void updateConstants(GPUDriver* driver, gpu3d::GPURegister constantsType);

    static const acd_uint MAX_SHADER_ATTRIBUTES = (gpu3d::MAX_VERTEX_ATTRIBUTES > gpu3d::MAX_FRAGMENT_ATTRIBUTES? gpu3d::MAX_VERTEX_ATTRIBUTES: gpu3d::MAX_FRAGMENT_ATTRIBUTES);

    void setInputRead(acd_uint inputReg, acd_bool read);

    void setOutputWritten(acd_uint outputReg, acd_bool written);

    void setMaxAliveTemps(acd_uint maxAlive);

    acd_bool getInputRead(acd_uint inputReg) const;

    acd_bool getOutputWritten(acd_uint outputReg) const;

    acd_uint getMaxAliveTemps() const;

    void setTextureUnitsUsage(acd_uint tu, acd_enum usage);

    acd_enum getTextureUnitsUsage(acd_uint tu);

    void setKillInstructions(acd_bool kill);

    acd_bool getKillInstructions();


private:

    enum { CONSTANT_BANK_REGISTERS = 512 };
    
    static const acd_uint MAX_PROGRAM_SIZE = 1024 * 16;

    mutable acd_ubyte* _assembler;
    mutable acd_uint _assemblerSize;

    void _computeASMCode();

    acd_ubyte* _bytecode;
    acd_ubyte* _optBytecode;
    acd_uint _bytecodeSize;
    acd_uint _optBytecodeSize;
    acd_float _constantBank[CONSTANT_BANK_REGISTERS][4];
    StateItem<ACDVector<acd_float,4> > _constantCache[CONSTANT_BANK_REGISTERS];
    acd_uint _lastConstantSet;

    acd_enum _textureUnitUsage[16/*MAX_TEXTURE_UNITS_ARB*/];
    acd_bool _killInstructions;


    std::set<acd_uint> _touched;    

    std::bitset<MAX_SHADER_ATTRIBUTES> _inputsRead;
    std::bitset<MAX_SHADER_ATTRIBUTES> _outputsWritten;
    acd_uint _maxAliveTemps;

};

}

#endif // ACD_SHADERPROGRAM_IMP

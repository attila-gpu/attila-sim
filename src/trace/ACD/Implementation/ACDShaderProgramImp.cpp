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

#include "ACDShaderProgramImp.h"
#include "ACDMacros.h"
#include "GPUDriver.h"

using namespace acdlib;

ACDShaderProgramImp::ACDShaderProgramImp() : _bytecode(0), _bytecodeSize(0), _optBytecode(0), 
        _optBytecodeSize(0), _lastConstantSet(0), _assembler(0), _assemblerSize(0), 
        _maxAliveTemps(0), _killInstructions(false)
{
    defineRegion(0);
    postReallocate(0);
    
    //  Initialize the constant bank.
    for(u32bit c = 0; c < CONSTANT_BANK_REGISTERS; c++)
    {
        _constantBank[c][0] = 0.0f;
        _constantBank[c][1] = 0.0f;
        _constantBank[c][2] = 0.0f;
        _constantBank[c][3] = 0.0f;
    }

    for (acd_uint tu = 0; tu < 16 /*MAX_TEX_UNITS*/; tu++)
        _textureUnitUsage[tu] = 0;
}

ACDShaderProgramImp::~ACDShaderProgramImp()
{
    delete[] _bytecode;
    delete[] _optBytecode;
    delete[] _assembler;
}

void ACDShaderProgramImp::setCode(const acd_ubyte* attilaByteCode, acd_uint sizeInBytes)
{    
    if ( sizeInBytes == _bytecodeSize && memcmp(attilaByteCode, _bytecode, sizeInBytes) == 0 )
        return ; // Ignore set code

    delete[] _bytecode;
    delete[] _optBytecode;
    
    _optBytecode = 0;
    _optBytecodeSize = 0;

    _bytecode = new acd_ubyte[(_bytecodeSize = sizeInBytes)];
    memcpy(_bytecode, attilaByteCode, sizeInBytes);

    _computeASMCode();

    postReallocate(0);
}

void ACDShaderProgramImp::setProgram(acd_ubyte *attilaASM)
{
    acd_ubyte *code = new acd_ubyte[MAX_PROGRAM_SIZE];
    acd_uint codeSize = MAX_PROGRAM_SIZE;
    
    codeSize = GPUDriver::assembleShaderProgram(attilaASM, code, codeSize);
    
    if ((codeSize == _bytecodeSize) && (memcmp(code, _bytecode, codeSize) == 0))
    {
        delete[] code;
        return; // Ignore set code
    }
    
    delete[] _bytecode;
    delete[] _optBytecode;
    
    _optBytecode = 0;
    _optBytecodeSize = 0;
    
    _bytecodeSize = codeSize;
    _bytecode = new acd_ubyte[_bytecodeSize];
    memcpy(_bytecode, code, _bytecodeSize);

    delete[] code;
    
    _computeASMCode();

//printf("ACDShaderProgramImp::setProgram => program binary\n");
//printBinary(std::cout);
//printf("ACDShaderProgramImp::setProgram => program asm\n");
//printASM(std::cout);

    postReallocate(0);
}

const acd_ubyte* ACDShaderProgramImp::getCode() const
{
    return _bytecode;
}

acd_uint ACDShaderProgramImp::getSize() const
{
    return _bytecodeSize;
}

void ACDShaderProgramImp::setConstant(acd_uint index, const acd_float* vect4)
{
    ACD_ASSERT(
        if ( index >= CONSTANT_BANK_REGISTERS )
            panic("ACDShaderProgramImp", "setConstant", "Constant index out of bounds");
    )

    // track the constant as updated
    _touched.insert(index);

    memcpy(_constantBank[index], vect4, 4*sizeof(acd_float));

    // track the last set constant (for constant printing sake of clarity).
    if (_lastConstantSet < index)
        _lastConstantSet = index;
}

void ACDShaderProgramImp::getConstant(acd_uint index, acd_float* vect4) const
{
    ACD_ASSERT(
        if ( index >= CONSTANT_BANK_REGISTERS )
            panic("ACDShaderProgramImp", "setConstant", "Constant index out of bounds");
    )
    memcpy(vect4, _constantBank[index], 4*sizeof(acd_float));
}

const acd_ubyte* ACDShaderProgramImp::memoryData(acd_uint region, acd_uint& memorySizeInBytes) const
{
    if ( _optBytecode != 0 ) {
        memorySizeInBytes = _optBytecodeSize;
        return _optBytecode;
    }
    memorySizeInBytes = _bytecodeSize;
    return _bytecode;
}

const acd_char* ACDShaderProgramImp::stringType() const
{
    return "SHADER_PROGRAM_OBJECT";
}

void ACDShaderProgramImp::setOptimizedCode(const acd_ubyte* optimizedCode, acd_uint sizeInBytes)
{
    delete[] _optBytecode;

    _optBytecode = new acd_ubyte[(_optBytecodeSize = sizeInBytes)];
    memcpy(_optBytecode, optimizedCode, sizeInBytes);

    _computeASMCode(); // recompute ASM Code

    // More efficient implementation will be coded soon
    // No realocate required, often just updating is enough
    postReallocate(0);
}

bool ACDShaderProgramImp::isOptimized() const
{
    return _optBytecode != 0;
}

void ACDShaderProgramImp::updateConstants(GPUDriver* driver, gpu3d::GPURegister constantsType)
{
    gpu3d::GPURegData data;

    set<acd_uint>::iterator it = _touched.begin();
    const set<acd_uint>::iterator end = _touched.end();

    for ( ; it != end; ++it ) {
        const acd_uint constantIndex = *it;
        const acd_float* cb = _constantBank[constantIndex];
        acd_float* qfVal = data.qfVal;
        *qfVal = *cb;
        *(qfVal+1) = *(cb+1);
        *(qfVal+2) = *(cb+2);
        *(qfVal+3) = *(cb+3);
        driver->writeGPURegister(constantsType, constantIndex, data);
    }
}

void ACDShaderProgramImp::setInputRead(acd_uint inputReg, acd_bool read)
{
    if (inputReg >= MAX_SHADER_ATTRIBUTES)
        panic("ACDShaderProgramImp","setInputRead","Input register out of bounds");

    _inputsRead.set(inputReg, read);
}

void ACDShaderProgramImp::setOutputWritten(acd_uint outputReg, acd_bool written)
{
    if (outputReg >= MAX_SHADER_ATTRIBUTES)
        panic("ACDShaderProgramImp","setOutputWritten","Output register out of bounds");

    _outputsWritten.set(outputReg, written);
}

void ACDShaderProgramImp::setMaxAliveTemps(acd_uint maxAlive)
{
    _maxAliveTemps = maxAlive;
}

acd_bool ACDShaderProgramImp::getInputRead(acd_uint inputReg) const
{
    if (inputReg >= MAX_SHADER_ATTRIBUTES)
        panic("ACDShaderProgramImp","getInputRead","Input register out of bounds");

    return _inputsRead.test(inputReg);
}

acd_bool ACDShaderProgramImp::getOutputWritten(acd_uint outputReg) const
{
    if (outputReg >= MAX_SHADER_ATTRIBUTES)
        panic("ACDShaderProgramImp","getOutputWritten","Output register out of bounds");

    return _outputsWritten.test(outputReg);
}

acd_uint ACDShaderProgramImp::getMaxAliveTemps() const
{
    return _maxAliveTemps;
}

void ACDShaderProgramImp::setTextureUnitsUsage(acd_uint tu, acd_enum usage)
{
    _textureUnitUsage[tu] = usage;
}

acd_enum ACDShaderProgramImp::getTextureUnitsUsage(acd_uint tu)
{
    return _textureUnitUsage[tu];
}

void ACDShaderProgramImp::setKillInstructions(acd_bool kill)
{
    _killInstructions = kill;
}

acd_bool ACDShaderProgramImp::getKillInstructions()
{
    return _killInstructions;
}


void ACDShaderProgramImp::printASM(std::ostream& os) const
{
    os << _assembler;
}

void ACDShaderProgramImp::printBinary(std::ostream& os) const
{
    // Select binary code
    acd_ubyte* binary = ( _optBytecode ? _optBytecode : _bytecode );
    acd_uint binSize = ( _optBytecode ? _optBytecodeSize : _bytecodeSize );

    char buffer[16];
    string byteCodeLine;
    for ( unsigned int i = 1; i <= binSize; i++ )
    {
        sprintf(buffer, "%02x ", binary[i - 1]);
        byteCodeLine.append(buffer);
        if ((i % 16) == 0)
        {
            os << byteCodeLine;
            os << endl;
            byteCodeLine.clear();
        }
    }
}

void ACDShaderProgramImp::printConstants(std::ostream& os) const
{
    for ( acd_uint i = 0; i <= _lastConstantSet; i++)
        os << "c" << i << ": {" << _constantBank[i][0] << "," << _constantBank[i][1] << "," << _constantBank[i][2] << "," << _constantBank[i][3] << "}" << endl;

    os << endl;
}

void ACDShaderProgramImp::_computeASMCode()
{
    delete[] _assembler; // delete previous computed assembler 

    // Select binary code
    acd_ubyte* binary = ( _optBytecode ? _optBytecode : _bytecode );
    acd_uint binSize = ( _optBytecode ? _optBytecodeSize : _bytecodeSize );

    if ( !binary )
        panic("ACDShaderProgramImp", "_computeASMCode", "Binary code not available");

    acd_uint asmCodeSize = binSize * 16; 
    _assembler = new acd_ubyte[asmCodeSize];
    
    _assemblerSize = GPUDriver::disassembleShaderProgram(binary, binSize, _assembler, asmCodeSize, _killInstructions);
}

//////////////////////////////////////////////
//  END OF PRINT DEFINITIONS AND FUNCTIONS  //
//////////////////////////////////////////////

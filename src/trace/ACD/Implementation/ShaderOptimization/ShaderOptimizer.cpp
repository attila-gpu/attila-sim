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

#include "ShaderOptimizer.h"
#include "OptimizationSteps.h"

using namespace acdlib;
using namespace acdlib_opt;
using namespace std;
using namespace gpu3d;

OptimizationStep::OptimizationStep(ShaderOptimizer* shOptimizer)
: _shOptimizer(shOptimizer)
{
}

//SHADER_ARCH_PARAMS::SHADER_ARCH_PARAMS(const acd_uint latTable[])
SHADER_ARCH_PARAMS::SHADER_ARCH_PARAMS()
: 
  nWay(4), 
  temporaries(32), 
  outputRegs(16), 
  addrRegs(1),
  predRegs(32)

{
    //for(acd_uint i=0; i < LATENCY_TABLE_SIZE; i++)
    //    latencyTable[i] = latTable[i];
    shArchParams = ShaderArchitectureParameters::getShaderArchitectureParameters();
};

ShaderOptimizer::ShaderOptimizer(const SHADER_ARCH_PARAMS& shArchP)
: _reOptimize(true), 
  _inputCode(0), 
  _inputSizeInBytes(0), 
  _outputCode(0), 
  _outputSizeInBytes(0), 
  _shArchParams(shArchP)
{
    // Build the optimization steps. The push_back order will
    // be the optimization step execution order.
    // According to this, if any optimization steps uses
    // a previous step result, it must be added after the required one.

    //_optimizationSteps.push_back(new StaticInstructionScheduling(this));
    _optimizationSteps.push_back(new MaxLiveTempsAnalysis(this));
    _optimizationSteps.push_back(new RegisterUsageAnalysis(this));
    
}

void ShaderOptimizer::setCode(const acd_ubyte* code, acd_uint sizeInBytes)
{
    // Delete the previous input code and structures contents.

    if (_inputCode)
        delete[] _inputCode;

    vector<InstructionInfo*>::iterator iter = _inputInstrInfoVect.begin();
    
    while (iter != _inputInstrInfoVect.end())
    {
        delete (*iter);
        iter++;
    }

    _inputInstrInfoVect.clear();

    // Initialize the code structures again.

    _inputCode = new acd_ubyte[sizeInBytes];
    _inputSizeInBytes = sizeInBytes;
    memcpy((void *)_inputCode, (const void*)code, sizeInBytes);

    // Build the required structures for the code optimization
    // dissassembling the binary code.

    acd_ubyte* binary_pointer = _inputCode;
    acd_uint instrCount = 0;

    acd_bool programEnd = false;
    
    while (!programEnd && (binary_pointer < (_inputCode + _inputSizeInBytes)))
    {
        ShaderInstruction shInstr((u8bit*)binary_pointer);

        //if ((acd_uint)shInstr.getOpcode() >= LATENCY_TABLE_SIZE)
        //    panic("ShaderOptimizer","setCode","Invalid opcode or incomplete latency table");

        char asmStr[128];
        shInstr.disassemble(asmStr);

        //_inputInstrInfoVect.push_back(new InstructionInfo(shInstr, instrCount, _shArchParams.latencyTable[shInstr.getOpcode()], string(asmStr)));        
        _inputInstrInfoVect.push_back(new InstructionInfo(shInstr, instrCount,
                                                          _shArchParams.shArchParams->getExecutionLatency(shInstr.getOpcode()),
                                                          string(asmStr)));
        
        instrCount++;

        binary_pointer += gpu3d::ShaderInstruction::SHINSTRSIZE;
        
        programEnd = shInstr.getEndFlag();
    }
}

void ShaderOptimizer::setConstants(const std::vector<ACDVector<acd_float,4> >& constants)
{
    _inputConstants = constants;
}

void ShaderOptimizer::optimize()
{
    //////////////////////////////////////////////////
    // Iterate and apply all the optimization steps //
    //////////////////////////////////////////////////

    list<OptimizationStep*>::iterator it = _optimizationSteps.begin();

    while (it != _optimizationSteps.end())
    {
        (*it)->optimize();
        it++;
    }

    // No optimizations are performed related to the constant bank.
    // Thus, it´s directly copied.
    _outputConstants = _inputConstants;

    if (_outputCode)
        delete[] _outputCode;

    _outputSizeInBytes = _inputInstrInfoVect.size() * gpu3d::ShaderInstruction::SHINSTRSIZE;
    _outputCode = new acd_ubyte[_outputSizeInBytes];
    
    vector<InstructionInfo*>::const_iterator iter = _inputInstrInfoVect.begin();

    acd_ubyte* outputPointer = _outputCode;

    acd_uint listCount = 0;

    // Find the last non-NOP instruction
    acd_uint pointer = _inputInstrInfoVect.size() - 1;

    acd_bool found = false;

    while ( pointer >= 0 && !found)
    {
        if (_inputInstrInfoVect[pointer]->operation.opcode != NOP)
            found = true;
        else    
            pointer--;
    }

    acd_uint lastInstr;
    
    if (pointer >= 0)
        lastInstr = pointer;
    else
        lastInstr = 0;

    while ( iter != _inputInstrInfoVect.end() )
    {
        ShaderInstruction* shInstr;

        // Put the end flag to the last instruction

        if ( listCount == lastInstr )
            shInstr = (*iter)->getShaderInstructionCopy(true);
        else
            shInstr = (*iter)->getShaderInstructionCopy(false);

        shInstr->getCode((u8bit *)outputPointer);
        delete shInstr;
        outputPointer += gpu3d::ShaderInstruction::SHINSTRSIZE;
        iter++;
        listCount++;
    }

    ////////////////////////////////////////////////////
    // Perform final Atila architecture code add-ons: //
    ////////////////////////////////////////////////////
    //    1. Add the 16 required NOPS for the shader fetch unit.

    /*acd_ubyte* auxPointer = new acd_ubyte[_outputSizeInBytes + gpu3d::ShaderInstruction::SHINSTRSIZE * 16];
    memcpy((void *)auxPointer,(const void*)_outputCode, _outputSizeInBytes);
    
    for (acd_uint i=0; i < 16; i++)
    {
        ShaderInstruction shInstr(NOP);
        shInstr.getCode((u8bit *)(auxPointer + _outputSizeInBytes + i * gpu3d::ShaderInstruction::SHINSTRSIZE));
    }

    delete[] _outputCode;

    _outputCode = auxPointer;

    _outputSizeInBytes += gpu3d::ShaderInstruction::SHINSTRSIZE * 16;*/
}

const acd_ubyte* ShaderOptimizer::getCode(acd_uint& sizeInBytes) const
{
    sizeInBytes = _outputSizeInBytes;
    
    return _outputCode;
}

const std::vector<ACDVector<acd_float,4> >& ShaderOptimizer::getConstants() const
{
    return _outputConstants;
}

void ShaderOptimizer::getOptOutputInfo(OPTIMIZATION_OUTPUT_INFO& optOutInfo) const
{
    optOutInfo = _optOutInf;
}

ShaderOptimizer::~ShaderOptimizer()
{
    if (_inputCode)
        delete[] _inputCode;

    if (_outputCode)
        delete[] _outputCode;

    vector<InstructionInfo*>::iterator iter = _inputInstrInfoVect.begin();

    while (iter != _inputInstrInfoVect.end())
    {
        delete (*iter);
        iter++;
    }

    // Delete the optimization steps

    list<OptimizationStep*>::iterator iter2 = _optimizationSteps.begin();

    while (iter2 != _optimizationSteps.end())
    {
        delete (*iter2);
        iter2++;
    }
}

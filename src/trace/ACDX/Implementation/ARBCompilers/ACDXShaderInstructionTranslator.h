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

#ifndef ACDX_SHADERINSTRUCTIONTRANSLATOR_H
    #define ACDX_SHADERINSTRUCTIONTRANSLATOR_H

#include <list>
#include <iostream>

#include "ACDXRBank.h"
#include "ACDXGenericInstruction.h"
#include "ACDXProgramExecutionEnvironment.h"
#include "ShaderInstruction.h"

namespace acdlib
{

namespace GenerationCode
{

class ACDXShaderInstructionTranslator: public ACDXShaderCodeGeneration
{
private:

    class ShaderInstrOperand
    {
    public:
        u32bit idReg;
        gpu3d::Bank idBank;
        gpu3d::SwizzleMode swizzleMode;
        bool absoluteFlag;
        bool negateFlag;
        bool relativeModeFlag;
        u32bit relModeAddrReg;
        u8bit relModeAddrComp;
        s16bit relModeOffset;
    
        ShaderInstrOperand();
    };
    
    class ShaderInstrResult 
    {
    public:
        gpu3d::MaskMode writeMask;
        u32bit idReg;
        gpu3d::Bank idBank;
        ShaderInstrResult();
    };    
        
    bool maxLiveTempsComputed;
    u32bit maxLiveTemps;

    std::list<gpu3d::ShaderInstruction*> shaderCode; ///< Specific hardware-dependent instruction code

    acdlib::ACDXRBank<f32bit> paramBank;
    acdlib::ACDXRBank<f32bit> tempBank;
    
    unsigned int readPortsInBank;
    unsigned int readPortsOutBank;
    unsigned int readPortsParamBank;
    unsigned int readPortsTempBank;
    unsigned int readPortsAddrBank;

    //void clean();

    /* Auxiliar functions */
    
    void translateOperandReferences(const ACDXGenericInstruction::OperandInfo& genop, ShaderInstrOperand& shop) const;                                                                         
    
    void translateResultReferences(const ACDXGenericInstruction::ResultInfo& genres, ShaderInstrResult& shres) const;

    bool thereIsEquivalentInstruction(ACDXGenericInstruction::Opcode opc) const;

    bool thereAreOperandAccessConflicts( const ShaderInstrOperand& shop1, const ShaderInstrOperand& shop2,
                                         const ShaderInstrOperand& shop3, unsigned int num_operands,
                                         bool& preloadOp1, bool& preloadOp2) const;
    
    std::list<gpu3d::ShaderInstruction*> preloadOperands(int line, 
                                                        ShaderInstrOperand& shop1, ShaderInstrOperand& shop2,
                                                        ShaderInstrOperand& shop3,
                                                        bool preloadOp1, bool preloadOp2, 
                                                        unsigned preloadReg1, unsigned preloadReg2) const;
                                             
    
    
    std::list<gpu3d::ShaderInstruction*> translateInstruction(ACDXGenericInstruction::Opcode opcode, 
                                                         int line, int num_operands,
                                                         ShaderInstrOperand shop1,
                                                         ShaderInstrOperand shop2,
                                                         ShaderInstrOperand shop3,
                                                         ShaderInstrResult shres,
                                                         unsigned int textureImageUnit,
                                                         unsigned int killedSample,
                                                         unsigned int exportSample,
                                                         bool lastInstruction);

    gpu3d::ShOpcode translateOpcode(ACDXGenericInstruction::Opcode opc, bool &saturated, bool &texture, bool &sample);    
       
    gpu3d::SwizzleMode composeSwizzles(gpu3d::SwizzleMode swz1, gpu3d::SwizzleMode swz2) const;
    
    std::list<gpu3d::ShaderInstruction*> generateEquivalentInstructionList(ACDXGenericInstruction::Opcode opcode, 
                                                                          int line,int num_operands,
                                                                          ShaderInstrOperand shop1,
                                                                          ShaderInstrOperand shop2,
                                                                          ShaderInstrOperand shop3,
                                                                          ShaderInstrResult shres,
                                                                          ACDXGenericInstruction::SwizzleInfo swz,
                                                                          bool& calculatedPreloadReg1,
                                                                          unsigned int& preloadReg1,
                                                                          bool lastInstruction);
    void clean();
    

public:
    
    /* Constructor */
    ACDXShaderInstructionTranslator(acdlib::ACDXRBank<f32bit>& parametersBank,
                                acdlib::ACDXRBank<f32bit>& temporariesBank,
                                unsigned int readPortsInBank = 3,
                                unsigned int readPortsOutBank = 0,
                                unsigned int readPortsParamBank = 3,
                                unsigned int readPortsTempBank = 3,
                                unsigned int readPortsAddrBank = 1);
    
    acdlib::ACDXRBank<f32bit> getParametersBank() {    return paramBank;   }

    void returnGPUCode(u8bit* bin, u32bit& size);

    void translateCode( std::list<ACDXGenericInstruction*>& lgi, bool optimize = true, bool reorderCode = true);
    
    void returnASMCode(u8bit *ptr, u32bit& size);
    
    void printTranslated(std::ostream& os=std::cout);

    ~ACDXShaderInstructionTranslator();
    
};

} // CodeGeneration

} // namespace acdlib

#endif // ACDX_SHADERINSTRUCTIONTRANSLATOR_H

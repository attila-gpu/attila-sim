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

#ifndef SHADERINSTRUCTIONTRANSLATOR_H
    #define SHADERINSTRUCTIONTRANSLATOR_H

#include <list>
#include <iostream>

#include "RBank.h"
#include "GenericInstruction.h"
#include "ProgramExecutionEnvironment.h"
#include "ShaderInstruction.h"
#include "Scheduler.h"

namespace libgl
{

namespace GenerationCode
{

class ShaderInstructionTranslator: public ShaderCodeGeneration
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

    RBank<f32bit> paramBank;
    RBank<f32bit> tempBank;
    
    unsigned int readPortsInBank;
    unsigned int readPortsOutBank;
    unsigned int readPortsParamBank;
    unsigned int readPortsTempBank;
    unsigned int readPortsAddrBank;

    //void clean();

    /* Auxiliar functions */
    
    void translateOperandReferences(const GenericInstruction::OperandInfo& genop, ShaderInstrOperand& shop) const;                                                                         
    
    void translateResultReferences(const GenericInstruction::ResultInfo& genres, ShaderInstrResult& shres) const;

    bool thereIsEquivalentInstruction(GenericInstruction::Opcode opc) const;

    bool thereAreOperandAccessConflicts( const ShaderInstrOperand& shop1, const ShaderInstrOperand& shop2,
                                         const ShaderInstrOperand& shop3, unsigned int num_operands,
                                         bool& preloadOp1, bool& preloadOp2) const;
    
    std::list<gpu3d::ShaderInstruction*> preloadOperands(int line, 
                                                        ShaderInstrOperand& shop1, ShaderInstrOperand& shop2,
                                                        ShaderInstrOperand& shop3,
                                                        bool preloadOp1, bool preloadOp2, 
                                                        unsigned preloadReg1, unsigned preloadReg2) const;
                                             
    
    
    gpu3d::ShOpcode translateOpcode(GenericInstruction::Opcode opc, bool &saturated, bool &texture, bool &sample);
    
    std::list<gpu3d::ShaderInstruction*> translateInstruction(GenericInstruction::Opcode opcode, 
                                                              int line, int num_operands,
                                                              ShaderInstrOperand shop1,
                                                              ShaderInstrOperand shop2,
                                                              ShaderInstrOperand shop3,
                                                              ShaderInstrResult shres,
                                                              unsigned int textureImageUnit,
                                                              bool lastInstruction);
    
    gpu3d::SwizzleMode composeSwizzles(gpu3d::SwizzleMode swz1, gpu3d::SwizzleMode swz2) const;
    
    std::list<gpu3d::ShaderInstruction*> generateEquivalentInstructionList(GenericInstruction::Opcode opcode, 
                                                                          int line,int num_operands,
                                                                          ShaderInstrOperand shop1,
                                                                          ShaderInstrOperand shop2,
                                                                          ShaderInstrOperand shop3,
                                                                          ShaderInstrResult shres,
                                                                          GenericInstruction::SwizzleInfo swz,
                                                                          bool& calculatedPreloadReg1,
                                                                          unsigned int& preloadReg1,
                                                                          bool lastInstruction);
    void clean();
    
    void applyOptimizations(bool reorderCode);

public:
    
    /* Constructor */
    ShaderInstructionTranslator(RBank<f32bit>& parametersBank,
                                RBank<f32bit>& temporariesBank,
                                unsigned int readPortsInBank = 3,
                                unsigned int readPortsOutBank = 0,
                                unsigned int readPortsParamBank = 3,
                                unsigned int readPortsTempBank = 3,
                                unsigned int readPortsAddrBank = 1);
    
    RBank<f32bit> getParametersBank() {    return paramBank;   }

    void returnGPUCode(u8bit* bin, u32bit& size);

    u32bit getMaxAliveTemps();
    
    void translateCode( std::list<GenericInstruction*>& lgi, bool optimize = true, bool reorderCode = true);
    
    void returnASMCode(u8bit *ptr, u32bit& size);
    
    void printTranslated(std::ostream& os=std::cout);

    ~ShaderInstructionTranslator();
    
};

} // CodeGeneration

} // namespace libgl

#endif

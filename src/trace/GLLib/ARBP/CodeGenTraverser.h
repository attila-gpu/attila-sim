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

#ifndef CODEGENTRAVERSER_H
    #define CODEGENTRAVERSER_H

#include "IRTraverser.h"
#include "RBank.h"
#include "SymbolTable.h"
#include "GenericInstruction.h"
#include "SharedDefinitions.h"
#include "ImplementationLimits.h"
#include "ProgramExecutionEnvironment.h"
#include <string>
#include <list>

namespace libgl
{

namespace GenerationCode
{

class CodeGenTraverser : public IRTraverser
{
private:

    SymbolTable<IdentInfo* > symtab;            ///< Reference to symbol Table filled in semantic check
    std::list<IdentInfo*> identinfoCollector;  ///< Required because the Symbol Table doesnït manages IdentInfo destruction.
    
    std::list<GenericInstruction*> genericCode;  ///< Generic Instruction code generated.
    
    // Auxiliar attributes to construct instructions

    RBank<f32bit> fragmentInBank;   ///< Bank for vertex in attributes. Not used
    RBank<f32bit> fragmentOutBank;      ///< Bank for vertex out attributes. Not used

    RBank<f32bit> parametersBank;   ///< Bank for constants, local parameters,
                                    ///< environment parameters and GL states.
    RBank<f32bit> temporariesBank;   ///< Bank for temporaries in code.
    RBank<f32bit> addressBank;      ///< Bank for address registers in code.

    GenericInstruction::OperandInfo op[3];          ///< Operands structs info for instruction generation
    GenericInstruction::ResultInfo res;             ///< Result struct info for instruction generation
    GenericInstruction::SwizzleInfo swz;            ///< Swizzle struct info for the SWZ instruction generation
    GenericInstruction::TextureTarget texTarget;    ///< Texture Target info for 
                                                    ///< the TEX instruction generation
    unsigned int texImageUnit;  ///< Texture Unit info for the TEX instruction generation
    
    unsigned int paramBindingRegister[MAX_PROGRAM_PARAMETERS_ARB];
    unsigned int paramBindingIndex;
    unsigned int operandIndex;  ///< Index of operand in process
    unsigned int registerPosition;

public:
    // Auxiliar functions
    static unsigned int processSwizzleSuffix(const char* suffix);
    static unsigned int processWriteMask(const char* mask);
    static unsigned int fragmentAttributeBindingMapToRegister(const std::string attribute);
    static unsigned int vertexAttributeBindingMapToRegister(const std::string attribute);
    static unsigned int fragmentResultAttribBindingMaptoRegister(const std::string attribute);
    static unsigned int vertexResultAttribBindingMaptoRegister(const std::string attribute);
    static unsigned int paramStateBindingIdentifier(const std::string state, unsigned int& rowIndex);
    static std::string getWordWithoutIndex(std::string str, unsigned int& index);
    static std::string getWordBetweenDots(std::string str, unsigned int level);
    IdentInfo *recursiveSearchSymbol(std::string id); ///< Needed for ALIAS name resolve

public:
    CodeGenTraverser();
    
    RBank<f32bit>& getParametersBank() { return parametersBank; }
    RBank<f32bit>& getTemporariesBank() { return temporariesBank;   }

    std::list<GenericInstruction*>& getGenericCode() {  return genericCode; }

    bool visitProgram(bool preVisitAction, IRProgram*, IRTraverser*);
    bool visitVP1Option(bool preVisitAction, VP1IROption*, IRTraverser*);
    bool visitFP1Option(bool preVisitAction, FP1IROption*, IRTraverser*);
    bool visitStatement(bool preVisitAction, IRStatement*, IRTraverser*);
    bool visitInstruction(bool preVisitAction, IRInstruction*, IRTraverser*);
    bool visitSampleInstruction(bool preVisitAction, IRSampleInstruction*, IRTraverser*);
    bool visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents*, IRTraverser*);
    bool visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction*, IRTraverser*);
    bool visitDstOperand(bool preVisitAction, IRDstOperand*, IRTraverser*);
    bool visitSrcOperand(bool preVisitAction, IRSrcOperand*, IRTraverser*);
    bool visitArrayAddressing(bool preVisitAction, IRArrayAddressing*, IRTraverser*);
    bool visitNamingStatement(bool preVisitAction, IRNamingStatement*, IRTraverser*);
    bool visitALIASStatement(bool preVisitAction, IRALIASStatement*, IRTraverser*);
    bool visitTEMPStatement(bool preVisitAction, IRTEMPStatement*, IRTraverser*);
    bool visitADDRESSStatement(bool preVisitAction, IRADDRESSStatement*, IRTraverser*);
    bool visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement*, IRTraverser*);
    bool visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement*, IRTraverser*);
    bool visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement*, IRTraverser*);
    bool visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement*, IRTraverser*);
    bool visitPARAMStatement(bool preVisitAction, IRPARAMStatement*, IRTraverser*);
    bool visitParamBinding(bool preVisitAction, IRParamBinding*, IRTraverser*);
    bool visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding*, IRTraverser*);
    bool visitConstantBinding(bool preVisitAction, IRConstantBinding*, IRTraverser*);
    bool visitStateBinding(bool preVisitAction, IRStateBinding*, IRTraverser*);
    
    void dumpGenericCode(std::ostream& os);

    ~CodeGenTraverser(void);

};

} // namespace GenerationCode

} // namespace libgl

#endif // CODEGENTRAVERSER_H

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

#ifndef ACDX_CODEGENTRAVERSER_H
    #define ACDX_CODEGENTRAVERSER_H

#include "ACDXIRTraverser.h"
#include "ACDXRBank.h"
#include "ACDXSymbolTable.h"
#include "ACDXGenericInstruction.h"
#include "ACDXSharedDefinitions.h"
#include "ACDXImplementationLimits.h"
#include "ACDXProgramExecutionEnvironment.h"
#include <string>
#include <list>

namespace acdlib
{

namespace GenerationCode
{

class ACDXCodeGenTraverser : public ACDXIRTraverser
{
private:

    ACDXSymbolTable<IdentInfo* > symtab;            ///< Reference to symbol Table filled in semantic check
    std::list<IdentInfo*> identinfoCollector;  ///< Required because the Symbol Table doesnït manages IdentInfo destruction.
    
    std::list<ACDXGenericInstruction*> genericCode;  ///< Generic Instruction code generated.
    
    // Auxiliar attributes to construct instructions

    acdlib::ACDXRBank<f32bit> fragmentInBank;   ///< Bank for vertex in attributes. Not used
    acdlib::ACDXRBank<f32bit> fragmentOutBank;      ///< Bank for vertex out attributes. Not used

    acdlib::ACDXRBank<f32bit> parametersBank;   ///< Bank for constants, local parameters,
                                    ///< environment parameters and GL states.
    acdlib::ACDXRBank<f32bit> temporariesBank;   ///< Bank for temporaries in code.
    acdlib::ACDXRBank<f32bit> addressBank;      ///< Bank for address registers in code.

    ACDXGenericInstruction::OperandInfo op[3];          ///< Operands structs info for instruction generation
    ACDXGenericInstruction::ResultInfo res;             ///< Result struct info for instruction generation
    ACDXGenericInstruction::SwizzleInfo swz;            ///< Swizzle struct info for the SWZ instruction generation
    ACDXGenericInstruction::TextureTarget texTarget;    ///< Texture Target info for 
                                                    ///< the TEX instruction generation
    unsigned int texImageUnit;  ///< Texture Unit info for the TEX instruction generation
    
    unsigned int killSample;    ///< killed sample for the KLS instruction generation.
    unsigned int exportSample;  ///< export sample for the ZXS instruction generation.
    bool isFixedPoint; 

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
    ACDXCodeGenTraverser();
    
    acdlib::ACDXRBank<f32bit>& getParametersBank() { return parametersBank; }
    acdlib::ACDXRBank<f32bit>& getTemporariesBank() { return temporariesBank;   }

    std::list<ACDXGenericInstruction*>& getGenericCode() {  return genericCode; }

    bool visitProgram(bool preVisitAction, IRProgram*, ACDXIRTraverser*);
    bool visitVP1Option(bool preVisitAction, VP1IROption*, ACDXIRTraverser*);
    bool visitFP1Option(bool preVisitAction, FP1IROption*, ACDXIRTraverser*);
    bool visitStatement(bool preVisitAction, IRStatement*, ACDXIRTraverser*);
    bool visitInstruction(bool preVisitAction, IRInstruction*, ACDXIRTraverser*);
    bool visitSampleInstruction(bool preVisitAction, IRSampleInstruction*, ACDXIRTraverser*);
    bool visitKillInstruction(bool preVisitAction, IRKillInstruction*, ACDXIRTraverser*);
    bool visitZExportInstruction(bool preVisitAction, IRZExportInstruction*, ACDXIRTraverser*);
    bool visitCHSInstruction(bool preVisitAction, IRCHSInstruction*, ACDXIRTraverser*);
    bool visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents*, ACDXIRTraverser*);
    bool visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction*, ACDXIRTraverser*);
    bool visitDstOperand(bool preVisitAction, IRDstOperand*, ACDXIRTraverser*);
    bool visitSrcOperand(bool preVisitAction, IRSrcOperand*, ACDXIRTraverser*);
    bool visitArrayAddressing(bool preVisitAction, IRArrayAddressing*, ACDXIRTraverser*);
    bool visitNamingStatement(bool preVisitAction, IRNamingStatement*, ACDXIRTraverser*);
    bool visitALIASStatement(bool preVisitAction, IRALIASStatement*, ACDXIRTraverser*);
    bool visitTEMPStatement(bool preVisitAction, IRTEMPStatement*, ACDXIRTraverser*);
    bool visitADDRESSStatement(bool preVisitAction, IRADDRESSStatement*, ACDXIRTraverser*);
    bool visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement*, ACDXIRTraverser*);
    bool visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement*, ACDXIRTraverser*);
    bool visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement*, ACDXIRTraverser*);
    bool visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement*, ACDXIRTraverser*);
    bool visitPARAMStatement(bool preVisitAction, IRPARAMStatement*, ACDXIRTraverser*);
    bool visitParamBinding(bool preVisitAction, IRParamBinding*, ACDXIRTraverser*);
    bool visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding*, ACDXIRTraverser*);
    bool visitConstantBinding(bool preVisitAction, IRConstantBinding*, ACDXIRTraverser*);
    bool visitStateBinding(bool preVisitAction, IRStateBinding*, ACDXIRTraverser*);
    
    void dumpGenericCode(std::ostream& os);

    ~ACDXCodeGenTraverser(void);

};

} // namespace GenerationCode

} // namespace acdlib

#endif // ACDX_CODEGENTRAVERSER_H

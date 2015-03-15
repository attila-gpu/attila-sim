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

#ifndef SEMANTICTRAVERSER_H
    #define SEMANTICTRAVERSER_H

#include <iostream>
#include <sstream>
#include <bitset>

#include "IRTraverser.h"
#include "SymbolTable.h"
#include "SharedDefinitions.h"
//#include "ProgramExecutionEnvironment.h"
#include "GPUTypes.h"
#include "ImplementationLimits.h"
#include "gl.h"

namespace libgl
{

namespace GenerationCode
{

class SemanticTraverser : public IRTraverser//, public ShaderSemanticCheck
{
public:

    typedef std::bitset<MAX_PROGRAM_ATTRIBS_ARB> AttribMask;
    
private:
    std::stringstream errorOut;
    
    SymbolTable<IdentInfo *> symtab;
    std::list<IdentInfo*> identinfoCollector;  ///< Required because the Symbol Table doesnït manages IdentInfo destruction.
    
    bool semanticErrorInTraverse;

    /* Auxiliar functions */
    bool swizzleMaskCheck(const char *smask);
    bool writeMaskCheck(const char *wmask);
    IdentInfo *recursiveSearchSymbol(std::string id, bool& found); ///< Needed for ALIAS name resolve

    /* Resource usage statistics */
    unsigned int numberOfInstructions;
    unsigned int numberOfStatements;
    unsigned int numberOfParamRegs;
    unsigned int numberOfTempRegs;
    unsigned int numberOfAttribs;
    unsigned int numberOfAddrRegs;
    
    AttribMask inputRegisterRead;
    AttribMask outputRegisterWritten;
    
    /* For semantic check about sampling from multiple texture targets 
       on the same texture image unit */
    
    GLenum TextureImageUnitsTargets[MAX_TEXTURE_UNITS_ARB];
    

public:


    SemanticTraverser();

    std::string getErrorString() const  {   return errorOut.str();  }
    bool foundErrors() const            {   return semanticErrorInTraverse; }
    
    u32bit getNumberOfInstructions() const { return numberOfInstructions; }
    u32bit getNumberOfParamRegs() const  { return numberOfParamRegs; }
    u32bit getNumberOfTempRegs() const { return numberOfTempRegs; }
    u32bit getNumberOfAddrRegs() const { return numberOfAddrRegs; }
    u32bit getNumberOfAttribs() const { return numberOfAttribs; }
    
    GLenum getTextureUnitTarget(GLuint tu) const { return TextureImageUnitsTargets[tu];    }
    
    AttribMask getAttribsWritten() const;
    AttribMask getAttribsRead() const;

    virtual bool visitProgram(bool preVisitAction, IRProgram*, IRTraverser*);
    virtual bool visitVP1Option(bool preVisitAction, VP1IROption*, IRTraverser*);
    virtual bool visitFP1Option(bool preVisitAction, FP1IROption*, IRTraverser*);
    virtual bool visitStatement(bool preVisitAction, IRStatement*, IRTraverser*);
    virtual bool visitInstruction(bool preVisitAction, IRInstruction*, IRTraverser*);
    virtual bool visitSampleInstruction(bool preVisitAction, IRSampleInstruction*, IRTraverser*);
    virtual bool visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents*, IRTraverser*);
    virtual bool visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction*, IRTraverser*);
    virtual bool visitDstOperand(bool preVisitAction, IRDstOperand*, IRTraverser*);
    virtual bool visitArrayAddressing(bool preVisitAction, IRArrayAddressing *, IRTraverser*);
    virtual bool visitSrcOperand(bool preVisitAction, IRSrcOperand*, IRTraverser*);
    virtual bool visitNamingStatement(bool preVisitAction, IRNamingStatement*, IRTraverser*);
    virtual bool visitALIASStatement(bool preVisitAction, IRALIASStatement*, IRTraverser*);
    virtual bool visitTEMPStatement(bool preVisitAction, IRTEMPStatement*, IRTraverser*);
    virtual bool visitADDRESSStatement(bool preVisitAction, IRADDRESSStatement* addrstmnt, IRTraverser* it);
    virtual bool visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement*, IRTraverser*);
    virtual bool visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement*, IRTraverser*);
    virtual bool visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement*, IRTraverser*);
    virtual bool visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement*, IRTraverser*);
    virtual bool visitPARAMStatement(bool preVisitAction, IRPARAMStatement*, IRTraverser*);
    virtual bool visitParamBinding(bool preVisitAction, IRParamBinding*, IRTraverser*);
    virtual bool visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding*, IRTraverser*);
    virtual bool visitConstantBinding(bool preVisitAction, IRConstantBinding*, IRTraverser*);
    virtual bool visitStateBinding(bool preVisitAction, IRStateBinding*, IRTraverser*);

    virtual ~SemanticTraverser(void);
};

} // CodeGeneration

} // namespace libgl


#endif // SEMANTICTRAVERSER_H

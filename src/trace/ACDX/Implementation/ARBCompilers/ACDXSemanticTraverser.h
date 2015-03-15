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

#include "ACDXIRTraverser.h"
#include "ACDXSymbolTable.h"
#include "ACDXSharedDefinitions.h"
//#include "ACDXProgramExecutionEnvironment.h"
#include "GPUTypes.h"
#include "ACDXImplementationLimits.h"
#include "gl.h"
#include "glext.h"

namespace acdlib
{

namespace GenerationCode
{

class ACDXSemanticTraverser  : public ACDXIRTraverser//, public ShaderSemanticCheck
{
public:

    typedef std::bitset<MAX_PROGRAM_ATTRIBS_ARB> AttribMask;
    
private:
    std::stringstream errorOut;
    
    ACDXSymbolTable<IdentInfo *> symtab;
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
    int textureUnitUsage [MAX_TEXTURE_UNITS_ARB];
    bool killInstructions;
    
    AttribMask inputRegisterRead;
    AttribMask outputRegisterWritten;
    
    /* For semantic check about sampling from multiple texture targets 
       on the same texture image unit */
    
    GLenum TextureImageUnitsTargets[MAX_TEXTURE_UNITS_ARB];
    

public:


    ACDXSemanticTraverser ();

    std::string getErrorString() const  {   return errorOut.str();  }
    bool foundErrors() const            {   return semanticErrorInTraverse; }
    
    u32bit getNumberOfInstructions() const { return numberOfInstructions; }
    u32bit getNumberOfParamRegs() const  { return numberOfParamRegs; }
    u32bit getNumberOfTempRegs() const { return numberOfTempRegs; }
    u32bit getNumberOfAddrRegs() const { return numberOfAddrRegs; }
    u32bit getNumberOfAttribs() const { return numberOfAttribs; }
    
    GLenum getTextureUnitTarget(GLuint tu) const { return TextureImageUnitsTargets[tu];    }
    GLboolean getKillInstructions() const { return killInstructions; }
    
    AttribMask getAttribsWritten() const;
    AttribMask getAttribsRead() const;

    virtual bool visitProgram(bool preVisitAction, IRProgram*, ACDXIRTraverser*);
    virtual bool visitVP1Option(bool preVisitAction, VP1IROption*, ACDXIRTraverser*);
    virtual bool visitFP1Option(bool preVisitAction, FP1IROption*, ACDXIRTraverser*);
    virtual bool visitStatement(bool preVisitAction, IRStatement*, ACDXIRTraverser*);
    virtual bool visitInstruction(bool preVisitAction, IRInstruction*, ACDXIRTraverser*);
    virtual bool visitSampleInstruction(bool preVisitAction, IRSampleInstruction*, ACDXIRTraverser*);
    virtual bool visitKillInstruction(bool preVisitAction, IRKillInstruction*, ACDXIRTraverser*);
    virtual bool visitZExportInstruction(bool preVisitAction, IRZExportInstruction*, ACDXIRTraverser*);
    virtual bool visitCHSInstruction(bool preVisitAction, IRCHSInstruction*, ACDXIRTraverser*);
    virtual bool visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents*, ACDXIRTraverser*);
    virtual bool visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction*, ACDXIRTraverser*);
    virtual bool visitDstOperand(bool preVisitAction, IRDstOperand*, ACDXIRTraverser*);
    virtual bool visitArrayAddressing(bool preVisitAction, IRArrayAddressing *, ACDXIRTraverser*);
    virtual bool visitSrcOperand(bool preVisitAction, IRSrcOperand*, ACDXIRTraverser*);
    virtual bool visitNamingStatement(bool preVisitAction, IRNamingStatement*, ACDXIRTraverser*);
    virtual bool visitALIASStatement(bool preVisitAction, IRALIASStatement*, ACDXIRTraverser*);
    virtual bool visitTEMPStatement(bool preVisitAction, IRTEMPStatement*, ACDXIRTraverser*);
    virtual bool visitADDRESSStatement(bool preVisitAction, IRADDRESSStatement* addrstmnt, ACDXIRTraverser* it);
    virtual bool visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement*, ACDXIRTraverser*);
    virtual bool visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement*, ACDXIRTraverser*);
    virtual bool visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement*, ACDXIRTraverser*);
    virtual bool visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement*, ACDXIRTraverser*);
    virtual bool visitPARAMStatement(bool preVisitAction, IRPARAMStatement*, ACDXIRTraverser*);
    virtual bool visitParamBinding(bool preVisitAction, IRParamBinding*, ACDXIRTraverser*);
    virtual bool visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding*, ACDXIRTraverser*);
    virtual bool visitConstantBinding(bool preVisitAction, IRConstantBinding*, ACDXIRTraverser*);
    virtual bool visitStateBinding(bool preVisitAction, IRStateBinding*, ACDXIRTraverser*);

    virtual ~ACDXSemanticTraverser (void);
};

} // CodeGeneration

} // namespace acdlib


#endif // SEMANTICTRAVERSER_H

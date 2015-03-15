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

#include "ACDXIRNode.h"
#include "ACDXIRTraverser.h"
#include <sstream>

using namespace std;
using namespace acdlib::GenerationCode;
using namespace acdlib;

ACDXIRNode::ACDXIRNode()
: line(0)
{
}

ACDXIRNode::ACDXIRNode(unsigned int line)
: line(line)
{
}

ACDXIRNode::~ACDXIRNode()
{
}

IRProgram::IRProgram()
: headerString("")
{
}

IRProgram::IRProgram(const string& headerstring)
: headerString(headerstring), programOptions(0), programStatements(0)
{
}

//
// This function is the one to call externally to start the traversal.
// Individual functions can be initialized to 0 to skip processing of that
// type of node.  It's children will still be processed.
//

void IRProgram::dump(ostream& os)
{
    ACDXOutputTraverser it(os);
    traverse(&it);
}

IRProgram::~IRProgram()
{
    if (programOptions != 0)
    {
        list<IROption*>::iterator it = this->programOptions->begin();
        for (;it != this->programOptions->end();it++)
            delete (*it);

        delete programOptions;
    }
    
    if (programStatements != 0)
    {
        list<IRStatement*>::iterator it2 = this->programStatements->begin();
        for (;it2 != this->programStatements->end();it2++)
            delete (*it2);

        delete programStatements;
    }        
}

IROption::IROption(unsigned int line, const string& optionString)
: ACDXIRNode(line), optionString(optionString)
{
}

VP1IROption::VP1IROption(unsigned int line, const std::string& optionString)
:   IROption(line,optionString)
{
}

FP1IROption::FP1IROption(unsigned int line, const std::string& optionString)
:   IROption(line,optionString)
{
}

IRStatement::IRStatement(unsigned int line)
:   ACDXIRNode(line)
{
}

IRStatement::~IRStatement()
{
}

IRInstruction::IRInstruction(unsigned int line, const string& opcode)
:   IRStatement(line), opCode(opcode), source0(0), source1(0), source2(0), dest(0), isFixedPoint(false)
{

}

IRInstruction::~IRInstruction()
{
    
    if (source0 != 0) delete source0;
    if (source1 != 0) delete source1;
    if (source2 != 0) delete source2;
    if (dest != 0) delete dest;
    
}

IRSampleInstruction::IRSampleInstruction(unsigned int line, const string& opcode)
:   IRInstruction(line,opcode), textureImageUnit(0), glTextureTarget("")
{
}

IRKillInstruction::IRKillInstruction(unsigned int line, const string& opcode)
:   IRInstruction(line, opcode), isKillSampleInstr(false), killSample(0)
{
}

IRZExportInstruction::IRZExportInstruction(unsigned int line, const std::string& opcode)
:   IRInstruction(line, opcode), isExpSampleInstr(false), exportSample(0)
{
}

IRCHSInstruction::IRCHSInstruction(unsigned int line, const string& opcode)
:   IRInstruction(line, opcode)
{
}

IRSwizzleComponents::IRSwizzleComponents()
{
    swizzleComp[0] = X;
    swizzleComp[1] = Y;
    swizzleComp[2] = Z;
    swizzleComp[3] = W;

    swizzleNegFlag[0] = swizzleNegFlag[1] = swizzleNegFlag[2] = swizzleNegFlag[3] = false;
}

IRSwizzleInstruction::IRSwizzleInstruction(unsigned int line, const string& opcode)
:   IRInstruction(line,opcode), swizzleComps(0)
{
}

IRSwizzleInstruction::~IRSwizzleInstruction()
{
    if (swizzleComps != 0)
        delete swizzleComps;
}

IRDstOperand::IRDstOperand(const string& destination)
:   destination(destination), writeMask("xyzw"), isFragmentResultRegister(false) ,isVertexResultRegister(false)
{
}


IRArrayAddressing::IRArrayAddressing(unsigned int arrayAddressOffset, bool relativeAddress)
: arrayAddressOffset(arrayAddressOffset), relativeAddress(relativeAddress), 
  relativeAddressReg(std::string("")), relativeAddressComp(std::string("")),
  relativeAddressOffset(0) 
{
}

void IRArrayAddressing::setRelativeAddressing(std::string& relativeAddressReg, 
                                              std::string& relativeAddressComp,
                                              unsigned int relativeAddressOffset)
{
    this->relativeAddress = true;
    this->relativeAddressReg = relativeAddressReg;
    this->relativeAddressComp = relativeAddressComp;
    this->relativeAddressOffset = relativeAddressOffset;
}


IRSrcOperand::IRSrcOperand(const string& source)
:   source(source), isFragmentRegister(false), isVertexRegister(false),
    isParamBinding(false), paramBinding(0), swizzleMask("xyzw"), negateFlag(false),
    isArrayAddressing(false), arrayAddrInfo(0)
{
}

IRSrcOperand::~IRSrcOperand()
{
    if (isParamBinding != 0)
        delete paramBinding;
    if (arrayAddrInfo != 0)
        delete arrayAddrInfo;
}



IRNamingStatement::IRNamingStatement(unsigned int line, const string& name)
:   IRStatement(line), name(name)
{
}

IRNamingStatement::~IRNamingStatement()
{
}

IRALIASStatement::IRALIASStatement(unsigned int line, const string& name)
:   IRNamingStatement(line,name)
{
}

IRTEMPStatement::IRTEMPStatement(unsigned int line, const string& name)
:   IRNamingStatement(line,name)
{
}

IRTEMPStatement::~IRTEMPStatement()
{
    delete otherTemporariesNames;
}

IRADDRESSStatement::IRADDRESSStatement(unsigned int line, const string& name)
:   IRNamingStatement(line,name)
{
}

IRADDRESSStatement::~IRADDRESSStatement()
{
    delete otherAddressRegisters;
}


IRATTRIBStatement::IRATTRIBStatement(unsigned int line, const string& name)
:   IRNamingStatement(line,name)
{
}

VP1IRATTRIBStatement::VP1IRATTRIBStatement(unsigned int line, const string& name)
:   IRATTRIBStatement(line,name)
{
}

FP1IRATTRIBStatement::FP1IRATTRIBStatement(unsigned int line, const string& name)
:   IRATTRIBStatement(line,name)
{
}

IROUTPUTStatement::IROUTPUTStatement(unsigned int line, const string& name)
:   IRNamingStatement(line,name)
{
}

VP1IROUTPUTStatement::VP1IROUTPUTStatement(unsigned int line, const string& name)
:   IROUTPUTStatement(line,name)
{
}

FP1IROUTPUTStatement::FP1IROUTPUTStatement(unsigned int line, const string& name)
:   IROUTPUTStatement(line,name)
{
}

IRPARAMStatement::IRPARAMStatement(unsigned int line, const string& name, bool isMultipleBindings)
:   IRNamingStatement(line,name), isMultipleBindings(isMultipleBindings), paramBindings(0)
{
}

IRPARAMStatement::~IRPARAMStatement()
{
    if (paramBindings != 0) 
    {    
        list<IRParamBinding *>::iterator it = paramBindings->begin();
        for (;it != paramBindings->end();it++)
            delete (*it);
        
        delete paramBindings;
    }
}

IRParamBinding::IRParamBinding()
:  isImplicitBinding(false) 
{
}

IRLocalEnvBinding::IRLocalEnvBinding(PType paramType)
: paramType(paramType), minIndex(0), maxIndex(0) 
{
}

IRConstantBinding::IRConstantBinding(float v1, float v2, float v3, float v4, bool isScalarConst)
: value1(v1), value2(v2), value3(v3), value4(v4), isScalarConst(isScalarConst)
{
}

IRStateBinding::IRStateBinding(const std::string& state, bool isMatrixState)
: state(state), isMatrixState(isMatrixState)
{
}

/**
 *      Traverser functions. For visitor pattern
 */

void IRProgram::traverse(ACDXIRTraverser* it)
{
    bool visit = true;

    if (it->preVisit)
        visit = it->visitProgram(true,this,it);

    if (visit)
    {
        ++it->depth;
        list<IROption *>::iterator iter = programOptions->begin();
        while (iter != programOptions->end())
        {
            it->depthProcess();
            (*iter)->traverse(it);
            iter++;
        }
        
        list<IRStatement *>::iterator iter2 = programStatements->begin();
        while (iter2 != programStatements->end())
        {
            it->depthProcess();
            (*iter2)->traverse(it);
            iter2++;
        }
        --it->depth;
    }

    if (visit && it->postVisit)
        it->visitProgram(false,this,it);
}

void VP1IROption::traverse(ACDXIRTraverser* it)
{
    it->visitVP1Option(true,this,it);
}

void FP1IROption::traverse(ACDXIRTraverser *it)
{
    it->visitFP1Option(true,this,it);
}

void IRInstruction::traverse(ACDXIRTraverser *it)
{
    bool visit = true;

    if (it->preVisit)
        visit = it->visitInstruction(true,this,it);

    if (visit)
    {
        ++it->depth;
        if (dest)
        {
            it->depthProcess();
            dest->traverse(it);
        }
        if (source0)
        {
            it->depthProcess();
            source0->traverse(it);
        }
        if (source1)
        {
            it->depthProcess();
            source1->traverse(it);
        }
        if (source2)
        {
            it->depthProcess();
            source2->traverse(it);
        }
        --it->depth;
    }
    
    if (visit && it->postVisit)
        it->visitInstruction(false,this,it);

}

void IRSampleInstruction::traverse(ACDXIRTraverser *it)
{
    bool visit = true;

    if (it->preVisit)
        visit = it->visitSampleInstruction(true,this,it);

    if (visit)
    {
        ++it->depth;

        it->depthProcess();
        dest->traverse(it);

        it->depthProcess();
        source0->traverse(it);

         --it->depth;
    }
    
    if (visit && it->postVisit)
        it->visitSampleInstruction(false,this,it);
}

void IRKillInstruction::traverse(ACDXIRTraverser *it)
{
    bool visit = true;

    if (it->preVisit)
        it->visitKillInstruction(true,this,it);

    if (visit)
    {
        it->depthProcess();
        source0->traverse(it);
    }
    
    if (visit && it->postVisit)
        it->visitKillInstruction(false,this,it);
}

void IRZExportInstruction::traverse(ACDXIRTraverser *it)
{
    bool visit = true;

    if (it->preVisit)
        it->visitZExportInstruction(true,this,it);

    if (visit)
    {
        it->depthProcess();
        source0->traverse(it);
    }
    
    if (visit && it->postVisit)
        it->visitZExportInstruction(false,this,it);
}

void IRCHSInstruction::traverse(ACDXIRTraverser *it)
{
    bool visit = true;

    if (it->preVisit)
        visit = it->visitCHSInstruction(true,this,it);

    if (visit && it->postVisit)
        it->visitCHSInstruction(false,this,it);
}

void IRSwizzleComponents::traverse(ACDXIRTraverser *it)
{
    it->visitSwizzleComponents(true,this,it);
}

void IRSwizzleInstruction::traverse(ACDXIRTraverser *it)
{
    bool visit = true;

    if (it->preVisit)
        visit = it->visitSwizzleInstruction(true,this,it);

    if (visit)
    {
        it->depthProcess();
        dest->traverse(it);

        it->depthProcess();
        source0->traverse(it);

        ++it->depth;
        it->depthProcess();
        swizzleComps->traverse(it);
        --it->depth;
    }
    
    if (visit && it->postVisit)
        it->visitSwizzleInstruction(false,this,it);
}

void IRDstOperand::traverse(ACDXIRTraverser* it)
{
    it->visitDstOperand(true,this,it);
}

void IRArrayAddressing::traverse(ACDXIRTraverser* it)
{
    it->visitArrayAddressing(true,this,it);
}

void IRSrcOperand::traverse(ACDXIRTraverser* it)
{
    bool visit = true;

    if (it->preVisit)
        visit = it->visitSrcOperand(true,this,it);

    if (visit)
    {
        ++it->depth;
        if (isParamBinding)
        {
            it->depthProcess();
            paramBinding->traverse(it);
        }
        if (isArrayAddressing)
        {
            it->depthProcess();
            arrayAddrInfo->traverse(it);
        }
        --it->depth;

    }
    
    if (visit && it->postVisit)
        it->visitSrcOperand(false,this,it);
}

void IRALIASStatement::traverse(ACDXIRTraverser* it)
{
    it->visitALIASStatement(true,this,it);
}

void IRTEMPStatement::traverse(ACDXIRTraverser* it)
{
    it->visitTEMPStatement(true,this,it);
}

void IRADDRESSStatement::traverse(ACDXIRTraverser* it)
{
    it->visitADDRESSStatement(true,this,it);
}

void VP1IRATTRIBStatement::traverse(ACDXIRTraverser* it)
{
    it->visitVP1ATTRIBStatement(true,this,it);
}

void FP1IRATTRIBStatement::traverse(ACDXIRTraverser* it)
{
    it->visitFP1ATTRIBStatement(true,this,it);
}

void VP1IROUTPUTStatement::traverse(ACDXIRTraverser* it)
{
    it->visitVP1OUTPUTStatement(true,this,it);
}

void FP1IROUTPUTStatement::traverse(ACDXIRTraverser* it)
{
    it->visitFP1OUTPUTStatement(true,this,it);
}

void IRPARAMStatement::traverse(ACDXIRTraverser* it)
{
    bool visit = true;

    if (it->preVisit)
        visit = it->visitPARAMStatement(true,this,it);

    if (visit)
    {
        ++it->depth;
        list<IRParamBinding *>::iterator iter = this->paramBindings->begin();
        while (iter != this->paramBindings->end())
        {
            it->depthProcess();
            (*iter)->traverse(it);
            iter++;
        }
        --it->depth;
    }
    
    if (visit && it->postVisit)
        it->visitPARAMStatement(false,this,it);
    
}

void IRLocalEnvBinding::traverse(ACDXIRTraverser* it)
{
    it->visitLocalEnvBinding(true,this,it);
}

void IRConstantBinding::traverse(ACDXIRTraverser* it)
{
    it->visitConstantBinding(true,this,it);
}

void IRStateBinding::traverse(ACDXIRTraverser* it)
{
    it->visitStateBinding(true,this,it);
}

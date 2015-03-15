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

#include "IRTraverser.h"
#include "support.h"

using namespace libgl::GenerationCode;
using namespace libgl;
using namespace std;

void OutputTraverser::depthProcess()
{
    int i;
    
    for (i = 0; i < depth; ++i)
        os << "  ";
}

bool OutputTraverser::visitProgram(bool preVisitAction, IRProgram* program, IRTraverser*)
{
    os << program->getLine() << ": " << program->getHeaderString() << endl;
    return true;
}
bool OutputTraverser::visitVP1Option(bool preVisitAction, VP1IROption* option, IRTraverser*)
{
    os << option->getLine() << ": " << "ARBVp 1.0 Option: "<< option->getOptionString() << endl;
    return true;
}

bool OutputTraverser::visitFP1Option(bool preVisitAction, FP1IROption* option, IRTraverser*)
{
    os << option->getLine() << ": " << "ARBFp 1.0 Option: "<< option->getOptionString() << endl;
    return true;
}

bool OutputTraverser::visitStatement(bool preVisitAction, IRStatement* statement, IRTraverser*)
{
    os << statement->getLine() << ": ";
    return true;
}
bool OutputTraverser::visitInstruction(bool preVisitAction, IRInstruction* instruction, IRTraverser* it)
{
    visitStatement(preVisitAction, instruction, it);
    os << "Instruction: " << instruction->getOpcode() << endl;
    return true;
}

bool OutputTraverser::visitSampleInstruction(bool preVisitAction, IRSampleInstruction *sinstr,IRTraverser* it)
{
    visitInstruction(preVisitAction, sinstr,it);
    depthProcess();
    os << "Texture Image Unit: " << sinstr->getTextureImageUnit();
    os << " Texture Target: " << sinstr->getGLTextureTarget() << endl;
    return true;
}

bool OutputTraverser::visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents* swzcomps, IRTraverser*)
{
    os << "Swizzle: ";
    for (unsigned int i=0;i<4;i++)
    {
        if (swzcomps->getNegFlag(i)) os << "-";
        switch (swzcomps->getComponent(i))
        {
            case IRSwizzleComponents::ZERO: os << "0"; break;
            case IRSwizzleComponents::ONE:  os << "1"; break;
            case IRSwizzleComponents::X:       os << "X"; break;
            case IRSwizzleComponents::Y:       os << "Y"; break;
            case IRSwizzleComponents::Z:       os << "Z"; break;
            case IRSwizzleComponents::W:       os << "W"; break;
        }
        if (i<3) os << ",";
    }

    os << endl;
    return true;
}

bool OutputTraverser::visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction *swinstr, IRTraverser* it)
{
    visitInstruction(preVisitAction,swinstr,it);
    return true;
}

bool OutputTraverser::visitDstOperand(bool preVisitAction, IRDstOperand *dstop, IRTraverser *)
{
    os << "DstOp: ";
    
    if (dstop->getIsVertexResultRegister() || dstop->getIsFragmentResultRegister()) os << "result";
    
    os << dstop->getDestination();
    
    if (dstop->getWriteMask().compare("xyzw"))
        os << " WriteMask: " << dstop->getWriteMask();
    
    os << endl;
    return true;
}

bool OutputTraverser::visitArrayAddressing(bool preVisitAction, IRArrayAddressing *arrayaddr, IRTraverser* it)
{
    os << "Array Access:";
    if (!arrayaddr->getIsRelativeAddress())
    {
        os << " Absolute Offset: " << arrayaddr->getArrayAddressOffset() << endl;
    }
    else // relative addressing
    {
        os << " Address Register: " << arrayaddr->getRelativeAddressReg();
        os << " Component: " << arrayaddr->getRelativeAddressComp();
        os << " Relative Offset: " << arrayaddr->getRelativeAddressOffset();
    }
    return true;
}

bool OutputTraverser::visitSrcOperand(bool preVisitAction, IRSrcOperand *srcop, IRTraverser *)
{
    os << "SrcOp: ";

    if (srcop->getIsFragmentRegister()) os << "fragment";

    if (srcop->getIsVertexRegister()) os << "vertex";

    os << srcop->getSource();

    if (srcop->getSwizzleMask().compare("xyzw"))
        os << " SwizzleMask: " << srcop->getSwizzleMask();

    if (srcop->getNegateFlag()) os << " Negated(-)";    

    os << endl;

    return true;
}

bool OutputTraverser::visitNamingStatement(bool preVisitAction, IRNamingStatement* namingstmnt, IRTraverser* it)
{
    visitStatement(preVisitAction,namingstmnt,it);
    os << "Naming Statement: ";
    return true;
}

bool OutputTraverser::visitALIASStatement(bool preVisitAction, IRALIASStatement* aliasstmnt, IRTraverser* it)
{
    visitNamingStatement(preVisitAction,aliasstmnt,it);
    os << "Alias: " << aliasstmnt->getName() << " = " << aliasstmnt->getAlias() << endl;
    return true;    
}

bool OutputTraverser::visitTEMPStatement(bool preVisitAction, IRTEMPStatement* tempstmnt, IRTraverser* it)
{
    visitNamingStatement(preVisitAction,tempstmnt,it);
    os << "Temp: " << tempstmnt->getName();
    list<string>::iterator iter = tempstmnt->getOtherTemporaries()->begin();
        
    while (iter != tempstmnt->getOtherTemporaries()->end()) {
        os << ", " << (*iter);
        iter++;
    }
    os << endl;
    return true;
}

bool OutputTraverser::visitADDRESSStatement(bool preVisitAction,IRADDRESSStatement* addrstmnt, IRTraverser* it)
{
    visitNamingStatement(preVisitAction,addrstmnt,it);
    os << "Addr: " << addrstmnt->getName();

    list<string>::iterator iter = addrstmnt->getOtherAddressRegisters()->begin();
        
    while (iter != addrstmnt->getOtherAddressRegisters()->end()) {
        os << ", " << (*iter);
        iter++;
    }
    os << endl;
    return true;
}

bool OutputTraverser::visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement* attribstmnt, IRTraverser* it)
{
    visitNamingStatement(preVisitAction,attribstmnt,it);
    os << "Vertex Input Attrib: " << attribstmnt->getName() << " = vertex";
    os << attribstmnt->getInputAttribute();
    os << endl;
    return true;
}

bool OutputTraverser::visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement* attribstmnt, IRTraverser* it)
{
    visitNamingStatement(preVisitAction,attribstmnt,it);
    os << "Fragment Input Attrib: " << attribstmnt->getName() << " = fragment";
    os << attribstmnt->getInputAttribute();
    os << endl;
    return true;
}

bool OutputTraverser::visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement *outputstmnt, IRTraverser*it)
{
    visitNamingStatement(preVisitAction,outputstmnt,it);
    os << "Vertex Output Attrib: " << outputstmnt->getName() << " = result";
    os << outputstmnt->getOutputAttribute();
    os << endl;
    return true;
}

bool OutputTraverser::visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement *outputstmnt, IRTraverser*it)
{
    visitNamingStatement(preVisitAction,outputstmnt,it);
    os << "Fragment Output Attrib: " << outputstmnt->getName() << " = result";
    os << outputstmnt->getOutputAttribute();
    os << endl;
    return true;
}


bool OutputTraverser::visitPARAMStatement(bool preVisitAction, IRPARAMStatement* paramstmnt, IRTraverser* it)
{
    visitNamingStatement(preVisitAction,paramstmnt,it);
    os << "Param: " << paramstmnt->getName();
    if (paramstmnt->getIsMultipleBindings()) os << "[";
    if (paramstmnt->getSize() >= 0)
    {
        os << paramstmnt->getSize();
    }
    if (paramstmnt->getIsMultipleBindings()) os << "]";
    os << " = ";
    return true;
}

bool OutputTraverser::visitParamBinding(bool preVisitAction, IRParamBinding* parambind, IRTraverser*)
{
    if (parambind->getIsImplicitBinding()) os << "Implicit ";
    return true;
}

bool OutputTraverser::visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding* localenvbind, IRTraverser* it)
{
    visitParamBinding(preVisitAction,localenvbind,it);
    switch(localenvbind->getType())
    {
        case IRLocalEnvBinding::LOCALPARAM: os << "Local "; break;
        case IRLocalEnvBinding::ENVPARAM: os << "Environment "; break;
    }
    os << "Parameter Binding [" << localenvbind->getMinIndex() << "..";
    os << localenvbind->getMaxIndex() << "]" << endl;
    return true;
}

bool OutputTraverser::visitConstantBinding(bool preVisitAction, IRConstantBinding* constbind, IRTraverser*it)
{
    visitParamBinding(preVisitAction,constbind,it);
    os << "Constant: ";
    if (constbind->getIsScalarConst())
    {
        os << constbind->getValue1();
    }
    else /* Vector constant */
    {
        os << "{ " << constbind->getValue1() << ", " << constbind->getValue2();
        os << ", " << constbind->getValue3() << ", " << constbind->getValue4() << " }";
    }
    os << endl;
    return true;
}

bool OutputTraverser::visitStateBinding(bool preVisitAction, IRStateBinding *statebind, IRTraverser*it)
{
    visitParamBinding(preVisitAction,statebind,it);
    os << "State: " << statebind->getState() << endl;
    return true;
}

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

#include "ACDXIRTraverser.h"
#include "support.h"

using namespace acdlib::GenerationCode;
using namespace acdlib;
using namespace std;

void ACDXOutputTraverser::depthProcess()
{
    int i;
    
    for (i = 0; i < depth; ++i)
        os << "  ";
}

bool ACDXOutputTraverser::visitProgram(bool , IRProgram* program, ACDXIRTraverser* )
{
    os << program->getLine() << ": " << program->getHeaderString() << endl;
    return true;
}
bool ACDXOutputTraverser::visitVP1Option(bool , VP1IROption* option, ACDXIRTraverser* )
{
    os << option->getLine() << ": " << "ARBVp 1.0 Option: "<< option->getOptionString() << endl;
    return true;
}

bool ACDXOutputTraverser::visitFP1Option(bool , FP1IROption* option, ACDXIRTraverser* )
{
    os << option->getLine() << ": " << "ARBFp 1.0 Option: "<< option->getOptionString() << endl;
    return true;
}

bool ACDXOutputTraverser::visitStatement(bool , IRStatement* statement, ACDXIRTraverser* )
{
    os << statement->getLine() << ": ";
    return true;
}
bool ACDXOutputTraverser::visitInstruction(bool preVisitAction, IRInstruction* instruction, ACDXIRTraverser* it)
{
    visitStatement(preVisitAction, instruction, it);
    os << "Instruction: " << instruction->getOpcode();
    if (instruction->getIsFixedPoint())
    {
        os << " (Fixed Point)" << endl;
    }
    else
    {
        os << endl;
    }
    return true;
}

bool ACDXOutputTraverser::visitSampleInstruction(bool preVisitAction, IRSampleInstruction *sinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, sinstr,it);
    depthProcess();
    os << " Texture Image Unit: " << sinstr->getTextureImageUnit();
    os << " Texture Target: " << sinstr->getGLTextureTarget() << endl;
    return true;
}

bool ACDXOutputTraverser::visitKillInstruction(bool preVisitAction, IRKillInstruction *kinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, kinstr, it);
    depthProcess();
    if (kinstr->getIsKillSampleInstr())
    {
        os << " Kill Sample: " << kinstr->getKillSample() << endl;
    }
    return true;
}

bool ACDXOutputTraverser::visitZExportInstruction(bool preVisitAction, IRZExportInstruction *zxpinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, zxpinstr, it);
    depthProcess();
    if (zxpinstr->getIsExpSampleInstr())
    {
        os << " Export Sample: " << zxpinstr->getExportSample() << endl;
    }
    return true;
}

bool ACDXOutputTraverser::visitCHSInstruction(bool preVisitAction, IRCHSInstruction *chsinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, chsinstr, it);
    return true;
}

bool ACDXOutputTraverser::visitSwizzleComponents(bool , IRSwizzleComponents* swzcomps, ACDXIRTraverser* )
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

bool ACDXOutputTraverser::visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction *swinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction,swinstr,it);
    return true;
}

bool ACDXOutputTraverser::visitDstOperand(bool , IRDstOperand *dstop, ACDXIRTraverser * )
{
    os << "DstOp: ";
    
    if (dstop->getIsVertexResultRegister() || dstop->getIsFragmentResultRegister()) os << "result";
    
    os << dstop->getDestination();
    
    if (dstop->getWriteMask().compare("xyzw"))
        os << " WriteMask: " << dstop->getWriteMask();
    
    os << endl;
    return true;
}

bool ACDXOutputTraverser::visitArrayAddressing(bool , IRArrayAddressing *arrayaddr, ACDXIRTraverser* )
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

bool ACDXOutputTraverser::visitSrcOperand(bool , IRSrcOperand *srcop, ACDXIRTraverser *)
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

bool ACDXOutputTraverser::visitNamingStatement(bool preVisitAction, IRNamingStatement* namingstmnt, ACDXIRTraverser* it)
{
    visitStatement(preVisitAction,namingstmnt,it);
    os << "Naming Statement: ";
    return true;
}

bool ACDXOutputTraverser::visitALIASStatement(bool preVisitAction, IRALIASStatement* aliasstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction,aliasstmnt,it);
    os << "Alias: " << aliasstmnt->getName() << " = " << aliasstmnt->getAlias() << endl;
    return true;    
}

bool ACDXOutputTraverser::visitTEMPStatement(bool preVisitAction, IRTEMPStatement* tempstmnt, ACDXIRTraverser* it)
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

bool ACDXOutputTraverser::visitADDRESSStatement(bool preVisitAction,IRADDRESSStatement* addrstmnt, ACDXIRTraverser* it)
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

bool ACDXOutputTraverser::visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement* attribstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction,attribstmnt,it);
    os << "Vertex Input Attrib: " << attribstmnt->getName() << " = vertex";
    os << attribstmnt->getInputAttribute();
    os << endl;
    return true;
}

bool ACDXOutputTraverser::visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement* attribstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction,attribstmnt,it);
    os << "Fragment Input Attrib: " << attribstmnt->getName() << " = fragment";
    os << attribstmnt->getInputAttribute();
    os << endl;
    return true;
}

bool ACDXOutputTraverser::visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement *outputstmnt, ACDXIRTraverser*it)
{
    visitNamingStatement(preVisitAction,outputstmnt,it);
    os << "Vertex Output Attrib: " << outputstmnt->getName() << " = result";
    os << outputstmnt->getOutputAttribute();
    os << endl;
    return true;
}

bool ACDXOutputTraverser::visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement *outputstmnt, ACDXIRTraverser*it)
{
    visitNamingStatement(preVisitAction,outputstmnt,it);
    os << "Fragment Output Attrib: " << outputstmnt->getName() << " = result";
    os << outputstmnt->getOutputAttribute();
    os << endl;
    return true;
}


bool ACDXOutputTraverser::visitPARAMStatement(bool preVisitAction, IRPARAMStatement* paramstmnt, ACDXIRTraverser* it)
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

bool ACDXOutputTraverser::visitParamBinding(bool , IRParamBinding* parambind, ACDXIRTraverser* )
{
    if (parambind->getIsImplicitBinding()) os << "Implicit ";
    return true;
}

bool ACDXOutputTraverser::visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding* localenvbind, ACDXIRTraverser* it)
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

bool ACDXOutputTraverser::visitConstantBinding(bool preVisitAction, IRConstantBinding* constbind, ACDXIRTraverser* it)
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

bool ACDXOutputTraverser::visitStateBinding(bool preVisitAction, IRStateBinding *statebind, ACDXIRTraverser* it)
{
    visitParamBinding(preVisitAction,statebind,it);
    os << "State: " << statebind->getState() << endl;
    return true;
}

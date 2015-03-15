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

#ifndef ACDX_IRTRAVERSER_H
    #define ACDX_IRTRAVERSER_H

#include <iostream>
#include "ACDXIRNode.h"

namespace acdlib
{

namespace GenerationCode
{

class ACDXIRTraverser {

protected:

    ACDXIRTraverser() : 
        depth(0),
        preVisit(true),
        postVisit(false),
        rightToLeft(false) {}

public:
    

    virtual void depthProcess() {};

    virtual bool visitProgram(bool, IRProgram*, ACDXIRTraverser*) { return true; }
    virtual bool visitVP1Option(bool, VP1IROption*, ACDXIRTraverser*) { return true; }
    virtual bool visitFP1Option(bool, FP1IROption*, ACDXIRTraverser*) { return true; }
    virtual bool visitStatement(bool, IRStatement*, ACDXIRTraverser*) { return true; }
    virtual bool visitInstruction(bool, IRInstruction*, ACDXIRTraverser*) { return true; }
    virtual bool visitSampleInstruction(bool, IRSampleInstruction*, ACDXIRTraverser*) { return true; }
    virtual bool visitKillInstruction(bool, IRKillInstruction*, ACDXIRTraverser*) { return true; }
    virtual bool visitZExportInstruction(bool, IRZExportInstruction*, ACDXIRTraverser*) { return true; }
    virtual bool visitCHSInstruction(bool, IRCHSInstruction*, ACDXIRTraverser*) { return true; }
    virtual bool visitSwizzleComponents(bool, IRSwizzleComponents*, ACDXIRTraverser*) { return true; }
    virtual bool visitSwizzleInstruction(bool, IRSwizzleInstruction*, ACDXIRTraverser*) { return true; }
    virtual bool visitDstOperand(bool, IRDstOperand*, ACDXIRTraverser*) { return true; }
    virtual bool visitSrcOperand(bool, IRSrcOperand*, ACDXIRTraverser*) { return true; }
    virtual bool visitArrayAddressing(bool, IRArrayAddressing*, ACDXIRTraverser*)    { return true;  }
    virtual bool visitNamingStatement(bool, IRNamingStatement*, ACDXIRTraverser*)    { return true; }
    virtual bool visitALIASStatement(bool, IRALIASStatement*, ACDXIRTraverser*)  { return true;  }
    virtual bool visitTEMPStatement(bool, IRTEMPStatement*, ACDXIRTraverser*)    { return true;  }
    virtual bool visitADDRESSStatement(bool, IRADDRESSStatement*, ACDXIRTraverser*) { return true;   }
    virtual bool visitVP1ATTRIBStatement(bool, VP1IRATTRIBStatement*, ACDXIRTraverser*)  {   return true;    }
    virtual bool visitFP1ATTRIBStatement(bool, FP1IRATTRIBStatement*, ACDXIRTraverser*)  {   return true;    }
    virtual bool visitVP1OUTPUTStatement(bool, VP1IROUTPUTStatement*, ACDXIRTraverser*)  {   return true;    }
    virtual bool visitFP1OUTPUTStatement(bool, FP1IROUTPUTStatement*, ACDXIRTraverser*)  {   return true;    }
    virtual bool visitPARAMStatement(bool, IRPARAMStatement*, ACDXIRTraverser*)  {   return true;    }
    virtual bool visitParamBinding(bool, IRParamBinding*, ACDXIRTraverser*) {    return true; }
    virtual bool visitLocalEnvBinding(bool, IRLocalEnvBinding*, ACDXIRTraverser*) { return true; }
    virtual bool visitConstantBinding(bool, IRConstantBinding*, ACDXIRTraverser*) { return true; }
    virtual bool visitStateBinding(bool, IRStateBinding*, ACDXIRTraverser*) { return true;   }
    
    virtual ~ACDXIRTraverser() {}    
    
    int  depth;
    bool preVisit;
    bool postVisit;
    bool rightToLeft;
};

class ACDXOutputTraverser: public ACDXIRTraverser
{
private:
    std::ostream& os;
    unsigned int indentation;
    
    void depthProcess();

public:
    ACDXOutputTraverser(std::ostream& ostr): os(ostr), indentation(0) {};
    virtual bool visitProgram(bool preVisitAction, IRProgram*, ACDXIRTraverser* it);
    virtual bool visitVP1Option(bool preVisitAction, VP1IROption*, ACDXIRTraverser* it);
    virtual bool visitFP1Option(bool preVisitAction, FP1IROption*, ACDXIRTraverser* it);
    virtual bool visitStatement(bool preVisitAction, IRStatement*, ACDXIRTraverser* it);
    virtual bool visitInstruction(bool preVisitAction, IRInstruction*, ACDXIRTraverser* it);
    virtual bool visitSampleInstruction(bool preVisitAction, IRSampleInstruction*, ACDXIRTraverser* it);
    virtual bool visitKillInstruction(bool preVisitAction, IRKillInstruction*, ACDXIRTraverser* it);
    virtual bool visitZExportInstruction(bool, IRZExportInstruction*, ACDXIRTraverser*);
    virtual bool visitCHSInstruction(bool preVisitAction, IRCHSInstruction*, ACDXIRTraverser* it);
    virtual bool visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents*, ACDXIRTraverser* it);
    virtual bool visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction*, ACDXIRTraverser* it);
    virtual bool visitDstOperand(bool preVisitAction, IRDstOperand*, ACDXIRTraverser* it);
    virtual bool visitSrcOperand(bool preVisitAction, IRSrcOperand*, ACDXIRTraverser* it);
    virtual bool visitArrayAddressing(bool preVisitAction, IRArrayAddressing*, ACDXIRTraverser* it);
    virtual bool visitNamingStatement(bool preVisitAction, IRNamingStatement*, ACDXIRTraverser* it);
    virtual bool visitALIASStatement(bool preVisitAction, IRALIASStatement*, ACDXIRTraverser* it);
    virtual bool visitTEMPStatement(bool preVisitAction, IRTEMPStatement*, ACDXIRTraverser* it);
    virtual bool visitADDRESSStatement(bool preVisitAction, IRADDRESSStatement*, ACDXIRTraverser* it);
    virtual bool visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement*, ACDXIRTraverser* it);
    virtual bool visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement*, ACDXIRTraverser* it);
    virtual bool visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement*, ACDXIRTraverser* it);
    virtual bool visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement*, ACDXIRTraverser* it);
    virtual bool visitPARAMStatement(bool preVisitAction, IRPARAMStatement*, ACDXIRTraverser* it);
    virtual bool visitParamBinding(bool preVisitAction, IRParamBinding*, ACDXIRTraverser* it);
    virtual bool visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding*, ACDXIRTraverser* it);
    virtual bool visitConstantBinding(bool preVisitAction, IRConstantBinding*, ACDXIRTraverser* it);
    virtual bool visitStateBinding(bool preVisitAction, IRStateBinding*, ACDXIRTraverser* it);
    
};  

} // namespace GenerationCode

} // namespace acdlib

#endif // ACDX_IRTRAVERSER_H

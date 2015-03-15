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

#ifndef IRTRAVERSER_H
    #define IRTRAVERSER_H

#include <iostream>
#include "IRNode.h"

namespace libgl
{

namespace GenerationCode
{

class IRTraverser {

protected:

    IRTraverser() : 
        depth(0),
        preVisit(true),
        postVisit(false),
        rightToLeft(false) {}

public:
    

    virtual void depthProcess() {};

    virtual bool visitProgram(bool preVisitAction, IRProgram*, IRTraverser*) { return true; }
    virtual bool visitVP1Option(bool preVisitAction, VP1IROption*, IRTraverser*) { return true; }
    virtual bool visitFP1Option(bool preVisitAction, FP1IROption*, IRTraverser*) { return true; }
    virtual bool visitStatement(bool preVisitAction, IRStatement*, IRTraverser*) { return true; }
    virtual bool visitInstruction(bool preVisitAction, IRInstruction*, IRTraverser*) { return true; }
    virtual bool visitSampleInstruction(bool preVisitAction, IRSampleInstruction*, IRTraverser*) { return true; }
    virtual bool visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents*, IRTraverser*) { return true; }
    virtual bool visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction*, IRTraverser*) { return true; }
    virtual bool visitDstOperand(bool preVisitAction, IRDstOperand*, IRTraverser*) { return true; }
    virtual bool visitSrcOperand(bool preVisitAction, IRSrcOperand*, IRTraverser*) { return true; }
    virtual bool visitArrayAddressing(bool preVisitAction, IRArrayAddressing*, IRTraverser*)    { return true;  }
    virtual bool visitNamingStatement(bool preVisitAction, IRNamingStatement*, IRTraverser*)    { return true; }
    virtual bool visitALIASStatement(bool preVisitAction, IRALIASStatement*, IRTraverser*)  { return true;  }
    virtual bool visitTEMPStatement(bool preVisitAction, IRTEMPStatement*, IRTraverser*)    { return true;  }
    virtual bool visitADDRESSStatement(bool preVisitAction, IRADDRESSStatement*, IRTraverser*) { return true;   }
    virtual bool visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement*, IRTraverser*)  {   return true;    }
    virtual bool visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement*, IRTraverser*)  {   return true;    }
    virtual bool visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement*, IRTraverser*)  {   return true;    }
    virtual bool visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement*, IRTraverser*)  {   return true;    }
    virtual bool visitPARAMStatement(bool preVisitAction, IRPARAMStatement*, IRTraverser*)  {   return true;    }
    virtual bool visitParamBinding(bool preVisitAction, IRParamBinding*, IRTraverser*) {    return true; }
    virtual bool visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding*, IRTraverser*) { return true; }
    virtual bool visitConstantBinding(bool preVisitAction, IRConstantBinding*, IRTraverser*) { return true; }
    virtual bool visitStateBinding(bool preVisitAction, IRStateBinding*, IRTraverser*) { return true;   }
    
    virtual ~IRTraverser() {}    
    
    int  depth;
    bool preVisit;
    bool postVisit;
    bool rightToLeft;
};





class OutputTraverser: public IRTraverser
{
private:
    std::ostream& os;
    unsigned int indentation;
    
    void depthProcess();

public:
    OutputTraverser(std::ostream& ostr): os(ostr), indentation(0) {};
    virtual bool visitProgram(bool preVisitAction, IRProgram*, IRTraverser*);
    virtual bool visitVP1Option(bool preVisitAction, VP1IROption*, IRTraverser*);
    virtual bool visitFP1Option(bool preVisitAction, FP1IROption*, IRTraverser*);
    virtual bool visitStatement(bool preVisitAction, IRStatement*, IRTraverser*);
    virtual bool visitInstruction(bool preVisitAction, IRInstruction*, IRTraverser*);
    virtual bool visitSampleInstruction(bool preVisitAction, IRSampleInstruction*, IRTraverser*);
    virtual bool visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents*, IRTraverser*);
    virtual bool visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction*, IRTraverser*);
    virtual bool visitDstOperand(bool preVisitAction, IRDstOperand*, IRTraverser*);
    virtual bool visitSrcOperand(bool preVisitAction, IRSrcOperand*, IRTraverser*);
    virtual bool visitArrayAddressing(bool preVisitAction, IRArrayAddressing*, IRTraverser*);
    virtual bool visitNamingStatement(bool preVisitAction, IRNamingStatement*, IRTraverser*);
    virtual bool visitALIASStatement(bool preVisitAction, IRALIASStatement*, IRTraverser*);
    virtual bool visitTEMPStatement(bool preVisitAction, IRTEMPStatement*, IRTraverser*);
    virtual bool visitADDRESSStatement(bool preVisitAction, IRADDRESSStatement*, IRTraverser*);
    virtual bool visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement*, IRTraverser*);
    virtual bool visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement*, IRTraverser*);
    virtual bool visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement*, IRTraverser*);
    virtual bool visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement*, IRTraverser*);
    virtual bool visitPARAMStatement(bool preVisitAction, IRPARAMStatement*, IRTraverser*);
    virtual bool visitParamBinding(bool preVisitAction, IRParamBinding*, IRTraverser*);
    virtual bool visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding*, IRTraverser*);
    virtual bool visitConstantBinding(bool preVisitAction, IRConstantBinding*, IRTraverser*);
    virtual bool visitStateBinding(bool preVisitAction, IRStateBinding*, IRTraverser*);
    
};  

} // namespace GenerationCode

} // namespace libgl

#endif // IRTRAVERSER_H

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

#ifndef SCHEDULER_H
    #define SCHEDULER_H

#include <vector>
#include <list>
#include <iostream>
#include <utility>
#include "ShaderInstruction.h"
#include "DependencyGraph.h"

namespace libgl
{

namespace GenerationCode
{

class Register
{
public:

    unsigned int bank;
    unsigned int reg;
    bool componentX;
    bool componentY;
    bool componentZ;
    bool componentW;
    bool negate;
    bool absolute;
    unsigned int swizzleMode;

    Register();
    Register(unsigned int bank, unsigned int reg, unsigned int swizzleMode, bool componentX , bool componentY,
             bool componentZ , bool componentW, bool negate, bool absolute);

    bool matches(const Register& r) const
    {
        return ( (r.bank == bank) && (r.reg == reg) &&
                ( r.componentX && componentX || 
                r.componentY && componentY || 
                r.componentZ && componentZ || 
                r.componentW && componentW ) );
    }
    
    bool matches(unsigned int component) const
    {
        switch(component)
        {
            case 0: return componentX;
            case 1: return componentY;
            case 2: return componentZ;
            case 3: return componentW;
            default: return false;
        }
    }
    
    bool isTemporary() const;
    
    bool isOutputRegister() const;
    
    bool isAddressRegister() const;
    
    unsigned int swizzle() const;

    void print(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const Register& r)
    {
        r.print(os);
        return os;
    }

};

class OperationInfo
{
public: 
    
    unsigned int opcode;
    unsigned int numOperands;
    bool writesAnyRegister;
    bool saturate;
    bool relative;
    unsigned int relModeComp;
    unsigned int relModeOffset;
    unsigned int relModeReg;
    bool setCC;
    unsigned int executionLatency;
    
    OperationInfo();
    OperationInfo(unsigned int opcode, unsigned int numOperands, bool writesAnyRegister, bool saturate,
                  bool relative, unsigned int relModeComp,  unsigned int relModeOffset, unsigned int relModeReg,
                  unsigned int executionLatency);
    
    void print(std::ostream& os) const;
    
    friend std::ostream& operator<<(std::ostream& os, const OperationInfo& oi)
    {
        oi.print(os);
        return os;
    }

};

class InstructionInfo
{
public:

    unsigned int line;
    std::string str;

    OperationInfo operation;
    
    Register writeReg;
    Register readReg0;
    Register readReg1;
    Register readReg2;

    void print(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const InstructionInfo& ii)
    {
        ii.print(os);
        return os;
    }

};
    
typedef std::pair<unsigned int /* instruction id */,unsigned int /* issue cycle */> IssueInfo;

class Scheduler
{

/**
 * Scheduler class for ShaderInstructionTranslator optimizations.
 *  Performs basic instruction reordering optimizations based on data
 *  dependencies and structural dependencies of Shader Unit.
 *
 * @author Jordi Roca Monfort - jroca@ac.upc.edu
 * @date 13/4/2005
 * @ver 1.0
 *
 */
private:
    
    /* Auxiliar functions */

    mutable u32bit maxAliveTemps;
    mutable bool maxAliveTempsComputed;

    std::vector<InstructionInfo*>* extractInstructionInfo(const std::list<gpu3d::ShaderInstruction*>& originalCode);

    DependencyGraph* buildDependencyGraph(const std::vector<InstructionInfo*>& code, unsigned int temporaries, unsigned int outputRegs, unsigned int addrRegs) const;

    void printDependencyGraphAtCycle(const DependencyGraph& dg, unsigned int cycle, bool initialGraph = false) const;

    std::list<unsigned int> buildInitialUnScheduledSet(const std::vector<InstructionInfo*>& code) const;
    
    void reorderCode(std::vector<InstructionInfo*>& originalCode, const std::vector<IssueInfo>& orderList) const;

    std::list<gpu3d::ShaderInstruction*> createNewCode(const std::vector<InstructionInfo*>& newcode) const;

public:

    Scheduler();
    
    std::list<gpu3d::ShaderInstruction*> optimizeCode(std::list<gpu3d::ShaderInstruction*>& originalCode, unsigned int nway = 4, unsigned int temporaries = 16,  unsigned int outputRegs = 12, unsigned int addrRegs = 1, bool reorderCode = true);

    u32bit getMaxAliveTemps() const;

    ~Scheduler();
};


} // namespace GenerationCode

} // namespace libgl


#endif // SCHEDULER_H

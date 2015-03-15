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

#include "Scheduler.h"
#include "CompilerSteps.h"
#include <fstream>
#include <sstream>
#include <queue>

#include "ShaderArchitectureParameters.h"

using namespace std;
using namespace libgl;
using namespace GenerationCode;
using namespace gpu3d;

/////////////////////////////////////////////////////////////////////////
//////////// Implementation of Register, OperationInfo and //////////////
//////////////////////// InstructionInfo classes ////////////////////////
/////////////////////////////////////////////////////////////////////////

Register::Register()
    : bank(0), reg(0), swizzleMode(XYWZ), componentX(true), componentY(true),
      componentZ(true), componentW(true), negate(false), absolute(false)
{

}

Register::Register(unsigned int bank, unsigned int reg, unsigned int swizzleMode, bool componentX,
                   bool componentY, bool componentZ, bool componentW, bool negate, bool absolute)
    : bank(bank), reg(reg), swizzleMode(swizzleMode), componentX(componentX), componentY(componentY),
      componentZ(componentZ), componentW(componentW), negate(negate), absolute(absolute)
{
}

bool Register::isTemporary() const
{
    return (bank == TEMP);
}

bool Register::isOutputRegister() const
{
    return (bank == OUT);
}

bool Register::isAddressRegister() const
{
    return (bank == ADDR);
}

unsigned int Register::swizzle() const
{
    return swizzleMode;
}

void Register::print(ostream& os) const
{
    os << "BankId: " << bank << " ";
    os << "RegId: " << reg << " ";
    os << "Components used: ";

    if (componentX) os << "X";
    if (componentY) os << "Y";
    if (componentZ) os << "Z";
    if (componentW) os << "W";

    os << "Flags: ";
    if (negate) os << "N ";
    if (absolute) os << "A ";
    os << endl;
}

OperationInfo::OperationInfo()
{
}

OperationInfo::OperationInfo(unsigned int opcode, unsigned int numOperands,
                             bool writesAnyRegister, bool saturate,
                             bool relative, unsigned int relModeComp,
                             unsigned int relModeOffset, unsigned int relModeReg,
                             unsigned int executionLatency)

    : opcode(opcode), numOperands(numOperands), writesAnyRegister(writesAnyRegister),
      saturate(saturate), relative(relative),
      relModeComp(relModeComp), relModeOffset(relModeOffset), relModeReg(relModeReg),
      executionLatency(executionLatency)
{
}

void OperationInfo::print(ostream& os) const
{
    os << "0x" << hex << opcode;
    os << dec << " Latency: " << executionLatency << endl;
}

void InstructionInfo::print(ostream& os) const
{
    os << line << ": ";
    os << operation;

    if (operation.writesAnyRegister) os << "WriteReg: " << writeReg << " ";
    if (operation.numOperands > 0) os << "ReadReg0: " << readReg0 << " ";
    if (operation.numOperands > 1) os << "ReadReg1: " << readReg1 << " ";
    if (operation.numOperands > 2) os << "ReadReg2: " << readReg2 << " ";

    os << "Flags: ";
    if (operation.saturate) os << "S ";
    os << endl;
}

/////////////////////////////////////////////////////////////////////////
////////////////// Implementation of Scheduler class ////////////////////
/////////////////////////////////////////////////////////////////////////

Scheduler::Scheduler() : maxAliveTempsComputed(false)
{
}

vector<InstructionInfo*>* Scheduler::extractInstructionInfo(const list<ShaderInstruction*>& originalCode)
{
    vector<InstructionInfo*>* returnedList = new vector<InstructionInfo*>;

    ShaderArchitectureParameters *shArchParams = ShaderArchitectureParameters::getShaderArchitectureParameters();

    unsigned int instructionCounter = 0;

    list<ShaderInstruction*>::const_iterator iter = originalCode.begin();

    while( iter != originalCode.end() )
    {
            InstructionInfo *ii = new InstructionInfo;

            ii->line = instructionCounter;
            char instr[128];
            (*iter)->disassemble(instr);
            ii->str = string(instr);
            ii->operation = OperationInfo((*iter)->getOpcode(),
                                        (*iter)->getNumOperands(),
                                        (((*iter)->getOpcode() != NOP) && ((*iter)->getOpcode() != KIL) && ((*iter)->getOpcode() != KLS) && ((*iter)->getOpcode() != ZXP) && ((*iter)->getOpcode() != ZXS)),
                                        (*iter)->getSaturatedRes(),
                                        (*iter)->getRelativeModeFlag(),
                                        (*iter)->getRelMAddrRegComp(),
                                        (*iter)->getRelMOffset(),
                                        (*iter)->getRelMAddrReg(),
                                        shArchParams->getExecutionLatency((*iter)->getOpcode()));
                                        //latencyTable[(*iter)->getOpcode()]);

            if (ii->operation.writesAnyRegister)
            {
                ii->writeReg = Register((unsigned int)(*iter)->getBankRes(),
                                        (unsigned int)(*iter)->getResult(),
                                        (unsigned int)(*iter)->getResultMaskMode(),
                                        ((*iter)->getResultMaskMode() & 0x8)> 0 ? true : false,
                                        ((*iter)->getResultMaskMode() & 0x4)> 0 ? true : false,
                                        ((*iter)->getResultMaskMode() & 0x2)> 0 ? true : false,
                                        ((*iter)->getResultMaskMode() & 0x1)> 0 ? true : false,
                                        false, false);
            }
            if (ii->operation.numOperands > 0)
            {
                ii->readReg0 = Register((unsigned int)(*iter)->getBankOp1(),
                                        (unsigned int)(*iter)->getOp1(),
                                        (unsigned int)(*iter)->getOp1SwizzleMode(),
                                        (bool)( (((*iter)->getOp1SwizzleMode() & 0xC0) == 0x00 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x30) == 0x00 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x0C) == 0x00 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x03) == 0x00 ) ),
                                        (bool)( (((*iter)->getOp1SwizzleMode() & 0xC0) == 0x40 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x30) == 0x10 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x0C) == 0x04 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x03) == 0x01 ) ),
                                        (bool)( (((*iter)->getOp1SwizzleMode() & 0xC0) == 0x80 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x30) == 0x20 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x0C) == 0x08 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x03) == 0x02 ) ),
                                        (bool)( (((*iter)->getOp1SwizzleMode() & 0xC0) == 0xC0 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x30) == 0x30 ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x0C) == 0x0C ) ||
                                                (((*iter)->getOp1SwizzleMode() & 0x03) == 0x03 ) ),
                                                (*iter)->getOp1NegateFlag(), (*iter)->getOp1AbsoluteFlag());
            }
            if (ii->operation.numOperands > 1)
            {
                ii->readReg1 = Register((unsigned int)(*iter)->getBankOp2(),
                                        (unsigned int)(*iter)->getOp2(),
                                        (unsigned int)(*iter)->getOp2SwizzleMode(),
                                        (bool)( (((*iter)->getOp2SwizzleMode() & 0xC0) == 0x00 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x30) == 0x00 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x0C) == 0x00 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x03) == 0x00 ) ),
                                        (bool)( (((*iter)->getOp2SwizzleMode() & 0xC0) == 0x40 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x30) == 0x10 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x0C) == 0x04 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x03) == 0x01 ) ),
                                        (bool)( (((*iter)->getOp2SwizzleMode() & 0xC0) == 0x80 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x30) == 0x20 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x0C) == 0x08 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x03) == 0x02 ) ),
                                        (bool)( (((*iter)->getOp2SwizzleMode() & 0xC0) == 0xC0 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x30) == 0x30 ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x0C) == 0x0C ) ||
                                                (((*iter)->getOp2SwizzleMode() & 0x03) == 0x03 ) ),
                                               (*iter)->getOp2NegateFlag(), (*iter)->getOp2AbsoluteFlag());
            }
            if (ii->operation.numOperands > 2)
            {
                ii->readReg2 = Register((unsigned int)(*iter)->getBankOp3(),
                                        (unsigned int)(*iter)->getOp3(),
                                        (unsigned int)(*iter)->getOp3SwizzleMode(),
                                        (bool)( (((*iter)->getOp3SwizzleMode() & 0xC0) == 0x00 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x30) == 0x00 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x0C) == 0x00 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x03) == 0x00 ) ),
                                        (bool)( (((*iter)->getOp3SwizzleMode() & 0xC0) == 0x40 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x30) == 0x10 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x0C) == 0x04 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x03) == 0x01 ) ),
                                        (bool)( (((*iter)->getOp3SwizzleMode() & 0xC0) == 0x80 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x30) == 0x20 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x0C) == 0x08 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x03) == 0x02 ) ),
                                        (bool)( (((*iter)->getOp3SwizzleMode() & 0xC0) == 0xC0 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x30) == 0x30 ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x0C) == 0x0C ) ||
                                                (((*iter)->getOp3SwizzleMode() & 0x03) == 0x03 ) ),
                                                (*iter)->getOp3NegateFlag(), (*iter)->getOp3AbsoluteFlag());
            }
            returnedList->push_back(ii);
            instructionCounter++;
        iter++;

    }
    return returnedList;
}


DependencyGraph* Scheduler::buildDependencyGraph(const vector<InstructionInfo*>& code, unsigned int temporaries, unsigned int outputRegs, unsigned int addrRegs) const
{
    DependencyGraph* dg = new DependencyGraph(code.size());

    /* Revise all the code inserting dependencies into graph */

    /*
     * Track history of last instructions that writes and reads
     * each temporary register in a component-wise form :
    *
     * lastWriter[0] is t0.x
     * lastWriter[1] is t0.y
     * ...
     * lastWriter[4] is t1.x
     * ...
     */

    vector<vector<int> > registerWriters(temporaries*4);
    
    vector<vector<int> > outputRegisterWriters(outputRegs*4);
    
    vector<vector<int> > addressRegisterWriters(addrRegs*4);
    
    vector<vector<int> > registerReaders(temporaries*4);
    
    vector<vector<int> > addressRegisterReaders(addrRegs*4);

    vector<InstructionInfo*>::const_iterator iter = code.begin();

    while( iter != code.end() )
    {
        dg->insertInstructionInfo((*iter)->line,(*iter)->str,(*iter)->operation.executionLatency);

        int aux;

        if ((*iter)->operation.writesAnyRegister)
        {
            if ((*iter)->writeReg.isTemporary())
            {
                
                for (int i=0; i<4; i++) /* Loop over all register components. */
                {
                    if ((*iter)->writeReg.matches(i))
                    {
                        /* Check for false dependencies with temporal register readers */

                        vector<int>::const_iterator iter2 = registerReaders[(*iter)->writeReg.reg * 4 + i].begin();
                        
                        while ( iter2 != registerReaders[(*iter)->writeReg.reg * 4 + i].end() )
                        {
                            if ((*iter2) != (*iter)->line) /* Avoid self-instruction dependencies */
                            {
                                dg->insertFalseDependency(code[(*iter2)]->line, (*iter)->line);
                            }
                            iter2++;
                        }

                        /* Check for output dependencies with last temporary register writer */

                        if (registerWriters[(*iter)->writeReg.reg * 4 + i].size() > 0)
                        {
                            aux = registerWriters[(*iter)->writeReg.reg * 4 + i].back();
                            
                            if (aux != (*iter)->line) /* Avoid self-instruction dependencies */
                            {
                                    dg->insertOutputDependency(code[aux]->line, (*iter)->line);
                            }
                        }
                    }

                }
            }
            
            else if ((*iter)->writeReg.isOutputRegister())
            {          

                /* Output registers have not false dependences because they cannot be readed */
                
                for (int i=0; i<4; i++) /* Loop over all register components. */
                {
                    if ((*iter)->writeReg.matches(i))
                    {
                        /* Check for output dependencies with output register writer */
                        if (outputRegisterWriters[(*iter)->writeReg.reg * 4 + i].size() > 0)
                        {
                            aux = outputRegisterWriters[(*iter)->writeReg.reg * 4 + i].back();
                            
                            if (aux != (*iter)->line) /* Avoid self-instruction dependencies */
                            {
                                    dg->insertOutputDependency(code[aux]->line, (*iter)->line);
                            }
                        }
                    }

                }
            }
            else if ((*iter)->writeReg.isAddressRegister())
            {
                for (int i=0; i<4; i++) /* Loop over all register components. */
                {
                    if ((*iter)->writeReg.matches(i))
                    {
                        /* Check for false dependencies with instructions using relative address register */

                        vector<int>::const_iterator iter2 = addressRegisterReaders[(*iter)->writeReg.reg * 4 + i].begin();
                        
                        while ( iter2 != addressRegisterReaders[(*iter)->writeReg.reg * 4 + i].end() )
                        {
                            if ((*iter2) != (*iter)->line) /* Avoid self-instruction dependencies */
                            {
                                dg->insertFalseDependency(code[(*iter2)]->line, (*iter)->line);
                            }
                            iter2++;
                        }

                        /* Check for output dependencies with last ARL instruction */

                        if (addressRegisterWriters[(*iter)->writeReg.reg * 4 + i].size() > 0)
                        {
                            aux = addressRegisterWriters[(*iter)->writeReg.reg * 4 + i].back();
                            
                            if (aux != (*iter)->line) /* Avoid self-instruction dependencies */
                            {
                                    dg->insertOutputDependency(code[aux]->line, (*iter)->line);
                            }
                        }
                    }

                }
            }
            else
                panic("Scheduler","buildDependencyGraph","Instruction writes a register that is not a temporal, and output or an address register");
        }

       /* Check for true dependencies of instruction read operands with last register writer */

        if ((*iter)->operation.numOperands > 0)
        {
            if ((*iter)->readReg0.isTemporary())
            {
                for (int i=0; i<4; i++)
                {
                    if ((*iter)->readReg0.matches(i))
                    {
                        if (registerWriters[(*iter)->readReg0.reg * 4 + i].size() > 0)
                        {
                             aux = registerWriters[(*iter)->readReg0.reg * 4 + i].back();
                             
                             if (aux != (*iter)->line) /* Avoid self-instruction dependencies */

                                  dg->insertTrueDependency(code[aux]->line, (*iter)->line);
                        }

                        registerReaders[(*iter)->readReg0.reg * 4 + i].push_back((*iter)->line);
                    }
                }
            }
        }

        if ((*iter)->operation.numOperands > 1)
        {
            if ((*iter)->readReg1.isTemporary())
            {
                for (int i=0; i<4; i++)
                {
                    if ((*iter)->readReg1.matches(i))
                    {
                        if (registerWriters[(*iter)->readReg1.reg * 4 + i].size() > 0)
                        {
                             aux = registerWriters[(*iter)->readReg1.reg * 4 + i].back();
                             
                             if (aux != (*iter)->line) /* Avoid self-instruction dependencies */

                                  dg->insertTrueDependency(code[aux]->line, (*iter)->line);
                        }

                        registerReaders[(*iter)->readReg1.reg * 4 + i].push_back((*iter)->line);
                    }
                }
            }
        }

        if ((*iter)->operation.numOperands > 2)
        {
            if ((*iter)->readReg2.isTemporary())
            {
                for (int i=0; i<4; i++)
                {
                    if ((*iter)->readReg2.matches(i))
                    {
                        if (registerWriters[(*iter)->readReg2.reg * 4 + i].size() > 0)
                        {
                             aux = registerWriters[(*iter)->readReg2.reg * 4 + i].back();
                             
                             if (aux != (*iter)->line) /* Avoid self-instruction dependencies */

                                  dg->insertTrueDependency(code[aux]->line, (*iter)->line);
                        }
                        registerReaders[(*iter)->readReg2.reg * 4 + i].push_back((*iter)->line);
                    }
                }
            }
        }

        /* Check for true dependencies with relative address registers writers, namely, ARLïs. */
        
        if ((*iter)->operation.relative)
        {
            for (int i=0; i<4; i++)
            {
                if ((*iter)->operation.relModeComp == i)
                {
                    if (addressRegisterWriters[(*iter)->operation.relModeReg * 4 + i].size() > 0)
                    {
                         aux = addressRegisterWriters[(*iter)->operation.relModeReg * 4 + i].back();
                         
                         if (aux != (*iter)->line) /* Avoid self-instruction dependencies */
        
                              dg->insertTrueDependency(code[aux]->line, (*iter)->line);
                              
                    }
                    addressRegisterReaders[(*iter)->operation.relModeReg * 4 + i].push_back((*iter)->line);           
                }
            }
        }
        
        /* Push back current instruction as last register writer. Itïs necessary to do it here because we
         * need to avoid that and instruction detects its result register as a RaW dependence.
         */  

        if ((*iter)->operation.writesAnyRegister)

            if ((*iter)->writeReg.isTemporary())
            {
                for (int i=0; i<4; i++)

                    if ((*iter)->writeReg.matches(i)) registerWriters[(*iter)->writeReg.reg *4 + i].push_back((*iter)->line);
            }
            else if ((*iter)->writeReg.isOutputRegister())
            {
                for (int i=0; i<4; i++)

                    if ((*iter)->writeReg.matches(i)) outputRegisterWriters[(*iter)->writeReg.reg *4 + i].push_back((*iter)->line);
            }
            else if ((*iter)->writeReg.isAddressRegister())
            {
                for (int i=0; i<4; i++)
                
                    if ((*iter)->writeReg.matches(i)) addressRegisterWriters[(*iter)->writeReg.reg *4 + i].push_back((*iter)->line);
            }
            else
                panic("Scheduler","buildDependencyGraph","Instruction writes a register that is not a temporal, and output or an address register");
                
        iter++;
    }

    return dg;
}

list<unsigned int> Scheduler::buildInitialUnScheduledSet(const vector<InstructionInfo*>& code) const
{
    list<unsigned int> returnedList;

    vector<InstructionInfo*>::const_iterator iter = code.begin();

    while( iter != code.end() )
    {
        returnedList.push_back((*iter)->line);
        iter++;
    }

    return returnedList;
}



void Scheduler::reorderCode(std::vector<InstructionInfo*>& originalCode, const vector<IssueInfo>& orderList) const
{
    // Copy the initial vector. This makes instruction reordering safe.
    vector<InstructionInfo*> aux = originalCode;

    // Insert in original code the required space to hold new
    // instructions inserted by scheduler

    unsigned int count = 0;

    vector<InstructionInfo*>::iterator iter = originalCode.begin();

    while( count < originalCode.size() )
    {
        (*iter) = aux[orderList[count].first];
        count++;
        iter++;
    }

    unsigned int NOPsToInsert = orderList.size() - originalCode.size();

    // Insert the auxiliary NOP instructions at lasts positions
    for(unsigned int i=0; i < NOPsToInsert; i++)
    {
        InstructionInfo *nop = new InstructionInfo;
        nop->operation.opcode = NOP;
        nop->operation.numOperands = 0;
        nop->operation.writesAnyRegister = false;
        originalCode.push_back(nop);
    }
}

list<ShaderInstruction*> Scheduler::createNewCode(const vector<InstructionInfo*>& newcode) const
{
    list<ShaderInstruction*> returned;
    vector<InstructionInfo*>::const_iterator iter = newcode.begin();
    unsigned int usefulInstructions = 0;
    unsigned int instructionCount = 0;

    bool found = false;
    
    while ( iter != newcode.end() & !found )
    {  
        if ((*iter)->operation.opcode == NOP)
            found = true;
        else
            usefulInstructions++;
            
        iter++;
    }
    
    unsigned int lastPosition = usefulInstructions - 1;
    
    // Rewind the iterator
    iter = newcode.begin();
    
    while( iter != newcode.end() )
    {
        bool lastInstruction = (instructionCount == lastPosition);

        returned.push_back(new ShaderInstruction((ShOpcode)(*iter)->operation.opcode,
                                                  (Bank)(*iter)->readReg0.bank,
                                                  (*iter)->readReg0.reg,
                                                  (*iter)->readReg0.negate,(*iter)->readReg0.absolute,
                                                  (SwizzleMode)(*iter)->readReg0.swizzle(),
                                                  (Bank)(*iter)->readReg1.bank,
                                                  (*iter)->readReg1.reg,
                                                  (*iter)->readReg1.negate,(*iter)->readReg1.absolute,
                                                  (SwizzleMode)(*iter)->readReg1.swizzle(),
                                                  (Bank)(*iter)->readReg2.bank,(*iter)->readReg2.reg,
                                                  (*iter)->readReg2.negate,(*iter)->readReg2.absolute,
                                                  (SwizzleMode)(*iter)->readReg2.swizzle(),
                                                  (Bank)(*iter)->writeReg.bank,
                                                  (*iter)->writeReg.reg,
                                                  (*iter)->operation.saturate,
                                                  (MaskMode)(*iter)->writeReg.swizzle(),
                                                  false, false, 0,
                                                  (*iter)->operation.relative,(*iter)->operation.relModeReg,
                                                  (*iter)->operation.relModeComp,(*iter)->operation.relModeOffset,
                                                  lastInstruction));

        instructionCount++;
        iter++;
    }
    return returned;
}

void Scheduler::printDependencyGraphAtCycle(const DependencyGraph& dg, unsigned int cycle, bool initialGraph) const
{
    stringstream ss;

    ss << cycle;

    string str;

    if (initialGraph)
    {
        str = "grap0000.dot"; // This name is used to be alphabetical order less than graphXXX.dot, and
                              // file is displayed before anyone else by the Image Display App.
    }
    else
    {
        str = "graph";
        for (unsigned int i=0; i< 3- (ss.str().size()); i++) str += "0";
        str += ss.str() + ".dot";
    }

    ofstream of(str.c_str());

    dg.printDOTFormat(of);

    of.close();
}

list<ShaderInstruction*> Scheduler::optimizeCode(list<ShaderInstruction*>& originalCode, unsigned int nway, unsigned int temporaries, unsigned int outputRegs, unsigned int addrRegs, bool _reorderCode)
{   
    /**************************************************************/
    /* Extract the essential instruction info needed for analysis */
    /**************************************************************/
    vector<InstructionInfo*>* code = extractInstructionInfo(originalCode);
    DependencyGraph* depgraph = 0;

    if ( !_reorderCode )
    {
        maxAliveTempsComputed = true;
        maxAliveTemps = computeMaxLiveTemps(*code);
        return list<ShaderInstruction*>();
    }

    /******************************/
    /* Initial optimization steps */
    /******************************/

    //basicOptimizations(*code);

    /******************************/
    /*      Register Renaming     */
    /******************************/

    //registerRenaming(*code, temporaries);


    /* Reserve the last instruction index + 1 to NOP references */
    unsigned int nopBaseRef = code->size();

    /**********************************************/
    /* Build dependency graph needed for analysis */
    /**********************************************/

    //DependencyGraph* depgraph = buildDependencyGraph(*code, temporaries, outputRegs, addrRegs);
    depgraph = buildDependencyGraph(*code, temporaries, outputRegs, addrRegs);

    /* Print pre-scheduling dependency graph */
    //printDependencyGraphAtCycle(*depgraph,0,true);

    /***********************************************/
    /* Build initial sets for scheduling algorithm */
    /***********************************************/

    vector<int> chosenCandidate(nway);

    list<unsigned int> unscheduledSet = buildInitialUnScheduledSet(*code);
    vector<IssueInfo> scheduledSet;
    list<unsigned int> readySet;

    /***************************/
    /*   Start Scheduling      */
    /***************************/

    unsigned int cycle = 0;

    while( !unscheduledSet.empty() )
    {
    // Get the instructions that potentially can be scheduled at the time
    // because of register dependencies.
    readySet = getReadyOps(*depgraph, cycle, scheduledSet, *code);

    // Sort the ready ops by some special policy like f.e: maximize free dependence instructions.
    sortReadyOps(readySet, *depgraph);

    /* Try to assign a ready op to each way */
        for(unsigned int i=0; i < nway; i++)
        {
            if (!readySet.empty())
            {
                /* Good luck case: We have instructions to assing */
                chosenCandidate[i] = readySet.front();
                readySet.remove(chosenCandidate[i]);
                scheduledSet.push_back(make_pair(chosenCandidate[i],cycle));
                unscheduledSet.remove(chosenCandidate[i]);
                depgraph->setScheduled(chosenCandidate[i],cycle);
            }
            else
            {
                /* Bad luck case: No ready instructions are available.
                 * Take some policy, f.e: schedule whatever instruction not scheduled yet.
                 */
                if (!unscheduledSet.empty())
                {
                    chosenCandidate[i] = unscheduledSet.front();
                    scheduledSet.push_back(make_pair(chosenCandidate[i],cycle));
                    unscheduledSet.remove(chosenCandidate[i]);
                    depgraph->setScheduled(chosenCandidate[i],cycle);
                }
                else
                {
                    /* No more unscheduled instructions available. Insert NOP´s. */
                    chosenCandidate[i] = nopBaseRef;
                    nopBaseRef++;
                    scheduledSet.push_back(make_pair(chosenCandidate[i],cycle));
                }
            }
        }
        /* Debug Scheduling steps */
        //printDependencyGraphAtCycle(*depgraph,cycle);

        cycle++;
    }

    /********************************/
    /* Final Instruction Reordering */
    /********************************/
    //if (_reorderCode) 
        reorderCode(*code, scheduledSet);

    /******************************************************
     * Compute max alive registers for the reordered code *
     ******************************************************/
    maxAliveTempsComputed = true;
    maxAliveTemps = computeMaxLiveTemps(*code);

    /********************************/
    /*  Final code reconstruction   */
    /********************************/
    list<ShaderInstruction*> returnedCode = createNewCode(*code);

    /************************************/
    /*    Delete auxiliar structures    */
    /************************************/
    vector<InstructionInfo*>::iterator iter = code->begin();

    while( iter != code->end() )
    {
        delete (*iter);
        iter++;
    }
    
    code->clear();
    
    delete code;
    
    delete depgraph;
        
    return returnedCode;
}

u32bit Scheduler::getMaxAliveTemps() const
{
    if ( maxAliveTempsComputed )
        return maxAliveTemps;
    panic("Scheduler", "getMaxAliveTemps", "Optimize code must be called before call this method");
    return 0;
}

Scheduler::~Scheduler()
{
}

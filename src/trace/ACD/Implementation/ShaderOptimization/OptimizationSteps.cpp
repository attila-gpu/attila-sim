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

#include "OptimizationSteps.h"

#include <map>
#include <sstream>
#include <fstream>
#include <queue>

using namespace acdlib;
using namespace acdlib_opt;
using namespace std;

//////////////////////////////////////////////////////////////////////////
//                MaxLiveTempsAnalysis Implementation                        //
//////////////////////////////////////////////////////////////////////////

MaxLiveTempsAnalysis::MaxLiveTempsAnalysis(ShaderOptimizer* shOptimizer)
: OptimizationStep(shOptimizer)
{
}

void MaxLiveTempsAnalysis::optimize()
{
    set<pair<acd_uint,acd_uint> > ranges;

    map<acd_uint, acd_uint> defs;
    map<acd_uint, acd_uint>::iterator defIter;

    vector<InstructionInfo*> code = _shOptimizer->_inputInstrInfoVect;

    vector<InstructionInfo*>::iterator it = code.begin();

    // Find first definition of ech temporary register
    for ( ; it != code.end(); it++ )
    {
        if ( (*it)->writeReg.isTemporary() )
        {
            acd_uint idReg = (*it)->writeReg.reg;
            defIter = defs.find(idReg);
            if ( defIter == defs.end() )
                defs.insert(make_pair(idReg, (*it)->line));
        }
    }

    /*
    cout << "\n---------------------" << endl;
    for ( defIter = defs.begin(); defIter != defs.end(); defIter++ )
        cout << "Def: temp" << defIter->first << " defined at position: " << defIter->second << endl;
    return 0;
    */
    // Find last use of each register
    vector<InstructionInfo*>::reverse_iterator itRev = code.rbegin();

    for ( ; itRev != code.rend(); itRev++ )
    {
        acd_uint nOperands = (*itRev)->operation.numOperands;
        if ( nOperands >= 1 )
        {
            // check first operand
            if ( (*itRev)->readReg0.isTemporary() )
            {
                // find definition of source operand 0
                defIter = defs.find((*itRev)->readReg0.reg);
                if ( defIter != defs.end() )
                {
                    ranges.insert(make_pair(defIter->second, (*itRev)->line));
                    defs.erase(defIter);
                }
            }       
        }
        if ( nOperands >= 2 )
        {
            // check first operand
            if ( (*itRev)->readReg1.isTemporary() )
            {
                // find definition of source operand 0
                defIter = defs.find((*itRev)->readReg1.reg);
                if ( defIter != defs.end() )
                {
                    ranges.insert(make_pair(defIter->second, (*itRev)->line));
                    defs.erase(defIter);
                }
            }       
        }
        if ( nOperands >= 3 )
        {
            // check first operand
            if ( (*itRev)->readReg2.isTemporary() )
            {
                // find definition of source operand 0
                defIter = defs.find((*itRev)->readReg2.reg);
                if ( defIter != defs.end() )
                {
                    ranges.insert(make_pair(defIter->second, (*itRev)->line));
                    defs.erase(defIter);
                }
            }
        }
    }
    
    // ranges contains all def-use ranges
    // complexity O(#instructions*#ranges)
    acd_uint max = 0;
    for ( acd_uint i = 0; i < code.size(); i++ )
    {
        acd_uint temp = _countMatchingRanges(ranges, i);
        //cout << "Instruction: " << i << "  Current alive register: " << temp << endl;
        if ( temp > max )
            max = temp;
    }

    //cout << "Max: " << max << endl;
    _shOptimizer->_optOutInf.maxAliveTemps = max;
}

acd_uint MaxLiveTempsAnalysis::_countMatchingRanges(std::set<std::pair<acd_uint,acd_uint> >& ranges, acd_uint instructionPos)
{
    typedef set<pair<acd_uint, acd_uint> > Ranges;
    typedef Ranges::iterator RangeIter;

    acd_uint count = 0;
    for ( RangeIter it = ranges.begin(); it != ranges.end(); it++ )
    {
        // check matching
        if ( it->first <= instructionPos && instructionPos <= it->second )
            count++;
    }
    return count;
}

//////////////////////////////////////////////////////////////////////////
//                RegisterUsageAnalysis Implementation                        //
//////////////////////////////////////////////////////////////////////////

RegisterUsageAnalysis::RegisterUsageAnalysis(ShaderOptimizer* shOptimizer)
: OptimizationStep(shOptimizer)
{

}

void RegisterUsageAnalysis::optimize()
{
    vector<InstructionInfo*> code = _shOptimizer->_inputInstrInfoVect;

    vector<InstructionInfo*>::iterator it = code.begin();

    for (acd_uint i=0; i < MAX_SHADER_ATTRIBUTES; i++)
    {
        _shOptimizer->_optOutInf.inputsRead[i] = false;
        _shOptimizer->_optOutInf.outputsWritten[i] = false;
    }

    // Iterate over the instructions finding consumer and producer
    // instructions
    for ( ; it != code.end(); it++ )
    {
        if ( (*it)->operation.writesAnyRegister ) 
        {
            if ((*it)->writeReg.isOutputRegister() )
            {
                _shOptimizer->_optOutInf.outputsWritten[(*it)->writeReg.reg] = true;
            }
        }

        acd_uint nOperands = (*it)->operation.numOperands;
        if ( nOperands >= 1 )
        {
            // check first operand
            if ( (*it)->readReg0.isInputRegister() )
            {
                _shOptimizer->_optOutInf.inputsRead[(*it)->readReg0.reg] = true;
            }       
        }
        if ( nOperands >= 2 )
        {
            // check the second operand
            if ( (*it)->readReg1.isInputRegister() )
            {
                _shOptimizer->_optOutInf.inputsRead[(*it)->readReg1.reg] = true;
            }       
        }
        if ( nOperands >= 3 )
        {
            // check the third operand
            if ( (*it)->readReg2.isInputRegister() )
            {
                _shOptimizer->_optOutInf.inputsRead[(*it)->readReg2.reg] = true;
            }       
        }
    }
}

//////////////////////////////////////////////////////////////////////////
//              StaticInstructionScheduling Implementation                //
//////////////////////////////////////////////////////////////////////////

StaticInstructionScheduling::StaticInstructionScheduling(ShaderOptimizer* shOptimizer)
: OptimizationStep(shOptimizer)
{

}

void StaticInstructionScheduling::optimize()
{
    // Build the dependency graph
    DependencyGraph* depgraph = _buildDependencyGraph(_shOptimizer->_inputInstrInfoVect,
                                                      _shOptimizer->_shArchParams.temporaries,
                                                      _shOptimizer->_shArchParams.outputRegs,
                                                      _shOptimizer->_shArchParams.addrRegs,
                                                      _shOptimizer->_shArchParams.predRegs);

    // Perform the instruction scheduling
    _schedule(*depgraph, _shOptimizer->_inputInstrInfoVect);

    delete depgraph;
}

DependencyGraph* StaticInstructionScheduling::_buildDependencyGraph(const vector<InstructionInfo*>& code, unsigned int temporaries,
                                                                    unsigned int outputRegs, unsigned int addrRegs, unsigned int predRegs) const
{
    DependencyGraph* dg = new DependencyGraph(code.size());

    /* Explore the entire code inserting dependencies into graph */

    /*
     * Track history of last instructions writing and reading
     * each of the temporary register in a component-wise form :
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
    
    vector<vector<int> > predicateRegisterWriters(predRegs);
    
    vector<vector<int> > registerReaders(temporaries*4);
    
    vector<vector<int> > addressRegisterReaders(addrRegs*4);

    vector<vector<int> > predicateRegisterReaders(predRegs);

    list<int> threadStateModifiers;
    list<int> sampleIdChangers;

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
            else if ((*iter)->writeReg.isPredicateRegister())
            {
                // Check for output dependencies with predicate register writers.
                if (predicateRegisterWriters[(*iter)->writeReg.reg].size() > 0)
                {
                    aux = predicateRegisterWriters[(*iter)->writeReg.reg].back();
                    
                    // Avoid self-instruction dependencies
                    if (aux != (*iter)->line) 
                        dg->insertOutputDependency(code[aux]->line, (*iter)->line);
                }
            }
            else
                panic("StaticInstructionScheduling","_buildDependencyGraph","Instruction writes a register that is not a temporal, and output or an address register");
        }
        
        if ((*iter)->operation.setsThreadState)
        {
            list<int>::const_iterator iter2 = sampleIdChangers.begin();
                        
            while ( iter2 != sampleIdChangers.end() )
            {
                dg->insertFalseDependency((*iter2), (*iter)->line);
                iter2++;
            }

            threadStateModifiers.push_back((*iter)->line);
        }
        
        if ((*iter)->operation.changesSampleId)
        {
            list<int>::const_iterator iter2 = threadStateModifiers.begin();
                        
            while ( iter2 != threadStateModifiers.end() )
            {
                dg->insertFalseDependency((*iter2), (*iter)->line);
                iter2++;
            }

            sampleIdChangers.push_back((*iter)->line);
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
            
            if ((*iter)->readReg0.isPredicateRegister())
            {

                if (predicateRegisterWriters[(*iter)->readReg0.reg].size() > 0)
                {
                    aux = predicateRegisterWriters[(*iter)->readReg0.reg].back();

                    //  Avoid self-instruction dependencies.
                    if (aux != (*iter)->line)
                        dg->insertTrueDependency(code[aux]->line, (*iter)->line);
                }

                predicateRegisterReaders[(*iter)->readReg0.reg].push_back((*iter)->line);
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

            if ((*iter)->readReg1.isPredicateRegister())
            {

                if (predicateRegisterWriters[(*iter)->readReg1.reg].size() > 0)
                {
                    aux = predicateRegisterWriters[(*iter)->readReg1.reg].back();

                    //  Avoid self-instruction dependencies.
                    if (aux != (*iter)->line)
                        dg->insertTrueDependency(code[aux]->line, (*iter)->line);
                }

                predicateRegisterReaders[(*iter)->readReg1.reg].push_back((*iter)->line);
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
        
        //  Check for true dependencies with a predicate register.  If the instruction is predicated.
        if ((*iter)->operation.predicated)
        {
            if (predicateRegisterWriters[(*iter)->operation.predicateRegister].size() > 0)
            {
                aux = predicateRegisterWriters[(*iter)->operation.predicateRegister].back();

                //  Avoid self-instruction dependencies.
                if (aux != (*iter)->line)
                    dg->insertTrueDependency(code[aux]->line, (*iter)->line);
            }

            predicateRegisterReaders[(*iter)->operation.predicateRegister].push_back((*iter)->line);
        }
        
        /* Push back current instruction as last register writer. Itïs necessary to do it here because we
         * need to avoid that and instruction detects its result register as a RaW dependence.
         */  

        if ((*iter)->operation.writesAnyRegister)
        {
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
            else if ((*iter)->writeReg.isPredicateRegister())
            {
                predicateRegisterWriters[(*iter)->writeReg.reg].push_back((*iter)->line);
            }
            else
                panic("StaticInstructionScheduling","_buildDependencyGraph","Instruction writes a register that is not a temporal, and output or an address register");
        }
                
        iter++;
    }

    return dg;
}

void StaticInstructionScheduling::_printDependencyGraph(const DependencyGraph& dg, unsigned int cycle, bool isInitialGraph) const
{
    string filename;

    if (isInitialGraph)
    {
        filename = "grap0000.dot"; // This name is used to be alphabetical order less than graphXXX.dot, and
                                   // file is displayed before anyone else by the Image Display App.
    }
    else
    {
        stringstream ss;

        char oldfill = ss.fill();
        unsigned int oldwidth = ss.width();
        
        ss.fill('0');
        ss.width(3);

        ss << "graph" << cycle << ".dot";

        ss.fill(oldfill);
        ss.width(oldwidth);

        filename = ss.str();
    }

    ofstream of(filename.c_str());

    dg.printDOTFormat(of);

    of.close();
}

void StaticInstructionScheduling::_schedule(DependencyGraph& dg, vector<InstructionInfo*>& code)
{
    // Get the architecture shader fetch wide
    unsigned int nWay = _shOptimizer->_shArchParams.nWay;

    // Reserve the last instruction index + 1 to NOP references.
    unsigned int nopBaseRef = code.size();

    /////////////////////////////////////////////////
    // Build initial sets for scheduling algorithm //
    /////////////////////////////////////////////////

    vector<int> chosenCandidate(nWay);

    list<unsigned int> unscheduledSet = _buildInitialUnScheduledSet(code);
    vector<IssueInfo> scheduledSet;
    list<unsigned int> readySet;

    ///////////////////////////////////////////////////////////////
    // Print dependency graph prior to the first execution cycle //
    ///////////////////////////////////////////////////////////////

    //_printDependencyGraphAtCycle(dg,cycle);

    /////////////////////////////
    //    Start Scheduling     //
    /////////////////////////////

    unsigned int cycle = 0;

    while( !unscheduledSet.empty() )
    {
        // Get the instructions that potentially can be scheduled at the time
        // because of register dependencies.
        readySet = _getReadyOps(dg, cycle, scheduledSet, code);

        // Sort the ready ops by some special policy like f.e: maximize free dependence instructions.
        _sortReadyOps(readySet, dg);

        /* Try to assign a ready op to each way */
        
        for(unsigned int i=0; i < nWay; i++)
        {
            if (!readySet.empty())
            {
                /* Good luck case: We have instructions to assing */
                chosenCandidate[i] = readySet.front();
                readySet.remove(chosenCandidate[i]);
                scheduledSet.push_back(make_pair(chosenCandidate[i],cycle));
                unscheduledSet.remove(chosenCandidate[i]);
                dg.setScheduled(chosenCandidate[i],cycle); // Update the Dependency Graph with the choosen scheduled instruction.
            }
            else
            {
                /* Bad luck case: No ready instructions are available.
                 * Take some policy, f.e: schedule whatever instruction not scheduled yet.
                 * NOTE: In case of VLIW architectures with no hardware dependence checking
                 *       here we should insert NOPs.
                 */
                if (!unscheduledSet.empty())
                {
                    chosenCandidate[i] = unscheduledSet.front();
                    scheduledSet.push_back(make_pair(chosenCandidate[i],cycle));
                    unscheduledSet.remove(chosenCandidate[i]);
                    dg.setScheduled(chosenCandidate[i],cycle);
                }
                else
                {
                    // No more unscheduled instructions available. Insert NOP´s.
                    chosenCandidate[i] = nopBaseRef;
                    nopBaseRef++;
                    scheduledSet.push_back(make_pair(chosenCandidate[i],cycle));
                }
            }
        }
        /////////////////////////////////////
        // Debug the scheduling each cycle //
        /////////////////////////////////////
        //_printDependencyGraphAtCycle(dg,cycle);

        cycle++;
    }

    //////////////////////////////////
    // Final Instruction Reordering //
    //////////////////////////////////
    _reorderCode(code, scheduledSet);
}

list<unsigned int> StaticInstructionScheduling::_buildInitialUnScheduledSet(const vector<InstructionInfo*>& code) const
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

/* 
 * The criterium to mark a free instruction proposed by the
 * dependency graph as ready, is that all true dependent instructions 
 * are at enough distance according to their execution latencies.
 */
list<unsigned int> StaticInstructionScheduling::_getReadyOps(const DependencyGraph& dg, unsigned int cycle, const vector<IssueInfo>& scheduledSet, const vector<InstructionInfo*>& code) const
{
    list<unsigned int> returnedList;

    // Recover current free nodes of dependency graph
    list<unsigned int> freeOps = dg.getFreeNodes();

    list<unsigned int>::iterator iter = freeOps.begin();

    bool conflictsFree;

    while( iter != freeOps.end() )
    {
        // For each free node test operand conflicts of true dependencies
        list<unsigned int> dependencies = dg.getTrueDependencies((*iter));
        
        list<unsigned int>::iterator iter2 = dependencies.begin();

        conflictsFree = true;

        while( iter2 != dependencies.end() && conflictsFree )
        {
            // Store execution latency of the true dependence op
            unsigned int executionLatency = code[(*iter2)]->operation.executionLatency;

            // Compute distance between free op and the true dependence op
            // Search the true dependence op in scheduled set
            
            bool found = false;
            unsigned int i=0;

            while( (i < scheduledSet.size()) && !found )
            {
                if (scheduledSet[i].first == (*iter2))
                    found = true;
                else
                    i++;
            }

            unsigned int cycleDistance = cycle - scheduledSet[i].second;
            
            // Check if exists conflict
            if (cycleDistance < executionLatency) conflictsFree = false;

            iter2++;
        }
        
        if (conflictsFree) returnedList.push_back((*iter));
        iter++;
    }

    return returnedList;
}

class QueueElement
{
public:
    unsigned int op;
    unsigned int numDependents;

    QueueElement( unsigned int op, unsigned int numDependents ): op(op), numDependents(numDependents) {}
    
    // The comparison operator used to order the
    // priority queue.
    //
    bool operator<( const QueueElement &a ) const
    {
        return (numDependents < a.numDependents);
    }
};

/* 
 * The used criterium is to sort operations by the number of child dependent instructions. 
 * More dependent instructions is more priority. Implemented with a priority queue.
 */
void StaticInstructionScheduling::_sortReadyOps(list<unsigned int>& readySet, const DependencyGraph& dg) const
{
    priority_queue<QueueElement> q;

    list<unsigned int>::const_iterator iter = readySet.begin();
    while( iter != readySet.end() )
    {
        q.push(QueueElement((*iter),dg.countDependentInstructions((*iter))));
        iter++;
    }
    
    readySet.clear();

    while( !q.empty() )
    {
        readySet.push_back(q.top().op);
        q.pop();
    }
}

void StaticInstructionScheduling::_reorderCode(vector<InstructionInfo*>& originalCode, const vector<IssueInfo>& orderList) const
{
    // Copy the initial vector. This makes instruction reordering safe.
    vector<InstructionInfo*> aux = originalCode;

    // Insert in original code the required space to hold new
    // instructions inserted by the Scheduler

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
        InstructionInfo *nop = new InstructionInfo(gpu3d::NOP, originalCode.size() + i);
        originalCode.push_back(nop);
    }
}

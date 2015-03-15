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

#include "CompilerSteps.h"
#include <queue>
#include <map>
#include <set>

using namespace std;
using namespace libgl;
using namespace GenerationCode;

/////////////////////////////////////////////////////////////////////////
/////////////// Implementation of CompilerSteps Methods /////////////////
/////////////////////////////////////////////////////////////////////////

u32bit GenerationCode::computeMaxLiveTemps(vector<InstructionInfo*>& code)
{
    set<pair<u32bit,u32bit> > ranges;

    map<u32bit, u32bit> defs;
    map<u32bit, u32bit>::iterator defIter;

    vector<InstructionInfo*>::iterator it = code.begin();

    // Find first definition of ech temporary register
    for ( ; it != code.end(); it++ )
    {
        if ( (*it)->writeReg.isTemporary() )
        {
            u32bit idReg = (*it)->writeReg.reg;
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
        u32bit nOperands = (*itRev)->operation.numOperands;
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
    u32bit max = 0;
    for ( u32bit i = 0; i < code.size(); i++ )
    {
        u32bit temp = countMatchingRanges(ranges, i);
        //cout << "Instruction: " << i << "  Current alive register: " << temp << endl;
        if ( temp > max )
            max = temp;
    }
    //cout << "Max: " << max << endl;
    return max;
}

u32bit GenerationCode::countMatchingRanges(std::set<std::pair<u32bit,u32bit> >& ranges, u32bit instructionPos)
{
    typedef set<pair<u32bit,u32bit> > Ranges;
    typedef Ranges::iterator RangeIter;

    u32bit count = 0;
    for ( RangeIter it = ranges.begin(); it != ranges.end(); it++ )
    {
        // check matching
        if ( it->first <= instructionPos && instructionPos <= it->second )
            count++;
    }
    return count;
}

void GenerationCode::registerRenaming(vector<InstructionInfo*>& code, unsigned int temporaries)
{
}

/* 
 * The criterium to mark a free instruction proposed by the
 * dependency graph as ready, is that all true dependent instructions 
 * are at enough distance according to their execution latencies.
 */
list<unsigned int> GenerationCode::getReadyOps(const DependencyGraph& dg, unsigned int cycle, const vector<IssueInfo>& scheduledSet, const vector<InstructionInfo*>& code)
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
void GenerationCode::sortReadyOps(list<unsigned int>& readySet, const DependencyGraph& dg)
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

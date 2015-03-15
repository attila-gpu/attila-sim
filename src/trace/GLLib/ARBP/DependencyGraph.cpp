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

#include "DependencyGraph.h"
#include "support.h"

using namespace std;
using namespace libgl;
using namespace GenerationCode;

/////////////////////////////////////////////////////////////////////////
/////////////// Implementation of DependencyGraph class /////////////////
/////////////////////////////////////////////////////////////////////////

bool libgl::GenerationCode::operator<(
    const DependencyGraph::Dependency& d1, 
    const DependencyGraph::Dependency& d2)
{
    return ( d1.first < d2.first );
}

DependencyGraph::DependencyInfo::DependencyInfo(DependencyType type, unsigned int distance)
    : type(type), distance(distance)
{
}

DependencyGraph::DependencyGraph(unsigned int numInstructions)
    : numNodes(numInstructions), trueDependencies(numInstructions),
      falseDependencies(numInstructions), outputDependencies(numInstructions)
{
    nodesScheduled.assign(numInstructions,make_pair(false,0));
    nodesLatencies.assign(numInstructions,0);
    nodesString.assign(numInstructions,"");
    dependentNodes.assign(numInstructions,0);
}


#define CHECK_NODE_RANGE(n,funcName) if ( n >= numNodes ) panic("Scheduler.cpp","funcName","Unexpected number of node");

void DependencyGraph::insertInstructionInfo(unsigned int node, std::string description, unsigned int instructionLatency)
{
    CHECK_NODE_RANGE(node,DependencyGraph::insertInstructionInfo)
    nodesString[node] = description;
    nodesLatencies[node] = instructionLatency;
}

void DependencyGraph::insertTrueDependency(unsigned int writeNode, unsigned int readNode)
{
    CHECK_NODE_RANGE(writeNode,DependencyGraph::insertTrueDependency)
    CHECK_NODE_RANGE(readNode,DependencyGraph::insertTrueDependency)
    trueDependencies[readNode].insert(make_pair(writeNode, DependencyInfo(TrueDependency, readNode - writeNode)));
    dependentNodes[writeNode]++;
}

void DependencyGraph::insertFalseDependency(unsigned int readNode, unsigned int writeNode)
{
    CHECK_NODE_RANGE(readNode,DependencyGraph::insertFalseDependency)
    CHECK_NODE_RANGE(writeNode,DependencyGraph::insertFalseDependency)
    falseDependencies[writeNode].insert(make_pair(readNode, DependencyInfo(FalseDependency, writeNode - readNode)));
}

void DependencyGraph::insertOutputDependency(unsigned int firstWriteNode, unsigned int lastWriteNode)
{
    CHECK_NODE_RANGE(firstWriteNode,DependencyGraph::insertOuputDependency)
    CHECK_NODE_RANGE(lastWriteNode,DependencyGraph::insertOutputDependency)
    outputDependencies[lastWriteNode].insert(make_pair(firstWriteNode, DependencyInfo(OutputDependency, lastWriteNode - firstWriteNode)));
}

void DependencyGraph::setScheduled(unsigned int node, unsigned int cycle)
{
    CHECK_NODE_RANGE(node,DependencyGraph::setScheduled)
    nodesScheduled[node] = make_pair(true,cycle);
}

list<unsigned int> DependencyGraph::getFreeNodes() const
{
    list<unsigned int> returnedList;

    for (unsigned int i=0; i < numNodes; i++)
    {
        set<Dependency>::const_iterator iter;
        bool allTrueDependenciesScheduled;
        bool allFalseDependenciesScheduled;
        bool allOutputDependenciesScheduled;
        
        iter = trueDependencies[i].begin();
        allTrueDependenciesScheduled = true;
        
        while( iter != trueDependencies[i].end() && allTrueDependenciesScheduled )
        {
            if (nodesScheduled[(*iter).first].first == false)
                allTrueDependenciesScheduled = false;
            else iter++;
        }
        
        iter = falseDependencies[i].begin();
        allFalseDependenciesScheduled = true;
        
        while( iter != falseDependencies[i].end() && allFalseDependenciesScheduled )
        {
            if (nodesScheduled[(*iter).first].first == false)
                allFalseDependenciesScheduled = false;
            else iter++;
        }
        
        iter = outputDependencies[i].begin();
        allOutputDependenciesScheduled = true;
        
        while( iter != outputDependencies[i].end() && allOutputDependenciesScheduled )
        {
            if (nodesScheduled[(*iter).first].first == false)
                allOutputDependenciesScheduled = false;
            else iter++;
        }
        
        if (allTrueDependenciesScheduled && allFalseDependenciesScheduled && allOutputDependenciesScheduled
            && !nodesScheduled[i].first) returnedList.push_back(i);
    }
    return returnedList;
}

list<unsigned int> DependencyGraph::getTrueDependencies(unsigned int readNode) const
{
    list<unsigned int> returnedList;
    
    CHECK_NODE_RANGE(readNode,DependencyGraph::getTrueDependencies)
    
    set<Dependency>::const_iterator iter = trueDependencies[readNode].begin();

    while( iter != trueDependencies[readNode].end() )
    {
        returnedList.push_back((*iter).first);
        iter++;
    }

    return returnedList;
}

list<unsigned int> DependencyGraph::getFalseDependencies(unsigned int writeNode) const
{
    list<unsigned int> returnedList;

    CHECK_NODE_RANGE(writeNode,DependencyGraph::getFalseDependencies)

    set<Dependency>::const_iterator iter = falseDependencies[writeNode].begin();

    while( iter != falseDependencies[writeNode].end() )
    {
        returnedList.push_back((*iter).first);
        iter++;
    }

    return returnedList;
}

list<unsigned int> DependencyGraph::getOutputDependencies(unsigned int lastWriterNode) const
{
    list<unsigned int> returnedList;

    CHECK_NODE_RANGE(lastWriterNode,DependencyGraph::getOutputDependencies)

    set<Dependency>::const_iterator iter = outputDependencies[lastWriterNode].begin();

    while( iter != outputDependencies[lastWriterNode].end() )
    {
        returnedList.push_back((*iter).first);
        iter++;
    }

    return returnedList;
}

unsigned int DependencyGraph::countDependentInstructions(unsigned int node) const
{
    CHECK_NODE_RANGE(node,DependencyGraph::countDependentInstructions)
    return dependentNodes[node];
}


void DependencyGraph::print(ostream& os) const
{
    unsigned int pos = 0;

    os << endl << "Dependency Graph: " << endl;

    while( pos < numNodes )
    {
        os << pos << ":" << endl;
        
        // Print dependencies

        if (!trueDependencies[pos].empty() || !falseDependencies[pos].empty() || !outputDependencies[pos].empty() )
            os << "Instruction dependencies: " << endl;

        set<Dependency>::const_iterator iter2 = 
            trueDependencies[pos].begin(); 

        while( iter2 != trueDependencies[pos].end() )
        {
            os << "True dependency with instruction: " << (*iter2).first;
            os << " at distance = " << (*iter2).second.distance << endl;
            iter2++;
        }

        iter2 = falseDependencies[pos].begin();

        while( iter2 != falseDependencies[pos].end() )
        {
            os << "False dependency with instruction: " << (*iter2).first;
            os << " at distance = " << (*iter2).second.distance << endl;
            iter2++;
        }

        iter2 = outputDependencies[pos].begin();

        while( iter2 != outputDependencies[pos].end() )
        {
            os << "Output dependency with instruction: " << (*iter2).first;
            os << " at distance = " << (*iter2).second.distance << endl;
            iter2++;
        }
        os << endl;

        pos++;
    }
    
}

void DependencyGraph::printDOTFormat(ostream& os) const
{
    unsigned int pos = 0;

    os << "digraph DependencyGraph" << endl;
    os << "{" << endl;

    //os << "size =\"10, 10\"" << endl;

    list<unsigned int> free = getFreeNodes();
    list<unsigned int>::const_iterator iter;
    
    while( pos < numNodes )
    {
        // Print dependencies
        
        os << pos;
        os << "[label=\"i" << pos << ":\\n " << nodesString[pos];
        if (nodesScheduled[pos].first)
            os << "\\n (cycle " << nodesScheduled[pos].second << ")";
        else
            os << "\\n           ";
            
        os << "\"";
        
        if (nodesScheduled[pos].first) os << " style=filled]";
        else os << "]";

        bool found = false;
        iter = free.begin();

        while( iter != free.end() && !found)
        {
            if ((*iter)==pos) found = true;
            else iter++;
        }

        if (found) os << "[color=red]";

        os << endl;

        set<Dependency>::const_iterator iter2 = trueDependencies[pos].begin(); 

        while( iter2 != trueDependencies[pos].end() )
        {
            os << (*iter2).first << " -> " << pos << "[arrowhead=normal label=\"lat: " << nodesLatencies[(*iter2).first] << "\"]" << endl;
            iter2++;
        }

        iter2 = falseDependencies[pos].begin();

        while( iter2 != falseDependencies[pos].end() )
        {
            os << (*iter2).first << " -> " << pos << "[arrowhead=tee]" << endl;
            iter2++;
        }

        iter2 = outputDependencies[pos].begin();

        while( iter2 != outputDependencies[pos].end() )
        {
            os << (*iter2).first << " -> " << pos << "[arrowhead=odot]" << endl;
            iter2++;
        }
        pos++;
    }
    os << "}" << endl;
}

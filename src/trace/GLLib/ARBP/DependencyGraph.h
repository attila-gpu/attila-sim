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

#ifndef DEPENDENCYGRAPH_H
    #define DEPENDENCYGRAPH_H

#include <vector>
#include <list>
#include <set>
#include <iostream>
#include <utility>
#include <string>

namespace libgl
{

namespace GenerationCode
{
/**
* Class implementing the Dependency Graph for Scheduler.
*
* @author Jordi Roca Monfort - jroca@ac.upc.es
* @date 13/4/2005
* @ver 1.0
*       
*/

class DependencyGraph
{
public:
    enum DependencyType { TrueDependency, FalseDependency, OutputDependency };
    
    class DependencyInfo
    {
    public:
    
        DependencyType type; 
        unsigned int distance; ///< Distance between dependent instructions in original code. It�s always positive.

        DependencyInfo(DependencyType type, unsigned int distance);
    };

    typedef std::pair<unsigned int /*node*/, DependencyInfo> Dependency;
    
private:    
    unsigned int numNodes; ///< Number of instructions of the graph
    
    friend bool operator< (const Dependency& d1, const Dependency& d2);
    
    /* 
     * "node" is node origin of dependency. For example:
     * In true dependency: the node that writes register
     * In false dependency: the node that reads register
     * In output dependency: the node that writes register the first time.
     */

    typedef std::set<Dependency> listDependencies;

    std::vector<listDependencies> trueDependencies;
    std::vector<listDependencies> falseDependencies;
    std::vector<listDependencies> outputDependencies;

    typedef std::pair<bool, unsigned int> SchedulingInfo;

    std::vector<SchedulingInfo> nodesScheduled;
    std::vector<std::string> nodesString;
    std::vector<unsigned int> nodesLatencies;
    std::vector<unsigned int> dependentNodes;

public:

    DependencyGraph(unsigned int numInstructions);

    void insertInstructionInfo(unsigned int instruction, std::string description, unsigned int instructionLatency);

    void insertTrueDependency(unsigned int writeNode, unsigned int readNode);

    void insertFalseDependency(unsigned int readNode, unsigned int writeNode);

    void insertOutputDependency(unsigned int firstWriteNode, unsigned int lastWriteNode);

    void setScheduled(unsigned int node, unsigned int cycle);

    std::list<unsigned int> getFreeNodes() const;

    std::list<unsigned int> getTrueDependencies(unsigned int readNode) const;

    std::list<unsigned int> getFalseDependencies(unsigned int writeNode) const;

    std::list<unsigned int> getOutputDependencies(unsigned int lastWriterNode) const;

    unsigned int countDependentInstructions(unsigned int node) const;

    void print(std::ostream& os) const;

    void printDOTFormat(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const DependencyGraph& dg)
    {
        dg.print(os);
        return os;
    }
};

bool operator<(
    const DependencyGraph::Dependency& d1,
    const DependencyGraph::Dependency& d2);

} // namespace GenerationCode

} // namespace libgl
/*
bool libgl::GenerationCode::operator<(
    const libgl::GenerationCode::DependencyGraph::Dependency& d1, 
    const libgl::GenerationCode::DependencyGraph::Dependency& d2)
{
    return ( d1.first < d2.first );
}
*/
#endif // DEPENDENCYGRAPH_H

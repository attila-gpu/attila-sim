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

#ifndef OPTIMIZATION_DATA_STRUCTURES_H
    #define OPTIMIZATION_DATA_STRUCTURES_H

#include <vector>
#include <list>
#include <iostream>
#include <set>
#include <utility>
#include <string>
#include "ShaderInstruction.h"
#include "ACDTypes.h"

namespace acdlib
{

namespace acdlib_opt
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

    bool isInputRegister() const;
    
    bool isAddressRegister() const;
    
    bool isPredicateRegister() const;
    
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
    bool setsThreadState;
    bool changesSampleId;
    bool jump;
    bool saturate;
    bool predicated;
    bool negatePredicate;
    u32bit predicateRegister;
    bool relative;
    unsigned int relModeComp;
    unsigned int relModeOffset;
    unsigned int relModeReg;
    int jumpOffset;
    unsigned int executionLatency;

    OperationInfo();
    OperationInfo(unsigned int opcode, unsigned int numOperands, bool writesAnyRegister, bool setsThreadState,
                  bool changesSampleId, bool jump, bool saturate, bool predicated, bool negPredicate, u32bit predReg, 
                  bool relative, unsigned int relModeComp, unsigned int relModeOffset, unsigned int relModeReg, int jumpOffset,
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

    InstructionInfo(gpu3d::ShaderInstruction& shInstr, acd_uint instrIndex, acd_uint execLat, const std::string& instrAsm);

    InstructionInfo(gpu3d::ShOpcode opcode, acd_uint instrIndex);

    gpu3d::ShaderInstruction* getShaderInstructionCopy(acd_bool lastInstruction) const;

    void print(std::ostream& os) const;

    friend std::ostream& operator<<(std::ostream& os, const InstructionInfo& ii)
    {
        ii.print(os);
        return os;
    }

};

/**
* Class implementing the Dependency Graph for the instruction scheduler.
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

    std::vector<bool> nodesRectangle;
    std::vector<bool> nodesStretchRect;
    std::vector<bool> nodesClear;
    std::vector<bool> nodesZST;
    std::vector<unsigned long long> nodesPixels;
    std::vector<unsigned long long> nodesSize;
    std::vector<std::string> nodesExtraString;
    std::vector<bool> nodesColorWriteEnable;
    bool group;
    bool cluster;
    unsigned long long maxSize;

public:

    DependencyGraph(unsigned int numInstructions);

    void insertInstructionInfo(unsigned int instruction, std::string description, unsigned int instructionLatency);

    //void insertInstructionInfoRectangle(unsigned int instruction, std::string description, unsigned int instructionLatency);

    void insertTrueDependency(unsigned int writeNode, unsigned int readNode);

    void insertFalseDependency(unsigned int readNode, unsigned int writeNode);

    void insertOutputDependency(unsigned int firstWriteNode, unsigned int lastWriteNode);

    void setScheduled(unsigned int node, unsigned int cycle);

    void groupNodes(bool g);

    void clusterGraph(bool c);

    //void setStretchRect(unsigned int node);

    void extraProperties(unsigned int node, bool rectangle, bool stretchRect, unsigned long long pixels,
        std::string extraDescription, bool colorWriteEnable, bool zst, unsigned long long size, bool clearNode);

    //void setMaxSize(unsigned int mSize);

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

typedef std::pair<unsigned int /* instruction id */,unsigned int /* issue cycle */> IssueInfo;

bool operator<(const DependencyGraph::Dependency& d1, const DependencyGraph::Dependency& d2);

} // namespace acdlib_opt

} // namespace acdlib

#endif // OPTIMIZATION_DATA_STRUCTURES_H

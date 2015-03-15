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

#include "OptimizationDataStructures.h"
#include "support.h"

using namespace std;
using namespace acdlib;
using namespace acdlib_opt;
using namespace gpu3d;

/////////////////////////////////////////////////////////////////////////
//////////// Implementation of Register, OperationInfo and //////////////
//////////////////////// InstructionInfo classes ////////////////////////
/////////////////////////////////////////////////////////////////////////

bool  acdlib::acdlib_opt::operator< (
    const DependencyGraph::Dependency& d1, 
    const DependencyGraph::Dependency& d2)
{
    return ( d1.first < d2.first );
}

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

bool Register::isInputRegister() const
{
    return (bank == IN);
}

bool Register::isAddressRegister() const
{
    return (bank == ADDR);
}

bool Register::isPredicateRegister() const
{
    return (bank == PRED);
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
                             bool writesAnyRegister, bool setsThreadState, 
                             bool changesSampleId, bool jump, bool saturate,
                             bool predicated, bool negPredicate, u32bit predReg,                   
                             bool relative, unsigned int relModeComp,
                             unsigned int relModeOffset, unsigned int relModeReg,
                             int jumpOffset, unsigned int executionLatency)

    : opcode(opcode), numOperands(numOperands), writesAnyRegister(writesAnyRegister),
      setsThreadState(setsThreadState), changesSampleId(changesSampleId), jump(jump),
      saturate(saturate),
      predicated(predicated), negatePredicate(negPredicate), predicateRegister(predReg),
      relative(relative), relModeComp(relModeComp), relModeOffset(relModeOffset), relModeReg(relModeReg),
      jumpOffset(jumpOffset), executionLatency(executionLatency)
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

InstructionInfo::InstructionInfo(gpu3d::ShOpcode opcode, acd_uint instrIndex)
{
    if (opcode != NOP)
        panic("InstructionInfo","Constructor","The single param constructor allows only construct NOP instructions");

    line = instrIndex;
    str = string("nop");

    operation = OperationInfo(opcode, 0, false, false, false, false, false, false, false, 0, false, 0, 0, 0, 0, 0);
}

InstructionInfo::InstructionInfo(ShaderInstruction& shInstr, acd_uint instrIndex, acd_uint execLat, const string& instrAsm)
{
    line = instrIndex;
    str = string(instrAsm);

    bool writesAnyRegister = ((shInstr.getOpcode() != NOP) && (shInstr.getOpcode() != END) && (shInstr.getOpcode() != KIL) &&
                              (shInstr.getOpcode() != KLS) && (shInstr.getOpcode() != ZXP) && (shInstr.getOpcode() != ZXS) &&
                              (shInstr.getOpcode() != CHS) && (shInstr.getOpcode() != JMP));
    bool setsThreadState = ((shInstr.getOpcode() == KIL) || (shInstr.getOpcode() == KLS) || (shInstr.getOpcode() == CMPKIL) ||
                            (shInstr.getOpcode() == ZXP) || (shInstr.getOpcode() == ZXS) || (shInstr.getOpcode() == JMP));
    bool changesSampleId = (shInstr.getOpcode() == CHS);

    operation = OperationInfo(shInstr.getOpcode(),
                                shInstr.getNumOperands(),
                                writesAnyRegister,
                                setsThreadState,
                                changesSampleId, 
                                shInstr.isAJump(),
                                shInstr.getSaturatedRes(),
                                shInstr.getPredicatedFlag(),
                                shInstr.getNegatePredicateFlag(),
                                shInstr.getPredicateReg(),
                                shInstr.getRelativeModeFlag(),
                                shInstr.getRelMAddrRegComp(),
                                shInstr.getRelMOffset(),
                                shInstr.getRelMAddrReg(),
                                shInstr.getJumpOffset(),
                                execLat);

    if (operation.writesAnyRegister)
    {
        writeReg = Register((unsigned int)shInstr.getBankRes(),
                                (unsigned int)shInstr.getResult(),
                                (unsigned int)shInstr.getResultMaskMode(),
                                (shInstr.getResultMaskMode() & 0x8)> 0 ? true : false,
                                (shInstr.getResultMaskMode() & 0x4)> 0 ? true : false,
                                (shInstr.getResultMaskMode() & 0x2)> 0 ? true : false,
                                (shInstr.getResultMaskMode() & 0x1)> 0 ? true : false,
                                false, false);
    }

    if (operation.numOperands > 0)
    {
        readReg0 = Register((unsigned int)shInstr.getBankOp1(),
                                (unsigned int)shInstr.getOp1(),
                                (unsigned int)shInstr.getOp1SwizzleMode(),
                                (bool)( ((shInstr.getOp1SwizzleMode() & 0xC0) == 0x00 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x30) == 0x00 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x0C) == 0x00 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x03) == 0x00 ) ),
                                (bool)( ((shInstr.getOp1SwizzleMode() & 0xC0) == 0x40 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x30) == 0x10 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x0C) == 0x04 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x03) == 0x01 ) ),
                                (bool)( ((shInstr.getOp1SwizzleMode() & 0xC0) == 0x80 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x30) == 0x20 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x0C) == 0x08 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x03) == 0x02 ) ),
                                (bool)( ((shInstr.getOp1SwizzleMode() & 0xC0) == 0xC0 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x30) == 0x30 ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x0C) == 0x0C ) ||
                                        ((shInstr.getOp1SwizzleMode() & 0x03) == 0x03 ) ),
                                        shInstr.getOp1NegateFlag(), shInstr.getOp1AbsoluteFlag());
    }
    
    if (operation.numOperands > 1)
    {
        readReg1 = Register((unsigned int)shInstr.getBankOp2(),
                                (unsigned int)shInstr.getOp2(),
                                (unsigned int)shInstr.getOp2SwizzleMode(),
                                (bool)( ((shInstr.getOp2SwizzleMode() & 0xC0) == 0x00 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x30) == 0x00 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x0C) == 0x00 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x03) == 0x00 ) ),
                                (bool)( ((shInstr.getOp2SwizzleMode() & 0xC0) == 0x40 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x30) == 0x10 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x0C) == 0x04 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x03) == 0x01 ) ),
                                (bool)( ((shInstr.getOp2SwizzleMode() & 0xC0) == 0x80 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x30) == 0x20 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x0C) == 0x08 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x03) == 0x02 ) ),
                                (bool)( ((shInstr.getOp2SwizzleMode() & 0xC0) == 0xC0 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x30) == 0x30 ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x0C) == 0x0C ) ||
                                        ((shInstr.getOp2SwizzleMode() & 0x03) == 0x03 ) ),
                                        shInstr.getOp2NegateFlag(), shInstr.getOp2AbsoluteFlag());
    }
    
    if (operation.numOperands > 2)
    {
        readReg2 = Register((unsigned int)shInstr.getBankOp3(),
                                (unsigned int)shInstr.getOp3(),
                                (unsigned int)shInstr.getOp3SwizzleMode(),
                                (bool)( ((shInstr.getOp3SwizzleMode() & 0xC0) == 0x00 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x30) == 0x00 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x0C) == 0x00 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x03) == 0x00 ) ),
                                (bool)( ((shInstr.getOp3SwizzleMode() & 0xC0) == 0x40 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x30) == 0x10 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x0C) == 0x04 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x03) == 0x01 ) ),
                                (bool)( ((shInstr.getOp3SwizzleMode() & 0xC0) == 0x80 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x30) == 0x20 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x0C) == 0x08 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x03) == 0x02 ) ),
                                (bool)( ((shInstr.getOp3SwizzleMode() & 0xC0) == 0xC0 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x30) == 0x30 ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x0C) == 0x0C ) ||
                                        ((shInstr.getOp3SwizzleMode() & 0x03) == 0x03 ) ),
                                        shInstr.getOp3NegateFlag(), shInstr.getOp3AbsoluteFlag());
    }
}

ShaderInstruction* InstructionInfo::getShaderInstructionCopy(acd_bool lastInstruction) const
{
    if (operation.opcode == gpu3d::NOP)
    {
        return new ShaderInstruction(NOP);
    }
    else
    {
        return new ShaderInstruction((ShOpcode)operation.opcode,
                                      (Bank)readReg0.bank,
                                      readReg0.reg,
                                      readReg0.negate,readReg0.absolute,
                                      (SwizzleMode)readReg0.swizzle(),
                                      (Bank)readReg1.bank,
                                      operation.jump ? operation.jumpOffset : readReg1.reg,
                                      readReg1.negate,readReg1.absolute,
                                      (SwizzleMode)readReg1.swizzle(),
                                      (Bank)readReg2.bank,readReg2.reg,
                                      readReg2.negate,readReg2.absolute,
                                      (SwizzleMode)readReg2.swizzle(),
                                      (Bank)writeReg.bank,
                                      writeReg.reg,
                                      operation.saturate,
                                      (MaskMode)writeReg.swizzle(),
                                      operation.predicated, operation.negatePredicate,
                                      operation.predicateRegister,
                                      operation.relative,operation.relModeReg,
                                      operation.relModeComp,operation.relModeOffset,
                                      lastInstruction);
    }
}

/////////////////////////////////////////////////////////////////////////
/////////////// Implementation of DependencyGraph class /////////////////
/////////////////////////////////////////////////////////////////////////

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

    nodesRectangle.assign(numInstructions, false);
    nodesStretchRect.assign(numInstructions, false);
    nodesClear.assign(numInstructions, false);
    nodesZST.assign(numInstructions, false);
    nodesPixels.assign(numInstructions, 0);
    nodesSize.assign(numInstructions, 0);
    nodesExtraString.assign(numInstructions, "");
    nodesColorWriteEnable.assign(numInstructions, false);
    group = false;
    cluster = false;
    maxSize = 0;
}


#define CHECK_NODE_RANGE(n,funcName) if ( n >= numNodes ) panic("Scheduler.cpp","funcName","Unexpected number of node");

void DependencyGraph::insertInstructionInfo(unsigned int node, std::string description, unsigned int instructionLatency)
{
    CHECK_NODE_RANGE(node,DependencyGraph::insertInstructionInfo)
    nodesString[node] = description;
    nodesLatencies[node] = instructionLatency;
}

/*void DependencyGraph::insertInstructionInfoRectangle(unsigned int node, std::string description, unsigned int instructionLatency)
{
    CHECK_NODE_RANGE(node,DependencyGraph::insertInstructionInfo)
    nodesString[node] = description;
    nodesLatencies[node] = instructionLatency;
    nodesRectangle[node] = true;
}*/

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

void DependencyGraph::groupNodes(bool g) {
    group = g;
}

void DependencyGraph::clusterGraph(bool c) {
    cluster = c;
}

/*void DependencyGraph::setStretchRect(unsigned int node) {
    nodesStretchRect[node] = true;
}*/

void DependencyGraph::extraProperties(unsigned int node, bool rectangle, bool stretchRect, unsigned long long pixels, std::string extraDescription, bool colorWriteEnable, bool zst, unsigned long long size, bool clearNode) {

    nodesRectangle[node] = rectangle;
    nodesStretchRect[node] = stretchRect;
    nodesClear[node] = clearNode;
    nodesZST[node] = zst;
    nodesPixels[node] = pixels;
    nodesSize[node] = size;
    nodesExtraString[node] = extraDescription;
    nodesColorWriteEnable[node] = colorWriteEnable;

    if (size > maxSize) maxSize = size;

}

/*void DependencyGraph::setMaxSize(unsigned int mSize) {
    maxSize = mSize;
}*/

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
    
    if (group) 
    {
        set<string> usedNodes;
        while( pos < numNodes ) 
        {
            if (usedNodes.count(nodesString[pos]) == 0) {
                unsigned int pos2 = pos;
                //if (!nodesStretchRect[pos]) {

                    if (cluster)
                        os << "subgraph \"cluster_" << nodesString[pos] << "\" { " << endl;
                    else
                        os << "subgraph \"" << nodesString[pos] << "\" { " << endl;

                    os << "style=filled;" << endl;
                    os << "color=navajowhite;" << endl;
                //}
                while( pos2 < numNodes )
                {
                    if (nodesString[pos].compare(nodesString[pos2]) == 0) { //os << pos2 << "; ";


                    
                        // Print dependencies
                        
                        os << pos2;

                        if (nodesStretchRect[pos2])
                            os << "[label=\"i" << pos2 << ":\\n " << nodesExtraString[pos2] << "\\n " << "SR";
                        else if (nodesClear[pos2])
                            os << "[label=\"Clear" /*<< "\\n " << nodesExtraString[pos2]*/;
                        else
                            os << "[label=\"i" << pos2 << ":\\n " << nodesString[pos2] << "\\n " << nodesExtraString[pos2];

                        /*if (nodesScheduled[pos2].first)
                            os << "\\n (cycle " << nodesScheduled[pos2].second << ")";
                        else
                            os << "\\n           ";*/
                            
                        os << "\"";
                        
                        if (nodesScheduled[pos2].first) os << " style=filled]";
                        else os << "]";

                        //os << "[fixedsize=true, fontsize=10, width=" << (float)nodesSize[pos2]/(float)maxSize * 4.5 + 0.5 << ", height=" << (float)nodesSize[pos2]/(float)maxSize * 4.5 + 0.5 <<"]";

                        /*if (nodesStretchRect[pos2]) os << "[fixedsize=true, width=0.5, height=0.5]";
                        else */if (nodesClear[pos2]) os << "[fixedsize=true, width=1.5, height=0.5]";
                        else os << "[fixedsize=true, width=" << (float)nodesSize[pos2]/(float)maxSize + 1.5f << ", height=" << (float)nodesSize[pos2]/(float)maxSize + 1.5f <<"]";

                        os << "[fontname=Sans, fontsize=10]";
                        /*if (nodesStretchRect[pos2]) os << "[color=orange, style=filled, fontcolor=darkorange4]";
                        else */if (nodesClear[pos2]) os << "[color=red3, style=filled, fontcolor=lightcoral]";
                        else if (!nodesColorWriteEnable[pos2]) os << "[color=grey77, style=filled, fontcolor=grey33]";
                        else {

                            /*float h = 0.65f; // ~blue
                            float s = (((float)nodesSize[pos2] / (float)maxSize) * 0.8f) + 0.1f;
                            float v = 0.90f - (((float)nodesSize[pos2] / (float)maxSize) * 0.8f);*/

                            float h = 0.65f; // ~blue

                            float overdraw;

                            if (nodesSize[pos2] == 0)
                                overdraw = 1.f;
                            else
                                overdraw = (float)nodesPixels[pos2] / ((float)nodesSize[pos2] * 10.f);

                            if (overdraw > 1.f) overdraw = 1.f;

                            float s = (overdraw * 0.8f) + 0.1f;
                            float v = 0.90f - (overdraw * 0.8f);

                            os << "[color=\"" << h << ", " << s << ", " << v << "\", style=filled, ";

                            /*if ((nodesSize[pos2] * 3.f) > maxSize) {
                                os << "fontcolor=white]";
                            }
                            else {
                                os << "fontcolor=black]";
                            }*/
                            if ((nodesPixels[pos2] * 5.f) > (nodesSize[pos2] * 10.f)) {
                                os << "fontcolor=white]";
                            }
                            else {
                                os << "fontcolor=black]";
                            }
                        }

                        if (nodesStretchRect[pos2]) os << "[shape=diamond]";
                        else if (nodesClear[pos2]) os << "[shape=box]";
                        else if (nodesRectangle[pos2]) os << "[shape=box]";
                        else os << "[shape=ellipse]";

                        bool found = false;
                        iter = free.begin();

                        while( iter != free.end() && !found)
                        {
                            if ((*iter)==pos2) found = true;
                            else iter++;
                        }

                        //if (found) os << "[color=red]";

                        os << endl;

                    }

                    pos2++;
                }
                /*if (!nodesStretchRect[pos])*/os << "}";
                os << endl;

                //os << "shape=box; color=blue; }" << endl;
                usedNodes.insert(nodesString[pos]);
            }

            pos++;
            

            
        }

    pos = 0;
            //os << "subgraph \"" << nodesString[pos] << "\" { ";
            while( pos < numNodes )
            {
                    set<Dependency>::const_iterator iter2 = trueDependencies[pos].begin(); 

                    while( iter2 != trueDependencies[pos].end() )
                    {
                        if (nodesZST[(*iter2).first] && !nodesClear[(*iter2).first])
                            os << (*iter2).first << " -> " << pos << "[arrowhead=normal, color=limegreen]" << endl;
                        /*else if (nodesClear[(*iter2).first])
                            os << (*iter2).first << " -> " << pos << "[arrowhead=normal, color=red1]" << endl;*/
                        else 
                            os << (*iter2).first << " -> " << pos << "[arrowhead=normal, color=grey15]" << endl;

                        iter2++;
                    }

                    iter2 = falseDependencies[pos].begin();

                    while( iter2 != falseDependencies[pos].end() )
                    {
                        os << (*iter2).first << " -> " << pos << "[arrowhead=tee, color=grey15]" << endl;
                        iter2++;
                    }

                    iter2 = outputDependencies[pos].begin();

                    while( iter2 != outputDependencies[pos].end() )
                    {
                        //os << (*iter2).first << " -> " << pos << "[arrowhead=odot, color=grey15]" << endl;
                        os << (*iter2).first << " -> " << pos << "[arrowhead=normal, color=red1]" << endl;
                        iter2++;
                    }
                //}

                pos++;

        }

    }
    else {

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

            if (nodesRectangle[pos]) os << "[shape=box]";
            else os << "[shape=ellipse]";

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
    }

    os << "}" << endl;
}

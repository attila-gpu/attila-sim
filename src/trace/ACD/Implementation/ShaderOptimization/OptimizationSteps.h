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

#ifndef OPTIMIZATION_STEPS_H
    #define OPTIMIZATION_STEPS_H

#include "ShaderOptimizer.h"

#include <set>
#include <utility>
#include <vector>

namespace acdlib
{

namespace acdlib_opt
{

class DependencyGraph;

class MaxLiveTempsAnalysis: public OptimizationStep
{
public:
    
    MaxLiveTempsAnalysis(ShaderOptimizer* shOptimizer);

    virtual void optimize();

private:

    acd_uint _countMatchingRanges(std::set<std::pair<acd_uint,acd_uint> >& ranges, acd_uint instructionPos);

};

class RegisterUsageAnalysis: public OptimizationStep
{
public:
    
    RegisterUsageAnalysis(ShaderOptimizer* shOptimizer);

    virtual void optimize();
};

class StaticInstructionScheduling: public OptimizationStep
{
public:

    StaticInstructionScheduling(ShaderOptimizer* shOptimizer);

    virtual void optimize();

private:

    void _schedule(DependencyGraph& dg, std::vector<InstructionInfo*>& code);

    DependencyGraph* _buildDependencyGraph(const std::vector<InstructionInfo*>& code, unsigned int temporaries,
                                           unsigned int outputRegs, unsigned int addrRegs, unsigned int predRegs) const;
    void _printDependencyGraph(const DependencyGraph& dg, unsigned int cycle, bool isInitialGraph = false) const;
    std::list<unsigned int> _buildInitialUnScheduledSet(const std::vector<InstructionInfo*>& code) const;
    std::list<unsigned int> _getReadyOps(const DependencyGraph& dg, unsigned int cycle, const std::vector<IssueInfo>& scheduledSet, const std::vector<InstructionInfo*>& code) const;
    void _sortReadyOps(std::list<unsigned int>& readySet, const DependencyGraph& dg) const;
    void _reorderCode(std::vector<InstructionInfo*>& originalCode, const std::vector<IssueInfo>& orderList) const;
};

} // namespace acdlib_opt

} // namespace acdlib

#endif // OPTIMIZATION_STEPS_H

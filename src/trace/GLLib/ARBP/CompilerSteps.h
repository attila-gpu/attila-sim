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

#ifndef COMPILERSTEPS_H
    #define COMPILERSTEPS_H

#include "Scheduler.h"
#include "DependencyGraph.h"
#include <vector>
#include <list>
#include <bitset>
#include <queue>
#include <set>

namespace libgl
{

namespace GenerationCode
{
/*
 * Performs previous basic optimizations such as: 
 *   * operation folding,
 *   * copy propagation,
 *   * common subexpression elimination,
 *   * dead code elimination
 *  and some important analysis as:
 *   * liveness analysis
 *  NOT IMPLEMENTED YET
 */
void basicOptimizations(std::vector<InstructionInfo*>& code);

/* 
 * Assigns an unused temporary each time to instructions that write a temporary while temporaries
 * are available. Removes false WAR and WAW dependencies.
 * NOT IMPLEMENTED YET
 */
void registerRenaming(std::vector<InstructionInfo*>& code, unsigned int temporaries);

/* 
 * Return the list of ready for execution instructions 
 * according to source operand values availability 
 */
std::list<unsigned int> getReadyOps(const DependencyGraph& dg,
                                    unsigned int cycle,
                                    const std::vector<IssueInfo>& scheduledSet,
                                    const std::vector<InstructionInfo*>& code);

/*
 * Sorts list of ready for execution instructions
 * according to some policy as number of child dependent instructions
 * that instruction can free if scheduled
 */
void sortReadyOps(std::list<unsigned int>& readySet, const DependencyGraph& dg);


u32bit computeMaxLiveTemps(std::vector<InstructionInfo*>& code);

// Aux, used inside computeMaxLiveTemps
u32bit countMatchingRanges(std::set<std::pair<u32bit,u32bit> >& ranges, u32bit instructionPos);

} // namespace GenerationCode

} // namespace libgl

#endif // COMPILERSTEPS_H

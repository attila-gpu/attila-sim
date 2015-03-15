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

#include "ACDXProgramExecutionEnvironment.h"
#include "ACDXImplementationLimits.h"
#include "ACDXSemanticTraverser.h"

using namespace std;
using namespace acdlib;

u8bit* ACDXProgramExecutionEnvironment::compile(const string& code, u32bit& size_binary, ACDXRBank<float>& clusterBank, ResourceUsage& resourceUsage)
{
    u8bit* binary = new u8bit[16*MAX_PROGRAM_INSTRUCTIONS_ARB*10];
    size_binary = 16*MAX_PROGRAM_INSTRUCTIONS_ARB*10;

    GenerationCode::ACDXSemanticTraverser * sem;
    ACDXShaderCodeGeneration* scg;    
    
    // Perform dependant compilation, depending on program type
    dependantCompilation((const u8bit *)code.c_str(), (u32bit)code.size(), sem, scg); 
        
    scg->returnGPUCode(binary, size_binary); 
    
    clusterBank = scg->getParametersBank();

    resourceUsage.numberOfInstructions = sem->getNumberOfInstructions();
    resourceUsage.numberOfParamRegs = sem->getNumberOfParamRegs();
    resourceUsage.numberOfTempRegs = sem->getNumberOfTempRegs();
    resourceUsage.numberOfAddrRegs = sem->getNumberOfAddrRegs();
    resourceUsage.numberOfAttribs = sem->getNumberOfAttribs();

    for (int tu = 0; tu < MAX_TEXTURE_UNITS_ARB; tu++)
        resourceUsage.textureUnitUsage[tu] = sem->getTextureUnitTarget(tu);

    resourceUsage.killInstructions = sem->getKillInstructions();
    
    delete sem;    
    delete scg;

    return binary;
}

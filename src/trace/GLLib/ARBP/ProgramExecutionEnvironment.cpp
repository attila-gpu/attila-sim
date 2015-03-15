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

#include "ProgramExecutionEnvironment.h"
#include "ImplementationLimits.h"
#include "glext.h"
#include "ProgramTarget.h"

using namespace libgl;

void ProgramExecutionEnvironment::compile(ProgramObject& po)
{
    GLubyte code[ProgramObject::maxSourceLength];   
    GLsizei size = sizeof(code);
    u8bit binary[16*MAX_PROGRAM_INSTRUCTIONS_ARB*10];
    u32bit size_binary = sizeof(binary);

    if ( !po.getSource(code, size) )
        panic("ProgramExecutionEnvironment","compile","Not enough space for source code");
    
    ProgramObject::ProgramResources& resources = po.getResources();
    
    if ( po.getFormat() != GL_PROGRAM_FORMAT_ASCII_ARB )
        panic("ProgramExecutionEnvironment", "compile()", "Program format unknown");

    GenerationCode::SemanticTraverser* sem;
    ShaderCodeGeneration* scg;    
    
    /* Perform dependant compilation, depending on program type */
    dependantCompilation(code, size, sem, scg); 
        
    scg->returnGPUCode(binary,size_binary); 
    
    po.getClusterBank() = scg->getParametersBank();
        
    po.setBinary((GLubyte *)binary,size_binary);    

    resources.numberOfInstructions = sem->getNumberOfInstructions();
    resources.numberOfParamRegs = sem->getNumberOfParamRegs();
    resources.numberOfTempRegs = sem->getNumberOfTempRegs();
    resources.numberOfAddrRegs = sem->getNumberOfAddrRegs();
    resources.numberOfAttribs = sem->getNumberOfAttribs();
    resources.maxAliveTemps = scg->getMaxAliveTemps();
    
    for (int i=0; i< ProgramObject::MaxTextureUnits; i++)
        resources.textureTargets[i] = sem->getTextureUnitTarget(i);
        
    for (int i=0; i<MAX_PROGRAM_ATTRIBS_ARB; i++)
    {
        resources.outputAttribsWritten[i] = sem->getAttribsWritten().test(i);
        resources.inputAttribsRead[i] = sem->getAttribsRead().test(i);
    }

    delete sem;    
    delete scg;
}


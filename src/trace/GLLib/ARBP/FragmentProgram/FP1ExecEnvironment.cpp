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
#include "support.h"
#include "FP1Bison.gen"
#include "SemanticTraverser.h"
#include "CodeGenTraverser.h"
#include "ShaderInstructionTranslator.h"

void FP1ExecEnvironment::dependantCompilation(const u8bit* code, u32bit size, SemanticTraverser*& ssc, ShaderCodeGeneration*& scg)
{
    YY_BUFFER_STATE parseBufferHandler = fp1_scan_bytes((const char*)code,size);

    // Intermediate Representation creation
    
    IRProgram *irtree = new IRProgram();

    fp1StartParse((void *)irtree);

    // Deleting Input Buffer used by Flex to scan bytes.
    
    fp1_delete_buffer(parseBufferHandler);

    //cout << endl << "Printing tree: " << endl << irtree << endl;

    /**
    * SemanticTraverser & ShaderInstructionTranslator must inherit from classes ShaderSemanticCheck
    * & ShaderCodeGeneration. This classes are probably just interfaces... (are not still written)
    */

    SemanticTraverser* semtrav = new SemanticTraverser;

    // Semantic Analysis

    irtree->traverse(semtrav);

    if (semtrav->foundErrors())
    {
        cout << semtrav->getErrorString() << endl;
        panic("FP1ExecEnvironment.cpp","dependantCompilation","Semantic error in Fragment Program");
    }

    CodeGenTraverser cgtrav;

    // Generic code generation
    irtree->traverse(&cgtrav);

    delete irtree;
    
    //cgtrav.dumpGenericCode(cout);

    // GPU instructions generation

    /**
     * Hardware-dependent translator from generic code
     * to hardware-dependent code.
     */

    ShaderInstructionTranslator* shTranslator =
        new ShaderInstructionTranslator(cgtrav.getParametersBank(),
                                        cgtrav.getTemporariesBank());

    shTranslator->translateCode(cgtrav.getGenericCode(), true, true);

    ssc = semtrav;
    scg = shTranslator;

}

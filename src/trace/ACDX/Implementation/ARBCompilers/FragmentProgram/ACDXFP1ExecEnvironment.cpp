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
#include "support.h"
#include "ACDXFP1Bison.gen"
#include "ACDXSemanticTraverser.h"
#include "ACDXCodeGenTraverser.h"
#include "ACDXShaderInstructionTranslator.h"

#include "GlobalProfiler.h"

void ACDXFP1ExecEnvironment::dependantCompilation(const u8bit* code, u32bit size, ACDXSemanticTraverser *& ssc, ACDXShaderCodeGeneration*& scg)
{
    YY_BUFFER_STATE parseBufferHandler = acdxfp1_scan_bytes((const char*)code,size);

    // Intermediate Representation creation
    
    IRProgram *irtree = new IRProgram();

    GLOBALPROFILER_ENTERREGION("FP compilation parser", "", "")
    acdxFp1StartParse((void *)irtree);
    GLOBALPROFILER_EXITREGION()

    // Deleting Input Buffer used by Flex to scan bytes.
    
    acdxfp1_delete_buffer(parseBufferHandler);

    //cout << endl << "Printing tree: " << endl << irtree << endl;

   /**
    * ACDXSemanticTraverser  & ACDXShaderInstructionTranslator must inherit from classes ShaderSemanticCheck
    * & ACDXShaderCodeGeneration. This classes are probably just interfaces... (are not still written)
    */

    ACDXSemanticTraverser * semtrav = new ACDXSemanticTraverser ;

    GLOBALPROFILER_ENTERREGION("FP compilation semantic analysis", "", "")
    // Semantic Analysis

    irtree->traverse(semtrav);
    GLOBALPROFILER_EXITREGION()

    if (semtrav->foundErrors())
    {
        cout << semtrav->getErrorString() << endl;
        panic("ACDXFP1ExecEnvironment.cpp","dependantCompilation","Semantic error in Fragment Program");
    }

    ACDXCodeGenTraverser cgtrav;

    GLOBALPROFILER_ENTERREGION("FP compilation code generation", "", "")
    // Generic code generation
    irtree->traverse(&cgtrav);
    GLOBALPROFILER_EXITREGION()

    delete irtree;
    
    //cgtrav.dumpGenericCode(cout);

    // GPU instructions generation

    /**
     * Hardware-dependent translator from generic code
     * to hardware-dependent code.
     */

    ACDXShaderInstructionTranslator* shTranslator =
        new ACDXShaderInstructionTranslator(cgtrav.getParametersBank(),
                                        cgtrav.getTemporariesBank());

    GLOBALPROFILER_ENTERREGION("FP compilation semantic code translation", "", "")
    shTranslator->translateCode(cgtrav.getGenericCode(), true, true);
    GLOBALPROFILER_EXITREGION()
    
    ssc = semtrav;
    scg = shTranslator;

}

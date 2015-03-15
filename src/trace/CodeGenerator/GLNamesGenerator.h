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

#ifndef GLNAMESGENERATOR_H
    #define GLNAMESGENERATOR_H

#include "CodeGenerator.h"
#include "FuncExtractor.h"
#include "ConstExtractor.h"
#include <iostream>

class GLNamesGenerator : public CodeGenerator
{

private:

    const FuncExtractor& fe;
    const ConstExtractor& ce;

    char* oDir2;

    bool generateGLConstantNames( std::ostream& f, const ConstExtractor& ce );
    bool generateGLFunctionNames( std::ostream& f, const FuncExtractor& fe );
    bool generateGLFunctionNamesList( std::ostream& f, const FuncExtractor& fe, bool withCommas = true, const char* prefix = "");


public:

    GLNamesGenerator( const FuncExtractor& fe, const ConstExtractor& ce, 
                      const char* oDir1, const char* oDir2 = 0);

    bool generateCode();

    void dumpInfo() const;

    ~GLNamesGenerator();
};


#endif // GLNAMESGENERATOR_H

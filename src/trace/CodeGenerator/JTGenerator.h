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

#ifndef JTGENERATOR
    #define JTGENERATOR

#include "CodeGenerator.h"


class JTGenerator : public CodeGenerator
{
private:

    const FuncExtractor& fe;

    const FuncExtractor& libFuncsSL;

    char jtName[256];

    bool generateJTFields( std::ostream& fout, const FuncExtractor& fe );
    
    bool generateJumpTableStaticInit( std::ostream& f, const FuncExtractor& libFuncsSL,
                                       const char* jtName );

public:

    JTGenerator( const FuncExtractor& fe, const FuncExtractor& libFuncsSL, const char* oDir, const char* jtName );
    
    bool generateCode();

    void dumpInfo() const;
};

#endif // JTGENERATOR

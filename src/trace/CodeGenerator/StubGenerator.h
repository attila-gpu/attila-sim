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

#ifndef STUBGENERATOR_H
    #define STUBGENERATOR_H

#include "CodeGenerator.h"

class StubGenerator : public FuncGenerator
{
private:

    const FuncExtractor& fe;

    bool generateApiCallStub( const FuncDescription& fd, std::ostream& f, const char* tr, const char* jt, const char* uct);
    
    bool generateAllApiCallStubs( const FuncExtractor& fe, std::ostream& f, const char* tr, const char* jt, const char* uct );

    bool generateAllApiCallStubsDeclarations( const FuncExtractor& fe, std::ofstream& f );

    // Add code here for special calls
    // setPixelFormat (example)
    bool checkSpecialCall(const char* fn);

public:

    StubGenerator( const FuncExtractor& fe, const char* oDir );

    bool generateCode();

    void dumpInfo() const;
};

#endif // STUBGENERATOR_H

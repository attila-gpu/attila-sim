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

#ifndef SWITCHBRANCHESGENERATOR_H
    #define SWITCHBRANCHESGENERATOR_H

#include "CodeGenerator.h"

class SwitchBranchesGenerator : public CodeGenerator
{
private:

    const FuncExtractor& fe;
    char jt[256];
    char uct[256];
    char tr[256];

public:

    SwitchBranchesGenerator( const FuncExtractor& fe, const char* trName,
                             const char* jtName, const char* uct, const char* oDir );
    
    bool generateCode();

    void dumpInfo() const;
};


#endif // SWITCHBRANCHESGENERATOR_H

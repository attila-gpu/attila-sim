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

#include "SwitchBranchesGenerator.h"
#include <iostream>
#include <cstring>

using namespace std;

SwitchBranchesGenerator::SwitchBranchesGenerator( const FuncExtractor& fe, const char* trName,
                                                 const char* jtName, const char* uctName, const char* oDir ) :
CodeGenerator(), fe(fe)
{
    setOutputDir(oDir);
    strcpy(tr, trName);
    strcpy(jt,jtName);
    strcpy(uct, uctName);
}

void SwitchBranchesGenerator::dumpInfo() const
{
    cout << "SwitchBranchesGenerator generates:\n  { SwitchBranches.gen }";
}

bool SwitchBranchesGenerator::generateCode()
{
    ofstream f;

    if ( openOFile(f,"SwitchBranches.gen") )
    {
        writePreface(f);

        f << "#ifndef SWITCHBRANCHES_GEN\n    #define SWITCHBRANCHES_GEN\n\n";
    
        int i, nFuncs;

        nFuncs = fe.count();
        const char* name;
        for ( i = 0; i < nFuncs; i++ )
        {
            name = fe.getFunction(i)->getName();
            f << "case " << "APICall_" << name << " :\n";
            f << "    " << "STUB_" << name << "(" << tr << "," << jt << "," << uct << ");\n    break;\n";
        }
    
        f << "\n#endif // SWITCHBRANCHES_GEN\n";
        f.close();
    }

    return true;

}

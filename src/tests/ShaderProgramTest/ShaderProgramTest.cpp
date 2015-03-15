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

#include "ShaderProgramTest.h"
#include "ShaderProgramTestBase.h"
#include <iostream>
#include "SPTConsole.h"
#include "ConfigLoader.h"
#include "OptimizedDynamicMemory.h"

using namespace gpu3d;
using namespace std;

int main(int argc, char **argv)
{   
    SimParameters simP;

    ConfigLoader cl("bGPU.ini");
    cl.getParameters(&simP);
    
    OptimizedDynamicMemory::initialize(simP.objectSize0, simP.bucketSize0, simP.objectSize1, simP.bucketSize1,
        simP.objectSize2, simP.bucketSize2);

    SPTConsole* sptconsole = new SPTConsole(simP, "MySPTest");

    vector<string> params;
    if ( argc > 1 ) {
        for ( int i = 1; i < argc; ++i ) {
            params.push_back(argv[i]);
        }
    }

    sptconsole->parseParams(params);

    sptconsole->printWelcomeMessage();

    while ( !sptconsole->finished() ) 
    {
        sptconsole->execute();   
    }

    return 0;
}

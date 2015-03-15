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

#ifndef GLIENTRYPOINTSGENERATOR_H
    #define GLIENTRYPOINTSGENERATOR_H

#include "CodeGenerator.h"
#include <string>
#include <vector>

/**
 * Generator code for file GLIEntryPoints required in GLInstrument project
 */
class GLIEntryPointsGenerator : public FuncGenerator
{
private:

    const FuncExtractor& fe;

    bool generateEntryPointCall(std::ostream& out, std::string gliName, 
                                const FuncDescription& fd);

    bool generateAllEntryPointCalls(std::ostream& out, std::string gliName,
                                const FuncExtractor& fe);

    // return parameters name's
    std::vector<std::string> generateHeader(std::ostream& out, const FuncDescription& fd);

public:

    GLIEntryPointsGenerator(const FuncExtractor& fe, std::string outputDir);

    bool generateCode();

    void dumpInfo() const;
};



#endif // GLIENTRYPOINTSGENERATOR_H

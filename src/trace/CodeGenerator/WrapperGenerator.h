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

#ifndef WRAPPERGENERATOR_H
    #define WRAPPERGENERATOR_H

#include "CodeGenerator.h"
#include <set>
#include <string>

/** 
 * generateCode: Generates files glWrapper.h & glWrapper.cpp
 */
class WrapperGenerator : public FuncGenerator
{
private:
    
    char oDirGen[256];
    const FuncExtractor& fe;
    const SpecificExtractor& se;
    const FuncExtractor& feStats;
    bool *isCallPlayable;
    std::set<std::string>* candidates; // avoid or generate code for tracing these calls
    bool genTraceCode; // avoid(false)/generate(true) flag
    
    /**
     * Dumps a list of unplayable call
     *
     * That is, the call is logged but cannot be player by GLPlayer
     */
    /*std::ofstream listOfUnplayableCalls;*/

    int countUnplayables; /* How many calls are unplayable */

    bool generateWrapperCall( std::ostream& fout, const char* writerName,
                              const char* jumpTableName, const FuncDescription& fd,
                              const SpecificItem* sfd, bool hasSpecificStat = false );

    bool generateAllWrapperCalls( std::ostream& f, const char* writerName, const char* jumpTableName, 
                              const FuncExtractor& fe );

public:
    
    WrapperGenerator( const FuncExtractor& fe, const SpecificExtractor& se, 
                      const FuncExtractor& feStats, const char* oDirWrapper, 
                      const char* oDirGen );
    
    WrapperGenerator( const FuncExtractor& fe, const SpecificExtractor& se, 
                      const FuncExtractor& feStats, const char* oDirWrapper, 
                      const char* oDirGen, std::set<std::string>& candidates,
                      bool genTraceCode);


    bool generateCode();

    void dumpInfo() const;
};

#endif // WRAPPERGENERATOR_H

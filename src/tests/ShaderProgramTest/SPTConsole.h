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

#ifndef SPTCONSOLE_H
    #define SPTCONSOLE_H

#include "ShaderProgramTestBase.h"
#include <vector>
#include <list>

namespace gpu3d
{

class SPTConsole : public ShaderProgramTestBase
{
private:

    static const unsigned int SPT_INVALID_INTEGER = static_cast<unsigned int>(-1);

    bool quitConsole;

    std::list<std::string> pendingCommands;

    std::string getPrompt() const;

    bool processCompileProgramCommand(const std::vector<std::string>& params);
    bool processDefineProgramInputCommand(const std::vector<std::string>& params);
    bool processDefineProgramConstantCommand(const std::vector<std::string>& params);
    bool processExecuteProgramCommand(const std::vector<std::string>& params);
    bool processDumpProgramCommand(const std::vector<std::string>& params);
    bool processTransformCommand(const std::vector<std::string>& params);
    bool processQuitCommand(const std::vector<std::string>& params);
    bool processHelpCommand(const std::vector<std::string>& params);
    bool processScriptCommand(const std::vector<std::string>& params);

public:

    void printWelcomeMessage() const;

    std::string parseParams(const std::vector<std::string>& params);

    bool finished();

    void execute();

    SPTConsole( const SimParameters& simP, const char* name );

};

} // namespace gpu3d

#endif // SPTCONSOLE_H

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

#ifndef MCTCONSOLE_H
    #define MCTCONSOLE_H

#include "MemoryControllerTestBase.h"
#include "MemoryRequestSplitter.h"
#include <vector>
#include <list>

namespace gpu3d
{


class MCTConsole : public MemoryControllerTestBase
{
private:

    struct Stats
    {
        u32bit gpuReadRequestsSent;
        u32bit gpuReadRequestsReceived;
        u32bit systemReadRequestsSent;
        u32bit systemReadRequestsReceived;
        u32bit gpuWriteRequestsSent;
        u32bit systemWriteRequestsSent;
    };

    Stats stats;

    static const u32bit MC_INVALID_INTEGER = static_cast<u32bit>(-1);

    u64bit currentCycle;
    u64bit startCycle;
    bool quitConsole;

    std::list<std::string> pendingCommands;

    bool processSendCommand(const std::vector<std::string>& params);
    bool processRunCommand(const std::vector<std::string>& params);
    bool processQuitCommand(const std::vector<std::string>& params);
    bool processHelpCommand(const std::vector<std::string>& params);
    bool processScriptCommand(const std::vector<std::string>& params);
    bool processSaveMemoryCommand(const std::vector<std::string>& params);
    bool processLoadMemoryCommand(const std::vector<std::string>& params);
    bool processArchCommand(const std::vector<std::string>& params);
    bool processStatsCommand(const std::vector<std::string>& params);
    bool processMCDebugCommand(const std::vector<std::string>& params);
    bool processPrintCommand(const std::vector<std::string>& params);
    bool processReadCommand(const std::vector<std::string>& params);
    bool processWriteCommand(const std::vector<std::string>& params);


    static std::string parseUnitName(string unitName, u32bit& unit, u32bit& subunit);
    u32bit parseAddress(string addressStr);

    std::string getPrompt() const;

    // virtual void handler_receiveTransaction(u64bit cycle, MemoryTransaction* mt);
    // virtual void handler_sendTransaction(u64bit cycle, MemoryTransaction* mt);

    // std::vector<memorycontroller::MemoryRequestSplitter*> splitterArray;
    memorycontroller::MemoryRequestSplitter* splitter;

    // Last data received from each client
    std::vector<std::string> lastData[LASTGPUBUS];
    

protected:

    void clock_test(u64bit cycle);

    void handler_receiveTransaction(u64bit cycle, MemoryTransaction* mt);
    void handler_sendTransaction(u64bit cycle, MemoryTransaction* mt);


public:

    std::string parseParams(const std::vector<std::string>& params);

    bool finished();

    MCTConsole( const SimParameters& simP,
              const char** tuPrefixes,
              const char** suPrefixes,
              const char** slPrefixes,
              const char* name, 
              const MemoryControllerProxy& mcProxy,
              Box* parentBox);

};

} // namespace gpu3d

#endif // MCTCONSOLE_H

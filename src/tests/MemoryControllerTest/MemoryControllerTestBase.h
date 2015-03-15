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

#ifndef MEMORYCONTROLLERTESTBASE_H
    #define MEMORYCONTROLLERTESTBASE_H

#include "MemoryControllerTest.h"

#include "Box.h"
#include <vector>
#include <queue>
#include "MemoryTransaction.h"
#include "MemoryControllerCommand.h"
#include "ConfigLoader.h"
#include "MemoryControllerProxy.h"

namespace gpu3d {

// From ConfigLoader.h (included in implementation .cpp file)
//struct SimParameters;

class MemoryControllerTestBase : public Box
{
private:

    u64bit currentCycle;

    /**
     * Emulates the memory client's interface
     */
    class ClientEmulator
    {
    private:

        u64bit memoryCycles;
        
        bool memoryWrite;
        bool memoryRead;
        
        std::queue<MemoryTransaction*> pendingMTs;

        GPUUnit unit;
        u32bit subunit;

        Signal* outputSignal;
        Signal* inputSignal;

        MemoryTransaction* currentMT;
        MemState memState;
        MemoryControllerTestBase* owner;

    public:

        // Temporary here
        static const u32bit MAX_REQUEST_SIZE = 256;
        char requestData[MAX_REQUEST_SIZE];
        u32bit nextID;

        ClientEmulator( MemoryControllerTestBase* owner, 
                        GPUUnit unit, u32bit subunit,
                        Signal* inputSignal, Signal* outputSignal);

        void enqueue(MemoryTransaction* mt);

        void clock(u64bit cycle);

    };

    std::vector<std::vector<ClientEmulator*> > clientEmulators;

    Signal* mcCommSignal; // Command signal from the command processor (offered to tests in case they want to send a MC command)

    std::queue<MemoryControllerCommand*> pendingMCCommands;

    const SimParameters& simParams;

    const MemoryControllerProxy& mcProxy;


protected:

    const SimParameters& SimParams() const { return simParams; }
    
    void sendMCCommand(MemoryControllerCommand* mccom);

    
    // By default deletes the transaction
    virtual void handler_receiveTransaction(u64bit cycle, MemoryTransaction* mt);

    virtual void handler_sendTransaction(u64bit cycle, MemoryTransaction* mt);

    enum RequestType
    {
        READ_REQUEST,
        WRITE_REQUEST
    };

    void sendRequest(GPUUnit unit, u32bit subunit, RequestType type, u32bit requestSize, u32bit address, u8bit* userData = 0);

    // void sendRequest(MemoryTransaction* mt);

    static void printTransaction(const MemoryTransaction* mt);

    /**
     * Redefine this function to implement your memory test
     */
    virtual void clock_test(u64bit cycle) = 0;

    const MemoryControllerProxy& getMCProxy() const;

public:

    virtual std::string parseParams(const std::vector<std::string>& params)
    {
        return std::string(); // OK
    }

    MemoryControllerTestBase( const SimParameters& simP,
                              const char** tuPrefixes,
                              const char** suPrefixes,
                              const char** slPrefixes,
                              const char* name,
                              const MemoryControllerProxy& memProxy,
                              Box* parentBox);

    void clock(u64bit cycle);

    virtual bool finished() = 0;

};

} // namespace gpu3d

#endif // MEMORYCONTROLLERTESTBASE_H

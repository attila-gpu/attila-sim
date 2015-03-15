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

#include "MemoryControllerTestBase.h"


using namespace gpu3d;

MemoryControllerTestBase::MemoryControllerTestBase( const SimParameters& simP,
                                                    const char** tuPrefixes,
                                                    const char** suPrefixes,
                                                    const char** slPrefixes,
                                                    const char* name,
                                                    const MemoryControllerProxy& mcProxy,
                                                    Box* parentBox ) : Box(name, parentBox), simParams(simP), currentCycle(0), mcProxy(mcProxy)
{

    clientEmulators.push_back(vector<ClientEmulator*>());
    clientEmulators.back().push_back( new ClientEmulator(
                                              this, COMMANDPROCESSOR, 0, 
                                              newInputSignal("CommProcMemoryRead", 2, 1, 0), 
                                              newOutputSignal("CommProcMemoryWrite", 1, 1, 0) 
                                          ) 
                                     );

    clientEmulators.push_back(vector<ClientEmulator*>());
    clientEmulators.back().push_back( new ClientEmulator(
                                              this, STREAMERFETCH, 0, 
                                              newInputSignal("StreamerFetchMemoryData", 2, 1, 0), 
                                              newOutputSignal("StreamerFetchMemoryRequest", 1, 1, 0) 
                                          ) 
                                     );

    clientEmulators.push_back(vector<ClientEmulator*>());
    for ( u32bit i = 0; i < simP.str.streamerLoaderUnits; ++i )
    {
        clientEmulators.back().push_back( new ClientEmulator(
                                                  this, STREAMERLOADER, i, 
                                                  newInputSignal("StreamerLoaderMemoryData", 2, 1, slPrefixes[i]), 
                                                  newOutputSignal("StreamerLoaderMemoryRequest", 1, 1, slPrefixes[i]) 
                                              ) 
                                         );
    }
    
    clientEmulators.push_back(vector<ClientEmulator*>());
    for (u32bit i = 0; i < simP.gpu.numStampUnits; i++ )
    {
        clientEmulators.back().push_back( new ClientEmulator(
                                                  this, ZSTENCILTEST, i, 
                                                  newInputSignal("ZStencilTestMemoryData", 2, 1, suPrefixes[i]), 
                                                  newOutputSignal("ZStencilTestMemoryRequest", 1, 1, suPrefixes[i])
                                              )
                                         );
    }

    clientEmulators.push_back(vector<ClientEmulator*>());
    for (u32bit i = 0; i < simP.gpu.numStampUnits; i++ )
    {
        clientEmulators.back().push_back( new ClientEmulator(
                                                  this, COLORWRITE, i, 
                                                  newInputSignal("ColorWriteMemoryData", 2, 1, suPrefixes[i]), 
                                                  newOutputSignal("ColorWriteMemoryRequest", 1, 1, suPrefixes[i])
                                              )
                                         );
    }
    
    clientEmulators.push_back(vector<ClientEmulator*>());
    clientEmulators.back().push_back( new ClientEmulator(
                                              this, DACB, 0, 
                                              newInputSignal("DACMemoryData", 2, 1, 0), 
                                              newOutputSignal("DACMemoryRequest", 1, 1, 0) 
                                          ) 
                                     );

    const u32bit numTexUnits = simP.gpu.numFShaders * simP.fsh.textureUnits;
    clientEmulators.push_back(vector<ClientEmulator*>());
    for ( u32bit i = 0; i < numTexUnits;i++ )
    {
        clientEmulators.back().push_back( new ClientEmulator(
                                                  this, TEXTUREUNIT, i, 
                                                  newInputSignal("TextureMemoryData", 2, 1, tuPrefixes[i]), 
                                                  newOutputSignal("TextureMemoryRequest", 1, 1, tuPrefixes[i])
                                              )
                                         );
    }


    mcCommSignal = newOutputSignal("MemoryControllerCommand", 1, 1, 0);

    MCT_DEBUG( cout << "MCT_DEBUG: MemoryTestBase ctor() completed!" << endl; )

}

void MemoryControllerTestBase::sendMCCommand(MemoryControllerCommand* mccom)
{
    pendingMCCommands.push(mccom);
}


MemoryControllerTestBase::ClientEmulator::ClientEmulator(MemoryControllerTestBase* owner, 
                                                         GPUUnit unit, u32bit subunit,
                                                         Signal* inputSignal, Signal* outputSignal) :
    owner(owner), unit(unit), subunit(subunit),
    inputSignal(inputSignal), outputSignal(outputSignal), memoryRead(false), memoryWrite(false), memoryCycles(0), nextID(0)
{
    MCT_DEBUG( cout << "MCT_DEBUG: Creating client emulator UNIT=" << MemoryTransaction::getBusName(unit) << " SUBUNIT=" << subunit << endl; )
}

void MemoryControllerTestBase::ClientEmulator::enqueue(MemoryTransaction* mt)
{
    pendingMTs.push(mt);
}

void MemoryControllerTestBase::ClientEmulator::clock(u64bit cycle)
{    
    MemoryTransaction* mt = 0;

    // read replies
    while ( inputSignal->read(cycle, (DynamicObject*&)mt) ) {
        // MCT_DEBUG( cout << "MT recived: " << mt->toString() << endl; )
        switch ( mt->getCommand() ) {
            case MT_STATE:
                memState = mt->getState();
                delete mt; // Get rid of state transactions
                break;
            case MT_READ_DATA:
                MCT_DEBUG( cout << "MCT_DEBUG (cycle " << cycle <<  "): MT recived: " << mt->toString() << endl; )
                GPU_ASSERT
                (
                    if ( memoryCycles > 0 ) {
                        panic("MemoryControllerTestBase::ClientEmulator", "clock", "Memory bus still busy when receiving a new transaction");
                    }
                )
                memoryCycles = mt->getBusCycles();
                memoryRead = true;
                // send received transaction back to test
                owner->handler_receiveTransaction(cycle, mt);
                break;
            case MT_PRELOAD_DATA:
                cout << "-------------------\n" << mt->toString() << "\n";
                panic("MemoryControllerTestBase::ClientEmulator", "clock", "Received an unexpected transaction from MC ( MT_PRELOAD_DATA )");
            case MT_READ_REQ:
                cout << "-------------------\n" << mt->toString() << "\n";
                panic("MemoryControllerTestBase::ClientEmulator", "clock", "Received an unexpected transaction from MC ( MT_READ_REQ )");
            case MT_WRITE_DATA:
                cout << "-------------------\n" << mt->toString() << "\n";
                panic("MemoryControllerTestBase::ClientEmulator", "clock", "Received an unexpected transaction from MC ( MT_WRITE_REQ )");
            default:
                panic("MemoryControllerTestBase::ClientEmulator", "clock", "Received an unexpected transaction from MC ( UNKNOWN )");
        }
    }

    if ( memoryCycles > 0 ) {
        --memoryCycles;
        if ( memoryCycles == 0 ) {
            if ( memoryRead ) {
                memoryRead = false;
            }
            if ( memoryWrite ) {
                memoryWrite = false;
            }
        }
        // else ( bus busy )
    }

    if ( !pendingMTs.empty() && !memoryWrite && !memoryRead ) {
        MemoryTransaction* mt = pendingMTs.front();
        switch ( mt->getCommand() ) {
            case MT_WRITE_DATA:
                if ( (memState & MS_WRITE_ACCEPT) != 0 ) {
                    memoryWrite = true;
                    memoryCycles = mt->getBusCycles();
                    // MCT_DEBUG( cout << "Cycle: " << cycle << ". Sending MT: " << mt->toString() << endl; )
                    owner->handler_sendTransaction(cycle, mt);
                    outputSignal->write(cycle, mt);
                    pendingMTs.pop();
                }
                break;
            case MT_READ_REQ:
                if ( (memState & MS_READ_ACCEPT) != 0 ) {
                    // MCT_DEBUG( cout << "Cycle: " << cycle << ". Sending MT: " << mt->toString() << endl; )
                    owner->handler_sendTransaction(cycle, mt);
                    outputSignal->write(cycle, mt);
                    pendingMTs.pop();
                }
                break;
            default:
                panic("MemoryControllerTestBase::ClientEmulator", "clock", "Unsupported memory transaction picked from the client's emulator queue");
        }
    }
}


void MemoryControllerTestBase::clock(u64bit cycle)
{
    currentCycle = cycle;

    // send the next MC command (if any)
    if ( !pendingMCCommands.empty() ) {
        MemoryControllerCommand* mccom = pendingMCCommands.front();
        pendingMCCommands.pop();
        mcCommSignal->write(cycle, mccom);
    }

    vector<vector<ClientEmulator*> >::iterator it = clientEmulators.begin();
    for ( ; it != clientEmulators.end(); ++it ) {
        vector<ClientEmulator*>::iterator it2 = it->begin();
        for ( ;it2 != it->end(); ++it2 )
            (*it2)->clock(cycle);
    }

    clock_test(cycle);
}

void MemoryControllerTestBase::printTransaction(const MemoryTransaction* mt)
{
    mt->dump(false);
}

void MemoryControllerTestBase::handler_receiveTransaction(u64bit cycle, MemoryTransaction* mt)
{
    cout << "C: " << cycle << ". Receiving: " << mt->toString(true) << endl;
    delete[] mt->getData();
    delete mt;
}

void MemoryControllerTestBase::handler_sendTransaction(u64bit cycle, MemoryTransaction* mt)
{
    cout << "C: " << cycle << ". Sending: " << mt->toString(true) << endl;

}

void MemoryControllerTestBase::sendRequest( GPUUnit unit, 
                                            u32bit subunit, 
                                            RequestType type, 
                                            u32bit requestSize,
                                            u32bit address,
                                            u8bit* data )
{
    /*
    cout << "DEBUG: sendRequest -> GPUUnit= " << unit << "[" << subunit << "]"
        << " " << (type == READ_REQUEST ? "READ" : "WRITE") 
        << " sz=" << requestSize << " Address (dec)=" << address << " DataPtr=" 
        << reinterpret_cast<u64bit>(data) << "\n";
    if ( data ) {
        cout << "Data: " << (const char*)data << endl;
    }
    */

    if ( static_cast<u32bit>(unit) >= clientEmulators.size() ) { 
        panic("MemoryControllerTestBase", "sendRequest", "Unit token unknown");
    }
    if ( subunit >= clientEmulators[unit].size() ) {
        panic("MemoryControllerTestBase", "sendRequest", "Subunit ID too high");
    }

    if ( requestSize > ClientEmulator::MAX_REQUEST_SIZE ) {
        stringstream ss;
        ss << "Request size is larger than MAX_REQUEST_SIZE which is defined as " << ClientEmulator::MAX_REQUEST_SIZE << " bytes";
        panic("MemoryControllerTestBase", "sendRequest", ss.str().c_str());
    }

    ClientEmulator* ce = clientEmulators[unit][subunit];

    memset(ce->requestData, 0, ClientEmulator::MAX_REQUEST_SIZE);
    if ( data != 0 )
        memcpy(ce->requestData, data, requestSize);

    MemoryTransaction* mt = 0;
    if ( type == WRITE_REQUEST ) {
        mt = new MemoryTransaction(
                    MT_WRITE_DATA,
                    address, 
                    requestSize,
                    (u8bit*)ce->requestData, 
                    unit,
                    subunit,
                    ce->nextID++  );
    }
    else if ( type == READ_REQUEST )  {
        mt = new MemoryTransaction(
                    MT_READ_REQ,
                    address, 
                    requestSize,
                    (u8bit*)new char[requestSize],
                    unit,
                    subunit,
                    ce->nextID++  );        
    }
    else {
        panic("MemoryControllerTestBase", "sendRequest", "Unsupported memory transaction type");
    }

    clientEmulators[unit][subunit]->enqueue(mt);
}


const MemoryControllerProxy& MemoryControllerTestBase::getMCProxy() const
{
    return mcProxy;
}
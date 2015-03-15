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
 * $RCSfile: CommandProcessor.cpp,v $
 * $Revision: 1.64 $
 * $Author: vmoya $
 * $Date: 2008-03-05 19:26:45 $
 *
 * Command Processor class implementation file.
 *
 */

/**
 *
 *  @file CommandProcessor.cpp
 *
 *  This class implements the Command Processor box.
 *
 *  The Command Processor box simulates the main control
 *  unit of a GPU.
 *
 */

#include "CommandProcessor.h"
#include <cstring>
#include "ShaderCommand.h"
#include "AGPTransaction.h"
#include "MemoryTransaction.h"
//#include "MemoryController.h"
#include "ShaderInstruction.h"
#include "RasterizerCommand.h"
#include "Rasterizer.h"
#include "StreamerStateInfo.h"
#include "PrimitiveAssemblyCommand.h"
#include "PrimitiveAssemblyStateInfo.h"
#include "ClipperCommand.h"
#include "ClipperStateInfo.h"
#include "GPUMath.h"
#include "MemoryControllerCommand.h"
#include <iostream>

using std::cout;
using std::endl;

using namespace gpu3d;

/*  Command Processor constructor.  */
CommandProcessor::CommandProcessor(TraceDriverInterface *tDriver, u32bit nVShaders, char **vshPrefixArray,
    u32bit nFShaders, char **fshPrefixArray, u32bit nTUs, char **tuPrefixArray, u32bit nStampUnits, char **suPrefixes,
    u32bit memSize, bool pipeBatches, bool dumpShaders, char *name, Box *parent):

regWrites(getSM().getNumericStatistic("RegisterWrites", u32bit(0), "CommandProcessor", "CP")),
bytesWritten(getSM().getNumericStatistic("BytesWritten", u32bit(0), "CommandProcessor", "CP")),
bytesRead(getSM().getNumericStatistic("BytesRead", u32bit(0), "CommandProcessor", "CP")),
writeTrans(getSM().getNumericStatistic("WriteTransactions", u32bit(0), "CommandProcessor", "CP")),
batches(getSM().getNumericStatistic("Batches", u32bit(0), "CommandProcessor", "CP")),
frames(getSM().getNumericStatistic("Frames", u32bit(0), "CommandProcessor", "CP")),
bitBlits(getSM().getNumericStatistic("Bit Blits", u32bit(0), "CommandProcessor", "CP")),
readyCycles(getSM().getNumericStatistic("Ready", u32bit(0), "CommandProcessor", "CP")),
drawCycles(getSM().getNumericStatistic("Draw", u32bit(0), "CommandProcessor", "CP")),
endGeomCycles(getSM().getNumericStatistic("EndGeometry", u32bit(0), "CommandProcessor", "CP")),
endFragCycles(getSM().getNumericStatistic("EndFragment", u32bit(0), "CommandProcessor", "CP")),
clearCycles(getSM().getNumericStatistic("Clear", u32bit(0), "CommandProcessor", "CP")),
swapCycles(getSM().getNumericStatistic("Swap", u32bit(0), "CommandProcessor", "CP")),
flushCycles(getSM().getNumericStatistic("Flush", u32bit(0), "CommandProcessor", "CP")),
saveRestoreStateCycles(getSM().getNumericStatistic("SaveRestoreState", u32bit(0), "CommandProcessor", "CP")),
memWriteCycles(getSM().getNumericStatistic("MemoryWrite", u32bit(0), "CommandProcessor", "CP")),
memReadCycles(getSM().getNumericStatistic("MemoryRead", u32bit(0), "CommandProcessor", "CP")),
memPreLoadCycles(getSM().getNumericStatistic("MemoryPreload", u32bit(0), "CommandProcessor", "CP")),
bitBlitCycles(getSM().getNumericStatistic("bitBlitCycles", u32bit(0), "CommandProcessor", "CP")),

Box(name, parent), numVShaders(nVShaders), numFShaders(nFShaders), numTextureUnits(nTUs),
numStampUnits(nStampUnits), memorySize(memSize), pipelinedBatches(pipeBatches), dumpShaderPrograms(dumpShaders),
skipDraw(false), skipFrames(false), forceTransaction(false), forcedCommand(false)

{
    u32bit i;

    /*  Check vertex shaders and shader signals prefixes.  */
    GPU_ASSERT(
        if (numVShaders == 0)
            panic("CommandProcessor", "CommandProcessor", "Invalid number of Vertex Shaders (must be >0).");

        if (vshPrefixArray == NULL)
            panic("CommandProcessor", "CommandProcessor", "The vertex shader prefix array is null.");

        for(i = 0; i < numVShaders; i++)
        {
            if (vshPrefixArray[i] == NULL)
                panic("CommandProcessor", "CommandProcessor", "Vertex shader prefix array component is null.");
        }
    )

    /*  Create the signal arrays for the vertex shaders.  */
    vshFCommSignal = new Signal*[numVShaders];
    vshDCommSignal = new Signal*[numVShaders];

    GPU_ASSERT(
        if (vshFCommSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Vertex Shader Fetch command signal array.");
        if (vshDCommSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Vertex Shader Decode command signal array.");
    )

    /*  Create the signals with each vertex shader unit.  */
    for (i = 0; i < numVShaders; i++)
    {
        vshFCommSignal[i] = newOutputSignal("CommShaderCommand", 1, 1, vshPrefixArray[i]);
        vshDCommSignal[i] = newOutputSignal("ShaderDecodeExecuteCommand", 1, 1, vshPrefixArray[i]);
    }

    /*  Create control signal to the Streamer.  */
    streamCtrlSignal = newOutputSignal("StreamerControl", 1, 1, NULL);

    /*  Create state signal from the Streamer.  */
    streamStateSignal = newInputSignal("StreamerState", 1, 1, NULL);


    /*  Create command signal to Primitive Assembly.  */
    paCommSignal = newOutputSignal("PrimitiveAssemblyCommand", 1, 1, NULL);

    /*  Create state signal from Primitive Assembly.  */
    paStateSignal = newInputSignal("PrimitiveAssemblyCommandState", 1, 1, NULL);


    /*  Create command signal to the Clipper.  */
    clipCommSignal = newOutputSignal("ClipperCommand", 1, 1, NULL);

    /*  Create state signal from the Clipper.  */
    clipStateSignal = newInputSignal("ClipperCommandState", 1, 1, NULL);


    /*  Create signal to the rasterizer.  */
    rastCommSignal = newOutputSignal("RasterizerCommand", 1, 1, NULL);

    /*  Create a state signal from the rasterizer.  */
    rastStateSignal = newInputSignal("RasterizerState", 1, 1, NULL);

    /*  Check fragment shaders and shader signals prefixes.  */
    GPU_ASSERT(
        if (numFShaders == 0)
            panic("CommandProcessor", "CommandProcessor", "Invalid number of Fragment Shaders (must be >0).");

        if (fshPrefixArray == NULL)
            panic("CommandProcessor", "CommandProcessor", "The fragment shader prefix array is null.");

        for(i = 0; i < numFShaders; i++)
        {
            if (fshPrefixArray[i] == NULL)
                panic("CommandProcessor", "CommandProcessor", "Fragment shader prefix array component is null.");
        }
    )

    /*  Create the fragment shader command signal arrays.  */
    fshFCommandSignal = new Signal*[numFShaders];
    fshDCommandSignal = new Signal*[numFShaders];

    GPU_ASSERT(
        if (fshFCommandSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Fragment Shader Fetch command signal array.");
        if (fshDCommandSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Fragment Shader Decode command signal array.");
    )

    /*  Create the signals with each fragment shader unit.  */
    for (i = 0; i < numFShaders; i++)
    {
        fshFCommandSignal[i] = newOutputSignal("CommShaderCommand", 1, 1, fshPrefixArray[i]);
        fshDCommandSignal[i] = newOutputSignal("ShaderDecodeExecuteCommand", 1, 1, fshPrefixArray[i]);
    }

    /*  Check texture units signal prefixes.  */
    GPU_ASSERT(
        if (numTextureUnits == 0)
            panic("CommandProcessor", "CommandProcessor", "Invalid number of Texture Units (must be >0).");

        if (tuPrefixArray == NULL)
            panic("CommandProcessor", "CommandProcessor", "The texture unit prefix array is null.");

        for(i = 0; i < numTextureUnits; i++)
        {
            if (tuPrefixArray[i] == NULL)
                panic("CommandProcessor", "CommandProcessor", "Texture unit prefix array component is null.");
        }
    )

    /*  Create the Texture Unit command signals.  */
    tuCommandSignal = new Signal*[numTextureUnits];

    GPU_ASSERT(
        if (tuCommandSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Texture Unit command signal array.");
    )

    /*  Create the signals with each Texture Unit unit.  */
    for (i = 0; i < numTextureUnits; i++)
    {
        tuCommandSignal[i] = newOutputSignal("TextureUnitCommand", 1, 1, tuPrefixArray[i]);
    }

    /*  Check number of stamp units and stamp unit signals prefixes.  */
    GPU_ASSERT(
        if (numStampUnits == 0)
            panic("CommandProcessor", "CommandProcessor", "Invalid number of Stamp Units (must be >0).");

        if (suPrefixes == NULL)
            panic("CommandProcessor", "CommandProcessor", "The stamp unit prefix array is null.");

        for(i = 0; i < numStampUnits; i++)
        {
            if (suPrefixes[i] == NULL)
                panic("CommandProcessor", "CommandProcessor", "Stamp unit prefix array component is null.");
        }
    )

    /*  Create the fragment shader command signal arrays.  */
    zStencilCommSignal = new Signal*[numStampUnits];
    zStencilStateSignal = new Signal*[numStampUnits];
    colorWriteCommSignal = new Signal*[numStampUnits];
    colorWriteStateSignal = new Signal*[numStampUnits];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (zStencilCommSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Z Stencil command signal array.");
        if (zStencilStateSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Z Stencil state signal array.");
        if (colorWriteCommSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Color Write command signal array.");
        if (colorWriteStateSignal == NULL)
            panic("CommandProcessor", "CommandProcessor", "Error allocating Color Write state signal array.");
    )

    /*  Create the signals with each stamp unit unit.  */
    for (i = 0; i < numStampUnits; i++)
    {
        /*  Create command signal to Z Stencil Test.  */
        zStencilCommSignal[i] = newOutputSignal("ZStencilTestCommand", 1, 1, suPrefixes[i]);

        /*  Create state signal from Z Stencil Test.  */
        zStencilStateSignal[i] = newInputSignal("ZStencilTestRasterizerState", 1, 1, suPrefixes[i]);


        /*  Create command signal to color write.  */
        colorWriteCommSignal[i] = newOutputSignal("ColorWriteCommand", 1, 1, suPrefixes[i]);

        /*  Create state signal from color write.  */
        colorWriteStateSignal[i] = newInputSignal("ColorWriteRasterizerState", 1, 1, suPrefixes[i]);
    }


    /*  Create command signal to DAC.  */
    dacCommSignal = newOutputSignal("DACCommand", 1, 1);

    /*  Create state signal from DAC.  */
    dacStateSignal = newInputSignal("DACState", 1, 1);


    /*  Create signal to the GPU memory interface.  */
    writeMemorySignal = newOutputSignal("CommProcMemoryWrite", 1, 1, NULL);

    /*  Create signal from the GPU memory interface.  */
    readMemorySignal = newInputSignal("CommProcMemoryRead", 2, 1, NULL);

    // Create memory controller command signal
    mcCommSignal = newOutputSignal("MemoryControllerCommand", 1, 1, NULL);


    /*  Allocate arrays for storing the state of the stamp units.  */
    zStencilState = new RasterizerState[numStampUnits];
    colorWriteState = new RasterizerState[numStampUnits];

    /*  Initialize the TraceDriver.  */
    driver = tDriver;


    /*  Start TraceDriver.  */
    driver->startTrace();

    /*  Set current AGP transaction to NULL.  */
    lastAGPTrans = NULL;

    /*  Set program code buffer pointer to null.  */
    programCode = NULL;

    /*  Set cycles for current AGP transaction end to 0.  */
    transCycles = 0;

    /*  Memory variables.  */
    currentTicket = 0;
    freeTickets = MAX_MEMORY_TICKETS;

    /*  Set trace not finished.  */
    traceEnd = FALSE;

    /*  Initialize update buffer.  */
    freeUpdates[GEOM_REG] = freeUpdates[FRAG_REG] = MAX_REGISTER_UPDATES;
    regUpdates[GEOM_REG] = nextFreeUpdate[GEOM_REG] = nextUpdate[GEOM_REG] = 0;
    regUpdates[FRAG_REG] = nextFreeUpdate[FRAG_REG] = nextUpdate[FRAG_REG] = 0;

    /*  Initialize process transaction flag.  */
    processNewTransaction = TRUE;

    /*  Initialize geometry rendering phase start flag.  */
    geometryStarted = FALSE;

    /*  Initialize buffered load fragment program flag.  */
    storedLoadFragProgram = FALSE;

    //  Reset the initialization phase end flag.
    initEnd = false;

    //  Reset cycle counter for flush delay (wait for HZ buffer update).
    flushDelayCycles = 0;

    //  Used to write shader programs to a file.
    vshProgID = 0;
    fshProgID = 0;

    //  Clear the agp transaction log.
    nextReadLog = 3;
    nextWriteLog = 0;
    agpTransactionLog[0].clear();
    agpTransactionLog[1].clear();
    
    // Set validation mode to disabled.
    enableValidation = false;
    
    /*  Initial GPU state.  */
    state.statusRegister = GPU_ST_RESET;
}

/*  Command Processor clock rutine.  */
void CommandProcessor::clock(u64bit cycle)
{
    u32bit i;
    MemoryTransaction *memTrans;
    ShaderCommand *shComm;
    StreamerCommand *streamComm;
    PrimitiveAssemblyCommand *paCommand;
    ClipperCommand *clipperCommand;
    RasterizerCommand *rastComm;
    RasterizerState rasterizerState;
    RasterizerStateInfo *rastStateInfo;
    RasterizerStateInfo *zStencilStateInfo;
    RasterizerStateInfo *colorWriteStateInfo;
    RasterizerState dacState;
    RasterizerStateInfo *dacStateInfo;
    StreamerStateInfo *streamStateInfo;
    AGPTransaction *auxAGPTrans;
    PrimitiveAssemblyStateInfo *paStateInfo;
    AssemblyState paState;
    ClipperStateInfo *clipStateInfo;
    ClipperState clipState;
    u32bit size;
    bool endAllZST;
    bool endAllCW;

    GPU_DEBUG_BOX( printf("CommandProcessor => Clock %lld\n", cycle);)

    GPU_DEBUG_BOX(
        printf("CommandProcessor => State: ");
        switch(state.statusRegister)
        {
            case GPU_ST_RESET: printf("GPU_ST_RESET\n"); break;
            case GPU_READY: printf("GPU_READY\n"); break;
            case GPU_DRAWING: printf("GPU_DRAWING\n"); break;
            case GPU_END_GEOMETRY: printf("GPU_END_GEOMETRY\n"); break;
            case GPU_END_FRAGMENT: printf("GPU_END_FRAGMENT\n"); break;
            case GPU_MEMORY_WRITE: printf("GPU_MEMORY_WRITE\n"); break;
            case GPU_MEMORY_READ: printf("GPU_MEMORY_READ\n"); break;
            case GPU_MEMORY_PRELOAD: printf("GPU_MEMORY_PRELOAD\n"); break;
            case GPU_SWAP: printf("GPU_SWAP\n"); break;
            case GPU_DUMPBUFFER: printf("GPU_DUMPBUFFER\n"); break;
            case GPU_BLITTING: printf("GPU_BLIT\n"); break;
            case GPU_CLEAR_COLOR: printf("GPU_CLEAR_COLOR\n"); break;
            case GPU_CLEAR_Z: printf("GPU_CLEAR_Z\n"); break;
            case GPU_FLUSH_COLOR: printf("GPU_FLUSH_COLOR\n"); break;
            case GPU_FLUSH_Z: printf("GPU_FLUSH_Z\n"); break;
            case GPU_SAVE_STATE_COLOR: printf("GPU_SAVE_STATE_COLOR\n"); break;
            case GPU_RESTORE_STATE_COLOR: printf("GPU_RESTORE_STATE_COLOR\n"); break;
            case GPU_SAVE_STATE_Z: printf("GPU_SAVE_STATE_Z\n"); break;
            case GPU_RESTORE_STATE_Z: printf("GPU_SAVE_STATE_Z\n"); break;
            case GPU_ERROR: printf("GPU_ERROR\n"); break;
            default: printf("undefined\n"); break;
        }
    )

    /*  Reset swap received flag.  No swap received yet in this cycle.  */
    swapReceived = FALSE;

    /*  Reset end of batch flag.  */
    batchEnd = false;

    //  Reset the end of last command flag.
    commandEnd = false;

    /*  Get current state of other GPU units.  */

    /*  Get memory transactions and state from Memory Controller.  */
    while(readMemorySignal->read(cycle, (DynamicObject *&) memTrans))
    {
        /*  Process memory transaction.  */
        processMemoryTransaction(memTrans);
    }


    /*  Get Streamer State.  */
    if (streamStateSignal->read(cycle, (DynamicObject *&) streamStateInfo))
    {
        /*  Store streamer state.  */
        streamState = streamStateInfo->getState();

        /*  Delete the streamer state info.  */
        delete streamStateInfo;

    }
    else
    {
        panic("CommandProcessor", "clock", "Missing state signal from the Streamer.");
    }

    /*  Get Primitive Assembly State.  */
    if (paStateSignal->read(cycle, (DynamicObject *&) paStateInfo))
    {
        /*  Store primitive assembly state.  */
        paState = paStateInfo->getState();

        /*  Delete primitive assembly state info object.  */
        delete paStateInfo;
    }
    else
    {
        panic("CommandProcessor", "clock", "Missing state signal from Primitive Assembly.");
    }

    /*  Get Clipper state.  */
    if (clipStateSignal->read(cycle, (DynamicObject *&) clipStateInfo))
    {
        /*  Store clip state.  */
        clipState = clipStateInfo->getState();

        /*  Delete state info object.  */
        delete clipStateInfo;
    }
    else
    {
        panic("CommandProcessor", "clock", "Missing state signal from the Clipper.");
    }

    /*  Get Rasterizer State.  */
    if(rastStateSignal->read(cycle, (DynamicObject *&) rastStateInfo))
    {
        /*  Store rasterizer state.  */
        rasterizerState = rastStateInfo->getState();

        /*  Delete the rasterizer state info received.  */
        delete rastStateInfo;
    }
    else
    {
        panic("CommandProcessor", "clock", "Missing state signal from the Rasterizer.");
    }


    /*  Get Z Stencil state.  */
    for(i = 0, endAllZST = TRUE; i < numStampUnits; i++)
    {
        if (zStencilStateSignal[i]->read(cycle, (DynamicObject *&) zStencilStateInfo))
        {
            /*  Store rasterizer state.  */
            zStencilState[i] = zStencilStateInfo->getState();

            /*  Update the flag storing if all the Z Stencil Test units are in END state.  */
            endAllZST = endAllZST && (zStencilState[i] == RAST_END);

            /*  Delete info object.  */
            delete zStencilStateInfo;
        }
        else
        {
            panic("CommandProcessor", "clock", "Missing state signal from the Color Write unit.");
        }
    }

    /*  Get color write state.  */
    for(i = 0, endAllCW = TRUE; i < numStampUnits; i++)
    {
        if (colorWriteStateSignal[i]->read(cycle, (DynamicObject *&) colorWriteStateInfo))
        {
            /*  Store rasterizer state.  */
            colorWriteState[i] = colorWriteStateInfo->getState();

            /*  Update the flag storing if all the color write units are in END state.  */
            endAllCW = endAllCW && (colorWriteState[i] == RAST_END);

            /*  Delete info object.  */
            delete colorWriteStateInfo;
        }
        else
        {
            panic("CommandProcessor", "clock", "Missing state signal from the Color Write unit.");
        }
    }

    /*  Get DAC state.  */
    if (dacStateSignal->read(cycle, (DynamicObject *&) dacStateInfo))
    {
        /*  Store rasterizer state.  */
        dacState = dacStateInfo->getState();

        /*  Delete info object.  */
        delete dacStateInfo;
    }
    else
    {
        panic("CommandProcessor", "clock", "Missing state signal from the DAC unit.");
    }

    //  Update the delay counter for consecutive draw calls.
    if (drawCommandDelay > 0)
        drawCommandDelay--;
        
    /*  Perform the tasks for the current GPU state.  */
    
    switch (state.statusRegister)
    {
        case GPU_ST_RESET:
            //  The GPU is reseting its state.

            //  Reset GPU state.
            reset();

//printf("CP %lld => Change to READY state\n", cycle);


            //  Change state to GPU_READY after everything is done.
            state.statusRegister = GPU_READY;

            break;

        case GPU_READY:
            /*  The GPU is ready to process a new AGP Transaction.  */

            /*  Check if there are pending register updates.  */
            if (regUpdates[GEOM_REG] > 0)
            {
                /*  Process the register write.  */
                processGPURegisterWrite(cycle, updateBuffer[GEOM_REG][nextUpdate[GEOM_REG]].reg,
                    updateBuffer[GEOM_REG][nextUpdate[GEOM_REG]].subReg,
                    updateBuffer[GEOM_REG][nextUpdate[GEOM_REG]].data);

                /*  Update number of pending register updates.  */
                regUpdates[GEOM_REG]--;

                /*  Update number of free update buffer entries.  */
                freeUpdates[GEOM_REG]++;

                /*  Update pointer to the next register update.  */
                nextUpdate[GEOM_REG] = GPU_MOD(nextUpdate[GEOM_REG] + 1, MAX_REGISTER_UPDATES);
            }
            else if (regUpdates[FRAG_REG] > 0)
            {
                /*  Process the register write.  */
                processGPURegisterWrite(cycle, updateBuffer[FRAG_REG][nextUpdate[FRAG_REG]].reg,
                    updateBuffer[FRAG_REG][nextUpdate[FRAG_REG]].subReg,
                    updateBuffer[FRAG_REG][nextUpdate[FRAG_REG]].data);

                /*  Update number of pending register updates.  */
                regUpdates[FRAG_REG]--;

                /*  Update number of free update buffer entries.  */
                freeUpdates[FRAG_REG]++;

                /*  Update pointer to the next register update.  */
                nextUpdate[FRAG_REG] = GPU_MOD(nextUpdate[FRAG_REG] + 1, MAX_REGISTER_UPDATES);
            }
            else
            {
                if (processNewTransaction)
                {
                    //  Check if there the force transaction flag is set
                    if (!forceTransaction)
                    {
                        //  Try to read an AGP transaction from the trace driver.
                        auxAGPTrans = driver->nextAGPTransaction();
//printf("CP (%lld) => Next AGP Transaction %p\n", cycle, auxAGPTrans);                        
                        
                    }
                    else
                    {
                        //  Force the external transaction.
                        auxAGPTrans = forcedTransaction;

                        //  Unset force transaction flag.
                        forceTransaction = false;
                    }

                    if (auxAGPTrans != NULL)
                    {
                        //auxAGPTrans->dump();

                        /*  Delete old AGP transaction.  */
                        if (lastAGPTrans != NULL)
                            delete lastAGPTrans;

                        /*  Set new AGP transaction as last AGP transaction.  */
                        lastAGPTrans = auxAGPTrans;

                        //  Check if validation mode is enabled.
                        if (enableValidation)
                        {
                            //  Clone the AGP transaction.
                            AGPTransaction *clonedAGPTrans = new AGPTransaction(auxAGPTrans);

                            //  Add the AGP Transaction to the log.
                            agpTransactionLog[nextWriteLog].push_back(clonedAGPTrans);
                        }
                    }
                    else
                    {
                        /*  No AGP transaction means that the trace has finished with the current model.  */
                        traceEnd = TRUE;
                    }
                }

                /*  Check if there is any transaction to process.  */
                if (!traceEnd)
                {
                    /*  Start the processing of this AGP transaction.  */
                    processNewAGPTransaction(cycle);
                }
            }

            /*  Update statistics.  */
            readyCycles++;

            break;

        case GPU_DRAWING:
            /*  The GPU is drawing a batch of triangles/vertices.  */

            /*  Try to read an AGP transaction from the trace driver.  */
            if (processNewTransaction)
            {
                //  Check if there the force swap flag is set
                if (!forceTransaction)
                {
                    //  Try to read an AGP transaction from the trace driver.
                    auxAGPTrans = driver->nextAGPTransaction();
                }
                else
                {
                    //  Force the external transaction.
                    auxAGPTrans = forcedTransaction;

                    //  Unset force transaction flag.
                    forceTransaction = false;
                }

                if (auxAGPTrans != NULL)
                {
                    //auxAGPTrans->dump();

                    /*  Delete old AGP transaction.  */
                    if (lastAGPTrans != NULL)
                        delete lastAGPTrans;

                    /*  Set new AGP transaction as last AGP transaction.  */
                    lastAGPTrans = auxAGPTrans;

                    //  Check if validation mode is enabled.
                    if (enableValidation)
                    {
                        //  Clone the AGP transaction.
                        AGPTransaction *clonedAGPTrans = new AGPTransaction(auxAGPTrans);

                        //  Add the AGP Transaction to the log.
                        agpTransactionLog[nextWriteLog].push_back(clonedAGPTrans);
                    }
                }
                else
                {
                    /*  No AGP transaction means that the trace has finished with the current model.  */
                    traceEnd = TRUE;
                }
            }

            /*  Check if there is any transaction to process.  */
            if (pipelinedBatches && !traceEnd)
            {
                /*  Start the processing of this AGP transaction.  */
                processNewAGPTransaction(cycle);
            }

            /*  Wait for end messages from Streamer, Vertex Shader and
                Fragment processor.  */

            /*  Check if the geometry section has finished and the current transaction hasn't
                changed the state (to memory write).  */
            if ((state.statusRegister == GPU_DRAWING) && (clipState == CLP_END))
            {
                /*  Create END command for the streamer.  */
                streamComm = new StreamerCommand(STCOM_END);

                /*  Copy cookies from original AGP transaction and add new cookie.  */
                streamComm->copyParentCookies(*lastAGPTrans);
                streamComm->addCookie();

                /*  Send command to the streamer.  */
                streamCtrlSignal->write(cycle, streamComm);

                /*  Create END command for Primitive Assembly.  */
                paCommand = new PrimitiveAssemblyCommand(PACOM_END);

                /*  Copy cookies from original AGP transaction and add a
                    new cookie.  */
                paCommand->copyParentCookies(*lastAGPTrans);
                paCommand->addCookie();

                /*  Send command to Primitive Assembly.  */
                paCommSignal->write(cycle, paCommand);

                /*  Create END command to the Clipper.  */
                clipperCommand = new ClipperCommand(CLPCOM_END);

                /*  Copy cookies from original AGP transaction and add
                    a new cookie.  */
                clipperCommand->copyParentCookies(*lastAGPTrans);
                clipperCommand->addCookie();

                /*  Send command to the Clipper.  */
                clipCommSignal->write(cycle, clipperCommand);

                /*  Set geometry rendering phase as finished.  */
                geometryStarted = FALSE;

//printf("CP %lld => Change to END_GEOMETRY state\n", cycle);

                /*  Change state to end of draw.  */
                state.statusRegister = GPU_END_GEOMETRY;
            }

            /*  Update statistics.  */
            drawCycles++;

            break;

        case GPU_END_GEOMETRY:

            /*  Check if there are pending geometry register updates.  */
            if ((regUpdates[GEOM_REG] > 0) && !geometryStarted)
            {
                /*  Process the register write.  */
                processGPURegisterWrite(cycle, updateBuffer[GEOM_REG][nextUpdate[GEOM_REG]].reg,
                    updateBuffer[GEOM_REG][nextUpdate[GEOM_REG]].subReg,
                    updateBuffer[GEOM_REG][nextUpdate[GEOM_REG]].data);

                /*  Update number of pending register updates.  */
                regUpdates[GEOM_REG]--;

                /*  Update number of free update buffer entries.  */
                freeUpdates[GEOM_REG]++;

                /*  Update pointer to the next register update.  */
                nextUpdate[GEOM_REG] = GPU_MOD(nextUpdate[GEOM_REG] + 1, MAX_REGISTER_UPDATES);

//printf("CP %lld => Updating buffered geometry state\n", cycle);
            }
            else
            {
                /*  Try to read an AGP transaction from the trace driver.  */
                if (processNewTransaction)
                {
                    //  Check if there the force transaction flag is set
                    if (!forceTransaction)
                    {
                        //  Try to read an AGP transaction from the trace driver.
                        auxAGPTrans = driver->nextAGPTransaction();
                    }
                    else
                    {
                        //  Force the extern transaction.
                        auxAGPTrans = forcedTransaction;

                        //  Unset force transaction flag.
                        forceTransaction = false;
                    }

                    if (auxAGPTrans != NULL)
                    {
                        /*  Delete old AGP transaction.  */
                        if (lastAGPTrans != NULL)
                            delete lastAGPTrans;

                        /*  Set new AGP transaction as last AGP transaction.  */
                        lastAGPTrans = auxAGPTrans;

                        //  Check if validation mode is enabled.
                        if (enableValidation)
                        {
                            //  Clone the AGP transaction.
                            AGPTransaction *clonedAGPTrans = new AGPTransaction(auxAGPTrans);

                            //  Add the AGP Transaction to the log.
                            agpTransactionLog[nextWriteLog].push_back(clonedAGPTrans);
                        }
                    }
                    else
                    {
                        /*  No AGP transaction means that the trace has finished with the current model.  */
                        traceEnd = TRUE;
                    }
                }

                /*  Check if there is any transaction to process.  */
                if (pipelinedBatches && !traceEnd)
                {
                    /*  Start the processing of this AGP transaction.  */
                    processNewAGPTransaction(cycle);
                }
            }

            /*  Check if the fragment section has finished and the current transaction
                hasn't changed the state (to memory write or memory read).  */
            if ((state.statusRegister == GPU_END_GEOMETRY) && endAllCW)
            {
                /*  Create END command for the rasterizer.  */
                rastComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original AGP transaction and add new cookie.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the command to the Rasterizer.  */
                rastCommSignal->write(cycle, rastComm);


                /*  Send END command to all the Z Stencil units.  */
                for(i = 0; i < numStampUnits; i++)
                {
                    /*  Create END command for the Z Stencil Test unit.  */
                    rastComm = new RasterizerCommand(RSCOM_END);

                    /*  Copy cookies from original AGP transaction and add new cookie.  */
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    /*  Send the command to all the Z Stencil Test.  */
                    zStencilCommSignal[i]->write(cycle, rastComm);
                }

                /*  Send END command to all the Color Write units.  */
                for(i = 0; i < numStampUnits; i++)
                {
                    /*  Create END command for the Color Write unit.  */
                    rastComm = new RasterizerCommand(RSCOM_END);

                    /*  Copy cookies from original AGP transaction and add new cookie.  */
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    /*  Send the command to the Color Write unit.  */
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

//printf("CP %lld => Change to END_FRAGMENT state\n", cycle);

                /*  Change state to end of draw.  */
                state.statusRegister = GPU_END_FRAGMENT;
            }

            /*  Update statistics.  */
            endGeomCycles++;

            break;

        case GPU_END_FRAGMENT:
            /*  The GPU has finished drawing the current batch of triangles/vertices.  */

            /*  Check if there are fragment register updates buffered.  */
            if (regUpdates[FRAG_REG] > 0)
            {
                /*  Process the register write.  */
                processGPURegisterWrite(cycle, updateBuffer[FRAG_REG][nextUpdate[FRAG_REG]].reg,
                    updateBuffer[FRAG_REG][nextUpdate[FRAG_REG]].subReg,
                    updateBuffer[FRAG_REG][nextUpdate[FRAG_REG]].data);

                /*  Update number of pending register updates.  */
                regUpdates[FRAG_REG]--;

                /*  Update number of free update buffer entries.  */
                freeUpdates[FRAG_REG]++;

                /*  Update pointer to the next register update.  */
                nextUpdate[FRAG_REG] = GPU_MOD(nextUpdate[FRAG_REG] + 1, MAX_REGISTER_UPDATES);

//printf("CP %lld => Updating buffered fragment state\n", cycle);
            }
            else
            {
                /*  Check if there is a buffered load fragment program transaction.  */
                if (storedLoadFragProgram)
                {
                    /*  Check if transaction has finished.  */
                    if (lastAGPTrans == loadFragProgram)
                    {
                        /*  Delete buffered transaction.  */
                        delete lastAGPTrans;
                        loadFragProgram = NULL;

                        /*  Restore original last AGP transaction and state.  */
                        lastAGPTrans = backupAGPTrans;
                        processNewTransaction = backupProcNewTrans;

                        /*  Unset buffer load fragment program transaction flag.  */
                        storedLoadFragProgram = FALSE;
                    }
                    else
                    {
                        /*  Keep current last AGP transaction and state.  */
                        backupAGPTrans = lastAGPTrans;
                        backupProcNewTrans = processNewTransaction;

                        /*  Set buffered transaction as last.  */
                        lastAGPTrans = loadFragProgram;

                        /*  Process transaction.  */
                        processNewAGPTransaction(cycle);
                   }
                }
                else
                {
                    /*  Determine next state, draw if the next batch has already started the geometry
                        phase and there are no more fragment register updates or ready state if pipelined
                        batch processing can't continue.  */
                    if (geometryStarted)
                    {
                        /*  Create a start drawing signal to the Rasterizer.  */
                        rastComm = new RasterizerCommand(RSCOM_DRAW);

                        /*  Copy cookies from original AGP transaction and a new cookie.  */
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        /*  Send the command to the Rasterizer.  */
                        rastCommSignal->write(cycle, rastComm);


                        /*  Send draw command to all the Z Stencil units.  */
                        for(i = 0; i < numStampUnits; i++)
                        {
                            /*  Create draw command to the Z Stencil Test unit.  */
                            rastComm = new RasterizerCommand(RSCOM_DRAW);

                            /*  Copy cookies from original AGP transaction and a new cookie.  */
                            rastComm->copyParentCookies(*lastAGPTrans);
                            rastComm->addCookie();

                            /*  Send the reset signal to the Z Stencil Test unit.  */
                            zStencilCommSignal[i]->write(cycle, rastComm);
                        }

                        /*  Send draw command to all the Color Write units.  */
                        for(i = 0; i < numStampUnits; i++)
                        {
                            /*  Create draw command to the Color Write unit.  */
                            rastComm = new RasterizerCommand(RSCOM_DRAW);

                            /*  Copy cookies from original AGP transaction and a new cookie.  */
                            rastComm->copyParentCookies(*lastAGPTrans);
                            rastComm->addCookie();

                            /*  Send the reset signal to the Color Write unit.  */
                            colorWriteCommSignal[i]->write(cycle, rastComm);
                        }

//printf("CP %lld => Change to DRAWING state.  Start pipelined batch rendering fragment phase.\n", cycle);

                        //  Set end of command flag.
                        commandEnd = true;

                        /*  Change state to GPU_DRAW for pipelined batch rendering.  */
                        state.statusRegister = GPU_DRAWING;
                    }
                    else
                    {

                        /*  Check if pipelined batch rendering is disabled.  */
                        if (!pipelinedBatches)
                        {
                            /*  After the batch has finished new transactions can be processed.  */
                            processNewTransaction = true;
                        }

//printf("CP %lld => Change to READY state.\n", cycle);

                        //  Set end of command flag.
                        commandEnd = true;

                        /*  Change state to GPU_READY after 'everything' is done.  */
                        state.statusRegister = GPU_READY;
                    }

                    /*  Current batch has finished (FRAGMENT PHASE).  */
                    batchEnd = true;
                }
            }

            /*  Update statistics.  */
            endFragCycles++;

            break;

        case GPU_MEMORY_WRITE:
            /*  Memory controller writing to GPU memory.  */

            /*  Check if data transmission in progress.  */
            if (transCycles > 0)
            {
                /*  Update transmission counter.  */
                transCycles--;

                GPU_DEBUG_BOX( printf("CommandProcessor => Writing to memory.  Remaining cycles: %d.\n", transCycles); )

                /*  Check if the current transaction has finished.  */
                if (transCycles == 0)
                {
                    /*  Update free memory tickets counter.  */
                    freeTickets++;
                }
            }

            /*  Check last agp transaction type.  */
            if (lastAGPTrans->getAGPCommand() == AGP_WRITE)
            {
                /*  Check if there are more data to write.  */
                if (sent < lastAGPTrans->getSize())
                {
                    /*  Create and send new write transaction if memory controller is available.  */
                    if ((transCycles == 0) && ((memoryState & MS_WRITE_ACCEPT) != 0) && (freeTickets > 0))
                    {
                        /*  Update free tickets counter.  */
                        freeTickets--;

                        /*  Set transaction size.  */
                        size = GPU_MIN(lastAGPTrans->getSize() - sent, MAX_TRANSACTION_SIZE);

                        /*  Create a new memory transaction.  */
                        memTrans = new MemoryTransaction(
                            MT_WRITE_DATA,
                            lastAGPTrans->getAddress() + sent,
                            size, &(lastAGPTrans->getData()[sent]),
                            COMMANDPROCESSOR,
                            currentTicket++);

                        /*  Copy original AGP transaction cookie and add new cookie.  */
                        memTrans->copyParentCookies(*lastAGPTrans);
                        memTrans->addCookie();

                        /*  Send the memory transaction to the Memory Controller.  */
                        writeMemorySignal->write(cycle, memTrans, 1);

                        /*  NOTE:  DOESN'T TAKES INTO ACCOUNT THE WIDHT OF THE AGP BUS.  IT IS ASSUMED
                            THAT THE AGP BUS HAS THE SAME WIDTH THAT THE BUS FROM THE COMMAND PROCESSOR
                            TO THE MEMORY CONTROLLER.  THERE ARE ALSO NO ADDITIONAL PENALTIES OR TRANSACTIONS
                            REQUIRED AFTER THE INITIAL ONE.  */

                        /*  Set AGP transmission counter.  */
                        transCycles = memTrans->getBusCycles();

                        /*  Update sent bytes counter.  */
                        sent += size;

                        /*  Update statistics.  */
                        bytesWritten.inc(size);
                    }
                }

                /*  Check there is no data transmission in progress and no more
                    data must be requested.  */
                if ((transCycles == 0) && (sent == lastAGPTrans->getSize()))
                {
//printf("CP %lld => Change to stacked state (%d)\n", cycle, stateStack);

                    //  Set end of command flag.
                    commandEnd = true;

                    /*  Return to previous state stored in the state stack.  */
                    //state.statusRegister = GPU_READY;
                    state.statusRegister = stateStack;
                }
            }
            else
            {
                panic("MemoryController", "clock", "Unsupported AGP command writing to memory.");
            }

            /*  Updata state statistics.  */
            memWriteCycles++;

            break;

        case GPU_MEMORY_PRELOAD:

            /*  Memory controller preloading data into GPU memory.  */

//printf("CP %lld => Change to stacked state (%d)\n", cycle, stateStack);

            /*  Just 1 cycle delay required with current implementation.
                Return to previous state stored in the state stack.  */
            //state.statusRegister = GPU_READY;
            state.statusRegister = stateStack;

            /*  Updata state statistics.  */
            memPreLoadCycles++;

            break;

        case GPU_MEMORY_READ:
            /*  Command Processor reading from GPU memory.  */

            /*  Check data transmission in progress.  */
            if (transCycles > 0)
            {
                /*  Update memory transmission counter.  */
                transCycles--;

                GPU_DEBUG_BOX( printf("CommandProcessor => Reading data from memory.  Remaining cycles: %d.\n", transCycles); )

                /*  Check if the memory transmission has finished.  */
                if (transCycles == 0)
                {

                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => End of data transmision.\n");
                    )

                    /*  Update received bytes from memory.  */
                    received += lastSize;

                    /*  Update free memory tickets counter.  */
                    freeTickets++;
                }
            }

            /*  Check that last agp transaction was a Command.  */
            if (lastAGPTrans->getAGPCommand() == AGP_COMMAND)
            {
                /*  Check load vertex program command.  */
                if (lastAGPTrans->getGPUCommand() == GPU_LOAD_VERTEX_PROGRAM)
                {
                    /*  Check if there is more data to request.  */
                    if (requested < state.vertexProgramSize)
                    {
                        /*  Request more program code.  */

                        /*  Check if memory controller is available.  */
                        if (((memoryState & MS_READ_ACCEPT) != 0) && (freeTickets > 0))
                        {
                            /*  Update free memory ticket counter.  */
                            freeTickets--;

                            /*  Set requested bytes to memory.  */
                            size = GPU_MIN(state.vertexProgramSize - requested, MAX_TRANSACTION_SIZE);

                            GPU_DEBUG_BOX(
                                printf("CommandProcessor => Requesting from memory at %x %d bytes.\n",
                                    state.programMemoryBaseAddr + state.vertexProgramAddr + requested,
                                    size);
                            )

                            /*  Create a memory transaction to request the
                                vertex program from the GPU memory.  */
                            memTrans = new MemoryTransaction(
                                MT_READ_REQ,
                                state.programMemoryBaseAddr + state.vertexProgramAddr + requested,
                                size,
                                &programCode[requested],
                                COMMANDPROCESSOR,
                                currentTicket++);

                            /*  Copy original AGP transaction cookie and add new cookie.  */
                            memTrans->copyParentCookies(*lastAGPTrans);
                            memTrans->addCookie();

                            /*  Send the memory transaction to the memory controller.  */
                            writeMemorySignal->write(cycle, memTrans, 1);

                            /*  Update requested counter.  */
                            requested += size;
                        }
                    }

                    /*  Check if all the program code has been received.  */
                    if (received == state.vertexProgramSize)
                    {
                        //  Check if dumping shader programs to files is enabled.
                        if (dumpShaderPrograms)
                        {
                            //  Write program into a file.
                            ofstream outFile;
                            char outFileName[256];
                            sprintf(outFileName, "vprogram%04d.out", vshProgID);
                            outFile.open(outFileName, ios::binary);
                            outFile.write((char *) programCode, state.vertexProgramSize);
                            outFile.close();
                            vshProgID++;
                        }

                        /*  Send the shader program to all the vertex shader units.  */
                        for (i = 0; i < numVShaders; i++)
                        {

                            GPU_DEBUG_BOX(
                                printf("CommandProcessor => Sending vertex program to vertex shader %d.\n",
                                    i);
                            )

                            /*  Create a shader command to send the vertex shader program code.  */
                            shComm = new ShaderCommand(state.vertexProgramStartPC, programCode, state.vertexProgramSize);

                            /*  Copy cookies from original AGP transaction and a new cookie.  */
                            shComm->copyParentCookies(*lastAGPTrans);
                            shComm->addCookie();

                            /*  Send the shader command to each vertex shader unit.  */
                            vshFCommSignal[i]->write(cycle, shComm, 1);
                        }

                        /*  SHOULD WAIT FOR THE TRANSMISSION TO THE VSH??  */

//printf("CP %lld => Change to stacked state (%d)\n", cycle, stateStack);

                        //  Set end of command flag.
                        commandEnd = true;

                        /*  Change state to the previous state stored in the stack.  */
                        state.statusRegister = stateStack;
                    }
                }
                else if (lastAGPTrans->getGPUCommand() == GPU_LOAD_FRAGMENT_PROGRAM)
                {
                    /*  Check if there is more data to request.  */
                    if (requested < state.fragProgramSize)
                    {
                        /*  Request more program code.  */

                        /*  Check if memory controller is available.  */
                        if (((memoryState & MS_READ_ACCEPT) != 0) && (freeTickets > 0))
                        {
                            /*  Update free memory ticket counter.  */
                            freeTickets--;

                            /*  Set requested bytes to memory.  */
                            size = GPU_MIN(state.fragProgramSize - requested, MAX_TRANSACTION_SIZE);

                            GPU_DEBUG_BOX(
                                printf("CommandProcessor => Requesting from memory at %x %d bytes.\n",
                                    state.programMemoryBaseAddr + state.fragProgramAddr + requested,
                                    size);
                            )

                            /*  Create a memory transaction to request the fragment program from the GPU memory.  */
                            memTrans = new MemoryTransaction(
                                MT_READ_REQ,
                                state.programMemoryBaseAddr + state.fragProgramAddr + requested,
                                size,
                                &programCode[requested],
                                COMMANDPROCESSOR,
                                currentTicket++);

                            /*  Copy original AGP transaction cookie and add new cookie.  */
                            memTrans->copyParentCookies(*lastAGPTrans);
                            memTrans->addCookie();

                            /*  Send the memory transaction to the memory controller.  */
                            writeMemorySignal->write(cycle, memTrans, 1);

                            /*  Update requested counter.  */
                            requested += size;
                        }
                    }

                    /*  Check if all the program code has been received.  */
                    if (received == state.fragProgramSize)
                    {
                        //  Check if dumping shader programs to files is enabled.
                        if (dumpShaderPrograms)
                        {
                            //  Write program into a file.
                            ofstream outFile;
                            char outFileName[256];
                            sprintf(outFileName, "fprogram%04d.out", fshProgID);
                            outFile.open(outFileName, ios::binary);
                            outFile.write((char *) programCode, state.fragProgramSize);
                            outFile.close();
                            fshProgID++;
                        }

                        /*  Send the shader program to all the fragment shader units.  */
                        for (i = 0; i < numFShaders; i++)
                        {

                            GPU_DEBUG_BOX(
                                printf("CommandProcessor => Sending fragment program to fragment shader %d.\n",
                                    i);
                            )

                            /*  Create a shader command to send the fragment shader program code.  */
                            shComm = new ShaderCommand(state.fragProgramStartPC, programCode, state.fragProgramSize);

                            /*  Copy cookies from original AGP transaction and a new cookie.  */
                            shComm->copyParentCookies(*lastAGPTrans);
                            shComm->addCookie();

                            /*  Send the shader command to each fragment shader unit.  */
                            fshFCommandSignal[i]->write(cycle, shComm, 1);
                        }

                        /*  SHOULD WAIT FOR THE TRANSMISSION TO THE FSH??  */

//printf("CP %lld => Change to stacked state (%d)\n", cycle, stateStack);

                        //  Set end of command flag.
                        commandEnd = true;

                        /*  Change state to the previous state stored in the stack.  */
                        state.statusRegister = stateStack;
                    }
                }
                else if (lastAGPTrans->getGPUCommand() == GPU_LOAD_SHADER_PROGRAM)
                {
                    //  Check if there is more data to request.
                    if (requested < state.programSize)
                    {
                        //  Request more program code.

                        //  Check if memory controller is available.
                        if (((memoryState & MS_READ_ACCEPT) != 0) && (freeTickets > 0))
                        {
                            //  Update free memory ticket counter.
                            freeTickets--;

                            //  Set requested bytes to memory.  */
                            size = GPU_MIN(state.programSize - requested, MAX_TRANSACTION_SIZE);

                            GPU_DEBUG_BOX(
                                printf("CommandProcessor => Requesting from memory at %x %d bytes.\n",
                                    state.programMemoryBaseAddr + state.programAddress + requested,
                                    size);
                            )

                            //  Create a memory transaction to request the fragment program from the GPU memory.
                            memTrans = new MemoryTransaction(
                                MT_READ_REQ,
                                state.programMemoryBaseAddr + state.programAddress + requested,
                                size,
                                &programCode[requested],
                                COMMANDPROCESSOR,
                                currentTicket++);

                            //  Copy original AGP transaction cookie and add new cookie.
                            memTrans->copyParentCookies(*lastAGPTrans);
                            memTrans->addCookie();

                            //  Send the memory transaction to the memory controller.
                            writeMemorySignal->write(cycle, memTrans, 1);

                            //  Update requested counter.
                            requested += size;
                        }
                    }

                    /*  Check if all the program code has been received.  */
                    if (received == state.programSize)
                    {
                        //  Check if dumping shader programs to files is enabled.
                        if (dumpShaderPrograms)
                        {
                            //  Write program into a file.
                            ofstream outFile;
                            char outFileName[256];
                            sprintf(outFileName, "shprogram%04d.out", shProgID);
                            outFile.open(outFileName, ios::binary);
                            outFile.write((char *) programCode, state.programSize);
                            outFile.close();
                            shProgID++;
                        }

                        //  Send the shader program to all the fragment shader units.
                        for (i = 0; i < numFShaders; i++)
                        {

                            GPU_DEBUG_BOX(
                                printf("CommandProcessor => Sending shader program to fragment shader %d.\n",
                                    i);
                            )

                            //  Create a shader command to send the fragment shader program code.
                            shComm = new ShaderCommand(state.programLoadPC, programCode, state.programSize);

                            //  Copy cookies from original AGP transaction and a new cookie.
                            shComm->copyParentCookies(*lastAGPTrans);
                            shComm->addCookie();

                            //  Send the shader command to each fragment shader unit.
                            fshFCommandSignal[i]->write(cycle, shComm, 1);
                        }

                        //  SHOULD WAIT FOR THE TRANSMISSION TO THE FSH??

//printf("CP %lld => Change to stacked state (%d)\n", cycle, stateStack);

                        //  Set end of command flag.
                        commandEnd = true;

                        //  Change state to the previous state stored in the stack.
                        state.statusRegister = stateStack;
                    }
                }
                else
                {
                    panic("CommandProcessor", "clock", "Unsupported GPU command reading from memory.");
                }
            }
            else
            {
                    panic("CommandProcessor", "clock", "Unsupported AGP transaction reading from memory.");
            }

            /*  Updata state statistics.  */
            memReadCycles++;

            break;

        case GPU_SWAP:

            /*  Swap state.  */

            /*  Wait for all Color Write units to be in end state.  */
            if (endAllCW && (!colorWriteEnd))
            {
                /*  Send END command to all the Color Write units.  */
                for(i = 0; i < numStampUnits; i++)
                {
                    /*  Create END command for the Color Write unit.  */
                    rastComm = new RasterizerCommand(RSCOM_END);

                    /*  Copy cookies from original AGP transaction and add new cookie.  */
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    /*  Send the command to the Color Write unit.  */
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

                /*  Send SWAP command to the DAC unit.  */
                rastComm = new RasterizerCommand(RSCOM_SWAP);

                /*  Copy cookies and add a cookie level.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the command to the DAC unit.  */
                dacCommSignal->write(cycle, rastComm);

                /*  Set end command to color write flag.  */
                colorWriteEnd = true;
            }

            /**  NOTE:  THE COMMAND PROCESSOR SHOULD NOT WAIT FOR THE
                 DAC, AS IT SHOULD JUST HAVE TO SIMULATE THE SCREEN/DISPLAY
                 REFRESH, BUT AS THERE IS NO VERTICAL SYNC IMPLEMENTED
                 THIS A WAY TO DO IT FOR NOW.  **/

            /*  Check if DAC has finished dumping the framebuffer.  */
            if (dacState == RAST_END)
            {
                /*  Send END command to the DAC unit.  */
                rastComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies and add a cookie level.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the command to the DAC unit.  */
                dacCommSignal->write(cycle, rastComm);

                /*  Reset batch number.  */
                batch = 0;

//printf("CP %lld => Change to READY state.\n", cycle);

                /*  Set swap command received flag.  */
                swapReceived = TRUE;

                //  Set end of command flag.
                commandEnd = true;

                /*  Change state to ready.  */
                state.statusRegister = GPU_READY;
            }

            /*  Update statistics.  */
            swapCycles++;

            break;

        case GPU_DUMPBUFFER:

            //  Dump buffer (debug mode only).

            switch(dumpBufferCommand)
            {
                case RSCOM_DUMP_DEPTH:
                case RSCOM_DUMP_STENCIL:

                    //  Wait for all ZStencil Units to be in end state.
                    if (endAllZST && (!zStencilTestEnd))
                    {
                        //  Send END command to all the Z Stencil Test units.
                        for(i = 0; i < numStampUnits; i++)
                        {
                            //  Create END command for the Z Stencil Test unit.
                            rastComm = new RasterizerCommand(RSCOM_END);

                            //  Copy cookies from original AGP transaction and add new cookie.
                            rastComm->copyParentCookies(*lastAGPTrans);
                            rastComm->addCookie();

                            //  Send the command to the Z Stencil Test unit.
                            zStencilCommSignal[i]->write(cycle, rastComm);
                        }

                        //  Send DUMP_BUFFER command to the DAC unit.
                        rastComm = new RasterizerCommand(dumpBufferCommand);

                        //  Copy cookies and add a cookie level.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the DAC unit.
                        dacCommSignal->write(cycle, rastComm);

                        //  Set END commands to Z Stencil Test units processed flag..
                        zStencilTestEnd = true;
                    }

                    break;

                case RSCOM_DUMP_COLOR:

                    //  Wait for all Color Write units to be in end state.
                    if (endAllCW && (!colorWriteEnd))
                    {
                        //  Send END command to all the Color Write units.
                        for(i = 0; i < numStampUnits; i++)
                        {
                            //  Create END command for the Color Write unit.
                            rastComm = new RasterizerCommand(RSCOM_END);

                            //  Copy cookies from original AGP transaction and add new cookie.
                            rastComm->copyParentCookies(*lastAGPTrans);
                            rastComm->addCookie();

                            //  Send the command to the Color Write unit.
                            colorWriteCommSignal[i]->write(cycle, rastComm);
                        }

                        //  Send SWAP command to the DAC unit.
                        rastComm = new RasterizerCommand(dumpBufferCommand);

                        //  Copy cookies and add a cookie level.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the DAC unit.
                        dacCommSignal->write(cycle, rastComm);

                        //  Set END command to Color Write units processed.
                        colorWriteEnd = true;
                    }

                    break;

                default:

                    panic("CommandProcessor", "clock", "Expected dump buffer rasterizer command.");
                    break;
            }

            //  Check if DAC has finished dumping the buffer.
            if (dacState == RAST_END)
            {
                //  Send END command to the DAC unit.
                rastComm = new RasterizerCommand(RSCOM_END);

                //  Copy cookies and add a cookie level.
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                //  Send the command to the DAC unit.
                dacCommSignal->write(cycle, rastComm);

                //  Set end of command flag.
                commandEnd = true;

                //  Change state to ready.
                state.statusRegister = GPU_READY;
            }

            break;

        case GPU_BLITTING:

            /*  Framebuffer blit state.  */

            /*  Wait for all Color Write units to be in end state.  */
            if (endAllCW && (!colorWriteEnd))
            {
                /*  Send END command to all the Color Write units.  */
                for(i = 0; i < numStampUnits; i++)
                {
                    /*  Create END command for the Color Write unit.  */
                    rastComm = new RasterizerCommand(RSCOM_END);

                    /*  Copy cookies from original AGP transaction and add new cookie.  */
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    /*  Send the command to the Color Write unit.  */
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

                /*  Send BLIT command to the DAC unit.  */
                rastComm = new RasterizerCommand(RSCOM_BLIT);

                /*  Copy cookies and add a cookie level.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the command to the DAC unit.  */
                dacCommSignal->write(cycle, rastComm);

                /*  Set end command to color write flag.  */
                colorWriteEnd = TRUE;
            }

            /*  Check if DAC has finished performing bit blit operation.  */
            if (dacState == RAST_END)
            {
                /*  Send END command to the DAC unit.  */
                rastComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies and add a cookie level.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the command to the DAC unit.  */
                dacCommSignal->write(cycle, rastComm);

//printf("CP %lld => Change to READY state.\n", cycle);

                //  Set end of command flag.
                commandEnd = true;

                /*  Change state to ready.  */
                state.statusRegister = GPU_READY;
            }

            /*  Update statistics.  */
            bitBlitCycles++;

            break;

        case GPU_CLEAR_COLOR:

            /*  Buffer clear state.  */

            /*  Wait  for all Color Write units to be in end state.  */
            if (endAllCW)
            {
                /*  Send END command to all the Color Write units.  */
                for(i = 0; i < numStampUnits; i++)
                {
                    /*  Create END command for the Color Write unit.  */
                    rastComm = new RasterizerCommand(RSCOM_END);

                    /*  Copy cookies from original AGP transaction and add new cookie.  */
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    /*  Send the command to the Color Write unit.  */
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

//printf("CP %lld => Change to READY state.\n", cycle);

                //  Set end of command flag.
                commandEnd = true;

                /*  Change state to ready.  */
                state.statusRegister = GPU_READY;
            }

            /*  Update statistics.  */
            clearCycles++;

            break;

        case GPU_CLEAR_Z:

            /*  Buffer clear state.  */

            /*  Wait  for Z Stencil Test end state.  */
            if (endAllZST && (rasterizerState == RAST_CLEAR_END))
            {

                /*  Send END command to all the Z Stencil units.  */
                for(i = 0; i < numStampUnits; i++)
                {
                    /*  Create END command for the Z Stencil Test unit.  */
                    rastComm = new RasterizerCommand(RSCOM_END);

                    /*  Copy cookies from original AGP transaction and add new cookie.  */
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    /*  Send the command to the Z Stencil Test unit.  */
                    zStencilCommSignal[i]->write(cycle, rastComm);
                }

                /*  Create END command to the Rasterizert.  */
                rastComm = new RasterizerCommand(RSCOM_END);

                /*  Copy cookies from original AGP transaction and add new cookie.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the command to the Rasterizer.  */
                rastCommSignal->write(cycle, rastComm);


//printf("CP %lld => Change to READY state.\n", cycle);

                //  Set end of command flag.
                commandEnd = true;

                /*  Change state to ready.  */
                state.statusRegister = GPU_READY;
            }

            /*  Update statistics.  */
            clearCycles++;

            break;

        case GPU_FLUSH_COLOR:

            //  Color caches flush state.

            //  Wait  for all Color Write units to be in end state.
            if (endAllCW)
            {
                //  Send END command to all the Color Write units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //  Create END command for the Color Write unit.
                    rastComm = new RasterizerCommand(RSCOM_END);

                    //  Copy cookies from original AGP transaction and add new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Color Write unit.
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

//printf("CP %lld => Change to READY state.\n", cycle);

                //  Set end of command flag.
                commandEnd = true;

                //  Change state to ready.
                state.statusRegister = GPU_READY;
            }

            //  Update statistics.
            flushCycles++;

            break;

        case GPU_FLUSH_Z:

            //  Z and stencil cache flush state.

            //  Wait  for Z Stencil Test end state.
            if (endAllZST && (flushDelayCycles == 0))
            {

                //  Send END command to all the Z Stencil units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //  Create END command for the Z Stencil Test unit.
                    rastComm = new RasterizerCommand(RSCOM_END);

                    //  Copy cookies from original AGP transaction and add new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Z Stencil Test unit.
                    zStencilCommSignal[i]->write(cycle, rastComm);
                }

                //  Start wait for HZ updates to finish.
                flushDelayCycles = 10000;
            }

            //  Check if waiting for HZ to finish receiving updates.
            if (flushDelayCycles > 0)
            {
                //  Update wait cycle counter.
                flushDelayCycles--;

                //  Check for en of wait.
                if (flushDelayCycles == 0)
                {
//printf("CP %lld => Change to READY state.\n", cycle);

                    //  Set end of command flag.
                    commandEnd = true;

                    //  Change state to ready.
                    state.statusRegister = GPU_READY;
                }
            }

            //  Update statistics.
            flushCycles++;

            break;

        case GPU_SAVE_STATE_COLOR:

            //  Save color block state info into memory state.

            //  Wait  for all Color Write units to be in end state.
            if (endAllCW)
            {
                //  Send END command to all the Color Write units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //  Create END command for the Color Write unit.
                    rastComm = new RasterizerCommand(RSCOM_END);

                    //  Copy cookies from original AGP transaction and add new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Color Write unit.
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

//printf("CP %lld => Change to READY state.\n", cycle);

                //  Set end of command flag.
                commandEnd = true;

                //  Change state to ready.
                state.statusRegister = GPU_READY;
            }

            //  Update statistics.
            saveRestoreStateCycles++;

            break;

        case GPU_RESTORE_STATE_COLOR:

            //  Restore color block state info from memory state.

            //  Wait  for all Color Write units to be in end state.
            if (endAllCW)
            {
                //  Send END command to all the Color Write units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //  Create END command for the Color Write unit.
                    rastComm = new RasterizerCommand(RSCOM_END);

                    //  Copy cookies from original AGP transaction and add new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Color Write unit.
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

//printf("CP %lld => Change to READY state.\n", cycle);

                //  Set end of command flag.
                commandEnd = true;

                //  Change state to ready.
                state.statusRegister = GPU_READY;
            }

            //  Update statistics.
            saveRestoreStateCycles++;

            break;

        case GPU_SAVE_STATE_Z:

            //  Save z/stencil block state info into memory state.

            //  Wait  for Z Stencil Test end state.
            if (endAllZST)
            {
                //  Send END command to all the Z Stencil units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //  Create END command for the Z Stencil Test unit.
                    rastComm = new RasterizerCommand(RSCOM_END);

                    //  Copy cookies from original AGP transaction and add new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Z Stencil Test unit.
                    zStencilCommSignal[i]->write(cycle, rastComm);
                }

//printf("CP %lld => Change to READY state.\n", cycle);

                //  Set end of command flag.
                commandEnd = true;

                //  Change state to ready.
                state.statusRegister = GPU_READY;
            }

            //  Update statistics.
            saveRestoreStateCycles++;

            break;

        case GPU_RESTORE_STATE_Z:

            //  Restore z/stencil block state info from memory state.

            //  Wait  for Z Stencil Test end state.
            if (endAllZST)
            {
                //  Send END command to all the Z Stencil units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //  Create END command for the Z Stencil Test unit.
                    rastComm = new RasterizerCommand(RSCOM_END);

                    //  Copy cookies from original AGP transaction and add new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Z Stencil Test unit.
                    zStencilCommSignal[i]->write(cycle, rastComm);
                }

                //  Set end of command flag.
                commandEnd = true;

//printf("CP %lld => Change to READY state.\n", cycle);

                //  Change state to ready.
                state.statusRegister = GPU_READY;
            }

            //  Update statistics.
            saveRestoreStateCycles++;

            break;

        case GPU_ERROR:
            /*  This shouldn't never happen ... for future error checking??  */

            panic("CommandProcessor", "clock", "GPU in ERROR.");

            break;

        default:
            /*  Unknown GPU state.  */

            panic("CommandProcessor", "clock", "Undefined GPU state.");

            break;
    }

}

/*  Starts to process a new AGP Transaction.  */
void CommandProcessor::processNewAGPTransaction(u64bit cycle)
{
    MemoryTransaction *memTrans;
    GPURegData data;
    u32bit regGroup;

    switch(lastAGPTrans->getAGPCommand())
    {
        case AGP_WRITE:
            /*  Data write to the GPU local memory.  */

            /*  Check current state and if transaction is locked.  */
            if ((state.statusRegister != GPU_READY) && (((state.statusRegister != GPU_DRAWING) &&
                ((state.statusRegister != GPU_END_GEOMETRY) || geometryStarted)) || lastAGPTrans->getLocked()))
            {
                /*  Wait until the end of the current batch before processing the transaction.  */
                processNewTransaction = FALSE;
            }
            else
            {
//printf("CP => State %d geometryStarted %d locked %d\n", state.statusRegister, geometryStarted,
//    lastAGPTrans->getLocked());

                GPU_DEBUG_BOX(
                    printf("CommandProcessor => Processing New AGP Transaction: AGP_WRITE.\n");
                    printf("CommandProcessor => AGP_WRITE addr %x size %d\n", lastAGPTrans->getAddress(),
                        lastAGPTrans->getSize());
                )

                /*  Check address space for write transaction.  Writes to mapped system memory are considered
                    as preloaded data, writes to gpu memory are considered as true writes.
                    Check for skip draw command flag.  To reduce skip time all the memory transactions are
                    considered PRELOAD writes.
                 */
                if (((lastAGPTrans->getAddress() & ADDRESS_SPACE_MASK) == SYSTEM_ADDRESS_SPACE) ||
                    skipDraw)
                {
//printf("CP %lld > Sending MT_PRELOAD_DATA @%x size %d data %p\n", cycle, lastAGPTrans->getAddress(),
//lastAGPTrans->getSize(), lastAGPTrans->getData());
                    /*  Create a new memory transaction.  */
                    memTrans = new MemoryTransaction(
                        MT_PRELOAD_DATA,
                        lastAGPTrans->getAddress(),
                        lastAGPTrans->getSize(),
                        lastAGPTrans->getData(),
                        COMMANDPROCESSOR,
                        currentTicket++);

                    /*  Copy original AGP transaction cookie and add new cookie.  */
                    memTrans->copyParentCookies(*lastAGPTrans);
                    memTrans->addCookie();

                    /*  Send the memory transaction to the Memory Controller.  */
                    writeMemorySignal->write(cycle, memTrans, 1);

                    /*  Store current state into the state stack.  */
                    stateStack = state.statusRegister;

//printf("CP %lld => Change to MEMORY_PRELOAD state.\n", cycle);

                    /*  Set GPU state to preload to memory.  This gives time (1 cycle) to the Memory Controller to
                        copy the data from the AGP transaction buffer.  */
                    state.statusRegister = GPU_MEMORY_PRELOAD;
                }
                else
                {
                    /*  Check Memory Controller status and issue a memory write transaction.  */
                    if ((memoryState & MS_WRITE_ACCEPT) != 0)
                    {

                        /**  SAME THAT OTHER ONE.  WHAT IS THE PURPOSE OF THE TICKETS.  **/

                        /*  Check memory ticket available.  */
                        GPU_ASSERT(
                            if (freeTickets == 0)
                                panic("CommandProcessor", "processNewAGPTransaction", "No available memory tickets.");
                        )

                        /*  Update free tickets counter.  */
                        freeTickets--;

                        /*  Set first write transaction bytes to sent.  */
                        sent = GPU_MIN(lastAGPTrans->getSize(), MAX_TRANSACTION_SIZE);

                        /*  Create a new memory transaction.  */
                        memTrans = new MemoryTransaction(
                            MT_WRITE_DATA,
                            lastAGPTrans->getAddress(),
                            sent,
                            lastAGPTrans->getData(),
                            COMMANDPROCESSOR,
                            currentTicket++);

                        /*  Copy original AGP transaction cookie and add new cookie.  */
                        memTrans->copyParentCookies(*lastAGPTrans);
                        memTrans->addCookie();

                        /*  Send the memory transaction to the Memory Controller.  */
                        writeMemorySignal->write(cycle, memTrans, 1);

                        /*  NOTE:  DOESN'T TAKES INTO ACCOUNT THE WIDHT OF THE AGP BUS.  IT IS ASSUMED
                            THAT THE AGP BUS HAS THE SAME WIDTH THAT THE BUS FROM THE COMMAND PROCESSOR
                            TO THE MEMORY CONTROLLER.  THERE ARE ALSO NO ADDITIONAL PENALTIES OR TRANSACTIONS
                            REQUIRED.  */

                        /*  Set AGP transmission counter.  */
                        transCycles = memTrans->getBusCycles();

                    }
                    else
                    {
                        /*  Memory controller is not ready.  Wait until it is ready.  */

                        /*  Reset sent bytes counter.  */
                        sent = 0;
                    }

                    /*  Update statistics.  */
                    writeTrans++;
                    bytesWritten.inc(sent);

                    /*  Store current state into the state stack.  */
                    stateStack = state.statusRegister;

//printf("CP %lld => Change to MEMORY_WRITE state.\n", cycle);

                    /*  Set GPU state to writing to memory.  */
                    state.statusRegister = GPU_MEMORY_WRITE;
                }

                /*  Allow a new transaction to be requested as this has been processed.  */
                processNewTransaction = TRUE;
            }

            break;

        case AGP_PRELOAD:
            /*  Data preload into system or GPU memory.  */

            /*  Check current state and if transaction is locked.  */
            if ((state.statusRegister != GPU_READY) && (((state.statusRegister != GPU_DRAWING) &&
                ((state.statusRegister != GPU_END_GEOMETRY) || geometryStarted)) || lastAGPTrans->getLocked()))
            {
                /*  Wait until the end of the current batch before processing the transaction.  */
                processNewTransaction = FALSE;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => Processing New AGP Transaction: AGP_PRELOAD.\n");
                    printf("CommandProcessor => AGP_PRELOAD addr %x size %d\n", lastAGPTrans->getAddress(),
                        lastAGPTrans->getSize());
                )

                /*  Preload the data from the transaction in GPU memory or system memory space.  */

                /*  Create a new memory transaction.  */
                memTrans = new MemoryTransaction(
                    MT_PRELOAD_DATA,
                    lastAGPTrans->getAddress(),
                    lastAGPTrans->getSize(),
                    lastAGPTrans->getData(),
                    COMMANDPROCESSOR,
                    currentTicket++);

                /*  Copy original AGP transaction cookie and add new cookie.  */
                memTrans->copyParentCookies(*lastAGPTrans);
                memTrans->addCookie();

                /*  Send the memory transaction to the Memory Controller.  */
                writeMemorySignal->write(cycle, memTrans, 1);

                /*  Store current state into the state stack.  */
                stateStack = state.statusRegister;

//printf("CP %lld => Change to MEMORY_PRELOAD state.\n", cycle);

                /*  Set GPU state to preload to memory.  This gives time (1 cycle) to the Memory Controller to
                    copy the data from the AGP transaction buffer.  */
                state.statusRegister = GPU_MEMORY_PRELOAD;

                /*  Allow a new transaction to be requested as this has been processed.  */
                processNewTransaction = TRUE;
            }

            break;

        case AGP_READ:
            /*  Data read request from GPU local memory.  */

            GPU_DEBUG_BOX( printf("CommandProcessor => Procesing AGP Transaction: AGP_READ.\n"); )

            /*  Check that the AGP transaction can be processed.  */

            /*  Check Memory Controller status and issue a memory read
                request.  */

            /*  !!! AGP_READ UNSOPPORTED RIGHT NOW !!!!  */
            panic("CommandProcessor", "processNewAGPTransaction", "AGP_READ transaction unimplemented.");

            break;

        case AGP_REG_WRITE:

            /*  Determine register type.  */
            //regGroup = (lastAGPTrans->getGPURegister() < GPU_LAST_GEOMETRY_REGISTER)?GEOM_REG:FRAG_REG;
            if (lastAGPTrans->getGPURegister() < GPU_LAST_GEOMETRY_REGISTER)
                regGroup = GEOM_REG;
            else
                regGroup = FRAG_REG;

            /*  Check current GPU state.  */
            if ((state.statusRegister == GPU_DRAWING) ||
                ((state.statusRegister == GPU_END_GEOMETRY) &&
                ((regGroup == FRAG_REG) || ((regGroup == GEOM_REG) && geometryStarted))))
            {
                /*  Check register group and if batch has started in end geometry state.  */
                if ((state.statusRegister == GPU_END_GEOMETRY) && (regGroup == FRAG_REG) && geometryStarted)
                {
                    /*  Fragment register updates after a batch can't be stored until the fragment phase
                        starts the same batch.  */

                    /*  The transaction couldn't be processed, so wait until it can be.  */
                    processNewTransaction = FALSE;
                }
                else
                {
                    /*  Check if the register update buffer isn't full.  */
                    if (freeUpdates[regGroup] > 0)
                    {
                        /*  NOTE:  IN CURRENT IMPLEMENTATION SUPPORT FOR REGISTER WRITE COOKIES IS
                            BROKEN.  THE AGP TRANSACTION OR THE COOKIES SHOULD BE KEPT UNTIL THE
                            UPDATE IS SENT TO THE UNIT.  */

                        GPU_DEBUG_BOX(
                            printf("CommandProcessor => Procesing AGP Transaction: AGP_REG_WRITE.\n");
                        )

//printf("CP => Buffering register change for phase %s\n", regGroup == GEOM_REG?"GEOMETRY":"FRAGMENT");

                        /*  Store register update.  */
                        updateBuffer[regGroup][nextFreeUpdate[regGroup]].reg = lastAGPTrans->getGPURegister();
                        updateBuffer[regGroup][nextFreeUpdate[regGroup]].subReg = lastAGPTrans->getGPUSubRegister();
                        updateBuffer[regGroup][nextFreeUpdate[regGroup]].data = lastAGPTrans->getGPURegData();

                        /*  Update number of register updates.  */
                        regUpdates[regGroup]++;

                        /*  Update number of free register update entries.  */
                        freeUpdates[regGroup]--;

                        /*  Update pointer to the next free register update entry.  */
                        nextFreeUpdate[regGroup] = GPU_MOD(nextFreeUpdate[regGroup] + 1, MAX_REGISTER_UPDATES);

                        /*  Allow processing the next transaction as current has finished.  */
                        processNewTransaction = TRUE;
                    }
                    else
                    {
// printf("CP => No free update buffer entries for group %s\n", regGroup == GEOM_REG?"GEOMETRY":"FRAGMENT");
// panic("CommandProcessor", "processNewAGPTransaction", "JUST TO KNOW ABOUT IT!!");                        /*  The transaction couldn't be processed, so wait until it can be.  */
                        processNewTransaction = FALSE;
                    }
                }
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => Procesing AGP Transaction: AGP_REG_WRITE.\n");
                )

                /*  Process GPU register write.  */
                processGPURegisterWrite(cycle, lastAGPTrans->getGPURegister(),
                    lastAGPTrans->getGPUSubRegister(), lastAGPTrans->getGPURegData());

                /*  Allow processing the next transaction as current has finished.  */
                processNewTransaction = TRUE;
            }

            /*  Update statistics.  */
            regWrites++;

            break;

        case AGP_REG_READ:

            GPU_DEBUG_BOX( printf("CommandProcessor => Procesing AGP Transaction: AGP_REG_READ.\n"); )

            /*  Process a register read request.  */
            processGPURegisterRead(lastAGPTrans->getGPURegister(), lastAGPTrans->getGPUSubRegister(),
                data);

            /*  Create an AGP transaction to return the read value.  */
            //readAGPTrans = new AGPTransaction(AGP_REG_READ, lastAGPTrans->getGPURegister(),
            //    lastAGPTrans->getSubRegister(), data);

            /*  Send the transaction to the driver.  */
            //driver->sendAGPTransaction(readAGPTrans);

            break;

        case AGP_COMMAND:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Procesing AGP Transaction: AGP_COMMAND.\n");
            )

            /*  Process GPU command.  */
            processGPUCommand(cycle, lastAGPTrans->getGPUCommand());

            break;


        case AGP_INIT_END:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Processing AGP Transaction : AGP_INIT_END.\n");
            )

            //  Wait until ready state.
            if (state.statusRegister != GPU_READY)
            {
                //  Keep the transaction until READY state.
                processNewTransaction = false;
            }
            else
            {
                //  Set the initialization phase end flag.
                initEnd = true;
            }

            break;

        case AGP_EVENT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Processign AGP Transaction : AGP_EVENT.\n");
            )

            //  Process the event.
            processGPUEvent(cycle, lastAGPTrans->getGPUEvent(), lastAGPTrans->getGPUEventMsg());

            break;

        default:
            /*  Unknown AGP Transaction command.  */

            panic("CommandProcessor", "processNewAGPTransaction", "Undefined AGP Transaction type.");
            break;
    }

}

/*  Process a GPU register write.  */
void CommandProcessor::processGPURegisterWrite(u64bit cycle, GPURegister gpuReg, u32bit gpuSubReg,
    GPURegData gpuData)
{
    StreamerCommand *streamComm;
    ShaderCommand *shComm;
    PrimitiveAssemblyCommand *paComm;
    ClipperCommand *clipperCommand;
    RasterizerCommand *rastCom;
    u32bit textUnit;
    u32bit mipmap;
    u32bit cubemap;
    u32bit i;

    /*  Write request to a GPU register:
        *  Check that the AGP transaction can be processed.
        *  Write value to GPU register.
        *  Issue the state change to other GPU units.  */

    switch (gpuReg)
    {
        /*  GPU state registers.  */
        case GPU_STATUS:
            /*  Read Only register.  */
            panic("CommandProcessor", "processGPURegisterWrite", "Error writing to GPU status register not allowed.");
            break;

        /*  GPU display registers.  */
        case GPU_DISPLAY_X_RES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DISPLAY_X_RES.\n");
            )

            /*  Check x resolution range.  */
            GPU_ASSERT(
                if (gpuData.uintVal > MAX_DISPLAY_RES_X)
                    panic("CommandProcessor", "processGPURegisterWrite", "Horizontal display resolution not supported.");
            )

            /*  Set GPU display x resolution register.  */
            state.displayResX = gpuData.uintVal;


            /*  Send state to Rasterizer.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_DISPLAY_Y_RES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DISPLAY_Y_RES.\n");
            )

            /*  Check y resolution range.  */
            GPU_ASSERT(
                if (gpuData.uintVal > MAX_DISPLAY_RES_Y)
                    panic("CommandProcessor", "processGPURegisterWrite", "Vertical display resolution not supported.");
            )

            /*  Set GPU display y resolution register.  */
            state.displayResY = gpuData.uintVal;


            /*  Send state to Rasterizer.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_D3D9_PIXEL_COORDINATES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_D3D9_PIXEL_COORDINATES.\n");
            )

            //  Set GPU use D3D9 pixel coordinates convention register.
            state.d3d9PixelCoordinates = gpuData.booleanVal;


            //  Send state to the Rasterizer.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to rasterizer.
            rastCommSignal->write(cycle, rastCom);


            //  Send state to DAC.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);


            break;

        case GPU_VIEWPORT_INI_X:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VIEWPORT_INI_X.\n");
            )

            /*  Check viewport x range.  */
            GPU_ASSERT(
                if((gpuData.intVal < MIN_VIEWPORT_X) || (gpuData.intVal > MAX_VIEWPORT_X))
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range viewport initial x position.");
            )

            /*  Set GPU viewport initial x register.  */
            state.viewportIniX = gpuData.intVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_VIEWPORT_INI_Y:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VIEWPORT_INI_Y.\n");
            )

            /*  Check viewport y range.  */
            GPU_ASSERT(
                if((gpuData.intVal < MIN_VIEWPORT_Y) || (gpuData.intVal > MAX_VIEWPORT_Y))
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range viewport initial y position.");
            )

            /*  Set GPU viewport initial y register.  */
            state.viewportIniY = gpuData.intVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_VIEWPORT_HEIGHT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VIEWPORT_HEIGHT.\n");
            )

            /*  Check viewport y range.  */
            GPU_ASSERT(
                if(((gpuData.intVal + state.viewportIniY) < MIN_VIEWPORT_Y) ||
                    ((gpuData.intVal + state.viewportIniY) > MAX_VIEWPORT_Y))
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range viewport final y position.");
            )

            /*  Set GPU viewport width register.  */
            state.viewportHeight = gpuData.uintVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_VIEWPORT_WIDTH:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VIEWPORT_WIDTH.\n");
            )

            /*  Check viewport x range.  */
            GPU_ASSERT(
                if(((gpuData.intVal + state.viewportIniX) < MIN_VIEWPORT_X) ||
                    ((gpuData.intVal + state.viewportIniX) > MAX_VIEWPORT_X))
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range viewport final x position.");
            )

            /*  Set GPU viewport width register.  */
            state.viewportWidth = gpuData.uintVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_DEPTH_RANGE_NEAR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTHRANGE_NEAR.\n");
            )

            /*  Check depth near range (clamped to [0, 1]).  */
            GPU_ASSERT(
                if((gpuData.f32Val < 0.0) || (gpuData.f32Val > 1.0))
                    panic("CommandProcessor", "processGPURegisterWrite", "Near depth range must be clamped to 0..1.");
            )

            /*  Set GPU near depth range register.  */
            state.nearRange = gpuData.f32Val;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_DEPTH_RANGE_FAR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTHRANGE_FAR.\n");
            )

            /*  Check depth far range (clamped to [0, 1]).  */
            GPU_ASSERT(
                if((gpuData.f32Val < 0.0) || (gpuData.f32Val > 1.0))
                    panic("CommandProcessor", "processGPURegisterWrite", "Far depth range must be clamped to 0..1.");
            )

            /*  Set GPU far depth range register.  */
            state.farRange = gpuData.f32Val;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_COLOR_BUFFER_CLEAR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_COLOR_BUFFER_CLEAR = {%f, %f, %f, %f}.\n",
                    gpuData.qfVal[0], gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);
            )

            /*  Set GPU color buffer clear value register.  */
            state.colorBufferClear[0] = gpuData.qfVal[0];
            state.colorBufferClear[1] = gpuData.qfVal[1];
            state.colorBufferClear[2] = gpuData.qfVal[2];
            state.colorBufferClear[3] = gpuData.qfVal[3];


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_INI_X:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_INI_X = %x.\n", gpuData.uintVal);
            )

            /*  Set GPU blit initial x coordinate register.  */
            state.blitIniX = gpuData.uintVal;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_INI_Y:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_INI_Y = %x.\n", gpuData.uintVal);
            )

            /*  Set GPU blit initial y coordinate register.  */
            state.blitIniY = gpuData.uintVal;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_X_OFFSET:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_X_OFFSET = %x.\n", gpuData.uintVal);
            )

            /*  Set GPU blit x offset coordinate register.  */
            state.blitXOffset = gpuData.uintVal;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_Y_OFFSET:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_Y_OFFSET = %x.\n", gpuData.uintVal);
            )

            /*  Set GPU blit x offset coordinate register.  */
            state.blitYOffset = gpuData.uintVal;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_WIDTH:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_WIDTH = %x.\n", gpuData.uintVal);
            )

            /*  Set GPU blit width register.  */
            state.blitWidth = gpuData.uintVal;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_HEIGHT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_HEIGHT = %x.\n", gpuData.uintVal);
            )

            /*  Set GPU blit height register.  */
            state.blitHeight = gpuData.uintVal;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_DST_ADDRESS:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_DST_ADDRESS = %x.\n", gpuData.uintVal);
            )

            /*  Set GPU blit GPU memory destination address register.  */
            state.blitDestinationAddress = gpuData.uintVal;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_DST_TX_WIDTH2:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_DST_TX_WIDTH2 = %x.\n", gpuData.txFormat);
            )

            /*  Set GPU blit GPU memory destination address register.  */
            state.blitTextureWidth2 = gpuData.uintVal;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_DST_TX_FORMAT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_DST_TX_FORMAT = %x.\n", gpuData.txFormat);
            )

            /*  Set GPU blit GPU memory destination address register.  */
            state.blitDestinationTextureFormat = gpuData.txFormat;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BLIT_DST_TX_BLOCK:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLIT_DST_TX_BLOCK = %x.\n", gpuData.txFormat);
            )

            /*  Set GPU blit GPU memory destination address register.  */
            state.blitDestinationTextureBlocking = gpuData.txBlocking;

            /*  Send state to DAC/Blitter.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_Z_BUFFER_CLEAR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_Z_BUFFER_CLEAR.\n");
            )

            /*  Set GPU Z buffer clear value register.  */
            state.zBufferClear = gpuData.uintVal;


            /*  Send state to Rasterizer.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_Z_BUFFER_BIT_PRECISSION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_Z_BUFFER_BIT_PRECISSION.\n");
            )

            /*  Check z buffer bit precission.  */
            GPU_ASSERT(
                if ((gpuData.uintVal != 16) && (gpuData.uintVal != 24) &&
                    (gpuData.uintVal != 32))
                    panic("CommandProcessor", "processGPURegisterWrite", "Not supported depth buffer bit precission.");
            )

            /*  Set GPU Z buffer bit precission register.  */
            state.zBufferBitPrecission = gpuData.uintVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_STENCIL_BUFFER_CLEAR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STENCIL_BUFFER_CLEAR.\n");
            )

            /*  Set GPU Z buffer clear value register.  */
            state.stencilBufferClear = gpuData.uintVal;

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        /*  GPU memory registers.  */
        case GPU_FRONTBUFFER_ADDR:

            /*  Check address range.  */

            /*  Set GPU front buffer address in GPU memory register.  */
            state.frontBufferBaseAddr = gpuData.uintVal;
            state.rtAddress[0] = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_FRONTBUFFER_ADDR = %x.\n", gpuData.uintVal);
            )


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_BACKBUFFER_ADDR:

            /*  Check address range.  */

            /*  Set GPU back buffer address in GPU memory register.  */
            state.backBufferBaseAddr = gpuData.uintVal;
            state.rtAddress[0] = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_BACKBUFFER_ADDR = %x.\n", gpuData.uintVal);
            )


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_ZSTENCILBUFFER_ADDR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_ZSTENCILBUFFER_ADDR.\n");
            )

            /*  Check address range.  */

            /*  Set GPU Z/Stencil buffer address in GPU memory register.  */
            state.zStencilBufferBaseAddr = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_ZSTENCILBUFFER_ADDR = %x.\n", gpuData.uintVal);
            )

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }


            //  Send state to DAC.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_COLOR_STATE_BUFFER_MEM_ADDR:

            /*  Check address range.  */

            /*  Set GPU color block state buffer memory address register.  */
            state.colorStateBufferAddr = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_COLOR_STATE_BUFFER_MEM_ADDR = %x.\n", gpuData.uintVal);
            )


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR.\n");
            )

            /*  Check address range.  */

            /*  Set GPU Z/Stencil block state buffer address in GPU memory register.  */
            state.zstencilStateBufferAddr = gpuData.uintVal;

            GPU_DEBUG_BOX( printf("CommandProcessor => GPU_ZSTENCIL_STATE_BUFFER_MEM_ADDR = %x.\n", gpuData.uintVal); )

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_TEXTURE_MEM_ADDR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MEM_ADDR.\n");
            )

            /*  Check address range.  */

            /*  Set GPU texture memory base address in GPU memory register.  */
            state.textureMemoryBaseAddr = gpuData.uintVal;

            GPU_DEBUG_BOX( printf("CommandProcessor => GPU_TEXTURE_MEM_ADDR = %x.\n", gpuData.uintVal); )

            /*  Send state change to texture unit.  */

            break;

        case GPU_PROGRAM_MEM_ADDR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_PROGRAM_MEM_ADDR.\n");
            )

            /*  Check address range.  */

            /*  Set GPU program memory base address in GPU memory register.  */
            state.programMemoryBaseAddr = gpuData.uintVal;

            GPU_DEBUG_BOX( printf("CommandProcessor => GPU_FRONTBUFFER_ADDR = %x.\n", gpuData.uintVal); )

            /*  Send state change to vertex shader unit.  */

            break;

        /*  GPU vertex shader.  */
        case GPU_VERTEX_PROGRAM:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VERTEX_PROGRAM.\n");
            )

            /*  Check address range.  */

            /*  Set vertex program address register.  */
            state.vertexProgramAddr = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_VERTEX_PROGRAM = %x.\n", gpuData.uintVal);
            )

            break;

        case GPU_VERTEX_PROGRAM_PC:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VERTEX_PROGRAM_PC.\n");
            )

            /*  Check vertex program PC range.  */

            /*  Set vertex program start PC register.  */
            state.vertexProgramStartPC = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_VERTEX_PROGRAM_PC = %x.\n", gpuData.uintVal);
            )

            /*  Send state change to all vertex shader units.  */
            for (i = 0; i < numVShaders; i++)
            {
                /*  Create shader command to change the initial PC.  */
                shComm = new ShaderCommand(SET_INIT_PC, VERTEX_TARGET, state.vertexProgramStartPC);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                vshFCommSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_VERTEX_PROGRAM_SIZE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VERTEX_PROGRAM_SIZE.\n");
            )

            /*  Set vertex program to load size (instructions).  */
            state.vertexProgramSize = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_VERTEX_PROGRAM_SIZE = %x.\n", gpuData.uintVal);
            )

            break;

        case GPU_VERTEX_THREAD_RESOURCES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VERTEX_THREAD_RESOURCES.\n");
            )

            /*  Set vertex program to load size (instructions).  */
            state.vertexThreadResources = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_THREAD_RESOURCES = %x.\n", gpuData.uintVal);
            )

            /*  Send state change to all vertex shader units.  */
            for (i = 0; i < numVShaders; i++)
            {
                /*  Create shader command to change the initial PC.  */
                shComm = new ShaderCommand(SET_THREAD_RES, VERTEX_TARGET, state.vertexThreadResources);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                vshFCommSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_VERTEX_CONSTANT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VERTEX_CONSTANT.\n");
            )

            /*  Check constant range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_CONSTANTS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range vertex constant register.");
            )

            /*  Set vertex constant register.  */
            state.vConstants[gpuSubReg].setComponents(gpuData.qfVal[0],
                gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);


            /*  Send state change to all the vertex shader units.  */
            for (i = 0; i < numVShaders; i++)
            {
                /*  Create shader command to store a vertex constant value.  */
                shComm = new ShaderCommand(&state.vConstants[gpuSubReg], gpuSubReg, 1);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to each vertex shader unit.  */
                vshFCommSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_VERTEX_OUTPUT_ATTRIBUTE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VERTEX_OUTPUT_ATTRIBUTE[%d] = %s.\n",
                    gpuSubReg, gpuData.booleanVal?"ENABLED":"DISABLED");
            )

            /*  Check vertex attribute identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range vertex attribute identifier.");
            )

            /*  Set GPU vertex attribute mapping register.  */
            state.outputAttribute[gpuSubReg] = gpuData.booleanVal;

            /*  Send state change to all vertex shader units.  */
            for (i = 0; i < numVShaders; i++)
            {
                /*  Create shader command to setup the vertex program output attributes.  */
                shComm = new ShaderCommand(SET_OUT_ATTR, gpuSubReg, gpuData.booleanVal);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                vshFCommSignal[i]->write(cycle, shComm, 1);
            }

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and add new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            /*  Create a primitive assembly command.  */
            paComm = new PrimitiveAssemblyCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and add new cookie.  */
            paComm->copyParentCookies(*lastAGPTrans);
            paComm->addCookie();

            /*  Send state change to the primitive assembly unit.  */
            paCommSignal->write(cycle, paComm);

            break;



        /*  GPU vertex stream buffer registers.  */
        case GPU_VERTEX_ATTRIBUTE_MAP:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VERTEX_ATTRIBUTE_MAP.\n");
            )

            /*  Check vertex attribute identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range vertex attribute identifier.");
            )

            /*  Set GPU vertex attribute mapping register.  */
            state.attributeMap[gpuSubReg] = gpuData.uintVal;

            /*  Send state change to all vertex shader units.  */
            for (i = 0; i < numVShaders; i++)
            {
                /*  Create shader command to setup the vertex program input attributes.  */
                shComm = new ShaderCommand(SET_IN_ATTR, gpuSubReg, (gpuData.uintVal != ST_INACTIVE_ATTRIBUTE));

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                vshFCommSignal[i]->write(cycle, shComm, 1);
            }

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_VERTEX_ATTRIBUTE_DEFAULT_VALUE.\n");
            )

            /*  Check vertex attribute identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_VERTEX_ATTRIBUTES)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range vertex attribute identifier.");
            )

            /*  Set GPU vertex attribute default value.  */
            state.attrDefValue[gpuSubReg][0] = gpuData.qfVal[0];
            state.attrDefValue[gpuSubReg][1] = gpuData.qfVal[1];
            state.attrDefValue[gpuSubReg][2] = gpuData.qfVal[2];
            state.attrDefValue[gpuSubReg][3] = gpuData.qfVal[3];

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_STREAM_ADDRESS:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STREAM_ADDRESS.\n");
            )

            /*  Check stream identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range stream buffer identifier.");
            )

            /*  Set GPU stream buffer address register.  */
            state.streamAddress[gpuSubReg] = gpuData.uintVal;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_STREAM_STRIDE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STREAM_STRIDE.\n");
            )

            /*  Check stream identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range stream buffer identifier.");
            )

            /*  Set GPU stream buffer stride register.  */
            state.streamStride[gpuSubReg] = gpuData.uintVal;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_STREAM_DATA:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STREAM_DATA.\n");
            )

            /*  Check stream identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range stream buffer identifier.");
            )

            /*  Set GPU stream buffer data type register.  */
            state.streamData[gpuSubReg] = gpuData.streamData;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_STREAM_ELEMENTS:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STREAM_ELEMENTS.\n");
            )

            /*  Check stream identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range stream buffer identifier.");
            )

            /*  Set GPU stream buffer number of elements per entry register.  */
            state.streamElements[gpuSubReg] = gpuData.uintVal;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_STREAM_FREQUENCY:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STREAM_FREQUENCY.\n");
            )

            //  Check stream identifier range.
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range stream buffer identifier.");
            )

            //  Set GPU stream buffer read frequency.
            state.streamFrequency[gpuSubReg] = gpuData.uintVal;

            //  Create streamer command.
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            //  Send state change to the streamer unit.
            streamCtrlSignal->write(cycle, streamComm);

            /*//  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }*/

            break;

        case GPU_STREAM_START:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STREAM_START.\n");
            )

            /*  Set GPU stream start position register.  */
            state.streamStart = gpuData.uintVal;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            break;

        case GPU_STREAM_COUNT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STREAM_COUNT.\n");
            )

            /*  Set GPU stream count register.  */
            state.streamCount = gpuData.uintVal;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);


            /*  Create a primitive assembly command.  */
            paComm = new PrimitiveAssemblyCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and add new cookie.  */
            paComm->copyParentCookies(*lastAGPTrans);
            paComm->addCookie();

            /*  Send state change to the primitive assembly unit.  */
            paCommSignal->write(cycle, paComm);

            break;

        case GPU_STREAM_INSTANCES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STREAM_INSTANCES.\n");
            )

            //  Set GPU stream instances register.
            state.streamInstances = gpuData.uintVal;

            //  Create streamer command.
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            //  Send state change to the streamer unit.
            streamCtrlSignal->write(cycle, streamComm);


            //  Create a primitive assembly command.
            paComm = new PrimitiveAssemblyCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and add new cookie.
            paComm->copyParentCookies(*lastAGPTrans);
            paComm->addCookie();

            //  Send state change to the primitive assembly unit.
            paCommSignal->write(cycle, paComm);

            break;

        case GPU_INDEX_MODE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_INDEX_MODE.\n");
            )

            /*  Set GPU indexed batching mode register.  */
            state.indexedMode = gpuData.booleanVal;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            break;

        case GPU_INDEX_STREAM:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_INDEX_STREAM.\n");
            )

            /*  Check stream identifier range.  */
            GPU_ASSERT(
                if (gpuData.uintVal >= MAX_STREAM_BUFFERS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range stream buffer identifier.");
            )

            /*  Set GPU stream buffer for index buffer register.  */
            state.indexStream = gpuData.uintVal;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            break;

        case GPU_D3D9_COLOR_STREAM:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_D3D9_COLOR_STREAM.\n");
            )

            /*  Check stream identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_STREAM_BUFFERS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range stream buffer identifier.");
            )

            /*  Set GPU D3D9 color stream register.  */
            state.d3d9ColorStream[gpuSubReg] = gpuData.booleanVal;

            /*  Create streamer command.  */
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send state change to the streamer unit.  */
            streamCtrlSignal->write(cycle, streamComm);

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_ATTRIBUTE_LOAD_BYPASS:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_ATTRIBUTE_LOAD_BYPASS.\n");
            )

            //  Set attribute load bypass register.
            state.attributeLoadBypass = gpuData.booleanVal;

            //  Create streamer command.
            streamComm = new StreamerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            //  Send state change to the streamer unit.
            streamCtrlSignal->write(cycle, streamComm);

            break;

        /*  GPU primitive assembly registers.  */

        case GPU_PRIMITIVE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_PRIMITIVE.\n");
            )

            /*  Set GPU primitive mode register.  */
            state.primitiveMode = gpuData.primitive;

            /*  Create a primitive assembly command.  */
            paComm = new PrimitiveAssemblyCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and add new cookie.  */
            paComm->copyParentCookies(*lastAGPTrans);
            paComm->addCookie();

            /*  Send state change to the primitive assembly unit.  */
            paCommSignal->write(cycle, paComm);

            break;


        /*  GPU clipping and culling registers.  */
        case GPU_FRUSTUM_CLIPPING:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_FRUSTUM_CLIPPING.\n");
            )

            /*  Set GPU frustum clipping flag register.  */
            state.frustumClipping = gpuData.booleanVal;

            /*  Create a Clipper command.  */
            clipperCommand = new ClipperCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and add a new
                cookie.  */
            clipperCommand->copyParentCookies(*lastAGPTrans);
            clipperCommand->addCookie();

            /*  Send state change to the Clipper.  */
            clipCommSignal->write(cycle, clipperCommand);


            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_USER_CLIP:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_USER_CLIP.\n");
            )

            /*  Check user clip plane identifier range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_USER_CLIP_PLANES)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range user clip plane identifier.");
            )

            /*  Set GPU user clip plane.  */
            state.userClip[gpuSubReg].setComponents(gpuData.qfVal[0],
                gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);

            /*  Create a Clipper command.  */
            clipperCommand = new ClipperCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and add a new
                cookie.  */
            clipperCommand->copyParentCookies(*lastAGPTrans);
            clipperCommand->addCookie();

            /*  Send state change to the Clipper.  */
            clipCommSignal->write(cycle, clipperCommand);


            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_USER_CLIP_PLANE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_USER_CLIP_PLANE.\n");
            )

            /*  Set GPU user clip planes enable register.  */
            state.userClipPlanes = gpuData.booleanVal;

            /*  Create a Clipper command.  */
            clipperCommand = new ClipperCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and add a new
                cookie.  */
            clipperCommand->copyParentCookies(*lastAGPTrans);
            clipperCommand->addCookie();

            /*  Send state change to the Clipper.  */
            clipCommSignal->write(cycle, clipperCommand);


            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_FACEMODE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_FACEMODE.\n");
            )

            /*  Set GPU face mode (vertex order for front facing triangles) register.  */
            state.faceMode = gpuData.faceMode;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_CULLING:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_CULLING.\n");
            )

            /*  Set GPU culling mode (none, front, back, front/back) register.  */
            state.cullMode = gpuData.culling;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        /*  Hierarchical Z registers.  */
        case GPU_HIERARCHICALZ:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_HIERARCHICALZ.\n");
            )

            /*  Set GPU hierarchical Z enable flag register.  */
            state.hzTest = gpuData.booleanVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        /*  Hierarchical Z registers.  */
        case GPU_EARLYZ:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_EARLYZ.\n");
            )

            /*  Set GPU early Z test enable flag register.  */
            state.earlyZ = gpuData.booleanVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            /*  Send register write to all the Z Stencil units.  */
            //for(i = 0; i < numStampUnits; i++)
            //{
                /*  Create rasterizer command.  */
            //    rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
            //    rastCom->copyParentCookies(*lastAGPTrans);
            //    rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
            //    zStencilCommSignal[i]->write(cycle, rastCom);

                /*  Create rasterizer command.  */
            //    rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
            //    rastCom->copyParentCookies(*lastAGPTrans);
            //    rastCom->addCookie();

                /*  Send state change to Color Write.  */
                //colorWriteCommSignal[i]->write(cycle, rastCom);
            //}

            break;

        /*  GPU rasterization registers.  */

        case GPU_SCISSOR_TEST:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_SCISSOR_TEST.\n");
            )

            state.scissorTest = gpuData.booleanVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_SCISSOR_INI_X:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_SCISSOR_INI_X.\n");
            )

            /*  Set GPU scissor initial x register.  */
            state.scissorIniX = gpuData.intVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_SCISSOR_INI_Y:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_SCISSOR_INI_Y.\n");
            )

            /*  Set GPU scissor initial y register.  */
            state.scissorIniY = gpuData.intVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_SCISSOR_WIDTH:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_SCISSOR_WIDTH.\n");
            )

            /*  Set GPU scissor width register.  */
            state.scissorWidth = gpuData.uintVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_SCISSOR_HEIGHT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_SCISSOR_HEIGHT.\n");
            )

            /*  Set GPU scissor height register.  */
            state.scissorHeight = gpuData.uintVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_DEPTH_SLOPE_FACTOR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTH_SLOPE_FACTOR.\n");
            )

            /*  Set GPU depth slope factor.  */
            state.slopeFactor = gpuData.f32Val;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_DEPTH_UNIT_OFFSET:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTH_UNIT_OFFSET.\n");
            )

            /*  Set GPU depth unit offset.  */
            state.unitOffset = gpuData.f32Val;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_D3D9_DEPTH_RANGE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_D3D9_DEPTH_RANGE = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU use d3d9 depth range at clip space register.
            state.d3d9DepthRange = gpuData.booleanVal;

            //  Create a Clipper command.
            clipperCommand = new ClipperCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and add a new cookie.
            clipperCommand->copyParentCookies(*lastAGPTrans);
            clipperCommand->addCookie();

            //  Send state change to the Clipper.
            clipCommSignal->write(cycle, clipperCommand);


            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to rasterizer.
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_D3D9_RASTERIZATION_RULES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_D3D9_RASTERIZATION_RULES = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set GPU use d3d9 rasterization rules register.
            state.d3d9RasterizationRules = gpuData.booleanVal;

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to rasterizer.
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_TWOSIDED_LIGHTING:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TWOSIDED_LIGHTING.\n");
            )

            /*  Set GPU two sided lighting color selection register.  */
            state.twoSidedLighting = gpuData.booleanVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_MULTISAMPLING:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_MULTISAMPLING.\n");
            )

            /*  Set multisampling (MSAA) enabling register.  */
            state.multiSampling = gpuData.booleanVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send register write to all fragment shader units.  */
            for (i = 0; i < numFShaders; i++)
            {
                /*  Create shader command to setup the shader input attributes as active.  */
                shComm = new ShaderCommand(gpuData.booleanVal);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                fshFCommandSignal[i]->write(cycle, shComm, 1);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_MSAA_SAMPLES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_MSAA_SAMPLES.\n");
            )

            /*  Set GPU number of z samples to generate per fragment for MSAA register.  */
            state.msaaSamples = gpuData.uintVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            /*  Send register write to all fragment shader units.  */
            for (i = 0; i < numFShaders; i++)
            {
                /*  Create shader command to setup the shader input attributes as active.  */
                shComm = new ShaderCommand(gpuData.uintVal);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                fshFCommandSignal[i]->write(cycle, shComm, 1);
            }

            /*  Send state to DAC.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to DAC.  */
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_INTERPOLATION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_INTERPOLATION.\n");
            )

            /*  Set GPU fragments attribute interpolation mode (GL flat/smoth) register.  */
            state.interpolation[gpuSubReg] = gpuData.booleanVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            break;

        case GPU_FRAGMENT_INPUT_ATTRIBUTES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_FRAGMENT_INPUT_ATTRIBUTES[%d] = %s.\n",
                gpuSubReg, gpuData.booleanVal?"ENABLED":"DISABLED");
            )

            /*  Check fragment attribute range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_FRAGMENT_ATTRIBUTES)
                    panic("CommandProcessor", "processGPURegisterWrite", "Fragment attribute identifier out of range.");
            )

            /*  Set fragment input attribute active flag GPU register.  */
            state.fragmentInputAttributes[gpuSubReg] = gpuData.booleanVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);

            /*  Send state change to all fragment shader units.  */
            for (i = 0; i < numFShaders; i++)
            {
                /*  Create shader command to setup the shader input attributes as active.  */
                shComm = new ShaderCommand(SET_IN_ATTR, gpuSubReg, gpuData.booleanVal);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                fshFCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        /*  GPU fragment registers.  */
        case GPU_FRAGMENT_PROGRAM:

            /*  Check address range.  */

            /*  Set fragment program address register.  */
            state.fragProgramAddr = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_FRAGMENT_PROGRAM = %x.\n", gpuData.uintVal);
            )

            break;

        case GPU_FRAGMENT_PROGRAM_PC:

            /*  Check fragment program PC range.  */

            /*  Set fragment program start PC register.  */
            state.fragProgramStartPC = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_FRAGMENT_PROGRAM_PC = %x.\n", gpuData.uintVal);
            )

            /*  Send state change to all fragment shader units.  */
            for (i = 0; i < numFShaders; i++)
            {
                /*  Create shader command to change the initial PC.  */
                shComm = new ShaderCommand(SET_INIT_PC, FRAGMENT_TARGET, state.fragProgramStartPC);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                fshFCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_FRAGMENT_PROGRAM_SIZE:

            /*  Check fragment program size.  */

            /*  Set fragment program to load size (instructions).  */
            state.fragProgramSize = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_FRAGMENT_PROGRAM_SIZE = %x.\n", gpuData.uintVal);
            )

            break;

        case GPU_FRAGMENT_THREAD_RESOURCES:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_FRAGMENT_THREAD_RESOURCES.\n");
            )

            /*  Set per fragment thread resource usage.  */
            state.fragThreadResources = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_FRAGMENT_THREAD_RESOURCE = %x.\n", gpuData.uintVal);
            )

            /*  Send state change to all fragment shader units.  */
            for (i = 0; i < numFShaders; i++)
            {
                /*  Create shader command to change the initial PC.  */
                shComm = new ShaderCommand(SET_THREAD_RES, FRAGMENT_TARGET, state.fragThreadResources);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the command to each shader.  */
                fshFCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_FRAGMENT_CONSTANT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_FRAGMENT_CONSTANT.\n");
            )

            /*  Check constant range.  */
            GPU_ASSERT(
                if (gpuSubReg >= MAX_FRAGMENT_CONSTANTS)
                    panic("CommandProcessor", "processGPURegisterWrite", "Out of range fragment constant register.");
            )

            /*  Set fragment constant register.  */
            state.fConstants[gpuSubReg].setComponents(gpuData.qfVal[0],
                gpuData.qfVal[1], gpuData.qfVal[2], gpuData.qfVal[3]);


            /*  Send state change to all the fragment shader units.  */
            for (i = 0; i < numFShaders; i++)
            {
                /*  Create shader command to store a fragment constant value.  */
                shComm = new ShaderCommand(&state.fConstants[gpuSubReg], gpuSubReg, 1);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to each fragment shader unit.  */
                fshFCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_MODIFY_FRAGMENT_DEPTH:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_MODIFY_FRAGMENT_DEPTH.\n");
            )

            /*  Set fragment constant register.  */
            state.modifyDepth = gpuData.booleanVal;

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();


            /*  Send state change to Hierarchical Z.  */
            rastCommSignal->write(cycle, rastCom);

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        //  GPU Shader Program registers.
        case GPU_SHADER_PROGRAM_ADDRESS:

            //  Check address range.

            //  Set shader program address register.  */
            state.programAddress = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_SHADER_PROGRAM_ADDDRESS = %x.\n", gpuData.uintVal);
            )

            break;

        case GPU_SHADER_PROGRAM_SIZE:

            //  Check shader program size.

            //  Set shader program size (bytes).
            state.programSize = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_SHADER_PROGRAM_SIZE = %x.\n", gpuData.uintVal);
            )

            break;

        case GPU_SHADER_PROGRAM_LOAD_PC:

            //  Check fragment program PC range.

            //  Set shader program load PC register.
            state.programLoadPC = gpuData.uintVal;

            GPU_DEBUG_BOX(
                printf("CommandProcessor => GPU_SHADER_PROGRAM_LOAD_PC = %x.\n", gpuData.uintVal);
            )

            break;


        case GPU_SHADER_PROGRAM_PC:

            {

                //  Check fragment program PC range.

                //  Set shader program PC register for the corresponding shader target.
                state.programStartPC[gpuSubReg] = gpuData.uintVal;

                //  Get shader target for which the per-thread resource register is changed.
                ShaderTarget shTarget;
                switch(gpuSubReg)
                {
                    case VERTEX_TARGET:
                        shTarget = VERTEX_TARGET;
                        break;
                    case FRAGMENT_TARGET:
                        shTarget = FRAGMENT_TARGET;
                        break;
                    case TRIANGLE_TARGET:
                        shTarget = TRIANGLE_TARGET;
                        break;
                    case MICROTRIANGLE_TARGET:
                        shTarget = MICROTRIANGLE_TARGET;
                        break;
                    default:
                        panic("CommandProcessor", "processGPURegisterWrite", "Undefined shader target.");
                        break;
                }

                GPU_DEBUG_BOX(
                    printf("CommandProcessor => GPU_SHADER_PROGRAM_LOAD_PC[");

                    switch(gpuSubReg)
                    {
                        case VERTEX_TARGET:
                            printf("VERTEX_TARGET");
                            break;
                        case FRAGMENT_TARGET:
                            printf("FRAGMENT_TARGET");
                            break;
                        case TRIANGLE_TARGET:
                            printf("TRIANGLE_TARGET");
                            break;
                        case MICROTRIANGLE_TARGET:
                            printf("MICROTRIANGLE_TARGET");
                            break;
                        default:
                            panic("CommandProcessor", "processGPURegisterWrite", "Undefined shader target.");
                            break;
                    }

                    printf("] = %x.\n", gpuData.uintVal);
                )

                //  Send state change to all fragment shader units.
                for (i = 0; i < numFShaders; i++)
                {
                    //  Create shader command to change the initial PC.
                    shComm = new ShaderCommand(SET_INIT_PC, shTarget, gpuData.uintVal);

                    //  Copy cookies from original AGP transaction and a new cookie.
                    shComm->copyParentCookies(*lastAGPTrans);
                    shComm->addCookie();

                    //  Send the command to each shader.
                    fshFCommandSignal[i]->write(cycle, shComm, 1);
                }

            }

            break;

        case GPU_SHADER_THREAD_RESOURCES:

            {
                //  Set per fragment thread resource usage register for the corresponding shader target.
                state.programResources[gpuSubReg] = gpuData.uintVal;

                //  Get shader target for which the per-thread resource register is changed.
                ShaderTarget shTarget;
                switch(gpuSubReg)
                {
                    case VERTEX_TARGET:
                        shTarget = VERTEX_TARGET;
                        break;
                    case FRAGMENT_TARGET:
                        shTarget = FRAGMENT_TARGET;
                        break;
                    case TRIANGLE_TARGET:
                        shTarget = TRIANGLE_TARGET;
                        break;
                    case MICROTRIANGLE_TARGET:
                        shTarget = MICROTRIANGLE_TARGET;
                        break;
                    default:
                        panic("CommandProcessor", "processGPURegisterWrite", "Undefined shader target.");
                        break;
                }

                GPU_DEBUG_BOX(
                    printf("CommandProcessor => GPU_SHADER_THREAD_RESOURCES[");

                    switch(gpuSubReg)
                    {
                        case VERTEX_TARGET:
                            printf("VERTEX_TARGET");
                            break;
                        case FRAGMENT_TARGET:
                            printf("FRAGMENT_TARGET");
                            break;
                        case TRIANGLE_TARGET:
                            printf("TRIANGLE_TARGET");
                            break;
                        case MICROTRIANGLE_TARGET:
                            printf("MICROTRIANGLE_TARGET");
                            break;
                        default:
                            panic("CommandProcessor", "processGPURegisterWrite", "Undefined shader target.");
                            break;
                    }

                    printf("] = %x.\n", gpuData.uintVal);
                )

                //  Send state change to all fragment shader units.
                for (i = 0; i < numFShaders; i++)
                {
                    //  Create shader command to change the initial PC.
                    shComm = new ShaderCommand(SET_THREAD_RES, shTarget, gpuData.uintVal);

                    //  Copy cookies from original AGP transaction and a new cookie.
                    shComm->copyParentCookies(*lastAGPTrans);
                    shComm->addCookie();

                    //  Send the command to each shader.
                    fshFCommandSignal[i]->write(cycle, shComm, 1);
                }
            }

            break;

        /*  GPU Texture Unit registers.  */
        case GPU_TEXTURE_ENABLE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_ENABLE.\n");
            )

            /*  Write texture enable register.  */
            state.textureEnabled[gpuSubReg] = gpuData.booleanVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_MODE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MODE.\n");
            )

            /*  Write texture mode register.  */
            state.textureMode[gpuSubReg] = gpuData.txMode;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_ADDRESS:

            /*  WARNING:  As the current simulator only support an index per register we must
                decode the texture unit, mipmap and cubemap image.  To do so the cubemap images of
                a mipmap are stored sequentally and then the mipmaps for a texture unit
                are stored sequentially and then the mipmap addresses of the other texture units
                are stored sequentially.  */

            /*  Calculate texture unit and mipmap level.  */
            textUnit = gpuSubReg / (MAX_TEXTURE_SIZE * CUBEMAP_IMAGES);
            mipmap = GPU_MOD(gpuSubReg / CUBEMAP_IMAGES, MAX_TEXTURE_SIZE);
            cubemap = GPU_MOD(gpuSubReg, CUBEMAP_IMAGES);

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_ADDRESS.\n");
            )

            /*  Write texture address register (per cubemap image, mipmap and texture unit).  */
            state.textureAddress[textUnit][mipmap][cubemap] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_WIDTH:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_WIDTH.\n");
            )

            /*  Write texture width (first mipmap).  */
            state.textureWidth[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_HEIGHT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_HEIGHT.\n");
            )

            /*  Write texture height (first mipmap).  */
            state.textureHeight[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_DEPTH:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_DEPTH.\n");
            )

            /*  Write texture width (first mipmap).  */
            state.textureDepth[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_WIDTH2:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_WIDTH2.\n");
            )

            /*  Write texture width (log of 2 of the first mipmap).  */
            state.textureWidth2[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_HEIGHT2:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_HEIGHT2.\n");
            )

            /*  Write texture height (log 2 of the first mipmap).  */
            state.textureHeight2[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_DEPTH2:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_DEPTH2.\n");
            )

            /*  Write texture depth (log of 2 of the first mipmap).  */
            state.textureDepth2[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_BORDER:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_BORDER.\n");
            )

            /*  Write texture border register.  */
            state.textureBorder[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;


        case GPU_TEXTURE_FORMAT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_FORMAT.\n");
            )

            /*  Write texture format register.  */
            state.textureFormat[gpuSubReg] = gpuData.txFormat;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_REVERSE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_REVERSE.\n");
            )

            /*  Write texture reverse register.  */
            state.textureReverse[gpuSubReg] = gpuData.booleanVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_D3D9_COLOR_CONV:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_D3D9_COLOR_CONV.\n");
            )

            /*  Write texture D3D9 color order conversion register.  */
            state.textD3D9ColorConv[gpuSubReg] = gpuData.booleanVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_D3D9_V_INV:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_D3D9_V_INV.\n");
            )

            /*  Write texture D3D9 v coordinate inversion register.  */
            state.textD3D9VInvert[gpuSubReg] = gpuData.booleanVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_COMPRESSION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_COMPRESSION.\n");
            )

            /*  Write texture compression register.  */
            state.textureCompr[gpuSubReg] = gpuData.txCompression;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_BLOCKING:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_BLOCKING.\n");
            )

            //  Write texture blocking mode register.
            state.textureBlocking[gpuSubReg] = gpuData.txBlocking;

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_BORDER_COLOR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_COLOR.\n");
            )

            /*  Write texture border color register.  */
            state.textBorderColor[gpuSubReg][0] = gpuData.qfVal[0];
            state.textBorderColor[gpuSubReg][1] = gpuData.qfVal[1];
            state.textBorderColor[gpuSubReg][2] = gpuData.qfVal[2];
            state.textBorderColor[gpuSubReg][3] = gpuData.qfVal[3];

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_WRAP_S:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_WRAP_S.\n");
            )

            /*  Write texture wrap in s dimension register.  */
            state.textureWrapS[gpuSubReg] = gpuData.txClamp;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_WRAP_T:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_WRAP_T.\n");
            )

            /*  Write texture wrap in t dimension register.  */
            state.textureWrapT[gpuSubReg] = gpuData.txClamp;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_WRAP_R:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_WRAP_R.\n");
            )

            /*  Write texture wrap in r dimension register.  */
            state.textureWrapR[gpuSubReg] = gpuData.txClamp;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_NON_NORMALIZED:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_NON_NORMALIZED.\n");
            )

            //  Write texture non-normalized coordinates register.
            state.textureNonNormalized[gpuSubReg] = gpuData.booleanVal;

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;


        case GPU_TEXTURE_MIN_FILTER:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MIN_FILTER.\n");
            )

            /*  Write texture minification filter register.  */
            state.textureMinFilter[gpuSubReg] = gpuData.txFilter;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_MAG_FILTER:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MAG_FILTER.\n");
            )

            /*  Write texture magnification filter register.  */
            state.textureMagFilter[gpuSubReg] = gpuData.txFilter;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_ENABLE_COMPARISON:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_ENABLE_COMPARISON = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Write texture enable comparison (PCF) register.
            state.textureEnableComparison[gpuSubReg] = gpuData.booleanVal;

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_COMPARISON_FUNCTION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_COMPARISON_FUNCTION = ");
                switch(gpuData.compare)
                {
                    case GPU_NEVER:
                        printf("NEVER.\n");
                        break;

                    case GPU_ALWAYS:
                        printf("ALWAYS.\n");
                        break;

                    case GPU_LESS:
                        printf("LESS.\n");
                        break;

                    case GPU_LEQUAL:
                        printf("LEQUAL.\n");
                        break;

                    case GPU_EQUAL:
                        printf("EQUAL.\n");
                        break;

                    case GPU_GEQUAL:
                        printf("GEQUAL.\n");
                        break;

                    case GPU_GREATER:
                        printf("GREATER.\n");
                        break;

                    case GPU_NOTEQUAL:
                        printf("NOTEQUAL.\n");

                    default:
                        panic("CommandProcessor", "processGPURegisterWrite", "Undefined compare function mode");
                        break;
                }
            )

            //  Write texture comparison function (PCF) register.
            state.textureComparisonFunction[gpuSubReg] = gpuData.compare;

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_SRGB:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_SRGB = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Write texture SRGB space to linear space conversion register.
            state.textureSRGB[gpuSubReg] = gpuData.booleanVal;

            //  Send register write to all Texture Units.
            for(i = 0; i < numTextureUnits; i++)
            {
                //  Create shader command to write a register.
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the shader command to a texture unit.
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;


        case GPU_TEXTURE_MIN_LOD:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MIN_LOD.\n");
            )

            /*  Write texture minimum lod register.  */
            state.textureMinLOD[gpuSubReg] = gpuData.f32Val;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_MAX_LOD:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MAX_LOD.\n");
            )

            /*  Write texture maximum lod register.  */
            state.textureMaxLOD[gpuSubReg] = gpuData.f32Val;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_LOD_BIAS:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_LOD_BIAS.\n");
            )

            /*  Write texture lod bias register.  */
            state.textureLODBias[gpuSubReg] = gpuData.f32Val;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_MIN_LEVEL:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MIN_LEVEL.\n");
            )

            /*  Write texture minimum mipmap level register.  */
            state.textureMinLevel[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_MAX_LEVEL:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MAX_LEVEL.\n");
            )

            /*  Write texture maximum mipmap level register.  */
            state.textureMaxLevel[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;


        case GPU_TEXT_UNIT_LOD_BIAS:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXT_UNIT_LOD_BIAS.\n");
            )

            /*  Write texture unit lod bias register.  */
            state.textureUnitLODBias[gpuSubReg] = gpuData.f32Val;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        case GPU_TEXTURE_MAX_ANISOTROPY:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_TEXTURE_MAX_ANISOTROPY.\n");
            )

            /*  Write texture unit max anisotropy register.  */
            state.maxAnisotropy[gpuSubReg] = gpuData.uintVal;

            /*  Send register write to all Texture Units.  */
            for(i = 0; i < numTextureUnits; i++)
            {
                /*  Create shader command to write a register.  */
                shComm = new ShaderCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to a texture unit.  */
                tuCommandSignal[i]->write(cycle, shComm, 1);
            }

            break;

        /*  GPU Stencil test registers.  */
        case GPU_STENCIL_TEST:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STENCIL_TEST.\n");
            )

            /*  Set stencil test enable flag.  */
            state.stencilTest = gpuData.booleanVal;

            /*  Send state to the Rasterizer/Fragment FIFO.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }


            break;

        case GPU_STENCIL_FUNCTION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STENCIL_FUNCTION.\n");
            )

            /*  Set stencil test compare function.  */
            state.stencilFunction = gpuData.compare;

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_STENCIL_REFERENCE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STENCIL_REFERENCE.\n");
            )

            /*  Set stencil test reference value.  */
            state.stencilReference = u8bit(gpuData.uintVal);

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }


            break;

        case GPU_STENCIL_COMPARE_MASK:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STENCIL_COMPARE_MASK.\n");
            )

            /*  Set stencil test compare mask.  */
            state.stencilTestMask = u8bit(gpuData.uintVal);

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }


            break;

        case GPU_STENCIL_UPDATE_MASK:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_UPDATE_MASK.\n");
            )

            /*  Set stencil test update mask.  */
            state.stencilUpdateMask = u8bit(gpuData.uintVal);

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_STENCIL_FAIL_UPDATE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_STENCIL_FAIL_UPDATE.\n");
            )

            /*  Set stencil fail stencil update function.  */
            state.stencilFail = gpuData.stencilUpdate;

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_DEPTH_FAIL_UPDATE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTH_FAIL_UPDATE.\n");
            )

            /*  Set stencil update function for depth fail.  */
            state.depthFail = gpuData.stencilUpdate;

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_DEPTH_PASS_UPDATE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTH_PASS_UPDATE.\n");
            )

            /*  Set stencil update function for depth pass.  */
            state.depthPass = gpuData.stencilUpdate;

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        /*  GPU Depth test registers.  */
        case GPU_DEPTH_TEST:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTH_TEST.\n");
            )

            /*  Set depth test enable flag.  */
            state.depthTest = gpuData.booleanVal;

            /*  Send state to the Rasterizer/Fragment FIFO.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }


            break;

        case GPU_DEPTH_FUNCTION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTH_FUNCION.\n");
            )

            /*  Set depth test compare function.  */
            state.depthFunction = gpuData.compare;

            /*  Send state to Rasterizer/Hierarchical Z.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_DEPTH_MASK:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_DEPTH_MASK.\n");
            )

            /*  Set depth test update mask.  */
            state.depthMask = gpuData.booleanVal;

            /*  Send state to Z Stencil Test.  */

            /*  Send register write to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Z Stencil Test.  */
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_ZSTENCIL_COMPRESSION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_ZSTENCIL_COMPRESSION.\n");
            )

            //  Set Z/Stencil compression enable/disable register.
            state.zStencilCompression = gpuData.booleanVal;

            //  Send state to Z Stencil Test.

            //  Send register write to all the Z Stencil units.
            for(i = 0; i < numStampUnits; i++)
            {
                //  Create rasterizer command.
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                //  Send state change to Z Stencil Test.
                zStencilCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC/Blitter.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        //  GPU Color Buffer and Render Target registers.

        case GPU_COLOR_BUFFER_FORMAT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_COLOR_BUFFER_FORMAT.\n");
            )

            //  Set color buffer format register.
            state.colorBufferFormat = gpuData.txFormat;

            //  Send state to Color Write.

            //  Send register write to all the Color Write units.
            for(i = 0; i < numStampUnits; i++)
            {
                //  Create rasterizer command.
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                //  Send state change to Color Write.
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC/Blitter.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_COLOR_COMPRESSION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_COLOR_COMPRESSION = %s.\n", gpuData.booleanVal ? "T" : "F");
            )

            //  Set color compression enable/disable register.
            state.colorCompression = gpuData.booleanVal;

            //  Send state to Color Write.

            //  Send register write to all the Color Write units.
            for(i = 0; i < numStampUnits; i++)
            {
                //  Create rasterizer command.
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                //  Send state change to Color Write.
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC/Blitter.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_COLOR_SRGB_WRITE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_COLOR_WRITE_SRGB.\n");
            )

            //  Set linear to sRGB space conversion on color write register.
            state.colorSRGBWrite = gpuData.txFormat;

            //  Send state to Color Write.

            //  Send register write to all the Color Write units.
            for(i = 0; i < numStampUnits; i++)
            {
                //  Create rasterizer command.
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                //  Send state change to Color Write.
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;
            
        case GPU_RENDER_TARGET_ENABLE:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_RENDER_TARGET_ENABLE.\n");
            )

            //  Set render target enable/disable register.
            state.rtEnable[gpuSubReg] = gpuData.booleanVal;

            //  Send state to the Rasterizer/Fragment FIFO.
            
            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to rasterizer.
            rastCommSignal->write(cycle, rastCom);
            
            //  Send output attribute enable to the fragment shader.
            
            //  Send state change to all fragment shader units.
            for (i = 0; i < numFShaders; i++)
            {
                //  Create shader command to setup the shader output attributes as active.
                shComm = new ShaderCommand(SET_OUT_ATTR, gpuSubReg, gpuData.booleanVal);

                //  Copy cookies from original AGP transaction and a new cookie.
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                //  Send the command to each shader.
                fshFCommandSignal[i]->write(cycle, shComm, 1);
            }

            //  Send state to Color Write.

            //  Send register write to all the Color Write units.
            for(i = 0; i < numStampUnits; i++)
            {
                //  Create rasterizer command.
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                //  Send state change to Color Write.
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_RENDER_TARGET_FORMAT:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_RENDER_TARGET_FORMAT.\n");
            )

            //  Set render target format register.
            state.rtFormat[gpuSubReg] = gpuData.txFormat;

            //  Send state to Color Write.

            //  Send register write to all the Color Write units.
            for(i = 0; i < numStampUnits; i++)
            {
                //  Create rasterizer command.
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                //  Send state change to Color Write.
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);

            break;

        case GPU_RENDER_TARGET_ADDRESS:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_RENDER_TARGET_ADDRESS[%d] = %08x.\n", gpuSubReg, gpuData.uintVal);
            )

            //  Set render target base address register.
            state.rtAddress[gpuSubReg] = gpuData.uintVal;
            
            if (gpuSubReg == 0)
                state.backBufferBaseAddr = gpuData.uintVal;

            //  Send state to Color Write.

            //  Send register write to all the Color Write units.
            for(i = 0; i < numStampUnits; i++)
            {
                //  Create rasterizer command.
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                //  Copy cookies from original AGP transaction and a new cookie.
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                //  Send state change to Color Write.
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            //  Send state to DAC.

            //  Create rasterizer command.
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            //  Copy cookies from original AGP transaction and a new cookie.
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            //  Send state change to DAC.
            dacCommSignal->write(cycle, rastCom);
            
            break;

        /*  GPU Color Blend registers.  */
        case GPU_COLOR_BLEND:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_COLOR_BLEND.\n");
            )

            /*  Set color blend enable flag.  */
            state.colorBlend[gpuSubReg] = gpuData.booleanVal;


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_BLEND_EQUATION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_EQUATION.\n");
            )

            /*  Set color blend equation.  */
            state.blendEquation[gpuSubReg] = gpuData.blendEquation;


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;


        case GPU_BLEND_SRC_RGB:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_SRC_RGB.\n");
            )

            /*  Set color blend source RGB weight factor.  */
            state.blendSourceRGB[gpuSubReg] = gpuData.blendFunction;


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }


            break;

        case GPU_BLEND_DST_RGB:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_DST_RGB.\n");
            )

            /*  Set color blend destination RGB weight factor.  */
            state.blendDestinationRGB[gpuSubReg] = gpuData.blendFunction;


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_BLEND_SRC_ALPHA:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_SRC_ALPHA.\n");
            )

            /*  Set color blend source ALPHA weight factor.  */
            state.blendSourceAlpha[gpuSubReg] = gpuData.blendFunction;


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_BLEND_DST_ALPHA:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_DST_ALPHA.\n");
            )

            /*  Set color blend destination Alpha weight factor.  */
            state.blendDestinationAlpha[gpuSubReg] = gpuData.blendFunction;


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_BLEND_COLOR:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_COLOR.\n");
            )

            /*  Set color blend constant color.  */
            state.blendColor[gpuSubReg][0] = gpuData.qfVal[0];
            state.blendColor[gpuSubReg][1] = gpuData.qfVal[1];
            state.blendColor[gpuSubReg][2] = gpuData.qfVal[2];
            state.blendColor[gpuSubReg][3] = gpuData.qfVal[3];


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_COLOR_MASK_R:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_MASK_R.\n");
            )

            /*  Set color mask for red component.  */
            state.colorMaskR[gpuSubReg] = gpuData.booleanVal;

            /*  Send state to the Rasterizer/Fragment FIFO.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;


        case GPU_COLOR_MASK_G:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_MASK_G.\n");
            )

            /*  Set color mask for green component.  */
            state.colorMaskG[gpuSubReg] = gpuData.booleanVal;

            /*  Send state to the Rasterizer/Fragment FIFO.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_COLOR_MASK_B:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_MASK_B.\n");
            )

            /*  Set color mask for blue component.  */
            state.colorMaskB[gpuSubReg] = gpuData.booleanVal;

            /*  Send state to the Rasterizer/Fragment FIFO.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;


        case GPU_COLOR_MASK_A:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_BLEND_MASK_A.\n");
            )

            /*  Set color mask for alpha component.  */
            state.colorMaskA[gpuSubReg] = gpuData.booleanVal;

            /*  Send state to the Rasterizer/Fragment FIFO.  */

            /*  Create rasterizer command.  */
            rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastCom->copyParentCookies(*lastAGPTrans);
            rastCom->addCookie();

            /*  Send state change to rasterizer.  */
            rastCommSignal->write(cycle, rastCom);


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        /*  GPU Color Logical Operation registers.  */
        case GPU_LOGICAL_OPERATION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_LOGICAL_OPERATION.\n");
            )

            /*  Set color logical operation enable flag.  */
            state.logicalOperation = gpuData.booleanVal;


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;

        case GPU_LOGICOP_FUNCTION:

            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_LOGICOP_FUNCTION.\n");
            )

            /*  Set color mask for red component.  */
            state.logicOpFunction = gpuData.logicOp;


            /*  Send state to Color Write.  */

            /*  Send register write to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create rasterizer command.  */
                rastCom = new RasterizerCommand(gpuReg, gpuSubReg, gpuData);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastCom->copyParentCookies(*lastAGPTrans);
                rastCom->addCookie();

                /*  Send state change to Color Write.  */
                colorWriteCommSignal[i]->write(cycle, rastCom);
            }

            break;
        case GPU_MCV2_2ND_INTERLEAVING_START_ADDR:
            GPU_DEBUG_BOX(
                printf("CommandProcessor => Write GPU_MCV2_2ND_INTERLEAVING_START_ADDR.\n");
            )
            state.mcSecondInterleavingStartAddr = gpuData.uintVal;
            {
                MemoryControllerCommand* mccom =
                    MemoryControllerCommand::createRegWrite(gpuReg, gpuData);
                mccom->copyParentCookies(*lastAGPTrans);
                mccom->addCookie();

                /* Send state change to Memory Controller */
                mcCommSignal->write(cycle, mccom);
            }
            break;
        default:
            /*  Unknown GPU register.  */

            panic("CommandProcessor", "processGPURegisterWrite", "Undefined GPU register identifier.");

            break;

    }
}

/*  Process a GPU register read.  */
void CommandProcessor::processGPURegisterRead(GPURegister gpuReg, u32bit gpuSubReg,
    GPURegData &gpuData)
{

    /*  Read request from a GPU register.  */

    /*  Read GPU state.  */

    /*  Send back the read value through the AGP port.  */

    switch (gpuReg)
    {
        /*  GPU state registers.  */
        case GPU_STATUS:

            /*  Return the current state of the GPU.  */
            gpuData.status = state.statusRegister;

            break;

        case GPU_MEMORY:

            gpuData.uintVal = memorySize;

            break;

        default:
            /*  Unknown GPU register.  */

            panic("CommandProcessor", "processGPURegisterRead", "Undefined GPU register identifier.");

            break;

    }
}

/*  Processes a GPU command.  */
void CommandProcessor::processGPUCommand(u64bit cycle, GPUCommand gpuComm)
{

    MemoryTransaction *memTrans;
    StreamerCommand *streamComm;
    ShaderCommand *shComm;
    RasterizerCommand *rastComm;
    PrimitiveAssemblyCommand *paComm;
    ClipperCommand *clipComm;
    u32bit i;

    /*  Control command issued to the GPU Command Processor:
        *  Check that the AGP transaction can be processed.
        *  Start the command mode.
        *  Issue commands to the other GPU units.  */

    switch (gpuComm)
    {
        case GPU_RESET:
            /*  Send a reset signal to all GPU units.  */

            GPU_DEBUG_BOX(
                printf("CommandProcessor => RESET command.\n");
            )

            /*  Create a reset command to the Streamer unit.  */
            streamComm = new StreamerCommand(STCOM_RESET);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            streamComm->copyParentCookies(*lastAGPTrans);
            streamComm->addCookie();

            /*  Send reset signal to the Streamer Unit.  */
            streamCtrlSignal->write(cycle, streamComm);


            /*  Send reset signal to the vertex shaders.  */
            for(i = 0; i < numVShaders; i++)
            {
                /*  Create a shader command to send a RESET signal to the vertex shader.  */
                shComm = new ShaderCommand(RESET);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to each vertex shader unit.  */
                vshDCommSignal[i]->write(cycle, shComm, 1);
            }

            /*  Create a reset command to the Primitive Assembly unit.  */
            paComm = new PrimitiveAssemblyCommand(PACOM_RESET);

            /*  Copy cookies from original AGP transaction and add new cookie.  */
            paComm->copyParentCookies(*lastAGPTrans);
            paComm->addCookie();

            /*  Send reset signal to the Primitive Assembly Unit.  */
            paCommSignal->write(cycle, paComm);


            /*  Create a reset command to the Clipper.  */
            clipComm = new ClipperCommand(CLPCOM_RESET);

            /*  Copy cookies from original AGP transaction and add a new
                cookie.  */
            clipComm->copyParentCookies(*lastAGPTrans);
            clipComm->addCookie();

            /*  Send reset signal to the Clipper.  */
            clipCommSignal->write(cycle, clipComm);


            /*  Create reset command to the Rasterizer unit.  */
            rastComm = new RasterizerCommand(RSCOM_RESET);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            rastComm->copyParentCookies(*lastAGPTrans);
            rastComm->addCookie();

            /*  Send the reset signal to the Rasterizer.  */
            rastCommSignal->write(cycle, rastComm);


            /*  Send reset command to all the Z Stencil units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create reset command to the Z Stencil Test unit.  */
                rastComm = new RasterizerCommand(RSCOM_RESET);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the reset signal to the Z Stencil Test unit.  */
                zStencilCommSignal[i]->write(cycle, rastComm);
            }

            /*  Send reset command to all the Color Write units.  */
            for(i = 0; i < numStampUnits; i++)
            {
                /*  Create reset command to the Color Write unit.  */
                rastComm = new RasterizerCommand(RSCOM_RESET);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the reset signal to the Color Write unit.  */
                colorWriteCommSignal[i]->write(cycle, rastComm);
            }

            /*  Send reset signal to the fragment shaders.  */
            for(i = 0; i < numFShaders; i++)
            {
                /*  Create a shader command to send a RESET signal to the fragment shader.  */
                shComm = new ShaderCommand(RESET);

                /*  Copy cookies from original AGP transaction and a new cookie.  */
                shComm->copyParentCookies(*lastAGPTrans);
                shComm->addCookie();

                /*  Send the shader command to each fragment shader unit.  */
                fshDCommandSignal[i]->write(cycle, shComm, 1);
            }

            //  Clear last event cycle registers.
            for(u32bit e = 0; e < GPU_NUM_EVENTS; e++)
                lastEventCycle[e] = 0;

            /*  The AGP transaction finished.  Process a new one.  */
            processNewTransaction = TRUE;

//printf("CP %lld => Change to RESET state.\n", cycle);

            /*  Set GPU state to reset.  */
            state.statusRegister = GPU_ST_RESET;

            break;

        case GPU_DRAW:
            /*  Start drawing a batch of vertices.  */

            /*  Check current state.  */
            if (((state.statusRegister != GPU_READY) && (state.statusRegister != GPU_END_GEOMETRY)) || (streamState != ST_READY) ||
                (drawCommandDelay > 0))
            {
                /*  AGP transaction couldn't be processed so wait.  */
                processNewTransaction = FALSE;
            }
            else
            {
                /*  Check Streamer state.  */
                //GPU_ASSERT(
                //    if (streamState != ST_READY)
                //        panic("CommandProcessor", "processGPUCommand", "Draw command while still drawing not supported.");
                //)

                GPU_DEBUG_BOX(
                    printf("CommandProcessor => DRAW command.\n");
                )

//printf("CP => Starting batch %d\n", batch);

                /*  Update batch counter.  */
                batch++;

                //  Check the skip draw command flag
                if (!skipDraw)
                {
                    /*  Create a start drawing signal to the Streamer unit.  */
                    streamComm = new StreamerCommand(STCOM_START);

                    /*  Copy cookies from original AGP transaction and a new cookie.  */
                    streamComm->copyParentCookies(*lastAGPTrans);
                    streamComm->addCookie();

                    /*  Send the command to the Streamer.  */
                    streamCtrlSignal->write(cycle, streamComm);


                    /*  Create a draw start command to the Primitive Assembly unit.  */
                    paComm = new PrimitiveAssemblyCommand(PACOM_DRAW);

                    /*  Copy cookies from original AGP transaction and add new cookie.  */
                    paComm->copyParentCookies(*lastAGPTrans);
                    paComm->addCookie();

                    /*  Send reset signal to the Primitive Assembly Unit.  */
                    paCommSignal->write(cycle, paComm);


                    /*  Create a start command to the Clipper.  */
                    clipComm = new ClipperCommand(CLPCOM_START);

                    /*  Copy cookies from original AGP transaction and add a new
                        cookie.  */
                    clipComm->copyParentCookies(*lastAGPTrans);
                    clipComm->addCookie();

                    /*  Send reset signal to the Clipper.  */
                    clipCommSignal->write(cycle, clipComm);

                    /*  Set geometry rendering phase as started.  */
                    geometryStarted = TRUE;

                    //  Set the delay counter for consecutive draw calls.
                    drawCommandDelay = CONSECUTIVE_DRAW_DELAY;
                    
                    /*  Determine if fragment phase has to receive the batch start command.  */
                    if (state.statusRegister == GPU_READY)
                    {
//printf("CP %lld => Sending DRAW to fragment phase\n", cycle);

                        /*  Create a start drawing signal to the Rasterizer.  */
                        rastComm = new RasterizerCommand(RSCOM_DRAW);

                        /*  Copy cookies from original AGP transaction and a new cookie.  */
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        /*  Send the command to the Rasterizer.  */
                        rastCommSignal->write(cycle, rastComm);


                        /*  Send draw command to all the Z Stencil units.  */
                        for(i = 0; i < numStampUnits; i++)
                        {
                            /*  Create draw command to the Z Stencil Test unit.  */
                            rastComm = new RasterizerCommand(RSCOM_DRAW);

                            /*  Copy cookies from original AGP transaction and a new cookie.  */
                            rastComm->copyParentCookies(*lastAGPTrans);
                            rastComm->addCookie();

                            /*  Send the reset signal to the Z Stencil Test unit.  */
                            zStencilCommSignal[i]->write(cycle, rastComm);
                        }

                        /*  Send draw command to all the Color Write units.  */
                        for(i = 0; i < numStampUnits; i++)
                        {
                            /*  Create draw command to the Color Write unit.  */
                            rastComm = new RasterizerCommand(RSCOM_DRAW);

                            /*  Copy cookies from original AGP transaction and a new cookie.  */
                            rastComm->copyParentCookies(*lastAGPTrans);
                            rastComm->addCookie();

                            /*  Send the reset signal to the Color Write unit.  */
                            colorWriteCommSignal[i]->write(cycle, rastComm);
                        }

//printf("CP %lld => Change to DRAWING state.\n", cycle);

                        /*  Set GPU state to all GPU_DRAWING.  */
                        state.statusRegister = GPU_DRAWING;
                    }
                }
                else
                {
                    //  Set end of batch flag.
                    batchEnd = true;
                }

                //  Send DRAW command to the DAC unit.
                rastComm = new RasterizerCommand(RSCOM_DRAW);

                //  Copy cookies and add a cookie level.
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                //  Send the command to the DAC unit.
                dacCommSignal->write(cycle, rastComm);

                /*  Update statistics.  */
                batches++;

                /*  Check if pipelined batch rendering is enabled or the draw comamnd is being ignored.  */
                if (pipelinedBatches || skipDraw)
                {
                    /*  The AGP transaction finished.  Process a new one.  */
                    processNewTransaction = true;

                }
                else
                {
                    /*  Do not process new transactions until the end of the batch.  */
                    processNewTransaction = false;
                }
                
                //  Change to the next agp transaction log.
                nextWriteLog = (nextWriteLog + 1) & 0x03;
            }

            break;

        case GPU_SWAPBUFFERS:
            /*  Swap front buffer with back buffer.  Marks the end of frame.  */
            /*  Exchange front buffer and back buffer address.  */
            /*  Signal the display unit to display the image.  */

            /*  Check current state.  */
            if (state.statusRegister != GPU_READY)
            {
                /*  AGP transaction couldn't be processed so wait.  */
                processNewTransaction = FALSE;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => SWAPBUFFERS command.\n");
                )

                //  Check if the Command Processor is skipping draw commands
                if (!skipDraw)
                {
                    /*  Send swap command to all the Color Write units.  */
                    for(i = 0; i < numStampUnits; i++)
                    {
                        /*   Create rasterizer command to swap buffers.  */
                        rastComm = new RasterizerCommand(RSCOM_SWAP);

                        /*  Copy cookies from original AGP transaction and a new cookie.  */
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        /*  Send the command to the Color Write.  */
                        colorWriteCommSignal[i]->write(cycle, rastComm);
                    }

                    /*  Update statistics.  */
                    frames++;

                    /*  Unset color write end command issued.  */
                    colorWriteEnd = FALSE;

                    /*  AGP transaction finished, process a new one.  */
                    processNewTransaction = TRUE;

//printf("CP %lld => Change to SWAP state.\n", cycle);

                    /*  Change to swap state.  */
                    state.statusRegister = GPU_SWAP;
                }
                else
                {
                    //  Send FRAME CHANGE command to the DAC unit.
                    rastComm = new RasterizerCommand(RSCOM_FRAME_CHANGE);

                    //  Copy cookies and add a cookie level.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the DAC unit.
                    dacCommSignal->write(cycle, rastComm);

                    //  Update statistics.
                    frames++;

                    //  Reset batch counter.
                    batch = 0;

                    //  Set swap flag.
                    swapReceived = true;

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = true;
                }
            }

            break;

        case GPU_DUMPCOLOR:

            //  Dump color buffer to a file (debug mode only!).

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = false;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => DUMPCOLOR command.\n");
                )

                //  Set comamnd for the DAC unit.
                dumpBufferCommand = RSCOM_DUMP_COLOR;

                //  Send DUMPBUFFER command to all the Color Write units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //   Create DUMPBUFFER rasterizer command.
                    rastComm = new RasterizerCommand(dumpBufferCommand);

                    //  Copy cookies from original AGP transaction and a new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Color Write unit.
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

                //  Reset END command processed by Color Write units flag.
                colorWriteEnd = false;

                //  AGP transaction finished, process a new one.
                processNewTransaction = true;

//printf("CP %lld => Change to DUMPBUFFER state.\n", cycle);

                //  Change to swap state.
                state.statusRegister = GPU_DUMPBUFFER;
            }

            break;

        case GPU_DUMPDEPTH:

            //  Dump depth buffer to a file (debug mode only!).

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = false;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => DUMPDEPTH command.\n");
                )

                //  Set comamnd for the DAC unit.
                dumpBufferCommand = RSCOM_DUMP_DEPTH;

                //  Send DUMPBUFFER command to all the Z Stencil Test units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //   Create DUMPBUFFER rasterizer command.
                    rastComm = new RasterizerCommand(dumpBufferCommand);

                    //  Copy cookies from original AGP transaction and a new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Z Stencil Test unit.
                    zStencilCommSignal[i]->write(cycle, rastComm);
                }

                //  Reset END command processed by Z Stencil Test units flag.
                zStencilTestEnd = false;

                //  AGP transaction finished, process a new one.
                processNewTransaction = true;

//printf("CP %lld => Change to DUMPBUFFER state.\n", cycle);

                //  Change to swap state.
                state.statusRegister = GPU_DUMPBUFFER;
            }

            break;

        case GPU_DUMPSTENCIL:

            //  Dump stencil buffer to a file (debug mode only!).

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = false;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => DUMPSTENCIL command.\n");
                )

                //  Set comamnd for the DAC unit.
                dumpBufferCommand = RSCOM_DUMP_STENCIL;

                //  Send DUMPBUFFER command to all the Z Stencil Test units.
                for(i = 0; i < numStampUnits; i++)
                {
                    //   Create DUMPBUFFER rasterizer command.
                    rastComm = new RasterizerCommand(dumpBufferCommand);

                    //  Copy cookies from original AGP transaction and a new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Z Stencil Test unit.
                    zStencilCommSignal[i]->write(cycle, rastComm);
                }

                //  Reset END command processed by Z Stencil Test units flag.
                zStencilTestEnd = false;

                //  AGP transaction finished, process a new one.
                processNewTransaction = true;

//printf("CP %lld => Change to DUMPBUFFER state.\n", cycle);

                //  Change to swap state.
                state.statusRegister = GPU_DUMPBUFFER;
            }

            break;

        case GPU_BLIT:

            /*  Performs the blitting of a framebuffer region into   */
            /*  a texture in the GPU local memory.                   */
            /*  Sends the Flush command to all the CW in order to    */
            /*  make the framebuffer coherent with the CW compressed */
            /*  lines state. Then, signals the DAC unit to transfer  */
            /*  and convert data.                                    */

            /*  Check current state.  */
            if (state.statusRegister != GPU_READY)
            {
                /*  AGP transaction couldn't be processed so wait.  */
                processNewTransaction = FALSE;
            }
            else
            {
                //  Check for the skip frames flag before issuing the command.
                if (!skipFrames)
                {
                    GPU_DEBUG(
                        printf("CommandProcessor => BLIT command.\n");
                    )

                    /*  Send blit command to all the Color Write units.  */
                    for(i = 0; i < numStampUnits; i++)
                    {
                        /*   Create rasterizer command to perform flush.  */
                        rastComm = new RasterizerCommand(RSCOM_FLUSH);

                        /*  Copy cookies from original AGP transaction and a new cookie.  */
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        /*  Send the command to the Color Write.  */
                        colorWriteCommSignal[i]->write(cycle, rastComm);
                    }

                    /*  Update statistics.  */
                    bitBlits++;

                    /*  Unset color write end command issued.  */
                    colorWriteEnd = FALSE;

                    /*  AGP transaction finished, process a new one.  */
                    processNewTransaction = TRUE;

//printf("CP %lld => Change to BLITTING state.\n", cycle);

                    /*  Change to blitting state.  */
                    state.statusRegister = GPU_BLITTING;
                }
                else
                {
                    //  Skip command and process next transaction.
                    processNewTransaction = true;
                }
            }

            break;

        case GPU_CLEARBUFFERS:
            /*  Clear Z, Stencil and Color buffer.  */

            GPU_DEBUG_BOX(printf("CommandProcessor => CLEARBUFFERS command.\n"); )

            panic("CommandProcessor", "processGPUCommand", "CLEARBUFFERS command not implemented.");

            break;

        case GPU_CLEARZBUFFER:
            /*  Clear Z buffer.  */

            GPU_DEBUG_BOX(
                printf("CommandProcessor => CLEARZBUFFER command.\n");
            )

            //panic("CommandProcessor", "processGPUCommand", "CLEARZBUFFER command not supported.");

            /*  Create a start drawing signal to the Rasterizer.  */
            //rastComm = new RasterizerCommand(RSCOM_CLEAR_Z_BUFFER);

            /*  Copy cookies from original AGP transaction and a new cookie.  */
            //rastComm->copyParentCookies(*lastAGPTrans);
            //rastComm->addCookie();

            /*  Send the command to the Rasterizer.  */
            //rastCommSignal->write(cycle, rastComm);

            panic("CommandProcessor", "processGPUCommand", "CLEARZSTENCILBUFFERS command not implemented.");

            break;

        case GPU_CLEARZSTENCILBUFFER:
            /*  Clear Z/Stencil buffer.  */

            /*  Check current state.  */
            if (state.statusRegister != GPU_READY)
            {
                /*  AGP transaction couldn't be processed so wait.  */
                processNewTransaction = FALSE;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => CLEARZSTENCILBUFFERS command.\n");
                )

                /*  Create Z clear command to the Rasterizert.  */
                rastComm = new RasterizerCommand(RSCOM_CLEAR_ZSTENCIL_BUFFER);

                /*  Copy cookies from original AGP transaction and add new cookie.  */
                rastComm->copyParentCookies(*lastAGPTrans);
                rastComm->addCookie();

                /*  Send the command to the Rasterizer.  */
                rastCommSignal->write(cycle, rastComm);


                /*  Send clear z stencil comamnd to all the Z Stencil units.  */
                for(i = 0; i < numStampUnits; i++)
                {
                    /*  Create a start drawing signal to the Z Stencil Test unit.  */
                    rastComm = new RasterizerCommand(RSCOM_CLEAR_ZSTENCIL_BUFFER);

                    /*  Copy cookies from original AGP transaction and a new cookie.  */
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    /*  Send the command to the Z Stencil Test unit.  */
                    zStencilCommSignal[i]->write(cycle, rastComm);
                }

                /*  AGP transaction finished, process a new one.  */
                processNewTransaction = TRUE;

//printf("CP %lld => Change to CLEAR_Z state.\n", cycle);

                /*  Set GPU state to clear.  */
                state.statusRegister = GPU_CLEAR_Z;
            }

            break;

        case GPU_CLEARCOLORBUFFER:
            /*  Clear color buffer.  */

            /*  Check current state.  */
            if (state.statusRegister != GPU_READY)
            {
                /*  AGP transaction couldn't be processed so wait.  */
                processNewTransaction = FALSE;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => CLEARCOLORBUFFER command.\n");
                )

                /*  Send clear color command to all the Color Write units.  */
                for(i = 0; i < numStampUnits; i++)
                {
                    /*  Create a start drawing signal to the Color Write unit.  */
                    rastComm = new RasterizerCommand(RSCOM_CLEAR_COLOR_BUFFER);

                    /*  Copy cookies from original AGP transaction and a new cookie.  */
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    /*  Send the command to the Color Write unit.  */
                    colorWriteCommSignal[i]->write(cycle, rastComm);
                }

                /*  AGP transaction finished, process a new one.  */
                processNewTransaction = TRUE;

//printf("CP %lld => Change to CLEAR_COLOR state.\n", cycle);

                /*  Set GPU state to clear.  */
                state.statusRegister = GPU_CLEAR_COLOR;
            }

            break;

        case GPU_FLUSHZSTENCIL:

            //  Flush z and stencil caches.

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = FALSE;
            }
            else
            {
                //  Check if command has to be ignored because skip frames mode active.
                if (!skipFrames)
                {
                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => FLUSHZSTENCIL command.\n");
                    )

                    //  Send a flush z and stencil cache comamnd to all the Z Stencil units.
                    for(i = 0; i < numStampUnits; i++)
                    {
                        //  Create a flush caches signal to the Z Stencil Test unit.
                        rastComm = new RasterizerCommand(RSCOM_FLUSH);

                        //  Copy cookies from original AGP transaction and a new cookie.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the Z Stencil Test unit.
                        zStencilCommSignal[i]->write(cycle, rastComm);
                    }

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = TRUE;

//printf("CP %lld => Change to FLUSH_Z state.\n", cycle);

                    //  Set GPU state to flush z.
                    state.statusRegister = GPU_FLUSH_Z;
                }
                else
                {
                    //  Skip command and start processing the next transaction.
                    processNewTransaction = true;
                }
            }


            break;

        case GPU_FLUSHCOLOR:

            //  Flush color caches.

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = FALSE;
            }
            else
            {
                //  Check if command has to be ignored because skip frames mode active.
                if (!skipFrames)
                {
                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => FLUSHCOLOR command.\n");
                    )

                    //  Send flush color cache command to all the Color Write units.
                    for(i = 0; i < numStampUnits; i++)
                    {
                        //  Create a flush caches signal to the Color Write unit.
                        rastComm = new RasterizerCommand(RSCOM_FLUSH);

                        //  Copy cookies from original AGP transaction and a new cookie.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the Color Write unit.
                        colorWriteCommSignal[i]->write(cycle, rastComm);
                    }

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = TRUE;

//printf("CP %lld => Change to FLUSH_COLOR state.\n", cycle);

                    //  Set GPU state to flush color.
                    state.statusRegister = GPU_FLUSH_COLOR;
                }
                else
                {
                    //  Skip command and start processing the next transaction.
                    processNewTransaction = true;
                }
            }

            break;

        case GPU_LOAD_VERTEX_PROGRAM:
            /*  Loads a new vertex program into the vertex shader
                instruction memory.  */

            /*  Check current state.  */
            if ((state.statusRegister != GPU_READY) && ((state.statusRegister != GPU_END_GEOMETRY) || geometryStarted))
            {
                /*  AGP transaction couldn't be processed so wait.  */
                processNewTransaction = FALSE;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => LOAD_VERTEX_PROGRAM command.\n");
                )

                /*  Request the new vertex shader program code from
                    GPU memory.  */

                /*  Delete old program code buffer.  */
                delete[] programCode;

                /*  Allocate memory for the vertex shader code buffer.  */
                programCode = new u8bit[state.vertexProgramSize];

                /*  Check memory allocation.  */
                GPU_ASSERT(
                    if (programCode == NULL)
                        panic("CommandProcessor", "processGPUCommand", "Allocation of the vertex program code buffer failed.");
                )

                /*  Check if memory controller is available.  */
                if ((memoryState & MS_READ_ACCEPT) != 0)
                {

                    /*
                        WHAT IS THE PURPOSE OF THE TICKETS????!!!!!
                            1) DETECT UNHANDLED MEMORY READ DATA??
                            2) DETECT LOST MEMORY READ REQUESTS??
                            3) LIMIT NUMBER OF TRANSACTIONS ON FLY??
                            4) AND HOW ARE WRITES HANDLED?
                            5) OTHERS?
                     */

                    /*  Check available memory ticket.  */
                    GPU_ASSERT(
                        if (freeTickets == 0)
                            panic("CommandProcessor", "processGPUCommand", "No available memory tickets.");
                    )

                    /*  Update free memory ticket counter.  */
                    freeTickets--;

                    /*  Set requested bytes to memory.  */
                    requested = GPU_MIN(state.vertexProgramSize, MAX_TRANSACTION_SIZE);

                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => Requesting from memory at %x %d bytes\n",
                            state.programMemoryBaseAddr + state.vertexProgramAddr,
                            requested);
                    )

                    /*  Create a memory transaction to request the
                        vertex program from the GPU memory.  */
                    memTrans = new MemoryTransaction(
                        MT_READ_REQ,
                        state.programMemoryBaseAddr + state.vertexProgramAddr,
                        requested,
                        programCode,
                        COMMANDPROCESSOR,
                        currentTicket++);

                    /*  Copy original AGP transaction cookie and add new cookie.  */
                    memTrans->copyParentCookies(*lastAGPTrans);
                    memTrans->addCookie();

                    /*  Send the memory transaction to the memory controller.  */
                    writeMemorySignal->write(cycle, memTrans, 1);
                }
                else
                {
                    /*  Reset requested bytes counter.  */
                    requested = 0;
                }

                /*  Reset received bytes from memory.  */
                received = 0;

                /*  AGP transaction finished, process a new one.  */
                processNewTransaction = TRUE;

                /*  Store current state in the state stack.  */
                stateStack = state.statusRegister;

//printf("CP %lld => Change to MEMORY_READ state.\n", cycle);

                /*  Change state to memory read.  */
                state.statusRegister = GPU_MEMORY_READ;
            }

            break;


        case GPU_LOAD_FRAGMENT_PROGRAM:
            /*  Loads a new fragment program into the fragment shader instruction memory.  */

            /*  Check current state.  */
            if ((state.statusRegister != GPU_READY) && (state.statusRegister != GPU_END_GEOMETRY) &&
                (state.statusRegister != GPU_END_FRAGMENT))
            {
                /*  AGP transaction couldn't be processed so wait.  */
                processNewTransaction = FALSE;
            }
            else
            {
                /*  Check if the load can be started right now.  */
                if (state.statusRegister == GPU_END_GEOMETRY)
                {
                    /*  Store the load fragment shader program command until the END_FRAGMENT phase.  */
                    loadFragProgram = new AGPTransaction(GPU_LOAD_FRAGMENT_PROGRAM);
                    loadFragProgram->copyParentCookies(*lastAGPTrans);

                    /*  Set buffered load fragment program flag.  */
                    storedLoadFragProgram = TRUE;
                }
                else
                {
                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => LOAD_FRAGMENT_PROGRAM command.\n");
                    )

                    /*  Request the new fragment shader program code from GPU memory.  */

                    /*  Delete old program code buffer.  */
                    delete[] programCode;

                    /*  Allocate memory for the fragment shader code buffer.  */
                    programCode = new u8bit[state.fragProgramSize];

                    /*  Check memory allocation.  */
                    GPU_ASSERT(
                        if (programCode == NULL)
                            panic("CommandProcessor", "processGPUCommand", "Allocation of the fragment program code buffer failed.");
                    )

                    /*  Check if memory controller is available.  */
                    if ((memoryState & MS_READ_ACCEPT) != 0)
                    {

                        /*
                            WHAT IS THE PURPOSE OF THE TICKETS????!!!!!
                                1) DETECT UNHANDLED MEMORY READ DATA??
                                2) DETECT LOST MEMORY READ REQUESTS??
                                3) LIMIT NUMBER OF TRANSACTIONS ON FLY??
                                4) AND HOW ARE WRITES HANDLED?
                                5) OTHERS?
                         */

                        /*  Check available memory ticket.  */
                        GPU_ASSERT(
                            if (freeTickets == 0)
                                panic("CommandProcessor", "processGPUCommand", "No available memory tickets.");
                        )

                        /*  Update free memory ticket counter.  */
                        freeTickets--;

                        /*  Set requested bytes to memory.  */
                        requested = GPU_MIN(state.fragProgramSize, MAX_TRANSACTION_SIZE);

                        GPU_DEBUG_BOX(
                            printf("CommandProcessor => Requesting from memory at %x %d bytes\n",
                                state.programMemoryBaseAddr + state.fragProgramAddr,
                                requested);
                        )

                        /*  Create a memory transaction to request the fragment program from the GPU memory.  */
                        memTrans = new MemoryTransaction(
                            MT_READ_REQ,
                            state.programMemoryBaseAddr + state.fragProgramAddr,
                            requested,
                            programCode,
                            COMMANDPROCESSOR,
                            currentTicket++);

                        /*  Copy original AGP transaction cookie and add new cookie.  */
                        memTrans->copyParentCookies(*lastAGPTrans);
                        memTrans->addCookie();

                        /*  Send the memory transaction to the memory controller.  */
                        writeMemorySignal->write(cycle, memTrans, 1);
                    }
                    else
                    {
                        /*  Reset requested bytes counter.  */
                        requested = 0;
                    }

                    /*  Reset received bytes from memory.  */
                    received = 0;

                    /*  Store current state in the state stack.  */
                    stateStack = state.statusRegister;

//printf("CP %lld => Change to MEMORY_READ state.\n", cycle);

                    /*  Change state to memory read.  */
                    state.statusRegister = GPU_MEMORY_READ;
                }

                /*  AGP transaction finished, process a new one.  */
                processNewTransaction = TRUE;
            }

            break;

        case GPU_LOAD_SHADER_PROGRAM:

            //  Loads a new fragment program into the fragment shader instruction memory.

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  Shader program loads can't be pipelined witch the current rendering draw.
                processNewTransaction = false;
            }
            else
            {
                GPU_DEBUG_BOX(
                    printf("CommandProcessor => LOAD_SHADER_PROGRAM command.\n");
                )

                //  Request the new fragment shader program code from GPU memory.

                //  Delete old program code buffer.
                delete[] programCode;

                //  Allocate memory for the fragment shader code buffer.
                programCode = new u8bit[state.programSize];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (programCode == NULL)
                        panic("CommandProcessor", "processGPUCommand", "Allocation of the shader program code buffer failed.");
                )

                //  Check if memory controller is available.
                if ((memoryState & MS_READ_ACCEPT) != 0)
                {

                    /*
                        WHAT IS THE PURPOSE OF THE TICKETS????!!!!!
                            1) DETECT UNHANDLED MEMORY READ DATA??
                            2) DETECT LOST MEMORY READ REQUESTS??
                            3) LIMIT NUMBER OF TRANSACTIONS ON FLY??
                            4) AND HOW ARE WRITES HANDLED?
                            5) OTHERS?
                     */

                    //  Check available memory ticket.
                    GPU_ASSERT(
                        if (freeTickets == 0)
                            panic("CommandProcessor", "processGPUCommand", "No available memory tickets.");
                    )

                    //  Update free memory ticket counter.
                    freeTickets--;

                    //  Set requested bytes to memory.
                    requested = GPU_MIN(state.fragProgramSize, MAX_TRANSACTION_SIZE);

                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => Requesting from memory at %x %d bytes\n",
                            state.programMemoryBaseAddr + state.programAddress,
                            requested);
                    )

                    //  Create a memory transaction to request the fragment program from the GPU memory.
                    memTrans = new MemoryTransaction(
                        MT_READ_REQ,
                        state.programMemoryBaseAddr + state.programAddress,
                        requested,
                        programCode,
                        COMMANDPROCESSOR,
                        currentTicket++);

                    //  Copy original AGP transaction cookie and add new cookie.
                    memTrans->copyParentCookies(*lastAGPTrans);
                    memTrans->addCookie();

                    //  Send the memory transaction to the memory controller.
                    writeMemorySignal->write(cycle, memTrans, 1);
                }
                else
                {
                    //  Reset requested bytes counter.
                    requested = 0;
                }

                //  Reset received bytes from memory.
                received = 0;

                //  Store current state in the state stack.
                stateStack = state.statusRegister;

//printf("CP %lld => Change to MEMORY_READ state.\n", cycle);

                //  Change state to memory read.
                state.statusRegister = GPU_MEMORY_READ;

                //  AGP transaction finished, process a new one.
                processNewTransaction = TRUE;
            }

            break;

        case GPU_SAVE_COLOR_STATE:

            //  Save color block state info into memory.

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = FALSE;
            }
            else
            {
                //  Check if command has to be ignored because skip frames mode active.
                if (!skipFrames)
                {
                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => SAVE_COLOR_STATE command.\n");
                    )

                    //  Send save color state command to all the Color Write units.
                    for(i = 0; i < numStampUnits; i++)
                    {
                        //  Create a save state signal to the Color Write unit.
                        rastComm = new RasterizerCommand(RSCOM_SAVE_STATE);

                        //  Copy cookies from original AGP transaction and a new cookie.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the Color Write unit.
                        colorWriteCommSignal[i]->write(cycle, rastComm);
                    }

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = TRUE;

//printf("CP %lld => Change to SAVE_COLOR_STATE state.\n", cycle);

                    //  Set GPU state to save state color.
                    state.statusRegister = GPU_SAVE_STATE_COLOR;
                }
                else
                {
                    //  Skip current command and process new transaction.
                    processNewTransaction = true;
                }
            }

            break;

        case GPU_RESTORE_COLOR_STATE:

            //  Restore color block state info from memory.

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = FALSE;
            }
            else
            {
                //  Check if command has to be ignored because skip frames mode active.
                if (!skipFrames)
                {
                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => RESTORE_COLOR_STATE command.\n");
                    )

                    //  Send restore color state command to all the Color Write units.
                    for(i = 0; i < numStampUnits; i++)
                    {
                        //  Create a restore state signal to the Color Write unit.
                        rastComm = new RasterizerCommand(RSCOM_RESTORE_STATE);

                        //  Copy cookies from original AGP transaction and a new cookie.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the Color Write unit.
                        colorWriteCommSignal[i]->write(cycle, rastComm);
                    }

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = TRUE;

//printf("CP %lld => Change to RESTORE_COLOR_STATE state.\n", cycle);

                    //  Set GPU state to restore state color.
                    state.statusRegister = GPU_RESTORE_STATE_COLOR;
                }
                else
                {
                    //  Skip current command and process new transaction.
                    processNewTransaction = true;
                }
            }

            break;

        case GPU_SAVE_ZSTENCIL_STATE:

            //  Save z and stencil block state info into memory.

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = FALSE;
            }
            else
            {
                //  Check if command has to be ignored because skip frames mode active.
                if (!skipFrames)
                {
                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => SAVE_ZSTENCIL_STATE command.\n");
                    )

                    //  Send save zstencil block state comamnd to all the Z Stencil units.
                    for(i = 0; i < numStampUnits; i++)
                    {
                        //  Create a save state signal to the Z Stencil Test unit.
                        rastComm = new RasterizerCommand(RSCOM_SAVE_STATE);

                        //  Copy cookies from original AGP transaction and a new cookie.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the Z Stencil Test unit.
                        zStencilCommSignal[i]->write(cycle, rastComm);
                    }

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = TRUE;

//printf("CP %lld => Change to SAVE_STATE_Z state.\n", cycle);

                    //  Set GPU state to save state z.
                    state.statusRegister = GPU_SAVE_STATE_Z;
                }
                else
                {
                    //  Skip current command and process new transaction.
                    processNewTransaction = true;
                }
            }

            break;

        case GPU_RESTORE_ZSTENCIL_STATE:

            //  Restore z and stencil block state info into memory.

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = FALSE;
            }
            else
            {
                //  Check if command has to be ignored because skip frames mode active.
                if (!skipFrames)
                {
                    GPU_DEBUG_BOX(
                        printf("CP (%lld) => RESTORE_ZSTENCIL_STATE command.  Transaction %p\n", cycle, lastAGPTrans);
                    )

                    //  Send restore zstencil block state comamnd to all the Z Stencil units.
                    for(i = 0; i < numStampUnits; i++)
                    {
                        //  Create a restore state signal to the Z Stencil Test unit.
                        rastComm = new RasterizerCommand(RSCOM_RESTORE_STATE);

                        //  Copy cookies from original AGP transaction and a new cookie.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the Z Stencil Test unit.
                        zStencilCommSignal[i]->write(cycle, rastComm);
                    }

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = true;

//printf("CP %lld => Change to RESTORE_STATE_Z state.\n", cycle);

                    //  Set GPU state to restore state z.
                    state.statusRegister = GPU_RESTORE_STATE_Z;
                }
                else
                {
                    //  Skip current command and process new transaction.
                    processNewTransaction = true;
                }
            }
            
            break;

        case GPU_RESET_ZSTENCIL_STATE:

            //  Reset the z and stencil buffer block state (include HZ).

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = false;
            }
            else
            {            
                //  Check if command has to be ignored because skip frames mode active.
                if (!skipFrames)
                {
                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => GPU_RESET_ZSTENCIL_STATE command.\n");
                    )

                    //  Create a clear command for the rasterizer (HZ).
                    rastComm = new RasterizerCommand(RSCOM_CLEAR_ZSTENCIL_BUFFER);

                    //  Copy cookies from original AGP transaction and add new cookie.
                    rastComm->copyParentCookies(*lastAGPTrans);
                    rastComm->addCookie();

                    //  Send the command to the Rasterizer.
                    rastCommSignal->write(cycle, rastComm);


                    //  Send a buffer block state reset commmand to all the Z Stencil units.
                    for(i = 0; i < numStampUnits; i++)
                    {
                        //  Create a buffer block state reset commmand for the Z and Stencil unit.
                        rastComm = new RasterizerCommand(RSCOM_RESET_STATE);

                        //  Copy cookies from original AGP transaction and a new cookie.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        ///  Send the command to the Z Stencil Test unit.
                        zStencilCommSignal[i]->write(cycle, rastComm);
                    }

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = true;

                    //  Set GPU state to clear.
                    state.statusRegister = GPU_CLEAR_Z;
                }
                else
                {
                    //  Skip current command and process new transaction.
                    processNewTransaction = true;
                }
            }

            break;

        case GPU_RESET_COLOR_STATE:

            // Reset color buffer block state.

            //  Check current state.
            if (state.statusRegister != GPU_READY)
            {
                //  AGP transaction couldn't be processed so wait.
                processNewTransaction = false;
            }
            else
            {
                //  Check if command has to be ignored because skip frames mode active.
                if (!skipFrames)
                {
                    GPU_DEBUG_BOX(
                        printf("CommandProcessor => GPU_RESET_COLOR_STATE command.\n");
                    )

                    //  Send the reset color state command to all the Color Write units.
                    for(i = 0; i < numStampUnits; i++)
                    {
                        //  Create a reset buffer block state command for the Color Write unit.
                        rastComm = new RasterizerCommand(RSCOM_RESET_STATE);

                        //  Copy cookies from original AGP transaction and a new cookie.
                        rastComm->copyParentCookies(*lastAGPTrans);
                        rastComm->addCookie();

                        //  Send the command to the Color Write unit.
                        colorWriteCommSignal[i]->write(cycle, rastComm);
                    }

                    //  AGP transaction finished, process a new one.
                    processNewTransaction = true;

                    //  Set GPU state to clear.
                    state.statusRegister = GPU_CLEAR_COLOR;
                }
                else
                {
                    //  Skip current command and process new transaction.
                    processNewTransaction = true;
                }
            }

            break;

        default:
            /*  Unknown GPU command.  */

            panic("CommandProcessor", "processGPUCommand", "Undefined GPU Command.");

            break;

    }
}

//  Processes a GPU event
void CommandProcessor::processGPUEvent(u64bit cycle, GPUEvent gpuEvent, string eventMsg)
{
    //  Process event.
    switch(gpuEvent)
    {
        case GPU_UNNAMED_EVENT :

            //  This event is synchronized with command stream.
            if (state.statusRegister != GPU_READY)
            {
                //  Wait until the end of the current transaction.
                processNewTransaction = false;
            }
            else
            {
                //  Print the current cycle and the event message.
                cout << "CP => Cycle " << cycle << ". " << eventMsg << endl;

                //  Transaction was processed.
                processNewTransaction = true;
            }

            break;

        case GPU_END_OF_FRAME_EVENT :

            //  This event is synchronized with command stream.
            if (state.statusRegister != GPU_READY)
            {
                //  Wait until the end of the current transaction.
                processNewTransaction = false;
            }
            else
            {
                //  Check if the event message is empty
                if (eventMsg.empty())
                {
                    //  Just update the last cycle register for the event.
                    lastEventCycle[gpuEvent] = cycle;
                }
                else
                {
                    //  Print the event message.
                    cout << "CP => Cycle " << cycle << ". Lasted " << (cycle - lastEventCycle[gpuEvent]) << " cycles. " << eventMsg << endl;

                    //  Update the last cycle register for the event.
                    lastEventCycle[gpuEvent] = cycle;
                }

                //  Transaction was processed.
                processNewTransaction = true;
            }

            break;

        default :
            //  Unknown event
            panic("CommandProcessor", "processGPUEvent", "Undefined GPU Event.");
            break;

    }
}

/*  Processes a memory transaction.  */
void CommandProcessor::processMemoryTransaction(MemoryTransaction *memTrans)
{
    /*  Process the memory transaction.  */
    switch(memTrans->getCommand())
    {
        case MT_STATE:
            /*  Memory Controller is ready to receive new requests.  */

            GPU_DEBUG_BOX( printf("CommandProcessor => Memory transaction: MT_STATE.\n"); )

            memoryState = memTrans->getState();
            break;


        case MT_READ_DATA:
            /*  Return data from the previous memory request.  */

            GPU_DEBUG_BOX( printf("CommandProcessor => Memory transaction: MT_READ_DATA.  Trans. cycles: %d\n", memTrans->getBusCycles()); )

            /*  Check memory transmissions.  */
            GPU_ASSERT(
                if (transCycles != 0)
                    panic("CommandProcessor", "processMemoryTransaction", "Received memory transaction while bus was still busy.");
            )

            /*  Get number of cycles it must wait until end of data
                transmission.  */
            transCycles = memTrans->getBusCycles();

            /*  Get size of the transaction.  */
            lastSize = memTrans->getSize();

            /*  Update statistics.  */
            bytesRead.inc(lastSize);

            /*  !!! THIS TRANSACTION SHOULD BE RECEIVED JUST ONCE IN A CYCLE !!!  */

            break;

        default:
            panic("CommandProcessor", "clock", "Illegal memory transaction received.");
            break;

    }

    /*  Delete memory transaction.  */
    delete memTrans;
}


void CommandProcessor::reset()
{
    //  Reset base addresses.
    state.programMemoryBaseAddr = 0;

    state.viewportWidth = 400;
    state.viewportHeight = 400;
    state.viewportIniX = 0;
    state.viewportIniY = 0;
    
    state.vertexProgramAddr = 0;
    state.vertexProgramStartPC = 0;
    state.vertexProgramSize = 0;
    state.vertexThreadResources = 0;

    state.fragProgramAddr = 0;
    state.fragProgramStartPC = 0;
    state.fragProgramSize = 0;
    state.fragThreadResources = 0;
    
    state.programAddress = 0;
    state.programSize = 0;
    state.programLoadPC = 0;
    for(u32bit t = 0; t < MAX_SHADER_TARGETS; t++)
    {
        state.programStartPC[t] = 0;
        state.programResources[t] = 0;
    }

    //  Initialize reset buffer.
    freeUpdates[GEOM_REG] = freeUpdates[FRAG_REG] = MAX_REGISTER_UPDATES;
    regUpdates[FRAG_REG] = nextFreeUpdate[FRAG_REG] = nextUpdate[FRAG_REG] = 0;
    regUpdates[GEOM_REG] = nextFreeUpdate[GEOM_REG] = nextUpdate[GEOM_REG] = 0;

    //  Reset process transaction flag.
    processNewTransaction = true;

    //  Reset geometry rendering phase start flag.
    geometryStarted = false;

    //  Reset buffered load fragment program flag.
    storedLoadFragProgram = false;

    //  Reset batch number.
    batch = 0;

    //  Reset event start cycle registers.
    for(u32bit e = 0; e < GPU_NUM_EVENTS; e++)
        lastEventCycle[e] = 0;

    //  Reset delay between consecutive draw calls.
    drawCommandDelay = 0;
}


/*  Get if trace has finished.  */
bool CommandProcessor::isEndOfTrace()
{
    return (traceEnd && (state.statusRegister == GPU_READY));
}

/*  Get if swap was received.  */
bool CommandProcessor::isSwap()
{
    return swapReceived;
}

/*  Get end of batch flag.  */
bool CommandProcessor::endOfBatch()
{
    return batchEnd;
}

//  Returns a string with state information about the box.
void CommandProcessor::getState(string &stateString)
{
    stateString.clear();
    stateString.append("GPU state = ");

    switch(state.statusRegister)
    {
        case GPU_ST_RESET: printf("GPU_ST_RESET\n"); break;
        case GPU_READY: printf("GPU_READY\n"); break;
        case GPU_DRAWING: printf("GPU_DRAWING\n"); break;
        case GPU_END_GEOMETRY: printf("GPU_END_GEOMETRY\n"); break;
        case GPU_END_FRAGMENT: printf("GPU_END_FRAGMENT\n"); break;
        case GPU_MEMORY_WRITE: printf("GPU_MEMORY_WRITE\n"); break;
        case GPU_MEMORY_READ: printf("GPU_MEMORY_READ\n"); break;
        case GPU_MEMORY_PRELOAD: printf("GPU_MEMORY_PRELOAD\n"); break;
        case GPU_SWAP: printf("GPU_SWAP\n"); break;
        case GPU_DUMPBUFFER: printf("GPU_DUMPBUFFER\n"); break;
        case GPU_BLITTING: printf("GPU_BLIT\n"); break;
        case GPU_CLEAR_COLOR: printf("GPU_CLEAR_COLOR\n"); break;
        case GPU_CLEAR_Z: printf("GPU_CLEAR_Z\n"); break;
        case GPU_FLUSH_COLOR: printf("GPU_FLUSH_COLOR\n"); break;
        case GPU_FLUSH_Z: printf("GPU_FLUSH_Z\n"); break;
        case GPU_SAVE_STATE_COLOR: printf("GPU_SAVE_STATE_COLOR\n"); break;
        case GPU_RESTORE_STATE_COLOR: printf("GPU_RESTORE_STATE_COLOR\n"); break;
        case GPU_SAVE_STATE_Z: printf("GPU_SAVE_STATE_Z\n"); break;
        case GPU_RESTORE_STATE_Z: printf("GPU_RESTORE_STATE_Z\n"); break;
        case GPU_ERROR: printf("GPU_ERROR\n"); break;
        default: stateString.append("undefined"); break;
    }
}

//  Set the skip draw command flag.
void CommandProcessor::setSkipDraw(bool skip)
{
    skipDraw = skip;
}

//  Set the skip frames command flag.
void CommandProcessor::setSkipFrames(bool skip)
{
    skipFrames = skip;
    skipDraw = skip;
}

//  List the debug commands supported by the Command Processor
void CommandProcessor::getCommandList(std::string &commandList)
{
    commandList.append("forceswap       - Forces a SWAP command after the current AGP command\n");
    commandList.append("flushzst        - Forces a FLUSHZST command after the current AGP command\n");
    commandList.append("flushcolor      - Forces a FLUSHCOLOR command after the current AGP command\n");
    commandList.append("saveregisters   - Saves a snapshot of the GPU registers.\n");
    commandList.append("_saveregisters  - Saves a snapshot of the GPU registers (silent).\n");
    commandList.append("dumpcolor       - Dumps the color buffer.\n");
    commandList.append("dumpdepth       - Dumps the depth buffer.\n");
    commandList.append("dumpstencil     - Dumps the stencil buffer.\n");
}

//  Execute a debug command
void CommandProcessor::execCommand(stringstream &commandStream)
{
    string command;

    commandStream >> ws;
    commandStream >> command;

    if (!command.compare("forceswap"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "forceswap" << endl;
        }
        else
        {
            cout << " Forcing swap command -> ";

            //  Set forced command flag.
            forcedCommand = true;

            //  Check if no AGP transaction is pending from being finished.
            if (processNewTransaction)
            {
                //  Delete finished AGP transaction.
                if (lastAGPTrans != NULL)
                    delete lastAGPTrans;

                //  Insert the forced swap command as next AGP command to execute.
                lastAGPTrans = new AGPTransaction(GPU_SWAPBUFFERS);

                //  Unset new transaction flag as we have a pending AGP transaction now.
                processNewTransaction = false;

                cout << " as next command" << endl;
            }
            else
            {
                //  Set the force transation flag and wait until the current transaction finishes.
                forceTransaction = true;

                //  Create the swap command transaction that it's being forced on the simulator.
                forcedTransaction = new AGPTransaction(GPU_SWAPBUFFERS);

                cout << " as a delayed command" << endl;
            }
        }
    }
    else if(!command.compare("flushzst"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "flushzst" << endl;
        }
        else
        {
            cout << " Forcing flush z and stencil caches command -> ";

            //  Set forced command flag.
            forcedCommand = true;

            //  Check if no AGP transaction is pending from being finished.
            if (processNewTransaction)
            {
                //  Delete finished AGP transaction.
                if (lastAGPTrans != NULL)
                    delete lastAGPTrans;

                //  Insert the forced flush z and stencil caches command as next AGP command to execute.
                lastAGPTrans = new AGPTransaction(GPU_FLUSHZSTENCIL);

                //  Unset new transaction flag as we have a pending AGP transaction now.
                processNewTransaction = false;

                cout << " as next command" << endl;
            }
            else
            {
                //  Set the force transation flag and wait until the current transaction finishes.
                forceTransaction = true;

                //  Create the flush z and stencil caches command transaction that it's being forced on the simulator.
                forcedTransaction = new AGPTransaction(GPU_FLUSHZSTENCIL);

                cout << " as a delayed command" << endl;
            }
        }
    }
    else if(!command.compare("flushcolor"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "flushcolor" << endl;
        }
        else
        {
            cout << " Forcing flush color caches command -> ";

            //  Set forced command flag.
            forcedCommand = true;

            //  Check if no AGP transaction is pending from being finished.
            if (processNewTransaction)
            {
                //  Delete finished AGP transaction.
                if (lastAGPTrans != NULL)
                    delete lastAGPTrans;

                //  Insert the forced flush color caches command as next AGP command to execute.
                lastAGPTrans = new AGPTransaction(GPU_FLUSHCOLOR);

                //  Unset new transaction flag as we have a pending AGP transaction now.
                processNewTransaction = false;

                cout << " as next command" << endl;
            }
            else
            {
                //  Set the force transation flag and wait until the current transaction finishes.
                forceTransaction = true;

                //  Create the flush color caches command transaction that it's being forced on the simulator.
                forcedTransaction = new AGPTransaction(GPU_FLUSHCOLOR);

                cout << " as a delayed command" << endl;
            }
        }
    }
    else if (!command.compare("saveregisters"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "saveregisters" << endl;
        }
        else
        {
            cout << " Saving registers to file" << endl;
            saveRegisters();
        }
    }
    else if (!command.compare("_saveregisters"))
    {
        saveRegisters();
    }
    else if(!command.compare("dumpcolor"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "dumpcolor" << endl;
        }
        else
        {
            cout << " Forcing dump color buffer command -> ";

            //  Set forced command flag.
            forcedCommand = true;

            //  Check if no AGP transaction is pending from being finished.
            if (processNewTransaction)
            {
                //  Delete finished AGP transaction.
                if (lastAGPTrans != NULL)
                    delete lastAGPTrans;

                //  Insert the forced dump color command as next AGP command to execute.
                lastAGPTrans = new AGPTransaction(GPU_DUMPCOLOR);

                //  Unset new transaction flag as we have a pending AGP transaction now.
                processNewTransaction = false;

                cout << " as next command" << endl;
            }
            else
            {
                //  Set the force transation flag and wait until the current transaction finishes.
                forceTransaction = true;

                //  Create the dump color command transaction that it's being forced on the simulator.
                forcedTransaction = new AGPTransaction(GPU_DUMPCOLOR);

                cout << " as a delayed command" << endl;
            }
        }
    }
    else if(!command.compare("dumpdepth"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "dumpdepth" << endl;
        }
        else
        {
            cout << " Forcing dump depth buffer command -> ";

            //  Set forced command flag.
            forcedCommand = true;

            //  Check if no AGP transaction is pending from being finished.
            if (processNewTransaction)
            {
                //  Delete finished AGP transaction.
                if (lastAGPTrans != NULL)
                    delete lastAGPTrans;

                //  Insert the forced dump depth command as next AGP command to execute.
                lastAGPTrans = new AGPTransaction(GPU_DUMPDEPTH);

                //  Unset new transaction flag as we have a pending AGP transaction now.
                processNewTransaction = false;

                cout << " as next command" << endl;
            }
            else
            {
                //  Set the force transation flag and wait until the current transaction finishes.
                forceTransaction = true;

                //  Create the dump depth command transaction that it's being forced on the simulator.
                forcedTransaction = new AGPTransaction(GPU_DUMPDEPTH);

                cout << " as a delayed command" << endl;
            }
        }
    }
    else if(!command.compare("dumpstencil"))
    {
        if (!commandStream.eof())
        {
            cout << "Usage: " << endl;
            cout << "dumpstencil" << endl;
        }
        else
        {
            cout << " Forcing dump stencil buffer command -> ";

            //  Set forced command flag.
            forcedCommand = true;

            //  Check if no AGP transaction is pending from being finished.
            if (processNewTransaction)
            {
                //  Delete finished AGP transaction.
                if (lastAGPTrans != NULL)
                    delete lastAGPTrans;

                //  Insert the forced dump stencil command as next AGP command to execute.
                lastAGPTrans = new AGPTransaction(GPU_DUMPSTENCIL);

                //  Unset new transaction flag as we have a pending AGP transaction now.
                processNewTransaction = false;

                cout << " as next command" << endl;
            }
            else
            {
                //  Set the force transation flag and wait until the current transaction finishes.
                forceTransaction = true;

                //  Create the dump stencil command transaction that it's being forced on the simulator.
                forcedTransaction = new AGPTransaction(GPU_DUMPSTENCIL);

                cout << " as a delayed command" << endl;
            }
        }
    }
    else
    {
        cout << "Unsupported box command >> " << command;

        while (!commandStream.eof())
        {
            commandStream >> command;
            cout << " " << command;
        }

        cout << endl;
    }
}

//  Return if the initialization phase has finished.
bool CommandProcessor::endOfInitialization()
{
    return initEnd;
}

//  Return if the last command finished in this cycle.
bool CommandProcessor::endOfCommand()
{
    return commandEnd;
}

//  Return if the forced command finished in this cycle.
bool CommandProcessor::endOfForcedCommand()
{
    if (commandEnd && !forceTransaction && forcedCommand)
    {
        forcedCommand = false;

        return true;
    }
    else
        return false;
}


void CommandProcessor::saveRegisters()
{
    ofstream out;

    out.open("registers.snapshot", ios::binary);

    if (!out.is_open())
        panic("CommandProcessor", "saveRegisters", "Error creating register snapshot file.");

    out.write((char *) &state, sizeof(state));
    out.close();
}

vector<AGPTransaction*> &CommandProcessor::getAGPTransactionLog()
{
    nextReadLog = (nextReadLog + 1) & 0x03;
    
    return agpTransactionLog[nextReadLog];
}

void CommandProcessor::setValidationMode(bool enable)
{
    enableValidation = enable;
}



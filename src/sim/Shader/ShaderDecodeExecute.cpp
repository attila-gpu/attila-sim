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
 * $RCSfile: ShaderDecodeExecute.cpp,v $
 * $Revision: 1.28 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:53 $
 *
 * Shader Unit Decode and Execute Box (unified shader version).
 *
 */

/**
 *
 *  @file ShaderDecodeExecute-Unified.cpp
 *
 *  This file implements a ShaderDecodeExecute Box (unified shader version).
 *
 *  Defines and implements a simulator Box for the Decode,
 *  Execute and WriteBack stages of a Shader pipeline.
 *
 */

#include "ShaderDecodeExecute.h"
#include "TextureUnitStateInfo.h"
#include "TextureRequest.h"
#include "TextureResult.h"

using namespace gpu3d;


ShaderDecodeExecute::ShaderDecodeExecute(ShaderEmulator &shEmu, bool unified, u32bit threads,
    u32bit threadsPerCycle, u32bit instrPerCycle, u32bit threadsPerGroup, bool lockStepMode, u32bit textUnits,
    u32bit requestsCycle, u32bit requestsUnit, bool scalar, char *name, char *shPrefix, char *sh2Prefix, Box *parent):

    /*  Initializations  */
    Box(name, parent), shEmul(shEmu), unifiedModel(unified), numThreads(threads), threadsCycle(threadsPerCycle),
    instrCycle(instrPerCycle), threadGroup(threadsPerGroup), lockStep(lockStepMode), textureUnits(textUnits),
    textureRequestRate(requestsCycle), requestsPerTexUnit(requestsUnit), scalarALU(scalar), instrBufferSize(2)

{

    DynamicObject* defValue[1];
    u32bit i;
    char fullName[64];
    char postfix[32];
    char tuPrefix[64];

    /*  Check parameters.  */
    GPU_ASSERT(
        if (threadGroup == 0)
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "At least one thread per group required.");
        if (numThreads < threadGroup)
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "At least one shader thread group required.");
        if (threadsCycle == 0)
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "At least one shader thread must be decoded per cycle.");
        if (instrCycle == 0)
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "At least one instruction from a shader thread must be decoded per cycle.");
        if (scalarALU && (instrCycle != 2))
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Scalar ALU only supported for 2-way configuration.");
        if ((textureUnits > 0) && (textureRequestRate == 0))
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "The texture request rate must be at least 1.");
        if ((textureUnits > 0) && (requestsPerTexUnit == 0))
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "The request per texture unit must be at least 1.");
    )

    /*  Create the full name and postfix for the statistics.  */
    sprintf(fullName, "%s::%s", shPrefix, name);
    sprintf(postfix, "ShDX-%s", shPrefix);

//printf("ShDecExec %s -> %p \n", fullName, this);

    /*  Create statistics.  */
    execInstr = &getSM().getNumericStatistic("ExecutedInstructions", u32bit(0), fullName, postfix);
    blockedInstr = &getSM().getNumericStatistic("BlockedInstructions", u32bit(0), fullName, postfix);
    fakedInstr = &getSM().getNumericStatistic("FakedInstructions", u32bit(0), fullName, postfix);
    removedInstr = &getSM().getNumericStatistic("RemovedInstructions", u32bit(0), fullName, postfix);
    blocks = &getSM().getNumericStatistic("BlockCommands", u32bit(0), fullName, postfix);
    unblocks = &getSM().getNumericStatistic("UnblockCommands", u32bit(0), fullName, postfix);
    ends = &getSM().getNumericStatistic("EndCommands", u32bit(0), fullName, postfix);
    replays = &getSM().getNumericStatistic("ReplayCommands", u32bit(0), fullName, postfix);
    textures = &getSM().getNumericStatistic("TextureRequests", u32bit(0), fullName, postfix);
    zexports = &getSM().getNumericStatistic("ZExports", u32bit(0), fullName, postfix);

    /*  Initialize signals.  */

    /*  Check shader model.  */
    if (unifiedModel)
    {
        /*  Create command signal from the Command Processor.  */
        commandSignal[VERTEX_PARTITION] = newInputSignal("ShaderDecodeExecuteCommand", 1, 1, sh2Prefix);
        commandSignal[FRAGMENT_PARTITION] = newInputSignal("ShaderDecodeExecuteCommand", 1, 1, shPrefix);
    }
    else
    {
        /*  Create command signal from the Command Processor.  */
        commandSignal[PRIMARY_PARTITION] = newInputSignal("ShaderDecodeExecuteCommand", 1, 1, shPrefix);
    }

    /*  Create instruction signal from the Shader Fetch.  */
    instructionSignal = newInputSignal("ShaderInstruction", threadsCycle * instrCycle, 1, shPrefix);

    /*  Create control signal to the Shader Fetch.   */
    //controlSignal = newOutputSignal("ShaderControl", (2 + textureUnits) * GPU_MAX(threadGroup, threadsCycle) * instrCycle, 1, shPrefix);
    controlSignal = newOutputSignal("ShaderControl", textureUnits * STAMP_FRAGMENTS + ((MAX_EXEC_BW + 1) * threadsCycle * instrCycle), 1, shPrefix);

    /*  Create state signal to the Shader Fetch.  */
    decodeStateSignal = newOutputSignal("ShaderDecodeState", 1, 1, shPrefix);

    /*  Create signals that simulate the execution latencies.  */
    executionStartSignal = newOutputSignal("ShaderExecution", threadsCycle * instrCycle * MAX_EXEC_BW, MAX_EXEC_LAT, shPrefix);
    executionEndSignal = newInputSignal("ShaderExecution", threadsCycle * instrCycle * MAX_EXEC_BW, MAX_EXEC_LAT, shPrefix);

    /*  Build initial signal state array.  */
    defValue[0] = new ShaderDecodeStateInfo(SHDEC_READY);

    /*  Set ShaderDecodeState signal initial value. */
    decodeStateSignal->setData(defValue);

    /*  Allocate the array of signals with the texture units.  */
    if (textureUnits > 0)
    {
        textRequestSignal = new Signal*[textureUnits];
        textResultSignal = new Signal*[textureUnits];
        textUnitStateSignal = new Signal*[textureUnits];

        /*  Check allocation.  */
        GPU_ASSERT(
            if (textRequestSignal == NULL)
                panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Error allocating texture request signal array.");
            if (textResultSignal == NULL)
                panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Error allocating texture result signal array.");
            if (textUnitStateSignal == NULL)
                panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Error allocating texture unit state signal array.");
        )
    }

    /*  Create signals with the Texture Units.  */
    for(i = 0; i < textureUnits; i++)
    {
        /*  Create the texture unit prefix.  */
        sprintf(tuPrefix, "%sTU%d", shPrefix, i);

        /*  Create request signal to the texture unit.  */
        textRequestSignal[i] = newOutputSignal("TextureRequest", textureRequestRate, 1, tuPrefix);

        /*  Create result signal from the texture unit.  */
        textResultSignal[i] = newInputSignal("TextureData", 1, 1, tuPrefix);

        /*  Create state signal from the texture unit.  */
        textUnitStateSignal[i] = newInputSignal("TextureUnitState", 1, 1, tuPrefix);
    }

    /*  Allocate thread state table for decode and execution stage.  */
    threadInfo = new ThreadInfo[numThreads];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (threadInfo == NULL)
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Error allocating thread state table.");
    )

    /*  Allocate the instruction buffer.  */

    /*  Allocate enough space for the instruction and fedback latencies.  */
    instrBuffer = new ShaderExecInstruction**[instrBufferSize];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (instrBuffer == NULL)
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Error allocating instruction buffer.");
    )

    for(i = 0; i < instrBufferSize; i++)
    {
        /*  Allocate per cycle instruction buffers.  */
        instrBuffer[i] = new ShaderExecInstruction*[threadsCycle * instrCycle];

        GPU_ASSERT(
            if (instrBuffer[i] == NULL)
                panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Error allocating per cycle instruction buffer.");
        )
    }

    /*  Allocate auxiliary array for texture access finish threads.  */
    textThreads = new u32bit[threadGroup];

    /*  Check allocation.  */
    GPU_ASSERT(
        if (textThreads == NULL)
            panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Error allocating auxiliary array for threads ending texture access.");
    )

    /*  Allocate array of texture unit tickets.  */
    if (textureUnits > 0)
    {
        textureTickets = new u32bit[textureUnits];

        GPU_ASSERT(
            if (textureTickets == NULL)
                panic("ShaderDecodeExecute", "ShaderDecodeExecute", "Error allocating texture units ticket array.");
        )
    }

    /*  Calculate the number of thread groups that can be decoded and executed per cycle.  */
    groupsCycle = u32bit(GPU_CEIL(f32bit(threadsCycle) / u32bit(threadGroup)));

    //  Get the singleton Shader Architecture Parameters.
    shArchParams = ShaderArchitectureParameters::getShaderArchitectureParameters();
    
    /*  Change state to reset.  */
    state = SH_RESET;
}


/*  Clock function for the class.  Drives the time of the simulation.  */

void ShaderDecodeExecute::clock(u64bit cycle)
{
    ShaderExecInstruction *shExecInstr;
    ShaderDecodeState decodeState;
    ShaderCommand *shCommand;
    TextureAccess *textAccess;
    TextureRequest *textRequest;
    TextureResult *textResult;
    TextureUnitStateInfo *textUnitStateInfo;
    u32bit i;
    u32bit j;
    u32bit k;
    u32bit threadInstr;
    bool execute;
    bool noMoreFetch;
    bool someFetch;
    bool repeat;
    bool block;
    u32bit currentGroup;
    u32bit execGroups;

    /*  Get all instructions that have finished their execution in this
        cycle.  */

    GPU_DEBUG_BOX(
        printf("ShaderDecodeExecute => Clock %lld.\n", cycle);
    )

    /*  Check shader model.  */
    if (unifiedModel)
    {
        /*  Process commands from the Command Processor.  */
        if (commandSignal[VERTEX_PARTITION]->read(cycle, (DynamicObject *&) shCommand))
            processCommand(shCommand);

        /*  Process commands from the Command Processor.  */
        if (commandSignal[FRAGMENT_PARTITION]->read(cycle, (DynamicObject *&) shCommand))
            processCommand(shCommand);
    }
    else
    {
        /*  Process commands from the Command Processor.  */
        if (commandSignal[PRIMARY_PARTITION]->read(cycle, (DynamicObject *&) shCommand))
            processCommand(shCommand);
    }

    /*  Receive state signal from the Texture Unit.  */
    for(i = 0; i < textureUnits; i++)
    {
        /*  Check if the state signal is received.  */
        if (textUnitStateSignal[i]->read(cycle, (DynamicObject *&) textUnitStateInfo))
        {
//printf("ShDE(%p) %lld => Texture tickets received from Texture Unit %d\n", this,
//cycle, i);
            /*  Update number of texture tickets from Texture Unit.  */
            textureTickets[i] += 4;

            /*  Delete carrier object.  */
            delete textUnitStateInfo;
        }
    }

    /*  Check state of the shader.  */
    if (state == SH_RESET)
    {
        /*  Reset register writes table.  */
        for(i = 0; i < MAX_EXEC_LAT; i++)
            regWrites[i] = 0;

        /*  Reset pointer to the current cycle entry in the register write table.  */
        nextRegWrite = 0;

        /*  Reset current thread in the thread group counter.  */
        groupThread = 0;

        /*  Reset instructions executed for the first thread in a group counter.  */
        instrGroup = 0;

        /*  Reset instructions processed for the current thread in the group.  */
        instrThread = 0;

        /*  Reset instruction buffer pointers.  */
        nextInstr = nextInstrCycle = nextFreeInstr = nextFreeInstrCycle = 0;
        freeInstr = instrBufferSize;

        /*  Reset decode execute state.  */
        for(i = 0; i < numThreads; i ++)
        {
            /*  Reset thread state.  */
            threadInfo[i].pending = 0;
            threadInfo[i].ready = true;
            threadInfo[i].waitTexture = false;
            threadInfo[i].end = FALSE;
            threadInfo[i].waitRepeated = false;
            threadInfo[i].zexport = false;

            /*  Reset thread dependence tables.  */
            for(j = 0; j < MAX_TEMP_BANK_REGS; j++)
            {
                threadInfo[i].tempBankDep[j] = FALSE;
                threadInfo[i].tempBankWrite[j] = 0;
            }
            for(j = 0; j < MAX_ADDR_BANK_REGS; j++)
            {
                threadInfo[i].addrBankDep[j] = FALSE;
                threadInfo[i].addrBankWrite[j] = 0;
            }
            for(j = 0; j < MAX_PRED_BANK_REGS; j++)
            {
                threadInfo[i].predBankDep[j] = FALSE;
                threadInfo[i].predBankWrite[j] = 0;
            }
            for(j = 0; j < MAX_OUTP_BANK_REGS; j++)
            {
                threadInfo[i].outpBankDep[j] = FALSE;
                threadInfo[i].outpBankWrite[j] = 0;
            }
        }

        /*  Reset number of texture tickets.  */
        for(i = 0; i < textureUnits; i++)
            textureTickets[i] = 0;

        /*  Reset pointer to the next texture unit to which send requests.  */
        nextRequestTU = 0;

        /*  Reset number of requests send to the current texture unit.  */
        tuRequests = 0;

        /*  Change state to ready.  */
        state = SH_READY;
    }
    else
    {
        /*  Reset current entry in the register write table.  */
        regWrites[nextRegWrite] = 0;

        /*  Update pointer to the next entry in the register write table.  */
        nextRegWrite = GPU_MOD(nextRegWrite + 1, MAX_EXEC_LAT);

        /*  Update register write cycle counters.  */
        //for(i = 0; i < numThreads; i++)
        //{
        //    /*  Update address register bank write cycle counters.  */
        //    for(j = 0; j < MAX_ADDR_BANK_REGS; j++)
        //        if (threadInfo[i].addrBankWrite[j] > 0)
        //            threadInfo[i].addrBankWrite[j]--;

            /*  Update Temporal register bank write cycle counters.  */
        //   for(j = 0; j < MAX_TEMP_BANK_REGS; j++)
        //        if (threadInfo[i].tempBankWrite[j] > 0)
        //            threadInfo[i].tempBankWrite[j]--;

            /*  Update output register bank write cycle counters.  */
        //    for(j = 0; j < MAX_OUTP_BANK_REGS; j++)
        //        if (threadInfo[i].outpBankWrite[j] > 0)
        //            threadInfo[i].outpBankWrite[j]--;
        //}

        /*  End execution of instructions.  */
        endExecution(cycle);

        /*  Receive and store instructions from fetch.  */
        for(i = 0, noMoreFetch = false, someFetch = false, nextFreeInstrCycle = 0;
            (i < threadsCycle) && !noMoreFetch; i++)
        {
            /*  Instructions for a thread.  */
            for(j = 0; (j < instrCycle) && !noMoreFetch; j++)
            {
                /*  Check if a new instruction is available.  */
                if (instructionSignal->read(cycle, (DynamicObject *&) shExecInstr))
                {
//printf("ShDE %lld => Received instr %p stored at %d, %d freeInstr %d\n",
//cycle, shExecInstr, nextFreeInstr, nextFreeInstrCycle, freeInstr);

                    /*  Store instruction.  */
                    instrBuffer[nextFreeInstr][nextFreeInstrCycle] = shExecInstr;

                    /*  Update to the next position where to store a thread in the current cycle.  */
                    nextFreeInstrCycle++;

                    /*  Fetched an instruction.  */
                    someFetch = true;
                }
                else
                {
                    /*  Do not expect more instructions.  */
                    noMoreFetch = true;
                }
            }

            /*  Check if lock step mode and one thread received a different number of instructions.  */
            ///GPU_ASSERT(
            //    if (lockStep && (i > 0) && (j != instrCycle))
            //        panic("ShaderDecodeExecute", "clock", "Same number (fetchRate) of instructions per cycle and thread required for group execution.");
            //)

            /*  Check if thread lock step execution mode is active and a thread in the group didn't receive
                instructions.  */
            //if (lockStep && noMoreFetch && (i > 0))
            //    panic("ShaderDecodeExecute", "clock", "Missing instructions from threads in current cycle for lock step execution mode.");
        }

        /*  Check if instructions were received in the current cycle.  */
        if (someFetch)
        {
            GPU_ASSERT(
                if (freeInstr == 0)
                    panic("ShaderDecodeExecute", "clock", "No free entries in the instruction buffer.");
            )

            /*  Set to null all empty instruction slots.  */
            for(i = nextFreeInstrCycle; i < (threadsCycle * instrCycle); i++)
                instrBuffer[nextFreeInstr][i] = NULL;

            /*  Update pointer to the next free entry in the instruction buffer.  */
            nextFreeInstr = GPU_MOD(nextFreeInstr + 1, instrBufferSize);

            /*  Update number of free instruction buffer entries.  */
            freeInstr--;
        }

        /*  Decode and execute instructions received from the fetch stage.  */
        for(i = 0, execGroups = 0, block = false; (i < threadsCycle) && (execGroups < groupsCycle) &&
            (!block) && (freeInstr < instrBufferSize); i++)
        {
            /*  Reset reserved ALUs for the current executing thread.  */
            reserveSIMDALU = false;
            reserveScalarALU = false;

            /*  Instructions for a thread.  */
            for(j = 0; (j < instrCycle) && (instrThread < instrCycle) && !block; j++)
            {
                /*  Get next instruction to decode and execute from the instruction buffer.  */
                shExecInstr = instrBuffer[nextInstr][nextInstrCycle];

//printf("ShDE %lld => Reading instr %p stored at %d, %d freeInstr %d\n",
//cycle, shExecInstr, nextInstr, nextInstrCycle, freeInstr);

                /*  Check if there is an instruction in the slot.  */
                if (shExecInstr != NULL)
                {
                    /*  Check if the instruction is a faked fetch.  */
                    if (shExecInstr->getFakeFetch())
                    {
//printf("ShDE %lld => Fake fetch\n", cycle);
                        /*  Ignore the fake fetch.  */

                        /*  Delete the decoded instruction.  */
                        delete &shExecInstr->getShaderInstruction();

                        /*  Remove the instruction.  */
                        delete shExecInstr;

                        /*  Update statistics.  */
                        fakedInstr->inc();

                        /*  Update pointer to the next instruction to decode and execute.  */
                        nextInstrCycle++;

                        /*  Check if all the current fetch cycle instructions were decoded and executed.  */
                        if (nextInstrCycle == (instrCycle * threadsCycle))
                        {
                            /*  Update pointer to the next instruction buffer entry.  */
                            nextInstr = GPU_MOD(nextInstr + 1, instrBufferSize);

                            /*  Reset pointer to the next instruction in the buffer entry.  */
                            nextInstrCycle = 0;

                            /*  Update number of free instruction buffer entries.  */
                            freeInstr++;
                        }

                        /*  Update current fetch cycle instruction for the thread.  */
                        instrThread++;
                    }
                    else
                    {
                        /*  Check if instruction can be executed based on the instructions the first
                            thread in the thread group executed.  */
                        if ((groupThread > 0) && (instrThread >= instrGroup))
                        {
//printf("ShDE %lld => Ignore instr %p stored at %d, %d freeInstr %d groupThread %d instrGroup %d instrThread %d\n",
//cycle, shExecInstr, nextInstr, nextInstrCycle, freeInstr, groupThread, instrGroup, instrThread);
                            /*  Ignore the instruction.  */
                            execute = false;
                            repeat = false;
                        }
                        else
                        {
                            /*  Decode instruction.  */
                            execute = decodeInstruction(cycle, shExecInstr, repeat, (groupThread > 0));
                        }

                        /*  Check if the instruction can be executed.  */
                        if (execute)
                        {
//printf("ShDE %lld => executing instruction %d for thread in group %d\n", cycle, instrThread, groupThread);

                            /*  Start the execution of the instruction for the current group.  */
                            startExecution(cycle, shExecInstr);

                            /*  Check if instruction from the first thread in the thread group.  */
                            if (groupThread == 0)
                            {
                                /*  Update number of instructions to execute per cycle for the thread group.  */
                                instrGroup++;
                            }

                            /*  Update pointer to the next instruction to decode and execute.  */
                            nextInstrCycle++;

                            /*  Check if all the current fetch cycle instructions were decoded and executed.  */
                            if (nextInstrCycle == (instrCycle * threadsCycle))
                            {
                                /*  Update pointer to the next instruction buffer entry.  */
                                nextInstr = GPU_MOD(nextInstr + 1, instrBufferSize);

                                /*  Reset pointer to the next instruction in the buffer entry.  */
                                nextInstrCycle = 0;

                                /*  Update number of free instruction buffer entries.  */
                                freeInstr++;
                            }

                            /*  Update current fetch cycle instruction for the thread.  */
                            instrThread++;
                        }
                        else
                        {
                            /*  Repeat only the first instructions in a thread group.  Block
                                instructions in the middle of the thread group.  */
                            block = (groupThread > 0) && repeat;
                            repeat = repeat && !block;

                            /*  Check if the instruction must be repeated.  */
                            if (repeat)
                            {
//printf("ShDE %lld => repeating instruction %d for thread in group %d\n", cycle, instrThread, groupThread);
                                /*  Replay the instruction.   */
                                repeatInstruction(cycle, shExecInstr);
                            }

                            /*  Check if the instruction is waiting for dependences.  */
                            if (block)
                            {
//printf("ShDE %lld => blocking instruction %d for thread in group %d\n", cycle, instrThread, groupThread);
                                /*  Update statistics.  */
                                blockedInstr->inc();
                            }
                            else
                            {
//printf("ShDE %lld => removing instruction %d for thread in group %d\n", cycle, instrThread, groupThread);

                                /*  Delete the decoded instruction.  */
                                delete &shExecInstr->getShaderInstruction();

                                /*  Remove the instruction.  */
                                delete shExecInstr;

                                /*  Update statistics.  */
                                removedInstr->inc();

                                /*  Update pointer to the next instruction to decode and execute.  */
                                nextInstrCycle++;

                                /*  Check if all the current fetch cycle instructions were decoded and executed.  */
                                if (nextInstrCycle == (instrCycle * threadsCycle))
                                {
                                    /*  Update pointer to the next instruction buffer entry.  */
                                    nextInstr = GPU_MOD(nextInstr + 1, instrBufferSize);

                                    /*  Reset pointer to the next instruction in the buffer entry.  */
                                    nextInstrCycle = 0;

                                    /*  Update number of free instruction buffer entries.  */
                                    freeInstr++;
                               }

                                /*  Update current fetch cycle instruction for the thread.  */
                                instrThread++;
                            }
                        }
                    }
                }
                else
                {
                    /*  Update pointer to the next instruction to decode and execute.  */
                    nextInstrCycle++;

                    /*  Check if all the current fetch cycle instructions were decoded and executed.  */
                    if (nextInstrCycle == (instrCycle * threadsCycle))
                    {
                        /*  Update pointer to the next instruction buffer entry.  */
                        nextInstr = GPU_MOD(nextInstr + 1, instrBufferSize);

                        /*  Reset pointer to the next instruction in the buffer entry.  */
                        nextInstrCycle = 0;

                        /*  Update number of free instruction buffer entries.  */
                        freeInstr++;
                    }
                }
            }

            /*  Check if all the fetch cycle (instrCycle) instructions for the thread were processed.  */
            if (instrThread == instrCycle)
            {
                /*  Reset instructions processed for the cycle.  */
                instrThread = 0;

                /*  Update counter of the current thread in the thread group being decoded and executed.  */
                groupThread++;

                /*  Check if thread group was finished.  */
                if (groupThread == threadGroup)
                {
                    /*  Reset to first thread in the group.  */
                    groupThread = 0;

                    /*  Reset instructions to execute for each thread in the group.  */
                    instrGroup = 0;

                    /*  Update number of executed groups in the current cycle.  */
                    execGroups++;
                }
            }
        }


        /*  If Texture Unit attached process pending texture accesses.  */
        if (textureUnits > 0)
        {
            /*  Send texture requests to the current texture unit.  */
            for(i = 0; i < textureRequestRate; i++)
            {
                /*  Process texture accesses waiting in the shader emulator.  */
                textAccess = shEmul.nextTextureAccess();

                /*  Check if there was a texture access available.  */
                if (textAccess != NULL)
                {
                    /*  NOTE:  IT SHOULD COPY COOKIES FROM THE SHADER INSTRUCTIONS GENERATING THE ACCESS??  */

                    /*  Set the cycle the texture access was issued to the Texture Unit.  */
                    textAccess->cycle = cycle;

                    textRequest = new TextureRequest(textAccess);

                    /*  Copy cookies from original instruction and add a new cookie.  */
                    //textRequest->copyParentCookies(?shExecInstr);
                    //textRequest->addCookie();

                    GPU_DEBUG_BOX(
                        printf("ShaderDecodeExecute %lld => Sending texture request to texture unit %d\n", cycle,
                            nextRequestTU);
                    )

                    /*  Send the request for the texture access to the texture unit.  */
                    textRequestSignal[nextRequestTU]->write(cycle, textRequest);

                    /*  Update the number of texture requests send to the current texture unit.  */
                    tuRequests++;

                    /*  Update statistics.  */
                    textures->inc();
                }
                else
                {
                    /*  Check if enough requests sent for this texture unit.  */
                    if (tuRequests >= requestsPerTexUnit)
                    {
                        /*  Skip to the next texture unit.  */
                        tuRequests = 0;
                        nextRequestTU = GPU_MOD(nextRequestTU + 1, textureUnits);
                    }
                }
            }

            /*  Receive texture access results from the texture units.  */
            for(i = 0; i < textureUnits; i++)
            {
                /*  Receive texture access results from a texture unit.  */
                if (textResultSignal[i]->read(cycle, (DynamicObject *&) textResult))
                {
                    /*  Complete the texture access in the shader emulator.  */
                    shEmul.writeTextureAccess(textResult->getTextAccessID(), textResult->getTextSamples(), textThreads);

                    /*  Unblock the threads that finished the texture access.  */
                    for(j = 0; j < STAMP_FRAGMENTS; j++)
                    {
                        //  Unblock thread if thread hasn't processed END instruction
                        if (!threadInfo[textThreads[j]].end)
                            unblockThread(cycle, textThreads[j], 0, NULL);

                        //  Mark thread as no longer waiting for texture result.
                        threadInfo[textThreads[j]].waitTexture = false;

                        //  Check if thread has received the END instruction and finished all previous instructions.
                        if (threadInfo[textThreads[j]].end && (threadInfo[textThreads[j]].pending == 0))
                        {
                            /*  Remove thread end condition..  */
                            threadInfo[textThreads[j]].end = false;

                            /*  Remove thread block condition.  */
                            threadInfo[textThreads[j]].ready = true;

                            /*  Send END signal to Fetch.  */
                            endThread(cycle, textThreads[j], 0, NULL);
                        }
                    }

                    /*  Delete texture result object.  */
                    delete textResult;
                }
            }
        }
    }

//if (cycle > 71000)
//printf("ShDE %lld => textureUnits %d tickets[%d] = %d tuRequests %d\n",
//cycle, textureUnits, nextRequestTU, textureTickets[nextRequestTU], tuRequests);

    /*  Send decode/execute state to Shader Fetch.  As decode/execute blocking is
        not implemented we always send a ready signal.  */

    decodeState = (freeInstr >= 2)?SHDEC_READY:SHDEC_BUSY;

    decodeStateSignal->write(cycle, new ShaderDecodeStateInfo(decodeState));

}

/*  Ask the fetch stage to replay the current instruction because of a dependence.  */
void ShaderDecodeExecute::replayInstruction(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr)
{
    ShaderDecodeCommand *decodeCommand;

    /*  Ask the Shader Fetch to refetch the instruction later.  */
    decodeCommand = new ShaderDecodeCommand(REPEAT_LAST, numThread, PC);

    /*  Copy cookies from original instruction and add a new cookie.  */
    decodeCommand->copyParentCookies(*shExecInstr);
    decodeCommand->addCookie();

    /*  Send control command to fetch.  */
    controlSignal->write(cycle, decodeCommand) ;

    GPU_DEBUG_BOX(
//if (cycle > 73000)
        printf("ShaderDecodeExecute >> Sending REPEAT_LAST thread = %d PC = %x.\n", numThread, PC);
    )

    /*  Update statistics.  */
    replays->inc();
}

/*  Inform the fetch to the thread has finished.  */
void ShaderDecodeExecute::endThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr)
{
    ShaderDecodeCommand *decodeCommand;

    /*  Tell the Shader Fetch that the thread has finished.  */
    decodeCommand = new ShaderDecodeCommand(END_THREAD, numThread, PC);

    /*  Check if a dynamic instruction object was supplied.  */
    if (shExecInstr != NULL)
    {
        /*  Copy cookies from original instruction and add a new cookie.  */
        decodeCommand->copyParentCookies(*shExecInstr);
        decodeCommand->addCookie();
    }

    /*  Send control command to fetch.  */
    controlSignal->write(cycle, decodeCommand) ;

    GPU_DEBUG_BOX(
//if (cycle > 139500)
        printf("ShaderDecodeExecute => Sending END_THREAD thread = %d PC = %x.\n", numThread, PC);
    )

    /*  Update statistics.  */
    ends->inc();
}

//  Inform the fetch stage that a thread has executed a z export instruction.
void ShaderDecodeExecute::zExportThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr)
{
    ShaderDecodeCommand *decodeCommand;

    //  Construct the z export executed command.
    decodeCommand = new ShaderDecodeCommand(ZEXPORT_THREAD, numThread, PC);

    //  Check if a dynamic instruction object was supplied.
    if (shExecInstr != NULL)
    {
        //  Copy cookies from original instruction and add a new cookie.
        decodeCommand->copyParentCookies(*shExecInstr);
        decodeCommand->addCookie();
    }

    /*  Send control command to fetch.  */
    controlSignal->write(cycle, decodeCommand) ;

    GPU_DEBUG_BOX(
        printf("ShaderDecodeExecute => Sending ZEXPORT_THREAD thread = %d PC = %x.\n", numThread, PC);
    )

    //  Update statistics.
    zexports->inc();
}

/*  Make fetch block a thread.  */
void ShaderDecodeExecute::blockThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr)
{
    ShaderDecodeCommand *decodeCommand;

    /*  Check if thread was in ready state.  */
    if (threadInfo[numThread].ready)
    {
        /*  Set thread as blocked.  */
        threadInfo[numThread].ready = FALSE;

        /*  Ask the Shader Fetch to block the thread.  */
        decodeCommand = new ShaderDecodeCommand(BLOCK_THREAD, numThread, PC);

        /*  Copy cookies from original instruction and add a new cookie.  */
        decodeCommand->copyParentCookies(*shExecInstr);
        decodeCommand->addCookie();

        /*  Send control command to fetch.  */
        controlSignal->write(cycle, decodeCommand) ;

        GPU_DEBUG_BOX(
//if (cycle > 139500)
            printf("ShaderDecodeExecute => Sending BLOCK_THREAD thread = %d PC = %x.\n", numThread, PC);
        )

        /*  Update statistics.  */
        blocks->inc();
    }
}

/*  Unblock a thread in the fetch stage.  */
void ShaderDecodeExecute::unblockThread(u64bit cycle, u32bit numThread, u32bit PC, ShaderExecInstruction *shExecInstr)
{
    ShaderDecodeCommand *decodeCommand;

    if (!threadInfo[numThread].ready)
    {
        /*  Set thread as blocked.  */
        threadInfo[numThread].ready = TRUE;

        /*  Ask the Shader Fetch to unblock the thread.  */
        decodeCommand = new ShaderDecodeCommand(UNBLOCK_THREAD, numThread, PC);

        /*  Check if a dynamic instruction object was supplied.  */
        if (shExecInstr != NULL)
        {
            /*  Copy cookies from original instruction and add a new cookie.  */
            decodeCommand->copyParentCookies(*shExecInstr);
            decodeCommand->addCookie();
        }

        /*  Send control command to fetch.  */
        controlSignal->write(cycle, decodeCommand) ;

        GPU_DEBUG_BOX(
//if (cycle > 139500)
            printf("ShaderDecodeExecute => Sending UNBLOCK_THREAD thread = %d PC = %x.\n", numThread, PC);
        )

        /*  Update statistics.  */
        unblocks->inc();
    }
    else
    {
        panic("ShaderDecodeExecute", "unblockThread", "Unblocking ready thread.");
    }
}


/*  Processes a command from the Command Processor unit.  */
void ShaderDecodeExecute::processCommand(ShaderCommand *shCom)
{
    switch(shCom->getCommandType())
    {
        case RESET:

            GPU_DEBUG_BOX(
                printf("ShaderDecodeExecute => RESET command received.\n");
            )

            /*  Change shader decode execute to reset state.  */
            state = SH_RESET;

            break;

        default:
            panic("ShaderDecodeExecute", "processShaderCommand", "Unsupported shader command from the Command Processor.");
            break;
    }
}

/*  Processes the end of the execution of shader instructions.  */
void ShaderDecodeExecute::endExecution(u64bit cycle)
{
    ShaderExecInstruction *shExecInstr;
    ShaderInstruction *shInstr;
    ShaderInstruction::ShaderInstructionDecoded *shInstrDec;
    ShaderDecodeCommand *decodeCommand;
    u32bit numThread;
    u32bit pc;
    char dis[256];

    while(executionEndSignal->read(cycle, (DynamicObject *&) shExecInstr))
    {
        /*  Get the shader instruction.  */
        shInstrDec = &shExecInstr->getShaderInstruction();
        shInstr = shInstrDec->getShaderInstruction();

        /*  Get instruction shader thread number.  */
        numThread = shInstrDec->getNumThread();

        /*  Get shader thread PC after execution of the instruction.  */
        pc = shEmul.threadPC(numThread);

        GPU_DEBUG_BOX(
            shInstr->disassemble(&dis[0]);
            printf("ShaderDecodeExecute => End Execution Thread = %d PC = %x : %s\n",
                numThread, pc, dis);
        )

        /*  Emulate the instruction at the end of their execution latency.  */
        //shEmul.execShaderInstruction(shInstr);

        /*  Update number of pending instructions for the thread.  */
        threadInfo[numThread].pending--;

        /*  Check for KILL instruction.  */
        //if (shInstr->getOpcode() == KIL)
        //{
        //    /*  Check KILL instruction result.  */
        //    if (shEmul.threadKill(numThread))
        //    {
        //        /*  The thread must end.  */
        //        threadInfo[numThread].end = TRUE;

        //        /*  Check if there are pending instructions.  */
        //        if (threadInfo[numThread].pending > 0)
        //        {
        //            /*  Send BLOCK to Fetch.  */
        //            blockThread(cycle, numThread, pc, shExecInstr);
        //        }
        //        else
        //        {
        //            /*  Ignore any instruction for the thread received in the current and next
        //                cycle so we can end early and without blocking the thread.  */
        //        }
        //    }
        //}

        //  Check for Z exported condition.
        if (threadInfo[numThread].zexport)
        {
            //  Send Z EXPORT command to Fetch.
            zExportThread(cycle, numThread, pc, shExecInstr);

            //  Remove Z exported condition.
            threadInfo[numThread].zexport = false;
        }

        /*  Determine if all the shader instruction have finished.  */
        if (threadInfo[numThread].end && !threadInfo[numThread].waitTexture && (threadInfo[numThread].pending == 0))
        {
            /*  Remove thread end condition..  */
            threadInfo[numThread].end = FALSE;

            /*  Remove thread block condition.  */
            threadInfo[numThread].ready = TRUE;

            /*  Send END signal to Fetch.  */
            endThread(cycle, numThread, pc, shExecInstr);
        }

        /*  Determine if it is an END instruction.  */
        if (shInstr->isEnd())
        {
            /*  Nothing to do.  Wait for end of all pending instructions.   */
        }

        /*  Clear write dependences.  */
        clearDependences(cycle, shInstr, numThread);

        /*  Update statistics.  */
        execInstr->inc();

        /*  Check if the shader instruction isn't a texture load.  */
        if (!shInstr->isALoad())
        {
            /*  Delete the decoded shader instruction.  */
            delete shInstrDec;
        }

        /*  Delete the Shader Execution Instruction.  */
        delete shExecInstr;
    }
}

/*  Starts the execution of a shader instruction.  */
void ShaderDecodeExecute::startExecution(u64bit cycle, ShaderExecInstruction *shExecInstr)
{
    ShaderInstruction *shInstr;
    ShaderInstruction::ShaderInstructionDecoded *shInstrDec;
    u32bit instrLat;
    u32bit resReg;
    u32bit numThread;
    u32bit pc;
    char dis[256];

    /*  Get shader instruction.  */
    shInstrDec = &shExecInstr->getShaderInstruction();
    shInstr = shInstrDec->getShaderInstruction();

    /*  Get instruction shader thread number.  */
    numThread = shInstrDec->getNumThread();

    /*  Get shader thread PC.  */
    //pc = shEmul.threadPC(numThread);
    pc = shInstrDec->getPC();

    /*  Check if thread is blocked.  This happens for instructions starting after a texture instruction in the
        same cycle.   */
    //if (!threadInfo[numThread].ready)
    //{
        /*  Ignore any instruction from a blocked thread.  */

    //    GPU_DEBUG_BOX(
    //        printf("ShaderDecodeExecute => Ignoring instruction from blocked thread.\n");
    //    )
    //    return;
    //}

    /*  The latency is variable (per opcode).  The latency table stores the execution latency
        for each instruction opcode.  One cycle is added to simulate the write back stage.  */

    /*  Get instruction latency.  */
    //instrLat = latencyTable[shInstr->getOpcode()] + 1;
    instrLat = shArchParams->getExecutionLatency(shInstr->getOpcode()) + 1;

    GPU_ASSERT(
        if (instrLat > MAX_EXEC_LAT)
            panic("ShaderDecodeExecute", "startExecution", "Instruction execution latency larger than maximum execution latency.");
    )

    /*  Get result register number.  */
    resReg = shInstr->getResult();

    /*  If END instruction check that all previous instructions has finished.  */
    if (shInstr->isEnd())
    {
        /*  Set END received flag for the thread.  */
        threadInfo[numThread].end = TRUE;

        /*  Send a block signal to fetch.  */
        blockThread(cycle, numThread, pc, shExecInstr);
    }

    //  Check if the instruction is a Z export operation.
    if (shInstr->isZExport())
    {
        //  Set z export flag for the thread.
        threadInfo[numThread].zexport = true;
    }

    /*  Check if the instruction is a texture operation.  */
    if (shInstr->isALoad())
    {
        //  Set thread as waiting for texture result.
        threadInfo[numThread].waitTexture = true;

        /*  Consume a texture ticket.  */
        textureTickets[nextRequestTU]--;

        /*  Send a block signal to fetch.  */
        blockThread(cycle, numThread, pc, shExecInstr);
    }

    /*  Check if it is an instruction with a result.  */
    if (!shInstr->isEnd() && (shInstr->getOpcode() != NOP) && (shInstr->getOpcode() != KIL) && (shInstr->getOpcode() != KLS) && (shInstr->getOpcode() != ZXP) && (shInstr->getOpcode() != ZXS) && (shInstr->getOpcode() != CHS))
    {
        /*  Set pending register write tables.  */
        switch(shInstr->getBankRes())
        {
                case gpu3d::ADDR:

                    /*  Set pending status for address register.  */
                    threadInfo[numThread].addrBankDep[resReg] = TRUE;

                    break;

                case gpu3d::TEMP:

                    /*  Set pending status for temporary register.  */
                    threadInfo[numThread].tempBankDep[resReg] = TRUE;

                    break;

                case gpu3d::PRED:
                
                    //  Set pending status for predicate register.
                    threadInfo[numThread].predBankDep[resReg] = true;
                    
                    break;
                    
                case gpu3d::OUT:

                    /*  Set pending status for address register.  */
                    threadInfo[numThread].outpBankDep[resReg] = TRUE;

                    break;

                default:

                    panic("ShaderDecodeExecute", "startExecution", "Unsupported bank for register writes.");
                    break;
        }
    }

    /*  Start instruction execution.  */
    executionStartSignal->write(cycle, shExecInstr, instrLat);

    GPU_DEBUG_BOX(
        shInstr->disassemble(&dis[0]);

//if (cycle > 71000)
        printf("ShaderDecodeExecute => Execute Instr. Thread = %d PC = %x (Lat = %d) : %s\n",
            numThread, pc, instrLat - 1, dis);
    )

    /*  Emulate the instruction at the end of their execution latency.  */
    shEmul.execShaderInstruction(shInstrDec);

    /*  Update on execution instruction counter for the thread.  */
    threadInfo[numThread].pending++;

    /*  Check if it is an instruction with a result.  */
    if (!shInstr->isEnd() && (shInstr->getOpcode() != NOP) && (shInstr->getOpcode() != KIL) && (shInstr->getOpcode() != KLS) && (shInstr->getOpcode() != ZXP) && (shInstr->getOpcode() != ZXS) && (shInstr->getOpcode() != CHS))
    {
        /*  Set register write cycle tables.  */
        switch(shInstr->getBankRes())
        {
            case gpu3d::OUT:

                /*  If current write latency for the register is less than the instruction latency update the table.  */
                if (threadInfo[numThread].outpBankWrite[resReg] < (cycle + instrLat))
                {
                    threadInfo[numThread].outpBankWrite[resReg] = cycle + instrLat;
                }
                else
                    panic("ShaderDecodeExecute", "startExecution", "WAW dependence for output bank.");

                break;

            case gpu3d::ADDR:

                /*  If current write latency for the register is less than the instruction latency update the table.  */
               if (threadInfo[numThread].addrBankWrite[resReg] < (cycle + instrLat))
                {
                    threadInfo[numThread].addrBankWrite[resReg] = cycle + instrLat;
                }
                else
                    panic("ShaderDecodeExecute", "startExecution", "WAW dependence for address bank.");

                break;

            case gpu3d::TEMP:

                /*  If current write latency for the register is less than the instruction latency update the table.  */
                if (threadInfo[numThread].tempBankWrite[resReg] < (cycle + instrLat))
                {
                    threadInfo[numThread].tempBankWrite[resReg] = cycle + instrLat;
                }
                else
                    panic("ShaderDecodeExecute", "startExecution", "WAW dependence for temporary bank.");

                break;

            case gpu3d::PRED:

                //  If current write latency for the register is less than the instruction latency update the table.
                if (threadInfo[numThread].predBankWrite[resReg] < (cycle + instrLat))
                {
                    threadInfo[numThread].predBankWrite[resReg] = cycle + instrLat;
                }
                else
                    panic("ShaderDecodeExecute", "startExecution", "WAW dependence for predicate bank.");

                break;

            default:
                panic("ShaderDecodeExecute", "startExecution", "Unsupporte write bank.");
                break;
        }

    }

    /*  Check if SIMD + scalar mode enabled.  */
    if (scalarALU)
    {
        /*  Reserve SIMD or Scalar ALU.  */
        if (shInstr->isScalar())
            reserveScalarALU++;
        else
            reserveSIMDALU++;
    }

    /*
     *
     *  NOTE:  AS THE WRITE BW IS LIMITED BY THE EXECUTION SIGNAL BW INSTRUCTIONS THAT DON'T
     *  WRITE A RESULT REGISTER ARE STILL AFFECTED BY THE REGISTER WRITE PORT LIMITATIONS IN
     *  THE CURRENT IMPLEMENTATION.
     *
     */

    /*  Update register writes table.  */
    regWrites[GPU_MOD(nextRegWrite + instrLat, MAX_EXEC_LAT)]++;
}

/*  Blocks a shader instruction and sends replay command to shader fetch.  */
void ShaderDecodeExecute::repeatInstruction(u64bit cycle, ShaderExecInstruction *shExecInstr)
{
    ShaderInstruction *shInstr;
    ShaderInstruction::ShaderInstructionDecoded *shInstrDec;
    u32bit numThread;
    u32bit pc;
    char dis[256];

    /*  Get the instruction.  */
    shInstrDec = &shExecInstr->getShaderInstruction();
    shInstr = shInstrDec->getShaderInstruction();

    /*  Get the instruction thread ID.  */
    numThread = shInstrDec->getNumThread();

    /*  Check if the thread identifier is correct.  */
    GPU_ASSERT(
        if (numThread >= numThreads)
        {
            panic("ShaderDecodeExecute", "repeatInstruction", "Illegal thread number.");
        }
    )

    /*  Get instruction PC.  */
    pc = shInstrDec->getPC();

    GPU_DEBUG_BOX(
        shInstr->disassemble(&dis[0]);

        printf("ShaderDecodeExecute => Repeating Instr. Thread = %d  PC = %d : %s \n",
            numThread, pc, dis);
    )

    /*  Ask fetch to replay the instruction.  */
    replayInstruction(cycle, numThread, pc, shExecInstr);
}

/*  Decodes a shader instruction.  */
bool ShaderDecodeExecute::decodeInstruction(u64bit cycle, ShaderExecInstruction *shExecInstr, bool &repeat, bool block)
{
    ShaderInstruction *shInstr;
    ShaderInstruction::ShaderInstructionDecoded *shInstrDec;
    u32bit numThread;
    u32bit pc;
    u32bit instrLat;
    u32bit resReg;
    char dis[256];

    /*  Get the instruction.  */
    shInstrDec = &shExecInstr->getShaderInstruction();
    shInstr = shInstrDec->getShaderInstruction();

    /*  Get the instruction thread ID.  */
    numThread = shInstrDec->getNumThread();

    /*  Check if the thread identifier is correct.  */
    GPU_ASSERT(
        if (numThread >= numThreads)
        {
            panic("ShaderDecodeExecute", "decodeInstruction", "Illegal thread number.");
        }
    )

    /*  Get instruction PC.  */
    pc = shInstrDec->getPC();

    GPU_DEBUG_BOX(
        shInstr->disassemble(&dis[0]);

printf("ShDExec => %lld\n", cycle);
//if (cycle > 71000)
        printf("ShaderDecodeExecute => Decode Instr. Thread = %d  PC = %d : %s \n",
            numThread, pc, dis);
    )

    /*  Do not repeat the instruction.  */
    repeat = false;

    /*  If the instruction is a repeated one unset wait for repeated instruction flag.  */
    threadInfo[numThread].waitRepeated = threadInfo[numThread].waitRepeated && !shExecInstr->getRepeated();

    /*  Check if the instruction must be ignored because the last instruction for this thread had
        to be replayed in the previous cycle.  */
    if (threadInfo[numThread].waitRepeated)
    {
        /*  Ignore the instruction.  */

        GPU_DEBUG_BOX(
            printf("ShaderDecodeExecute => Ignoring instruction because of replay request last cycle.\n");
        )

        /*  Do not execute the instruction.  */
        return FALSE;
    }


    /*  Check branch dependencies.  Branch/call/ret must finish before a new instruction can be executed.  */
    /** NOT IMPLEMENTED **/

    /*  Check if an END instruction was decoded.  Ignore any further instructions different from END.  */
    if (threadInfo[numThread].end)
    {
        /*  Ignore any instruction after a thread receives END.  */

        GPU_DEBUG_BOX(
            printf("ShaderDecodeExecute => Ignoring instruction after END.\n");
        )

        /*  Do not execute the instruction.  */
        return FALSE;
    }

    /*  Check if thread is blocked.  */
    if (!threadInfo[numThread].ready)
   {
        /*  Ignore any instruction from a blocked thread.  */

        GPU_DEBUG_BOX(
            printf("ShaderDecodeExecute => Ignoring instruction from blocked thread.\n");
        )

        /*  Do not execute the instruction.  */
        return FALSE;
    }

    /*  Check if there is a texture ticket for the texture instruction.  */
    if (shInstr->isALoad() && (textureTickets[nextRequestTU] == 0))
    {
        GPU_DEBUG_BOX(
            printf("ShaderDecodeExecute => No texture ticket available for the texture instruction.\n");
        )

        /*  The instruction must be repeated.  */
        repeat = true;

        /*  Check if instruction must be repeated or blocked.  */
        if (!block)
        {
            /*  Wait for the repeated instruction.  */
            threadInfo[numThread].waitRepeated = true;
        }

        /*  Do not execute the instruction.  */
        return FALSE;
    }


    /*  Check if SIMD + scalar mode is enabled.  */
    if (scalarALU)
    {
        /*  Only allow one scalar op per cycle and thread rate.  */
        if (shInstr->isScalar() && reserveScalarALU)
        {
                GPU_DEBUG_BOX(
                    printf("ShaderDecodeExecute => No more ALU ops allowed in current cycle.\n");
                )

                /*  The instruction must be repeated.  */
                repeat = true;

                /*  Check if instruction must be repeated or blocked.  */
                if (!block)
                {
                    /*  Wait for the repeated instruction.  */
                    threadInfo[numThread].waitRepeated = true;
                }

                /*  Do not execute the instruction.  */
                return FALSE;
        }

        /*  Only allow one SIMD op per cycle and thread rate.  */
        if ((!shInstr->isScalar()) && reserveSIMDALU)
        {
                GPU_DEBUG_BOX(
                    printf("ShaderDecodeExecute => No more SIMD ops allowed in current cycle.\n");
                )

                /*  The instruction must be repeated.  */
                repeat = true;

                /*  Check if instruction must be repeated or blocked.  */
                if (!block)
                {
                    /*  Wait for the repeated instruction.  */
                    threadInfo[numThread].waitRepeated = true;
                }

                /*  Do not execute the instruction.  */
                return FALSE;
        }
    }

    /*  Check register dependence table.  */

    /*  Check RAW dependences for first source operand.  */
    if (shInstr->getNumOperands() > 0)
    {
        /*  Check RAW dependence.  */
        if (checkRAWDependence(numThread, shInstr->getBankOp1(), shInstr->getOp1()))
        {
            GPU_DEBUG_BOX(
                printf("ShaderDecodeExecute => RAW dependence found for first operand.\n");
            )

            /*  The instruction must be repeated/blocked.  */
            repeat = true;

            /*  Check if instruction must be repeated or blocked.  */
            if (!block)
            {
                /*  Wait for the repeated instruction.  */
                threadInfo[numThread].waitRepeated = true;
            }

            /*  Do not execute the instruction.  */
            return FALSE;
        }
    }


    /*  Check RAW dependences for second source operand.  */
    if (shInstr->getNumOperands() > 1)
    {
        /*  Check RAW dependence.  */
        if (checkRAWDependence(numThread, shInstr->getBankOp2(), shInstr->getOp2()))
        {
            GPU_DEBUG_BOX(
                printf("ShaderDecodeExecute => RAW dependence found for second operand.\n");
            )

            /*  The instruction must be repeated.  */
            repeat = true;

            /*  Check if instruction must be repeated or blocked.  */
            if (!block)
            {
                /*  Wait for the repeated instruction.  */
                threadInfo[numThread].waitRepeated = true;
            }


            /*  Do not execute the instruction.  */
            return FALSE;
        }
    }

    /*  Check RAW dependences for third source operand.  */
    if (shInstr->getNumOperands() > 2)
    {
        /*  Check RAW dependence.  */
        if (checkRAWDependence(numThread, shInstr->getBankOp3(), shInstr->getOp3()))
        {
            GPU_DEBUG_BOX(
                printf("ShaderDecodeExecute => RAW dependence found for third operand.\n");
            )

            /*  The instruction must be repeated.  */
            repeat = true;

            /*  Check if instruction must be repeated or blocked.  */
            if (!block)
            {
                /*  Wait for the repeated instruction.  */
                threadInfo[numThread].waitRepeated = true;
            }

            /*  Do not execute the instruction.  */
            return FALSE;
        }
    }

    /*  Check RAW dependences with the address register used in relative mode access to the
        constant bank.  */
    if (shInstr->getRelativeModeFlag())
    {
        /*  Check RAW dependence.  */
        if (checkRAWDependence(numThread, ADDR, shInstr->getRelMAddrReg()))
        {
            GPU_DEBUG_BOX(
                printf("ShaderDecodeExecute => RAW dependence for address register %d in relative access mode to the constant bank.\n",
                    shInstr->getRelMAddrReg());
            )

            /*  The instruction must be repeated.  */
            repeat = true;

            /*  Check if instruction must be repeated or blocked.  */
            if (!block)
            {
                /*  Wait for the repeated instruction.  */
                threadInfo[numThread].waitRepeated = true;
            }

            /*  Do not execute the instruction.  */
            return FALSE;
        }
    }


    /*  Get instruction latency.  */
    //instrLat = latencyTable[shInstr->getOpcode()] + 1;
    instrLat = shArchParams->getExecutionLatency(shInstr->getOpcode()) + 1;

    /*  Check if it is an instruction with a result.  */
    if (!shInstr->isEnd() && (shInstr->getOpcode() != NOP) && (shInstr->getOpcode() != KIL) &&
        (shInstr->getOpcode() != KLS) && (shInstr->getOpcode() != ZXP) && (shInstr->getOpcode() != ZXS) && (shInstr->getOpcode() != CHS))
    {
        /*  Get Result register.  */
        resReg = shInstr->getResult();

        /*  Check WAW dependences:
            If the current write latency for the result register of the instruction is
            larger than the instruction execution latency the execution of the instruction
            must wait.  */
        switch(shInstr->getBankRes())
        {
            case gpu3d::OUT:

                /*  Check WAW dependence for output bank register.  */
                if (threadInfo[numThread].outpBankWrite[resReg] >= (cycle + instrLat))
                {
                    GPU_DEBUG_BOX(
                        printf("ShaderDecodeExecute => WAW dependence for OUTPUT register %d.\n", resReg);
                    )

                    /*  The instruction must be repeated.  */
                    repeat = true;

                    /*  Check if instruction must be repeated or blocked.  */
                    if (!block)
                    {
                        /*  Wait for the repeated instruction.  */
                        threadInfo[numThread].waitRepeated = true;
                    }

                    /*  Do not execute the instruction.  */
                    return FALSE;
                }

                break;

            case gpu3d::ADDR:

                /*  Check WAW dependence for address bank register.  */
                if (threadInfo[numThread].addrBankWrite[resReg] >= (cycle + instrLat))
                {
                    GPU_DEBUG_BOX(
                        printf("ShaderDecodeExecute => WAW dependence for ADDRESS register %d.\n", resReg);
                    )

                    /*  The instruction must be repeated.  */
                    repeat = true;

                    /*  Check if instruction must be repeated or blocked.  */
                    if (!block)
                    {
                        /*  Wait for the repeated instruction.  */
                        threadInfo[numThread].waitRepeated = true;
                    }

                    /*  Do not execute the instruction.  */
                    return FALSE;
                }

                break;

            case gpu3d::TEMP:

                /*  Check WAW dependence for temporal bank register.  */
                if (threadInfo[numThread].tempBankWrite[resReg] >= (cycle + instrLat))
                {
                    GPU_DEBUG_BOX(
                        printf("ShaderDecodeExecute => WAW dependence for TEMPORAL register %d.\n", resReg);
                    )

                    /*  The instruction must be repeated.  */
                    repeat = true;

                    /*  Check if instruction must be repeated or blocked.  */
                    if (!block)
                    {
                        /*  Wait for the repeated instruction.  */
                        threadInfo[numThread].waitRepeated = true;
                    }

                    /*  Do not execute the instruction.  */
                    return FALSE;
                }

                break;

            case gpu3d::PRED:

                //  Check WAW dependence for predicate bank register.
                if (threadInfo[numThread].predBankWrite[resReg] >= (cycle + instrLat))
                {
                    GPU_DEBUG_BOX(
                        printf("ShaderDecodeExecute => WAW dependence for PREDICATE register %d.\n", resReg);
                    )

                    //  The instruction must be repeated.
                    repeat = true;

                    //  Check if instruction must be repeated or blocked.
                    if (!block)
                    {
                        //  Wait for the repeated instruction.
                        threadInfo[numThread].waitRepeated = true;
                    }

                    //  Do not execute the instruction.
                    return FALSE;
                }

                break;
        }
    }

    /*
     *
     *  NOTE:  AS THE WRITE BW IS LIMITED BY THE EXECUTION SIGNAL BW INSTRUCTIONS THAT DON'T
     *  WRITE A RESULT REGISTER ARE STILL AFFECTED BY THE REGISTER WRITE PORT LIMITATIONS IN
     *  THE CURRENT IMPLEMENTATION.
     *
     */

    /*  Check register write ports availability.  */
    if (regWrites[GPU_MOD(nextRegWrite + instrLat, MAX_EXEC_LAT)] >= (MAX_EXEC_BW * threadsCycle * instrCycle))
    {
       GPU_DEBUG_BOX(
            printf("ShaderDecodeExecute => Register write port limit reached.\n");
       )

        /*  The instruction must be repeated.  */
        repeat = true;

        /*  Check if instruction must be repeated or blocked.  */
        if (!block)
        {
            /*  Wait for the repeated instruction.  */
            threadInfo[numThread].waitRepeated = true;
        }

        /*  Do not execute the instruction.  */
        return FALSE;
    }

    /*  Execute the instruction.  */
    return TRUE;
}

/*  Checks if the register bank write ports are available for the instruction.  */
bool ShaderDecodeExecute::checkWritePorts(u64bit cycle, ShaderExecInstruction *shExecInstr)
{
    ShaderInstruction *shInstr;
    ShaderInstruction::ShaderInstructionDecoded *shInstrDec;
    u32bit numThread;
    u32bit pc;
    u32bit instrLat;
    char dis[256];

    /*  Get the instruction.  */
    shInstrDec = &shExecInstr->getShaderInstruction();
    shInstr = shInstrDec->getShaderInstruction();

    /*  Get the instruction thread ID.  */
    numThread = shInstrDec->getNumThread();

    /*  Check if the thread identifier is correct.  */
    GPU_ASSERT(
        if (numThread >= numThreads)
        {
            panic("ShaderDecodeExecute", "checkWritePorts", "Illegal thread number.");
        }
    )

    /*  Get instruction PC.  */
    pc = shInstrDec->getPC();

    GPU_DEBUG_BOX(
        shInstr->disassemble(&dis[0]);

        printf("ShaderDecodeExecute => Check write ports Instr. Thread = %d  PC = %d : %s.\n",
            numThread, pc, dis);
    )

    /*
     *
     *  NOTE:  AS THE WRITE BW IS LIMITED BY THE EXECUTION SIGNAL BW INSTRUCTIONS THAT DON'T
     *  WRITE A RESULT REGISTER ARE STILL AFFECTED BY THE REGISTER WRITE PORT LIMITATIONS IN
     *  THE CURRENT IMPLEMENTATION.
     *
     */

    /*  Get instruction latency.  */
    //instrLat = latencyTable[shInstr->getOpcode()] + 1;
    instrLat = shArchParams->getExecutionLatency(shInstr->getOpcode()) + 1;

    /*  Check register write ports availability.  */
    if (regWrites[GPU_MOD(nextRegWrite + instrLat, MAX_EXEC_LAT)] >= (MAX_EXEC_BW * threadsCycle * instrCycle))
    {
       GPU_DEBUG_BOX(
            printf("ShaderDecodeExecute => Register write port limit reached.\n");
       )

        /*  Wait for the repeated instruction.  */
        threadInfo[numThread].waitRepeated = true;

        /*  Do not execute the instruction.  */
        return FALSE;
    }

    return TRUE;
}
/*  Determines if there is a RAW dependence for a register.  */
bool ShaderDecodeExecute::checkRAWDependence(u32bit numThread, Bank bank, u32bit reg)
{
    bool dependenceFound;
    
    switch(bank)
    {
        case TEMP:
        
            //  Check if there are pending writes for the temporary register. 
            dependenceFound = threadInfo[numThread].tempBankDep[reg];
            break;
      
        case ADDR:
        
            //  Check if there are pending writes for the address register. 
            dependenceFound = threadInfo[numThread].addrBankDep[reg];            
            break;
            
        case PRED:
        
            //  Check if there are pending writes for the predicate register. 
            dependenceFound = threadInfo[numThread].predBankDep[reg];
            break;
            
        default:

            //  All other types of register are not writeable.
            dependenceFound = false;
            break;
    }

    return dependenceFound;
}

//  Clears register write dependences.
void ShaderDecodeExecute::clearDependences(u64bit cycle, ShaderInstruction *shInstr, u32bit threadID)
{
    //  Update pending registers write tables.

    //  Check if the instruction writes a register.
    if (!shInstr->isEnd() && (shInstr->getOpcode() != NOP) && (shInstr->getOpcode() != KIL) && (shInstr->getOpcode() != KLS) && (shInstr->getOpcode() != ZXP) && (shInstr->getOpcode() != ZXS) && (shInstr->getOpcode() != CHS))
    {
        //  Get the result register.
        u32bit resReg = shInstr->getResult();
        
        //  Determine register bank written by the instruction.
        switch(shInstr->getBankRes())
        {
            case gpu3d::ADDR:

                //  Check if clearing the last write pending for the address register.
                if (threadInfo[threadID].addrBankWrite[resReg] == cycle)
                {
                    //  Remove pending status for the address register.
                    threadInfo[threadID].addrBankDep[resReg] = false;
                }
                
                break;

            case gpu3d::TEMP:

                //  Check if clearing the last write pending for the temporal register.
                if (threadInfo[threadID].tempBankWrite[resReg] == cycle)
                {
                    //  Remove pending status for the temporal register.
                    threadInfo[threadID].tempBankDep[resReg] = false;
                }
                
                break;

            case gpu3d::PRED:

                //  Check if clearing the last write pending for the predicate register.
                if (threadInfo[threadID].predBankWrite[resReg] == cycle)
                {
                    //  Remove pending status for the predicate register.
                    threadInfo[threadID].predBankDep[resReg] = false;
                }
                
                break;

            case gpu3d::OUT:

                //  Check if clearing the last write pending for the output register.
                if (threadInfo[threadID].outpBankWrite[resReg] == cycle)
                {
                    //  Remove pending status for the output register.
                    threadInfo[threadID].outpBankDep[resReg] = false;
                }
                
                break;

            default:

                panic("ShaderDecodeExecute", "clearDependences", "Unsupported bank for register writes.");
                break;
        }
    }
}


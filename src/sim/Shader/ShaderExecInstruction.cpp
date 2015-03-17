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
 * $RCSfile: ShaderExecInstruction.cpp,v $
 * $Revision: 1.6 $
 * $Author: vmoya $
 * $Date: 2005-11-11 15:41:48 $
 *
 * Shader Execution Instruction.
 *
 */


#include "ShaderExecInstruction.h"
#include <cstring>
#include <stdio.h>

/**
 *
 *  @file ShaderExecInstruction.cpp
 *
 *  Implements the ShaderExecInstruction class members and
 *  functions.  This class describes the instance of an
 *  instruction while it is being executed through the
 *  shader pipeline.
 *
 */

using namespace gpu3d;


//  Constructor to be used with original Shader
ShaderExecInstruction::ShaderExecInstruction(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, u32bit pcInstr,
    u64bit startCycle, bool repeat, bool fakedFetch) :

    instr(shInstrDec), pc(pcInstr), cycle(startCycle), repeated(repeat), fake(fakedFetch), trace(false)
{
    ShaderInstruction *shInstr = instr.getShaderInstruction();

    state = FETCH;

    //  Set color for tracing.
    setColor(shInstr->getOpcode());

    //  Set instruction disassemble as info.
    if (!fake)
        shInstr->disassemble((char *) getInfo());
    else
        strcpy((char *) getInfo(), "FAKE INSTRUCTION");

    setTag("ShExIns");
}

//  Constructor to be used by VectorShader
ShaderExecInstruction::ShaderExecInstruction(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, u32bit thread, u32bit elem, 
    u32bit pcInstr, u64bit startCycle, bool fakedFetch) :
    
    instr(shInstrDec), threadID(thread), element(elem), pc(pcInstr), cycle(startCycle), repeated(false), fake(fakedFetch), trace(false)
{
    ShaderInstruction *shInstr = instr.getShaderInstruction();

    state = FETCH;

    //  Set color for tracing.
    setColor(shInstr->getOpcode());

    //  Set instruction disassemble as info.
    char disasmInstr[255];

    if (!fake)
    {
        shInstr->disassemble(disasmInstr);
        sprintf((char *) getInfo(), "ThID %03d Elem %03d PC h%04x -> %s", threadID, element, pc, disasmInstr);
    }
    else
        sprintf((char *) getInfo(), "ThID %03d Elem %03d PC h%04x -> FAKE INSTRUCTION", threadID, element, pc);

    
    setTag("ShExIns");

}

/*  Return the Shader Instruction of the dynamic instruction.  */
ShaderInstruction::ShaderInstructionDecoded &ShaderExecInstruction::getShaderInstruction()
{
    return instr;
}

/*  Returns the dynamic shader instruction start cycle (creation/fetch).  */
u64bit ShaderExecInstruction::getStartCycle()
{
    return cycle;
}

/*  Returns the dynamic shader instruction PC.  */
u32bit ShaderExecInstruction::getPC()
{
    return pc;
}

/*  Returns if the instruction is being replayed.  */
bool ShaderExecInstruction::getRepeated()
{
    return repeated;
}

/*  Returns the dynamic shader instruction state.  */
ExecInstrState ShaderExecInstruction::getState()
{
    return state;
}

/*  Returns a reference to the result from the instruction.  */
QuadFloat &ShaderExecInstruction::getResult()
{
    return result;
}

/*  Changes the state of the instruction.  */
void ShaderExecInstruction::changeState(ExecInstrState newState)
{
    state = newState;
}

/*  Sets the instruction result.  */
void ShaderExecInstruction::setResult(const QuadFloat &res)
{
    result = res;
}

/*  Gets if the instruction is faked.  */
bool ShaderExecInstruction::getFakeFetch() const
{
    return fake;
}

//  Gets if the instruction execution must be traced.
bool ShaderExecInstruction::getTraceExecution()
{
    return trace;
}

//  Enable tracing the execution of the instruction.
void ShaderExecInstruction::enableTraceExecution()
{
    trace = true;
}

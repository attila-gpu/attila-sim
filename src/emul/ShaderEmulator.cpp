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
 * $RCSfile: ShaderEmulator.cpp,v $
 * $Revision: 1.30 $
 * $Author: csolis $
 * $Date: 2008-05-31 14:50:41 $
 *
 * Shader Unit Emulator.
 *
 */

/**
 *
 * /file ShaderEmulator.cpp
 *
 *  Implementes the ShaderEmulator class.  This class provides services
 *  to emulate the functional behaviour of a Shader unit.
 *
 *
 */

#include "ShaderEmulator.h"
#include <cstring>
#include <cstdio>
#include <sstream>
#include "FixedPoint.h"
//#include "DebugDefinitions.h"

/*
 *  Shader Emulator constructor.
 *
 *  This function creates a multithreaded Shader Emulator of the
 *  demanded model.
 *
 */

using namespace gpu3d;

ShaderEmulator::ShaderEmulator(char *nameShader, ShaderModel shaderModel, u32bit numThreadsShader,
    bool storeDecInstr, TextureEmulator *textEmul, u32bit stampFrags, u32bit fxpDecBits) :

    textEmu(textEmul), stampFragments(stampFrags), fxpDecBits(fxpDecBits), model(shaderModel),
    numThreads(numThreadsShader), storeDecodedInstr(storeDecInstr)

{
    //  Name and type of the shader are copied to the object attributes.
    name = new char[strlen(nameShader) + 1];
    strcpy(name, nameShader);

    //  Allocate the PC table and initialize it.
    PCTable = new u32bit[numThreads];

    //  Check allocation.
    
    GPU_ASSERT(
        if (PCTable == NULL)
            panic("ShaderEmulator", "ShaderEmulator", "Error creating array of per thread program counters (PC).");
    )

    //  Initialize the array of per-thread program counters.    
    for(u32bit t = 0; t < numThreads; t++)
        PCTable[t] = 0;

    //  Allocate structures (register banks and instruction memory)
    //  for the type of shader defined.
    switch(model)
    {
        case UNIFIED:

            //  Instruction Memory.
            instructionMemory = new ShaderInstruction*[2 * UNIFIED_INSTRUCTION_MEMORY_SIZE];
            
            //  Check memory allocation.
            GPU_ASSERT(
                if (instructionMemory == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating instruction memory");
            )
            
            for(u32bit i = 0; i < (2 * UNIFIED_INSTRUCTION_MEMORY_SIZE); i++)
                instructionMemory[i] = new ShaderInstruction(INVOPC);

            //  Check if storing decoded instructions is enabled.
            if (storeDecodedInstr)
            {
                decodedInstructions = new ShaderInstruction::ShaderInstructionDecoded**[2 * UNIFIED_INSTRUCTION_MEMORY_SIZE];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (decodedInstructions == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating decoded instruction array");
                )

                for(u32bit i = 0; i < (2 * UNIFIED_INSTRUCTION_MEMORY_SIZE); i++)
                {
                    decodedInstructions[i] = new ShaderInstruction::ShaderInstructionDecoded*[numThreads];

                    //  Check memory allocation.
                    GPU_ASSERT(
                        if (decodedInstructions[i] == NULL)
                            panic("ShaderEmulator", "ShaderEmulator", "Error allocating decoded instruction array");
                    )

                    //  There is no shader program yet.  Set to NULL.
                    for(u32bit j = 0; j < numThreads; j++)
                        decodedInstructions[i][j] = NULL;
                }
            }
            else
            {
                decodedInstructions = NULL;
            }
            
            instructionMemorySize = 2 * UNIFIED_INSTRUCTION_MEMORY_SIZE;

            //  Input register bank.
            inputBank = new QuadFloat*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (inputBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating input register bank");
            )

            for(u32bit i = 0; i < numThreads; i++)
            {
                inputBank[i] = new QuadFloat[UNIFIED_INPUT_NUM_REGS];
                
                //  Check memory allocation.
                GPU_ASSERT(
                    if (inputBank[i] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating input register bank");
                )
            }
            
            numInputRegs = UNIFIED_INPUT_NUM_REGS;

            //  Output register bank.
            outputBank = new QuadFloat*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (outputBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating output register bank");
            )

            for(u32bit i = 0; i < numThreads; i++)
            {
                outputBank[i] = new QuadFloat[UNIFIED_OUTPUT_NUM_REGS];
                
                //  Check memory allocation.
                GPU_ASSERT(
                    if (outputBank[i] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating output register bank");
                )
            }
            
            numOutputRegs = UNIFIED_OUTPUT_NUM_REGS;

            //  Temporary register bank.
            temporaryBank = new u8bit[numThreads * UNIFIED_TEMPORARY_NUM_REGS * UNIFIED_TEMP_REG_SIZE] ;

            //  Check memory allocation.
            GPU_ASSERT(
                if (temporaryBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating temporary register bank");
            )

            numTemporaryRegs = UNIFIED_TEMPORARY_NUM_REGS;

            //  Constant register bank.
            constantBank = new u8bit[2 * UNIFIED_CONSTANT_NUM_REGS * UNIFIED_CONST_REG_SIZE];

            //  Check memory allocation.
            GPU_ASSERT(
                if (constantBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating constant register bank");
            )
            
            numConstantRegs = 2 * UNIFIED_CONSTANT_NUM_REGS;

            //  Address Register bank.
            addressBank = new QuadInt*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (addressBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating address register bank");
            )

            for(u32bit i = 0; i < numThreads; i++)
            {
                addressBank[i] = new QuadInt[UNIFIED_ADDRESS_NUM_REGS];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (addressBank[i] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating address register bank");
                )
            }
                            
            numAddressRegs = UNIFIED_ADDRESS_NUM_REGS;

            //  Predicate Register bank.
            predicateBank = new bool*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (predicateBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating predicate register bank");
            )

            for(u32bit t = 0; t < numThreads; t++)
            {
                predicateBank[t] = new bool[UNIFIED_PREDICATE_NUM_REGS];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (predicateBank[t] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating predicate register bank");
                )
            }
            
            numPredicateRegs = UNIFIED_PREDICATE_NUM_REGS;
            
            break;

        case UNIFIED_MICRO:

            //  Instruction Memory.
            instructionMemory = new ShaderInstruction*[2 * UNIFIED_INSTRUCTION_MEMORY_SIZE];
            
            //  Check memory allocation.
            GPU_ASSERT(
                if (instructionMemory == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating instruction memory");
            )
            
            for(u32bit i = 0; i < (2 * UNIFIED_INSTRUCTION_MEMORY_SIZE); i++)
                instructionMemory[i] = new ShaderInstruction(INVOPC);

            //  Check if storing decoded instructions is enabled.
            if (storeDecodedInstr)
            {
                decodedInstructions = new ShaderInstruction::ShaderInstructionDecoded**[2 * UNIFIED_INSTRUCTION_MEMORY_SIZE];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (decodedInstructions == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating decoded instruction array");
                )

                for(u32bit i = 0; i < (2 * UNIFIED_INSTRUCTION_MEMORY_SIZE); i++)
                {
                    decodedInstructions[i] = new ShaderInstruction::ShaderInstructionDecoded*[numThreads];

                    //  Check memory allocation.
                    GPU_ASSERT(
                        if (decodedInstructions[i] == NULL)
                            panic("ShaderEmulator", "ShaderEmulator", "Error allocating decoded instruction array");
                    )

                    //  There is no shader program yet.  Set to NULL.
                    for(u32bit j = 0; j < numThreads; j++)
                        decodedInstructions[i][j] = NULL;
                }
            }
            else
            {
                decodedInstructions = NULL;
            }
            
            instructionMemorySize = 2 * UNIFIED_INSTRUCTION_MEMORY_SIZE;

            //  Shader Input register bank.
            inputBank = new QuadFloat*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (inputBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating input register bank");
            )

            for(u32bit i = 0; i < numThreads; i++)
            {
                inputBank[i] = new QuadFloat[MAX_MICROFRAGMENT_ATTRIBUTES];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (inputBank[i] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating input register bank");
                )
            }
            
            numInputRegs = MAX_MICROFRAGMENT_ATTRIBUTES;

            //  Output register bank.
            outputBank = new QuadFloat*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (outputBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating output register bank");
            )

            for(u32bit i = 0; i < numThreads; i++)
            {
                outputBank[i] = new QuadFloat[UNIFIED_OUTPUT_NUM_REGS];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (outputBank[i] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating output register bank");
                )
            }
            
            numOutputRegs = UNIFIED_OUTPUT_NUM_REGS;

            //  Temporary register bank.
            temporaryBank = new u8bit[numThreads * UNIFIED_TEMPORARY_NUM_REGS * UNIFIED_TEMP_REG_SIZE] ;
            
            //  Check memory allocation.
            GPU_ASSERT(
                if (temporaryBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating temporary register bank");
            )

            numTemporaryRegs = UNIFIED_TEMPORARY_NUM_REGS;
            

            //  Constant register bank.
            constantBank = new u8bit[2 * UNIFIED_CONSTANT_NUM_REGS * UNIFIED_CONST_REG_SIZE];

            //  Check memory allocation.
            GPU_ASSERT(
                if (constantBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating constant register bank");
            )

            numConstantRegs = 2 * UNIFIED_CONSTANT_NUM_REGS;

            //  Address Register bank.
            addressBank = new QuadInt*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (addressBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating address register bank");
            )
            
            for(u32bit i = 0; i < numThreads; i++)
            {
                addressBank[i] = new QuadInt[UNIFIED_ADDRESS_NUM_REGS];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (addressBank[i] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating address register bank");
                )
            }
            numAddressRegs = UNIFIED_ADDRESS_NUM_REGS;

            //  Special Fixed Point accumulator bank used for rasterization.
            accumFXPBank = new FixedPoint*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (accumFXPBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating fixed point accumlator bank");
            )

            for(u32bit i = 0; i < numThreads; i++)
            {
                accumFXPBank[i] = new FixedPoint[4];


                //  Check memory allocation.
                GPU_ASSERT(
                    if (accumFXPBank[i] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating fixed point accumlator bank");
                )
            }
            
            //  Predicate Register bank.
            predicateBank = new bool*[numThreads];

            //  Check memory allocation.
            GPU_ASSERT(
                if (predicateBank == NULL)
                    panic("ShaderEmulator", "ShaderEmulator", "Error allocating address register bank");
            )

            for(u32bit t = 0; t < numThreads; t++)
            {
                predicateBank[t] = new bool[UNIFIED_PREDICATE_NUM_REGS];

                //  Check memory allocation.
                GPU_ASSERT(
                    if (predicateBank[t] == NULL)
                        panic("ShaderEmulator", "ShaderEmulator", "Error allocating address register bank");
                )
            }
            
            numPredicateRegs = UNIFIED_PREDICATE_NUM_REGS;

            break;

        default:

            panic("ShaderEmulator", "ShaderEmulator", "Undefined Shader Model");
            break;
    }

    //  Initialize the per-sample kill flag array for all the threads.
    kill = new bool*[numThreads];

    //  Check allocation.
    GPU_ASSERT(
        if (kill == NULL)
            panic("ShaderEmulator", "ShaderEmulator", "Error allocating kill flag per thread array.");
    )

    //  Initialize each thread's per-sample kill flag array.
    for(u32bit i = 0; i < numThreads; i++)
    {
        kill[i] = new bool[MAX_MSAA_SAMPLES];

        //  Check allocation.
        GPU_ASSERT(
            if (kill[i] == NULL)
                panic("ShaderEmulator", "ShaderEmulator", "Error allocating kill flag per thread array.");
        )

        // Initialize per-thread flag array.
        for (u32bit j = 0; j < MAX_MSAA_SAMPLES; j++)
        {
            kill[i][j] = false;
        }
    }

    //  Initialize the zexport thread (per sample) array.
    zexport = new f32bit*[numThreads];

    //  Check allocation.
    GPU_ASSERT(
        if (zexport == NULL)
            panic("ShaderEmulator", "ShaderEmulator", "Error allocating zexport array.");
    )

    //  Initialize each thread's per-sample zexport array.
    for(u32bit i = 0; i < numThreads; i++)
    {
        zexport[i] = new f32bit[MAX_MSAA_SAMPLES];

        //  Check allocation.
        GPU_ASSERT(
            if (zexport[i] == NULL)
                panic("ShaderEmulator", "ShaderEmulator", "Error allocating per-thread zexport array.");
        )

        // Initialize per-thread zexport array.
        for (u32bit j = 0; j < MAX_MSAA_SAMPLES; j++)
        {
            zexport[i][j] = 0.0f;
        }
    }

    //  Initialize the sampleIdx thread array.
    sampleIdx = new u32bit[numThreads];

    //  Check allocation.
    GPU_ASSERT(
        if (sampleIdx == NULL)
            panic("ShaderEmulator", "ShaderEmulator", "Error allocating sample idx array.");
    )

    // Initialize per-thread sampleIdx array.
    for (u32bit i = 0; i < numThreads; i++)
    {
        sampleIdx[i] = 0;
    }

    //  Initialize texture queue state.
    numFree = TEXT_QUEUE_SIZE;
    numWait = 0;
    firstFree = lastFree = firstWait = lastWait = 0;

    //  Initialize list of free entries in the texture queue.
    for(u32bit i = 0; i < TEXT_QUEUE_SIZE; i++)
    {
        freeTexture[i] = i;
    }

    //  Initialize Texture queue structures.
    for(u32bit i = 0; i < TEXT_QUEUE_SIZE; i++)
    {
        //  Allocate structures for a whole stamp.
        textQueue[i].shInstrD = new ShaderInstruction::ShaderInstructionDecoded*[stampFragments];
        textQueue[i].coordinates = new QuadFloat[stampFragments];
        textQueue[i].parameter = new f32bit[stampFragments];
        //textQueue[i].accessCookies = new DynamicObject;

        //  Check allocation.
        GPU_ASSERT(
            if (textQueue[i].shInstrD == NULL)
                panic("ShaderEmulator", "ShaderEmulator", "Error allocating array of instructions for texture queue entry.");
            if (textQueue[i].coordinates == NULL)
                panic("ShaderEmulator", "ShaderEmulator", "Error allocating array of texture coordinates for texture queue entry.");
            if (textQueue[i].parameter == NULL)
                panic("ShaderEmulator", "ShaderEmulator", "Error allocating array of per fragment parameter (lod/bias) for texture queue entry.");
            //if (textQueue[i].accessCookies == NULL)
            //    panic("ShaderEmulator", "ShaderEmulator", "Error allocating texture access cookies container.");
        )

        //  Reset requests counter.
        textQueue[i].requested = 0;
    }
    
    //  Initialize the current derivation operation structure.
    currentDerivation.baseThread = 0;
    currentDerivation.derived = 0;
    currentDerivation.shInstrD = new ShaderInstruction::ShaderInstructionDecoded*[4];
}

//  Resets the thread shader state.
void ShaderEmulator::resetShaderState(u32bit numThread)
{
    QuadFloat auxQF;
    QuadInt auxQI;

    //  Check if it is a valid thread number.
    GPU_ASSERT(
        if (numThread >= numThreads)
            panic("ShaderEmulator", "resetShaderState", "Incorrect thread number.");
    )

    //  Reset thread prougraom C to zero.
    PCTable[numThread] = 0;

    //  Load Input Bank default value.
    auxQF[0] =  auxQF[1] =  auxQF[2] =  auxQF[3] = 0.0f;

    //  Write default value in all Input Bank registers.
    for(u32bit r = 0; r < numInputRegs; r++)
        inputBank[numThread][r] = auxQF;

    //  Load Output Bank default value.
    auxQF[0] =  auxQF[1] =  auxQF[2] =  auxQF[3] = 0.0f;

    //  Write default value in all Output Bank registers.
    for(u32bit r = 0; r < numOutputRegs; r++)
        outputBank[numThread][r] = auxQF;

    //  Set temporary register bank to 0.
    memset(&temporaryBank[numThread * numTemporaryRegs * UNIFIED_TEMP_REG_SIZE], 0, numTemporaryRegs * UNIFIED_TEMP_REG_SIZE);

    //  Load the address register bank default value.
    auxQI[0] = auxQI[1] = auxQI[2] = auxQI[3] = 0;

    //  Write default value in all the Address Bank registers.
    for(u32bit r = 0; r < numAddressRegs; r++)
        addressBank[numThread][r] = auxQI;

    //  Reset thread per-sample kill flag.
    for(u32bit s = 0; s < MAX_MSAA_SAMPLES; s++)
        kill[numThread][s] = false;

    //  Reset thread per-sample zexport values.
    for(u32bit s = 0; s < MAX_MSAA_SAMPLES; s++)
        zexport[numThread][s] = 0.0f;

    //  Reset thread´s sample index value.
    sampleIdx[numThread] = 0;
}

//  Loads a value into one of the thread registers.  QuadFloat version.
void ShaderEmulator::loadShaderState(u32bit numThread, Bank bank, u32bit reg, QuadFloat data)
{
    //  Load the register in the appropiate register bank.
    switch(bank)
    {
        case IN:
        
            //  Check the range of the register index.
            GPU_ASSERT(
                if (reg >= numInputRegs)
                    panic("ShaderEmulator", "loadShaderState(QuadFloat)", "Incorrect register number.");
            )

            //  Check the range of the thread index.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "loadShaderState(QuadFloat)", "Illegal thread number.");
            )

            //  Write the value into the selected register
            inputBank[numThread][reg] = data;

            break;

        case PARAM:
        
            //  Check the range of the register index.
            GPU_ASSERT(
                if (reg >= numConstantRegs)
                    panic("ShaderEmulator", "loadShaderState(QuadFloat)", "Incorrect register number.");
            )

            //  Write the value into the selected register.
            ((f32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 0] = data[0];
            ((f32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 1] = data[1];
            ((f32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 2] = data[2];
            ((f32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 3] = data[3];

            break;

        default:

            panic("ShaderEmulator", "loadShaderState(QuadFloat)", "Access not allowed.");
            break;

    }
}

//  Loads a value into one of the thread registers.  QuadInt version.
void ShaderEmulator::loadShaderState(u32bit numThread, Bank bank, u32bit reg, QuadInt data)
{
    //  Load the register in the appropiate register bank.
    switch(bank)
    {
        case PARAM:
        
            //  Check the range of the register index.
            GPU_ASSERT(
                if (reg >= numConstantRegs)
                    panic("ShaderEmulator", "loadShaderState(QuadInt)", "Incorrect register number.");
            )

            //  Write the value into the selected register.
            ((u32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 0] = data[0];
            ((u32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 1] = data[1];
            ((u32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 2] = data[2];
            ((u32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 3] = data[3];

            break;

        default:

            panic("ShaderEmulator", "loadShaderState(QuadInt)", "Access not allowed.");
            break;

    }
}

//  Loads a value into one of the thread registers.  Boolean version.
void ShaderEmulator::loadShaderState(u32bit numThread, Bank bank, u32bit reg, bool data)
{
    panic("ShaderEmulator", "loadShaderState(bool)", "Not implemented.");
}

//  Loads new values into one of the thread register banks.  QuadFloat version.
void ShaderEmulator::loadShaderState(u32bit numThread, Bank bank, QuadFloat *data, u32bit startReg, u32bit nRegs)
{
    switch(bank)
    {
        case IN:
        
            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "loadShaderState(Bank, QuadFloat*)", "Illegal thread number.");
            )
            
            //  Check the range of the registers to write.
            GPU_ASSERT(
                if ((startReg >= numInputRegs) || ((startReg + nRegs) > numInputRegs))
                    panic("ShaderEmulator", "loadShaderState(QuadFloat*)", "Registers to load are out of range.");
            )

            //  Write all registers in the bank with the new values.
            for(u32bit r = startReg; r < (startReg + nRegs); r++)
                inputBank[numThread][r] = data[r - startReg];

            break;

        case PARAM:

            //  Check the range of the registers to write.
            GPU_ASSERT(
                if ((startReg >= numConstantRegs) || ((startReg + nRegs) > numConstantRegs))
                    panic("ShaderEmulator", "loadShaderState(QuadFloat*)", "Registers to load are out of range.");
            )

            //  Write all registers in the bank with the new values.
            for(u32bit r = startReg; r < (startReg + nRegs); r++)
            {
                ((f32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 0] = data[r - startReg][0];
                ((f32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 1] = data[r - startReg][1];
                ((f32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 2] = data[r - startReg][2];
                ((f32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 3] = data[r - startReg][3];
            }
            
            break;

        default:
            panic("ShaderEmulator", "loadShaderState(QuadFloat*)", "Access not allowed.");
            break;
    }
}

//  Loads values into all the registers of a thread register bank.
void ShaderEmulator::loadShaderState(u32bit numThread, Bank bank, QuadFloat *data)
{
    switch(bank)
    {
        case IN:
        
            //  Check if the thread number is correct.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "loadShaderState(full bank)", "Illegal thread number.");
            )

            //  Write all registers in the bank with the new values.
            for(u32bit r = 0; r < numInputRegs; r++)
                inputBank[numThread][r] = data[r];

            break;

        default:
            panic("ShaderEmulator", "loadShaderState(full bank)", "Access not allowed.");
            break;
    }

}


//  Reads the value of one of the thread registers.  QuadFloat version.
void ShaderEmulator::readShaderState(u32bit numThread, Bank bank, u32bit reg, QuadFloat &value)
{
    //  Select the bank for the register.
    switch(bank)
    {
        case IN:
        
            //  Check the range of the register identifier.
            GPU_ASSERT(
                if (reg >= numInputRegs)
                    panic("ShaderEmulator", "readShaderState(QuadFloat)", "Incorrect register number.");
            )

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadFloat)", "Illegal thread number.");
            )

            //  Read the value from the input register bank,
            value = inputBank[numThread][reg];

            break;

        case OUT:
        
            //  Check the range of the register identifier.
            GPU_ASSERT(
                if (reg >= numOutputRegs)
                    panic("ShaderEmulator", "readShaderState(QuadFloat)", "Incorrect register number.");
            )

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadFloat)", "Illegal thread number.");
            )

            //  Read the value from the output register bank,
            value = outputBank[numThread][reg];

            break;

        case PARAM:
        
            //  Check the range of the register identifier.
            GPU_ASSERT(
                if (reg >= numConstantRegs)
                    panic("ShaderEmulator", "readShaderState(QuadFloat)", "Incorrect register number.");
            )

            //  Read the value from the constant register bank.
            value[0] = ((f32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 0];
            value[1] = ((f32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 1];
            value[2] = ((f32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 2];
            value[3] = ((f32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 3];

            break;

        case TEMP:
        
            //  Check the range of the register identifier.
            GPU_ASSERT(
                if (reg >= numTemporaryRegs)
                    panic("ShaderEmulator", "readShaderState(QuadFloat)", "Incorrect register number.");
            )

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadFloat)", "Illegal thread number.");
            )

            //  Read the value from the temprary register bank.
            value[0] = ((f32bit *) temporaryBank)[(numThread * numTemporaryRegs + reg) * (UNIFIED_TEMP_REG_SIZE/4) + 0];
            value[1] = ((f32bit *) temporaryBank)[(numThread * numTemporaryRegs + reg) * (UNIFIED_TEMP_REG_SIZE/4) + 1];
            value[2] = ((f32bit *) temporaryBank)[(numThread * numTemporaryRegs + reg) * (UNIFIED_TEMP_REG_SIZE/4) + 2];
            value[3] = ((f32bit *) temporaryBank)[(numThread * numTemporaryRegs + reg) * (UNIFIED_TEMP_REG_SIZE/4) + 3];

            break;

        default:
        
            //  Access not allowed.
            panic("ShaderEmulator", "readShaderState(QuadFloat)", "Access not allowed.");

            break;
    }
}

//  Reads the value of one of the thread registers.  QuadInt version.
void ShaderEmulator::readShaderState(u32bit numThread, Bank bank, u32bit reg, QuadInt &value)
{
    //  Select the register bank.
    switch(bank)
    {
        case ADDR:
        
            //  Check the range of the register identifier.
            GPU_ASSERT(
                if (reg >= numAddressRegs)
                    panic("ShaderEmulator", "readShaderState(QuadInt)", "Incorrect register number.");
            )

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadInt)", "Illegal thread number.");
            )

            value = addressBank[numThread][reg];

            break;

        case PARAM:
        
            //  Check the range of the register identifier.
            GPU_ASSERT(
                if (reg >= numConstantRegs)
                    panic("ShaderEmulator", "readShaderState(QuadInt)", "Incorrect register number.");
            )

            //  Read the value from the constant register bank.
            value[0] = ((u32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 0];
            value[1] = ((u32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 1];
            value[2] = ((u32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 2];
            value[3] = ((u32bit *) constantBank)[reg * (UNIFIED_CONST_REG_SIZE/4) + 3];

            break;

        case TEMP:
        
            //  Check the range of the register identifier.
            GPU_ASSERT(
                if (reg >= numTemporaryRegs)
                    panic("ShaderEmulator", "readShaderState(QuadInt)", "Incorrect register number.");
            )

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadInt)", "Illegal thread number.");
            )

            //  Read the value from the temprary register bank.
            value[0] = ((u32bit *) temporaryBank)[(numThread * numTemporaryRegs + reg) * (UNIFIED_TEMP_REG_SIZE/4) + 0];
            value[1] = ((u32bit *) temporaryBank)[(numThread * numTemporaryRegs + reg) * (UNIFIED_TEMP_REG_SIZE/4) + 1];
            value[2] = ((u32bit *) temporaryBank)[(numThread * numTemporaryRegs + reg) * (UNIFIED_TEMP_REG_SIZE/4) + 2];
            value[3] = ((u32bit *) temporaryBank)[(numThread * numTemporaryRegs + reg) * (UNIFIED_TEMP_REG_SIZE/4) + 3];

            break;

        default:
        
            //  Access not allowed.
            panic("ShaderEmulator", "readShaderState(QuadInt)", "Access not allowed.");
            break;

    }
}

//  Reads the value of one of the thread registers.  Boolean version.
void ShaderEmulator::readShaderState(u32bit numThread, Bank bank, u32bit reg, bool &value)
{
    panic("ShaderEmulator", "readShaderState(u32bit, Bank, u32bit)", "Unimplemented.");
}

//  Reads the values of one of the thread register banks.  QuadFloat version.
void ShaderEmulator::readShaderState(u32bit numThread, Bank bank, QuadFloat *data)
{
    //  Select register bank.
    switch(bank)
    {
        case IN:
            
            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadFloat*)", "Illegal thread number.");
            )

            //  Reads all the registers in the input bank.
            for (u32bit r = 0; r < numInputRegs; r++)
                data[r] = inputBank[numThread][r];

            break;

        case OUT:

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadFloat*)", "Illegal thread number.");
            )

            //  Reads all the registers in the output bank.
            for (u32bit r = 0; r < numOutputRegs; r++)
                data[r] = outputBank[numThread][r];

            break;

        case PARAM:

            //  Read all the registers in the constants memory.
            for (u32bit r = 0; r < numConstantRegs; r++)
            {
                data[r][0] = ((f32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 0];
                data[r][1] = ((f32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 1];
                data[r][2] = ((f32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 2];
                data[r][3] = ((f32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 3];
            }
            
            break;

        case TEMP:

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadFloat *)", "Illegal thread number.");
            )

            //  Read all the registers in the temporary register bank.
            for (u32bit r = 0; r < numTemporaryRegs; r++)
            {
                //  Read the value from the temprary register bank.
                data[r][0] = ((f32bit *) temporaryBank)[(numThread * numTemporaryRegs + r) * (UNIFIED_TEMP_REG_SIZE/4) + 0];
                data[r][1] = ((f32bit *) temporaryBank)[(numThread * numTemporaryRegs + r) * (UNIFIED_TEMP_REG_SIZE/4) + 1];
                data[r][2] = ((f32bit *) temporaryBank)[(numThread * numTemporaryRegs + r) * (UNIFIED_TEMP_REG_SIZE/4) + 2];
                data[r][3] = ((f32bit *) temporaryBank)[(numThread * numTemporaryRegs + r) * (UNIFIED_TEMP_REG_SIZE/4) + 3];
            }
            
            break;

        default:
        
            panic("ShaderEmulator", "readShaderState(QuadFloat *)", "Access not allowed.");
            break;
    }

}

//  Reads the values of one of the thread register banks.  QuadInt version.
void ShaderEmulator::readShaderState(u32bit numThread, Bank bank, QuadInt *data)
{
    //  Select bank to read.
    switch(bank)
    {
        case ADDR:

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadInt *)", "Illegal thread number.");
            )

            //  Read all the registers in the address bank.
            for (u32bit r = 0; r < numAddressRegs; r++)
                data[r] = addressBank[numThread][r];

            break;

        case PARAM:

            //  Read all the registers in the constants memory.
            for (u32bit r = 0; r < numConstantRegs; r++)
            {
                data[r][0] = ((u32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 0];
                data[r][1] = ((u32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 1];
                data[r][2] = ((u32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 2];
                data[r][3] = ((u32bit *) constantBank)[r * (UNIFIED_CONST_REG_SIZE/4) + 3];
            }
            
            break;

        case TEMP:

            //  Check the range of the thread identifier.
            GPU_ASSERT(
                if (numThread >= numThreads)
                    panic("ShaderEmulator", "readShaderState(QuadFloat *)", "Illegal thread number.");
            )

            //  Read all the registers in the temporary register bank.
            for (u32bit r = 0; r < numTemporaryRegs; r++)
            {
                //  Read the value from the temprary register bank.
                data[r][0] = ((u32bit *) temporaryBank)[(numThread * numTemporaryRegs + r) * (UNIFIED_TEMP_REG_SIZE/4) + 0];
                data[r][1] = ((u32bit *) temporaryBank)[(numThread * numTemporaryRegs + r) * (UNIFIED_TEMP_REG_SIZE/4) + 1];
                data[r][2] = ((u32bit *) temporaryBank)[(numThread * numTemporaryRegs + r) * (UNIFIED_TEMP_REG_SIZE/4) + 2];
                data[r][3] = ((u32bit *) temporaryBank)[(numThread * numTemporaryRegs + r) * (UNIFIED_TEMP_REG_SIZE/4) + 3];
            }

            break;
            
        default:
            
            panic("ShaderEmulator", "readShaderState(QuadInt *)", "Access not allowed..");
            break;
    }
}


//  Reads the values of one of the thread register banks.  QuadInt version.
void ShaderEmulator::readShaderState(u32bit numThread, Bank bank, bool *data)
{
    panic("ShaderEmulator", "readShaderState(bool*)", "Not Implemented");
}


//  Load a shader program into the shader instruction memory and decode the shader program
//  for all the threads in the shader (expensive!!!).
void ShaderEmulator::loadShaderProgram(u8bit *code, u32bit address, u32bit sizeCode, u32bit partition)
{
    ShaderInstruction *shInstr;

    //  Check.  The size of the code must be a multiple of the instruction size.
    GPU_ASSERT(
        if (((sizeCode & ShaderInstruction::SHINSTRSZMASK) != 0) && (sizeCode > 0))
            panic("ShaderEmulator", "loadShaderProgram", "Shader Program incorrect size.");
    )

    //  Check the program address range.
    GPU_ASSERT(
        if (address >= instructionMemorySize )
            panic("ShaderEmulator", "loadShaderProgram", "Out of range shader program load address.");
    )

    //  Check if the Shader Program is too large for the instruction memory.
    GPU_ASSERT(
        if ((address + (sizeCode >> ShaderInstruction::SHINSTRSZLOG)) > instructionMemorySize)
            panic("ShaderEmulator", "loadShaderProgram", "Shader Program too large for Instruction Memory.");
    )

//char buffer[200];
//printf("ShEmu => loadShaderProgram address %x size %d\n", address, sizeCode);

    //  Load the shader program.
    for(u32bit i = 0; i < (sizeCode >> ShaderInstruction::SHINSTRSZLOG); i++)
    {
        //  Delete the old instruction.
        if (instructionMemory[i + address] != NULL)
            delete instructionMemory[i + address];

        //  Create and decode the new instruction.
        shInstr = instructionMemory[i + address] = new ShaderInstruction(&code[i << ShaderInstruction::SHINSTRSZLOG]);

//instructionMemory[i + address][j]->disassemble(buffer);
//printf("%04x : %s\n", i + address, buffer);

        //  Check if storing decoded shader instructions is enabled.
        if (storeDecodedInstr)
        {
            //  Decode the instruction for the 
            for(u32bit t = 0; t < numThreads; t++)
            {
                //  Destroy the old decoded instruction.
                if (decodedInstructions[i + address][t] != NULL)
                    delete decodedInstructions[i + address][t];

                //  Decode the shader instruction for the thread.
                decodedInstructions[i + address][t] = decodeShaderInstruction(shInstr, i + address, t, partition);
            }
        }
    }
}

//  Decode a shader instruction.
ShaderInstruction::ShaderInstructionDecoded *ShaderEmulator::decodeShaderInstruction(ShaderInstruction *shInstr,
    u32bit address, u32bit nThread, u32bit partition)
{
    u32bit numOps;  //  Number of instruction operands.
    ShaderInstruction::ShaderInstructionDecoded *shInstrDec;   //  Pointer to the Shader Instruction decoded per thread.

    //  Create and decode the new instruction.
    shInstrDec = new ShaderInstruction::ShaderInstructionDecoded(shInstr, address, nThread);

    //  Decode the registers that the instruction is going to use when executed.
    numOps = shInstr->getNumOperands();

    //  Get first operand address.
    if (numOps > 0)
    {
        if (shInstr->getRelativeModeFlag() && (shInstr->getBankOp1() == PARAM))
        {
            //  If this operand is a constant accessed through an address register
            //  load the address register in the first operand register pointer.
            shInstrDec->setShEmulOp1(decodeOpReg(nThread, ADDR, shInstr->getRelMAddrReg(), partition));
        }
        else
            shInstrDec->setShEmulOp1(decodeOpReg(nThread, shInstr->getBankOp1(), shInstr->getOp1(), partition));
    }

    //  Get second operand address.
    if (numOps > 1)
    {
        if (shInstr->getRelativeModeFlag() && (shInstr->getBankOp2() == PARAM))
        {
            //  If this operand is a constant accessed through an address register
            //  load the address register in the second operand register pointer.
            shInstrDec->setShEmulOp2(decodeOpReg(nThread, ADDR, shInstr->getRelMAddrReg(), partition));
        }
        else if (shInstr->getBankOp2() == IMM)
        {        
            //  Replicate four times the instruction 32-bit immediate value to the decoded instruction local store.
            u8bit *data = shInstrDec->getOp2Data();
            u32bit immediate = shInstr->getImmediate();
            for(u32bit t = 0; t < 4; t++)
                ((u32bit *) data)[t] = immediate;
            
            //  Set the decoded instruction to read the second operand from local data.
            shInstrDec->setShEmulOp2(0);
        }
        else
            shInstrDec->setShEmulOp2(decodeOpReg(nThread, shInstr->getBankOp2(), shInstr->getOp2(), partition));
    }

    //  Get third operand address.
    if (numOps > 2)
    {
        if (shInstr->getRelativeModeFlag() && (shInstr->getBankOp3() == PARAM))
        {
            //  If this operand is a constant accessed through an address register
            //    load the address register in the first operand register pointer.
            shInstrDec->setShEmulOp3(decodeOpReg(nThread, ADDR, shInstr->getRelMAddrReg()));
        }
        else
            shInstrDec->setShEmulOp3(decodeOpReg(nThread, shInstr->getBankOp3(), shInstr->getOp3(), partition));
    }

    if (shInstr->hasResult())
    {
        //  Get result operand address.
        shInstrDec->setShEmulResult(decodeOpReg(nThread, shInstr->getBankRes(), shInstr->getResult(), partition));
    }

    //  Decode the instruction predication.
    if (shInstr->getPredicatedFlag())
        shInstrDec->setShEmulPredicate((void *) &predicateBank[nThread][shInstr->getPredicateReg()]);

    //  Decodes and sets the function that will be used to emulate the instruction when executed.
    setEmulFunction(shInstrDec);

    return shInstrDec;
}

//  Load a shader program into the shader instruction memory.
void ShaderEmulator::loadShaderProgramLight(u8bit *code, u32bit address, u32bit sizeCode)
{
    ShaderInstruction *shInstr;     //  Pointer to the Shader Instruction.

    //  Check.  The size of the code must be a multiple of the instruction size.
    GPU_ASSERT(
        if (((sizeCode & ShaderInstruction::SHINSTRSZMASK) != 0) && (sizeCode > 0))
            panic("ShaderEmulator", "loadShaderProgramLight", "Shader Program incorrect size.");
    )

    //  Check the program address range.
    GPU_ASSERT(
        if (address >= instructionMemorySize)
            panic("ShaderEmulator", "loadShaderProgramLight", "Out of range shader program load address.");
    )

    //  Check if the Shader Program is too large for the instruction memory.
    GPU_ASSERT(
        if ((address + (sizeCode >> ShaderInstruction::SHINSTRSZLOG)) > instructionMemorySize)
            panic("ShaderEmulator", "loadShaderProgramLight", "Shader Program too large for Instruction Memory.");
    )

//char buffer[200];
//printf("ShEmu => loadShaderProgram address %x size %d\n", address, sizeCode);

    //  Load the shader program instructions.
    for(u32bit i = 0; i < (sizeCode >> ShaderInstruction::SHINSTRSZLOG); i++)
    {
        //  Destroy the old instructions.
        if (instructionMemory[i + address] != NULL)
            delete instructionMemory[i + address];

        /*  Create and decode the new instruction.  */
        /* i << 4 (i * 16) is the position of the first byte of the next instruction.  */
//printf("Byte Code : ");
//for(int cc = 0; cc < 16; cc++)
//printf("%02x ", code[(i << ShaderInstruction::SHINSTRSZLOG) + cc]);
//printf("\n");
        shInstr = instructionMemory[i + address] = new ShaderInstruction(&code[i << ShaderInstruction::SHINSTRSZLOG]);

//instructionMemory[i + address]->disassemble(buffer);
//printf("%04x : %s\n", i + address, buffer);
    }
}

// Decodes and returns the address to the register that the instruction is
// going to access when executed (per thread!)
void *ShaderEmulator::decodeOpReg(u32bit numThread, Bank bank, u32bit reg, u32bit partition)
{
    u32bit offset;

    switch(bank)
    {
        case IN:
        
            //  Get the input register address.
            return (void *) inputBank[numThread][reg].getVector();
            break;

        case OUT:
        
            // Get the output register address.
            return (void *) outputBank[numThread][reg].getVector();
            break;

        case PARAM:
        
            //  WARNING:  RELATIVE MODE ADDRESSING IN THE CONSTANT BANK MUST
            //  BE EMULATED AT RUNTIME (ACCESS IS RELATIVE TO A REGISTER).

            offset = (partition == FRAGMENT_PARTITION)? UNIFIED_CONSTANT_NUM_REGS:(partition == TRIANGLE_PARTITION)?200:0;

            //  Get constant address.  Add offset for shader partition constant bank.
            return (void *) &constantBank[(reg + offset) * UNIFIED_CONST_REG_SIZE];
            break;

        case TEMP:
        
            //  Get temporary register address.
            return (void *) &temporaryBank[(numThread * numTemporaryRegs + reg) * UNIFIED_TEMP_REG_SIZE];
            break;

        case ADDR:
        
            //   Get address register address.
            return (void *) &addressBank[numThread][reg];
            break;

        case PRED:
            return (void *) &predicateBank[numThread][reg];
            break;

        case TEXT:

            //  Texture unit identifier, not a register.
            return NULL;
            break;

        case SAMP:
            
            //  Fragment sample identifier, not a register.            
            return NULL;
            break;
    }

    panic("ShaderEmulator", "decodeOpReg(u32bit, Bank, u32bit)", "Unimplemented register bank.");
    return 0;
}


//  Sets the pointer to the emulated function for the instruction in the tabled for predecoded instructions.
void ShaderEmulator::setEmulFunction(ShaderInstruction::ShaderInstructionDecoded *shInstrDec)
{
    //  Store the emulation function.
    shInstrDec->setEmulFunc(shInstrEmulationTable[shInstrDec->getShaderInstruction()->getOpcode()]);
}


//  Returns the instruction in the Instruction Memory pointed by PC.
ShaderInstruction::ShaderInstructionDecoded *ShaderEmulator::fetchShaderInstruction(u32bit threadId, u32bit PC)
{

    //  Check if the PC and the thread number is valid.
    GPU_ASSERT(
        if (PC >= instructionMemorySize)
        {
            std::stringstream ss;
            ss << "PC (" << PC << ") overflows instruction memory size (" << instructionMemorySize << ").";
            panic("ShaderEmulator", "fetchShaderInstruction(u32bit)", ss.str().c_str());
        }

        if (threadId >= numThreads)
        {
            panic("ShaderEmulator", "fetchShaderInstruction(u32bit)", "Thread number not valid.");
        }
    )

        //  Check if storing decoded instructions is enabled.
    if (storeDecodedInstr)
    {
        //  Check if the instruction is already decoded.
        if (decodedInstructions[PC][threadId] == NULL)
        {
            //  Decode on demand.
            decodedInstructions[PC][threadId] = decodeShaderInstruction(instructionMemory[PC], PC, threadId, (PC / UNIFIED_INSTRUCTION_MEMORY_SIZE));
        }
        
        //  Return the instruction pointed.
        return decodedInstructions[PC][threadId];
    }
    else
    {
        ShaderInstruction::ShaderInstructionDecoded *decodedShInstr;
        
        decodedShInstr = decodeShaderInstruction(instructionMemory[PC], PC, threadId, (PC / UNIFIED_INSTRUCTION_MEMORY_SIZE));
        
        return decodedShInstr;
    }
}

//  Fetch and decode a shader instruction for the corresponding thread.
ShaderInstruction::ShaderInstructionDecoded *ShaderEmulator::fetchShaderInstructionLight(u32bit threadId, u32bit PC, u32bit partition)
{
    GPU_ASSERT(
        //  Check the instruction address range.
        if (PC >= instructionMemorySize)
        {
            std::stringstream ss;
            ss << "PC (" << PC << ") overflows instruction memory size (" << instructionMemorySize << ").";
            panic("ShaderEmulator", "fetchShaderInstructionLight", ss.str().c_str());
        }

        //  Check the range of the thread identifier.
        if (threadId >= numThreads)
        {
            panic("ShaderEmulator", "fetchShaderInstructionLight", "Thread number not valid.");
        }

        //  Check if the instruction is defined.
        if (instructionMemory[PC] == NULL)
        {
            char buffer[128];
            sprintf(buffer, "Shader instruction not defined for partition %d Thread %d PC %x", partition,
                threadId, PC);
            panic("ShaderEmulator", "fetchShaderInstructionLight", buffer);
        }
    )

    //  Fetch and decode the shader instruction.
    return decodeShaderInstruction(instructionMemory[PC], PC, threadId, partition);
}

//  Read a shader instruction from shader instruction memory.
ShaderInstruction *ShaderEmulator::readShaderInstruction(u32bit pc)
{
    return instructionMemory[pc];
}


//  Executes a Shader Instruction.
void ShaderEmulator::execShaderInstruction(ShaderInstruction::ShaderInstructionDecoded *shInstrDec)
{
    //  Check the range of the thread identifier.
    GPU_ASSERT(
        if (shInstrDec->getNumThread() >= numThreads)
            panic("ShaderEmulator", "execShaderInstruction(ShaderInstruction::ShaderInstructionDecoded)", "Illegal thread number.");
    )

    shInstrDec->getEmulFunc()(*shInstrDec, *this);
}

//  Returns the PC of a shader thread.
u32bit ShaderEmulator::threadPC(u32bit numThread)
{
    //  Check the range of the thread identifier.
    GPU_ASSERT(
        if (numThread > numThreads)
            panic("ShaderEmulator", "threadPC", "Illegal thread number.");
    )

    return PCTable[numThread];
}

//  Sets the thread PC.
u32bit ShaderEmulator::setThreadPC(u32bit numThread, u32bit pc)
{
    //  Check the range of the thread identifier.
    GPU_ASSERT(
        if (numThread > numThreads)
            panic("ShaderEmulator", "threadPC", "Illegal thread number.");
    )

    /*  Set the thread PC.  */
    PCTable[numThread] = pc;

    return 0; // ¿? (carlos)
}

//  Returns the thread kill flag.
bool ShaderEmulator::threadKill(u32bit numThread)
{
    return threadKill(numThread, 0);
}

//  Returns the thread's sample kill flag.
bool ShaderEmulator::threadKill(u32bit numThread, u32bit sample)
{
    //  Check the range of the thread identifier.
    GPU_ASSERT(
        if (numThread > numThreads)
            panic("ShaderEmulator", "threadKill", "Illegal thread number.");
    )

    return kill[numThread][sample];
}

//  Returns the thread's sample z export value.
f32bit ShaderEmulator::threadZExport(u32bit numThread, u32bit sample)
{
    //  Check the range of the thread identifier.
    GPU_ASSERT(
        if (numThread > numThreads)
            panic("ShaderEmulator", "threadZExport", "Illegal thread number.");
    )

    return zexport[numThread][sample];
}

//  Generates the next complete texture request for a stamp available in the shader emulator.
TextureAccess *ShaderEmulator::nextTextureAccess()
{
    TextureAccess *textAccess = NULL;

    //  Check if there are texture operations waiting to be processed.
    if (numWait > 0)
    {
        //  Create texture access from the entry.
        textAccess = textEmu->textureOperation(
            waitTexture[firstWait],
            textQueue[waitTexture[firstWait]].texOp,
            textQueue[waitTexture[firstWait]].coordinates,
            textQueue[waitTexture[firstWait]].parameter,
            textQueue[waitTexture[firstWait]].textUnit
            );

        //  Update number of texture accesses waiting to be processed.
        numWait--;

        //  Update pointer to the next entry waiting to be processed.
        firstWait = GPU_MOD(firstWait + 1, TEXT_QUEUE_SIZE);
    }

    return textAccess;
}

//  Generates the next complete texture request for a vertex texture access available in the shader emulator.
TextureAccess *ShaderEmulator::nextVertexTextureAccess()
{
    TextureAccess *textAccess = NULL;

    //  Check if there is a pending request.
    if (textQueue[freeTexture[firstFree]].requested > 0)
    {
        //  Fill with dummy data the other elements in the request.
        for(u32bit e = 1; e < STAMP_FRAGMENTS; e++)
        {
            //  Copy texture coordinates (index) and parameter.
            textQueue[freeTexture[firstFree]].coordinates[e] = textQueue[freeTexture[firstFree]].coordinates[0];
            textQueue[freeTexture[firstFree]].parameter[e] = textQueue[freeTexture[firstFree]].parameter[0];

            //  Set the shader instruction that produced the access.
            textQueue[freeTexture[firstFree]].shInstrD[e] = textQueue[freeTexture[firstFree]].shInstrD[0];

            //  Update requested fragments.
            textQueue[freeTexture[firstFree]].requested++;
        }
        
        //  Set as a vertex texture access.
        textQueue[freeTexture[firstFree]].vertexTextureAccess = true;
    
        //  Add entry to the wait list.
        waitTexture[lastWait] = freeTexture[firstFree];

        //  Update pointer to the last waiting entry.
        lastWait = GPU_MOD(lastWait + 1, TEXT_QUEUE_SIZE);

        //  Update number of entries waiting to be processed.
        numWait++;

        //  Update pointer to first free entry.
        firstFree = GPU_MOD(firstFree + 1, TEXT_QUEUE_SIZE);

        //  Update number of free entries.
        numFree--;
    }
    
    //  Check if there are texture operations waiting to be processed.
    if (numWait > 0)
    {
        //  Create texture access from the entry.
        textAccess = textEmu->textureOperation(
            waitTexture[firstWait],
            textQueue[waitTexture[firstWait]].texOp,
            textQueue[waitTexture[firstWait]].coordinates,
            textQueue[waitTexture[firstWait]].parameter,
            textQueue[waitTexture[firstWait]].textUnit
            );

        //  Update number of texture accesses waiting to be processed.
        numWait--;

        //  Update pointer to the next entry waiting to be processed.
        firstWait = GPU_MOD(firstWait + 1, TEXT_QUEUE_SIZE);
    }

    return textAccess;
}

//  Completes a texture access writing the sample in the result register.
void ShaderEmulator::writeTextureAccess(u32bit id, QuadFloat *sample, u32bit *threads, bool removeInstruction)
{
    //  Write the result of the texture accesses.
    for(u32bit i = 0; i < stampFragments; i++)
    {
        //  For vertex texture accesses only the first element in the texture access is valid/must be written.
        if (!textQueue[id].vertexTextureAccess || (i == 0))
        {
            //  Write texture sample into the texture access result register.
            writeResult(*textQueue[id].shInstrD[i], *this, sample[i].getVector());

            threads[i] = textQueue[id].shInstrD[i]->getNumThread();

            //  Delete the shader decoded instruction that generated the request.
            if (removeInstruction)
                delete textQueue[id].shInstrD[i];
        }
    }

    //  Reset requested fragments counter.
    textQueue[id].requested = 0;

    //  Add entry to the free list.
    freeTexture[lastFree] = id;

    //  Update number of free entries.
    numFree++;

    //  Update pointer to the last the free list.
    lastFree = GPU_MOD(lastFree + 1, TEXT_QUEUE_SIZE);
}

//  Adds a texture operation for a shader processing element to the texture queue.
void ShaderEmulator::textureOperation(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, TextureOperation texOp,
    QuadFloat coord, f32bit parameter)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    GPU_ASSERT(
        if (numFree == 0)
            panic("ShaderEmulator", "textureOperation", "No free entries in the texture queue.");
    )

    //  Determine if it is the first request for the stamp.
    if (textQueue[freeTexture[firstFree]].requested == 0)
    {
        //  Set the operation type.
        textQueue[freeTexture[firstFree]].texOp = texOp;

        //  Set the texture sampler or attribute for the operation.
        textQueue[freeTexture[firstFree]].textUnit = shInstr->getOp2();
        
        //  Set as not a vertex texture access.
        textQueue[freeTexture[firstFree]].vertexTextureAccess = false;
    }
    else
    {
        //  Check that the texture operation is the same for the whole 'stamp'.
        GPU_ASSERT(
            if (textQueue[freeTexture[firstFree]].texOp != texOp)
                panic("ShaderEmulator", "textureOperation", "Different operation for elements of the same stamp.");
        )

        //  Check that the texture sampler or attribute for the operation is the same for the whole 'stamp'.
        GPU_ASSERT(
            if (textQueue[freeTexture[firstFree]].textUnit != shInstr->getOp2())
                panic("ShaderEmulator", "textureOperation", "Accessing different texture units from the same stamp.");
        )
    }

    //  Copy texture coordinates (index) and parameter.
    textQueue[freeTexture[firstFree]].coordinates[textQueue[freeTexture[firstFree]].requested] = coord;
    textQueue[freeTexture[firstFree]].parameter[textQueue[freeTexture[firstFree]].requested] = parameter;

    //  Set the shader instruction that produced the access.
    textQueue[freeTexture[firstFree]].shInstrD[textQueue[freeTexture[firstFree]].requested] = &shInstrDec;

    //  Update requested fragments.
    textQueue[freeTexture[firstFree]].requested++;

    //  Check if all the fragments were requested.
    if (textQueue[freeTexture[firstFree]].requested == stampFragments)
    {
        //  Add entry to the wait list.
        waitTexture[lastWait] = freeTexture[firstFree];

        //  Update pointer to the last waiting entry.
        lastWait = GPU_MOD(lastWait + 1, TEXT_QUEUE_SIZE);

        //  Update number of entries waiting to be processed.
        numWait++;

        //  Update pointer to first free entry.
        firstFree = GPU_MOD(firstFree + 1, TEXT_QUEUE_SIZE);

        //  Update number of free entries.
        numFree--;
    }
}

//  Add information for a derivation operation.
void ShaderEmulator::derivOperation(gpu3d::ShaderInstruction::ShaderInstructionDecoded &shInstr, gpu3d::QuadFloat input)
{
    //  Compute the base thread/element for the derivation operation.
    u32bit baseThread = shInstr.getNumThread();
    baseThread = baseThread - (baseThread & 0x03);
    
    //  Check the number of derivations already received.
    if (currentDerivation.derived == 0)
    {
        //  Initialize the derivation structure with the base thread/element.
        currentDerivation.baseThread = baseThread;
        
        //  Store the information about the derivation.
        currentDerivation.input[currentDerivation.derived] = input.getVector();
        currentDerivation.shInstrD[currentDerivation.derived] = &shInstr;
        currentDerivation.derived++;       
    }
    else if (currentDerivation.derived < 3)
    {
        //  Check the base thread for the current operation.
        GPU_ASSERT(
            if (baseThread != currentDerivation.baseThread)
                panic("ShaderEmulator", "derivOperation", "The base thread for the incoming and on-going derivation operation is different.");
        )
        
        //  Store the information about the derivation.
        currentDerivation.input[currentDerivation.derived] = input.getVector();
        currentDerivation.shInstrD[currentDerivation.derived] = &shInstr;
        currentDerivation.derived++;       
    }
    else if (currentDerivation.derived == 3)
    {
        //  Check the base thread for the current operation.
        GPU_ASSERT(
            if (baseThread != currentDerivation.baseThread)
                panic("ShaderEmulator", "derivOperation", "The base thread for the incoming and on-going derivation operation is different.");
        )

        //  Store the information about the derivation.
        currentDerivation.input[currentDerivation.derived] = input.getVector();
        currentDerivation.shInstrD[currentDerivation.derived] = &shInstr;
        
        //  Perform the derivation computations.
        QuadFloat derivates[4];
        
        //  Check the derivation direction.
        switch(currentDerivation.shInstrD[0]->getShaderInstruction()->getOpcode())
        {
            case DDX:
            
                GPUMath::derivX(currentDerivation.input, derivates);
                break;
                
            case DDY:
                
                GPUMath::derivY(currentDerivation.input, derivates);
                break;
                
            default:
                panic("ShaderEmulator", "derivOperation", "Expected a derivation instruction.");
                break;
        }

        //  Write derivation operation results to the corresponding registers.
        for(u32bit e = 0; e < 4; e++)
            writeResult(*currentDerivation.shInstrD[e], *this, derivates[e].getVector());

        //  Clear current derivation operation.
        currentDerivation.derived = 0;
    }
    else
    {
        panic("ShaderEmulator", "derivOperation", "Unexpected value for the number of derivation operations.");
    }
}


//  Swizzle function.  Reorders and replicates the components of a 4-component float.
inline QuadFloat ShaderEmulator::swizzle(SwizzleMode mode, QuadFloat qf)
{
    QuadFloat out;

    //
    //  As SwizzleMode definition is ordered per components
    //  we can get the index for the component in each position
    //  using this method:
    //
    //  mode & 0x03 -> bits 1 - 0 -> Source component to copy to W destination component.
    //  mode & 0x0C -> bits 3 - 2 -> Source component to copy to Z destination component.
    //  mode & 0x30 -> bits 5 - 4 -> Source component to copy to Y destination component.
    //  mode & 0xC0 -> bits 7 - 6 -> Source component to copy to X destination component.
    //

    out[0] = qf[(mode & 0xC0) >> 6];    //  Swizzle X component in the output QuadFloat.
    out[1] = qf[(mode & 0x30) >> 4];    //  Swizzle Y component in the output QuadFloat.
    out[2] = qf[(mode & 0x0C) >> 2];    //  Swizzle Z component in the output QuadFloat.
    out[3] = qf[mode & 0x03];           //  Swizzle W component in the output QuadFloat.

    return out;     //  Return the swizzled value.
}

//  Swizzle function.  Reorders and replicates the components of a 4-component integer.
inline QuadInt ShaderEmulator::swizzle(SwizzleMode mode, QuadInt qi)
{
    QuadInt out;

    //
    //  As SwizzleMode definition is ordered per components
    //  we can get the index for the component in each position
    //  using this method:
    //
    //  mode & 0x03 -> bits 1 - 0 -> Source component to copy to W destination component.
    //  mode & 0x0C -> bits 3 - 2 -> Source component to copy to Z destination component.
    //  mode & 0x30 -> bits 5 - 4 -> Source component to copy to Y destination component.
    //  mode & 0xC0 -> bits 7 - 6 -> Source component to copy to X destination component.
    //

    out[0] = qi[(mode & 0xC0) >> 6];       //  Swizzle X component in the output QuadInt.
    out[1] = qi[(mode & 0x30) >> 4];       //  Swizzle Y component in the output QuadInt.
    out[2] = qi[(mode & 0x0C) >> 2];       //  Swizzle Z component in the output QuadInt.
    out[3] = qi[mode & 0x03];              //  Swizzle W component in the output QuadInt.

    return out;     //  Return the swizzled value.
}

//  Negate function.  Negates the 4 components in a 4-component float.
inline QuadFloat ShaderEmulator::negate(QuadFloat qf)
{
    QuadFloat out;

    //  Negate each component of the 4-component vector.
    out[0] = -qf[0];        //  Negate component X.
    out[1] = -qf[1];        //  Negate component Y.
    out[2] = -qf[2];        //  Negate component Z.
    out[3] = -qf[3];        //  Negate component W.

    return out;
}

//  Negate function.  Negates the 4 components in a 4-component integer.
inline QuadInt ShaderEmulator::negate(QuadInt qi)
{
    QuadInt out;

    //  Negate each component of the 4-component vector.
    out[0] = -qi[0];        //  Negate component X.
    out[1] = -qi[1];        //  Negate component Y.
    out[2] = -qi[2];        //  Negate component Z.
    out[3] = -qi[3];        //  Negate component W.

    return out;
}

//  Absolute function.  Returns the absolute value for the 4 components in a 4-component float.
inline QuadFloat ShaderEmulator::absolute(QuadFloat qf)
{
    QuadFloat out;

    //  Negate each component of the 4-component vector.
    out[0] = GPUMath::ABS(qf[0]);        //  Negate component X.
    out[1] = GPUMath::ABS(qf[1]);        //  Negate component Y.
    out[2] = GPUMath::ABS(qf[2]);        //  Negate component Z.
    out[3] = GPUMath::ABS(qf[3]);        //  Negate component W.

    return out;
}


//  Absolute function.  Returns the absolute value for the 4 components in a 4-component integer.
inline QuadInt ShaderEmulator::absolute(QuadInt qi)
{
    QuadInt out;

    //  Negate each component of the 4-component vector.
    out[0] = GPUMath::ABS(qi[0]);        //  Negate component X.
    out[1] = GPUMath::ABS(qi[1]);        //  Negate component Y.
    out[2] = GPUMath::ABS(qi[2]);        //  Negate component Z.
    out[3] = GPUMath::ABS(qi[3]);        //  Negate component W.

    return out;
}

//  Write result register with mask and predication.
inline void ShaderEmulator::writeResReg(ShaderInstruction &shInstr, f32bit *f, f32bit *res, bool predicate)
{
    //  Check if the instruction predication allows writing the result.
    if (predicate)
    {
        //  Component X must be written?
        if (shInstr.getResultMaskMode() & 0x08)
            res[0] = f[0];      //  Write component X in result.

        //  Component Y must be written?
        if (shInstr.getResultMaskMode() & 0x04)
            res[1] = f[1];      //  Write component Y in result.

        //  Component Z must be written?
        if (shInstr.getResultMaskMode() & 0x02)
            res[2] = f[2];      //  Write component Z in result.

        //  Component W must be written?
        if (shInstr.getResultMaskMode() & 0x01)
            res[3] = f[3];      //  Write component W in result.

//printf("writing to res = {%f, %f, %f, %f}\n", res[0], res[1], res[2], res[3]);
    }
}

//  Write result register with mask and predication.
inline void ShaderEmulator::writeResReg(ShaderInstruction &shInstr, s32bit *f, s32bit *res, bool predicate)
{
    //  Check if the instruction predication allows writing the result.
    if (predicate)
    {
        //  Component X must be written?
        if (shInstr.getResultMaskMode() & 0x08)
            res[0] = f[0];      //  Write component X in result.

        //  Component Y must be written?
        if (shInstr.getResultMaskMode() & 0x04)
            res[1] = f[1];      //  Write component Y in result.

        //  Component Z must be written?
        if (shInstr.getResultMaskMode() & 0x02)
            res[2] = f[2];      //  Write component Z in result.

        //  Component W must be written?
        if (shInstr.getResultMaskMode() & 0x01)
            res[3] = f[3];      //  Write component W in result.

//printf("writing to res = {%f, %f, %f, %f}\n", res[0], res[1], res[2], res[3]);
    }
}

//  Write result register with mask and predication.  Fixed point accumulator version.
inline void ShaderEmulator::writeResReg(ShaderInstruction &shInstr, FixedPoint *f, FixedPoint *res, bool predicate)
{
    //  Check if the instruction predication allows writing the result.
    if (predicate)
    {
        //  Check the write mask each component.  Update the result register.

        //  Component X must be written?
        if (shInstr.getResultMaskMode() & 0x08)
            res[0] = f[0];      //  Write component X in result.

        //  Component Y must be written?
        if (shInstr.getResultMaskMode() & 0x04)
            res[1] = f[1];      //  Write component Y in result.

        //  Component Z must be written?
        if (shInstr.getResultMaskMode() & 0x02)
            res[2] = f[2];      //  Write component Z in result.

        //  Component W must be written?
        if (shInstr.getResultMaskMode() & 0x01)
            res[3] = f[3];      //  Write component W in result.
    }
}


/*
 *  Read the Shader Instruction First Input Operands.
 *
 *  THIS IS A MACRO.  THIS IS A MACRO. THIS IS A MACRO.
 *
 *      BANK    is a getBankOpN() function name
 *      OPERAND is a getShEmulOpN() function name
 *      SHIMDEC is a ShaderInstruction::ShaderInstructionDecoded reference
 *      SHINSTR is a ShaderInstruction pointer
 *      SHEMUL  is a ShaderEmulator reference
 *      OPREGV  is a variable to store the read operand
 *      TYPE    is the type (QuadFloat, QuadInt, Int, Float, Bool)
 *
 */

#define READOPERAND(OPERAND, SHINDEC, SHINSTR, SHEMUL, OPREGV, TYPE)                    \
{                                                                                       \
    /*  If it a constant and relative mode addressing is enabled the    */              \
    /*    operand address must be recalculated.                         */              \
    if ((SHINSTR->getBankOp##OPERAND() == PARAM) && (SHINSTR->getRelativeModeFlag()))   \
    {                                                                                   \
        /*  WARNING:  CHECK IF THIS IS THE CORRECT IMPLEMENTATION OF    */              \
        /*    RELATIVE MODE ADDRESSING TO CONSTANTS.                    */              \
        /*                                                              */              \
        /*    c[A0.x + offset]                                          */              \
        /*                                                              */              \
        /*    AddressBank[AddressRegister][Component] + relativeOffset. */              \
        /*                                                              */              \
        /*    offset Should be Signed -256 to 255 (9 bits)              */              \
                                                                                        \
        /*  WARNING:  DECODING SHOULD ENSURE THAT ONLY A CONSTANT BANK  */              \
        /*  REGISTER IS ACCESSED USING RELATIVE MODE.  CONSTANTS ARE    */              \
        /*  ALWAYS QUADFLOAT.                                           */              \
                                                                                        \
        OPREGV = (TYPE *) &SHEMUL.constantBank[                                         \
         ((*(QuadInt *)SHINDEC.getShEmulOp##OPERAND())[SHINSTR->getRelMAddrRegComp()] + \
          SHINSTR->getRelMOffset() + SHINSTR->getOp##OPERAND()) *                       \
          UNIFIED_CONST_REG_SIZE];                                                      \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
        /*  If not, just get the precalculated register address.  */                    \
        OPREGV = (TYPE *) SHINDEC.getShEmulOp##OPERAND();                               \
    }                                                                                   \
}


//  Reads and returns the first quadfloat operand of a shader instruction.
inline void ShaderEmulator::readOperand1(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, QuadFloat &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get a pointer to the input register.
    READOPERAND(1, shInstrDec, shInstr, shEmul, op, f32bit)
}

//  Reads and returns the second quadfloat operand of a shader instruction.
inline void ShaderEmulator::readOperand2(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, QuadFloat &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get a pointer to the input register.
    READOPERAND(2, shInstrDec, shInstr, shEmul, op, f32bit)
}

//  Reads and returns the third quadfloat operand of a shader instruction.
inline void ShaderEmulator::readOperand3(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul,
    QuadFloat &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get a pointer to the input register.
    READOPERAND(3, shInstrDec, shInstr, shEmul, op, f32bit)
}

//  Reads and returns the first quadint operand of a shader instruction.
inline void ShaderEmulator::readOperand1(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, QuadInt &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get a pointer to the input register.
    READOPERAND(1, shInstrDec, shInstr, shEmul, op, s32bit)
}

//  Reads and returns the second quadint operand of a shader instruction.
inline void ShaderEmulator::readOperand2(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, QuadInt &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get a pointer to the input register.
    READOPERAND(2, shInstrDec, shInstr, shEmul, op, s32bit)
}

//  Reads and returns the first float scalar operand.
inline void ShaderEmulator::readScalarOp1(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, f32bit &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();
    QuadFloat reg;

    //  Get pointer to first QuadFloat operand register.
    READOPERAND(1, shInstrDec, shInstr, shEmul, reg, f32bit)

    //  Get component from the operand QuadFloat register.
    op = reg[shInstr->getOp1SwizzleMode() & 0x03];

    //  Check if absolute value flag is enable.
    if(shInstr->getOp1AbsoluteFlag())
        op = GPUMath::ABS(op);      //  Get absolute value of the first operand.

    //  Check if negate flag is enabled.
    if(shInstr->getOp1NegateFlag())
        op = -op;                   //  Negate.

//printf("reading scalar op1 = %f\n", op);
}

//  Reads and returns the second float scalar operand.
inline void ShaderEmulator::readScalarOp2(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, f32bit &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();
    QuadFloat reg;

    //  Get pointer to second QuadFloat operand register.
    READOPERAND(2, shInstrDec, shInstr, shEmul, reg, f32bit)

    //  Get component from the operand QuadFloat register.
    op = reg[shInstr->getOp2SwizzleMode() & 0x03];

    //  Check if absolute value flag is enable.
    if(shInstr->getOp2AbsoluteFlag())
        op = GPUMath::ABS(op);      //  Get absolute value of the first operand.

    //  Check if negate flag is enabled.
    if(shInstr->getOp2NegateFlag())
        op = -op;                   //  Negate.

//printf("reading scalar op2 = %f\n", op);
}

//  Reads and returns the first float scalar operand.
inline void ShaderEmulator::readScalarOp1(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, s32bit &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();
    QuadInt reg;

    //  Get pointer to first QuadFloat operand register.
    READOPERAND(1, shInstrDec, shInstr, shEmul, reg, s32bit)

    //  Get component from the operand QuadFloat register.
    op = reg[shInstr->getOp1SwizzleMode() & 0x03];

    //  Check if absolute value flag is enable.
    if(shInstr->getOp1AbsoluteFlag())
        op = GPUMath::ABS(op);      //  Get absolute value of the first operand.

    //  Check if negate flag is enabled.
    if(shInstr->getOp1NegateFlag())
        op = -op;                   //  Negate.

//printf("reading scalar op1 = %f\n", op);
}

//  Reads and returns the second float scalar operand.
inline void ShaderEmulator::readScalarOp2(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, s32bit &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();
    QuadInt reg;

    //  Get pointer to second QuadFloat operand register.
    READOPERAND(2, shInstrDec, shInstr, shEmul, reg, s32bit)

    //  Get component from the operand QuadFloat register.
    op = reg[shInstr->getOp2SwizzleMode() & 0x03];

    //  Check if absolute value flag is enable.
    if(shInstr->getOp2AbsoluteFlag())
        op = GPUMath::ABS(op);      //  Get absolute value of the first operand.

    //  Check if negate flag is enabled.
    if(shInstr->getOp2NegateFlag())
        op = -op;                   //  Negate.

//printf("reading scalar op2 = %f\n", op);
}

//  Reads and returns the first float scalar operand.
inline void ShaderEmulator::readScalarOp1(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, bool &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Check if the operand is an implicit boolean operand (aliased to absolute flag).
    if (shInstr->getOp1AbsoluteFlag())
    {
        // Get the implicit boolean operand (aliased to negate flag).
        op = shInstr->getOp1NegateFlag();
    }
    else
    {    
        //  Check if the operand comes from a constant register.
        if (shInstr->getBankOp1() == PARAM)
        {
            QuadInt reg;

            //  Get pointer to first QuadFloat operand register.
            READOPERAND(1, shInstrDec, shInstr, shEmul, reg, s32bit)

            //  Get component from the operand QuadInt register.
            op = (reg[shInstr->getOp1SwizzleMode() & 0x03] != 0);
        }
        else
        {
            //  Read the operand value from a predicate register.
            op = *((bool *) shInstrDec.getShEmulOp1());
        }
        
        //  Check if the operands is inverted (aliased to negate flag)
        if(shInstr->getOp1NegateFlag())
            op = !op;
    }
    
    
//printf("reading scalar op1 = %s\n", op ? "true" : "false");
}

//  Reads and returns the first float scalar operand.
inline void ShaderEmulator::readScalarOp2(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, bool &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Check if the operand is an implicit boolean operand (aliased to absolute flag).
    if (shInstr->getOp2AbsoluteFlag())
    {
        // Get the implicit boolean operand (aliased to negate flag).
        op = shInstr->getOp2NegateFlag();
    }
    else
    {    
        //  Check if the operand comes from a constant register.
        if (shInstr->getBankOp2() == PARAM)
        {
            QuadInt reg;
            
            //  Get pointer to first QuadFloat operand register.
            READOPERAND(2, shInstrDec, shInstr, shEmul, reg, s32bit)

            //  Get component from the operand QuadInt register.
            op = (reg[shInstr->getOp2SwizzleMode() & 0x03] != 0);
        }
        else
        {
            //  Read the operand value from a predicate register.
            op = *((bool *) shInstrDec.getShEmulOp2());
        }

        //  Check if the operands is inverted (aliased to negate flag)
        if(shInstr->getOp2NegateFlag())
            op = !op;
    }
    
//printf("reading scalar op1 = %s\n", op ? "true" : "false");
}

//  Reads operand from a 1-operand shader instruction.
inline void ShaderEmulator::read1Operands(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, QuadFloat &op)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get first operand.
    shEmul.readOperand1(shInstrDec, shEmul, op);

    //  Swizzle first operand.
    op = shEmul.swizzle(shInstr->getOp1SwizzleMode(), op);

    //  If the first operand absolute flag enabled.
    if(shInstr->getOp1AbsoluteFlag())
        op = shEmul.absolute(op);     //  Get absolute value of the first operand.

    //  Check if the first operand negate flag is enabled.
    if(shInstr->getOp1NegateFlag())
        op = shEmul.negate(op);

//printf("reading op1 = {%f, %f, %f, %f}\n", op[0], op[1], op[2], op[3]);
}

//  Reads operands from a 2-operands shader instruction.
inline void ShaderEmulator::read2Operands(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul,
                                          QuadFloat &op1, QuadFloat &op2)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get first operand.
    shEmul.readOperand1(shInstrDec, shEmul, op1);

    //  Swizzle first operand.
    op1 = shEmul.swizzle(shInstr->getOp1SwizzleMode(), op1);

    //  If the first operand absolute flag enabled.
    if(shInstr->getOp1AbsoluteFlag())
        op1 = shEmul.absolute(op1);     //  Get absolute value of the first operand.

    //  Check if the first operand negate flag is enabled.
    if(shInstr->getOp1NegateFlag())
        op1 = shEmul.negate(op1);       //  Negate.

    //  Get second operand.
    shEmul.readOperand2(shInstrDec, shEmul, op2);

    //  Swizzle second operand.
    op2 = shEmul.swizzle(shInstr->getOp2SwizzleMode(), op2);

    //  If the second operand absolute flag enabled.
    if(shInstr->getOp2AbsoluteFlag())
        op2 = shEmul.absolute(op2);     //  Get absolute value of the second operand.

    //  Check if the second operand negate flag is enabled.
    if(shInstr->getOp2NegateFlag())
        op2 = shEmul.negate(op2);       //  Negate.

//printf("reading op1 = {%f, %f, %f, %f}\n", op1[0], op1[1], op1[2], op1[3]);
//printf("reading op2 = {%f, %f, %f, %f}\n", op2[0], op2[1], op2[2], op2[3]);
}

//  Reads operands from a 3-operands shader instruction.
inline void ShaderEmulator::read3Operands(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul,
                                          QuadFloat &op1, QuadFloat &op2, QuadFloat &op3)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get first operand.
    shEmul.readOperand1(shInstrDec, shEmul, op1);

    //  Swizzle first operand.
    op1 = shEmul.swizzle(shInstr->getOp1SwizzleMode(), op1);

    //  If the first operand absolute flag enabled.
    if(shInstr->getOp1AbsoluteFlag())
        op1 = shEmul.absolute(op1);     //  Get absolute value of the first operand.

    //  Check if the first operand negate flag is enabled.
    if(shInstr->getOp1NegateFlag())
        op1 = shEmul.negate(op1);       //  Negate.

    //  Get second operand.
    shEmul.readOperand2(shInstrDec, shEmul, op2);

    //  Swizzle second operand.
    op2 = shEmul.swizzle(shInstr->getOp2SwizzleMode(), op2);

    //  If the second operand absolute flag enabled.
    if(shInstr->getOp2AbsoluteFlag())
        op2 = shEmul.absolute(op2);     //  Get absolute value of the second operand.

    //  Check if the second operand negate flag is enabled.
    if(shInstr->getOp2NegateFlag())
        op2 = shEmul.negate(op2);       //  Negate.

    //  Get third operand.
    shEmul.readOperand3(shInstrDec, shEmul, op3);

    //  Swizzle third operand.
    op3 = shEmul.swizzle(shInstr->getOp3SwizzleMode(), op3);

    //  If the third operand absolute flag enabled.
    if(shInstr->getOp3AbsoluteFlag())
        op3 = shEmul.absolute(op3);     //  Get absolute value of the third operand.

    //  Check if the third operand negate flag is enabled.
    if(shInstr->getOp3NegateFlag())
        op3 = shEmul.negate(op3);       //  Negate.

//printf("reading op1 = {%f, %f, %f, %f}\n", op1[0], op1[1], op1[2], op1[3]);
//printf("reading op2 = {%f, %f, %f, %f}\n", op2[0], op2[1], op2[2], op2[3]);
//printf("reading op3 = {%f, %f, %f, %f}\n", op3[0], op3[1], op3[2], op3[3]);
}

//  Reads operands from a 2-operands shader instruction.
inline void ShaderEmulator::read2Operands(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul,
                                          QuadInt &op1, QuadInt &op2)
{
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get first operand.
    shEmul.readOperand1(shInstrDec, shEmul, op1);

    //  Swizzle first operand.
    op1 = shEmul.swizzle(shInstr->getOp1SwizzleMode(), op1);

    //  If the first operand absolute flag enabled.
    if(shInstr->getOp1AbsoluteFlag())
        op1 = shEmul.absolute(op1);     //  Get absolute value of the first operand.

    //  Check if the first operand negate flag is enabled.
    if(shInstr->getOp1NegateFlag())
        op1 = shEmul.negate(op1);       //  Negate.

    //  Get second operand.
    shEmul.readOperand2(shInstrDec, shEmul, op2);

    //  Swizzle second operand.
    op2 = shEmul.swizzle(shInstr->getOp2SwizzleMode(), op2);

    //  If the second operand absolute flag enabled.
    if(shInstr->getOp2AbsoluteFlag())
        op2 = shEmul.absolute(op2);     //  Get absolute value of the second operand.

    //  Check if the second operand negate flag is enabled.
    if(shInstr->getOp2NegateFlag())
        op2 = shEmul.negate(op2);       //  Negate.

//printf("reading op1 = {%f, %f, %f, %f}\n", op1[0], op1[1], op1[2], op1[3]);
//printf("reading op2 = {%f, %f, %f, %f}\n", op2[0], op2[1], op2[2], op2[3]);
}


//  Writes the quadfloat result of a shader instruction.
inline void ShaderEmulator::writeResult(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, f32bit *result)
{
    f32bit *res;
    f32bit aux[4];
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();
   
    //  Get address for the result register.
    res = (f32bit *) shInstrDec.getShEmulResult();

    //  Get predication for the instruction.
    bool *predicateReg = (bool *) shInstrDec.getShEmulPredicate();
    bool predicateValue = (predicateReg == NULL) ? true : (( (*predicateReg) && !shInstr->getNegatePredicateFlag()) ||
                                                           (!(*predicateReg) &&  shInstr->getNegatePredicateFlag()));
    
    //  Check saturated result flag.
    if (shInstr->getSaturatedRes())
    {
        //  Clamp the result vector components to [0, 1].
        GPUMath::SAT(result, aux);


        //  Write Mask.  Only write in the result register the selected components.
        shEmul.writeResReg(*shInstr, aux, res, predicateValue);
    }
    else
    {
        //  Write Mask.  Only write in the result register the selected components.
        shEmul.writeResReg(*shInstr, result, res, predicateValue);
    }
}

//  Writes the quadint result of a shader instruction.
inline void ShaderEmulator::writeResult(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, s32bit *result)
{
    s32bit *res;
    s32bit aux[4];
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();
   
    //  Get address for the result register.
    res = (s32bit *) shInstrDec.getShEmulResult();

    //  Get predication for the instruction.
    bool *predicateReg = (bool *) shInstrDec.getShEmulPredicate();
    bool predicateValue = (predicateReg == NULL) ? true : (( (*predicateReg) && !shInstr->getNegatePredicateFlag()) ||
                                                           (!(*predicateReg) &&  shInstr->getNegatePredicateFlag()));
    
    //  Write Mask.  Only write in the result register the selected components.
    shEmul.writeResReg(*shInstr, result, res, predicateValue);
}


//  Writes the fixed point quad result to the special fixed point accumulator.
inline void ShaderEmulator::writeResult(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul,
    FixedPoint *result)
{
    FixedPoint *res;         //  GPU fixed point quad accumulator.

    //  Get the shader instruction.
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();

    //  Get the fixed point quad accumulator associated with the thread.
    res = accumFXPBank[shInstrDec.getNumThread()];

    //  NOTE:  Saturation not supported for fixed point quad accumulator.

    //  Get predication for the instruction.
    bool *predicateReg = (bool *) shInstrDec.getShEmulPredicate();
    bool predicateValue = (predicateReg == NULL) ? true : (( (*predicateReg) && !shInstr->getNegatePredicateFlag()) ||
                                                           (!(*predicateReg) &&  shInstr->getNegatePredicateFlag()));

    //  Write Mask.  Only write in the result register the selected components.
    shEmul.writeResReg(*shInstr, result, res, predicateValue);
}

inline void ShaderEmulator::writeResult(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul, bool result)
{
    bool *res;
    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();
   
    //  Get address for the result predicate register.
    res = (bool *) shInstrDec.getShEmulResult();

    //  Get predication for the instruction.
    bool *predicateReg = (bool *) shInstrDec.getShEmulPredicate();
    bool predicateValue = (predicateReg == NULL) ? true : (( (*predicateReg) && !shInstr->getNegatePredicateFlag()) ||
                                                           (!(*predicateReg) &&  shInstr->getNegatePredicateFlag()));

    //  Check if predication allows for the instruction to write a result.
    if (predicateValue)
    {
        //  Check if the result has to be negated (aliased to sature flag).
        if (shInstr->getSaturatedRes())
            result = !result;
    
        //  Write the result into the predicate result register.
        *res = result;
    }    
}

bool ShaderEmulator::checkJump(gpu3d::ShaderInstruction::ShaderInstructionDecoded *shInstrDec, u32bit vectorLength, u32bit &destPC)
{
    //  Get the first thread corresponding to the vector.
    u32bit startThread = shInstrDec->getNumThread() - GPU_MOD(shInstrDec->getNumThread(), vectorLength);

    bool jump;
    
    //  Check if the operand is an implicit boolean operand (aliased to absolute flag).
    if (shInstrDec->getShaderInstruction()->getOp1AbsoluteFlag())
    {
        // Get the implicit boolean operand (aliased to negate flag).
        jump = shInstrDec->getShaderInstruction()->getOp1NegateFlag();
    }
    else
    {    
        //  Get the register identifer for the first operand.
        u32bit op1 = shInstrDec->getShaderInstruction()->getOp1();
        
        //  Get if the first operand is inverted (aliased to negate flag).
        bool negated = shInstrDec->getShaderInstruction()->getOp1NegateFlag();
        
        //  Check if the operand comes from a constant register.
        if (shInstrDec->getShaderInstruction()->getBankOp1() == PARAM)
        {
            //  Get the component of the constant register that is used as operand.
            u32bit component = shInstrDec->getShaderInstruction()->getOp1SwizzleMode() & 0x03;
            
            //  Initialize jump condition for the whole vector.
            jump = true;
            
            //  Check all threads/elements in the vector.
            for(u32bit t = 0; t < vectorLength; t++)
            {
                //  Get the condition for the current thread.
                bool jumpThread = ((u32bit *) &constantBank)[op1 * (UNIFIED_CONST_REG_SIZE/4) + component];

                //  Invert if required.
                if (negated)
                    jumpThread = !jumpThread;
                                    
                //  Update the jump condition for the whole vector.
                jump = jump && jumpThread;
            }
            
        }
        else
        {
            //  Initialize jump condition for the whole vector.
            jump = true;
            
            //  Check all threads/elements in the vector.
            for(u32bit t = 0; t < vectorLength; t++)
            {
                //  Get the condition for the current thread.
                bool jumpThread = predicateBank[startThread + t][op1];

                //  Invert if required.
                if (negated)
                    jumpThread = !jumpThread;
                                    
                //  Update the jump condition for the whole vector.
                jump = jump && jumpThread;
            }
        }
    }
    
    //  If the jump condition is true update the PCs and return the destination PC.
    if (jump)
    {
        //  Compute the target PC.
        destPC = PCTable[startThread] + shInstrDec->getShaderInstruction()->getJumpOffset();
    }
    else
    {
        //  The destination PC is the next instruction.
        destPC = PCTable[startThread] + 1;
    }
    
    //  Update the PCs for all the threads/elements in the vector.
    for(u32bit t = 0; t < vectorLength; t++)
        PCTable[startThread + t] = destPC;
    
    return jump;
}


//
//  Shader instruction implementation.
//

void ShaderEmulator::shNOP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shADD(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::ADD(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);
//PRINT_OPERATION_RES_2
    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shARL(ShaderInstruction::ShaderInstructionDecoded &shInstrDec, ShaderEmulator &shEmul)
{
    QuadFloat op;
    QuadInt *res;
    s32bit *result;

    ShaderInstruction *shInstr = shInstrDec.getShaderInstruction();
    
    //  Get predication for the instruction.
    bool *predicateReg = (bool *) shInstrDec.getShEmulPredicate();
    bool predicateValue = (predicateReg == NULL) ? true : (( (*predicateReg) && !shInstr->getNegatePredicateFlag()) ||
                                                           (!(*predicateReg) &&  shInstr->getNegatePredicateFlag()));

    //  Check if the predicate value allows the instruction to write a result.
    if (predicateValue)
    {
        //  Read instruction operands.
        shEmul.read1Operands(shInstrDec, shEmul, op);

        //  Get the result address register.
        res = (QuadInt *) shInstrDec.getShEmulResult();

        //  Get pointer to the address register values.
        result = res->getVector();

        //  Perform instruction operation and write the result.
        GPUMath::ARL(result, op.getVector());
    }
    
    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstrDec.getNumThread()]++;

}

void ShaderEmulator::shCOS(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.readScalarOp1(shInstr, shEmul, op);

    /*  Perform instruction operation.  */
    GPUMath::COS(op, result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shSIN(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.readScalarOp1(shInstr, shEmul, op);

    /*  Perform instruction operation.  */
    GPUMath::SIN(op, result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shDP3(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::DP3(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shDP4(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::DP4(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;

}

void ShaderEmulator::shDPH(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::DPH(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shDST(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::DST(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shEX2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.readScalarOp1(shInstr, shEmul, op);

    /*  Perform instruction operation.  */
    GPUMath::EX2(op, result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shEXP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.readScalarOp1(shInstr, shEmul, op);

    /*  Perform instruction operation.  */
    GPUMath::EXP(op, result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shFLR(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    panic("ShaderEmulator", "shFLR",  "Instruction not implemented.");
}

void ShaderEmulator::shFRC(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Perform instruction operation.  */
    GPUMath::FRC(op1.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shLG2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.readScalarOp1(shInstr, shEmul, op);

    /*  Perform instruction operation.  */
    GPUMath::LG2(op, result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
    //panic("ShaderEmulator", "shLG2",  "Instruction not implemented.");
}

void ShaderEmulator::shLIT(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Perform instruction operation.  */
    GPUMath::LIT(op1.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shLOG(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.readScalarOp1(shInstr, shEmul, op);

    /*  Perform instruction operation.  */
    GPUMath::LOG(op, result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shMAD(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2, op3;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read3Operands(shInstr, shEmul, op1, op2, op3);

    /*  Perform instruction operation.  */
    GPUMath::MAD(op1.getVector(), op2.getVector(),op3.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);
//PRINT_OPERATION_RES_3
    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shMAX(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::MAX(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shMIN(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::MIN(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shMOV(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Perform instruction operation.  */
    GPUMath::MOV(op1.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shMUL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::MUL(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);
//PRINT_OPERATION_RES_2
    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shRCP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.readScalarOp1(shInstr, shEmul, op);

    /*  Perform instruction operation.  */
    GPUMath::RCP(op, result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

//PRINT_OPERATION_RES_1

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shRSQ(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.readScalarOp1(shInstr, shEmul, op);

    /*  Perform instruction operation.  */
    GPUMath::RSQ(op, result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shSGE(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::SGE(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shSLT(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    /*  Perform instruction operation.  */
    GPUMath::SLT(op1.getVector(), op2.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}


void ShaderEmulator::shTEX(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    /*  Read instruction operands. */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Add the texture access to the queue.  */
    shEmul.textureOperation(shInstr, TEXTURE_READ, op1, 0.0f);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shTXB(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    /*  Read instruction operands. */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Add the texture access to the queue.  */
    shEmul.textureOperation(shInstr, TEXTURE_READ, op1, op1[3]);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shTXP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    /*  Read instruction operands. */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Project texture coordinate.  */
    op1[0] = op1[0]/op1[3];
    op1[1] = op1[1]/op1[3];
    op1[2] = op1[2]/op1[3];

    /*  Add the texture access to the queue.  */
    shEmul.textureOperation(shInstr, TEXTURE_READ, op1, 0.0f);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shTXL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    /*  Read instruction operands. */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Add the texture access to the queue.  */
    shEmul.textureOperation(shInstr, TEXTURE_READ_WITH_LOD, op1, op1[3]);

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shKIL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    /*  Read instruction operands.  */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Check kill condition.  */
    if ((op1[0] < 0.0f) || (op1[1] < 0.0f) || (op1[2] < 0.0f) || (op1[3] < 0.0f))
    {
        /*  Set the thread kill flag.  */
        shEmul.kill[shInstr.getNumThread()][shEmul.sampleIdx[shInstr.getNumThread()]] = TRUE;
    }

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shKLS(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;
    u32bit sampleId;

    /*  Read instruction operands.  */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Read KLS's sample identifier.  */
    sampleId = shInstr.getShaderInstruction()->getOp2();

    /*  Check kill condition.  */
    if ((op1[0] < 0.0f) || (op1[1] < 0.0f) || (op1[2] < 0.0f) || (op1[3] < 0.0f))
    {
        /*  Set the thread kill flag.  */
        shEmul.kill[shInstr.getNumThread()][sampleId] = TRUE;
    }

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shZXP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    /*  Read instruction operands.  */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Export Z value  */
    shEmul.zexport[shInstr.getNumThread()][0] = (f32bit) op1[0];

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shZXS(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;
    u32bit sampleId;

    /*  Read instruction operands.  */
    shEmul.read1Operands(shInstr, shEmul, op1);

    /*  Read ZXS's sample identifier.  */
    sampleId = shInstr.getShaderInstruction()->getOp2();

    /*  Export Z values  */
    for (u32bit i = 0; i < 4; i++)
        shEmul.zexport[shInstr.getNumThread()][(shEmul.sampleIdx[shInstr.getNumThread()]/4) + i] = (f32bit) op1[i];

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shCMP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2, op3;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read3Operands(shInstr, shEmul, op1, op2, op3);

    /*  Perform instruction operation.  */
    GPUMath::CMP(op1.getVector(), op2.getVector(),op3.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);
//PRINT_OPERATION_RES_3
    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shCMPKIL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2, op3;
    f32bit result[4];

    /*  Read instruction operands. */
    shEmul.read3Operands(shInstr, shEmul, op1, op2, op3);

    /*  Perform instruction operation.  */
    GPUMath::CMP(op1.getVector(), op2.getVector(),op3.getVector(), result);

    /*  Write instruction result.  */
    shEmul.writeResult(shInstr, shEmul, result);
//PRINT_OPERATION_RES_3
    /*  Check kill condition.  */
    if (((shInstr.getShaderInstruction()->getResultMaskMode() & 0x08) && (result[0] < 0.0f)) ||
        ((shInstr.getShaderInstruction()->getResultMaskMode() & 0x04) && (result[1] < 0.0f)) ||
        ((shInstr.getShaderInstruction()->getResultMaskMode() & 0x02) && (result[2] < 0.0f)) ||
        ((shInstr.getShaderInstruction()->getResultMaskMode() & 0x01) && (result[3] < 0.0f)))
    {
        /*  Set the thread kill flag.  */
        shEmul.kill[shInstr.getNumThread()][shEmul.sampleIdx[shInstr.getNumThread()]] = TRUE;
    }

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shCHS(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    /*  NO INSTRUCTION OPERANDS TO READ.  */

    /*  Point sample index to the next.  */
    shEmul.sampleIdx[shInstr.getNumThread()]++;

    /*  Update ShaderEmulator PC.  */
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shLDA(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    //  Read instruction operands.
    shEmul.read1Operands(shInstr, shEmul, op1);

    //  Add the texture access to the queue.
    shEmul.textureOperation(shInstr, ATTRIBUTE_READ, op1, 0.0f);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}


void ShaderEmulator::shFXMUL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    FixedPoint fxpOp1[4];
    FixedPoint fxpOp2[4];
    FixedPoint fxpResult[4];

    //  Read instruction operands (fp 32-bit).
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    //  For all components of the registers.
    for(u32bit c = 0; c < 4; c++)
    {
        //  Convert first operand to fixed point.
        fxpOp1[c] = FixedPoint(op1[c], 16, shEmul.fxpDecBits);

        //  Convert second operand to fixed point.
        fxpOp2[c] = FixedPoint(op2[c], 16, shEmul.fxpDecBits);

        //  Initialize the result with 2x precision.
        fxpResult[c] = FixedPoint(1.0f, 32, 2 * shEmul.fxpDecBits);

        //  Perform multiplication with 2x precision.
        fxpResult[c] = fxpResult[c] * fxpOp1[c] * fxpOp2[c];
    }

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, fxpResult);
//PRINT_FXMUL
    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shFXMAD(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2;
    FixedPoint fxpOp1[4];
    FixedPoint fxpOp2[4];
    FixedPoint fxpOp3[4];
    FixedPoint fxpResult[4];
    f32bit result[4];

    //  NOTE:  No swizzle or modifiers for third operand supported.

    //  Read instruction operands (fp 32-bit).
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    //  For all components of the registers.
    for(u32bit c = 0; c < 4; c++)
    {
        //  Convert first operand to fixed point.
        fxpOp1[c] = FixedPoint(op1[c], 16, shEmul.fxpDecBits);

        //  Convert second operand to fixed point.
        fxpOp2[c] = FixedPoint(op2[c], 16, shEmul.fxpDecBits);

        //  Get third operand from the fixed point quad accumulator per thread.
        fxpOp3[c] = shEmul.accumFXPBank[shInstr.getNumThread()][c];

        //  Initialize the result with 2x precision.
        fxpResult[c] = FixedPoint(1.0f, 32, 2 * shEmul.fxpDecBits);

        //  Perform multiplication and addition with 32.16 precision.
        fxpResult[c] = fxpResult[c] * fxpOp1[c] * fxpOp2[c] + fxpOp3[c];

        //  Convert to fp 32-bit.
        result[c] = fxpResult[c].toFloat32();
    }

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);
//PRINT_FXMAD
    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shFXMAD2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1, op2, op3;
    FixedPoint fxpOp1[4];
    FixedPoint fxpOp2[4];
    FixedPoint fxpOp3[4];
    FixedPoint fxpResult[4];

    //  Read instruction operands (fp 32-bit).
    shEmul.read3Operands(shInstr, shEmul, op1, op2, op3);

    //  For all components of the registers.
    for(u32bit c = 0; c < 4; c++)
    {
        //  Convert first operand to fixed point.
        fxpOp1[c] = FixedPoint(op1[c], 16, shEmul.fxpDecBits);

        //  Convert second operand to fixed point.
        fxpOp2[c] = FixedPoint(op2[c], 16, shEmul.fxpDecBits);

        //  Convert third operand to fixed point.
        fxpOp3[c] = FixedPoint(op3[c], 16, shEmul.fxpDecBits);

        //  Initialize the result with 2x precision.
        fxpResult[c] = FixedPoint(1.0f, 32, 2 * shEmul.fxpDecBits);

        //  Perform multiplication and addition with 32.16 precision.
        fxpResult[c] = fxpResult[c] * fxpOp1[c] * fxpOp2[c] + fxpOp3[c];
    }

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, fxpResult);
//PRINT_FXMAD2
    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shEND(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    /*  This instruction does nothing in the emulator side.  */
}

void ShaderEmulator::shSETPEQ(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op1;
    f32bit op2;
    bool result;

    //  Read instruction first scalar operand.
    shEmul.readScalarOp1(shInstr, shEmul, op1);

    //  Read instruction second scalar operand.
    shEmul.readScalarOp2(shInstr, shEmul, op2);

    //  Perform instruction operation.
    GPUMath::SETPEQ(op1, op2, result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shSETPGT(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op1;
    f32bit op2;
    bool result;

    //  Read instruction first scalar operand.
    shEmul.readScalarOp1(shInstr, shEmul, op1);

    //  Read instruction second scalar operand.
    shEmul.readScalarOp2(shInstr, shEmul, op2);

    //  Perform instruction operation.
    GPUMath::SETPGT(op1, op2, result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shSETPLT(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    f32bit op1;
    f32bit op2;
    bool result;

    //  Read instruction first scalar operand.
    shEmul.readScalarOp1(shInstr, shEmul, op1);

    //  Read instruction second scalar operand.
    shEmul.readScalarOp2(shInstr, shEmul, op2);

    //  Perform instruction operation.
    GPUMath::SETPLT(op1, op2, result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shANDP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    bool op1;
    bool op2;
    bool result;

    //  Read instruction first scalar operand.
    shEmul.readScalarOp1(shInstr, shEmul, op1);

    //  Read instruction second scalar operand.
    shEmul.readScalarOp2(shInstr, shEmul, op2);

    //  Perform instruction operation.
    GPUMath::ANDP(op1, op2, result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shJMP(gpu3d::ShaderInstruction::ShaderInstructionDecoded &shInstr, gpu3d::ShaderEmulator &shEmul)
{
    //  This instruction affects control and multiple 'threads' so it has to be implemented
    //  outside the emulator.
}

void ShaderEmulator::shDDX(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    //  Read instruction operands.
    shEmul.read1Operands(shInstr, shEmul, op1);

    //  Add the derivation operation to the current derivation structure.
    shEmul.derivOperation(shInstr, op1);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shDDY(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadFloat op1;

    //  Read instruction operands.
    shEmul.read1Operands(shInstr, shEmul, op1);

    //  Add the derivation operation to the current derivation structure.
    shEmul.derivOperation(shInstr, op1);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shADDI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadInt op1;
    QuadInt op2;
    s32bit result[4];
    
    //  Read instruction operands.
    shEmul.read2Operands(shInstr, shEmul, op1, op2);

    //  Perform instruction operation.
    GPUMath::ADDI(op1.getVector(), op2.getVector(), result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shMULI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    QuadInt op1;
    QuadInt op2;
    s32bit result[4];
        
    //  Read instruction operands.
    shEmul.read2Operands(shInstr, shEmul, op1, op2);
    
    //  Perform instruction operation.
    GPUMath::MULI(op1.getVector(), op2.getVector(), result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shSTPEQI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    s32bit op1;
    s32bit op2;
    bool result;

    //  Read instruction first scalar operand.
    shEmul.readScalarOp1(shInstr, shEmul, op1);

    //  Read instruction second scalar operand.
    shEmul.readScalarOp2(shInstr, shEmul, op2);

    //  Perform instruction operation.
    GPUMath::STPEQI(op1, op2, result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shSTPGTI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    s32bit op1;
    s32bit op2;
    bool result;

    //  Read instruction first scalar operand.
    shEmul.readScalarOp1(shInstr, shEmul, op1);

    //  Read instruction second scalar operand.
    shEmul.readScalarOp2(shInstr, shEmul, op2);

    //  Perform instruction operation.
    GPUMath::STPGTI(op1, op2, result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}

void ShaderEmulator::shSTPLTI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    s32bit op1;
    s32bit op2;
    bool result;

    //  Read instruction first scalar operand.
    shEmul.readScalarOp1(shInstr, shEmul, op1);

    //  Read instruction second scalar operand.
    shEmul.readScalarOp2(shInstr, shEmul, op2);

    //  Perform instruction operation.
    GPUMath::STPEQI(op1, op2, result);

    //  Write instruction result.
    shEmul.writeResult(shInstr, shEmul, result);

    //  Update ShaderEmulator PC.
    shEmul.PCTable[shInstr.getNumThread()]++;
}


void ShaderEmulator::shIllegal(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul)
{
    panic("ShaderEmulator", "shIllegal",  "Instruction not implemented.");
}





/*    shIllegal                                       /*  Opcodes 2Ch - 2Fh  */


/*
 *  Shader Instruction Emulation functions table.
 *
 */

void (*ShaderEmulator::shInstrEmulationTable[])(ShaderInstruction::ShaderInstructionDecoded &, ShaderEmulator &) =
{
    shNOP,    shADD,     shADDI,   shARL,       //  Opcodes 00h - 03h
    shANDP,   shIllegal, shNOP,    shCOS,       //  Opcodes 04h - 07h
    shDP3,    shDP4,     shDPH,    shDST,       //  Opcodes 08h - 0Bh
    shEX2,    shEXP,     shFLR,    shFRC,       //  Opcodes 0Ch - 0Fh

    shLG2,    shLIT,     shLOG,    shMAD,       //  Opcodes 10h - 13h
    shMAX,    shMIN,     shMOV,    shMUL,       //  Opcodes 14h - 17h
    shMULI,   shRCP,     shNOP,    shRSQ,       //  Opcodes 18h - 1Bh
    shSETPEQ, shSETPGT,  shSGE,    shSETPLT,    //  Opcodes 1Ch - 1Fh

    shSIN,    shSTPEQI,  shSLT,    shSTPGTI,    //  Opcodes 20h - 23h
    shSTPLTI, shTXL,     shTEX,    shTXB,       //  Opcoded 24h - 27h
    shTXP,    shKIL,     shKLS,    shZXP,       //  Opcodes 28h - 2Bh
    shZXS,    shCMP,     shCMPKIL, shCHS,       //  Opcodes 2Ch - 2Fh
    shLDA,    shFXMUL,   shFXMAD,  shFXMAD2,    //  Opcodes 30h - 33h
    shDDX,    shDDY,     shJMP,    shEND        //  Opcodes 34h - 37h
};


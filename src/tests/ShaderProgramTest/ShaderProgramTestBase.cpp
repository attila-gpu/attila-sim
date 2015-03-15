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

#include "ShaderProgramTestBase.h"
#include "ConfigLoader.h"
#include "ACDXProgramExecutionEnvironment.h"
#include "ShaderOptimization.h"
#include "ShaderOptimizer.h"
#include "ShaderEmulator.h"
#include "TextureEmulator.h"
#include <limits>

using namespace gpu3d;
using namespace acdlib;

ShaderProgramTestBase::ShaderProgramTestBase( const SimParameters& simP )
:  compiledCode(0), codeSize(0), loadedProgram(false)
{
    // Initialize all the Compilers and Shader Emulation classes

    //  Create ARB 1.0 compilers.
    arbvp10comp = new acdlib::ACDXVP1ExecEnvironment;
    arbfp10comp = new acdlib::ACDXFP1ExecEnvironment;

    acdlib_opt::SHADER_ARCH_PARAMS shArchPar;

    shArchPar.nWay = 4;
    shArchPar.shArchParams->selectArchitecture("VarLatAOS");
    shArchPar.temporaries = 32;

    //  Create the shader optimizer module (static instruction scheduling)
    shOptimizer = new acdlib_opt::ShaderOptimizer(shArchPar);

    //  Create texture emulator object.
    texEmu = new TextureEmulator(
        STAMP_FRAGMENTS,                /*  Fragments per stamp.  */
        simP.fsh.textBlockDim,          /*  Texture block dimension (texels): 2^n x 2^n.  */
        simP.fsh.textSBlockDim,         /*  Texture superblock dimension (blocks): 2^m x 2^m.  */
        simP.fsh.anisoAlgo,             /*  Anisotropy algorithm selected.  */
        simP.fsh.forceMaxAniso,         /*  Force the maximum anisotropy from the configuration file for all textures.  */
        simP.fsh.maxAnisotropy,         /*  Maximum anisotropy allowed for any texture.  */
        simP.fsh.triPrecision,          /*  Trilinear precision.  */
        simP.fsh.briThreshold,          /*  Brilinear threshold.  */
        simP.fsh.anisoRoundPrec,        /*  Aniso ratio rounding precision.  */
        simP.fsh.anisoRoundThres,       /*  Aniso ratio rounding threshold.  */
        simP.fsh.anisoRatioMultOf2,     /*  Aniso ratio must be multiple of two.  */          
        simP.ras.overScanWidth,         /*  Over scan tile width (scan tiles).  */
        simP.ras.overScanHeight,        /*  Over scan tile height (scan tiles).  */
        simP.ras.scanWidth,             /*  Scan tile width (pixels).  */
        simP.ras.scanHeight,            /*  Scan tile height (pixels).  */
        simP.ras.genWidth,              /*  Generation tile width (pixels).  */
        simP.ras.genHeight              /*  Generation tile height (pixels).  */
        );

    //  Create the shader emulator object.
    shEmu = new ShaderEmulator(
        "ShaderUnit",             //  Shader name.
        UNIFIED,                  //  Shader model. 
        1,                        //  Threads supported by the shader (state).
        1,                        //  Threads active in the shader (executable).
        texEmu,                   //  Pointer to the texture emulator attached to the shader.
        STAMP_FRAGMENTS           //  Fragments per stamp for texture accesses.
    );

    //  Initialize input attributes vector
    programInputs = new QuadFloat[32];

    //  Initialize the constant registers;
    programConstants = new QuadFloat[512];
}

void ShaderProgramTestBase::printCompiledProgram(vector<ShaderInstruction*> program, bool showBinary)
{
    //  Disassemble the program.
    for(unsigned int instr = 0; instr < program.size(); instr++)
    {
        char instrDisasm[256];
        unsigned char instrCode[16];

        //  Disassemble the instruction.
        program[instr]->disassemble(instrDisasm);

        //  Print instruction PC
        printf("%03d :", instr);

        if (showBinary)
        {
            //  Get the instruction code.
            program[instr]->getCode(instrCode);

            printf("%04x :", instr * 16);
            for(u32bit b = 0; b < 16; b++)
                printf(" %02x", instrCode[b]);
        }

        //  Print the disassembled instruction.
        printf("    %s\n", instrDisasm);
    }
}

void ShaderProgramTestBase::compileProgram(const std::string& code, bool optimize, bool transform, compileModel compModel)
{
    vector<ACDVector<acd_float,4> > constants;

    vector<ShaderInstruction*> compiledProgram;
    vector<ShaderInstruction*> optimizedProgram;
    vector<ShaderInstruction*> transformedProgram;

    ResourceUsage resourceUsage;
    unsigned int maxLiveTempRegs;

    cout << code;

    if (compiledCode)
        delete compiledCode;

    codeSize = 0;
    maxLiveTempRegs = 0;

    shaderModel = compModel;

    switch(shaderModel)
    {
        case OGL_ARB_1_0:
        {
            ACDXRBank<float> constantBank(96,"");

            if (!code.compare( 0, 10, "!!ARBvp1.0" ))
            {    
                target = SPT_VERTEX_TARGET;
                compiledCode = arbvp10comp->compile(code, codeSize, constantBank, resourceUsage);
            }
            else if (!code.compare( 0, 10, "!!ARBfp1.0" ))
            {    
                target = SPT_FRAGMENT_TARGET;
                compiledCode = arbfp10comp->compile(code, codeSize, constantBank, resourceUsage);
            }
            else
                panic("ShaderProgramTestBase","compileProgram","Unexpected or not supported ARB 1.0 program target");
        
            constants.resize(constantBank.size());

            //  Copy constant values.
            for (unsigned int i = 0; i < constantBank.size(); i++)
            {
                constants[0] = constantBank.get(i)[0];
                constants[1] = constantBank.get(i)[1];   
                constants[2] = constantBank.get(i)[2];   
                constants[3] = constantBank.get(i)[3];   
            }

            break;
        }
        default:
            panic("ShaderProgramTestBase","compileProgram","Unexpected or not supported shader model");

    }
    
    ShaderOptimization::decodeProgram(compiledCode, codeSize, compiledProgram);

    //  Allocate space for the new transformed code.
    delete compiledCode;

    compiledCode = new unsigned char[compiledProgram.size() * SHINSTRSIZE];

    if (!compiledCode)
    {
        panic("ShaderProgramTestBase", "compileProgram", "Could not allocate memory for compiled program");
    }

    codeSize = compiledProgram.size() * SHINSTRSIZE;

    //  Encode binary.
    ShaderOptimization::encodeProgram(compiledProgram, compiledCode, codeSize);

    if (optimize)
    {        
        shOptimizer->setCode(compiledCode, codeSize);
        shOptimizer->setConstants(constants);

        shOptimizer->optimize();
      
        unsigned int optCodeSize;
        const acd_ubyte* optCode = shOptimizer->getCode(optCodeSize);
        
        u8bit* optimizedCode = new u8bit[optCodeSize];

        memcpy(optimizedCode, optCode, optCodeSize);
    
        //  Decode optimized program version again.
        ShaderOptimization::deleteProgram(compiledProgram);
        ShaderOptimization::decodeProgram(optimizedCode, optCodeSize, compiledProgram);

        delete optimizedCode;

        //  Perform low level driver optimizations.
        ShaderOptimization::optimize(compiledProgram, optimizedProgram, maxLiveTempRegs, false, false, false);
    
        //  Overwrite with the optimized version.
        ShaderOptimization::deleteProgram(compiledProgram);
        ShaderOptimization::copyProgram(optimizedProgram, compiledProgram);
    }
    
    //  Allocate space for the new transformed code.
    delete compiledCode;

    compiledCode = new unsigned char[compiledProgram.size() * SHINSTRSIZE];

    if (!compiledCode)
    {
        panic("ShaderProgramTestBase", "compileProgram", "Could not allocate memory for compiled program");
    }

    codeSize = compiledProgram.size() * SHINSTRSIZE;

    //  Encode binary.
    ShaderOptimization::encodeProgram(compiledProgram, compiledCode, codeSize);

    //  Set loaded flag.
    loadedProgram = true;

    //  Print compiled program.
    printCompiledProgram(compiledProgram, false);

    cout << "\nCompilation info: \n_________________\n";

    cout << "Number of instructions: " << compiledProgram.size();
    cout << "\nTotal program size: " << codeSize;
    cout << "\nMax ALive Registers: " << maxLiveTempRegs;
    cout << "\nALU:TEX Ratio: " << ShaderOptimization::getALUTEXRatio(compiledProgram) << "\n\n";

    //  Delete program.
    ShaderOptimization::deleteProgram(compiledProgram);
}

void ShaderProgramTestBase::defineProgramInput(unsigned int index, float* value)
{
    if (loadedProgram)
    {
        programInputs[index][0] = value[0];
        programInputs[index][1] = value[1];
        programInputs[index][2] = value[2];
        programInputs[index][3] = value[3];
    }
    else
    {
        cout << "Error: no shader program loaded." << endl;
    }
}

void ShaderProgramTestBase::defineProgramConstant(unsigned int index, float* value)
{
    unsigned int offset;

    if (loadedProgram)
    {
        if (target == FRAGMENT_TARGET)
        {
            switch(shaderModel)
            {
                case OGL_ARB_1_0: 
                case D3D_SHADER_MODEL_2_0:
                case D3D_SHADER_MODEL_3_0:

                    offset = VS2_CONSTANT_NUM_REGS;
                    
                    break;
                
                default:

                    offset = 0;
            }
        }
        else
        {
            offset = 0;
        }

        programConstants[index + offset][0] = value[0];
        programConstants[index + offset][1] = value[1];
        programConstants[index + offset][2] = value[2];
        programConstants[index + offset][3] = value[3];
    }
    else
    {
        cout << "Error: no shader program loaded." << endl;
    }
}

void ShaderProgramTestBase::executeProgram(int stopPC)
{
    ShaderInstruction::ShaderInstructionDecoded* instr;
    u32bit partition = (target == SPT_VERTEX_TARGET)? VERTEX_PARTITION : FRAGMENT_PARTITION;
    u32bit PC;
    bool lastInstr;
    u32bit initialPC;
    bool killFlag[MAX_MSAA_SAMPLES];
    u32bit i;

    if (loadedProgram)
    {
        //  Load current compiled shader program.
        shEmu->loadShaderProgram(compiledCode, VS2_INSTRUCTION_MEMORY_SIZE * partition, codeSize, partition);

        //  Initialize shader state (registers and kill flags set to zero/false).
        shEmu->resetShaderState(0);

        //  Load current input attributes
        shEmu->loadShaderState(0, IN, programInputs);
        
        //  Load shader program constants
        shEmu->loadShaderState(0, PARAM, programConstants);

        if (partition == FRAGMENT_PARTITION)
        {
            switch(shaderModel)
            {
                case OGL_ARB_1_0: 
                case D3D_SHADER_MODEL_2_0:
                case D3D_SHADER_MODEL_3_0:

                    initialPC = VS2_INSTRUCTION_MEMORY_SIZE;
                    
                    if (stopPC == -1)
                        stopPC = std::numeric_limits<int>::max();
                    else
                        stopPC += initialPC;
                    
                    break;
                
                default:
                    initialPC = 0;
            }
        }
        else
        {
            initialPC = 0;
        }

        lastInstr = false;

        PC = initialPC;

        //  Execution loop
        while (!lastInstr && PC < stopPC)
        {
            //  Fetch next instruction.
            instr = shEmu->fetchShaderInstruction(0, PC);

            //  Execute/emulate instruction.
            shEmu->execShaderInstruction(instr);

            //  Check if shader instruction was last.
            lastInstr = instr->getShaderInstruction()->isEnd();

            //  Increment PC.
            if (!lastInstr) PC++;
        }

        //  Allocate space to read registers.
        QuadFloat* programTemps = new QuadFloat[32];
        QuadFloat* programOutputs = new QuadFloat[32];

        //  Read temporary registers back.
        shEmu->readShaderState(0, TEMP, programTemps);

        //  Read output registers back.
        shEmu->readShaderState(0, OUT, programOutputs);

        //  Read per-sample thread kill state.
        for (i = 0; i < MAX_MSAA_SAMPLES; i++)
            killFlag[i] = shEmu->threadKill(0,i);

        //  Print registers.
        cout << "Shader state: PC=" << (PC - initialPC) << " kill vector (per sample): ";
        for (i = 0; i < MAX_MSAA_SAMPLES; i++)
            cout << "(" << i << "," << (killFlag[i]? "k":"a") << ")";
        
        cout << endl;
         
        cout << "Program temps" << endl;
        for (unsigned int i = 0; i < 32; i++)
        {
            cout << "{ " << programTemps[i][0];
            cout << ", " << programTemps[i][1];
            cout << ", " << programTemps[i][2];
            cout << ", " << programTemps[i][3];
            cout << "}" << endl;
        }

        cout << endl << "Program outputs" << endl;
        for (unsigned int i = 0; i < 32; i++)
        {
            cout << "{ " << programOutputs[i][0];
            cout << ", " << programOutputs[i][1];
            cout << ", " << programOutputs[i][2];
            cout << ", " << programOutputs[i][3];
            cout << "}" << endl;
        }

        delete[] programTemps;
        delete[] programOutputs; 
    }
    else
    {
        cout << "Error: no shader program loaded." << endl;
    }
}

void ShaderProgramTestBase::dumpProgram(ostream& out)
{
    if (loadedProgram)
    {
        out << hex;

        for (unsigned int i = 0; i < codeSize; i++)
        {
            if ((i % 16) == 0) out << endl;
            out << "0x" << (unsigned int) compiledCode[i] << ", ";
        }
    }
    else
    {
        cout << "Error: no shader program loaded." << endl;
    }

}

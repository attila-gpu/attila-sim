

#include "GPUTypes.h"
#include "OptimizedDynamicMemory.h"
#include "ShaderInstruction.h"
#include "ShaderOptimization.h"

#include <cstdio>
#include <vector>
#include <fstream>
#include <string>

using namespace gpu3d;

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Usage : \n");
        printf("  optimize [-lda|-soa|-setwaitpoints|-norename|-notrail|-verbose] <input file> <output file>\n");
        return -1;
    }

    bool verbose = false;
    bool useLDA = false;
    bool AOStoSOA = false;
    bool noTrailNOPs = false;
    bool noRename = false;
    bool setWaitPoints = false;

    bool endOptionSection = false;

    u32bit nextParameter = 1;

    for(; !endOptionSection && (nextParameter < argc); nextParameter++)
    {
        string parameter(argv[nextParameter]);

        //  Check if the parameter is an option flag.
        if (parameter[0] == '-')
        {
            if (parameter.compare("-lda") == 0)
                useLDA = true;
            else if (parameter.compare("-soa") == 0)
                AOStoSOA = true;
            else if (parameter.compare("-setwaitpoints") == 0)
                setWaitPoints = true;
            else if (parameter.compare("-norename") == 0)
                noRename = true;
            else if (parameter.compare("-notrail") == 0)
                noTrailNOPs = true;
            else if (parameter.compare("-verbose") == 0)
                verbose = true;
            else
            {
                printf("Usage : \n");
                printf("  optimize [-lda|-soa|-setwaitpoints|-norename|-notrail|-verbose] <input file> <output file>\n");
                return -1;
            }
        }
        else
            endOptionSection = true;
    }

    if (nextParameter == argc)
    {
        printf("Usage : \n");
        printf("  optimize [-lda|-soa|-setwaitpoints|-norename|-notrail|-verbose] <input file> <output file>\n");
        return -1;
    }

    if (endOptionSection)
        nextParameter--;

    string inputFileName = argv[nextParameter];

    ifstream inputFile;

    inputFile.open(inputFileName.c_str(), ios_base::binary);

    if (!inputFile.is_open())
    {
        printf("Error opening input file \"%s\"\n", inputFileName.c_str());
        return -1;
    }

    nextParameter++;

    string outputFileName = argv[nextParameter];

    ofstream outputFile;

    outputFile.open(outputFileName.c_str(), ios_base::binary);

    if (!outputFile.is_open())
    {
        printf("Error opening output file \"%s\"\n", outputFileName.c_str());
        return -1;
    }

    //  Initialize the dynamic memory manager.
    OptimizedDynamicMemory::initialize(512, 4096, 1024, 1024, 4096, 1024);

    u32bit instruction = 1;

    bool endFlaggedInstructionFound = false;

    vector<ShaderInstruction *> inputProgram;

    //  Read until the end of the input file.
    while(!inputFile.eof())
    {
        ShaderInstruction *shInstr;
        u8bit instrCode[16];

        //  Read an instruction from the input file.
        inputFile.read((char *) instrCode, 16);

        //  Check if all the instruction bytes were read.
        if ((inputFile.gcount() == 0) && (inputFile.eof()))
        {
            //  End loop.

        }
        else if (inputFile.gcount() != 16)
        {

            printf("Error reading instruction %d\n", instruction);
            return -1;
        }
        else
        {
            //  Decode the instruction
            shInstr = new ShaderInstruction(instrCode);

            //  Check if the instruction is beyond the last instruction in the program.
            if (endFlaggedInstructionFound && noTrailNOPs)
            {
                //  Ignore the instruction.  Generate a warning if the instruction is not a NOP.
                if (shInstr->getOpcode() != NOP)
                    printf("Warning: Instruction %d after program END is not a NOP\n", instruction);
            }
            else
            {
                //  Add instruction to the program.
                inputProgram.push_back(shInstr);
            }

            //  Check if the instruction is the last instruction in the program.
            endFlaggedInstructionFound = endFlaggedInstructionFound || shInstr->isEnd();

            //  Update the instruction counter.
            instruction++;
        }
    }

    if (verbose)
    {
        printf("Input program : \n");
        printf("----------------------------------------\n");

        ShaderOptimization::printProgram(inputProgram);

        printf("\n");
    }

    vector<ShaderInstruction *> programTemp1;
    vector<ShaderInstruction *> programTemp2;

    vector<ShaderInstruction *> programOptimized;
    vector<ShaderInstruction *> programFinal;

    if (useLDA)
    {
        ShaderOptimization::attribute2lda(inputProgram, programTemp1);

        if (verbose)
        {
            printf("Convert input registers to LDA : \n");
            printf("----------------------------------------\n");

            ShaderOptimization::printProgram(programTemp1);

            printf("\n");
        }
    }
    else
        ShaderOptimization::copyProgram(inputProgram, programTemp1);

    if (AOStoSOA)
    {
        ShaderOptimization::aos2soa(programTemp1, programTemp2);
        ShaderOptimization::deleteProgram(programTemp1);
        ShaderOptimization::copyProgram(programTemp2, programTemp1);
        ShaderOptimization::deleteProgram(programTemp2);

        if (verbose)
        {
            printf("AOS to SOA conversion : \n");
            printf("----------------------------------------\n");

            ShaderOptimization::printProgram(programTemp1);

            printf("\n");
        }
    }

    u32bit maxLiveRegs = 0;

    ShaderOptimization::optimize(programTemp1, programOptimized, maxLiveRegs, noRename, AOStoSOA, verbose);

    ShaderOptimization::deleteProgram(programTemp1);

    if (verbose)
    {
        printf("Optimized program : \n");
        printf("----------------------------------------\n");

        ShaderOptimization::printProgram(programOptimized);

        printf("\n");
    }

    if (setWaitPoints)
    {
        ShaderOptimization::assignWaitPoints(programOptimized, programFinal);

        if (verbose)
        {
            printf("Program with wait points : \n");
            printf("----------------------------------------\n");

            ShaderOptimization::printProgram(programFinal);

            printf("\n");
        }
    }
    else
    {
        ShaderOptimization::copyProgram(programOptimized, programFinal);
    }

    ShaderOptimization::deleteProgram(programOptimized);

    //  Write the final version of the program to the output file.
    for(u32bit instr = 0; instr < programFinal.size(); instr++)
    {
        u8bit instrCode[16];

        //  Get the instruction code.
        programFinal[instr]->getCode(instrCode);

        //  Write the code to the output file.
        outputFile.write((char *) instrCode, 16);
    }


    inputFile.close();
    outputFile.close();
}


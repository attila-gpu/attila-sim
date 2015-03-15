

#include "GPUTypes.h"
#include "OptimizedDynamicMemory.h"
#include "ShaderInstruction.h"

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
        printf("  assemble <input file> <output file>\n");
        return -1;
    }

    string inputFileName = argv[1];
    string outputFileName = argv[2];

    ifstream inputFile;
    ofstream outputFile;

    inputFile.open(inputFileName.c_str());

    if (!inputFile.is_open())
    {
        printf("Error opening input file \"%s\"\n", inputFileName.c_str());
        return -1;
    }

    outputFile.open(outputFileName.c_str());

    if (!outputFile.is_open())
    {
        printf("Error creating output file \"%s\"\n", outputFileName.c_str());
        return -1;
    }

    //  Initialize the dynamic memory manager.
    OptimizedDynamicMemory::initialize(512, 1024, 1024, 1024, 4096, 1024);


    vector<ShaderInstruction*> program;

    program.clear();

    u32bit line = 1;

    bool error = false;

    //  Read until the end of the input file or until there is an error assembling the instruction.
    while(!error && !inputFile.eof())
    {
        ShaderInstruction *shInstr;

        string currentLine;
        string errorString;

        //  Read a line from the input file.
        getline(inputFile, currentLine);

        if ((currentLine.length() == 0) && inputFile.eof())
        {
            //  End loop.
        }
        else
        {
//printf(">> Reading Line %d -> %s\n", line, currentLine.c_str());

            //  Assemble the line.
            shInstr = ShaderInstruction::assemble((char *) currentLine.c_str(), errorString);

            //  Check if the instruction was assembled.
            if (shInstr == NULL)
            {
                printf("Line %d.  ERROR : %s\n", line, errorString.c_str());
                error = true;
            }
            else
            {
                //  Add instruction to the program.
                program.push_back(shInstr);
            }

            //  Update line number.
            line++;
        }
    }

    //  Check if an error occurred.
    if (!error)
    {
        //  Encode the instructions and write to the output file.
        for(u32bit instr = 0; instr < program.size(); instr++)
        {
            u8bit instrCode[16];

            //  Check if this is the last instruction.  If so mark the instruction with the end flag.
            if (instr == (program.size() - 1))
                program[instr]->setEndFlag(true);

            //  Encode instruction.
            program[instr]->getCode(instrCode);

            //  Write the instruction code to the output file.
            outputFile.write((char *) instrCode, 16);
        }
    }

    /*
    string asmLine = "mad t0, t0, t1";

    ShaderInstruction *shInstr;
    string errorString;

    printf("Assembling ...\n");
    shInstr = ShaderInstruction::assemble((char *) asmLine.c_str(), errorString);
    printf("Assembled.\n");

    if (shInstr == NULL)
    {
        printf("ERROR: %s\n", errorString.c_str());
        printf("Instruction could not be assembled.\n");
        return -1;
    }

    char disasm[256];
    u8bit code[64];

    printf("Getting code.\n");
    shInstr->getCode(code);
    printf("Getting disassembled string.\n");
    shInstr->disassemble(disasm);

    printf("Ensambling instruction: \n");
    printf("   %s\n", asmLine.c_str());
    for(u32bit c = 0; c < 16; c++)
        printf(" %02x", code[c]);
    printf("\n");
    printf("Disassembled instruction: \n");
    printf("   %s\n", disasm);
    */

    inputFile.close();
    outputFile.close();
}


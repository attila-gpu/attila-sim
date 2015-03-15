

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
    if (argc < 2)
    {
        printf("Usage : \n");
        printf("  disassemble <input file>\n");
        return -1;
    }

    string inputFileName = argv[1];

    ifstream inputFile;

    inputFile.open(inputFileName.c_str(), ios_base::binary);

    if (!inputFile.is_open())
    {
        printf("Error opening input file \"%s\"\n", inputFileName.c_str());
        return -1;
    }

    //  Initialize the dynamic memory manager.
    OptimizedDynamicMemory::initialize(512, 1024, 1024, 1024, 4096, 1024);

    u32bit instruction = 1;

    bool error = false;

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

            char instrDisasm[256];

            //  Disassemble the instruction.
            shInstr->disassemble(instrDisasm);

            //  Print the instruction.
            printf("%04x :", (instruction - 1) * 16);
            for(u32bit b = 0; b < 16; b++)
                printf(" %02x", instrCode[b]);
            printf("    %s\n", instrDisasm);

            //  Update the instruction counter.
            instruction++;
        }
    }

    inputFile.close();
}


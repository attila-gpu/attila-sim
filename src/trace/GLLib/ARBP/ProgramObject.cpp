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

#include "ProgramObject.h"
#include "ProgramExecutionEnvironment.h"
#include "ProgramTarget.h"
#include <cstring>
#include <cstdio>
#include "glext.h"

using namespace std;
using namespace libgl;


/*  Tables used for writing the instruction.  */

/*  Shader Opcode to string.  */
char  *ProgramObject::shOpcode2Str[] =
{
    "nop", "add", "ara", "arl", "arr", "bra", "cal", "cos",     /*  Opcodes 00h - 07h  */
    "dp3", "dp4", "dph", "dst", "ex2", "exp", "flr", "frc",     /*  Opcodes 08h - 0Fh  */
    "lg2", "lit", "log", "mad", "max", "min", "mov", "mul",     /*  Opcodes 10h - 17h  */
    "rcc", "rcp", "ret", "rsq", "seq", "sfl", "sge", "sgt",     /*  Opcodes 18h - 1Fh  */
    "sin", "sle", "slt", "sne", "ssg", "str", "tex", "txb",     /*  Opcodes 20h - 27h  */
    "txp", "kil", "kls", "zxp", "zxs", "cmp", "cmp_kil", "chs", /*  Opcodes 28h - 2Fh  */
    "lda", "fxmul", "fxmad", "fxmad2", "end", "LAST_OPC"        /*  Opcodes 30h - 3Fh  */    
};

/*  Mask Mode to string.  */
char  *ProgramObject::maskMode2Str[] =
{
    "!INV!", ".w", ".z", ".zw",     /*  Mask Modes 0h - 3h  */
    ".y", ".yw", ".yz", ".yzw",     /*  Mask Modes 4h - 7h  */
    ".x", ".xw", ".xz", ".xzw",     /*  Mask Modes 8h - Bh  */
    ".xy", ".xy", ".xyz", ""        /*  Mask Modes Ch - Fh  */
};

/*  Condition Mode to string.  */

char  *ProgramObject::ccMode2Str[] =
{
    "EQ",     /*  CC Mode 0  */
    "GE",     /*  CC Mode 1  */
    "GT",     /*  CC Mode 2  */
    "LE",     /*  CC Mode 3  */
    "LT",     /*  CC Mode 4  */
    "NE",     /*  CC Mode 5  */
    "TR",     /*  CC Mode 6  */
    "FL"      /*  CC Mode 7  */
};

/*  Swizzle Mask to string.  */
char ProgramObject::swizzleMode2Str[] =
{
    'x',
    'y',
    'z',
    'w'
};

/*  Bank to string.  */
char *ProgramObject::bank2Str[] =
{
   "i",         /*  Bank 0h  */
   "o",         /*  Bank 1h  */
   "c",         /*  Bank 2h  */
   "t",         /*  Bank 3h  */
   "a",         /*  Bank 4h  */
   "CC",        /*  Bank 5h  */
   "texture",   /*  Bank 6h  */
   "INVALID",   /*  Bank 7h  */
};



ProgramObject::ProgramObject(GLenum name, GLenum targetName) : BaseObject(name, targetName),

localParams(96), clusterBank(96), source(0),
binary(0), sourceSize(0), binSize(0), gpuAddr(0), assembler(0), assemblerSz(0),
driverTranslated(false)

{
    // empty :-)
}



void ProgramObject::setSource(const GLvoid* program, GLsizei size_)
{
    // falta código para comprobar si este programa ya tenía un fuente previamente

    if ( source != 0 )
    {
        delete[] source;

        if ( binary != 0 )
        {
            delete[] binary;
        }

        if ( assembler != 0 )
        {
            delete[] assembler;
        }

        if ( gpuAddr != 0 )
        {
            // @note code for releasing from GPU memory is required
        }
    }

    assembler = 0;
    assemblerSz = 0;
    binary = 0;
    binSize = 0;
    sourceSize = size_;
    gpuAddr = 0;
    source = new GLubyte[sourceSize];
    memcpy(source, program, sourceSize);
}

void ProgramObject::setBinary(const GLubyte* bin, GLsizei size)
{
    if ( binary != 0 )
        delete[] binary;    
    
    forceRealloc(); // allocate or reallocate object (contents have changed)

    binSize = size;
    binary  = new GLubyte[size];
    memcpy(binary, bin, binSize);

    _computeASMCode();
}


bool ProgramObject::getSource(GLubyte* program, GLsizei& size) const
{
    if ( source == 0 )
        false;
    
    if ( sourceSize > size )
        return false;

    size = sourceSize;
    memcpy(program,source,size);

    return true;
}

string ProgramObject::getSource() const // new version
{
    if ( source == 0 )
        return string();
    return string((char *)source, sourceSize);
}


bool ProgramObject::compile()
{
    //  Check if the shader program is already compiled.
    if (!isCompiled())
    {
        static ProgramExecutionEnvironment* vpee;
        static ProgramExecutionEnvironment* fpee;
        
        GLenum t = static_cast<ProgramTarget&>(getTarget()).getName();
    
        // Another implementation can be done subclassing one class for each kind of program and redefining compile
        switch ( t )
        {
            case GL_VERTEX_PROGRAM_ARB:

                vpee = new VP1ExecEnvironment;
                vpee->compile(*this);
                break;

            case GL_FRAGMENT_PROGRAM_ARB:
                
                fpee = new FP1ExecEnvironment;
                fpee->compile(*this);
                break;
                
            default:
                {
                    char buffer[256];
                    sprintf(buffer, "Unknown target 0x%X, cannot compile", t);
                    panic("ProgramObject", "compile()", buffer);
                }
        }
        
        return true;
    }
    else
        return false; // No compilation needed.
}


GLuint ProgramObject::getGPUAddr() const
{
    return gpuAddr;
}

RBank<float>& ProgramObject::getLocalParams()
{
    return localParams;
}

RBank<float>& ProgramObject::getClusterBank()
{
    return clusterBank;
}


RBank<float>& ProgramObject::getEnvParams()
{
    return static_cast<ProgramTarget&>(getTarget()).getEnvironmentParameters();
}


void ProgramObject::releaseGPUResources()
{
    // not yet implemented
}

ProgramObject::ProgramResources& ProgramObject::getResources()
{
    return pr;
}

bool ProgramObject::isCompiled()
{
    return ( binary != 0 );
}

void ProgramObject::setAsTranslated()
{
    driverTranslated = true;
}

bool ProgramObject::isTranslated()
{
    return driverTranslated;
}

ProgramObject::~ProgramObject()
{
    if (source) delete[] source;
    if (binary) delete[] binary;
    if (assembler) delete[] assembler;
}


GLuint ProgramObject::getBinarySize() const
{
    return binSize;
}

GLuint ProgramObject::getSourceSize() const
{
    return sourceSize;
}

void ProgramObject::printSource() const
{
    char* txt = new char[sourceSize+1];
    memcpy(txt, source, sourceSize);
    txt[sourceSize] = 0;
    cout << txt << endl;
}


void ProgramObject::printASM() const
{
    GLubyte asmcode[4096*4]; // 16 Kbytes
    GLsizei asmsize = sizeof(asmcode);
    getASMCode(asmcode, asmsize);
    cout << asmcode << endl;
}

void ProgramObject::printBinary() const
{
    for ( int i = 1; i <= binSize; i++ )
    {
        cout << hex << "0x" << int(binary[i-1]) << ",";
        if ( i % 8 == 0 ) cout << endl;
        if ( i % 16 == 0) cout << endl;
    }
}

unsigned int ProgramObject::numOperands(unsigned int Opcode) const
{
    switch(Opcode)
    {
        case 0x00:      return 0;
        case 0x01:      return 2;
        case 0x02:      return 1;
        case 0x03:      return 1;
        case 0x04:      return 1;
        case 0x05:      return 1;
        case 0x06:      return 1;
        case 0x07:      return 1;
        case 0x08:      return 2;
        case 0x09:      return 2;
        case 0x0A:      return 2;
        case 0x0B:      return 2;
        case 0x0C:      return 1;
        case 0x0D:      return 1;
        case 0x0E:      return 1;
        case 0x0F:      return 1;
        case 0x10:      return 1;
        case 0x11:      return 1;
        case 0x12:      return 1;
        case 0x13:      return 3;
        case 0x14:      return 2;
        case 0x15:      return 2;
        case 0x16:      return 1;
        case 0x17:      return 2;
        case 0x18:      return 1;
        case 0x19:      return 1;
        case 0x1A:      return 0;
        case 0x1B:      return 1;
        case 0x1C:      return 2;
        case 0x1D:      return 2;
        case 0x1E:      return 2;
        case 0x1F:      return 2;
        case 0x20:      return 1;
        case 0x21:      return 2;
        case 0x22:      return 2;
        case 0x23:      return 2;
        case 0x24:      return 1;
        case 0x25:      return 2;
        case 0x26:      return 2;
        case 0x27:      return 2;
        case 0x28:      return 2;
        case 0x29:      return 1;
        case 0x2A:      return 2;
        case 0x2B:      return 1;
        case 0x2C:      return 2;
        case 0x2D:      return 3;
        case 0x2E:      return 3;
        case 0x2F:      return 0;
        case 0x30:      return 1;
        case 0x31:      return 2;
        case 0x32:      return 3;
        case 0x33:      return 3;
        case 0x34:      return 0;
    }
    panic("ProgramObject","numOperands()","Unexpected binary Opcode");
    return 0;
}

void ProgramObject::_computeASMCode() const
{
    if (assembler)
        delete[] assembler; // delete previous computed assembler 

    if ( !binary )
        panic("ProgramObject", "_computeASMCode", "Binary code not available");

    assemblerSz = 1024*16*10;
    assembler = new GLubyte[assemblerSz];

    GLsizei binary_pointer = 0;
    GLsizei asmlength = 0;

    char buffer[20];
    char instruction[100];

    assembler[0] = 0;

    while(binary_pointer < binSize)
    {
        /*  Write if the last instruction */

        if ((binary[binary_pointer+8] & 0x80))
            sprintf(instruction,"*");
        else
            sprintf(instruction,"");

        /*  Write the instruction name.  */
        sprintf(instruction, "%s%s", instruction, shOpcode2Str[binary[binary_pointer]]);

        /*  Mark if it sets the CC register.  */
        if (((binary[binary_pointer+1] & 0x02) == 0x02) && (binary[binary_pointer] != 0x2B) && (binary[binary_pointer] != 0x00))
            sprintf(instruction, "%sC", instruction);

        /*  Write saturated extension if saturated result is active.  */
        if ((binary[binary_pointer] != 0x2B) && (binary[binary_pointer] != 0x00) && (binary[binary_pointer+8] & 0x40))
            sprintf(instruction, "%s_SAT", instruction);

        /*  Branches, Call, Ret, END, nop are different!!!  */

        if ((binary[binary_pointer] == 0x2B) || (binary[binary_pointer] == 0x00))
        {
            /*  Do nothing.  These instructions don't have parameters.  */
        }
        else if ((binary[binary_pointer] == 0x05) || (binary[binary_pointer] == 0x2B))
        {
            /*  Branch and Call instructions.  */
                /*  Write CC swizzle mask.  */

            if ((binary[binary_pointer+13] == 0x00) || (binary[binary_pointer+13] == 0x55) ||
                (binary[binary_pointer+13] == 0xaa) || (binary[binary_pointer+13] == 0xff))
            {
                /*  Broadcast a single component to all other components. */
                sprintf(buffer, ".%c", swizzleMode2Str[binary[binary_pointer+13] & 0x03]);
            }
            else if (binary[binary_pointer+13] == 0x1b)
            {
                /*  The default swizzle mask mode isn't printed.  */
                sprintf(buffer, "");
            }
            else
            {
                /*  Build swizzle mask for each component.  */
                sprintf(buffer, ".%c%c%c%c",
                    swizzleMode2Str[(binary[binary_pointer+13] & 0xc0) >> 6],
                    swizzleMode2Str[(binary[binary_pointer+13] & 0x30) >> 4],
                    swizzleMode2Str[(binary[binary_pointer+13] & 0x0c) >> 2],
                    swizzleMode2Str[binary[binary_pointer+13] & 0x03]);
            }

            /*  If there is a conditional write, write it.  */
            if (((binary[binary_pointer+12] & 0x70) >> 4) != 0x06)
            {
                if (((binary[binary_pointer+1] & 0x01) == 0x01))
                    sprintf(instruction, "%s [a%d.%c + (%d)] (%s%s)", instruction, (binary[binary_pointer+14] & 0x03),
                        swizzleMode2Str[((binary[binary_pointer+14] & 0x0C) >> 2)], binary[binary_pointer+15],
                        ccMode2Str[((binary[binary_pointer+12] & 0x70) >> 4)], buffer);
                else
                    sprintf(instruction, "%s %d (%s%s)", instruction, binary[binary_pointer + 15],ccMode2Str[((binary[binary_pointer+12] & 0x70) >> 4)], buffer);
            }
            else
            {
                if (((binary[binary_pointer+1] & 0x01) == 0x01))
                    sprintf(instruction, "%s [a%d.%c + (%d)]", instruction, (binary[binary_pointer+14] & 0x03),
                        swizzleMode2Str[((binary[binary_pointer+14] & 0x0C) >> 2)], binary[binary_pointer+15]);
                else
                    sprintf(instruction, "%s %d", instruction, binary[binary_pointer + 15]);
            }


        }
        else if (binary[binary_pointer] == 0x1A)
        {
            /*  Ret instruction.  Doesn't have an address parameter.  */

            /*  Write CC swizzle mask.  */
            if ((binary[binary_pointer+13] == 0x00) || (binary[binary_pointer+13] == 0x55) ||
                (binary[binary_pointer+13] == 0xaa) || (binary[binary_pointer+13] == 0xff))
            {
                /*  Broadcast a single component to all other components. */
                sprintf(buffer, ".%c", swizzleMode2Str[binary[binary_pointer+13] & 0x03]);
            }
            else if (binary[binary_pointer+13] == 0x1b)
            {
                /*  The default swizzle mask mode isn't printed.  */
                sprintf(buffer, "");
            }
            else
            {
                /*  Build swizzle mask for each component.  */
                sprintf(buffer, ".%c%c%c%c",
                    swizzleMode2Str[(binary[binary_pointer+13] & 0xc0) >> 6],
                    swizzleMode2Str[(binary[binary_pointer+13] & 0x30) >> 4],
                    swizzleMode2Str[(binary[binary_pointer+13] & 0x0c) >> 2],
                    swizzleMode2Str[binary[binary_pointer+13] & 0x03]);
            }

            /*  If there is a conditional write, write it.  */
            if (((binary[binary_pointer+12] & 0x70) >> 4) != 0x06)
            {
                sprintf(instruction, "%s (%s%s)", instruction, ccMode2Str[((binary[binary_pointer+12] & 0x70) >> 4)], buffer);
            }

        }
        else
        {
            /*  All the other instructions follow the pattern:

                    op res, op1, op2, op3
            */

            /*  Write the result register, mask and condition.  */
            sprintf(instruction, "%s %s%d%s", instruction, bank2Str[((binary[binary_pointer+7] & 0x38) >> 3)], binary[binary_pointer+5], maskMode2Str[(binary[binary_pointer+12] & 0x0F)]);

            /*  Write CC swizzle mask.  */
            if ((binary[binary_pointer+13] == 0x00) || (binary[binary_pointer+13] == 0x55) ||
                (binary[binary_pointer+13] == 0xaa) || (binary[binary_pointer+13] == 0xff))
            {
                /*  Broadcast a single component to all other components. */
                sprintf(buffer, ".%c", swizzleMode2Str[binary[binary_pointer+13] & 0x03]);
            }
            else if (binary[binary_pointer+13] == 0x1b)
            {
                /*  The default swizzle mask mode isn't printed.  */
                sprintf(buffer, "");
            }
            else
            {
                /*  Build swizzle mask for each component.  */
                sprintf(buffer, ".%c%c%c%c",
                    swizzleMode2Str[(binary[binary_pointer+13] & 0xc0) >> 6],
                    swizzleMode2Str[(binary[binary_pointer+13] & 0x30) >> 4],
                    swizzleMode2Str[(binary[binary_pointer+13] & 0x0c) >> 2],
                    swizzleMode2Str[binary[binary_pointer+13] & 0x03]);
            }

            /*  If there is a conditional write, write it.  */
            if (((binary[binary_pointer+12] & 0x70) >> 4) != 0x06)
            {
                sprintf(instruction, "%s (%s%s)", instruction, ccMode2Str[((binary[binary_pointer+12] & 0x70) >> 4)], buffer);
            }

            /*  Write the operands.  */

            /*  First operand.  */
            if (numOperands(binary[binary_pointer]) > 0)
            {
                /*  Write wizzle mask to buffer.  */
                if ((binary[binary_pointer+9] == 0x00) || (binary[binary_pointer+9] == 0x55) ||
                    (binary[binary_pointer+9] == 0xaa) || (binary[binary_pointer+9] == 0xff))
                {
                    /*  Broadcast a single component to all other components. */
                    sprintf(buffer, ".%c", swizzleMode2Str[binary[binary_pointer+9] & 0x03]);
                }
                else if (binary[binary_pointer+9] == 0x1b)
                {
                    /*  The default swizzle mask mode isn't printed.  */
                    sprintf(buffer, "");
                }
                else
                {
                    /*  Build swizzle mask for each component.  */
                    sprintf(buffer, ".%c%c%c%c",
                    swizzleMode2Str[(binary[binary_pointer+9] & 0xc0) >> 6],
                    swizzleMode2Str[(binary[binary_pointer+9] & 0x30) >> 4],
                    swizzleMode2Str[(binary[binary_pointer+9] & 0x0c) >> 2],
                    swizzleMode2Str[binary[binary_pointer+9] & 0x03]);
                }

                if (((binary[binary_pointer+8] & 0x01) == 0x01))
                {
                    sprintf(instruction,"%s, -", instruction);
                }
                else
                    sprintf(instruction, "%s, ", instruction);

                if (((binary[binary_pointer+8] & 0x08) == 0x08))
                {
                    if (((binary[binary_pointer+1] & 0x01) == 0x01) && ((binary[binary_pointer + 6] & 0x07) == 0x02))
                        sprintf(instruction, "%s|c[a%d.%c + (%d)]%s|", instruction, (binary[binary_pointer+14] & 0x03),
                            swizzleMode2Str[((binary[binary_pointer+14] & 0x0C) >> 2)], binary[binary_pointer+15], buffer);
                    else
                        sprintf(instruction, "%s|%s%d%s|", instruction, bank2Str[(binary[binary_pointer + 6] & 0x07)], binary[binary_pointer+2], buffer);
                }
                else
                {
                    if (((binary[binary_pointer+1] & 0x01) == 0x01) && ((binary[binary_pointer + 6] & 0x07) == 0x02))
                        sprintf(instruction, "%sc[a%d.%c + (%d)]%s", instruction, (binary[binary_pointer+14] & 0x03),
                            swizzleMode2Str[((binary[binary_pointer+14] & 0x0C) >> 2)], binary[binary_pointer+15], buffer);
                    else
                        sprintf(instruction, "%s%s%d%s", instruction, bank2Str[(binary[binary_pointer + 6] & 0x07)], binary[binary_pointer+2], buffer);
                }
            }

            /*  Second Operand.  */
            if (numOperands(binary[binary_pointer]) > 1)
            {
                /*  Write wizzle mask to buffer.  */
                if ((binary[binary_pointer+10] == 0x00) || (binary[binary_pointer+10] == 0x55) ||
                    (binary[binary_pointer+10] == 0xaa) || (binary[binary_pointer+10] == 0xff))
                {
                    /*  Broadcast a single component to all other components. */
                    sprintf(buffer, ".%c", swizzleMode2Str[binary[binary_pointer+10] & 0x03]);
                }
                else if (binary[binary_pointer+10] == 0x1b)
                {
                    /*  The default swizzle mask mode isn't printed.  */
                    sprintf(buffer, "");
                }
                else
                {
                    /*  Build swizzle mask for each component.  */
                    sprintf(buffer, ".%c%c%c%c",
                    swizzleMode2Str[(binary[binary_pointer+10] & 0xc0) >> 6],
                    swizzleMode2Str[(binary[binary_pointer+10] & 0x30) >> 4],
                    swizzleMode2Str[(binary[binary_pointer+10] & 0x0c) >> 2],
                    swizzleMode2Str[binary[binary_pointer+10] & 0x03]);
                }

                if (((binary[binary_pointer+8] & 0x02) == 0x02))
                {
                    sprintf(instruction,"%s, -", instruction);
                }
                else
                    sprintf(instruction, "%s, ", instruction);

                if (((binary[binary_pointer+8] & 0x10) == 0x10))
                {
                    if (((binary[binary_pointer+1] & 0x01) == 0x01) && ((binary[binary_pointer + 6] & 0x038) == 0x02))
                        sprintf(instruction, "%s|c[a%d.%c + (%d)]%s|", instruction, (binary[binary_pointer+14] & 0x03),
                            swizzleMode2Str[((binary[binary_pointer+14] & 0x0C) >> 2)], binary[binary_pointer+15], buffer);
                    else
                        sprintf(instruction, "%s|%s%d%s|", instruction, bank2Str[((binary[binary_pointer + 6] & 0x038) >> 3)], binary[binary_pointer+3], buffer);
                }
                else
                {
                    if (((binary[binary_pointer+1] & 0x01) == 0x01) && ((binary[binary_pointer + 6] & 0x038) == 0x02))
                        sprintf(instruction, "%sc[a%d.%c + (%d)]%s", instruction, (binary[binary_pointer+14] & 0x03),
                            swizzleMode2Str[((binary[binary_pointer+14] & 0x0C) >> 2)], binary[binary_pointer+15], buffer);
                    else
                        sprintf(instruction, "%s%s%d%s", instruction, bank2Str[((binary[binary_pointer + 6] & 0x038) >> 3)], binary[binary_pointer+3], buffer);
                }
            }

            /*  Third operand.  */
            if (numOperands(binary[binary_pointer]) > 2)
            {
                /*  Write wizzle mask to buffer.  */
                if ((binary[binary_pointer+11] == 0x00) || (binary[binary_pointer+11] == 0x55) ||
                    (binary[binary_pointer+11] == 0xaa) || (binary[binary_pointer+11] == 0xff))
                {
                    /*  Broadcast a single component to all other components. */
                    sprintf(buffer, ".%c", swizzleMode2Str[binary[binary_pointer+11] & 0x03]);
                }
                else if (binary[binary_pointer+11] == 0x1b)
                {
                    /*  The default swizzle mask mode isn't printed.  */
                    sprintf(buffer, "");
                }
                else
                {
                    /*  Build swizzle mask for each component.  */
                    sprintf(buffer, ".%c%c%c%c",
                    swizzleMode2Str[(binary[binary_pointer+11] & 0xc0) >> 6],
                    swizzleMode2Str[(binary[binary_pointer+11] & 0x30) >> 4],
                    swizzleMode2Str[(binary[binary_pointer+11] & 0x0c) >> 2],
                    swizzleMode2Str[binary[binary_pointer+11] & 0x03]);
                }

                if (((binary[binary_pointer+8] & 0x04) == 0x04))
                {
                    sprintf(instruction,"%s, -", instruction);
                }
                else
                    sprintf(instruction, "%s, ", instruction);

                if (((binary[binary_pointer+8] & 0x20) == 0x20))
                {
                    if (((binary[binary_pointer+1] & 0x01) == 0x01) && ((binary[binary_pointer + 7] & 0x07) == 0x02))
                        sprintf(instruction, "%s|c[a%d.%c + (%d)]%s|", instruction, (binary[binary_pointer+14] & 0x03),
                            swizzleMode2Str[((binary[binary_pointer+14] & 0x0C) >> 2)], binary[binary_pointer+15], buffer);
                    else
                        sprintf(instruction, "%s|%s%d%s|", instruction, bank2Str[(binary[binary_pointer + 7] & 0x07)], binary[binary_pointer+4], buffer);
                }
                else
                {
                    if (((binary[binary_pointer+1] & 0x01) == 0x01) && ((binary[binary_pointer + 7] & 0x07) == 0x02))
                        sprintf(instruction, "%sc[a%d.%c + (%d)]%s", instruction, (binary[binary_pointer+14] & 0x03),
                            swizzleMode2Str[((binary[binary_pointer+14] & 0x0C) >> 2)], binary[binary_pointer+15], buffer);
                    else
                        sprintf(instruction, "%s%s%d%s", instruction, bank2Str[(binary[binary_pointer + 7] & 0x07)], binary[binary_pointer+4], buffer);
                }
            }
        }
        binary_pointer += 16;
        asmlength += strlen(instruction);
        if (asmlength <= assemblerSz)
            sprintf((char *)assembler,"%s%s\n",(char *)assembler,instruction);
        else
            panic("ProgramObject","_computeASMCode()","Not enough buffer length");
    }
    assemblerSz = asmlength;
}

void ProgramObject::getASMCode(GLubyte *assembler, GLsizei& size) const
{
    if ( !(this->assembler) )
        _computeASMCode();

    if ( size < assemblerSz )
        panic("ProgramObject","getASMCode()","Not enough buffer length");

    memcpy(assembler, this->assembler, assemblerSz);
    size = assemblerSz;
}


void ProgramObject::setFormat(GLenum format)
{
    if ( format != GL_PROGRAM_FORMAT_ASCII_ARB )
        panic("ProgramObject","setFormat","Format unsupported");
    this->format = format;
}


bool ProgramObject::getBinary(GLubyte* program, GLsizei& size) const
{
    if ( binSize > size )
    {
        panic("ProgramObject", "getBinary", "buffer size is not enough");
        return false;
    }

    size = binSize;
    memcpy(program, binary, binSize);

    return true;
}


GLuint ProgramObject::binarySize(GLuint portion)
{
    if ( portion != 0 )
        panic("ProgramObject", "binarySize", "A program object only contains 1 portion");

    //if ( binary == 0 )
    //    panic("ProgramObject", "binarySize", "Binary code not yet available");

    return binSize;
}

const GLubyte* ProgramObject::binaryData(GLuint portion)
{
    if ( portion != 0 )
        panic("ProgramObject", "binaryData", "A program object only contains 1 portion");

    if ( binary == 0 )
        panic("ProgramObject", "binaryData", "Binary code not yet available");
    return binary;

}

const char* ProgramObject::getStringID() const
{
    static const char* vsh = "VERTEX_PROGRAM";
    static const char* fsh = "FRAGMEN_PROGRAM";
    static const char* unknown = "UNKNOWN_PROGRAM";
    GLenum tname = getTargetName();
    if ( tname == GL_VERTEX_PROGRAM_ARB )
        return vsh;
    else if ( tname == GL_FRAGMENT_PROGRAM_ARB )
        return fsh;
    else
        return unknown;
}

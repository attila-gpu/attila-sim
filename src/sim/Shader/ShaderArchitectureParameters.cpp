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
 * Shader Architecture Parameters.
 *
 */

/**
 *  @file ShaderArchitectureParameters.cpp
 *
 *  This file implements the ShaderArchitectureParameters class.
 *
 *  The ShaderArchitectureParameters class defines a container for architecture parameters (instruction
 *  execution latencies, repeation rates, etc) for a simulated shader architecture.
 *
 */

#include "ShaderArchitectureParameters.h"

using namespace gpu3d;

//  Pointer to the single instance of the class.
ShaderArchitectureParameters *ShaderArchitectureParameters::shArchParams = NULL;

//
//  Latency execution and and repeat rate tables for the different architecture
//  configurations.
//

//  Execution latency table for Variable Execution Latency AOS Architecture
u32bit ShaderArchitectureParameters::varLatAOSExecLatencyTable[LASTOPC] =
{
    //  NOP     ADD     ADDI    ARL     ANDP    INV     INV     COS
    //  00h     01h     02h     03h     04h     05h     06h     07h
        1,      3,      2,      3,      1,      0,      0,      12,

    //  DP3     DP4     DPH     DST     EX2     EXP     FLR     FRC
    //  08h     09h     0Ah     0Bh     0Ch     0Dh     0Eh     0Fh
        3,      3,      3,      4,      5,      9,      0,      3,

    //  LG2     LIT     LOG     MAD     MAX     MIN     MOV     MUL
    //  10h     11h     12h     13h     14h     15h     16h     17h
        5,      4,      9,      3,      3,      3,      3,      3,

    //  MULI    RCP     INV     RSQ     SETPEQ  SETPGT  SGE     SETPLT
    //  18h     19h     1Ah     1Bh     1Ch     1Dh     1Eh     1Fh
        2,      5,      0,      5,      3,      3,      3,      3,

    //  SIN     STPEQI  SLT     STPGTI  STPLTI  TXL     TEX     TXB
    //  20h     21h     22h     23h     24h     25h     26h     27h
        12,     2,      3,      2,      2,      1,      1,      1,

    //  TXP     KIL     KLS     ZXP     ZXS     CMP     CMPKIL  CHS
    //  28h     29h     2Ah     2Bh     2Ch     2Dh     2Eh     2Fh
        5,      3,      3,      3,      3,      3,      3,      3,

    //  LDA     FXMUL   FXMAD   FXMAD2  DDX     DDY     JMP     END
    //  30h     31h     32h     33h     34h     35h     36h     37h
        1,      3,      3,      3,      16,     16,     1,      1
};

//  Repeat Rate table for Variable Execution Latency AOS Architecture
u32bit ShaderArchitectureParameters::varLatAOSRepeatRateTable[LASTOPC] =
{
    //  NOP     ADD     ADDI    ARL     ANDP    INV     INV     COS
    //  00h     01h     02h     03h     04h     05h     06h     07h
        1,      1,      1,      1,      1,      0,      0,      6,

    //  DP3     DP4     DPH     DST     EX2     EXP     FLR     FRC
    //  08h     09h     0Ah     0Bh     0Ch     0Dh     0Eh     0Fh
        1,      1,      1,      1,      2,      4,      0,      1,

    //  LG2     LIT     LOG     MAD     MAX     MIN     MOV     MUL
    //  10h     11h     12h     13h     14h     15h     16h     17h
        2,      1,      4,      1,      1,      1,      1,      1,

    //  MULI    RCP     INV,    RSQ     SETPEQ  SETPGT  SGE     SETPGT
    //  18h     19h     1Ah     1Bh     1Ch     1Dh     1Eh     1Fh
        1,      4,      0,      4,      1,      1,      1,      1,

    //  SIN     STPEQI  SLT     STPGTI  STPLTI  TXL     TEX     TXB
    //  20h     21h     22h     23h     24h     25h     26h     27h
        6,      1,      1,      1,      1,      1,      1,      1,

    //  TXP     KIL     KLS     ZXP     ZXS     CMP     CMPKIL  CHS
    //  28h     29h     2Ah     2Bh     2Ch     2Dh     2Eh     2Fh
        1,      1,      1,      1,      1,      1,      1,      1,

    //  LDA     FXMUL   FXMAD   FXMAD2  DDX     DDY     JMP     END
    //  30h     31h     32h     33h     34h     35h     36h     37h
        1,      1,      1,      1,      8,      8,      1,      1

};

//  Execution latency table for Fixed Execution Latency AOS Architecture
u32bit ShaderArchitectureParameters::fixedLatAOSExecLatencyTable[LASTOPC] =
{
    //  NOP     ADD     ADDI    ARL     ANDP    INV     INV     COS
    //  00h     01h     02h     03h     04h     05h     06h     07h
        3,      3,      3,      3,      3,      0,      0,      12,

    //  DP3     DP4     DPH     DST     EX2     EXP     FLR     FRC
    //  08h     09h     0Ah     0Bh     0Ch     0Dh     0Eh     0Fh
        3,      3,      3,      3,      3,      3,      0,      3,

    //  LG2     LIT     LOG     MAD     MAX     MIN     MOV     MUL
    //  10h     11h     12h     13h     14h     15h     16h     17h
        3,      3,      3,      3,      3,      3,      3,      3,

    //  MULI    RCP     INV     RSQ     SETPEQ  SETPGT  SGE     SETPLT
    //  18h     19h     1Ah     1Bh     1Ch     1Dh     1Eh     1Fh
        3,      3,      0,      3,      3,      3,      3,      3,

    //  SIN     STPEQI  SLT     STPGTI  STPLTI  TXL     TEX     TXB
    //  20h     21h     22h     23h     24h     25h     26h     27h
        12,     3,      3,      3,      3,      3,      3,      3,

    //  TXP     KIL     KLS     ZXP     ZXS     CMP     CMPKIL  CHS
    //  28h     29h     2Ah     2Bh     2Ch     2Dh     2Eh     2Fh
        3,      3,      3,      3,      3,      3,      3,      3,

    //  LDA     FXMUL   FXMAD   FXMAD2  DDX     DDY     JMP     END
    //  30h     31h     32h     33h     34h     35h     36h     37h
        3,      3,      3,      3,      16,     16,     3,      3
};

//  Repeat Rate table for Fixed Execution Latency AOS Architecture
u32bit ShaderArchitectureParameters::fixedLatAOSRepeatRateTable[LASTOPC] =
{
    //  NOP     ADD     ADDI    ARL     ANDP    INV     INV     COS
    //  00h     01h     02h     03h     04h     05h     06h     07h
        1,      1,      1,      1,      1,      0,      0,      6,

    //  DP3     DP4     DPH     DST     EX2     EXP     FLR     FRC
    //  08h     09h     0Ah     0Bh     0Ch     0Dh     0Eh     0Fh
        1,      1,      1,      1,      2,      4,      0,      1,

    //  LG2     LIT     LOG     MAD     MAX     MIN     MOV     MUL
    //  10h     11h     12h     13h     14h     15h     16h     17h
        2,      1,      4,      1,      1,      1,      1,      1,

    //  MULI    RCP     INV     RSQ     SETPEQ  SETPGT  SGE     SETPLT
    //  18h     19h     1Ah     1Bh     1Ch     1Dh     1Eh     1Fh
        1,      4,      0,      4,      1,      1,      1,      1,

    //  SIN     STPEQI  SLT     STPGTI  STPLTI  TXL     TEX     TXB
    //  20h     21h     22h     23h     24h     25h     26h     27h
        6,      1,      1,      1,      1,      1,      1,      1,

    //  TXP     KIL     KLS     ZXP     ZXS     CMP     CMPKIL  CHS
    //  28h     29h     2Ah     2Bh     2Ch     2Dh     2Eh     2Fh
        1,      1,      1,      1,      1,      1,      1,      1,

    //  LDA     FXMUL   FXMAD   FXMAD2  DDX     DDY     JMP     END
    //  30h     31h     32h     33h     34h     35h     36h     37h
        1,      1,      1,      1,      8,      8,      1,      1
};

//  Execution latency table for Variable Execution Latency SOA Architecture
u32bit ShaderArchitectureParameters::varLatSOAExecLatencyTable[LASTOPC] =
{
    //  NOP     ADD     ADDI    ARL     ANDP    INV     INV     COS
    //  00h     01h     02h     03h     04h     05h     06h     07h
        1,      3,      2,      3,      1,      0,      0,      12,

    //  DP3     DP4     DPH     DST     EX2     EXP     FLR     FRC
    //  08h     09h     0Ah     0Bh     0Ch     0Dh     0Eh     0Fh
        0,      0,      0,      4,      5,      9,      0,      3,

    //  LG2     LIT     LOG     MAD     MAX     MIN     MOV     MUL
    //  10h     11h     12h     13h     14h     15h     16h     17h
        5,      4,      9,      3,      3,      3,      3,      3,

    //  MULI    RCP     INV,    RSQ     SETPEQ  SETPGT  SGE     SETPLT
    //  18h     19h     1Ah     1Bh     1Ch     1Dh     1Eh     1Fh
        2,      5,      0,      5,      3,      3,      3,      3,

    //  SIN     STPEQI  SLT     STPGTI  STPLTI  TXL     TEX     TXB
    //  20h     21h     22h     23h     24h     25h     26h     27h
        12,     2,      3,      2,      2,      4,      3,      4,

    //  TXP     KIL     KLS     ZXP     ZXS     CMP     CMPKIL  CHS
    //  28h     29h     2Ah     2Bh     2Ch     2Dh     2Eh     2Fh
        7,      3,      3,      3,      3,      3,      3,      3,

    //  LDA     FXMUL   FXMAD   FXMAD2  DDX     DDY     JMP     END
    //  30h     31h     32h     33h     34h     35h     36h     37h
        1,      3,      3,      3,      16,     16,     1,      1
};

//  Repeat Rate table for Variable Execution Latency AOS Architecture
u32bit ShaderArchitectureParameters::varLatSOARepeatRateTable[LASTOPC] =
{
    //  NOP     ADD     ADDI    ARL     ANDP    INV     INV     COS
    //  00h     01h     02h     03h     04h     05h     06h     07h
        1,      1,      1,      1,      1,      0,      0,      6,

    //  DP3     DP4     DPH     DST     EX2     EXP     FLR     FRC
    //  08h     09h     0Ah     0Bh     0Ch     0Dh     0Eh     0Fh
        0,      0,      0,      8,      2,      4,      0,      1,

    //  LG2     LIT     LOG     MAD     MAX     MIN     MOV     MUL
    //  10h     11h     12h     13h     14h     15h     16h     17h
        2,      7,      4,      1,      1,      1,      1,      1,

    //  MULI    RCP     INV     RSQ     SETPEQ  SETPGT  SGE     SETPLT
    //  18h     19h     1Ah     1Bh     1Ch     1Dh     1Eh     1Fh
        1,      4,      0,      4,      1,      1,      1,      1,

    //  SIN     STPEQI  SLT     STGTI   STPLTI  TXL     TEX     TXB
    //  20h     21h     22h     23h     24h     25h     26h     27h
        6,      1,      1,      1,      1,      4,      3,      4,

    //  TXP     KIL     KLS     ZXP     ZXS     CMP     CMPKIL  CHS
    //  28h     29h     2Ah     2Bh     2Ch     2Dh     2Eh     2Fh
        4,      1,      1,      1,      1,      1,      1,      1,

    //  LDA     FXMUL   FXMAD   FXMAD2  DDX     DDY     JMP     END
    //  30h     31h     32h     33h     34h     35h     36h     37h
        1,      1,      1,      1,      8,      8,      1,      1
};

//  Execution latency table for Fixed Execution Latency SOA Architecture
u32bit ShaderArchitectureParameters::fixedLatSOAExecLatencyTable[LASTOPC] =
{
    //  NOP     ADD     ADDI    ARL     ANDP    INV     INV     COS
    //  00h     01h     02h     03h     04h     05h     06h     07h
        3,      3,      3,      3,      3,      0,      0,      12,

    //  DP3     DP4     DPH     DST     EX2     EXP     FLR     FRC
    //  08h     09h     0Ah     0Bh     0Ch     0Dh     0Eh     0Fh
        3,      3,      3,      3,      3,      3,      0,      3,

    //  LG2     LIT     LOG     MAD     MAX     MIN     MOV     MUL
    //  10h     11h     12h     13h     14h     15h     16h     17h
        3,      3,      3,      3,      3,      3,      3,      3,

    //  MULI    RCP     INV     RSQ     SETPEQ  SETPGT  SGE     SETPLT
    //  18h     19h     1Ah     1Bh     1Ch     1Dh     1Eh     1Fh
        3,      3,      0,      3,      3,      3,      3,      3,

    //  SIN     STPEQI  SLT     STPGTI  STPLTI  TXL     TEX     TXB
    //  20h     21h     22h     23h     24h     25h     26h     27h
        12,     3,      3,      3,      3,      3,      3,      3,

    //  TXP     KIL     KLS     ZXP     ZXS     CMP     CMPKIL  CHS
    //  28h     29h     2Ah     2Bh     2Ch     2Dh     2Eh     2Fh
        3,      3,      3,      3,      3,      3,      3,      3,

    //  LDA     FXMUL   FXMAD   FXMAD2  DDX     DDY     JMP     END
    //  30h     31h     32h     33h     34h     35h     36h     37h
        3,      3,      3,      3,      16,     16,     3,      3
};

//  Repeat Rate table for Fixed Execution Latency AOS Architecture
u32bit ShaderArchitectureParameters::fixedLatSOARepeatRateTable[LASTOPC] =
{
    //  NOP     ADD     ADDI    ARL     ANDP    INV     INV     COS
    //  00h     01h     02h     03h     04h     05h     06h     07h
        1,      1,      1,      1,      1,      0,      0,      6,

    //  DP3     DP4     DPH     DST     EX2     EXP     FLR     FRC
    //  08h     09h     0Ah     0Bh     0Ch     0Dh     0Eh     0Fh
        0,      0,      0,      8,      2,      4,      0,      1,

    //  LG2     LIT     LOG     MAD     MAX     MIN     MOV     MUL
    //  10h     11h     12h     13h     14h     15h     16h     17h
        2,      7,      4,      1,      1,      1,      1,      1,

    //  MULI    RCP     INV     RSQ     SETPEQ  SETPGT  SGE     SETPLT
    //  18h     19h     1Ah     1Bh     1Ch     1Dh     1Eh     1Fh
        1,      4,      0,      4,      1,      1,      1,      1,

    //  SIN     STPEQI  SLT     STPGTI  STPLTI  TXL     TEX     TXB
    //  20h     21h     22h     23h     24h     25h     26h     27h
        6,      1,      1,      1,      1,      4,      3,      4,

    //  TXP     KIL     KLS     ZXP     ZXS     CMP     CMPKIL  CHS
    //  28h     29h     2Ah     2Bh     2Ch     2Dh     2Eh     2Fh
        4,      1,      1,      1,      1,      1,      1,      1,

    //  LDA     FXMUL   FXMAD   FXMAD2  DDX     DDY     JMP     END
    //  30h     31h     32h     33h     34h     35h     36h     37h
        1,      1,      1,      1,      8,      8,      1,      1
};

//  Private constructor.
ShaderArchitectureParameters::ShaderArchitectureParameters()
{
    execLatencyTable = NULL;
    repeatRateTable = NULL;
}

//  Obtain pointer to singleton.  Create if required.
ShaderArchitectureParameters *ShaderArchitectureParameters::getShaderArchitectureParameters()
{
    //  Check if singleton has been created.
    if (shArchParams == NULL)
    {
        shArchParams = new ShaderArchitectureParameters;
    }

    return shArchParams;
}

//  Returns a list of shader architecture names.
void ShaderArchitectureParameters::architectureList(string &archList)
{
    archList = "VarLatAOS VarLatSOA FixedLatAOS FixedLatSOA";
}

//  Selects the shader architecture to query.
void ShaderArchitectureParameters::selectArchitecture(string archName)
{
    if (archName.compare("VarLatAOS") == 0)
    {
        execLatencyTable = varLatAOSExecLatencyTable;
        repeatRateTable = varLatAOSRepeatRateTable;
    }
    else if (archName.compare("VarLatSOA") == 0)
    {
        execLatencyTable = varLatSOAExecLatencyTable;
        repeatRateTable = varLatSOARepeatRateTable;
    }
    else if (archName.compare("FixedLatAOS") == 0)
    {
        execLatencyTable = fixedLatAOSExecLatencyTable;
        repeatRateTable = fixedLatAOSRepeatRateTable;
    }
    else if (archName.compare("FixedLatSOA") == 0)
    {
        execLatencyTable = fixedLatSOAExecLatencyTable;
        repeatRateTable = fixedLatSOARepeatRateTable;
    }
    else
    {
        execLatencyTable = NULL;
        repeatRateTable = NULL;
    }
}

//  Query the execution latency for the shader instruction opcode in the current architecture.
u32bit ShaderArchitectureParameters::getExecutionLatency(ShOpcode opc)
{

    GPU_ASSERT(
        if (opc == INVOPC)
            panic("ShaderArchitectureParameters", "getExecutionLatency", "Invalid opcode");

        if (opc >= LASTOPC)
            panic("ShaderArchitectureParameters", "getExecutionLatency", "Opcode out of valid range");

        if (execLatencyTable == NULL)
            panic("ShaderArchitectureParameters", "getExecutionLatency", "Shader architecture not defined");

        /*
            NOTE: CHECK MOVED. This check is now performed in shader decode execute.  The reason is
            that some instruction may be allowed in intermediate steps in the library but are not
            allowed in the shader (DPx in SOA mode.

        if (execLatencyTable[opc] == 0)
        {
            panic("ShaderArchitectureParameters", "getExecutionLatency", "Instruction not implemented");
        }*/
    )

    return execLatencyTable[opc];
}

//  Query the repeat rate for the shader instruction opcode in the current architecture.
u32bit ShaderArchitectureParameters::getRepeatRate(ShOpcode opc)
{
    GPU_ASSERT(
        if (opc == INVOPC)
            panic("ShaderArchitectureParameters", "getRepeatRate", "Invalid opcode");

        if (opc >= LASTOPC)
            panic("ShaderArchitectureParameters", "getRepeatRate", "Opcode out of valid range");

        if (repeatRateTable == NULL)
            panic("ShaderArchitectureParameters", "getRepeatRate", "Shader architecture not defined");

        /*
            NOTE: CHECK MOVED. This check is now performed in shader decode execute.  The reason is
            that some instruction may be allowed in intermediate steps in the library but are not
            allowed in the shader (DPx in SOA mode.

        if (repeatRateTable[opc] == 0)
        {
            panic("ShaderArchitectureParameters", "getRepeatRate", "Instruction not implemented");
        } */
    )

    return repeatRateTable[opc];
}

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
 * $RCSfile:$
 * $Revision:$
 * $Author:$
 * $Date:$
 *
 * Shader Optimization
 *
 */



/**
 *
 *  @file ShaderOptimization.cpp
 *
 *  Implements the ShaderOptimization class.
 *
 *  This class implements a number of optimization and code transformation passes for shader programs
 *  using the ShaderInstruction class.
 *
 */


#include "ShaderOptimization.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sstream>

using namespace gpu3d;


//
//
// Support tables
//
//

extern SwizzleMode translateSwizzleModeTable[];

u32bit ShaderOptimization::mappingTableComp0[] =
{
    0, 0, 0, 0, 0, 0,
    1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3
};

u32bit ShaderOptimization::mappingTableComp1[] =
{
    1, 1, 2, 2, 3, 3,
    0, 0, 2, 2, 3, 3,
    0, 0, 1, 1, 3, 3,
    0, 0, 1, 1, 2, 2
};

u32bit ShaderOptimization::mappingTableComp2[] =
{
    2, 3, 1, 3, 1, 2,
    2, 3, 0, 3, 0, 2,
    1, 3, 0, 3, 0, 1,
    1, 2, 0, 2, 0, 1
};

u32bit ShaderOptimization::mappingTableComp3[] =
{
    3, 2, 3, 1, 2, 1,
    3, 2, 3, 0, 2, 0,
    3, 1, 3, 0, 1, 0,
    2, 1, 2, 0, 1, 0
};


//
//
//  Instruction classification.
//
//   Scalar result (broadcast): DP3, DP4, DPH, EX2, FRC, LG2, RCP, RSQ
//   Vector operation : ADD, CMP, CMP_KIL, MAD, FXMAD, FXMAD2, MAX, MIN, MOV, MUL, FXMUL, SGE, SLT
//   SIMD4 result : DST, EXP, LDA, LIT, LOG, TEX, TXB, TXL, TXP
//   No result : KIL, KLS, ZXP, ZXS, NOP, CHS
//   Address register result: ARL
//   Not implemented : FLR
//

bool ShaderOptimization::hasScalarResult(ShOpcode opc)
{
    return (opc == DP3) || (opc == DP4) || (opc == DPH) || (opc == EX2) ||
           (opc == FRC) || (opc == LG2) || (opc == RCP) || (opc == RSQ) ||
           (opc == COS) || (opc == SIN);
}

bool ShaderOptimization::isVectorOperation(ShOpcode opc)
{
    return (opc == ADD)    || (opc == CMP) || (opc == CMPKIL)  || (opc == MAD) || (opc == FXMAD) ||
           (opc == FXMAD2) || (opc == MAX) || (opc == MIN)     || (opc == MOV) || (opc == MUL)   ||
           (opc == FXMUL)  || (opc == SGE) || (opc == SLT)     || (opc == DDX) || (opc == DDY) ||
           (opc == ADDI) || (opc == MULI);
}

bool ShaderOptimization::hasSIMD4Result(ShOpcode opc)
{
    return (opc == DST) || (opc == EXP) || (opc == LDA) || (opc == LIT) ||
           (opc == LOG) || (opc == TEX) || (opc == TXB) || (opc == TXL) ||
           (opc == TXP);
}

bool ShaderOptimization::hasNoResult(ShOpcode opc)
{
    return (opc == KIL) || (opc == KLS) || (opc == ZXP) || (opc == ZXS) || (opc == NOP) || (opc == CHS) || (opc == JMP);
}

bool ShaderOptimization::hasAddrRegResult(ShOpcode opc)
{
    return (opc == ARL);
}

bool ShaderOptimization::notImplemented(ShOpcode opc)
{
    return (opc == FLR);
}

bool ShaderOptimization::mustDisableEarlyZ(ShOpcode opc)
{
    return (opc == KIL) || (opc == KLS) || (opc == ZXP) || (opc == ZXS);
}


//
//
//  Support functions for the optimization and transformation passes.
//
//


ShaderInstruction *ShaderOptimization::copyInstruction(ShaderInstruction *origInstr)
{
    ShaderInstruction *copyInstr;

    copyInstr = new ShaderInstruction(
                        //  Opcode
                        origInstr->getOpcode(),
                        //  First operand
                        origInstr->getBankOp1(), origInstr->getOp1(), origInstr->getOp1NegateFlag(),
                        origInstr->getOp1AbsoluteFlag(), origInstr->getOp1SwizzleMode(),
                        //  Second operand
                        origInstr->getBankOp2(), origInstr->getOp2(), origInstr->getOp2NegateFlag(),
                        origInstr->getOp2AbsoluteFlag(), origInstr->getOp2SwizzleMode(),
                        //  Third operand
                        origInstr->getBankOp3(), origInstr->getOp3(), origInstr->getOp3NegateFlag(),
                        origInstr->getOp3AbsoluteFlag(), origInstr->getOp3SwizzleMode(),
                        //  Result
                        origInstr->getBankRes(), origInstr->getResult(), origInstr->getSaturatedRes(),
                        origInstr->getResultMaskMode(),
                        //  Predication
                        origInstr->getPredicatedFlag(), origInstr->getNegatePredicateFlag(), origInstr->getPredicateReg(),
                        //  Relative addressing mode
                        origInstr->getRelativeModeFlag(), origInstr->getRelMAddrReg(),
                        origInstr->getRelMAddrRegComp(), origInstr->getRelMOffset(),
                        //  End flag
                        origInstr->getEndFlag()
                    );

    return copyInstr;
}

ShaderInstruction *ShaderOptimization::patchedOpsInstruction(ShaderInstruction *origInstr, u32bit patchRegOp1, Bank patchBankOp1,
                                         u32bit patchRegOp2, Bank patchBankOp2, u32bit patchRegOp3, Bank patchBankOp3)
{
    ShaderInstruction *patchedInstr;

    patchedInstr = new ShaderInstruction(
                            //  Opcode
                            origInstr->getOpcode(),
                            //  First operand
                            patchBankOp1, patchRegOp1, origInstr->getOp1NegateFlag(), origInstr->getOp1AbsoluteFlag(),
                            origInstr->getOp1SwizzleMode(),
                            //  Second operand
                            patchBankOp2, patchRegOp2, origInstr->getOp2NegateFlag(), origInstr->getOp2AbsoluteFlag(),
                            origInstr->getOp2SwizzleMode(),
                            //  Third operand
                            patchBankOp3, patchRegOp3, origInstr->getOp3NegateFlag(), origInstr->getOp3AbsoluteFlag(),
                            origInstr->getOp3SwizzleMode(),
                            //  Result
                            origInstr->getBankRes(), origInstr->getResult(), origInstr->getSaturatedRes(),
                            origInstr->getResultMaskMode(),
                            //  Predication
                            origInstr->getPredicatedFlag(), origInstr->getNegatePredicateFlag(), origInstr->getPredicateReg(),
                            //  Relative addressing mode
                            origInstr->getRelativeModeFlag(), origInstr->getRelMAddrReg(),
                            origInstr->getRelMAddrRegComp(), origInstr->getRelMOffset(),
                            //  End flag
                            origInstr->getEndFlag()
                      );

    return patchedInstr;
}

ShaderInstruction *ShaderOptimization::patchResMaskInstruction(ShaderInstruction *origInstr, MaskMode resultMask)
{
    ShaderInstruction *patchedInstr;

    patchedInstr = new ShaderInstruction(
                        //  Opcode
                        origInstr->getOpcode(),
                        //  First operand
                        origInstr->getBankOp1(), origInstr->getOp1(), origInstr->getOp1NegateFlag(),
                        origInstr->getOp1AbsoluteFlag(), origInstr->getOp1SwizzleMode(),
                        //  Second operand
                        origInstr->getBankOp2(), origInstr->getOp2(), origInstr->getOp2NegateFlag(),
                        origInstr->getOp2AbsoluteFlag(), origInstr->getOp2SwizzleMode(),
                        //  Third operand
                        origInstr->getBankOp3(), origInstr->getOp3(), origInstr->getOp3NegateFlag(),
                        origInstr->getOp3AbsoluteFlag(), origInstr->getOp3SwizzleMode(),
                        //  Result
                        origInstr->getBankRes(), origInstr->getResult(), origInstr->getSaturatedRes(),
                        resultMask,
                        //  Predication
                        origInstr->getPredicatedFlag(), origInstr->getNegatePredicateFlag(), origInstr->getPredicateReg(),
                        //  Relative addressing mode
                        origInstr->getRelativeModeFlag(), origInstr->getRelMAddrReg(),
                        origInstr->getRelMAddrRegComp(), origInstr->getRelMOffset(),
                        //  End flag
                        origInstr->getEndFlag()
                    );

    return patchedInstr;
}


ShaderInstruction *ShaderOptimization::renameRegsInstruction(ShaderInstruction *origInstr, u32bit resName, u32bit op1Name, u32bit op2Name, u32bit op3Name)
{
    ShaderInstruction *renamedInstr;

    renamedInstr = new ShaderInstruction(
                        //  Opcode
                        origInstr->getOpcode(),
                        //  First operand
                        origInstr->getBankOp1(), op1Name, origInstr->getOp1NegateFlag(),
                        origInstr->getOp1AbsoluteFlag(), origInstr->getOp1SwizzleMode(),
                        //  Second operand
                        origInstr->getBankOp2(), op2Name, origInstr->getOp2NegateFlag(),
                        origInstr->getOp2AbsoluteFlag(), origInstr->getOp2SwizzleMode(),
                        //  Third operand
                        origInstr->getBankOp3(), op3Name, origInstr->getOp3NegateFlag(),
                        origInstr->getOp3AbsoluteFlag(), origInstr->getOp3SwizzleMode(),
                        //  Result
                        origInstr->getBankRes(), resName, origInstr->getSaturatedRes(),
                        origInstr->getResultMaskMode(),
                        //  Predication
                        origInstr->getPredicatedFlag(), origInstr->getNegatePredicateFlag(), origInstr->getPredicateReg(),
                        //  Relative addressing mode
                        origInstr->getRelativeModeFlag(), origInstr->getRelMAddrReg(),
                        origInstr->getRelMAddrRegComp(), origInstr->getRelMOffset(),
                        //  End flag
                        origInstr->getEndFlag()
                    );

    return renamedInstr;
}

ShaderInstruction *ShaderOptimization::setRegsAndCompsInstruction(ShaderInstruction *origInstr, u32bit resReg, MaskMode resMask,
                                              u32bit op1Reg, SwizzleMode op1Swz, u32bit op2Reg, SwizzleMode op2Swz, u32bit op3Reg, SwizzleMode op3Swz)
{
    ShaderInstruction *setRegCompsInstr;

    setRegCompsInstr = new ShaderInstruction(
                        //  Opcode
                        origInstr->getOpcode(),
                        //  First operand
                        origInstr->getBankOp1(), op1Reg, origInstr->getOp1NegateFlag(),
                        origInstr->getOp1AbsoluteFlag(), op1Swz,
                        //  Second operand
                        origInstr->getBankOp2(), op2Reg, origInstr->getOp2NegateFlag(),
                        origInstr->getOp2AbsoluteFlag(), op2Swz,
                        //  Third operand
                        origInstr->getBankOp3(), op3Reg, origInstr->getOp3NegateFlag(),
                        origInstr->getOp3AbsoluteFlag(), op3Swz,
                        //  Result
                        origInstr->getBankRes(), resReg, origInstr->getSaturatedRes(),
                        resMask,
                        //  Predication
                        origInstr->getPredicatedFlag(), origInstr->getNegatePredicateFlag(), origInstr->getPredicateReg(),
                        //  Relative addressing mode
                        origInstr->getRelativeModeFlag(), origInstr->getRelMAddrReg(),
                        origInstr->getRelMAddrRegComp(), origInstr->getRelMOffset(),
                        //  End flag
                        origInstr->getEndFlag()
                    );

    return setRegCompsInstr;
}

ShaderInstruction *ShaderOptimization::patchSOAInstruction(ShaderInstruction *origInstr,
                                                           SwizzleMode op1Comp, SwizzleMode op2Comp, SwizzleMode op3Comp, MaskMode resComp)
{
    ShaderInstruction *soaInstr;

    soaInstr = new ShaderInstruction(
                        //  Opcode
                        origInstr->getOpcode(),
                        //  First operand
                        origInstr->getBankOp1(), origInstr->getOp1(), origInstr->getOp1NegateFlag(),
                        origInstr->getOp1AbsoluteFlag(), op1Comp,
                        //  Second operand
                        origInstr->getBankOp2(), origInstr->getOp2(), origInstr->getOp2NegateFlag(),
                        origInstr->getOp2AbsoluteFlag(), op2Comp,
                        //  Third operand
                        origInstr->getBankOp3(), origInstr->getOp3(), origInstr->getOp3NegateFlag(),
                        origInstr->getOp3AbsoluteFlag(), op3Comp,
                        //  Result
                        origInstr->getBankRes(), origInstr->getResult(), origInstr->getSaturatedRes(),
                        resComp,
                        //  Predication
                        origInstr->getPredicatedFlag(), origInstr->getNegatePredicateFlag(), origInstr->getPredicateReg(),
                        //  Relative addressing mode
                        origInstr->getRelativeModeFlag(), origInstr->getRelMAddrReg(),
                        origInstr->getRelMAddrRegComp(), origInstr->getRelMOffset(),
                        //  End flag
                        false
                    );

    return soaInstr;
}


void ShaderOptimization::updateTempRegsUsed(ShaderInstruction *instr, bool *tempInUse)
{
    switch(instr->getOpcode())
    {
        case NOP:
        case END:
        case CHS:

            //  Don't use any register.

            break;

        case FLR:

            //  Unimplemented.

            break;

        case KIL:
        case KLS:
        case ZXP:
        case ZXS:
        case JMP:

            //  Does not use the result register.

            break;

        default:

            //  Check result register bank.
            if (instr->getBankRes() == TEMP)
                tempInUse[instr->getResult()] = true;

            //  NOTE:  Temporal registers shouldn't be used if not previously defined!!

            //  Check first operand bank.
            if ((instr->getNumOperands() >= 1) && (instr->getBankOp1() == TEMP))
            {
                //tempInUse[instr->getOp1()] = true;

                //  Check if register was written before being used.
                if (!tempInUse[instr->getOp1()])
                {
                    char buffer[256];
                    sprintf(buffer, "Temporal register %d used with no assigned value.", instr->getOp1());
                    panic("ShaderOptimization" , "updateTempRegsUsed", buffer);
                }
            }

            //  Check second operand bank.
            if ((instr->getNumOperands() >= 2) && (instr->getBankOp2() == TEMP))
            {
                //tempInUse[instr->getOp2()] = true;

                //  Check if register was written before being used.
                if (!tempInUse[instr->getOp2()])
                {
                    char buffer[256];
                    sprintf(buffer, "Temporal register %d used with no assigned value.", instr->getOp2());
                    panic("ShaderOptimization", "updateTempRegsUsed", buffer);
                }
            }

            //  Check third operand bank.
            if ((instr->getNumOperands() == 3) && (instr->getBankOp3() == TEMP))
            {
                //tempInUse[instr->getOp3()] = true;

                //  Check if register was written before being used.
                if (!tempInUse[instr->getOp3()])
                {
                    char buffer[256];
                    sprintf(buffer, "Temporal register %d used with no assigned value.", instr->getOp3());
                    panic("ShaderOptimization", "updateTempRegsUsed", buffer);
                }
            }

            break;
    }
}

void ShaderOptimization::updateInstructionType(ShaderInstruction *instr, unsigned int *instrType)
{
    switch(instr->getOpcode())
    {
        case FLR:

            //  Unimplemented.

            break;

        case LDA:
        case TEX:
        case TXB:
        case TXL:
        case TXP:

            // Texture/Load instructions.
            instrType[TEX_INSTR_TYPE]++;

            break;

        case ARL:
        case DPH:
        case DP3:
        case DP4:
        case DST:
        case LIT:
        case EX2:
        case EXP:
        case FRC:
        case LG2:
        case LOG:
        case RCP:
        case RSQ:
        case MUL:
        case FXMUL:
        case MAD:
        case FXMAD:
        case FXMAD2:
        case MAX:
        case MIN:
        case MOV:
        case SGE:
        case SLT:
        case CMP:
        case ADD:
        case SETPEQ:
        case SETPGT:
        case SETPLT:
        case ANDP:
        case DDX:
        case DDY:
        case ADDI:
        case MULI:
        case STPEQI:
        case STPGTI:
        case STPLTI:

            // ALU instructions.
            instrType[ALU_INSTR_TYPE]++;

            break;

        case NOP:
        case END:
        case KIL:
        case KLS:
        case ZXP:
        case ZXS:
        case CHS:
        case JMP:

            // Control instructions.
            instrType[CTRL_INSTR_TYPE]++;

            break;

        default:

            break;
    }
}

void ShaderOptimization::testSwizzle(SwizzleMode swizzle, bool& includesX, bool& includesY, bool& includesZ, bool& includesW)
{
    includesX = false;
    includesY = false;
    includesZ = false;
    includesW = false;

    switch((swizzle & 0xC0) >> 6)
    {
        case 0x00: includesX = true; break;
        case 0x01: includesY = true; break;
        case 0x02: includesZ = true; break;
        case 0x03: includesW = true; break;
    }

    switch((swizzle & 0x30) >> 4)
    {
        case 0x00: includesX = true; break;
        case 0x01: includesY = true; break;
        case 0x02: includesZ = true; break;
        case 0x03: includesW = true; break;
    }

    switch((swizzle & 0x0C) >> 2)
    {
        case 0x00: includesX = true; break;
        case 0x01: includesY = true; break;
        case 0x02: includesZ = true; break;
        case 0x03: includesW = true; break;
    }

    switch((swizzle & 0x03))
    {
        case 0x00: includesX = true; break;
        case 0x01: includesY = true; break;
        case 0x02: includesZ = true; break;
        case 0x03: includesW = true; break;
    }
}

void ShaderOptimization::getTempRegsUsed(vector<ShaderInstruction *> inProgram, bool *tempInUse)
{
    //  Clear the in use flag for all the temporal registers.
    for(u32bit reg = 0; reg < MAX_TEMPORAL_REGISTERS; reg++)
        tempInUse[reg] = false;

    //  Detect which temporal registers are in use.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        updateTempRegsUsed(inProgram[instr], tempInUse);
    }
}

void ShaderOptimization::extractOperandComponents(SwizzleMode opSwizzle, vector<SwizzleMode> &components)
{
    components.clear();
    components.resize(4);

    u32bit swizzleMode = opSwizzle;

    //  The components are ordered from first component to fourth component
    //  from the MSB to the LSB of the swizzle mode mask.
    for(u32bit comp = 4; comp > 0; comp--)
    {
        switch (swizzleMode & 0x03)
        {
            case 0:  //  Select X component.
                components[comp - 1] = XXXX;
                break;

            case 1: //  Select Y component.
                components[comp - 1] = YYYY;
                break;

            case 2: //  Select Y component.
                components[comp - 1] = ZZZZ;
                break;

            case 3: //  Select Y component.
                components[comp - 1] = WWWW;
                break;

        }

        //  Shift to next component in the swizzle mask.
        swizzleMode = swizzleMode >> 2;
    }
}

void ShaderOptimization::extractResultComponents(MaskMode resMask, vector<MaskMode> &componentsMask, vector<SwizzleMode> &componentsSwizzle)
{
    componentsMask.clear();
    componentsSwizzle.clear();

    switch(resMask)
    {
        case NNNN:
            break;

        case NNNW:
            componentsMask.push_back(NNNW);
            componentsSwizzle.push_back(WWWW);
            break;

        case NNZN:
            componentsMask.push_back(NNZN);
            componentsSwizzle.push_back(ZZZZ);
            break;

        case NNZW:
            componentsMask.push_back(NNZN);
            componentsSwizzle.push_back(ZZZZ);
            componentsMask.push_back(NNNW);
            componentsSwizzle.push_back(WWWW);
            break;

        case NYNN:
            componentsMask.push_back(NYNN);
            componentsSwizzle.push_back(YYYY);
            break;

        case NYNW:
            componentsMask.push_back(NYNN);
            componentsSwizzle.push_back(YYYY);
            componentsMask.push_back(NNNW);
            componentsSwizzle.push_back(WWWW);
            break;

        case NYZN:
            componentsMask.push_back(NYNN);
            componentsSwizzle.push_back(YYYY);
            componentsMask.push_back(NNZN);
            componentsSwizzle.push_back(ZZZZ);
            break;

        case NYZW:
            componentsMask.push_back(NYNN);
            componentsSwizzle.push_back(YYYY);
            componentsMask.push_back(NNZN);
            componentsSwizzle.push_back(ZZZZ);
            componentsMask.push_back(NNNW);
            componentsSwizzle.push_back(WWWW);
            break;

        case XNNN:
            componentsMask.push_back(XNNN);
            componentsSwizzle.push_back(XXXX);
            break;

        case XNNW:
            componentsMask.push_back(XNNN);
            componentsSwizzle.push_back(XXXX);
            componentsMask.push_back(NNNW);
            componentsSwizzle.push_back(WWWW);
            break;

        case XNZN:
            componentsMask.push_back(XNNN);
            componentsSwizzle.push_back(XXXX);
            componentsMask.push_back(NNZN);
            componentsSwizzle.push_back(ZZZZ);
            break;

        case XNZW:
            componentsMask.push_back(XNNN);
            componentsSwizzle.push_back(XXXX);
            componentsMask.push_back(NNZN);
            componentsSwizzle.push_back(ZZZZ);
            componentsMask.push_back(NNNW);
            componentsSwizzle.push_back(WWWW);
            break;

        case XYNN:
            componentsMask.push_back(XNNN);
            componentsSwizzle.push_back(XXXX);
            componentsMask.push_back(NYNN);
            componentsSwizzle.push_back(YYYY);
            break;

        case XYNW:
            componentsMask.push_back(XNNN);
            componentsSwizzle.push_back(XXXX);
            componentsMask.push_back(NYNN);
            componentsSwizzle.push_back(YYYY);
            componentsMask.push_back(NNNW);
            componentsSwizzle.push_back(WWWW);
            break;

        case XYZN:
            componentsMask.push_back(XNNN);
           componentsSwizzle.push_back(XXXX);
            componentsMask.push_back(NYNN);
            componentsSwizzle.push_back(YYYY);
            componentsMask.push_back(NNZN);
            componentsSwizzle.push_back(ZZZZ);
            break;

        case mXYZW:
            componentsMask.push_back(XNNN);
            componentsSwizzle.push_back(XXXX);
            componentsMask.push_back(NYNN);
            componentsSwizzle.push_back(YYYY);
            componentsMask.push_back(NNZN);
            componentsSwizzle.push_back(ZZZZ);
            componentsMask.push_back(NNNW);
            componentsSwizzle.push_back(WWWW);
            break;
    }
}

void ShaderOptimization::getReadOperandComponents(ShaderInstruction *instr, vector<SwizzleMode> resComponentsSwizzle,
                                                  vector<u32bit> &readComponentsOp1, vector<u32bit> &readComponentsOp2)
{

    //  The list of components written in the result register will be used as the reference
    //  to determine which components of the operand registers were actually read.
    //  However for some instructions there is no direct relation between the components written
    //  and the components read so we need to create a special list of components.
    switch(instr->getOpcode())
    {
        case NOP:
        case END:
        case CHS:

            //  These instructions don't set or use any register.

            break;

        case FLR:

            //  Unimplemented.

            break;

        case JMP:
        
            //  A single component is read.
            readComponentsOp1.clear();
            readComponentsOp1.push_back(0);
            break;
            
        case DP4:
        case KIL:
        case KLS:
        case ZXP:
        case ZXS:

            //  For those instructions all the operand components are read.
            readComponentsOp1.clear();
            readComponentsOp1.push_back(0);
            readComponentsOp1.push_back(1);
            readComponentsOp1.push_back(2);
            readComponentsOp1.push_back(3);

            readComponentsOp2.clear();
            readComponentsOp2.push_back(0);
            readComponentsOp2.push_back(1);
            readComponentsOp2.push_back(2);
            readComponentsOp2.push_back(3);

            break;

        case TXB:
        case TXL:
        case TXP:

            //  For those instructions all the operand components are read.
            readComponentsOp1.clear();
            readComponentsOp1.push_back(0);
            readComponentsOp1.push_back(1);
            readComponentsOp1.push_back(2);
            readComponentsOp1.push_back(3);
            
            readComponentsOp2.clear();
            //readComponentsOp2.push_back(0);
            //readComponentsOp2.push_back(1);
            //readComponentsOp2.push_back(2);
            //readComponentsOp2.push_back(3);


            break;

        case DPH:

            //  DPH is a very special case.  From the first operand the three first components are read.
            //  But for the second operand all four components are read.

            readComponentsOp1.clear();
            readComponentsOp1.push_back(0);
            readComponentsOp1.push_back(1);
            readComponentsOp1.push_back(2);

            readComponentsOp1.clear();
            readComponentsOp2.push_back(0);
            readComponentsOp2.push_back(1);
            readComponentsOp2.push_back(2);
            readComponentsOp2.push_back(3);

            break;

        case DP3:

            //  The three first components are read.  Not affected by the result components.
            readComponentsOp1.clear();
            readComponentsOp1.push_back(0);
            readComponentsOp1.push_back(1);
            readComponentsOp1.push_back(2);

            readComponentsOp2.clear();
            readComponentsOp2.push_back(0);
            readComponentsOp2.push_back(1);
            readComponentsOp2.push_back(2);

            break;

        case TEX:

            //  The three first components are read.  Not affected by the result components.
            readComponentsOp1.clear();
            readComponentsOp1.push_back(0);
            readComponentsOp1.push_back(1);
            readComponentsOp1.push_back(2);
            
            readComponentsOp2.clear();
            //readComponentsOp2.push_back(0);
            //readComponentsOp2.push_back(1);
            //readComponentsOp2.push_back(2);

            break;

        case LDA:

            //  LDA only reads the index from the first component.
            //  Only the first operand is actually read.
            readComponentsOp1.clear();
            readComponentsOp1.push_back(0);

            readComponentsOp2.clear();
            //readComponentsOp2.push_back(0);

            break;

        case DST:

            //  DST is a very special case.
            //
            //  For the first operand:
            //
            //    1st component is not read
            //    2nd component is read if the 2nd result component is written
            //    3rd component is read if the 3rd result component is written
            //    4th component is read if the 4th result component is written
            //
            //  For the second operand:
            //
            //    2nd component is read if the 2nd result component is written
            //    all other components are not read
            //

            readComponentsOp1.clear();
            readComponentsOp2.clear();

            for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
            {
                if ((resComponentsSwizzle[comp] & 0x03) != 0)
                    readComponentsOp1.push_back(resComponentsSwizzle[comp] & 0x03);

                if ((resComponentsSwizzle[comp] & 0x03) == 1)
                    readComponentsOp2.push_back(1);
            }

            break;

        case LIT:

            //  LIT is a special case.
            //
            //  LIT has a single operand:
            //
            //    1st component is read if 2nd or 3rd result components are written
            //    2nd component is read if 3rd result component is written
            //    4th component is read if 3rd result component is written
            //

            {
                readComponentsOp1.clear();
                readComponentsOp2.clear();

                bool secondResCompWritten = false;
                bool thirdResCompWritten = false;


                for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                {
                    secondResCompWritten = secondResCompWritten || ((resComponentsSwizzle[comp] & 0x03) == 1);
                    thirdResCompWritten = thirdResCompWritten || ((resComponentsSwizzle[comp] & 0x03) == 2);
                }

                if (thirdResCompWritten)
                {
                    readComponentsOp1.push_back(0);
                    readComponentsOp1.push_back(1);
                    readComponentsOp1.push_back(3);
                }
                else if (secondResCompWritten)
                {
                    readComponentsOp1.push_back(0);
                }
            }

            break;

        case EX2:
        case EXP:
        case FRC:
        case LG2:
        case LOG:
        case RCP:
        case RSQ:
        case SETPEQ:
        case SETPGT:
        case SETPLT:
        case ANDP:
        case STPEQI:
        case STPGTI:
        case STPLTI:

            //  Scalar operations but with result broadcast (result mask may not be a single component).
            //  For scalar instructions the swizzle mask for the only existing operand should always
            //  be a scalar/broadcast (.x, .y, .z or .w).  As the result can be broadcasted to multiple
            //  result components we need to return multiple read components.

            readComponentsOp1.clear();
            readComponentsOp2.clear();

            for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
            {
                readComponentsOp1.push_back(resComponentsSwizzle[comp] & 0x03);
                readComponentsOp2.push_back(resComponentsSwizzle[comp] & 0x03);
            }

            break;

        default:

            //  For all other cases the read components depend on the written components in
            //  the result register.

            readComponentsOp1.clear();
            readComponentsOp2.clear();

            for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
            {
                readComponentsOp1.push_back(resComponentsSwizzle[comp] & 0x03);
                readComponentsOp2.push_back(resComponentsSwizzle[comp] & 0x03);
            }

            break;
    }
}

//
//
//  Shader program management.
//

void ShaderOptimization::copyProgram(vector<ShaderInstruction *> inProgram, vector<ShaderInstruction *> &outProgram)
{
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        ShaderInstruction *copyInstr;

        copyInstr = copyInstruction(inProgram[instr]);
        outProgram.push_back(copyInstr);
    }
}

void ShaderOptimization::concatenateProgram(vector<ShaderInstruction *> srcProgram, vector<ShaderInstruction *>& destProgram)

{
    if (!destProgram.empty())
    {
        //  Reset end flag in the last instruction
        destProgram[destProgram.size() - 1]->setEndFlag(false);
    }

    //  Concatenate source shader code.
    for(u32bit instr = 0; instr < srcProgram.size(); instr++)
    {
        ShaderInstruction *copyInstr;

        //  Copy the instruction.
        copyInstr = copyInstruction(srcProgram[instr]);
        destProgram.push_back(copyInstr);
    }

}

void ShaderOptimization::deleteProgram(vector<ShaderInstruction *> &inProgram)
{
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        delete inProgram[instr];
    }

    inProgram.clear();
}

void ShaderOptimization::printProgram(vector<ShaderInstruction *> inProgram)
{
    //  Disassemble the program.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        char instrDisasm[256];
        u8bit instrCode[16];

        //  Disassemble the instruction.
        inProgram[instr]->disassemble(instrDisasm);

        //  Get the instruction code.
        inProgram[instr]->getCode(instrCode);

        //  Print the instruction.
        printf("%04x :", instr * 16);
        for(u32bit b = 0; b < 16; b++)
            printf(" %02x", instrCode[b]);
        printf("    %s\n", instrDisasm);
    }
}

MaskMode ShaderOptimization::encodeWriteMask(bool *activeResComp)
{
    if (activeResComp[0] && activeResComp[1] && activeResComp[2] && activeResComp[3])
        return mXYZW;
    else if (activeResComp[0] && activeResComp[1] && activeResComp[2])
        return XYZN;
    else if (activeResComp[0] && activeResComp[1] && activeResComp[3])
        return XYNW;
    else if (activeResComp[0] && activeResComp[2] && activeResComp[3])
        return XNZW;
    else if (activeResComp[1] && activeResComp[2] && activeResComp[3])
        return NYZW;
    else if (activeResComp[0] && activeResComp[1])
        return XYNN;
    else if (activeResComp[0] && activeResComp[2])
        return XNZN;
    else if (activeResComp[0] && activeResComp[3])
        return XNNW;
    else if (activeResComp[1] && activeResComp[2])
        return NYZN;
    else if (activeResComp[1] && activeResComp[3])
        return NYNW;
    else if (activeResComp[2] && activeResComp[3])
        return NNZW;
    else if (activeResComp[0])
        return XNNN;
    else if (activeResComp[1])
        return NYNN;
    else if (activeResComp[2])
        return NNZN;
    else if (activeResComp[3])
        return NNNW;
    else
        return NNNN;
}

MaskMode ShaderOptimization::removeComponentsFromWriteMask(MaskMode inResMask, bool *removeResComp)
{
    bool activeResComp[4];

    activeResComp[0] = (inResMask == XNNN) || (inResMask == XNNW) || (inResMask == XNZN) ||
                       (inResMask == XNZW) || (inResMask == XYNN) || (inResMask == XYNW) ||
                       (inResMask == XYZN) || (inResMask == mXYZW);

    activeResComp[1] = (inResMask == NYNN) || (inResMask == NYNW) || (inResMask == NYZN) ||
                       (inResMask == NYZW) || (inResMask == XYNN) || (inResMask == XYNW) ||
                       (inResMask == XYZN) || (inResMask == mXYZW);

    activeResComp[2] = (inResMask == NNZN) || (inResMask == NNZW) || (inResMask == NYZN) ||
                       (inResMask == NYZW) || (inResMask == XNZN) || (inResMask == XNZW) ||
                       (inResMask == XYZN) || (inResMask == mXYZW);

    activeResComp[3] = (inResMask == NNNW) || (inResMask == NNZW) || (inResMask == NYNW) ||
                       (inResMask == NYZW) || (inResMask == XNNW) || (inResMask == XNZW) ||
                       (inResMask == XYNW) || (inResMask == mXYZW);

    activeResComp[0] = activeResComp[0] && !removeResComp[0];
    activeResComp[1] = activeResComp[1] && !removeResComp[1];
    activeResComp[2] = activeResComp[2] && !removeResComp[2];
    activeResComp[3] = activeResComp[3] && !removeResComp[3];

    return encodeWriteMask(activeResComp);
}

SwizzleMode ShaderOptimization::encodeSwizzle(u32bit *opSwizzle)
{
    u32bit swizzleMode = 0;

    for(u32bit comp = 0; comp < 4; comp++)
    {
        swizzleMode = (swizzleMode << 2) | (opSwizzle[comp] & 0x03);
    }

    //return translateSwizzleMode[swizzleMode];
    return translateSwizzleModeTable[swizzleMode];
}

//
//  Transformation and optimization passes.
//
//

void ShaderOptimization::attribute2lda(vector<ShaderInstruction *> inProgram, vector<ShaderInstruction *> &outProgram)
{
    bool tempInUse[MAX_TEMPORAL_REGISTERS];

    getTempRegsUsed(inProgram, tempInUse);

    outProgram.clear();

    u32bit input2temp[MAX_INPUT_ATTRIBUTES];

    for(u32bit attr = 0; attr < MAX_INPUT_ATTRIBUTES; attr++)
        input2temp[attr] = MAX_TEMPORAL_REGISTERS;

    //  Now convert input attribute registers into temporal registers loaded using LDA.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        //  Check MOV instructions.
        if ((inProgram[instr]->getOpcode() == MOV) && (inProgram[instr]->getBankOp1() == IN))
        {
            //  Check if the attribute was already loaded into a temporal register.
            if (input2temp[inProgram[instr]->getOp1()] == MAX_TEMPORAL_REGISTERS)
            {
                //  If the attribute was not already loaded check if the attribute data
                //  is modified by swizzling or negate, absolute modifiers.
                if (inProgram[instr]->getOp1AbsoluteFlag() || inProgram[instr]->getOp1NegateFlag() ||
                    inProgram[instr]->getOp1SwizzleMode() != XYZW)
                {
                    //  Add LDA instruction to load attribute into a temporal register.

                    u32bit newTemp;

                    //  Select the next free temporal register to store the attribute.
                    for(newTemp = 0; (newTemp < MAX_TEMPORAL_REGISTERS) && tempInUse[newTemp]; newTemp++);

                    //  Check if there are no more temporal registers.
                    if (newTemp == MAX_TEMPORAL_REGISTERS)
                    {
                        panic("ShaderOptimization", "attribute2lda", "There no unused temporal registers available.");
                    }

                    //  Set temporal register as in use and associate with attribute.
                    tempInUse[newTemp] = true;
                    input2temp[inProgram[instr]->getOp1()] = newTemp;

                    ShaderInstruction *newLDAInstr;

                    newLDAInstr = new ShaderInstruction(
                                            //  Opcode
                                            LDA,
                                            //  First operand
                                            IN, INDEX_ATTRIBUTE, false, false, XXXX,
                                            //  Second operand
                                            TEXT, inProgram[instr]->getOp1(), false, false, XYZW,
                                            //  Third operand
                                            INVALID, 0, false, false, XYZW,
                                            //  Result
                                            TEMP, newTemp, false, mXYZW,
                                            //  Predication
                                            false, false, 0,
                                            //  Relative addressing mode
                                            false, 0, 0, 0,
                                            //  End flag
                                            false
                                      );

                    outProgram.push_back(newLDAInstr);

                    //  Patch the MOV instruction with the new temporal register.
                    ShaderInstruction *newMOVInstr;

                    newMOVInstr = patchedOpsInstruction(inProgram[instr], newTemp, TEMP,
                                                        inProgram[instr]->getOp2(), inProgram[instr]->getBankOp2(),
                                                        inProgram[instr]->getOp3(), inProgram[instr]->getBankOp3());

                    outProgram.push_back(newMOVInstr);
                }
                else
                {
                    //  MOVs from an input attribute register are directly converted into LDA when
                    //  the read value is not modified (abs, neg or swizzle).
                    ShaderInstruction *newLDAInstr;

                    newLDAInstr = new ShaderInstruction(
                                            //  Opcode
                                            LDA,
                                            //  First operand
                                            IN, INDEX_ATTRIBUTE, false, false, XXXX,
                                            //  Second operand
                                            TEXT, inProgram[instr]->getOp1(), false, false, XYZW,
                                            //  Third operand
                                            INVALID, 0, false, false, XYZW,
                                            //  Result
                                            inProgram[instr]->getBankRes(), inProgram[instr]->getResult(), inProgram[instr]->getSaturatedRes(),
                                            inProgram[instr]->getResultMaskMode(),
                                            //  Predication
                                            false, false, 0,
                                            //  Relative addressing mode
                                            false, 0, 0, 0,
                                            //  End flag
                                            inProgram[instr]->getEndFlag()
                                      );

                    outProgram.push_back(newLDAInstr);
                }
            }
            else
            {
                //  Recode the MOV instruction to use the temporal register.
                ShaderInstruction *newMOVInstr;

                newMOVInstr = patchedOpsInstruction(inProgram[instr], input2temp[inProgram[instr]->getOp1()], TEMP,
                                                    inProgram[instr]->getOp2(), inProgram[instr]->getBankOp2(),
                                                    inProgram[instr]->getOp3(), inProgram[instr]->getBankOp3());

                outProgram.push_back(newMOVInstr);
            }
        }
        else
        {
            u32bit op1Reg = inProgram[instr]->getOp1();
            Bank op1Bank = inProgram[instr]->getBankOp1();
            u32bit op2Reg = inProgram[instr]->getOp2();
            Bank op2Bank = inProgram[instr]->getBankOp2();
            u32bit op3Reg = inProgram[instr]->getOp3();
            Bank op3Bank = inProgram[instr]->getBankOp3();

            bool patchInstruction = false;

            //  Check if the first operand is an input attribute register.
            if ((inProgram[instr]->getNumOperands() >= 1) && (op1Bank == IN))
            {
                //  Check if the attribute was already loaded.
                if (input2temp[op1Reg] == MAX_TEMPORAL_REGISTERS)
                {
                    //  Add LDA instruction to load attribute into a temporal register.
                    u32bit newTemp;

                    //  Select the next free temporal register to store the attribute.
                    for(newTemp = 0; (newTemp < MAX_TEMPORAL_REGISTERS) && tempInUse[newTemp]; newTemp++);

                    //  Check if there are no more temporal registers.
                    if (newTemp == MAX_TEMPORAL_REGISTERS)
                    {
                        panic("ShaderOptimization", "attribute2lda", "There no unused temporal registers available.");
                    }

                    //  Set temporal register as in use and associate with attribute.
                    tempInUse[newTemp] = true;
                    input2temp[op1Reg] = newTemp;

                    ShaderInstruction *newLDAInstr;

                    newLDAInstr = new ShaderInstruction(
                                            //  Opcode
                                            LDA,
                                            //  First operand
                                            IN, INDEX_ATTRIBUTE, false, false, XXXX,
                                            //  Second operand
                                            TEXT, op1Reg, false, false, XYZW,
                                            //  Third operand
                                            INVALID, 0, false, false, XYZW,
                                            //  Result
                                            TEMP, newTemp, false, mXYZW,
                                            //  Predication
                                            false, false, 0,
                                            //  Relative addressing mode
                                            false, 0, 0, 0,
                                            //  End flag
                                            false
                                      );

                    outProgram.push_back(newLDAInstr);

                    //  Use the new temporal register.
                    op1Reg = newTemp;
                }
                else
                {
                    //  Use the temporal register already loaded.
                    op1Reg = input2temp[op1Reg];
                }

                patchInstruction = true;
                op1Bank = TEMP;
            }

            //  Check if the second operand is an input attribute register.
            if ((inProgram[instr]->getNumOperands() >= 2) && (op2Bank == IN))
            {
                //  Check if the attribute was already loaded.
                if (input2temp[op2Reg] == MAX_TEMPORAL_REGISTERS)
                {
                    //  Add LDA instruction to load attribute into a temporal register.
                    u32bit newTemp;

                    //  Select the next free temporal register to store the attribute.
                    for(newTemp = 0; (newTemp < MAX_TEMPORAL_REGISTERS) && tempInUse[newTemp]; newTemp++);

                    //  Check if there are no more temporal registers.
                    if (newTemp == MAX_TEMPORAL_REGISTERS)
                    {
                        panic("ShaderOptimization", "attribute2lda", "There no unused temporal registers available.");
                    }

                    //  Set temporal register as in use and associate with attribute.
                    tempInUse[newTemp] = true;
                    input2temp[op2Reg] = newTemp;

                    ShaderInstruction *newLDAInstr;

                    newLDAInstr = new ShaderInstruction(
                                            //  Opcode
                                            LDA,
                                            //  First operand
                                            IN, INDEX_ATTRIBUTE, false, false, XXXX,
                                            //  Second operand
                                            TEXT, op2Reg, false, false, XYZW,
                                            //  Third operand
                                            INVALID, 0, false, false, XYZW,
                                            //  Result
                                            TEMP, newTemp, false, mXYZW,
                                             //  Predication
                                             false, false, 0,
                                            //  Relative addressing mode
                                            false, 0, 0, 0,
                                            //  End flag
                                            false
                                      );

                    outProgram.push_back(newLDAInstr);

                    //  Use the new temporal register.
                    op2Reg = newTemp;
                }
                else
                {
                    //  Use the temporal register already loaded.
                    op2Reg = input2temp[op2Reg];
                }

                patchInstruction = true;
                op2Bank = TEMP;
            }

            //  Check if the third operand is an input attribute register.
            if ((inProgram[instr]->getNumOperands() >= 3) && (op3Bank == IN))
            {
                //  Check if the attribute was already loaded.
                if (input2temp[op3Reg] == MAX_TEMPORAL_REGISTERS)
                {
                    //  Add LDA instruction to load attribute into a temporal register.
                    u32bit newTemp;

                    //  Select the next free temporal register to store the attribute.
                    for(newTemp = 0; (newTemp < MAX_TEMPORAL_REGISTERS) && tempInUse[newTemp]; newTemp++);

                    //  Check if there are no more temporal registers.
                    if (newTemp == MAX_TEMPORAL_REGISTERS)
                    {
                        panic("ShaderOptimization", "attribute2lda", "There no unused temporal registers available.");
                    }

                    //  Set temporal register as in use and associate with attribute.
                    tempInUse[newTemp] = true;
                    input2temp[op3Reg] = newTemp;

                    ShaderInstruction *newLDAInstr;

                    newLDAInstr = new ShaderInstruction(
                                            //  Opcode
                                            LDA,
                                            //  First operand
                                            IN, INDEX_ATTRIBUTE, false, false, XXXX,
                                            //  Second operand
                                            TEXT, op3Reg, false, false, XYZW,
                                            //  Third operand
                                            INVALID, 0, false, false, XYZW,
                                            //  Result
                                            TEMP, newTemp, false, mXYZW,
                                            //  Predication
                                            false, false, 0,
                                            //  Relative addressing mode
                                            false, 0, 0, 0,
                                            //  End flag
                                            false
                                      );

                    outProgram.push_back(newLDAInstr);

                    //  Use the new temporal register.
                    op3Reg = newTemp;
                }
                else
                {
                    //  Use the temporal register already loaded.
                    op3Reg = input2temp[op3Reg];
                }

                patchInstruction = true;
                op3Bank = TEMP;
            }

            //  Check if the instruction has to be patched.
            if (patchInstruction)
            {
                //  Patch the instruction with the temporal registers.
                ShaderInstruction *patchedInstr;

                patchedInstr = patchedOpsInstruction(inProgram[instr], op1Reg, op1Bank, op2Reg, op2Bank, op3Reg, op3Bank);

                outProgram.push_back(patchedInstr);
            }
            else
            {
                //  Copy shader instruction.
                ShaderInstruction *copyInstr;

                copyInstr = copyInstruction(inProgram[instr]);
                outProgram.push_back(copyInstr);
            }

        }
    }
}




float ShaderOptimization::getALUTEXRatio(vector<ShaderInstruction *> inProgram)
{
    unsigned int instrType[3];

    for (unsigned int i = 0; i < 3; i++)
    {
        instrType[i] = 0;
    }

    //  replace input registers references with temporary register references in instructions.
    for(unsigned int instr = 0; instr < inProgram.size(); instr++)
    {
        updateInstructionType(inProgram[instr], instrType);
    }

    return (instrType[TEX_INSTR_TYPE] > 0)? ((float)instrType[ALU_INSTR_TYPE] / (float)instrType[TEX_INSTR_TYPE]) : 0.0f;
}

void ShaderOptimization::aos2soa(vector<ShaderInstruction *> inProgram, vector<ShaderInstruction *> &outProgram)
{
    bool tempInUse[MAX_TEMPORAL_REGISTERS];

    for(u32bit tr = 0; tr < MAX_TEMPORAL_REGISTERS; tr++)
        tempInUse[tr] = false;

    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        ShaderInstruction *copyInstr;

        vector<SwizzleMode> op1Components;
        vector<SwizzleMode> op2Components;
        vector<SwizzleMode> op3Components;

        vector<MaskMode> resComponentsMask;
        vector<SwizzleMode> resComponentsSwizzle;

        vector <u32bit> readComponentsOp1;
        vector <u32bit> readComponentsOp2;

/*printf("aos2soa => Converting instruction ");

char instrDisasm[256];
u8bit instrCode[16];

//  Disassemble the instruction.
inProgram[instr]->disassemble(instrDisasm);

//  Get the instruction code.
inProgram[instr]->getCode(instrCode);

//  Print the instruction.
printf("%04x :", instr * 16);
for(u32bit b = 0; b < 16; b++)
    printf(" %02x", instrCode[b]);
printf("    %s\n", instrDisasm);*/

        //  Update the temporal registers used.
        updateTempRegsUsed(inProgram[instr], tempInUse);

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case TEX:
            case TXB:
            case TXL:
            case TXP:
            case LDA:
            case KIL:
            case KLS:
            case ZXP:
            case ZXS:
            case CHS:
            case SETPEQ:
            case SETPGT:
            case SETPLT:
            case ANDP:
            case JMP:
            case STPEQI:
            case STPGTI:
            case STPLTI:

                //  Special instructions that are not affected by the conversion.

                //  Copy the instruction.
                copyInstr = copyInstruction(inProgram[instr]);
                outProgram.push_back(copyInstr);

                break;

            case DP3:
            case DP4:
            case DPH:
                //  Dot product instruction requires special conversion.

                //  Extract components for first operand.
                extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                //  Extract components for second operand.
                extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                //  Extract components for third operand.
                extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                //  Extract components for result.
                extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                //  Retrieve which operand components are actually read.
                getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                {
                    ShaderInstruction *mulInstr;
                    ShaderInstruction *mad0Instr;
                    ShaderInstruction *mad1Instr;
                    ShaderInstruction *mad2Instr;
                    ShaderInstruction *mad3Instr;

                    Bank resBank;
                    u32bit resReg;

                    //  Check if the result bank can be read back (is not an output attribute register).
                    if (inProgram[instr]->getBankRes() != OUT)
                    {
                        resBank = inProgram[instr]->getBankRes();
                        resReg = inProgram[instr]->getResult();
                    }
                    else
                    {
                        u32bit newTemp;

                        //  Select the next free temporal register to be used to store the intermediate results.
                        for(newTemp = 0; (newTemp < MAX_TEMPORAL_REGISTERS) && tempInUse[newTemp]; newTemp++);

                        //  Check if there are no more temporal registers.
                        if (newTemp == MAX_TEMPORAL_REGISTERS)
                        {
                            panic("ShaderOptimization", "aos2soa", "There no unused temporal registers available.");
                        }

                        //  Set temporal register as in use.
                        tempInUse[newTemp] = true;

                        //  The temporal register will be used for the intermediate values of the dot product operation.
                        //  The final result will be stored in the output attribute register.
                        resBank = TEMP;
                        resReg = newTemp;
                    }

                    //
                    // NOTE: When translating a DPx instruction with the saturated result flag it must be
                    //       taken into account that the result has only to be saturated for the last MAD.
                    //

                    if (inProgram[instr]->getOpcode() != DPH)
                    {
                        mulInstr = new ShaderInstruction(
                                                         //  Opcode
                                                         MUL,
                                                         //  First operand
                                                         inProgram[instr]->getBankOp1(), inProgram[instr]->getOp1(), inProgram[instr]->getOp1NegateFlag(),
                                                         inProgram[instr]->getOp1AbsoluteFlag(), op1Components[0],
                                                         //  Second operand
                                                         inProgram[instr]->getBankOp2(), inProgram[instr]->getOp2(), inProgram[instr]->getOp2NegateFlag(),
                                                         inProgram[instr]->getOp2AbsoluteFlag(), op2Components[0],
                                                         //  Third operand
                                                         INVALID, 0, false, false, XYZW,
                                                         //  Result
                                                         resBank, resReg, false,
                                                         resComponentsMask[0],
                                                         //  Predication
                                                         inProgram[instr]->getPredicatedFlag(),
                                                         inProgram[instr]->getNegatePredicateFlag(),
                                                         inProgram[instr]->getPredicateReg(),
                                                         //  Relative addressing mode
                                                         inProgram[instr]->getRelativeModeFlag(), inProgram[instr]->getRelMAddrReg(),
                                                         inProgram[instr]->getRelMAddrRegComp(), inProgram[instr]->getRelMOffset(),
                                                         //  End flag
                                                         false
                                                         );
                    }
                    else
                    {
                        mad0Instr = new ShaderInstruction(
                                                         //  Opcode
                                                         MAD,
                                                         //  First operand
                                                         inProgram[instr]->getBankOp1(), inProgram[instr]->getOp1(), inProgram[instr]->getOp1NegateFlag(),
                                                         inProgram[instr]->getOp1AbsoluteFlag(), op1Components[0],
                                                         //  Second operand
                                                         inProgram[instr]->getBankOp2(), inProgram[instr]->getOp2(), inProgram[instr]->getOp2NegateFlag(),
                                                         inProgram[instr]->getOp2AbsoluteFlag(), op2Components[0],
                                                         //  Third operand
                                                         inProgram[instr]->getBankOp2(), inProgram[instr]->getOp2(), inProgram[instr]->getOp2NegateFlag(),
                                                         inProgram[instr]->getOp2AbsoluteFlag(), op2Components[3],
                                                         //  Result
                                                         resBank, resReg, false,
                                                         resComponentsMask[0],
                                                         //  Predication
                                                         inProgram[instr]->getPredicatedFlag(),
                                                         inProgram[instr]->getNegatePredicateFlag(),
                                                         inProgram[instr]->getPredicateReg(),
                                                         //  Relative addressing mode
                                                         inProgram[instr]->getRelativeModeFlag(), inProgram[instr]->getRelMAddrReg(),
                                                         inProgram[instr]->getRelMAddrRegComp(), inProgram[instr]->getRelMOffset(),
                                                         //  End flag
                                                         false
                                                         );
                    }

                    mad1Instr = new ShaderInstruction(
                                                     //  Opcode
                                                     MAD,
                                                     //  First operand
                                                     inProgram[instr]->getBankOp1(), inProgram[instr]->getOp1(), inProgram[instr]->getOp1NegateFlag(),
                                                     inProgram[instr]->getOp1AbsoluteFlag(), op1Components[1],
                                                     //  Second operand
                                                     inProgram[instr]->getBankOp2(), inProgram[instr]->getOp2(), inProgram[instr]->getOp2NegateFlag(),
                                                     inProgram[instr]->getOp2AbsoluteFlag(), op2Components[1],
                                                     //  Third operand
                                                     resBank, resReg, false, false, resComponentsSwizzle[0],
                                                     //  Result
                                                     resBank, resReg, false,
                                                     resComponentsMask[0],
                                                     //  Predication
                                                     inProgram[instr]->getPredicatedFlag(),
                                                     inProgram[instr]->getNegatePredicateFlag(),
                                                     inProgram[instr]->getPredicateReg(),
                                                     //  Relative addressing mode
                                                     inProgram[instr]->getRelativeModeFlag(), inProgram[instr]->getRelMAddrReg(),
                                                     inProgram[instr]->getRelMAddrRegComp(), inProgram[instr]->getRelMOffset(),
                                                     //  End flag
                                                     false
                                                     );

                    if (inProgram[instr]->getOpcode() != DP4)
                    {
                        mad2Instr = new ShaderInstruction(
                                                         //  Opcode
                                                         MAD,
                                                         //  First operand
                                                         inProgram[instr]->getBankOp1(), inProgram[instr]->getOp1(), inProgram[instr]->getOp1NegateFlag(),
                                                         inProgram[instr]->getOp1AbsoluteFlag(), op1Components[2],
                                                         //  Second operand
                                                         inProgram[instr]->getBankOp2(), inProgram[instr]->getOp2(), inProgram[instr]->getOp2NegateFlag(),
                                                         inProgram[instr]->getOp2AbsoluteFlag(), op2Components[2],
                                                         //  Third operand
                                                         resBank, resReg, false, false, resComponentsSwizzle[0],
                                                         //  Result
                                                         inProgram[instr]->getBankRes(), inProgram[instr]->getResult(), inProgram[instr]->getSaturatedRes(),
                                                         resComponentsMask[0],
                                                         //  Predication
                                                         inProgram[instr]->getPredicatedFlag(),
                                                         inProgram[instr]->getNegatePredicateFlag(),
                                                         inProgram[instr]->getPredicateReg(),
                                                         //  Relative addressing mode
                                                         inProgram[instr]->getRelativeModeFlag(), inProgram[instr]->getRelMAddrReg(),
                                                         inProgram[instr]->getRelMAddrRegComp(), inProgram[instr]->getRelMOffset(),
                                                         //  End flag
                                                         inProgram[instr]->getEndFlag() && (resComponentsMask.size() == 1)
                                                         );

                    }
                    else
                    {
                        mad2Instr = new ShaderInstruction(
                                                         //  Opcode
                                                         MAD,
                                                         //  First operand
                                                         inProgram[instr]->getBankOp1(), inProgram[instr]->getOp1(), inProgram[instr]->getOp1NegateFlag(),
                                                         inProgram[instr]->getOp1AbsoluteFlag(), op1Components[2],
                                                         //  Second operand
                                                         inProgram[instr]->getBankOp2(), inProgram[instr]->getOp2(), inProgram[instr]->getOp2NegateFlag(),
                                                         inProgram[instr]->getOp2AbsoluteFlag(), op2Components[2],
                                                         //  Third operand
                                                         resBank, resReg, false, false, resComponentsSwizzle[0],
                                                         //  Result
                                                         resBank, resReg, false,
                                                         resComponentsMask[0],
                                                         //  Predication
                                                         inProgram[instr]->getPredicatedFlag(),
                                                         inProgram[instr]->getNegatePredicateFlag(),
                                                         inProgram[instr]->getPredicateReg(),
                                                         //  Relative addressing mode
                                                         inProgram[instr]->getRelativeModeFlag(), inProgram[instr]->getRelMAddrReg(),
                                                         inProgram[instr]->getRelMAddrRegComp(), inProgram[instr]->getRelMOffset(),
                                                         //  End flag
                                                         false
                                                         );

                        mad3Instr = new ShaderInstruction(
                                                         //  Opcode
                                                         MAD,
                                                         //  First operand
                                                         inProgram[instr]->getBankOp1(), inProgram[instr]->getOp1(), inProgram[instr]->getOp1NegateFlag(),
                                                         inProgram[instr]->getOp1AbsoluteFlag(), op1Components[3],
                                                         //  Second operand
                                                         inProgram[instr]->getBankOp2(), inProgram[instr]->getOp2(), inProgram[instr]->getOp2NegateFlag(),
                                                         inProgram[instr]->getOp2AbsoluteFlag(), op2Components[3],
                                                         //  Third operand
                                                         resBank, resReg, false, false, resComponentsSwizzle[0],
                                                         //  Result
                                                         inProgram[instr]->getBankRes(), inProgram[instr]->getResult(), inProgram[instr]->getSaturatedRes(),
                                                         resComponentsMask[0],
                                                         //  Predication
                                                         inProgram[instr]->getPredicatedFlag(),
                                                         inProgram[instr]->getNegatePredicateFlag(),
                                                         inProgram[instr]->getPredicateReg(),
                                                         //  Relative addressing mode
                                                         inProgram[instr]->getRelativeModeFlag(), inProgram[instr]->getRelMAddrReg(),
                                                         inProgram[instr]->getRelMAddrRegComp(), inProgram[instr]->getRelMOffset(),
                                                         //  End flag
                                                         inProgram[instr]->getEndFlag() && (resComponentsMask.size() == 1)
                                                         );
                    }


                    if (inProgram[instr]->getOpcode() != DPH)
                        outProgram.push_back(mulInstr);
                    else
                        outProgram.push_back(mad0Instr);

                    outProgram.push_back(mad1Instr);
                    outProgram.push_back(mad2Instr);

                    if (inProgram[instr]->getOpcode() == DP4)
                        outProgram.push_back(mad3Instr);

                    //  Broadcast result to other result components if required.
                    for(u32bit resComp = 1; resComp < resComponentsMask.size(); resComp++)
                    {
                        ShaderInstruction *movInstr;

                        movInstr = new ShaderInstruction(
                                                         //  Opcode
                                                         MOV,
                                                         //  First operand
                                                         resBank, resReg, false, false,
                                                         resComponentsSwizzle[0],
                                                         //  Second operand
                                                         INVALID, 0, false, false, XYZW,
                                                         //  Third operand
                                                         INVALID, 0, false, false, XYZW,
                                                         //  Result
                                                         inProgram[instr]->getBankRes(), inProgram[instr]->getResult(), inProgram[instr]->getSaturatedRes(),
                                                         resComponentsMask[resComp],
                                                         //  Predication
                                                         inProgram[instr]->getPredicatedFlag(),
                                                         inProgram[instr]->getNegatePredicateFlag(),
                                                         inProgram[instr]->getPredicateReg(),
                                                         //  Relative addressing mode
                                                         inProgram[instr]->getRelativeModeFlag(), inProgram[instr]->getRelMAddrReg(),
                                                         inProgram[instr]->getRelMAddrRegComp(), inProgram[instr]->getRelMOffset(),
                                                         //  End flag
                                                         inProgram[instr]->getEndFlag() && (resComp == (resComponentsMask.size() - 1))
                                                         );

                        outProgram.push_back(movInstr);
                    }

                    //  If a temporal register was allocated mark it as reusable again.
                    if (inProgram[instr]->getBankRes() == OUT)
                    {
                        tempInUse[resReg] = false;
                    }

                }

                break;

            case DST:

                //  DST instruction requires special conversion.


            case EXP:

                //  EXP instruction requires special conversion.


            case LIT:

                //  LIT instruction requires special conversion.


            case LOG:

                //  LOG instruction requires special conversion.

                //  NOTE:  The implementation of some of these instructions may be complex in a SOA architecture
                //         due to limitations in the current shader ISA.  For example loading a constant value
                //         of 0.0f or 1.0f (the constant bank can not be manipulated directly) or because some
                //         functions are quite complex like the LIT function for the .z component.
                //         As a temporal solution the instructions will be treated as standard SIMD instruction
                //         and make the assumption that the ALUs implement the write of the four components
                //         in multiple cycles.

                /*
                //  Extract components for first operand.
                extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                //  Extract components for second operand.
                extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                //  Extract components for third operand.
                extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                //  Extract components for result.
                extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                //  Retrieve which operand components are actually read.
                getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                //  Convert instruction into multiple scalar instructions.  One per component of the result.
                for(u32bit resComp = 0; resComp < resComponentsMask.size(); resComp++)
                {
                    ShaderInstruction *soaInstr;

                    soaInstr = patchSOAInstruction(inProgram[instr], op1Components[resComp], op2Components[resComp],
                                                    op3Components[resComp], resComponentsMask[resComp]);

                    if (inProgram[instr]->isEnd() && (resComp == (resComponentsMask.size() - 1)))
                        soaInstr->setEndFlag(true);

                    outProgram.push_back(soaInstr);
                }*/

                //  Copy the instruction.
                copyInstr = copyInstruction(inProgram[instr]);
                outProgram.push_back(copyInstr);

                break;

            case ADD:
            case ARL:
            case CMP:
            case MAD:
            case FXMAD:
            case FXMAD2:
            case MAX:
            case MIN:
            case MOV:
            case MUL:
            case FXMUL:
            case SGE:
            case SLT:
            case DDX:
            case DDY:
            case ADDI:
            case MULI:

                //  SIMD instructions.

                //  Extract components for first operand.
                extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                //  Extract components for second operand.
                extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                //  Extract components for third operand.
                extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                //  Extract components for result.
                extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                //  Retrieve which operand components are actually read.
                getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                //  Convert instruction into multiple scalar instructions.  One per component of the result.
                for(u32bit resComp = 0; resComp < resComponentsMask.size(); resComp++)
                {
                    ShaderInstruction *soaInstr;

                    soaInstr = patchSOAInstruction(inProgram[instr], op1Components[readComponentsOp1[resComp]],
                                                   op2Components[readComponentsOp2[resComp]],
                                                   op3Components[readComponentsOp1[resComp]], resComponentsMask[resComp]);

                    if (inProgram[instr]->isEnd() && (resComp == (resComponentsMask.size() - 1)))
                        soaInstr->setEndFlag(true);

                    outProgram.push_back(soaInstr);
                }

                break;

            case EX2:
            case FRC:
            case LG2:
            case RCP:
            case RSQ:

                //  Scalar instructions.

                //  Extract components for first operand.
                extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                //  Extract components for second operand.
                extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                //  Extract components for third operand.
                extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                //  Extract components for result.
                extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                //  Retrieve which operand components are actually read.
                getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                //  The instruction is scalar.  Compute the scalar result and then broadcast.
                ShaderInstruction *soaInstr;

                soaInstr = patchSOAInstruction(inProgram[instr], op1Components[readComponentsOp1[0]],
                                               op2Components[readComponentsOp2[0]],
                                               op3Components[readComponentsOp1[0]], resComponentsMask[0]);

                outProgram.push_back(soaInstr);

                if (inProgram[instr]->isEnd() && (resComponentsMask.size() == 1))
                    soaInstr->setEndFlag(true);

                //  Broadcast result to other result components if required.
                for(u32bit resComp = 1; resComp < resComponentsMask.size(); resComp++)
                {
                    ShaderInstruction *movInstr;

                    movInstr = new ShaderInstruction(
                                                     //  Opcode
                                                     MOV,
                                                     //  First operand
                                                     inProgram[instr]->getBankRes(), inProgram[instr]->getResult(), false, false,
                                                     resComponentsSwizzle[0],
                                                     //  Second operand
                                                     INVALID, 0, false, false, XYZW,
                                                     //  Third operand
                                                     INVALID, 0, false, false, XYZW,
                                                     //  Result
                                                     inProgram[instr]->getBankRes(), inProgram[instr]->getResult(), inProgram[instr]->getSaturatedRes(),
                                                     resComponentsMask[resComp],
                                                     //  Predication
                                                     inProgram[instr]->getPredicatedFlag(),
                                                     inProgram[instr]->getNegatePredicateFlag(),
                                                     inProgram[instr]->getPredicateReg(),
                                                     //  Relative addressing mode
                                                     inProgram[instr]->getRelativeModeFlag(), inProgram[instr]->getRelMAddrReg(),
                                                     inProgram[instr]->getRelMAddrRegComp(), inProgram[instr]->getRelMOffset(),
                                                     //  End flag
                                                     inProgram[instr]->getEndFlag() && (resComp == (resComponentsMask.size() - 1))
                                                     );

                    if (inProgram[instr]->isEnd() && (resComp == (resComponentsMask.size() - 1)))
                        movInstr->setEndFlag(true);

                    outProgram.push_back(movInstr);
                }

                break;

            case FLR:

                // Unimplemented instructions.

                break;

        }
    }
}

bool ShaderOptimization::deadCodeElimination(vector<ShaderInstruction *> inProgram, u32bit namesUsed, vector<ShaderInstruction *> &outProgram)
{
    vector<RegisterState> tempRegState;

    //  Set size of the temporal register state to the number of names used.
    tempRegState.clear();
    tempRegState.resize(namesUsed + 1);

    vector<InstructionInfo> instructionInfo;

    //  Clear the state of the temporal registers.
    for(u32bit tempReg = 0; tempReg < tempRegState.size(); tempReg++)
    {
        for(u32bit comp = 0; comp < 4; comp++)
        {
            tempRegState[tempReg].compWasWritten[comp] = false;
            tempRegState[tempReg].compWasRead[comp] = false;
        }
    }

    //  Set and clear the state of the instruction database.
    instructionInfo.resize(inProgram.size());

    for(u32bit instr = 0; instr < instructionInfo.size(); instr++)
    {
        for(u32bit comp = 0; comp < 4; comp++)
            instructionInfo[instr].removeResComp[comp] = false;
    }

    //  Traverse the program to mark the instructions that can be removed.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        vector<MaskMode> resComponentsMask;
        vector<SwizzleMode> resComponentsSwizzle;
        vector<u32bit> readComponentsOp1;
        vector<u32bit> readComponentsOp2;

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case CHS:

                //  These instructions don't set or use any register.

                break;

            case FLR:

                //  Unimplemented.

                break;

            default:

                //  Only writes to temporal registers are considered for removing unnecesary instructions.
                //  The usage of the output registers is not known and for now the usage of the address
                //  registers is not taken into account.

                //  Extract components for result.
                extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                //  Retrieve which operand components are actually read.
                getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                //  Check if the first operand is active and a temporal register is being read.
                if ((inProgram[instr]->getNumOperands() > 0) && (inProgram[instr]->getBankOp1() == TEMP))
                {
                    vector <SwizzleMode> op1Components;

                    //  Extract components for first operand.
                    extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                    u32bit op1Reg = inProgram[instr]->getOp1();

                    //  Set register component usage flag.  Only the components of the swizzled operand
                    //  that are actually read.
                    for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp1[comp];

                        u32bit op1RegComp = op1Components[readComp] & 0x03;

                        //  Set register component as used.
                        tempRegState[op1Reg].compWasRead[op1RegComp] = true;
                    }
                }

                //  Check if the second operand is active and a temporal register is being read.
                if ((inProgram[instr]->getNumOperands() > 1) && (inProgram[instr]->getBankOp2() == TEMP))
                {
                    vector <SwizzleMode> op2Components;

                    //  Extract components for second operand.
                    extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                    u32bit op2Reg = inProgram[instr]->getOp2();

                    //  Set register component usage flag.  Only the components of the swizzled operand
                    //  that are actually read.
                    for(u32bit comp = 0; comp < readComponentsOp2.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp2[comp];

                        u32bit op2RegComp = op2Components[readComp] & 0x03;

                        //  Set register component as used.
                        tempRegState[op2Reg].compWasRead[op2RegComp] = true;
                    }
                }

                //  Check if the first operand is active and a temporal register is being read.
                if ((inProgram[instr]->getNumOperands() == 3) && (inProgram[instr]->getBankOp3() == TEMP))
                {
                    vector <SwizzleMode> op3Components;

                    //  Extract components for first operand.
                    extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                    u32bit op3Reg = inProgram[instr]->getOp3();

                    //  Set register component usage flag.  Only the components of the swizzled operand
                    //  that are actually read.  The components read are the same than those read for the
                    //  first operand (MAD instruction).
                    for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp1[comp];

                        u32bit op3RegComp = op3Components[readComp] & 0x03;

                        //  Set register component as used.
                        tempRegState[op3Reg].compWasRead[op3RegComp] = true;
                    }
                }

                //  Check if the result is written into a temporal register.
                //  The KIL, KLS, ZXP and ZXS instructions don't write a register.
                if ((inProgram[instr]->getBankRes() == TEMP) && inProgram[instr]->hasResult())
                {
                    u32bit resReg = inProgram[instr]->getResult();

                    for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                    {
                        //  Extract component index.
                        u32bit resRegComp = resComponentsSwizzle[comp] & 0x03;

                        //  Check if the register component was already written but not read.
                        if (tempRegState[resReg].compWasWritten[resRegComp] && !tempRegState[resReg].compWasRead[resRegComp])
                        {
                            //  Set the result component of the instruction that set the value as to be removed if
                            //  the instruction is not predicated.
                            instructionInfo[tempRegState[resReg].compWriteInstruction[resRegComp]].removeResComp[resRegComp] = !inProgram[instr]->getPredicatedFlag();
                        }

                        //  Set the register component as written.  Set instruction that create the value in the register.
                        //  For predicated instructions is the previous value was read keep the component as read.
                        tempRegState[resReg].compWasWritten[resRegComp] = true;
                        tempRegState[resReg].compWasRead[resRegComp] = (inProgram[instr]->getPredicatedFlag() && tempRegState[resReg].compWasRead[resRegComp]);
                        tempRegState[resReg].compWriteInstruction[resRegComp] = instr;
                    }
                }

                break;
        }
    }

    //  Mark final results that are not used.
    for(u32bit tempReg = 0; tempReg < tempRegState.size(); tempReg++)
    {
        for(u32bit comp = 0; comp < 4; comp++)
        {
            //  Check if the register component was written but not read
            if (tempRegState[tempReg].compWasWritten[comp] && !tempRegState[tempReg].compWasRead[comp])
            {
                //  Set the result component of the instruction that wrote the register as to be removed.
                instructionInfo[tempRegState[tempReg].compWriteInstruction[comp]].removeResComp[comp] = true;
            }
        }
    }

    bool instrComponentsRemoved = false;

    //  Regenerate the program removing result components and instructions.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        vector<SwizzleMode> resComponentsSwizzle;

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case KIL:
            case KLS:
            case ZXP:
            case ZXS:
            case CHS:
            case JMP:

                //  These instruction don't set any result so they can't be removed.

                {
                    ShaderInstruction *copyInstr;

                    copyInstr = copyInstruction(inProgram[instr]);

                    outProgram.push_back(copyInstr);
                }

                break;

            case CMPKIL:

                //  This instruction is special, as it sets a result but can´t be removed if
                //  the result is no later used, because it in addition sets the kill flag.

                {
                    ShaderInstruction *copyInstr;

                    copyInstr = copyInstruction(inProgram[instr]);

                    outProgram.push_back(copyInstr);
                }

                break;

            case FLR:

                //  Unimplemented.

                break;

            default:

                {
                    MaskMode resultMask;

                    resultMask = removeComponentsFromWriteMask(inProgram[instr]->getResultMaskMode(), instructionInfo[instr].removeResComp);

                    //  Check if result components were removed.
                    instrComponentsRemoved = instrComponentsRemoved || (resultMask != inProgram[instr]->getResultMaskMode());

                    if (resultMask != NNNN)
                    {
                        ShaderInstruction *patchedResMaskInstr;

                        patchedResMaskInstr = patchResMaskInstruction(inProgram[instr], resultMask);

                        outProgram.push_back(patchedResMaskInstr);
                    }
                    else
                    {
                        //  Check if we are removing the instruction marked with the end flag.
                        if (inProgram[instr]->isEnd())
                        {
                            //  Check that the out program is not empty.
                            if (outProgram.size() > 0)
                            {
                                //  Mark the last instruction added to the program with end flag.
                                outProgram.back()->setEndFlag(true);
                            }
                            else
                            {
                                printf("WARNING: The whole program was eliminated!!\n");
                            }
                        }
                    }

                }

                break;

        }
    }

    return instrComponentsRemoved;
}

u32bit ShaderOptimization::renameRegisters(vector<ShaderInstruction *> inProgram, vector<ShaderInstruction *> &outProgram, bool AOStoSOA)
{
    u32bit registerName[MAX_TEMPORAL_REGISTERS][4];

    u32bit nextRegisterName = 1;

    for(u32bit tempReg = 0; tempReg < MAX_TEMPORAL_REGISTERS; tempReg++)
    {
        for(u32bit comp = 0; comp < 4; comp++)
            registerName[tempReg][comp] = 0;
    }

    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        ShaderInstruction *renamedInstr;
        ShaderInstruction *copyInstr;

        vector<MaskMode> resComponentsMask;
        vector<SwizzleMode> resComponentsSwizzle;

        vector <u32bit> readComponentsOp1;
        vector <u32bit> readComponentsOp2;

        u32bit resName;
        u32bit op1Name;
        u32bit op2Name;
        u32bit op3Name;

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case CHS:

                //  Those instruction don't set or use registers.

                //  Copy the instruction.
                copyInstr = copyInstruction(inProgram[instr]);

                outProgram.push_back(copyInstr);

                break;

            case FLR:

                //  Unimplemented.

                break;

            default:

                //  Extract components for result.
                extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                //  Retrieve which operand components are actually read.
                getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                //  Check if the first operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() >= 1) && (inProgram[instr]->getBankOp1() == TEMP))
                {
                    vector <SwizzleMode> op1Components;

                    //  Extract components for first operand.
                    extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                    op1Name = 0;

                    //  Get the most recent name based on the operand components that are read.
                    for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp1[comp];

                        u32bit op1RegComp = op1Components[readComp] & 0x03;

                        //  The renamed name for the register component.
                        u32bit op1CompName = registerName[inProgram[instr]->getOp1()][op1RegComp];

                        //  Get the most recent name of all components.
                        if (op1Name < op1CompName)
                            op1Name = op1CompName;
                    }

                    //  Check if the register was actually renamed.
                    if (op1Name == 0)
                    {
                        //  Check for special case.  SLT/SGE can be used to set a register to zero or one.
                        if (((inProgram[instr]->getOpcode() == SLT) || (inProgram[instr]->getOpcode() == SGE)) &&
                            (inProgram[instr]->getBankOp1() == TEMP) && (inProgram[instr]->getBankOp2() == TEMP) &&
                            (inProgram[instr]->getOp1SwizzleMode() == inProgram[instr]->getOp2SwizzleMode()) &&
                            (inProgram[instr]->getOp1NegateFlag() == inProgram[instr]->getOp2NegateFlag()) &&
                            (inProgram[instr]->getOp1AbsoluteFlag() == inProgram[instr]->getOp2AbsoluteFlag()))
                        {
                            //  Setting result to constant value 0.0f or 1.0f.
                        }
                        else
                        {
                            printf("At instruction %d temporal register %d was used with no assigned value.\n", instr,
                                inProgram[instr]->getOp1());
                        }
                    }
                }
                else
                {
                    //  Input, constant, address registers and sampler identifiers are not renamed.
                    op1Name = inProgram[instr]->getOp1();
                }

                //  Check if the second operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() >= 2) && (inProgram[instr]->getBankOp2() == TEMP))
                {
                    vector <SwizzleMode> op2Components;

                    //  Extract components for second operand.
                    extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                    op2Name = 0;

                    //  Get the most recent name based on the operand components that are read.
                    for(u32bit comp = 0; comp < readComponentsOp2.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp2[comp];

                        u32bit op2RegComp = op2Components[readComp] & 0x03;

                        //  The renamed name for the register component.
                        u32bit op2CompName = registerName[inProgram[instr]->getOp2()][op2RegComp];

                        //  Get the most recent name of all components.
                        if (op2Name < op2CompName)
                            op2Name = op2CompName;
                    }

                    //  Check if the register was actually renamed.
                    if (op2Name == 0)
                    {
                        //  Check for special case.  SLT/SGE can be used to set a register to zero or one.
                        if (((inProgram[instr]->getOpcode() == SLT) || (inProgram[instr]->getOpcode() == SGE)) &&
                            (inProgram[instr]->getBankOp1() == TEMP) && (inProgram[instr]->getBankOp2() == TEMP) &&
                            (inProgram[instr]->getOp1SwizzleMode() == inProgram[instr]->getOp2SwizzleMode()) &&
                            (inProgram[instr]->getOp1NegateFlag() == inProgram[instr]->getOp2NegateFlag()) &&
                            (inProgram[instr]->getOp1AbsoluteFlag() == inProgram[instr]->getOp2AbsoluteFlag()))
                        {
                            //  Setting result to constant value 0.0f or 1.0f.
                        }
                        else
                        {
                            printf("At instruction %d temporal register %d was used with no assigned value.\n", instr,
                                inProgram[instr]->getOp2());
                        }
                    }
                }
                else
                {
                    //  Input, constant, address registers and sampler identifiers are not renamed.
                    op2Name = inProgram[instr]->getOp2();
                }

                //  Check if the third operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() == 3) && (inProgram[instr]->getBankOp3() == TEMP))
                {
                    vector <SwizzleMode> op3Components;

                    //  Extract components for third operand.
                    extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                    op3Name = 0;

                    //  Get the most recent name based on the operand components that are read.
                    //  The order of the operands actually read for the third operand is the same than for the first operand.
                    for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp1[comp];

                        u32bit op3RegComp = op3Components[readComp] & 0x03;

                        //  The renamed name for the register component.
                        u32bit op3CompName = registerName[inProgram[instr]->getOp3()][op3RegComp];

                        //  Get the most recent name of all components.
                        if (op3Name < op3CompName)
                            op3Name = op3CompName;
                    }

                    //  Check if the register was actually renamed.
                    if (op3Name == 0)
                    {
                        printf("At instruction %d temporal register %d was used with no assigned value.\n", instr,
                            inProgram[instr]->getOp3());
                    }
                }
                else
                {
                    //  Input, constant, address registers and sampler identifiers are not renamed.
                    op3Name = inProgram[instr]->getOp3();
                }

                //  Check if the result register is a temporal register.
                //  Discard instructions that don't write a result.
                if ((inProgram[instr]->getBankRes() == TEMP) && inProgram[instr]->hasResult())
                {
                    //  Extract components for result.
                    extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                    //  If some of the result components were not written copy those components from the previous name.
                    if (resComponentsSwizzle.size() != 4)
                    {
                        bool writtenComponents[4];

                        //  Reset the components to remove.
                        for(u32bit comp = 0; comp < 4; comp++)
                            writtenComponents[comp] = false;

                        //  Set the components to remove based on the components written by the instruction.
                        //  For predicated instructions we don't know which component are actually written.
                        for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                            writtenComponents[resComponentsSwizzle[comp] & 0x03] = !inProgram[instr]->getPredicatedFlag();

                        resName = 0;

                        //  Get most recent name for the result register from all it's components.
                        for(u32bit comp = 0; comp < 4; comp++)
                        {
                            u32bit resNameComp = registerName[inProgram[instr]->getResult()][comp];

                            //  Get the most recent name but only from the components that are not written.
                            if ((resName < resNameComp) && !writtenComponents[comp])
                                resName = resNameComp;
                        }


                        //  If none of the components was actually written before there is no need to
                        //  copy the component data.
                        if (resName != 0)
                        {
                            MaskMode copyComponentsMask;

                            //  Check if the input program is in SOA format.
                            if (AOStoSOA)
                            {
                                for(u32bit comp = 0; comp < 4; comp++)
                                {
                                    //  Get the most recent name for the register component.
                                    //u32bit resNameComp = registerName[inProgram[instr]->getResult()][comp];

                                    //  Check if the instructions was not written but was defined.
                                    if (!writtenComponents[comp])
                                    {
                                        //  Set component to copy.
                                        switch(comp)
                                        {
                                            case 0 :
                                                copyComponentsMask = XNNN;
                                                break;
                                            case 1 :
                                                copyComponentsMask = NYNN;
                                                break;
                                            case 2 :
                                                copyComponentsMask = NNZN;
                                                break;
                                            case 3 :
                                                copyComponentsMask = NNNW;
                                                break;
                                        }

                                        ShaderInstruction *movInstr;

                                        movInstr = new ShaderInstruction(
                                                                         //  Opcode
                                                                         MOV,
                                                                         //  First operand
                                                                         TEMP, resName, false, false, XYZW,
                                                                         //  Second operand
                                                                         INVALID, 0, false, false, XYZW,
                                                                         //  Third operand
                                                                         INVALID, 0, false, false, XYZW,
                                                                         //  Result
                                                                         TEMP, nextRegisterName, false, copyComponentsMask,
                                                                         //  Predication
                                                                         false, false, 0,
                                                                         //  Relative addressing mode
                                                                         false, 0, 0, 0,
                                                                         //  End flag
                                                                         false
                                                                         );

                                        outProgram.push_back(movInstr);
                                    }
                                }
                            }
                            else
                            {
                                //  Compute the result mask for components not written by the instruction (remove written components).
                                copyComponentsMask = removeComponentsFromWriteMask(mXYZW, writtenComponents);

                                ShaderInstruction *movInstr;

                                movInstr = new ShaderInstruction(
                                                                 //  Opcode
                                                                 MOV,
                                                                 //  First operand
                                                                 TEMP, resName, false, false, XYZW,
                                                                 //  Second operand
                                                                 INVALID, 0, false, false, XYZW,
                                                                 //  Third operand
                                                                 INVALID, 0, false, false, XYZW,
                                                                 //  Result
                                                                 TEMP, nextRegisterName, false, copyComponentsMask,
                                                                 //  Predication
                                                                 false, false, 0,
                                                                 //  Relative addressing mode
                                                                 false, 0, 0, 0,
                                                                 //  End flag
                                                                 false
                                                                 );

                                outProgram.push_back(movInstr);
                            }
                        }
                    }

                    //  Associate original register and component with the new name.  All the components
                    //  of the result register get the new name because we are copying in the components not written
                    //  the value of the components of the previous most recent name.
                    for(u32bit comp = 0; comp < 4; comp++)
                    {
                        registerName[inProgram[instr]->getResult()][comp] = nextRegisterName;
                    }

                    //  Set the new name for the result register.
                    resName = nextRegisterName;

                    //  Update counter used to assign new names to the registers.
                    nextRegisterName++;

                }
                else
                {
                    //  Get the result register name.  Output and address registers are not renamed.
                    resName = inProgram[instr]->getResult();
                }

                //  Copy the shader instruction renaming the temporal registers.
                renamedInstr = renameRegsInstruction(inProgram[instr], resName, op1Name, op2Name, op3Name);

                //  Add renamed instruction to the program.
                outProgram.push_back(renamedInstr);


                break;
        }
    }

    //  Return how many 'register' names have been used for the program.
    return (nextRegisterName - 1);
}

u32bit ShaderOptimization::reduceLiveRegisters(vector<ShaderInstruction *> inProgram, u32bit names, vector<ShaderInstruction *> &outProgram)
{
    vector<NameUsage> nameUsage;

    u32bit nextFreeReg = 0;

    //  Resize table storing name usage to the number of names used by the program.
    nameUsage.resize(names + 1);

    //  Clear name usage table.
    for(u32bit name = 0; name < (names + 1); name++)
    {
        u32bit numInstructions = (u32bit) inProgram.size();

        nameUsage[name].allocated = false;

        for(u32bit comp = 0; comp < 4; comp++)
        {
            nameUsage[name].createdByInstr[comp] = 0;
            nameUsage[name].usedByInstrOp1[comp].resize(numInstructions);
            nameUsage[name].usedByInstrOp2[comp].resize(numInstructions);
            nameUsage[name].usedByInstrOp3[comp].resize(numInstructions);
            nameUsage[name].lastUsedByInstr[comp] = 0;

            for(u32bit instr = 0; instr < numInstructions; instr++)
            {
                nameUsage[name].usedByInstrOp1[comp][instr] = false;
                nameUsage[name].usedByInstrOp2[comp][instr] = false;
                nameUsage[name].usedByInstrOp3[comp][instr] = false;
            }

            nameUsage[name].copiedFromInstr[comp] = 0;
            nameUsage[name].copiedRegister[comp] = 0;
            nameUsage[name].copiedComponent[comp] = 0;
            nameUsage[name].copiedByInstr[comp].clear();
            nameUsage[name].copies[comp].clear();

            nameUsage[name].allocReg[comp] = -1;
            nameUsage[name].allocComp[comp] = -1;
        }

        nameUsage[name].masterName = 0;

        nameUsage[name].maxPackedCompUse = 4;  //  Used to force register aggregation!!!
        nameUsage[name].instrPackedCompUse.resize(numInstructions);

        for(u32bit instr = 0; instr < numInstructions; instr++)
            nameUsage[name].instrPackedCompUse[instr] = 0;
    }

    //  Analyze register usage in the program.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        vector<MaskMode> resComponentsMask;
        vector<SwizzleMode> resComponentsSwizzle;

        vector <u32bit> readComponentsOp1;
        vector <u32bit> readComponentsOp2;

        bool resultIsACopy = false;

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case CHS:

                //  Those instruction don't set or use registers.

                break;

            case FLR:

                //  Unimplemented.

                break;

            default:

                //  Extract components for result.
                extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                //  Retrieve which operand components are actually read.
                getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                //  Check if the first operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() >= 1) && (inProgram[instr]->getBankOp1() == TEMP))
                {
                    vector <SwizzleMode> op1Components;

                    //  Extract components for first operand.
                    extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                    //  Get the name for the first operand.
                    u32bit op1Name = inProgram[instr]->getOp1();

                    //  Set usage for all the components actually used of the first operand.
                    for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp1[comp];

                        u32bit op1RegComp = op1Components[readComp] & 0x03;

                        //  Set name component as used by the current instruction.
                        nameUsage[op1Name].usedByInstrOp1[op1RegComp][instr] = true;

                        //  Set name component as last used by the current instruction (analysis is performed in program order).
                        nameUsage[op1Name].lastUsedByInstr[op1RegComp] = instr + 1;
                    }

                    //  Compute how many components of the are used packed in the instruction.
                    u32bit packedCompUse = 0;
                    for(u32bit comp = 0; comp < 4; comp++)
                        if (nameUsage[op1Name].usedByInstrOp1[comp][instr])
                            packedCompUse++;

                    //  Set the maximum packed component use for the name.
                    nameUsage[op1Name].maxPackedCompUse = (nameUsage[op1Name].maxPackedCompUse < packedCompUse) ?
                                                          packedCompUse : nameUsage[op1Name].maxPackedCompUse;

                    //  Set the packed component use for the name at the current instruction.
                    nameUsage[op1Name].instrPackedCompUse[instr] = (nameUsage[op1Name].instrPackedCompUse[instr] < packedCompUse) ?
                                                                   packedCompUse : nameUsage[op1Name].instrPackedCompUse[instr];

                    //  Add copy information for MOV instructions without result or source modifiers.
                    if ((inProgram[instr]->getOpcode() == MOV) && (!inProgram[instr]->getSaturatedRes()) &&
                        (!inProgram[instr]->getOp1AbsoluteFlag()) && (!inProgram[instr]->getOp1NegateFlag()))
                    {
                        //  Get result register bank.
                        Bank resBank = inProgram[instr]->getBankRes();

                        //  Get result register.
                        u32bit resReg = inProgram[instr]->getResult();

                        if (resBank == TEMP)
                        {
                            //  Track register copies.

                            //  Set copies.
                            for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                            {
                                //  Get the actual input component (swizzled) to be written over the output component.
                                u32bit readComp = readComponentsOp1[comp];
                                u32bit op1RegComp = op1Components[readComp] & 0x03;

                                //  Get the result component being written.
                                u32bit resComp = resComponentsSwizzle[comp] & 0x03;

                                RegisterComponent regComp;

                                regComp.reg = resReg;
                                regComp.comp = resComp;

                                //  Register the copy.
                                nameUsage[resReg].copiedFromInstr[resComp] = nameUsage[op1Name].createdByInstr[op1RegComp];
                                nameUsage[resReg].copiedRegister[resComp] = op1Name;
                                nameUsage[resReg].copiedComponent[resComp] = op1RegComp;
                                nameUsage[op1Name].copiedByInstr[op1RegComp].push_back(instr + 1);
                                nameUsage[op1Name].copies[op1RegComp].push_back(regComp);
                            }

                            //  Result is a copy.  Don't update the name usage table.
                            resultIsACopy = true;
                        }
                    }
                }

                //  Check if the second operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() >= 2) && (inProgram[instr]->getBankOp2() == TEMP))
                {
                    vector <SwizzleMode> op2Components;

                    //  Extract components for second operand.
                    extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                    //  Get the name for the second operand.
                    u32bit op2Name = inProgram[instr]->getOp2();

                    //  Set usage for all the components actually used of the second operand.
                    for(u32bit comp = 0; comp < readComponentsOp2.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp2[comp];

                        u32bit op2RegComp = op2Components[readComp] & 0x03;

                        //  Set name component as used by the current instruction.
                        nameUsage[op2Name].usedByInstrOp2[op2RegComp][instr] = true;

                        //  Set name component as last used by the current instruction (analysis is performed in program order).
                        nameUsage[op2Name].lastUsedByInstr[op2RegComp] = instr + 1;
                    }

                    //  Compute how many components of the are used packed in the instruction.
                    u32bit packedCompUse = 0;
                    for(u32bit comp = 0; comp < 4; comp++)
                        if (nameUsage[op2Name].usedByInstrOp2[comp][instr])
                            packedCompUse++;

                    //  Set the maximum packed component use for the name.
                    nameUsage[op2Name].maxPackedCompUse = (nameUsage[op2Name].maxPackedCompUse < packedCompUse) ?
                                                          packedCompUse : nameUsage[op2Name].maxPackedCompUse;

                    //  Set the packed component use for the name at the current instruction.
                    nameUsage[op2Name].instrPackedCompUse[instr] = (nameUsage[op2Name].instrPackedCompUse[instr] < packedCompUse) ?
                                                                   packedCompUse : nameUsage[op2Name].instrPackedCompUse[instr];
                }

                //  Check if the third operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() == 3) && (inProgram[instr]->getBankOp3() == TEMP))
                {
                    vector <SwizzleMode> op3Components;

                    //  Extract components for third operand.
                    extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                    //  Get the name for the third operand.
                    u32bit op3Name = inProgram[instr]->getOp3();

                    //  Set usage for all the components actually used of the third operand.
                    //  The components actually used for the third operand are the same that the
                    //  components used for the first operand.
                    for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                    {
                        u32bit readComp = readComponentsOp1[comp];

                        u32bit op3RegComp = op3Components[readComp] & 0x03;

                        //  Set name component as used by the current instruction.
                        nameUsage[op3Name].usedByInstrOp3[op3RegComp][instr] = true;

                        //  Set name component as last used by the current instruction (analysis is performed in program order).
                        nameUsage[op3Name].lastUsedByInstr[op3RegComp] = instr + 1;
                    }

                    //  Compute how many components of the are used packed in the instruction.
                    u32bit packedCompUse = 0;
                    for(u32bit comp = 0; comp < 4; comp++)
                        if (nameUsage[op3Name].usedByInstrOp3[comp][instr])
                            packedCompUse++;

                    //  Set the maximum packed component use for the name.
                    nameUsage[op3Name].maxPackedCompUse = (nameUsage[op3Name].maxPackedCompUse < packedCompUse) ?
                                                          packedCompUse : nameUsage[op3Name].maxPackedCompUse;

                    //  Set the packed component use for the name at the current instruction.
                    nameUsage[op3Name].instrPackedCompUse[instr] = (nameUsage[op3Name].instrPackedCompUse[instr] < packedCompUse) ?
                                                                   packedCompUse : nameUsage[op3Name].instrPackedCompUse[instr];
                }

                //  Check if the result register is a temporal register.
                //  Discard KIL, KLS, ZXP and ZXS instructions as they don't write a result.
                if ((inProgram[instr]->getBankRes() == TEMP) && (inProgram[instr]->getOpcode() != KIL) &&
                    (inProgram[instr]->getOpcode() != KLS) && (inProgram[instr]->getOpcode() != ZXP) &&
                    (inProgram[instr]->getOpcode() != ZXS))
                {
                    //  Extract components for result.
                    extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                    //  Get the result register name.
                    u32bit resName = inProgram[instr]->getResult();

                    //  Set creation instruction for all the name components.
                    for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                    {
                        u32bit resComp = resComponentsSwizzle[comp] & 0x03;

                        //  Check that the name component was not already created.
                        if (nameUsage[resName].createdByInstr[resComp] != 0)
                        {
                            //  Check if the current instruction is predicated.
                            if (!inProgram[instr]->getPredicatedFlag())
                            {
                                char buffer[256];
                                sprintf(buffer, "Name %d component %d was already created at instruction %d.  Current instruction is %d.", resName,
                                    resComp, nameUsage[resName].createdByInstr[resComp] - 1, instr);
                                panic("ShaderOptimization", "reduceLiveRegisters", buffer);
                            }
                        }

                        //  Set name component as created by the current instruction.
                        nameUsage[resName].createdByInstr[resComp] = instr + 1;

                        //  Check if the result is a copy from another name (MOV instruction).
                        if (!resultIsACopy)
                        {
                            //  The current value isn't a copy.
                            nameUsage[resName].copiedFromInstr[resComp] = 0;
                            nameUsage[resName].copiedRegister[resComp] = 0;
                            nameUsage[resName].copiedComponent[resComp] = 0;

                            //  The current value doesn't have copies.
                            nameUsage[resName].copiedByInstr[resComp].clear();
                            nameUsage[resName].copies[resComp].clear();
                        }
                    }

                    //  Some shader instructions produce a SIMD4 result (not a vector result) that prevents result component
                    //  renaming because the result register doesn't allows swizzling.  For those instructions the full
                    //  temporal register must be allocated for the name.  To this effect the packed usage is forced to 4
                    //  and a fake use for all four components is added in the same instruction.
                    bool forcePackedUseTo4 = hasSIMD4Result(inProgram[instr]->getOpcode());

                    //  Set fake first use for all components.
                    if (forcePackedUseTo4)
                    {
                        for(u32bit comp = 0; comp < 4; comp++)
                        {
                            nameUsage[resName].createdByInstr[comp] = instr + 1;
                            nameUsage[resName].lastUsedByInstr[comp] = instr + 1;
                        }
                    }

                    //  Force max packed component use to 4 if required.
                    u32bit packedWrite = forcePackedUseTo4 ? 4 : (u32bit) resComponentsSwizzle.size();

                    //  Set the maximum packed component use for the name.
                    nameUsage[resName].maxPackedCompUse = (nameUsage[resName].maxPackedCompUse < packedWrite) ?
                                                           packedWrite : nameUsage[resName].maxPackedCompUse;
                }

                break;
        }
    }


    //  Aggregate compatible names related by copies.
    for(u32bit name = 1; name < (names + 1); name++)
    {
//printf("---------------------------------------\n");
//printf("Evaluating name %d for aggregation\n", name);
        //  Evaluate all the name components.
        for(u32bit comp = 0; comp < 4; comp++)
        {
            //  Get master for this name.  If the name has no master the master name is
            //  the current name.
            u32bit masterName = nameUsage[name].masterName;
            masterName = (masterName == 0) ? name : masterName;

            //  Evaluate if the copies of the name are compatible
            for(u32bit copy = 0; copy < nameUsage[name].copies[comp].size(); copy++)
            {
//if (copy == 0)
//printf(" -> Master name is %d\n", masterName);
                //  Get the copy name and component.
                u32bit copyName = nameUsage[name].copies[comp][copy].reg;

//printf(" -> Evaluating copy %d = (reg %d comp %d)\n", copy, nameUsage[name].copies[comp][copy].reg, nameUsage[name].copies[comp][copy].comp);
                //  Check if the name wasn't already aggregated.
                if (nameUsage[copyName].masterName == 0)
                {
                    bool aggregateName = true;

                    //  Evaluate if all the components of the copy name can be aggregated to the
                    //  master name.
                    for(u32bit copyComp = 0; copyComp < 4; copyComp++)
                    {
                        //  Evaluate if the component in they copy name is free.
                        bool copyComponentIsFree = (nameUsage[copyName].createdByInstr[copyComp] == 0);
                        
                        //  Evaluate if the component in the copy name is a copy from the same master (or master master) component.
                        bool copyCompIsCopyFromMasterComp = (((nameUsage[copyName].copiedRegister[copyComp] == masterName) ||
                                                              (nameUsage[nameUsage[copyName].copiedRegister[copyComp]].masterName == masterName)) &&
                                                             (nameUsage[copyName].copiedComponent[copyComp] == copyComp));

                        //  Evaluate if the same component in the master name is free before the component in the copy gets a value.
                        bool masterCompIsFree = (nameUsage[copyName].createdByInstr[copyComp] >= nameUsage[masterName].lastUsedByInstr[copyComp]);
           
//printf(" -> Evaluate copy comp %d | created by instr %d | copy from (%d, %d) | copy created by instr %d | master comp last used by instr %d | aggregate? %s\n",
//copyComp, nameUsage[copyName].createdByInstr[copyComp],
//nameUsage[copyName].copiedRegister[copyComp],
//nameUsage[copyName].copiedComponent[copyComp],
//nameUsage[copyName].createdByInstr[copyComp],
//nameUsage[masterName].lastUsedByInstr[copyComp],
//aggregateName ? "T" : "F");

                        //  Evalute if the copy component can be aggregated to the same component in the master.
                        aggregateName = aggregateName && (copyComponentIsFree || copyCompIsCopyFromMasterComp || masterCompIsFree);

                        //  Evaluate if a component of the copy name can be aggregated.
                        //  A component of the copy name can be aggregated to the master name if:
                        //
                        //   - The component was not created
                        //       or
                        //   - The component is a copy of the same master component
                        //       or
                        //   - The master component is the same that the copy component and it was used for the
                        //     last time before the copy component creation
                        //
                        /*aggregateName = aggregateName &&
                                        ((nameUsage[copyName].createdByInstr[copyComp] == 0) ||
                                         (((nameUsage[copyName].copiedRegister[copyComp] == masterName) ||
                                           (nameUsage[nameUsage[copyName].copiedRegister[copyComp]].masterName == masterName)) &&
                                          (masterComp == copyComp)) ||
                                         ((nameUsage[copyName].createdByInstr[copyComp] >= nameUsage[masterName].lastUsedByInstr[masterComp]) &&
                                          (masterComp == copyComp)));*/
//printf(" -> Evaluate copy comp %d | created by instr %d | copy from (%d, %d) | copy source master %d | copy created by instr %d | master comp last used by instr %d | aggregate? %s\n",
//copyComp, nameUsage[copyName].createdByInstr[copyComp],
//nameUsage[copyName].copiedRegister[copyComp], masterComp,
//nameUsage[nameUsage[copyName].copiedRegister[copyComp]].masterName,
//nameUsage[copyName].createdByInstr[copyComp],
//nameUsage[masterName].lastUsedByInstr[masterComp],
//aggregateName ? "T" : "F");
                    }

                    //  Check if the copy name can be aggregated
                    if (aggregateName)
                    {
                        //  Set the master name for the copy name.
                        nameUsage[copyName].masterName = masterName;

                        //  Update usage information for the master name.
                        for(u32bit copyComp = 0; copyComp < 4; copyComp++)
                        {
                            //
                            // NOTE : Translation no longer required due to the restriction that copy components and master components
                            //        must be the same (see above).
                            //
                            //  Translate copy component into the corresponing master component.
                            //u32bit masterComp = nameUsage[copyName].copiedComponent[copyComp];
                            u32bit masterComp = copyComp;

//printf(" -> Updating master creation/usage information => copy comp %d | master comp %d | copy created by instr %d | copy last used by instr %d\n",
//copyComp, masterComp, nameUsage[copyName].createdByInstr[copyComp], nameUsage[copyName].lastUsedByInstr[copyComp]);

                            //  Set the master name component (first) creation if the name component was not
                            //  created.
                            if (nameUsage[masterName].createdByInstr[copyComp] == 0)
                                nameUsage[masterName].createdByInstr[copyComp] = nameUsage[copyName].createdByInstr[copyComp];
                                
                            //  Update the last use of the master component with the copy component last use.
                            if ((nameUsage[copyName].lastUsedByInstr[copyComp] != 0) &&
                                (nameUsage[copyName].lastUsedByInstr[copyComp] > nameUsage[masterName].lastUsedByInstr[copyComp]))
                                nameUsage[masterName].lastUsedByInstr[copyComp] = nameUsage[copyName].lastUsedByInstr[copyComp];
                            
                            //  Update packed usage for the master name.
                            if (nameUsage[masterName].maxPackedCompUse < nameUsage[copyName].maxPackedCompUse)
                                nameUsage[masterName].maxPackedCompUse = nameUsage[copyName].maxPackedCompUse;

                            //  Update per instruction information in the master name.
                            for(u32bit instr = 0; instr < inProgram.size(); instr++)
                            {
                                //  Update operands accessed.
                                if (nameUsage[copyName].usedByInstrOp1[copyComp][instr])
                                {
                                    GPU_ASSERT(
                                        if (nameUsage[masterName].usedByInstrOp1[copyComp][instr])
                                        {
                                            char buffer[256];
                                            sprintf(buffer, "Aggregating name %d to master name %d but master comp %d already used for operand 1 at instr %d\n",
                                                copyName, masterName, copyComp, instr);
                                            panic("ShaderOptimization", "reduceLiveRegisters", buffer);
                                        }
                                    )

                                    nameUsage[masterName].usedByInstrOp1[copyComp][instr] = true;
                                }


                                if (nameUsage[copyName].usedByInstrOp2[copyComp][instr])
                                {
                                    GPU_ASSERT(
                                        if (nameUsage[masterName].usedByInstrOp2[copyComp][instr])
                                        {
                                            char buffer[256];
                                            sprintf(buffer, "Aggregating name %d to master name %d but master comp %d already used for operand 2 at instr %d\n",
                                                copyName, masterName, copyComp, instr);
                                            panic("ShaderOptimization", "reduceLiveRegisters", buffer);
                                        }
                                    )

                                    nameUsage[masterName].usedByInstrOp2[copyComp][instr] = true;
                                }

                                if (nameUsage[copyName].usedByInstrOp3[copyComp][instr])
                                {
                                    GPU_ASSERT(
                                        if (nameUsage[masterName].usedByInstrOp3[copyComp][instr])
                                        {
                                            char buffer[256];
                                            sprintf(buffer, "Aggregating name %d to master name %d but master comp %d already used for operand 3 at instr %d\n",
                                                copyName, masterName, copyComp, instr);
                                            panic("ShaderOptimization", "reduceLiveRegisters", buffer);
                                        }
                                    )

                                    nameUsage[masterName].usedByInstrOp3[masterComp][instr] = true;
                                }

                                //  Updated packed component usage.
                                if (nameUsage[masterName].instrPackedCompUse[instr] < nameUsage[copyName].instrPackedCompUse[instr])
                                    nameUsage[masterName].instrPackedCompUse[instr] = nameUsage[copyName].instrPackedCompUse[instr];
                            }
                        }
                    }
                }
            }
        }
    }


    /*for(u32bit name = 1; name < (names + 1); name++)
    {
        printf("Name %d, packed component max use is %d\n", name, nameUsage[name].maxPackedCompUse);
        printf(" Packet component usage : \n");
        for(u32bit instr = 0; instr < inProgram.size(); instr++)
            printf(" %d", nameUsage[name].instrPackedCompUse[instr]);
        printf("\n");

        if (nameUsage[name].masterName != 0)
            printf(" Master name %d\n", nameUsage[name].masterName);
        else
            printf(" The name has no master.\n");

        for(u32bit comp = 0; comp < 4; comp++)
        {
            if (nameUsage[name].createdByInstr[comp] != 0)
                printf("    Component %d was created by instruction %d (%04x)\n",
                    comp, nameUsage[name].createdByInstr[comp], (nameUsage[name].createdByInstr[comp] - 1) * 16);
            else
                printf("    Component %d was not created by any instruction\n", comp);

            if (nameUsage[name].lastUsedByInstr > 0)
            {
                printf("    Component %d was last used by instruction %d (%04x)\n", comp,
                    nameUsage[name].lastUsedByInstr[comp], (nameUsage[name].lastUsedByInstr[comp] - 1) * 16);
                printf("      Per operand usage:\n");
                printf("        Operand1 ->");
                for(u32bit instr = 0; instr < inProgram.size(); instr++)
                    printf(" %s", nameUsage[name].usedByInstrOp1[comp][instr] ? "Y" : "N");
                printf("\n");
                printf("        Operand2 ->");
                for(u32bit instr = 0; instr < inProgram.size(); instr++)
                    printf(" %s", nameUsage[name].usedByInstrOp2[comp][instr] ? "Y" : "N");
                printf("\n");
                printf("        Operand3 ->");
                for(u32bit instr = 0; instr < inProgram.size(); instr++)
                    printf(" %s", nameUsage[name].usedByInstrOp3[comp][instr] ? "Y" : "N");
                printf("\n");
            }
            else
                printf("    Component %d was not used by any instruction\n", comp);

            if (nameUsage[name].copiedFromInstr[comp] > 0)
                printf("    Value was copied from reg %d comp %d created by instruction %d (%04x)\n",
                    nameUsage[name].copiedRegister[comp], nameUsage[name].copiedComponent[comp],
                    nameUsage[name].copiedFromInstr[comp], (nameUsage[name].copiedFromInstr[comp] - 1) * 16);
            else
                printf("    Value is not a copy\n");
            if (nameUsage[name].copiedByInstr[comp].size() > 0)
            {
                printf("    Value was copied at instructions :");
                for(u32bit instr = 0; instr < nameUsage[name].copiedByInstr[comp].size(); instr++)
                    printf(" %d (%04x)", nameUsage[name].copiedByInstr[comp][instr], (nameUsage[name].copiedByInstr[comp][instr] - 1) * 16);
                printf("\n");
            }
            else
            {
                printf("    Value was not copied.\n");
            }
            if (nameUsage[name].copies[comp].size() > 0)
            {
                printf("    Copies of the value :");
                for(u32bit copy = 0; copy < nameUsage[name].copies[comp].size(); copy++)
                    printf(" (%d.%d)", nameUsage[name].copies[comp][copy].reg, nameUsage[name].copies[comp][copy].comp);
                printf("\n");
            }
            else
            {
                printf("    No copies of the value.\n");
            }
        }
    }*/

    NameComponent registerMapping[MAX_TEMPORAL_REGISTERS][4];

    //  Set all the temporal register components as free;
    for(u32bit tempReg = 0; tempReg < MAX_TEMPORAL_REGISTERS; tempReg++)
    {
        for(u32bit comp = 0; comp < 4; comp++)
        {
            registerMapping[tempReg][comp].name = 0;
            registerMapping[tempReg][comp].freeFromInstr = 0;
        }
    }


    u32bit maxLiveRegisters = 0;

    //  Allocate registers.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {

        u32bit patchedResReg;
        MaskMode patchedResMask;
        u32bit patchedOp1Reg;
        u32bit patchedOp2Reg;
        u32bit patchedOp3Reg;
        SwizzleMode patchedSwzModeOp1;
        SwizzleMode patchedSwzModeOp2;
        SwizzleMode patchedSwzModeOp3;

        u32bit currentLiveRegisters = 0;

        //  Count the number of live allocated temporal registers.
        for(u32bit tempReg = 0; tempReg < MAX_TEMPORAL_REGISTERS; tempReg++)
        {
            bool allCompsFree = true;

            for(u32bit comp = 0; comp < 4; comp++)
                allCompsFree = allCompsFree && (registerMapping[tempReg][comp].freeFromInstr <= instr);

            if (!allCompsFree)
                currentLiveRegisters++;
        }

        maxLiveRegisters = (maxLiveRegisters < currentLiveRegisters) ? currentLiveRegisters : maxLiveRegisters;

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case CHS:

                //  Those instruction don't set or use registers.

                ShaderInstruction *copyInstr;

                //  Copy the instruction to the output program.
                copyInstr = copyInstruction(inProgram[instr]);
                outProgram.push_back(copyInstr);

                break;

            case FLR:

                //  Unimplemented.

                break;

            default:

//printf(" => Instr %d\n", instr);

                //  Check if the first operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() > 0) && (inProgram[instr]->getBankOp1() == TEMP))
                {
//printf(" => Renaming first operand.\n");

                    //  Get the third operand name.
                    u32bit name = inProgram[instr]->getOp1();

//printf(" => Name = %d\n", name);

                    bool readComponents[4];

                    u32bit firstComponent = 5;

                    //  Get name components read from the operand.
                    for(u32bit comp = 0; comp < 4; comp++)
                    {
                        readComponents[comp] = nameUsage[name].usedByInstrOp1[comp][instr];

                        if ((firstComponent == 5) && (nameUsage[name].usedByInstrOp1[comp][instr]))
                            firstComponent = comp;
                    }

//printf(" => firstComponent = %d readComponents = {%d, %d, %d, %d}\n", firstComponent, readComponents[0], readComponents[1],
//readComponents[2], readComponents[3]);

                    //  Check if the name is aggregated to a master name.
                    if ((nameUsage[name].maxPackedCompUse > 1) && (nameUsage[name].masterName != 0))
                    {
                        //  Change the name to use for register allocation information to the
                        //  master name.
                        name = nameUsage[name].masterName;
                    }

//printf(" => Name = %d\n", name);

                    //  Set translated first operand register.
                    if (nameUsage[name].allocReg[firstComponent] == -1)
                    {
                        //  The first used component in the name was not defined/renamed.                        
                        //  Search for a defined/renamed component for the name.
                        u32bit c;
                        for(c = 0; (c < 4) && (nameUsage[name].allocReg[c] == -1); c++);
                        
                        if (c == 4)
                        {
                            //  None of the name components was defined/renamed!!!
                            panic("ShaderOptmization", "reduceLiveRegisters", "None of the first operand components was defined.");
                        }
                        else
                        {
                            patchedOp1Reg = nameUsage[name].allocReg[c];
                        }
                    }
                    else
                        patchedOp1Reg = nameUsage[name].allocReg[firstComponent];

//printf(" => patchedOp1Reg = %d\n", patchedOp1Reg);

                    //  Translate the swizzle mode for the operand.

                    vector <SwizzleMode> opComponents;

                    //  Extract components for operand.
                    extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), opComponents);

//printf(" => components for operand = %d %d %d %d\n", opComponents[0], opComponents[1], opComponents[2], opComponents[3]);

                    u32bit translatedSwz[4];

                    //  Translate the components in the swizzle mode.
                    for(u32bit comp = 0; comp < 4; comp++)
                    {
                        //  If the instruction is actually read rename component name.
                        if (readComponents[opComponents[comp] & 0x03])
                            translatedSwz[comp] = nameUsage[name].allocComp[opComponents[comp] & 0x03];
                        else
                            translatedSwz[comp] = opComponents[comp] & 0x03;
                    }

//printf(" => translated swizzle = %d %d %d %d\n", translatedSwz[0], translatedSwz[1], translatedSwz[2], translatedSwz[3]);

                    //  Recreate the swizzle mode for the operand.
                    if (nameUsage[name].allocReg[firstComponent] == -1)
                        patchedSwzModeOp1 = inProgram[instr]->getOp1SwizzleMode();
                    else
                        patchedSwzModeOp1 = encodeSwizzle(translatedSwz);

                }
                else
                {
                    patchedOp1Reg = inProgram[instr]->getOp1();
                    patchedSwzModeOp1 = inProgram[instr]->getOp1SwizzleMode();
                }

                //  Check if the second operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() > 1) && (inProgram[instr]->getBankOp2() == TEMP))
                {
//printf(" => Renaming second operand.\n");

                    //  Get the third operand name.
                    u32bit name = inProgram[instr]->getOp2();

//printf(" => Name = %d\n", name);

                    bool readComponents[4];

                    u32bit firstComponent = 5;

                    //  Get name components read from the operand.
                    for(u32bit comp = 0; comp < 4; comp++)
                    {
                        readComponents[comp] = nameUsage[name].usedByInstrOp2[comp][instr];

                        if ((firstComponent == 5) && (nameUsage[name].usedByInstrOp2[comp][instr]))
                            firstComponent = comp;
                    }

//printf(" => firstComponent = %d readComponents = {%d, %d, %d, %d}\n", firstComponent, readComponents[0], readComponents[1],
//readComponents[2], readComponents[3]);

                    //  Check if the name is aggregated to a master name.
                    if ((nameUsage[name].maxPackedCompUse > 1) && (nameUsage[name].masterName != 0))
                    {
                        //  Change the name to use for register allocation information to the
                        //  master name.
                        name = nameUsage[name].masterName;
                    }

//printf(" => Name = %d\n", name);

                    //  Set translated second operand register.
                    if (nameUsage[name].allocReg[firstComponent] == -1)
                    {
                        //  The first used component in the name was not defined/renamed.                        
                        //  Search for a defined/renamed component for the name.
                        u32bit c;
                        for(c = 0; (c < 4) && (nameUsage[name].allocReg[c] == -1); c++);
                        
                        if (c == 4)
                        {
                            //  None of the name components was defined/renamed!!!
                            panic("ShaderOptmization", "reduceLiveRegisters", "None of the second operand components was defined.");
                        }
                        else
                        {
                            patchedOp2Reg = nameUsage[name].allocReg[c];
                        }
                    }
                    else
                        patchedOp2Reg = nameUsage[name].allocReg[firstComponent];

//printf(" => patchedOp2Reg = %d\n", patchedOp2Reg);

                    //  Translate the swizzle mode for the operand.

                    vector <SwizzleMode> opComponents;

                    //  Extract components for operand.
                    extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), opComponents);

//printf(" => components for operand = %d %d %d %d\n", opComponents[0], opComponents[1], opComponents[2], opComponents[3]);

                    u32bit translatedSwz[4];

                    //  Translate the components in the swizzle mode.
                    for(u32bit comp = 0; comp < 4; comp++)
                    {
                        //  If the instruction is actually read rename component name.
                        if (readComponents[opComponents[comp] & 0x03])
                            translatedSwz[comp] = nameUsage[name].allocComp[opComponents[comp] & 0x03];
                        else
                            translatedSwz[comp] = opComponents[comp] & 0x03;
                    }

//printf(" => translated swizzle = %d %d %d %d\n", translatedSwz[0], translatedSwz[1], translatedSwz[2], translatedSwz[3]);

                    //  Recreate the swizzle mode for the operand.
                    if (nameUsage[name].allocReg[firstComponent] == -1)
                        patchedSwzModeOp2 = inProgram[instr]->getOp2SwizzleMode();
                    else
                        patchedSwzModeOp2 = encodeSwizzle(translatedSwz);
                }
                else
                {
                    patchedOp2Reg = inProgram[instr]->getOp2();
                    patchedSwzModeOp2 = inProgram[instr]->getOp2SwizzleMode();
                }

                //  Check if the third operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() == 3) && (inProgram[instr]->getBankOp3() == TEMP))
                {
//printf(" => Renaming third operand.\n");

                    //  Get the third operand name.
                    u32bit name = inProgram[instr]->getOp3();

//printf(" => Name = %d\n", name);

                    bool readComponents[4];

                    u32bit firstComponent = 5;

                    //  Get name components read from the operand.
                    for(u32bit comp = 0; comp < 4; comp++)
                    {
                        readComponents[comp] = nameUsage[name].usedByInstrOp3[comp][instr];

                        if ((firstComponent == 5) && (nameUsage[name].usedByInstrOp3[comp][instr]))
                            firstComponent = comp;
                    }

//printf(" => firstComponent = %d readComponents = {%d, %d, %d, %d}\n", firstComponent, readComponents[0], readComponents[1],
//readComponents[2], readComponents[3]);

                    //  Check if the name is aggregated to a master name.
                    if ((nameUsage[name].maxPackedCompUse > 1) && (nameUsage[name].masterName != 0))
                    {
                        //  Change the name to use for register allocation information to the
                        //  master name.
                        name = nameUsage[name].masterName;
                    }

//printf(" => Name = %d\n", name);

                    //  Set translated third operand register.
                    if (nameUsage[name].allocReg[firstComponent] == -1)
                    {
                        //  The first used component in the name was not defined/renamed.                        
                        //  Search for a defined/renamed component for the name.
                        u32bit c;
                        for(c = 0; (c < 4) && (nameUsage[name].allocReg[c] == -1); c++);
                        
                        if (c == 4)
                        {
                            //  None of the name components was defined/renamed!!!
                            panic("ShaderOptmization", "reduceLiveRegisters", "None of the third operand components was defined.");
                        }
                        else
                        {
                            patchedOp3Reg = nameUsage[name].allocReg[c];
                        }                        
                    }
                    else
                        patchedOp3Reg = nameUsage[name].allocReg[firstComponent];

//printf(" => patchedOp3Reg = %d\n", patchedOp3Reg);

                    //  Translate the swizzle mode for the operand.

                    vector <SwizzleMode> opComponents;

                    //  Extract components for operand.
                    extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), opComponents);

//printf(" => components for operand = %d %d %d %d\n", opComponents[0], opComponents[1], opComponents[2], opComponents[3]);

                    u32bit translatedSwz[4];

                    //  Translate the components in the swizzle mode.
                    for(u32bit comp = 0; comp < 4; comp++)
                    {
                        //  If the instruction is actually read rename component name.
                        if (readComponents[opComponents[comp] & 0x03])
                            translatedSwz[comp] = nameUsage[name].allocComp[opComponents[comp] & 0x03];
                        else
                            translatedSwz[comp] = opComponents[comp] & 0x03;
                    }

//printf(" => translated swizzle = %d %d %d %d\n", translatedSwz[0], translatedSwz[1], translatedSwz[2], translatedSwz[3]);

                    //  Recreate the swizzle mode for the operand.
                    if (nameUsage[name].allocReg[firstComponent] == -1)
                        patchedSwzModeOp3 = inProgram[instr]->getOp3SwizzleMode();
                    else
                        patchedSwzModeOp3 = encodeSwizzle(translatedSwz);

                }
                else
                {
                    patchedOp3Reg = inProgram[instr]->getOp3();
                    patchedSwzModeOp3 = inProgram[instr]->getOp3SwizzleMode();
                }

                //  Check if the result register is a temporal register.
                //  Discard KIL, KLS, ZXP and ZXS instructions as they don't write a result.
                if ((inProgram[instr]->getBankRes() == TEMP) && inProgram[instr]->hasResult())
                {
                    //  Get name written by the instruction.
                    u32bit name = inProgram[instr]->getResult();

                    //  Check if the name requires packed allocation.
                    if (nameUsage[name].maxPackedCompUse > 1)
                    {
                        //  Check if the name is aggregated to a master name.
                        if (nameUsage[name].masterName != 0)
                        {
                            //  If so use the allocation information from the master name.
                            name = nameUsage[name].masterName;
                        }

                        //  Check if name was already allocated.
                        if (nameUsage[name].allocated)
                        {
                            //  Get the allocation and update the register mapping table.
                            for(u32bit comp = 0; comp < 4; comp++)
                            {
                                //  Check if the component was created by the instruction.
                                if (nameUsage[name].createdByInstr[comp] == (instr + 1))
                                {
                                    //  Allocating name component with register component.
                                    registerMapping[nameUsage[name].allocReg[comp]][nameUsage[name].allocComp[comp]].name = name;
                                    registerMapping[nameUsage[name].allocReg[comp]][nameUsage[name].allocComp[comp]].component = comp;
                                }
                            }
                        }
                        else
                        {
                            u32bit tempReg = 0;
                            bool foundTempReg = false;
                            u32bit nameMapping = 0;

                            //  Search in the register mapping table for a suitable free register.
                            while ((tempReg < MAX_TEMPORAL_REGISTERS) && !foundTempReg)
                            {
                                bool nameCompCanBeMappedToRegComp[4][4];

                                //  Check for every name component the temporal register components that could be used.
                                for(u32bit nameComp = 0; nameComp < 4; nameComp++)
                                    for(u32bit regComp = 0; regComp < 4; regComp++)
                                    {
                                        nameCompCanBeMappedToRegComp[nameComp][regComp] =
                                            (nameUsage[name].createdByInstr[nameComp] == 0) ||
                                            (registerMapping[tempReg][regComp].freeFromInstr <= nameUsage[name].createdByInstr[nameComp]);

                                        //if (hasSIMD4Result(inProgram[instr]->getOpcode()) && (nameComp != regComp))
                                        //    nameCompCanBeMappedToRegComp[nameComp][regComp] = false;
                                        for(u32bit i = 0; i < inProgram.size(); i++)
                                        {
                                            if (nameUsage[name].usedByInstrOp1[nameComp][i] ||
                                                nameUsage[name].usedByInstrOp2[nameComp][i] ||
                                                nameUsage[name].usedByInstrOp3[nameComp][i])
                                            {
                                                if (hasSIMD4Result(inProgram[i]->getOpcode()) && (nameComp != regComp))
                                                    nameCompCanBeMappedToRegComp[nameComp][regComp] = false;
                                            }
                                        }
                                    }
                                bool mappingFound = false;

                                for(u32bit mapping = 0; (mapping < 24) && !mappingFound; mapping++)
                                {
                                    //  Get the components for the mapping.
                                    u32bit compToRegComp0 = mappingTableComp0[mapping];
                                    u32bit compToRegComp1 = mappingTableComp1[mapping];
                                    u32bit compToRegComp2 = mappingTableComp2[mapping];
                                    u32bit compToRegComp3 = mappingTableComp3[mapping];

                                    //  Check if the mapping is possible.
                                    mappingFound = nameCompCanBeMappedToRegComp[0][compToRegComp0] &&
                                        nameCompCanBeMappedToRegComp[1][compToRegComp1] &&
                                        nameCompCanBeMappedToRegComp[2][compToRegComp2] &&
                                        nameCompCanBeMappedToRegComp[3][compToRegComp3];

                                    //  Name mapping to use.
                                    nameMapping = mapping;
                                }

                                //  Associate name and components to register and components.
                                if (mappingFound)
                                {
                                    //  The temporal register used to allocate the name components was found.
                                    foundTempReg = true;
                                }
                                else
                                {
                                    //  Try with the next temporal register.
                                    tempReg++;
                                }
                            }

                            if (!foundTempReg)
                            {
                                panic("ShaderOptimization", "reduceLiveRegister", "No temporal register available to allocate name.");
                            }

                            //  Set the final mapping between name components and register components.
                            u32bit nameCompToRegComp[4];

                            nameCompToRegComp[0] = mappingTableComp0[nameMapping];
                            nameCompToRegComp[1] = mappingTableComp1[nameMapping];
                            nameCompToRegComp[2] = mappingTableComp2[nameMapping];
                            nameCompToRegComp[3] = mappingTableComp3[nameMapping];

                            //  Set name as allocated to a register.
                            nameUsage[name].allocated = true;

                            //  Allocate registers for the components written by the instruction.
                            for(u32bit comp = 0; comp < 4; comp++)
                            {
                                //  Check if the component was created by the instruction.
                                if (nameUsage[name].createdByInstr[comp] == (instr + 1))
                                {
                                    //  Allocating name component with register component.
                                    registerMapping[tempReg][nameCompToRegComp[comp]].name = name;
                                    registerMapping[tempReg][nameCompToRegComp[comp]].component = comp;
                                    registerMapping[tempReg][nameCompToRegComp[comp]].freeFromInstr = nameUsage[name].lastUsedByInstr[comp];

                                    nameUsage[name].allocReg[comp] = tempReg;
                                    nameUsage[name].allocComp[comp] = nameCompToRegComp[comp];
                                }
                                else if (nameUsage[name].createdByInstr[comp] > (instr + 1))
                                {
                                    //  Associate name component with register component for future allocation.
                                    registerMapping[tempReg][nameCompToRegComp[comp]].freeFromInstr = nameUsage[name].lastUsedByInstr[comp];
                                    nameUsage[name].allocReg[comp] = tempReg;
                                    nameUsage[name].allocComp[comp] = nameCompToRegComp[comp];
                                }
                            }
                        }
                    }
                    else
                    {
                        //  Allocate registers for the components written by the instruction.
                        for(u32bit nameComp = 0; nameComp < 4; nameComp++)
                        {
                            //  Check if the component was created by the instruction.
                            if (nameUsage[name].createdByInstr[nameComp] == (instr + 1))
                            {
                                //  Search for the first free register component.

                                u32bit tempReg = 0;
                                u32bit regComp = 0;
                                bool foundTempRegComp = false;

                                //  Search in the register mapping table for a suitable free register component.
                                while ((tempReg < MAX_TEMPORAL_REGISTERS) && !foundTempRegComp)
                                {
                                    //  Check the temporal register component is free.
                                    if (registerMapping[tempReg][regComp].freeFromInstr <= nameUsage[name].createdByInstr[nameComp])
                                    {
                                        //  A register component was found.
                                        foundTempRegComp = true;
                                    }
                                    else
                                    {
                                        //  Check the next component.
                                        regComp++;

                                        //  Check if it was the last component in the register.
                                        if (regComp == 4)
                                        {
                                            //  Check the next temporal register.
                                            tempReg++;
                                            regComp = 0;
                                        }
                                    }
                                }

                                if (!foundTempRegComp)
                                {
                                    panic("ShaderOptimization", "reduceLiveRegister", "No temporal register available to allocate name.");
                                }

                                //  Allocating name component with register component.
                                registerMapping[tempReg][regComp].name = name;
                                registerMapping[tempReg][regComp].component = nameComp;
                                registerMapping[tempReg][regComp].freeFromInstr = nameUsage[name].lastUsedByInstr[nameComp];

                                nameUsage[name].allocReg[nameComp] = tempReg;
                                nameUsage[name].allocComp[nameComp] = regComp;
                            }
                        }
                    }

                    vector<MaskMode> resComponentsMask;
                    vector<SwizzleMode> resComponentsSwizzle;

                    //  Extract components for result.
                    extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                    //  Set translated result register.
                    patchedResReg = nameUsage[name].allocReg[resComponentsSwizzle[0] & 0x03];

                    //  Translate write mask.
                    bool resCompMapping[4];

                    for(u32bit resComp = 0; resComp < 4; resComp++)
                        resCompMapping[resComp] = false;

                    bool resultCompRenamed = false;

                    for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                    {
                        resCompMapping[nameUsage[name].allocComp[resComponentsSwizzle[comp] & 0x03]] = true;

                        //  Check if any of the result components were renamed.
                        resultCompRenamed |=  (resComponentsSwizzle[comp] & 0x03) != nameUsage[name].allocComp[resComponentsSwizzle[comp] & 0x03];
                    }

                    patchedResMask = encodeWriteMask(resCompMapping);

                    //  Vector operations require changes in the operand swizzle modes if the result register
                    //  components were renamed.
                    if (resultCompRenamed && isVectorOperation(inProgram[instr]->getOpcode()))
                    {
                        //  Apply the result register remapping to the swizzle modes of the operands.
                        vector<SwizzleMode> op1Comps;
                        vector<SwizzleMode> op2Comps;
                        vector<SwizzleMode> op3Comps;

                        u32bit translatedSwzOp1[4];
                        u32bit translatedSwzOp2[4];
                        u32bit translatedSwzOp3[4];

                        //  Extract components accessed by the operands.
                        extractOperandComponents(patchedSwzModeOp1, op1Comps);
                        extractOperandComponents(patchedSwzModeOp2, op2Comps);
                        extractOperandComponents(patchedSwzModeOp3, op3Comps);

                        //  Check for scalar instructions (single component output).
                        if (resComponentsSwizzle.size() == 1)
                        {
                            //  Use broadcast to make clear that it's a scalar instruction.
                            for(u32bit comp = 0; comp < 4; comp++)
                            {
                                translatedSwzOp1[comp] = op1Comps[resComponentsSwizzle[0] & 0x03];
                                translatedSwzOp2[comp] = op2Comps[resComponentsSwizzle[0] & 0x03];
                                translatedSwzOp3[comp] = op3Comps[resComponentsSwizzle[0] & 0x03];
                            }
                        }
                        else
                        {
                            //  Reset translated modes for all operands.
                            for(u32bit comp = 0; comp < 4; comp++)
                            {
                                translatedSwzOp1[comp] = 0;
                                translatedSwzOp2[comp] = 0;
                                translatedSwzOp3[comp] = 0;
                            }

                            //  Swizzle the original result name component to the renamed result register component.
                            for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                            {
                                translatedSwzOp1[nameUsage[name].allocComp[resComponentsSwizzle[comp] & 0x03]] = op1Comps[resComponentsSwizzle[comp] & 0x03];
                                translatedSwzOp2[nameUsage[name].allocComp[resComponentsSwizzle[comp] & 0x03]] = op2Comps[resComponentsSwizzle[comp] & 0x03];
                                translatedSwzOp3[nameUsage[name].allocComp[resComponentsSwizzle[comp] & 0x03]] = op3Comps[resComponentsSwizzle[comp] & 0x03];
                            }
                        }
                        
                        //  Recreate the swizzle mode for the operands.
                        patchedSwzModeOp1 = encodeSwizzle(translatedSwzOp1);
                        patchedSwzModeOp2 = encodeSwizzle(translatedSwzOp2);
                        patchedSwzModeOp3 = encodeSwizzle(translatedSwzOp3);
                    }
                    else if (resultCompRenamed && hasSIMD4Result(inProgram[instr]->getOpcode()))
                    {
                        printf("instr = %d\n", instr);
                        panic("ShaderOptimization", "reduceLiveRegisters",
                            "Result component renaming not possible for instructions that produce a SIMD4 result.");
                    }
                }
                else
                {
                    patchedResReg = inProgram[instr]->getResult();
                    patchedResMask = inProgram[instr]->getResultMaskMode();
                }

                //  Patch instruction with the allocated temporal registers.
                ShaderInstruction *patchedInstr;

                patchedInstr = setRegsAndCompsInstruction(inProgram[instr], patchedResReg, patchedResMask,
                                                          patchedOp1Reg, patchedSwzModeOp1,
                                                          patchedOp2Reg, patchedSwzModeOp2,
                                                          patchedOp3Reg, patchedSwzModeOp3);

                //  Add patched instruction to the output program.
                outProgram.push_back(patchedInstr);

                break;
        }
    }

    /*
    for(u32bit name = 1; name < (names + 1); name++)
    {
        for(u32bit comp = 0; comp < 4; comp++)
        {
            if (nameUsage[name].createdByInstr[comp] != 0)
                printf("Name %d component %d was allocated to register %d component %d\n", name, comp,
                    nameUsage[name].allocReg[comp], nameUsage[name].allocComp[comp]);

        }
    }
    */

    return maxLiveRegisters;
}

void ShaderOptimization::removeRedundantMOVs(vector<ShaderInstruction *> inProgram, vector<ShaderInstruction *> &outProgram)
{
    //  Removes redundant MOVs introduced by renaming.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        bool removeInstruction = false;

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case CHS:

                break;

            case FLR:

                //  Unimplemented.

                break;

            case MOV:

                //  Check if the move is saturating the result.  A mov_sat is a saturation operation,
                //  not an actual mov.
                if (!inProgram[instr]->getSaturatedRes())
                {
                    //  Check if the first operand is a temporal register and doesn't uses the negate or absolute modifiers.
                    if ((inProgram[instr]->getNumOperands() >= 1) && (inProgram[instr]->getBankOp1() == TEMP) &&
                        (!inProgram[instr]->getOp1AbsoluteFlag()) && (!inProgram[instr]->getOp1NegateFlag()))
                    {
                        //  Get operand register.
                        u32bit op1Reg = inProgram[instr]->getOp1();

                        //  Get result register bank.
                        Bank resBank = inProgram[instr]->getBankRes();

                        //  Get result register.
                        u32bit resReg = inProgram[instr]->getResult();

                        //  Check if the input and result register are the same.
                        if ((resBank == TEMP) && (resReg == op1Reg))
                        {
                            vector<MaskMode> resComponentsMask;
                            vector<SwizzleMode> resComponentsSwizzle;

                            vector <u32bit> readComponentsOp1;
                            vector <u32bit> readComponentsOp2;

                            vector <SwizzleMode> op1Components;

                            //  Extract components for result.
                            extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                            //  Retrieve which operand components are actually read.
                            getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                            //  Extract components for first operand.
                            extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                            bool sameComponents = true;

                            //  Check if it's copying the input components over the same output component.
                            for(u32bit comp = 0; (comp < readComponentsOp1.size()) && sameComponents; comp++)
                            {
                                //  Get the actual input component (swizzled) to be written over the output component.
                                u32bit readComp = readComponentsOp1[comp];
                                u32bit op1RegComp = op1Components[readComp] & 0x03;

                                //  Get the result component being written.
                                u32bit resComp = resComponentsSwizzle[comp] & 0x03;

                                //  Check if it is the same component.
                                sameComponents = sameComponents && (op1RegComp == resComp);
                            }

                            //  Set the instruction as to be removed if the input components are copied over the
                            //  same output components.
                            removeInstruction = sameComponents;
                        }
                    }
                }

            default:

                break;
        }

        //  Check if the instruction can be removed.
        if (!removeInstruction)
        {
            //  Copy the instruction to the output program.
            ShaderInstruction *copyInstr;

            copyInstr = copyInstruction(inProgram[instr]);
            outProgram.push_back(copyInstr);
        }
    }
}

//  No longer used.  Code not mantained.
//
#if 0
void ShaderOptimization::copyPropagation(vector<ShaderInstruction *> inProgram, u32bit namesUsed, vector<ShaderInstruction *> &outProgram)
{
    vector<ValueCopy> copyTable;

    //  Allocate vale copy table for all the names used by the program.
    copyTable.clear();

    copyTable.resize(namesUsed + 1);

    //  Clear the table used to track copies.
    for (u32bit tempReg = 0; tempReg < copyTable.size(); tempReg++)
    {
        for(u32bit comp = 0; comp < 4; comp++)
        {
            copyTable[tempReg].createdByInstr[comp] = 0;
            copyTable[tempReg].copiedFromInstr[comp] = 0;
            copyTable[tempReg].copiedRegister[comp] = 0;
            copyTable[tempReg].copiedComponent[comp] = 0;
            copyTable[tempReg].copiedByInstr[comp].clear();
            copyTable[tempReg].copies[comp].clear();
        }
    }

    //  First pass to obtain the information about copies in the program.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        vector<MaskMode> resComponentsMask;
        vector<SwizzleMode> resComponentsSwizzle;

        vector <u32bit> readComponentsOp1;
        vector <u32bit> readComponentsOp2;

        //  Extract components for result.
        extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

        //  Retrieve which operand components are actually read.
        getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case CHS:

                break;

            case COS:
            case FLR:
            case SIN:

                //  Unimplemented.

                break;

            case MOV:

                //  Check if the first operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() >= 1) && (inProgram[instr]->getBankOp1() == TEMP))
                {
                    //  Get operand register.
                    u32bit op1Reg = inProgram[instr]->getOp1();

                    //  Get result register bank.
                    Bank resBank = inProgram[instr]->getBankRes();

                    //  Get result register.
                    u32bit resReg = inProgram[instr]->getResult();

                    if (resBank == TEMP)
                    {
                        //  Track register copies.

                        vector <SwizzleMode> op1Components;

                        //  Extract components for first operand.
                        extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                        //  Set copies.
                        for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                        {
                            //  Get the actual input component (swizzled) to be written over the output component.
                            u32bit readComp = readComponentsOp1[comp];
                            u32bit op1RegComp = op1Components[readComp] & 0x03;

                            //  Get the result component being written.
                            u32bit resComp = resComponentsSwizzle[comp] & 0x03;

                            RegisterComponent regComp;

                            regComp.reg = resReg;
                            regComp.comp = resComp;

                            //  Register the copy.
                            copyTable[resReg].createdByInstr[resComp] = instr + 1;
                            copyTable[resReg].copiedFromInstr[resComp] = copyTable[op1Reg].createdByInstr[op1RegComp];
                            copyTable[resReg].copiedRegister[resComp] = op1Reg;
                            copyTable[resReg].copiedComponent[resComp] = op1RegComp;
                            copyTable[op1Reg].copiedByInstr[op1RegComp].push_back(instr + 1);
                            copyTable[op1Reg].copies[op1RegComp].push_back(regComp);
                        }
                    }
                }
                else
                {
                    //  Register value creations and remove copies on register write.
                    if (inProgram[instr]->getBankRes() == TEMP)
                    {
                        //  Get result register.
                        u32bit resReg = inProgram[instr]->getResult();

                        //  Register new value and destroy previous copies.
                        for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                        {
                            //  Get the result component being written.
                            u32bit resComp = resComponentsSwizzle[comp] & 0x03;

                            //  Register that a new value was written into the register by the current instruction.
                            copyTable[resReg].createdByInstr[resComp] = instr + 1;

                            //  The current value isn't a copy.
                            copyTable[resReg].copiedFromInstr[resComp] = 0;
                            copyTable[resReg].copiedRegister[resComp] = 0;
                            copyTable[resReg].copiedComponent[resComp] = 0;

                            //  The current value doesn't have copies.
                            copyTable[resReg].copiedByInstr[resComp].clear();
                            copyTable[resReg].copies[resComp].clear();
                        }
                    }
                }

                break;

            default:

                //  Register value creations and remove copies on register write.
                if (inProgram[instr]->getBankRes() == TEMP)
                {
                    //  Get result register.
                    u32bit resReg = inProgram[instr]->getResult();

                    //  Register new value and destroy previous copies.
                    for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                    {
                        //  Get the result component being written.
                        u32bit resComp = resComponentsSwizzle[comp] & 0x03;

                        //  Register that a new value was written into the register by the current instruction.
                        copyTable[resReg].createdByInstr[resComp] = instr + 1;

                        //  The current value isn't a copy.
                        copyTable[resReg].copiedFromInstr[resComp] = 0;
                        copyTable[resReg].copiedRegister[resComp] = 0;
                        copyTable[resReg].copiedComponent[resComp] = 0;

                        //  The current value doesn't have copies.
                        copyTable[resReg].copiedByInstr[resComp].clear();
                        copyTable[resReg].copies[resComp].clear();
                    }
                }

                break;
        }
    }


    /*for(u32bit tempReg = 0; tempReg < copyTable.size(); tempReg++)
    {
        printf("Name %d\n", tempReg);
        for(u32bit comp = 0; comp < 4; comp++)
        {
            printf("  Comp %d -> createdByInstr %d (%04x) copiedFromInstr %d (%04x) copiedRegister %d copiedComponent %d\n",
                comp,
                copyTable[tempReg].createdByInstr[comp], (copyTable[tempReg].createdByInstr[comp] - 1) * 16,
                copyTable[tempReg].copiedFromInstr[comp], (copyTable[tempReg].copiedFromInstr[comp] - 1) * 16,
                copyTable[tempReg].copiedRegister[comp], copyTable[tempReg].copiedComponent[comp]);
            printf("             copiedByInstr :");
            for(u32bit instr = 0; instr < copyTable[tempReg].copiedByInstr[comp].size(); instr++)
                printf("%d (%04x) ", copyTable[tempReg].copiedByInstr[comp][instr], (copyTable[tempReg].copiedByInstr[comp][instr] - 1) * 16);
            printf("\n");
            printf("             copies :");
            for(u32bit instr = 0; instr < copyTable[tempReg].copies[comp].size(); instr++)
                printf("(%d.%d) ", copyTable[tempReg].copies[comp][instr].reg, copyTable[tempReg].copies[comp][instr].comp);
            printf("\n");
        }
    }*/

    //  Second pass to propagate copies.
    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        u32bit copyRegOp1;
        u32bit copyRegOp2;
        u32bit copyRegOp3;

        //u32bit copyCompOp1[4];
        //u32bit copyCompOp2[4];
        //u32bit copyCompOp3[4];

        vector<MaskMode> resComponentsMask;
        vector<SwizzleMode> resComponentsSwizzle;

        vector <u32bit> readComponentsOp1;
        vector <u32bit> readComponentsOp2;

        u32bit patchedResReg;
        MaskMode patchedResMask;
        SwizzleMode patchedSwzModeOp1;
        SwizzleMode patchedSwzModeOp2;
        SwizzleMode patchedSwzModeOp3;

        bool patchInstruction = false;

        //  Extract components for result.
        extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

        //  Retrieve which operand components are actually read.
        getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

        /*switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case CHS:

                break;

            case COS:
            case FLR:
            case SIN:

                //  Unimplemented.

                break;

            default:

                //  Get original registers and swizzle/mask modes.
                copyRegOp1 = inProgram[instr]->getOp1();
                copyRegOp2 = inProgram[instr]->getOp2();
                copyRegOp3 = inProgram[instr]->getOp3();
                patchedSwzModeOp1 = inProgram[instr]->getOp1SwizzleMode();
                patchedSwzModeOp2 = inProgram[instr]->getOp2SwizzleMode();
                patchedSwzModeOp3 = inProgram[instr]->getOp3SwizzleMode();
                patchedResReg = inProgram[instr]->getResult();
                patchedResMask = inProgram[instr]->getResultMaskMode();

                //  Propagate copies in first operand.
                if ((inProgram[instr]->getNumOperands() > 0) && (inProgram[instr]->getBankOp1() == TEMP))
                {
                    vector <SwizzleMode> op1Components;

                    //  Extract components for first operand.
                    extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                    //  Get first operand register.
                    u32bit op1Reg = inProgram[instr]->getOp1();

                    bool copySearchEnd = false;

                    //  Clear component to copy.
                    for(u32bit comp = 0; comp < 4; comp++)
                        copyCompOp1[comp] = 5;

                    //  Scan for copies.
                    for(u32bit comp = 0; comp < readComponentsOp1.size() && !copySearchEnd; comp++)
                    {
                        //  Get the actual input component (swizzled) to be written over the output component.
                        u32bit readComp = readComponentsOp1[comp];
                        u32bit op1RegComp = op1Components[readComp] & 0x03;

                        //  Check if the register and component were copied.
                        if (copyTable[op1Reg].copiedFromInstr[op1RegComp] > 0)
                        {
                            //  Get the register and component that were copied.
                            u32bit copiedReg = copyTable[op1Reg].copiedRegister[op1RegComp];
                            u32bit copiedComp = copyTable[op1Reg].copiedComponent[op1RegComp];

                            bool firstCopyFound = true;
                            bool stopSearchForFirstCopy = false;
                            u32bit firstCopyReg = op1Reg;
                            u32bit firstCopyComp = op1RegComp;

                            while(!stopSearchForFirstCopy)
                            {
                                //  Check if the copied register was overwritten.
                                if (copyTable[copiedReg].createdByInstr[copiedComp] <= copyTable[firstCopyReg].copiedFromInstr[firstCopyComp])
                                {
                                    //  A valid copy was found.
                                    firstCopyFound = true;

                                    //  Set as a possible first copy.
                                    firstCopyReg = copiedReg;
                                    firstCopyComp = copiedComp;

                                    //  Check if the copied register is also a copy.
                                    if (copyTable[copiedReg].copiedFromInstr[copiedComp] > 0)
                                    {
                                        //  Check the next copy in the chain.
                                        copiedReg = copyTable[firstCopyReg].copiedRegister[firstCopyComp];
                                        copiedComp = copyTable[firstCopyReg].copiedComponent[firstCopyComp];
                                    }
                                    else
                                    {
                                        //  First copy was found.
                                        stopSearchForFirstCopy = true;
                                    }
                                }
                                else
                                {
                                    //  Stop searching as the current copy in the chain is not valid.
                                    stopSearchForFirstCopy = true;
                                }
                            }

                            //  Check if the first copy was found.
                            if (firstCopyFound)
                            {
                                //  If first component scanned set the copy register for the operand.
                                if (comp == 0)
                                {
                                    copyRegOp1 = firstCopyReg;
                                }
                                else
                                {
                                    //  Check that all the components of the operand use the same register as the first copy.
                                    if (copyRegOp1 != firstCopyReg)
                                    {
                                        //  Stop searching.
                                        copySearchEnd = true;
                                    }
                                }

                                //  Set the component to be copied.
                                copyCompOp1[comp] = firstCopyComp;
                            }
                            else
                            {
                                copySearchEnd = true;
                            }
                        }
                        else
                        {
                            copySearchEnd = true;
                        }
                    }

                    if (!copySearchEnd)
                    {
                        u32bit translatedSwz[4];

                        //  Translate the components in the swizzle mode.
                        for(u32bit comp = 0; comp < 4; comp++)
                        {
                            //  If the instruction is actually read rename component name.
                            if (copyCompOp1[comp] != 5)
                                translatedSwz[comp] = copyCompOp1[comp];
                            else
                                translatedSwz[comp] = op1Components[comp] & 0x03;
                        }

                        //  Recreate the swizzle mode for the operand.
                        patchedSwzModeOp1 = encodeSwizzle(translatedSwz);

                        patchInstruction = true;
                    }
                }

                //  Propagate copies in second operand.
                if ((inProgram[instr]->getNumOperands() > 1) && (inProgram[instr]->getBankOp2() == TEMP))
                {
                    vector <SwizzleMode> op2Components;

                    //  Extract components for second operand.
                    extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                    //  Get second operand register.
                    u32bit op2Reg = inProgram[instr]->getOp2();

                    bool copySearchEnd = false;

                    //  Clear component to copy.
                    for(u32bit comp = 0; comp < 4; comp++)
                        copyCompOp2[comp] = 5;

                    //  Scan for copies.
                    for(u32bit comp = 0; comp < readComponentsOp2.size() && !copySearchEnd; comp++)
                    {
                        //  Get the actual input component (swizzled) to be written over the output component.
                        u32bit readComp = readComponentsOp2[comp];
                        u32bit op2RegComp = op2Components[readComp] & 0x03;

                        //  Check if the register and component were copied.
                        if (copyTable[op2Reg].copiedFromInstr[op2RegComp] > 0)
                        {
                            //  Get the register and component that were copied.
                            u32bit copiedReg = copyTable[op2Reg].copiedRegister[op2RegComp];
                            u32bit copiedComp = copyTable[op2Reg].copiedComponent[op2RegComp];

                            bool firstCopyFound = false;
                            bool stopSearchForFirstCopy = false;
                            u32bit firstCopyReg = op2Reg;
                            u32bit firstCopyComp = op2RegComp;

                            while(!stopSearchForFirstCopy)
                            {
                                //  Check if the copied register was overwritten.
                                if (copyTable[copiedReg].createdByInstr[copiedComp] <= copyTable[firstCopyReg].copiedFromInstr[firstCopyComp])
                                {
                                    //  A valid copy was found.
                                    firstCopyFound = true;

                                    //  Set as a possible first copy.
                                    firstCopyReg = copiedReg;
                                    firstCopyComp = copiedComp;

                                    //  Check if the copied register is also a copy.
                                    if (copyTable[copiedReg].copiedFromInstr[copiedComp] > 0)
                                    {
                                        //  Check the next copy in the chain.
                                        copiedReg = copyTable[firstCopyReg].copiedRegister[firstCopyComp];
                                        copiedComp = copyTable[firstCopyReg].copiedComponent[firstCopyComp];
                                    }
                                    else
                                    {
                                        //  First copy was found.
                                        stopSearchForFirstCopy = true;
                                    }
                                }
                                else
                                {
                                    //  Stop searching as the current copy in the chain is not valid.
                                    stopSearchForFirstCopy = true;
                                }
                            }

                            //  Check if the first copy was found.
                            if (firstCopyFound)
                            {
                                //  If first component scanned set the copy register for the operand.
                                if (comp == 0)
                                {
                                    copyRegOp2 = firstCopyReg;
                                }
                                else
                                {
                                    //  Check that all the components of the operand use the same register as the first copy.
                                    if (copyRegOp2 != firstCopyReg)
                                    {
                                        //  Stop searching.
                                        copySearchEnd = true;
                                    }
                                }

                                //  Set the component to be copied.
                                copyCompOp2[comp] = firstCopyComp;
                            }
                            else
                            {
                                copySearchEnd = true;
                            }
                        }
                        else
                        {
                            copySearchEnd = true;
                        }
                    }

                    if (!copySearchEnd)
                    {
                        u32bit translatedSwz[4];

                        //  Translate the components in the swizzle mode.
                        for(u32bit comp = 0; comp < 4; comp++)
                        {
                            //  If the instruction is actually read rename component name.
                            if (copyCompOp2[comp] != 5)
                                translatedSwz[comp] = copyCompOp2[comp];
                            else
                                translatedSwz[comp] = op2Components[comp] & 0x03;
                        }

                        //  Recreate the swizzle mode for the operand.
                        patchedSwzModeOp2 = encodeSwizzle(translatedSwz);

                        patchInstruction = true;
                    }
                }

                //  Propagate copies in third operand.
                if ((inProgram[instr]->getNumOperands() == 3) && (inProgram[instr]->getBankOp3() == TEMP))
                {
                    vector <SwizzleMode> op3Components;

                    //  Extract components for third operand.
                    extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                    //  Get second operand register.
                    u32bit op3Reg = inProgram[instr]->getOp3();

                    bool copySearchEnd = false;

                    //  Clear component to copy.
                    for(u32bit comp = 0; comp < 4; comp++)
                        copyCompOp3[comp] = 5;

                    //  Scan for copies.
                    for(u32bit comp = 0; comp < readComponentsOp1.size() && !copySearchEnd; comp++)
                    {
                        //  Get the actual input component (swizzled) to be written over the output component.
                        u32bit readComp = readComponentsOp1[comp];
                        u32bit op3RegComp = op3Components[readComp] & 0x03;

                        //  Check if the register and component were copied.
                        if (copyTable[op3Reg].copiedFromInstr[op3RegComp] > 0)
                        {
                            //  Get the register and component that were copied.
                            u32bit copiedReg = copyTable[op3Reg].copiedRegister[op3RegComp];
                            u32bit copiedComp = copyTable[op3Reg].copiedComponent[op3RegComp];

                            bool firstCopyFound = false;
                            bool stopSearchForFirstCopy = false;
                            u32bit firstCopyReg = op3Reg;
                            u32bit firstCopyComp = op3RegComp;

                            while(!stopSearchForFirstCopy)
                            {
                                //  Check if the copied register was overwritten.
                                if (copyTable[copiedReg].createdByInstr[copiedComp] <= copyTable[firstCopyReg].copiedFromInstr[firstCopyComp])
                                {
                                    //  A valid copy was found.
                                    firstCopyFound = true;

                                    //  Set as a possible first copy.
                                    firstCopyReg = copiedReg;
                                    firstCopyComp = copiedComp;

                                    //  Check if the copied register is also a copy.
                                    if (copyTable[copiedReg].copiedFromInstr[copiedComp] > 0)
                                    {
                                        //  Check the next copy in the chain.
                                        copiedReg = copyTable[firstCopyReg].copiedRegister[firstCopyComp];
                                        copiedComp = copyTable[firstCopyReg].copiedComponent[firstCopyComp];
                                    }
                                    else
                                    {
                                        //  First copy was found.
                                        stopSearchForFirstCopy = true;
                                    }
                                }
                                else
                                {
                                    //  Stop searching as the current copy in the chain is not valid.
                                    stopSearchForFirstCopy = true;
                                }
                            }

                            //  Check if the first copy was found.
                            if (firstCopyFound)
                            {
                                //  If first component scanned set the copy register for the operand.
                                if (comp == 0)
                                {
                                    copyRegOp3 = firstCopyReg;
                                }
                                else
                                {
                                    //  Check that all the components of the operand use the same register as the first copy.
                                    if (copyRegOp3 != firstCopyReg)
                                    {
                                        //  Stop searching.
                                        copySearchEnd = true;
                                    }
                                }

                                //  Set the component to be copied.
                                copyCompOp3[comp] = firstCopyComp;
                            }
                            else
                            {
                                copySearchEnd = true;
                            }
                        }
                        else
                        {
                            copySearchEnd = true;
                        }
                    }

                    if (!copySearchEnd)
                    {
                        u32bit translatedSwz[4];

                        //  Translate the components in the swizzle mode.
                        for(u32bit comp = 0; comp < 4; comp++)
                        {
                            //  If the instruction is actually read rename component name.
                            if (copyCompOp3[comp] != 5)
                                translatedSwz[comp] = copyCompOp3[comp];
                            else
                                translatedSwz[comp] = op3Components[comp] & 0x03;
                        }

                        //  Recreate the swizzle mode for the operand.
                        patchedSwzModeOp3 = encodeSwizzle(translatedSwz);

                        patchInstruction = true;
                    }
                }


                //  Register value creations and remove copies on register write.

                if (inProgram[instr]->getBankRes() == TEMP)
                {
                    //  Get result register.
                    u32bit resReg = inProgram[instr]->getResult();

                    //  Register new value and destroy previous copies.
                    for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                    {


                        //  Get the result component being written.
                        u32bit resComp = resComponentsSwizzle[comp] & 0x03;

                        //  Register that a new value was written into the register by the current instruction.
                        copyTable[resReg].createdByInstr[resComp] = instr + 1;

                        //  The current value isn't a copy.
                        copyTable[resReg].copiedFromInstr[resComp] = 0;
                        copyTable[resReg].copiedRegister[resComp] = 0;
                        copyTable[resReg].copiedComponent[resComp] = 0;

                        //  The current value doesn't have copies.
                        copyTable[resReg].copies[resComp].clear();
                    }
                }

                break;
        }*/

        if (patchInstruction)
        {
            //  Patch instruction with the allocated temporal registers.
            ShaderInstruction *patchedInstr;

            patchedInstr = setRegsAndCompsInstruction(inProgram[instr], patchedResReg, patchedResMask,
                                                                        copyRegOp1, patchedSwzModeOp1,
                                                                        copyRegOp2, patchedSwzModeOp2,
                                                                        copyRegOp3, patchedSwzModeOp3);

            //  Add patched instruction to the output program.
            outProgram.push_back(patchedInstr);
        }
        else
        {
            //  Copy the instruction to the output program.
            ShaderInstruction *copyInstr;

            copyInstr = copyInstruction(inProgram[instr]);
            outProgram.push_back(copyInstr);
        }
    }
}
#endif

void ShaderOptimization::optimize(vector<ShaderInstruction *> inProgram, vector<ShaderInstruction *> &outProgram,
                                  u32bit &maxLiveTempRegs, bool noRename, bool AOStoSOA, bool verbose)
{

    bool codeToEliminate = true;
    u32bit pass = 0;
    u32bit namesUsed = 32;
    u32bit liveRegisters = 0;

    vector<ShaderInstruction *> tempInProgram;
    vector<ShaderInstruction *> tempOutProgram;
    copyProgram(inProgram, tempInProgram);

    if (!noRename)
    {
        //  Rename temporal registers.
        deleteProgram(tempOutProgram);
        namesUsed = renameRegisters(tempInProgram, tempOutProgram, AOStoSOA);

        if (verbose)
        {
            printf("Pass %d. Register renaming : \n", pass);
            printf("----------------------------------------\n");

            printProgram(tempOutProgram);

            printf("\n");
        }

        //  Prepare for next optimization pass.
        deleteProgram(tempInProgram);
        copyProgram(tempOutProgram, tempInProgram);
        pass++;
    }

    //  Eliminate dead code (registers and components).
    while(codeToEliminate)
    {
        deleteProgram(tempOutProgram);
        codeToEliminate = deadCodeElimination(tempInProgram, namesUsed, tempOutProgram);

        if (verbose)
        {
            printf("Pass %d. Dead code elimination : \n", pass);
            printf("----------------------------------------\n");

            if (codeToEliminate)
                printProgram(tempOutProgram);
            else
                printf(" No code eliminated.\n");

            printf("\n");
        }

        //  Prepare for next optimization pass.
        deleteProgram(tempInProgram);
        copyProgram(tempOutProgram, tempInProgram);
        pass++;
    }

    /*deleteProgram(tempOutProgram);
    copyPropagation(tempInProgram, namesUsed, tempOutProgram);
    copyProgram(tempInProgram, tempOutProgram);

    if (verbose)
    {
        printf("Pass %d.  Copy Propagation : \n", pass);
        printf("----------------------------------------\n");

        printProgram(tempOutProgram);

        printf("\n");
    }*/

    if (!noRename)
    {
        deleteProgram(tempOutProgram);
        liveRegisters = reduceLiveRegisters(tempInProgram, namesUsed, tempOutProgram);

        if (verbose)
        {
            printf("Pass %d.  Live registers reduction : \n", pass);
            printf("----------------------------------------\n");

            printProgram(tempOutProgram);

            printf("\n");
            printf("Max live registers used by the program : %d\n", liveRegisters);

            printf("\n");
        }

        maxLiveTempRegs = liveRegisters;

        deleteProgram(tempInProgram);
        copyProgram(tempOutProgram, tempInProgram);
        pass++;
    }

    deleteProgram(tempOutProgram);
    removeRedundantMOVs(tempInProgram, tempOutProgram);

    if (verbose)
    {
        printf("Pass %d.  Remove redundant MOVs : \n", pass);
        printf("----------------------------------------\n");

        printProgram(tempOutProgram);

        printf("\n");
    }

    deleteProgram(tempInProgram);
    copyProgram(tempOutProgram, tempInProgram);
    pass++;

    codeToEliminate = true;

    //  Eliminate dead code (registers and components).
    while(codeToEliminate)
    {
        deleteProgram(tempOutProgram);
        codeToEliminate = deadCodeElimination(tempInProgram, namesUsed, tempOutProgram);

        if (verbose)
        {
            printf("Pass %d. Dead code elimination : \n", pass);
            printf("----------------------------------------\n");

            if (codeToEliminate)
                printProgram(tempOutProgram);
            else
                printf(" No code eliminated.\n");

            printf("\n");
        }

        //  Prepare for next optimization pass.
        deleteProgram(tempInProgram);
        copyProgram(tempOutProgram, tempInProgram);
        pass++;
    }

    deleteProgram(tempInProgram);
    //copyProgram(tempOutProgram, tempInProgram);
    pass++;

    copyProgram(tempOutProgram, outProgram);
    deleteProgram(tempOutProgram);
}

void ShaderOptimization::assignWaitPoints(vector<ShaderInstruction *> inProgram, vector<ShaderInstruction *> &outProgram)
{
    bool regPendingFromLoad[MAX_TEMPORAL_REGISTERS][4];

    //  Clear all registers/components as not pending from a previous texture or attribute load.
    for(u32bit reg = 0; reg < MAX_TEMPORAL_REGISTERS; reg++)
    {
        for(u32bit comp = 0; comp < 4; comp++)
        {
            regPendingFromLoad[reg][comp] = false;
        }
    }

    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        ShaderInstruction *copyInstr;

        vector<MaskMode> resComponentsMask;
        vector<SwizzleMode> resComponentsSwizzle;

        vector <u32bit> readComponentsOp1;
        vector <u32bit> readComponentsOp2;

        bool setWaitPoint = false;

        switch(inProgram[instr]->getOpcode())
        {
            case NOP:
            case END:
            case CHS:

                //  Those instruction don't set or use registers.

                break;

            case FLR:

                //  Unimplemented.

                break;

            default:

                //  Extract components for result.
                extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                //  Retrieve which operand components are actually read.
                getReadOperandComponents(inProgram[instr], resComponentsSwizzle, readComponentsOp1, readComponentsOp2);

                //  Check if the first operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() >= 1) && (inProgram[instr]->getBankOp1() == TEMP))
                {
                    vector <SwizzleMode> op1Components;

                    //  Extract components for first operand.
                    extractOperandComponents(inProgram[instr]->getOp1SwizzleMode(), op1Components);

                    //  Check if any of the operand components is pending from a previous texture or attribute load
                    //  instruction.
                    for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                    {
                        //  Get the actual component being accessed after applying swizzle.
                        u32bit readComp = readComponentsOp1[comp];
                        u32bit op1RegComp = op1Components[readComp] & 0x03;

                        //  Set a wait point in the previous instruction if the component is still pending from
                        //  a previous texture or attribute load operation.
                        setWaitPoint |= regPendingFromLoad[inProgram[instr]->getOp1()][op1RegComp];
                    }
                }

                //  Check if the second operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() >= 2) && (inProgram[instr]->getBankOp2() == TEMP))
                {
                    vector <SwizzleMode> op2Components;

                    //  Extract components for second operand.
                    extractOperandComponents(inProgram[instr]->getOp2SwizzleMode(), op2Components);

                    //  Check if any of the operand components is pending from a previous texture or attribute load
                    //  instruction.
                    for(u32bit comp = 0; comp < readComponentsOp2.size(); comp++)
                    {
                        //  Get the actual component being accessed after applying swizzle.
                        u32bit readComp = readComponentsOp2[comp];
                        u32bit op2RegComp = op2Components[readComp] & 0x03;

                        //  Set a wait point in the previous instruction if the component is still pending from
                        //  a previous texture or attribute load operation.
                        setWaitPoint |= regPendingFromLoad[inProgram[instr]->getOp2()][op2RegComp];
                    }
                }

                //  Check if the third operand is a temporal register.
                if ((inProgram[instr]->getNumOperands() == 3) && (inProgram[instr]->getBankOp3() == TEMP))
                {
                    vector <SwizzleMode> op3Components;

                    //  Extract components for third operand.
                    extractOperandComponents(inProgram[instr]->getOp3SwizzleMode(), op3Components);

                    //  Check if any of the operand components is pending from a previous texture or attribute load
                    //  instruction.
                    //  The order of the operands actually read for the third operand is the same than for the first operand.
                    for(u32bit comp = 0; comp < readComponentsOp1.size(); comp++)
                    {
                        //  Get the actual component being accessed after applying swizzle.
                        u32bit readComp = readComponentsOp1[comp];
                        u32bit op3RegComp = op3Components[readComp] & 0x03;

                        //  Set a wait point in the previous instruction if the component is still pending from
                        //  a previous texture or attribute load operation.
                        setWaitPoint |= regPendingFromLoad[inProgram[instr]->getOp3()][op3RegComp];
                    }
                }
                
                //  Check for WAW dependences.  Writes on temporary register with a pending result from the texture unit
                //  must wait until the texture unit writes the result.                
                if (inProgram[instr]->getBankRes() == TEMP)
                {
                    for(u32bit comp = 0; comp < 4; comp++)
                        setWaitPoint |= regPendingFromLoad[inProgram[instr]->getResult()][comp];
                }

                //  Check if the previous instruction must be marked as a wait point for texture or attributes loaded.
                if (setWaitPoint)
                {
                    //  Mark the previous instruction as a wait point.
                    outProgram[instr - 1]->setWaitPointFlag();

                    //  After a wait point all the load operations are finished so the pending registers can be cleared.

                    //  Clear all registers/components as not pending from a previous texture or attribute load.
                    for(u32bit reg = 0; reg < MAX_TEMPORAL_REGISTERS; reg++)
                    {
                        for(u32bit comp = 0; comp < 4; comp++)
                        {
                            regPendingFromLoad[reg][comp] = false;
                        }
                    }
                }

                //  Check for texture or attribute load instructions.  If the result register is a temporal register
                //  mark the register as pending from load operation.
                if ((inProgram[instr]->getBankRes() == TEMP) &&
                        ((inProgram[instr]->getOpcode() == TEX) || (inProgram[instr]->getOpcode() == TXB) ||
                         (inProgram[instr]->getOpcode() == TXL) || (inProgram[instr]->getOpcode() == TXP) ||
                         (inProgram[instr]->getOpcode() == LDA)))
                {
                    //  Extract components for result.
                    extractResultComponents(inProgram[instr]->getResultMaskMode(), resComponentsMask, resComponentsSwizzle);

                    //  Set the written components as pending from load operation.
                    for(u32bit comp = 0; comp < resComponentsSwizzle.size(); comp++)
                    {
                        //  Get the actual component being written.
                        u32bit writtenComp = resComponentsSwizzle[comp] & 0x03;
                        
                        //  Set written component as pending from load operation.
                        regPendingFromLoad[inProgram[instr]->getResult()][writtenComp] = true;
                    }
                }

                break;
        }

        //  Copy the instruction.
        copyInstr = copyInstruction(inProgram[instr]);
        outProgram.push_back(copyInstr);
    }
}


void ShaderOptimization::decodeProgram(u8bit *code, u32bit size, vector<ShaderInstruction *> &outProgram, u32bit &numTemps, bool &hasJumps)
{
    bool usedTemps[MAX_TEMPORAL_REGISTERS];
    
    //  Initialize the decoded program.
    outProgram.clear();

    //  No jumps found yet in the program.
    hasJumps = false;

    //  Initialize the used temporary register array.
    for(u32bit r = 0; r < MAX_TEMPORAL_REGISTERS; r++)
        usedTemps[r] = false;

    //  No temporary registers used yet.
    numTemps = 0;
        
    //  Decode the instructions of the program.
    for(u32bit b = 0; b < size; b += ShaderInstruction::SHINSTRSIZE)
    {
        ShaderInstruction *shInstr;

        shInstr = new ShaderInstruction(&code[b]);
        
        //  Check for new temporary register used as first operand.
        if ((shInstr->getNumOperands() > 0) && (shInstr->getBankOp1() == TEMP) && !usedTemps[shInstr->getOp1()])
        {
            //  Update the number of temporary registers found in the program.
            numTemps++;
            
            //  Update the array of used temporary registers.
            usedTemps[shInstr->getOp1()];
        }

        //  Check for new temporary register used as second operand.
        if ((shInstr->getNumOperands() > 1) && (shInstr->getBankOp2() == TEMP) && !usedTemps[shInstr->getOp2()])
        {
            //  Update the number of temporary registers found in the program.
            numTemps++;
            
            //  Update the array of used temporary registers.
            usedTemps[shInstr->getOp2()];
        }
            
        //  Check for new temporary register used as second operand.
        if ((shInstr->getNumOperands() > 2) && (shInstr->getBankOp3() == TEMP) && !usedTemps[shInstr->getOp3()])
        {
            //  Update the number of temporary registers found in the program.
            numTemps++;
            
            //  Update the array of used temporary registers.
            usedTemps[shInstr->getOp3()];
        }
        
        //  Check if the instruction is a jump.
        hasJumps |= shInstr->isAJump();

        outProgram.push_back(shInstr);
    }
}


void ShaderOptimization::encodeProgram(vector<ShaderInstruction *> inProgram, u8bit *code, u32bit &size)
{
    //  Check size buffer size.
    GPU_ASSERT(
        if (size < (inProgram.size() * ShaderInstruction::SHINSTRSIZE))
            panic("ShaderOptimization", "encodeProgram", "Buffer for shader program is too small.");
    )

    size = (u32bit) inProgram.size() * ShaderInstruction::SHINSTRSIZE;

    for(u32bit instr = 0; instr < inProgram.size(); instr++)
    {
        inProgram[instr]->getCode(&code[instr * ShaderInstruction::SHINSTRSIZE]);
    }
}

void ShaderOptimization::assembleProgram(u8bit *program, u8bit *code, u32bit &size)
{
    stringstream inputProgram;
    vector<ShaderInstruction*> shaderProgram;

    inputProgram.clear();
    inputProgram.str((char *) program);
    shaderProgram.clear();

    u32bit line = 1;

    bool error = false;

    //  Read until the end of the input program or until there is an error assembling the instruction.
    while(!error && !inputProgram.eof())
    {
        ShaderInstruction *shInstr;

        string currentLine;
        string errorString;

        //  Read a line from the input file.
        getline(inputProgram, currentLine);

        if ((currentLine.length() == 0) && inputProgram.eof())
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
                shaderProgram.push_back(shInstr);
            }

            //  Update line number.
            line++;
        }
    }

    //  Check if an error occurred.
    if (!error)
    {
        //  Set the end instruction flag to the last instruction in the program.
        shaderProgram[shaderProgram.size() - 1]->setEndFlag(true);
        
        //  Encode the shader program.
        encodeProgram(shaderProgram, code, size);
        deleteProgram(shaderProgram);
    }
    else
    {
        panic("ShaderOptimization", "assembleProgram", "Error assembling program.");
    }
}

void ShaderOptimization::disassembleProgram(u8bit *code, u32bit codeSize, u8bit *program, u32bit &size, bool &disableEarlyZ)
{
    vector<ShaderInstruction*> shaderProgram;
    
    u32bit numTemps;
    bool hasJumps;
    
    decodeProgram(code, codeSize, shaderProgram, numTemps, hasJumps);
    
    u32bit asmCodeSize = 0;
    
    string asmCode;
    asmCode.clear();
    
    disableEarlyZ = false;
    for(u32bit instr = 0; instr < shaderProgram.size(); instr++)
    {
        char buffer[1024];
        shaderProgram[instr]->disassemble(buffer);
        
        disableEarlyZ |= mustDisableEarlyZ(shaderProgram[instr]->getOpcode());
        
        asmCode.append(buffer);
        asmCode.append("\n");
    }
    
    deleteProgram(shaderProgram);
    
    if (asmCode.size() > size)
    {
        char buffer[128];
        sprintf(buffer, "The buffer for the disassembled shader program is too small %d bytes, required %d bytes\n",
            size, asmCode.size());
    }
    
    strcpy((char *) program, asmCode.c_str());
    
    size = asmCode.size();
}

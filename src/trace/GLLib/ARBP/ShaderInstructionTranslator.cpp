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

#define MAXIMUM(x,y)    x > y ? x : y

#include "GPULibInternals.h" // TO access driver directly !!!

#include "ShaderInstructionTranslator.h"
#include "ImplementationLimits.h"
#include "ShaderInstruction.h"

#include <iostream>

using namespace std;
using namespace libgl;
using namespace libgl::GenerationCode;
using namespace gpu3d;

ShaderInstructionTranslator::ShaderInstrOperand::ShaderInstrOperand()
: idReg(0), idBank(INVALID), swizzleMode(XYZW),
  absoluteFlag(false), negateFlag(false), relativeModeFlag(false),
  relModeAddrReg(0), relModeAddrComp(0), relModeOffset(0)
{
}

ShaderInstructionTranslator::ShaderInstrResult::ShaderInstrResult()
 : writeMask(mXYZW), idReg(0), idBank(INVALID)
{
}

ShaderInstructionTranslator::ShaderInstructionTranslator(RBank<f32bit>& parametersBank,
                                                         RBank<f32bit>& temporariesBank,
                                                         unsigned int readPortsInBank,
                                                         unsigned int readPortsOutBank,
                                                         unsigned int readPortsParamBank,
                                                         unsigned int readPortsTempBank,
                                                         unsigned int readPortsAddrBank) :
   paramBank(parametersBank),
   tempBank(temporariesBank),
   readPortsInBank(readPortsInBank),
   readPortsOutBank(readPortsOutBank),
   readPortsParamBank(readPortsParamBank),
   readPortsTempBank(readPortsTempBank),
   readPortsAddrBank(readPortsAddrBank),
   maxLiveTempsComputed(false)
{

}


bool ShaderInstructionTranslator::thereIsEquivalentInstruction(GenericInstruction::Opcode opc) const
{
    switch (opc)
    {
        case GenericInstruction::G_ABS: return false;
        case GenericInstruction::G_LRP: return false;
        case GenericInstruction::G_POW: return false;
        case GenericInstruction::G_SCS: return false;
        case GenericInstruction::G_SUB: return false;
        case GenericInstruction::G_SWZ: return false;
        case GenericInstruction::G_XPD: return false;
        case GenericInstruction::G_ABS_SAT: return false;
        case GenericInstruction::G_LRP_SAT: return false;
        case GenericInstruction::G_POW_SAT: return false;
        case GenericInstruction::G_SCS_SAT: return false;
        case GenericInstruction::G_SUB_SAT: return false;
        case GenericInstruction::G_SWZ_SAT: return false;
        case GenericInstruction::G_XPD_SAT: return false;
        default: return true;
    }
}

 /* NV VP2 Instructions Limitations:
  * Ref (NV_vertex_program2.txt) Section 2.14.1.8,  Vertex Program Specification
  * subsection Semantic Restrictions
  *
  * "A vertex program fails to load if any instruction sources more than one
  *  unique program parameter register.  An instruction can match the
  *  <progParamRegister> rule more than once only if all such matches are
  *  identical.
  *
  *  A vertex program fails to load if any instruction sources more than one
  *  unique vertex attribute register.  An instruction can match the
  *  <vtxAttribRegister> rule more than once only if all such matches refer to
  *  the same register."
  */

bool ShaderInstructionTranslator::thereAreOperandAccessConflicts(
                                    const ShaderInstrOperand& shop1,
                                    const ShaderInstrOperand& shop2,
                                    const ShaderInstrOperand& shop3,
                                    unsigned int num_operands,
                                    bool& preloadOp1,
                                    bool& preloadOp2) const
{
    preloadOp1 = false;
    preloadOp2 = false;

    int diferentReadsToInBank = 0;
    int diferentReadsToParamBank = 0;
    int diferentReadsToTempBank = 0;
    int diferentReadsToAddrBank = 0;

    /* For instructions with only an operand there are no conflicts */
    if (num_operands < 2) return false;

    switch(shop1.idBank)
    {
            case gpu3d::IN:      diferentReadsToInBank++; break;
            case gpu3d::PARAM:   diferentReadsToParamBank++; break;
            case gpu3d::TEMP:    diferentReadsToTempBank++; break;
            case gpu3d::ADDR:    diferentReadsToAddrBank++; break;
            case gpu3d::INVALID: ;
    }

    switch(shop2.idBank)
    {
            case gpu3d::IN:
                if (shop1.idReg != shop2.idReg || shop1.idBank != shop2.idBank)
                        diferentReadsToInBank++;
                        break;
            case gpu3d::PARAM:
                if (shop1.idReg != shop2.idReg || shop1.idBank != shop2.idBank)
                        diferentReadsToParamBank++;
                        break;
            case gpu3d::TEMP:
                if (shop1.idReg != shop2.idReg || shop1.idBank != shop2.idBank)
                        diferentReadsToTempBank++;
                        break;
            case gpu3d::ADDR:
                if (shop1.idReg != shop2.idReg || shop1.idBank != shop2.idBank)
                        diferentReadsToAddrBank++;
                        break;
            case gpu3d::INVALID: ;
    }

    if (num_operands == 3)
    {
        switch(shop3.idBank)
        {
                case gpu3d::IN:    if ((shop1.idReg != shop3.idReg || shop1.idBank != shop3.idBank) &&
                                        (shop2.idReg != shop3.idReg || shop2.idBank != shop3.idBank))
                                        diferentReadsToInBank++;
                                        break;
                case gpu3d::PARAM: if ((shop1.idReg != shop3.idReg || shop1.idBank != shop3.idBank) &&
                                        (shop2.idReg != shop3.idReg || shop2.idBank != shop3.idBank))
                                        diferentReadsToParamBank++;
                                        break;
                case gpu3d::TEMP:    if ((shop1.idReg != shop3.idReg || shop1.idBank != shop2.idBank) &&
                                         (shop2.idReg != shop3.idReg || shop2.idBank != shop3.idBank))
                                        diferentReadsToTempBank++;
                                        break;
                case gpu3d::ADDR:    if ((shop1.idReg != shop3.idReg || shop1.idBank != shop3.idBank) &&
                                         (shop2.idReg != shop3.idReg || shop2.idBank != shop3.idBank))
                                        diferentReadsToAddrBank++;
                                        break;
                case gpu3d::INVALID: ;
        }
    }

    switch(MAXIMUM((diferentReadsToInBank - readPortsInBank),0))
    {
        case 1: preloadOp1 = true; break;
        case 2: preloadOp1 = true; preloadOp2 = true; break;
        default: break;
    }

    switch(MAXIMUM((diferentReadsToParamBank - readPortsParamBank),0))
    {
        case 1: preloadOp1 = true; break;
        case 2: preloadOp1 = true; preloadOp2 = true; break;
        default: break;
    }

    switch(MAXIMUM((diferentReadsToTempBank - readPortsTempBank),0))
    {
        case 1: preloadOp1 = true; break;
        case 2: preloadOp1 = true; preloadOp2 = true; break;
        default: break;
    }

    switch(MAXIMUM((diferentReadsToAddrBank - readPortsAddrBank),0))
    {
        case 1: preloadOp1 = true; break;
        case 2: preloadOp1 = true; preloadOp2 = true; break;
        default: break;
    }


    if (shop1.idBank == PARAM && shop2.idBank == PARAM &&
        shop1.relativeModeFlag && shop2.relativeModeFlag)
    {
       /* Operand1 and Operand2 access to parameter registers with and relative
        * address register. Check if they access to the same register
        * with the same address register, component register and register offset.
        */
        if (shop1.idReg != shop2.idReg ||
            shop1.relModeAddrReg != shop2.relModeAddrReg ||
            shop1.relModeAddrComp != shop2.relModeAddrComp ||
            shop1.relModeOffset != shop2.relModeOffset) preloadOp1 = true;
    }

    if (num_operands == 3)
    {
        if (shop1.idBank == PARAM && shop3.idBank == PARAM &&
            shop1.relativeModeFlag && shop3.relativeModeFlag)
        {
           /* Operand1 and Operand3 access to parameter registers with and relative
            * address register. Check if they access to the same register
            * with the same address register, component register and register offset.
            */
            if (shop1.idReg != shop3.idReg ||
                shop1.relModeAddrReg != shop3.relModeAddrReg ||
                shop1.relModeAddrComp != shop3.relModeAddrComp ||
                shop1.relModeOffset != shop3.relModeOffset) preloadOp1 = true;
        }
        if (shop2.idBank == PARAM && shop3.idBank == PARAM &&
            shop2.relativeModeFlag && shop3.relativeModeFlag)
        {
           /* Operand2 and Operand3 access to parameter registers with and relative
            * address register. Check if they access to the same register
            * with the same address register, component register and register offset.
            * If not we preload Operand2.
            */
            if (shop2.idReg != shop3.idReg ||
                shop2.relModeAddrReg != shop3.relModeAddrReg ||
                shop2.relModeAddrComp != shop3.relModeAddrComp ||
                shop2.relModeOffset != shop3.relModeOffset) preloadOp2 = true;
        }
    }
    return (preloadOp1 || preloadOp2);
}


list<ShaderInstruction*> ShaderInstructionTranslator::preloadOperands(int line,
                                                                      ShaderInstrOperand& shop1,
                                                                      ShaderInstrOperand& shop2,
                                                                      ShaderInstrOperand& shop3,
                                                                      bool preloadOp1, bool preloadOp2,
                                                                      unsigned preloadReg1, unsigned preloadReg2) const
{
    list<ShaderInstruction*> code;
    ShaderInstrResult aux;

    aux.idBank = TEMP;
    aux.writeMask = mXYZW;

    if (preloadOp1) /* Operand 1 has to be preloaded to a temporary register */
    {
        aux.idReg = preloadReg1;
        code.push_back(new ShaderInstruction(MOV, shop1.idBank, shop1.idReg,
                                             shop1.negateFlag, shop1.absoluteFlag,
                                             shop1.swizzleMode,
                                             INVALID,0,false,false,XYZW,
                                             INVALID,0,false,false,XYZW,
                                             aux.idBank, aux.idReg, false, aux.writeMask,
                                             false, false, 0,
                                             shop1.relativeModeFlag,
                                             shop1.relModeAddrReg,
                                             shop1.relModeAddrComp,
                                             shop1.relModeOffset));
        shop1.idReg = aux.idReg,
        shop1.idBank = aux.idBank;
        shop1.swizzleMode = XYZW;
        shop1.absoluteFlag = false;
        shop1.negateFlag = false;
        shop1.relativeModeFlag = false;
    }

    if (preloadOp2)  /* Operand 2 has to be preloaded to a temporary register */
    {
        aux.idReg = preloadReg2;
        code.push_back(new ShaderInstruction(MOV, shop2.idBank, shop2.idReg,
                                             shop2.negateFlag, shop2.absoluteFlag,
                                             shop2.swizzleMode,
                                             INVALID,0,false,false,XYZW,
                                             INVALID,0,false,false,XYZW,
                                             aux.idBank, aux.idReg, false, aux.writeMask,
                                             false, false, 0,
                                             shop2.relativeModeFlag,
                                             shop2.relModeAddrReg,
                                             shop2.relModeAddrComp,
                                             shop2.relModeOffset));
        shop2.idReg = aux.idReg,
        shop2.idBank = aux.idBank;
        shop2.swizzleMode = XYZW;
        shop2.absoluteFlag = false;
        shop2.negateFlag = false;
        shop2.relativeModeFlag = false;
    }
    return code;
}

void ShaderInstructionTranslator::translateOperandReferences(const GenericInstruction::OperandInfo& genop, ShaderInstrOperand& shop) const
{
    shop.swizzleMode = (SwizzleMode)genop.swizzleMode;
    shop.negateFlag = genop.negateFlag;
    shop.absoluteFlag = false;

    shop.idBank = (Bank) genop.idBank;
    shop.idReg = genop.idReg;

    if (genop.arrayAccessModeFlag && genop.relativeModeFlag)
    {
        shop.relativeModeFlag = true;
        shop.relModeAddrReg = (u32bit)genop.relModeAddrReg;
        shop.relModeAddrComp = (u32bit)genop.relModeAddrComp;
        shop.relModeOffset = (s16bit)genop.relModeOffset;
    }
    else // Not array access mode
    {
        shop.relativeModeFlag = false;
    }
}

void ShaderInstructionTranslator::translateResultReferences(const GenericInstruction::ResultInfo& genres, ShaderInstrResult& shres) const
{
    shres.idReg = genres.idReg;
    shres.idBank = (Bank)genres.idBank;
    shres.writeMask = (MaskMode)genres.writeMask;
}

ShOpcode ShaderInstructionTranslator::translateOpcode(GenericInstruction::Opcode opc, bool &saturated, bool &texture, bool &sample)
{
    ShOpcode opcode;
    
    saturated = false;
    texture = false;
    sample = false;
    
    switch(opc)
    {
        case GenericInstruction::G_ADD:
            opcode = ADD;
            break;    
        case GenericInstruction::G_ADD_SAT:
            opcode = ADD;
            saturated = true;
            break;
        case GenericInstruction::G_ARL:
            opcode = ARL;            
            break;
        case GenericInstruction::G_CMP:
            opcode = CMP;
            break;
        case GenericInstruction::G_CMP_SAT:
            opcode = CMP;
            saturated = true;
            break;
        case GenericInstruction::G_COS:
            opcode = COS;
            break;
        case GenericInstruction::G_COS_SAT:
            opcode = COS;
            saturated = true;
            break;
        case GenericInstruction::G_DP3:
            opcode = DP3;
            break;
        case GenericInstruction::G_DP3_SAT:
            opcode = DP3;
            saturated = true;
            break;
        case GenericInstruction::G_DP4:
            opcode = DP4;
            break;
        case GenericInstruction::G_DP4_SAT:
            opcode = DP4;
            saturated = true;
        case GenericInstruction::G_DPH:
            opcode = DPH;
            break;
        case GenericInstruction::G_DPH_SAT:
            opcode = DPH;
            saturated = true;
            break;
        case GenericInstruction::G_DST:
            opcode = DST;
            break;
        case GenericInstruction::G_DST_SAT:
            opcode = DST;
            saturated = true;
            break;
        case GenericInstruction::G_EX2:
            opcode = EX2;
            break;
        case GenericInstruction::G_EX2_SAT:
            opcode = EX2;
            saturated = true;
            break;
        case GenericInstruction::G_EXP:
            opcode = EXP;
            break;
        case GenericInstruction::G_FLR:
            opcode = FLR;
            break;
        case GenericInstruction::G_FLR_SAT:
            opcode = FLR;
            saturated = true;
        case GenericInstruction::G_FRC:
            opcode = FRC;
            break;
        case GenericInstruction::G_FRC_SAT:
            opcode = FRC;
            saturated = true;
            break;
        case GenericInstruction::G_KIL:
            opcode = KIL;
            break;
        case GenericInstruction::G_LG2:
            opcode = LG2;
            break;
        case GenericInstruction::G_LG2_SAT:
            opcode = LG2;
            saturated = true;
            break;
        case GenericInstruction::G_LIT:
            opcode = LIT;
            break;
        case GenericInstruction::G_LIT_SAT:
            opcode = LIT;
            saturated = true;
            break;
        case GenericInstruction::G_LOG:
            opcode = LOG;
            break;
        case GenericInstruction::G_MAD:
            opcode = MAD;
            break;
         case GenericInstruction::G_MAD_SAT:
            opcode = MAD;
            saturated = true;
            break;
        case GenericInstruction::G_MAX:
            opcode = MAX;
            break;
        case GenericInstruction::G_MAX_SAT:
            opcode = MAX;
            saturated = true;
            break;
        case GenericInstruction::G_MIN:
            opcode = MIN;
            break;
        case GenericInstruction::G_MIN_SAT:
            opcode = MIN;
            saturated = true;
            break;
        case GenericInstruction::G_MOV:
            opcode = MOV;
            break;
        case GenericInstruction::G_MOV_SAT:
            opcode = MOV;
            saturated = true;
            break;
        case GenericInstruction::G_MUL:
            opcode = MUL;
            break;
        case GenericInstruction::G_MUL_SAT:
            opcode = MUL;
            saturated = true;
            break;
        case GenericInstruction::G_RCP:
            opcode = RCP;
            break;
        case GenericInstruction::G_RCP_SAT:
            opcode = RCP;
            saturated = true;
            break;
        case GenericInstruction::G_RSQ:
            opcode = RSQ;
            break;
        case GenericInstruction::G_RSQ_SAT:
            opcode = RSQ;
            saturated = true;
            break;
        case GenericInstruction::G_SGE:
            opcode = SGE;
            break;
        case GenericInstruction::G_SGE_SAT:
            opcode = SGE;
            saturated = true;
            break;
        case GenericInstruction::G_SIN:
            opcode = SIN;
            break;
        case GenericInstruction::G_SIN_SAT:
            opcode = SIN;
            saturated = true;
            break;
        case GenericInstruction::G_SLT:
            opcode = SLT;
            break;
        case GenericInstruction::G_SLT_SAT:
            opcode = SLT;
            saturated = true;
            break;
        case GenericInstruction::G_TEX:
            opcode = TEX;
            texture = true;
            break;
        case GenericInstruction::G_TEX_SAT:
            opcode = TEX;
            texture = true;
            saturated = true;
            break;
        case GenericInstruction::G_TXB:
            opcode = TXB;
            texture = true;
            break;
        case GenericInstruction::G_TXB_SAT:
            opcode = TXB;
            texture = true;
            saturated = true;
            break;
        case GenericInstruction::G_TXP:
            opcode = TXP;
            texture = true;
            break;
        case GenericInstruction::G_TXP_SAT:
            opcode = TXP;
            texture = true;
            saturated = true;
            break;
        default:
            panic("ShaderInstructionTranslator", "translateOpcode", "Undefined opcode");
            break;
    }
    
    return opcode;
}


list<ShaderInstruction*> ShaderInstructionTranslator::translateInstruction(GenericInstruction::Opcode opcode, int line, int num_operands,
                                                                           ShaderInstrOperand shop1,
                                                                           ShaderInstrOperand shop2,
                                                                           ShaderInstrOperand shop3,
                                                                           ShaderInstrResult shres,
                                                                           unsigned int textureImageUnit,
                                                                           bool lastInstruction)
{
    list<ShaderInstruction*> code;
    ShaderInstrOperand *relop;

    if (shop1.relativeModeFlag && num_operands >0) relop = &shop1;
    else if (shop2.relativeModeFlag && num_operands >1) relop = &shop2;
    else if (shop3.relativeModeFlag && num_operands >2) relop = &shop3;
    else relop = &shop1;

    ShOpcode opcodeTranslated;
    bool saturated;
    bool texture; 
    bool sample;
    
    opcodeTranslated = translateOpcode(opcode, saturated, texture, sample);
    
    if (texture)
    {
        shop2.idBank = TEXT;
        shop2.idReg = textureImageUnit;
        shop2.negateFlag = false;
        shop2.absoluteFlag = false;
        shop2.swizzleMode = XYZW;
    }
       
    code.push_back(new ShaderInstruction(opcodeTranslated,
                                         shop1.idBank, shop1.idReg, shop1.negateFlag, shop1.absoluteFlag, shop1.swizzleMode,
                                         shop2.idBank, shop2.idReg, shop2.negateFlag, shop2.absoluteFlag, shop2.swizzleMode,
                                         shop3.idBank, shop3.idReg, shop3.negateFlag, shop3.absoluteFlag,  shop3.swizzleMode,
                                         shres.idBank, shres.idReg, saturated, shres.writeMask,
                                         false, false, 0,
                                         relop->relativeModeFlag, relop->relModeAddrReg, relop->relModeAddrComp, relop->relModeOffset,
                                         lastInstruction));

/*    switch(opcode)
    {
        case GenericInstruction::G_ADD:
                code.push_back(new ShaderInstruction(ADD,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_ADD_SAT:
                code.push_back(new ShaderInstruction(ADD,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_ARL:
                code.push_back(new ShaderInstruction(ARL,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_CMP:
                code.push_back(new ShaderInstruction(CMP,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_CMP_SAT:
                code.push_back(new ShaderInstruction(CMP,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_COS:
                code.push_back(new ShaderInstruction(COS,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_COS_SAT:
                code.push_back(new ShaderInstruction(COS,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_DP3:
                code.push_back(new ShaderInstruction(DP3,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
                case GenericInstruction::G_DP3_SAT:
                code.push_back(new ShaderInstruction(DP3,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_DP4:
                code.push_back(new ShaderInstruction(DP4,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_DP4_SAT:
                code.push_back(new ShaderInstruction(DP4,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_DPH:
                code.push_back(new ShaderInstruction(DPH,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_DPH_SAT:
                code.push_back(new ShaderInstruction(DPH,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_DST:
                code.push_back(new ShaderInstruction(DST,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg,  false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_DST_SAT:
                code.push_back(new ShaderInstruction(DST,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg,  true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_EX2:
                code.push_back(new ShaderInstruction(EX2,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_EX2_SAT:
                code.push_back(new ShaderInstruction(EX2,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_EXP:
                code.push_back(new ShaderInstruction(EXP,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_FLR:
                code.push_back(new ShaderInstruction(FLR,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_FLR_SAT:
                code.push_back(new ShaderInstruction(FLR,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_FRC:
                code.push_back(new ShaderInstruction(FRC,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_FRC_SAT:
                code.push_back(new ShaderInstruction(FRC,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_KIL:
                code.push_back(new ShaderInstruction(KIL,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_LG2:
                code.push_back(new ShaderInstruction(LG2,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            false,
                                            lastInstruction)); break;
        case GenericInstruction::G_LG2_SAT:
                code.push_back(new ShaderInstruction(LG2,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_LIT:
                code.push_back(new ShaderInstruction(LIT,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_LIT_SAT:
                code.push_back(new ShaderInstruction(LIT,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_LOG:
                code.push_back(new ShaderInstruction(LOG,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MAD:
                code.push_back(new ShaderInstruction(MAD,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MAD_SAT:
                code.push_back(new ShaderInstruction(MAD,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MAX:
                code.push_back(new ShaderInstruction(MAX,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MAX_SAT:
                code.push_back(new ShaderInstruction(MAX,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MIN:
                code.push_back(new ShaderInstruction(MIN,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MIN_SAT:
                code.push_back(new ShaderInstruction(MIN,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MOV:
                code.push_back(new ShaderInstruction(MOV,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MOV_SAT:
                code.push_back(new ShaderInstruction(MOV,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MUL:
                code.push_back(new ShaderInstruction(MUL,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_MUL_SAT:
                code.push_back(new ShaderInstruction(MUL,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_RCP:
                code.push_back(new ShaderInstruction(RCP,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_RCP_SAT:
                code.push_back(new ShaderInstruction(RCP,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_RSQ:
                code.push_back(new ShaderInstruction(RSQ,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_RSQ_SAT:
                code.push_back(new ShaderInstruction(RSQ,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_SGE:
                code.push_back(new ShaderInstruction(SGE,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_SGE_SAT:
                code.push_back(new ShaderInstruction(SGE,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_SIN:
                code.push_back(new ShaderInstruction(SIN,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_SIN_SAT:
                code.push_back(new ShaderInstruction(SIN,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_SLT:
                code.push_back(new ShaderInstruction(SLT,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_SLT_SAT:
                code.push_back(new ShaderInstruction(SLT,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            shop2.idBank, shop2.idReg,
                                            shop2.negateFlag, shop2.absoluteFlag,
                                            shop2.swizzleMode,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_TEX:
                code.push_back(new ShaderInstruction(TEX,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            (Bank)0x6, textureImageUnit, false, false, XYZW,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_TEX_SAT:
                code.push_back(new ShaderInstruction(TEX,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            (Bank)0x6, textureImageUnit, false, false, XYZW,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_TXB:
                code.push_back(new ShaderInstruction(TXB,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            (Bank)0x6, textureImageUnit, false, false, XYZW,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_TXB_SAT:
                code.push_back(new ShaderInstruction(TXB,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            (Bank)0x6, textureImageUnit, false, false, XYZW,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_TXP:
                code.push_back(new ShaderInstruction(TXP,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            (Bank)0x6, textureImageUnit, false, false, XYZW,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, false,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;
        case GenericInstruction::G_TXP_SAT:
                code.push_back(new ShaderInstruction(TXB,
                                            shop1.idBank, shop1.idReg,
                                            shop1.negateFlag, shop1.absoluteFlag,
                                            shop1.swizzleMode,
                                            (Bank)0x6, textureImageUnit, false, false, XYZW,
                                            shop3.idBank, shop3.idReg,
                                            shop3.negateFlag, shop3.absoluteFlag,
                                            shop3.swizzleMode,
                                            shres.idBank, shres.idReg, true,
                                            shres.writeMask,
                                            relop->relativeModeFlag,
                                            relop->relModeAddrReg,
                                            relop->relModeAddrComp,
                                            relop->relModeOffset,
                                            lastInstruction)); break;

        default:
                        panic("ShaderInstructionTranslator","translateInstruction()","Instruction Opcode not expected");
    }*/
    
    return code;
}

/* ***********************************
 * composeSwizzles() Auxiliar function
 * ***********************************
 *  This function combines(composes) the swizzles modes swz1 and swz2
 *  doing the following operation:
 *  resultSwizzle[firstcomponent]  = swz1[swz2[firstcomponent]];
 *  resultSwizzle[secondcomponent] = swz1[swz2[secondcomponent]];
 *  resultSwizzle[thirdcomponent]  = swz1[swz2[thirdcomponent]];
 *  resultSwizzle[fourthcomponent] = swz1[swz2[fourthcomponent]];
 *
 *  For example: if swz1 = XYZZ and swz2 = XZWX then
 *  the swizzle result is:
 *      resultSwizzle[X] = swz1[X] ( = X )
 *      resultSwizzle[Y] = swz1[Z] ( = Z )
 *      resultSwizzle[Z] = swz1[W] ( = Z )
 *      resultSwizzle[W] = swz1[X] ( = X )
 */

SwizzleMode ShaderInstructionTranslator::composeSwizzles(SwizzleMode swz1, SwizzleMode swz2) const
{
    SwizzleMode ret = (SwizzleMode)0;
    unsigned int swzcomponent1[4];
    unsigned int swzcomponent2[4];

    swzcomponent1[0] = ((unsigned int)swz1 & 0xC0) >> 6;
    swzcomponent1[1] = ((unsigned int)swz1 & 0x30) >> 4;
    swzcomponent1[2] = ((unsigned int)swz1 & 0x0C) >> 2;
    swzcomponent1[3] = (unsigned int)swz1 & 0x03;

    swzcomponent2[0] = ((unsigned int)swz2 & 0xC0) >> 6;
    swzcomponent2[1] = ((unsigned int)swz2 & 0x30) >> 4;
    swzcomponent2[2] = ((unsigned int)swz2 & 0x0C) >> 2;
    swzcomponent2[3] = (unsigned int)swz2 & 0x03;

    ret = (SwizzleMode)((int)ret + ((swzcomponent1[swzcomponent2[0]] & (0x03)) << 6));
    ret = (SwizzleMode)((int)ret + ((swzcomponent1[swzcomponent2[1]] & (0x03)) << 4));
    ret = (SwizzleMode)((int)ret + ((swzcomponent1[swzcomponent2[2]] & (0x03)) << 2));
    ret = (SwizzleMode)((int)ret + ((swzcomponent1[swzcomponent2[3]] & (0x03)) << 0));

    return ret;
}

/*  How to emulate Generic instructions with NV VS 2.0 instructions:
 *  Ref (ARB1_0_vertex_program.txt) Interactions with NV_vertex_program section, line 4976
 *   "- ARB programs support the folowing instructions not supported by NV
 *      "VP1.0" programs:
 *
 *          * ABS:  absolute value.  Supported on VP1.1 NV programs, but not
 *            on VP1.0 programs.  Equivalent to "MAX dst, src, -src".
 *
 *          * EX2:  exponential base 2.  On VP1.0 and VP1.1 hardware, this
 *            instruction will be emulated using EXP and a number of
 *            additional instructions.
 *
 *          * FLR:  floor.  On VP1.0 and VP1.1 hardware, this instruction will
 *            be emulated using an EXP and an ADD instruction.
 *
 *          * FRC:  fraction.  On VP1.0 and VP1.1 hardware, this instruction
 *            will be emulated using an EXP instruction, and possibly a MOV
 *            instruction to replicate the scalar result.
 *
 *          * LG2:  logarithm base 2.  On VP1.0 and VP1.1 hardware, this
 *            instruction will be emulated using LOG and a number of
 *            additional instructions.
 *
 *          * POW:  exponentiation.  On VP1.0 and VP1.1 hardware, this
 *            instruction will be emulated using LOG, MUL, and EXP
 *            instructions, and possibly additional instructions to generate a
 *            high-precision result.
 *
 *          * SUB:  subtraction.  Supported on VP1.1 NV programs, but not on
 *            VP1.0 programs.  Equivalent to "ADD dst, src1, -src2".
 *
 *          * SWZ:  extended swizzle.  On VP1.0 and VP1.1 hardware, this
 *            instruction will be emulated using a single MAD instruction and
 *            a program parameter constant.
 *
 *          * XPD:  cross product.  On VP1.0 and VP1.1 hardware, this
 *            instruction will be emulated using a MUL and a MAD instruction.
 *
 *  Emulation of fragment program instructions:
 *  Ref (ARB1_0_fragment_program.txt) Section: Interactions with NV_fragment_program
 *          line 4147
 *
 *    - ARB_fragment_program provides several instructions not found in
 *      NV_fragment_program, and there are a few instruction set
 *      differences:
 *
 *        * CMP:  compare.  Roughly equivalent to the following
 *            sequence, but may be optimized further:
 *
 *              SLT tmp, src0;
 *              LRP dst, tmp, src1, src2;
 *              or
 *              SLT tmp, src0;
 *              ADD tmp, tmp, -src2;
 *              MAD dst, tmp, src1, src2;
 *
 *        * SCS:  sine/cosine.  Emulated using the separate SIN and COS
 *            instructions in NV_fragment_program, which also have no
 *            restriction on the input values.
 *
 *        * LRP: linear interpolation. Roughly equivalent to the following
 *            sequence:
 *              ADD dst, src1, -src2;
 *              MAD dst, dst, src0, src2;
 *
 */

list<ShaderInstruction*> ShaderInstructionTranslator::generateEquivalentInstructionList(
                                                               GenericInstruction::Opcode opcode,
                                                               int line,int num_operands,
                                                               ShaderInstrOperand shop1,
                                                               ShaderInstrOperand shop2,
                                                               ShaderInstrOperand shop3,
                                                               ShaderInstrResult shres,
                                                               GenericInstruction::SwizzleInfo swz,
                                                               bool& calculatedPreloadReg1,
                                                               unsigned int& preloadReg1,
                                                               bool lastInstruction)
{
    list<ShaderInstruction*> code;
    ShaderInstrOperand *relop;

    if (shop1.relativeModeFlag && num_operands >0) relop = &shop1;
    else if (shop2.relativeModeFlag && num_operands >1) relop = &shop2;
    else if (shop3.relativeModeFlag && num_operands >2) relop = &shop3;
    else relop = &shop1;

    switch(opcode){
        case GenericInstruction::G_ABS:
            /* Ron Fosner: "Real Time Shader Programming", Editorial Morgan Kaufmann
             * 2003. Chapter 8: Shader Reference, pag 205:
             * abs macro equivalent shader code:
             * "Equivalent instructions to: ABS Dest0, Source0:
             *          max Dest0, Source0, -Source0
             */
                code.push_back(new ShaderInstruction(MAX,
                                                     shop1.idBank, shop1.idReg,
                                                     shop1.negateFlag, shop1.absoluteFlag,
                                                     shop1.swizzleMode,
                                                     shop1.idBank, shop1.idReg,
                                                     !shop1.negateFlag, shop1.absoluteFlag,
                                                     shop1.swizzleMode,
                                                     INVALID,0,false,false,XYZW,
                                                     shres.idBank, shres.idReg, false,
                                                     shres.writeMask,
                                                     false, false, 0,
                                                     relop->relativeModeFlag,
                                                     relop->relModeAddrReg,
                                                     relop->relModeAddrComp,
                                                     relop->relModeOffset,
                                                     lastInstruction));
                break;
        case GenericInstruction::G_ABS_SAT:
                code.push_back(new ShaderInstruction(MAX,
                                                     shop1.idBank, shop1.idReg,
                                                     shop1.negateFlag, shop1.absoluteFlag,
                                                     shop1.swizzleMode,
                                                     shop1.idBank, shop1.idReg,
                                                     !shop1.negateFlag, shop1.absoluteFlag,
                                                     shop1.swizzleMode,
                                                     INVALID,0,false,false,XYZW,
                                                     shres.idBank, shres.idReg, true,
                                                     shres.writeMask,
                                                     false, false, 0,
                                                     relop->relativeModeFlag,
                                                     relop->relModeAddrReg,
                                                     relop->relModeAddrComp,
                                                     relop->relModeOffset,
                                                     lastInstruction));
                break;

/* Code to emulate CMP instruction with SLT, MAD and ADD

            QuadReg<float> value(0.0f,0.0f,0.0f,0.0f);
            if (!calculatedPreloadReg1) {
                preloadReg1 = tempBank.findFree();
                tempBank.set(preloadReg1,value);
                calculatedPreloadReg1 = true;
            }
            code.push_back(new ShaderInstruction(SLT, shop1.idBank, shop1.idReg,
                              shop1.negateFlag, shop1.absoluteFlag,
                              shop1.swizzleMode,
                              shop2.idBank,shop2.idReg,
                              shop2.negateFlag, shop2.absoluteFlag,
                              shop2.swizzleMode,
                              shop3.idBank, shop3.idReg, shop1.negateFlag,
                              shop3.absoluteFlag,shop3.swizzleMode,
                              TEMP, preloadReg1, false, mXYZW,
                              TR,XYZW,
                              shop2.relativeModeFlag,
                              shop2.relModeAddrReg,
                              shop2.relModeAddrComp,
                              shop2.relModeOffset,
                              false));

            code.push_back(new ShaderInstruction(ADD, TEMP, preloadReg1,
                              false, false,XYZW,
                              shop3.idBank, shop3.idReg, !shop1.negateFlag,
                              shop3.absoluteFlag,shop3.swizzleMode,
                              INVALID,0,false,false,XYZW,
                              TEMP, preloadReg1, false, mXYZW,
                              TR,XYZW,
                              shop1.relativeModeFlag,
                              shop1.relModeAddrReg,
                              shop1.relModeAddrComp,
                              shop1.relModeOffset,
                              false));

            code.push_back(new ShaderInstruction(MAD, TEMP, preloadReg1,
                              false, false,XYZW,
                              shop2.idBank,shop2.idReg,
                              shop2.negateFlag, shop2.absoluteFlag,
                              shop2.swizzleMode,
                              shop3.idBank, shop3.idReg, shop1.negateFlag,
                              shop3.absoluteFlag,shop3.swizzleMode,
                              shres.idBank, shres.idReg, false, shres.writeMask,
                              TR,XYZW,
                              shop2.relativeModeFlag,
                              shop2.relModeAddrReg,
                              shop2.relModeAddrComp,
                              shop2.relModeOffset,
                              false));

            break;
        }
*/
/* Code to emulate CMP instruction with SLT, MAD and ADD
        {
            QuadReg<float> value(0.0f,0.0f,0.0f,0.0f);
            if (!calculatedPreloadReg1) {
                preloadReg1 = tempBank.findFree();
                tempBank.set(preloadReg1,value);
                calculatedPreloadReg1 = true;
            }
            code.push_back(new ShaderInstruction(SLT, shop1.idBank, shop1.idReg,
                              shop1.negateFlag, shop1.absoluteFlag,
                              shop1.swizzleMode,
                              shop2.idBank,shop2.idReg,
                              shop2.negateFlag, shop2.absoluteFlag,
                              shop2.swizzleMode,
                              shop3.idBank, shop3.idReg, shop1.negateFlag,
                              shop3.absoluteFlag,shop3.swizzleMode,
                              TEMP, preloadReg1, false, mXYZW,
                              TR,XYZW,
                              shop2.relativeModeFlag,
                              shop2.relModeAddrReg,
                              shop2.relModeAddrComp,
                              shop2.relModeOffset,
                              false));

            code.push_back(new ShaderInstruction(ADD, TEMP, preloadReg1,
                              false, false,XYZW,
                              shop3.idBank, shop3.idReg, !shop1.negateFlag,
                              shop3.absoluteFlag,shop3.swizzleMode,
                              INVALID,0,false,false,XYZW,
                              TEMP, preloadReg1, false, mXYZW,
                              TR,XYZW,
                              shop1.relativeModeFlag,
                              shop1.relModeAddrReg,
                              shop1.relModeAddrComp,
                              shop1.relModeOffset,
                              false));

            code.push_back(new ShaderInstruction(MAD, TEMP, preloadReg1,
                              false, false,XYZW,
                              shop2.idBank,shop2.idReg,
                              shop2.negateFlag, shop2.absoluteFlag,
                              shop2.swizzleMode,
                              shop3.idBank, shop3.idReg, shop1.negateFlag,
                              shop3.absoluteFlag,shop3.swizzleMode,
                              shres.idBank, shres.idReg, true, shres.writeMask,
                              TR,XYZW,
                              shop2.relativeModeFlag,
                              shop2.relModeAddrReg,
                              shop2.relModeAddrComp,
                              shop2.relModeOffset,
                              false));

            break;
        }
*/
        case GenericInstruction::G_LRP:
        {
            QuadReg<float> value(0.0f,0.0f,0.0f,0.0f);
            if (!calculatedPreloadReg1) {
                preloadReg1 = tempBank.findFree();
                tempBank.set(preloadReg1,value);
                calculatedPreloadReg1 = true;
            }
            code.push_back(new ShaderInstruction(ADD,
                                                 shop2.idBank, shop2.idReg,
                                                 shop2.negateFlag, shop2.absoluteFlag,
                                                 shop2.swizzleMode,
                                                 shop3.idBank, shop3.idReg, !shop3.negateFlag,
                                                 shop3.absoluteFlag,shop3.swizzleMode,
                                                 INVALID,0,false,false,XYZW,
                                                 TEMP, preloadReg1, false, mXYZW,
                                                 false, false, 0,
                                                 shop2.relativeModeFlag,
                                                 shop2.relModeAddrReg,
                                                 shop2.relModeAddrComp,
                                                 shop2.relModeOffset));

            code.push_back(new ShaderInstruction(MAD,
                                                 TEMP, preloadReg1,
                                                 false, false,XYZW,
                                                 shop1.idBank, shop1.idReg,
                                                 shop1.negateFlag, shop1.absoluteFlag,
                                                 shop1.swizzleMode,
                                                 shop3.idBank, shop3.idReg, shop3.negateFlag,
                                                 shop3.absoluteFlag,shop3.swizzleMode,
                                                 shres.idBank, shres.idReg, false, shres.writeMask,
                                                 false, false, 0,
                                                 shop1.relativeModeFlag,
                                                 shop1.relModeAddrReg,
                                                 shop1.relModeAddrComp,
                                                 shop1.relModeOffset,
                                                 lastInstruction));
            break;
        }

        case GenericInstruction::G_LRP_SAT:
        {
            QuadReg<float> value(0.0f,0.0f,0.0f,0.0f);
            if (!calculatedPreloadReg1) {
                preloadReg1 = tempBank.findFree();
                tempBank.set(preloadReg1,value);
                calculatedPreloadReg1 = true;
            }
            code.push_back(new ShaderInstruction(ADD,
                                                 shop2.idBank, shop2.idReg,
                                                 shop2.negateFlag, shop2.absoluteFlag,
                                                 shop2.swizzleMode,
                                                 shop3.idBank, shop3.idReg, !shop3.negateFlag,
                                                 shop3.absoluteFlag,shop3.swizzleMode,
                                                 INVALID,0,false,false,XYZW,
                                                 TEMP, preloadReg1, false, mXYZW,
                                                 false, false, 0,
                                                 shop2.relativeModeFlag,
                                                 shop2.relModeAddrReg,
                                                 shop2.relModeAddrComp,
                                                 shop2.relModeOffset));

            code.push_back(new ShaderInstruction(MAD,
                                                 TEMP, preloadReg1,
                                                 false, false,XYZW,
                                                 shop1.idBank, shop1.idReg,
                                                 shop1.negateFlag, shop1.absoluteFlag,
                                                 shop1.swizzleMode,
                                                 shop3.idBank, shop3.idReg, shop3.negateFlag,
                                                 shop3.absoluteFlag,shop3.swizzleMode,
                                                 shres.idBank, shres.idReg, true, shres.writeMask,
                                                 false, false, 0,
                                                 shop1.relativeModeFlag,
                                                 shop1.relModeAddrReg,
                                                 shop1.relModeAddrComp,
                                                 shop1.relModeOffset,
                                                 lastInstruction));
            break;
        }
        case GenericInstruction::G_POW:
        {
            /* Ron Fosner: "Real Time Shader Programming", Editorial Morgan Kaufmann
             * 2003. Chapter 8: Shader Reference, pag 320:
             * Pow macro equivalent shader code:
             * " The math used is b^x = 2^(x*log2(b)).
             *   Equivalent instructions to: POW Dest0.mask, Source0.scalar1, Source1.scalar2:
             *      LG2 Dest0.mask, Source0.scalar1
             *      MUL Dest0.mask, Dest0.swz*, Source1.scalar2
             *      EXP Dest0.mask, Dest0.swz*
             *
             *       * one of the active components in "mask"
             */


            code.push_back(new ShaderInstruction(LG2,
                                                 shop1.idBank, shop1.idReg,
                                                 shop1.negateFlag, shop1.absoluteFlag,
                                                 shop1.swizzleMode,
                                                 INVALID,0,false,false,XYZW,
                                                 INVALID,0,false,false,XYZW,
                                                 shres.idBank, shres.idReg, false, shres.writeMask,
                                                 false, false, 0,
                                                 shop1.relativeModeFlag,
                                                 shop1.relModeAddrReg,
                                                 shop1.relModeAddrComp,
                                                 shop1.relModeOffset));

            // Choose one of the destination register components selected by the writeMask
            // to work with.

            bool found = false;
            u32bit component = (u32bit) XNNN; // component = 0x08;
            u8bit component_id = 0x00; // component_id = X (0x00);

            while (component > NNNN && !found)
            {
                if ((shres.writeMask & component) == component)
                    found = true;
                else
                {
                    component = component >> 1; // component = NYNN(0x04), component = NNZN(0x02), component = NNNW (0x01) ...
                    component_id++; // component_id = Y (0x01), component_id = Z (0x02), component_id = W (0x11),
                }
            }

            // Use the first component found as writable by the instruction to construct the properly mask

            u32bit swizzleMask = (component_id << 6) + (component_id << 4) + (component_id << 2) + (component_id);

            code.push_back(new ShaderInstruction(MUL,
                                                 shres.idBank, shres.idReg,
                                                 false, false,(SwizzleMode)swizzleMask,
                                                 shop2.idBank,shop2.idReg,
                                                 shop2.negateFlag, shop2.absoluteFlag,
                                                 shop2.swizzleMode,
                                                 INVALID,0,false,false,XYZW,
                                                 shres.idBank, shres.idReg, false, shres.writeMask,
                                                 false, false, 0,
                                                 shop2.relativeModeFlag,
                                                 shop2.relModeAddrReg,
                                                 shop2.relModeAddrComp,
                                                 shop2.relModeOffset));
            code.push_back(new ShaderInstruction(EX2,
                                                 shres.idBank, shres.idReg,
                                                 false, false,(SwizzleMode)swizzleMask,
                                                 INVALID,0,false,false,XYZW,
                                                 INVALID,0,false,false,XYZW,
                                                 shres.idBank, shres.idReg, false, shres.writeMask,
                                                 false, false, 0,
                                                 false,0,0,0, lastInstruction));

            break;
        }
        case GenericInstruction::G_SCS:
            /*
             * 3.11.5.23  SCS:  Sine/Cosine
             *
             * The SCS instruction approximates the trigonometric sine and cosine
             * of the angle specified by the scalar operand and places the cosine
             * in the x component and the sine in the y component of the result
             * vector.  The z and w components of the result vector are undefined.
             * The angle is specified in radians and must be in the range [-PI,PI].
             *
             *   tmp = ScalarLoad(op0);
             *   result.x = ApproxCosine(tmp);
             *   result.y = ApproxSine(tmp);
             *
             * If the scalar operand is not in the range [-PI,PI], the result
             * vector is undefined.
             *
             */
                code.push_back(new ShaderInstruction(COS,
                                                     shop1.idBank, shop1.idReg,
                                                     shop1.negateFlag, shop1.absoluteFlag,
                                                     shop1.swizzleMode,
                                                     shop2.idBank, shop2.idReg,
                                                     shop2.negateFlag, shop2.absoluteFlag,
                                                     shop2.swizzleMode,
                                                     shop3.idBank, shop3.idReg,
                                                     shop3.negateFlag, shop3.absoluteFlag,
                                                     shop3.swizzleMode,
                                                     shres.idBank, shres.idReg, false,
                                                     XNNN,
                                                     false, false, 0,
                                                     relop->relativeModeFlag,
                                                     relop->relModeAddrReg,
                                                     relop->relModeAddrComp,
                                                     relop->relModeOffset,
                                                     lastInstruction));

                code.push_back(new ShaderInstruction(SIN,
                                                     shop1.idBank, shop1.idReg,
                                                     shop1.negateFlag, shop1.absoluteFlag,
                                                     shop1.swizzleMode,
                                                     shop2.idBank, shop2.idReg,
                                                     shop2.negateFlag, shop2.absoluteFlag,
                                                     shop2.swizzleMode,
                                                     shop3.idBank, shop3.idReg,
                                                     shop3.negateFlag, shop3.absoluteFlag,
                                                     shop3.swizzleMode,
                                                     shres.idBank, shres.idReg, false,
                                                     NYNN,
                                                     false, false, 0,
                                                     relop->relativeModeFlag,
                                                     relop->relModeAddrReg,
                                                     relop->relModeAddrComp,
                                                     relop->relModeOffset,
                                                     lastInstruction));
            break;
        case GenericInstruction::G_SUB:
                /* SUB instruction is equivalent to ADD instruction with the second
                 * operand negated:
                 *  SUB Dest0, Source0, Source1 --> ADD Dest0, Source0, -Source1
                 */
                code.push_back(new ShaderInstruction(ADD,
                                                     shop1.idBank, shop1.idReg,
                                                     shop1.negateFlag, shop1.absoluteFlag,
                                                     shop1.swizzleMode,
                                                     shop2.idBank, shop2.idReg,
                                                     !shop2.negateFlag, shop2.absoluteFlag,
                                                     shop2.swizzleMode,
                                                     INVALID,0,false,false,XYZW,
                                                     shres.idBank, shres.idReg, false,
                                                     shres.writeMask,
                                                     false, false, 0,
                                                     relop->relativeModeFlag,
                                                     relop->relModeAddrReg,
                                                     relop->relModeAddrComp,
                                                     relop->relModeOffset,
                                                     lastInstruction));
                break;
        case GenericInstruction::G_SUB_SAT:
                code.push_back(new ShaderInstruction(ADD,
                                                     shop1.idBank, shop1.idReg,
                                                     shop1.negateFlag, shop1.absoluteFlag,
                                                     shop1.swizzleMode,
                                                     shop2.idBank, shop2.idReg,
                                                     !shop2.negateFlag, shop2.absoluteFlag,
                                                     shop2.swizzleMode,
                                                     INVALID,0,false,false,XYZW,
                                                     shres.idBank, shres.idReg, true,
                                                     shres.writeMask,
                                                     false, false, 0,
                                                     relop->relativeModeFlag,
                                                     relop->relModeAddrReg,
                                                     relop->relModeAddrComp,
                                                     relop->relModeOffset,
                                                     lastInstruction));
                break;
        case GenericInstruction::G_SWZ:
        {
            /* We create the {0.0, 1.0, -1.0, 0.0} vector. With this vector we
             * can create any possible mask by swizzling. For example, if we
             * want to construct a mask for negate the first three components and
             * null the fourth, we can swizzle like this:
             *  {0.0, 1.0, -1.0, 0.0}.zzzx;
             * With one only mask and the MUL instruction, we can negate operands
             * (-x, -y, -z or -w with -1.0), let the same operand with a 1.0 and
             * null an operand with 0.0 but we can´t specify a 1.0/-1.0 value yet.
             *  This can be done adding to the result a second mask (the same operand
             * constant but swizzled different to avoid spend a new param bank
             * register). To do it in a single instruction we make use of MAD.
             * For example, for instruction: SWZ Dest0, Source0, -x, -1, 0, z
             * the equivalent MAD instruction is:
             *
             *  C[0] = { 0.0, 1.0, -1.0, 0.0 };
             *  MAD Dest0, Source0.xxxz, C[0].zxxy, C[0].xzxx;
             *
             *  We don´t need to compose Source0 original swizzle mode with the
             * swizzle mode required to emulate the generic instruction with a MAD
             * instruction, because SWZ doesn´t not allow to specify a swizzle mode
             * modificator and this is checked by the grammar.
             */
            QuadReg<float> value(0.0f,1.0f,-1.0f,0.0f);

            /* We construct the first swizzle mask */

            SwizzleMode swizzleMask1;
            SwizzleMode swizzleMask2;
            SwizzleMode swizzleMask3;

            switch(swz.xSelect)
            {
                case GenericInstruction::ZERO: swizzleMask1 = (SwizzleMode)0;
                       swizzleMask2 = (SwizzleMode)0;
                       swizzleMask3 = (SwizzleMode)0;
                       break;
                case GenericInstruction::ONE:  swizzleMask1 = (SwizzleMode)0;
                       swizzleMask2 = (SwizzleMode)0;
                       if (swz.xNegate) swizzleMask3 = (SwizzleMode)2;
                       else swizzleMask3 = (SwizzleMode)1;
                       break;
                case GenericInstruction::X:    swizzleMask1 = (SwizzleMode)0;
                       if (swz.xNegate) swizzleMask2 = (SwizzleMode)2;
                       else swizzleMask2 = (SwizzleMode)1;
                       swizzleMask3 = (SwizzleMode)0;
                       break;
                case GenericInstruction::Y:    swizzleMask1 = (SwizzleMode)1;
                       if (swz.xNegate) swizzleMask2 = (SwizzleMode)2;
                       else swizzleMask2 = (SwizzleMode)1;
                       swizzleMask3 = (SwizzleMode)0;
                       break;
                case GenericInstruction::Z:    swizzleMask1 = (SwizzleMode)2;
                       if (swz.xNegate) swizzleMask2 = (SwizzleMode)2;
                       else swizzleMask2 = (SwizzleMode)1;
                       swizzleMask3 = (SwizzleMode)0;
                       break;
                case GenericInstruction::W:    swizzleMask1 = (SwizzleMode)3;
                       if (swz.xNegate) swizzleMask2 = (SwizzleMode)2;
                       else swizzleMask2 = (SwizzleMode)1;
                       swizzleMask3 = (SwizzleMode)0;
                       break;
            }

            switch(swz.ySelect)
            {
                case GenericInstruction::ZERO: swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 2));
                       swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((0 & 0x03) << 2));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 2));
                       break;
                case GenericInstruction::ONE:  swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 2));
                       swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((0 & 0x03) << 2));
                       if (swz.yNegate)
                        swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((2 & 0x03) << 2));
                       else swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((1 & 0x03) << 2));
                       break;
                case GenericInstruction::X:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 2));
                       if (swz.yNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 2));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 2));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 2));
                       break;
                case GenericInstruction::Y:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((1 & 0x03) << 2));
                       if (swz.yNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 2));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 2));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 2));
                       break;
                case GenericInstruction::Z:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((2 & 0x03) << 2));
                       if (swz.yNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 2));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 2));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 2));
                       break;
                case GenericInstruction::W:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((3 & 0x03) << 2));
                       if (swz.yNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 2));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 2));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 2));
                       break;
            }

            switch(swz.zSelect)
            {
                case GenericInstruction::ZERO: swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 4));
                       swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((0 & 0x03) << 4));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 4));
                       break;
                case GenericInstruction::ONE:  swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 4));
                       swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((0 & 0x03) << 4));
                       if (swz.zNegate)
                        swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((2 & 0x03) << 4));
                       else swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((1 & 0x03) << 4));
                       break;
                case GenericInstruction::X:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 4));
                       if (swz.zNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 4));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 4));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 4));
                       break;
                case GenericInstruction::Y:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((1 & 0x03) << 4));
                       if (swz.zNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 4));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 4));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 4));
                       break;
                case GenericInstruction::Z:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((2 & 0x03) << 4));
                       if (swz.zNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 4));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 4));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 4));
                       break;
                case GenericInstruction::W:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((3 & 0x03) << 4));
                       if (swz.zNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 4));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 4));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 4));
                       break;
            }

            switch(swz.wSelect)
            {
                case GenericInstruction::ZERO: swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 6));
                       swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((0 & 0x03) << 6));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 6));
                       break;
                case GenericInstruction::ONE:  swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 6));
                       swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((0 & 0x03) << 6));
                       if (swz.wNegate)
                        swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((2 & 0x03) << 6));
                       else swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((1 & 0x03) << 6));
                       break;
                case GenericInstruction::X:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((0 & 0x03) << 6));
                       if (swz.wNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 6));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 6));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 6));
                       break;
                case GenericInstruction::Y:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((1 & 0x03) << 6));
                       if (swz.wNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 6));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 6));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 6));
                       break;
                case GenericInstruction::Z:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((2 & 0x03) << 6));
                       if (swz.wNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 6));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 6));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 6));
                       break;
                case GenericInstruction::W:    swizzleMask1 = (SwizzleMode)((int)swizzleMask1 + ((3 & 0x03) << 6));
                       if (swz.wNegate)
                        swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((2 & 0x03) << 6));
                       else swizzleMask2 = (SwizzleMode)((int)swizzleMask2 + ((1 & 0x03) << 6));
                       swizzleMask3 = (SwizzleMode)((int)swizzleMask3 + ((0 & 0x03) << 6));
                       break;
            }

            bool found;

            GLuint constantReg = paramBank.getRegister(found,value,QR_CONSTANT);

            if (!found)
            {
                paramBank.set(constantReg,value);
                paramBank.setType(constantReg,QR_CONSTANT);
            }

            code.push_back(new ShaderInstruction(MAD,
                                                 shop1.idBank, shop1.idReg,
                                                 false, false,
                                                 swizzleMask1,
                                                 PARAM, constantReg, false, false, swizzleMask2,
                                                 PARAM, constantReg, false, false, swizzleMask3,
                                                 shres.idBank, shres.idReg, false, shres.writeMask,
                                                 false, false, 0,
                                                 shop1.relativeModeFlag,
                                                 shop1.relModeAddrReg,
                                                 shop1.relModeAddrComp,
                                                 shop1.relModeOffset,
                                                 lastInstruction));
            break;
        }

        case GenericInstruction::G_XPD:

            /* The equivalent instruction code to: XPD Dest0, Source0, Source1:
             *      MUL Dest0.xyz, Source0.zxy, Source1.yzx;
             *      MAD Dest0.xyz, Source0.yzx, Source1.zxy, -Dest0.xyz;
             * We need to compose the original Source Operands swizzle masks with
             * the our specific swizzles mask to emulate the XPD instruction. This
             * is done by the auxiliar function called composeSwizzles().
             */

            code.push_back(new ShaderInstruction(MUL,
                                                 shop1.idBank, shop1.idReg,
                                                 shop1.negateFlag, shop1.absoluteFlag,
                                                 composeSwizzles(shop1.swizzleMode,ZXYY),
                                                 shop2.idBank, shop2.idReg,
                                                 shop2.negateFlag, shop2.absoluteFlag,
                                                 composeSwizzles(shop2.swizzleMode,YZXX),
                                                 INVALID,0,false,false,XYZW,
                                                 shres.idBank, shres.idReg, false, XYZN,
                                                 false, false, 0,
                                                 shop1.relativeModeFlag,
                                                 shop1.relModeAddrReg,
                                                 shop1.relModeAddrComp,
                                                 shop1.relModeOffset));

            code.push_back(new ShaderInstruction(MAD,
                                                 shop1.idBank,
                                                 shop1.idReg,
                                                 shop1.negateFlag, shop1.absoluteFlag,
                                                 composeSwizzles(shop1.swizzleMode,YZXX),
                                                 shop2.idBank, shop2.idReg,
                                                 shop2.negateFlag, shop2.absoluteFlag,
                                                 composeSwizzles(shop2.swizzleMode,ZXYY),
                                                 shres.idBank, shres.idReg, true, false, XYZZ,
                                                 shres.idBank, shres.idReg, false, shres.writeMask,
                                                 false, false, 0,
                                                 shop2.relativeModeFlag,
                                                 shop2.relModeAddrReg,
                                                 shop2.relModeAddrComp,
                                                 shop2.relModeOffset,
                                                 lastInstruction));
            break;

        default:
            panic("ShaderInstructionTranslator","generateEquivalentInstructionList()","Instruction Opcode unexpected");
    }
    return code;
}



void ShaderInstructionTranslator::translateCode( list<GenericInstruction*>& lgi, bool optimize, bool reorderCode)
{
    if (!shaderCode.empty()) clean(); /* clean current shader code */

    list<GenericInstruction*>::iterator actual = lgi.begin();

    ShaderInstrOperand shop1,shop2,shop3;
    ShaderInstrResult  shres;

    bool preloadOp1, preloadOp2;
    unsigned int preloadReg1 = 0;
    unsigned int preloadReg2 = 0;
    bool calculatedPreloadReg1 = false;
    bool calculatedPreloadReg2 = false;

    while (actual != lgi.end())
    {
        /* Per-instruction generic-independent to hardware-dependent translate */

        if ((*actual)->getNumOperands()>0) /* We don´t translate NOP generic instructions */
        {
            translateOperandReferences((*actual)->getOp1(), shop1);

            if ((*actual)->getNumOperands()>1)
            {
                translateOperandReferences((*actual)->getOp2(), shop2);

                if ((*actual)->getNumOperands()>2)
                    translateOperandReferences((*actual)->getOp3(), shop3);
            }

            if ((*actual)->getOpcode() != GenericInstruction::G_KIL)
                translateResultReferences((*actual)->getRes(), shres);

            if (thereAreOperandAccessConflicts(shop1,shop2,shop3,(*actual)->getNumOperands(),
                                               preloadOp1,preloadOp2))
            {
                QuadReg<float> value(0.0f,0.0f,0.0f,0.0f);

                if (preloadOp1 && !calculatedPreloadReg1)
                {
                    preloadReg1 = tempBank.findFree();
                    tempBank.set(preloadReg1,value);
                    calculatedPreloadReg1 = true;
                }

                if (preloadOp2 && !calculatedPreloadReg2)
                {
                    preloadReg2 = tempBank.findFree();
                    tempBank.set(preloadReg2,value);
                    calculatedPreloadReg2 = true;
                }

                list<ShaderInstruction*> preloadCode = preloadOperands((*actual)->getLine(), shop1,shop2,shop3,
                                                                       preloadOp1, preloadOp2,
                                                                       preloadReg1, preloadReg2);

                shaderCode.splice(shaderCode.end(), preloadCode);


                /* Important: There is an update of shader instruction operands
                 *            information.
                 */
            }

            if (!thereIsEquivalentInstruction((*actual)->getOpcode()))  /* Instruction not supported. It has to be emulated */
            {
                list<ShaderInstruction*> equivalent = generateEquivalentInstructionList((*actual)->getOpcode(),
                                                                                        (*actual)->getLine(),
                                                                                        (*actual)->getNumOperands(),
                                                                                        shop1,shop2,shop3,shres,
                                                                                        (*actual)->getSwz(),
                                                                                        calculatedPreloadReg1,
                                                                                        preloadReg1,
                                                                                        (*actual)->getIsLastInstruction());
                shaderCode.splice(shaderCode.end(), equivalent);

            }
            else
            { /**
               * There is an equivalent instruction that do the same in the
               * hardware dependent instruction set with the same operands
               * requeriments and there is no required to generate additional
               * instructions or reinterpret operand semantics.
               */
                list<ShaderInstruction*> translated = translateInstruction((*actual)->getOpcode(),
                                                                              (*actual)->getLine(),
                                                                              (*actual)->getNumOperands(),
                                                                               shop1,shop2,shop3,
                                                                               shres,
                                                                            (*actual)->getTextureImageUnit(),
                                                                            (*actual)->getIsLastInstruction());
                shaderCode.splice(shaderCode.end(), translated);

            }
        }
        actual++;
    }

        /* Optimization phase. Various reordering code optimizations are applied */
    if (optimize) applyOptimizations(reorderCode);

    /* Our GPU vertex shader requires a final sequence of END NOP NOP
     * instructions to work properly. They are generated here
     */

    //shaderCode.push_back(new ShaderInstruction(END));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));

    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));

    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));

    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
    shaderCode.push_back(new ShaderInstruction(NOP));
}

void ShaderInstructionTranslator::returnGPUCode(u8bit *bin, u32bit& size)
{
    if ( shaderCode.size()*16 > size )
        panic("ShaderInstructionTranslator","returnTranslated()", "Buffer overflow");

    list<ShaderInstruction*>::iterator it = shaderCode.begin();
    size = 0;
    for ( ; it != shaderCode.end(); it++ )
    {
        (*it)->getCode(bin + size);
        size += 16;
    }
}

u32bit ShaderInstructionTranslator::getMaxAliveTemps()
{
    if ( maxLiveTempsComputed )
        return maxLiveTemps;
    panic("ShaderInstructionTranslator", "getMaxAliveTemps", "ApplyOptimizations must be called before this method");
    return 0;
}

void ShaderInstructionTranslator::printTranslated(ostream& os)
{
    u8bit assm[4096*4];
    u32bit size = sizeof(assm);
    returnASMCode(assm,size);
    os << assm;
}

void ShaderInstructionTranslator::returnASMCode(u8bit *ptr, u32bit& size)
{
    char instr[128];
    *ptr = 0;
    list<ShaderInstruction*>::iterator it = shaderCode.begin();
    u32bit totalSize = 0;

    for ( ; it != shaderCode.end(); it++ )
    {
        (*it)->disassemble(instr);
        totalSize += (strlen(instr)+1);
        if ( totalSize >= size )
            panic("ShaderInstructionTranslator","returnASMCode()", "Not enough size in output buffer");
        strcat((char *)ptr, instr);
        strcat((char *)ptr, "\n");
    }
}

void ShaderInstructionTranslator::clean()
{
    list<ShaderInstruction*>::iterator iter = shaderCode.begin();

    while( iter != shaderCode.end() )
    {
        delete (*iter);
        iter++;
    }

    shaderCode.clear();
}

void ShaderInstructionTranslator::applyOptimizations(bool reorderCode)
{
    Scheduler schd;

    // Try to generate optimized code
    list<ShaderInstruction*> optimized = schd.optimizeCode(shaderCode, driver->getFetchRate(), 16, driver->getMaxVertexAttribs(), driver->getMaxAddrRegisters(), reorderCode);
    if ( !optimized.empty() ) {
        clean();
        shaderCode = optimized; // If optimized code is available, replace the current code with the optimized version
    }

    maxLiveTempsComputed = true;
    maxLiveTemps = schd.getMaxAliveTemps();
}

ShaderInstructionTranslator::~ShaderInstructionTranslator()
{
    clean();
}

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

#include "GenericInstruction.h"
#include <cstdio>
#include <cstring>

using namespace std;
using namespace libgl::GenerationCode;
using namespace libgl;


GenericInstruction::GenericInstruction(unsigned int line, const string opcodeString, 
                       GenericInstruction::OperandInfo op1, 
                       GenericInstruction::OperandInfo op2, 
                       GenericInstruction::OperandInfo op3,
                       GenericInstruction::ResultInfo res,
                       GenericInstruction::SwizzleInfo swz, 
                       unsigned int texImageUnit,
                       GenericInstruction::TextureTarget texTarget,
                       bool lastInstruction)
                       
    : line(line), opcodeString(opcodeString), op1(op1), op2(op2), op3(op3), res(res),
      swz(swz), textureImageUnit(texImageUnit), textureTarget(texTarget), 
      lastInstruction(lastInstruction)
{
    if      (!opcodeString.compare("ABS"))      { opcode =  GenericInstruction::G_ABS; num_operands = 1; }
    else if (!opcodeString.compare("ADD"))      { opcode =  GenericInstruction::G_ADD; num_operands = 2; }
    else if (!opcodeString.compare("ARL"))      { opcode =  GenericInstruction::G_ARL; num_operands = 1; }
    else if (!opcodeString.compare("CMP"))      { opcode =  GenericInstruction::G_CMP; num_operands = 3; }
    else if (!opcodeString.compare("COS"))      { opcode =  GenericInstruction::G_COS; num_operands = 1; }
    else if (!opcodeString.compare("DP3"))      { opcode =  GenericInstruction::G_DP3; num_operands = 2; }  
    else if (!opcodeString.compare("DP4"))      { opcode =  GenericInstruction::G_DP4; num_operands = 2; }
    else if (!opcodeString.compare("DPH"))      { opcode =  GenericInstruction::G_DPH; num_operands = 2; }
    else if (!opcodeString.compare("DST"))      { opcode =  GenericInstruction::G_DST; num_operands = 2; }
    else if (!opcodeString.compare("EX2"))      { opcode =  GenericInstruction::G_EX2; num_operands = 1; }
    else if (!opcodeString.compare("EXP"))      { opcode =  GenericInstruction::G_EXP; num_operands = 1; }
    else if (!opcodeString.compare("FLR"))      { opcode =  GenericInstruction::G_FLR; num_operands = 1; }
    else if (!opcodeString.compare("FRC"))      { opcode =  GenericInstruction::G_FRC; num_operands = 1; }
    else if (!opcodeString.compare("KIL"))      { opcode =  GenericInstruction::G_KIL; num_operands = 1; }
    else if (!opcodeString.compare("LG2"))      { opcode =  GenericInstruction::G_LG2; num_operands = 1; }
    else if (!opcodeString.compare("LIT"))      { opcode =  GenericInstruction::G_LIT; num_operands = 1; }
    else if (!opcodeString.compare("LOG"))      { opcode =  GenericInstruction::G_LOG; num_operands = 1; }
    else if (!opcodeString.compare("LRP"))      { opcode =  GenericInstruction::G_LRP; num_operands = 3; }
    else if (!opcodeString.compare("MAD"))      { opcode =  GenericInstruction::G_MAD; num_operands = 3; }
    else if (!opcodeString.compare("MAX"))      { opcode =  GenericInstruction::G_MAX; num_operands = 2; }
    else if (!opcodeString.compare("MIN"))      { opcode =  GenericInstruction::G_MIN; num_operands = 2; }
    else if (!opcodeString.compare("MOV"))      { opcode =  GenericInstruction::G_MOV; num_operands = 1; }
    else if (!opcodeString.compare("MUL"))      { opcode =  GenericInstruction::G_MUL; num_operands = 2; }
    else if (!opcodeString.compare("POW"))      { opcode =  GenericInstruction::G_POW; num_operands = 2; }
    else if (!opcodeString.compare("RCP"))      { opcode =  GenericInstruction::G_RCP; num_operands = 1; }
    else if (!opcodeString.compare("RSQ"))      { opcode =  GenericInstruction::G_RSQ; num_operands = 1; }
    else if (!opcodeString.compare("SGE"))      { opcode =  GenericInstruction::G_SGE; num_operands = 2; }
    else if (!opcodeString.compare("SLT"))      { opcode =  GenericInstruction::G_SLT; num_operands = 2; }
    else if (!opcodeString.compare("SUB"))      { opcode =  GenericInstruction::G_SUB; num_operands = 2; }
    else if (!opcodeString.compare("SWZ"))      { opcode =  GenericInstruction::G_SWZ; num_operands = 1; }
    else if (!opcodeString.compare("TEX"))      { opcode =  GenericInstruction::G_TEX; num_operands = 1; }
    else if (!opcodeString.compare("TXB"))      { opcode =  GenericInstruction::G_TXB; num_operands = 1; }
    else if (!opcodeString.compare("TXP"))      { opcode =  GenericInstruction::G_TXP; num_operands = 1; }
    else if (!opcodeString.compare("XPD"))      { opcode =  GenericInstruction::G_XPD; num_operands = 2; }
    else if (!opcodeString.compare("SCS"))      { opcode =  GenericInstruction::G_SCS; num_operands = 1; }
    else if (!opcodeString.compare("SIN"))      { opcode =  GenericInstruction::G_SIN; num_operands = 1; }
    else if (!opcodeString.compare("ABS_SAT"))  { opcode =  GenericInstruction::G_ABS_SAT; num_operands = 1; }
    else if (!opcodeString.compare("ADD_SAT"))  { opcode =  GenericInstruction::G_ADD_SAT; num_operands = 2; }
    else if (!opcodeString.compare("CMP_SAT"))  { opcode =  GenericInstruction::G_CMP_SAT; num_operands = 3; }
    else if (!opcodeString.compare("COS_SAT"))  { opcode =  GenericInstruction::G_COS_SAT; num_operands = 1; }
    else if (!opcodeString.compare("DP3_SAT"))  { opcode =  GenericInstruction::G_DP3_SAT; num_operands = 2; }  
    else if (!opcodeString.compare("DP4_SAT"))  { opcode =  GenericInstruction::G_DP4_SAT; num_operands = 2; }
    else if (!opcodeString.compare("DPH_SAT"))  { opcode =  GenericInstruction::G_DPH_SAT; num_operands = 2; }
    else if (!opcodeString.compare("DST_SAT"))  { opcode =  GenericInstruction::G_DST_SAT; num_operands = 2; }
    else if (!opcodeString.compare("EX2_SAT"))  { opcode =  GenericInstruction::G_EX2_SAT; num_operands = 1; }
    else if (!opcodeString.compare("FLR_SAT"))  { opcode =  GenericInstruction::G_FLR_SAT; num_operands = 1; }
    else if (!opcodeString.compare("FRC_SAT"))  { opcode =  GenericInstruction::G_FRC_SAT; num_operands = 1; }
    else if (!opcodeString.compare("LG2_SAT"))  { opcode =  GenericInstruction::G_LG2_SAT; num_operands = 1; }
    else if (!opcodeString.compare("LIT_SAT"))  { opcode =  GenericInstruction::G_LIT_SAT; num_operands = 1; }
    else if (!opcodeString.compare("LRP_SAT"))  { opcode =  GenericInstruction::G_LRP_SAT; num_operands = 3; }
    else if (!opcodeString.compare("MAD_SAT"))  { opcode =  GenericInstruction::G_MAD_SAT; num_operands = 3; }
    else if (!opcodeString.compare("MAX_SAT"))  { opcode =  GenericInstruction::G_MAX_SAT; num_operands = 2; }
    else if (!opcodeString.compare("MIN_SAT"))  { opcode =  GenericInstruction::G_MIN_SAT; num_operands = 2; }
    else if (!opcodeString.compare("MOV_SAT"))  { opcode =  GenericInstruction::G_MOV_SAT; num_operands = 1; }
    else if (!opcodeString.compare("MUL_SAT"))  { opcode =  GenericInstruction::G_MUL_SAT; num_operands = 2; }
    else if (!opcodeString.compare("POW_SAT"))  { opcode =  GenericInstruction::G_POW_SAT; num_operands = 2; }
    else if (!opcodeString.compare("RCP_SAT"))  { opcode =  GenericInstruction::G_RCP_SAT; num_operands = 1; }
    else if (!opcodeString.compare("RSQ_SAT"))  { opcode =  GenericInstruction::G_RSQ_SAT; num_operands = 1; }
    else if (!opcodeString.compare("SCS_SAT"))  { opcode =  GenericInstruction::G_SCS_SAT; num_operands = 1; }
    else if (!opcodeString.compare("SGE_SAT"))  { opcode =  GenericInstruction::G_SGE_SAT; num_operands = 2; }
    else if (!opcodeString.compare("SIN_SAT"))  { opcode =  GenericInstruction::G_SIN_SAT; num_operands = 1; }
    else if (!opcodeString.compare("SLT_SAT"))  { opcode =  GenericInstruction::G_SLT_SAT; num_operands = 2; }
    else if (!opcodeString.compare("SUB_SAT"))  { opcode =  GenericInstruction::G_SUB_SAT; num_operands = 2; }
    else if (!opcodeString.compare("SWZ_SAT"))  { opcode =  GenericInstruction::G_SWZ_SAT; num_operands = 1; }
    else if (!opcodeString.compare("TEX_SAT"))  { opcode =  GenericInstruction::G_TEX_SAT; num_operands = 1; }
    else if (!opcodeString.compare("TXB_SAT"))  { opcode =  GenericInstruction::G_TXB_SAT; num_operands = 1; }
    else if (!opcodeString.compare("TXP_SAT"))  { opcode =  GenericInstruction::G_TXP_SAT; num_operands = 1; }
    else if (!opcodeString.compare("XPD_SAT"))  { opcode =  GenericInstruction::G_XPD_SAT; num_operands = 2; }
    else { opcode = INVALID_OP; num_operands = 0; }
}

void GenericInstruction::writeOperand(unsigned int numOperand, GenericInstruction::OperandInfo op, std::ostream& os) const
{
    char aux[100];
    char operand[10000];
    
    sprintf(aux,"\t\tOperand %d:\n",numOperand);
    strcpy(operand,aux);
    
    sprintf(aux,"\t\t\tArrayAccessModeFlag: %d\n",(int)op.arrayAccessModeFlag);
    strcat(operand,aux);
    
    if (op.arrayAccessModeFlag && !op.relativeModeFlag)
    {
        sprintf(aux,"\t\t\t\tArrayModeOffset: %d\n",(int)op.arrayModeOffset);
        strcat(operand,aux);
    }
    
    sprintf(aux,"\t\t\tRelativeModeFlag: %d\n",(int)op.relativeModeFlag);
    strcat(operand,aux);
    
    if (op.relativeModeFlag)
    {
        sprintf(aux,"\t\t\t\tRelModeAddrReg: %u\n",op.relModeAddrReg);
        strcat(operand,aux);
        sprintf(aux,"\t\t\t\tRelModeAddrComp: (0:x|1:y|2:z|3:w) %u\n",
                                op.relModeAddrComp);
        strcat(operand,aux);
        sprintf(aux,"\t\t\t\tRelModeOffset: %d\n",(int)op.relModeOffset);
        strcat(operand,aux);
    }

    

    sprintf(aux,"\t\t\tSwizzleMode: 0x%.2X\n",(int)op.swizzleMode);
    strcat(operand,aux);
    
    sprintf(aux,"\t\t\tNegateFlag: %d\n",(int)op.negateFlag);
    strcat(operand,aux);
    
    sprintf(aux,"\t\t\tIdReg: %d\n",(int)op.idReg);
    strcat(operand,aux);
    
    sprintf(aux,"\t\t\tIdBank: %d\n",(int)op.idBank);
    strcat(operand,aux);
    
    os << operand; 
    
}

void GenericInstruction::writeResult(GenericInstruction::ResultInfo res, std::ostream& os) const 
{
    char aux[100];
    char result[10000];
    
    sprintf(aux,"\t\tResult:\n");
    strcpy(result,aux);
    
    sprintf(aux,"\t\t\tWriteMask: 0x%.1X\n",(int)res.writeMask);
    strcat(result,aux);
    
    sprintf(aux,"\t\t\tIdReg: %d\n",(int)res.idReg);
    strcat(result,aux);
    
    sprintf(aux,"\t\t\tIdBank: %d\n",(int)res.idBank);
    strcat(result,aux);
    
    os << result;
}

void GenericInstruction::writeSwizzle(GenericInstruction::SwizzleInfo swz, std::ostream& os) const
{
    char aux[100];
    char swizzle[10000];
    
    sprintf(aux,"\t\tSwizzle Info: Select(0: 0.0|1: 1.0|2: x|3: y|4: z|5: w)\n");
    strcpy(swizzle,aux);
    
    sprintf(aux,"\t\t\txSelect: %d Negate: %d\n",(int)swz.xSelect,(int)swz.xNegate);
    strcat(swizzle,aux);
    
    sprintf(aux,"\t\t\tySelect: %d Negate: %d\n",(int)swz.ySelect,(int)swz.yNegate);
    strcat(swizzle,aux);
    
    sprintf(aux,"\t\t\tzSelect: %d Negate: %d\n",(int)swz.zSelect,(int)swz.zNegate);
    strcat(swizzle,aux);
    
    sprintf(aux,"\t\t\twSelect: %d Negate: %d\n",(int)swz.wSelect,(int)swz.wNegate);
    strcat(swizzle,aux);
    
    os << swizzle; 
}

void GenericInstruction::writeTextureTarget(GenericInstruction::TextureTarget texTarget, std::ostream& os) const
{
    os << "\t\tTexture Target: ";
    
    switch(textureTarget)
    {
        case TEXTURE_1D: os << "TEXTURE_1D" << endl; break;
        case TEXTURE_2D: os << "TEXTURE_1D" << endl; break;
        case TEXTURE_3D: os << "TEXTURE_1D" << endl; break;
        case CUBE_MAP: os << "CUBE_MAP" << endl; break;
        case RECT_MAP: os << "RECT" << endl; break;
    }
}

void GenericInstruction::writeTextureImageUnit(unsigned int texImageUnit, std::ostream& os) const
{
    os << "\t\tTexture Image Unit: " << texImageUnit << endl;
}

void GenericInstruction::print(std::ostream& os) const
{
    
    char instr[1000];
    char aux[100];

    if (opcode != INVALID_OP) 
    {
        sprintf(aux,"[Line: %d] ",line);
        strcpy(instr,aux);
        os << opcodeString;
        if (lastInstruction)
            os << "(LAST)";
        if (opcode != G_KIL)
            writeResult(res,os);
        writeOperand(1,op1,os);
        switch(opcode)
        {
            case GenericInstruction::G_ABS: break;
            case GenericInstruction::G_ADD: writeOperand(2,op2,os); break;
            case GenericInstruction::G_ARL: break;
            case GenericInstruction::G_CMP: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case GenericInstruction::G_COS: break;
            case GenericInstruction::G_DP3: writeOperand(2,op2,os); break;
            case GenericInstruction::G_DP4: writeOperand(2,op2,os); break;
            case GenericInstruction::G_DPH: writeOperand(2,op2,os); break;
            case GenericInstruction::G_DST: writeOperand(2,op2,os); break;
            case GenericInstruction::G_EX2: break;
            case GenericInstruction::G_EXP: break;
            case GenericInstruction::G_FLR: break;
            case GenericInstruction::G_FRC: break;
            case GenericInstruction::G_KIL: break;
            case GenericInstruction::G_LG2: break;
            case GenericInstruction::G_LIT: break;
            case GenericInstruction::G_LOG: break;
            case GenericInstruction::G_LRP: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case GenericInstruction::G_MAD: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case GenericInstruction::G_MAX: writeOperand(2,op2,os); break;
            case GenericInstruction::G_MIN: writeOperand(2,op2,os); break;
            case GenericInstruction::G_MOV: break;
            case GenericInstruction::G_MUL: writeOperand(2,op2,os); break;
            case GenericInstruction::G_POW: writeOperand(2,op2,os); break;
            case GenericInstruction::G_RCP: break;
            case GenericInstruction::G_RSQ: break;
            case GenericInstruction::G_SCS: break;
            case GenericInstruction::G_SGE: writeOperand(2,op2,os); break;
            case GenericInstruction::G_SIN: break;
            case GenericInstruction::G_SLT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_SUB: writeOperand(2,op2,os); break;
            case GenericInstruction::G_SWZ: writeSwizzle(swz,os); break;
            case GenericInstruction::G_TEX: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case GenericInstruction::G_TXB: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case GenericInstruction::G_TXP: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case GenericInstruction::G_XPD: writeOperand(2,op2,os); break;
            case GenericInstruction::G_ABS_SAT: break;
            case GenericInstruction::G_ADD_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_CMP_SAT: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case GenericInstruction::G_COS_SAT: break;
            case GenericInstruction::G_DP3_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_DP4_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_DPH_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_DST_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_EX2_SAT: break;
            case GenericInstruction::G_FLR_SAT: break;
            case GenericInstruction::G_FRC_SAT: break;
            case GenericInstruction::G_LG2_SAT: break;
            case GenericInstruction::G_LIT_SAT: break;
            case GenericInstruction::G_LRP_SAT: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case GenericInstruction::G_MAD_SAT: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case GenericInstruction::G_MAX_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_MIN_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_MOV_SAT: break;
            case GenericInstruction::G_MUL_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_POW_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_RCP_SAT: break;
            case GenericInstruction::G_RSQ_SAT: break;
            case GenericInstruction::G_SCS_SAT: break;
            case GenericInstruction::G_SGE_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_SIN_SAT: break;
            case GenericInstruction::G_SLT_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_SUB_SAT: writeOperand(2,op2,os); break;
            case GenericInstruction::G_SWZ_SAT: writeSwizzle(swz,os); break;
            case GenericInstruction::G_TEX_SAT: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case GenericInstruction::G_TXB_SAT: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case GenericInstruction::G_TXP_SAT: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case GenericInstruction::G_XPD_SAT: writeOperand(2,op2,os); break;
        }
    }

}

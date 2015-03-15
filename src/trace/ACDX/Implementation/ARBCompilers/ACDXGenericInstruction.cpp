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

#include "ACDXGenericInstruction.h"
#include <cstdio>
#include <cstring>

using namespace std;
using namespace acdlib::GenerationCode;
using namespace acdlib;


ACDXGenericInstruction::ACDXGenericInstruction(unsigned int line, const string opcodeString, 
                       ACDXGenericInstruction::OperandInfo op1, 
                       ACDXGenericInstruction::OperandInfo op2, 
                       ACDXGenericInstruction::OperandInfo op3,
                       ACDXGenericInstruction::ResultInfo res,
                       ACDXGenericInstruction::SwizzleInfo swz, 
                       bool isFixedPoint,
                       unsigned int texImageUnit,
                       unsigned int killSample,
                       unsigned int exportSample,
                       ACDXGenericInstruction::TextureTarget texTarget,
                       bool lastInstruction)
                       
    : line(line), opcodeString(opcodeString), op1(op1), op2(op2), op3(op3), res(res),
      swz(swz), isFixedPoint(isFixedPoint), textureImageUnit(texImageUnit), killSample(killSample), 
      exportSample(exportSample), textureTarget(texTarget), lastInstruction(lastInstruction)
{
    if      (!opcodeString.compare("ABS"))      { opcode =  ACDXGenericInstruction::G_ABS; num_operands = 1; }
    else if (!opcodeString.compare("ADD"))      { opcode =  ACDXGenericInstruction::G_ADD; num_operands = 2; }
    else if (!opcodeString.compare("ARL"))      { opcode =  ACDXGenericInstruction::G_ARL; num_operands = 1; }
    else if (!opcodeString.compare("CMP"))      { opcode =  ACDXGenericInstruction::G_CMP; num_operands = 3; }
    else if (!opcodeString.compare("CMP_KIL"))  { opcode =  ACDXGenericInstruction::G_CMPKIL; num_operands = 3; }
    else if (!opcodeString.compare("CHS"))      { opcode =  ACDXGenericInstruction::G_CHS; num_operands = 0; }
    else if (!opcodeString.compare("COS"))      { opcode =  ACDXGenericInstruction::G_COS; num_operands = 1; }
    else if (!opcodeString.compare("DP3"))      { opcode =  ACDXGenericInstruction::G_DP3; num_operands = 2; }  
    else if (!opcodeString.compare("DP4"))      { opcode =  ACDXGenericInstruction::G_DP4; num_operands = 2; }
    else if (!opcodeString.compare("DPH"))      { opcode =  ACDXGenericInstruction::G_DPH; num_operands = 2; }
    else if (!opcodeString.compare("DST"))      { opcode =  ACDXGenericInstruction::G_DST; num_operands = 2; }
    else if (!opcodeString.compare("EX2"))      { opcode =  ACDXGenericInstruction::G_EX2; num_operands = 1; }
    else if (!opcodeString.compare("EXP"))      { opcode =  ACDXGenericInstruction::G_EXP; num_operands = 1; }
    else if (!opcodeString.compare("FLR"))      { opcode =  ACDXGenericInstruction::G_FLR; num_operands = 1; }
    else if (!opcodeString.compare("FRC"))      { opcode =  ACDXGenericInstruction::G_FRC; num_operands = 1; }
    else if (!opcodeString.compare("KIL"))      { opcode =  ACDXGenericInstruction::G_KIL; num_operands = 1; }
    else if (!opcodeString.compare("KLS"))      { opcode =  ACDXGenericInstruction::G_KLS; num_operands = 1; }
    else if (!opcodeString.compare("LG2"))      { opcode =  ACDXGenericInstruction::G_LG2; num_operands = 1; }
    else if (!opcodeString.compare("LIT"))      { opcode =  ACDXGenericInstruction::G_LIT; num_operands = 1; }
    else if (!opcodeString.compare("LOG"))      { opcode =  ACDXGenericInstruction::G_LOG; num_operands = 1; }
    else if (!opcodeString.compare("LRP"))      { opcode =  ACDXGenericInstruction::G_LRP; num_operands = 3; }
    else if (!opcodeString.compare("MAD"))      { opcode =  ACDXGenericInstruction::G_MAD; num_operands = 3; }
    else if (!opcodeString.compare("MAX"))      { opcode =  ACDXGenericInstruction::G_MAX; num_operands = 2; }
    else if (!opcodeString.compare("MIN"))      { opcode =  ACDXGenericInstruction::G_MIN; num_operands = 2; }
    else if (!opcodeString.compare("MOV"))      { opcode =  ACDXGenericInstruction::G_MOV; num_operands = 1; }
    else if (!opcodeString.compare("MUL"))      { opcode =  ACDXGenericInstruction::G_MUL; num_operands = 2; }
    else if (!opcodeString.compare("POW"))      { opcode =  ACDXGenericInstruction::G_POW; num_operands = 2; }
    else if (!opcodeString.compare("RCP"))      { opcode =  ACDXGenericInstruction::G_RCP; num_operands = 1; }
    else if (!opcodeString.compare("RSQ"))      { opcode =  ACDXGenericInstruction::G_RSQ; num_operands = 1; }
    else if (!opcodeString.compare("SCS"))      { opcode =  ACDXGenericInstruction::G_SCS; num_operands = 1; }
    else if (!opcodeString.compare("SGE"))      { opcode =  ACDXGenericInstruction::G_SGE; num_operands = 2; }
    else if (!opcodeString.compare("SIN"))      { opcode =  ACDXGenericInstruction::G_SIN; num_operands = 1; }
    else if (!opcodeString.compare("SLT"))      { opcode =  ACDXGenericInstruction::G_SLT; num_operands = 2; }
    else if (!opcodeString.compare("SUB"))      { opcode =  ACDXGenericInstruction::G_SUB; num_operands = 2; }
    else if (!opcodeString.compare("SWZ"))      { opcode =  ACDXGenericInstruction::G_SWZ; num_operands = 1; }
    else if (!opcodeString.compare("TEX"))      { opcode =  ACDXGenericInstruction::G_TEX; num_operands = 1; }
    else if (!opcodeString.compare("TXB"))      { opcode =  ACDXGenericInstruction::G_TXB; num_operands = 1; }
    else if (!opcodeString.compare("TXP"))      { opcode =  ACDXGenericInstruction::G_TXP; num_operands = 1; }
    else if (!opcodeString.compare("XPD"))      { opcode =  ACDXGenericInstruction::G_XPD; num_operands = 2; }
    else if (!opcodeString.compare("ZXP"))      { opcode =  ACDXGenericInstruction::G_ZXP; num_operands = 1; }
    else if (!opcodeString.compare("ZXS"))      { opcode =  ACDXGenericInstruction::G_ZXS; num_operands = 1; }
    else if (!opcodeString.compare("FXMUL"))    { opcode =  ACDXGenericInstruction::G_FXMUL; num_operands = 2; }
    else if (!opcodeString.compare("FXMAD"))    { opcode =  ACDXGenericInstruction::G_FXMAD; num_operands = 3; }
    else if (!opcodeString.compare("FXMAD2"))   { opcode =  ACDXGenericInstruction::G_FXMAD2; num_operands = 3; }
    else if (!opcodeString.compare("ABS_SAT"))  { opcode =  ACDXGenericInstruction::G_ABS_SAT; num_operands = 1; }
    else if (!opcodeString.compare("ADD_SAT"))  { opcode =  ACDXGenericInstruction::G_ADD_SAT; num_operands = 2; }
    else if (!opcodeString.compare("CMP_SAT"))  { opcode =  ACDXGenericInstruction::G_CMP_SAT; num_operands = 3; }
    else if (!opcodeString.compare("COS_SAT"))  { opcode =  ACDXGenericInstruction::G_COS_SAT; num_operands = 1; }
    else if (!opcodeString.compare("DP3_SAT"))  { opcode =  ACDXGenericInstruction::G_DP3_SAT; num_operands = 2; }  
    else if (!opcodeString.compare("DP4_SAT"))  { opcode =  ACDXGenericInstruction::G_DP4_SAT; num_operands = 2; }
    else if (!opcodeString.compare("DPH_SAT"))  { opcode =  ACDXGenericInstruction::G_DPH_SAT; num_operands = 2; }
    else if (!opcodeString.compare("DST_SAT"))  { opcode =  ACDXGenericInstruction::G_DST_SAT; num_operands = 2; }
    else if (!opcodeString.compare("EX2_SAT"))  { opcode =  ACDXGenericInstruction::G_EX2_SAT; num_operands = 1; }
    else if (!opcodeString.compare("FLR_SAT"))  { opcode =  ACDXGenericInstruction::G_FLR_SAT; num_operands = 1; }
    else if (!opcodeString.compare("FRC_SAT"))  { opcode =  ACDXGenericInstruction::G_FRC_SAT; num_operands = 1; }
    else if (!opcodeString.compare("LG2_SAT"))  { opcode =  ACDXGenericInstruction::G_LG2_SAT; num_operands = 1; }
    else if (!opcodeString.compare("LIT_SAT"))  { opcode =  ACDXGenericInstruction::G_LIT_SAT; num_operands = 1; }
    else if (!opcodeString.compare("LRP_SAT"))  { opcode =  ACDXGenericInstruction::G_LRP_SAT; num_operands = 3; }
    else if (!opcodeString.compare("MAD_SAT"))  { opcode =  ACDXGenericInstruction::G_MAD_SAT; num_operands = 3; }
    else if (!opcodeString.compare("MAX_SAT"))  { opcode =  ACDXGenericInstruction::G_MAX_SAT; num_operands = 2; }
    else if (!opcodeString.compare("MIN_SAT"))  { opcode =  ACDXGenericInstruction::G_MIN_SAT; num_operands = 2; }
    else if (!opcodeString.compare("MOV_SAT"))  { opcode =  ACDXGenericInstruction::G_MOV_SAT; num_operands = 1; }
    else if (!opcodeString.compare("MUL_SAT"))  { opcode =  ACDXGenericInstruction::G_MUL_SAT; num_operands = 2; }
    else if (!opcodeString.compare("POW_SAT"))  { opcode =  ACDXGenericInstruction::G_POW_SAT; num_operands = 2; }
    else if (!opcodeString.compare("RCP_SAT"))  { opcode =  ACDXGenericInstruction::G_RCP_SAT; num_operands = 1; }
    else if (!opcodeString.compare("RSQ_SAT"))  { opcode =  ACDXGenericInstruction::G_RSQ_SAT; num_operands = 1; }
    else if (!opcodeString.compare("SCS_SAT"))  { opcode =  ACDXGenericInstruction::G_SCS_SAT; num_operands = 1; }
    else if (!opcodeString.compare("SGE_SAT"))  { opcode =  ACDXGenericInstruction::G_SGE_SAT; num_operands = 2; }
    else if (!opcodeString.compare("SIN_SAT"))  { opcode =  ACDXGenericInstruction::G_SIN_SAT; num_operands = 1; }
    else if (!opcodeString.compare("SLT_SAT"))  { opcode =  ACDXGenericInstruction::G_SLT_SAT; num_operands = 2; }
    else if (!opcodeString.compare("SUB_SAT"))  { opcode =  ACDXGenericInstruction::G_SUB_SAT; num_operands = 2; }
    else if (!opcodeString.compare("SWZ_SAT"))  { opcode =  ACDXGenericInstruction::G_SWZ_SAT; num_operands = 1; }
    else if (!opcodeString.compare("TEX_SAT"))  { opcode =  ACDXGenericInstruction::G_TEX_SAT; num_operands = 1; }
    else if (!opcodeString.compare("TXB_SAT"))  { opcode =  ACDXGenericInstruction::G_TXB_SAT; num_operands = 1; }
    else if (!opcodeString.compare("TXP_SAT"))  { opcode =  ACDXGenericInstruction::G_TXP_SAT; num_operands = 1; }
    else if (!opcodeString.compare("XPD_SAT"))  { opcode =  ACDXGenericInstruction::G_XPD_SAT; num_operands = 2; }
    else { opcode = INVALID_OP; num_operands = 0; }
}

void ACDXGenericInstruction::writeOperand(unsigned int numOperand, ACDXGenericInstruction::OperandInfo op, std::ostream& os) const
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

void ACDXGenericInstruction::writeResult(ACDXGenericInstruction::ResultInfo res, std::ostream& os) const 
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

void ACDXGenericInstruction::writeSwizzle(ACDXGenericInstruction::SwizzleInfo swz, std::ostream& os) const
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

void ACDXGenericInstruction::writeTextureTarget(ACDXGenericInstruction::TextureTarget texTarget, std::ostream& os) const
{
    os << "\t\tTexture Target: ";
    
    switch(texTarget)
    {
        case TEXTURE_1D: os << "TEXTURE_1D" << endl; break;
        case TEXTURE_2D: os << "TEXTURE_1D" << endl; break;
        case TEXTURE_3D: os << "TEXTURE_1D" << endl; break;
        case CUBE_MAP: os << "CUBE_MAP" << endl; break;
        case RECT_MAP: os << "RECT" << endl; break;
    }
}

void ACDXGenericInstruction::writeTextureImageUnit(unsigned int texImageUnit, std::ostream& os) const
{
    os << "\t\tTexture Image Unit: " << texImageUnit << endl;
}

void ACDXGenericInstruction::writeSample(unsigned int sample, std::ostream& os) const
{
    os << "\t\tSample: " << sample << endl;
}

void ACDXGenericInstruction::print(std::ostream& os) const
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
        if (opcode != G_KIL && opcode != G_KLS && opcode != G_ZXP && opcode != G_ZXS && opcode != G_CHS)
            writeResult(res,os);
        writeOperand(1,op1,os);
        switch(opcode)
        {
            case ACDXGenericInstruction::G_ABS: break;
            case ACDXGenericInstruction::G_ADD: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_ARL: break;
            case ACDXGenericInstruction::G_CMP: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_CMPKIL: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_CHS: break;
            case ACDXGenericInstruction::G_COS: break;
            case ACDXGenericInstruction::G_DP3: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_DP4: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_DPH: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_DST: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_EX2: break;
            case ACDXGenericInstruction::G_EXP: break;
            case ACDXGenericInstruction::G_FLR: break;
            case ACDXGenericInstruction::G_FRC: break;
            case ACDXGenericInstruction::G_KIL: break;
            case ACDXGenericInstruction::G_KLS: writeSample(exportSample, os); break;
            case ACDXGenericInstruction::G_LG2: break;
            case ACDXGenericInstruction::G_LIT: break;
            case ACDXGenericInstruction::G_LOG: break;
            case ACDXGenericInstruction::G_LRP: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_MAD: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_FXMAD: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_FXMAD2: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_MAX: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_MIN: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_MOV: break;
            case ACDXGenericInstruction::G_MUL: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_FXMUL: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_POW: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_RCP: break;
            case ACDXGenericInstruction::G_RSQ: break;
            case ACDXGenericInstruction::G_SCS: break;
            case ACDXGenericInstruction::G_SGE: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_SIN: break;
            case ACDXGenericInstruction::G_SLT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_SUB: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_SWZ: writeSwizzle(swz,os); break;
            case ACDXGenericInstruction::G_TEX: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case ACDXGenericInstruction::G_TXB: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case ACDXGenericInstruction::G_TXP: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case ACDXGenericInstruction::G_XPD: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_ZXP: break;
            case ACDXGenericInstruction::G_ZXS: writeSample(exportSample, os); break;
            case ACDXGenericInstruction::G_ABS_SAT: break;
            case ACDXGenericInstruction::G_ADD_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_CMP_SAT: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_COS_SAT: break;
            case ACDXGenericInstruction::G_DP3_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_DP4_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_DPH_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_DST_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_EX2_SAT: break;
            case ACDXGenericInstruction::G_FLR_SAT: break;
            case ACDXGenericInstruction::G_FRC_SAT: break;
            case ACDXGenericInstruction::G_LG2_SAT: break;
            case ACDXGenericInstruction::G_LIT_SAT: break;
            case ACDXGenericInstruction::G_LRP_SAT: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_MAD_SAT: writeOperand(2,op2,os); writeOperand(3,op3,os); break;
            case ACDXGenericInstruction::G_MAX_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_MIN_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_MOV_SAT: break;
            case ACDXGenericInstruction::G_MUL_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_POW_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_RCP_SAT: break;
            case ACDXGenericInstruction::G_RSQ_SAT: break;
            case ACDXGenericInstruction::G_SCS_SAT: break;
            case ACDXGenericInstruction::G_SGE_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_SIN_SAT: break;
            case ACDXGenericInstruction::G_SLT_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_SUB_SAT: writeOperand(2,op2,os); break;
            case ACDXGenericInstruction::G_SWZ_SAT: writeSwizzle(swz,os); break;
            case ACDXGenericInstruction::G_TEX_SAT: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case ACDXGenericInstruction::G_TXB_SAT: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case ACDXGenericInstruction::G_TXP_SAT: writeTextureImageUnit(textureImageUnit,os); writeTextureTarget(textureTarget,os); break;
            case ACDXGenericInstruction::G_XPD_SAT: writeOperand(2,op2,os); break;
        }
    }

}

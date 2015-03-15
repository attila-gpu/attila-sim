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

 /* Generic Instructions Justification:
  * Ref (ARB1_0_vertex_program.txt) Issues section, Issue 14, line 441
  *
  * "(14) What semantic restrictions, if any, should be imposed on using
  *  multiple vertex attributes or program parameters in the same instruction?
  *
  *  RESOLVED:  None.  If the underlying hardware implementation does not
  *  support reads of multiple attributes or program parameters, the driver
  *  may need to transparently insert additional instructions and/or consume
  *  temporaries to perform the operation."
  *
  *  Section: functional differences between ARB_vertex_program and NV_vertex_program
  *  Line 4971.
  *
  *  "ARB programs allow single instructions to source multiple distinct
  *  vertex attributes or program parameters.  NV programs do not.  On
  *  urrent NV_vertex_program hardware, such instructions may require
  *  additional instructions and temporaries to execute."
  *
  * This issues justify the need of assemble the ARBVP1.0 font code string to
  * a generic or hardware independent set of instructions that permits multiple reads 
  * of attributes and program parametres. Also, vertex programs can use a set of 
  * ARB_vertex_program instructions that has no direct equivalence in hardware dependent
  * instructions but they can be emulated combining some of they.
  * 
  * The independent view of hardware instruction set has also a register bank space. Each
  * bank has a set of identified registers that are referenced (along with bank identifier)
  * in instructions as operands or destination registers. If hardware dependent bank space
  * has no environment parameter bank (the most probable one), the translation to hardware
  * dependent instruction code will probably unify the environment parameter bank and the 
  * constant bank and will update the register and bank references in instructions.
  * 
  */
  
 /* Ref (ARB1_0_vertex_program.txt) Section 2.14.4,  Vertex Program Execution Environment
  *         line 3488
  * 
  *  There are twenty-seven vertex program instructions.  The instructions and
  *  their respective input and output parameters are summarized in Table X.5.
  *
  *   Instruction    Inputs  Output   Description
  *   -----------    ------  ------   --------------------------------
  *   ABS            v       v        absolute value
  *   ADD            v,v     v        add
  *   ARL            v       a        address register load
  *   DP3            v,v     ssss     3-component dot product
  *   DP4            v,v     ssss     4-component dot product
  *   DPH            v,v     ssss     homogeneous dot product
  *   DST            v,v     v        distance vector
  *   EX2            s       ssss     exponential base 2
  *   EXP            s       v        exponential base 2 (approximate)
  *   FLR            v       v        floor
  *   FRC            v       v        fraction
  *   LG2            s       ssss     logarithm base 2
  *   LIT            v       v        compute light coefficients
  *   LOG            s       v        logarithm base 2 (approximate)
  *   MAD            v,v,v   v        multiply and add
  *   MAX            v,v     v        maximum
  *   MIN            v,v     v        minimum
  *   MOV            v       v        move
  *   MUL            v,v     v        multiply
  *   POW            s,s     ssss     exponentiate
  *   RCP            s       ssss     reciprocal
  *   RSQ            s       ssss     reciprocal square root
  *   SGE            v,v     v        set on greater than or equal
  *   SLT            v,v     v        set on less than
  *   SUB            v,v     v        subtract
  *   SWZ            v       v        extended swizzle
  *   XPD            v,v     v        cross product
  *
  */

 /* New Fragment program instruction OpCodes added:
  * Ref (ARB1_0_fragment_program.txt)  3.11.4  Fragment Program Execution Environment
  *         line 2571
  *
  * There are 33 fragment program instructions.  The instructions and 
  * their respective input and output parameters are summarized in 
  * Table X.5.
  *
  *
  *
  *   Instruction    Inputs  Output   Description
  *   -----------    ------  ------   --------------------------------
  *   CMP            v,v,v   v        compare
  *   COS            s       ssss     cosine with reduction to [-PI,PI]
  *   KIL            v       v        kill fragment
  *   LRP            v,v,v   v        linear interpolation
  *   SCS            s       ss--     sine/cosine without reduction
  *   SIN            s       ssss     sine with reduction to [-PI,PI]
  *   TEX            v,u,t   v        texture sample
  *   TXB            v,u,t   v        texture sample with bias
  *   TXP            v,u,t   v        texture sample with projection
  *
  * Saturated versions: all output values are clamped in [0,1] range.
  *
  *   Instruction    Inputs  Output   Description
  *   -----------    ------  ------   --------------------------------
  *   ABS_SAT        v       v        absolute value
  *   ADD_SAT        v,v     v        add
  *   CMP_SAT        v,v,v   v        compare
  *   COS_SAT        s       ssss     cosine with reduction to [-PI,PI]
  *   DP3_SAT        v,v     ssss     3-component dot product
  *   DP4_SAT        v,v     ssss     4-component dot product
  *   DPH_SAT        v,v     ssss     homogeneous dot product
  *   DST_SAT        v,v     v        distance vector
  *   EX2_SAT        s       ssss     exponential base 2
  *   FLR_SAT        v       v        floor
  *   FRC_SAT        v       v        fraction
  *   LG2_SAT        s       ssss     logarithm base 2
  *   LIT_SAT        v       v        compute light coefficients
  *   LRP_SAT        v,v,v   v        linear interpolation
  *   MAD_SAT        v,v,v   v        multiply and add
  *   MAX_SAT        v,v     v        maximum
  *   MIN_SAT        v,v     v        minimum
  *   MOV_SAT        v       v        move
  *   MUL_SAT        v,v     v        multiply
  *   POW_SAT        s,s     ssss     exponentiate
  *   RCP_SAT        s       ssss     reciprocal
  *   RSQ_SAT        s       ssss     reciprocal square root
  *   SCS_SAT        s       ss--     sine/cosine without reduction
  *   SGE_SAT        v,v     v        set on greater than or equal
  *   SIN_SAT        s       ssss     sine with reduction to [-PI,PI]
  *   SLT_SAT        v,v     v        set on less than
  *   SUB_SAT        v,v     v        subtract
  *   SWZ_SAT        v       v        extended swizzle
  *   TEX_SAT        v,u,t   v        texture sample
  *   TXB_SAT        v,u,t   v        texture sample with bias
  *   TXP_SAT        v,u,t   v        texture sample with projection
  *   XPD_SAT        v,v     v        cross product
  *
  */

#ifndef GENERICINSTRUCTION_H
    #define GENERICINSTRUCTION_H


#include <iostream>
#include <string>

namespace libgl
{

namespace GenerationCode
{

class GenericInstruction 
{
public:

    enum TextureTarget {TEXTURE_1D, TEXTURE_2D, TEXTURE_3D, CUBE_MAP, RECT_MAP};

    enum Opcode 
    {
        G_ABS, G_ADD, G_ARL, G_DP3, G_DP4, G_DPH, G_DST, G_EX2, G_EXP, G_FLR,
        G_FRC, G_LG2, G_LIT, G_LOG, G_MAD, G_MAX, G_MIN, G_MOV, G_MUL, G_POW,
        G_RCP, G_RSQ, G_SGE, G_SLT, G_SUB, G_SWZ, G_XPD, G_CMP, G_COS, G_KIL,
        G_LRP, G_SCS, G_SIN, G_TEX, G_TXB, G_TXP,
        G_ABS_SAT, G_ADD_SAT, G_CMP_SAT, G_COS_SAT, G_DP3_SAT, G_DP4_SAT,
        G_DPH_SAT, G_DST_SAT, G_EX2_SAT, G_FLR_SAT, G_FRC_SAT, G_KIL_SAT,
        G_LG2_SAT, G_LIT_SAT, G_LRP_SAT, G_MAD_SAT, G_MAX_SAT, G_MIN_SAT,
        G_MOV_SAT, G_MUL_SAT, G_POW_SAT, G_RCP_SAT, G_RSQ_SAT, G_SCS_SAT,
        G_SGE_SAT, G_SIN_SAT, G_SLT_SAT, G_SUB_SAT, G_SWZ_SAT, G_TEX_SAT,
        G_TXB_SAT, G_TXP_SAT, G_XPD_SAT, INVALID_OP
    };
    
    enum BankType
    {
        G_IN,
        G_OUT,
        G_PARAM,
        G_TEMP,
        G_ADDR,
        INVALID_BANK
    };

    enum SwzComponent
    {
        ZERO, 
        ONE, 
        X, 
        Y, 
        Z, 
        W 
    };
    
    /**
     * OperandInfo class
     *
     * Information about an operand.
     */
 
    class OperandInfo
    {
    public:
        bool relativeModeFlag;
        bool arrayAccessModeFlag;
        unsigned int arrayModeOffset;
        unsigned int relModeAddrReg;
        unsigned int relModeAddrComp;
        unsigned int relModeOffset;
        unsigned char swizzleMode;
        bool negateFlag;
        unsigned int idReg;
        BankType idBank;
        
        OperandInfo() : relativeModeFlag(false), arrayAccessModeFlag(false), arrayModeOffset(0), 
                        relModeAddrReg(0), relModeAddrComp(0), relModeOffset(0), swizzleMode(0x1B),
                        negateFlag(false), idReg(0), idBank(INVALID_BANK) { }
    };

    /**
     * ResultInfo class
     *
     * Information about a destination register in a instruction.
     */
 
    class ResultInfo
    {
    public:
        unsigned char writeMask;
        unsigned int idReg;
        BankType idBank;
        
        ResultInfo(): writeMask(0x0F), idReg(0), idBank(INVALID_BANK) { }
    };

    
    /**
     * SwizzleInfo class
     *
     * Information about the swizzle mode in a SWZ instruction
     */
    class SwizzleInfo
    {
    public:
        SwzComponent xSelect;
        SwzComponent ySelect;
        SwzComponent zSelect;
        SwzComponent wSelect;
        bool xNegate;   
        bool yNegate;
        bool zNegate;
        bool wNegate;
        
        SwizzleInfo(): xSelect(X), ySelect(Y), zSelect(Z), wSelect(W), xNegate(false),
                       yNegate(false), zNegate(false), wNegate(false) { }
    };                 
                       
private:               

    /**
     * The attributes of a generic instruction
     */
     
    unsigned int line;
    std::string opcodeString;
    Opcode opcode;
    OperandInfo op1, op2, op3;
    ResultInfo res;
    SwizzleInfo swz;
    TextureTarget textureTarget;
    unsigned int textureImageUnit;
    unsigned int num_operands;
    bool lastInstruction; ///< Is the last instruction of the program

    /**
     * Auxiliar functions for printing 
     */
    void writeOperand(unsigned int numOperand, OperandInfo op, std::ostream& os) const;
    void writeResult(ResultInfo res, std::ostream& os) const;
    void writeSwizzle(SwizzleInfo swz, std::ostream& os) const;
    void writeTextureTarget(TextureTarget texTarget, std::ostream& os) const;
    void writeTextureImageUnit(unsigned int texImageUnit, std::ostream& os) const;

public:

    GenericInstruction(unsigned int line, 
                       const std::string opcodeString, 
                       OperandInfo op1,
                       OperandInfo op2,
                       OperandInfo op3,
                       ResultInfo res,
                       SwizzleInfo swz, 
                       unsigned int texImageUnit = 0,
                       TextureTarget texTarget = TEXTURE_1D,
                       bool lastInstruction = false);
    
    void setLastInstruction()                            {lastInstruction = true;}

    unsigned int getLine() const                        {return line;}
    Opcode getOpcode() const                            {return opcode;}
    unsigned int getNumOperands()                       {return num_operands;}
    OperandInfo getOp1() const                          {return op1;}
    OperandInfo getOp2() const                          {return op2;}
    OperandInfo getOp3() const                          {return op3;}
    ResultInfo  getRes() const                          {return res;}
    SwizzleInfo getSwz() const                          {return swz;}
    unsigned int getTextureImageUnit() const            {return textureImageUnit;}
    TextureTarget getTextureTarget() const              {return textureTarget;}
    bool getIsLastInstruction()    const                    {return lastInstruction;}

    void print(std::ostream& os) const;
};

} // namespace CodeGeneration

} // namespace libgl

#endif

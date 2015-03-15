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
 * $RCSfile: ShaderEmulator.h,v $
 * $Revision: 1.20 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:42 $
 *
 * Shader Unit Emulator.
 *
 */

/**
 *
 * \file ShaderEmulator.h
 *
 *  Defines the ShaderEmulator class.  This class provides services
 *  to emulate the functional behaviour of a Shader unit.
 *
 *
 */

#include "GPUTypes.h"
#include "support.h"
#include "QuadFloat.h"
#include "QuadInt.h"
#include "GPUMath.h"
#include "TextureEmulator.h"
#include "GPU.h"
#include "ShaderInstruction.h"
#include "DynamicObject.h"
#include "FixedPoint.h"

#ifndef _SHADEREMULATOR_
#define _SHADEREMULATOR_

namespace gpu3d
{

/**  Defines the shader models supported.  */
enum ShaderModel
{
    UNIFIED,        /**<  Unified shader model.  */
    UNIFIED_MICRO   /**<  Unified shader model with wider (3x) shader input vector.  */
};

/**  Defines the maximum number of texture accesses supported on fly.  */
static const u32bit TEXT_QUEUE_SIZE = 8192;

/**
 *  This class defines a Shader Emulator.
 *
 *  A Shader Emulator contains the functions and structures
 *  to emulate the functional behaviour of a Shader (Vertex
 *  or Pixel Shader) following a stream based programming
 *  model.
 *
 */

class ShaderInstruction;    /*  For cross declaration problems ... */


/**
 *
 *  Defines a queued texture access for a stamp.
 *
 */

struct TextureQueue
{
    TextureOperation texOp;         /**<  Defines the operation requested to the Texture Unit.  */
    QuadFloat *coordinates;         /**<  Pointer to the parameters (coordinates, index) for the operation.  */
    f32bit *parameter;              /**<  Pointer to the per fragment parameter (lod/bias) for the operation (only for texture reads).  */
    u32bit textUnit;                /**<  Identifier of the texture sampler (texture reads) or attribute (attribute read) to be used for the operation.  */
    u32bit requested;               /**<  Number of requests received for the stamp.  */
    bool vertexTextureAccess;       /**<  The texture access comes from a vertex shader (only for the ATTILA emulator!!!).  */
    ShaderInstruction::ShaderInstructionDecoded **shInstrD; /**<  Pointer to the shader instructions that produced the texture access.  */
    DynamicObject *accessCookies;                           /**<  Cookies for the texture access.  */
};


/**
 *
 *  Stores information about an on-going derivative computation operation (DDX and DDY).
 *
 */
struct DerivativeInfo
{
    QuadFloat input[4];     /**<  Stores the four input values for the derivation operation.  */
    u32bit baseThread;      /**<  Stores the base thread/element for the derivation operation.  */
    u32bit derived;         /**<  Stores the number of derivation operations already received.  */
    ShaderInstruction::ShaderInstructionDecoded **shInstrD;     /**<  Pointer to the shader instructions corresponding with the derivation operation.  */
};

/*  Defines the state partitions (instruction memory and constants) for
    unified shader model.  */
enum
{
    VERTEX_PARTITION = 0,       /**<  Identifies the partition for vertices.  */
    FRAGMENT_PARTITION = 1,     /**<  Identifies the partition for fragments.  */
    TRIANGLE_PARTITION = 2,     /**<  Identifies the partition for triangles.  */
    MICROTRIANGLE_PARTITION = 3 /**<  Identifies the partition for microtriangle fragments.  */
};

class ShaderEmulator
{

private:
  
    static const u32bit UNIFIED_TEMP_REG_SIZE = 16;         /**<  Defines the size in bytes of a temporary register.  */
    static const u32bit UNIFIED_CONST_REG_SIZE = 16;        /**<  Defines the size in bytes of a constant register.  */   
    
    //  Shader parameters.
    char *name;                 /**<  Shader name. */
    ShaderModel model;          /**<  Shader model. */
    u32bit numThreads;          /**<  Number of active threads supported by the shader.  */
    TextureEmulator *textEmu;   /**<  Pointer to the texture emulator attached to the shader.  */
    u32bit stampFragments;      /**<  Number of fragments per stamp for mipmaped texture access.  */
    u32bit fxpDecBits;          /**<  Decimal bits for the FixedPoint Accumulator register.  */
    bool storeDecodedInstr;     /**<  Stores if the decoded shader instructions are stored in an array for future use.  */

    //  Shader Model parameters
    u32bit instructionMemorySize;   /**<  Size (in instructions) of the Instruction Memory.  */
    u32bit numInputRegs;            /**<  Number of registers in the Input Bank.  */
    u32bit numOutputRegs;           /**<  Number of registers in the Output Bank.  */
    u32bit numTemporaryRegs;        /**<  Number of registers in the Temporary Register Bank.  */
    u32bit numConstantRegs;         /**<  Number of constants in the Constant Register Bank.  */
    u32bit numAddressRegs;          /**<  Number of registers in the Address Register Bank.  */
    u32bit numPredicateRegs;        /**<  Number of registers in the Predicate Register bank.  */

    //  Shader State.
    ShaderInstruction **instructionMemory;      /**< Shader Instruction Memory  */
    ShaderInstruction::ShaderInstructionDecoded ***decodedInstructions; /**<  Decoded, per thread shader instructions.  */
    QuadFloat **inputBank;                      /**<  Shader Input Register Bank.  */
    QuadFloat **outputBank;                     /**<  Shader Output Register Bank. */
    u8bit *temporaryBank;                       /**<  Shader Temporary Register Bank.  */
    u8bit *constantBank;                        /**<  Shader Constant Register Bank.  */
    QuadInt **addressBank;                      /**<  Shader Address Register Bank (integer).  */
    FixedPoint **accumFXPBank;                  /**<  FixedPoint Accumulator (implicit) bank.  */
    bool **predicateBank;                       /**<  Shader Predicate Register bank.  */
    u32bit *PCTable;                            /**<  Thread PC table.  */
    bool **kill;                                /**<  Stores the thread (per-sample) kill flag that marks the thread for premature ending.  */
    f32bit **zexport;                           /**<  Stores the thread (per-sample) z export value (MicroPolygon rasterizer).  */
    u32bit *sampleIdx;                          /**<  Stores the thread current sample identifier pointer (changed/incremented by the CHS instruction).  */
    TextureQueue textQueue[TEXT_QUEUE_SIZE];    /**<  Texture access queue.  */
    u32bit freeTexture[TEXT_QUEUE_SIZE];        /**<  List of free entries in the texture queue.  */
    u32bit waitTexture[TEXT_QUEUE_SIZE];        /**<  List of entries in the texture queue waiting to be processed.  */
    u32bit firstFree;                           /**<  Pointer to the first entry in the free texture list.  */
    u32bit lastFree;                            /**<  Pointer to the last entry in the free texture list.  */
    u32bit numFree;                             /**<  Number of free entries in the texture queue.  */
    u32bit firstWait;                           /**<  Pointer to the first entry in the wait texture list.  */
    u32bit lastWait;                            /**<  Pointer to the last entry in the wait texture list.  */
    u32bit numWait;                             /**<  Number of entries waiting to be processed in the texture queue.  */

    DerivativeInfo  currentDerivation;          /**<  Stores the information related with the current derivation operation.  */

    /**
     *  Shader Instruction Emulation functions table.
     *
     *  This table stores pointers to the functions that
     *  emulate the different shader instructions.  They
     *  are ordered by opcode so they can be accessed
     *  using the decoded ShaderInstruction opcode field.
     *
     */

    static void (*shInstrEmulationTable[])(ShaderInstruction::ShaderInstructionDecoded &, ShaderEmulator &);

    /**
     *  Shader Instruction Emulation functions table.  For instructions that set
     *  the condition codes.
     *
     *  This table stores pointer to the functions that emulate the different
     *  shader instructions.  They are ordered by opcode so they can be accessed
     *  using the decoded ShaderInstruction opcode field.
     *  This table contains functions for emulating the version of
     *  the instruction that updates the condition code register.
     *
     *
     */

    static void (*shInstrEmulationTableCC[])(ShaderInstruction::ShaderInstructionDecoded &, ShaderEmulator &);

    /*                     */
    /*  PRIVATE FUNCTIONS  */
    /*                     */

    /**
     *  Sets the pointer to the emulated function for the instruction
     *  in the table for predecoded instructions.
     *
     *  \param shInstr  Shader Instruction for which select an emulation
     *  function.
     *
     */

    void setEmulFunction(ShaderInstruction::ShaderInstructionDecoded *shInstr);

    /*
     *  Decodes and returns the address to the register that the instruction is
     *  going to access when executed (per thread!).
     *
     *  @param numThread  Thread in which the instruction is going to be executed.
     *  @param bank Bank of the register to be decoded.
     *  @param reg Register to in the bank to be decoded.
     *  @param partition Shader partition in unified shader model to be used.
     *
     *  @return A pointer (the address) to the register.
     *
     */

    void *decodeOpReg(u32bit numThread, Bank bank, u32bit reg, u32bit partition = VERTEX_PARTITION);

    /**
     *
     *  Adds a texture operation for a shader processing element to the texture queue.
     *
     *  @param shInstr Reference to the instruction producing the texture operation.
     *  @param operation Type of operation requested to the texture unit.
     *  @param coord Coordinates of the texture access/Index for the attribute load.
     *  @param parameter Per fragment parameter (lod/bias) for the texture access.
     *
     */

    void textureOperation(ShaderInstruction::ShaderInstructionDecoded &shInstr, TextureOperation operation, QuadFloat coord, f32bit parameter);

    /**
     *
     *  Adds the information about a derivation operation to the current derivation structure.
     *
     *  @param shInstr Reference to the shader instruction corresponding with the derivation operation.
     *  @param input Input values for the derivation operation.
     *
     */
    
    void derivOperation(ShaderInstruction::ShaderInstructionDecoded &shInstr, QuadFloat input);     
     
    /**
     *  Swizzle function.  Reorders and replicates the components of a 4D
     *  float point vector.
     *
     */

    QuadFloat swizzle(SwizzleMode mode, QuadFloat qf);

    /**
     *  Swizzle function.  Reorders and replicates the components of a 4D
     *  integer vector.
     *
     */

    QuadInt swizzle(SwizzleMode mode, QuadInt qi);

    /**
     *  Negate function.  Negates the 4 components in a 4D float point vector.
     *
     */

    QuadFloat negate(QuadFloat qf);

    /**
     *  Negate function.  Negates the 4 components in a 4D integer vector.
     *
     */

    QuadInt negate(QuadInt qi);

    /**
     *  Absolute function.  Returns the absolute value for the 4 components
     *  in a 4D float point vector.
     *
     */

    QuadFloat absolute(QuadFloat qf);

    /**
     *  Absolute function.  Returns the absolute value for the 4 components
     *  in a 4D float point vector.
     *
     */

    QuadInt absolute(QuadInt qi);
    
    /**
     *  Write Mask for VS2 Shader Model.  Components in the outpyt 4D
     *  float point register are written only if the write mask is
     *  enabled for that component and the condition mask is true for the
     *  corresponding swizzled component of the condition code register.
     *
     */

    void writeResReg(ShaderInstruction &shInstr, f32bit *f, f32bit *res, bool predicate = true);

    /**
     *  Write Mask for VS2 Shader Model.  Components in the outpyt 4D
     *  integer register are written only if the write mask is
     *  enabled for that component and the condition mask is true for the
     *  corresponding swizzled component of the condition code register.
     *
     */

    void writeResReg(ShaderInstruction &shInstr, s32bit *f, s32bit *res, bool predicate = true);

    /**
     *  Write Mask for VS2 Shader Model.  Components in the outpyt 4D
     *  float point register are written only if the write mask is
     *  enabled for that component and the condition mask is true for the
     *  corresponding swizzled component of the condition code register.
     *
     */

    void writeResReg(ShaderInstruction &shInstr, FixedPoint *f, FixedPoint *res, bool predicate = true);

    /**
     *  Read the first operand value of the instruction.
     *
     *  @param shInstr  Referente to the shader instruction.
     *  @param numThread  Thread in which the instruction is executed.
     *  @param PC PC of the executed instruction.
     *  @param op Reference to the QuadFloat where to store the first operand.
     *
     */

    void readOperand1(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, QuadFloat &op);

    /**
     *  Read the second operand value of the instruction.
     *
     *  @param shInstr  Referente to the shader instruction.
     *  @param numThread  Thread in which the instruction is executed.
     *  @param PC PC of the executed instruction.
     *  @param op Reference to the QuadFloat where to store the second operand.
     *
     */

    void readOperand2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, QuadFloat &op);

    /**
     *  Read the third operand value of the instruction.
     *
     *  @param shInstr  Referente to the shader instruction.
     *  @param numThread  Thread in which the instruction is executed.
     *  @param PC PC of the executed instruction.
     *  @param op Reference to the QuadFloat where to store the third operand.
     *
     */

    void readOperand3(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, QuadFloat &op);

    /**
     *  Read the first operand value of the instruction.
     *
     *  @param shInstr  Referente to the shader instruction.
     *  @param numThread  Thread in which the instruction is executed.
     *  @param PC PC of the executed instruction.
     *  @param op Reference to the QuadInt where to store the first operand.
     *
     */

    void readOperand1(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, QuadInt &op);

    /**
     *  Read the second operand value of the instruction.
     *
     *  @param shInstr  Referente to the shader instruction.
     *  @param numThread  Thread in which the instruction is executed.
     *  @param PC PC of the executed instruction.
     *  @param op Reference to the QuadInt where to store the second operand.
     *
     */

    void readOperand2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, QuadInt &op);

    /**
     *
     *  This function reads and returns the quadfloat operand of a single operand shader instruction.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op1 A reference to a QuadFloat to return the instruction first operand value.
     *
     */

    void read1Operands(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, QuadFloat &op1);

    /**
     *
     *  This function reads and returns the two quadfloat operands of a two operands shader instruction.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op1 A reference to a QuadFloat to return the instruction first operand value.
     *  @param op2 A reference to a QuadFloat to return the instruction second operand value.
     *
     */

    void read2Operands(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, QuadFloat &op1, QuadFloat &op2);

    /**
     *
     *  This function reads and returns the three quadfloat operands of a three operands shader instruction.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op1 A reference to a QuadFloat to return the instruction first operand value.
     *  @param op2 A reference to a QuadFloat to return the instruction second operand value.
     *  @param op3 A reference to a QuadFloat to return the instruction third operand value.
     *
     */

    void read3Operands(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul,
                       QuadFloat &op1, QuadFloat &op2, QuadFloat &op3);

    /**
     *
     *  This function reads and returns the two quadint operands of a two operands shader instruction.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op1 A reference to a QuadInt to return the instruction first operand value.
     *  @param op2 A reference to a QuadInt to return the instruction second operand value.
     *
     */

    void read2Operands(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, QuadInt &op1, QuadInt &op2);

    /**
     *
     *  This function reads and returns the first float point scalar operand for a shader instruction.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op A reference to a 32 bit float variable where to return the instruction scalar operand value.
     *
     */

    void readScalarOp1(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, f32bit &op);

    /**
     *
     *  This function reads and returns the second float point scalar operand for a shader instruction.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op A reference to a 32 bit float variable where to return the instruction scalar operand value.
     *
     */

    void readScalarOp2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, f32bit &op);


    /**
     *
     *  This function reads and returns the first integer scalar operand for a shader instruction.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op A reference to a 32 bit integer variable where to return the instruction scalar operand value.
     *
     */

    void readScalarOp1(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, s32bit &op);

    /**
     *
     *  This function reads and returns the second integer scalar operand for a shader instruction.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op A reference to a 32 bit integer variable where to return the instruction scalar operand value.
     *
     */

    void readScalarOp2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, s32bit &op);

    /**
     *
     *  This function reads and returns the first boolean scalar operand for a shader instruction.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op A reference to a boolean varable where to return the instruction first boolean scalar operand value.
     *
     */
    void readScalarOp1(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, bool &op);

    /**
     *
     *  This function reads and returns the second boolean scalar operand for a shader instruction.
     *
     *  @param shInstr Reference to a shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param op A reference to a boolean varable where to return the instruction second boolean scalar operand value.
     *
     */
    void readScalarOp2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, bool &op);

    /**
     *
     *  This function writes a shader instruction quadfloat result.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to the shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param result A pointer to a four component float point
     *  result.
     *
     */

    void writeResult(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, f32bit *result);

    /**
     *
     *  This function writes a shader instruction quadint result.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to the shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param result A pointer to a four component float point
     *  result.
     *
     */

    void writeResult(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, s32bit *result);

    /**
     *
     *  This function writes a shader instruction quad fixed point result.
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to the shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param result A pointer to a four component fixed point result.
     *
     */

    void writeResult(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, FixedPoint *result);

    /**
     *
     *  This function writes a shader instruction boolean result (predicate register).
     *  THIS FUNCTION SHOULD BE INLINED.
     *
     *  @param shInstr Reference to the shader instruction.
     *  @param shEmul Reference to the ShaderEmulator object where the instruction is executed.
     *  @param result The result boolean value to write.
     *
     */

    void writeResult(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul, bool result);

    /*  Shader Emulation Functions.  */

    static void shNOP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shADD(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shARL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shCOS(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shDP3(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shDP4(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shDPH(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shDST(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shEX2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shEXP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shFLR(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shFRC(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shLG2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shLIT(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shLOG(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shMAD(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shMAX(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shMIN(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shMOV(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shMUL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shRCP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shRSQ(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shSGE(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shSIN(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shSLT(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    
    static void shTEX(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shTXB(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shTXP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shTXL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    
    static void shKIL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shKLS(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
   
    static void shZXP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shZXS(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);    
    static void shCHS(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shCMP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shCMPKIL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shLDA(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shFXMUL(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shFXMAD(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shFXMAD2(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shEND(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

    static void shSETPEQ(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shSETPGT(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shSETPLT(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shANDP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shJMP(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    
    static void shDDX(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shDDY(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    
    static void shADDI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shMULI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shSTPEQI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shSTPGTI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    static void shSTPLTI(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);
    
    static void shIllegal(ShaderInstruction::ShaderInstructionDecoded &shInstr, ShaderEmulator &shEmul);

public:

    /**
     *
     *   Constructor function for the class.
     *
     *   This function creates a multithreaded Shader Emulator for a type
     *   of Shader Emulator.  Allocates space for the input, output and other
     *   register banks of the Shader, and for the Shader Instruction Memory.
     *   All the resources that aren't shared between the threads are replicated.
     *
     *   @param name Name of the Shader.  Identifies the shader accessed.
     *   @param model Defines the shader model to use.
     *   @param numThreads Number of threads supported by a multithreaded shader.
     *   @param storeDecodedInstr Flag that defines if decoded instructions are stored for future use (may require a lot of memory space).
     *   @param textEmul Pointer to a texture emulator attached to the shader.
     *   @param stampFrags Fragments per stamp for texture accesses.
     *   @param fxpDecBits Decimal bits for the FixedPoint Accumulator register.
     *
     *   @return Returns a new ShaderEmulator object.
     *
     */

    ShaderEmulator(char *name, ShaderModel model, u32bit numThreads = 1, bool storeDecodedInstr = false, TextureEmulator *textEmul = NULL,
                   u32bit stampFrags = 4, u32bit fxpDecBits = 16);

    /**
     *
     *  This function resets the Shader state (for example when a new vertex/pixel is
     *  loaded).  Temporal registers are loaded with default values.
     *
     *  \param numThread Identifies the thread which state should be reseted.
     *  \return Returns Nothing.
     *
     */

    void resetShaderState(u32bit numThread);

    /**
     *
     *  This function loads register or parameters values into the Shader state.
     *
     *  \param numThread Identifies the thread which state should be modified.
     *  \param bank Bank identifier where the data is going to be written.
     *  \param reg  Register identifier where the data is going to be written.
     *  \param data Data to write (4D Float).
     *  \return Returns Nothing.
     *
     */

    void loadShaderState(u32bit numThread, Bank bank, u32bit reg, QuadFloat data);

    /**
     *
     *  This function loads register or parameters values into the Shader state.
     *
     *  \param numThread Identifies the thread which state should be modified.
     *  \param bank Bank identifier where the data is going to be written.
     *  \param reg  Register identifier where the data is going to be written.
     *  \param data Data to write (4D Integer).
     *  \return Returns Nothing.
     *
     */

    void loadShaderState(u32bit numThread, Bank bank, u32bit reg, QuadInt data);

    /**
     *
     *  This function loads register or parameters values into the Shader state.
     *
     *  \param numThread Identifies the thread which state should be modified.
     *  \param bank Bank identifier where the data is going to be written.
     *  \param reg  Register identifier where the data is going to be written.
     *  \param data Data to write (Boolean).
     *  \return Returns Nothing.
     *
     */

    void loadShaderState(u32bit numThread, Bank bank, u32bit reg, bool data);

    /**
     *
     *  This function partially loads a register bank into the Shader state.
     *
     *  \param numThread Identifies the thread which state should be modified.
     *  \param bank Bank identifier where the data is going to be written.
     *  \param data Pointer to an array of QuadFloat values to write into the register.
     *  \param startReg First register to write.
     *  \param nRegs Number of data float vectors to copy in the register bank.
     *  \return Returns Nothing.
     *
     */

    void loadShaderState(u32bit numThread, Bank bank, QuadFloat *data, u32bit startReg, u32bit nRegs);

    /**
     *
     *  This function loads a full register bank into the Shader state.
     *
     *  \param numThread Identifies the thread which state should be modified.
     *  \param bank Bank identifier where the data is going to be written.
     *  \param data Data to write (4D float).
     *  \return Returns Nothing.
     *
     */

    void loadShaderState(u32bit numThread, Bank bank, QuadFloat *data);

    /**
     *
     *  This function reads register or parameters values from the Shader state.
     *
     *  \param numThread Identifies the thread which state should be read.
     *  \param bank Bank identifier from where the data is going to be read.
     *  \param reg  Register identifier from where the data is going to be read.
     *  \param value Reference to the QuadFloat where the value is going to be written.
     *  \return Returns Nothing.
     *
     */

    void readShaderState(u32bit numThread, Bank bank, u32bit reg, QuadFloat &value);

    /**
     *
     *  This function reads register or parameters values from the Shader state.
     *
     *  \param numThread Identifies the thread which state should be read.
     *  \param bank Bank identifier from where the data is going to be read.
     *  \param reg  Register identifier from where the data is going to be read.
     *  \param value  Reference to the QuadInt where the value is going to be written.
     *  \return Returns Nothing.
     *
     */

    void readShaderState(u32bit numThread, Bank bank, u32bit reg, QuadInt &value);


    /**
     *
     *  This function reads register or parameters values from the Shader state.
     *
     *  \param numThread Identifies the thread which state should be read.
     *  \param bank Bank identifier from where the data is going to be read.
     *  \param reg  Register identifier from where the data is going to be read.
     *  \param value Reference to the bool variable where the value is going to
     *  be written.
     *  \return Returns The value of the register (Boolean).
     *
     */

    void readShaderState(u32bit numThread, Bank bank, u32bit reg, bool &value);

    /**
     *
     *  This function reads a full register bank from the Shader state (4D float).
     *
     *  \param numThread Identifies the thread which state should be read.
     *  \param bank Bank identifier from where the data is going to be read.
     *  \param data Pointer to the array where the data is going to be returned.
     *  \return Returns Nothing.
     *
     */

    void readShaderState(u32bit numThread, Bank bank, QuadFloat *data);

    /**
     *
     *  This function reads a full register bank from the Shader state (4D Integer).
     *
     *  \param numThread Identifies the thread which state should be read.
     *  \param bank Bank identifier from where the data is going to be read.
     *  \param data Pointer to the variable where the data is going to be returned.
     *  \return Returns Nothing.
     *
     */

    void readShaderState(u32bit numThread, Bank bank, QuadInt *data);

    /**
     *
     *  This function reads a full register bank from the Shader state (Boolean).
     *
     *  \param numThread Identifies the thread which state should be read.
     *  \param bank Bank identifier from where the data is going to be read.
     *  \param data Pointer to the variable where the data is going to be returned.
     *  \return Returns Nothing.
     *
     */

    void readShaderState(u32bit numThread, Bank bank, bool *data);

    /**
     *
     *  This function loads a new Shader Program into the Shader.
     *
     *  @param code Pointer to a buffer with the shader program in binary format.
     *  @param address Address inside the shader program memory where to store the program.
     *  @param sizeCode Size of the Shader Program code in binary format (bytes).
     *  @param partition Shader partition for which to load the shader program.
     *
     *  @return Returns Nothing.
     *
     */

    void loadShaderProgram(u8bit *code, u32bit address, u32bit sizeCode, u32bit partition);

    /**
     *
     *  This function loads a new Shader Program into the Shader (instruction is not decoded).
     *
     *  @param code Pointer to a buffer with the shader program in binary format.
     *  @param address Address inside the shader program memory where to store the program.
     *  @param sizeCode Size of the Shader Program code in binary format (bytes).
     *
     *  @return Returns Nothing.
     *
     */

    void loadShaderProgramLight(u8bit *code, u32bit address, u32bit sizeCode);

    /**
     *
     *  This functions decodes a shader instruction for a shader thread and returns the
     *  corresponding decoded shader instruction object.
     *
     *  @param shInstr The shader instruction to decode.
     *  @param address The address in shader instruction memory of the shader instruction to decode.
     *  @param nThread The shader thread for which the instruction is decoded.
     *  @param partition Defines the shader partition (vertex or fragment) to use for a given
     *  program for unified shader model.
     *
     *  @return A ShaderInstructionDecoded object with the shader instruction decoded
     *  information.
     *
     */

    ShaderInstruction::ShaderInstructionDecoded *decodeShaderInstruction(ShaderInstruction *shInstr,
        u32bit address, u32bit nThread, u32bit partition);

    /**
     *
     *  This function returns a Shader Instruction from the current
     *  Shader Instruction Memory.
     *
     *  \param threadId Identifier of the thread from which to fetch the instruction.
     *  \param PC Instruction Memory address from where to fetch the instruction.
     *  \return Returns a pointer to a ShaderInstruction object with the decoded
     *  instruction that had to be read.
     *
     */

    ShaderInstruction::ShaderInstructionDecoded *fetchShaderInstruction(u32bit threadID, u32bit PC);

    /**
     *
     *  This function returns a Shader Instruction from the current
     *  Shader Instruction Memory (decode instruction at fetch version).
     *
     *  @param threadId Identifier of the thread from which to fetch the instruction.
     *  @param PC Instruction Memory address from where to fetch the instruction.
     *  @param partition Defines the shader partition (vertex or fragment) to use for a given
     *  program for unified shader model.
     *
     *  @return Returns a pointer to a ShaderInstruction object with the decoded
     *  instruction that had to be read.
     *
     */

    ShaderInstruction::ShaderInstructionDecoded *fetchShaderInstructionLight(u32bit threadID, u32bit PC, u32bit partition);

    /**
     *
     *  Read the shader instruction at the given instruction memory address for the defined partition.
     *
     *  @param pc Address of the shader instruction in the instruction memory.
     *
     *  @return Returns a pointer to the ShaderInstruction in that position of the instruction memory.
     *
     */

    ShaderInstruction *readShaderInstruction(u32bit pc);

    /**
     *
     *  This function executes the instruction provided as parameter and
     *  updates the Shader State with the result of the instruction execution.
     *
     *  \param instruction  A ShaderInstruction object that defines the instruction
     *  to be executed.
     *
     */

    void execShaderInstruction(ShaderInstruction::ShaderInstructionDecoded *instruction);

    /**
     *
     *  This function returns the emulated PC stored for a Shader Emulator  thread.
     *
     *  @param numThread The shader thread from which to get the PC.
     *
     *  @return The emulated PC of the shader thread.
     *
     */

    u32bit threadPC(u32bit numThread);

    /**
     *
     *  This functions sets the emulated PC for a thread shader in the emulator.
     *
     *  @param numThread The shader thread for which to set the PC.
     *  @param pc The new thread pc.
     *
     */

    u32bit setThreadPC(u32bit numThread, u32bit pc);

    /**
     *
     *  This function returns the kill flag stored for a Shader Emulator thread.
     *
     *  @param The shader thread from which to get the kill flag value.
     *
     *  @return The kill flag of the shader thread.
     *
     */

    bool threadKill(u32bit numThread);

    /**
     *
     *  This function returns the sample kill flag stored for a Shader Emulator thread.
     *
     *  @param numThread The shader thread from which to get the kill flag value.
     *  @param sample The sample identifier.
     *  @return The sample kill flag of the shader thread.
     *
     */

    bool threadKill(u32bit numThread, u32bit sample);

    /**
     *
     *  This function returns the sample z export value stored for a Shader Emulator thread.
     *
     *  @param numThread The shader thread from which to get the kill flag value.
     *  @param sample The sample identifier.
     *  @return The sample z export value of the shader thread.
     *
     */

    f32bit threadZExport(u32bit numThread, u32bit sample);

    /**
    *
     *  Generates the next complete texture request for a stamp available in the shader emulator.
     *
     *  @return A TextureAccess object with the information about the texture accesses for
     *  a whole stamp.
     *
     */

    TextureAccess *nextTextureAccess();

    /**
    *
     *  Generates the next complete texture request for a vertex texture access available in the shader emulator.
     *  Only the ATTILA emulator should use this function.
     *
     *  @return A TextureAccess object with the information about the texture accesses for a vertex texture access.
     *
     */

    TextureAccess *nextVertexTextureAccess();

    /**
     *
     *  Completes a texture access writing the sample in the result register.
     *
     *  @param id Pointer to the texture queue entry to be completed and written
     *  to the thread register bank.
     *  @param sample Pointer to an array with the result of the texture access
     *  for the whole stamp represented by the texture queue entry to complete.
     *  @param threads Pointer to an array where to store the threads that generated
     *  the texture access.
     *  @param removeInstr Boolean value that defines if the shaded decoded instruction
     *  that generated the texture request has to be deleted.  Provided for emulator
     *  support.
     *
     */

    void writeTextureAccess(u32bit id, QuadFloat *sample, u32bit *threads, bool removeInstr = true);

    /**
     *
     *  Checks a jump instruction condition for a vector and performs the jump if required.
     *
     *  @param shInstrDec Pointer to a shader decoded instruction correspoding with the representative
     *  instruction for the vector.
     *  @param vectorLenght Number of elements/threads per vector that must execute in lock-step.
     *  @param targetPC Reference to a variable where to store the destination PC of the jump.
     *
     *  @return Returns if the jump instruction condition was true and the PC changed to the
     *  jump destination PC.
     *
     */
     
     bool checkJump(ShaderInstruction::ShaderInstructionDecoded *shInstrDec, u32bit vectorLenght, u32bit &destPC);

};

} // namespace gpu3d

#endif

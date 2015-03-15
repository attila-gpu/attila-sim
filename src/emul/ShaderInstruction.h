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
 * $RCSfile: ShaderInstruction.h,v $
 * $Revision: 1.15 $
 * $Author: vmoya $
 * $Date: 2008-02-22 18:32:42 $
 *
 * Shader Instruction.
 *
 */

/**
 *  \file ShaderInstruction.h
 *
 *  Defines the Shader Instruction Class.  This class stores a shader
 *  instruction in decoded format.
 *
 */

#ifndef _SHADERINSTRUCTION_

#define _SHADERINSTRUCTION_

#include "GPUTypes.h"
#include "GPU.h"
#include "OptimizedDynamicMemory.h"
#include <string>

using namespace std;

namespace gpu3d
{
    class ShaderEmulator;

// Disable macros IN/OUT/BOOL (avoid conflicts)
#ifdef IN
    #undef IN
#endif
#ifdef OUT
    #undef OUT
#endif

/**
 *  Defines the different register banks.
 *
 */
enum Bank
{
    IN      = 0x00,     /**<  Shader input register bank.  */
    OUT     = 0x01,     /**<  Shader output register bank.  */
    PARAM   = 0x02,     /**<  Constant register bank.  */
    TEMP    = 0x03,     /**<  Temporary register bank.  */
    ADDR    = 0x04,     /**<  Address register bank (integer).  */
    PARAM2  = 0x05,     /**<  Constant register bank (extension).  */
    IMM     = 0x06,     /**<  Immediate value.  */
    TEXT    = 0x08,     /**<  Texture sampler (uncodified implicit bank).  */
    SAMP    = 0x09,     /**<  Pixel sample (uncodified implicit bank).  */
    PRED    = 0x0A,     /**<  Predicate register bank (uncodified implicit bank).  */
    INVALID = 0xFF      /**<  Invalid Register Bank (uncodified, only for decoded instructions).  */
};


/**
 *  Defines opcodes for the shader.
 *
 */
enum ShOpcode
{
    //  Opcodes 00h - 07h
    NOP     = 0x00,
    ADD     = 0x01,
    ADDI    = 0x02,
    ARL     = 0x03,
    ANDP    = 0x04,    
    //BRA     = 0x05
    //CAL     = 0x06
    COS     = 0x07,
    
    //  Opcodes 08h - 0Fh
    DP3     = 0x08,
    DP4     = 0x09,
    DPH     = 0x0A,
    DST     = 0x0B,
    EX2     = 0x0C,
    EXP     = 0x0D,
    FLR     = 0x0E,
    FRC     = 0x0F,
      
    //  Opcodes 10h - 17h
    LG2     = 0x10,
    LIT     = 0x11,
    LOG     = 0x12,
    MAD     = 0x13,
    MAX     = 0x14,
    MIN     = 0x15,
    MOV     = 0x16,
    MUL     = 0x17,
    
    //  Opcodes 18h - 1Fh    
    MULI    = 0x18,
    RCP     = 0x19,
    //RET     = 0x1A,
    RSQ     = 0x1B,
    SETPEQ  = 0x1C,
    SETPGT  = 0x1D,
    SGE     = 0x1E,
    SETPLT  = 0x1F,
    
    //  Opcodes 20h - 27h
    SIN     = 0x20,
    STPEQI  = 0x21,
    SLT     = 0x22,
    STPGTI  = 0x23,
    STPLTI  = 0x24,
    TXL     = 0x25,
    TEX     = 0x26,
    TXB     = 0x27,

    //  Opcodes 28h - 30h
    TXP     = 0x28,
    KIL     = 0x29,
    KLS     = 0x2A,
    ZXP     = 0x2B,
    ZXS     = 0x2C,
    CMP     = 0x2D,
    CMPKIL  = 0x2E,
    CHS     = 0x2F,
    
    //  Opcodes 30h - 36h
    LDA     = 0x30,
    FXMUL   = 0x31,
    FXMAD   = 0x32,
    FXMAD2  = 0x33,
    DDX     = 0x34,
    DDY     = 0x35,
    JMP     = 0x36,
    END     = 0x37,
    
    //  Unused opcodes 38h - FFh

    //  Last valid opcode.
    LASTOPC = 0x38,
    
    //  Maximum number of opcodes supported.    
    MAXOPC  = 0xFF,
    INVOPC  = 0x100
};

/*  To avoid double definitions.  Defined in ShaderInstruction.cpp.  */
extern ShOpcode translateShOpcodeTable[];

/**
 *  Defines the different mask modes available for register write. The
 *  mask mode N indicates that this component is not modified.
 *
 */

enum MaskMode
{
    NNNN = 0x00,
    NNNW = 0x01,
    NNZN = 0x02,
    NNZW = 0x03,
    NYNN = 0x04,
    NYNW = 0x05,
    NYZN = 0x06,
    NYZW = 0x07,

    XNNN = 0x08,
    XNNW = 0x09,
    XNZN = 0x0a,
    XNZW = 0x0b,
    XYNN = 0x0c,
    XYNW = 0x0d,
    XYZN = 0x0e,
    mXYZW = 0x0f
};

/*  To avoid double definitions.  Defined in ShaderInstruction.cpp.  */
extern MaskMode translateMasModeTable[];

/**
 *  Defines the different swizzle modes for reading operands.
 *
 */

enum SwizzleMode
    {

// X---

      XXXX, XXXY, XXXZ, XXXW,
      XXYX, XXYY, XXYZ, XXYW,
      XXZX, XXZY, XXZZ, XXZW,
      XXWX, XXWY, XXWZ, XXWW,

      XYXX, XYXY, XYXZ, XYXW,
      XYYX, XYYY, XYYZ, XYYW,
      XYZX, XYZY, XYZZ, XYZW,
      XYWX, XYWY, XYWZ, XYWW,

      XZXX, XZXY, XZXZ, XZXW,
      XZYX, XZYY, XZYZ, XZYW,
      XZZX, XZZY, XZZZ, XZZW,
      XZWX, XZWY, XZWZ, XZWW,

      XWXX, XWXY, XWXZ, XWXW,
      XWYX, XWYY, XWYZ, XWYW,
      XWZX, XWZY, XWZZ, XWZW,
      XWWX, XWWY, XWWZ, XWWW,

// Y---

      YXXX, YXXY, YXXZ, YXXW,
      YXYX, YXYY, YXYZ, YXYW,
      YXZX, YXZY, YXZZ, YXZW,
      YXWX, YXWY, YXWZ, YXWW,

      YYXX, YYXY, YYXZ, YYXW,
      YYYX, YYYY, YYYZ, YYYW,
      YYZX, YYZY, YYZZ, YYZW,
      YYWX, YYWY, YYWZ, YYWW,

      YZXX, YZXY, YZXZ, YZXW,
      YZYX, YZYY, YZYZ, YZYW,
      YZZX, YZZY, YZZZ, YZZW,
      YZWX, YZWY, YZWZ, YZWW,

      YWXX, YWXY, YWXZ, YWXW,
      YWYX, YWYY, YWYZ, YWYW,
      YWZX, YWZY, YWZZ, YWZW,
      YWWX, YWWY, YWWZ, YWWW,

// Z---

      ZXXX, ZXXY, ZXXZ, ZXXW,
      ZXYX, ZXYY, ZXYZ, ZXYW,
      ZXZX, ZXZY, ZXZZ, ZXZW,
      ZXWX, ZXWY, ZXWZ, ZXWW,

      ZYXX, ZYXY, ZYXZ, ZYXW,
      ZYYX, ZYYY, ZYYZ, ZYYW,
      ZYZX, ZYZY, ZYZZ, ZYZW,
      ZYWX, ZYWY, ZYWZ, ZYWW,

      ZZXX, ZZXY, ZZXZ, ZZXW,
      ZZYX, ZZYY, ZZYZ, ZZYW,
      ZZZX, ZZZY, ZZZZ, ZZZW,
      ZZWX, ZZWY, ZZWZ, ZZWW,

      ZWXX, ZWXY, ZWXZ, ZWXW,
      ZWYX, ZWYY, ZWYZ, ZWYW,
      ZWZX, ZWZY, ZWZZ, ZWZW,
      ZWWX, ZWWY, ZWWZ, ZWWW,

// W---

      WXXX, WXXY, WXXZ, WXXW,
      WXYX, WXYY, WXYZ, WXYW,
      WXZX, WXZY, WXZZ, WXZW,
      WXWX, WXWY, WXWZ, WXWW,

      WYXX, WYXY, WYXZ, WYXW,
      WYYX, WYYY, WYYZ, WYYW,
      WYZX, WYZY, WYZZ, WYZW,
      WYWX, WYWY, WYWZ, WYWW,

      WZXX, WZXY, WZXZ, WZXW,
      WZYX, WZYY, WZYZ, WZYW,
      WZZX, WZZY, WZZZ, WZZW,
      WZWX, WZWY, WZWZ, WZWW,

      WWXX, WWXY, WWXZ, WWXW,
      WWYX, WWYY, WWYZ, WWYW,
      WWZX, WWZY, WWZZ, WWZW,
      WWWX, WWWY, WWWZ, WWWW,

    };


/*  To avoid double definitions.  Defined in ShaderInstruction.cpp.  */
extern SwizzleMode translateSwizzleModeTable[];

/*  To avoid double definitions.  Defined in ShaderInstruction.cpp.  */
extern Bank translateBankTable[];


/**
 *
 *  Shader Instruction Size in bytes.
 *  Shader Instruction Size Log2.
 *  Shader Instruction Size Mask.
 *
 */



/**
 *
 *  This class defines a Shader Instruction.
 *
 *  Contains decode information about a Shader instruction.
 *
 *  This class inherits from the OptimizedDynamicMemory object that provides custom optimized
 *  memory allocation and deallocation support
 *
 */


class ShaderInstruction : public OptimizedDynamicMemory
{

public:

    //  Define constants.
    
    static const u32bit SHINSTRSIZE = 16;
    static const u32bit SHINSTRSZLOG = 4;
    static const u32bit SHINSTRSZMASK = 0x0f;
    static const u32bit MAX_OPCODE_LENGTH = 6;

private:

    /*
     *
     *  NOTE:  OLD CODIFICATION FOR REFERENCE PURPOSES.  SEE BELOW FOR NEW CODIFICATION.
     *
     *
     *  Instruction codification (normal):
     *
     *   Field     Size  Count    Encoding  Byte
     *
     *   opcode:     8b  (0 - 7)     (7 - 0)         0
     *   waitpoint:  1b  (8)         (8)             1
     *   predicated: 1b  (9)         (9)             1
     *   negatepred: 1b  (10)        (10)            1 
     *   predreg:    5b  (11 - 15)   (11 - 15)       1
     *   op1reg:     8b  (16 - 23)   (23 - 16)       2
     *   op2reg:     8b  (24 - 31)   (31 - 24)       3
     *   op3reg:     8b  (32 - 39)   (39 - 32)       4
     *   resreg:     8b  (40 - 47)   (47 - 40)       5
     *   op1bank:    3b  (48 - 50)   (50 - 48)       6
     *   op2bank:    3b  (51 - 53)   (53 - 51)       6
     *   op3bank:    3b  (54 - 56)   (58 - 56)       7
     *   resbank:    3b  (57 - 59)   (61 - 59)       7
     *   negfop1:    1b  (60)        (64)            8
     *   negfop2:    1b  (61)        (65)            8
     *   negfop3:    1b  (62)        (66)            8
     *   absfop1:    1b  (63)        (67)            8
     *   absfop2:    1b  (64)        (68)            8
     *   absfop3:    1b  (65)        (69)            8
     *   satres:     1b  (66)        (70)            8
     *   endflag:    1b  (67)        (71)            8
     *   swzop1:     8b  (68 - 75)   (79 - 72)       9
     *   swzop2:     8b  (76 - 83)   (87 - 80)       10
     *   swzop3:     8b  (84 - 91)   (95 - 88)       11
     *   mask:       4b  (92 - 95)   (99 - 96)       12
     *   relmode:    1b  (96)        (100)           12
     *   reladreg:   2b  (97 - 98)   (113 - 112)     14
     *   reladcomp:  2b  (99 - 100)  (115 - 114)     14
     *   reloffst:   9b  (101 - 109) (127 - 119)     14 - 15
     *
     *   unused:    18b  (110 - 127) 
     *
     *             Byte     Unused bits     Count
     *              6         54 - 55         2
     *              7         62 - 63         2
     *              12        101 - 103       3
     *              13        104 - 111       8
     *              14        116 - 118       3
     *
     *
     *  Instruction codification (for jumps):
     *
     *   Field        Size  Count      Encoding       Byte
     *
     *   opcode:       8b  (0 - 7)     (7 - 0)         0
     *   waitpoint:    1b  (8)         (8)             1
     *   predicated:   1b  (9)         (9)             1
     *   negatepred:   1b  (10)        (10)            1 
     *   predreg:      5b  (11 - 15)   (11 - 15)       1
     *   op1reg:       8b  (16 - 23)   (23 - 16)       2
     *   offsetlo24:  24b  (24 - 47)   (24 - 47)       3 - 5
     *   op1bank:      3b  (48 - 50)   (50 - 48)       6
     *   offsethi8:    8b  (51 - 58)   (63 - 56)       7
     *   negfop1:      1b  (59)        (64)            8
     *   absfop1:      1b  (60)        (67)            8
     *   endflag:      1b  (61)        (71)            8
     *   swzop1:       8b  (62 - 69)   (79 - 72)       9
     *   relmode:      1b  (70)        (100)           12
     *   reladreg:     2b  (71 - 72)   (113 - 112)     14
     *   reladcomp:    2b  (73 - 74)   (115 - 114)     14
     *   reloffst:     9b  (75 - 83)   (127 - 119)     14 - 15
     *
     *   unused:    44  (84 - 127) 
     *
     *             Byte     Unused bits             Count
     *              3         31 - 27                 8
     *              4         39 - 32                 8
     *              5         47 - 40                 8
     *              6         51 - 55                 5
     *              7         63 - 56                 8
     *              8         65, 66, 68 - 70         5
     *              10        80 - 87                 8
     *              11        95 - 88                 8
     *              12        96 - 99, 101 - 103      7
     *              13        104 - 111               8
     *              14        116 - 118               3
     *
     *
     *
     *  code[ 0] -> bit   0 -   7
     *  code[ 1] -> bit   8 -  15
     *  code[ 2] -> bit  16 -  23
     *  code[ 3] -> bit  24 -  31
     *  code[ 4] -> bit  32 -  39
     *  code[ 5] -> bit  40 -  47
     *  code[ 6] -> bit  48 -  55
     *  code[ 7] -> bit  56 -  63
     *  code[ 8] -> bit  64 -  71
     *  code[ 9] -> bit  72 -  79
     *  code[10] -> bit  80 -  87
     *  code[11] -> bit  88 -  95
     *  code[12] -> bit  96 - 103
     *  code[13] -> bit 104 - 111
     *  code[14] -> bit 112 - 119
     *  code[15] -> bit 120 - 127
     *
     *  This codification format sucks a bit though ...
     *
     *
     *
     *
     */

    /**
     *
     *  Codification for the lower 64-bits of a shader instruction.
     *
     */
    struct InstructionFieldsLo64
    {
        u64bit opcode      : 8,         /**<  Bits 00 - 07  =>  Instruction opcode.  */
               endflag     : 1,         /**<  Bit       08  =>  Instruction finishes current program.  */
               waitpoint   : 1,         /**<  Bit       09  =>  Instruction must wait for pending texture/memory requests.  */
               predicated  : 1,         /**<  Bit       10  =>  Instruction is predicated.  */
               invertpred  : 1,         /**<  Bit       11  =>  Instruction predicate is inverted.  */
               predreg     : 5,         /**<  Bits 12 - 16  =>  Instruction predicate register.  */
               op1bank     : 3,         /**<  Bits 17 - 19  =>  Instruction first operand bank.  */
               op1negate   : 1,         /**<  Bit       20  =>  Instruction negate, invert (for predicate bank) or implicit value
                                                                true/false (if absolute bit set for predicate bank)
                                                                for first operand.  */
               op1absolute : 1,         /**<  Bit       21  =>  Instruction absolute value or use implicit value (for predicate bank)
                                                                for first operand.  */
               op2bank     : 3,         /**<  Bits 22 - 24  =>  Instruction second operand bank.  */
               op2negate   : 1,         /**<  Bit       25  =>  Instruction negate, invert (for predicate bank) or implicit value
                                                                true/false (if absolute bit set for predicate bank)
                                                                for second operand.  */
               op2absolute : 1,         /**<  Bit       26  =>  Instruction absolute value or use implicit value (for predicate bank)
                                                                for second operand.  */
               op3bank     : 3,         /**<  Bits 27 - 29  =>  Instruction third operand bank.  */
               op3negate   : 1,         /**<  Bit       30  =>  Instruction negate, invert (for predicate bank) or implicit value
                                                                true/false (if absolute bit set for predicate bank)
                                                                for third operand.  */
               op3absolute : 1,         /**<  Bit       31  =>  Instruction absolute value or use implicit value (for predicate bank)
                                                                 for third operand.  */
               resbank     : 3,         /**<  Bits 32 - 34  =>  Instruction result bank.  */
               saturateres : 1,         /**<  Bit       35  =>  Instruction saturate/clamp result to [0, 1] range or invert result
                                                                (for predicate bank).  */
               mask        : 4,         /**<  Bits 36 - 39  =>  Instruction result write mask.  */
               relmode     : 1,         /**<  Bit       40  =>  Instruction constant bank relative access mode.  */
               reladreg    : 2,         /**<  Bits 41 - 42  =>  Instruction constant bank relative access address register.  */
               reladcomp   : 2,         /**<  Bits 43 - 44  =>  Instruction constant bank relative access address register component.  */
               reloffset   : 9,         /**<  Bits 45 - 53  =>  Instruction constant bank relative access offset.  */
               reserved    : 10;        /**<  Bits 54 - 63  =>  Reserved area.  */
    };
    
    /**
     *
     *  Codification for the higher 64-bits of a shader instruction with up to three register operands.
     *
     */
    struct InstructionFieldsHi64Operands
    {
        u64bit op1reg     : 8,          /**<  Bits 00 - 07  =>  Instruction first operand register identifier.  */
               op1swizzle : 8,          /**<  Bits 08 - 15  =>  Instruction first operand swizzle.  */
               resreg     : 8,          /**<  Bits 16 - 23  =>  Instruction result register identifier.  */
               op2reg     : 8,          /**<  Bits 24 - 31  =>  Instruction second operand register identifier.  */
               op2swizzle : 8,          /**<  Bits 32 - 39  =>  Instruction second operand swizzle.  */
               op3reg     : 8,          /**<  Bits 40 - 47  =>  Instruction third operand register identifier.  */
               op3swizzle : 8,          /**<  Bits 48 - 55  =>  Instruction third operand register identifier.  */
               reserved   : 8;          /**<  Bits 56 - 63  =>  Reserved area.  */
    };
    
    /**
     *
     *  Codification for the higher 64-bits of a shader instruction with one register operand and one immediate operand.
     *
     */
    struct InstructionFieldsHi64Immediate
    {
        u64bit op1reg     : 8,          /**<  Bits 00 - 07  =>  Instruction first operand register identifier.  */
               op1swizzle : 8,          /**<  Bits 08 - 15  =>  Instruction first operand swizzle.  */
               resreg     : 8,          /**<  Bits 16 - 23  =>  Instruction result register identifier.  */
               reserved   : 8,          /**<  Bits 24 - 31  =>  Reserved area.  */
               immediate  : 32;         /**<  Bits 32 - 63  =>  Instruction 32-bit immediate value.  */
    };
    
    /**
     *
     *  Codification for the higher 64-bits of a shader instruction.
     *
     */
    struct InstructionFieldsHi64
    {
        union
        {
            InstructionFieldsHi64Operands operands;
            InstructionFieldsHi64Immediate immediate;
        };
    };
    
    /**
     *
     *  Codification for a shader instruction.
     *
     */
    struct InstructionFields
    {
        InstructionFieldsLo64 lo64Bits;
        InstructionFieldsHi64 hi64Bits;
    };

    /**
     *
     *  Instruction codification.
     *
     */
    struct InstructionCode
    {
        union
        {
            u8bit code8[SHINSTRSIZE];
            u16bit code16[SHINSTRSIZE / 8];
            u32bit code32[SHINSTRSIZE / 4];
            u64bit code64[SHINSTRSIZE / 8];
            InstructionFields fields;
        };
    };
    
    
    //  Information about the instruction.

    ShOpcode    opcode;             /**<  Which operation realices the instruction.  */
    //u8bit       code[SHINSTRSIZE];  /**<  Instruction code.  */
    InstructionCode code;           /**<  Instruction code.  */
    u32bit      numOperands;        /**<  Number of input operands in the instruction.  */
    u32bit      op1;                /**<  First operand.  Register Pointer.  */
    Bank        bankOp1;            /**<  Register bank for Operand 1.  */
    u32bit      op2;                /**<  Second operand.  Register Pointer.  */
    Bank        bankOp2;            /**<  Register bank for Operand 2.  */
    u32bit      op3;                /**<  Third operand.  Register Pointer.  */
    Bank        bankOp3;            /**<  Register bank for Operand 3.  */
    u32bit      res;                /**<  Result.  Register Pointer.  */
    Bank        bankRes;            /**<  Register bank for Result.  */
    bool        predicatedFlag;     /**<  The shader instruction is predicated.  */
    bool        negatePredFlag;     /**<  Negate the predicate register value.  */
    u32bit      predicateReg;       /**<  Predicate register.  */
    SwizzleMode swizzleModeOp1;     /**<  Swizzle Mode for Operand 1.  */
    bool        negateFlagOp1;      /**<  Negate Flag for Operand 1.  */
    bool        absoluteFlagOp1;    /**<  Absolute value Flag for Operand 1.  */
    SwizzleMode swizzleModeOp2;     /**<  Swizzle Mode for Operand 2.  */
    bool        negateFlagOp2;      /**<  Negate Flag for Operand 2.  */
    bool        absoluteFlagOp2;    /**<  Absolute value Flag for Operand 2.  */
    SwizzleMode swizzleModeOp3;     /***  Swizzle Mode for Operand 3.  */
    bool        negateFlagOp3;      /**<  Negate Flag for Operand 3.  */
    bool        absoluteFlagOp3;    /**<  Absolute value Flag for Operand 3.  */
    bool        saturatedRes;       /**<  Clamps result components to [0,1].  */
    bool        endFlag;            /**<  Sets the instruction (arithmetic) as the last instruction in a kernel.  */
    bool        waitPointFlag;      /**<  Marks the instruction as a wait point (thread switch required).  */
    MaskMode    maskMode;           /**<  Ouput Mask Mode for the result.  */
    u32bit      relModeAddrReg;     /**<  Address register used in relative access mode (constant registers).  */
    u32bit      relModeAddrRegComp; /**<  Address register component used in relative access mode (constant registers).  */
    s16bit      relModeOffset;      /**<  Offset in relative access mode -256 to +255 (constant registers).  */
    bool        relativeModeFlag;   /**<  The instruction uses relative access mode to constant register.  */
    u32bit      immediate;          /**<  Instruction 32-bit immediate value.  */
    bool        isEndB;             /**<  Is an end instruction?  */
    bool        isIntegerB;         /**<  The instruction is an integer instruction?  */
    bool        isFloatB;           /**<  The instruction is a float point instruction?  */
    bool        isScalarB;          /**<  The instruction is a scalar operation.  */
    bool        isSOACompatibleB;   /**<  The instruction is compatible with a SOA (scalar) ALU architecture.  */
    bool        isALoadB;           /**<  The instruction is a load instruction.  */
    bool        isAStoreB;          /**<  The instruction is a store instruction.  */
    bool        isZExportB;         /**<  The instruction is a Z export instruction.  */
    bool        hasResultB;         /**<  The instruction writes a result.  */
    bool        isPredInstrB;       /**<  The instruction result is a predicate register.  */
    bool        isAJumpB;           /**<  The instruction is a jump/branch instruction.  */
    bool        hasImmediateB;      /**<  The instruction has an immediate value.  */
    
    /*  Information for executing the instruction in ShaderEmulator.  */

    /*  PRIVATE FUNCTIONS.  */

    /**
     *
     *  Determines the number of the operands of the instruction based
     *  in the instruction opcode.
     *
     *  \param opcode The instruction opcode.
     *  \return Returns the number of operands for that opcode.
     *
     */

    static u32bit setNumOperands(ShOpcode opcode);

    /**
     *
     *  Determines if the instruction is a scalar operation based on the opcode and the result write mask.
     *
     *  @param opcode The instruction opcode.
     *  @param writeMask The instruction result write mask.
     *
     *  @return Returns if the instruction is a scalar operation.
     *
     */

    bool setIsScalar(ShOpcode opcode, MaskMode writeMask);

    /**
     *
     *  Determines if the instruction is SOA compatible (scalar ALU architecture) based on the opcode and the result write mask.
     *
     *  @param opcode The instruction opcode.
     *  @param writeMask The instruction result write mask.
     *
     *  @return Returns if the instruction is SOA compatible.
     *
     */

    bool setIsSOACompatible(ShOpcode opcode, MaskMode writeMask);

    /**
     *
     *  Determines if the instruction writes a result.
     *
     *  @param opcode The instruction opcode.
     *
     *  @return Returns if the instruction writes a result.     
     *
     */
     
    bool setHasResult(ShOpcode opcode);
         
    /**
     *
     *  Parses a substring for relative (or indexed for bra/cal) addressing
     *  mode.
     *
     *  @param line A pointer to a line containing a shader assembly instruction.
     *  @param i Position from the line to start the parsing.
     *  @param length Length of the line.
     *  @param relModeReg Reference to the relative mode address register field.
     *  @param relModeComp Reference to the relative mode addr. reg. component field.
     *  @param relModeOff Reference to the relative mode offset field.
     *
     *  @return Returns TRUE if it was able to parse a correct relative mode
     *  addressing specification, FALSE if it was not.
     *
     */

    static bool parseRelAddr(char *line, u32bit &i, u32bit length,
        u32bit &relModeReg, u8bit &relModeComp, s16bit &relModeOff);

    /**
     *
     *  Parses a substring inside a shader assembly line for a swizzle mode
     *  specification.
     *
     *  @param line A pointer to a shader assembly line.
     *  @param i Position in the line from which to start parsing.
     *  @param length Length of the line.
     *  @param swz A reference to a swizzle mode field.
     *
     *  @return Returns TRUE if it was able to parse a correct swizzle
     *  specification, FALSE if it was not.
     *
     */

    static bool parseSwizzle(char *line, u32bit &i, u32bit length, SwizzleMode &swz);

    /**
     *
     *  Parses a substring inside a shader assembly line for an operand.
     *
     *  @param line A pointer to a shader assembly line.
     *  @param i Position in the line from which to start parsing.
     *  @param length Length of the line.
     *  @param opR Reference to a register number field.
     *  @param opB Reference to a bank identifier field.
     *  @param opN Reference to a negate flag field.
     *  @param opA Reference to an absolute value flag field.
     *  @param opM Reference to a swizzle mode field.
     *  @param integer Specifies how to parse immediate operands, as integer or float point values.
     *  @param relMode Reference to the relative mode flag field.
     *  @param relModeR Reference to the rel. mode address register field.
     *  @param relModeComp Reference to the rel mode. addr. reg. component field.
     *  @param relModeOff Reference to the rel. mode offset.
     *
     *  @return Returns TRUE if it was able to parse a correct operand, FALSE
     *  if it was not.
     *
     */

    static bool parseOperand(char *line, u32bit &i, u32bit length,
        u32bit &opR, Bank &opB, bool &opN, bool &opA, SwizzleMode &opM, bool integer,
        bool &relMode, u32bit &relModeR, u8bit &relModeComp, s16bit &relModeOff);


    /**
     *
     *  Returns if the bank identifier is valid for an operand (read).
     *
     *  @param opBank The operand bank idenfier.
     *
     *  @return TRUE if the bank identifier is valid as for an operand (read).
     *          FALSE if the bank identifier is not valid for an operand (read).
     */

    bool validOperandBank(Bank opBank);
    

    /**
     *
     *  Returns if the bank identifier is valid for a result (write).
     *
     *  @param opBank The operand bank idenfier.
     *
     *  @return TRUE if the bank identifier is valid as for a result (write).
     *          FALSE if the bank identifier is not valid for a result (write).
     */

    bool validResultBank(Bank opBank);
    
    /**
     *
     *  Returns the string corresponding with the specified register bank.
     *
     *  @param bank The register bank identifier.
     *
     *  @return A constant character string (can only be copied or compared) with the
     *          character string corresponding with the specified register bank.
     *
     */
     
    const char *getBankString(Bank bank);
   
    /**
     *
     *  Returns the string corresponding with the specified instruction opcode.
     *
     *  @param opcode The instruction opcode.
     *
     *  @return A constant character string (can only be copied or compared) with the
     *          character string corresponding with the specified instruction opcode.
     *
     */
     
    const char *getOpcodeString(ShOpcode opcode);

     /**
      *
      *  Returns if the result of the instruction corresponding with opcode 
      *  is a predicate register.
      *
      *  @param opcode The instruction opcode.
      *
      *  @return Returns TRUE if the instruction result is a predicate register.
      *
      */
      
     bool resultIsPredicateReg(ShOpcode opcode);
      
public:

    class ShaderInstructionDecoded: public OptimizedDynamicMemory
    {

    private:

        ShaderInstruction   *shInstr;   /**<  Reference to the shader instruction for the decoded instruction.  */

        /***  Pointer to the function that emulates the instruction in ShaderEmulator.   */
        void (*emulFunc)(ShaderInstructionDecoded &, ShaderEmulator &);

        u32bit      PC;                 /**<  Instruction PC when loaded in a Shader Emulator Unit.  */
        u32bit      numThread;          /**<  Instruction Thread Number when loaded in a Shader Emulator.  */

        u8bit       op1[16];            /**<  Local data array for first operand.  */
        u8bit       op2[16];            /**<  Local data array for second operand.  */
        u8bit       op3[16];            /**<  Local data array for third operand.  */
        u8bit       res[16];            /**<  Local data array for result.  */
        
        void        *shEmulOp1;         /**<  Pointer to the first operand register in a ShaderEmulator.  */
        void        *shEmulOp2;         /**<  Pointer to the second operand register in a ShaderEmmulator.  */
        void        *shEmulOp3;         /**<  Pointer to the third operand register in a ShaderEmulator.  */
        void        *shEmulResult;      /**<  Pointer to the result register in a ShaderEmulator.  */
        void        *shEmulPredicate;   /**<  Pointer to the predicate register in a ShaderEmulator.  */

    public:

        /**
         *
         *  ShaderInstructionDecoded constructor
         *
         *  @param shInstr A referent to the Shader Instruction to be stored decoded.
         *  @param pc The PC for the Shader Instruction to be stored decoded.
         *  @param thread The thread identifier for the ShaderInstruction to be stored decoded.
         *
         *
         *  @return A new ShaderInstructionDecoded instruction.
         *
         */

        ShaderInstructionDecoded(ShaderInstruction *shInstr, u32bit pc, u32bit thread);

        /**
         *
         *  Get the Shader Instruction for decoded instruction.
         *
         *  @return The pointer to the shader instruction for the decoded instruction.
         *
         */

        ShaderInstruction *getShaderInstruction() const;

        /**
         *  Set the emulation function for the instruction.  This function
         *  should be one of the provided by ShaderEmulator.
         *
         *  \param emulFunc  Pointer to the function that emulates the
         *  instruction.
         *
         */

        void setEmulFunc(void (*emulFunc)(ShaderInstruction::ShaderInstructionDecoded &, ShaderEmulator &));

        /**
         *  Get the emulation function of the instruction.
         *
         *  \return Returns a pointer to the function in ShaderEmulator
         *  that emulates the instruction.
         *
         */

        void (*getEmulFunc())(ShaderInstruction::ShaderInstructionDecoded &, ShaderEmulator &) ;

        /**
         *  Set the Instruction PC inside a ShaderEmulator Instruction Memory.
         *
         *  \param PC PC of the instruction in the instruction memory of a
         *  ShaderEmulator.
         *
         */

        void setPC(u32bit PC);

        /**
         *  Get the Instruction PC in a ShaderEmulator Instruction Memory.
         *
         *  \return Returns the PC of the instruction in a ShaderEmulator
         *  Instruction Memory.
         *
         */

        u32bit getPC() const;

        /**
         *  Set the Thread for the instruction in a ShaderEmulator.
         *
         *  \param numThread  Identifier of the thread in which this
         *  instruction is going to be executed inside a ShaderEmulator.
         *
         */

        void setNumThread(u32bit numThread);

        /**
         *  Get the Thread of the instruction.
         *
         *  \return Returns the identifier of a thread in a ShaderEmulator
         *  where the instruction is going to be executed.
         *
         */

        u32bit getNumThread() const;

        /**
         *  Set the pointer of the first operand register in a ShaderEmulator.
         *
         *  \param reg  Pointer to a ShaderEmulator register.
         *
         */

        void setShEmulOp1(void *reg);

        /**
         *  Get the pointer for the first operand register in a ShaderEmulator.
         *
         *  \return Returns a pointer to a ShaderEmulator register.
         *
         */

        void *getShEmulOp1();

        /**
         *  Set the pointer of the second operand register in a ShaderEmulator.
         *
         *  \param reg  Pointer to a ShaderEmulator register.
         *
         */
        void setShEmulOp2(void *reg);

        /**
         *  Get the pointer for the second operand register in a ShaderEmulator.
         *
         *  \return Returns a pointer to a ShaderEmulator register.
         *
         */

        void *getShEmulOp2();

        /**
         *  Set the pointer of the third operand register in a ShaderEmulator.
         *
         *  \param reg  Pointer to a ShaderEmulator register.
         *
         */
        void setShEmulOp3(void *reg);

        /**
         *  Get the pointer for the third operand register in a ShaderEmulator.
         *
         *  \return Returns a pointer to a ShaderEmulator register.
         *
         */

        void *getShEmulOp3();

        /**
         *  Set the pointer of the result register in a ShaderEmulator.
         *
         *  \param reg  Pointer to a ShaderEmulator register.
         *
         */

        void setShEmulResult(void *reg);

        /**
         *  Get the pointer for the result register in a ShaderEmulator.
         *
         *  \return Returns a pointer to a ShaderEmulator register.
         *
         */

        void *getShEmulResult();
        
        /**
         *  Set the pointer to the predicate register in the ShaderEmulator.
         *
         *  @param reg Pointer to the ShaderEmulator register.
         *
         */
         
        void setShEmulPredicate(void *reg);
        
        /**
         *
         *  Get the pointer to the predicate register in a ShaderEmulator.
         *
         *  @return Returns a pointer to a ShaderEmulator register.
         *
         */
         
        void *getShEmulPredicate();
        
        /**
         *
         *  Get the pointer to the local data array for the instruction first operand.
         *
         *  @return data Pointer the local data array used to store the instruction first operand.
         *
         */

        u8bit *getOp1Data();

        /**
         *
         *  Get the pointer to the local data array for the instruction second operand.
         *
         *  @return data Pointer the local data array used to store the instruction second operand.
         *
         */

        u8bit *getOp2Data();
         
        /**
         *
         *  Get the pointer to the local data array for the instruction third operand.
         *
         *  @return data Pointer the local data array used to store the instruction third operand.
         *
         */

        u8bit *getOp3Data();
       
        /**
         *
         *  Get the pointer to the local data array for the instruction result.
         *
         *  @return data Pointer the local data array used to store the instruction result.
         *
         */
        
        u8bit *getResultData();

    };

    /**
     *  Shader Instruction Constructor.
     *
     *  Creates a Shader instructions from a 64bit instruction word.
     *  Decodes the instruction word provided to the internal representation.
     *
     *  \param opcode A pointer to a 128bit shader instruction in binary format.
     *  \return Returns a ShaderInstruction object with the decoded instruction.
     *
     */

    ShaderInstruction(u8bit *code);

    /**
     *
     *  Shader Instruction Constructor.
     *
     *  Creates a Shader Instruction using as defined by the parameters.
     *  Encodes the instruction word using the information defined in the
     *  parameters.  For math instructions.
     *
     *  @param opc The shader instruction opcode.
     *
     *  @param op1B The first operand bank.
     *  @param op1R The first operand register.
     *  @param op1N The first operand negate flag.
     *  @param op1A The first operand absolute flag.
     *  @param op1M The first operand swizzle mask.
     *
     *  @param op2B The second operand bank.
     *  @param op2R The second operand register.
     *  @param op2N The second operand negate flag.
     *  @param op2A The second operand absolute flag.
     *  @param op2M The second operand swizzle mask.
     *
     *  @param op3B The third operand bank.
     *  @param op3R The third operand register.
     *  @param op3N The third operand negate flag.
     *  @param op3A The third operand absolute flag.
     *  @param op3M The third operand swizzle mask.
     *
     *  @param resB The result bank.
     *  @param resR The result register.
     *  @param resM The result mask.
     *  @param resSat Result saturate flag.
     *
     *  @param predicated The predicated instruction flag.
     *  @param negatePred Negate predicated register flag.     
     *  @param predR The predicate register for the instruction.
     *
     *  @param relMode  The relative mode flag.
     *  @param relModeReg  The relative mode address register.
     *  @param relModeComp  The relative mode address register component.
     *  @param relModeOff  The relative mode offset.
     *
     *
     *  @param lastIntr Sets the end flag marking the instruction as the last in a kernel.
     *
     *  @return Returns a ShaderInstruction object and encodes the
     *  instruction binary representation.
     *
     */

    ShaderInstruction(ShOpcode opc,                                     //  Instruction Opcode.
        Bank op1B, u32bit op1R, bool op1N, bool op1A, SwizzleMode op1M, //  First Operand parameters.
        Bank op2B, u32bit op2R, bool op2N, bool op2A, SwizzleMode op2M, //  Second Operand parameters.
        Bank op3B, u32bit op3R, bool op3N, bool op3A, SwizzleMode op3M, //  Second Operand parameters.
        Bank resB, u32bit resR, bool resSat, MaskMode resM,             //  Result parameters.
        bool predicated, bool negatePred, u32bit predR,                 //  Predication parameters.
        bool relMode,                                                   //  Relative Mode Flag.
        u32bit relModeReg, u8bit relModeComp, s16bit relModeOff,        //  Relative Mode parameters.
        bool lastInstr = false,                                         //  Marks as the last instruction in a kernel.
        bool waitPoint = false                                          //  Marks the instruction as a wait point.
        );

    /**
     *
     *  Shader Instruction Constructor.
     *
     *  Creates a Shader Instruction using as defined by the parameters.
     *  Encodes the instruction word using the information defined in the
     *  parameters.  For instructions without parameters (END, NOP).
     *
     *  @param opc The shader instruction opcode.
     *
     *  @return Returns a ShaderInstruction object and encodes the
     *  instruction binary representation
     */

    ShaderInstruction(ShOpcode opc);        /*  Instruction Opcode.  */

    /**
     *
     *  Writes the disassembled instruction to a string.
     *
     *  @param dis Pointer to a string where the instruction will
     *  be written disassembled.
     *
     */

    void disassemble(char *dis);

    /**
     *  Converts a text line with an assembly shader instruction
     *  to a ShaderInstruction object.
     *
     *  @param asm Pointer to a string with a shader instruction in
     *  assembly format.
     *  @param errorString A string used to return assembly error messages.
     *
     *  @return A pointer to a ShaderInstruction that represents the
     *  assembly instruction, a NULL if the assembly instruction could
     *  not be correctly assembled.
     *
     */

    static ShaderInstruction *assemble(char *asmline, string &errorString);

    /**
     *  Get the number of input operands in the instruction.
     *
     *  \return Returns the number of operands in the instruction.
     *
     */

    u32bit getNumOperands();

    /**
     *  Check if the instruction is an End.
     *
     *  \return Returns if the instruction is an End instruction.
     *
     */

    bool isEnd();

    /**
     *  Check if the instruction is an integer instruction.
     *
     *  \return Returns if the instruction is an integer instruction.
     *
     */

    bool isInteger();

    /**
     *  Check if the instruction is a float point instruction.
     *
     *  \return Returns if the instruction is a float point instruction.
     *
     */

    bool isFloat();

    /**
     *  Check if the instruction is a scalar operation.
     *
     *  \return Returns if the instruction is a scalar operation.
     *
     */

    bool isScalar();

    /**
     *
     *  Check fi the instruction is SOA compatible.
     *
     *  @return A boolean value informing if the instruction is SOA compatible (scalar ALU architecture).
     *
     */

    bool isSOACompatible();

    /**
     *  Check if the instruction is a load instruction.
     *
     *  \return Returns if the instruction is a load instruction.
     *
     */

    bool isALoad();

    /**
     *  Check if the instruction is a store instruction.
     *
     *  \return Returns if the instruction is a store instruction.
     *
     */

    bool isAStore();

    /**
     *  Check if the instruction is a Z export instruction.
     *
     *  \return Returns if the instruction is a Z export instruction.
     *
     */

    bool isZExport();

    /**
     *
     *  Check if the instruction is a jump/branch instruction.
     *
     *  @return Returns if the instruction is a jump/branch instruction.
     *
     */
    bool isAJump();
     
    /**
     *  Get the instruction opcode.
     *
     *  \return Returns the instruction opcode.
     *
     */

    ShOpcode getOpcode();

    /**
     *  Get the instruction binary codification.
     *
     *  \return Returns the instruction binary codification.
     *
     */

    void getCode(u8bit *code);

    /**
     *  Get the first operand register number.
     *
     *  \return Returns the first operand register number.
     *
     */

    u32bit getOp1();

    /**
     *  Get the second operand register number.
     *
     *  \return Returns the second operand register number.
     *
     */

    u32bit getOp2();

    /**
     *  Get the third operand register number.
     *
     *  \return Returns the third operand register number.
     *
     */

    u32bit getOp3();

    /**
     *  Get the first operand register bank.
     *
     *  \return Returns the first operand register bank identifier.
     *
     */

    Bank getBankOp1();

    /**
     *  Get the third operand register bank.
     *
     *  \return Returns the second operand register bank identifier.
     *
     */

    Bank getBankOp2();

    /**
     *  Get the third operand register bank.
     *
     *  \return Returns the third operand register bank identifier.
     *
     */

    Bank getBankOp3();

    /**
     *  Get the first operand swizzle mode.
     *
     *  \return Returns the first operand swizzle mode.
     *
     */

    SwizzleMode getOp1SwizzleMode();

    /**
     *  Get if the first operand must be negated.
     *
     *  \return Returns if the first operand must be negateed
     *
     */

    bool getOp1NegateFlag();

    /**
     *  Get if the first operand must be the absolute value of the register.
     *
     *  \return Returns if the first operand must be the absolute value of the register.
     *
     */

    bool getOp1AbsoluteFlag();

    /**
     *  Get the second operand swizzle mode.
     *
     *  \return Returns the first operand swizzle mode.
     *
     */

    SwizzleMode getOp2SwizzleMode();

    /**
     *  Get if the second operand must be negated.
     *
     *  \return Returns if the first operand must be negateed
     *
     */

    bool getOp2NegateFlag();

    /**
     *  Get if the second operand must be the absolute value of the register.
     *
     *  \return Returns if the second operand must be the absolute value of the register.
     *
     */

    bool getOp2AbsoluteFlag();

    /**
     *  Get the third operand swizzle mode.
     *
     *  \return Returns the third operand swizzle mode.
     *
     */

    SwizzleMode getOp3SwizzleMode();

    /**
     *  Get if the third operand must be negated.
     *
     *  \return Returns if the third operand must be negateed
     *
     */

    bool getOp3NegateFlag();

    /**
     *  Get if the third operand must be the absolute value of the register.
     *
     *  \return Returns if the third operand must be the absolute value of the register.
     *
     */

    bool getOp3AbsoluteFlag();

    /**
     *  Get the register number for the result register.
     *
     *  \return Returns the result register number.
     *
     */

    u32bit getResult();

    /**
     *  Get the result register bank.
     *
     *  \return Returns the result register bank.
     *
     */

    Bank getBankRes();

    /**
     *
     *  Get the saturated result flag for the instruction.
     *
     *  @return If saturated results is enabled for the instruction.
     *
     */

    bool getSaturatedRes();

    /**
     *
     *  Returns the end flag for the instruction.
     *
     *  @return If last instruction flag is enabled for the instruction.
     *
     */

    bool getEndFlag();

    /**
     *  Get the result write mask mode.
     *
     *  \return Returns the result write mask mode.
     *
     */

    MaskMode getResultMaskMode();

    /**
     *
     *  Get predicated instruction flag.
     *
     *  @return Returns if the instruction is predicated.
     *  
     */
     
    bool getPredicatedFlag();
    
    /**
     *
     *  Get negate predicate register flag.
     *
     *  @return Returns if the predicate register must be negated.
     *
     */
     
    bool getNegatePredicateFlag();
    
    /**
     *
     *  Get predicate register.
     *
     *  @return The identifier of the predicate register associated with the instruction.
     *
     */
     
     u32bit getPredicateReg();
         
    /**
     *  Check if the instruction uses relative mode accessing to
     *  the constant bank.
     *
     *  \return Returns if the instruction uses relative mode addressing to
     *   the constant bank.
     */

    bool getRelativeModeFlag();

    /**
     *  Get the address register used for the relative mode addressing.
     *
     *  \return Returns the address register number used for relative mode addressing.
     *
     */

    u32bit getRelMAddrReg();

    /**
     *  Get the address register component used for the relative mode addressing.
     *
     *  \return Returns the address register component number used for relative mode addressing.
     *
     */

    u32bit getRelMAddrRegComp();

    /**
     *  Get the relative addressing mode offset.
     *
     *  \return Returns the relative addressing mode offset.
     *
     */

    s16bit getRelMOffset();

    /**
     *
     *  Get the jump offset for a jump instruction.
     *
     */
  
    s32bit getJumpOffset();
    
    /**
     *
     *  Set the jump offset for a jump instruction.
     *
     */
     
    void setJumpOffset(s32bit offset);

    /**
     *
     *  Get the instruction 32-bit immediate value.
     *
     *  @return The instruction immediate 32-bit value.
     *
     */
     
    u32bit getImmediate();
    
    /**
     *
     *  Sets the end flag of the instruction.
     *
     */

    void setEndFlag(bool end);

    /**
     *
     *  Sets the wait point flag for the instruction.
     *
     */

    void setWaitPointFlag();

    /**
     *
     *  Returns if the instruction is marked as a wait point.
     *
     *  @return A boolean value that is true if the instruction is marked as a wait point.
     *
     */

    bool isWaitPoint();

    /**
     *
     *  Returns if the instruction writes a result.
     *
     *  @return A boolean value that is true if the instruction writes a result.
     *
     */
     
    bool hasResult();     
};

} // namespace gpu3d

#endif

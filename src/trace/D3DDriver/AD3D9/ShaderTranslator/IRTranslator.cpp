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


#include "IRTranslator.h"
#include <stdio.h>

using namespace std;
using namespace gpu3d;

// Macros
#ifdef D3D_DEBUG_ON
    #define D3D_DEBUG(expr) { expr }
#else
    #define D3D_DEBUG(expr) { }
#endif

#undef IN
#undef OUT

// NOTE 1: For pixel shader versions < 0x0200 value of r0 is the color output according to specs.
//         A MOV instruction is generated to put the value of the attila register that r0 is mapped to
//         in the attila register that d3d color output is mapped to.


//  NOTE 2: Extra code will be added to handle alpha test, this way:
//            - Initially, references to color output will be mapped to a TEMP gpu register.
//            - A MOV instruction is genererated if needed, see NOTE 1.
//            - At the end of translation the color output will be unmapped from the TEMP and
//              declared as usual, mapped to an OUTPUT gpu register.
//            - Extra code will take gpu TEMP register as input and store result in gpu OUTPUT register.


//  Constructor.
IRTranslator::IRTranslator()
{
    
    //  Create the map that translates the D3D9 write mask mode to the ATTILA write mask mode.
    DWORD W0 = D3DSP_WRITEMASK_0;
    DWORD W1 = D3DSP_WRITEMASK_1;
    DWORD W2 = D3DSP_WRITEMASK_2;
    DWORD W3 = D3DSP_WRITEMASK_3;

    map<DWORD, MaskMode> &mmm = maskModeMap;

    mmm[0  | 0  | 0  | 0 ] = NNNN;
    mmm[0  | 0  | 0  | W3] = NNNW;
    mmm[0  | 0  | W2 | 0 ] = NNZN;
    mmm[0  |  0 | W2 | W3] = NNZW;
    mmm[0  | W1 | 0  | 0 ] = NYNN;
    mmm[0  | W1 | 0  | W3] = NYNW;
    mmm[0  | W1 | W2 | 0 ] = NYZN;
    mmm[0  | W1 | W2 | W3] = NYZW;

    mmm[W0 | 0  | 0  | 0 ] = XNNN;
    mmm[W0 | 0  | 0  | W3] = XNNW;
    mmm[W0 | 0  | W2 | 0 ] = XNZN;
    mmm[W0 | 0  | W2 | W3] = XNZW;
    mmm[W0 | W1 | 0  | 0 ] = XYNN;
    mmm[W0 | W1 | 0  | W3] = XYNW;
    mmm[W0 | W1 | W2 | 0 ] = XYZN;
    mmm[W0 | W1 | W2 | W3] = mXYZW;

    //  Create the table that translates dhe D3D9 swizzle mode to the ATTILA swizzle mode.
    DWORD xx = D3DVS_X_X;
    DWORD xy = D3DVS_X_Y;
    DWORD xz = D3DVS_X_Z;
    DWORD xw = D3DVS_X_W;
    DWORD yx = D3DVS_Y_X;
    DWORD yy = D3DVS_Y_Y;
    DWORD yz = D3DVS_Y_Z;
    DWORD yw = D3DVS_Y_W;
    DWORD zx = D3DVS_Z_X;
    DWORD zy = D3DVS_Z_Y;
    DWORD zz = D3DVS_Z_Z;
    DWORD zw = D3DVS_Z_W;
    DWORD wx = D3DVS_W_X;
    DWORD wy = D3DVS_W_Y;
    DWORD wz = D3DVS_W_Z;
    DWORD ww = D3DVS_W_W;

    map< DWORD, SwizzleMode > &mm = swizzleModeMap;

    mm[xx|yx|zx|wx] = XXXX; mm[xx|yx|zx|wy] = XXXY;
    mm[xx|yx|zx|wz] = XXXZ; mm[xx|yx|zx|ww] = XXXW;
    mm[xx|yx|zy|wx] = XXYX; mm[xx|yx|zy|wy] = XXYY;
    mm[xx|yx|zy|wz] = XXYZ; mm[xx|yx|zy|ww] = XXYW;
    mm[xx|yx|zz|wx] = XXZX; mm[xx|yx|zz|wy] = XXZY;
    mm[xx|yx|zz|wz] = XXZZ; mm[xx|yx|zz|ww] = XXZW;
    mm[xx|yx|zw|wx] = XXWX; mm[xx|yx|zw|wy] = XXWY;
    mm[xx|yx|zw|wz] = XXWZ; mm[xx|yx|zw|ww] = XXWW;

    mm[xx|yy|zx|wx] = XYXX; mm[xx|yy|zx|wy] = XYXY;
    mm[xx|yy|zx|wz] = XYXZ; mm[xx|yy|zx|ww] = XYXW;
    mm[xx|yy|zy|wx] = XYYX; mm[xx|yy|zy|wy] = XYYY;
    mm[xx|yy|zy|wz] = XYYZ; mm[xx|yy|zy|ww] = XYYW;
    mm[xx|yy|zz|wx] = XYZX; mm[xx|yy|zz|wy] = XYZY;
    mm[xx|yy|zz|wz] = XYZZ; mm[xx|yy|zz|ww] = XYZW;
    mm[xx|yy|zw|wx] = XYWX; mm[xx|yy|zw|wy] = XYWY;
    mm[xx|yy|zw|wz] = XYWZ; mm[xx|yy|zw|ww] = XYWW;

    mm[xx|yz|zx|wx] = XZXX; mm[xx|yz|zx|wy] = XZXY;
    mm[xx|yz|zx|wz] = XZXZ; mm[xx|yz|zx|ww] = XZXW;
    mm[xx|yz|zy|wx] = XZYX; mm[xx|yz|zy|wy] = XZYY;
    mm[xx|yz|zy|wz] = XZYZ; mm[xx|yz|zy|ww] = XZYW;
    mm[xx|yz|zz|wx] = XZZX; mm[xx|yz|zz|wy] = XZZY;
    mm[xx|yz|zz|wz] = XZZZ; mm[xx|yz|zz|ww] = XZZW;
    mm[xx|yz|zw|wx] = XZWX; mm[xx|yz|zw|wy] = XZWY;
    mm[xx|yz|zw|wz] = XZWZ; mm[xx|yz|zw|ww] = XZWW;

    mm[xx|yw|zx|wx] = XWXX; mm[xx|yw|zx|wy] = XWXY;
    mm[xx|yw|zx|wz] = XWXZ; mm[xx|yw|zx|ww] = XWXW;
    mm[xx|yw|zy|wx] = XWYX; mm[xx|yw|zy|wy] = XWYY;
    mm[xx|yw|zy|wz] = XWYZ; mm[xx|yw|zy|ww] = XWYW;
    mm[xx|yw|zz|wx] = XWZX; mm[xx|yw|zz|wy] = XWZY;
    mm[xx|yw|zz|wz] = XWZZ; mm[xx|yw|zz|ww] = XWZW;
    mm[xx|yw|zw|wx] = XWWX; mm[xx|yw|zw|wy] = XWWY;
    mm[xx|yw|zw|wz] = XWWZ; mm[xx|yw|zw|ww] = XWWW;

//////////////////////////////////

    mm[xy|yx|zx|wx] = YXXX; mm[xy|yx|zx|wy] = YXXY;
    mm[xy|yx|zx|wz] = YXXZ; mm[xy|yx|zx|ww] = YXXW;
    mm[xy|yx|zy|wx] = YXYX; mm[xy|yx|zy|wy] = YXYY;
    mm[xy|yx|zy|wz] = YXYZ; mm[xy|yx|zy|ww] = YXYW;
    mm[xy|yx|zz|wx] = YXZX; mm[xy|yx|zz|wy] = YXZY;
    mm[xy|yx|zz|wz] = YXZZ; mm[xy|yx|zz|ww] = YXZW;
    mm[xy|yx|zw|wx] = YXWX; mm[xy|yx|zw|wy] = YXWY;
    mm[xy|yx|zw|wz] = YXWZ; mm[xy|yx|zw|ww] = YXWW;

    mm[xy|yy|zx|wx] = YYXX; mm[xy|yy|zx|wy] = YYXY;
    mm[xy|yy|zx|wz] = YYXZ; mm[xy|yy|zx|ww] = YYXW;
    mm[xy|yy|zy|wx] = YYYX; mm[xy|yy|zy|wy] = YYYY;
    mm[xy|yy|zy|wz] = YYYZ; mm[xy|yy|zy|ww] = YYYW;
    mm[xy|yy|zz|wx] = YYZX; mm[xy|yy|zz|wy] = YYZY;
    mm[xy|yy|zz|wz] = YYZZ; mm[xy|yy|zz|ww] = YYZW;
    mm[xy|yy|zw|wx] = YYWX; mm[xy|yy|zw|wy] = YYWY;
    mm[xy|yy|zw|wz] = YYWZ; mm[xy|yy|zw|ww] = YYWW;

    mm[xy|yz|zx|wx] = YZXX; mm[xy|yz|zx|wy] = YZXY;
    mm[xy|yz|zx|wz] = YZXZ; mm[xy|yz|zx|ww] = YZXW;
    mm[xy|yz|zy|wx] = YZYX; mm[xy|yz|zy|wy] = YZYY;
    mm[xy|yz|zy|wz] = YZYZ; mm[xy|yz|zy|ww] = YZYW;
    mm[xy|yz|zz|wx] = YZZX; mm[xy|yz|zz|wy] = YZZY;
    mm[xy|yz|zz|wz] = YZZZ; mm[xy|yz|zz|ww] = YZZW;
    mm[xy|yz|zw|wx] = YZWX; mm[xy|yz|zw|wy] = YZWY;
    mm[xy|yz|zw|wz] = YZWZ; mm[xy|yz|zw|ww] = YZWW;

    mm[xy|yw|zx|wx] = YWXX; mm[xy|yw|zx|wy] = YWXY;
    mm[xy|yw|zx|wz] = YWXZ; mm[xy|yw|zx|ww] = YWXW;
    mm[xy|yw|zy|wx] = YWYX; mm[xy|yw|zy|wy] = YWYY;
    mm[xy|yw|zy|wz] = YWYZ; mm[xy|yw|zy|ww] = YWYW;
    mm[xy|yw|zz|wx] = YWZX; mm[xy|yw|zz|wy] = YWZY;
    mm[xy|yw|zz|wz] = YWZZ; mm[xy|yw|zz|ww] = YWZW;
    mm[xy|yw|zw|wx] = YWWX; mm[xy|yw|zw|wy] = YWWY;
    mm[xy|yw|zw|wz] = YWWZ; mm[xy|yw|zw|ww] = YWWW;

///////////////////////

    mm[xz|yx|zx|wx] = ZXXX; mm[xz|yx|zx|wy] = ZXXY;
    mm[xz|yx|zx|wz] = ZXXZ; mm[xz|yx|zx|ww] = ZXXW;
    mm[xz|yx|zy|wx] = ZXYX; mm[xz|yx|zy|wy] = ZXYY;
    mm[xz|yx|zy|wz] = ZXYZ; mm[xz|yx|zy|ww] = ZXYW;
    mm[xz|yx|zz|wx] = ZXZX; mm[xz|yx|zz|wy] = ZXZY;
    mm[xz|yx|zz|wz] = ZXZZ; mm[xz|yx|zz|ww] = ZXZW;
    mm[xz|yx|zw|wx] = ZXWX; mm[xz|yx|zw|wy] = ZXWY;
    mm[xz|yx|zw|wz] = ZXWZ; mm[xz|yx|zw|ww] = ZXWW;

    mm[xz|yy|zx|wx] = ZYXX; mm[xz|yy|zx|wy] = ZYXY;
    mm[xz|yy|zx|wz] = ZYXZ; mm[xz|yy|zx|ww] = ZYXW;
    mm[xz|yy|zy|wx] = ZYYX; mm[xz|yy|zy|wy] = ZYYY;
    mm[xz|yy|zy|wz] = ZYYZ; mm[xz|yy|zy|ww] = ZYYW;
    mm[xz|yy|zz|wx] = ZYZX; mm[xz|yy|zz|wy] = ZYZY;
    mm[xz|yy|zz|wz] = ZYZZ; mm[xz|yy|zz|ww] = ZYZW;
    mm[xz|yy|zw|wx] = ZYWX; mm[xz|yy|zw|wy] = ZYWY;
    mm[xz|yy|zw|wz] = ZYWZ; mm[xz|yy|zw|ww] = ZYWW;

    mm[xz|yz|zx|wx] = ZZXX; mm[xz|yz|zx|wy] = ZZXY;
    mm[xz|yz|zx|wz] = ZZXZ; mm[xz|yz|zx|ww] = ZZXW;
    mm[xz|yz|zy|wx] = ZZYX; mm[xz|yz|zy|wy] = ZZYY;
    mm[xz|yz|zy|wz] = ZZYZ; mm[xz|yz|zy|ww] = ZZYW;
    mm[xz|yz|zz|wx] = ZZZX; mm[xz|yz|zz|wy] = ZZZY;
    mm[xz|yz|zz|wz] = ZZZZ; mm[xz|yz|zz|ww] = ZZZW;
    mm[xz|yz|zw|wx] = ZZWX; mm[xz|yz|zw|wy] = ZZWY;
    mm[xz|yz|zw|wz] = ZZWZ; mm[xz|yz|zw|ww] = ZZWW;


    mm[xz|yw|zx|wx] = ZWXX; mm[xz|yw|zx|wy] = ZWXY;
    mm[xz|yw|zx|wz] = ZWXZ; mm[xz|yw|zx|ww] = ZWXW;
    mm[xz|yw|zy|wx] = ZWYX; mm[xz|yw|zy|wy] = ZWYY;
    mm[xz|yw|zy|wz] = ZWYZ; mm[xz|yw|zy|ww] = ZWYW;
    mm[xz|yw|zz|wx] = ZWZX; mm[xz|yw|zz|wy] = ZWZY;
    mm[xz|yw|zz|wz] = ZWZZ; mm[xz|yw|zz|ww] = ZWZW;
    mm[xz|yw|zw|wx] = ZWWX; mm[xz|yw|zw|wy] = ZWWY;
    mm[xz|yw|zw|wz] = ZWWZ; mm[xz|yw|zw|ww] = ZWWW;

///////////////////////////////

    mm[xw|yx|zx|wx] = WXXX; mm[xw|yx|zx|wy] = WXXY;
    mm[xw|yx|zx|wz] = WXXZ; mm[xw|yx|zx|ww] = WXXW;
    mm[xw|yx|zy|wx] = WXYX; mm[xw|yx|zy|wy] = WXYY;
    mm[xw|yx|zy|wz] = WXYZ; mm[xw|yx|zy|ww] = WXYW;
    mm[xw|yx|zz|wx] = WXZX; mm[xw|yx|zz|wy] = WXZY;
    mm[xw|yx|zz|wz] = WXZZ; mm[xw|yx|zz|ww] = WXZW;
    mm[xw|yx|zw|wx] = WXWX; mm[xw|yx|zw|wy] = WXWY;
    mm[xw|yx|zw|wz] = WXWZ; mm[xw|yx|zw|ww] = WXWW;

    mm[xw|yy|zx|wx] = WYXX; mm[xw|yy|zx|wy] = WYXY;
    mm[xw|yy|zx|wz] = WYXZ; mm[xw|yy|zx|ww] = WYXW;
    mm[xw|yy|zy|wx] = WYYX; mm[xw|yy|zy|wy] = WYYY;
    mm[xw|yy|zy|wz] = WYYZ; mm[xw|yy|zy|ww] = WYYW;
    mm[xw|yy|zz|wx] = WYZX; mm[xw|yy|zz|wy] = WYZY;
    mm[xw|yy|zz|wz] = WYZZ; mm[xw|yy|zz|ww] = WYZW;
    mm[xw|yy|zw|wx] = WYWX; mm[xw|yy|zw|wy] = WYWY;
    mm[xw|yy|zw|wz] = WYWZ; mm[xw|yy|zw|ww] = WYWW;

    mm[xw|yz|zx|wx] = WZXX; mm[xw|yz|zx|wy] = WZXY;
    mm[xw|yz|zx|wz] = WZXZ; mm[xw|yz|zx|ww] = WZXW;
    mm[xw|yz|zy|wx] = WZYX; mm[xw|yz|zy|wy] = WZYY;
    mm[xw|yz|zy|wz] = WZYZ; mm[xw|yz|zy|ww] = WZYW;
    mm[xw|yz|zz|wx] = WZZX; mm[xw|yz|zz|wy] = WZZY;
    mm[xw|yz|zz|wz] = WZZZ; mm[xw|yz|zz|ww] = WZZW;
    mm[xw|yz|zw|wx] = WZWX; mm[xw|yz|zw|wy] = WZWY;
    mm[xw|yz|zw|wz] = WZWZ; mm[xw|yz|zw|ww] = WZWW;


    mm[xw|yw|zx|wx] = WWXX; mm[xw|yw|zx|wy] = WWXY;
    mm[xw|yw|zx|wz] = WWXZ; mm[xw|yw|zx|ww] = WWXW;
    mm[xw|yw|zy|wx] = WWYX; mm[xw|yw|zy|wy] = WWYY;
    mm[xw|yw|zy|wz] = WWYZ; mm[xw|yw|zy|ww] = WWYW;
    mm[xw|yw|zz|wx] = WWZX; mm[xw|yw|zz|wy] = WWZY;
    mm[xw|yw|zz|wz] = WWZZ; mm[xw|yw|zz|ww] = WWZW;
    mm[xw|yw|zw|wx] = WWWX; mm[xw|yw|zw|wy] = WWWY;
    mm[xw|yw|zw|wz] = WWWZ; mm[xw|yw|zw|ww] = WWWW;

    //  Translator state inicializations.
    alpha_func = D3DCMP_ALWAYS;
}

//  Definition node visitor.
void IRTranslator::visit(DefinitionIRNode *n)
{
    //  Get the destionation register of the definition.
    DestinationParameterIRNode *dest = n->getDestination();
    
    //  Create the D3D9 register identifier for the destination register.
    D3DRegisterId d3dreg(dest->getNRegister(), dest->getRegisterType());
    
    //  Check the type of destination register.
    switch(dest->getRegisterType())
    {
        case D3DSPR_CONST:
            {        
                //  Read the 4 tokens with the values defined for the destination register.
                float ftokens[4];
                for(u32bit i = 0; i < 4; i++)
                {
                    FloatIRNode *f = static_cast<FloatIRNode *>(n->getChilds()[1 + i]);
                    ftokens[i] = f->getValue();
                }
                
                //  Create a constant value from the values recieved 
                ConstValue value(ftokens);
                
                //  Declare and set the value of the D3D9 constant register and map to an ATTILA constant register.
                declareMapAndReserveConst(d3dreg, value);
            }
                        
            break;
            
        case D3DSPR_CONST2:
        case D3DSPR_CONST3:
            panic("IRTranslator", "visit(DefinitionNode)", "Extra float point constant register spaces not implemented.");
            break;

        case D3DSPR_CONSTBOOL:
            {        
                //  Read 1 token with the value defined for the destination register.
                bool btoken;
                BoolIRNode *b = static_cast<BoolIRNode *>(n->getChilds()[1]);
                btoken = b->getValue();
                
                //  Create a constant value from the values recieved 
                ConstValue value(btoken);
                
                //  Declare and set the value of the D3D9 constant register and map to an ATTILA constant register.
                declareMapAndReserveConst(d3dreg, value);
            }
            break;

        case D3DSPR_CONSTINT:
            {        
                //  Read the 4 tokens with the values defined for the destination register.
                s32bit intTokens[4];
                for(u32bit i = 0; i < 4; i++)
                {
                    IntegerIRNode *t = static_cast<IntegerIRNode *>(n->getChilds()[1 + i]);
                    intTokens[i] = t->getValue();
                }
                
                //  Create a constant value from the values recieved 
                ConstValue value(intTokens);
                
                //  Declare and set the value of the D3D9 constant register and map to an ATTILA constant register.
                declareMapAndReserveConst(d3dreg, value);
            }
            break;
                 
        default:
            panic("IRTranslator", "visit(DefinitionNode)", "Register type doesn't support value definition.");
            break;
    }            
}

//  Visitor for end node.
void IRTranslator::visit(EndIRNode *n)
{

    //  Check for pixel shader 1.x.
    if((type == PIXEL_SHADER) && (version < 0x0200))
    {
        //  The D3D9 r0 register (temporary register) is aliased to the color output register.
        //  Generate a MOV instruction from the temporary register to the actual color output register.
        generateMov(mappedRegister(D3DRegisterId(0,D3DSPR_COLOROUT)), mappedRegister(D3DRegisterId(0, D3DSPR_TEMP)));
    }
    
    //  Check for pixel shader version 2.x or below and pixel fog emulation.
    if ((type == PIXEL_SHADER) && (version < 0x0300) && fog_enabled)
    {
        //
        //  Emulate per pixel fog.
        //
        //      ColorOut = ColorIn * FogFactor + (1 - FogFactor) * FogColor
        //
        //      mad color, color, fogFactor.x, fogColor
        //      mad color, fogColor, -fogFactor.x, color
        //
        
        ShaderInstructionBuilder builder;
        Operand op1, op2, op3;
        Result res;
        
        //  Get the ATTILA register to use.
        GPURegisterId color = mappedRegister(D3DRegisterId(0, D3DSPR_COLOROUT));
        GPURegisterId fogColor = GPURegisterId(fog_declaration.fog_const_color, gpu3d::PARAM);
        GPURegisterId fogFactor = GPURegisterId(13, gpu3d::IN);
        
        //  Reset operands and results.
        op1 = op2 = op3 = Operand();
        res = Result();

        //  MAD color, color, fogFactor.x, fogColor
        builder.setOpcode(gpu3d::MAD);
        op1.registerId = color;
        builder.setOperand(0, op1);
        op2.registerId = fogFactor;
        op2.swizzle = gpu3d::XXXX;
        builder.setOperand(1, op2);
        op3.registerId = fogColor;
        builder.setOperand(2, op3);
        res.registerId = color;
        res.maskMode = gpu3d::XYZN;
        builder.setResult(res);
        instructions.push_back(builder.buildInstruction());
        
        //  Reset operands and results.
        op1 = op2 = op3 = Operand();
        res = Result();

        //  MAD color, fogColor, -fogFactor.x, color
        builder.setOpcode(gpu3d::MAD);
        op1.registerId = fogColor;
        builder.setOperand(0, op1);
        op2.registerId = fogFactor;
        op2.negate = true;
        op2.swizzle = gpu3d::XXXX;
        builder.setOperand(1, op2);
        op3.registerId = color;
        builder.setOperand(2, op3);
        res.registerId = color;
        res.maskMode = gpu3d::XYZN;
        builder.setResult(res);
        instructions.push_back(builder.buildInstruction());
    }   
    
    //  Check for pixel shader and alpha test emulation.
    if((type == PIXEL_SHADER) && (alpha_func != D3DCMP_ALWAYS))
    {        
        //  When alpha test emulation is required the D3D9 color output register from the pixel shader is mapped
        //  to a temporary register.  Before generating the instructions for alpha test emulation
        //  the D3D9 color output register is unmapped from the temporary register and mapped to the
        //  actual ATTILA color output register.
        
        //  Unmap color output from temp register
        GPURegisterId color_temp = unmapRegister(D3DRegisterId(0, D3DSPR_COLOROUT));
        releaseTemp(color_temp);
        
        //  Reserve temp register again to forbid extra code generation to use it as temp.
        reserveTemp(color_temp);
        
        //  Declare color output as usual
        GPURegisterId color_out = declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, 0), D3DRegisterId(0,D3DSPR_COLOROUT));
        
        //  Generate the extra code to emulate the alpha test.
        generate_extra_code(color_temp, color_out);
    }
    else if ((type == PIXEL_SHADER) && (alpha_func == D3DCMP_ALWAYS) && fog_enabled)
    {
        //  Unmap color output from temp register
        GPURegisterId color_temp = unmapRegister(D3DRegisterId(0, D3DSPR_COLOROUT));
        releaseTemp(color_temp);
        
        //  Reserve temp register again to forbid extra code generation to use it as temp.
        reserveTemp(color_temp);

        //  Declare color output as usual
        GPURegisterId color_out = declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, 0), D3DRegisterId(0,D3DSPR_COLOROUT));

        //  Copy the result color to the output color register.
        generateMov(color_out, color_temp);    
    }

    //  Check if the last jump instruction is jumping beyond the end of the program.
    if (lastJumpTarget == instructions.size())
    {
        //  Add an END instruction.
        ShaderInstructionBuilder builder;
        builder.setOpcode(END);
        instructions.push_back(builder.buildInstruction());
    }
    else
    {
        //  Set end flag in the last shader instruction.
        instructions.back()->setEndFlag(true);
    }
    
    // NOTE: All D3D9 constant registers must be declared because with VS relative
    //       addressing not only constants that appear in the code are used.
    // TODO: Is this really required?  Use a flag checking for relative addressing mode.
    //       Take care with integration with alpha test/fog code because they also use some constants.
    //       Pixel shader modes that support relative addressing.    
    /*if(type == VERTEX_SHADER)
    {
        for (u32bit c = 0; c < 256; c ++)
        {
            if (!isMapped(D3DRegisterId(c, D3DSPR_CONST)))
            {
                declareMapAndReserveConst(D3DRegisterId(c, D3DSPR_CONST));
            }
        }
    }*/
}

//  Generate a MOV shader instruction that copies the value from a source register to a destination register.
void IRTranslator::generateMov(GPURegisterId dest, GPURegisterId src)
{
    ShaderInstructionBuilder builder;
    builder.setOpcode(MOV);
    Operand op;
    op.registerId = src;
    builder.setOperand(0, op);
    Result res;
    res.registerId = dest;
    builder.setResult(res);
    instructions.push_back(builder.buildInstruction());
}

//  Release an ATTILA temporary register.
void IRTranslator::releaseTemp(GPURegisterId tmp)
{
    //  Check that the register being released is a temporary register.
    if(tmp.bank != TEMP)
        panic("IRTranslator", "releaseTemp", "Trying to release a non temporary register");
        
    //  Add to the list of available temporary registers.
    availableTemp.insert(tmp);
}

//  Reserves an ATTILA temporary register and maps the D3D9 register to the ATTILA temporary
//  register.
GPURegisterId IRTranslator::reserveAndMapTemp(D3DRegisterId d3dreg)
{
    //  Reserve a temporary register.
    GPURegisterId reserved = reserveTemp();
    
    //  Map the D3D9 register to the ATTILA temporary register.
    mapRegister(d3dreg, reserved);
    return reserved;
}

//  Reserves an ATTILA temporary register.
GPURegisterId IRTranslator::reserveTemp(GPURegisterId gpu_temp)
{
    //  Check that the input register is a temporary register.
    if (gpu_temp.bank != gpu3d::TEMP)
        panic("IRTranslator", "reserveTemp", "Trying to declare a non temporary register as temporary register.");
    
    //  Check if the temporary register requested is available.
    set<GPURegisterId>::iterator itT = availableTemp.find(gpu_temp);
    if (itT == availableTemp.end())
        panic("IRTranslator", "reserveTemp", "Trying to declare a reserved temp register.");
        
    //  Remove the temporary register from the list of available temporary registers.
    availableTemp.erase(itT);
    
    return gpu_temp;
}


//  Reserves an ATTILA temporary register.
GPURegisterId IRTranslator::reserveTemp()
{
    //  Check if there are available temporary registers.
    if (availableTemp.begin() == availableTemp.end())    
        panic("IRTranslator", "reserveTemp", "Run out of temporary registers");
        
    //  Get the next available temporary register.
    set<GPURegisterId>::iterator itN = availableTemp.begin();
    GPURegisterId declared = *itN;
    
    //  Remove the temporary register from the list of available temporary registers.
    availableTemp.erase(itN);

    return declared;
}

//  Releases a predicate register.
void IRTranslator::releasePredicate(GPURegisterId pred)
{
    //  Check the type of the requested register.
    if(pred.bank != gpu3d::PRED)
        panic("IRTranslator", "releasePredicate", "Trying to undeclare a non temp register");
    
    //  Add to the list of available predicate registers.
    availablePredicates.insert(pred);
}

//  Reserv and map a predicate register.
GPURegisterId IRTranslator::reserveAndMapPredicate(D3DRegisterId d3dreg)
{
    //  Reserve a predicate register.
    GPURegisterId reserved = reservePredicate();
    
    //  Map the D3D9 register to the ATTILA predicate register.
    mapRegister(d3dreg, reserved);
    
    return reserved;
}

//  Reserve a predicate register.
GPURegisterId IRTranslator::reservePredicate(GPURegisterId pred)
{
    //  Check the type of the requested register.
    if(pred.bank != gpu3d::PRED)
        panic("IRTranslator", "reservePredicate", "Trying to declare a non temp register as temp");
    
    //  Search for the register in the list of available predicate registers.
    set<GPURegisterId>::iterator itT = availablePredicates.find(pred);
    
    //  Check if the predicate register is no longer available.
    if(itT == availablePredicates.end())
        panic("IRTranslator", "reservePredicate", "Trying to declare a reserved temp register");
        
    //  Remove the predicate register from the list of available predicate registers.
    availablePredicates.erase(itT);
    
    return pred;
}


//  Reserve a predicate register.
GPURegisterId IRTranslator::reservePredicate()
{
    //  Check if there are no available predicate registers.
    if (availablePredicates.begin() == availablePredicates.end())
        panic("IRTranslator", "reservePredicate", "Run out of predicate registers");
        
    //  Get the next available predicate register.
    set<GPURegisterId>::iterator itN = availablePredicates.begin();
    GPURegisterId declared = *itN;
    
    //  Remove the predicate register from the list of available predicate registers.    
    availablePredicates.erase(itN);

    return declared;
}

//  Declares a D3D9 sampler register, reserves an ATTILA texture register and maps the D3D9 sampler
//  register to the ATTILA texture register.
GPURegisterId IRTranslator::declareMapAndReserveSampler(D3DSAMPLER_TEXTURE_TYPE textureType, D3DRegisterId d3d_register)
{
    //  Direct mapping from D3D9 samplers to ATTILA texture units.
    
    //  Check the range of the D3D9 sampler.
    if (d3d_register.num >= MAX_TEXTURES)
        panic("IRTranslator", "declareMapAndReserveSampler", "Sampler identifier out of range.");
    
    //  Create the corresponding ATTILA texture register.
    GPURegisterId declared;
    declared.bank = gpu3d::TEXT;
    
    //  Check for vertex shader version 3.0.
    if ((type == VERTEX_SHADER) && (version == 0x300))
    {
        //  Map vertex texture samplers to ATTILA sampler 12 to 15.
        declared.num = 12 + d3d_register.num;
    }
    else
        declared.num = d3d_register.num;
    
    //  Declare the D3D9 sampler register.
    samplerDeclaration.push_back(SamplerDeclaration(d3d_register.num, declared.num, textureType));

    //  Map the D3D9 sampler register to the ATTILA texture register.
    mapRegister(d3d_register, declared);

    return declared;
}

//  Declares a D3D9 register as output register with a given output usage, reserves an ATTILA register
//  and maps the D3D9 register to the ATTILA register.
GPURegisterId IRTranslator::declareMapAndReserveOutput(D3DUsageId usage, D3DRegisterId d3d_register)
{
    //  Mapping of output registers to ATTILA register is fixed and depends on the output usage.
    
    //  Get the ATTILA register associated with the given output usage.
    GPURegisterId reserved;
    reserved = outputUsage[usage];

    //  Check if there is an ATTILA output register reserved for the requested output semantic usage.
    if (reserved.bank == gpu3d::INVALID)
        panic("IRTranslator", "declareMapAndReserveOutput", "Output semantic usage is not mapped to an ATTILA output register.");
    
    //  Declare the output.
    outputDeclaration.push_back(OutputRegisterDeclaration(usage.usage, usage.index, reserved.num));

    //  Map the D3D9 register to the ATTILA register.
    mapRegister(d3d_register, reserved);

    return reserved;
}

//  Declares a D3D9 register as input register with a given input ussage, reserves an ATTILA register
//  and maps the D3D9 register to the ATTILA register.
GPURegisterId IRTranslator::declareMapAndReserveInput(D3DUsageId usage, D3DRegisterId d3d_register)
{
    GPURegisterId reserved;

    //  Check shader type.
    if(type == VERTEX_SHADER)
    {
        // Get the next available ATTILA input register
        set<GPURegisterId>::iterator it = availableInput.begin();        
        reserved = *it;
        
        //  Remove the ATTILA input register from the list of available input registers.
        availableInput.erase(it);
    }
    else
    { 
        // PIXEL_SHADER
        
        //  Check shader version.        
        if(version < 0x0300)
        {
            //  For PS 1.x and 2.x usage is defined by register type and number.
            switch (d3d_register.type)
            {
                case D3DSPR_INPUT:
                    usage.index = d3d_register.num;
                    usage.usage = D3DDECLUSAGE_COLOR;
                    break;
                case D3DSPR_TEXTURE:
                    usage.index = d3d_register.num;
                    usage.usage = D3DDECLUSAGE_TEXCOORD;
                    break;
                default:
                    panic("IRTranslator", "declareMapAndReserveInput", "Undefined input register type for pixel shader 1.x and 2.x");
                    break;
            }
        }
        else
        {
            //  Check for special input registers in ps 3.0.
            
            //  Check for vFace input register.
            if (d3d_register == D3DRegisterId(D3DSMO_FACE, D3DSPR_MISCTYPE))
            {
                //  Set usage to position and index to 15.
                usage.index = 15;
                usage.usage = D3DDECLUSAGE_POSITION;
            }
        }

        //  For pixel shaders the mapping to ATTILA input registers is fixed.
        
        //  Get the ATTILA input register associated with the input usage.
        reserved = inputUsage[usage];

        //  Check if there is an ATTILA input register reserved for the requested output semantic usage.
        if (reserved.bank == gpu3d::INVALID)
            panic("IRTranslator", "declareMapAndReserveInput", "Input semantic usage is not mapped to an ATTILA input register.");

        //  Remove the ATTILA input register from the list of available ATTILA input registers.
        availableInput.erase(reserved);
    }

    //  Declare the input.
    inputDeclaration.push_back(InputRegisterDeclaration(usage.usage, usage.index, reserved.num));
    
    //  Map the D3D9 input register to the ATTILA register.
    mapRegister(d3d_register, reserved);

    return reserved;
}

//  Checks if a D3D9 register is mapped to an ATTILA register.
bool IRTranslator::isMapped(D3DRegisterId d3dreg)
{
    return registerMap.find(d3dreg) != registerMap.end();
}

//  Reserves an ATTILA register and map the D3D9 register to the reserved ATTILA register.
GPURegisterId IRTranslator::mapAndReserveRegister(D3DRegisterId d3dreg)
{
    // Check if the D3D9 register is already mapped to an ATTILA register.
    if (registerMap.find(d3dreg) != registerMap.end())
        panic("IRTranslator", "mapAndReserveRegister", "Register already declared.");

    GPURegisterId reserved;
    
    //  Check the D3D9 register type.
    switch(d3dreg.type)
    {
        case D3DSPR_CONST:
        case D3DSPR_CONSTINT:
        case D3DSPR_CONSTBOOL:
        
            //  Declare, reserve and map a constnat register.
            reserved = declareMapAndReserveConst(d3dreg);
            break;

        case D3DSPR_TEMP:
        
            //  Reserve and map a temporary register.
            reserved = reserveAndMapTemp(d3dreg);
            break;
            
        default:
            
            D3D_DEBUG( cout << "IRTranslator: WARNING: D3D register type not supported" << endl; )
            ///@note The intention is to avoid halting simulation
            reserved = GPURegisterId(0, gpu3d::INVALID);
            error = true;
            break;
    }
    
    return reserved;
}

//  Maps a D3D9 register to an ATTILA register.
void IRTranslator::mapRegister(D3DRegisterId d3dreg, GPURegisterId gpureg)
{
    // Check if the D3D9 register is already mapped to an ATTILA register.
    if (registerMap.find(d3dreg) != registerMap.end())
        panic("IRTranslator", "mapRegister", "Register already mapped.");

    //  Map the D3D9 register to the ATTILA register.
    registerMap[d3dreg] = gpureg;
}

//  Remove the mapping of D3D9 register to an ATTILA register.
GPURegisterId IRTranslator::unmapRegister(D3DRegisterId d3dreg)
{
    //  Find the mapping for the D3D9 register.
    map<D3DRegisterId, GPURegisterId>::iterator it;
    it = registerMap.find(d3dreg);
    
    // Check if the D3D9 register was not mapped to an ATTILA register.
    if (it == registerMap.end())
        panic("IRTranslator", "unmapRegister", "Register not mapped.");

    //  Get ATTILA register to which the D3D9 register is mapped.
    GPURegisterId result = (*it).second;
    
    //  Remove the mapping.
    registerMap.erase(it);
    
    return result;
}

//  Declare a D3D9 constant register, reserve an ATTILA constant register and map the D3D9 register to the
//  ATTILA constant register.
GPURegisterId IRTranslator::declareMapAndReserveConst(D3DRegisterId d3dreg)
{
    // NOTE: D3D constants register is assigned to a gpu register keeping the same number.
    //       this is important because constants are sometimes accesed in indexed mode,
    //       so the order has to be the same.

    GPURegisterId reserved;
    reserved.bank = gpu3d::PARAM;
    
    //  Check the type of constant register.
    switch(d3dreg.type)
    {
        case D3DSPR_CONST:
        
            //  Get the ATTILA constant register to which the D3D9 constant register is mapped (direct mapping).
            reserved.num = d3dreg.num;
            break;

        case D3DSPR_CONST2:
        case D3DSPR_CONST3:
            panic("IRTranslator", "declareMapAndReserveConst", "Extra float point constant space not implemented.");
            break;
            
        case D3DSPR_CONSTINT:

            //  Get the ATTILA constant register to which the D3D9 constant register is mapped (direct mapping).
            reserved.num = d3dreg.num + INTEGER_CONSTANT_START;
            break;

        case D3DSPR_CONSTBOOL:

            //  Get the ATTILA constant register to which the D3D9 constant register is mapped (direct mapping).
            reserved.num = d3dreg.num + BOOLEAN_CONSTANT_START;
            break;
    }

    //  Check if the ATTILA constant register is available.    
    set<GPURegisterId>::iterator it = availableConst.find(reserved);
    if(it == availableConst.end())
    {
        char buffer[128];
        sprintf(buffer, "Constant %d not available.", d3dreg.num);
        panic("IRTranslator", "declareMapAndReserveConst", buffer);
    }
    
    //  Remove the ATTILA constant register from the list of available ATTILA constant registers.
    availableConst.erase(it);

    //  Map the D3D9 constant register to the ATTILA constant register.
    mapRegister(d3dreg, reserved);

    //  Declare the constant with default value.
    ConstValue value;
    constDeclaration.push_back(ConstRegisterDeclaration(reserved.num, d3dreg.num, false, value));
    
    return reserved;
}

//  Declare a D3D9 constant register with the defined value, reserve the ATTILA constant register
//  and map the D3D9 register to the ATTILA constant register.
GPURegisterId IRTranslator::declareMapAndReserveConst(D3DRegisterId d3dreg, ConstValue value)
{
    //  NOTE: D3D constants register is assigned to a gpu register keeping the same number.
    //        this is important because constants are sometimes accesed in indexed mode,
    //        so the order has to be the same.

    GPURegisterId reserved;
    reserved.bank = gpu3d::PARAM;
    
    //  Check the type of constant register.
    switch(d3dreg.type)
    {
        case D3DSPR_CONST:
        
            //  Get the ATTILA constant register to which the D3D9 constant register is mapped (direct mapping).
            reserved.num = d3dreg.num;
            break;

        case D3DSPR_CONST2:
        case D3DSPR_CONST3:
            panic("IRTranslator", "declareMapAndReserveConst(ConstantValue)", "Extra float point constant space not implemented.");
            break;
            
        case D3DSPR_CONSTINT:

            //  Get the ATTILA constant register to which the D3D9 constant register is mapped (direct mapping).
            reserved.num = d3dreg.num + INTEGER_CONSTANT_START;
            break;

        case D3DSPR_CONSTBOOL:

            //  Get the ATTILA constant register to which the D3D9 constant register is mapped (direct mapping).
            reserved.num = d3dreg.num + BOOLEAN_CONSTANT_START;
            break;
    }

    //  Check if the ATTILA constant register is available.    
    set<GPURegisterId>::iterator it = availableConst.find(reserved);
    if (it == availableConst.end())
        panic("IRTranslator", "declareMapAndReserveConst(ConstantValue)", "Constant not available");
    
    //  Remove the ATTILA constant register from the list of available ATTILA constant registers.
    availableConst.erase(it);

    //  Map the D3D9 constant register to the ATTILA constant register.
    mapRegister(d3dreg, reserved);

    //  Declare the constant with the defined value.
    constDeclaration.push_back(ConstRegisterDeclaration(reserved.num, d3dreg.num, true, value));
    
    return reserved;
}

//  Initialize the translation of the D3D9 shader program.
void IRTranslator::begin()
{
    //  Clear declarations.
    inputDeclaration.clear();
    outputDeclaration.clear();
    constDeclaration.clear();
    samplerDeclaration.clear();
    
    //  Set default fixed function state.
    alpha_test_declaration.comparison = alpha_func;
    alpha_test_declaration.alpha_const_minus_one = 0;
    alpha_test_declaration.alpha_const_ref = 0;
    fog_declaration.fog_enabled = fog_enabled;
    fog_declaration.fog_const_color = 0;
    
    //  Set information about the current translation.
    untranslatedControlFlow = false;
    untranslatedInstructions = 0;
    insideIFBlock = false;
    insideELSEBlock = false;
    currentIFBlocks.clear();
    insideREPBlock = false;    
    currentREPBlocks.clear();
    lastJumpTarget = 0;

    predication.clear();
    
    //  Add a first level with no predication.
    PredicationInfo topPredication;
    topPredication.predicated = false;
    topPredication.negatePredicate = false;
    topPredication.predicateRegister = 0;
    predication.push_back(topPredication);
    
    //  Delete the shader instructions from previous translations.
    delete_instructions();
}

//  Destructor.
IRTranslator::~IRTranslator()
{
    //  Delete the shader instructions from previous translations.
    delete_instructions();
}

//  Delete the shader instructions from previous translations.
void IRTranslator::delete_instructions()
{
    vector<ShaderInstruction*>::iterator it_i;
    for(it_i = instructions.begin(); it_i != instructions.end(); it_i++)
        delete(*it_i);
    instructions.clear();
}

//  Creates a NativeShader object with the shader translation and the associated information.
NativeShader *IRTranslator::build_native_shader()
{
    //  Compute the size of the ATTILA shader program in bytes.
    u32bit lenght = 16 * instructions.size();
    
    //  Allocate memory for the ATTILA shader program.
    u8bit *bytecode = new u8bit[lenght];
    
    vector<ShaderInstruction *>::iterator it;
    u32bit offset = 0;

    //  Get the bytecode for the ATTILA shader program.
    for(it = instructions.begin(); it != instructions.end(); it++)
    {
        //  Get the bytecode for the current instruction.
        (*it)->getCode(&bytecode[offset]);
        offset += 16;
    }

    //  Create declaration information for the shader program.
    NativeShaderDeclaration declaration(constDeclaration, inputDeclaration,
                                        outputDeclaration, samplerDeclaration,
                                        alpha_test_declaration, fog_declaration);
                                        
    return new NativeShader(declaration, bytecode, lenght);
}

//  Return a reference to the list of ATTILA shader instructions for the translated program.
std::vector<ShaderInstruction*> &IRTranslator::get_instructions()
{
    return instructions;
}

//  Translate the D3D9 write mask mode to the corresponding ATTILA write mask mode.
MaskMode IRTranslator::nativeMaskMode(DWORD d3dmask)
{
    //  Search for the corresponding ATTILA write mask mode.
    map<DWORD, MaskMode>::iterator itM = maskModeMap.find(d3dmask);
    
    //  Check if a translation was found.    
    if (itM == maskModeMap.end())
        panic("IRTranslator", "IRTranslator", "Unknown d3d mask");
    
    return (*itM).second;
}

//  Translate the D3D9 swizzle mode to the corresponding ATTILA swizzle mode.
SwizzleMode IRTranslator::nativeSwizzleMode(DWORD d3dswizz)
{
    //  Search for the corresponding ATTILA swizzle mode.
    map<DWORD, SwizzleMode>::iterator itS = swizzleModeMap.find(d3dswizz);
    
    //  Check if a translation was found.
    if (itS != swizzleModeMap.end())
        return (*itS).second;
    else
        return XYZW;
}

//  Initialize the shader registers based on the shader type and version.
void IRTranslator::initializeRegisters()
{
    // NOTE: If alpha test is enabled d3d two constant registers are not mapped to any gpu param register
    //       because they are used to store alpha reference and (-1, -1, -1, -1)    

    //  Allocate constant registers.
    availableConst.clear();
    for(u32bit c = 0; c < UNIFIED_CONSTANT_NUM_REGS; c++)
        availableConst.insert(GPURegisterId(c, gpu3d::PARAM));

    //  Allocate temporary registers.
    availableTemp.clear();
    for(u32bit r = 0; r < UNIFIED_TEMPORARY_NUM_REGS; r++)
        availableTemp.insert(GPURegisterId(r, gpu3d::TEMP));
    
    //  Allocate input registers.
    availableInput.clear();
    for(u32bit r = 0; r < UNIFIED_INPUT_NUM_REGS; r++)
        availableInput.insert(GPURegisterId(r, gpu3d::IN));
    
    //  Allocate output registers.
    availableOutput.clear();
    for(u32bit r = 0; r < UNIFIED_OUTPUT_NUM_REGS; r++)
        availableOutput.insert(GPURegisterId(r, gpu3d::OUT));

    //  Allocate predicate registers.
    availablePredicates.clear();
    for(u32bit p = 0; p < UNIFIED_PREDICATE_NUM_REGS; p++)
        availablePredicates.insert(GPURegisterId(p, gpu3d::PRED));

    //  Allocate texture units/samplers.
    availableSamplers.clear();
    for(u32bit t = 0; t < MAX_TEXTURES; t++)
        availableSamplers.insert(GPURegisterId(t, gpu3d::TEXT));
    
    //  Clear the mapping of D3D9 registers to ATTILA regsiters.
    registerMap.clear();
    
    //  Clear the D3D9 input semantic usages.
    inputUsage.clear();

    //  Clear the D3D9 output semantic usages.
    outputUsage.clear();

    //  Check the shader type.
    if(type == VERTEX_SHADER)
    {
        //  Restrict ATTILA output register 0 for the position output semantic usage.
        outputUsage[D3DUsageId(D3DDECLUSAGE_POSITION, 0)] = GPURegisterId(0, gpu3d::OUT);

        //  Restrict ATTILA output registers 1 and 2 for the two color output semantic usages.
        for(u32bit r = 0; r < 2; r++)
            outputUsage[D3DUsageId(D3DDECLUSAGE_COLOR, r)] = GPURegisterId(1 + r, gpu3d::OUT);

        //  Restrict ATTILA output registers 3 to 12 for texture coordinates.
        for(u32bit r = 0; r < 10; r++)
            outputUsage[D3DUsageId(D3DDECLUSAGE_TEXCOORD, r)] = GPURegisterId(3 + r, gpu3d::OUT);

        //  Restrict ATTILA output register 14 for the fog output semantic usage.
        outputUsage[D3DUsageId(D3DDECLUSAGE_FOG, 0)] = GPURegisterId(13, gpu3d::OUT);
        
        //  Restrict ATTILA output register 15 for point size output semantic usage.
        outputUsage[D3DUsageId(D3DDECLUSAGE_PSIZE, 0)] = GPURegisterId(15, gpu3d::OUT);

        //  Check the shader version.
        switch(version)
        {
            case 0x0101:
                // v#
                // Input registers will be mapped when a declaration is found.
                
                // c#
                // Constants will be declared when found
                
                // r#
                // Temps will be declared when found
                
                // a0 (address register)
                mapRegister(D3DRegisterId(0, D3DSPR_ADDR), GPURegisterId(0, gpu3d::ADDR));
                
                // oPos (position output register)
                declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_POSITION, 0),  D3DRegisterId(D3DSRO_POSITION, D3DSPR_RASTOUT));
                
                // oFog (fog output register)
                declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_FOG, 0),  D3DRegisterId(D3DSRO_FOG, D3DSPR_RASTOUT));
                
                // oPts (point size output register)
                declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_PSIZE, 0),  D3DRegisterId(D3DSRO_POINT_SIZE, D3DSPR_RASTOUT));
                
                // oD# (two color output registers)
                for(u32bit r = 0; r < 2; r++)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, r), D3DRegisterId(r, D3DSPR_ATTROUT));
                
                // oT# (eight texture output registers)
                for(u32bit r = 0; r < 8; r++)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_TEXCOORD, r), D3DRegisterId(r, D3DSPR_OUTPUT));
                
                break;

            case 0x0200:
            case 0x0201:
            case 0x02FF:

                // v#
                // Input registers will be mapped when a declaration is found.
                
                // c#
                // Constants will be declared when found
                
                // r#
                // Temps will be declared when found
                
                // a0 (address register)
                mapRegister(D3DRegisterId(0,D3DSPR_ADDR), GPURegisterId(0, gpu3d::ADDR));
                
                // b#
                // Not supported
                
                // i#
                // Not supported
                
                // aL
                // Not supported
                
                // oPos (position output register)
                declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_POSITION, 0),  D3DRegisterId(D3DSRO_POSITION, D3DSPR_RASTOUT));
                
                // oFog (fog output register)
                declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_FOG, 0),  D3DRegisterId(D3DSRO_FOG, D3DSPR_RASTOUT));
                
                // oPts (point size output register)
                declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_PSIZE, 0),  D3DRegisterId(D3DSRO_POINT_SIZE, D3DSPR_RASTOUT));
                
                // oD# (two color output registers)
                for(u32bit r = 0; r < 2; r++)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, r), D3DRegisterId(r, D3DSPR_ATTROUT));
                
                // oT# (eight texture output registers)
                for(DWORD r = 0; r < 8; r++)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_TEXCOORD, r), D3DRegisterId(r, D3DSPR_OUTPUT));
                   
                break;

            case 0x0300:
                // v#
                // Input registers will be mapped when a declaration is found.
                
                // c#
                // Constants will be declared when found
                
                // r#
                // Temps will be declared when found
                
                // a0 (address register)
                mapRegister(D3DRegisterId(0, D3DSPR_ADDR), GPURegisterId(0, gpu3d::ADDR));
                
                // i#                
                // Not supported
                
                // b#
                // Not supported
                
                // p0
                // Not supported
                
                // s#
                // Samplers will be mapped when a declaration is found.
                
                // o#
                // Output registers will be mapped when a declaration is found.
                
                break;

            default:
                {
                    char buff[1024];
                    sprintf(buff, "Vertex shader version %04x not supported\n", version);
                    panic("IRTranslator", "initializeRegisters", buff);
                }
                break;                
        }
    }
    else
    {
    
        // IR_PIXEL_SHADER

        //  Check the alpha test function.            
        if (alpha_func != D3DCMP_ALWAYS)
        {
            //  Reserve the two constant registers required for alpha test emulation.
            declareMapAndReserveConst(D3DRegisterId(ALPHA_TEST_CONSTANT_MINUS_ONE, D3DSPR_CONST));
            declareMapAndReserveConst(D3DRegisterId(ALPHA_TEST_CONSTANT_REF, D3DSPR_CONST));
            
            alpha_test_declaration.alpha_const_minus_one = ALPHA_TEST_CONSTANT_MINUS_ONE;
            alpha_test_declaration.alpha_const_ref = ALPHA_TEST_CONSTANT_REF;
        }
        
        //  Check for fog.  Only for pixel shader 2.x and below.
        if ((version < 0x0300) && fog_enabled)
        {
            //  Reserve a constant register for fixed point pixel fog emulation.
            declareMapAndReserveConst(D3DRegisterId(FOG_CONSTANT_COLOR, D3DSPR_CONST));
            fog_declaration.fog_const_color = FOG_CONSTANT_COLOR;
        }

        //  Restrict ATTILA input register 0 for position input semantic usage.
        inputUsage[D3DUsageId(D3DDECLUSAGE_POSITION, 0)] = GPURegisterId(0, gpu3d::IN);

        //  Restrict ATTILA input registers 1 and 2 for color input semantic usage.
        for(u32bit r = 0; r < 2; r++) 
            inputUsage[D3DUsageId(D3DDECLUSAGE_COLOR, r)] = GPURegisterId(1 + r, gpu3d::IN);
        
        //  Reserve ATTILA input registers 3 to 12 for texture coordinate input semantic usage.
        for(u32bit r = 0; r < 10; r ++)
            inputUsage[D3DUsageId(D3DDECLUSAGE_TEXCOORD, r)] = GPURegisterId(3 + r, gpu3d::IN);
        
        //  Reserve ATTILA output registers 1 to 4 for color output.
        for(u32bit r = 0; r < 4; r++)
            outputUsage[D3DUsageId(D3DDECLUSAGE_COLOR, r)] = GPURegisterId(1 + r, gpu3d::OUT);
        
        //  Restrict ATTILA input register 15 for adhoc semantic created for face input register (vFace).
        inputUsage[D3DUsageId(D3DDECLUSAGE_POSITION, 15)] = GPURegisterId(15, gpu3d::IN);

        //  Reserver ATTILA output register 0 for depth output.
        outputUsage[D3DUsageId(D3DDECLUSAGE_POSITION, 0)] = GPURegisterId(0, gpu3d::OUT);

        GPURegisterId input;
        GPURegisterId textureTemp;
        switch(version)
        {
            case 0x0101:
            case 0x0102:
            case 0x0103:
                // c#
                // Constants will be declared when found
                
                // r#
                // Temps will be declared when found
                
                // oC0 (color output register)
                // If alpha test or pixel fog emulation is required map color output register to a temporary register.
                if ((alpha_func == D3DCMP_ALWAYS) && !fog_enabled)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, 0), D3DRegisterId(0, D3DSPR_COLOROUT));
                else
                    reserveAndMapTemp(D3DRegisterId(0, D3DSPR_COLOROUT));

                // t# (4 texture input registers)
                for(u32bit r = 0; r < 4; r++)
                {
                    //  Declare and map texture input registers to ATTILA registers.
                    declareMapAndReserveInput(D3DUsageId(D3DDECLUSAGE_TEXCOORD, r), D3DRegisterId(r, D3DSPR_TEXTURE));
                    
                    //  For pixel shader models 1.x the texture coordinate input registers are aliased to temporary registers.
                    
                    //  Unmap texture coordinate register from the ATTILA input register and map to a temporary register.
                    input = unmapRegister(D3DRegisterId(r, D3DSPR_TEXTURE));
                    textureTemp = reserveAndMapTemp(D3DRegisterId(r, D3DSPR_TEXTURE));
                    
                    //  Copy from the input ATTILA register to the temporary register.
                    generateMov(textureTemp, input);
                }
                
                // v# (color input registers)
                for(u32bit r = 0; r < 2; r++)
                    declareMapAndReserveInput(D3DUsageId(D3DDECLUSAGE_COLOR, r), D3DRegisterId(r, D3DSPR_INPUT));
                
                break;

            case 0x0104:
                // c#
                // Constants will be declared when found

                // r#
                // Temps will be declared when found

                // oC0 (color output register)
                // If alpha test or pixel fog emulation is required map color output semantic usage to a temporary register.
                if ((alpha_func == D3DCMP_ALWAYS) && !fog_enabled)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, 0), D3DRegisterId(0, D3DSPR_COLOROUT));
                else
                    reserveAndMapTemp(D3DRegisterId(0, D3DSPR_COLOROUT));
                 
                // t# (texture input registers)
                for(u32bit r = 0; r < 6; r ++)
                    declareMapAndReserveInput(D3DUsageId(D3DDECLUSAGE_TEXCOORD, r), D3DRegisterId(r, D3DSPR_TEXTURE));
                
                // v# (color input registers)
                for(u32bit r = 0; r < 2; r++)
                    declareMapAndReserveInput(D3DUsageId(D3DDECLUSAGE_COLOR, r), D3DRegisterId(r, D3DSPR_INPUT));
                
                break;

            case 0x0200:
            case 0x0201:
            case 0x02FF:
                // v#
                // Input registers are mapped when a declaration is found

                // c#
                // Constants will be declared when found

                // r#
                // Temps will be declared when found

                // i#
                // Not supported

                // b#
                // Not supported

                // p0

                // Not supported

                // s#
                // Samplers are mapped when a declaration is found

                // t# (texture input registers)
                // Will be mapped when a declaration is found.
                
                // oC# (four color output registers)
                // oC0 (color output register)
                // If alpha test or pixel fog emulation is required map color output semantic usage to a temporary register.
                if ((alpha_func == D3DCMP_ALWAYS) && !fog_enabled)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, 0), D3DRegisterId(0, D3DSPR_COLOROUT));
                else
                    reserveAndMapTemp(D3DRegisterId(0,D3DSPR_COLOROUT));

                for(u32bit r = 1; r < 4; r++)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, r), D3DRegisterId(r, D3DSPR_COLOROUT));
                    
                // oDepth0 (depth output register)
                declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_POSITION, 0), D3DRegisterId(0, D3DSPR_DEPTHOUT));
                
                break;
                
            case 0x0300:

                // v#
                // Input registers are mapped when a declaration is found               
                
                // c#
                // Constants will be declared when found
                
                // r#
                // Temps will be declared when found
                
                // i#
                // Not supported
                
                // b#
                // Not supported
                
                // p0
                // Not supported
                
                // s#
                // Samplers are mapped when a declaration is found
                
                // vFace
                // Will be mapped when a declaration is found.
                
                // vPos
                // Will be mapped when a declaration is found.
                
                // aL
                // Not supported
                
                // oC# (four color output registers)
                
                // oC0 (color output registers)
                // If alpha test emulation is required map color output semantic usage to a temporary register.
                if(alpha_func == D3DCMP_ALWAYS)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, 0), D3DRegisterId(0, D3DSPR_COLOROUT));
                else
                    reserveAndMapTemp(D3DRegisterId(0, D3DSPR_COLOROUT));
                
                for(u32bit r = 1; r < 4; r++)
                    declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_COLOR, r), D3DRegisterId(r, D3DSPR_COLOROUT));

                // oDepth0 (depth output register)
                declareMapAndReserveOutput(D3DUsageId(D3DDECLUSAGE_POSITION, 0), D3DRegisterId(0, D3DSPR_DEPTHOUT));
                
                break;
                
            default:
                {
                    char buff[1024];
                    sprintf(buff, "Pixel shader version %04x not supported\n", version);
                    panic("IRTranslator", "initializeRegisters", buff);
                }
                break;                
        }
    }
}

//  Initialize the mapping of D3D9 opcodes to ATTILA opcodes.
void IRTranslator::initializeOpcodes()
{
    opcodeMap.clear();
    
    //  Create the mapping table based on the shader type and version.
    if(type == VERTEX_SHADER)
    {
        opcodeMap[D3DSIO_ADD]  = ADD;
        opcodeMap[D3DSIO_DP3]  = DP3;
        opcodeMap[D3DSIO_DP4]  = DP4;
        opcodeMap[D3DSIO_DST]  = DST;
        opcodeMap[D3DSIO_EXP]  = EX2;
        if (version == 0x0101)
            opcodeMap[D3DSIO_EXPP] = EXP;
        else
            opcodeMap[D3DSIO_EXPP] = EX2;
        opcodeMap[D3DSIO_FRC]  = FRC;
        opcodeMap[D3DSIO_LIT]  = LIT;
        opcodeMap[D3DSIO_LOG]  = LG2;
        opcodeMap[D3DSIO_LOGP] = LOG;
        opcodeMap[D3DSIO_MAD]  = MAD;
        opcodeMap[D3DSIO_MAX]  = MAX;
        opcodeMap[D3DSIO_MIN]  = MIN;
        opcodeMap[D3DSIO_MOV]  = MOV;
        opcodeMap[D3DSIO_MOVA] = ARL;
        opcodeMap[D3DSIO_MUL]  = MUL;
        opcodeMap[D3DSIO_NOP]  = NOP;
        opcodeMap[D3DSIO_RCP]  = RCP;
        opcodeMap[D3DSIO_RSQ]  = RSQ;
        opcodeMap[D3DSIO_SGE]  = SGE;
        opcodeMap[D3DSIO_SLT]  = SLT;
        if(version >= 0x0300)
            opcodeMap[D3DSIO_TEXLDL] = TXL;
    }
    else
    {
        //IR_PIXEL_SHADER
        opcodeMap[D3DSIO_ADD]     = ADD;
        opcodeMap[D3DSIO_DP3]     = DP3;
        opcodeMap[D3DSIO_DP4]     = DP4;
        opcodeMap[D3DSIO_EXP]     = EX2;
        opcodeMap[D3DSIO_EXPP]    = EXP;
        opcodeMap[D3DSIO_FRC]     = FRC;
        opcodeMap[D3DSIO_LOG]     = LG2;
        opcodeMap[D3DSIO_MAD]     = MAD;
        opcodeMap[D3DSIO_MAX]     = MAX;
        opcodeMap[D3DSIO_MIN]     = MIN;
        opcodeMap[D3DSIO_MOV]     = MOV;
        opcodeMap[D3DSIO_MUL]     = MUL;
        opcodeMap[D3DSIO_NOP]     = NOP;
        opcodeMap[D3DSIO_RCP]     = RCP;
        opcodeMap[D3DSIO_RSQ]     = RSQ;
        opcodeMap[D3DSIO_TEXKILL] = KIL;
        if(version >= 0x0200)
        {
            opcodeMap[D3DSIO_TEX]    = TEX;
            opcodeMap[D3DSIO_TEXLDL] = TXL;
            opcodeMap[D3DSIO_DSX]    = DDX;
            opcodeMap[D3DSIO_DSY]    = DDY;
        }
	}
}

//  Visitor for version node.
void IRTranslator::visit(VersionIRNode *n)
{
    //  Get the shader program type and version.
    type = n->getType();
    version = n->getVersion();

    //  Initialize the translator state based on the shader type and version.
    initializeRegisters();
    initializeOpcodes();
}

//  Visitor for declaration node.
void IRTranslator::visit(DeclarationIRNode *n)
{
    // TODO check if there's exceptions, this castings get me nervous
    DestinationParameterIRNode *destination = static_cast<DestinationParameterIRNode*>(n->getChilds()[1]);
    D3DRegisterId d3dreg(destination->getNRegister(), destination->getRegisterType());

    SemanticIRNode *semantic = 0;
    SamplerInfoIRNode *info = 0;
    D3DUsageId usage;
    
    //  Check the register type.
    switch (destination->getRegisterType())
    {
        case D3DSPR_INPUT:
        
            //  Input register.  Get semantic usage and declare/map/reserve as input register.
            semantic = static_cast<SemanticIRNode*>(n->getChilds()[0]);
            usage.usage = semantic->getUsage();
            usage.index = semantic->getUsageIndex();
            declareMapAndReserveInput(usage, d3dreg);
            break;

        case D3DSPR_MISCTYPE:
        
            //  Special input registers in PS3.0.  Get semantic usage and declare/map/reserve as input register.
            switch(destination->getNRegister())
            {
                case D3DSMO_POSITION:
                    semantic = static_cast<SemanticIRNode*>(n->getChilds()[0]);
                    usage.usage = semantic->getUsage();
                    usage.index = semantic->getUsageIndex();
                    declareMapAndReserveInput(usage, d3dreg);
                    break;
                case D3DSMO_FACE:
                    semantic = static_cast<SemanticIRNode*>(n->getChilds()[0]);
                    usage.usage = semantic->getUsage();
                    usage.index = semantic->getUsageIndex();
                    declareMapAndReserveInput(usage, d3dreg);
                    break;
            }
            break;
            
        case D3DSPR_TEXTURE:
        
            //  Texture input registers (PS2.0).  Declare/map/reserve as input registers.
            semantic = static_cast<SemanticIRNode*>(n->getChilds()[0]);
            usage.usage = semantic->getUsage();
            usage.index = semantic->getUsageIndex();
            declareMapAndReserveInput(usage, d3dreg);
            break;

        case D3DSPR_SAMPLER:
           
            //  Sampler register.  Declare, reserve and map.        
            info = static_cast<SamplerInfoIRNode*>(n->getChilds()[0]);
            declareMapAndReserveSampler(info->getTextureType(), d3dreg);
            break;

        case D3DSPR_TEXCRDOUT:

            //  Output registers (VS3.0).  Declare/map/reserve as output registers.
            semantic = static_cast<SemanticIRNode*>(n->getChilds()[0]);
            usage.usage = semantic->getUsage();
            usage.index = semantic->getUsageIndex();
            declareMapAndReserveOutput(usage, d3dreg);
            break;
        
        default:
            {
                char buffer[128];
                sprintf(buffer, "Unexpected destination type %d for declaration at offset %x.", destination->getRegisterType(), n->getOffset());
                panic("IRTranslator", "visit(DeclarationIRNode)", buffer);
            }
            break;
    }
}

//  Process an instruction source operand (source parameter nodes associated with an instruction node).
void IRTranslator::visitInstructionSource(SourceParameterIRNode *n)
{
    //  Get the D3D9 register used as instruction source.
    D3DRegisterId d3dreg(n->getNRegister(), n->getRegisterType());
    
    //  Check if the D3D9 register is already mapped to an ATTILA register.
    if (!isMapped(d3dreg))
    {
        //  Map the D3D9 register to an ATTILA register.
        mapAndReserveRegister(d3dreg);
    }

    Operand op;    

    //  Get the ATTILA register to which the D3D9 register is mapped to.
    op.registerId = mappedRegister(d3dreg);    
    
    //  Translate the operand swizzling mode.
    op.swizzle = nativeSwizzleMode(n->getSwizzle(0) | n->getSwizzle(1) | n->getSwizzle(2) | n->getSwizzle(3));
    
    //  Check for registers with implicit swizzle.
    
    //  The pixel shader vFace register is mapped to the w component of input attribute 15.
    if (d3dreg == D3DRegisterId(D3DSMO_FACE, D3DSPR_MISCTYPE))
        op.swizzle = gpu3d::WWWW;
    
    //  Set operand modifiers.    
    switch(n->getModifier())
    {
        case D3DSPSM_NONE:
        
            //  By default already defined to no absolute and no negate.
            break;
            
        case D3DSPSM_NEG:
            
            //  Set negate modifer.
            op.negate = true;
            break;
            
        case D3DSPSM_ABS:
        
            //  Set absolute value modifier.
            op.absolute = true;
            break;
            
        case D3DSPSM_ABSNEG:
        
            //  Set absolute value and negate modifiers.
            op.absolute = true;
            op.negate = true;
            break;
            
        case D3DSPSM_BIAS:
        case D3DSPSM_BIASNEG:
        case D3DSPSM_SIGN:
        case D3DSPSM_SIGNNEG:
        case D3DSPSM_COMP:
        case D3DSPSM_X2:
        case D3DSPSM_X2NEG:
        case D3DSPSM_DZ:
        case D3DSPSM_DW:
        case D3DSPSM_NOT:
        
            //panic("IRTranslator", "visitInstructionSource", "Source operand modifier not implemented.");
            printf("IRTranslator::visitInstructionSource => Source operand modifier %08x not implemented\n", n->getModifier());
            break;
            
        default:
        
            panic("IRTranslator", "visitInstructionSource", "Undefined source operand modifier.");
            break;
    }
    
    //  Set the 
    
    //  Check if the source parameter uses relative addressing.
    if((n->getAddressMode() == D3DSHADER_ADDRMODE_RELATIVE))
    {
        //  Set the relative adressing parameters.
        op.addressing = true;
        relative.enabled = true;
        
        //  Check the shader version.
        if(version < 0x0200)
        {
            // Relative addressing uses a0 implicitely for version < 2.0
            relative.component = 0;
            relative.registerN = 0;
        }
        else
        {
            // There's a relative addressing token for versions >= 2.0
            RelativeAddressingIRNode *n_rel = n->getRelativeAddressing();
            relative.registerN = n_rel->getNRegister();
            relative.component = n_rel->getComponentIndex();
        }
        
        // NOTE: In simulator: cN[a0.x + offset]
        //       In Direct3D:  cN[a0.x]
        //       So offset is 0
        relative.offset = 0;
    }

    //  Add the instruction operand to the vector of operands for the current instruction
    //  being translated.
    operands.push_back(op);
}

//  Process instruction results (destination parameter node associated with an instruction node).
void IRTranslator::visitInstructionDestination(DestinationParameterIRNode *n)
{
    //  Get the D3D9 register used as the instruction result.
    D3DRegisterId d3dreg(n->getNRegister(), n->getRegisterType());
    
    //  Check if the D3D9 register is already mapped to an ATTILA register.
    if (!isMapped(d3dreg))
    {
        //  Map the D3D9 register to an ATTILA register.
        mapAndReserveRegister(d3dreg);
    }
    
    //  Set the instruction result information.
    result.registerId = mappedRegister(d3dreg);
    result.maskMode = nativeMaskMode(n->getWriteMask());
    result.saturate = ((n->getModifier() & D3DSPDM_SATURATE) != 0);
    
    //  Check for special registers with implicit write masks.
    
    //  Vertex shader fog output register is mapped to the x component of output attribute 13.
    if (d3dreg == D3DRegisterId(D3DSRO_FOG, D3DSPR_RASTOUT))
        result.maskMode = gpu3d::XNNN;

    //  Vertex shader point size register is mapped to the w component of output attribute 15.
    if (d3dreg == D3DRegisterId(D3DSRO_POINT_SIZE, D3DSPR_RASTOUT))
        result.maskMode = gpu3d::NNNW;

    //  Pixel shader depth output register is mapped to the z component of output attribute 0.
    if (d3dreg == D3DRegisterId(0, D3DSPR_DEPTHOUT))
        result.maskMode = gpu3d::NNZN;
    
    //  Check for special register that require implicit saturation.
    if (d3dreg == D3DRegisterId(D3DSRO_FOG, D3DSPR_RASTOUT))
        result.saturate = true;

}

//  Gets the ATTILA register to which a D3D9 register is mapped to.
GPURegisterId IRTranslator::mappedRegister(D3DRegisterId d3dreg)
{
    //  Search the D3D9 register in the mapping table.
    map<D3DRegisterId, GPURegisterId>::iterator itR;
    itR = registerMap.find(d3dreg);
    
    //  Check if the D3D9 register is mapped to an ATTILA register.
    if(itR != registerMap.end())
        return (*itR).second;
    else
    {
        D3D_DEBUG( cout << "IRTranslator: WARNING: Unmapped d3d register" << endl; )
        ///@note The intention is to avoid halting simulation
        error = true;
        return GPURegisterId(0, gpu3d::INVALID);
    }
}

//  Translate the M4X4 shader instruction to ATTILA shader instructions.
void IRTranslator::emulateM4X4(InstructionIRNode *n)
{
    // M4x4 references implicitly the three constants following source1,
    // so is necessary to declare them
    SourceParameterIRNode *source = n->getSources()[1];
    D3DRegisterId d3dconst(source->getNRegister(), source->getRegisterType());
    for(u32bit i = 0; i < 3; i ++)
    {
        d3dconst.num++;
        if (!isMapped(d3dconst))
            mapAndReserveRegister(d3dconst);
    }


    ShaderInstructionBuilder builder;
    builder.setOpcode(DP4);
    builder.setOperand(0, operands[0]);
    Operand auxOp = operands[1];
    Result auxRes = result;

    auxRes.maskMode = nativeMaskMode(D3DSP_WRITEMASK_0);
    builder.setResult(auxRes);
    builder.setOperand(1, auxOp);
    instructions.push_back(builder.buildInstruction());

    auxRes.maskMode = nativeMaskMode(D3DSP_WRITEMASK_1);
    builder.setResult(auxRes);
    auxOp.registerId.num ++;
    builder.setOperand(1, auxOp);
    instructions.push_back(builder.buildInstruction());

    auxRes.maskMode = nativeMaskMode(D3DSP_WRITEMASK_2);
    builder.setResult(auxRes);
    auxOp.registerId.num ++;
    builder.setOperand(1, auxOp);
    instructions.push_back(builder.buildInstruction());

    auxRes.maskMode = nativeMaskMode(D3DSP_WRITEMASK_3);
    builder.setResult(auxRes);
    auxOp.registerId.num ++;
    builder.setOperand(1, auxOp);
    instructions.push_back(builder.buildInstruction());

}

//  Translate the SUB instruction to ATTILA shader instructions.
void IRTranslator::emulateSUB()
{
    //  SUB dst, src0, src1
    //  -------------------------
    //  ADD dst, src0, -src1
    
    ShaderInstructionBuilder builder;
    builder.setOpcode(ADD);
    builder.setOperand(0, operands[0]);
    Operand auxOp = operands[1];
    auxOp.negate = !auxOp.negate;
    builder.setOperand(1, auxOp);
    builder.setResult(result);
    builder.setRelativeMode(relative);
    instructions.push_back(builder.buildInstruction());
}

//  Translate the TEXLD1314 instruction to ATTILA shader instructions.
void IRTranslator::emulateTEXLD1314(InstructionIRNode *n)
{
    DestinationParameterIRNode *destN;
    D3DRegisterId resultD3DRegister;
    D3DRegisterId sourceD3DRegister;
    ShaderInstructionBuilder builder;
    GPURegisterId resultRegister;
    Operand operand;
    
    //  Check the shader version.
    switch(version)
    {
        case 0x0101:
        case 0x0102:
        case 0x0103:
        
            destN = n->getDestination();
            resultD3DRegister = D3DRegisterId(destN->getNRegister(), destN->getRegisterType());
            builder.setOpcode(TEX);
            builder.setResult(result);
            resultRegister = result.registerId;
            operand.registerId = result.registerId;
            builder.setOperand(0, operand);
            operand.registerId = GPURegisterId(resultD3DRegister.num, gpu3d::TEXT);
            builder.setOperand(1, operand);
            instructions.push_back(builder.buildInstruction());
            break;
            
        case 0x0104:
        
            destN = n->getDestination();
            resultD3DRegister = D3DRegisterId(destN->getNRegister(), destN->getRegisterType());
            builder.setResult(result);
            builder.setOperand(0, operands[0]);
            operand.registerId = GPURegisterId(resultD3DRegister.num, gpu3d::TEXT);
            builder.setOperand(1, operand);
            instructions.push_back(builder.buildInstruction());
            break;
            
        default:
            {
                char buff[1024];
                sprintf(buff, "Pixel shader version %04x not supported\n", version);
                panic("IRTranslator", "emulateTEXLD1314", buff);
            }
            break;                
    }
}

//  Translate the LRP instruction to ATTILA shader instructions.
void IRTranslator::emulateLRP()
{

    //  LRP dst, src0, src1, src2
    //  -------------------------
    //  ADD t0, src1, -src2
    //  MAD dst, src0, t0, src2
        
    ShaderInstructionBuilder builder;
    GPURegisterId t0 = reserveTemp();

    // ADD t0, src1, -src2
    builder.resetParameters();
    builder.setOpcode(ADD);
    builder.setOperand(0, operands[1]);
    Operand op2neg = operands[2];
    op2neg.negate = !op2neg.negate;
    builder.setOperand(1, op2neg);
    Result res_t0;
    res_t0.registerId = t0;
    builder.setResult(res_t0);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());

    //    MAD dst, src0, r0, src2
    builder.resetParameters();
    builder.setOpcode(MAD);
    builder.setOperand(0, operands[0]);
    Operand op_t0;
    op_t0.registerId = t0;
    builder.setOperand(1, op_t0);
    builder.setOperand(2, operands[2]);
    builder.setResult(result);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());

    releaseTemp(t0);
}

//  Translate the POW instruction to ATTILA shader instructions.
void IRTranslator::emulatePOW()
{

    // POW(ABS(src0),src1) == EXP(src1 * LOG(ABS(src0)))
    // ---------------------------------------------------------------
    // POW dst, src0, src1
    // -------------------
    // LG2 t0, |src0|
    // MUL t0, t0, src1
    // EX2 dst, t0

    ShaderInstructionBuilder builder;
    GPURegisterId t0 = reserveTemp();

    //  Build => LG2 t0, |src0|
    builder.resetParameters();
    builder.setOpcode(gpu3d::LG2);
    Operand opTemp = operands[0];
    opTemp.absolute = true;
    builder.setOperand(0, opTemp);
    Result resTemp;
    resTemp.registerId = t0;
    resTemp.maskMode = gpu3d::XNNN;   
    builder.setResult(resTemp);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());

    //  Build => MUL t0, t0, src1
    builder.resetParameters();
    builder.setOpcode(gpu3d::MUL);
    opTemp.registerId = t0;
    opTemp.absolute = opTemp.negate = false;
    opTemp.swizzle = gpu3d::XXXX;    
    builder.setOperand(0, opTemp);
    builder.setOperand(1, operands[1]);
    builder.setResult(resTemp);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());
    
    //  Build => EX2 dst, dst
    builder.resetParameters();
    builder.setOpcode(gpu3d::EX2);
    builder.setOperand(0, opTemp);
    builder.setResult(result);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());    

    releaseTemp(t0);
}

//  Translate the NRM instruction to ATTILA shader instructions.
void IRTranslator::emulateNRM()
{
    // NRM dest, src0
    // ----------------------------
    // DP3 t0, src0, src0
    // RSQ t0, t0
    // MUL dest, src0, t0
    
    ShaderInstructionBuilder builder;
    GPURegisterId t0 = reserveTemp();

    //  Build => DP3 t0, src0, src0
    builder.resetParameters();
    builder.setOpcode(gpu3d::DP3);
    builder.setOperand(0, operands[0]);
    builder.setOperand(1, operands[0]);
    Result t0_res;
    t0_res.registerId = t0;
    t0_res.maskMode = gpu3d::XNNN;
    builder.setResult(t0_res);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());
   
    //  Build => RSQ t0, t0
    builder.resetParameters();
    builder.setOpcode(gpu3d::RSQ);
    Operand t0_tempOp;
    t0_tempOp.registerId = t0;
    t0_tempOp.swizzle = gpu3d::XXXX;
    builder.setOperand(0, t0_tempOp);
    builder.setResult(t0_res);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());
    
    //  Build => MUL dest, src0, t0
    builder.resetParameters();
    builder.setOpcode(gpu3d::MUL);
    builder.setOperand(0, operands[0]);
    builder.setOperand(1, t0_tempOp);
    builder.setResult(result);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());

    releaseTemp(t0);
}

//  Translate the CMP instruction to ATTILA shader instructions.
void IRTranslator::emulateCMP()
{
    //  Using the CMP instruction already implemented in the simulator:
    //
    //  CMP dst, src0, src1, src2
    //
    //     dst = (src < 0) ? src1 : src2
    // 
    //  D3D9 CMP dst, src0, src1, src2
    //
    //     dst = (src >= 0) ? src1 : src2
    //
    //  So the implementation has only to swap src1 with src2.
    //

    ShaderInstructionBuilder builder;

    builder.resetParameters();
    builder.setOpcode(CMP);
    builder.setOperand(0, operands[0]);
    builder.setOperand(1, operands[2]);
    builder.setOperand(2, operands[1]);
    builder.setResult(result);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());   
}

//  Translate the DP2ADD instruction to ATTILA shader instructions.
void IRTranslator::emulateDP2ADD()
{

    // DP2ADD dest, src0, src1, src2
    // ----------------------------
    //
    //  result is scalar (broadcast)
    //  src2 is scalar (broadcast)
    //
    //   dest = src0.x * src1.x + src0.y * src1.y + src2.x
    //
    //   MAD t0, src0.x, src1.x, src2
    //   MAD dest, src0.y, src1.y, t0
    //
    
    ShaderInstructionBuilder builder;
    GPURegisterId t0 = reserveTemp();

    Operand mad_src0;
    Operand mad_src1;
    Operand mad_src2;
    Result mad_res;
    
    //   MAD t0, src0.x, src1.x, src2
    builder.resetParameters();
    builder.setOpcode(MAD);        
    mad_src0 = operands[0];
    selectSwizzledComponent(0, operands[0], mad_src0);
    mad_src1 = operands[1];
    selectSwizzledComponent(0, operands[1], mad_src1);        
    builder.setOperand(0, mad_src0);
    builder.setOperand(1, mad_src1);
    builder.setOperand(2, operands[2]);    
    mad_res.registerId = t0;
    mad_res.maskMode = gpu3d::XNNN;
    builder.setResult(mad_res);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());

    //   MAD dest, src0.y, src1.y, t0
    builder.resetParameters();
    builder.setOpcode(MAD);    
    selectSwizzledComponent(1, operands[0], mad_src0);
    selectSwizzledComponent(1, operands[1], mad_src1);
    mad_src2.registerId = t0;
    mad_src2.swizzle = gpu3d::XXXX;
    builder.setOperand(0, mad_src0);
    builder.setOperand(1, mad_src1);
    builder.setOperand(2, mad_src2);
    builder.setResult(result);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());

    releaseTemp(t0);
}    

//  Translate the ABS shader instruction to ATTILA shader instructions.
void IRTranslator::emulateABS()
{
    // ABS dest, src
    // ----------------------------
    // MOV dest, |src|


    ShaderInstructionBuilder builder;
    Operand absOp0 = operands[0];
    absOp0.absolute = true;
    builder.resetParameters();
    builder.setOpcode(MOV);
    builder.setOperand(0, absOp0);        
    builder.setResult(result);
    builder.setPredication(predication.back());
    instructions.push_back(builder.buildInstruction());
}

//  Translate the SINCOS shader instruction to ATTILA shader instructions.
void IRTranslator::emulateSINCOS()
{

    // SINCOS dest, src
    // ----------------------------
    //
    //  if dest.x
    //
    //      COS dest.x, src
    //
    //  if dest.y
    //
    //      SIN dest.y, src
    //
    //  if dest.xy
    //
    //      COS dest.x, src
    //      SIN dest.y, src
    //
    
    ShaderInstructionBuilder builder;

    Result cos_res;
    Result sin_res;

    //  Check if the cosinus has to be computed.    
    if ((result.maskMode == gpu3d::XNNN) || (result.maskMode == gpu3d::XYNN))
    {
        //   COS dest.x, src
        builder.resetParameters();
        builder.setOpcode(COS);
        builder.setOperand(0, operands[0]);
        cos_res = result;
        cos_res.maskMode = gpu3d::XNNN;
        builder.setResult(cos_res);
        builder.setPredication(predication.back());
        instructions.push_back(builder.buildInstruction());
    }

    //  Check if the sinus has to be computed.    
    if ((result.maskMode == gpu3d::NYNN) || (result.maskMode == gpu3d::XYNN))
    {
        //   SIN dest.y, src
        builder.resetParameters();
        builder.setOpcode(SIN);
        builder.setOperand(0, operands[0]);
        sin_res = result;
        sin_res.maskMode = gpu3d::NYNN;
        builder.setResult(sin_res);
        builder.setPredication(predication.back());
        instructions.push_back(builder.buildInstruction());
    }
}    

//  Generates the instructions required to compute the predicate for the condition in an if instruction.
void IRTranslator::computeIFPredication(D3DSHADER_COMPARISON compareOp)
{
    //  Compute the predicate register based on an IF instruction condition.
    ShaderInstructionBuilder builder;
    
    //  Select a predicate register.
    Result predResult;
    GPURegisterId predReg = reservePredicate();
    
    //  Set the predicate register as result register.
    predResult.registerId = predReg;

    PredicationInfo currentPredication;
    
    //  Set the current instruction predication state.
    currentPredication.predicated = true;
    currentPredication.predicateRegister = predReg.num;

    //  Set the set predicate instruction operands and result registers.    
    builder.resetParameters();
    builder.setOperand(0, operands[0]);
    builder.setOperand(1, operands[1]);    
    builder.setResult(predResult);
    builder.setPredication(predication.back());
    
    //  Check the if comparison function.
    switch(compareOp)
    {
        case D3DSPC_GT:
            currentPredication.negatePredicate = false;
            builder.setOpcode(SETPGT);
            break;
            
        case D3DSPC_EQ:
            currentPredication.negatePredicate = false;
            builder.setOpcode(SETPEQ);
            break;
            
        case D3DSPC_LT:
            currentPredication.negatePredicate = false;
            builder.setOpcode(SETPLT);
            break;

        case D3DSPC_GE:        
            currentPredication.negatePredicate = true;
            builder.setOpcode(SETPLT);
            break;
        case D3DSPC_NE:
            currentPredication.negatePredicate = true;
            builder.setOpcode(SETPEQ);
            break;

        case D3DSPC_LE:
            currentPredication.negatePredicate = true;
            builder.setOpcode(SETPGT);
            break;
        
        case D3DSPC_RESERVED0:
        case D3DSPC_RESERVED1:
            panic("IRTranslator", "computeIFPredication", "Reserved comparison mode.");
            break;
        default:
            panic("IRTranslator", "computeIFPredication", "Undefined comparison mode.");
            break;
    }    

    //  Add the instruction setting the predicate.
    instructions.push_back(builder.buildInstruction());        

    //  Check for previous predications.
    if (predication.size() > 1)
    {
        //  Combine current predication with previous level predication.
        
        //  Create operand for the previous predication level.
        Operand op1;
        op1.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
        op1.negate = predication.back().negatePredicate;
        
        //  Create operand for the new predication level.
        Operand op2;
        op2.registerId = predReg;

        builder.resetParameters();        
        builder.setOpcode(gpu3d::ANDP);
        builder.setOperand(0, op1);
        builder.setOperand(1, op2);
        builder.setResult(predResult);
        builder.setPredication(predication[0]);
        
        //  Add the instruction to combine the two predicates.
        instructions.push_back(builder.buildInstruction());        
    }
    
    //  Add a new predicate to the predicate stack.
    predication.push_back(currentPredication);
}

//  Generates the instructions required to compute the predicate for the condition in a BREAKC instruction.
void IRTranslator::computeBREAKCPredication(D3DSHADER_COMPARISON compareOp, u32bit predBREAKC)
{
    //  Compute the predicate register based on an IF instruction condition.
    ShaderInstructionBuilder builder;
    
    //  Set the predicate register as result register.
    Result predResult;
    predResult.registerId = GPURegisterId(predBREAKC, gpu3d::PRED);

    //  Set the set predicate instruction operands and result registers.    
    builder.resetParameters();
    builder.setOperand(0, operands[0]);
    builder.setOperand(1, operands[1]);    
    builder.setPredication(predication.back());
    
    //  Check the BREAKC comparison mode.
    switch(compareOp)
    {
        case D3DSPC_GT:
        
            builder.setOpcode(SETPGT);
            
            //  Aliased to invert (NOT) result.  Do not invert result.
            predResult.saturate = false; 

            break;
            
        case D3DSPC_EQ:

            builder.setOpcode(SETPEQ);

            //  Aliased to invert (NOT) result.  Do not invert result.
            predResult.saturate = false; 

            break;
            
        case D3DSPC_LT:

            builder.setOpcode(SETPLT);

            //  Aliased to invert (NOT) result.  Do not invert result.
            predResult.saturate = false; 

            break;

        case D3DSPC_GE:        

            builder.setOpcode(SETPLT);

            //  Aliased to invert (NOT) result.  Invert result.
            predResult.saturate = true; 
            
            break;
            
        case D3DSPC_NE:

            builder.setOpcode(SETPEQ);

            //  Aliased to invert (NOT) result.  Invert result.
            predResult.saturate = true; 
            
            break;

        case D3DSPC_LE:

            builder.setOpcode(SETPGT);

            //  Aliased to invert (NOT) result.  Invert result.
            predResult.saturate = true; 
            
            break;
        
        case D3DSPC_RESERVED0:
        case D3DSPC_RESERVED1:
            panic("IRTranslator", "computePredicate", "Reserved comparison mode.");
            break;
        default:
            panic("IRTranslator", "computePredicate", "Undefined comparison mode.");
            break;
    }    

    builder.setResult(predResult);

    //  Add the instruction setting the predicate.
    instructions.push_back(builder.buildInstruction());        
}


//  Generates an ATTILA NOP instruction
void IRTranslator::generateNOP()
{
    ShaderInstructionBuilder builder;
    builder.resetParameters();
    builder.setOpcode(NOP);
    instructions.push_back(builder.buildInstruction());
}

//  Selects a written component from an instruction result register.
void IRTranslator::selectComponentFromResult(Result result, Operand &opTemp)
{
    //  Use one of the components of the result register
    //  to hold the temporal values.
    opTemp.registerId = result.registerId;
    
    //  Clear modifier and flags (copied from src0).
    opTemp.absolute = false;
    opTemp.addressing = false;    
    opTemp.negate = false;
    
    //  Select one of the components written by the
    //  result to hold the temporal value.
    switch(result.maskMode)
    {
        case gpu3d::NNNW:
            opTemp.swizzle = gpu3d::WWWW;
            break;
        case gpu3d::NNZN:
        case gpu3d::NNZW:
            opTemp.swizzle = gpu3d::ZZZZ;
            break;
        case gpu3d::NYNN:
        case gpu3d::NYNW:
        case gpu3d::NYZN:
        case gpu3d::NYZW:
            opTemp.swizzle = gpu3d::YYYY;
            break;        
        case gpu3d::XNNN:
        case gpu3d::XNNW:
        case gpu3d::XNZN:
        case gpu3d::XNZW:
        case gpu3d::XYNN:
        case gpu3d::XYNW:
        case gpu3d::XYZN:
        case gpu3d::mXYZW:
            opTemp.swizzle = gpu3d::XXXX;
            break;
        default:
            panic("IRTranslator", "selectComponentFromResult", "Undefined write mask mode.");
            break;
    }
}

//  Selects the specified swizzle component from an instruction operand setting a broadcast from that component in output operand.
void IRTranslator::selectSwizzledComponent(u32bit component, Operand in, Operand &out)
{
    //  Get the selected swizzled component.
    u32bit swizzledComponent = ((in.swizzle >> ((3 - component) * 2)) & 0x03);
    
    //  Set the swizzled component in the output operand.
    switch(swizzledComponent)
    {
        case 0:
            out.swizzle = gpu3d::XXXX;
            break;
        case 1:
            out.swizzle = gpu3d::YYYY;
            break;
        case 2:
            out.swizzle = gpu3d::ZZZZ;
            break;
        case 3:
            out.swizzle = gpu3d::WWWW;
            break;
        default:
            panic("IRTranslator", "selectSwizzledComponent", "Undefined component identifier.");
            break;
    }    
}

//  Generates code for an IFC instruction.
void IRTranslator::generateCodeForIFC(D3DSHADER_COMPARISON comparisonMode)
{
    //  Initialize a new IF block info object.
    IFBlockInfo currentIFBlock;
    currentIFBlock.insideIFBlock = true;
    currentIFBlock.insideELSEBlock = false;
    
    //  Create predication for the new IF block.
    computeIFPredication(comparisonMode);

    //  Add jump instruction to skip the IF block.
    ShaderInstructionBuilder builder;
    
    //  Create operand for current predication.
    Operand op;
    op.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
    op.negate = !predication.back().negatePredicate;

    builder.resetParameters();
    builder.setOpcode(gpu3d::JMP);
    builder.setOperand(0, op);
    
    //  Add jump instruction to the program.
    instructions.push_back(builder.buildInstruction());                                        
                    
    //  Get the IF block predication information.
    currentIFBlock.predicateRegister = predication.back().predicateRegister;
    currentIFBlock.predicateNegated = predication.back().negatePredicate; 
    
    //  Get the position of the jump instruction for the IF condition.
    currentIFBlock.jumpForIF = instructions.size() - 1;
    
    //  Add the IF block info object.
    currentIFBlocks.push_back(currentIFBlock);
}

//  Generates code for an IFB instruction.
void IRTranslator::generateCodeForIFB()
{
    ShaderInstructionBuilder builder;
    
    //  Initialize a new IF block info object.
    IFBlockInfo currentIFBlock;
    currentIFBlock.insideIFBlock = true;
    currentIFBlock.insideELSEBlock = false;
    
    //  Create predication for the new IF block.
    //  Select a predicate register.
    Result predResult;
    GPURegisterId predReg = reservePredicate();
    
    //  Set the predicate register as result register.
    predResult.registerId = predReg;

    PredicationInfo currentPredication;
    
    //  Set the current instruction predication state.
    currentPredication.predicated = true;
    currentPredication.predicateRegister = predReg.num;
    currentPredication.negatePredicate = false;

    //  Set second operand to true.
    Operand op2;
    op2.registerId = GPURegisterId(0, gpu3d::PRED);
    op2.absolute = true;
    op2.negate = true;
    
    //  Read x component from boolean constants.
    operands[0].swizzle = gpu3d::XXXX;
    
    builder.resetParameters();
    builder.setOpcode(gpu3d::ANDP);
    builder.setOperand(0, operands[0]);
    builder.setOperand(1, op2);
    builder.setResult(predResult);
    builder.setPredication(predication[0]);
    
    //  Add instruction to load predicate from boolean constant.
    instructions.push_back(builder.buildInstruction());
    
    //  Check for previous predications.
    if (predication.size() > 1)
    {
        //  Combine current predication with previous level predication.
        
        //  Create operand for the previous predication level.
        Operand op1;
        op1.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
        op1.negate = predication.back().negatePredicate;
        
        //  Create operand for the new predication level.
        Operand op2;
        op2.registerId = predReg;

        builder.resetParameters();        
        builder.setOpcode(gpu3d::ANDP);
        builder.setOperand(0, op1);
        builder.setOperand(1, op2);
        builder.setResult(predResult);
        builder.setPredication(predication[0]);
        
        //  Add the instruction to combine the two predicates.
        instructions.push_back(builder.buildInstruction());        
    }
    
    //  Add a new predicate to the predicate stack.
    predication.push_back(currentPredication);

    //  Add jump instruction to skip the IF block.

    //  Create operand for current predication.
    Operand op;
    op.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
    op.negate = !predication.back().negatePredicate;

    builder.resetParameters();
    builder.setOpcode(gpu3d::JMP);
    builder.setOperand(0, op);
    
    //  Add jump instruction to the program.
    instructions.push_back(builder.buildInstruction());                                        
                    
    //  Get the IF block predication information.
    currentIFBlock.predicateRegister = predication.back().predicateRegister;
    currentIFBlock.predicateNegated = predication.back().negatePredicate; 
    
    //  Get the position of the jump instruction for the IF condition.
    currentIFBlock.jumpForIF = instructions.size() - 1;
    
    //  Add the IF block info object.
    currentIFBlocks.push_back(currentIFBlock);
}

//  Generate code for an ELSE instruction.
void IRTranslator::generateCodeForELSE()
{
    //  Check if inside an IF block.
    if (currentIFBlocks.size() > 0)
    {
        //  Check if already inside the current ELSE block.
        if (!currentIFBlocks.back().insideELSEBlock)
        {
            //  Change to the ELSE side of the condition.
            currentIFBlocks.back().insideELSEBlock = true;
            currentIFBlocks.back().insideIFBlock = false;

            //  Compute the offset between the start of the IF block to the start of the ELSE block.
            s32bit jumpOffset = instructions.size() - currentIFBlocks.back().jumpForIF;
            u32bit jumpAddress = instructions.size();
            
            //  Check if there is a predication level below the current IF predication.
            if (predication.size() > 2)
            {
                //  Combine the previous predicate to get the predicate for the ELSE side.
                ShaderInstructionBuilder builder;
                
                //  Create operand for the previous predicate level.
                Operand op1;
                op1.registerId = GPURegisterId(predication[predication.size() - 2].predicateRegister, gpu3d::PRED);
                op1.negate = predication[predication.size() - 2].negatePredicate;

                //  Create operand for the current predicate level, ELSE side.
                Operand op2;
                op2.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
                op2.negate = !predication.back().negatePredicate;

                //  Create result for the current predication level.
                Result res;
                res.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
                
                //  Create the instruction to combine the previous predication with the else side
                //  predication.
                builder.resetParameters();
                builder.setOpcode(gpu3d::ANDP);
                builder.setOperand(0, op1);
                builder.setOperand(1, op2);
                builder.setResult(res);

                //  Add the instruction to the instruction stream.
                instructions.push_back(builder.buildInstruction());
            
                //  Change current predication parameters.
                predication.back().negatePredicate = false;
                currentIFBlocks.back().predicateNegated = false;
            }
            else
            {
                //  No previous predication, so just invert the current predicate.
                
                //  Change current predication parameters.
                predication.back().negatePredicate = !predication.back().negatePredicate;
                currentIFBlocks.back().predicateNegated = !currentIFBlocks.back().predicateNegated;
                
                //  No extra computing required for ELSE predicate so skip over the ELSE jump
                //  when jumping from the IF side.
                jumpOffset = jumpOffset + 1;
            }
            
            //  Add jump instruction to skip the ELSE block.
            ShaderInstructionBuilder builder;
            
            //  Create operand for current predication.
            Operand op;
            op.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
            op.negate = !predication.back().negatePredicate;

            builder.resetParameters();
            builder.setOpcode(gpu3d::JMP);
            builder.setOperand(0, op);
            
            //  Add jump instruction to the program.
            instructions.push_back(builder.buildInstruction());      
                      
            //  Check if it's worth keeping the jump.
            if (jumpOffset < IF_ELSE_MIN_INSTR_PER_JUMP)
            {
                //  Delete the jump as it may be more expensive than actually jumping.
                instructions.erase(instructions.begin() + currentIFBlocks.back().jumpForIF);
                
                //  Patch pointers to instruction in the current REP block.
                if (currentREPBlocks.size() > 0)
                {
                    if (currentIFBlocks.back().jumpForIF < currentREPBlocks.back().jumpForBREAK)
                        currentREPBlocks.back().jumpForBREAK--;
                }                            
            }
            else
            {
                //  Patch the jump for the IF condition.
                instructions[currentIFBlocks.back().jumpForIF]->setJumpOffset(jumpOffset);
                
                //  Update last jump target address.
                lastJumpTarget = jumpAddress;
            }
            
            //  Get the position of the jump instruction for the ELSE condition.
            currentIFBlocks.back().jumpForELSE = instructions.size() - 1;
        }
        else
        {
            //  Unexpected ELSE.
            error = true;
        }
    }
    else
    {
        //  Unexpected ELSE.
        error = true;
    }
}

//  Generate code for ENDIF instruction.
void IRTranslator::generateCodeForENDIF()
{
    //  Check if inside an IF block
    if (currentIFBlocks.size() > 0)
    {
        //  Check if a ELSE block was found.
        if (currentIFBlocks.back().insideELSEBlock)
        {
            //  Compute the offset between the start of the ELSE block to the end of the ELSE block.
            s32bit jumpOffset = instructions.size() - currentIFBlocks.back().jumpForELSE;
            u32bit jumpAddress = instructions.size();
            
            //  Check if it's worth keeping the jump.
            if (jumpOffset < IF_ELSE_MIN_INSTR_PER_JUMP)
            {
                //  Delete the the andp and jump instructions as it may be more expensive than actually jumping.
                instructions.erase(instructions.begin() + currentIFBlocks.back().jumpForELSE);

                //  Patch pointers to instruction in the current REP block.
                if (currentREPBlocks.size() > 0)
                {
                    if (currentIFBlocks.back().jumpForELSE < currentREPBlocks.back().jumpForBREAK)
                        currentREPBlocks.back().jumpForBREAK--;
                }
                
                //  Patch jump for IF.
                s32bit jumpOffsetIF = instructions[currentIFBlocks.back().jumpForIF]->getJumpOffset();
                jumpOffsetIF = jumpOffsetIF - 1;
                instructions[currentIFBlocks.back().jumpForIF]->setJumpOffset(jumpOffsetIF);
            }
            else
            {
                //  Patch the jump for the ELSE condition.
                instructions[currentIFBlocks.back().jumpForELSE]->setJumpOffset(jumpOffset);
                
                //  Update last jump target address.
                lastJumpTarget = jumpAddress;
            }
        }
        else
        {
            //  Compute the offset between the start of the IF block to the start of the ELSE block.
            s32bit jumpOffset = instructions.size() - currentIFBlocks.back().jumpForIF;
            u32bit jumpAddress = instructions.size();

            //  Check if it's worth keeping the jump.
            if (jumpOffset < IF_ELSE_MIN_INSTR_PER_JUMP)
            {
                //  Delete the jump as it may be more expensive than actually jumping.
                instructions.erase(instructions.begin() + currentIFBlocks.back().jumpForIF);

                //  Patch pointers to instruction in the current REP block.
                if (currentREPBlocks.size() > 0)
                {
                    if (currentIFBlocks.back().jumpForIF < currentREPBlocks.back().jumpForBREAK)
                        currentREPBlocks.back().jumpForBREAK--;
                }                            
            }
            else
            {
                //  Patch the jump for the IF condition.
                instructions[currentIFBlocks.back().jumpForIF]->setJumpOffset(jumpOffset);

                //  Update last jump target address.
                lastJumpTarget = jumpAddress;
            }
        }
    
        //  Release the current predication register.
        GPURegisterId predReg(predication.back().predicateRegister, gpu3d::PRED);
        releasePredicate(predReg);

        //  Remove one level of predication.
        predication.pop_back();
        
        //  Remove the current IF block.
        currentIFBlocks.pop_back();
    }
    else
    {
        //  Unexpected ENDIF.
        error = true;
    }
}

//  Generate code for REP instruction.
void IRTranslator::generateCodeForREP()
{
    //  Set inside REP block flag.
    insideREPBlock = true;

    //  Get the integer constant register storing the number of iterations of the REP block.
    u32bit iteratorReg = operands[0].registerId.num;
                        
    REPBlockInfo repBlock;
                        
    //  Set start address for the REP block.
    repBlock.start = instructions.size();

    //  Get if the constant integer value is defined.
    list<ConstRegisterDeclaration>::iterator it = constDeclaration.begin();                    
    while ((it != constDeclaration.end()) && (it->native_register != iteratorReg)) it++;
    if (it != constDeclaration.end() && it->defined)
    {                                        
        //  Set the number of iterations of the REP block.
        repBlock.iterations = it->value.value.intValue.x;
    }
    else
    {
        //  The number of iterations is not defined in the shader declaration.
        repBlock.iterations = 0;
    }
    
    //  Set if a break instruction was found inside the REP block.
    repBlock.foundBREAK = false;
    
    //  Add header code.
    ShaderInstructionBuilder builder;
    Operand op1, op2;
    Result res;
    
    //  Initialize iteration counter:
    //
    //      addi counter.x, constIter.x, 0
    //
    
    //  Allocate a new temporal register for the iteration counter.
    GPURegisterId counter = reserveTemp();
    
    res.registerId = counter;
    res.maskMode = gpu3d::XNNN;
    
    op1.registerId = operands[0].registerId;
    op1.swizzle = gpu3d::XXXX;
    
    op2.registerId = GPURegisterId(0, gpu3d::IMM);
    
    builder.resetParameters();
    builder.setOpcode(gpu3d::ADDI);
    builder.setOperand(0, operands[0]);
    builder.setOperand(1, op2);
    builder.setResult(res);
    builder.setPredication(predication.back());
    
    //  Add instruction to the program.
    instructions.push_back(builder.buildInstruction());
    repBlock.headerInstr++;
    
    
    //  Initialize the predicate for the REP block.
    //
    //      stpgti predREP, counter.x, 0
    //
    
    //  Allocate a new predicate register for the REP block.
    GPURegisterId predREP = reservePredicate();

    res.registerId = predREP;
   
    op1.registerId = counter;
    op1.swizzle = gpu3d::XXXX;
    
    builder.resetParameters();
    builder.setOpcode(gpu3d::STPGTI);
    builder.setOperand(0, op1);
    builder.setOperand(1, op2);
    builder.setResult(res);
    builder.setPredication(predication.back());
    
    //  Add instruction to the program.
    instructions.push_back(builder.buildInstruction());
    repBlock.headerInstr++;

    //  Create predication for the REP block.
    PredicationInfo currentPredication;
    currentPredication.negatePredicate = false;
    currentPredication.predicated = true;
    currentPredication.predicateRegister = predREP.num;
    
    //  Check for previous predications.
    if (predication.size() > 1)
    {
        //  Combine current predication with previous level predication.
        
        //  Create operand for the previous predication level.
        op1.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
        op1.negate = predication.back().negatePredicate;
        op1.swizzle = gpu3d::XXXX;
        
        //  Create operand for the new predication level.
        op2.registerId = predREP;
        op2.swizzle = gpu3d::XXXX;

        //  Write combined predication in the current predication register.
        res.registerId = predREP;
        
        builder.resetParameters();        
        builder.setOpcode(gpu3d::ANDP);
        builder.setOperand(0, op1);
        builder.setOperand(1, op2);
        builder.setResult(res);
        builder.setPredication(predication[0]);
        
        //  Add the instruction to combine the two predicates.
        instructions.push_back(builder.buildInstruction());        
        repBlock.headerInstr++;
    }
    
    //  Add a new predicate to the predicate stack.
    predication.push_back(currentPredication);
                        
    //  Set iteration counter register for the REP block.
    repBlock.tempCounter = counter.num;
    
    //  Set predicate register for the REP block.
    repBlock.predicateREP = predREP.num;
    
    //  Add the current REP block.
    currentREPBlocks.push_back(repBlock);
}

//  Generate code for ENDREP instruction.
void IRTranslator::generateCodeForENDREP()
{
    //  Check if inside a REP code block.
    if (insideREPBlock)
    {
        REPBlockInfo repBlock = currentREPBlocks.back();
        
        //  Check if a break instruction was found inside the current REP code block.
        if (!repBlock.foundBREAK)
        {                   
            //  Add footer code.
            
            //  Update iteration counter.
            //
            //      addi counter.x, counter.x, -1
            //
            
            GPURegisterId counter = GPURegisterId(repBlock.tempCounter, gpu3d::TEMP);
            
            ShaderInstructionBuilder builder;
            
            Operand op1, op2;
            Result res;
            
            op1.registerId = counter;
            op1.swizzle = gpu3d::XXXX;
            
            op2.registerId = GPURegisterId(u32bit(-1), gpu3d::IMM);
            
            res.registerId = counter;
            res.maskMode = gpu3d::XNNN;
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::ADDI);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setResult(res);
            builder.setPredication(predication.back());

            //  Add instruction to the program.
            instructions.push_back(builder.buildInstruction());                        
            repBlock.footerInstr++;
            
            //
            //  Recompute predicate for the REP block with the new counter:
            //
            //      stpgti predREP, counter.x, 0
            //
            
            GPURegisterId predREP = GPURegisterId(repBlock.predicateREP, gpu3d::PRED);
            
            op2.registerId = GPURegisterId(0, gpu3d::IMM);
            
            res.registerId = predREP;
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::STPGTI);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setResult(res);
            builder.setPredication(predication.back());
            repBlock.footerInstr++;

            //  Add instruction to the program.
            instructions.push_back(builder.buildInstruction());                        
            
            //  Check for previous predications.
            if (predication.size() > 2)
            {
                //  Combine current predication with previous level predication.
                
                //  Create operand for the previous predication level.
                op1.registerId = GPURegisterId(predication[predication.size() - 2].predicateRegister, gpu3d::PRED);
                op1.negate = predication[predication.size() - 2].negatePredicate;
                op1.swizzle = gpu3d::XXXX;
                
                //  Create operand for the new predication level.
                op2.registerId = predREP;
                op2.swizzle = gpu3d::XXXX;

                //  Write combined predication in the current predication register.
                res.registerId = predREP;
                
                builder.resetParameters();        
                builder.setOpcode(gpu3d::ANDP);
                builder.setOperand(0, op1);
                builder.setOperand(1, op2);
                builder.setResult(res);
                builder.setPredication(predication[0]);
                repBlock.footerInstr++;
                
                //  Add the instruction to combine the two predicates.
                instructions.push_back(builder.buildInstruction());        
            }
            
            //  Add jump to the start of the REP block.
            //
            //      jmp pREP, start
            //
            
            op1.registerId = predREP;
            op1.negate = false;
            op1.absolute = false;
            
            //  Compute jump offset.
            s32bit jumpOffset = -(instructions.size() - repBlock.start - repBlock.headerInstr);
            
            op2.registerId = GPURegisterId(u32bit(jumpOffset), gpu3d::IMM);
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::JMP);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setPredication(predication[0]);
           
            //  Add instruction.
            instructions.push_back(builder.buildInstruction());
            repBlock.footerInstr++;
            
            //  Remove predication for the REP block.
            predication.pop_back();
            
            //  Release counter register.
            releaseTemp(counter);
        
            //  Release predicate register.
            releasePredicate(predREP);
            
            //
            //  NEED TO ENABLE UNROLLING IF THE ITERATIONS ARE KNOWN, THE INSTRUCTIONS PER INTERATION AND ITERATIONS 
            //  ARE LOW AND THERE IS NO BREAK
            //
            /*
            //  Check if the number of iterations was declared in the shader.
            if (repBlock.iterations > 0)
            {
                //  Get the current instruction offset (end of the REP block).
                u32bit endREPBlock = instructions.size();
                
                ShaderInstructionBuilder builder;
                
                //  Replicate the instructions in the REP block for the number of defined iterations.
                //  The first iteration has already been translated so skip it.
                for(u32bit iteration = 1; iteration < repBlock.iterations; iteration++)
                {
                    //  Replicate the instructions in the REP block.
                    for(u32bit instr = repBlock.start; instr < endREPBlock; instr++)
                    {
                        //  Replicate one of the instructions in the REP block.
                        instructions.push_back(builder.copyInstruction(instructions[instr]));
                    }
                }
            }
            else
            {
                //  Translation when the number of iterations is not defined in the shader not yet implemented.
                untranslatedControlFlow = true;
                error = true;
            }*/
        }
        else
        {
            //  Add footer code.
            
            //  Update iteration counter.
            //
            //      addi counter.x, counter.x, -1
            //
            
            GPURegisterId counter = GPURegisterId(repBlock.tempCounter, gpu3d::TEMP);
            
            ShaderInstructionBuilder builder;
            
            Operand op1, op2;
            Result res;
            
            op1.registerId = counter;
            op1.swizzle = gpu3d::XXXX;
            
            op2.registerId = GPURegisterId(u32bit(-1), gpu3d::IMM);
            
            res.registerId = counter;
            res.maskMode = gpu3d::XNNN;
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::ADDI);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setResult(res);
            builder.setPredication(predication.back());

            //  Add instruction to the program.
            instructions.push_back(builder.buildInstruction());                        
            repBlock.footerInstr++;
            
            //
            //  Recompute predicate for the REP block with the new counter:
            //
            //      stpgti predTemp, counter.x, 0
            //
            
            GPURegisterId predTemp = reservePredicate();
            
            op2.registerId = GPURegisterId(0, gpu3d::IMM);
            
            res.registerId = predTemp;
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::STPGTI);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setResult(res);
            builder.setPredication(predication.back());
            repBlock.footerInstr++;

            //  Add instruction to the program.
            instructions.push_back(builder.buildInstruction());                        

            //
            //  Update with predication from the previous iteration as it may have changed
            //  due to break instructions.
            //
            //      andp predREP, predTemp, predREP
            //
            
            GPURegisterId predREP = GPURegisterId(repBlock.predicateREP, gpu3d::PRED);
            
            op1.registerId = predTemp;
            
            op2.registerId = predREP;
            
            res.registerId = predREP;
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::ANDP);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setResult(res);
            builder.setPredication(predication.back());
            repBlock.footerInstr++;

            //  Add instruction to the program.
            instructions.push_back(builder.buildInstruction());                        

            //  Check for previous predications.
            if (predication.size() > 2)
            {
                //  Combine current predication with previous level predication.
                
                //  Create operand for the previous predication level.
                op1.registerId = GPURegisterId(predication[predication.size() - 2].predicateRegister, gpu3d::PRED);
                op1.negate = predication[predication.size() - 2].negatePredicate;
                op1.swizzle = gpu3d::XXXX;
                
                //  Create operand for the new predication level.
                op2.registerId = predREP;
                op2.swizzle = gpu3d::XXXX;

                //  Write combined predication in the current predication register.
                res.registerId = predREP;
                
                builder.resetParameters();        
                builder.setOpcode(gpu3d::ANDP);
                builder.setOperand(0, op1);
                builder.setOperand(1, op2);
                builder.setResult(res);
                builder.setPredication(predication[0]);
                repBlock.footerInstr++;
                
                //  Add the instruction to combine the two predicates.
                instructions.push_back(builder.buildInstruction());        
            }
            
            //  Add jump to the start of the REP block.
            //
            //      jmp pREP, start
            //
            
            op1.registerId = predREP;
            op1.negate = false;
            op1.absolute = false;
            
            //  Compute jump offset.
            s32bit jumpOffset = -(instructions.size() - repBlock.start - repBlock.headerInstr);
            
            op2.registerId = GPURegisterId(u32bit(jumpOffset), gpu3d::IMM);
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::JMP);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setPredication(predication[0]);
           
            //  Add instruction.
            instructions.push_back(builder.buildInstruction());
            repBlock.footerInstr++;
            
            //  Remove predication for the REP block.
            predication.pop_back();
            
            //  Release counter register.
            releaseTemp(counter);
        
            //  Relese temporal predicate register.
            releasePredicate(predTemp);
            
            //  Release predicate register.
            releasePredicate(predREP);
            
            //  Update the offset in the jump instruction for BREAK if required.
            if (repBlock.jumpBREAK)
            {
                //  Patch jump offset.
                s32bit jumpOffset = instructions.size() - repBlock.jumpForBREAK;
                instructions[repBlock.jumpForBREAK]->setJumpOffset(jumpOffset);
                lastJumpTarget = instructions.size();;
            }
        }
        
        //  Remove the information about the current REP block.
        currentREPBlocks.pop_back();
        
        //  Set if still inside a REP block.
        insideREPBlock = (currentREPBlocks.size() > 0);
    }
    else
    {
        //  Unexpected ENDREP instruction.
        untranslatedControlFlow = true;
        error = true;
    }
}

//  Generate code for BREAK instruction.
void IRTranslator::generateCodeForBREAK()
{
    //  Check if inside a REP/ENDREP block.
    if (insideREPBlock)
    {
        //  Check if the current predication is the same than the REP block predication.
        if (currentREPBlocks.back().predicateREP == predication.back().predicateRegister)
        {
            //  Just jump outside the REP block.
            //
            //      jmp true, offset
            //
            ShaderInstructionBuilder builder;
            
            Operand op1, op2;
            
            //  First operand is the implict value 'true'.
            op1.negate = true;
            op1.absolute = true;
            
            //  Second operand is the offset but set to 0 to be patched at the end of the REP block.
            op2.registerId = GPURegisterId(0, gpu3d::IMM);
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::JMP);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setPredication(predication[0]);
            
            //  Add instruction.
            instructions.push_back(builder.buildInstruction());
            
            //  Set pointer to the jump instruction for the BREAK.
            currentREPBlocks.back().jumpForBREAK = instructions.size() - 1;
            
            //  Set the flag that tells that a jump was generated for the BREAK.
            currentREPBlocks.back().jumpBREAK = true;
        }
        else
        {
            //  Needs to update the predication down to the REP block level.
            u32bit nextLevel = predication.size() - 2;
            u32bit levels = 0;
            bool foundREPPredication = false;
            
            while(!foundREPPredication)
            {
                //  Update the predication levels down to REP block predication removing
                //  the pixels with the current predication enabled.
                //
                //      andp pi, pi, !pcurrent
                //
                
                ShaderInstructionBuilder builder;
                
                Operand op1;
                Operand op2;
                Result res;
                
                op1.registerId = GPURegisterId(predication[nextLevel].predicateRegister, gpu3d::PRED);
                
                op2.registerId = GPURegisterId(predication.back().predicateRegister, gpu3d::PRED);
                op2.negate = true;
                
                res.registerId = GPURegisterId(predication[nextLevel].predicateRegister, gpu3d::PRED);
                
                builder.resetParameters();
                builder.setOpcode(gpu3d::ANDP);
                builder.setOperand(0, op1);
                builder.setOperand(1, op2);
                builder.setResult(res);
                builder.setPredication(predication[0]);
                
                //  Add instruction to the program.
                instructions.push_back(builder.buildInstruction());
                
                //  Check if the REP predication level was reached.
                foundREPPredication = (predication[nextLevel].predicateRegister == currentREPBlocks.back().predicateREP);
                
                //  Update the number of levels visited until reaching the REP block predicate.
                levels++;
                
                //  Update the pointer to the next level.
                nextLevel--;
            }
                                    
            //  Check if only current predication is above REP predication.
            if (levels == 1)
            {
                //  We can add a jump to skip over to the end REP block.
                //
                //      jmp !pREPUpdated, offset
                //
                ShaderInstructionBuilder builder;
                
                Operand op1, op2;
                
                //  First operand is the implict value 'true'.
                op1.registerId = GPURegisterId(currentREPBlocks.back().predicateREP, gpu3d::PRED);
                op1.negate = true;
                
                //  Second operand is the offset but set to 0 to be patched at the end of the REP block.
                op2.registerId = GPURegisterId(0, gpu3d::IMM);
                
                builder.resetParameters();
                builder.setOpcode(gpu3d::JMP);
                builder.setOperand(0, op1);
                builder.setOperand(1, op2);
                builder.setPredication(predication[0]);
                
                //  Add instruction.
                instructions.push_back(builder.buildInstruction());
                
                //  Set pointer to the jump instruction for the BREAK.
                currentREPBlocks.back().jumpForBREAK = instructions.size() - 1;
                
                //  Set the flag that tells that a jump was generated for the BREAK.
                currentREPBlocks.back().jumpBREAK = true;
            }
        }
        
        
        //  Set the break inside REP block flag for the current REP code block.
        currentREPBlocks.back().foundBREAK = true;
    }
    else
    {
        //  BREAK found outside a REP/ENDREP block.
        untranslatedControlFlow = true;
        error = true;
    }
}

//  Generate code for BREAKC instruction.
void IRTranslator::generateCodeForBREAKC(D3DSHADER_COMPARISON comparisonMode)
{
    //  Check if inside a REP/ENDREP block.
    if (insideREPBlock)
    {
        //  Get a predicate register where to store the value of the BREAKC condition.
        GPURegisterId predBREAKC = reservePredicate();

        //  Compute predicate for break condition.
        computeBREAKCPredication(comparisonMode, predBREAKC.num);
        
        //  Needs to update the predication down to the REP block level.
        u32bit nextLevel = predication.size() - 1;
        u32bit levels = 0;
        bool foundREPPredication = false;
        
        while(!foundREPPredication)
        {
            //  Update the predication levels down to REP block predication removing
            //  removing the pixels with the break condition set to true.
            //
            //      andp pi, pi, !pBREAKC
            //
            
            ShaderInstructionBuilder builder;
            
            Operand op1;
            Operand op2;
            Result res;
            
            op1.registerId = GPURegisterId(predication[nextLevel].predicateRegister, gpu3d::PRED);
            
            op2.registerId = predBREAKC;
            op2.negate = true;
            
            res.registerId = GPURegisterId(predication[nextLevel].predicateRegister, gpu3d::PRED);
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::ANDP);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setResult(res);
            builder.setPredication(predication[0]);
            
            //  Add instruction to the program.
            instructions.push_back(builder.buildInstruction());
            
            //  Check if the REP predication level was reached.
            foundREPPredication = (predication[nextLevel].predicateRegister == currentREPBlocks.back().predicateREP);
            
            //  Update the number of levels visited until reaching the REP block predicate.
            levels++;
            
            //  Update the pointer to the next level.
            nextLevel--;
        }
                                
        //  Check if only current predication is above REP predication.
        if (levels == 1)
        {
            //  We can add a jump to skip over to the end REP block.
            //
            //      jmp !pREPUpdated, offset
            //
            ShaderInstructionBuilder builder;
            
            Operand op1, op2;
            
            //  First operand is the implict value 'true'.
            op1.registerId = GPURegisterId(currentREPBlocks.back().predicateREP, gpu3d::PRED);
            op1.negate = true;
            
            //  Second operand is the offset but set to 0 to be patched at the end of the REP block.
            op2.registerId = GPURegisterId(0, gpu3d::IMM);
            
            builder.resetParameters();
            builder.setOpcode(gpu3d::JMP);
            builder.setOperand(0, op1);
            builder.setOperand(1, op2);
            builder.setPredication(predication[0]);
            
            //  Add instruction.
            instructions.push_back(builder.buildInstruction());
            
            //  Set pointer to the jump instruction for the BREAK.
            currentREPBlocks.back().jumpForBREAK = instructions.size() - 1;
            
            //  Set the flag that tells that a jump was generated for the BREAK.
            currentREPBlocks.back().jumpBREAK = true;
        }
        
        //  Release the predication register used to compute the BREAKC condition.
        releasePredicate(predBREAKC);
        
        //  Set the break inside REP block flag for the current REP code block.
        currentREPBlocks.back().foundBREAK = true;
    }
    else
    {
        //  BREAKC found outside a REP/ENDREP block.
        untranslatedControlFlow = true;
        error = true;
    }
}

//  Visitor for instruciton nodes.
void IRTranslator::visit(InstructionIRNode *n)
{
    // Clear error
    error = false;

    // Clear the instruction operand and result information.
    operands.clear();   
    result = Result();    
    relative = RelativeMode();
    
    // Check if the instruction has result register.
    if(n->getDestination() != 0)
    {
        //  Process the instruction destination parameter token to obtain the result information.
        visitInstructionDestination(n->getDestination());
    }   

    //  Process the instruction source parameter tokens to obtain the operands information.
    for(u32bit o = 0; o < n->getSources().size(); o++)
        visitInstructionSource(n->getSources()[o]);

    //  Check for errors when gathering information about the instruction operands and result.
    if(error)
    {
        //  Generate a NOP when the instruction can not be translated.
        generateNOP();
        
        //  Update the counter of untranslated instructions.
        untranslatedInstructions++;
        
        return;
    }

    // Try to find a direct translation
    map<D3DSHADER_INSTRUCTION_OPCODE_TYPE, ShOpcode>::const_iterator it_opc;
    it_opc = opcodeMap.find(n->getOpcode());
    if(it_opc != opcodeMap.end())
    {
        ShaderInstructionBuilder builder;

        // texkill seems doesn't admit swizzling and the swizzle calculated from the bytecode seems incorrect.
        // this will be a temporal solution.
        if (n->getOpcode() == D3DSIO_TEXKILL)
        {
            operands[0].swizzle = XYZW;
            builder.setOperand(0, operands[0]);
        }
        else
        {
            //  Set the instruction operands in the ATTILA instruction builder.
            for(u32bit i = 0; i < operands.size(); i ++)
                builder.setOperand(i, operands[i]);
        }
        
        //  Set the relative addressing mode in the ATTILA instruction builder.
        builder.setRelativeMode(relative);
        
        //  Set the instruction result in the ATTILA instruction builder.
        builder.setResult(result);
        
        ShOpcode opcode;
        
        //  texldp and texldb are encoded using the specific control field.        
        if (n->getOpcode() == D3DSIO_TEX)
        {
            if (n->getSpecificControls() == D3DSI_TEXLD_PROJECT)
                opcode = gpu3d::TXP;
            else if (n->getSpecificControls() == D3DSI_TEXLD_BIAS)
                opcode = gpu3d::TXB;
            else
                opcode = gpu3d::TEX;
        }
        else
            opcode = (*it_opc).second;

        // NOTE: In D3D, address register can be written by a MOV instruction.
        //       ATTILA handles address register writes with ARL instruction.
        //       So when a D3D MOV instruction is found, it's translated to either
        //       ATTILA ARL or ATTILA MOV depending on if instruction destination
        //       register is the address register or not
        if((type == VERTEX_SHADER) && (result.registerId.bank == ADDR) && (opcode == MOV))
            opcode = ARL;

        //  Set instruction predication in the ATTILA instruction builder.
        builder.setPredication(predication.back());
        
        //  Set the opcode in the ATTILA instruction builder.
        builder.setOpcode(opcode);
        
        //  Build and add the ATTILA instruction to the translated instruction list.
        instructions.push_back(builder.buildInstruction());
    }
    else
    {
        // Special instructions
        switch(n->getOpcode())
        {
            //  Control flow instructions.
            case D3DSIO_IFC:
            
                //  Generate code for the start of a IFC/ELSE/ENDIF block.
                //  Update the corresponding structures: predication stack, IF/ELSE/ENDIF block stack.
                generateCodeForIFC(n->getComparison());
                
                break;

            case D3DSIO_IF:
            
                //  Generate code for the start of a IFB/ELSE/ENDIF block.
                //  Update the corresponding structures: predication stack, IF/ELSE/ENDIF block stack.
                generateCodeForIFB();
                
                break;
            
            case D3DSIO_ELSE:
            
                //  Generate code for the ELSE instruction.
                //  Patch the code at the start of the IF/ELSE/ENDIF block.
                //  Update predication and IF/ELSE/ENDIF block state.
                generateCodeForELSE();
                
                break;
                
            case D3DSIO_ENDIF:
            
                //  Generate code for the ENDIF instruction.
                //  Patch the code for IF and ELSE sides of the code block.
                //  Update predication and IF/ELSE/ENDIF block state.
                generateCodeForENDIF();
                
                break;
                
            case D3DSIO_REP:
            
                //  Generate code at the start of a REP/ENDREP block.
                //  Update predication and REP/ENDREP block state.
                generateCodeForREP();               
                
                break;
                
            case D3DSIO_ENDREP:
            
                //  Generate code at the end of a REP/ENDREP block.
                //  Update predication and REP/ENDREP block state.
                //  Patch BREAK code if required.
                generateCodeForENDREP();              
                
                break;
                
            case D3DSIO_BREAK:
            
                //  Generate code for the BREAK instruction.
                //  Update predication and REP/ENDREP block state.
                generateCodeForBREAK();
                                
                break;
            
            case D3DSIO_BREAKC:
            
                //  Generate code for the BREAKC instruction.
                //  Update predication and REP/ENDREP block state.
                generateCodeForBREAKC(n->getComparison());
                                
                break;
                
            case D3DSIO_BREAKP:
            
                //  Check if inside a REP block.
                if (insideREPBlock)
                {
                    //  Set the break inside REP block flag for the current REP code block.
                    currentREPBlocks.back().foundBREAK = true;
                }
                
                //  Translation of BREAK instructions not yet implemented.
                untranslatedControlFlow = true;
                error = true;
                
                break;
                
            //  Emulated instructions.            
            case D3DSIO_M4x4:
                emulateM4X4(n);
                break;
            case D3DSIO_SUB:
                emulateSUB();
                break;
            case D3DSIO_TEX:
                emulateTEXLD1314(n);
                break;
            case D3DSIO_LRP:
                emulateLRP();
                break;
            case D3DSIO_POW:
                emulatePOW();
                break;
            case D3DSIO_NRM:
                emulateNRM();
                break;
            case D3DSIO_CMP:
                emulateCMP();
                break;
            case D3DSIO_DP2ADD:
                emulateDP2ADD();
                break;
            case D3DSIO_ABS:
                emulateABS();
                break;
            case D3DSIO_SINCOS:
                emulateSINCOS();
                break;
            
            default:
            
                ///@note The intention is to avoid halting simulation, so a NOP is generated.
                char message[40];
                sprintf(message, "Cannot translate Opcode %d", n->getOpcode());
                D3D_DEBUG( cout << "IRTranslator: WARNING: " << message << endl; )
                error = true;

                //  Check for control flow instruction not yet implemented.
                if (n->isJmp())
                    untranslatedControlFlow = true;

                break;
        }
    }

    //  Check for an error in the instruction translation.
    if (error)
    {
        //  Generate a NOP when the instruction can not be translated.
        generateNOP();
        
        //  Update the counter of untranslated instructions.
        untranslatedInstructions++;
        
        return;
    }
}

//  Set the alpha test function for emulation in the pixel shader.
void IRTranslator::setAlphaTest(D3DCMPFUNC _alpha_func)
{
    alpha_func = _alpha_func;
}

//  Set the fog enable for emulation in the pixel shader.
void IRTranslator::setFogEnabled(bool _fog_enabled)
{
    fog_enabled = _fog_enabled;
}

//  Returns if control flow instruction that could not be translated was found while translating the shader program.
bool IRTranslator::getUntranslatedControlFlow()
{
    return untranslatedControlFlow;
}

u32bit IRTranslator::getUntranslatedInstructions()
{
    return untranslatedInstructions;
}

//  Implements fixed function emulation in the pixel shader (alpha test, fog ...)
void IRTranslator::generate_extra_code(GPURegisterId color_temp, GPURegisterId color_out)
{
    ShaderInstructionBuilder builder;
    Operand op0, op1;
    Result res;

    //  Check the selected alpha test function.
    switch(alpha_func)
    {
        case D3DCMP_NEVER:
            {
                //
                //  Kill all fragments.
                //
                //  c = {-1, -1, -1, -1}
                //
                //  KIL c
                //

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();
                
                //  KIL c
                builder.setOpcode(KIL);
                op0.registerId = GPURegisterId(alpha_test_declaration.alpha_const_minus_one, gpu3d::PARAM);
                builder.setOperand(0, op0);
                
                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());
            }
            break;

        case D3DCMP_LESS:
            {
                //
                //  SGE temp, color_temp.w, alpha_ref.w
                //  KIL -temp
                //
                
                //  Reserve a temporary register.
                GPURegisterId temp = reserveTemp();

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // SGE temp, color_temp.w, alpha_ref.w
                builder.setOpcode(SGE);
                op0.registerId = color_temp;
                op0.swizzle = gpu3d::WWWW;
                op1.registerId = GPURegisterId(alpha_test_declaration.alpha_const_ref, gpu3d::PARAM);
                op1.swizzle = gpu3d::WWWW;
                res.registerId = temp;
                builder.setOperand(0, op0);
                builder.setOperand(1, op1);
                builder.setResult(res);
                
                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());
                
                //  Initialize the ATTILA instruction builder.
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // KIL -temp
                builder.setOpcode(KIL);
                op0.registerId = temp;
                op0.negate = true;
                builder.setOperand(0, op0);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());

                //  Release temporary register.
                releaseTemp(temp);
            }
            break;
            
        case D3DCMP_LESSEQUAL:
            {
                //
                //  ADD temp, alpha_ref.w, -color_temp.v
                //  KIL temp
                //
                
                //  Reserve a temporary register.
                GPURegisterId temp = reserveTemp();
                
                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();
                
                // ADD temp, alpha_ref.w, -color_temp.w
                builder.setOpcode(ADD);
                op0.registerId = GPURegisterId(alpha_test_declaration.alpha_const_ref, gpu3d::PARAM);
                op0.swizzle = gpu3d::WWWW;
                op1.registerId = color_temp;
                op1.swizzle = gpu3d::WWWW;
                op1.negate = true;
                res.registerId = temp;
                builder.setOperand(0, op0);
                builder.setOperand(1, op1);
                builder.setResult(res);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());
                
                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();
                
                // KIL temp
                builder.setOpcode(KIL);
                op0.registerId = temp;
                builder.setOperand(0, op0);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());
                
                //  Release temporary register.
                releaseTemp(temp);
            }
            break;
            
        case D3DCMP_EQUAL:
            {
                //
                //  ADD temp, alpha_ref.w, -color_temp.w
                //  KIL -|temp|
                //
                
                //  Reserve a temporary register.
                GPURegisterId temp = reserveTemp();
                
                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // ADD temp, alpha_ref.w, -color_temp.w
                builder.setOpcode(ADD);
                op0.registerId = GPURegisterId(alpha_test_declaration.alpha_const_ref, gpu3d::PARAM);
                op0.swizzle = gpu3d::WWWW;
                op1.registerId = color_temp;
                op1.swizzle = gpu3d::WWWW;
                op1.negate = true;
                res.registerId = temp;
                builder.setOperand(0, op0);
                builder.setOperand(1, op1);
                builder.setResult(res);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());
                
                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // KIL -|temp|
                builder.setOpcode(KIL);
                op0.registerId = temp;
                op0.negate = true;
                op0.absolute = true;
                builder.setOperand(0, op0);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());

                //  Release temporary register.
                releaseTemp(temp);
            }
            break;
            
        case D3DCMP_GREATEREQUAL:
            {
                //
                //  ADD temp, color_temp.w, -alpha_ref.w
                //  KIL temp
                //
                
                //  Reserve a temporary register.
                GPURegisterId temp = reserveTemp();
               
                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // ADD temp, color_temp.w, -alpha_ref.w
                builder.setOpcode(ADD);
                op0.registerId = color_temp;
                op0.swizzle = gpu3d::WWWW;
                op1.registerId = GPURegisterId(alpha_test_declaration.alpha_const_ref, gpu3d::PARAM);
                op1.swizzle = gpu3d::WWWW;
                op1.negate = true;
                res.registerId = temp;
                builder.setOperand(0, op0);
                builder.setOperand(1, op1);
                builder.setResult(res);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // KIL temp
                builder.setOpcode(KIL);
                op0.registerId = temp;
                builder.setOperand(0, op0);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());
                
                //  Release temporary register.
                releaseTemp(temp);
            }
            break;
            
        case D3DCMP_GREATER:
        {
                //
                //  SGE temp, -color_temp.w, -alpha_ref.w
                //  KIL -temp
                //
                
                //  Reserve a temporary register.
                GPURegisterId temp = reserveTemp();

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // SGE temp, -color_temp.w, -alpha_ref.w
                builder.setOpcode(SGE);
                op0.registerId = color_temp;
                op0.swizzle = gpu3d::WWWW;
                op0.negate = true;
                op1.registerId = GPURegisterId(alpha_test_declaration.alpha_const_ref, gpu3d::PARAM);
                op1.swizzle = gpu3d::WWWW;
                op1.negate = true;
                res.registerId = temp;
                builder.setOperand(0, op0);
                builder.setOperand(1, op1);
                builder.setResult(res);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // KIL -temp
                builder.setOpcode(KIL);
                op0.registerId = temp;
                op0.negate = true;
                builder.setOperand(0, op0);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());

                //  Release temporary register.
                releaseTemp(temp);
            }
            break;
            
        case D3DCMP_NOTEQUAL:
        {
                //  SGE temp.x, color_temp.w, alpha_ref.w
                //  SGE temp.y, -color_temp.w, -alpha_ref.w
                //  MUL temp, temp.x, temp.y
                //  KIL -temp
                //
                
                //  Reserve a temporary register.
                GPURegisterId temp = reserveTemp();

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // SGE temp.x, color_temp.w, alpha_ref.w
                builder.setOpcode(SGE);
                op0.registerId = color_temp;
                op0.swizzle = gpu3d::WWWW;
                op1.registerId = GPURegisterId(alpha_test_declaration.alpha_const_ref, gpu3d::PARAM);
                op1.swizzle = gpu3d::WWWW;
                res.registerId = temp;
                res.maskMode = XNNN;
                builder.setOperand(0, op0);
                builder.setOperand(1, op1);
                builder.setResult(res);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // SGE temp.y, -color_temp.w, -alpha_ref.w
                builder.setOpcode(SGE);
                op0.registerId = color_temp;
                op0.swizzle = gpu3d::WWWW;
                op0.negate = true;
                op1.registerId = GPURegisterId(alpha_test_declaration.alpha_const_ref, gpu3d::PARAM);
                op1.swizzle = gpu3d::WWWW;
                op1.negate = true;
                res.registerId = temp;
                res.maskMode = NYNN;
                builder.setOperand(0, op0);
                builder.setOperand(1, op1);
                builder.setResult(res);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // MUL temp temp.x temp.y
                builder.setOpcode(MUL);
                op0.registerId = temp;
                op0.swizzle = XXXX;
                op1.registerId = temp;
                op1.swizzle = YYYY;
                res.registerId = temp;
                builder.setOperand(0, op0);
                builder.setOperand(1, op1);
                builder.setResult(res);

                //  Build instruction and add the the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());

                //  Initialize the ATTILA instruction builder.                
                builder.resetParameters();
                op0 = op1 = Operand();
                res = Result();

                // KIL -temp
                builder.setOpcode(KIL);
                op0.registerId = temp;
                op0.negate = true;
                builder.setOperand(0, op0);

                //  Build instruction and add to the list of translated ATTILA shader instructions.
                instructions.push_back(builder.buildInstruction());
                
                //  Release temporary register.
                releaseTemp(temp);
            }
            
            break;
            
        default:
            panic("IRTranslator", "generate_extra_code", "Undefined alpha test comparison function.");
            break;
    }
    
    //  Copy the result color to the output color register.
    generateMov(color_out, color_temp);
}

void IRTranslator::visit(CommentIRNode *n)
{
}

void IRTranslator::visit(CommentDataIRNode *n)
{
}

void IRTranslator::visit(ParameterIRNode *n)
{
}

void IRTranslator::visit(SourceParameterIRNode *n)
{
}

void IRTranslator::visit(DestinationParameterIRNode *n)
{
}

void IRTranslator::visit(BoolIRNode *n)
{
}

void IRTranslator::visit(FloatIRNode *n)
{
}

void IRTranslator::visit(IntegerIRNode *n)
{
}

void IRTranslator::visit(SamplerInfoIRNode *n)
{
}

//  Visitor for semantic node.
void IRTranslator::visit(SemanticIRNode *n)
{
}

void IRTranslator::visit(RelativeAddressingIRNode *n)
{
}

void IRTranslator::visit(IRNode *n)
{
}

void IRTranslator::end()
{
}


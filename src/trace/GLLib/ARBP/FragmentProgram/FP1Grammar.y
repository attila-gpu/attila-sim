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

%{
#include <string>
#include <sstream>

using namespace std;

int yylex();

#include "IRNode.h"

using namespace libgl::GenerationCode;
using namespace libgl;

#ifndef YYDEBUG
    #define YYDEBUG 0
#endif

#define APPEND_STR(str,str1)                       { (str).append(str1); }
#define APPEND_NUMBER_SELECTION(string,number)     { stringstream ss; ss << "[" << number << "]"; (string).append(ss.str()); }
#define APPEND_ROW_RANGE_SELECTION(string,min,max) { stringstream ss; ss << "[" << min << ".." << max << "]"; (string).append(ss.str()); }
#define STRING_TO_VAR(str, var)                    { stringstream ss(str, ios::in); ss >> var; }

#define CREATE_STRING(str_ptr,TEXT)                { (str_ptr) = new string(TEXT); fp1ASTStringCollector.push_back(str_ptr); }

#define CREATE_EMPTY_STRING(str_ptr)               CREATE_STRING(str_ptr, "")


void yyerror(void* irtree, char *s)
{
    panic("FP1Grammar.y","yyerror()","Sintactic error in Fragment Program");
}

typedef struct{
  string* texto;
  int linea;
} type_token_atrib;  

list<string*> fp1LexSymbolCollector;

list<string*> fp1ASTStringCollector;

%}
%union{         
  type_token_atrib                          token_atrib;
  string*                                   str;
  float                                     floatconst;
  int                                       intconst;
  bool                                      boolean;
  list<string>*                             strlist;
  list<IROption*>*                          optlist;
  list<IRStatement*>*                       stmntlist;
  list<IRParamBinding*>*                    pbindlist;
  IRNode*                                   irnode;
  IRProgram*                                irprogram;
  IROption*                                 iroption;
  IRStatement*                              irstatement;
  IRInstruction*                            irinstruction;
  IRSampleInstruction*                      irsamplinstr;
  IRSwizzleInstruction*                     irswzinstr;
  IRSwizzleComponents*                      irswzcomps;
  IRSwizzleComponents::swzComponent         irswzcomp;
  IRDstOperand*                             irdstoperand;
  IRSrcOperand*                             irsrcoperand;
  IRALIASStatement*                         iraliasstmnt;
  IRTEMPStatement*                          irtempstmnt;
  IRATTRIBStatement*                        irattrstmnt;
  IROUTPUTStatement*                        iroutputstmnt;
  IRPARAMStatement*                         irparamstmnt;
  IRParamBinding*                           irparambind;
  IRLocalEnvBinding*                        irlocalenvbind;
}

%token <token_atrib> TOK_ARBFP10_HEADER TOK_OPTION TOK_PARAM TOK_ATTRIB TOK_TEMP TOK_OUTPUT TOK_ALIAS TOK_ABS
 TOK_ADD TOK_CMP TOK_COS TOK_DP3 TOK_DP4 TOK_DPH TOK_DST TOK_EX2 TOK_FLR TOK_FRC TOK_KIL TOK_LG2 TOK_LIT TOK_LRP
 TOK_MAD TOK_MAX TOK_MIN TOK_MOV TOK_MUL TOK_POW TOK_RCP TOK_RSQ TOK_SIN TOK_SCS TOK_SGE TOK_SLT TOK_SUB TOK_SWZ
 TOK_TEX TOK_TXB TOK_TXP TOK_XPD TOK_ABS_SAT TOK_ADD_SAT TOK_CMP_SAT TOK_COS_SAT TOK_DP3_SAT TOK_DP4_SAT TOK_DPH_SAT
 TOK_DST_SAT TOK_EX2_SAT TOK_FLR_SAT TOK_FRC_SAT TOK_LG2_SAT TOK_LIT_SAT TOK_LRP_SAT TOK_MAD_SAT TOK_MAX_SAT
 TOK_MIN_SAT TOK_MOV_SAT TOK_MUL_SAT TOK_POW_SAT TOK_RCP_SAT TOK_RSQ_SAT TOK_SIN_SAT TOK_SCS_SAT TOK_SGE_SAT
 TOK_SLT_SAT TOK_SUB_SAT TOK_SWZ_SAT TOK_TEX_SAT TOK_TXB_SAT TOK_TXP_SAT TOK_XPD_SAT TOK_PROGRAM TOK_RESULT
 TOK_STATE TOK_FRAGMENT TOK_TEXTURE TOK_COLOR TOK_PRIMARY TOK_SECONDARY TOK_TEXCOORD TOK_FOGCOORD TOK_POSITION
 TOK_ATTRIB_M TOK_ENV TOK_LOCAL TOK_MATERIAL TOK_AMBIENT TOK_DIFFUSE TOK_SPECULAR TOK_EMISSION TOK_SHININESS TOK_FRONT TOK_BACK
 TOK_LIGHT TOK_SPOT TOK_DIRECTION TOK_HALF TOK_ATTENUATION TOK_LIGHTMODEL TOK_SCENECOLOR TOK_LIGHTPROD TOK_TEXENV
 TOK_FOG TOK_PARAMS TOK_DEPTH TOK_RANGE TOK_MATRIX TOK_MODELVIEW TOK_PROJECTION TOK_MVP TOK_PALETTE
 TOK_INVERSE TOK_TRANSPOSE TOK_INVTRANS TOK_ROW TOK_POINT_POINT TOK_INT_CONST TOK_FLOAT_CONST TOK_IDENT TOK_COMPONENT
 TOK_1D TOK_2D TOK_3D TOK_CUBE TOK_RECT TOK_END

%parse-param { void* irtree }

%type <irprogram> program ARBFP10_program
%type <iroption> option
%type <irstatement> statement namingStatement
%type <irinstruction> instruction ALUInstruction TexInstruction KIL_instruction
                      VECTORop_instruction VECTORop SCALARop_instruction SCALARop 
                      BINSCop_instruction BINSCop BINop_instruction BINop 
                      TRIop_instruction TRIop
%type <irsamplinstr> SAMPLE_instruction SAMPLEop
%type <irswzinstr> SWZ_instruction SWZop
%type <irswzcomps> extendedSwizzle
%type <irswzcomp> ExtSwizSel xyzw_rgbaComponent

%type <str> identifier texTarget component establishName establishedName optFaceType
            optColorType optionalMask xyzw_rgbaMask resultBinding
            fragAttribBinding fragAttribItem scalarSuffix optionalSuffix
            progParamArray stateLightProperty stateMatProperty stateSpotProperty
            stateLModProperty stateLProdProperty  stateTexEnvProperty stateFogProperty
            stateDepthProperty stateMaterialItem stateLightItem stateLightProdItem
            stateLightModelItem stateTexEnvItem stateFogItem
            stateDepthItem stateMatrixItem stateMatrixRow stateOptMatModifier
            stateMatModifier stateMatrixName stateMatrixRows optMatrixRows
            
%type <strlist> varNameList
%type <optlist> optionSequence
%type <stmntlist> statementSequence
%type <pbindlist> paramMultInitList paramSingleInit paramMultipleInit
            
%type <floatconst> floatConstant signedFloatConstant
%type <intconst> integer fragAttribNum optTexImageUnitNum texImageUnit texImageUnitNum progParamArrayAbs
                 optTexCoordNum texCoordNum optArraySize progEnvParamNum progLocalParamNum
                 stateLightNumber legacyTexUnitNum optLegacyTexUnitNum stateMatrixRowNum
                 stateModMatNum statePaletteMatNum stateProgramMatNum stateOptModMatNum
%type <boolean> optionalSign

%type <irdstoperand> maskedDstReg dstReg
%type <irsrcoperand> scalarSrcReg vectorSrcReg srcReg progParamReg

%type <iraliasstmnt> ALIAS_statement
%type <irtempstmnt> TEMP_statement
%type <irattrstmnt> ATTRIB_statement
%type <iroutputstmnt> OUTPUT_statement
%type <irparamstmnt> PARAM_statement PARAM_singleStmt PARAM_multipleStmt

%type <irparambind> paramSingleItemUse stateSingleItem programSingleItem paramConstUse
                    paramConstScalarUse paramConstVector paramSingleItemDecl paramConstDecl
                    paramConstScalarDecl paramMultipleItem stateMultipleItem programMultipleItem 
                    progEnvParams progLocalParams
                    
%type <irlocalenvbind>  progEnvParamNums progEnvParam progLocalParamNums progLocalParam

%start program
%%

/*********************************************************************************************/
/*                            FRAGMENT PROGRAM BODY RELATED RULES                            */
/*********************************************************************************************/

    program      : TOK_ARBFP10_HEADER ARBFP10_program
        { 
            ((IRProgram *)irtree)->setHeaderString(string($1.texto->c_str()));
        }
    ;
    ARBFP10_program      : optionSequence statementSequence TOK_END
        {
            ((IRProgram *)irtree)->addOptionList($1);
            ((IRProgram *)irtree)->addStatementList($2);
        }
    ;
    optionSequence       : optionSequence option
        {   
            $1->push_back($2);
            $$ = $1;
        }
                             | 
        {   
            $$ = new list<IROption *>;
        }
    ;
    option               : TOK_OPTION identifier ';'
        {
            $$ = new FP1IROption($1.linea,*$2);
        }
    ;
    statementSequence    : statementSequence statement
        {
            $1->push_back($2);
            $$ = $1;
        }
                             | 
        {   
            $$ = new list<IRStatement *>;
        }
    ;
    statement            : instruction ';'
        {
            $$ = $1;
        }
                             | namingStatement ';'
        {
            $$ = $1;
        }
    ;

/**********************************************************************************************/
/*                     FRAGMENT PROGRAM INSTRUCTIONS RELATED RULES                            */
/**********************************************************************************************/

    instruction          : ALUInstruction
        {   
            $$ = $1;
        }
                             | TexInstruction
        {   
            $$ = $1;
        }
    ;
    ALUInstruction       : VECTORop_instruction
        {
            $$ = $1;
        }
                             | SCALARop_instruction
        {
            $$ = $1;
        }
                             | BINSCop_instruction
        {
            $$ = $1;
        }
                             | BINop_instruction
        {
            $$ = $1;
        }
                             | TRIop_instruction
        {
            $$ = $1;
        }
                             | SWZ_instruction
        {
            $$ = $1;
        }
    ;
    TexInstruction       : SAMPLE_instruction
        {   
            $$ = $1;
        }
                             | KIL_instruction
        {
            $$ = $1;
        }
    ;
    VECTORop_instruction : VECTORop maskedDstReg ',' 
                               vectorSrcReg
        {   
            $2->setLine($1->getLine());
            $4->setLine($1->getLine());
            $1->setDest($2);
            $1->setSource0($4);
            $$=$1;
        }
    ;
    VECTORop             : TOK_ABS 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_ABS_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_FLR 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_FLR_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_FRC 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_FRC_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_LIT 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_LIT_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MOV 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MOV_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    SCALARop_instruction : SCALARop maskedDstReg ','
                               scalarSrcReg 
        {   
            $2->setLine($1->getLine());
            $4->setLine($1->getLine());
            $1->setDest($2);
            $1->setSource0($4);
            $$=$1;
        }
    ;
    SCALARop             : TOK_COS 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_COS_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_EX2 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_EX2_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_LG2 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_LG2_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_RCP 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_RCP_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_RSQ 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_RSQ_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SIN 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SIN_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SCS 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SCS_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    BINSCop_instruction  : BINSCop maskedDstReg ',' 
                               scalarSrcReg ',' scalarSrcReg 
        {   
            $2->setLine($1->getLine());
            $4->setLine($1->getLine());
            $6->setLine($1->getLine());
            $1->setDest($2);
            $1->setSource0($4);
            $1->setSource1($6);
            $$=$1;
        }
    ;
    BINSCop              : TOK_POW 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_POW_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    BINop_instruction    : BINop maskedDstReg ','
                               vectorSrcReg ',' vectorSrcReg
        {   
            $2->setLine($1->getLine());
            $4->setLine($1->getLine());
            $6->setLine($1->getLine());
            $1->setDest($2);
            $1->setSource0($4);
            $1->setSource1($6);
            $$=$1;
        }
    ;
    BINop                : TOK_ADD 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_ADD_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DP3 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DP3_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DP4 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DP4_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DPH 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DPH_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DST 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DST_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MAX 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MAX_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MIN 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MIN_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MUL 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MUL_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SGE 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SGE_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SLT 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SLT_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SUB 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SUB_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_XPD 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_XPD_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    TRIop_instruction    : TRIop maskedDstReg ','
                               vectorSrcReg ',' vectorSrcReg ','
                               vectorSrcReg
        {   
            $2->setLine($1->getLine());
            $4->setLine($1->getLine());
            $6->setLine($1->getLine());
            $8->setLine($1->getLine());
            $1->setDest($2);
            $1->setSource0($4);
            $1->setSource1($6);
            $1->setSource2($8);
            $$=$1;
        }
    ;
    TRIop                : TOK_CMP 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_CMP_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_LRP 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_LRP_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MAD 
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MAD_SAT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    SWZ_instruction      : SWZop maskedDstReg ',' 
                               srcReg ',' extendedSwizzle
        {
            $2->setLine($1->getLine());
            $4->setLine($1->getLine());
            $6->setLine($1->getLine());
            $1->setDest($2);
            $1->setSource0($4);
            $1->setSwizzleComponents($6);
            $$ = $1;
        }
    ;
    SWZop                : TOK_SWZ 
        {   
            $$ = new IRSwizzleInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SWZ_SAT
        {
            $$ = new IRSwizzleInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    SAMPLE_instruction   : SAMPLEop maskedDstReg ','
                               vectorSrcReg ',' texImageUnit ',' 
                               texTarget
        {
            $2->setLine($1->getLine());
            $4->setLine($1->getLine());
            $1->setDest($2);
            $1->setSource0($4);
            $1->setTextureImageUnit($6);
            $1->setTextureTarget(*$8);
            $$ = $1;
        }
    ;
    SAMPLEop             : TOK_TEX 
        {
            $$ = new IRSampleInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_TEX_SAT
        {
            $$ = new IRSampleInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_TXP 
        {
            $$ = new IRSampleInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_TXP_SAT
        {
            $$ = new IRSampleInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_TXB 
        {
            $$ = new IRSampleInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_TXB_SAT
        {
            $$ = new IRSampleInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    KIL_instruction      : TOK_KIL vectorSrcReg
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
            $2->setLine($$->getLine());
            $$->setSource0($2);
        }
    ;
    texImageUnit         : TOK_TEXTURE optTexImageUnitNum
        {
            $$ = $2;
        }
    ;
    texTarget            : TOK_1D
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_2D
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_3D
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_CUBE
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_RECT
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    optTexImageUnitNum   : 
        {   
            $$ = 0; // Default value of texture when no specified unit number
        }
                             | '[' texImageUnitNum ']'
        {   
            $$ = $2;
        }
    ;
    texImageUnitNum      : integer /* from 0 to MAX_TEXTURE_IMAGE_UNITS_ARB-1 */
        {
            $$ = $1;
        }
    ;
/**********************************************************************************************/
/*                     FRAGMENT PROGRAM OPERAND RELATED RULES                                 */
/**********************************************************************************************/

    scalarSrcReg         : optionalSign srcReg scalarSuffix
        {
            if ($1) $2->setNegateFlag();
            $2->setSwizzleMask(*$3);
            $$ = $2;
        }
    ;
    vectorSrcReg         : optionalSign srcReg optionalSuffix 
        {
            if ($1) $2->setNegateFlag();
            $2->setSwizzleMask(*$3);
            $$ = $2;
        }
    ;
    maskedDstReg         : dstReg optionalMask
        {
            $1->setWriteMask(*$2);
            $$ = $1;
        }
    ;
    extendedSwizzle  : optionalSign ExtSwizSel ','
                       optionalSign ExtSwizSel ',' 
                       optionalSign ExtSwizSel ',' 
                       optionalSign ExtSwizSel
        {
            $$ = new IRSwizzleComponents();
            $$->setComponent(0,$2,$1);
            $$->setComponent(1,$5,$4);
            $$->setComponent(2,$8,$7);
            $$->setComponent(3,$11,$10);
        }
    ;
    ExtSwizSel       : integer /* Must be 0 or 1 */
        {
            if ($1==0) $$ = IRSwizzleComponents::ZERO;
            else $$ = IRSwizzleComponents::ONE;
        }
                             | xyzw_rgbaComponent 
        {
            $$ = $1;
        }
    ;
    srcReg               : establishedName   /* Must be an established fragment attribute 
                                              * register, an established temporary register
                                              * or a program parameter single variable
                                              */
        {
            $$ = new IRSrcOperand(*$1);
        }
                             | fragAttribBinding
        {
            $$ = new IRSrcOperand(*$1);
            $$->setIsFragmentRegister(true);
        }
                             | progParamReg
        {   
            $$ = $1;
        }
    ;
    dstReg               : establishedName /* Must be a temporary Register or a fragment
                                            * result register
                                            */
        {
            $$ = new IRDstOperand(*$1);
        }
                             | resultBinding
        {   
            $$ = new IRDstOperand(*$1);
            $$->setIsFragmentResultRegister(true);
        }
    ;
    progParamReg         : progParamArray '[' progParamArrayAbs ']'
        {
            $$ = new IRSrcOperand(*$1);
            $$->setArrayAddressing(new IRArrayAddressing($3));
        }
                             | paramSingleItemUse
        {
            $$ = new IRSrcOperand(string(""));
            $$->setParamBinding($1);
        }
    ;
    progParamArray       : establishedName
        {
            $$ = $1;
        }
    ;
    progParamArrayAbs    : integer
        {
            $$ = $1;
        }
    ;
    scalarSuffix         : component /* Must be ".[xyzw]" or ".[rgba]" (scalar sufix) */
        {
            $$= $1;
        }
    ;
    optionalSuffix       :  
        {
            CREATE_STRING($$,"xyzw"); 
            //$$ = new string("xyzw");
        }
                             | component
        {
            $$ = $1;
        }
    ;
    xyzw_rgbaComponent        : identifier /* It has to be 'x','y','z','w',
                                            * 'r','g','b' or 'a'
                                            */
        {   
            if (!$1->compare("x")) $$ = IRSwizzleComponents::X;
            else if (!$1->compare("y")) $$ = IRSwizzleComponents::Y;
            else if (!$1->compare("z")) $$ = IRSwizzleComponents::Z;
            else if (!$1->compare("w")) $$ = IRSwizzleComponents::W;
            else if (!$1->compare("r")) $$ = IRSwizzleComponents::X;
            else if (!$1->compare("g")) $$ = IRSwizzleComponents::Y;
            else if (!$1->compare("b")) $$ = IRSwizzleComponents::Z;
            else if (!$1->compare("a")) $$ = IRSwizzleComponents::W;
        }
    ;
    optionalMask         : 
        {
            CREATE_STRING($$,"xyzw"); //$$ = new string("xyzw");
        }
                             | xyzw_rgbaMask
        {
            $$ = $1;
        }
    ;
    xyzw_rgbaMask        : component 
                         /* Must be ".x",".y",".xy",
                          * ".z",".xz",".yz",
                          * ".xyz",".w",".xw",
                          * ".yw",".xyw",".zw",
                          * ".xzw",".yzw",".xyzw",
                          * ".y",".xy",".z",
                          * ".xz",".yz",".xyz",
                          * ".w",".xw",".yw",
                          * ".xyw",".zw",".xzw",
                          * ".yzw",".xyzw" 
                          * or must be ".r",".g",".rg",
                          * ".b",".rb",".gb",
                          * ".rgb",".a",".ra",
                          * ".ga",".rga",".ba",
                          * ".rba",".gba",".rgba",
                          * ".g",".rg",".b",
                          * ".rb",".gb",".rgb",
                          * ".a",".ra",".ga",
                          * ".rga",".ba",".rba",
                          * ".gba",".rgba" 
                          */
        {
            $$ = $1;
        }
    ;

/**********************************************************************************************/
/*                     FRAGMENT PROGRAM NAMING STATEMENTS RELATED RULES                       */
/**********************************************************************************************/

    namingStatement      : ATTRIB_statement
        {
            $$ = $1;
        }
                             | PARAM_statement
        {
            $$ = $1;
        }
                             | TEMP_statement
        {
            $$ = $1;
        }
                             | OUTPUT_statement
        {
            $$ = $1;
        }
                             | ALIAS_statement
        {
            $$ = $1;
        }
    ;
    ATTRIB_statement     : TOK_ATTRIB establishName '='
                                   fragAttribBinding
        {
            $$ = new FP1IRATTRIBStatement($1.linea,*$2);
            $$->setInputAttribute(*$4);
        }
    ;
    fragAttribBinding    : TOK_FRAGMENT fragAttribItem
        {
            $$ = $2;
        }
    ;
    fragAttribItem       : TOK_COLOR optColorType
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
        }
                             | TOK_TEXCOORD optTexCoordNum
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
        }
                             | TOK_FOGCOORD
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_POSITION
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_ATTRIB_M '[' fragAttribNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
        }
    ;
    fragAttribNum     : integer /* from 0 to MAX_VERTEX_ATTRIBS_ARB-1 */
        {
            $$ = $1;
        }        
    ;
    
    ;
    PARAM_statement      : PARAM_singleStmt
        {
            $$ = $1;
        }
                             | PARAM_multipleStmt
        {   
            $$ = $1;
        }
    ;
    PARAM_singleStmt     : TOK_PARAM establishName paramSingleInit
        {
            $$ = new IRPARAMStatement($1.linea,*$2,false);
            $$->setSize(-1);
            $$->setParamBindings($3);
        }
    ;
    PARAM_multipleStmt   : TOK_PARAM establishName '[' optArraySize ']'
                                   paramMultipleInit
        {
            $$ = new IRPARAMStatement($1.linea,*$2,true);
            $$->setSize($4);
            $$->setParamBindings($6);
        }
    ;
    optArraySize         : 
        {
            $$ = -1; // It means that no optional size is specified
        }
                             | integer /* from 1 to MAX_PROGRAM_PARAMETERS_ARB 
                                        * (maximum number of allowed program 
                                        * parameter bindings) 
                                        */
        {   
            $$ = $1;
        }
    ;
    paramSingleInit      : '=' paramSingleItemDecl
        {
            $$ = new list<IRParamBinding *>;
            $$->push_back($2);
        }
    ;
    paramMultipleInit    : '=' '{' paramMultInitList '}'
        {
            $$ = $3;
        }
    ;
    paramMultInitList    : paramMultipleItem
        {
            $$ = new list<IRParamBinding *>;
            $$->push_back($1);
        }
                             |  paramMultInitList ',' paramMultipleItem  
        {
            $1->push_back($3);
            $$ = $1;
        }
    ;
    paramSingleItemDecl  : stateSingleItem
        {
            $$ = $1;
        }
                             | programSingleItem
        {
            $$ = $1;
        }
                             | paramConstDecl
        {
            $$ = $1;
        }
    ;
    paramSingleItemUse   : stateSingleItem
        {
            $1->setIsImplicitBinding(true);
            $$ = $1;
        }
                             | programSingleItem
        {   
            $1->setIsImplicitBinding(true);
            $$ = $1;
        }
                             | paramConstUse
        {
            $1->setIsImplicitBinding(true);
            $$ = $1;
        }
    ;
    paramMultipleItem    : stateMultipleItem
        {
            $$ = $1;
        }
                             | programMultipleItem
        {   
            $$ = $1;
        }
                             | paramConstDecl
        {
            $$ = $1;
        }
    ;
    stateMultipleItem    : stateSingleItem
        {
            $$ = $1;
        }
                             | TOK_STATE stateMatrixRows
        {
            $$ = new IRStateBinding(*$2,true);
        }
    ;
    stateSingleItem      : TOK_STATE stateMaterialItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateLightItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateLightModelItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateLightProdItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateTexEnvItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateFogItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateDepthItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateMatrixRow
        {
            $$ = new IRStateBinding(*$2,true);
        }
    ;
    stateMaterialItem    : TOK_MATERIAL optFaceType stateMatProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
            APPEND_STR(*$$,*$3)
        }
    ;
    stateMatProperty     : TOK_AMBIENT
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_DIFFUSE
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SPECULAR
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_EMISSION
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SHININESS
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateLightItem       : TOK_LIGHT '[' stateLightNumber ']' stateLightProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
            APPEND_STR(*$$,*$5)
        }
    ;
    stateLightProperty   : TOK_AMBIENT
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_DIFFUSE
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SPECULAR
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_POSITION
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_ATTENUATION
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SPOT stateSpotProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
        }
                             | TOK_HALF
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateSpotProperty    : TOK_DIRECTION
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateLightModelItem  : TOK_LIGHTMODEL stateLModProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
        }
    ;
    stateLModProperty    : TOK_AMBIENT
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | optFaceType TOK_SCENECOLOR
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*$1)
            APPEND_STR(*$$,*($2.texto))
        }
    ;
    stateLightProdItem   : TOK_LIGHTPROD '[' stateLightNumber ']'
                                 optFaceType stateLProdProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
            APPEND_STR(*$$,*$5)
            APPEND_STR(*$$,*$6)
        }
    ;
    stateLProdProperty   : TOK_AMBIENT
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_DIFFUSE
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SPECULAR
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateLightNumber     : integer /* from 0 to MAX_LIGHTS-1 */
        {
            $$ = $1;
        }
    ;
    stateTexEnvItem      : TOK_TEXENV optLegacyTexUnitNum stateTexEnvProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
            APPEND_STR(*$$,*$3)
        }
    ;
    stateTexEnvProperty  : TOK_COLOR
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    optLegacyTexUnitNum  : 
        {
            $$ = 0; // Default value
        }
                             | '[' legacyTexUnitNum ']'
        {   
            $$ = $2;
        }
    ;
    legacyTexUnitNum     : integer /* from 0 to MAX_TEXTURE_UNITS-1 */
        {
            $$ = $1;
        }
    ;
    stateFogItem         : TOK_FOG stateFogProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto));
            APPEND_STR(*$$,*$2);
        }
    ;
    stateFogProperty     : TOK_COLOR
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_PARAMS
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateDepthItem       : TOK_DEPTH stateDepthProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto));
            APPEND_STR(*$$,*$2);
        }
    ;
    stateDepthProperty   : TOK_RANGE
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateMatrixRow       : stateMatrixItem TOK_ROW '[' stateMatrixRowNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$, *$1)
            APPEND_STR(*$$,".row")
            APPEND_NUMBER_SELECTION(*$$,$4)
        }
    ;
    stateMatrixRows      : stateMatrixItem optMatrixRows
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$, *$1)
            APPEND_STR(*$$, *$2)
        }
    ;
    optMatrixRows        : 
        {
            CREATE_STRING($$,".row[0..3]"); 
            //$$ = new string(".row[0..3]");
        }
                             | TOK_ROW '[' stateMatrixRowNum TOK_POINT_POINT stateMatrixRowNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,".row[")
            APPEND_ROW_RANGE_SELECTION(*$$,$3,$5)
        }
    ;
    stateMatrixItem      : TOK_MATRIX stateMatrixName stateOptMatModifier
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$, *($1.texto))
            APPEND_STR(*$$, *$2)
            APPEND_STR(*$$, *$3)
        }
    ;
    stateOptMatModifier  : 
        {
            CREATE_STRING($$,".normal"); 
            //$$ = new string(".normal");
        }
                             | stateMatModifier
        {
            $$ = $1;
        }
    ;
    stateMatModifier     : TOK_INVERSE
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_TRANSPOSE
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_INVTRANS
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateMatrixRowNum    : integer /* from 0 to 3 */
        {
            $$ = $1;
        }
    ;
    stateMatrixName      : TOK_MODELVIEW stateOptModMatNum
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$, *($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
        }
                             | TOK_PROJECTION
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_MVP
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_TEXTURE optTexCoordNum
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$, *($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
        }
                             | TOK_PALETTE '[' statePaletteMatNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$, *($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
        }
                             | TOK_PROGRAM '[' stateProgramMatNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$, *($1.texto));
            APPEND_NUMBER_SELECTION(*$$,$3);
        }
    ;                        
    stateOptModMatNum    : 
        {
            $$ = 0; // Default value
        }
                             | '[' stateModMatNum ']'
        {
            $$ = $2;
        }
    ;
    stateModMatNum       : integer /* from 0 to MAX_VERTEX_UNITS_ARB-1 */
        {
            $$ = $1;
        }
    ;
    optTexCoordNum       : 
        {
            $$ = 0; // Default value
        }
                             | '[' texCoordNum ']'
        {   
            $$ = $2;
        }
    ;
    texCoordNum          : integer /* from 0 to MAX_TEXTURE_COORDS_ARB-1 */
        {
            $$ = $1;
        }
    ;
    statePaletteMatNum   : integer /* from 0 to MAX_PALETTE_MATRICES_ARB-1 */
        {
            $$ = $1;
        }
    ;
    stateProgramMatNum   : integer /* from 0 to MAX_PROGRAM_MATRICES_ARB-1 */
        {
            $$ = $1;
        }
    ;
    programSingleItem    : progEnvParam
        {   
            $$ = $1;
        }
                             | progLocalParam
        {   
            $$ = $1;
        }
    ;
    programMultipleItem  : progEnvParams
        {   
            $$ = $1;
        }
                             | progLocalParams
        {
            $$ = $1;
        }
    ;
    progEnvParams        : TOK_PROGRAM TOK_ENV '[' progEnvParamNums ']'
        {
            $4->setLine($1.linea);
            $$ = $4;
        }
    ;
    progEnvParamNums     : progEnvParamNum
        {
            $$ = new IRLocalEnvBinding(IRLocalEnvBinding::ENVPARAM);
            $$->setIndices($1,$1);
        }
                             | progEnvParamNum TOK_POINT_POINT progEnvParamNum
        {
            $$ = new IRLocalEnvBinding(IRLocalEnvBinding::ENVPARAM);
            $$->setIndices($1,$3);
        }
    ;
    progEnvParam         : TOK_PROGRAM TOK_ENV '[' progEnvParamNum ']'
        {
            $$ = new IRLocalEnvBinding(IRLocalEnvBinding::ENVPARAM);
            $$->setLine($1.linea);
            $$->setIndices($4,$4);
        }
    ;
    progLocalParams      : TOK_PROGRAM TOK_LOCAL '[' progLocalParamNums ']'
        {
            $4->setLine($1.linea);
            $$ = $4;
        }
    ;
    progLocalParamNums   : progLocalParamNum
        {
            $$ = new IRLocalEnvBinding(IRLocalEnvBinding::LOCALPARAM);
            $$->setIndices($1,$1);
        }
                             | progLocalParamNum TOK_POINT_POINT progLocalParamNum
        {
            $$ = new IRLocalEnvBinding(IRLocalEnvBinding::LOCALPARAM);
            $$->setIndices($1,$3);
        }
    ;
    progLocalParam       : TOK_PROGRAM TOK_LOCAL '[' progLocalParamNum ']'
        {
            $$ = new IRLocalEnvBinding(IRLocalEnvBinding::LOCALPARAM);
            $$->setLine($1.linea);
            $$->setIndices($4,$4);
        }
    ;
    progEnvParamNum      : integer /* from 0 to MAX_PROGRAM_ENV_PARAMETERS_ARB - 1 */
        {
            $$ = $1;
        }
    ;
    progLocalParamNum    : integer /* from 0 to MAX_PROGRAM_LOCAL_PARAMETERS_ARB - 1 */
        {
            $$ = $1;
        }
    ;
    paramConstDecl       : paramConstScalarDecl
        {
            $$ = $1;
        }
                             | paramConstVector
        {
            $$ = $1;
        }
    ;
    paramConstUse        : paramConstScalarUse
        {
            $$ = $1;
        }
                             | paramConstVector
        {
            $$ = $1;
        }
    ;
    paramConstScalarDecl : signedFloatConstant
        {
            $$ = new IRConstantBinding($1,0.0f,0.0f,0.0f,true);
        }
    ;
    paramConstScalarUse  : floatConstant
        {
            $$ = new IRConstantBinding($1,$1,$1,$1,true);
        }
    ;
    paramConstVector     : '{' signedFloatConstant '}'
        {
            $$ = new IRConstantBinding($2,0.0f,0.0f,1.0f);
        }
                             | '{' signedFloatConstant ',' 
                                   signedFloatConstant '}'
        {
            $$ = new IRConstantBinding($2,$4,0.0f,1.0f);
        }
                             | '{' signedFloatConstant ',' 
                                   signedFloatConstant ','
                                   signedFloatConstant '}'
        {
            $$ = new IRConstantBinding($2,$4,$6,1.0f);
        }
                             | '{' signedFloatConstant ',' 
                                   signedFloatConstant ','
                                   signedFloatConstant ',' 
                                   signedFloatConstant '}'
        {
            $$ = new IRConstantBinding($2,$4,$6,$8);
        }
    ;
    signedFloatConstant  : optionalSign floatConstant
        {
            if ($1) $$ = (-1)*$2;
            else $$ = $2;
        }
    ;
    optionalSign         : 
        {
            $$ = false;
        }
                             | '-'
        {
            $$ = true;
        }
                             | '+'
        {
            $$ = false;
        }
    ;
    TEMP_statement       : TOK_TEMP establishName varNameList
        {
            $$ = new IRTEMPStatement($1.linea,*$2);
            $$->setOtherTemporaries($3);
        }
    ;
    varNameList          :
        {
            $$ = new list<string>();
        }
                             | varNameList ',' establishName 
        {
            $1->push_back(*$3);
            $$ = $1;
        }
    ;
    OUTPUT_statement     : TOK_OUTPUT establishName '='
                                 resultBinding
        {
            $$ = new FP1IROUTPUTStatement($1.linea,*$2);
            $$->setOutputAttribute(*$4);
        }
    ;
    resultBinding        : TOK_RESULT TOK_COLOR
        {
            CREATE_STRING($$,*($2.texto)); 
            //$$ = new string(*($2.texto));
        }
                             | TOK_RESULT TOK_DEPTH
        {
            CREATE_STRING($$,*($2.texto)); 
            //$$ = new string(*($2.texto));
        }
    ;
    optFaceType          : 
        {
            CREATE_STRING($$,".front"); 
            //$$ = new string(".front"); // Default
        }
                             | TOK_FRONT
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_BACK
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    optColorType         : 
        {
            CREATE_STRING($$,".primary"); 
            //$$ = new string(".primary"); // Default
        }
                             | TOK_PRIMARY
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SECONDARY
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    ALIAS_statement      : TOK_ALIAS establishName '='
                                 establishedName
        {
            $$ = new IRALIASStatement($1.linea,*$2);
            $$->setAlias(*$4);
        }
    ;
    establishName        : identifier
        {   
            $$ = $1;
        }
    ;
    establishedName      : identifier
        {
            $$ = $1;
        }
    ;
    identifier           : TOK_IDENT
        {
            CREATE_STRING($$,*($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    component        : TOK_COMPONENT
        {
            CREATE_STRING($$,&($1.texto->c_str()[1])); 
            //$$ = new string(&($1.texto->c_str()[1]));
        }
    ;
    integer              : TOK_INT_CONST
        {
            STRING_TO_VAR(*($1.texto), $$)
        }
    ;
    floatConstant        : TOK_FLOAT_CONST
        {
            STRING_TO_VAR(*($1.texto), $$)
        }
                             | TOK_INT_CONST /* For implicit conversion of int to float */
        {
            STRING_TO_VAR(*($1.texto), $$)
        }
    ;
%%

#include "FP1Flex.gen" 

int fp1StartParse(void *ptr)
{
    yyparse(ptr);
    
    list<string*>::iterator iter = fp1LexSymbolCollector.begin();
    
    while(iter != fp1LexSymbolCollector.end())
    {
        delete (*iter);
        iter++;
    }
    
    fp1LexSymbolCollector.clear();

    iter = fp1ASTStringCollector.begin();
    
    while(iter != fp1ASTStringCollector.end())
    {   
        delete (*iter);
        iter++;
    }
    
    fp1ASTStringCollector.clear();
    
    return 0;
}

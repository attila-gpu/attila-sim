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
#include <iostream>
#include <string>
#include <sstream>

using namespace std;

int yylex();

#include "IRNode.h"

using namespace libgl::GenerationCode;
using namespace libgl;

#define APPEND_STR(str,str1)                       { (str).append(str1); }
#define APPEND_NUMBER_SELECTION(string,number)     { stringstream ss; ss << "[" << number << "]"; (string).append(ss.str()); }
#define APPEND_ROW_RANGE_SELECTION(string,min,max) { stringstream ss; ss << "[" << min << ".." << max << "]"; (string).append(ss.str()); }
#define STRING_TO_VAR(str, var)                    { stringstream ss(str, ios::in); ss >> var; }

#define CREATE_STRING(str_ptr,TEXT)                { (str_ptr) = new string(TEXT); vp1ASTStringCollector.push_back(str_ptr); }

#define CREATE_EMPTY_STRING(str_ptr)               CREATE_STRING(str_ptr, "")


void yyerror(void* irtree, char *s)
{
    panic("VP1Grammar.y","yyerror()","Sintactic error in Vertex Program");
}

typedef struct{
  string* texto;
  int linea;
} type_token_atrib;  

list<string*> vp1LexSymbolCollector;

list<string*> vp1ASTStringCollector;

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
  IRADDRESSStatement*                       iraddrstmnt;
  IRATTRIBStatement*                        irattrstmnt;
  IROUTPUTStatement*                        iroutputstmnt;
  IRPARAMStatement*                         irparamstmnt;
  IRParamBinding*                           irparambind;
  IRArrayAddressing*                        irarrayaddr;
  IRLocalEnvBinding*                        irlocalenvbind;
}

%token <token_atrib> TOK_ARBVP10_HEADER TOK_END TOK_OPTION TOK_ARL TOK_ABS TOK_ADD TOK_DP3 TOK_DP4 TOK_DPH TOK_DST
TOK_EX2 TOK_EXP TOK_FLR TOK_FRC TOK_LG2 TOK_LIT TOK_LOG TOK_MAD TOK_MAX TOK_MIN TOK_MOV TOK_MUL TOK_POW TOK_RCP 
TOK_RSQ TOK_SGE TOK_SLT TOK_SUB TOK_SWZ TOK_XPD TOK_PARAM TOK_ATTRIB TOK_TEMP TOK_ADDRESS TOK_OUTPUT TOK_ALIAS
TOK_VERTEX TOK_PROGRAM TOK_STATE TOK_RESULT TOK_INT_CONST TOK_FLOAT_CONST TOK_IDENT TOK_COMPONENT TOK_ROW TOK_POSITION
TOK_NORMAL TOK_FOGCOORD TOK_WEIGHT TOK_COLOR TOK_TEXCOORD TOK_MATRIXINDEX TOK_ATTRIB_M TOK_MATERIAL TOK_AMBIENT
TOK_DIFFUSE TOK_SPECULAR TOK_EMISSION TOK_SHININESS TOK_LIGHT TOK_SPOT TOK_HALF TOK_DIRECTION TOK_LIGHTMODEL
TOK_SCENECOLOR TOK_LIGHTPROD TOK_TEXGEN TOK_EYE TOK_OBJECT TOK_S_COORD TOK_T_COORD TOK_R_COORD TOK_Q_COORD TOK_FOG 
TOK_PARAMS TOK_CLIP TOK_PLANE TOK_POINT_C TOK_SIZE_C TOK_ATTENUATION TOK_MATRIX TOK_INVERSE TOK_TRANSPOSE
TOK_INVTRANS TOK_MODELVIEW TOK_PROJECTION TOK_MVP TOK_TEXTURE TOK_PALETTE TOK_FRONT TOK_BACK TOK_PRIMARY TOK_SECONDARY
TOK_POINTSIZE TOK_POINT_POINT TOK_ENV TOK_LOCAL

%parse-param { void* irtree }

%type <irprogram> program ARBVP10_program

%type <iroption> option
%type <irstatement> statement namingStatement

%type <irinstruction> instruction ARL_instruction
                      VECTORop_instruction VECTORop SCALARop_instruction SCALARop 
                      BINSCop_instruction BINSCop BINop_instruction BINop 
                      TRIop_instruction TRIop

%type <irswzinstr> SWZ_instruction
%type <irswzcomps> extendedSwizzle
%type <irswzcomp> ExtSwizSel xyzw_rgbaComponent

%type <str> identifier component establishName establishedName optFaceType
            optColorType optionalMask xyzw_rgbaMask resultBinding resultColBinding
            vtxAttribBinding vtxAttribItem vertexAttribReg scalarSuffix swizzleSuffix 
            progParamArray stateLightProperty stateMatProperty stateSpotProperty
            stateLModProperty stateLProdProperty stateFogProperty vertexResultReg
            stateMaterialItem stateLightItem stateLightProdItem addrComponent
            stateLightModelItem stateFogItem addrWriteMask
            stateMatrixItem stateMatrixRow stateOptMatModifier
            stateMatModifier stateMatrixName stateMatrixRows optMatrixRows
            stateTexGenItem stateTexGenType stateTexGenCoord stateClipPlaneItem
            statePointItem statePointProperty addrReg 

%type <strlist> varTempNameList varAddrNameList
%type <optlist> optionSequence
%type <stmntlist> statementSequence
%type <pbindlist> paramMultInitList paramSingleInit paramMultipleInit

%type <floatconst> floatConstant signedFloatConstant
%type <intconst> integer vtxAttribNum vtxOptWeightNum vtxWeightNum
                 optTexCoordNum texCoordNum optArraySize progEnvParamNum progLocalParamNum
                 stateLightNumber stateMatrixRowNum stateClipPlaneNum
                 stateModMatNum statePaletteMatNum stateProgramMatNum stateOptModMatNum
                 addrRegRelOffset addrRegPosOffset addrRegNegOffset

%type <boolean> optionalSign

%type <irdstoperand> maskedDstReg dstReg maskedAddrReg
%type <irsrcoperand> scalarSrcReg swizzleSrcReg srcReg progParamReg

%type <iraliasstmnt> ALIAS_statement
%type <irtempstmnt> TEMP_statement
%type <iraddrstmnt> ADDRESS_statement
%type <irattrstmnt> ATTRIB_statement
%type <iroutputstmnt> OUTPUT_statement
%type <irparamstmnt> PARAM_statement PARAM_singleStmt PARAM_multipleStmt

%type <irparambind> paramSingleItemUse stateSingleItem programSingleItem paramConstUse
                    paramConstScalarUse paramConstVector paramSingleItemDecl paramMultipleItem
                    stateMultipleItem programMultipleItem paramConstDecl paramConstScalarDecl
                    progEnvParams progLocalParams
                    
%type <irarrayaddr> progParamArrayMem progParamArrayRel progParamArrayAbs
%type <irlocalenvbind>  progEnvParamNums progEnvParam progLocalParamNums progLocalParam

%start program
%%

/*********************************************************************************************/
/*                            VERTEX PROGRAM BODY RELATED RULES                              */
/*********************************************************************************************/

    program      : TOK_ARBVP10_HEADER ARBVP10_program
        { 
            ((IRProgram *)irtree)->setHeaderString(string($1.texto->c_str()));
        }
    ;
    ARBVP10_program      : optionSequence statementSequence TOK_END
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
            $$ = new list<IROption*>;
        }
    ;
    option               : TOK_OPTION identifier ';'
        {
            $$ = new VP1IROption($1.linea,*$2);
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
/*                     VERTEX PROGRAM INSTRUCTIONS RELATED RULES                              */
/**********************************************************************************************/

    instruction          : ARL_instruction
        {
            $$ = $1;
        }
                             | VECTORop_instruction
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
    ARL_instruction      : TOK_ARL maskedAddrReg ',' scalarSrcReg
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
            $2->setLine($$->getLine());
            $4->setLine($$->getLine());
            $$->setDest($2);
            $$->setSource0($4);
        }
    ;
    VECTORop_instruction : VECTORop maskedDstReg ',' swizzleSrcReg
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
                             | TOK_FLR
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_FRC
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_LIT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MOV
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    SCALARop_instruction : SCALARop maskedDstReg ',' scalarSrcReg
        {   
            $2->setLine($1->getLine());
            $4->setLine($1->getLine());
            $1->setDest($2);
            $1->setSource0($4);
            $$=$1;
        }
    ;
    SCALARop             : TOK_EX2
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_EXP
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_LG2
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }                             
                             | TOK_LOG
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }                             
                            | TOK_RCP
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_RSQ
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    BINSCop_instruction  : BINSCop maskedDstReg ',' scalarSrcReg ',' scalarSrcReg 
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
    ;
    BINop_instruction    : BINop maskedDstReg ',' swizzleSrcReg ',' swizzleSrcReg
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
                             | TOK_DP3
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DP4
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DPH
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_DST
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MAX
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MIN
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_MUL
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SGE
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SLT
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_SUB
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
                             | TOK_XPD
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    TRIop_instruction    : TRIop maskedDstReg ',' swizzleSrcReg ',' swizzleSrcReg ',' 
                        swizzleSrcReg
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
    TRIop                : TOK_MAD
        {
            $$ = new IRInstruction($1.linea,string($1.texto->c_str()));
        }
    ;
    SWZ_instruction      : TOK_SWZ maskedDstReg ',' srcReg ',' extendedSwizzle
        {
            $$ = new IRSwizzleInstruction($1.linea,string($1.texto->c_str()));
            $2->setLine($$->getLine());
            $4->setLine($$->getLine());
            $6->setLine($$->getLine());
            $$->setDest($2);
            $$->setSource0($4);
            $$->setSwizzleComponents($6);
        }
    ;

/**********************************************************************************************/
/*                     VERTEX PROGRAM OPERAND RELATED RULES                                 */
/**********************************************************************************************/

    scalarSrcReg         : optionalSign srcReg scalarSuffix
        {
            if ($1) $2->setNegateFlag();
            $2->setSwizzleMask(*$3);
            $$ = $2;
        }
    ;
    swizzleSrcReg        : optionalSign srcReg swizzleSuffix
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
    maskedAddrReg        : addrReg addrWriteMask
        {
            $$ = new IRDstOperand(*$1);
            $$->setWriteMask(*$2);
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
    srcReg               : vertexAttribReg
        {
            $$ = new IRSrcOperand(*$1);
            $$->setIsVertexRegister(true);
        }
                             | progParamReg
        {   
            $$ = $1;
        }
                 | establishedName /* Must be an established vertex attribute 
                          register, an established temporary register
                          or a program parameter single variable */
        {
            $$ = new IRSrcOperand(*$1);
        }
    ;
    dstReg               : vertexResultReg
        {   
            $$ = new IRDstOperand(*$1);
            $$->setIsVertexResultRegister(true);
        }
                             | establishedName /* Must be and established vertex result
                                                  register or an established temporary 
                                                  register */
        {
            $$ = new IRDstOperand(*$1);
        }
    ;
    vertexAttribReg      : vtxAttribBinding
        {
            $$ = $1;
        }
    ;
    progParamReg         : progParamArray '[' progParamArrayMem ']'
        {
            $$ = new IRSrcOperand(*$1);
            $$->setArrayAddressing($3);
        }
                             | paramSingleItemUse
        {
            $$ = new IRSrcOperand(string(""));
            $$->setParamBinding($1);
        }
    ;
    progParamArray       : establishedName /* Must be a program param array */
        {
            $$ = $1;
        }
    ;
    progParamArrayMem    : progParamArrayAbs
        {
            $$ = $1;           
        }
                             | progParamArrayRel
        {
            $$ = $1;
        }
    ;                      
    progParamArrayAbs    : integer  
        {
            $$ = new IRArrayAddressing($1);
        }
    ;
    progParamArrayRel    : addrReg addrComponent addrRegRelOffset
        {
            $$ = new IRArrayAddressing(0,true);
            $$->setRelativeAddressing(*$1, *$2, $3);
        }
    ;
    addrRegRelOffset     : 
        { 
            $$ = 0; // Default offset
        }
                             | '+' addrRegPosOffset
        {
            $$ = $2;
        }
                             | '-' addrRegNegOffset
        {
            $$ = -$2;
        }
        
    ;
    addrRegPosOffset     : integer /* from 0 to 63 */
        {
            $$ = $1;
        }
    ;
    addrRegNegOffset     : integer /* from 0 to 64 */
        {
            $$ = $1;
        }
    ;
    vertexResultReg      : resultBinding
        {
            $$ = $1;
        }
    ;
    addrReg              : establishedName
        {
            $$ = $1;
        }
    ;
    addrComponent        : component /* Must be ".x" */
        { 
            $$ = $1;
        }
    ;
    addrWriteMask        : component /* Must be ".x" */
        {
            $$ = $1;
        }
    ;
    scalarSuffix         : component /* Must be ".[xyzw]" (scalar sufix) */
        {
            $$= $1;
        }
    ;
    swizzleSuffix        :  
        {
            CREATE_STRING($$, "xyzw"); 
            // $$ = new string("xyzw");
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
            CREATE_STRING($$, "xyzw"); 
            //$$ = new string("xyzw");
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
/*                     VERTEX PROGRAM NAMING STATEMENTS RELATED RULES                         */
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
                             | ADDRESS_statement
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
    ATTRIB_statement     : TOK_ATTRIB establishName '=' vtxAttribBinding
        {
            $$ = new VP1IRATTRIBStatement($1.linea,*$2);
            $$->setInputAttribute(*$4);
        }
    ;
    vtxAttribBinding     : TOK_VERTEX vtxAttribItem
        {
            $$ = $2;
        }
    ;
    vtxAttribItem        : TOK_POSITION  
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                 | TOK_WEIGHT vtxOptWeightNum
        {
            CREATE_EMPTY_STRING($$);
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
        }
                 | TOK_NORMAL
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                 | TOK_COLOR optColorType
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
        }
                 | TOK_FOGCOORD
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                 | TOK_TEXCOORD optTexCoordNum
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
        }
                 | TOK_MATRIXINDEX '[' vtxWeightNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
        }
                 | TOK_ATTRIB_M '[' vtxAttribNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
        }
    ;
    vtxAttribNum     : integer /* from 0 to MAX_VERTEX_ATTRIBS_ARB-1 */
        {
            $$ = $1;
        }
    ;
    vtxOptWeightNum  : integer
        {
            $$ = 0; // Default value
        }
         
                     | '[' vtxWeightNum ']'
        {
            $$ = $2;
        }
    ;
    vtxWeightNum          : integer /* from 0 to MAX_VERTEX_UNITS_ARB-1 and must be
                                       divisible by four*/
        {
            $$ = $1;
        }
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
                                 (maximum number of allowed program 
                                  parameter bindings) */
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
            $$ = new list<IRParamBinding*>;
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
                             | TOK_STATE stateTexGenItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateFogItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE stateClipPlaneItem
        {
            $$ = new IRStateBinding(*$2);
        }
                             | TOK_STATE statePointItem
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
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                 | TOK_DIFFUSE 
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                 | TOK_SPECULAR
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                 | TOK_EMISSION
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                 | TOK_SHININESS
        {
            CREATE_STRING($$, *($1.texto)); 
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
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_DIFFUSE
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SPECULAR
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_POSITION
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }

                             | TOK_ATTENUATION
        {
            CREATE_STRING($$, *($1.texto)); 
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
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateSpotProperty    : TOK_DIRECTION 
        {
            CREATE_STRING($$, *($1.texto)); 
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
            CREATE_STRING($$, *($1.texto)); 
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
            CREATE_STRING($$, *($1.texto)); 
            // $$ = new string(*($1.texto));
        }
                             | TOK_DIFFUSE
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SPECULAR
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateLightNumber     : integer /* from 0 to MAX_LIGHTS-1 */
        {
            $$ = $1;
        }
    ;
    stateTexGenItem      : TOK_TEXGEN optTexCoordNum stateTexGenType stateTexGenCoord
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
            APPEND_STR(*$$,*$3)
            APPEND_STR(*$$,*$4)
        }
    ;
    stateTexGenType      : TOK_EYE
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_OBJECT
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }

    ;
    stateTexGenCoord     : TOK_S_COORD
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_T_COORD 
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_R_COORD
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_Q_COORD
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateFogItem         : TOK_FOG stateFogProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
        }
    ;
    stateFogProperty     : TOK_COLOR
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_PARAMS
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    stateClipPlaneItem   : TOK_CLIP '[' stateClipPlaneNum ']' TOK_PLANE
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
            APPEND_STR(*$$,*($5.texto))
        }
    ;
    stateClipPlaneNum    : integer /* from 0 to MAX_CLIP_PLANES-1 */
        {
            $$ = $1;
        }
    ;
    statePointItem       : TOK_POINT_C statePointProperty
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
        }
    ;
    statePointProperty   : TOK_SIZE_C
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_ATTENUATION
        {
            CREATE_STRING($$, *($1.texto)); 
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
            CREATE_STRING($$, ".row[0..3]"); 
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
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
            APPEND_STR(*$$,*$3)
        }
    ;
    stateOptMatModifier  : 
        {
            CREATE_STRING($$, ".normal"); 
            //$$ = new string(".normal");
        }
                             | stateMatModifier
        {
            $$ = $1;
        }
    ;
    stateMatModifier     : TOK_INVERSE
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_TRANSPOSE
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_INVTRANS
        {
            CREATE_STRING($$, *($1.texto)); 
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
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
        }
                             | TOK_PROJECTION
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_MVP
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_TEXTURE optTexCoordNum
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$2)
        }
                             | TOK_PALETTE '[' statePaletteMatNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
        }
                             | '.' TOK_PROGRAM '[' stateProgramMatNum ']'
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($2.texto))
            APPEND_NUMBER_SELECTION(*$$,$4)
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
    TEMP_statement       : TOK_TEMP establishName varTempNameList
        {
            $$ = new IRTEMPStatement($1.linea,*$2);
            $$->setOtherTemporaries($3);
        }
    ;
    ADDRESS_statement    : TOK_ADDRESS establishName varAddrNameList
        {
            $$ = new IRADDRESSStatement($1.linea,*$2);
            $$->setOtherAddressRegisters($3);
        }
    ;
    varTempNameList      : 
        {
            $$ = new list<string>();
        }
                             | varTempNameList ',' establishName
        {
            $1->push_back(*$3);
            $$ = $1;
        }
    ;
    varAddrNameList      : 
        {
            $$ = new list<string>();
        }
                             | varAddrNameList ',' establishName
        {
            $1->push_back(*$3);
            $$ = $1;
        }
    ;
    OUTPUT_statement     : TOK_OUTPUT establishName '='
                                 resultBinding
        {
            $$ = new VP1IROUTPUTStatement($1.linea,*$2);
            $$->setOutputAttribute(*$4);
        }
    ;
    resultBinding         : TOK_RESULT TOK_POSITION
        {
            CREATE_STRING($$, *($2.texto)); 
            //$$ = new string(*($2.texto));
        }
                     | TOK_RESULT resultColBinding
        {   
            $$ = $2;
        }
                             | TOK_RESULT TOK_FOGCOORD
        {
            CREATE_STRING($$, *($2.texto)); 
            //$$ = new string(*($2.texto));
        }
                             | TOK_RESULT TOK_POINTSIZE
        {
            CREATE_STRING($$, *($2.texto)); 
            //$$ = new string(*($2.texto));
        }
                             | TOK_RESULT TOK_TEXCOORD optTexCoordNum
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($2.texto))
            APPEND_NUMBER_SELECTION(*$$,$3)
        }
    ;
    resultColBinding     : TOK_COLOR optFaceType optColorType
        {
            CREATE_EMPTY_STRING($$); 
            //$$ = new string;
            APPEND_STR(*$$,*($1.texto))
            APPEND_STR(*$$,*$2)
            APPEND_STR(*$$,*$3)
        }
        ;
    optFaceType          : 
        {
            CREATE_STRING($$, ".front"); 
            //$$ = new string(".front"); // Default
        }
                             | TOK_FRONT
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_BACK
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    optColorType         : 
        {
            CREATE_STRING($$, ".primary"); 
            // $$ = new string(".primary"); // Default
        }
                             | TOK_PRIMARY
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
                             | TOK_SECONDARY
        {
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
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
    texCoordNum          : integer /* from 0 to MAX_TEXTURE_UNITS_ARB-1 */
        {
            $$ = $1;
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
            CREATE_STRING($$, *($1.texto)); 
            //$$ = new string(*($1.texto));
        }
    ;
    component        : TOK_COMPONENT
        {
            CREATE_STRING($$, &($1.texto->c_str()[1])); 
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

#include "VP1Flex.gen" 

int vp1StartParse(void *ptr)
{  
    yyparse(ptr);
    
    list<string*>::iterator iter = vp1LexSymbolCollector.begin();
    
    while(iter != vp1LexSymbolCollector.end())
    {   
        //delete (*iter);
        iter++;
    }
    
    vp1LexSymbolCollector.clear();

    iter = vp1ASTStringCollector.begin();
    
    while(iter != vp1ASTStringCollector.end())
    {   
        //delete (*iter);
        iter++;
    }
    
    vp1ASTStringCollector.clear();
    
    return 0;
}

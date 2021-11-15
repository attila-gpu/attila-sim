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
static int lin=1;

#ifndef FLEXDEBUG
    #define ECHO
    #define PRINTLINE
#else
    #define PRINTLINE printf("%3d: ",lin)
#endif

#define AssignTextAndLine    yylval.token_atrib.texto = new string(yytext);\
                             acdxFp1LexSymbolCollector.push_back(yylval.token_atrib.texto);\
                             yylval.token_atrib.linea = lin;

#define AssignLine           yylval.token_atrib.linea = lin;

/*
 *  To distinguish between a float constant in the form "4." (meaning 4.0) and
 *  the prefix of a range ("4..5") we have a condition:
 *  
 *  {UNSIGNED_INTEGER}"."[^.]        {AssignTextAndLine  ReturnLastSymbol ECHO;
 *                                    return FLOAT_CONST; }
 *  
 *  it means that we cannot tokenize a "4." as a Float constant if it has another 
 *  dot after. But if it´s recognized because next is not a dot (for example a 4., 
 *      of a string "{ 4.,3.,2.,1. }") we have to return the "," in this case for future 
 *  token analysis. Flex has the yyless() function to return last symbol analyzed.
 */
    
#define ReturnLastSymbol     yyless(yyleng-1);

/* The rules section has this parts:
 *  - Token macros
 *  - The initial fragment program version token.   
 *  - Directive statement reserved words (in upcase).
 *  - Instruction mnemonic reserved words (in upcase).
 *  - Instruction satured version mnemonic reserved words (in upcase).  
 *  - OpenGL target texture specifiers.
 *  - Other reserved words (in lowercase).
 *  - Components and properties starting with a dot. They are not reserved words because 
 *    an identifier cannot start with a dot.
 *  - Integer and float constants. 
 *  - Identifiers.
 *  - Components as write or swizzle masks. Because they also 
 *    start with a dot, this clause can´t be set before 
 *    component and properties tokens specified above.
 */
 
%}

%option noyywrap

DIGIT               [0-9]
LETTER              [a-zA-Z\_\$]
INTEGER             [+-]?{DIGIT}+
UNSIGNED_INTEGER    {DIGIT}+
FRACTION            {UNSIGNED_INTEGER}
EXPONENT            [Ee]INTEGER

%%

"!!ARBfp1.0"                     {AssignTextAndLine  ECHO; return TOK_ARBFP10_HEADER; }

"END"                            {AssignLine         ECHO; return TOK_END; }
"OPTION"                         {AssignLine         ECHO; return TOK_OPTION; }
"PARAM"                          {AssignLine         ECHO; return TOK_PARAM; }
"ATTRIB"                         {AssignLine         ECHO; return TOK_ATTRIB; }
"TEMP"                           {AssignLine         ECHO; return TOK_TEMP; }
"OUTPUT"                         {AssignLine         ECHO; return TOK_OUTPUT; }
"ALIAS"                          {AssignLine         ECHO; return TOK_ALIAS; }


"ABS"                            {AssignTextAndLine  ECHO; return TOK_ABS; }
"ADD"                            {AssignTextAndLine  ECHO; return TOK_ADD; }
"CMP"                            {AssignTextAndLine  ECHO; return TOK_CMP; }
"COS"                            {AssignTextAndLine  ECHO; return TOK_COS; }
"DP3"                            {AssignTextAndLine  ECHO; return TOK_DP3; }
"DP4"                            {AssignTextAndLine  ECHO; return TOK_DP4; }
"DPH"                            {AssignTextAndLine  ECHO; return TOK_DPH; }
"DST"                            {AssignTextAndLine  ECHO; return TOK_DST; }
"EX2"                            {AssignTextAndLine  ECHO; return TOK_EX2; }
"FLR"                            {AssignTextAndLine  ECHO; return TOK_FLR; }
"FRC"                            {AssignTextAndLine  ECHO; return TOK_FRC; }
"KIL"                            {AssignTextAndLine  ECHO; return TOK_KIL; }


"KLS"							 {AssignTextAndLine  ECHO; return TOK_KLS; }
"ZXP"							 {AssignTextAndLine  ECHO; return TOK_ZXP; }
"ZXS"							 {AssignTextAndLine  ECHO; return TOK_ZXS; }
"CMP_KIL"                        {AssignTextAndLine  ECHO; return TOK_CMPKIL; }
"CHS"                            {AssignTextAndLine  ECHO; return TOK_CHS; }
"FXMUL"                          {AssignTextAndLine  ECHO; return TOK_FXMUL; }
"FXMAD"                          {AssignTextAndLine  ECHO; return TOK_FXMAD; }
"FXMAD2"                         {AssignTextAndLine  ECHO; return TOK_FXMAD2; }
"LG2"                            {AssignTextAndLine  ECHO; return TOK_LG2; }
"LIT"                            {AssignTextAndLine  ECHO; return TOK_LIT; }
"LRP"                            {AssignTextAndLine  ECHO; return TOK_LRP; }
"MAD"                            {AssignTextAndLine  ECHO; return TOK_MAD; }
"MAX"                            {AssignTextAndLine  ECHO; return TOK_MAX; }
"MIN"                            {AssignTextAndLine  ECHO; return TOK_MIN; }
"MOV"                            {AssignTextAndLine  ECHO; return TOK_MOV; }
"MUL"                            {AssignTextAndLine  ECHO; return TOK_MUL; }
"POW"                            {AssignTextAndLine  ECHO; return TOK_POW; }
"RCP"                            {AssignTextAndLine  ECHO; return TOK_RCP; }
"RSQ"                            {AssignTextAndLine  ECHO; return TOK_RSQ; }
"SIN"                            {AssignTextAndLine  ECHO; return TOK_SIN; }
"SCS"                            {AssignTextAndLine  ECHO; return TOK_SCS; }
"SGE"                            {AssignTextAndLine  ECHO; return TOK_SGE; }
"SLT"                            {AssignTextAndLine  ECHO; return TOK_SLT; }
"SUB"                            {AssignTextAndLine  ECHO; return TOK_SUB; }
"SWZ"                            {AssignTextAndLine  ECHO; return TOK_SWZ; }
"TEX"                            {AssignTextAndLine  ECHO; return TOK_TEX; }
"TXB"                            {AssignTextAndLine  ECHO; return TOK_TXB; }
"TXP"                            {AssignTextAndLine  ECHO; return TOK_TXP; }
"XPD"                            {AssignTextAndLine  ECHO; return TOK_XPD; }

"ABS_SAT"                            {AssignTextAndLine  ECHO; return TOK_ABS_SAT; }
"ADD_SAT"                            {AssignTextAndLine  ECHO; return TOK_ADD_SAT; }
"CMP_SAT"                            {AssignTextAndLine  ECHO; return TOK_CMP_SAT; }
"COS_SAT"                            {AssignTextAndLine  ECHO; return TOK_COS_SAT; }
"DP3_SAT"                            {AssignTextAndLine  ECHO; return TOK_DP3_SAT; }
"DP4_SAT"                            {AssignTextAndLine  ECHO; return TOK_DP4_SAT; }
"DPH_SAT"                            {AssignTextAndLine  ECHO; return TOK_DPH_SAT; }
"DST_SAT"                            {AssignTextAndLine  ECHO; return TOK_DST_SAT; }
"EX2_SAT"                            {AssignTextAndLine  ECHO; return TOK_EX2_SAT; }
"FLR_SAT"                            {AssignTextAndLine  ECHO; return TOK_FLR_SAT; }
"FRC_SAT"                            {AssignTextAndLine  ECHO; return TOK_FRC_SAT; }
"LG2_SAT"                            {AssignTextAndLine  ECHO; return TOK_LG2_SAT; }
"LIT_SAT"                            {AssignTextAndLine  ECHO; return TOK_LIT_SAT; }
"LRP_SAT"                            {AssignTextAndLine  ECHO; return TOK_LRP_SAT; }
"MAD_SAT"                            {AssignTextAndLine  ECHO; return TOK_MAD_SAT; }
"MAX_SAT"                            {AssignTextAndLine  ECHO; return TOK_MAX_SAT; }
"MIN_SAT"                            {AssignTextAndLine  ECHO; return TOK_MIN_SAT; }
"MOV_SAT"                            {AssignTextAndLine  ECHO; return TOK_MOV_SAT; }
"MUL_SAT"                            {AssignTextAndLine  ECHO; return TOK_MUL_SAT; }
"POW_SAT"                            {AssignTextAndLine  ECHO; return TOK_POW_SAT; }
"RCP_SAT"                            {AssignTextAndLine  ECHO; return TOK_RCP_SAT; }
"RSQ_SAT"                            {AssignTextAndLine  ECHO; return TOK_RSQ_SAT; }
"SIN_SAT"                            {AssignTextAndLine  ECHO; return TOK_SIN_SAT; }
"SCS_SAT"                            {AssignTextAndLine  ECHO; return TOK_SCS_SAT; }
"SGE_SAT"                            {AssignTextAndLine  ECHO; return TOK_SGE_SAT; }
"SLT_SAT"                            {AssignTextAndLine  ECHO; return TOK_SLT_SAT; }
"SUB_SAT"                            {AssignTextAndLine  ECHO; return TOK_SUB_SAT; }
"SWZ_SAT"                            {AssignTextAndLine  ECHO; return TOK_SWZ_SAT; }
"TEX_SAT"                            {AssignTextAndLine  ECHO; return TOK_TEX_SAT; }
"TXB_SAT"                            {AssignTextAndLine  ECHO; return TOK_TXB_SAT; }
"TXP_SAT"                            {AssignTextAndLine  ECHO; return TOK_TXP_SAT; }
"XPD_SAT"                            {AssignTextAndLine  ECHO; return TOK_XPD_SAT; }

"1D"                                 {AssignTextAndLine  ECHO; return TOK_1D; }
"2D"                                 {AssignTextAndLine  ECHO; return TOK_2D; }
"3D"                                 {AssignTextAndLine  ECHO; return TOK_3D; }
"CUBE"                               {AssignTextAndLine  ECHO; return TOK_CUBE; }
"RECT"                               {AssignTextAndLine  ECHO; return TOK_RECT; }


"program"                        {AssignTextAndLine  ECHO; return TOK_PROGRAM; }
"result"                         {AssignLine         ECHO; return TOK_RESULT; }
"state"                          {AssignLine         ECHO; return TOK_STATE; }
"fragment"                       {AssignLine         ECHO; return TOK_FRAGMENT; }
"texture"                        {AssignLine         ECHO; return TOK_TEXTURE; }
"sample"                         {AssignLine         ECHO; return TOK_SAMPLE; }

".color"                         {AssignTextAndLine  ECHO; return TOK_COLOR; }
".primary"                       {AssignTextAndLine  ECHO; return TOK_PRIMARY; }
".secondary"                     {AssignTextAndLine  ECHO; return TOK_SECONDARY; }
".texcoord"                      {AssignTextAndLine  ECHO; return TOK_TEXCOORD; }
".fogcoord"                      {AssignTextAndLine  ECHO; return TOK_FOGCOORD; }
".position"                      {AssignTextAndLine  ECHO; return TOK_POSITION; }
".attrib"                        {AssignTextAndLine  ECHO; return TOK_ATTRIB_M; }
".env"                           {AssignLine         ECHO; return TOK_ENV; }
".local"                         {AssignLine         ECHO; return TOK_LOCAL; }
".material"                      {AssignTextAndLine  ECHO; return TOK_MATERIAL; }
".ambient"                       {AssignTextAndLine  ECHO; return TOK_AMBIENT; }
".diffuse"                       {AssignTextAndLine  ECHO; return TOK_DIFFUSE; }
".specular"                      {AssignTextAndLine  ECHO; return TOK_SPECULAR; }
".emission"                      {AssignTextAndLine  ECHO; return TOK_EMISSION; }
".shininess"                     {AssignTextAndLine  ECHO; return TOK_SHININESS; }
".front"                         {AssignTextAndLine  ECHO; return TOK_FRONT; }
".back"                          {AssignTextAndLine  ECHO; return TOK_BACK; }
".light"                         {AssignTextAndLine  ECHO; return TOK_LIGHT; }
".spot"                          {AssignTextAndLine  ECHO; return TOK_SPOT; }
".direction"                     {AssignTextAndLine  ECHO; return TOK_DIRECTION; }
".half"                          {AssignTextAndLine  ECHO; return TOK_HALF; }
".attenuation"                   {AssignTextAndLine  ECHO; return TOK_ATTENUATION; }
".lightmodel"                    {AssignTextAndLine  ECHO; return TOK_LIGHTMODEL; }
".scenecolor"                    {AssignTextAndLine  ECHO; return TOK_SCENECOLOR; }
".lightprod"                     {AssignTextAndLine  ECHO; return TOK_LIGHTPROD; }
".texenv"                        {AssignTextAndLine  ECHO; return TOK_TEXENV; }
".fog"                           {AssignTextAndLine  ECHO; return TOK_FOG; }
".params"                        {AssignTextAndLine  ECHO; return TOK_PARAMS; }
".depth"                         {AssignTextAndLine  ECHO; return TOK_DEPTH; }
".range"                         {AssignTextAndLine  ECHO; return TOK_RANGE; }
".matrix"                        {AssignTextAndLine  ECHO; return TOK_MATRIX; }
".modelview"                     {AssignTextAndLine  ECHO; return TOK_MODELVIEW; }
".projection"                    {AssignTextAndLine  ECHO; return TOK_PROJECTION; }
".mvp"                           {AssignTextAndLine  ECHO; return TOK_MVP; }
".texture"                       {AssignTextAndLine  ECHO; return TOK_TEXTURE; }
".palette"                       {AssignTextAndLine  ECHO; return TOK_PALETTE; }
".inverse"                       {AssignTextAndLine  ECHO; return TOK_INVERSE; }
".transpose"                     {AssignTextAndLine  ECHO; return TOK_TRANSPOSE; }
".invtrans"                      {AssignTextAndLine  ECHO; return TOK_INVTRANS; }
".row"                           {AssignTextAndLine  ECHO; return TOK_ROW; }


".."                             {AssignLine         ECHO; return TOK_POINT_POINT; }


{UNSIGNED_INTEGER}                           {AssignTextAndLine  ECHO; return TOK_INT_CONST; }
{UNSIGNED_INTEGER}"."{FRACTION}              {AssignTextAndLine  ECHO; return TOK_FLOAT_CONST; } 
"."{FRACTION}                                {AssignTextAndLine  ECHO; return TOK_FLOAT_CONST; } 
{UNSIGNED_INTEGER}"."{FRACTION}{EXPONENT}    {AssignTextAndLine  ECHO; return TOK_FLOAT_CONST; } 
{UNSIGNED_INTEGER}"."{EXPONENT}              {AssignTextAndLine  ECHO; return TOK_FLOAT_CONST; } 
"."{FRACTION}{EXPONENT}                      {AssignTextAndLine  ECHO; return TOK_FLOAT_CONST; } 
{UNSIGNED_INTEGER}"."[^.]                    {AssignTextAndLine  ReturnLastSymbol ECHO;
                                              return TOK_FLOAT_CONST; }

{LETTER}+({LETTER}|{DIGIT})*                 {AssignTextAndLine  ECHO; return TOK_IDENT; }

"."{LETTER}+({LETTER}|{DIGIT})*              {AssignTextAndLine  ECHO; return TOK_COMPONENT; }

[ \t]+                                       {ECHO;} /* skip spaces */
\#[^\n]*                                     {;} /* comments */
[\r]?\n                                      {ECHO; lin++; PRINTLINE;}
.                                            {ECHO; return *yytext;}
%%
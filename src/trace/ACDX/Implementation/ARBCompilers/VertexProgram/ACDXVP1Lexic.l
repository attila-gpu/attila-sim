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
                             acdxVp1LexSymbolCollector.push_back(yylval.token_atrib.texto);\
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
 *  - The initial vertex program version token. 
 *  - Directive statement reserved words (in upcase).
 *  - Instruction mnemonic reserved words (in upcase).  
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
EXPONENT            [Ee]{INTEGER}

%%

 
"!!ARBvp1.0"                     {AssignTextAndLine  ECHO; return TOK_ARBVP10_HEADER; }


"END"                            {AssignLine         ECHO; return TOK_END; }
"OPTION"                         {AssignLine         ECHO; return TOK_OPTION; }
"PARAM"                          {AssignLine         ECHO; return TOK_PARAM; }
"ATTRIB"                         {AssignLine         ECHO; return TOK_ATTRIB; }
"TEMP"                           {AssignLine         ECHO; return TOK_TEMP; }
"ADDRESS"                        {AssignLine         ECHO; return TOK_ADDRESS; }
"OUTPUT"                         {AssignLine         ECHO; return TOK_OUTPUT; }
"ALIAS"                          {AssignLine         ECHO; return TOK_ALIAS; }


"ARL"                            {AssignTextAndLine  ECHO; return TOK_ARL; }
"ABS"                            {AssignTextAndLine  ECHO; return TOK_ABS; }
"ADD"                            {AssignTextAndLine  ECHO; return TOK_ADD; }
"DP3"                            {AssignTextAndLine  ECHO; return TOK_DP3; }
"DP4"                            {AssignTextAndLine  ECHO; return TOK_DP4; }
"DPH"                            {AssignTextAndLine  ECHO; return TOK_DPH; }
"DST"                            {AssignTextAndLine  ECHO; return TOK_DST; }
"EX2"                            {AssignTextAndLine  ECHO; return TOK_EX2; }
"EXP"                            {AssignTextAndLine  ECHO; return TOK_EXP; }
"FLR"                            {AssignTextAndLine  ECHO; return TOK_FLR; }
"FRC"                            {AssignTextAndLine  ECHO; return TOK_FRC; }
"LG2"                            {AssignTextAndLine  ECHO; return TOK_LG2; }
"LIT"                            {AssignTextAndLine  ECHO; return TOK_LIT; }
"LOG"                            {AssignTextAndLine  ECHO; return TOK_LOG; }
"MAD"                            {AssignTextAndLine  ECHO; return TOK_MAD; }
"MAX"                            {AssignTextAndLine  ECHO; return TOK_MAX; }
"MIN"                            {AssignTextAndLine  ECHO; return TOK_MIN; }
"MOV"                            {AssignTextAndLine  ECHO; return TOK_MOV; }
"MUL"                            {AssignTextAndLine  ECHO; return TOK_MUL; }
"POW"                            {AssignTextAndLine  ECHO; return TOK_POW; }
"RCP"                            {AssignTextAndLine  ECHO; return TOK_RCP; }
"RSQ"                            {AssignTextAndLine  ECHO; return TOK_RSQ; }
"SGE"                            {AssignTextAndLine  ECHO; return TOK_SGE; }
"SLT"                            {AssignTextAndLine  ECHO; return TOK_SLT; }
"SUB"                            {AssignTextAndLine  ECHO; return TOK_SUB; }
"SWZ"                            {AssignTextAndLine  ECHO; return TOK_SWZ; }
"XPD"                            {AssignTextAndLine  ECHO; return TOK_XPD; }


"program"                        {AssignTextAndLine  ECHO; return TOK_PROGRAM; }
"result"                         {AssignLine         ECHO; return TOK_RESULT; }
"state"                          {AssignLine         ECHO; return TOK_STATE; }
"vertex"                         {AssignLine         ECHO; return TOK_VERTEX; }


".position"                      {AssignTextAndLine  ECHO; return TOK_POSITION; }
".normal"                        {AssignTextAndLine  ECHO; return TOK_NORMAL; }
".fogcoord"                      {AssignTextAndLine  ECHO; return TOK_FOGCOORD; }
".weight"                        {AssignTextAndLine  ECHO; return TOK_WEIGHT; }
".color"                         {AssignTextAndLine  ECHO; return TOK_COLOR; }
".texcoord"                      {AssignTextAndLine  ECHO; return TOK_TEXCOORD; }
".matrixindex"                   {AssignTextAndLine  ECHO; return TOK_MATRIXINDEX; } 
".attrib"                        {AssignTextAndLine  ECHO; return TOK_ATTRIB_M; }
".material"                      {AssignTextAndLine  ECHO; return TOK_MATERIAL; }
".ambient"                       {AssignTextAndLine  ECHO; return TOK_AMBIENT; }
".diffuse"                       {AssignTextAndLine  ECHO; return TOK_DIFFUSE; }
".specular"                      {AssignTextAndLine  ECHO; return TOK_SPECULAR; }
".emission"                      {AssignTextAndLine  ECHO; return TOK_EMISSION; }
".shininess"                     {AssignTextAndLine  ECHO; return TOK_SHININESS; }
".light"                         {AssignTextAndLine  ECHO; return TOK_LIGHT; }
".spot"                          {AssignTextAndLine  ECHO; return TOK_SPOT; }
".half"                          {AssignTextAndLine  ECHO; return TOK_HALF; }
".direction"                     {AssignTextAndLine  ECHO; return TOK_DIRECTION; }
".lightmodel"                    {AssignTextAndLine  ECHO; return TOK_LIGHTMODEL; }
".scenecolor"                    {AssignTextAndLine  ECHO; return TOK_SCENECOLOR; }
".lightprod"                     {AssignTextAndLine  ECHO; return TOK_LIGHTPROD; }
".texgen"                        {AssignTextAndLine  ECHO; return TOK_TEXGEN; }
".eye"                           {AssignTextAndLine  ECHO; return TOK_EYE; }
".object"                        {AssignTextAndLine  ECHO; return TOK_OBJECT; }
".s"                             {AssignTextAndLine  ECHO; return TOK_S_COORD; }
".t"                             {AssignTextAndLine  ECHO; return TOK_T_COORD; }
".r"                             {AssignTextAndLine  ECHO; return TOK_R_COORD; }
".q"                             {AssignTextAndLine  ECHO; return TOK_Q_COORD; }
".fog"                           {AssignTextAndLine  ECHO; return TOK_FOG; }
".params"                        {AssignTextAndLine  ECHO; return TOK_PARAMS; }
".clip"                          {AssignTextAndLine  ECHO; return TOK_CLIP; }
".plane"                         {AssignTextAndLine  ECHO; return TOK_PLANE; }
".point"                         {AssignTextAndLine  ECHO; return TOK_POINT_C; }
".size"                          {AssignTextAndLine  ECHO; return TOK_SIZE_C; }
".attenuation"                   {AssignTextAndLine  ECHO; return TOK_ATTENUATION; }
".row"                           {AssignTextAndLine  ECHO; return TOK_ROW; }
".matrix"                        {AssignTextAndLine  ECHO; return TOK_MATRIX; }
".inverse"                       {AssignTextAndLine  ECHO; return TOK_INVERSE; }
".transpose"                     {AssignTextAndLine  ECHO; return TOK_TRANSPOSE; }
".invtrans"                      {AssignTextAndLine  ECHO; return TOK_INVTRANS; }
".modelview"                     {AssignTextAndLine  ECHO; return TOK_MODELVIEW; }
".projection"                    {AssignTextAndLine  ECHO; return TOK_PROJECTION; }
".mvp"                           {AssignTextAndLine  ECHO; return TOK_MVP; }
".texture"                       {AssignTextAndLine  ECHO; return TOK_TEXTURE; }
".palette"                       {AssignTextAndLine  ECHO; return TOK_PALETTE; }
".front"                         {AssignTextAndLine  ECHO; return TOK_FRONT; }
".back"                          {AssignTextAndLine  ECHO; return TOK_BACK; }
".primary"                       {AssignTextAndLine  ECHO; return TOK_PRIMARY; }
".secondary"                     {AssignTextAndLine  ECHO; return TOK_SECONDARY; }
".env"                           {AssignTextAndLine  ECHO; return TOK_ENV; }
".local"                         {AssignTextAndLine  ECHO; return TOK_LOCAL; }
".pointsize"                     {AssignTextAndLine  ECHO; return TOK_POINTSIZE; }
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
\#[^\n]*                                     {;}     /* comments */
[\r]										 {;}
[\r]?\n                                      {ECHO; lin++; PRINTLINE;}
.                                            {ECHO; return *yytext;}
%%

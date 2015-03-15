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

#include "ACDXCodeGenTraverser.h"
#include "ShaderInstruction.h"
#include "GPU.h"

#include <cstdlib>

using namespace acdlib;
using namespace acdlib::GenerationCode;
using namespace std;
using namespace gpu3d;
using namespace acdlib;

/* ****************************************************************************
 *                      AUXILIAR FUNCTIONS
 * ****************************************************************************
 */


/**
 * ProcessSwizzeSuffix
 * 
 * We encode the component x of the suffix with binary code 0x00, y with binary code 0x01,
 * z with binary code 0x10 and w with binary code 0x11. 
 * The swizzle suffix has four components (c1,c2,c3,c4) that can be applied with these codes.
 */

unsigned int ACDXCodeGenTraverser::processSwizzleSuffix(const char* suffix) {

    unsigned int ret = 0;

    if (strlen(suffix) == 1)
    {                          /* only one component specified. We assume:
                                *     .x is equivalent to .xxxx,
                                *     .y is equivalent to .yyyy,
                                *     .z is equivalent to .zzzz,
                                *     .w is equivalent to .wwww,
                                *     .r is equivalent to .rrrr,
                                *     .g is equivalent to .gggg,
                                *     .b is equivalent to .bbbb,
                                *     .a is equivalent to .aaaa
                                */
        switch(suffix[0])
        {
            case 'x': ret = 0x00; break;
            case 'y': ret = 0x55; break;
            case 'z': ret = 0xAA; break;
            case 'w': ret = 0xFF; break;
            case 'r': ret = 0x00; break;
            case 'g': ret = 0x55; break;
            case 'b': ret = 0xAA; break;
            case 'a': ret = 0xFF; break;
        }
    }
    else
    { /* Suffix with the four components */
        int desp = 6;
        ret = 0;
        for (int i=0;i<4;i++)
        {
            switch(suffix[i])
            {
                case 'x': ret = ret + (0 << desp); break;
                case 'y': ret = ret + (1 << desp); break;
                case 'z': ret = ret + (2 << desp); break;
                case 'w': ret = ret + (3 << desp); break;
                case 'r': ret = ret + (0 << desp); break;
                case 'g': ret = ret + (1 << desp); break;
                case 'b': ret = ret + (2 << desp); break;
                case 'a': ret = ret + (3 << desp); break;
            }
            desp = desp - 2;
        }
    }
    return ret;
}

/* ****************
 * ProcessWriteMask
 * ****************
 * The write mask can specify 1, 2, 3 or the 4 components of the result vector register.
 * if w component is specified then the least significant bit of the result is active, otherwise
 * not. If z component is specified then the next bit more significant is active, ...
 * We return the number that encodes this mask of bits.
 */
 
unsigned int ACDXCodeGenTraverser::processWriteMask(const char* mask)
{
    unsigned int ret = 0x00;
    unsigned int i;
    
    for (i=0;i<strlen(mask);i++)
    {
        switch(mask[i])
        {
            case 'x': ret = ret + 8; break;
            case 'y': ret = ret + 4; break;
            case 'z': ret = ret + 2; break;
            case 'w': ret = ret + 1; break;
            case 'r': ret = ret + 8; break;
            case 'g': ret = ret + 4; break;
            case 'b': ret = ret + 2; break;
            case 'a': ret = ret + 1; break;
        }
    }
    return ret;
}

/**
 *   Fragment Attribute Binding      Components  Underlying State               Register
 *   --------------------------      ----------  ----------------------------   --------
 *   fragment.position               (x,y,z,1/w) window position                POSITION_ATTRIBUTE
 *   fragment.color                  (r,g,b,a)   primary color                  COLOR_ATTRIBUTE
 *   fragment.color.primary          (r,g,b,a)   primary color                  COLOR_ATTRIBUTE
 *   fragment.color.secondary        (r,g,b,a)   secondary color                COLOR_ATTRIBUTE_SEC
 *   fragment.fogcoord               (f,0,0,1)   fog distance/coordinate        5
 *   fragment.texcoord               (s,t,r,q)   texture coordinate, unit 0     6
 *   fragment.texcoord[n]            (s,t,r,q)   texture coordinate, unit n     6+n
 */

unsigned int ACDXCodeGenTraverser::fragmentAttributeBindingMapToRegister(const string attribute)
{
    unsigned int ret = 0;
    unsigned int index;
        
    if (!getWordBetweenDots(attribute,0).compare("position"))
        ret = POSITION_ATTRIBUTE;

    else if (!getWordBetweenDots(attribute,0).compare("color"))
    {
        ret = COLOR_ATTRIBUTE;
        if (!getWordBetweenDots(attribute,1).compare("secondary"))
            ret = COLOR_ATTRIBUTE_SEC;
    }
    else if (!getWordBetweenDots(attribute,0).compare("fogcoord"))
        ret = 5;
    
    else if (!getWordWithoutIndex(getWordBetweenDots(attribute,0),index).compare("texcoord"))
    {
        ret = (6 + index);
    }
    else if (!getWordWithoutIndex(getWordBetweenDots(attribute,0),index).compare("attrib"))
    {
        ret = index;
    }
    return ret;
}
/**
 *    Vertex Attribute Binding       Components  Underlying State               Register
 *    ------------------------       ----------  ---------------------------    --------
 *    vertex.position                (x,y,z,w)   object coordinates             0      
 *    vertex.weight                  (w,w,w,w)   vertex weights 0-3             1      
 *    vertex.weight[n]               (w,w,w,w)   vertex weights n-n+3           1      
 *    vertex.normal                  (x,y,z,1)   normal                         2      
 *    vertex.color                   (r,g,b,a)   primary color                  3      
 *    vertex.color.primary           (r,g,b,a)   primary color                  3      
 *    vertex.color.secondary         (r,g,b,a)   secondary color                4      
 *    vertex.fogcoord                (f,0,0,1)   fog coordinate                 5      
 *    vertex.texcoord                (s,t,r,q)   texture coordinate, unit 0     8      
 *    vertex.texcoord[n]             (s,t,r,q)   texture coordinate, unit n     8 + n      
 *    vertex.matrixindex             (i,i,i,i)   vertex matrix indices 0-3      8 + M     
 *    vertex.matrixindex[n]          (i,i,i,i)   vertex matrix indices n-n+3    9 + M     
 *    vertex.attrib[n]               (x,y,z,w)   generic vertex attribute n     n     
 *                 
 *    Where M = MAX_TEXTURE_UNITS_ARB
 */

unsigned int ACDXCodeGenTraverser::vertexAttributeBindingMapToRegister(const string attribute)
{
    unsigned int ret=0;
    unsigned int index;
        
    if (!getWordBetweenDots(attribute,0).compare("position"))
        ret = 0;

    else if (!getWordWithoutIndex(getWordBetweenDots(attribute,0),index).compare("weight"))
    {
        ret = 1;        
    }
    else if (!getWordBetweenDots(attribute,0).compare("normal"))
    {
        ret = 2;
    }
    else if (!getWordBetweenDots(attribute,0).compare("color"))
    {
        ret = 3;
        if (!getWordBetweenDots(attribute,1).compare("secondary"))
            ret = 4;
    }
    else if (!getWordBetweenDots(attribute,0).compare("fogcoord"))
        ret = 5;
    
    else if (!getWordWithoutIndex(getWordBetweenDots(attribute,0),index).compare("texcoord"))
    {
        ret = (8 + index);
    }
    else if (!getWordWithoutIndex(getWordBetweenDots(attribute,0),index).compare("matrixindex"))
    {
        ret = (8 + MAX_TEXTURE_UNITS_ARB + index);
    }
    else if (!getWordWithoutIndex(getWordBetweenDots(attribute,0),index).compare("attrib"))
    {
        ret = index;
    }
    return ret;
}

/*
 *  Binding                          Components  Description                    Register
 *  -----------------------------    ----------  ----------------------------   --------
 *  result.depth                     (*,*,d,*)   depth coordinate               POSITION_ATTRIBUTE
 *  result.color                     (r,g,b,a)   color                          COLOR_ATTRIBUTE
 */

unsigned int ACDXCodeGenTraverser::fragmentResultAttribBindingMaptoRegister(const string attribute)
{
    unsigned int ret = 0;
    
    if (!getWordBetweenDots(attribute,0).compare("depth"))
        ret = POSITION_ATTRIBUTE;

    else if (!getWordBetweenDots(attribute,0).compare("color"))
        ret = COLOR_ATTRIBUTE;

    return ret;
}

 
/**
 *    Binding                        Components  Description                    Register
 *    -----------------------------  ----------  ----------------------------   --------
 *    result.position                (x,y,z,w)   position in clip coordinates   POSITION_ATTRIBUTE
 *    result.color                   (r,g,b,a)   front-facing primary color     COLOR_ATTRIBUTE
 *    result.color.primary           (r,g,b,a)   front-facing primary color     COLOR_ATTRIBUTE
 *    result.color.secondary         (r,g,b,a)   front-facing secondary color   COLOR_ATTRIBUTE_SEC
 *    result.color.front             (r,g,b,a)   front-facing primary color     COLOR_ATTRIBUTE
 *    result.color.front.primary     (r,g,b,a)   front-facing primary color     COLOR_ATTRIBUTE
 *    result.color.front.secondary   (r,g,b,a)   front-facing secondary color   COLOR_ATTRIBUTE_SEC
 *    result.color.back              (r,g,b,a)   back-facing primary color      COLOR_ATTRIBUTE_BACK_PRI
 *    result.color.back.primary      (r,g,b,a)   back-facing primary color      COLOR_ATTRIBUTE_BACK_PRI
 *    result.color.back.secondary    (r,g,b,a)   back-facing secondary color    COLOR_ATTRIBUTE_BACK_SEC
 *    result.fogcoord                (f,*,*,*)   fog coordinate                 5
 *    result.texcoord                (s,t,r,q)   texture coordinate, unit 0     6
 *    result.texcoord[n]             (s,t,r,q)   texture coordinate, unit n     6 + n
 *    result.pointsize               (s,*,*,*)   point size                     6 + M
 *
 *    Where M = MAX_TEXTURE_UNITS_ARB
 */    

unsigned int ACDXCodeGenTraverser::vertexResultAttribBindingMaptoRegister(const string attribute)
{
    unsigned int ret = 0;
    unsigned int index;
    
    if (!getWordBetweenDots(attribute,0).compare("position"))
        ret = POSITION_ATTRIBUTE;

    else if (!getWordBetweenDots(attribute,0).compare("color"))
    {
        if (!getWordBetweenDots(attribute,1).compare("front"))
        {
            if ((!getWordBetweenDots(attribute,2).compare("primary")))
                ret = COLOR_ATTRIBUTE;
            else // secondary
                ret = COLOR_ATTRIBUTE_SEC;
        }
        else // Back
        {
            if ((!getWordBetweenDots(attribute,2).compare("primary")))
                ret = COLOR_ATTRIBUTE_BACK_PRI;
            else // secondary
                ret = COLOR_ATTRIBUTE_BACK_SEC;
        }
    }
    else if (!getWordBetweenDots(attribute,0).compare("fogcoord"))
        ret = 5;

    else if (!getWordBetweenDots(attribute,0).compare("pointsize"))
        ret = (6 + MAX_TEXTURE_UNITS_ARB);

    else if (!getWordWithoutIndex(getWordBetweenDots(attribute,0),index).compare("texcoord"))
        ret = (6 + index);    

    return ret;
}


string ACDXCodeGenTraverser::getWordBetweenDots(string str, unsigned int level)
{
    string aux;
    aux = str.substr(1); // Skip the first dot

    unsigned int i = 0;
    while(i<level)
    {
        aux = aux.substr(aux.find(".")+1);
        i++;
    }
    return aux.substr(0,aux.find("."));
}

string ACDXCodeGenTraverser::getWordWithoutIndex(string str, unsigned int& index)
{
    string aux;
    aux = str.substr(0,str.find("["));
    index = atoi(str.substr(str.find("[")+1,str.find("]")).c_str());
    return aux;
}

unsigned int ACDXCodeGenTraverser::paramStateBindingIdentifier(const std::string state, unsigned int& rowIndex)
{
    unsigned int ret = 0;
    unsigned int index;
    
    rowIndex = 0;

    if (!getWordBetweenDots(state,0).compare("material"))
    {
        // ret += BASE_MATERIAL;
        if (!getWordBetweenDots(state,1).compare("front"))
            ret += MATERIAL_FRONT;
        else 
            ret += MATERIAL_BACK;

        if (!getWordBetweenDots(state,2).compare("ambient"))
            ret += MATERIAL_AMBIENT_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("diffuse"))
            ret += MATERIAL_DIFFUSE_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("specular"))
            ret += MATERIAL_SPECULAR_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("emission"))
            ret += MATERIAL_EMISSION_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("shininess"))
            ret += MATERIAL_SHINNINESS_OFFSET;
    }
    else if (!getWordWithoutIndex(getWordBetweenDots(state,0),index).compare("light"))
    {
        ret += BASE_LIGHTS;
        ret += index*7;

        if (!getWordBetweenDots(state,1).compare("ambient"))
            ret += LIGHT_AMBIENT_OFFSET;
        else if (!getWordBetweenDots(state,1).compare("diffuse"))
            ret += LIGHT_DIFFUSE_OFFSET;
        else if (!getWordBetweenDots(state,1).compare("specular"))
            ret += LIGHT_SPECULAR_OFFSET;
        else if (!getWordBetweenDots(state,1).compare("position"))
            ret += LIGHT_POSITION_OFFSET;
        else if (!getWordBetweenDots(state,1).compare("attenuation"))
            ret += LIGHT_ATTENUATION_OFFSET;
        else if (!getWordBetweenDots(state,1).compare("spot"))
            ret += LIGHT_SPOTDIRECTION_OFFSET;
        else if (!getWordBetweenDots(state,1).compare("half"))
            ret += LIGHT_HALF_OFFSET;

    }
    else if (!getWordBetweenDots(state,0).compare("lightmodel"))
    {
        //ret += BASE_LIGHTMODEL;
        if (!getWordBetweenDots(state,1).compare("ambient"))
            ret += LIGHTMODEL_AMBIENT;
        else if (!getWordBetweenDots(state,1).compare("front"))
            ret += LIGHTMODEL_FRONT_SCENECOLOR;
        else
            ret += LIGHTMODEL_BACK_SCENECOLOR;
        
    }
    else if (!getWordWithoutIndex(getWordBetweenDots(state,0),index).compare("lightprod"))
    {
        ret += BASE_LIGHTPROD;
        ret += index*6;
        
        if (!getWordBetweenDots(state,1).compare("front"))
            ret += LIGHTPROD_FRONT_OFFSET;
        else 
            ret += LIGHTPROD_BACK_OFFSET;

        if (!getWordBetweenDots(state,2).compare("ambient"))
            ret += LIGHTPROD_AMBIENT_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("diffuse"))
            ret += LIGHTPROD_DIFFUSE_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("specular"))
            ret += LIGHTPROD_SPECULAR_OFFSET;
    }
    else if (!getWordWithoutIndex(getWordBetweenDots(state,0),index).compare("texgen"))
    {
        ret += BASE_TEXGEN;
        ret += index*8;

        if (!getWordBetweenDots(state,1).compare("eye"))
            ret += TEXGEN_EYE_OFFSET;
        else 
            ret += TEXGEN_OBJECT_OFFSET;

        if (!getWordBetweenDots(state,2).compare("s"))
            ret += TEXGEN_S_COORD_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("t"))
            ret += TEXGEN_T_COORD_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("r"))
            ret += TEXGEN_R_COORD_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("q"))
            ret += TEXGEN_Q_COORD_OFFSET;

    }
    else if (!getWordBetweenDots(state,0).compare("fog"))
    {
        // ret += BASE_FOG;
        if (!getWordBetweenDots(state,1).compare("color"))
            ret += FOG_COLOR;
        else
            ret += FOG_PARAMS;
        
    }
    else if (!getWordWithoutIndex(getWordBetweenDots(state,0),index).compare("clip"))
    {
        ret += BASE_CLIP;
        ret += index;
    }
    else if (!getWordBetweenDots(state,0).compare("point"))
    {
        //ret += BASE_POINT;
        if (!getWordBetweenDots(state,1).compare("size"))
            ret += POINT_SIZE;
        else
            ret += POINT_ATTENUATION;
    }
    else if (!getWordBetweenDots(state,0).compare("matrix"))
    {
        ret += BASE_STATE_MATRIX;
        if (!getWordWithoutIndex(getWordBetweenDots(state,1),index).compare("modelview"))
        {
            ret += STATE_MATRIX_MODELVIEW_OFFSET;
            ret += index*4;
        }
        else if (!getWordWithoutIndex(getWordBetweenDots(state,1),index).compare("projection"))
            ret += STATE_MATRIX_PROJECTION_OFFSET;
        else if (!getWordWithoutIndex(getWordBetweenDots(state,1),index).compare("mvp"))
            ret += STATE_MATRIX_MVP_OFFSET;
        else if (!getWordWithoutIndex(getWordBetweenDots(state,1),index).compare("texture"))
        {
            ret += STATE_MATRIX_TEXTURE_OFFSET;
            ret += index*4;
        }
        else if (!getWordWithoutIndex(getWordBetweenDots(state,1),index).compare("palette"))
        {       
            ret += STATE_MATRIX_PALETTE_OFFSET;
            ret += index*4;
        }
        else if (!getWordWithoutIndex(getWordBetweenDots(state,1),index).compare("program"))
        {       
            ret += STATE_MATRIX_PROGRAM_OFFSET;
            ret += index*4;
        }
        
        if (!getWordBetweenDots(state,2).compare("normal"))
            ret += STATE_MATRIX_NORMAL_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("inverse"))
            ret += STATE_MATRIX_INVERSE_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("transpose"))
            ret += STATE_MATRIX_TRANSPOSE_OFFSET;
        else if (!getWordBetweenDots(state,2).compare("invtrans"))
            ret += STATE_MATRIX_INVTRANS_OFFSET;
        
        if (!getWordWithoutIndex(getWordBetweenDots(state,3),rowIndex).compare("row")) ;
        else
            rowIndex = 0;
        
    }
    else if (!getWordWithoutIndex(getWordBetweenDots(state,0),index).compare("texenv"))
    {
        ret += BASE_TEXENV;
        ret += index;
    }
    else if (!getWordBetweenDots(state,0).compare("depth"))
    {
        ret += BASE_DEPTH;
        if (!getWordBetweenDots(state,1).compare("range"))
            ret += DEPTH_RANGE_OFFSET;
    }
    
    return ret;
}

IdentInfo *ACDXCodeGenTraverser::recursiveSearchSymbol(std::string id)
{
    IdentInfo *idinfo = symtab.searchSymbol(id);
    
    if (idinfo->type != ALIAS_REF)
    {
        return idinfo;    // Base case 1 of recursion. Symbol found 
    }
    else
    {
        return recursiveSearchSymbol(string(idinfo->alias_reference));
    }
}

ACDXCodeGenTraverser::ACDXCodeGenTraverser():
    fragmentInBank(MAX_VERTEX_ATTRIBS_ARB,"Attrib In Bank Id=0"),
    fragmentOutBank(VERTEX_RESULT_BANK_SIZE,"Attrib Out Bank Id=1"),
    parametersBank(MAX_PROGRAM_PARAMETERS_ARB,"Param Bank Id=2"),
    temporariesBank(MAX_PROGRAM_TEMPORARIES_ARB,"Temp Bank Id=3"),
    addressBank(MAX_PROGRAM_ADDRESS_REGISTERS_ARB,"Address Bank Id=4")
{
}

ACDXCodeGenTraverser::~ACDXCodeGenTraverser(void)
{
    list<ACDXGenericInstruction*>::iterator it = genericCode.begin();

    for ( ; it != genericCode.end(); it++ )
        delete *it;
    genericCode.clear();
    
    list<IdentInfo*>::iterator it2 = identinfoCollector.begin();
    while ( it2 != identinfoCollector.end() )
    {
         delete (*it2);
         it2++;                  
    }
    identinfoCollector.clear();
}

void ACDXCodeGenTraverser::dumpGenericCode(std::ostream& os)
{
    os << endl << fragmentInBank << endl;
    os << fragmentOutBank << endl;
    os << parametersBank << endl;
    os << temporariesBank << endl;
    os << addressBank << endl; 
    
    list<ACDXGenericInstruction*>::iterator it = genericCode.begin();

    for ( ; it != genericCode.end(); it++ )
    {
        (*it)->print(os);
    }
}

/**
 *  Visitor pattern methods
 *
 */

bool ACDXCodeGenTraverser::visitProgram(bool preVisitAction, IRProgram*, ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        symtab.pushBlock();
        postVisit = true;
        return true;
    }
    else // Post Visit
    {
        // Set last instruction flag to the last instruction of the code generated
        genericCode.back()->setLastInstruction();
        symtab.popBlock();
        return true;
    }
}

bool ACDXCodeGenTraverser::visitVP1Option(bool preVisitAction, VP1IROption *option , ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        if (!option->getOptionString().compare("ARB_position_invariant"))
        {   
            /**
             * Program Option: ARB_position_invariant
             */
        
            ACDXQuadReg<float> value(0.0f,0.0f,0.0f,0.0f);

            bool found;

            acd_uint position0 = parametersBank.getRegister(found,value,QR_GLSTATE,BASE_STATE_MATRIX+STATE_MATRIX_MVP_OFFSET,0);
            if (!found) 
            {
                parametersBank.set(position0,value);
                parametersBank.setType(position0,QR_GLSTATE,BASE_STATE_MATRIX+STATE_MATRIX_MVP_OFFSET,0);
            }
      
            acd_uint position1 = parametersBank.getRegister(found,value,QR_GLSTATE,BASE_STATE_MATRIX+STATE_MATRIX_MVP_OFFSET,1);
            if (!found)
            {
                parametersBank.set(position1,value);
                parametersBank.setType(position1,QR_GLSTATE,BASE_STATE_MATRIX+STATE_MATRIX_MVP_OFFSET,1);
            }

            acd_uint position2 = parametersBank.getRegister(found,value,QR_GLSTATE,BASE_STATE_MATRIX+STATE_MATRIX_MVP_OFFSET,2);
            if (!found)
            {
                parametersBank.set(position2,value);
                parametersBank.setType(position2,QR_GLSTATE,BASE_STATE_MATRIX+STATE_MATRIX_MVP_OFFSET,2);
            }

            acd_uint position3 = parametersBank.getRegister(found,value,QR_GLSTATE,BASE_STATE_MATRIX+STATE_MATRIX_MVP_OFFSET,3);
            if (!found)
            {
                parametersBank.set(position3,value);
                parametersBank.setType(position3,QR_GLSTATE,BASE_STATE_MATRIX+STATE_MATRIX_MVP_OFFSET,3);
            }
      
            /* Crear información operandos */

            op[0].arrayAccessModeFlag = true;
            op[0].arrayModeOffset = 0;
            op[0].idReg = position0;
            op[0].idBank = ACDXGenericInstruction::G_PARAM;
            op[0].negateFlag = false;
            op[0].relativeModeFlag = false;
            op[0].swizzleMode = XYZW;

            op[1].arrayAccessModeFlag = false;
            op[1].idReg = 0; /* Homogeneous space coordinates register */
            op[1].idBank = ACDXGenericInstruction::G_IN;
            op[1].negateFlag = false;
            op[1].relativeModeFlag = false;
            op[1].swizzleMode = XYZW;

            res.idBank = ACDXGenericInstruction::G_OUT;
            res.idReg = 0;
            res.writeMask = XNNN;

            genericCode.push_back(new ACDXGenericInstruction(0,"DP4", op[0], op[1], op[2], res, swz, false));
            op[0].arrayModeOffset = 1;
            op[0].idReg = position1;
            res.writeMask = NYNN;

             genericCode.push_back(new ACDXGenericInstruction(0,"DP4",op[0], op[1], op[2], res, swz, false));
            op[0].arrayModeOffset = 2;
            op[0].idReg = position2;
            res.writeMask = NNZN;
      
            genericCode.push_back(new ACDXGenericInstruction(0,"DP4",op[0], op[1], op[2], res, swz, false));
            op[0].arrayModeOffset = 3;
            op[0].idReg = position3;
            res.writeMask = NNNW;
      
            genericCode.push_back(new ACDXGenericInstruction(0,"DP4",op[0], op[1], op[2], res, swz, false));
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitFP1Option(bool , FP1IROption *, ACDXIRTraverser*)
{
    /**
     * ARB fragment program Options not implemented yet
     *
     */
    return true;
}

bool ACDXCodeGenTraverser::visitStatement(bool , IRStatement*, ACDXIRTraverser*)
{
    return true;
}

bool ACDXCodeGenTraverser::visitInstruction(bool preVisitAction, IRInstruction* instr, ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        if (instr->getIsFixedPoint())
            this->isFixedPoint = true;
        else
            this->isFixedPoint = false;

        /* This is the visitor order setup for instructions: previsit process - children - postvisit process */
        preVisit = true;
        postVisit = true;

        /* Code to do in code generation before visit the operands:
         * Set the first operand index
         */
        operandIndex = 0;
        return true;
    }
    else /* Post visit. All operands and destination children has been visited. 
          * Construct the generic instruction.
          */
    {
        genericCode.push_back(
            new ACDXGenericInstruction(instr->getLine(), instr->getOpcode().c_str(), op[0], op[1], op[2], res, swz, 
                                       isFixedPoint, texImageUnit, killSample, exportSample, texTarget));
        
        return true;
    }
}

bool ACDXCodeGenTraverser::visitKillInstruction(bool preVisitAction, IRKillInstruction* killinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, killinstr, it);
    if (preVisitAction)
    {
        if (killinstr->getIsKillSampleInstr())
        {
            this->killSample = killinstr->getKillSample();
        }
        return true;
    }
    else // PostVisit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitZExportInstruction(bool preVisitAction, IRZExportInstruction* zxpinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, zxpinstr, it);
    if (preVisitAction)
    {
        if (zxpinstr->getIsExpSampleInstr())
        {
            this->exportSample = zxpinstr->getExportSample();
        }
        return true;
    }
    else // PostVisit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitCHSInstruction(bool preVisitAction, IRCHSInstruction* chsinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, chsinstr, it);
    return true;
}

bool ACDXCodeGenTraverser::visitSampleInstruction(bool preVisitAction, IRSampleInstruction* smpinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, smpinstr, it);
    if (preVisitAction)
    {
        if (!smpinstr->getGLTextureTarget().compare("1D"))
            this->texTarget = ACDXGenericInstruction::TEXTURE_1D;
        else if (!smpinstr->getGLTextureTarget().compare("2D"))
            this->texTarget = ACDXGenericInstruction::TEXTURE_2D;
        else if (!smpinstr->getGLTextureTarget().compare("3D"))
            this->texTarget = ACDXGenericInstruction::TEXTURE_3D;
        else if (!smpinstr->getGLTextureTarget().compare("CUBE"))
            this->texTarget = ACDXGenericInstruction::CUBE_MAP;
        else if (!smpinstr->getGLTextureTarget().compare("RECT"))
            this->texTarget = ACDXGenericInstruction::RECT_MAP;
    
        this->texImageUnit = smpinstr->getTextureImageUnit();
        return true;
    }
    else // PostVisit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitSwizzleComponents(bool , IRSwizzleComponents*, ACDXIRTraverser*)
{
    return true;
}

bool ACDXCodeGenTraverser::visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction* swzinstr, ACDXIRTraverser* it)
{
    visitInstruction(preVisitAction, swzinstr, it);

    if (preVisitAction)
    {
        IRSwizzleComponents *swzComps = swzinstr->getSwizzleComponents();

        this->swz.xNegate = swzComps->getNegFlag(0);
        
        switch (swzComps->getComponent(0))
        {
            case IRSwizzleComponents::ONE:  this->swz.xSelect = ACDXGenericInstruction::ONE; break;
            case IRSwizzleComponents::ZERO: this->swz.xSelect = ACDXGenericInstruction::ZERO; break;
            case IRSwizzleComponents::X:    this->swz.xSelect = ACDXGenericInstruction::X; break;
            case IRSwizzleComponents::Y:    this->swz.xSelect = ACDXGenericInstruction::Y; break;
            case IRSwizzleComponents::Z:    this->swz.xSelect = ACDXGenericInstruction::Z; break;
            case IRSwizzleComponents::W:    this->swz.xSelect = ACDXGenericInstruction::W; break;
        }
    
        this->swz.yNegate = swzComps->getNegFlag(1);
        switch (swzComps->getComponent(1))
        {
            case IRSwizzleComponents::ONE:  this->swz.ySelect = ACDXGenericInstruction::ONE; break;
            case IRSwizzleComponents::ZERO: this->swz.ySelect = ACDXGenericInstruction::ZERO; break;
            case IRSwizzleComponents::X:    this->swz.ySelect = ACDXGenericInstruction::X; break;
            case IRSwizzleComponents::Y:    this->swz.ySelect = ACDXGenericInstruction::Y; break;
            case IRSwizzleComponents::Z:    this->swz.ySelect = ACDXGenericInstruction::Z; break;
            case IRSwizzleComponents::W:    this->swz.ySelect = ACDXGenericInstruction::W; break;
        }
    
        this->swz.zNegate = swzComps->getNegFlag(2);
        switch (swzComps->getComponent(2))
        {
            case IRSwizzleComponents::ONE:  this->swz.zSelect = ACDXGenericInstruction::ONE; break;
            case IRSwizzleComponents::ZERO: this->swz.zSelect = ACDXGenericInstruction::ZERO; break;
            case IRSwizzleComponents::X:    this->swz.zSelect = ACDXGenericInstruction::X; break;
            case IRSwizzleComponents::Y:    this->swz.zSelect = ACDXGenericInstruction::Y; break;
            case IRSwizzleComponents::Z:    this->swz.zSelect = ACDXGenericInstruction::Z; break;
            case IRSwizzleComponents::W:    this->swz.zSelect = ACDXGenericInstruction::W; break;
        }
    
        this->swz.wNegate = swzComps->getNegFlag(3);
        switch (swzComps->getComponent(3))
        {
            case IRSwizzleComponents::ONE:  this->swz.wSelect = ACDXGenericInstruction::ONE; break;
            case IRSwizzleComponents::ZERO: this->swz.wSelect = ACDXGenericInstruction::ZERO; break;
            case IRSwizzleComponents::X:    this->swz.wSelect = ACDXGenericInstruction::X; break;
            case IRSwizzleComponents::Y:    this->swz.wSelect = ACDXGenericInstruction::Y; break;
            case IRSwizzleComponents::Z:    this->swz.wSelect = ACDXGenericInstruction::Z; break;
            case IRSwizzleComponents::W:    this->swz.wSelect = ACDXGenericInstruction::W; break;
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitDstOperand(bool preVisitAction, IRDstOperand *dstop, ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        res.writeMask = (unsigned char)this->processWriteMask(dstop->getWriteMask().c_str());
    
        if (dstop->getIsVertexResultRegister())
        {
            res.idBank = ACDXGenericInstruction::G_OUT;
            res.idReg = vertexResultAttribBindingMaptoRegister(dstop->getDestination());
        }
        else if (dstop->getIsFragmentResultRegister())
        {
            res.idBank = ACDXGenericInstruction::G_OUT;
            res.idReg = fragmentResultAttribBindingMaptoRegister(dstop->getDestination());
        }
        else /* It is a temporary, address register or a result attribute named register (possibly aliased) */
        {
            IdentInfo *idinfo = recursiveSearchSymbol(dstop->getDestination());
            switch(idinfo->type)
            {
                case TEMP_REGISTER: res.idBank = ACDXGenericInstruction::G_TEMP; break;
                case FRAGMENT_RESULT_REG: res.idBank = ACDXGenericInstruction::G_OUT; break;
                case VERTEX_RESULT_REG: res.idBank = ACDXGenericInstruction::G_OUT; break;
                case ADDRESS_REG: res.idBank = ACDXGenericInstruction::G_ADDR; break;
            }
            res.idReg = idinfo->regId;
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitArrayAddressing(bool preVisitAction, IRArrayAddressing* arrayaddr, ACDXIRTraverser* )
{
    if (preVisitAction)
    {
        if (!arrayaddr->getIsRelativeAddress())
        {   // Absolute addressing
            op[operandIndex].arrayModeOffset = arrayaddr->getArrayAddressOffset();
            op[operandIndex].relativeModeFlag = false;
        }
        else // Relative addressing
        {
            op[operandIndex].relativeModeFlag = true;
            IdentInfo* idinfo = recursiveSearchSymbol(arrayaddr->getRelativeAddressReg());
            op[operandIndex].relModeAddrReg = idinfo->regId;
            op[operandIndex].relModeAddrComp = 0;
            op[operandIndex].relModeOffset = arrayaddr->getRelativeAddressOffset();
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitSrcOperand(bool preVisitAction, IRSrcOperand* srcop, ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        /* Prepare binding index to 0 for implicits parameter bindings in the source operand */
        paramBindingIndex = 0;
        return true;
    }
    else // Post Visit
    {
        op[operandIndex].swizzleMode = (unsigned char)this->processSwizzleSuffix(srcop->getSwizzleMask().c_str());
        op[operandIndex].negateFlag = srcop->getNegateFlag();

        if (srcop->getIsFragmentRegister())
        {
            op[operandIndex].idBank = ACDXGenericInstruction::G_IN;
            op[operandIndex].idReg = fragmentAttributeBindingMapToRegister(srcop->getSource());
            op[operandIndex].arrayAccessModeFlag = false;
        }
        else if (srcop->getIsVertexRegister())
        {
            op[operandIndex].idBank = ACDXGenericInstruction::G_IN;
            op[operandIndex].idReg = vertexAttributeBindingMapToRegister(srcop->getSource());
            op[operandIndex].arrayAccessModeFlag = false;
        }
        else if (srcop->getIsParamBinding()) /* It´s  a parameter implicit binding */
        {
            op[operandIndex].idBank = ACDXGenericInstruction::G_PARAM;
            op[operandIndex].idReg = paramBindingRegister[0];
            op[operandIndex].arrayAccessModeFlag = false;
        }
        else    /* It´s a parameter, temporary or input attribute named register (possibly aliased) */
        {
            IdentInfo *idinfo = recursiveSearchSymbol(srcop->getSource());
        
            if (idinfo->type == TEMP_REGISTER)
            {
                op[operandIndex].idBank = ACDXGenericInstruction::G_TEMP;
                op[operandIndex].idReg = idinfo->regId;
                op[operandIndex].arrayAccessModeFlag = false;
            }
            
            if (idinfo->type == FRAGMENT_ATTRIB_REG || idinfo->type == VERTEX_ATTRIB_REG)
            {
                op[operandIndex].idBank = ACDXGenericInstruction::G_IN;
                op[operandIndex].idReg = idinfo->regId;
                op[operandIndex].arrayAccessModeFlag = false;
            }
        
            if (idinfo->type == PROGRAM_PARAMETER)
            {
                op[operandIndex].idBank = ACDXGenericInstruction::G_PARAM;
                if (idinfo->parameter_array && !srcop->getArrayAddressing()->getIsRelativeAddress())
                { // Is parameter array with absolute addressing mode
                    op[operandIndex].idReg = idinfo->paramRegId[srcop->getArrayAddressing()->getArrayAddressOffset()];
                    op[operandIndex].arrayAccessModeFlag = true;                    
                }
                else if (idinfo->parameter_array) 
                { // Is parameter array with relative addressing mode
                    op[operandIndex].idReg = idinfo->paramRegId[0];
                    op[operandIndex].arrayAccessModeFlag = true;
                }
                else
                { // Is not parameter array
                    op[operandIndex].idReg = idinfo->paramRegId[0];
                    op[operandIndex].arrayAccessModeFlag = false;
                }
            }
        }
        operandIndex++;
        return true;
    }
}

bool ACDXCodeGenTraverser::visitNamingStatement(bool preVisitAction, IRNamingStatement* namestmnt, ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        /* This is the visitor order setup for naming statements: 
         * first node, second children and at last a node tratament.
         */
        preVisit = true;
        postVisit = true;

        IdentInfo *idinfo = new IdentInfo();
        identinfoCollector.push_back(idinfo);
        symtab.insertSymbol(namestmnt->getName(),idinfo);
        return true;
    }
    else // Post Visit
    {
        return true;
    }
}


bool ACDXCodeGenTraverser::visitALIASStatement(bool preVisitAction, IRALIASStatement* aliasstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction, aliasstmnt,it);
    if (preVisitAction)
    {
        IdentInfo *idinfo = symtab.searchSymbol(aliasstmnt->getName());
        idinfo->type = ALIAS_REF;
        idinfo->alias_reference = aliasstmnt->getAlias();
        return true;
    }
    else // Post Visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitTEMPStatement(bool preVisitAction, IRTEMPStatement* tmpstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction, tmpstmnt,it);
    if (preVisitAction)
    {
        // We reserve a register in temporaries bank for each temporary variable

        IdentInfo *idinfo;
        ACDXQuadReg<float> tempReg(0.0,0.0,0.0,0.0);

        idinfo = symtab.searchSymbol(tmpstmnt->getName());
        idinfo->type = TEMP_REGISTER;

        idinfo->regId = temporariesBank.findFree();
        temporariesBank.set(idinfo->regId,tempReg);

        list<string>* temporariesList = tmpstmnt->getOtherTemporaries();
        list<string>::iterator iter = temporariesList->begin();

        while(iter != temporariesList->end()) 
        {
            idinfo = new IdentInfo();
            identinfoCollector.push_back(idinfo);
            idinfo->type = TEMP_REGISTER;
            idinfo->regId = temporariesBank.findFree();
            temporariesBank.set(idinfo->regId,tempReg);
            symtab.insertSymbol((*iter),idinfo);
            iter++;
        }
        return true;
    }
    else // Post Visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitADDRESSStatement(bool preVisitAction,IRADDRESSStatement *addrstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction, addrstmnt,it);
    
    if (preVisitAction)
    {
        // We reserve a register in address bank

        IdentInfo *idinfo;
        ACDXQuadReg<float> addrReg(0.0,0.0,0.0,0.0);

        idinfo = symtab.searchSymbol(addrstmnt->getName());
        idinfo->type = ADDRESS_REG;

        idinfo->regId = addressBank.findFree();
        addressBank.set(idinfo->regId,addrReg);

        list<string>* addressRegistersList = addrstmnt->getOtherAddressRegisters();
        list<string>::iterator iter = addressRegistersList->begin();

        while(iter != addressRegistersList->end()) 
        {
            idinfo = new IdentInfo();
            identinfoCollector.push_back(idinfo);
            idinfo->type = ADDRESS_REG;
            idinfo->regId = addressBank.findFree();
            addressBank.set(idinfo->regId,addrReg);
            symtab.insertSymbol((*iter),idinfo);
            iter++;
        }
        return true;
    }
    else // Post Visit
    {
        return true;
    }   
}

bool ACDXCodeGenTraverser::visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement* attribstmnt, ACDXIRTraverser*it)
{
    visitNamingStatement(preVisitAction, attribstmnt,it);
    if (preVisitAction)
    {
        IdentInfo *idinfo = symtab.searchSymbol(attribstmnt->getName());
        idinfo->type = VERTEX_ATTRIB_REG;
        idinfo->regId = vertexAttributeBindingMapToRegister(attribstmnt->getInputAttribute());
        return true;
    }
    else // Post Visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement* attribstmnt, ACDXIRTraverser*it)
{
    visitNamingStatement(preVisitAction, attribstmnt,it);
    if (preVisitAction)
    {
        IdentInfo *idinfo = symtab.searchSymbol(attribstmnt->getName());
        idinfo->type = FRAGMENT_ATTRIB_REG;
        idinfo->regId = fragmentAttributeBindingMapToRegister(attribstmnt->getInputAttribute());
        return true;
    }
    else // Post Visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement* outputstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction, outputstmnt,it);
    if (preVisitAction)
    {
        IdentInfo *idinfo = symtab.searchSymbol(outputstmnt->getName());
        idinfo->type = VERTEX_RESULT_REG;
        idinfo->regId = vertexResultAttribBindingMaptoRegister(outputstmnt->getOutputAttribute());
        return true;
    }
    else // PostVisit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement* outputstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction, outputstmnt,it);
    if (preVisitAction)
    {
        IdentInfo *idinfo = symtab.searchSymbol(outputstmnt->getName());
        idinfo->type = FRAGMENT_RESULT_REG;
        idinfo->regId = fragmentResultAttribBindingMaptoRegister(outputstmnt->getOutputAttribute());
        return true;
    }
    else // PostVisit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitPARAMStatement(bool preVisitAction, IRPARAMStatement*paramstmnt, ACDXIRTraverser* it)
{
    visitNamingStatement(preVisitAction, paramstmnt,it);
    if (preVisitAction)
    {
        IdentInfo *idinfo = symtab.searchSymbol(paramstmnt->getName());
        idinfo->type = PROGRAM_PARAMETER;
        if (paramstmnt->getIsMultipleBindings())    // Array vector parameter
        {
            idinfo->parameter_array = true;
        }
        else
        {
            idinfo->parameter_array = false;
        }
        idinfo->implicit_binding = false;
        paramBindingIndex = 0;
        return true;
    }
    else // Post Visit
    {
        IdentInfo *idinfo = symtab.searchSymbol(paramstmnt->getName());
        if (idinfo->parameter_array)
        {
            for (unsigned int i=0;i<paramBindingIndex;i++)
            idinfo->paramRegId[i] = paramBindingRegister[i];
        }
        else
        {
            idinfo->paramRegId[0] = paramBindingRegister[0];
        }
        return true;
    }
}

bool ACDXCodeGenTraverser::visitParamBinding(bool , IRParamBinding* , ACDXIRTraverser*)
{
    return true;
}

bool ACDXCodeGenTraverser::visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding* localenvbind, ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        ACDXQuadReg<float> param(0.0f,0.0f,0.0f,0.0f);
        bool found;

        for (unsigned int i=0;i<localenvbind->getVectorsBinded();i++)
        {
            if (localenvbind->getType() == IRLocalEnvBinding::LOCALPARAM)
            {
                registerPosition = 
                    parametersBank.getRegister(found,param,QR_PARAM_LOCAL,localenvbind->getMinIndex()+i,0);
                if (!found)
                {
                    parametersBank.set(registerPosition,param);
                    parametersBank.setType(registerPosition,QR_PARAM_LOCAL,localenvbind->getMinIndex()+i,0);
                }
            }
            else
            {
                registerPosition = 
                    parametersBank.getRegister(found,param,QR_PARAM_ENV,localenvbind->getMinIndex()+i,0);
                if (!found)
                {
                    parametersBank.set(registerPosition,param);
                    parametersBank.setType(registerPosition,QR_PARAM_ENV,localenvbind->getMinIndex()+i,0);
                }
            }
            paramBindingRegister[paramBindingIndex+i] = registerPosition;
        }
        paramBindingIndex += localenvbind->getVectorsBinded();
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitConstantBinding(bool preVisitAction, IRConstantBinding* constbind, ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        ACDXQuadReg<float> constant(constbind->getValue1(),constbind->getValue2(),constbind->getValue3(),constbind->getValue4());
        bool found;

        for (unsigned int i=0;i<constbind->getVectorsBinded();i++)
        {
            registerPosition = 
                parametersBank.getRegister(found,constant,QR_CONSTANT,0,0);

            // Note: in relative adress parameter access the values have to be consecutive      
            if (!found)
            {
                parametersBank.set(registerPosition,constant);
                parametersBank.setType(registerPosition,QR_CONSTANT,0,0);
            }
            paramBindingRegister[paramBindingIndex+i] = registerPosition;
        }
  
        paramBindingIndex += constbind->getVectorsBinded();
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool ACDXCodeGenTraverser::visitStateBinding(bool preVisitAction, IRStateBinding* statebind, ACDXIRTraverser*)
{
    if (preVisitAction)
    {
        ACDXQuadReg<float>  value(0.0f,0.0f,0.0f,0.0f);
        
        unsigned int rowIndex;
        
        unsigned int stateIdentifier = paramStateBindingIdentifier(statebind->getState(),rowIndex);
    
        bool found;

        for (unsigned int i=0;i<statebind->getVectorsBinded();i++)
        {
            registerPosition = 
                parametersBank.getRegister(found,value,QR_GLSTATE,stateIdentifier,i);
    
            // Note: in relative adress parameter access the values have to be consecutive      
            if (!found)
            {
                parametersBank.set(registerPosition,value);
                parametersBank.setType(registerPosition,QR_GLSTATE,stateIdentifier,i);
            }
            paramBindingRegister[paramBindingIndex+i] = registerPosition + rowIndex;
        }

        paramBindingIndex += statebind->getVectorsBinded();
        return true;
    }
    else // Post visit
    {
        return true;
    }
}


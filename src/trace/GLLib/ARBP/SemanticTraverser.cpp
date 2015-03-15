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

#include "SemanticTraverser.h"
#include "ImplementationLimits.h"
#include "CodeGenTraverser.h"
#include "GPU.h"

using namespace libgl::GenerationCode;
using namespace libgl;
using namespace std;
using namespace gpu3d;


bool SemanticTraverser::swizzleMaskCheck(const char *smask)
{
    int len = strlen(smask);
    if (len != 4 && len != 1) return false;
    for (int i=0;i<len;i++)
    {
        if (smask[i]!='x' && smask[i]!='y' && smask[i]!='z' && smask[i]!='w' &&
            smask[i]!='r' && smask[i]!='g' && smask[i]!='b' && smask[i]!='a')
            return false;
    }
    return true;
}

bool SemanticTraverser::writeMaskCheck(const char *wmask)
{
    int len = strlen(wmask);
    int actual_weight = 0;
    if (len > 4) return false;
    for (int i=0;i<len;i++)
    {
        switch(wmask[i])
        {
            case 'x': if (actual_weight < 1) actual_weight = 1; else return false; break;
            case 'y': if (actual_weight < 2) actual_weight = 2; else return false; break;
            case 'z': if (actual_weight < 3) actual_weight = 3; else return false; break;
            case 'w': if (actual_weight < 4) actual_weight = 4; else return false; break;
            case 'r': if (actual_weight < 1) actual_weight = 1; else return false; break;
            case 'g': if (actual_weight < 2) actual_weight = 2; else return false; break;
            case 'b': if (actual_weight < 3) actual_weight = 3; else return false; break;
            case 'a': if (actual_weight < 4) actual_weight = 4; else return false; break;
        }
    }
    return true;
}

IdentInfo *SemanticTraverser::recursiveSearchSymbol(std::string id, bool& found)
{
    bool foundRecursive;
    
    IdentInfo *idinfo = symtab.searchSymbol(id,foundRecursive);
    
    if (foundRecursive)
    {
        found = true;
        if (idinfo->type != ALIAS_REF)
        {
            return idinfo;  // Base case 1 of recursion. Symbol found 
        }
        else
        {
            return recursiveSearchSymbol(idinfo->alias_reference,found);
        }
    }
    else
    {
        found = false;  // Base case 2 of recursion. Symbol not found
        return 0;
    }
}

SemanticTraverser::SemanticTraverser()
:   semanticErrorInTraverse(false)
{
    inputRegisterRead.reset();
    outputRegisterWritten.reset();
    
    for (int i=0; i< MAX_TEXTURE_UNITS_ARB; i++)
        TextureImageUnitsTargets[i] = GL_NONE;
}

SemanticTraverser::~SemanticTraverser(void)
{
     std::list<IdentInfo*>::iterator it = identinfoCollector.begin();
     while ( it != identinfoCollector.end() )
     {
          delete (*it);
          it++;                  
     }
     identinfoCollector.clear();
}

SemanticTraverser::AttribMask SemanticTraverser::getAttribsWritten() const
{
    return outputRegisterWritten;
}

SemanticTraverser::AttribMask SemanticTraverser::getAttribsRead() const
{
    return inputRegisterRead;
}

/**
 * Visitor pattern implemented methods
 *
 */

bool SemanticTraverser::visitProgram(bool preVisitAction, IRProgram* program, IRTraverser*)
{
    if (preVisitAction)
    {
        // Visitor order setup for semantic traverser
        preVisit = true;
        postVisit = false;

        // Initialization of error string and statistics
        errorOut << "Semantical Errors String:" << endl;
        numberOfStatements = 0;
        numberOfInstructions = 0;
        numberOfParamRegs = 0;
        numberOfTempRegs = 0;
        numberOfAttribs = 0;
        numberOfAddrRegs = 0;
        symtab.pushBlock();
        
        return true;
    }
    else // Post Visit
    {
        symtab.popBlock();
        return true;
    }
}

bool SemanticTraverser::visitVP1Option(bool preVisitAction, VP1IROption *option , IRTraverser*)
{
    if (preVisitAction)
    {
        if (!option->getOptionString().compare("ARB_position_invariant")) 
        {
            outputRegisterWritten.set(POSITION_ATTRIBUTE);
            return true;
        }

        errorOut << "Line: " << option->getLine();
        errorOut << " Option parameter " << option->getOptionString();
        errorOut << " not supported. Remember that language is case sensitive" << endl;

        semanticErrorInTraverse = true;

        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitFP1Option(bool preVisitAction, FP1IROption *option , IRTraverser*)
{
    if (preVisitAction)
    {
        if (!option->getOptionString().compare("ARB_fog_exp")) return true;
        else if (!option->getOptionString().compare("ARB_fog_exp2")) return true;
        else if (!option->getOptionString().compare("ARB_fog_linear")) return true;
        else if (!option->getOptionString().compare("ARB_precision_hint_fastest")) return true;
        else if (!option->getOptionString().compare("ARB_precision_hint_nicest")) return true;
    
        errorOut << "Line: " << option->getLine();
        errorOut << " Option parameter " << option->getOptionString();
        errorOut << " not supported. Remember that language is case sensitive" << endl;
        semanticErrorInTraverse = true;

        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitStatement(bool preVisitAction, IRStatement*, IRTraverser*)
{
    if (preVisitAction)
    {
        numberOfStatements++;
        return true;
    }
    else
    {
        return true;
    }
}

bool SemanticTraverser::visitInstruction(bool preVisitAction, IRInstruction*, IRTraverser*)
{
    if (preVisitAction)
    {
        numberOfInstructions++;
        return true;
    }
    else
    {
        return true;
    }
}

bool SemanticTraverser::visitSampleInstruction(bool preVisitAction, IRSampleInstruction* sample, IRTraverser* it)
{
    visitInstruction(preVisitAction,sample,it);
    
    GLenum target;
    
    if (!sample->getGLTextureTarget().compare("1D"))
        target = GL_TEXTURE_1D;
        
    else if (!sample->getGLTextureTarget().compare("2D"))
        target = GL_TEXTURE_2D;
        
    else if (!sample->getGLTextureTarget().compare("3D"))
        target = GL_TEXTURE_3D;
    
    else if (!sample->getGLTextureTarget().compare("CUBE"))
        target = GL_TEXTURE_CUBE_MAP;
        
    else if (!sample->getGLTextureTarget().compare("RECT"))
    {
        errorOut << "Line: " << sample->getLine();
        errorOut << " Texture Target RECT not supported by this implementation for now" << endl;
        
        semanticErrorInTraverse = true;

        return true;
    }
    
    if ( (TextureImageUnitsTargets[sample->getTextureImageUnit()] != GL_NONE) && 
         (TextureImageUnitsTargets[sample->getTextureImageUnit()] != target) )
    {
        errorOut << "Line: " << sample->getLine();
        errorOut << " Programs are not allowed to reference different texture targets in the same texture unit" << endl;

        semanticErrorInTraverse = true;
    }
    else
        TextureImageUnitsTargets[sample->getTextureImageUnit()] = target;
    
    return true;
}

bool SemanticTraverser::visitSwizzleComponents(bool preVisitAction, IRSwizzleComponents*, IRTraverser*)
{
    return true;
}

bool SemanticTraverser::visitSwizzleInstruction(bool preVisitAction, IRSwizzleInstruction* swzinstr, IRTraverser* it)
{
    visitInstruction(preVisitAction,swzinstr,it);
    return true;
}

bool SemanticTraverser::visitDstOperand(bool preVisitAction, IRDstOperand *dstop, IRTraverser*)
{
    if (preVisitAction)
    {
        if (dstop->getIsVertexResultRegister())
        {
            outputRegisterWritten.set(CodeGenTraverser::vertexResultAttribBindingMaptoRegister(dstop->getDestination()));
        }
        else if (dstop->getIsFragmentResultRegister()) 
        {
            outputRegisterWritten.set(CodeGenTraverser::fragmentResultAttribBindingMaptoRegister(dstop->getDestination()));
        }
        else // It has to be a temporary register. Possibly aliased.
        {
            bool found;
            IdentInfo *idinfo = recursiveSearchSymbol(dstop->getDestination(),found);
            if (!found)
            {   // Undefined identifier
                errorOut << "Line: " << dstop->getLine();
                errorOut << " Identifier " << dstop->getDestination() << " not declared previously" << endl;
                semanticErrorInTraverse = true;
            }
            else
            {   // Defined register identifier that cannot be written.
                if (idinfo->type == VERTEX_ATTRIB_REG ||
                    idinfo->type == FRAGMENT_ATTRIB_REG ||
                    idinfo->type == PROGRAM_PARAMETER)
                {
                    errorOut << "Line: " << dstop->getLine();
                    errorOut << " Program parameter variables and fragment or vertex attributes"
                                " variables can never be modified" << endl;
                    semanticErrorInTraverse = true;
                }
                else if (idinfo->type == VERTEX_RESULT_REG || 
                         idinfo->type == FRAGMENT_RESULT_REG)
                {
                    outputRegisterWritten.set(idinfo->regId);
                }
            }
        }
        
        // Check Swizzle mask
        if (!writeMaskCheck(dstop->getWriteMask().c_str()))
        {
            errorOut << "Line: " << dstop->getLine();
            errorOut << " Incorrect write mask \"" << dstop->getWriteMask() << "\" in result register" << endl;
            semanticErrorInTraverse = true;
        }
    
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitArrayAddressing(bool preVisitAction,IRArrayAddressing* arrayaddr, IRTraverser *it)
{
    if (preVisitAction)
    {
        if (arrayaddr->getIsRelativeAddress())
        {
            bool found;
            IdentInfo *idinfo = recursiveSearchSymbol(arrayaddr->getRelativeAddressReg(),found);
            if (!found)
            {
                errorOut << "Line: " << arrayaddr->getLine();
                errorOut << " Identifier " << arrayaddr->getRelativeAddressReg() << " not declared previously" << endl;
                semanticErrorInTraverse = true;
            }
            else // identifier found
            {   
                if (idinfo->type != ADDRESS_REG)
                {   // Addressing with not an address register????
                    errorOut << "Line: " << arrayaddr->getLine();
                    errorOut << " Symbol " << arrayaddr->getRelativeAddressReg() << " is not an address register" << endl;
                    semanticErrorInTraverse = true;
                }
            }
            // Check swizzle scalar component 
            if (arrayaddr->getRelativeAddressComp().compare("x"))
            {
                errorOut << "Line: " << arrayaddr->getLine();
                errorOut << " No valid address register component specified " << arrayaddr->getRelativeAddressReg() << endl;
                semanticErrorInTraverse = true;
            }
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitSrcOperand(bool preVisitAction, IRSrcOperand* srcop, IRTraverser*)
{
    if (preVisitAction)
    {
        if (!srcop->getIsVertexRegister() && 
            !srcop->getIsFragmentRegister() &&
            !srcop->getIsParamBinding())
        {
            /* It is just an identifier name. Search it */
            bool found;
            IdentInfo *idinfo = recursiveSearchSymbol(srcop->getSource(),found);
            if (!found)
            {
                errorOut << "Line: " << srcop->getLine();
                errorOut << " Identifier " << srcop->getSource() << " not declared previously" << endl;
                semanticErrorInTraverse = true;
            }
            /* Identifier is an output register */
            else if (idinfo->type == VERTEX_RESULT_REG || idinfo->type == FRAGMENT_RESULT_REG) 
            {
                errorOut << "Line: " << srcop->getLine();
                errorOut << " Vertex or Fragment result variables can be never read" << endl;
                semanticErrorInTraverse = true;
            }
            /* Identifier is not a program parameter and it is array accessed */
            else if (idinfo->type != PROGRAM_PARAMETER && srcop->getIsArrayAddressing())
            {
                // Identifier is not a program parameter
                errorOut << "Line: " << srcop->getLine();
                errorOut << " Identifier " << srcop->getSource();
                errorOut << " is not a program parameter array" << endl;
                semanticErrorInTraverse = true;
            }
            /* Identifier is a program parameter but is not a array and it is array accessed */
            else if (idinfo->type == PROGRAM_PARAMETER && !idinfo->parameter_array && srcop->getIsArrayAddressing())
            {
                errorOut << "Line: " << srcop->getLine();
                errorOut << " Identifier " << srcop->getSource();
                errorOut << " is not a program parameter array" << endl;
                semanticErrorInTraverse = true;
            }
            /* Identifier is a program parameter array but itïs not array accessed */
            else if (idinfo->type == PROGRAM_PARAMETER && idinfo->parameter_array && !srcop->getIsArrayAddressing())
            {
                // Identifier is a program parameter array but is not array accessed
                errorOut << "Line: " << srcop->getLine();
                errorOut << " Missing row number in parameter " << srcop->getSource();
                errorOut << " array access" << endl;
                semanticErrorInTraverse = true;
            }
            else // All correct
            {
                if (idinfo->type == VERTEX_ATTRIB_REG || 
                    idinfo->type == FRAGMENT_ATTRIB_REG)
                {
                    inputRegisterRead.set(idinfo->regId);
                }
            }    
            
        }
        
        if (srcop->getIsVertexRegister())
        {
            inputRegisterRead.set(CodeGenTraverser::vertexAttributeBindingMapToRegister(srcop->getSource()));
        }
        else if (srcop->getIsFragmentRegister())
        {
            inputRegisterRead.set(CodeGenTraverser::fragmentAttributeBindingMapToRegister(srcop->getSource()));
        }
        
        // Check Swizzle mask
        if (!swizzleMaskCheck(srcop->getSwizzleMask().c_str()))
        {
            errorOut << "Line: " << srcop->getLine();
            errorOut << " Incorrect swizzle mask \"" << srcop->getSwizzleMask()<< "\" in operand" << endl;
            semanticErrorInTraverse = true;
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitNamingStatement(bool preVisitAction, IRNamingStatement* namestmnt, IRTraverser*)
{
    if (preVisitAction)
    {
        bool found;
        symtab.searchSymbol(namestmnt->getName(),found);
        if (found)
        {
            // Check not using an already declared identifier
            errorOut << "Line: " << namestmnt->getLine();
            errorOut << " Identifier " << namestmnt->getName() << " already declared" << endl;
            semanticErrorInTraverse = true;
            return false;
        }
        else
        {   // Insert in symbol table. The specialized naming statement will
            // rescue this struct and will fill it with specialized naming information.
            IdentInfo *idinfo = new IdentInfo();
            identinfoCollector.push_back(idinfo);
            symtab.insertSymbol(namestmnt->getName(),idinfo);
        }
        return true;
    }
    else // Post Visit
    {
        return true;
    }
}

bool SemanticTraverser::visitALIASStatement(bool preVisitAction, IRALIASStatement* aliasstmnt, IRTraverser* it)
{
    bool correctNameBinding = visitNamingStatement(preVisitAction,aliasstmnt,it);
    if (preVisitAction)
    {
        bool found;
        IdentInfo *idinfo2 = recursiveSearchSymbol(aliasstmnt->getAlias(),found);
        if (!found)
        {
            errorOut << "Line: " << aliasstmnt->getLine();
            errorOut << " Identifier " << aliasstmnt->getAlias() << " not declared previously" << endl;
            semanticErrorInTraverse = true;
        }
        else // found a valid root identifier that is not an alias (If more than one alias chained). 
             // All is correct.
        {
            if (correctNameBinding) // Check if Name binding correct.
            {
                IdentInfo *idinfo = symtab.searchSymbol(aliasstmnt->getName());
                idinfo->type = ALIAS_REF;
                idinfo->alias_reference = aliasstmnt->getAlias();
            }
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitTEMPStatement(bool preVisitAction, IRTEMPStatement* tmpstmnt, IRTraverser* it)
{
    bool correctNameBinding = visitNamingStatement(preVisitAction,tmpstmnt,it);
    if (preVisitAction)
    {
        if (correctNameBinding)
        {   // Rescue Identifier info and set type
            IdentInfo* idinfo = symtab.searchSymbol(tmpstmnt->getName());
            idinfo->type = TEMP_REGISTER;
        }

        numberOfTempRegs += tmpstmnt->getOtherTemporaries()->size() + 1;

        // Check the auxiliar list of temporaries declared

        list<string>* temporariesList = tmpstmnt->getOtherTemporaries();

        list<string>::iterator iter = temporariesList->begin();
        
        bool found;
        
        while(iter != temporariesList->end()) 
        {
            symtab.searchSymbol((*iter),found);
            if (found)
            {
                errorOut << "Line: " << tmpstmnt->getLine();
                errorOut << " Identifier " << (*iter) << "already declared" << endl;
                semanticErrorInTraverse = true;
            }       
            else // Put new identifier in Symbol Table
            {
                IdentInfo* idinfo = new IdentInfo();
                identinfoCollector.push_back(idinfo);
                idinfo->type = TEMP_REGISTER;
                symtab.insertSymbol((*iter),idinfo);
            }
            iter++;
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitADDRESSStatement(bool preVisitAction, IRADDRESSStatement* addrstmnt, IRTraverser* it)
{
    bool correctNameBinding = visitNamingStatement(preVisitAction,addrstmnt,it);
    
    if (preVisitAction)
    {
        // Rescue Identifier info and set type
        if (correctNameBinding)
        {
            IdentInfo* idinfo = symtab.searchSymbol(addrstmnt->getName());
            idinfo->type = ADDRESS_REG;
        }

        // Check auxiliar list of address registers declared
        
        list<string>* addressRegisterList = addrstmnt->getOtherAddressRegisters();

        list<string>::iterator iter = addressRegisterList->begin();
        
        bool found;

        while(iter != addressRegisterList->end()) 
        {
            symtab.searchSymbol((*iter),found);
            if (found)
            {
                errorOut << "Line: " << addrstmnt->getLine();
                errorOut << " Identifier " << (*iter) << "already declared" << endl;
                semanticErrorInTraverse = true;
            }   
            else // Put new identifier in Symbol Table
            {
                IdentInfo* idinfo = new IdentInfo();
                identinfoCollector.push_back(idinfo);
                idinfo->type = ADDRESS_REG;
                symtab.insertSymbol((*iter),idinfo);
            }
            iter++;
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitVP1ATTRIBStatement(bool preVisitAction, VP1IRATTRIBStatement* attrstmnt, IRTraverser* it)
{
    numberOfAttribs++;
    bool correctNameBinding = visitNamingStatement(preVisitAction,attrstmnt,it);
    if (preVisitAction)
    {
        if (correctNameBinding)
        {
            IdentInfo *idinfo = symtab.searchSymbol(attrstmnt->getName());
            idinfo->regId = CodeGenTraverser::vertexAttributeBindingMapToRegister(attrstmnt->getInputAttribute());
            idinfo->type = VERTEX_ATTRIB_REG;
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitFP1ATTRIBStatement(bool preVisitAction, FP1IRATTRIBStatement* attrstmnt, IRTraverser* it)
{
    numberOfAttribs++;
    bool correctNameBinding = visitNamingStatement(preVisitAction,attrstmnt,it);
    if (preVisitAction)
    {
        if (correctNameBinding)
        {
            IdentInfo *idinfo = symtab.searchSymbol(attrstmnt->getName());
            idinfo->regId = CodeGenTraverser::fragmentAttributeBindingMapToRegister(attrstmnt->getInputAttribute());
            idinfo->type = FRAGMENT_ATTRIB_REG;
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitVP1OUTPUTStatement(bool preVisitAction, VP1IROUTPUTStatement* outputstmnt, IRTraverser* it)
{
    bool correctNameBinding = visitNamingStatement(preVisitAction,outputstmnt,it);
    if (preVisitAction)
    {
        if (correctNameBinding)
        {
            IdentInfo *idinfo = symtab.searchSymbol(outputstmnt->getName());
            idinfo->type = VERTEX_RESULT_REG;
            idinfo->regId = CodeGenTraverser::vertexResultAttribBindingMaptoRegister(outputstmnt->getOutputAttribute());
        }
        return true;
    }
    else
    {
        return true;
    }
}

bool SemanticTraverser::visitFP1OUTPUTStatement(bool preVisitAction, FP1IROUTPUTStatement* outputstmnt, IRTraverser* it)
{
    bool correctNameBinding = visitNamingStatement(preVisitAction,outputstmnt,it);
    if (preVisitAction)
    {
        if (correctNameBinding)
        {
            IdentInfo *idinfo = symtab.searchSymbol(outputstmnt->getName());
            idinfo->type = FRAGMENT_RESULT_REG;
            idinfo->regId = CodeGenTraverser::fragmentResultAttribBindingMaptoRegister(outputstmnt->getOutputAttribute());
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitPARAMStatement(bool preVisitAction, IRPARAMStatement* paramstmnt, IRTraverser* it)
{
    bool correctNameBinding = visitNamingStatement(preVisitAction,paramstmnt,it);
    if (preVisitAction)
    {
        if (correctNameBinding)
        {
            IdentInfo* idinfo = symtab.searchSymbol(paramstmnt->getName());
            idinfo->type = PROGRAM_PARAMETER;
            idinfo->implicit_binding = false;
        
            if (paramstmnt->getIsMultipleBindings())
                idinfo->parameter_array = true;
            else
                idinfo->parameter_array = false;
        }

        if (paramstmnt->getIsMultipleBindings())    // Array vector parameter
        {
            // Check if specified size match with list size
            unsigned int totalVectorsBinded = 0;
        
            list<IRParamBinding*>* paramList =  paramstmnt->getParamBindings();
            list<IRParamBinding*>::iterator iter = paramList->begin();

            while(iter!= paramList->end())
            {
                totalVectorsBinded += (*iter)->getVectorsBinded();
                iter++;
            }

            if (paramstmnt->getSize() > 0) // An array size was specificated
            {
                if (totalVectorsBinded != paramstmnt->getSize())
                {
                    errorOut << "Line: " << paramstmnt->getLine();
                    errorOut << " The optional size array does not match the number of parameter bindings in the initialization list" << endl;
                    semanticErrorInTraverse = true;
                }
            }
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitParamBinding(bool preVisitAction, IRParamBinding*, IRTraverser*)
{
    return true;
}

bool SemanticTraverser::visitLocalEnvBinding(bool preVisitAction, IRLocalEnvBinding* localenvbind, IRTraverser*)
{
    if (preVisitAction)
    {
        if (localenvbind->getMinIndex() > localenvbind->getMaxIndex())
        {
            errorOut << "Line: " << localenvbind->getLine();
            errorOut << " Invalid range in multiple array parameter binding" << endl;
            semanticErrorInTraverse = true;
        }
        return true;
    }
    else // Post visit
    {
        return true;
    }
}

bool SemanticTraverser::visitConstantBinding(bool preVisitAction, IRConstantBinding*, IRTraverser*)
{
    return true;
}

bool SemanticTraverser::visitStateBinding(bool preVisitAction, IRStateBinding*, IRTraverser*)
{
    return true;
}

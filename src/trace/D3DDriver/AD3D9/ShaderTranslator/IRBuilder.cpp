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

#include "IRBuilder.h"

//  InstructionsSyntax constructor.
InstructionsSyntax::InstructionsSyntax()
{
    //  Initialize the syntactic information for all the shader opcodes.
    syntax[D3DSIO_NOP]          = InstructionSyntax(0, 0);
    syntax[D3DSIO_MOV]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_ADD]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_SUB]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_MAD]          = InstructionSyntax(1, 3);
    syntax[D3DSIO_MUL]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_RCP]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_RSQ]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_DP3]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_DP4]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_MIN]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_MAX]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_SLT]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_SGE]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_EXP]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_LOG]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_LIT]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_DST]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_LRP]          = InstructionSyntax(1, 3);
    syntax[D3DSIO_FRC]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_M4x4]         = InstructionSyntax(1, 2);
    syntax[D3DSIO_M4x3]         = InstructionSyntax(1, 2);
    syntax[D3DSIO_M3x4]         = InstructionSyntax(1, 2);
    syntax[D3DSIO_M3x3]         = InstructionSyntax(1, 2);
    syntax[D3DSIO_M3x2]         = InstructionSyntax(1, 2);
    syntax[D3DSIO_CALL]         = InstructionSyntax(0, 1);
    syntax[D3DSIO_CALLNZ]       = InstructionSyntax(0, 2);
    syntax[D3DSIO_LOOP]         = InstructionSyntax(0, 2);
    syntax[D3DSIO_RET]          = InstructionSyntax(0, 0);
    syntax[D3DSIO_ENDLOOP]      = InstructionSyntax(0, 0);
    syntax[D3DSIO_LABEL]        = InstructionSyntax(0, 1);
    syntax[D3DSIO_DCL]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_POW]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_CRS]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_SGN]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_ABS]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_NRM]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_SINCOS]       = InstructionSyntax(0, 0);
    syntax[D3DSIO_REP]          = InstructionSyntax(0, 1);
    syntax[D3DSIO_ENDREP]       = InstructionSyntax(0, 0);
    syntax[D3DSIO_IF]           = InstructionSyntax(0, 1);
    syntax[D3DSIO_IFC]          = InstructionSyntax(0, 2);
    syntax[D3DSIO_ELSE]         = InstructionSyntax(0, 0);
    syntax[D3DSIO_ENDIF]        = InstructionSyntax(0, 0);
    syntax[D3DSIO_BREAK]        = InstructionSyntax(0, 0);
    syntax[D3DSIO_BREAKC]       = InstructionSyntax(0, 2);
    syntax[D3DSIO_MOVA]         = InstructionSyntax(1, 1);
    syntax[D3DSIO_DEFB]         = InstructionSyntax(1, 1);
    syntax[D3DSIO_DEFI]         = InstructionSyntax(1, 4);
    syntax[D3DSIO_TEXCOORD]     = InstructionSyntax(0, 0);
    syntax[D3DSIO_TEX]          = InstructionSyntax(0, 0);
    syntax[D3DSIO_TEXKILL]      = InstructionSyntax(0, 1);
    syntax[D3DSIO_TEXBEM]       = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXBEML]      = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXREG2AR]    = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXREG2GB]    = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXM3x2PAD]   = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXM3x2TEX]   = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXM3x3PAD]   = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXM3x3TEX]   = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXM3x3SPEC]  = InstructionSyntax(1, 2);
    syntax[D3DSIO_TEXM3x3VSPEC] = InstructionSyntax(1, 1);
    syntax[D3DSIO_EXPP]         = InstructionSyntax(1, 1);
    syntax[D3DSIO_LOGP]         = InstructionSyntax(1, 1);
    syntax[D3DSIO_CND]          = InstructionSyntax(1, 3);
    syntax[D3DSIO_DEF]          = InstructionSyntax(1, 4);
    syntax[D3DSIO_TEXREG2RGB]   = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXDP3TEX]    = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXM3x2DEPTH] = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXDP3]       = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXM3x3]      = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXDEPTH]     = InstructionSyntax(1, 0);
    syntax[D3DSIO_CMP]          = InstructionSyntax(1, 3);
    syntax[D3DSIO_BEM]          = InstructionSyntax(1, 2);
    syntax[D3DSIO_DP2ADD]       = InstructionSyntax(1, 3);
    syntax[D3DSIO_DSX]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_DSY]          = InstructionSyntax(1, 1);
    syntax[D3DSIO_TEXLDD]       = InstructionSyntax(1, 4);
    syntax[D3DSIO_SETP]         = InstructionSyntax(1, 2);
    syntax[D3DSIO_TEXLDL]       = InstructionSyntax(1, 2);
    syntax[D3DSIO_BREAKP]       = InstructionSyntax(0, 1);
    syntax[D3DSIO_PHASE]        = InstructionSyntax(0, 0);
}

//  Get the syntactic information for the shader instruction opcode.
const InstructionSyntax &InstructionsSyntax::getSyntax(D3DSHADER_INSTRUCTION_OPCODE_TYPE opc)
{
    return (*syntax.find(opc)).second;;
}

//  Configured based on the shader version the syntactic information for the shader instruction.
void InstructionsSyntax::configure(DWORD version)
{
    if(version < 0x0104)
        syntax[D3DSIO_TEXCOORD] = InstructionSyntax(1, 0);
    else
        syntax[D3DSIO_TEXCOORD] = InstructionSyntax(1, 1);
        
    if(version < 0x0104)
        syntax[D3DSIO_TEX] = InstructionSyntax(1, 0);
    else if( version == 0x0104)
        syntax[D3DSIO_TEX] = InstructionSyntax(1, 1);
    else 
        syntax[D3DSIO_TEX] = InstructionSyntax(1, 2);

    if(version < 0x0300)
        syntax[D3DSIO_SINCOS] = InstructionSyntax(1, 3);
    else
        syntax[D3DSIO_SINCOS] = InstructionSyntax(1, 1);
}

//  Build the shader program.
IR *IRBuilder::build(const DWORD *d3dShader, u32bit &numTokens)
{
    IR *ir = new IR();

    D3DToken d3dtk;
    unsigned int index = 0;
    
    
    //  Get the version token.
    VersionIRNode *vtk;
    d3dtk.dword = d3dShader[index++];   
    vtk = new VersionIRNode(d3dtk, index - 1);
    
    //  Get the shader type and version.
    version = vtk->getVersion();
    type = vtk->getType();
    
    //  Configure the syntax based on the shader version.
    syntax.configure(version);
    
    //  Add version token to the program.
    ir->tokens.push_back(vtk);

    IRNode *n;
    bool foundEndD3DToken = false;

    //  Process tokens until the end token has been found.
    while (!foundEndD3DToken)
    {
        //  Read the next token.
        d3dtk.dword = d3dShader[index++];

        //  Select the type of token.
        switch(d3dtk.instruction.opcode)
        {
            case D3DSIO_COMMENT:
                
                //  Build a comment node.
                n = buildComment(d3dtk, d3dShader, index);
                break;
                
            case D3DSIO_END:
            
                //  Build an end node.
                n = buildEnd(d3dtk, d3dShader, index);
                
                //  End of program found.
                foundEndD3DToken = true;
                
                break;
                
            case D3DSIO_DCL:
            
                //  Build a declaration node.
                n = buildDeclaration(d3dtk, d3dShader, index);
                break;
                
            case D3DSIO_DEF:
            case D3DSIO_DEFB:                
            case D3DSIO_DEFI:
            
                //  Build a definition node.
                n = buildDefinition(d3dtk, d3dShader, index);
                
                break;

            default:
            
                //  Build an instruction node.
                n = buildInstruction(d3dtk, d3dShader, index);
                
                break;
        }
        
        //  Add node to the shader program.
        ir->tokens.push_back(n);
    }

    //  Return the number of tokens (DWORDs) in the shader program.
    numTokens = index - 1;
    
    //  Return the intermediate representation for the shader program.
    return ir;
}

//  Build a declaration.
IRNode *IRBuilder::buildDeclaration(D3DToken &d3dtk, const DWORD *d3dShader, unsigned int &index)
{
    //  Create the declaration token.
    DeclarationIRNode *decIRNode = new DeclarationIRNode(d3dtk, index - 1);
    
    //  Read the declaration token.
    D3DToken firstTk;
    firstTk.dword = d3dShader[index++];
    
    //  Read the destination parameter token.
    D3DToken destTk;
    destTk.dword = d3dShader[index++];
    
    //  Create a destination parameter node.
    DestinationParameterIRNode *destIRNode = new DestinationParameterIRNode(destTk, index - 1);
    
    //  Check if the register type is a texture sampler.
    if (destIRNode->getRegisterType() != D3DSPR_SAMPLER)
    {
        //if((destIRNode->getRegisterType() == D3DSPR_INPUT) &
        //    ((type == IR_VERTEX_SHADER) || (version>=0x0300))) {

        //  Create a semantic node using the first token.
        SemanticIRNode *semanticIRNode = new SemanticIRNode(firstTk, index - 2);

        //  Add the semantic node as a child of the declaration node.
        decIRNode->addChild(semanticIRNode);
    }
    else
    {
        //else if((destIRNode->getRegisterType() == D3DSPR_SAMPLER) &
        //    ((type == IR_PIXEL_SHADER) || (version>=0x0300))) {

        //  Create a sampler info node using the first token.
        SamplerInfoIRNode *samplerInfoIRNode = new SamplerInfoIRNode(firstTk, index - 2);

        //  Add the sampler info node as a child of the declaration node.
        decIRNode->addChild(samplerInfoIRNode);
    }
    
    //  Add the destination parameter node as a child of the declaration node.    
    decIRNode->addChild(destIRNode);
    
    return decIRNode;
}

//  Build a shader definition.
IRNode *IRBuilder::buildDefinition(D3DToken &d3dtk, const DWORD *d3dShader, unsigned int &index)
{

    //  Create the definition node.
    DefinitionIRNode *defIRNode = new DefinitionIRNode(d3dtk, index - 1);
    
    D3DToken paramTk;
    DestinationParameterIRNode *dstParameter;
    RelativeAddressingIRNode *relAddr;
    
    // Not sure this is necessary
    bool destRelAddrEnabled = ((type == VERTEX_SHADER) && (version >= 0x0300));

    //  Get the number of destination and source parameters in the definition instruction.
    InstructionSyntax instSyntax = syntax.getSyntax(defIRNode->getOpcode());

    //  Check if the defintion has a destination register.
   if (instSyntax.dest == 1)
    {
        //  Read the destination parameter token.
        paramTk.dword = d3dShader[index++];
        
        //  Create a destination parameter node.
        dstParameter = new DestinationParameterIRNode(paramTk, index - 1);
        
        // Check if relative addressing is enabled.
        if(destRelAddrEnabled & (dstParameter->getAddressMode() == D3DSHADER_ADDRMODE_RELATIVE))
        {
            panic("IRBuilder", "buildDefinition", "Relative addressing on a definition!!!");
            
            paramTk.dword = d3dShader[index]; index ++;
            relAddr = new RelativeAddressingIRNode(paramTk, index - 1);
            dstParameter->addChild(relAddr);
            dstParameter->setRelativeAddressing(relAddr);
        }
        
        //  Add the destination parameter as a child of the definition node.
        defIRNode->addChild(dstParameter);
        
        //  Set the destination parameter node as the destination parameter of the definition node.
        defIRNode->setDestination(dstParameter);
    }
    else if (instSyntax.dest > 1)
    {
        panic("IRBuilder", "buildDefinition", "Only one destination parameter allowed.");
    }
    
    //  Process all the source parameters defined for the definition opcode.
    int sources = instSyntax.source;
    for(int i = 0; i < sources; i ++)
    {
        //  Get the next token.
        paramTk.dword = d3dShader[index++];
        
        //  Create a value node depending on the definition opcode.
        switch(defIRNode->getOpcode())
        {
            case D3DSIO_DEFB:
            
                //  Create a boolean value node and add as a child of the definition node.
                defIRNode->addChild(new BoolIRNode(paramTk, index - 1));
                break;
                
            case D3DSIO_DEFI:

                //  Create an integer value node and add as a child of the definition node.
                defIRNode->addChild(new IntegerIRNode(paramTk, index - 1));
                
                break;
            case D3DSIO_DEF:
            
                //  Create a float point value node and add as a child of the definition node.
                defIRNode->addChild(new FloatIRNode(paramTk, index - 1));

                break;
        }        
    }

    return defIRNode;
}

//  Build a comment node.
IRNode *IRBuilder::buildComment(D3DToken &d3dtk, const DWORD *d3dShader, unsigned int &index)
{
    D3DToken commentDataToken;
    
    //  Create the comment node.
    CommentIRNode *commentIRNode = new CommentIRNode(d3dtk, index - 1);
    
    CommentDataIRNode *commentDataIRNode;

    //  Read all the comment data tokens.
    for(unsigned int i = 0; i < d3dtk.comment.length; i++)
    {
        //  Read the next token.
        commentDataToken.dword = d3dShader[index++];
        
        //  Create a comment data node.
        commentDataIRNode = new CommentDataIRNode(commentDataToken, index - 1);
        
        //  Add the commend data node as a child to the comment node.
        commentIRNode->addChild(commentDataIRNode);
    }

    return commentIRNode;
}

//  Build an instruction node.
IRNode *IRBuilder::buildInstruction(D3DToken &instrToken, const DWORD *d3dShader, unsigned int &index)
{
    D3DToken paramTk;
    SourceParameterIRNode *srcParameter;
    DestinationParameterIRNode *dstParameter;
    RelativeAddressingIRNode *relAddr;

    //  Determine based on the shader type and version if relative addressing in source parameters is allowed.
    bool sourceRelAddrEnabled = (((type == PIXEL_SHADER) && (version >= 0x0300)) | ((type == VERTEX_SHADER) && (version >= 0x0200)));
    
    //  Determine based on the shader type and version if relative addressing in destination paraemters is allowed.
    bool destRelAddrEnabled = ((type == VERTEX_SHADER) && (version >= 0x0300));
    
    //  Determine based on the shader type and version if instruction predication is allowed.
    bool predicateEnabled = (version >= 0x0200);

    //  Create the instruction node.
    InstructionIRNode *instructionIRNode = new InstructionIRNode(instrToken, index - 1);

    //  Get the number of destination and source parameters for the instruction.
    InstructionSyntax instSyntax = syntax.getSyntax(instructionIRNode->getOpcode());

    //  Check if the instruction has a destination parameter.
    if (instSyntax.dest == 1)
    {
        //  Read the next token.
        paramTk.dword = d3dShader[index++];
        
        //  Create a destination parameter node.
        dstParameter = new DestinationParameterIRNode(paramTk, index - 1);
        
        //  Check if relative addressing is enabled and the destination parameter uses the relative
        //  addressing mode.
        if (destRelAddrEnabled && (dstParameter->getAddressMode() == D3DSHADER_ADDRMODE_RELATIVE))
        {
            //  Read the next token.
            paramTk.dword = d3dShader[index++];
            
            //  Create a relative addressing node.
            relAddr = new RelativeAddressingIRNode(paramTk, index - 1);
            
            //  Add the relative addressing node as a child to the destination parameter node.
            dstParameter->addChild(relAddr);
            dstParameter->setRelativeAddressing(relAddr);
        }
        
        //  Add the destination parameter node as a child to the instruction node.
        instructionIRNode->addChild(dstParameter);
        instructionIRNode->setDestination(dstParameter);
    }
    else if (instSyntax.dest > 1)
    {
        panic("IRBuilder", "buildInstruction", "Only one destination parameter allowed");
    }

    //  If the instruction is predicate there is an extra source parameter.
    int sources = instSyntax.source;
    if  (predicateEnabled && instructionIRNode->isPredicate())
        sources++;

    //  Process all the instruction source parameters.
    for(int i = 0; i < sources; i ++)
    {
        //  Read the next token.
        paramTk.dword = d3dShader[index++];
        
        //  Create a source parameter node.
        srcParameter = new SourceParameterIRNode(paramTk, index - 1);
        
        //  Check if relative addressing is enabled and the source parameter uses the relative 
        //  addressing mode.
        if (sourceRelAddrEnabled && (srcParameter->getAddressMode() == D3DSHADER_ADDRMODE_RELATIVE))
        {
            //  Read the next token.
            paramTk.dword = d3dShader[index++];
            
            //  Create a relative addressing node.
            relAddr = new RelativeAddressingIRNode(paramTk, index - 1);
            
            //  Add the relative addressing node as child of the source parameter node.
            srcParameter->addChild(relAddr);
            srcParameter->setRelativeAddressing(relAddr);
        }
        
        //  Add the source parameter node as a child of the instruction node.
        instructionIRNode->addChild(srcParameter);
        instructionIRNode->addSource(srcParameter);
    }

    return instructionIRNode;
}

//  Build an End node.
IRNode *IRBuilder::buildEnd(D3DToken &endToken, const DWORD *d3dShader, unsigned int &index)
{
    //  Create an end node.
    return new EndIRNode(endToken, index - 1);
}

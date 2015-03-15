#ifndef __IR_BUILDER
#define __IR_BUILDER

#include "IR.h"

#include <map>

/**
 *
 *  Store syntactic information about a shader instruction.
 *
 */
struct InstructionSyntax
{
    unsigned int dest;      /**<  Number of destination parameters for the instruction.  */
    unsigned int source;    /**<  Number of source parameters for the instruction.  */
    
    //  Empty constructor.
    InstructionSyntax(): dest(0), source(0) {}
    
    //  Constructor.
    InstructionSyntax(unsigned int d, unsigned int s): dest(d), source(s) {}
};
    
/**
 *
 *  Holds the syntax of shader instructions (i. e. MOV -> 1 dest 2 source).
 *  Can be configured for different shader versions.
 *
 */
class InstructionsSyntax
{
public:

    /**
     *
     *  Configure the syntactic information of shader instruction based on the shader version.
     *
     *  @param version The shader version.
     *
     */
     
    void configure(DWORD version);
    
    /**
     *
     *  Get the syntactic information for a given shader instruction.
     *
     *  @param opc The shader instruction opcode for which to obtain the syntactic information.
     *
     *  @return A reference to a syntactic information object.
     *
     */
     
    const InstructionSyntax &getSyntax(D3DSHADER_INSTRUCTION_OPCODE_TYPE opc);
    
    /**
     *
     *  InstructionsSyntax constructor.
     *
     *  @return A new initialized InstructionsSyntax object.
     *
     */
     
    InstructionsSyntax();

private:
    
    std::map<D3DSHADER_INSTRUCTION_OPCODE_TYPE, InstructionSyntax> syntax;  /**<  Map of instruction opcodes to syntactic information objects.  */
};


/**
 *
 *  Builds a shader program (intermediate representation) from shader bytecode.
 *
 *
 */
class IRBuilder
{
public:

    /** 
     *
     *  Build the intermediate representation for a shader program based on the provided bytecode.
     *
     *  @param d3dShader A pointer to the shader program bytecode.
     *
     *  @return A pointer to the intermediate representation of the shader program.
     *
     */
     
    IR *build(const DWORD *d3dShader);

private:

    DWORD version;              /**<  The shader program version.  */
    ShaderType type;            /**<  The shader program type (vertex or pixel).  */
    InstructionsSyntax syntax;  /**<  Syntactic information for the shader type and version.  */

    /** 
     *
     *  Build a comment node.
     *  Children nodes for comment data token are created and added as children as required.
     *
     *  @param d3dtk The comment token.
     *  @param d3dShader Pointer to the shader program bytecode.
     *  @param index Reference to the variable that holds the current parsing position inside
     *         the shader program bytecode (index in 32-bit tokens).
     *
     *  @return A pointer to the comment node.
     *
     */
     
    IRNode *buildComment(D3DToken &d3dtk, const DWORD *d3dShader, unsigned int &index);

    /** 
     *
     *  Build an instruction node.
     *  Children nodes for instruction parameters are created and added as children as required.
     *
     *  @param d3dtk The instruction token.
     *  @param d3dShader Pointer to the shader program bytecode.
     *  @param index Reference to the variable that holds the current parsing position inside
     *         the shader program bytecode (index in 32-bit tokens).
     *
     *  @return A pointer to the instruction node.
     *
     */

    IRNode *buildInstruction(D3DToken &d3dtk, const DWORD *d3dShader, unsigned int &index);

    /** 
     *
     *  Build a declaration node.
     *  Children nodes for declaration parameters are created and added as children as required.
     *
     *  @param d3dtk The instruction token.
     *  @param d3dShader Pointer to the shader program bytecode.
     *  @param index Reference to the variable that holds the current parsing position inside
     *         the shader program bytecode (index in 32-bit tokens).
     *
     *  @return A pointer to the declaration node.
     *
     */

    IRNode *buildDeclaration(D3DToken &d3dtk, const DWORD *d3dShader, unsigned int &index);

    /** 
     *
     *  Build a definition node.
     *  Children nodes for defintion parameters are created and added as children as required.
     *
     *  @param d3dtk The instruction token.
     *  @param d3dShader Pointer to the shader program bytecode.
     *  @param index Reference to the variable that holds the current parsing position inside
     *         the shader program bytecode (index in 32-bit tokens).
     *
     *  @return A pointer to the definition node.
     *
     */

    IRNode *buildDefinition(D3DToken &d3dtk, const DWORD *d3dShader, unsigned int &index);

    /** 
     *
     *  Build the end node.
     *
     *  @param d3dtk The instruction token.
     *  @param d3dShader Pointer to the shader program bytecode.
     *  @param index Reference to the variable that holds the current parsing position inside
     *         the shader program bytecode (index in 32-bit tokens).
     *
     *  @return A pointer to the end node.
     *
     */

    IRNode *buildEnd(D3DToken &d3dtk, const DWORD *d3dShader, unsigned int &index);

};

#endif

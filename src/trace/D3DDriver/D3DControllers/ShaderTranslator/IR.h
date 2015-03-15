/**
 * Intermediate representation of D3D Shader tokens
 * @author Chema Solís
 */

#ifndef __IR
#define __IR

#include "Types.h"

#include <vector>

////////////////////////////////////////////////////////////////////////
// SHADING TYPES                                                      //
////////////////////////////////////////////////////////////////////////

enum ShaderType
{
    PIXEL_SHADER,
    VERTEX_SHADER
};

/**
 *
 * Bitfield for a Comment D3D Token
 *
 */
struct CommentBitfield
{
    DWORD opcode   : 16,      //  Must be 0xFFFE (comment token).
          length   : 15,      //  Length in dwords of the comments that follow.
          mbz31    : 1;       //  Must be zero
};

/**
 *
 * Bitfield for a Version D3D Token
 *
 */
struct VersionBitfield
{
    DWORD minorVersion : 8,     // Minor version number
          majorVersion : 8,     // Major version number
          type         : 16;    // For pixel shader 0xFFFF, for vertex shader 0xFFFE
};

/**
 *
 * Bitfield for a Instruction D3D Token
 *
 */
struct InstructionBitfield
{
    DWORD opcode    : 16,       // Operation code (D3DSIO_*).
          specific  : 8,        // Specific controls related with the operation opcode
                                // (D3DSI_* for texld controls, D3DSPC_* for comparison controls)
          length    : 4,        // For vertex shader version earlier than 2_0 must be 0
                                // For vertex shader version 2_0 and later, the number of tokens that comprise the instruction
                                // excluding the instruction token.
          predicate : 1,        // For pixel and vertex shader versions earlier than 2_0 mus be 0
                                // For pixel and vertex shader versions 2_0 and later, indicates that the instruction is predicated (value 1)
                                // and contains an extra predicate source at the end of the shader code.
          mbz29     : 1,        // Must be zero
          coissue   : 1,        // For pixel shader versions earlier than 2_0 represents the coissue bit (value 1)
                                // For pixel shader versions 2_0 and later this bit must be 0.
          mbz31     : 1;        // Must be zero
};

/**
 *
 * Bitfield for a Parameter D3D Token (generalization of the Source and Destination Parameters).
 *
 */
struct ParameterBitfield
{
    DWORD regNumber : 11,       //  Register number (offset in the register file)
          regTypeHi : 2,        //  Higher 2-bits of the register type (D3DSPR_*)
          specifc   : 15,       //  Specific for source and destination parameters
          regTypeLo : 3,        //  Lower 3-bits of the register type (D3DSPR_*)
          mbo31     : 1;        //  Must be one
};

/**
 *
 * Bitfield for a source Parameter D3D Token (specialization of the Parameter D3D Token)
 *
 */
struct SourceParameterBitfield
{
    DWORD regNumber  : 11,  //  Register number (offset in the register file)
          regTypeHi  : 2,   //  Higher 2-bits of the register type (D3DSPR_*)
          relAddress : 1,   //  For pixel shader versions earlier than 3_0 bit must be zero
                            //  For pixel shader versions 3_0 and later and all vertex shader versions
                            //  indicates relative addressing (value 1)
          mbz15_14   : 2,   //  Must be zero
          swizzleX   : 2,   //  Determines channel swizzling for channel x (D3DVS_X_*)
          swizzleY   : 2,   //  Determines channel swizzling for channel y (D3DVS_Y_*)
          swizzleZ   : 2,   //  Determines channel swizzling for channel z (D3DVS_Z_*)
          swizzleW   : 2,   //  Determines channel swizzling for channel w (D3DVS_W_*)
          modifier   : 4,   //  Source modifier (D3DSPSM_*)
          regTypeLo  : 3,   //  Lower 3-bits of the register type (D3DSPR_*)
          mbo31      : 1;   //  Must be one
};

/**
 *
 * Bitfield for a destination Parameter D3D Token (specialization of the Parameter D3D Token)
 *
 */
struct DestParameterBitfield
{
    DWORD regNumber  : 11,  //  Register number (offset in the register file)
          regTypeHi  : 2,   //  Higher 2-bits of the register type (D3DSPR_*)
          relAddress : 1,   //  For vertex shader versions 3_0 and later indicates relative addressing (value 1)
                            //  For vertex shader versions earlier than 3_0 and all pixel shader versions must be zero.
          mbz15_14   : 2,   //  Must be zero
          writeMaskX : 1,   //  Determines write mask for channel x.
          writeMaskY : 1,   //  Determines write mask for channel x.
          writeMaskZ : 1,   //  Determines write mask for channel x.
          writeMaskW : 1,   //  Determines write mask for channel x.
          modifier   : 4,   //  Result modifier (D3DSPDM_*)
          shift      : 4,   //  For pixel shader versions earlier than 2_0 specifies the shift scale (signed shift) (D3
                            //  For pixel shader version 2_0 and later and all vertex shader versions must be 0                        
          regTypeLo  : 3,   //  Lower 3-bits of the register type (D3DSPR_*)
          mbo31      : 1;   //  Must be one
};


/**
 *
 * Bitfield for accessing a D3D Token bit level
 *
 */
struct BitsBitfield
{
    DWORD bit0  : 1, bit1  : 1, bit2  : 1, bit3  : 1,
          bit4  : 1, bit5  : 1, bit6  : 1, bit7  : 1,
          bit8  : 1, bit9  : 1, bit10 : 1, bit11 : 1,
          bit12 : 1, bit13 : 1, bit14 : 1, bit15 : 1,
          bit16 : 1, bit17 : 1, bit18 : 1, bit19 : 1,
          bit20 : 1, bit21 : 1, bit22 : 1, bit23 : 1,
          bit24 : 1, bit25 : 1, bit26 : 1, bit27 : 1,
          bit28 : 1, bit29 : 1, bit30 : 1, bit31 : 1;
};


/**
 *
 * Bitfield for accessing to nibbles (4 bits) of a D3D Token.
 *
 */
struct NibblesBitfield
{
    DWORD nibble0 : 4, nibble1 : 4,
          nibble2 : 4, nibble3 : 4,
          nibble4 : 4, nibble5 : 4,
          nibble6 : 4, nibble7 : 4;
};

/**
 *
 * Bitfield for accessing to bytes of a D3D Token.
 *
 */
struct BytesBitfield
{
    DWORD byte0 : 8,
          byte1 : 8,
          byte2 : 8,
          byte3 : 8;
};

/**
 *
 * Bitfield for accessing to words of a D3D Token.
 *
 */
struct WordsBitfield
{
    DWORD word0 : 16,
          word1 : 16;
};

/**
 *
 * At low level a D3DToken is simply a DWORD, altough for convenience
 * a union is used to simplify bit level access.
 *
 */
union D3DToken
{
    DWORD dword;
    BitsBitfield bits;
    NibblesBitfield nibbles;
    BytesBitfield bytes;
    WordsBitfield words;
    VersionBitfield version;
    InstructionBitfield instruction;
    CommentBitfield comment;
    ParameterBitfield parameter;
    SourceParameterBitfield sourceParameter;
    DestParameterBitfield destParameter;
};

class IRVisitor;

/**
 *
 * Base class for the intermediate representation of a token.
 *
 */
class IRNode
{
public:
    
    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     *  @note Must be implemented in specialized classes.
     *
     */   
    
    virtual void accept(IRVisitor *v) = 0;
    
    /**
     *
     *  Get the D3D Token associated with the node.
     *
     *  @return The D3D Token associated with the node.
     *
     */
     
    D3DToken getToken();
    
    /**
     *
     *  Get the offset of the D3D Token associated with the node in the program stream.
     *  The offset counts DWORDs (32-bit words).
     *
     *  @return The offset in DWORDs (32-bit words) of the associated D3D Token in the
     *  program stream.
     *
     */
    unsigned int getOffset();
    
    /**
     *
     *  Adds a child node to the current node.
     *
     *  @param n Pointer to the child node to add.
     *
     */
     
    void addChild(IRNode *n);
    
    /**
     *
     *  Get the node children nodes.
     *
     *  @return A reference to the vector storing the pointers to the current node children nodes.
     *
     */
     
    const std::vector<IRNode *> &getChilds();
    
    /**
     *
     *  IR Node constructor.
     *
     *  @param token The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized IRNode object.
     *
     */
     
    IRNode(D3DToken token, unsigned int offset);

protected:

    D3DToken d3dToken;              /**<  The D3D Token associated with the IR node.  */
    unsigned int offset;            /**<  The offset of the D3D Token in the program stream.  */
    std::vector<IRNode *> childs;   /**<  A vector storing pointers to the node children nodes.  */
};

/**
 *
 *  IR Node derived class for semantic tokens (follow a instruction token with the dcl opcode).
 *
 */
class SemanticIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */
     
    void accept(IRVisitor *v);

    /**
     *
     *  Get the semantic usage from the associated semantic D3D token.
     *
     *  @return The semantic usage associated with the semantic D3D token.
     *
     */
     
    DWORD getUsage();

    /** 
     *
     *  Get the semantic usage index from the associted semantic D3D Token.
     *
     *  @return The semantic usage index associated with the semantic D3D token.
     *
     */
     
    DWORD getUsageIndex();


    /**
     *
     *  Semantic IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized SemanticIRNode object.
     *
     */
     
    SemanticIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for Sampler Info tokens (follow a instruction token with the dcl opcode)
 *
 */
class SamplerInfoIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Get the texture type associated with the sampler info D3D Token.
     *
     *  @return The sampler type associated with the sampler info D3D token.
     *
     */
    
    D3DSAMPLER_TEXTURE_TYPE getTextureType();

    /**
     *
     *  Sampler Info IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized SamplerInfoIRNode object.
     *
     */

    SamplerInfoIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for comment data tokens (follow a comment token).
 *
 */ 
class CommentDataIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Comment Data IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized CommentDataIRNode object.
     *
     */
    CommentDataIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for comment tokens.
 * 
 */
class CommentIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);
    
    /**
     *
     *  Comment IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized CommentIRNode object.
     *
     */
    CommentIRNode(D3DToken d3dtk, unsigned int offset);
};

class RelativeAddressingIRNode;

/**
 *
 *  IR Node derived class for parameter D3D tokens (generic).
 *
 */
class ParameterIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */
     
    void accept(IRVisitor *v);

    /**
     *
     *  Get the register number for the parameter.
     *
     *  @return The register number for the parameter.
     *
     */
      
    unsigned int getNRegister();

    /**
     *
     *  Get the register type for the parameter.
     *
     *  @return The register type for the parameter.
     *
     */
     
    D3DSHADER_PARAM_REGISTER_TYPE getRegisterType();

    /**
     *
     *  Get the addressing mode for the parameter (relative/absolute).
     *
     *  @return The addressing mode for the parameter.
     *
     */
     
    D3DSHADER_ADDRESSMODE_TYPE getAddressMode();

    /**
     *
     *  Get the relative addressing mode node for the parameter.
     *
     *  @return A pointer to the relative addressing mode for the parameter.
     *
     */
     
    RelativeAddressingIRNode *getRelativeAddressing();
    
    /**
     *
     *  Set the relative addressing mode node for the parameter.
     *
     *  @param relAddr Pointer to a relative addressing node.
     *
     */
     
    void setRelativeAddressing(RelativeAddressingIRNode *_relAddr);

    /**
     *
     *  Parameter IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized ParameterIRNode object.
     *
     */
     
    ParameterIRNode(D3DToken d3dtk, unsigned int offset);
    
private:

    RelativeAddressingIRNode *relativeAddressing;   /**<  Pointer to the parameter relative addressing node.  */
};

/**
 *
 *  IR Node derived class for source parameter D3D tokens (specific).
 *
 */
class SourceParameterIRNode : public ParameterIRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Get the source swizzle for the selected component.
     *
     *  @param component The component for which to obtain the swizzle:
     *                    x -> 0
     *                    y -> 1
     *                    z -> 2
     *                    w -> 3
     *
     *  @return Token masked for desired component.
     *
     */
    DWORD getSwizzle(unsigned int component);

    /**
     *
     *  Get the source parameter modifier.
     *
     *  @return The source parameter modifier.
     *
     */
     
    D3DSHADER_PARAM_SRCMOD_TYPE getModifier();

    /**
     *
     *  Source Parameter IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized SourceParameterIRNode object.
     *
     */

    SourceParameterIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for relative addressign D3D tokens (derived from parameter IR Node).
 *
 */
class RelativeAddressingIRNode : public ParameterIRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Get the index for the component in the address register to be used for relative addressing.
     *
     *  @return The index for the component in the address register to be used for relative addressing:
     *            x -> 0
     *            y -> 1
     *            z -> 2
     *            w -> 3
     *
     */
    unsigned int getComponentIndex();

    /**
     *
     *  Relative Addressing IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized RelativeAddressingIRNode object.
     *
     */
    RelativeAddressingIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for destination parameter D3D tokens (specific).
 *
 */
class DestinationParameterIRNode : public ParameterIRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Get the destination parameter write mask.
     *
     *  @return The destination parameter write mask.
     *
     */
     
    DWORD getWriteMask();

    /**
     *
     *  Get the destination parameter modifier.
     *
     *  @return The destination parameter modifier.
     *
     */
     
    DWORD getModifier();

    /**
     *
     *  Get the destination parameter shift scale.
     *
     *  @return The destination parameter shift scale.
     *
     */
     
    DWORD getShiftScale();

    /**
     *
     *  Destination Parameter IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized DestinationParameterIRNode object.
     *
     */
    DestinationParameterIRNode(D3DToken d3dtk, unsigned int offset);
};


/**
 *
 *  IR Node derived class for instruction D3D Tokens.
 *
 */
class InstructionIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Get the instruction opcode.
     *
     *  @return The instruction opcode.
     *
     */
     
    D3DSHADER_INSTRUCTION_OPCODE_TYPE getOpcode();

    /**
     *
     *  Returns if the instruction is a control flow instruction.
     *
     *  @return The instruction is a control flow instruction.
     *
     */

    bool isJmp();

    /**
     *
     *  Return if the instruction coissues.
     *
     *  @return If the instruction is coissued.
     *
     */
     
    bool isCoissue();

    /**
     *
     *  Return if the instruction is predicated.
     *
     *  @return If the instruction is predicated.
     *
     */
     
    bool isPredicate();

    /**
     *
     *  Return the number of parameter tokens associated with the instruction.
     *
     *  @return The number of parameter tokens associated with the instruction.
     *
     */
     
    unsigned int getLength();

    /**
     *
     *  Get the instruction comparison control code (for control flow instructions).
     *
     *  @return The instruction comparison control code.
     *
     */
     
    D3DSHADER_COMPARISON getComparison();

    /**
     *
     *  Get the instruction specific control bits.
     *
     *  @return The instruction specific control bits.
     *
     */
     
    DWORD getSpecificControls();

    /**
     *
     *  Get the sampler texture type
     *
     *  @return The sampler texture type.
     *
     */
    //D3DSAMPLER_TEXTURE_TYPE getSamplerTextureType();

    /**
     *
     *  Add a source parameter node to the instruction node.
     *
     *  @param source Pointer to a source parameter node.
     *
     */
     
    void addSource(SourceParameterIRNode *source);
    
    /**
     *
     *  Get the vector of source parameter nodes for the instruction.
     *
     *  @return A reference to the vector with pointers to the instruction
     *  source parameter nodes.
     *
     */
     
    std::vector<SourceParameterIRNode *> &getSources();

    /**
     *  Gets the destination parameter node for the instruction node.
     *
     *  @return The pointer to the instruction destination parameter node.
     *
     */
     
    DestinationParameterIRNode *getDestination();
    
    /**
     *  Sets the destination parameter nodoe for the instruction node.
     *
     *  @param destination Pointer to a destination parameter node.
     *
     */
     
    void setDestination(DestinationParameterIRNode *destination);

    /**
     *
     *  Instruction IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized InstructionIRNode object.
     *
     */
     
    InstructionIRNode(D3DToken d3dtk, unsigned int offset);
    
private:
    
    std::vector<SourceParameterIRNode *>sources;    /**<  Vector with pointers to the instruction source parameter nodes.  */
    DestinationParameterIRNode *destination;        /**<  Pointer to the instruction destination parameter node.  */
};

/**
 *
 *  IR Node derived class for Declaration D3D Tokens (dcl opcode).
 *
 */
class DeclarationIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Get the instruction opcode.
     *
     *  @return The instruction opcode.
     *
     */

    D3DSHADER_INSTRUCTION_OPCODE_TYPE getOpcode();

    /**
     *
     *  Declaration IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized DeclarationIRNode object.
     *
     */

    DeclarationIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for Definition D3D Tokens (def opcode).
 *
 */
class DefinitionIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */
     
    void accept(IRVisitor *v);

    /**
     *
     *  Get the instruction opcode.
     *
     *  @return The instruction opcode.
     *
     */

    D3DSHADER_INSTRUCTION_OPCODE_TYPE getOpcode();

    /**
     *
     *  Gets the destination parameter node for the definition.
     *
     *  @return A pointer to the destination parameter node for the definition.
     *
     */
     
    DestinationParameterIRNode *getDestination();
    
    /**
     *
     *  Set the destionation parameter node for the definition.
     *
     *  @param destination Pointer to a destination parameter node.
     *
     */
     
    void setDestination(DestinationParameterIRNode *destination);

    /**
     *
     *  Definition IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized DefinitionIRNode object.
     *
     */

    DefinitionIRNode(D3DToken d3dtk, unsigned int offset);
    
private:

    DestinationParameterIRNode *destination;    /**<  Pointer to the definiton destination parameter node.  */
};


/**
 *
 *  IR Node derived class for end D3D Tokens (end of program mark).
 *
 */
class EndIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */
     
    void accept(IRVisitor *v);

    /**
     *
     *  End IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized EndIRNode object.
     *
     */
     
    EndIRNode(D3DToken _d3dtk, unsigned int _offset);
};

/**
 *
 *  IR Node derived class for boolean values (follow def opcode tokens)
 *
 */
class BoolIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);
    
    /**
     *
     *  Get the value of the boolean token.
     *
     *  @return The boolean value for the token.
     *
     */
   
   bool getValue();
   
    /**
     *
     *  Bool IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized BoolIRNode object.
     *
     */

    BoolIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for float point values (follow def opcode tokens)
 *
 */
class FloatIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Get the value of the float point token.
     *
     *  @return The float point value for the token.
     *
     */
   
    float getValue();

    /**
     *
     *  Float IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized FloatIRNode object.
     *
     */

    FloatIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for integer values (follow def opcode tokens)
 *
 */
class IntegerIRNode : public IRNode
{
public:
    
    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);
    
    /**
     *
     *  Get the value of the integer node.
     *
     *  @return The integer value for the node.
     *
     */

    int getValue();
    
    /**
     *
     *  Integer IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized IntegerIRNode object.
     *
     */

    IntegerIRNode(D3DToken d3dtk, unsigned int offset);
};

/**
 *
 *  IR Node derived class for version D3D tokens.
 *
 */
class VersionIRNode : public IRNode
{
public:

    /**
     *
     *  Access function for visitor classes.
     *
     *  @param v Pointer to a visitor class object
     *
     */

    void accept(IRVisitor *v);

    /**
     *
     *  Get the shader version.
     *
     *  @return The shader version.
     *
     */
     
    DWORD getVersion();
    
    /**
     *
     *  Get the shader type.
     *
     *  @return The shader type: VERTEX_SHADER or PIXEL_SHADER.
     *
     */
     
    ShaderType  getType();

    /**
     *
     *  Version IR Node constructor.
     *
     *  @param d3dtk The D3D Token associated with the node.
     *  @param offset The offset of the D3D Token, counted in DWORDs (32-bit words) in the program stream.
     *
     *  @return A new initialized VersionIRNode object.
     *
     */

    VersionIRNode(D3DToken d3dtk, unsigned int offset);
};



/**
 *
 *  Class that stores and traverses the nodes of a shader program.
 *
 */
class IR
{
friend class IRBuilder;

public:

    /**
     *
     *  Traverse the nodes of the shader program using the provided visitor object and the defined order.
     *
     *  @param v Pointer to a visitor class object.
     *  @param preorder Defines the order in which nodes are visited.  Set to true for parent first, set to
     *  false for children nodes first.
     *
     */
     
    void traverse(IRVisitor *v, bool preorder = false);

    /**
     *
     *  IR destructor.
     *
     */
     
    ~IR();
    
private:

    /**
     *
     *  Delete recursively the node and it's children nodes.
     *
     *  @param tk Pointer to a IRNode object to recursively delete.
     *
     */
     
    void deleteRecursive(IRNode *tk);
    
    /**
     *
     *  Recursively traverse using the provided visitor and in the defined order the
     *  node and it's children nodes.
     *
     *  @param tk Pointer to the node to recursively traverse.
     *  @param v Pointer to a visitor object.
     *  @param preorder Defines the order in which nodes are visited.  Set to true for parent first,
     *  set to false for children nodes first.
     *
     */
     
    void traverseRecursive(IRNode *tk, IRVisitor *v, bool preorder);
    
    std::list<IRNode *> tokens;     /**<  List of pointers to the top nodes in the shader program.  */
};

/**
 *
 * Visitor pattern for token processing
 *
 */
class IRVisitor
{

public:

    virtual void begin() = 0;
    virtual void visit(IRNode *n) = 0;
    virtual void visit(CommentIRNode *n) = 0;
    virtual void visit(CommentDataIRNode *n) = 0;
    virtual void visit(VersionIRNode *n) = 0;
    virtual void visit(InstructionIRNode *n) = 0;
    virtual void visit(DeclarationIRNode *n) = 0;
    virtual void visit(EndIRNode *n) = 0;
    virtual void visit(DefinitionIRNode *n) = 0;
    virtual void visit(ParameterIRNode *n) = 0;
    virtual void visit(SourceParameterIRNode *n) = 0;
    virtual void visit(DestinationParameterIRNode *n) = 0;
    virtual void visit(RelativeAddressingIRNode *n) = 0;
    virtual void visit(BoolIRNode *n) = 0;
    virtual void visit(FloatIRNode *n) = 0;
    virtual void visit(IntegerIRNode *n) = 0;
    virtual void visit(SemanticIRNode *n) = 0;
    virtual void visit(SamplerInfoIRNode *n) = 0;
    virtual void end() = 0;
};

#endif

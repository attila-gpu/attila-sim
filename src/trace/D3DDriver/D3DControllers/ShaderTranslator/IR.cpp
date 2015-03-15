
#include "IR.h"

#include <vector>

using namespace std;

//  Traverse the shader program nodes with the provided visitor and the defined order.
void IR::traverse(IRVisitor *v, bool preorder)
{
    v->begin();
    list<IRNode *>::iterator itTk;
    
    //  Traverse recursively all the top nodes.
    for(itTk = tokens.begin(); itTk != tokens.end(); itTk ++)
    {
        traverseRecursive(*itTk ,v, preorder);
    }

    v->end();
}

//  Traverse recursively the nodes of the shader program with the provided visitor and the defined order.
void IR::traverseRecursive(IRNode *tk, IRVisitor *v, bool preorder)
{
    //  Check if parent node must be processed first.
    if (preorder)
        tk->accept(v);

    const vector<IRNode *> &childs = tk->getChilds();
    vector<IRNode *>::const_iterator itC;
    
    //  Traverse recursively all the children nodes.
    for(itC = childs.begin(); itC != childs.end(); itC ++)
        traverseRecursive(*itC, v, preorder);

    //  Check if the parent node must be processed last.
    if (!preorder)
        tk->accept(v);
}

//  IR class destructor
IR::~IR()
{
    list<IRNode *>::iterator itTk;
    
    //  Recursively delete the top nodes.
    for(itTk = tokens.begin(); itTk != tokens.end(); itTk ++) 
    {
        deleteRecursive(*itTk);
    }
}

//  Recursively delete a node.
void IR::deleteRecursive(IRNode *n)
{
    const vector<IRNode *> &childs = n->getChilds();
    vector<IRNode *>::const_iterator itC;
    
    //  Recursively delete the node children nodes.    
    for(itC = childs.begin(); itC != childs.end(); itC ++)
        deleteRecursive(*itC);
    delete n;
}


//  IRNode Constructor
IRNode::IRNode(D3DToken _token, unsigned int _offset) :
    d3dToken(_token), offset(_offset)
{
    // Nothing to do.
}


//  The the D3D token associated with IR node.
D3DToken IRNode::getToken()
{
    return d3dToken;
}

//  Get the offset in the program stream corresponding with the IR node.
unsigned int IRNode::getOffset()
{
    return offset;
}

//  Add a child node to the current node.
void IRNode::addChild(IRNode *n)
{
    childs.push_back(n);
}

//  Get a vector with pointers to the node children nodes.
const std::vector<IRNode *> &IRNode::getChilds()
{
    return childs;
}

//  Semantic IR Node constructor.
SemanticIRNode::SemanticIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function visitor classes.
void SemanticIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get semantic usage from the token.
DWORD SemanticIRNode::getUsage()
{
    return (d3dToken.dword & D3DSP_DCL_USAGE_MASK);
}

//  Get semantic usage index from the token.
DWORD SemanticIRNode::getUsageIndex()
{
    return (d3dToken.dword & D3DSP_DCL_USAGEINDEX_MASK) >> D3DSP_DCL_USAGEINDEX_SHIFT;
}

//  Constructor for Sampler Info IR Node.
SamplerInfoIRNode::SamplerInfoIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void SamplerInfoIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the texture type from the token.
D3DSAMPLER_TEXTURE_TYPE SamplerInfoIRNode::getTextureType()
{
    return static_cast<D3DSAMPLER_TEXTURE_TYPE>(d3dToken.dword & D3DSP_TEXTURETYPE_MASK);
}


//  Comment Data IR Node constructor.
CommentDataIRNode::CommentDataIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)
{
    //  NOthing to do.
}

//  Access function for visitor classes.
void CommentDataIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  CommentIRNode constructor.
CommentIRNode::CommentIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)    
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void CommentIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  InstructionIRNode constructor.
InstructionIRNode::InstructionIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset), destination(0)
{
    //   Nothing to do.
}

//  Access function for visitor classes.
void InstructionIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the instruction opcode.
D3DSHADER_INSTRUCTION_OPCODE_TYPE InstructionIRNode::getOpcode()
{
    return static_cast<D3DSHADER_INSTRUCTION_OPCODE_TYPE>(d3dToken.dword & D3DSI_OPCODE_MASK);
}

//  Returns if the instruction is a control flow instruction.
bool InstructionIRNode::isJmp()
{
    return ((getOpcode() == D3DSIO_CALL)    || (getOpcode() == D3DSIO_CALLNZ) || 
            (getOpcode() == D3DSIO_LOOP)    || (getOpcode() == D3DSIO_RET)    || 
            (getOpcode() == D3DSIO_REP)     || (getOpcode() == D3DSIO_ENDREP) || 
            (getOpcode() == D3DSIO_IF)      || (getOpcode() == D3DSIO_IFC)    || 
            (getOpcode() == D3DSIO_ELSE)    || (getOpcode() == D3DSIO_ENDIF)  || 
            (getOpcode() == D3DSIO_BREAK)   || (getOpcode() == D3DSIO_BREAKC) || 
            (getOpcode() == D3DSIO_ENDLOOP));
}

//  Return if the instruction coissues.
bool InstructionIRNode::isCoissue()
{
    return (d3dToken.dword & D3DSI_COISSUE) != 0;
}

//  Return if the instruction is predicated.
bool InstructionIRNode::isPredicate()
{
    return (d3dToken.dword & D3DSHADER_INSTRUCTION_PREDICATED) != 0;
}

//  Return the number of parameter tokens associated with the instruction.
unsigned int InstructionIRNode::getLength()
{
    return (d3dToken.dword & D3DSI_INSTLENGTH_MASK) >> D3DSI_INSTLENGTH_SHIFT;
}

//  Get the instruction comparison control code (for control flow instructions).
D3DSHADER_COMPARISON InstructionIRNode::getComparison()
{
    return static_cast<D3DSHADER_COMPARISON>((d3dToken.dword & D3DSHADER_COMPARISON_MASK) >> D3DSP_OPCODESPECIFICCONTROL_SHIFT);
}

//  Get the instruction specific control bits.
DWORD InstructionIRNode::getSpecificControls()
{
    return d3dToken.dword & D3DSP_OPCODESPECIFICCONTROL_MASK;
}

//  Get the sampler texture type (???).
/*D3DSAMPLER_TEXTURE_TYPE InstructionIRNode::getSamplerTextureType()
{
    return static_cast<D3DSAMPLER_TEXTURE_TYPE>(d3dToken.dword & D3DSP_TEXTURETYPE_MASK);
}*/

//  Add a source parameter node to the instruction node.
void InstructionIRNode::addSource(SourceParameterIRNode *_source)
{
    sources.push_back(_source);
}

//  Get the vector of source parameter nodes for the instruction.
std::vector<SourceParameterIRNode *> &InstructionIRNode::getSources()
{
    return sources;
}

//  Gets the destination parameter node for the instruction node.
DestinationParameterIRNode *InstructionIRNode::getDestination()
{
    return destination;
}

//  Sets the destination parameter nodoe for the instruction node.
void InstructionIRNode::setDestination(DestinationParameterIRNode *_destination)
{
    destination = _destination;
}


//  Declaration IR Node constructor.
DeclarationIRNode::DeclarationIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)    
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void DeclarationIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the instruction opcode.
D3DSHADER_INSTRUCTION_OPCODE_TYPE DeclarationIRNode::getOpcode()
{
    return static_cast<D3DSHADER_INSTRUCTION_OPCODE_TYPE>(d3dToken.dword & D3DSI_OPCODE_MASK);
}


//  Definition IR Node constructor.
DefinitionIRNode::DefinitionIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void DefinitionIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the instruction opcode.
D3DSHADER_INSTRUCTION_OPCODE_TYPE DefinitionIRNode::getOpcode()
{
    return static_cast<D3DSHADER_INSTRUCTION_OPCODE_TYPE>(d3dToken.dword & D3DSI_OPCODE_MASK);
}

//  Get the destination parameter node for the definition.
DestinationParameterIRNode *DefinitionIRNode::getDestination()
{
    return destination;
}

//  Set the destination parameter node for the definition.
void DefinitionIRNode::setDestination(DestinationParameterIRNode *_destination)
{
    destination = _destination;
}

//  End IR Node constructor.
EndIRNode::EndIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void EndIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}


//  Parameter IR node constructor.
ParameterIRNode::ParameterIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset), relativeAddressing(0)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void ParameterIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the register number for the parameter.
unsigned int ParameterIRNode::getNRegister()
{
    return d3dToken.dword & D3DSP_REGNUM_MASK;
}

//  Get the address mode for the paramter.
D3DSHADER_ADDRESSMODE_TYPE ParameterIRNode::getAddressMode()
{
    return static_cast<D3DSHADER_ADDRESSMODE_TYPE>(d3dToken.dword & D3DSHADER_ADDRESSMODE_MASK);
}

//  Get register type.
D3DSHADER_PARAM_REGISTER_TYPE ParameterIRNode::getRegisterType()
{
    // Register type is splitted in two parts
    return  static_cast<D3DSHADER_PARAM_REGISTER_TYPE>(((d3dToken.parameter.regTypeHi << 3) | (d3dToken.parameter.regTypeLo)));
}

//  Get relative addressing node.
RelativeAddressingIRNode *ParameterIRNode::getRelativeAddressing()
{
    return relativeAddressing;
}

//  Set Relative addressing node.
void ParameterIRNode::setRelativeAddressing(RelativeAddressingIRNode *_relAddr)
{
    relativeAddressing = _relAddr;
}

//  Source Parameter IR node constructor.
SourceParameterIRNode::SourceParameterIRNode(D3DToken _d3dtk, unsigned int _offset) :
    ParameterIRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void SourceParameterIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the swizzle for the selected component.
DWORD SourceParameterIRNode::getSwizzle(unsigned int component)
{
    return d3dToken.dword & (3<< (D3DVS_SWIZZLE_SHIFT + component * 2));
}

//  Get source modifier.
D3DSHADER_PARAM_SRCMOD_TYPE SourceParameterIRNode::getModifier()
{
    return static_cast<D3DSHADER_PARAM_SRCMOD_TYPE>(d3dToken.dword & D3DSP_SRCMOD_MASK);
}

//  Relative addressign IR node constructor.
RelativeAddressingIRNode::RelativeAddressingIRNode(D3DToken _d3dtk, unsigned int _offset) :
    ParameterIRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void RelativeAddressingIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the index for the component of the address register to be used for addressing.
unsigned int RelativeAddressingIRNode::getComponentIndex()
{
    return (d3dToken.dword & 0x00030000) >> 16;
}

//  Destination Parameter IR Node constructor.
DestinationParameterIRNode::DestinationParameterIRNode(D3DToken _d3dtk, unsigned int _offset) :
    ParameterIRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void DestinationParameterIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the destination parameter write mask.
DWORD DestinationParameterIRNode::getWriteMask()
{
    return d3dToken.dword & 0x000F0000;
}

//  Get the destination parameter modifier.
DWORD DestinationParameterIRNode::getModifier()
{
    return d3dToken.dword & D3DSP_DSTMOD_MASK;
}

//  Get the destination parameter shift scale.
DWORD DestinationParameterIRNode::getShiftScale()
{
    return d3dToken.dword & D3DSP_DSTSHIFT_MASK;
}

//  Bool IR Node constructor.
BoolIRNode::BoolIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)    
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void BoolIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the boolean value from the token.
bool BoolIRNode::getValue()
{
    DWORD token = getToken().dword;
    
    return (token != 0);
}

//  Float point IR node constructor.
FloatIRNode::FloatIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void FloatIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the float point value from the token.
float FloatIRNode::getValue()
{
    float result;
    *((DWORD *) &result) = getToken().dword;
    return result;
}


//  Integer IR Node constructor.
IntegerIRNode::IntegerIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void IntegerIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the integer value from the node.
int IntegerIRNode::getValue()
{
    return static_cast<int>(getToken().dword);
}

//  Version IR Node constructor.
VersionIRNode::VersionIRNode(D3DToken _d3dtk, unsigned int _offset) :
    IRNode(_d3dtk, _offset)
{
    //  Nothing to do.
}

//  Access function for visitor classes.
void VersionIRNode::accept(IRVisitor *v)
{
    v->visit(this);
}

//  Get the shader version from the node.
DWORD VersionIRNode::getVersion()
{
    return d3dToken.dword & 0x0000FFFF;
}

//  Get the shader type from the node.
ShaderType VersionIRNode::getType()
{
    return (d3dToken.version.type == 0xFFFF ? PIXEL_SHADER : VERTEX_SHADER);
}

